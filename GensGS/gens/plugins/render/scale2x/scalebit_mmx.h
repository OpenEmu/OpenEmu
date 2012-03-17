/*
 * This file is part of the Scale2x project.
 *
 * Copyright (C) 2003 Andrea Mazzoleni
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
 * This file contains an example implementation of the Scale effect
 * applyed to a generic bitmap.
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

#ifndef __SCALEBIT_MMX_H
#define __SCALEBIT_MMX_H

// Scale2x uses GNU inline assembler.
#if defined(__GNUC__) && defined(__i386__)

#ifdef __cplusplus
extern "C" {
#endif

void scale2x_mmx(void* void_dst, unsigned int dst_slice,
		 const void* void_src, unsigned int src_slice,
		 unsigned int pixel, unsigned int width, unsigned int height);

void scale4x_mmx(void* void_dst, unsigned int dst_slice,
		 const void* void_src, unsigned int src_slice,
		 unsigned int pixel, unsigned int width, unsigned int height);

#ifdef __cplusplus
}
#endif

#endif /* defined(__GNUC__) && defined(__i386__) */

#endif /* __SCALEBIT_H */
