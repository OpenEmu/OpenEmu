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

#ifndef __SCALE2X_H
#define __SCALE2X_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void scale2x_16_def(uint16_t* dst0, uint16_t* dst1,
		    const uint16_t* src0, const uint16_t* src1,
		    const uint16_t* src2, unsigned int count);

void scale2x_32_def(uint32_t* dst0, uint32_t* dst1,
		    const uint32_t* src0, const uint32_t* src1,
		    const uint32_t* src2, unsigned count);

#if defined(__GNUC__) && defined(__i386__)

void scale2x_16_mmx(uint16_t* dst0, uint16_t* dst1,
		    const uint16_t* src0, const uint16_t* src1,
		    const uint16_t* src2, unsigned count);

void scale2x_32_mmx(uint32_t* dst0, uint32_t* dst1,
		    const uint32_t* src0, const uint32_t* src1,
		    const uint32_t* src2, unsigned count);

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

#ifdef __cplusplus
}
#endif

#endif /* __SCALE2X_H */
