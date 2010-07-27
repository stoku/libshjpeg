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

#include "override.h"
#include <stdio.h>
#include "dlfcn.h"

hooks_t libjpeg_hooks, *active_hooks, withjpu_hooks, jpumode_hooks;
extern char const * const hook_names[];
void *lib_handle;

static void set_common_overrides(hooks_t *overrides) {
/*common functions*/
	overrides->jpeg_CreateDecompress = shjpeg_CreateDecompress;
	overrides->jpeg_CreateCompress = shjpeg_CreateCompress;

	overrides->jpeg_abort_compress = libjpeg_hooks.jpeg_abort_compress;
	overrides->jpeg_destroy_compress = libjpeg_hooks.jpeg_destroy_compress;
	overrides->jpeg_abort_decompress = libjpeg_hooks.jpeg_abort_decompress;
	overrides->jpeg_destroy_decompress = libjpeg_hooks.jpeg_destroy_decompress;
	overrides->jpeg_destroy = shjpeg_destroy;
	overrides->jpeg_abort = shjpeg_abort;

	overrides->jpeg_std_error = shjpeg_std_error;
}
static void set_withjpu_overrides(hooks_t *overrides) {
	overrides->jpeg_start_compress = shjpeg_start_compress;
	overrides->jpeg_start_decompress = shjpeg_start_decompress;
	overrides->jpeg_read_header = shjpeg_read_header;
	set_common_overrides(overrides);
}

static void set_jpumode_overrides(hooks_t *overrides) {
	overrides->jpeg_read_scanlines = shjpeg_read_scanlines;
	overrides->jpeg_write_scanlines = shjpeg_write_scanlines;
	overrides->jpeg_finish_compress = shjpeg_finish_compress;
	overrides->jpeg_finish_decompress = shjpeg_finish_decompress;
	set_common_overrides(overrides);
}

void __attribute__ ((constructor)) init_lib (void) {
	char const * const * hooklist = hook_names;
	void (**fn_handle)(void) = &libjpeg_hooks.unimplemented;
	void (**wjpu_handle)(void) = &withjpu_hooks.unimplemented;
	void (**jpumode_handle)(void) = &jpumode_hooks.unimplemented;
	lib_handle = dlopen("libjpeg.so",
		RTLD_NOW | RTLD_LOCAL);

	if (!lib_handle) {
		printf("cannot open library\n");
	}
	*fn_handle = *wjpu_handle = *jpumode_handle = &libjpeg_unimplented;
	fn_handle++;
	wjpu_handle++;
	jpumode_handle++;
	/*Load the symbols from the library*/
	while (*hooklist) {
		char const * fname = * hooklist;
		*fn_handle = *wjpu_handle = dlsym(lib_handle, fname);
		if (!*fn_handle) {
			*fn_handle = libjpeg_unimplented;
		}
		*jpumode_handle = jpumode_error;
		hooklist++;
		fn_handle++;
		wjpu_handle++;
		jpumode_handle++;
	}
	set_withjpu_overrides(&withjpu_hooks);
	set_jpumode_overrides(&jpumode_hooks);
	active_hooks = &withjpu_hooks;

}
void __attribute__ ((destructor)) exit_lib (void) {
	if (dlclose(lib_handle)) {
		printf("Error closing library\n");
	}
}
