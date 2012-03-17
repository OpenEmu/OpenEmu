/***************************************************************************
 * Gens: Plugin Manager.                                                   *
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
#endif /* HAVE_CONFIG_H */

#include "pluginmgr.hpp"
#include "macros/hashtable.hpp"

// C++ includes
#include <algorithm>
#include <string>
#include <utility>
#include <vector>
using std::pair;
using std::string;
using std::vector;

// CPU flags
#include "gens_core/misc/cpuflags.h"

// Render plugins
#include "render/normal/mdp_render_1x_plugin.h"
#include "render/double/mdp_render_2x_plugin.h"
#include "render/scanline/mdp_render_scanline_plugin.h"
#include "render/scanline_50/mdp_render_scanline_50_plugin.h"
#include "render/scanline_25/mdp_render_scanline_25_plugin.h"
#include "render/interpolated/mdp_render_interpolated_plugin.h"
#include "render/interpolated_scanline/mdp_render_interpolated_scanline_plugin.h"
#include "render/interpolated_scanline_50/mdp_render_interpolated_scanline_50_plugin.h"
#include "render/interpolated_scanline_25/mdp_render_interpolated_scanline_25_plugin.h"
#include "render/scale2x/mdp_render_scale2x_plugin.h"
#include "render/scale2x/mdp_render_scale3x_plugin.h"
#include "render/scale2x/mdp_render_scale4x_plugin.h"

#ifdef GENS_X86_ASM
#include "render/hq2x/mdp_render_hq2x_plugin.h"
#include "render/hq2x/mdp_render_hq3x_plugin.h"
#include "render/hq2x/mdp_render_hq4x_plugin.h"
#include "render/2xsai/mdp_render_2xsai_plugin.h"
#endif /* GENS_X86_ASM */

// Internal plugins
static MDP_t* mdp_internal[] =
{
	&mdp_render_1x,
	&mdp_render_2x,
	&mdp_render_scanline,
	&mdp_render_scanline_50,
	&mdp_render_scanline_25,
	&mdp_render_interpolated,
	&mdp_render_interpolated_scanline,
	&mdp_render_interpolated_scanline_50,
	&mdp_render_interpolated_scanline_25,
	&mdp_render_scale2x,
	&mdp_render_scale3x,
	&mdp_render_scale4x,
#ifdef GENS_X86_ASM
	&mdp_render_hq2x,
	&mdp_render_hq3x,
	&mdp_render_hq4x,
	&mdp_render_2xsai,
#endif /* GENS_X86_ASM */
	NULL
};


/**
 * vRenderPlugins, tblRenderPlugins: Vector and map containing render plugins.
 */
vector<MDP_t*> PluginMgr::vRenderPlugins;
mapStrToInt PluginMgr::tblRenderPlugins;


/**
 * initPlugin_Render(): Initialize a rendering plugin.
 */
inline void PluginMgr::initPlugin_Render(MDP_t* plugin)
{
	MDP_Render_t *rendPlugin = static_cast<MDP_Render_t*>(plugin->plugin_t);
	
	// Check the render interface version.
	if (MDP_VERSION_MAJOR(rendPlugin->interfaceVersion) !=
	    MDP_VERSION_MAJOR(MDP_RENDER_INTERFACE_VERSION))
	{
		// Incorrect major interface version.
		// TODO: Add to a list of "incompatible" plugins.
		return;
	}
	
	// Check if a plugin with this tag already exists.
	string tag = rendPlugin->tag;
	std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
	mapStrToInt::iterator existingMDP = tblRenderPlugins.find(tag);
	if (existingMDP != tblRenderPlugins.end())
	{
		// Plugin with this tag already exists.
		// TODO: Show an error.
		return;
	}
	
	// TODO: Check the minor version.
	// Probably not needed right now, but may be needed later.
	
	// Add the plugin to the vector.
	vRenderPlugins.push_back(plugin);
	if (plugin->func && plugin->func->init)
		plugin->func->init(&MDP_Host);
	
	// Add the plugin tag to the map.
	tblRenderPlugins.insert(pairStrToInt(tag, vRenderPlugins.size() - 1));
}


/**
 * init(): Initialize the plugin system.
 */
void PluginMgr::init(void)
{
	// Make sure all loaded plugins are shut down.
	end();
	
	// Load all internal plugins.
	unsigned int i = 0;
	while (mdp_internal[i])
	{
		// Check the MDP_t version.
		if (MDP_VERSION_MAJOR(mdp_internal[i]->interfaceVersion) !=
		    MDP_VERSION_MAJOR(MDP_INTERFACE_VERSION))
		{
			// Incorrect major interface version.
			// TODO: Add to a list of "incompatible" plugins.
			i++;
			continue;
		}
		
		// Check required CPU flags.
		uint32_t cpuFlagsRequired = mdp_internal[i]->cpuFlagsRequired;
		if ((cpuFlagsRequired & CPU_Flags) != cpuFlagsRequired)
		{
			// CPU does not support some required CPU flags.
			// TODO: Add to a list of "incompatible" plugins.
			i++;
			continue;
		}
		
		// TODO: Check the minor version.
		// Probably not needed right now, but may be needed later.
		
		switch (mdp_internal[i]->type)
		{
			case MDPT_RENDER:
				initPlugin_Render(mdp_internal[i]);
				break;
			
			default:
				// Unknown plugin type.
				break;
		}
		
		// Next plugin.
		i++;
	}
}

/**
 * end(): Shut down the plugin system.
 */
void PluginMgr::end(void)
{
	// Shut down all render plugins.
	for (unsigned int i = 0; i < vRenderPlugins.size(); i++)
	{
		MDP_Func_t *func = vRenderPlugins.at(i)->func;
		if (func && func->end)
			func->end();
	}
	
	// Clear the vector and map of render plugins.
	vRenderPlugins.clear();
	tblRenderPlugins.clear();
}


/**
 * getPluginFromID_Render(): Get a render plugin from its ID number.
 * @param id ID number.
 * @return Render plugin, or NULL if it wasn't found.
 */
MDP_Render_t* PluginMgr::getPluginFromID_Render(int id)
{
	if (id < 0 || id >= vRenderPlugins.size())
		return NULL;
	
	return static_cast<MDP_Render_t*>(vRenderPlugins.at(id)->plugin_t);
}
