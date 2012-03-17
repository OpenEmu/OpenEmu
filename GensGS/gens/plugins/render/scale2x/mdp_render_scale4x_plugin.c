/***************************************************************************
 * Gens: [MDP] Scale4x renderer. (Plugin Data File)                        *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 * Scale4x Copyright (c) 2001 by Andrea Mazzoleni                          *
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

#include "mdp_render_scale4x.h"
#include "mdp_render_scale2x_icon.h"

static MDP_Desc_t MDP_Desc =
{
	.name = "Scale4x Renderer",
	.author_mdp = "David Korth",
	.author_orig = "Andrea Mazzoleni",
	.description = "Scale4x v2.2.",
	.website = "http://scale2x.sourceforge.net/",
	.license = MDP_LICENSE_GPL_2,
	
	.icon = mdp_render_scale2x_icon,
	.iconLength = sizeof(mdp_render_scale2x_icon)
};

static MDP_Render_t MDP_Render =
{
	.interfaceVersion = MDP_RENDER_INTERFACE_VERSION,
	.blit = mdp_render_scale4x_cpp,
	.scale = 4,
	.flags = 0,
	.tag = "Scale4x"
};

MDP_t mdp_render_scale4x =
{
	.interfaceVersion = MDP_INTERFACE_VERSION,
	.pluginVersion = MDP_VERSION(0, 1, 0),
	.type = MDPT_RENDER,
	
	// UUID: 46e175f5-0949-40c2-b414-6635e0e550ee
	.uuid = {0x46, 0xE1, 0x75, 0xF5,
		 0x09, 0x49,
		 0x40, 0xC2,
		 0xB4, 0x14,
		 0x66, 0x35, 0xE0, 0xE5, 0x50, 0xEE},
	
	// CPU flags
	.cpuFlagsSupported = MDP_CPUFLAG_MMX,
	.cpuFlagsRequired = 0,
	
	// Description
	.desc = &MDP_Desc,
	
	// Functions
	.func = NULL,
	
	.plugin_t = (void*)&MDP_Render
};
