/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - CPU Flag Definitions.                    *
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

// The definitions in this file match gens_core/misc/cpuflags.h.

#ifndef GENS_MDP_CPUFLAGS_H
#define GENS_MDP_CPUFLAGS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

// CPU flags (IA32/x86_64)
#define MDP_CPUFLAG_MMX		(1 << 0)
#define MDP_CPUFLAG_SSE		(1 << 1)
#define MDP_CPUFLAG_SSE2	(1 << 2)
#define MDP_CPUFLAG_SSE3	(1 << 3)
#define MDP_CPUFLAG_SSSE3	(1 << 4)
#define MDP_CPUFLAG_SSE41	(1 << 5)
#define MDP_CPUFLAG_SSE42	(1 << 6)
#define MDP_CPUFLAG_SSE4A	(1 << 7)
#define MDP_CPUFLAG_SSE5	(1 << 8)

// CPU flags (IA32/x86_64; AMD only)
#define MDP_CPUFLAG_MMXEXT	(1 << 9)
#define MDP_CPUFLAG_3DNOW	(1 << 10)
#define MDP_CPUFLAG_3DNOWEXT	(1 << 11)

#endif /* GENS_MDP_CPUFLAGS_H */
