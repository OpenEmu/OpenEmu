/***************************************************************************
 * Gens: [MDP] 2xSaI renderer. (Plugin Data File)                          *
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

#include <stdint.h>
#include <string.h>

#include "plugins/mdp.h"
#include "plugins/mdp_cpuflags.h"

#include "mdp_render_2xsai.h"

static MDP_Desc_t MDP_Desc =
{
	.name = "2xSaI Renderer",
	.author_mdp = "David Korth",
	.author_orig = "Derek Liauw Kie Fa and Robert J. Ohannessian",
	.description = "2x Scale and Interpolation renderer.",
	.website = NULL,
	.license = MDP_LICENSE_LGPL_21
};

static MDP_Render_t MDP_Render =
{
	.interfaceVersion = MDP_RENDER_INTERFACE_VERSION,
	.blit = mdp_render_2xsai_cpp,
	.scale = 2,
	.flags = MDP_RENDER_FLAG_SRC16DST32,
	.tag = "2xSaI"
};

MDP_t mdp_render_2xsai =
{
	.interfaceVersion = MDP_INTERFACE_VERSION,
	.pluginVersion = MDP_VERSION(0, 1, 0),
	.type = MDPT_RENDER,
	
	// UUID: 5c03f2ff-1c20-4f9b-b81c-458de6c7880d
	.uuid = {0x5C, 0x03, 0xF2, 0xFF,
		 0x1C, 0x20,
		 0x4F, 0x9B,
		 0xB8, 0x1C,
		 0x45, 0x8D, 0xE6, 0xC7, 0x88, 0x0D},
	
	// CPU flags
	.cpuFlagsSupported = MDP_CPUFLAG_MMX,
	.cpuFlagsRequired = MDP_CPUFLAG_MMX,
	
	// Description
	.desc = &MDP_Desc,
	
	// Functions
	.func = NULL,
	
	.plugin_t = (void*)&MDP_Render
};
