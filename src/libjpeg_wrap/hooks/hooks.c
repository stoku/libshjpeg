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

