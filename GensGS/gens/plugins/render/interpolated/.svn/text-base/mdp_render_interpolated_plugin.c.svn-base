/***************************************************************************
 * Gens: [MDP] Interpolated renderer. (Plugin Data File)                   *
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

#include "mdp_render_interpolated.hpp"
#include "mdp_render_interpolated_icon.h"

static MDP_Desc_t MDP_Desc =
{
	.name = "Interpolated Renderer",
	.author_mdp = "David Korth",
	.author_orig = "Stéphane Dallongeville",
	.description = "Interpolated renderer.",
	.website = NULL,
	.license = MDP_LICENSE_GPL_2,
	
	.icon = mdp_render_interpolated_icon,
	.iconLength = sizeof(mdp_render_interpolated_icon)
};

static MDP_Render_t MDP_Render =
{
	.interfaceVersion = MDP_RENDER_INTERFACE_VERSION,
	.blit = mdp_render_interpolated_cpp,
	.scale = 2,
	.flags = 0,
	.tag = "Interpolated"
};

MDP_t mdp_render_interpolated =
{
	.interfaceVersion = MDP_INTERFACE_VERSION,
	.pluginVersion = MDP_VERSION(0, 1, 0),
	.type = MDPT_RENDER,
	
	// UUID: fd39b461-9488-4eff-b401-a9e764fdc0b7
	.uuid = {0xFD, 0x39, 0xB4, 0x61,
		 0x94, 0x88,
		 0x4E, 0xFF,
		 0xB4, 0x01,
		 0xA9, 0xE7, 0x64, 0xFD, 0xC0, 0xB7},
	
	// CPU flags
	.cpuFlagsSupported = 0,
	.cpuFlagsRequired = 0,
	
	// Description
	.desc = &MDP_Desc,
	
	// Functions
	.func = NULL,
	
	.plugin_t = (void*)&MDP_Render
};
