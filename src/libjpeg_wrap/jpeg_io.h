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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA	 02110-1301 USA
 */

#include <stdio.h>
#include "jpeglib.h"
int file_init (void *private_data);
int file_read (void *private_data, size_t *n_bytes, void *dataptr);
int file_write (void *private_data, size_t *n_bytes, void *dataptr);

/*read from and source using the libjpeg callback functions*/
int jpeg_src_init(void *private_data);
void jpeg_src_finalize(void *private_data);
int jpeg_src_read (void *private_data, size_t *n_bytes, void *dataptr);
int jpeg_src_read_header (void *private_data, size_t *n_bytes, void *dataptr);

/*write to any destination using the libjpeg callback functions*/
int jpeg_dest_init(void *private_data);
void jpeg_dest_finalize(void *private_data);
int jpeg_dest_write (void *private_data, size_t *n_bytes, void *dataptr);

boolean cache_input_buffer (j_decompress_ptr cinfo);
