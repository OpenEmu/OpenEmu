/*
 * This file is part of the Scale2x project.
 *
 * Copyright (C) 2003, 2004 Andrea Mazzoleni
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
 * Gens/GS MDP specific code.
 * This is basically the mmx-only code, split out into a separate file.
 * It allows for the Scale?x plugin to be compiled with support for both
 * CPUs that support mmx and CPUs that don't support mmx.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "scale2x.h"
#include "scale3x.h"

#if HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include <assert.h>
#include <stdlib.h>

#define SSDST(bits, num) (uint##bits##_t *)dst##num
#define SSSRC(bits, num) (const uint##bits##_t *)src##num

// Scale2x uses GNU inline assembler.
#if defined(__GNUC__) && defined(__i386__)

/**
 * Apply the Scale2x effect on a group of rows. Used internally.
 */
static inline void stage_scale2x_mmx(void* dst0, void* dst1,
				     const void* src0, const void* src1, const void* src2,
	 unsigned int pixel, unsigned int pixel_per_row)
{
	switch (pixel)
	{
		case 2:
			scale2x_16_mmx(SSDST(16,0), SSDST(16,1),
				       SSSRC(16,0), SSSRC(16,1), SSSRC(16,2),
					     pixel_per_row);
			break;
		
		case 4:
			scale2x_32_mmx(SSDST(32,0), SSDST(32,1),
				       SSSRC(32,0), SSSRC(32,1), SSSRC(32,2),
					     pixel_per_row);
			break;
	}
}

/**
 * Apply the Scale4x effect on a group of rows. Used internally.
 */
static inline void stage_scale4x_mmx(void* dst0, void* dst1, void* dst2, void* dst3,
				     const void* src0, const void* src1, const void* src2, const void* src3,
	 unsigned int pixel, unsigned int pixel_per_row)
{
	stage_scale2x_mmx(dst0, dst1, src0, src1, src2, pixel, 2 * pixel_per_row);
	stage_scale2x_mmx(dst2, dst3, src1, src2, src3, pixel, 2 * pixel_per_row);
}

#define SCDST(i) (dst+(i)*dst_slice)
#define SCSRC(i) (src+(i)*src_slice)
#define SCMID(i) (mid[(i)])

/**
 * Apply the Scale2x effect on a bitmap.
 * The destination bitmap is filled with the scaled version of the source bitmap.
 * The source bitmap isn't modified.
 * The destination bitmap must be manually allocated before calling the function,
 * note that the resulting size is exactly 2x2 times the size of the source bitmap.
 * \param void_dst Pointer at the first pixel of the destination bitmap.
 * \param dst_slice Size in bytes of a destination bitmap row.
 * \param void_src Pointer at the first pixel of the source bitmap.
 * \param src_slice Size in bytes of a source bitmap row.
 * \param pixel Bytes per pixel of the source and destination bitmap.
 * \param width Horizontal size in pixels of the source bitmap.
 * \param height Vertical size in pixels of the source bitmap.
 */
void scale2x_mmx(void* void_dst, unsigned int dst_slice,
		 const void* void_src, unsigned int src_slice,
		 unsigned int pixel, unsigned int width, unsigned int height)
{
	unsigned char* dst = (unsigned char*)void_dst;
	const unsigned char* src = (const unsigned char*)void_src;
	unsigned int count;
	
	assert(height >= 2);
	
	count = height;
	
	stage_scale2x_mmx(SCDST(0), SCDST(1), SCSRC(0), SCSRC(0), SCSRC(1), pixel, width);
	
	dst = SCDST(2);
	
	count -= 2;
	while (count)
	{
		stage_scale2x_mmx(SCDST(0), SCDST(1), SCSRC(0), SCSRC(1), SCSRC(2), pixel, width);
		
		dst = SCDST(2);
		src = SCSRC(1);
		
		--count;
	}
	
	stage_scale2x_mmx(SCDST(0), SCDST(1), SCSRC(0), SCSRC(1), SCSRC(1), pixel, width);
	
	scale2x_mmx_emms();
}

/**
 * Apply the Scale4x effect on a bitmap.
 * The destination bitmap is filled with the scaled version of the source bitmap.
 * The source bitmap isn't modified.
 * The destination bitmap must be manually allocated before calling the function,
 * note that the resulting size is exactly 4x4 times the size of the source bitmap.
 * \note This function requires also a small buffer bitmap used internally to store
 * intermediate results. This bitmap must have at least an horizontal size in bytes of 2*width*pixel,
 * and a vertical size of 6 rows. The memory of this buffer must not be allocated
 * in video memory because it's also read and not only written. Generally
 * a heap (malloc) or a stack (alloca) buffer is the best choice.
 * \param void_dst Pointer at the first pixel of the destination bitmap.
 * \param dst_slice Size in bytes of a destination bitmap row.
 * \param void_mid Pointer at the first pixel of the buffer bitmap.
 * \param mid_slice Size in bytes of a buffer bitmap row.
 * \param void_src Pointer at the first pixel of the source bitmap.
 * \param src_slice Size in bytes of a source bitmap row.
 * \param pixel Bytes per pixel of the source and destination bitmap.
 * \param width Horizontal size in pixels of the source bitmap.
 * \param height Vertical size in pixels of the source bitmap.
 */
static inline void scale4x_buf_mmx(void* void_dst, unsigned int dst_slice,
				   void* void_mid, unsigned int mid_slice,
				   const void* void_src, unsigned int src_slice,
				   unsigned int pixel, unsigned int width, unsigned int height)
{
	unsigned char* dst = (unsigned char*)void_dst;
	const unsigned char* src = (const unsigned char*)void_src;
	unsigned int count;
	unsigned char* mid[6];
	
	assert(height >= 4);
	
	count = height;
	
	/* set the 6 buffer pointers */
	mid[0] = (unsigned char*)void_mid;
	mid[1] = mid[0] + mid_slice;
	mid[2] = mid[1] + mid_slice;
	mid[3] = mid[2] + mid_slice;
	mid[4] = mid[3] + mid_slice;
	mid[5] = mid[4] + mid_slice;
	
	stage_scale2x_mmx(SCMID(-2+6), SCMID(-1+6), SCSRC(0), SCSRC(0), SCSRC(1), pixel, width);
	stage_scale2x_mmx(SCMID(0), SCMID(1), SCSRC(0), SCSRC(1), SCSRC(2), pixel, width);
	stage_scale2x_mmx(SCMID(2), SCMID(3), SCSRC(1), SCSRC(2), SCSRC(3), pixel, width);
	stage_scale4x_mmx(SCDST(0), SCDST(1), SCDST(2), SCDST(3), SCMID(-2+6), SCMID(-2+6), SCMID(-1+6), SCMID(0), pixel, width);
	
	dst = SCDST(4);
	
	stage_scale4x_mmx(SCDST(0), SCDST(1), SCDST(2), SCDST(3), SCMID(-1+6), SCMID(0), SCMID(1), SCMID(2), pixel, width);
	
	dst = SCDST(4);
	
	count -= 4;
	while (count)
	{
		unsigned char* tmp;
		
		stage_scale2x_mmx(SCMID(4), SCMID(5), SCSRC(2), SCSRC(3), SCSRC(4), pixel, width);
		stage_scale4x_mmx(SCDST(0), SCDST(1), SCDST(2), SCDST(3), SCMID(1), SCMID(2), SCMID(3), SCMID(4), pixel, width);
		
		dst = SCDST(4);
		src = SCSRC(1);
		
		tmp = SCMID(0); /* shift by 2 position */
		SCMID(0) = SCMID(2);
		SCMID(2) = SCMID(4);
		SCMID(4) = tmp;
		tmp = SCMID(1);
		SCMID(1) = SCMID(3);
		SCMID(3) = SCMID(5);
		SCMID(5) = tmp;
		
		--count;
	}
	
	stage_scale2x_mmx(SCMID(4), SCMID(5), SCSRC(2), SCSRC(3), SCSRC(3), pixel, width);
	stage_scale4x_mmx(SCDST(0), SCDST(1), SCDST(2), SCDST(3), SCMID(1), SCMID(2), SCMID(3), SCMID(4), pixel, width);
	
	dst = SCDST(4);
	
	stage_scale4x_mmx(SCDST(0), SCDST(1), SCDST(2), SCDST(3), SCMID(3), SCMID(4), SCMID(5), SCMID(5), pixel, width);
	
	scale2x_mmx_emms();
}

/**
 * Apply the Scale4x effect on a bitmap.
 * The destination bitmap is filled with the scaled version of the source bitmap.
 * The source bitmap isn't modified.
 * The destination bitmap must be manually allocated before calling the function,
 * note that the resulting size is exactly 4x4 times the size of the source bitmap.
 * \note This function operates like ::scale4x_buf() but the intermediate buffer is
 * automatically allocated in the stack.
 * \param void_dst Pointer at the first pixel of the destination bitmap.
 * \param dst_slice Size in bytes of a destination bitmap row.
 * \param void_src Pointer at the first pixel of the source bitmap.
 * \param src_slice Size in bytes of a source bitmap row.
 * \param pixel Bytes per pixel of the source and destination bitmap.
 * \param width Horizontal size in pixels of the source bitmap.
 * \param height Vertical size in pixels of the source bitmap.
 */
void scale4x_mmx(void* void_dst, unsigned int dst_slice,
		 const void* void_src, unsigned int src_slice,
		 unsigned int pixel, unsigned int width, unsigned int height)
{
	unsigned int mid_slice;
	void* mid;
	
	mid_slice = 2 * pixel * width; /* required space for 1 row buffer */
	
	mid_slice = (mid_slice + 0x7) & ~0x7; /* align to 8 bytes */
	
#if HAVE_ALLOCA
	mid = alloca(6 * mid_slice); /* allocate space for 6 row buffers */
	
	assert(mid != 0); /* alloca should never fails */
#else
	mid = malloc(6 * mid_slice); /* allocate space for 6 row buffers */
	
	if (!mid)
		return;
#endif
	
	scale4x_buf_mmx(void_dst, dst_slice, mid, mid_slice, void_src, src_slice, pixel, width, height);
	
#if !HAVE_ALLOCA
	free(mid);
#endif
}

#endif /* defined(__GNUC__) && defined(__i386__) */
