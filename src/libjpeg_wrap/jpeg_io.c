/*
 * libshjpeg: A library for controlling SH-Mobile JPEG hardware codec
 *
 * Copyright (C) 2010 IGEL Co.,Ltd.
 * Copyright (C) 2008,2009 Renesas Technology Corp.
 * Copyright (C) 2008 Denis Oliver Kropp
 *
 * This library is dual licensed.
 * You are free to use this library under either the MIT or
 * the GNU LGPL version 2 license.
 *
 * For more information please refer to the licensing files
 * in the root directory of this library package.
 *
 * GNU LGPL license: COPYING_LGPL
 * MIT license: COPYING_MIT
 */

#include "libmessage.h"
#include "jpeg_io.h"
#include "override.h"
#include <string.h>
#include <stdlib.h>

boolean cache_input_buffer(j_decompress_ptr cinfo)
{
	buffer_cache_context_t *cache_con;
	cinfo_context_t *ctx = get_cinfo_context((j_common_ptr) cinfo);

	if (!ctx)
		ERREXIT(cinfo, SHJMSG_INVALID_CONTEXT);

	cache_con = &ctx->cache_con;
	if (cache_con->current_start) {
		void *temp_buf = malloc(cache_con->total_buffer_size +
					cache_con->last_buffer_size);
		if (cache_con->buffer_cache) {
			memcpy(temp_buf, cache_con->buffer_cache,
			       cache_con->total_buffer_size);
			free(cache_con->buffer_cache);
		}
		memcpy(temp_buf + cache_con->total_buffer_size,
		       cache_con->current_start, cache_con->last_buffer_size);
		cache_con->buffer_cache = cache_con->cache_read = temp_buf;
		cache_con->total_buffer_size += cache_con->last_buffer_size;
	}
	cache_con->fill_buffer_function(cinfo);
	cache_con->last_buffer_size = cinfo->src->bytes_in_buffer;
	cache_con->current_start = cache_con->current_read =
	    (void *) cinfo->src->next_input_byte;
	return TRUE;
}

/*Streams operations*/
/*Using libjpeg src/dest callback functions to access data via JPU
  library */

int jpeg_src_init(void *private_data)
{
	cinfo_context_t *ctx = (cinfo_context_t *) private_data;
	j_decompress_ptr cinfo = (j_decompress_ptr) ctx->cinfo;

	cinfo->src->init_source(cinfo);
	return 0;
}

void jpeg_src_finalize(void *private_data)
{
	cinfo_context_t *ctx = (cinfo_context_t *) private_data;
	j_decompress_ptr cinfo = (j_decompress_ptr) ctx->cinfo;

	cinfo->src->term_source(cinfo);
}

int
jpeg_src_read_header(void *private_data, size_t * n_bytes, void *dataptr)
{
	cinfo_context_t *ctx = (cinfo_context_t *) private_data;
	j_decompress_ptr cinfo = (j_decompress_ptr) ctx->cinfo;
	shjpeg_context_t *context = ctx->context;
	buffer_cache_context_t *cache_con;
	cache_con = &ctx->cache_con;

	size_t copybytes = 0, copied, local_n_bytes = *n_bytes;

	if (cache_con->total_buffer_size) {
		copybytes = *n_bytes > cache_con->total_buffer_size ?
		    cache_con->total_buffer_size : *n_bytes;
		memcpy(dataptr, cache_con->cache_read, copybytes);
		cache_con->total_buffer_size -= copybytes;
		dataptr += copybytes;
		if (!cache_con->total_buffer_size) {
			free(cache_con->buffer_cache);
			cache_con->buffer_cache = NULL;
			cache_con->cache_read = NULL;
		} else {
			cache_con->cache_read += copybytes;
			return 0;
		}
		local_n_bytes -= copybytes;
	}
	copied = cache_con->last_buffer_size - cinfo->src->bytes_in_buffer;
	copybytes = local_n_bytes > copied ? copied : local_n_bytes;

	memcpy(dataptr, cache_con->current_read, copybytes);
	cache_con->last_buffer_size -= copybytes;
	dataptr += copybytes;

	if (cache_con->last_buffer_size == cinfo->src->bytes_in_buffer) {
		cache_con->current_start = NULL;
		cache_con->current_read = NULL;
		cache_con->last_buffer_size = 0;
		local_n_bytes -= copybytes;
		copied = *n_bytes - local_n_bytes;
		jpeg_src_read(private_data, &local_n_bytes, dataptr);
		*n_bytes = copied + local_n_bytes;
		context->sops->read = jpeg_src_read;
	} else {
		cache_con->current_read += copybytes;
		return 0;
	}
	return 0;
}
/***********************
* jpeg_src_read - Read JPEG data using the registered libjpeg callbacks
*
*
* Since the libjpeg callbacks do not return any end-of-file information 
* we have to look for the EOI tag in the JPEG stream.  This should only
* occur at the end of a buffer, but we have to check the last 2 bytes of 
* every buffer.
*
* Buffer boundary aligned EOI
* ===========================
* End-of-file is indicated to the hardware routine whenever
* less than n_bytes is returned from this call. If the
* EOI is perfectly aligned on the buffer boundary, the end-of-file 
* will not be determined until the next call which will
* cause libjpeg std_io_src to issue a warning. We avoid this by
* padding an extra control byte at the end of this buffer, pushing the
* EOI into the next frame
************************/
int jpeg_src_read(void *private_data, size_t * n_bytes, void *dataptr)
{
	cinfo_context_t *ctx = (cinfo_context_t *) private_data;
	j_decompress_ptr cinfo = (j_decompress_ptr) ctx->cinfo;

	struct jpeg_source_mgr *src = cinfo->src;
	size_t datacnt = *n_bytes;
	JOCTET *bufend;
	if (src->bytes_in_buffer == 0) {
		src->fill_input_buffer(cinfo);
	}
	while (src->bytes_in_buffer <= datacnt) {
		bufend = (JOCTET *) src->next_input_byte +
		    src->bytes_in_buffer - 2;
		if (src->bytes_in_buffer == 0) {
			*n_bytes -= datacnt;
			return 0;
		} else if (src->bytes_in_buffer == 1) {
			if (*(bufend + 1) == JPEG_EOI) {
				*n_bytes -= (datacnt - src->bytes_in_buffer);
				datacnt = src->bytes_in_buffer;
				break;
			}
		} else if (*(bufend) == 0xFF && *(bufend + 1) == JPEG_EOI) {
			*n_bytes -= (datacnt - src->bytes_in_buffer);
			if (datacnt == src->bytes_in_buffer) {
				*(((JOCTET *) dataptr) + datacnt - 1) = 0xFF;
				datacnt--;
			} else {
				datacnt = src->bytes_in_buffer;
			}
			break;
		}
		memcpy(dataptr, src->next_input_byte,
		       src->bytes_in_buffer);
		datacnt -= src->bytes_in_buffer;
		dataptr += src->bytes_in_buffer;
		src->fill_input_buffer(cinfo);
	}
	memcpy(dataptr, src->next_input_byte, datacnt);
	src->bytes_in_buffer -= datacnt;
	src->next_input_byte += datacnt;
	return 0;
}

int jpeg_dest_init(void *private_data)
{
	cinfo_context_t *ctx = (cinfo_context_t *) private_data;
	j_compress_ptr cinfo = (j_compress_ptr) ctx->cinfo;

	cinfo->dest->init_destination(cinfo);
	return 0;
}

void jpeg_dest_finalize(void *private_data)
{
	cinfo_context_t *ctx = (cinfo_context_t *) private_data;
	j_compress_ptr cinfo = (j_compress_ptr) ctx->cinfo;

	cinfo->dest->term_destination(cinfo);
}

int jpeg_dest_write(void *private_data, size_t * n_bytes, void *dataptr)
{
	cinfo_context_t *ctx = (cinfo_context_t *) private_data;
	j_compress_ptr cinfo = (j_compress_ptr) ctx->cinfo;

	struct jpeg_destination_mgr *dest = cinfo->dest;
	int datacnt = *n_bytes;
	while (cinfo->dest->free_in_buffer < datacnt) {
		memcpy(dest->next_output_byte, dataptr,
		       dest->free_in_buffer);
		datacnt -= dest->free_in_buffer;
		dataptr += dest->free_in_buffer;
		dest->empty_output_buffer(cinfo);
	}
	memcpy(dest->next_output_byte, dataptr, datacnt);
	dest->free_in_buffer -= datacnt;
	return TRUE;
}
