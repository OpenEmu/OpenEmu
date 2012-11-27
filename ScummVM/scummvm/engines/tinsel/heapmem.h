/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * This file contains the handle based Memory Manager defines
 */

#ifndef TINSEL_HEAPMEM_H
#define TINSEL_HEAPMEM_H

#include "tinsel/dw.h"		// new data types

namespace Tinsel {

struct MEM_NODE;


/*----------------------------------------------------------------------*\
|*			Memory Function Prototypes			*|
\*----------------------------------------------------------------------*/

void MemoryInit();			// initializes the memory manager
void MemoryDeinit();		// deinitializes the memory manager

// reserves a memory node for a movable & discardable block
MEM_NODE *MemoryNoAlloc();

// allocates a fixed block with the specified number of bytes
MEM_NODE *MemoryAllocFixed(long size);

void MemoryDiscard(		// discards the specified memory object
	MEM_NODE *pMemNode);	// node of the memory object

void *MemoryLock(		// locks a memory object and returns a pointer to the first byte of the objects memory block
	MEM_NODE *pMemNode);	// node of the memory object

void MemoryReAlloc(	// changes the size or attributes of a specified memory object
	MEM_NODE *pMemNode,	// node of the memory object
	long size);		// new size of block

void MemoryUnlock(		// unlocks a memory object
	MEM_NODE *pMemNode);	// node of the memory object

// 'touch' the memory object, i.e., update its "least recently used" counter.
void MemoryTouch(MEM_NODE *pMemNode);

// Dereference a given memory node
uint8 *MemoryDeref(MEM_NODE *pMemNode);

} // End of namespace Tinsel

#endif
