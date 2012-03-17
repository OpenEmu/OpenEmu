/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Base Interface Definitions.              *
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

#ifndef GENS_MDP_H
#define GENS_MDP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Function call definitions.
#include "mdp_fncall.h"

// Version number macros.
#include "mdp_version.h"

// MDP interface version.
#define MDP_INTERFACE_VERSION MDP_VERSION(0, 1, 0)

// Licenses
#define MDP_LICENSE_GPL_2	"GPL-2"
#define MDP_LICENSE_GPL_3	"GPL-3"
#define MDP_LICENSE_LGPL_2	"LGPL-2"
#define MDP_LICENSE_LGPL_21	"LGPL-2.1"
#define MDP_LICENSE_LGPL_3	"LGPL-3"
#define MDP_LICENSE_BSD		"BSD"

// Plugin description struct.
typedef struct
{
	const char* name;
	const char* author_mdp;
	const char* author_orig;
	const char* description;
	const char* website;
	const char* license;
	
	// Filler for alignment purposes.
	const void* reserved1;
	const void* reserved2;
	
	// Icon data. (PNG format)
	const unsigned char* icon;
	const unsigned int iconLength;
} MDP_Desc_t;

// Plugin types.
enum MDP_Type
{
	MDPT_NULL = 0,
	MDPT_RENDER,
};

// MDP Host Services.
#include "mdp_host.h"

// Plugin functions.
typedef void (MDP_FNCALL *mdp_init)(MDP_Host_t *hostSrv);
typedef void (MDP_FNCALL *mdp_end)(void);

// Plugin function struct.
typedef struct
{
	// Init/Shutdown functions
	mdp_init	init;
	mdp_end		end;
} MDP_Func_t;

typedef struct
{
	const uint32_t interfaceVersion;
	const uint32_t pluginVersion;
	const uint32_t type;
	const uint32_t reserved1;
	
	// UUID - each plugin must have a unique ID.
	const unsigned char uuid[16];
	
	// CPU flag information.
	const uint32_t cpuFlagsSupported;
	const uint32_t cpuFlagsRequired;
	
	// Plugin description.
	MDP_Desc_t *desc;
	
	// Plugin functions.
	MDP_Func_t *func;
	
	// Struct definition for the specified plugin type.
	void *plugin_t;
} MDP_t;


#ifdef __cplusplus
}
#endif

#endif /* GENS_MDP_H */
