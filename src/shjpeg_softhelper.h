/*
 * libshjpeg: A library for controlling SH-Mobile JPEG hardware codec
 *
 * Copyright (C) 2009 IGEL Co.,Ltd.
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

#include "shjpeg_internal.h"

#define BY_WORD
#define USE_CACHED
#define CONV_CHUNK_SIZE 4

#ifdef BY_WORD
#	define soft_toYCbCr soft_toYCbCr_byword
#	define soft_fromYCbCr soft_fromYCbCr_byword
#else
#	define soft_toYCbCr soft_toYCbCr_bybyte
#	define soft_fromYCbCr soft_fromYCbCr_bybyte
#endif

typedef struct {
	size_t          mapbuflen;
	int             mapfd;
	void            *mapaddr;
} vmap_data_t;

void
free_frame_buffer_virtual(vmap_data_t *mdata);

int
get_frame_buffer_virtual(shjpeg_internal_t * data,
		shjpeg_context_t * context,
		vmap_data_t * mdata,
		shjpeg_pixelformat format,
		unsigned long phys);

void
soft_get_src_jpu(shjpeg_internal_t * data, void **ydata, void **cdata);

int
soft_toYCbCr_byword(shjpeg_internal_t * data,
	     shjpeg_context_t * context,
	     unsigned char *src_ydata,
	     unsigned char *src_cdata,
	     unsigned char *out_bufffer, int lines);

int
soft_fromYCbCr_byword(shjpeg_internal_t * data,
	       shjpeg_context_t * context,
	       unsigned char *dst_ydata,
	       unsigned char *dst_cdata,
	       unsigned char *in_buffer, int lines);
int
soft_toYCbCr_bybyte(shjpeg_internal_t * data,
	     shjpeg_context_t * context,
	     unsigned char *src_ydata,
	     unsigned char *src_cdata,
	     unsigned char *out_bufffer, int lines);
int
soft_fromYCbCr_bybyte(shjpeg_internal_t * data,
	       shjpeg_context_t * context,
	       unsigned char *dst_ydata,
	       unsigned char *dst_cdata,
	       unsigned char *in_buffer, int lines);
