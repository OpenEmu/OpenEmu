/***************************************************************************
 * Gens: [MDP] Interpolated 25% Scanline renderer. (Plugin Data File)      *
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

#include <stdint.h>
#include <string.h>

#include "plugins/mdp.h"
#include "plugins/mdp_cpuflags.h"

#include "mdp_render_interpolated_scanline_25.hpp"
#include "mdp_render_interpolated_scanline_25_icon.h"

static MDP_Desc_t MDP_Desc =
{
	.name = "Interpolated 25% Scanline Renderer",
	.author_mdp = "David Korth",
	.author_orig = "Stéphane Dallongeville",
	.description = "Interpolated 25% scanline renderer.",
	.website = NULL,
	.license = MDP_LICENSE_GPL_2,
	
	.icon = mdp_render_interpolated_scanline_25_icon,
	.iconLength = sizeof(mdp_render_interpolated_scanline_25_icon)
};

static MDP_Render_t MDP_Render =
{
	.interfaceVersion = MDP_RENDER_INTERFACE_VERSION,
	.blit = mdp_render_interpolated_scanline_25_cpp,
	.scale = 2,
	.flags = 0,
	.tag = "Interpolated 25% Scanline"
};

MDP_t mdp_render_interpolated_scanline_25 =
{
	.interfaceVersion = MDP_INTERFACE_VERSION,
	.pluginVersion = MDP_VERSION(0, 1, 0),
	.type = MDPT_RENDER,
	
	// UUID: d38a3b65-3dd2-4ae0-a741-a749535d7b87
	.uuid = {0xD3, 0x8A, 0x3B, 0x65,
		 0x3D, 0xD2,
		 0x4A, 0xE0,
		 0xA7, 0x41,
		 0xA7, 0x49, 0x53, 0x5D, 0x7B, 0x87},
	
	// CPU flags
	.cpuFlagsSupported = 0,
	.cpuFlagsRequired = 0,
	
	// Description
	.desc = &MDP_Desc,
	
	// Functions
	.func = NULL,
	
	.plugin_t = (void*)&MDP_Render
};
