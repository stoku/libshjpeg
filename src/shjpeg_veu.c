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

#include <stdio.h>
#include <unistd.h>

#include <shjpeg/shjpeg.h>
#include "shjpeg_internal.h"
#include "shjpeg_jpu.h"
#include "shjpeg_veu.h"

/*
 * Initialize VEU
 */

int shjpeg_veu_init(shjpeg_internal_t * data, shjpeg_veu_t * veu)
{
	/*
	 * Setup VEU for conversion/scaling (from line buffer to surface).
	 */

	/*
	 * 0. Reset
	 */

	/* reset VEU module */
	shjpeg_veu_setreg32(data, VEU_VBSRR, 0x00000100);

	/*
	 * 1. Set source details
	 */

	/* set source stride */
	shjpeg_veu_setreg32(data, VEU_VESWR, veu->src.pitch);

	/* set width and height */
	shjpeg_veu_setreg32(data, VEU_VESSR,
			    (veu->src.height << 16) | veu->src.width);

	/* set source Y address */
	shjpeg_veu_setreg32(data, VEU_VSAYR, veu->src.yaddr);

	/* set source C address */
	shjpeg_veu_setreg32(data, VEU_VSACR, veu->src.caddr);

	/* set lines to read during bundle mode */
	shjpeg_veu_setreg32(data, VEU_VBSSR, veu->vbssr);

	/*
	 * 2. Set destination details
	 */

	/* set destination stride */
	shjpeg_veu_setreg32(data, VEU_VEDWR, veu->dst.pitch);

	/* set destination Y address */
	shjpeg_veu_setreg32(data, VEU_VDAYR, veu->dst.yaddr);

	/* set destination C address */
	shjpeg_veu_setreg32(data, VEU_VDACR, veu->dst.caddr);

	/*
	 * 3. Set transformation related parameters
	 */

	/* set transformation parameter */
	shjpeg_veu_setreg32(data, VEU_VTRCR, veu->vtrcr);

	/* set resize register */
	shjpeg_veu_setreg32(data, VEU_VRFCR, 0);	/* XXX: no resize for now */
	shjpeg_veu_setreg32(data, VEU_VRFSR,
			    (veu->dst.height << 16) | veu->dst.width);

	/* edge enhancer */
	shjpeg_veu_setreg32(data, VEU_VENHR, veu->venhr);

	/* filter mode */
	shjpeg_veu_setreg32(data, VEU_VFMCR, veu->vfmcr);

	/* chroma-key */
	shjpeg_veu_setreg32(data, VEU_VAPCR, veu->vapcr);

	/* byte swap setting */
	shjpeg_veu_setreg32(data, VEU_VSWPR, veu->vswpr);

	/*
	 * 4. VEU3F work around
	 */
//    if (data->uio_caps & UIO_CAPS_VEU3F) {
//      shjpeg_veu_setreg32(data, VEU_VRPBR, 0x00000000);
	shjpeg_veu_setreg32(data, VEU_VRPBR, 0x00400040);
//    }

	/*
	 * 5. Enable interrupt
	 */
	shjpeg_veu_setreg32(data, VEU_VEIER, 0x00000101);

	return 0;
}

/*
 * Set JPU as the destination of VEU
 */

void shjpeg_veu_set_dst_jpu(shjpeg_internal_t * data)
{
#ifdef SHJPEG_DEBUG1
	u32 vdayr = shjpeg_veu_getreg32(data, VEU_VDAYR);
	u32 vdacr = shjpeg_veu_getreg32(data, VEU_VDACR);
#endif

	shjpeg_veu_setreg32(data, VEU_VDAYR,
			    (data->veu_linebuf) ?
			    shjpeg_jpu_getreg32(data, JPU_JIFESYA2) :
			    shjpeg_jpu_getreg32(data, JPU_JIFESYA1));
	shjpeg_veu_setreg32(data, VEU_VDACR,
			    (data->veu_linebuf) ?
			    shjpeg_jpu_getreg32(data, JPU_JIFESCA2) :
			    shjpeg_jpu_getreg32(data, JPU_JIFESCA1));

#ifdef SHJPEG_DEBUG1
	D_INFO("		-> SWAP, "
	       "VEU_VSAYR = %08x (%08x->%08x, %08x->%08x)",
	       shjpeg_veu_getreg32(data, VEU_VSAYR), vdayr,
	       shjpeg_veu_getreg32(data, VEU_VDAYR), vdacr,
	       shjpeg_veu_getreg32(data, VEU_VDACR));
#endif
}

/*
 * Set JPU as the source of VEU
 */

void shjpeg_veu_set_src_jpu(shjpeg_internal_t * data)
{
	u32 ydata, cdata;

	if (!data->veu_linebuf) {
		ydata = data->jpeg_lb1;
	} else {
		ydata = data->jpeg_lb2;
	}
	cdata = ydata + SHJPEG_JPU_LINEBUFFER_SIZE_Y;

	shjpeg_veu_setreg32(data, VEU_VSAYR, ydata);
	shjpeg_veu_setreg32(data, VEU_VSACR, cdata);
}

/*
 * Set VEU Source to the given address
 */

void shjpeg_veu_set_src(shjpeg_internal_t * data, u32 src_y, u32 src_c)
{
	shjpeg_veu_setreg32(data, VEU_VSAYR, src_y);
	shjpeg_veu_setreg32(data, VEU_VSACR, src_c);
}

/*
 * Start VEU
 */

void shjpeg_veu_start(shjpeg_internal_t * data, int bundle_mode)
{
	shjpeg_veu_setreg32(data, VEU_VESTR,
			    (bundle_mode) ? 0x101 : 0x001);
}

/*
 * Stop VEU
 */
void shjpeg_veu_stop(shjpeg_internal_t * data)
{
	/* Nothing */
}
