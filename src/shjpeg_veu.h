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

#ifndef __shjpeg_veu_h__
#define __shjpeg_veu_h__

#include "shjpeg_regs.h"
#include "shjpeg_utils.h"

typedef struct {
	u32 width;
	u32 height;
	u32 pitch;
	u32 yaddr;
	u32 caddr;
} shjpeg_veu_plane_t;

typedef struct {
	shjpeg_veu_plane_t src;	/* source plane setting */
	shjpeg_veu_plane_t dst;	/* destination plane setting */
	u32 vbssr;		/* # of lines for bundle read mode */
	u32 vtrcr;		/* transform register */
	u32 venhr;
	u32 vfmcr;
	u32 vapcr;		/* chroma key */
	u32 vswpr;		/* swap register */
} shjpeg_veu_t;


static inline u32
shjpeg_veu_getreg32(shjpeg_internal_t * data, u32 address)
{
	D_ASSERT(address < data->veu_size);

	return *(volatile u32 *) (data->veu_base + address);
}

static inline void
shjpeg_veu_setreg32(shjpeg_internal_t * data, u32 address, u32 value)
{
	D_ASSERT(address < data->veu_size);

	*(volatile u32 *) (data->veu_base + address) = value;

#ifdef SHJPEG_DEBUG
	{
		shjpeg_context_t *context = data->context;
		D_INFO("%s: written %08x(%08x) at %s(%08x)",
		       __FUNCTION__, value, shjpeg_veu_getreg32(data,
								address),
		       veu_reg_str[address >> 2], address);
	}
#endif
}

/* external function */
int shjpeg_veu_init(shjpeg_internal_t * data, shjpeg_veu_t * veu);
void shjpeg_veu_set_dst_jpu(shjpeg_internal_t *);
void shjpeg_veu_set_src_jpu(shjpeg_internal_t *);
void shjpeg_veu_set_src(shjpeg_internal_t *, u32, u32);
void shjpeg_veu_start(shjpeg_internal_t *, int);
void shjpeg_veu_stop(shjpeg_internal_t *);

#endif				/* !__shjpeg_jpu_h__ */
