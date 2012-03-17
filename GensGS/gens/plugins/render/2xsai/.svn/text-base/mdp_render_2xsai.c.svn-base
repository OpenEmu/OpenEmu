/***************************************************************************
 * Gens: [MDP] 2xSaI renderer.                                             *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 * 2xSaI Copyright (c) by Derek Liauw Kie Fa and Robert J. Ohannessian     *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation; either version 2.1 of the License, or  *
 * (at your option) any later version.                                     *
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

#include "mdp_render_2xsai.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// CPU flags
#include "plugins/mdp_cpuflags.h"

// x86 asm versions
#ifdef GENS_X86_ASM
#include "mdp_render_2xsai_x86.h"
#endif /* GENS_X86_ASM */


/**
 * mdp_render_2xsai_cpp(): 2xSaI rendering function.
 * @param renderInfo Render information.
 */
void MDP_FNCALL mdp_render_2xsai_cpp(MDP_Render_Info_t *renderInfo)
{
	if (!renderInfo)
		return;
	
#ifdef GENS_X86_ASM
	if (renderInfo->cpuFlags & MDP_CPUFLAG_MMX)
	{
		mdp_render_2xsai_16_x86_mmx(
			    (uint16_t*)renderInfo->destScreen,
			    (uint16_t*)renderInfo->mdScreen,
			    renderInfo->destPitch, renderInfo->srcPitch,
			    renderInfo->width, renderInfo->height,
			    (renderInfo->bpp == 15));
	}
#else /* !GENS_X86_ASM */
	T_mdp_render_2xsai_cpp(
		    (uint16_t*)renderInfo->destScreen,
		    (uint16_t*)renderInfo->mdScreen,
		    renderInfo->destPitch, renderInfo->srcPitch,
		    renderInfo->width, renderInfo->height);
#endif /* GENS_X86_ASM */
}
