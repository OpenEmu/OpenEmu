/***************************************************************************
 * Gens: [MDP] Scanline renderer.                                          *
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

#include "mdp_render_scanline.hpp"
#include <string.h>
#include <stdint.h>

// CPU flags
#include "plugins/mdp_cpuflags.h"

// x86 asm versions
#ifdef GENS_X86_ASM
#include "mdp_render_scanline_x86.h"
#endif /* GENS_X86_ASM */


#ifndef GENS_X86_ASM
/**
 * T_mdp_render_scanline_cpp: Blits the image to the screen, 2x size, scanlines.
 * @param destScreen Pointer to the screen buffer.
 * @param mdScreen Pointer to the MD screen buffer.
 * @param destPitch Pitch of destScreen.
 * @param srcPitch Pitch of mdScreen.
 * @param width Width of the image.
 * @param height Height of the image.
 */
template<typename pixel>
static inline void T_mdp_render_scanline_cpp(pixel *destScreen, pixel *mdScreen,
					     int destPitch, int srcPitch,
					     int width, int height)
{
	// Pitch difference.
	destPitch /= sizeof(pixel);
	int nextLine = destPitch + (destPitch - (width * 2));
	
	srcPitch /= sizeof(pixel);
	
	pixel *line = destScreen;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			*line++ = *mdScreen;
			*line++ = *mdScreen++;
		}
		
		// Next line.
		mdScreen += (srcPitch - width);
		line += nextLine;
	}
	
	// Zero the extra lines.
	line = destScreen + destPitch;
	for (unsigned short i = 0; i < height; i++)
	{
		memset(line, 0x00, width * 2 * sizeof(pixel));
		line += (destPitch * 2);
	}
}
#endif /* GENS_X86_ASM */


void MDP_FNCALL mdp_render_scanline_cpp(MDP_Render_Info_t *renderInfo)
{
	if (!renderInfo)
		return;
	
	if (renderInfo->bpp == 16 || renderInfo->bpp == 15)
	{
#ifdef GENS_X86_ASM
		if (renderInfo->cpuFlags & MDP_CPUFLAG_MMX)
		{
			mdp_render_scanline_16_x86_mmx(
				    (uint16_t*)renderInfo->destScreen,
				    (uint16_t*)renderInfo->mdScreen,
				    renderInfo->destPitch, renderInfo->srcPitch,
				    renderInfo->width, renderInfo->height);
		}
		else
		{
			mdp_render_scanline_16_x86(
				    (uint16_t*)renderInfo->destScreen,
				    (uint16_t*)renderInfo->mdScreen,
				    renderInfo->destPitch, renderInfo->srcPitch,
				    renderInfo->width, renderInfo->height);
		}
#else /* !GENS_X86_ASM */
		T_mdp_render_scanline_cpp(
			    (uint16_t*)renderInfo->destScreen,
			    (uint16_t*)renderInfo->mdScreen,
			    renderInfo->destPitch, renderInfo->srcPitch,
			    renderInfo->width, renderInfo->height);
#endif /* GENS_X86_ASM */
	}
	else
	{
#ifdef GENS_X86_ASM
		if (renderInfo->cpuFlags & MDP_CPUFLAG_MMX)
		{
			mdp_render_scanline_32_x86_mmx(
				    (uint32_t*)renderInfo->destScreen,
				    (uint32_t*)renderInfo->mdScreen,
				    renderInfo->destPitch, renderInfo->srcPitch,
				    renderInfo->width, renderInfo->height);
		}
		else
		{
			mdp_render_scanline_32_x86(
				    (uint32_t*)renderInfo->destScreen,
				    (uint32_t*)renderInfo->mdScreen,
				    renderInfo->destPitch, renderInfo->srcPitch,
				    renderInfo->width, renderInfo->height);
		}
#else /* !GENS_X86_ASM */
		T_mdp_render_scanline_cpp(
			    (uint32_t*)renderInfo->destScreen,
			    (uint32_t*)renderInfo->mdScreen,
			    renderInfo->destPitch, renderInfo->srcPitch,
			    renderInfo->width, renderInfo->height);
#endif /* GENS_X86_ASM */
	}
}
