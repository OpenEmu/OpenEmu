/***************************************************************************
 * Gens: [MDP] Scale2x renderer.                                           *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 * Scale2x Copyright (c) 2001 by Andrea Mazzoleni                          *
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

#include "mdp_render_scale2x.h"
#include <string.h>
#include <stdint.h>

// Scale2x frontend.
#include "scalebit.h"
#include "scalebit_mmx.h"

// CPU flags
#include "plugins/mdp_cpuflags.h"

// TODO: Conditionalize MMX.
// Scale2x always defaults to using MMX right now.

void MDP_FNCALL mdp_render_scale2x_cpp(MDP_Render_Info_t *renderInfo)
{
	if (!renderInfo)
		return;

	const unsigned int bytespp = (renderInfo->bpp == 15 ? 2 : renderInfo->bpp / 8);
	
#if defined(__GNUC__) && defined(__i386__)
	if (renderInfo->cpuFlags & MDP_CPUFLAG_MMX)
	{
		scale2x_mmx(renderInfo->destScreen, renderInfo->destPitch,
			    renderInfo->mdScreen, renderInfo->srcPitch,
			    bytespp, renderInfo->width, renderInfo->height);
	}
	else
#endif /* defined(__GNUC__) && defined(__i386__) */
	{
		scale2x(renderInfo->destScreen, renderInfo->destPitch,
			renderInfo->mdScreen, renderInfo->srcPitch,
			bytespp, renderInfo->width, renderInfo->height);
	}
}
