/***************************************************************************
 * Gens: [MDP] Interpolated 25% Scanline renderer.                         *
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
#endif

#include "mdp_render_interpolated_scanline_25.hpp"
#include <string.h>
#include <stdint.h>

// CPU flags
#include "plugins/mdp_cpuflags.h"

#undef GENS_X86_ASM
// x86 asm versions
#ifdef GENS_X86_ASM
#include "mdp_render_interpolated_scanline_25_x86.h"
#endif /* GENS_X86_ASM */

// Mask constants
#define MASK_DIV2_15		((uint16_t)(0x3DEF))
#define MASK_DIV2_16		((uint16_t)(0x7BEF))
#define MASK_DIV2_15_ASM	((uint32_t)(0x3DEF3DEF))
#define MASK_DIV2_16_ASM	((uint32_t)(0x7BEF7BEF))
#define MASK_DIV2_32		((uint32_t)(0x7F7F7F7F))

#define MASK_DIV4_15		((uint16_t)(0x1CE7))
#define MASK_DIV4_16		((uint16_t)(0x39E7))
#define MASK_DIV4_15_ASM	((uint32_t)(0x1CE71CE7))
#define MASK_DIV4_16_ASM	((uint32_t)(0x39E739E7))
#define MASK_DIV4_32		((uint32_t)(0x3F3F3F3F))

#define BLEND(a, b, mask) ((((a) >> 1) & mask) + (((b) >> 1) & mask))


//#ifndef GENS_X86_ASM
/**
 * T_mdp_render_interpolated_scanline_25_cpp: Blits the image to the screen, 2x size, interpolation with 25% scanlines.
 * @param destScreen Pointer to the destination screen buffer.
 * @param mdScreen Pointer to the MD screen buffer.
 * @param destPitch Pitch of destScreen.
 * @param srcPitch Pitch of mdScreen.
 * @param width Width of the image.
 * @param height Height of the image.
 * @param mask2 50% mask for the interpolation data.
 * @param mask4 25% mask for the interpolation data.
 */
template<typename pixel>
static inline void T_mdp_render_interpolated_scanline_25_cpp(pixel *destScreen, pixel *mdScreen,
							     int destPitch, int srcPitch,
							     int width, int height, pixel mask2, pixel mask4)
{
	destPitch /= sizeof(pixel);
	srcPitch /= sizeof(pixel);
	
	// TODO: Figure out why the interpolation function is using the line
	// below the source line instead of using the current source line.
	for (int y = 0; y < height; y++)
	{
		pixel *SrcLine = &mdScreen[y * srcPitch];
		pixel *DstLine1 = &destScreen[(y * 2) * destPitch];
		pixel *DstLine2 = &destScreen[((y * 2) + 1) * destPitch];
		
		for (int x = 0; x < width; x++)
		{
			pixel C = *(SrcLine);
			pixel R = *(SrcLine + 1);
			pixel D = *(SrcLine + srcPitch);
			pixel DR = *(SrcLine + srcPitch + 1);
			
			*DstLine1++ = C;
			*DstLine1++ = BLEND(C, R, mask2);
			
			pixel tmpD = BLEND(C, D, mask2);
			*DstLine2++ = ((tmpD >> 1) & mask2) + ((tmpD >> 2) & mask4);
			
			pixel tmpDR = BLEND(BLEND(C, R, mask2), BLEND(D, DR, mask2), mask2);
			*DstLine2++ = ((tmpDR >> 1) & mask2) + ((tmpDR >> 2) & mask4);
			
			SrcLine++;
		}
	}
}
//#endif /* GENS_X86_ASM */


void MDP_FNCALL mdp_render_interpolated_scanline_25_cpp(MDP_Render_Info_t *renderInfo)
{
	if (!renderInfo)
		return;
	
	if (renderInfo->bpp == 16 || renderInfo->bpp == 15)
	{
#ifdef GENS_X86_ASM
		if (renderInfo->cpuFlags & MDP_CPUFLAG_MMX)
		{
			mdp_render_interpolated_scanline_25_16_x86_mmx(
				    (uint16_t*)renderInfo->destScreen,
				    (uint16_t*)renderInfo->mdScreen,
				    renderInfo->destPitch, renderInfo->srcPitch,
				    renderInfo->width, renderInfo->height,
				    (renderInfo->bpp == 15));
		}
		else
#endif /* GENS_X86_ASM */
		{
			T_mdp_render_interpolated_scanline_25_cpp(
				    (uint16_t*)renderInfo->destScreen,
				    (uint16_t*)renderInfo->mdScreen,
					    renderInfo->destPitch, renderInfo->srcPitch,
					    renderInfo->width, renderInfo->height,
				    (renderInfo->bpp == 15 ? MASK_DIV2_15 : MASK_DIV2_16),
				    (renderInfo->bpp == 15 ? MASK_DIV4_15 : MASK_DIV4_16));
		}
	}
	else
	{
		T_mdp_render_interpolated_scanline_25_cpp(
			    (uint32_t*)renderInfo->destScreen,
			    (uint32_t*)renderInfo->mdScreen,
			    renderInfo->destPitch, renderInfo->srcPitch,
			    renderInfo->width, renderInfo->height,
			    MASK_DIV2_32, MASK_DIV4_32);
	}
}
