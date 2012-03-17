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

#ifndef GENS_PLUGINMGR_HPP
#define GENS_PLUGINMGR_HPP

#include "mdp.h"
#include "mdp_render.h"

#include <vector>
#include "macros/hashtable.hpp"

class PluginMgr
{
	public:
		static void init(void);
		static void end(void);
		
		static MDP_Render_t* getPluginFromID_Render(int id);
		
		static std::vector<MDP_t*> vRenderPlugins;
		static mapStrToInt tblRenderPlugins;
		
	protected:
		static inline void initPlugin_Render(MDP_t *plugin);
};

#endif /* GENS_PLUGINMGR_HPP */
