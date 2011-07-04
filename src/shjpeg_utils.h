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

#ifndef __utils_h__
#define __utils_h__

/**
 * \file utils.h
 *
 * Utilities
 */

#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#define D_ERROR(s, x...) \
    { if (context->verbose) fprintf(stderr, s "\n", ## x); }

#define D_DERROR(r, s, x...) \
    { if (context->verbose) \
	    fprintf(stderr, s " - return value=%d\n", ## x, r); }

#define D_PERROR(s, x...) \
    { if (context->verbose) \
	    fprintf(stderr, s " - %s.\n", ## x, strerror(errno)); }

#ifdef SHJPEG_DEBUG

#  define D_BUG(s, x...)	{ if (context->verbose) \
					fprintf( stderr, s "\n", ## x ); }
#  define D_INFO(s, x...)	{ if (context->verbose) \
					fprintf( stderr, s "\n", ## x ); }
#  define D_ONCE(s, x...) \
    { static int once = 1; \
      if (once-- > 0) fprintf( stderr, s "\n", ## x ); }
#  define D_DEBUG_AT(d, s, x...)   \
	if (context->verbose) \
		fprintf( stderr, "libshjpeg: %s - " s "\n", __FUNCTION__, ## x )
#  define D_ASSERT(exp)  assert(exp)
#  define D_UNIMPLEMENTED()  \
    fprintf( stderr, "Unimplemented %s!\n", __FUNCTION__ )

#else

#  define D_BUG(x...)
#  define D_INFO(x...)
#  define D_ONCE(x...)
#  define D_DEBUG_AT(d,x...)
#  define D_ASSERT(exp)
#  define D_UNIMPLEMENTED()

#endif

#define _PAGE_SIZE (getpagesize())
#define _PAGE_ALIGN(len) (((len) + _PAGE_SIZE - 1) & ~(_PAGE_SIZE - 1))

#define direct_page_align(a)	_PAGE_ALIGN(a)	// deprecated

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

/*
 * register access
 */

static inline u32 shjpeg_getreg32(void *base, u32 address)
{
	return *(volatile u32 *) (base + address);
}

static inline void shjpeg_setreg32(void *base, u32 address, u32 value)
{
	*(volatile u32 *) (base + address) = value;
}

#endif
