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
#include "libjpeg_wrap/hooks.h"

#undef SHLIBJPEG_API
#define SHLIBJPEG_API(ret, func, ...) #func,
char const * const hook_names[] = {
	#include "libjpeg_wrap/entries.in"
	NULL
};
#undef SHLIBJPEG_API

void libjpeg_unimplented (void) {
	fprintf(stderr, "Unimplented library call atttmepted\n");
}
void jpumode_error (void) {
	fprintf(stderr, "State error.  Complete current JPEG before\n"
		        "attempting to start a new conversion\n");
}

