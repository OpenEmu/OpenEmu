/***************************************************************************
 * Gens: [MDP] Interpolated Scanline renderer. (Plugin Data File)          *
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

#include "mdp_render_interpolated_scanline.hpp"
#include "mdp_render_interpolated_scanline_icon.h"

static MDP_Desc_t MDP_Desc =
{
	.name = "Interpolated Scanline Renderer",
	.author_mdp = "David Korth",
	.author_orig = "Stéphane Dallongeville",
	.description = "Interpolated scanline renderer.",
	.website = NULL,
	.license = MDP_LICENSE_GPL_2,
	
	.icon = mdp_render_interpolated_scanline_icon,
	.iconLength = sizeof(mdp_render_interpolated_scanline_icon)
};

static MDP_Render_t MDP_Render =
{
	.interfaceVersion = MDP_RENDER_INTERFACE_VERSION,
	.blit = mdp_render_interpolated_scanline_cpp,
	.scale = 2,
	.flags = 0,
	.tag = "Interpolated Scanline"
};

MDP_t mdp_render_interpolated_scanline =
{
	.interfaceVersion = MDP_INTERFACE_VERSION,
	.pluginVersion = MDP_VERSION(0, 1, 0),
	.type = MDPT_RENDER,
	
	// UUID: f3c528ec-3dfa-4c83-bbef-00ecc3ed0e95
	.uuid = {0xF3, 0xC5, 0x28, 0xEC,
		 0x3D, 0xFA,
		 0x4C, 0x83,
		 0xBB, 0xEF,
		 0x00, 0xEC, 0xC3, 0xED, 0x0E, 0x95},
	
	// CPU flags
	.cpuFlagsSupported = MDP_CPUFLAG_MMX,
	.cpuFlagsRequired = 0,
	
	// Description
	.desc = &MDP_Desc,
	
	// Functions
	.func = NULL,
	
	.plugin_t = (void*)&MDP_Render
};
