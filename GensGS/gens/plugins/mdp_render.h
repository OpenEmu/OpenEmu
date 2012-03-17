/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Render Plugin Interface Definitions.     *
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

#ifndef GENS_MDP_RENDER_H
#define GENS_MDP_RENDER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "mdp_fncall.h"
#include "mdp_version.h"

// MDP Render Plugin interface version.
#define MDP_RENDER_INTERFACE_VERSION MDP_VERSION(0, 1, 0)

// Gens rendering info
typedef struct
{
	// Screen buffers.
	void *destScreen;
	void *mdScreen;
	
	// Screen buffer pitch.
	int destPitch;
	int srcPitch;
	
	// Image parameters.
	int width;
	int height;
	
	// Current bpp.
	uint32_t bpp;
	
	// CPU flags.
	uint32_t cpuFlags;
	
	// Rendering flags. Currently unused.
	uint32_t renderFlags;
} MDP_Render_Info_t;

// Render plugin flags.

// SRC16DST32: Plugin only supports 16-bit color; Gens/GS has to convert it to 32-bit.
#define MDP_RENDER_FLAG_SRC16DST32	(1 << 0)

// Render plugin definition.
typedef void (MDP_FNCALL *MDP_Render_Fn)(MDP_Render_Info_t *renderInfo);
typedef struct
{
	// Render interface version.
	const uint32_t interfaceVersion;
	
	// Blit function.
	MDP_Render_Fn blit;
	
	// Scaling ratio. (1 == 320x240; 2 = 640x480; etc)
	const int scale;
	
	// Render flags.
	const uint32_t flags;
	
	// Render tag.
	const char* tag;
} MDP_Render_t;


#ifdef __cplusplus
}
#endif

#endif /* GENS_MDP_RENDER_H */
