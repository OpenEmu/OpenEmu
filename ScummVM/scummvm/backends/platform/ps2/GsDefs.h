/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef __GSDEFS_H__
#define __GSDEFS_H__

#include "sysdefs.h"

// Gs2dScreen defines:

#define PAL_NTSC_FLAG		(*(volatile uint8*)0x1FC7FF52)

#define GS_PMODE			*((volatile uint64*)0x12000000)
#define GS_CSR				*((volatile uint64*)0x12001000)
#define GS_DISPFB1			*((volatile uint64*)0x12000070)
#define GS_DISPLAY1			*((volatile uint64*)0x12000080)
#define GS_BGCOLOUR			*((volatile uint64*)0x120000E0)

enum GS_CSR_FIELDS {
    CSR_SIGNAL = 1 << 0,
	CSR_FINISH = 1 << 1,
	CSR_HSYNC  = 1 << 2,
	CSR_VSYNC  = 1 << 3,
	CSR_FLUSH  = 1 << 8,
	CSR_RESET  = 1 << 9
};

#define GS_SET_PMODE(readC1, readC2, alphaSel, alphaOut, alphaBlend, alphaFixed) \
	((readC1) | ((readC2) << 1) | ((alphaSel) << 5) | ((alphaOut) << 6) | ((alphaBlend) << 7) | ((alphaFixed) << 8))

#define GS_SET_DISPLAY(width, height, xpos, ypos) \
	(((uint64)(height - 1) << 44) | ((uint64)0x9FF << 32) | \
	((((2560 + (width - 1)) / width) - 1)<<23) | \
	(ypos << 12) | (xpos * (2560 / width)))

#define GS_SET_DISPFB(frameBufPtr, frameBufWidth, psm) \
	(((frameBufPtr) / 8192) | (((frameBufWidth) / 64) << 9) | ((psm) << 15))

#define GS_RGBA(r, g, b, a) \
	((r) | ((g) << 8) | ((b) << 16) | ((a) << 24))

//DmaPipe defines:

enum GsRegs {
	GPR_PRIM = 0x00,	// Select and configure current drawing primitive
	GPR_RGBAQ,			// Setup current vertex color
	GPR_ST,				// ...
	GPR_UV,				// Specify Vertex Texture Coordinates
	GPR_XYZF2,			// Set vertex coordinate
	GPR_XYZ2,			// Set vertex coordinate and 'kick' drawing
	GPR_TEX0_1,			// Texture Buffer Setup (Context 1)
	GPR_TEX0_2,			// Texture Buffer Setup (Context 2)
	GPR_CLAMP_1,		// ...
	GPR_CLAMP_2,		// ...
	GPR_FOG,			// ...

	GPR_XYZF3 = 0x0C,	// ...
	GPR_XYZ3,			// ...

	GPR_TEX1_1 = 0x14,	// ...
	GPR_TEX1_2,			// ...
	GPR_TEX2_1,			// ...
	GPR_TEX2_2,			// ...
	GPR_XYOFFSET_1,		// Mapping from Primitive to Window coordinate system (Context 1)
	GPR_XYOFFSET_2,		// Mapping from Primitive to Window coordinate system (Context 2)
	GPR_PRMODECONT,		// ...
	GPR_PRMODE,			// ...
	GPR_TEXCLUT,		// ...

	GPR_SCANMSK	= 0x22,	// ...

	GPR_MIPTBP1_1 = 0x34,	// ...
	GPR_MIPTBP1_2,		// ...
	GPR_MIPTBP2_1,		// ...
	GPR_MIPTBP2_2,		// ...

	GPR_TEXA = 0x3b,	// ...

	GPR_FOGCOL = 0x3d,	// ...

	GPR_TEXFLUSH = 0x3f,// Write to this register before using newly loaded texture
	GPR_SCISSOR_1,		// Setup clipping rectangle (Context 1)
	GPR_SCISSOR_2,		// Setup clipping rectangle (Context 2)
	GPR_ALPHA_1,		// Setup Alpha Blending Parameters (Context 1)
	GPR_ALPHA_2,		// Setup Alpha Blending Parameters (Context 2)
	GPR_DIMX,			// ...
	GPR_DTHE,			// ...
	GPR_COLCLAMP,		// ...
	GPR_TEST_1,			// ...
	GPR_TEST_2,			// ...
	GPR_PABE,			// ...
	GPR_FBA_1,			// ...
	GPR_FBA_2,			// ...
	GPR_FRAME_1,		// Frame buffer settings (Context 1)
	GPR_FRAME_2,		// Frame buffer settings (Context 2)
	GPR_ZBUF_1,			// ...
	GPR_ZBUF_2,			// ...
	GPR_BITBLTBUF,		// Setup Image Transfer Between EE and GS
	GPR_TRXPOS,			// Setup Image Transfer Coordinates
	GPR_TRXREG,			// Setup Image Transfer Size
	GPR_TRXDIR,			// Set Image Transfer Directon + Start Transfer
	GPR_HWREG,

	GPR_SIGNAL = 0x60,
	GPR_FINISH,
	GPR_LABEL
};

enum PrimTypes {
	PR_POINT = 0,
	PR_LINE,
	PR_LINESTRIP,
	PR_TRIANGLE,
	PR_TRIANGLESTRIP,
	PR_TRIANGLEFAN,
	PR_SPRITE
};

#define GS_PSMCT32		0x00
#define GS_PSMCT24		0x01
#define GS_PSMCT16		0x02
#define GS_PSMCT16S		0x0A
#define GS_PSMT8		0x13
#define GS_PSMT4		0x14
#define GS_PSMT4HL		0x24
#define GS_PSMT4HH		0x2C
#define GS_PSMT8H		0x1B

/*#define GS_SET_BITBLTBUF(sbp, sbw, spsm, dbp, dbw, dpsm) \
	((uint64)(sbp)         | ((uint64)(sbw) << 16) | \
	((uint64)(spsm) << 24) | ((uint64)(dbp) << 32) | \
	((uint64)(dbw) << 48)  | ((uint64)(dpsm) << 56))*/

#define GS_SET_SRC_BLTBUF(sbp, sbw, spsm) \
	((sbp) | ((uint64)sbw << 16) | ((uint64)spsm << 24))

#define GS_SET_DEST_BLTBUF(dbp, dbw, dpsm) \
	(((uint64)(dbp) << 32) | ((uint64)(dbw) << 48)  | ((uint64)(dpsm) << 56))

#define GS_SET_SRC_TRXPOS(ssax, ssay) \
	((ssax) | ((uint64)ssay << 16))

#define GS_SET_DEST_TRXPOS(dsax, dsay) \
	(((uint64)(dsax) << 32) | ((uint64)(dsay) << 48))

#define GS_SET_TRXREG(rrw, rrh) \
	((uint64)(rrw) | ((uint64)(rrh) << 32))

#define GS_SET_TEXA(ta0, aem, ta1) \
	((uint64)(ta0) | ((uint64)(aem) << 15) | ((uint64)(ta1) << 32))

#define GS_SET_TEX0(tbp, tbw, psm, tw, th, tcc, tfx, cbp, cpsm, csm, csa, cld) \
	((uint64)(tbp)         | ((uint64)(tbw) << 14) | ((uint64)(psm) << 20)  | ((uint64)(tw) << 26) | \
	((uint64)(th) << 30)   | ((uint64)(tcc) << 34) | ((uint64)(tfx) << 35)  | ((uint64)(cbp) << 37) | \
	((uint64)(cpsm) << 51) | ((uint64)(csm) << 55) | ((uint64)(csa) << 56)  | ((uint64)(cld) << 61))

#define GIF_SET_TAG(nloop, eop, pre, prim, flg, nreg) \
	( (uint64)(nloop) | ((uint64)(eop)<<15) | ((uint64)(pre) << 46) | \
	((uint64)(prim)<<47) | ((uint64)(flg)<<58) | ((uint64)(nreg)<<60) )

#define GS_SET_UV(u, v) ((uint64)(u) | ((uint64)(v) << 16))

#define GS_SET_XYZ(x, y, z) \
	((uint64)(x) | ((uint64)(y) << 16) | ((uint64)(z) << 32))

#define GS_SET_XYOFFSET(ofx, ofy) ((uint64)(ofx) | ((uint64)(ofy) << 32))

#define GS_SET_SCISSOR(scax0, scax1, scay0, scay1) \
	( (uint64)(scax0) | ((uint64)(scax1) << 16) | ((uint64)(scay0) << 32) | ((uint64)(scay1) << 48) )

#define GS_SET_FRAME(fbp, fbw, psm, fbmask) \
	( (uint64)(fbp) | (uint64)((uint64)(fbw) << 16) | (uint64)((uint64)(psm) << 24) | (uint64)((uint64)(fbmask) << 32) )

#define GS_SET_TEST(ate, atst, aref, afail, date, datm, zte, ztst) \
	( (uint64)(ate)         | ((uint64)(atst) << 1) | ((uint64)(aref) << 4)  | ((uint64)(afail) << 12) | \
	((uint64)(date) << 14) | ((uint64)(datm) << 15) | ((uint64)(zte) << 16)  | ((uint64)(ztst) << 17) )

#define FILTER_NEAREST 0
#define FILTER_LINEAR 1

#define GS_SET_TEX1(lcm, mxl, mmag, mmin, mtba, l, k) \
	((uint64)(lcm) | ((uint64)(mxl) << 2)  | ((uint64)(mmag) << 5) | ((uint64)(mmin) << 6) | \
	((uint64)(mtba) << 9) | ((uint64)(l) << 19) | ((uint64)(k) << 32))

enum AlphaBlendColor {
	SOURCE_COLOR = 0,
	DEST_COLOR,
	ZERO_COLOR
};

enum AlphaBlendAlpha {
	SOURCE_ALPHA = 0,
	DEST_ALPHA,
    FIXED_ALPHA
};

#define GS_SET_ALPHA(a, b, c, d, fix) \
	((uint64)(a) | ((uint64)(b)<<2) | ((uint64)(c)<<4) | ((uint64)(d)<<6) | ((uint64)(fix)<<32))

#define GS_SET_PRIM(prim, iip, tme, fge, abe, aa1, fst, ctxt, fix) \
	((uint64)(prim)      | ((uint64)(iip) << 3)  | ((uint64)(tme) << 4) | \
	((uint64)(fge) << 5) | ((uint64)(abe) << 6)  | ((uint64)(aa1) << 7) | \
	((uint64)(fst) << 8) | ((uint64)(ctxt) << 9) | ((uint64)(fix) << 10))

#define GS_SET_COLQ(c) (0x3f80000000000000 | c)

#endif // __GSDEFS_H__
