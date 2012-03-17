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

/*
 * This file contains a C and MMX implementation of the Scale2x effect.
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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "scale3x.h"

#include <assert.h>

// "restrict" is only available in C99 and later.
#if __STDC_VERSION__ >= 199901L
#define RESTRICT restrict
#else
#define RESTRICT
#endif

/***************************************************************************/
/* Scale3x C implementation */

/**
 * Define the macro USE_SCALE_RANDOMWRITE to enable
 * an optimized version which writes memory in random order.
 * This version is a little faster if you write in system memory.
 * But it's a lot slower if you write in video memory.
 * So, enable it only if you are sure to never write directly in video memory.
 */
/* #define USE_SCALE_RANDOMWRITE */

static inline void scale3x_16_def_whole(uint16_t* RESTRICT dst0, uint16_t* RESTRICT dst1, uint16_t* RESTRICT dst2,
					const uint16_t* RESTRICT src0, const uint16_t* RESTRICT src1,
					const uint16_t* RESTRICT src2, unsigned int count)
{
	assert(count >= 2);
	
	/* first pixel */
	if (src0[0] != src2[0] && src1[0] != src1[1])
	{
		dst0[0] = src1[0];
		dst0[1] = (src1[0] == src0[0] && src1[0] != src0[1]) || (src1[1] == src0[0] && src1[0] != src0[0]) ? src0[0] : src1[0];
		dst0[2] = src1[1] == src0[0] ? src1[1] : src1[0];
		dst1[0] = (src1[0] == src0[0] && src1[0] != src2[0]) || (src1[0] == src2[0] && src1[0] != src0[0]) ? src1[0] : src1[0];
		dst1[1] = src1[0];
		dst1[2] = (src1[1] == src0[0] && src1[0] != src2[1]) || (src1[1] == src2[0] && src1[0] != src0[1]) ? src1[1] : src1[0];
		dst2[0] = src1[0];
		dst2[1] = (src1[0] == src2[0] && src1[0] != src2[1]) || (src1[1] == src2[0] && src1[0] != src2[0]) ? src2[0] : src1[0];
		dst2[2] = src1[1] == src2[0] ? src1[1] : src1[0];
	}
	else
	{
		dst0[0] = src1[0];
		dst0[1] = src1[0];
		dst0[2] = src1[0];
		dst1[0] = src1[0];
		dst1[1] = src1[0];
		dst1[2] = src1[0];
		dst2[0] = src1[0];
		dst2[1] = src1[0];
		dst2[2] = src1[0];
	}
	
	++src0;
	++src1;
	++src2;
	dst0 += 3;
	dst1 += 3;
	dst2 += 3;
	
	/* central pixels */
	count -= 2;
	while (count)
	{
		if (src0[0] != src2[0] && src1[-1] != src1[1])
		{
			dst0[0] = src1[-1] == src0[0] ? src1[-1] : src1[0];
			dst0[1] = (src1[-1] == src0[0] && src1[0] != src0[1]) || (src1[1] == src0[0] && src1[0] != src0[-1]) ? src0[0] : src1[0];
			dst0[2] = src1[1] == src0[0] ? src1[1] : src1[0];
			dst1[0] = (src1[-1] == src0[0] && src1[0] != src2[-1]) || (src1[-1] == src2[0] && src1[0] != src0[-1]) ? src1[-1] : src1[0];
			dst1[1] = src1[0];
			dst1[2] = (src1[1] == src0[0] && src1[0] != src2[1]) || (src1[1] == src2[0] && src1[0] != src0[1]) ? src1[1] : src1[0];
			dst2[0] = src1[-1] == src2[0] ? src1[-1] : src1[0];
			dst2[1] = (src1[-1] == src2[0] && src1[0] != src2[1]) || (src1[1] == src2[0] && src1[0] != src2[-1]) ? src2[0] : src1[0];
			dst2[2] = src1[1] == src2[0] ? src1[1] : src1[0];
		}
		else
		{
			dst0[0] = src1[0];
			dst0[1] = src1[0];
			dst0[2] = src1[0];
			dst1[0] = src1[0];
			dst1[1] = src1[0];
			dst1[2] = src1[0];
			dst2[0] = src1[0];
			dst2[1] = src1[0];
			dst2[2] = src1[0];
		}
			
		++src0;
		++src1;
		++src2;
		dst0 += 3;
		dst1 += 3;
		dst2 += 3;
		--count;
	}
	
	/* last pixel */
	if (src0[0] != src2[0] && src1[-1] != src1[0])
	{
		dst0[0] = src1[-1] == src0[0] ? src1[-1] : src1[0];
		dst0[1] = (src1[-1] == src0[0] && src1[0] != src0[0]) || (src1[0] == src0[0] && src1[0] != src0[-1]) ? src0[0] : src1[0];
		dst0[2] = src1[0];
		dst1[0] = (src1[-1] == src0[0] && src1[0] != src2[-1]) || (src1[-1] == src2[0] && src1[0] != src0[-1]) ? src1[-1] : src1[0];
		dst1[1] = src1[0];
		dst1[2] = (src1[0] == src0[0] && src1[0] != src2[0]) || (src1[0] == src2[0] && src1[0] != src0[0]) ? src1[0] : src1[0];
		dst2[0] = src1[-1] == src2[0] ? src1[-1] : src1[0];
		dst2[1] = (src1[-1] == src2[0] && src1[0] != src2[0]) || (src1[0] == src2[0] && src1[0] != src2[-1]) ? src2[0] : src1[0];
		dst2[2] = src1[0];
	}
	else
	{
		dst0[0] = src1[0];
		dst0[1] = src1[0];
		dst0[2] = src1[0];
		dst1[0] = src1[0];
		dst1[1] = src1[0];
		dst1[2] = src1[0];
		dst2[0] = src1[0];
		dst2[1] = src1[0];
		dst2[2] = src1[0];
	}
}

static inline void scale3x_16_def_border(uint16_t* RESTRICT dst, const uint16_t* RESTRICT src0,
					 const uint16_t* RESTRICT src1, const uint16_t* RESTRICT src2,
					 unsigned int count)
{
	assert(count >= 2);
	
	/* first pixel */
	if (src0[0] != src2[0] && src1[0] != src1[1])
	{
		dst[0] = src1[0];
		dst[1] = (src1[0] == src0[0] && src1[0] != src0[1]) || (src1[1] == src0[0] && src1[0] != src0[0]) ? src0[0] : src1[0];
		dst[2] = src1[1] == src0[0] ? src1[1] : src1[0];
	}
	else
	{
		dst[0] = src1[0];
		dst[1] = src1[0];
		dst[2] = src1[0];
	}
	
	++src0;
	++src1;
	++src2;
	dst += 3;
	
	/* central pixels */
	count -= 2;
	while (count)
	{
		if (src0[0] != src2[0] && src1[-1] != src1[1])
		{
			dst[0] = src1[-1] == src0[0] ? src1[-1] : src1[0];
			dst[1] = (src1[-1] == src0[0] && src1[0] != src0[1]) || (src1[1] == src0[0] && src1[0] != src0[-1]) ? src0[0] : src1[0];
			dst[2] = src1[1] == src0[0] ? src1[1] : src1[0];
		}
		else
		{
			dst[0] = src1[0];
			dst[1] = src1[0];
			dst[2] = src1[0];
		}
		
		++src0;
		++src1;
		++src2;
		dst += 3;
		--count;
	}
	
	/* last pixel */
	if (src0[0] != src2[0] && src1[-1] != src1[0])
	{
		dst[0] = src1[-1] == src0[0] ? src1[-1] : src1[0];
		dst[1] = (src1[-1] == src0[0] && src1[0] != src0[0]) || (src1[0] == src0[0] && src1[0] != src0[-1]) ? src0[0] : src1[0];
		dst[2] = src1[0];
	}
	else
	{
		dst[0] = src1[0];
		dst[1] = src1[0];
		dst[2] = src1[0];
	}
}

static inline void scale3x_16_def_center(uint16_t* RESTRICT dst, const uint16_t* RESTRICT src0,
					 const uint16_t* RESTRICT src1, const uint16_t* RESTRICT src2,
					 unsigned int count)
{
	assert(count >= 2);
	
	/* first pixel */
	if (src0[0] != src2[0] && src1[0] != src1[1])
	{
		dst[0] = (src1[0] == src0[0] && src1[0] != src2[0]) || (src1[0] == src2[0] && src1[0] != src0[0]) ? src1[0] : src1[0];
		dst[1] = src1[0];
		dst[2] = (src1[1] == src0[0] && src1[0] != src2[1]) || (src1[1] == src2[0] && src1[0] != src0[1]) ? src1[1] : src1[0];
	}
	else
	{
		dst[0] = src1[0];
		dst[1] = src1[0];
		dst[2] = src1[0];
	}
	
	++src0;
	++src1;
	++src2;
	dst += 3;
	
	/* central pixels */
	count -= 2;
	while (count)
	{
		if (src0[0] != src2[0] && src1[-1] != src1[1])
		{
			dst[0] = (src1[-1] == src0[0] && src1[0] != src2[-1]) || (src1[-1] == src2[0] && src1[0] != src0[-1]) ? src1[-1] : src1[0];
			dst[1] = src1[0];
			dst[2] = (src1[1] == src0[0] && src1[0] != src2[1]) || (src1[1] == src2[0] && src1[0] != src0[1]) ? src1[1] : src1[0];
		}
		else
		{
			dst[0] = src1[0];
			dst[1] = src1[0];
			dst[2] = src1[0];
		}
		
		++src0;
		++src1;
		++src2;
		dst += 3;
		--count;
	}

	/* last pixel */
	if (src0[0] != src2[0] && src1[-1] != src1[0])
	{
		dst[0] = (src1[-1] == src0[0] && src1[0] != src2[-1]) || (src1[-1] == src2[0] && src1[0] != src0[-1]) ? src1[-1] : src1[0];
		dst[1] = src1[0];
		dst[2] = (src1[0] == src0[0] && src1[0] != src2[0]) || (src1[0] == src2[0] && src1[0] != src0[0]) ? src1[0] : src1[0];
	}
	else
	{
		dst[0] = src1[0];
		dst[1] = src1[0];
		dst[2] = src1[0];
	}
}

static inline void scale3x_32_def_whole(uint32_t* RESTRICT dst0, uint32_t* RESTRICT dst1, uint32_t* RESTRICT dst2,
					const uint32_t* RESTRICT src0, const uint32_t* RESTRICT src1,
					const uint32_t* RESTRICT src2, unsigned count)
{
	assert(count >= 2);
	
	/* first pixel */
	if (src0[0] != src2[0] && src1[0] != src1[1])
	{
		dst0[0] = src1[0];
		dst0[1] = (src1[0] == src0[0] && src1[0] != src0[1]) || (src1[1] == src0[0] && src1[0] != src0[0]) ? src0[0] : src1[0];
		dst0[2] = src1[1] == src0[0] ? src1[1] : src1[0];
		dst1[0] = (src1[0] == src0[0] && src1[0] != src2[0]) || (src1[0] == src2[0] && src1[0] != src0[0]) ? src1[0] : src1[0];
		dst1[1] = src1[0];
		dst1[2] = (src1[1] == src0[0] && src1[0] != src2[1]) || (src1[1] == src2[0] && src1[0] != src0[1]) ? src1[1] : src1[0];
		dst2[0] = src1[0];
		dst2[1] = (src1[0] == src2[0] && src1[0] != src2[1]) || (src1[1] == src2[0] && src1[0] != src2[0]) ? src2[0] : src1[0];
		dst2[2] = src1[1] == src2[0] ? src1[1] : src1[0];
	}
	else
	{
		dst0[0] = src1[0];
		dst0[1] = src1[0];
		dst0[2] = src1[0];
		dst1[0] = src1[0];
		dst1[1] = src1[0];
		dst1[2] = src1[0];
		dst2[0] = src1[0];
		dst2[1] = src1[0];
		dst2[2] = src1[0];
	}
	
	++src0;
	++src1;
	++src2;
	dst0 += 3;
	dst1 += 3;
	dst2 += 3;

	/* central pixels */
	count -= 2;
	while (count)
	{
		if (src0[0] != src2[0] && src1[-1] != src1[1])
		{
			dst0[0] = src1[-1] == src0[0] ? src1[-1] : src1[0];
			dst0[1] = (src1[-1] == src0[0] && src1[0] != src0[1]) || (src1[1] == src0[0] && src1[0] != src0[-1]) ? src0[0] : src1[0];
			dst0[2] = src1[1] == src0[0] ? src1[1] : src1[0];
			dst1[0] = (src1[-1] == src0[0] && src1[0] != src2[-1]) || (src1[-1] == src2[0] && src1[0] != src0[-1]) ? src1[-1] : src1[0];
			dst1[1] = src1[0];
			dst1[2] = (src1[1] == src0[0] && src1[0] != src2[1]) || (src1[1] == src2[0] && src1[0] != src0[1]) ? src1[1] : src1[0];
			dst2[0] = src1[-1] == src2[0] ? src1[-1] : src1[0];
			dst2[1] = (src1[-1] == src2[0] && src1[0] != src2[1]) || (src1[1] == src2[0] && src1[0] != src2[-1]) ? src2[0] : src1[0];
			dst2[2] = src1[1] == src2[0] ? src1[1] : src1[0];
		}
		else
		{
			dst0[0] = src1[0];
			dst0[1] = src1[0];
			dst0[2] = src1[0];
			dst1[0] = src1[0];
			dst1[1] = src1[0];
			dst1[2] = src1[0];
			dst2[0] = src1[0];
			dst2[1] = src1[0];
			dst2[2] = src1[0];
		}
		
		++src0;
		++src1;
		++src2;
		dst0 += 3;
		dst1 += 3;
		dst2 += 3;
		--count;
	}
	
	/* last pixel */
	if (src0[0] != src2[0] && src1[-1] != src1[0])
	{
		dst0[0] = src1[-1] == src0[0] ? src1[-1] : src1[0];
		dst0[1] = (src1[-1] == src0[0] && src1[0] != src0[0]) || (src1[0] == src0[0] && src1[0] != src0[-1]) ? src0[0] : src1[0];
		dst0[2] = src1[0];
		dst1[0] = (src1[-1] == src0[0] && src1[0] != src2[-1]) || (src1[-1] == src2[0] && src1[0] != src0[-1]) ? src1[-1] : src1[0];
		dst1[1] = src1[0];
		dst1[2] = (src1[0] == src0[0] && src1[0] != src2[0]) || (src1[0] == src2[0] && src1[0] != src0[0]) ? src1[0] : src1[0];
		dst2[0] = src1[-1] == src2[0] ? src1[-1] : src1[0];
		dst2[1] = (src1[-1] == src2[0] && src1[0] != src2[0]) || (src1[0] == src2[0] && src1[0] != src2[-1]) ? src2[0] : src1[0];
		dst2[2] = src1[0];
	}
	else
	{
		dst0[0] = src1[0];
		dst0[1] = src1[0];
		dst0[2] = src1[0];
		dst1[0] = src1[0];
		dst1[1] = src1[0];
		dst1[2] = src1[0];
		dst2[0] = src1[0];
		dst2[1] = src1[0];
		dst2[2] = src1[0];
	}
}

static inline void scale3x_32_def_border(uint32_t* RESTRICT dst, const uint32_t* RESTRICT src0,
					 const uint32_t* RESTRICT src1, const uint32_t* RESTRICT src2,
					 unsigned int count)
{
	assert(count >= 2);
	
	/* first pixel */
	if (src0[0] != src2[0] && src1[0] != src1[1])
	{
		dst[0] = src1[0];
		dst[1] = (src1[0] == src0[0] && src1[0] != src0[1]) || (src1[1] == src0[0] && src1[0] != src0[0]) ? src0[0] : src1[0];
		dst[2] = src1[1] == src0[0] ? src1[1] : src1[0];
	}
	else
	{
		dst[0] = src1[0];
		dst[1] = src1[0];
		dst[2] = src1[0];
	}
	
	++src0;
	++src1;
	++src2;
	dst += 3;
	
	/* central pixels */
	count -= 2;
	while (count)
	{
		if (src0[0] != src2[0] && src1[-1] != src1[1])
		{
			dst[0] = src1[-1] == src0[0] ? src1[-1] : src1[0];
			dst[1] = (src1[-1] == src0[0] && src1[0] != src0[1]) || (src1[1] == src0[0] && src1[0] != src0[-1]) ? src0[0] : src1[0];
			dst[2] = src1[1] == src0[0] ? src1[1] : src1[0];
		}
		else
		{
			dst[0] = src1[0];
			dst[1] = src1[0];
			dst[2] = src1[0];
		}
		
		++src0;
		++src1;
		++src2;
		dst += 3;
		--count;
	}
	
	/* last pixel */
	if (src0[0] != src2[0] && src1[-1] != src1[0])
	{
		dst[0] = src1[-1] == src0[0] ? src1[-1] : src1[0];
		dst[1] = (src1[-1] == src0[0] && src1[0] != src0[0]) || (src1[0] == src0[0] && src1[0] != src0[-1]) ? src0[0] : src1[0];
		dst[2] = src1[0];
	}
	else
	{
		dst[0] = src1[0];
		dst[1] = src1[0];
		dst[2] = src1[0];
	}
}

static inline void scale3x_32_def_center(uint32_t* RESTRICT dst, const uint32_t* RESTRICT src0,
					 const uint32_t* RESTRICT src1, const uint32_t* RESTRICT src2,
					 unsigned count)
{
	assert(count >= 2);
	
	/* first pixel */
	if (src0[0] != src2[0] && src1[0] != src1[1])
	{
		dst[0] = (src1[0] == src0[0] && src1[0] != src2[0]) || (src1[0] == src2[0] && src1[0] != src0[0]) ? src1[0] : src1[0];
		dst[1] = src1[0];
		dst[2] = (src1[1] == src0[0] && src1[0] != src2[1]) || (src1[1] == src2[0] && src1[0] != src0[1]) ? src1[1] : src1[0];
	}
	else
	{
		dst[0] = src1[0];
		dst[1] = src1[0];
		dst[2] = src1[0];
	}
	
	++src0;
	++src1;
	++src2;
	dst += 3;
	
	/* central pixels */
	count -= 2;
	while (count)
	{
		if (src0[0] != src2[0] && src1[-1] != src1[1])
		{
			dst[0] = (src1[-1] == src0[0] && src1[0] != src2[-1]) || (src1[-1] == src2[0] && src1[0] != src0[-1]) ? src1[-1] : src1[0];
			dst[1] = src1[0];
			dst[2] = (src1[1] == src0[0] && src1[0] != src2[1]) || (src1[1] == src2[0] && src1[0] != src0[1]) ? src1[1] : src1[0];
		}
		else
		{
			dst[0] = src1[0];
			dst[1] = src1[0];
			dst[2] = src1[0];
		}
		
		++src0;
		++src1;
		++src2;
		dst += 3;
		--count;
	}
	
	/* last pixel */
	if (src0[0] != src2[0] && src1[-1] != src1[0])
	{
		dst[0] = (src1[-1] == src0[0] && src1[0] != src2[-1]) || (src1[-1] == src2[0] && src1[0] != src0[-1]) ? src1[-1] : src1[0];
		dst[1] = src1[0];
		dst[2] = (src1[0] == src0[0] && src1[0] != src2[0]) || (src1[0] == src2[0] && src1[0] != src0[0]) ? src1[0] : src1[0];
	}
	else
	{
		dst[0] = src1[0];
		dst[1] = src1[0];
		dst[2] = src1[0];
	}
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
void scale3x_16_def(uint16_t* dst0, uint16_t* dst1, uint16_t* dst2,
		    const uint16_t* src0, const uint16_t* src1, const uint16_t* src2,
		    unsigned int count)
{
#ifdef USE_SCALE_RANDOMWRITE
	scale3x_16_def_whole(dst0, dst1, dst2, src0, src1, src2, count);
#else
	scale3x_16_def_border(dst0, src0, src1, src2, count);
	scale3x_16_def_center(dst1, src0, src1, src2, count);
	scale3x_16_def_border(dst2, src2, src1, src0, count);
#endif
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
void scale3x_32_def(uint32_t* dst0, uint32_t* dst1, uint32_t* dst2,
		    const uint32_t* src0, const uint32_t* src1, const uint32_t* src2,
		    unsigned int count)
{
#ifdef USE_SCALE_RANDOMWRITE
	scale3x_32_def_whole(dst0, dst1, dst2, src0, src1, src2, count);
#else
	scale3x_32_def_border(dst0, src0, src1, src2, count);
	scale3x_32_def_center(dst1, src0, src1, src2, count);
	scale3x_32_def_border(dst2, src2, src1, src0, count);
#endif
}

