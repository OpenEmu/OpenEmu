/***************************************************************************
 * Gens: [MDP] Scale3x renderer. (Plugin Data File)                        *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 * Scale3x Copyright (c) 2001 by Andrea Mazzoleni                          *
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

#include "mdp_render_scale3x.h"
#include "mdp_render_scale2x_icon.h"

static MDP_Desc_t MDP_Desc =
{
	.name = "Scale3x Renderer",
	.author_mdp = "David Korth",
	.author_orig = "Andrea Mazzoleni",
	.description = "Scale3x v2.2.",
	.website = "http://scale2x.sourceforge.net/",
	.license = MDP_LICENSE_GPL_2,
	
	.icon = mdp_render_scale2x_icon,
	.iconLength = sizeof(mdp_render_scale2x_icon)
};

static MDP_Render_t MDP_Render =
{
	.interfaceVersion = MDP_RENDER_INTERFACE_VERSION,
	.blit = mdp_render_scale3x_cpp,
	.scale = 3,
	.flags = 0,
	.tag = "Scale3x"
};

MDP_t mdp_render_scale3x =
{
	.interfaceVersion = MDP_INTERFACE_VERSION,
	.pluginVersion = MDP_VERSION(0, 1, 0),
	.type = MDPT_RENDER,
	
	// UUID: 273b0d9f-aedf-4876-b269-bcb3125d5b48
	.uuid = {0x27, 0x3B, 0x0D, 0x9F,
		 0xAE, 0xDF,
		 0x48, 0x76,
		 0xB2, 0x69,
		 0xBC, 0xB3, 0x12, 0x5D, 0x5B, 0x48},
	
	// CPU flags
	.cpuFlagsSupported = 0,
	.cpuFlagsRequired = 0,
	
	// Description
	.desc = &MDP_Desc,
	
	// Functions
	.func = NULL,
	
	.plugin_t = (void*)&MDP_Render
};
