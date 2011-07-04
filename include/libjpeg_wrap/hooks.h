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

#ifndef HOOKS_H
#define HOOKS_H
#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>
#include <jerror.h>


#undef SHLIBJPEG_API
#define SHLIBJPEG_API(ret, func, ...) ret (*func)(__VA_ARGS__);

typedef struct {
	SHLIBJPEG_API(void, unimplemented, void);
	#include "entries.in"
} hooks_t;
#undef SHLIBJPEG_API

#define CALL_RETURN_API_FUNC(_func, ...) \
		return active_hooks->_func(__VA_ARGS__);

#define CALL_API_FUNC(_func, ...) active_hooks->_func(__VA_ARGS__);

void libjpeg_unimplented(void);
void jpumode_error(void);

#endif
