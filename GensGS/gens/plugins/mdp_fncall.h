/***************************************************************************
 * Gens: MDP: Mega Drive Plugin - Function Call Definitions.               *
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

#ifndef GENS_MDP_FNCALL_H
#define GENS_MDP_FNCALL_H

#ifdef __cplusplus
extern "C" {
#endif

// Function pointer calling conventions from SDL's begin_code.h
// TODO: Combine this with the #defines from compress.h
#ifndef MDP_FNCALL
#if defined(__WIN32__) && !defined(__GNUC__)
#define MDP_FNCALL __cdecl
#else
#ifdef __OS2__
#define MDP_FNCALL _System
#else
#define MDP_FNCALL
#endif
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* GENS_MDP_FNCALL_H */
