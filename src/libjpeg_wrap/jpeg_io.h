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
