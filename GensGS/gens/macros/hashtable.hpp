/***************************************************************************
 * Gens: Hash table macros.                                                *
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

#ifndef GENS_HASHTABLE_HPP
#define GENS_HASHTABLE_HPP

#ifdef __cplusplus

#if defined(GENS_HASHTABLE_TR1_UNORDERED_MAP)

// TR1 unordered_map
#include <tr1/unordered_map>
#define GENS_HASHTABLE std::tr1::unordered_map

#elif defined(GENS_HASHTABLE_GNU_HASH_MAP)

// GNU hash_map
#include <ext/hash_map>
#define GENS_HASHTABLE __gnu_cxx::hash_map

#elif defined(GENS_HASHTABLE_STD_MAP)

// Standard std::map
#include <map>
#define GENS_HASHTABLE std::map

#else

// No hash table macro defined.
#error No hash table macro was defined.

#endif

// Useful predefined hashtables and pairs.
#include <string>
#include <utility>

// StrToInt: Used to map strings to ints.
typedef GENS_HASHTABLE<std::string, int> mapStrToInt;
typedef std::pair<std::string, int> pairStrToInt;

#endif /* __cplusplus */

#endif /* GENS_HASHTABLE_HPP */
