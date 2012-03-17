/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Host Services.                           *
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

#include "mdp_host.h"
#include <stdio.h>
#include <stdlib.h>

// Host service functions.
void* mdp_host_ref_ptr(MDP_PTR ptrID);
void  mdp_host_unref_ptr(MDP_PTR ptrID);


// MDP_PTR functions.
static inline void* mdp_host_ref_ptr_LUT16to32(void);
static inline void  mdp_host_unref_ptr_LUT16to32(void);
static int* mdp_ptr_LUT16to32 = NULL;
static int  mdp_ptr_LUT16to32_count = 0;


MDP_Host_t MDP_Host =
{
	.interfaceVersion = MDP_HOST_INTERFACE_VERSION,
	
	.refPtr = mdp_host_ref_ptr,
	.unrefPtr = mdp_host_unref_ptr,
};


/**
 * mdp_host_ref_ptr(): Reference a pointer.
 * @param ptrID Pointer ID.
 * @return Pointer.
 */
void* mdp_host_ref_ptr(MDP_PTR ptrID)
{
	switch (ptrID)
	{
		case MDP_PTR_LUT16to32:
			return mdp_host_ref_ptr_LUT16to32();
		
		default:
			fprintf(stderr, "%s: Invalid ptrID: 0x%08X\n", __func__, ptrID);
			return NULL;
	}
}

/**
 * mdp_host_unref_ptr(): Unreference a pointer.
 * @param ptrID Pointer ID.
 * @return Pointer.
 */
void mdp_host_unref_ptr(MDP_PTR ptrID)
{
	switch (ptrID)
	{
		case MDP_PTR_LUT16to32:
			mdp_host_unref_ptr_LUT16to32();
			break;
		
		default:
			fprintf(stderr, "%s: Invalid ptrID: 0x%08X\n", __func__, ptrID);
			break;
	}
}


/**
 * mdp_host_ref_ptr_LUT16to32(): Get a reference for LUT16to32.
 * @return LUT16to32.
 */
static inline void* mdp_host_ref_ptr_LUT16to32(void)
{
	if (!mdp_ptr_LUT16to32)
	{
		// Initialize the lookup table.
		mdp_ptr_LUT16to32 = (int*)(malloc(65536 * sizeof(int)));
		
		// Initialize the 16-bit to 32-bit conversion table.
		int i;
		for (i = 0; i < 65536; i++)
			mdp_ptr_LUT16to32[i] = ((i & 0xF800) << 8) + ((i & 0x07E0) << 5) + ((i & 0x001F) << 3);
	}
	
	// Increment the reference counter.
	mdp_ptr_LUT16to32_count++;
	
	// Return the pointer.
	return mdp_ptr_LUT16to32;
}

/**
 * mdp_host_unref_ptr_LUT16to32(): Unreference LUT16to32.
 * @return LUT16to32.
 */
static inline void mdp_host_unref_ptr_LUT16to32(void)
{
	// Decrement the reference counter.
	mdp_ptr_LUT16to32_count--;
	
	if (mdp_ptr_LUT16to32_count <= 0)
	{
		// All references are gone. Free the lookup table.
		free(mdp_ptr_LUT16to32);
		mdp_ptr_LUT16to32 = NULL;
	}
}
