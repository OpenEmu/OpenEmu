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

#include "blitters.h"
#define CHARSET_MASK_TRANSPARENCY 253

//#define PERFECT_5_TO_4_RESCALING

namespace DS {

void asmDrawStripToScreen(int height, int width, byte const *text, byte const *src, byte *dst,
	int vsPitch, int vmScreenWidth, int textSurfacePitch) {

	if (height <= 0)
		height = 1;
	if (width < 4)
		return;

	width &= ~4;
//	src = (const byte *) (((int) (src)) & (~4));
//	dst = (byte *) (((int) (dst)) & (~4));
//	text = (const byte *) (((int) (text)) & (~4));

	asm (	"mov r5, %0\n"				// Height
			"yLoop:\n"
			"mov r3, #0\n"				// X pos

			"xLoop:\n"

			"ldr r4, [%2, r3]\n"		// Load text layer word
			"cmp r4, %5\n"
			"bne singleByteCompare\n"
			"ldr r4, [%3, r3]\n"
			"str r4, [%4, r3]\n"
			"add r3, r3, #4\n"
			"cmp r3, %1\n"				// x == width?
			"blt xLoop\n"

			"add %2, %2, %8\n"			// src += vs->pitch
			"add %3, %3, %6\n"			// dst += _vm->_screenWidth
			"add %4, %4, %7\n"			// text += _textSurface.pitch
			"sub r5, r5, #1\n"			// y -= 1
			"cmp r5, #0\n"				// y == 0?
			"bne yLoop\n"
			"b end\n"


			"singleByteCompare:\n"
			"ldrb r4, [%2, r3]\n"		// Load text byte
			"cmps r4, %5, lsr #24\n"	// Compare with mask
			"strneb r4, [%4, r3]\n"		// Store if not equal
			"ldreqb r4, [%3, r3]\n"		// Otherwise Load src byte
			"streqb r4, [%4, r3]\n"		// Store it
			"add r3, r3, #1\n"

			"ldrb r4, [%2, r3]\n"		// Load text byte
			"cmps r4, %5, lsr #24\n"	// Compare with mask
			"strneb r4, [%4, r3]\n"		// Store if not equal
			"ldreqb r4, [%3, r3]\n"		// Otherwise Load src byte
			"streqb r4, [%4, r3]\n"		// Store it
			"add r3, r3, #1\n"

			"ldrb r4, [%2, r3]\n"		// Load text byte
			"cmps r4, %5, lsr #24\n"	// Compare with mask
			"strneb r4, [%4, r3]\n"		// Store if not equal
			"ldreqb r4, [%3, r3]\n"		// Otherwise Load src byte
			"streqb r4, [%4, r3]\n"		// Store it
			"add r3, r3, #1\n"

			"ldrb r4, [%2, r3]\n"		// Load text byte
			"cmps r4, %5, lsr #24\n"	// Compare with mask
			"strneb r4, [%4, r3]\n"		// Store if not equal
			"ldreqb r4, [%3, r3]\n"		// Otherwise Load src byte
			"streqb r4, [%4, r3]\n"		// Store it
			"add r3, r3, #1\n"

			"cmps r3, %1\n"				// x == width?
			"blt xLoop\n"				// Repeat
			"add %2, %2, %8\n"			// src += vs->pitch
			"add %3, %3, %6\n"			// dst += _vm->_screenWidth
			"add %4, %4, %7\n"			// text += _textSurface.pitch
			"sub r5, r5, #1\n"			// y -= 1
			"cmp r5, #0\n"				// y == 0?
			"bne yLoop\n"

			"end:\n"
		: /* no output registers */
		: "r" (height), "r" (width), "r" (text), "r" (src), "r" (dst), "r" (CHARSET_MASK_TRANSPARENCY | (CHARSET_MASK_TRANSPARENCY << 8) | (CHARSET_MASK_TRANSPARENCY << 16) | (CHARSET_MASK_TRANSPARENCY << 24)),
			"r" (vsPitch), "r" (vmScreenWidth), "r" (textSurfacePitch)
		: "r5", "r3", "r4", "%2", "%3", "%4", "memory");
}



void asmCopy8Col(byte *dst, int dstPitch, const byte *src, int height) {
	asm("ands r0, %3, #1\n"
		 "addne %3, %3, #1\n"
		 "bne roll2\n"

		 "yLoop2:\n"
		 "ldr r0, [%2, #0]\n"
		 "str r0, [%0, #0]\n"
		 "ldr r0, [%2, #4]\n"
		 "str r0, [%0, #4]\n"
		 "add %0, %0, %1\n"
		 "add %2, %2, %1\n"
		 "roll2:\n"
		 "ldr r0, [%2, #0]\n"
		 "str r0, [%0, #0]\n"
		 "ldr r0, [%2, #4]\n"
		 "str r0, [%0, #4]\n"
		 "add %0, %0, %1\n"
		 "add %2, %2, %1\n"
		 "subs %3, %3, #2\n"
		 "bne yLoop2\n"

		: /* no output registers */
		: "r" (dst), "r" (dstPitch), "r" (src), "r" (height)
		: "r0", "%0", "%2", "%3");
}

static bool isDivBy5Ready = false;
static u32  DIV_BY_5[160];

void ComputeDivBy5TableIFN() {
    if (isDivBy5Ready)
        return;
    isDivBy5Ready = true;

    for (int i = 0; i < 160; ++i) {
        DIV_BY_5[i] = (2*i+5)/10;
    }
}

#ifdef PERFECT_5_TO_4_RESCALING
static inline void RescaleBlock_5x1555_To_4x1555( u16 s0, u16 s1, u16 s2, u16 s3, u16 s4,
                                                    u16 *dest) {
    u32 bs0 = s0 & 0x1F;
    u32 bs1 = s1 & 0x1F;
    u32 bs2 = s2 & 0x1F;
    u32 bs3 = s3 & 0x1F;
    u32 bs4 = s4 & 0x1F;

#if 0
    u32 gs0 = (s0 >> 5) & 0x1F;
    u32 gs1 = (s1 >> 5) & 0x1F;
    u32 gs2 = (s2 >> 5) & 0x1F;
    u32 gs3 = (s3 >> 5) & 0x1F;
    u32 gs4 = (s4 >> 5) & 0x1F;

    u32 rs0 = (s0 >> 10) & 0x1F;
    u32 rs1 = (s1 >> 10) & 0x1F;
    u32 rs2 = (s2 >> 10) & 0x1F;
    u32 rs3 = (s3 >> 10) & 0x1F;
    u32 rs4 = (s4 >> 10) & 0x1F;
#else
    // The compiler absolutely wants to use 0x1F as an immediate, which makes it unable to fold the shift during the and
    u32 mask = 0x1F;
    u32 gs0, gs1, gs2, gs3, gs4;
    asm("and %0, %2, %1, lsr #5" : "=r"(gs0) : "r"(s0), "r"(mask) : );
    asm("and %0, %2, %1, lsr #5" : "=r"(gs1) : "r"(s1), "r"(mask) : );
    asm("and %0, %2, %1, lsr #5" : "=r"(gs2) : "r"(s2), "r"(mask) : );
    asm("and %0, %2, %1, lsr #5" : "=r"(gs3) : "r"(s3), "r"(mask) : );
    asm("and %0, %2, %1, lsr #5" : "=r"(gs4) : "r"(s4), "r"(mask) : );
    u32 rs0, rs1, rs2, rs3, rs4;
    asm("and %0, %2, %1, lsr #10" : "=r"(rs0) : "r"(s0), "r"(mask) : );
    asm("and %0, %2, %1, lsr #10" : "=r"(rs1) : "r"(s1), "r"(mask) : );
    asm("and %0, %2, %1, lsr #10" : "=r"(rs2) : "r"(s2), "r"(mask) : );
    asm("and %0, %2, %1, lsr #10" : "=r"(rs3) : "r"(s3), "r"(mask) : );
    asm("and %0, %2, %1, lsr #10" : "=r"(rs4) : "r"(s4), "r"(mask) : );
#endif

    u32 rd0 = 4*rs0 +   rs1;
    u32 rd1 = 2*rs1 + rs1 + 2*rs2;
    u32 rd2 = 2*rs2 + 2*rs3 + rs3;
    u32 rd3 =   rs3 + 4*rs4;

    u32 gd0 = 4*gs0 +   gs1;
    u32 gd1 = 2*gs1 + gs1 + 2*gs2;
    u32 gd2 = 2*gs2 + 2*gs3 + gs3;
    u32 gd3 =   gs3 + 4*gs4;

    u32 bd0 = 4*bs0 +   bs1;
    u32 bd1 = 2*bs1 + bs1 + 2*bs2;
    u32 bd2 = 2*bs2 + 2*bs3 + bs3;
    u32 bd3 =   bs3 + 4*bs4;

#if 0
    // Offsetting for correct rounding
    rd0 = rd0*2+5; rd1 = rd1*2+5; rd2 = rd2*2+5; rd3 = rd3*2+5;
    gd0 = gd0*2+5; gd1 = gd1*2+5; gd2 = gd2*2+5; gd3 = gd3*2+5;
    bd0 = bd0*2+5; bd1 = bd1*2+5; bd2 = bd2*2+5; bd3 = bd3*2+5;

	rd0 = (rd0 * 51) >> 9; rd1 = (rd1 * 51) >> 9; rd2 = (rd2 * 51) >> 9; rd3 = (rd3 * 51) >> 9;
	gd0 = (gd0 * 51) >> 9; gd1 = (gd1 * 51) >> 9; gd2 = (gd2 * 51) >> 9; gd3 = (gd3 * 51) >> 9;
	bd0 = (bd0 * 51) >> 9; bd1 = (bd1 * 51) >> 9; bd2 = (bd2 * 51) >> 9; bd3 = (bd3 * 51) >> 9;
#else
	rd0 = DIV_BY_5[rd0]; rd1 = DIV_BY_5[rd1]; rd2 = DIV_BY_5[rd2]; rd3 = DIV_BY_5[rd3];
	gd0 = DIV_BY_5[gd0]; gd1 = DIV_BY_5[gd1]; gd2 = DIV_BY_5[gd2]; gd3 = DIV_BY_5[gd3];
	bd0 = DIV_BY_5[bd0]; bd1 = DIV_BY_5[bd1]; bd2 = DIV_BY_5[bd2]; bd3 = DIV_BY_5[bd3];
#endif

    u32 d10 = 0x80008000 | (rd1 << 26) | (gd1 << 21) | (bd1 << 16) | (rd0 << 10) | (gd0 << 5) | bd0;
    u32 d32 = 0x80008000 | (rd3 << 26) | (gd3 << 21) | (bd3 << 16) | (rd2 << 10) | (gd2 << 5) | bd2;

    ((u32 *)dest)[0] = d10;
    ((u32 *)dest)[1] = d32;
}
#else
static inline void RescaleBlock_5x1555_To_4x1555( u16 s0, u16 s1, u16 s2, u16 s3, u16 s4,
                                                    u16 *dest) {
    static const u32 MASK = 0x03E07C1F;

    u32 argbargbs0 = u32(s0) | (u32(s0) << 16);
    u32 argbargbs1 = u32(s1) | (u32(s1) << 16);
    u32 argbargbs2 = u32(s2) | (u32(s2) << 16);
    u32 argbargbs3 = u32(s3) | (u32(s3) << 16);
    u32 argbargbs4 = u32(s4) | (u32(s4) << 16);

    u32 grbs0 = argbargbs0 & MASK;
    u32 grbs1 = argbargbs1 & MASK;
    u32 grbs2 = argbargbs2 & MASK;
    u32 grbs3 = argbargbs3 & MASK;
    u32 grbs4 = argbargbs4 & MASK;

    u32 grbd0 = (3*grbs0 +   grbs1) >> 2;
    u32 grbd1 = (  grbs1 +   grbs2) >> 1;
    u32 grbd2 = (  grbs2 +   grbs3) >> 1;
    u32 grbd3 = (  grbs3 + 3*grbs4) >> 2;

    grbd0 &= MASK;
    grbd1 &= MASK;
    grbd2 &= MASK;
    grbd3 &= MASK;

    u32 d0 = grbd0 | (grbd0 >> 16);
    u32 d1 = grbd1 | (grbd1 >> 16);
    u32 d2 = grbd2 | (grbd2 >> 16);
    u32 d3 = grbd3 | (grbd3 >> 16);

    d0 &= 0xFFFF;
    d1 &= 0xFFFF;
    d2 &= 0xFFFF;
    d3 &= 0xFFFF;

    d0 |= 0x8000;
    d1 |= 0x8000;
    d2 |= 0x8000;
    d3 |= 0x8000;

    dest[0] = d0;
    dest[1] = d1;
    dest[2] = d2;
    dest[3] = d3;
}
#endif

static inline void RescaleBlock_5x8888_To_4x1555( u32 s0, u32 s1, u32 s2, u32 s3, u32 s4,
                                                    u16 *dest) {
    u32 d0 = 4*s0 +   s1;
    u32 d1 = 2*s1 +   s1 + 2*s2;

    u32 bd0 = (d0 << 24) >> 24;
    u32 bd1 = (d1 << 24) >> 24;
    u32 gd0 = (d0 << 16) >> 24;
    u32 gd1 = (d1 << 16) >> 24;
    u32 rd0 = (d0 >> 16);
    u32 rd1 = (d1 >> 16);

	rd0 = DIV_BY_5[rd0]; rd1 = DIV_BY_5[rd1];
	gd0 = DIV_BY_5[gd0]; gd1 = DIV_BY_5[gd1];
	bd0 = DIV_BY_5[bd0]; bd1 = DIV_BY_5[bd1];
    u32 d10 = 0x80008000 | (rd1 << 26) | (gd1 << 21) | (bd1 << 16) | (rd0 << 10) | (gd0 << 5) | bd0;
    ((u32 *)dest)[0] = d10;

    u32 d2 = 2*s2 + 2*s3 +   s3;
    u32 d3 =   s3 + 4*s4;

    u32 bd2 = (d2 << 24) >> 24;
    u32 bd3 = (d3 << 24) >> 24;
    u32 gd2 = (d2 << 16) >> 24;
    u32 gd3 = (d3 << 16) >> 24;
    u32 rd2 = (d2 >> 16);
    u32 rd3 = (d3 >> 16);

    rd2 = DIV_BY_5[rd2]; rd3 = DIV_BY_5[rd3];
    gd2 = DIV_BY_5[gd2]; gd3 = DIV_BY_5[gd3];
    bd2 = DIV_BY_5[bd2]; bd3 = DIV_BY_5[bd3];
    u32 d32 = 0x80008000 | (rd3 << 26) | (gd3 << 21) | (bd3 << 16) | (rd2 << 10) | (gd2 << 5) | bd2;

    ((u32 *)dest)[1] = d32;
}

// Can't work in place
#ifdef PERFECT_5_TO_4_RESCALING
static inline void Rescale_320xPAL8Scanline_To_256x1555Scanline(u16 *dest, const u8 *src, const u32 *palette) {
    ComputeDivBy5TableIFN();

    for (size_t i = 0; i < 64; ++i) {
        u32 s0 = palette[src[5*i+0]];
        u32 s1 = palette[src[5*i+1]];
        u32 s2 = palette[src[5*i+2]];
        u32 s3 = palette[src[5*i+3]];
        u32 s4 = palette[src[5*i+4]];

        RescaleBlock_5x8888_To_4x1555(s0, s1, s2, s3, s4, dest+4*i);
    }
}
#else
static inline void Rescale_320xPAL8Scanline_To_256x1555Scanline(u16 *dest, const u8 *src, const u16 *palette) {
    for (size_t i = 0; i < 64; ++i) {
        u16 s0 = palette[src[5*i+0]];
        u16 s1 = palette[src[5*i+1]];
        u16 s2 = palette[src[5*i+2]];
        u16 s3 = palette[src[5*i+3]];
        u16 s4 = palette[src[5*i+4]];

        RescaleBlock_5x1555_To_4x1555(s0, s1, s2, s3, s4, dest+4*i);
    }
}
#endif


// Can work in place, because it's a contraction
static inline void Rescale_320x1555Scanline_To_256x1555Scanline(u16 *dest, const u16 *src) {
    ComputeDivBy5TableIFN();

    for (size_t i = 0; i < 64; ++i) {
        u16 s0 = src[5*i+0];
        u16 s1 = src[5*i+1];
        u16 s2 = src[5*i+2];
        u16 s3 = src[5*i+3];
        u16 s4 = src[5*i+4];

        RescaleBlock_5x1555_To_4x1555(s0, s1, s2, s3, s4, dest+4*i);
    }
}

#ifdef PERFECT_5_TO_4_RESCALING
void Rescale_320x256xPAL8_To_256x256x1555(u16 *dest, const u8 *src, int destStride, int srcStride, const u16 *palette) {
	u32 fastRam[768];

    // Palette lookup -> 0_888
    for (size_t i = 0; i < 256; ++i) {
        u32 col = palette[i];
        u32 result = col & 0x0000001F;
        result |= (col << 3) & 0x00001F00;
        result |= (col << 6) & 0x001F0000;

        fastRam[i] = result;
    }

	for (size_t i = 0; i < 200; ++i) {
		Rescale_320xPAL8Scanline_To_256x1555Scanline(dest + i*destStride, src + i *srcStride, fastRam);
	}
}
#else
void Rescale_320x256xPAL8_To_256x256x1555(u16 *dest, const u8 *src, int destStride, int srcStride, const u16 *palette) {
	u16 fastRam[256];
    for (size_t i = 0; i < 128; ++i)
        ((u32 *)fastRam)[i] = ((const u32*)palette)[i];

	for (size_t i = 0; i < 200; ++i) {
		Rescale_320xPAL8Scanline_To_256x1555Scanline(dest + i*destStride, src + i *srcStride, fastRam);
	}
}
#endif

void Rescale_320x256x1555_To_256x256x1555(u16 *dest, const u16 *src, int destStride, int srcStride) {
	for (size_t i = 0; i < 200; ++i) {
		Rescale_320x1555Scanline_To_256x1555Scanline(dest + i*destStride, src + i *srcStride);
	}
}

}	// End of namespace DS
