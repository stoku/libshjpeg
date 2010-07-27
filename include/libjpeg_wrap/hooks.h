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
