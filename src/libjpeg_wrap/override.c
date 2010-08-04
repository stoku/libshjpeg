/*
 * libshjpeg: A library for controlling SH-Mobile JPEG hardware codec
 *
 * Copyright (C) 2010 IGEL Co.,Ltd.
 * Copyright (C) 2008,2009 Renesas Technology Corp.
 * Copyright (C) 2008 Denis Oliver Kropp
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA  02110-1301 USA
 */

#include <string.h>
#include "jpeg_io.h"
#include "override.h"
#include "libmessage.h"
#include "shjpeg/shjpeg.h"
#include "shjpeg/shjpeg_types.h"

extern hooks_t libjpeg_hooks, *active_hooks, withjpu_hooks, jpumode_hooks;

/*Keep one global context for now.  Will be replaced with a
 *hash lookup based on cinfo pointer in the future*/
cinfo_context cictxt;
shjpeg_context_t *context  = NULL;
buffer_cache_context_t cache_con = {0};

shjpeg_sops jpeg_src_ops = {
    .init = jpeg_src_init,
    .read = jpeg_src_read,
    .write = NULL,
    .finalize = jpeg_src_finalize,
};

shjpeg_sops jpeg_dest_ops = {
    .init = jpeg_dest_init,
    .read = NULL,
    .write = jpeg_dest_write,
    .finalize = jpeg_dest_finalize,
};


static shjpeg_pixelformat get_shjpeg_pixelformat(J_COLOR_SPACE color_space) {
    switch(color_space) {
        case JCS_RGB:
            return SHJPEG_PF_RGB24;
#ifdef ANDROID_RGB
        case JCS_RGBA_8888:
            return SHJPEG_PF_RGB32;
        case JCS_RGB_565:
            return SHJPEG_PF_RGB16;
#endif
        case JCS_GRAYSCALE:
    //TODO: grayscale support
        case JCS_YCbCr:
    //TODO: YUV support
        default:
            return SHJPEG_PF_NONE;
    }
}
static boolean is_jpu_supported_decompress(j_decompress_ptr cinfo) {
        my_src_ptr src = (my_src_ptr) cinfo->src;
    /*If no file input specified revert to libjpeg*/
    if (!src) {
        return FALSE;
    }
    /*We do not support on-the-fly colormapping*/
    if (cinfo->output_components != cinfo->out_color_components)
        return FALSE;

    /*Check for a valid colour space*/
    if (get_shjpeg_pixelformat(cinfo->out_color_space) == SHJPEG_PF_NONE)
        return FALSE;

    /*Use shjpeg unless 4:4:4 colour mode*/
    if (cinfo->out_color_space == JCS_YCbCr &&
        (cinfo->comp_info[1].h_samp_factor ==
         cinfo->comp_info[0].h_samp_factor) &&
        (cinfo->comp_info[1].v_samp_factor ==
         cinfo->comp_info[0].v_samp_factor) &&
        (cinfo->comp_info[2].h_samp_factor ==
         cinfo->comp_info[0].h_samp_factor) &&
        (cinfo->comp_info[2].v_samp_factor ==
         cinfo->comp_info[0].v_samp_factor) )
            return FALSE;

    return TRUE;

}

void shjpeg_CreateDecompress(j_decompress_ptr cinfo, int version, size_t structsize) {
    /*double check the version again here, since we may be compiled
      against a different version from both the loaded library and the
      appication*/
        if (version != JPEG_LIB_VERSION) {
               cinfo->mem = NULL;
               ERREXIT2(cinfo, SHJMSG_BAD_VERSION, JPEG_LIB_VERSION, version);
        }
    libjpeg_hooks.jpeg_CreateDecompress(cinfo, version, structsize);
}

int shjpeg_read_header(j_decompress_ptr cinfo, boolean require_image) {
    int ret;

    cictxt.cache_con = &cache_con;
    cictxt.cinfo = (j_common_ptr) cinfo;

    cache_con.fill_buffer_function = cinfo->src->fill_input_buffer;
    cinfo->src->fill_input_buffer = cache_input_buffer;

    cache_con.total_buffer_size = 0;
    cache_con.last_buffer_size = cinfo->src->bytes_in_buffer;
    cache_con.current_start = cache_con.current_read =
        (void *) cinfo->src->next_input_byte;

    ret = libjpeg_hooks.jpeg_read_header(cinfo, require_image);
    cinfo->src->fill_input_buffer = cache_con.fill_buffer_function;
    jpeg_src_ops.read = jpeg_src_read_header;
    return ret;
}

boolean shjpeg_start_decompress(j_decompress_ptr cinfo) {
    shjpeg_pixelformat format;
    if (context && context->active_object)
        ERREXIT(cinfo, SHJMSG_INVALID_CONTEXT);

    if (!is_jpu_supported_decompress(cinfo)) {
        TRACEMS(cinfo, 1, SHJMSG_LIBJPEG_MODE);
        return libjpeg_hooks.jpeg_start_decompress(cinfo);
    }
    TRACEMS(cinfo, 1, SHJMSG_JPU_MODE);
    /*libjpeg should be initialized and the headers should
      have been read by this point, so we can skip over that
      part of the shjpeg_decode_init*/
    CALL_API_FUNC(jpeg_calc_output_dimensions,cinfo)

    context = shjpeg_init(0);
    if (!context) {
        TRACEMS(cinfo, 1, SHJMSG_LIBJPEG_MODE);
        return libjpeg_hooks.jpeg_start_decompress(cinfo);
    }
    context->cache_con = &cache_con;
    context->width = cinfo->output_width;
    context->height = cinfo->output_height;
    context->pitch = context->width * cinfo->out_color_components;
    context->pitch = (context->pitch + 7) & ~7; // 8 byte align

    context->mode444 = 0;

    /* True if 4:2:0 */
    context->mode420 =
        (cinfo->comp_info[1].h_samp_factor ==
        cinfo->comp_info[0].h_samp_factor / 2) &&
        (cinfo->comp_info[1].v_samp_factor ==
        cinfo->comp_info[0].v_samp_factor / 2) &&
        (cinfo->comp_info[2].h_samp_factor ==
        cinfo->comp_info[0].h_samp_factor / 2) &&
        (cinfo->comp_info[2].v_samp_factor ==
        cinfo->comp_info[0].v_samp_factor / 2);

    context->libjpeg_disabled=1;
    context->sops = &jpeg_src_ops;

    cictxt.cinfo = (j_common_ptr) cinfo;
    cictxt.context = context;
    context->private = (void *) &cictxt;

    format = get_shjpeg_pixelformat(cinfo->out_color_space);

    if (shjpeg_decode_run(context, format,
        SHJPEG_USE_DEFAULT_BUFFER, context->width,
        context->height, context->pitch) < 0) {
        //this is not a recoverable failure... abort
        shjpeg_decode_shutdown(context);
        shjpeg_shutdown(context);
        return libjpeg_hooks.jpeg_start_decompress(cinfo);
    }
    context->active_object = (j_common_ptr) cinfo;
    shjpeg_get_frame_buffer(context, &context->buffer.phys,
        &context->buffer.buffer, &context->buffer.size);
    active_hooks = &jpumode_hooks;
    return TRUE;
}

JDIMENSION shjpeg_read_scanlines(j_decompress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION max_lines) {
    int linecnt = 0;
    void *buffer;
    if ((j_common_ptr) cinfo != context->active_object) {
        ERREXIT(cinfo, SHJMSG_INVALID_CONTEXT);
    }
    buffer = context->buffer.buffer;
    max_lines = max_lines + cinfo->output_scanline > context->height ?
        context->height - cinfo->output_scanline : max_lines;
    for (linecnt = 0; linecnt < max_lines; linecnt++) {
        memcpy(scanlines[linecnt],
        buffer + (cinfo->output_scanline*context->pitch),
        context->width*cinfo->out_color_components);
        cinfo->output_scanline++;
    }
    return max_lines;
}

boolean shjpeg_finish_decompress(j_decompress_ptr cinfo) {
    if ((j_common_ptr)cinfo != context->active_object) {
        ERREXIT(cinfo, SHJMSG_INVALID_CONTEXT);
    }
    if (context->sops->finalize) {
        context->sops->finalize(context->private);
    }
    context->active_object = NULL;
    active_hooks = &withjpu_hooks;
    CALL_API_FUNC(jpeg_abort_decompress, cinfo)
    //unnecessary .. handled in shjpeg_abort_decompress
    //CALL_API_FUNC(jpeg_abort_decompress, &context->jpeg_decomp)
    return TRUE;
}

/*Compress Object functions*/

void shjpeg_CreateCompress(j_compress_ptr cinfo, int version, size_t structsize) {
    /*double check the version again here, since we may be compiled
      against a different version from both the loaded library and the
      appication*/
    if (version != JPEG_LIB_VERSION) {
        cinfo->mem = NULL;
        ERREXIT2(cinfo, SHJMSG_BAD_VERSION, JPEG_LIB_VERSION, version);
    }
    libjpeg_hooks.jpeg_CreateCompress(cinfo, version, structsize);
}

static boolean is_jpu_supported_compress(j_compress_ptr cinfo, boolean wat) {
    my_dest_ptr dest = (my_dest_ptr) cinfo->dest;
    /*If no file output specified*/
    if (!dest) {
        return FALSE;
    }
    if (!wat)
        return FALSE;
    if (get_shjpeg_pixelformat(cinfo->in_color_space) == SHJPEG_PF_NONE)
        return FALSE;
    return TRUE;
}
void shjpeg_start_compress(j_compress_ptr cinfo, boolean write_all_tables) {
    if (context && context->active_object)
        ERREXIT(cinfo, SHJMSG_INVALID_CONTEXT);

    if (!is_jpu_supported_compress(cinfo, write_all_tables))  {
        TRACEMS(cinfo, 1, SHJMSG_LIBJPEG_MODE);
        libjpeg_hooks.jpeg_start_compress (cinfo, write_all_tables);
        return;
    }
    TRACEMS(cinfo, 1, SHJMSG_JPU_MODE);
    context = shjpeg_init(0);
    if (!context) {
        TRACEMS(cinfo, 1, SHJMSG_LIBJPEG_MODE);
        libjpeg_hooks.jpeg_start_compress (cinfo, write_all_tables);
        return;
    }
    context->width = cinfo->image_width;
    context->height = cinfo->image_height;
    context->pitch = context->width * cinfo->input_components;
    context->pitch = (context->pitch + 7) & ~7; // 8 byte align

    context->libjpeg_disabled=1;

    context->sops = &jpeg_dest_ops;
    cictxt.cinfo = (j_common_ptr) cinfo;
    cictxt.context = context;

    context->private = (void *) &cictxt;

    context->active_object = (j_common_ptr) cinfo;
    shjpeg_get_frame_buffer(context, &context->buffer.phys,
        &context->buffer.buffer, &context->buffer.size);
    active_hooks = &jpumode_hooks;
}

JDIMENSION shjpeg_write_scanlines(j_compress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION num_lines) {
    int linecnt = 0;
    void *buffer;
    if ((j_common_ptr) cinfo != context->active_object) {
        ERREXIT(cinfo, SHJMSG_INVALID_CONTEXT);
    }
    buffer = context->buffer.buffer;
    num_lines = num_lines + cinfo->next_scanline > context->height ?
    context->height - cinfo->next_scanline : num_lines;
    for (linecnt = 0; linecnt < num_lines; linecnt++) {
        memcpy(buffer + (cinfo->next_scanline*context->pitch),
        scanlines[linecnt],
        context->width*cinfo->input_components);
        cinfo->next_scanline++;
    }
    return num_lines;
}

void shjpeg_finish_compress(j_compress_ptr cinfo) {
    shjpeg_pixelformat format;
    format = get_shjpeg_pixelformat(cinfo->in_color_space);
    if ((j_common_ptr) cinfo != context->active_object) {
        ERREXIT(cinfo, SHJMSG_INVALID_CONTEXT);
    }
    /*TODO: support arbitrary physical buffer*/
    if (shjpeg_encode(context, format,
        SHJPEG_USE_DEFAULT_BUFFER, context->width,
        context->height, context->pitch) < 0) {
        ERREXIT(cinfo, SHJMSG_COMPRESS_ERR);
    }
    if (context->sops->finalize) {
        context->sops->finalize(context->private);
    }
    active_hooks = &withjpu_hooks;
}

/*redefine the error message table as a concatenation of the standard
  errors and our newly defined ones*/

const char * const shjpeg_message_table[] = {
#define JMESSAGE(code,string)   string ,
#include <jerror.h>
#define JMESSAGE(code,string)   "SHJPEG: "string ,
#include "libmessage.h"
  NULL
};

struct jpeg_error_mgr *shjpeg_std_error(struct jpeg_error_mgr * err) {
    struct jpeg_error_mgr *ret;
    ret = libjpeg_hooks.jpeg_std_error(err);
    ret->jpeg_message_table = shjpeg_message_table;
    ret->last_jpeg_message = (int) SHJMSG_LASTMSGCODE-1;
    return ret;
}

/*jpeg_abort and jpeg_destroy can be called at
  any time, so we cannot make assumptions about the current state*/
void shjpeg_abort(j_common_ptr cinfo) {
    if (context && context->active_object &&
            context->active_object ==  cinfo) {
        if (cinfo->is_decompressor) {
            libjpeg_hooks.jpeg_abort_decompress
                (&context->jpeg_decomp);
        }
        context->active_object = NULL;
        active_hooks = &withjpu_hooks;
    }
    libjpeg_hooks.jpeg_abort(cinfo);
}

void shjpeg_destroy(j_common_ptr cinfo) {
    if (context && context->active_object &&
            context->active_object == cinfo) {
        if (cinfo->is_decompressor) {
            shjpeg_decode_shutdown(context);
        }
        shjpeg_shutdown(context);
        active_hooks = &withjpu_hooks;
        context = NULL;
    }
    libjpeg_hooks.jpeg_destroy(cinfo);
}

