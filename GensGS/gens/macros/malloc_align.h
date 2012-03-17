/***************************************************************************
 * Gens: Aligned memory allocation macros.                                 *
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

#ifndef GENS_MALLOC_ALIGN_H
#define GENS_MALLOC_ALIGN_H

#ifdef GENS_OS_WIN32

#include <malloc.h>
static inline void* gens_malloc_align(size_t size, size_t alignment)
{
	return _aligned_malloc(size, alignment);
}

#else /* !GENS_OS_WIN32 */

#include <stdlib.h>
static inline void* gens_malloc_align(size_t size, size_t alignment)
{
#if _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600
	void *mem;
	posix_memalign(&mem, alignment, size);
	return mem;
#else
	// TODO: Write a wrapper class for aligned malloc.
	((void)alignment);
	return malloc(size);
#endif
}

#endif /* GENS_OS_WIN32 */

#endif /* GENS_MALLOC_ALIGN_H */
