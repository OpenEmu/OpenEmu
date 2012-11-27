/*
 * This file is part of the Scale2x project.
 *
 * Copyright (C) 2001, 2002, 2003, 2004 Andrea Mazzoleni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef SCALER_SCALE2X_H
#define SCALER_SCALE2X_H

#if defined(_MSC_VER)
#define __restrict__
#endif

#ifdef __sgi
#define __restrict__ __restrict
#endif


typedef unsigned char scale2x_uint8;
typedef unsigned short scale2x_uint16;
typedef unsigned scale2x_uint32;

void scale2x_8_def(scale2x_uint8* dst0, scale2x_uint8* dst1, const scale2x_uint8* src0, const scale2x_uint8* src1, const scale2x_uint8* src2, unsigned count);
void scale2x_16_def(scale2x_uint16* dst0, scale2x_uint16* dst1, const scale2x_uint16* src0, const scale2x_uint16* src1, const scale2x_uint16* src2, unsigned count);
void scale2x_32_def(scale2x_uint32* dst0, scale2x_uint32* dst1, const scale2x_uint32* src0, const scale2x_uint32* src1, const scale2x_uint32* src2, unsigned count);

#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))

void scale2x_8_mmx(scale2x_uint8* dst0, scale2x_uint8* dst1, const scale2x_uint8* src0, const scale2x_uint8* src1, const scale2x_uint8* src2, unsigned count);
void scale2x_16_mmx(scale2x_uint16* dst0, scale2x_uint16* dst1, const scale2x_uint16* src0, const scale2x_uint16* src1, const scale2x_uint16* src2, unsigned count);
void scale2x_32_mmx(scale2x_uint32* dst0, scale2x_uint32* dst1, const scale2x_uint32* src0, const scale2x_uint32* src1, const scale2x_uint32* src2, unsigned count);

/**
 * End the use of the MMX instructions.
 * This function must be called before using any floating-point operations.
 */
static inline void scale2x_mmx_emms(void)
{
	__asm__ __volatile__ (
		"emms"
	);
}

#endif

#if defined(USE_ARM_SCALER_ASM)

extern "C" void scale2x_8_arm(scale2x_uint8* dst0, scale2x_uint8* dst1, const scale2x_uint8* src0, const scale2x_uint8* src1, const scale2x_uint8* src2, unsigned count);
extern "C" void scale2x_16_arm(scale2x_uint16* dst0, scale2x_uint16* dst1, const scale2x_uint16* src0, const scale2x_uint16* src1, const scale2x_uint16* src2, unsigned count);
extern "C" void scale2x_32_arm(scale2x_uint32* dst0, scale2x_uint32* dst1, const scale2x_uint32* src0, const scale2x_uint32* src1, const scale2x_uint32* src2, unsigned count);

#endif

#endif
