/***************************************************************************
 * Gens: Fast Blur function.                                               *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "fastblur.hpp"
#include "fastblur_x86.h"
#include "emulator/g_main.hpp"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/misc/cpuflags.h"

// Mask constants
#define MASK_DIV2_15		((uint16_t)(0x3DEF))
#define MASK_DIV2_16		((uint16_t)(0x7BEF))
#define MASK_DIV2_15_ASM	((uint32_t)(0x3DEF3DEF))
#define MASK_DIV2_16_ASM	((uint32_t)(0x7BEF7BEF))
#define MASK_DIV2_32		((uint32_t)(0x007F7F7F))


#ifndef GENS_X86_ASM
/**
 * T_Fast_Blur: Apply a fast blur effect to the MD screen buffer.
 * @param mdScreen MD screen buffer.
 * @param mask MSB mask for pixel data.
 */
template<typename pixel>
static inline void T_Fast_Blur(pixel *mdScreen, pixel mask)
{
	pixel color = 0;
	pixel blurColor = 0;
	
	// Start at the 8th pixel.
	// MD screen has an 8-pixel-wide buffer at the left-most side.
	mdScreen += 8;
	
	// Number of pixels to process.
	const int numPixels = (336 * 240) - 8;
	
	for (int i = 0; i < numPixels; i++)
	{
		// Get the current pixel.
		color = (*mdScreen >> 1) & mask;
		
		// Blur the color with the previous pixel.
		blurColor += color;
		
		// Draw the new pixel.
		*(mdScreen - 1) = blurColor;
		
		// Save the color for the next pixel.
		blurColor = color;
		
		// Increment the MD screen pointer.
		mdScreen++;
	}
}
#endif /* GENS_X86_ASM */


/**
 * Fast_Blur: Apply a fast blurring algorithm to the onscreen image.
 */
void Fast_Blur(void)
{
	// TODO: Make it so fast blur doesn't apply to screenshots.
	if (bppMD == 16 || bppMD == 15)
	{
#ifdef GENS_X86_ASM
		if (CPU_Flags & CPUFLAG_MMX)
			Fast_Blur_16_x86_mmx();
		else
			Fast_Blur_16_x86();
#else
		T_Fast_Blur(MD_Screen, (bppMD == 15 ? MASK_DIV2_15 : MASK_DIV2_16));
#endif
	}
	else //if (bppMD == 32)
	{
#ifdef GENS_X86_ASM
		if (CPU_Flags & CPUFLAG_MMX)
			Fast_Blur_32_x86_mmx();
		else
			Fast_Blur_32_x86();
#else
		T_Fast_Blur(MD_Screen32, MASK_DIV2_32);
#endif
	}
}
