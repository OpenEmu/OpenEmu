/***************************************************************************
 * Gens: Inline video functions.                                           *
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

#ifndef GENS_V_INLINE_H
#define GENS_V_INLINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "emulator/g_main.hpp"
#include "gens_core/vdp/vdp_io.h"

// Inline functions for determining MD resolution settings.
static inline int isFullXRes(void)
{
	return ((VDP_Reg.Set4 & 0x1) || Debug || !Game /*|| !FrameCount*/);
}
static inline int isFullYRes(void)
{
	return ((VDP_Reg.Set2 & 0x8) || Debug || !Game /*|| !FrameCount*/);
}

#ifdef __cplusplus
}
#endif

#endif /* GENS_V_INLINE_HPP */
