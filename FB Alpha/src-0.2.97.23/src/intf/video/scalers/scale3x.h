/*
 * This file is part of the Scale2x project.
 *
 * Copyright (C) 2001-2003 Andrea Mazzoleni
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

/*
 * This file contains a C implementation of the Scale3x effect.
 *
 * You can find an high level description of the effect at :
 *
 * http://scale2x.sourceforge.net/
 *
 * Alternatively at the previous license terms, you are allowed to use this
 * code in your program with these conditions:
 * - the program is not used in commercial activities.
 * - the whole source code of the program is released with the binary.
 * - derivative works of the program are allowed.
 */

#ifndef __SCALE3X_H
#define __SCALE3X_H

#include <assert.h>

/***************************************************************************/
/* Basic types */

typedef unsigned char scale3x_uint8;
typedef unsigned short scale3x_uint16;
typedef unsigned scale3x_uint32;

/***************************************************************************/
/* Scale3x C implementation */

static void scale3x_8_def_single(scale3x_uint8* dst, const scale3x_uint8* src0, const scale3x_uint8* src1, const scale3x_uint8* src2, unsigned count)
{
	assert(count >= 2);

	/* first pixel */
	dst[0] = src1[0];
	dst[1] = src1[0];
	if (src1[1] == src0[0] && src2[0] != src0[0])
		dst[2] = src0[0];
	else
		dst[2] = src1[0];
	++src0;
	++src1;
	++src2;
	dst += 3;

	/* central pixels */
	count -= 2;
	while (count) {
		if (src1[-1] == src0[0] && src2[0] != src0[0] && src1[1] != src0[0])
			dst[0] = src0[0];
		else
			dst[0] = src1[0];
		dst[1] = src1[0];
		if (src1[1] == src0[0] && src2[0] != src0[0] && src1[-1] != src0[0])
			dst[2] = src0[0];
		else
			dst[2] = src1[0];

		++src0;
		++src1;
		++src2;

		dst += 3;
		--count;
	}

	/* last pixel */
	if (src1[-1] == src0[0] && src2[0] != src0[0])
		dst[0] = src0[0];
	else
		dst[0] = src1[0];
	dst[1] = src1[0];
	dst[2] = src1[0];
}

static void scale3x_8_def_fill(scale3x_uint8* dst, const scale3x_uint8* src, unsigned count)
{
	while (count) {
		dst[0] = src[0];
		dst[1] = src[0];
		dst[2] = src[0];

		++src;
		dst += 3;
		--count;
	}
}

static void scale3x_16_def_single(scale3x_uint16* dst, const scale3x_uint16* src0, const scale3x_uint16* src1, const scale3x_uint16* src2, unsigned count)
{
	assert(count >= 2);

	/* first pixel */
	dst[0] = src1[0];
	dst[1] = src1[0];
	if (src1[1] == src0[0] && src2[0] != src0[0])
		dst[2] = src0[0];
	else
		dst[2] = src1[0];
	++src0;
	++src1;
	++src2;
	dst += 3;

	/* central pixels */
	count -= 2;
	while (count) {
		if (src1[-1] == src0[0] && src2[0] != src0[0] && src1[1] != src0[0])
			dst[0] = src0[0];
		else
			dst[0] = src1[0];
		dst[1] = src1[0];
		if (src1[1] == src0[0] && src2[0] != src0[0] && src1[-1] != src0[0])
			dst[2] = src0[0];
		else
			dst[2] = src1[0];

		++src0;
		++src1;
		++src2;

		dst += 3;
		--count;
	}

	/* last pixel */
	if (src1[-1] == src0[0] && src2[0] != src0[0])
		dst[0] = src0[0];
	else
		dst[0] = src1[0];
	dst[1] = src1[0];
	dst[2] = src1[0];
}

static void scale3x_16_def_fill(scale3x_uint16* dst, const scale3x_uint16* src, unsigned count)
{
	while (count) {
		dst[0] = src[0];
		dst[1] = src[0];
		dst[2] = src[0];

		++src;
		dst += 3;
		--count;
	}
}

static void scale3x_32_def_single(scale3x_uint32* dst, const scale3x_uint32* src0, const scale3x_uint32* src1, const scale3x_uint32* src2, unsigned count)
{
	assert(count >= 2);

	/* first pixel */
	dst[0] = src1[0];
	dst[1] = src1[0];
	if (src1[1] == src0[0] && src2[0] != src0[0])
		dst[2] = src0[0];
	else
		dst[2] = src1[0];
	++src0;
	++src1;
	++src2;
	dst += 3;

	/* central pixels */
	count -= 2;
	while (count) {
		if (src1[-1] == src0[0] && src2[0] != src0[0] && src1[1] != src0[0])
			dst[0] = src0[0];
		else
			dst[0] = src1[0];
		dst[1] = src1[0];
		if (src1[1] == src0[0] && src2[0] != src0[0] && src1[-1] != src0[0])
			dst[2] = src0[0];
		else
			dst[2] = src1[0];

		++src0;
		++src1;
		++src2;

		dst += 3;
		--count;
	}

	/* last pixel */
	if (src1[-1] == src0[0] && src2[0] != src0[0])
		dst[0] = src0[0];
	else
		dst[0] = src1[0];
	dst[1] = src1[0];
	dst[2] = src1[0];
}

static void scale3x_32_def_fill(scale3x_uint32* dst, const scale3x_uint32* src, unsigned count)
{
	while (count) {
		dst[0] = src[0];
		dst[1] = src[0];
		dst[2] = src[0];

		++src;
		dst += 3;
		--count;
	}
}

/**
 * Scale by a factor of 3 a row of pixels of 8 bits.
 * The function is implemented in C.
 * The pixels over the left and right borders are assumed of the same color of
 * the pixels on the border.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows.
 * It must be at least 2.
 * \param dst0 First destination row, triple length in pixels.
 * \param dst1 Second destination row, triple length in pixels.
 * \param dst2 Third destination row, triple length in pixels.
 */
static inline void scale3x_8_def(scale3x_uint8* dst0, scale3x_uint8* dst1, scale3x_uint8* dst2, const scale3x_uint8* src0, const scale3x_uint8* src1, const scale3x_uint8* src2, unsigned count)
{
	assert(count >= 2);

	scale3x_8_def_single(dst0, src0, src1, src2, count);
	scale3x_8_def_fill(dst1, src1, count);
	scale3x_8_def_single(dst2, src2, src1, src0, count);
}

/**
 * Scale by a factor of 3 a row of pixels of 16 bits.
 * This function operates like scale3x_8_def() but for 16 bits pixels.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows.
 * It must be at least 2.
 * \param dst0 First destination row, triple length in pixels.
 * \param dst1 Second destination row, triple length in pixels.
 * \param dst2 Third destination row, triple length in pixels.
 */
static inline void scale3x_16_def(scale3x_uint16* dst0, scale3x_uint16* dst1, scale3x_uint16* dst2, const scale3x_uint16* src0, const scale3x_uint16* src1, const scale3x_uint16* src2, unsigned count)
{
	assert(count >= 2);

	scale3x_16_def_single(dst0, src0, src1, src2, count);
	scale3x_16_def_fill(dst1, src1, count);
	scale3x_16_def_single(dst2, src2, src1, src0, count);
}

/**
 * Scale by a factor of 3 a row of pixels of 32 bits.
 * This function operates like scale3x_8_def() but for 32 bits pixels.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows.
 * It must be at least 2.
 * \param dst0 First destination row, triple length in pixels.
 * \param dst1 Second destination row, triple length in pixels.
 * \param dst2 Third destination row, triple length in pixels.
 */
static inline void scale3x_32_def(scale3x_uint32* dst0, scale3x_uint32* dst1, scale3x_uint32* dst2, const scale3x_uint32* src0, const scale3x_uint32* src1, const scale3x_uint32* src2, unsigned count)
{
	assert(count >= 2);

	scale3x_32_def_single(dst0, src0, src1, src2, count);
	scale3x_32_def_fill(dst1, src1, count);
	scale3x_32_def_single(dst2, src2, src1, src0, count);
}

#endif
