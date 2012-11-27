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
 *
 */

#ifndef TONY_MPAL_MEMORY
#define TONY_MPAL_MEMORY

#include "common/scummsys.h"
#include "common/list.h"

namespace Tony {

namespace MPAL {

typedef void *MpalHandle;

struct MemoryItem {
	uint32 _id;
	uint32 _size;
	int _lockCount;
	byte _data[1];

	// Casting for access to data
	operator void *() { return &_data[0]; }
};

class MemoryManager {
private:
	static MemoryItem *getItem(MpalHandle handle);
public:
	static MpalHandle allocate(uint32 size, uint flags);
	static void *alloc(uint32 size, uint flags);
	static void freeBlock(MpalHandle handle);
	static void destroyItem(MpalHandle handle);
	static uint32 getSize(MpalHandle handle);
	static byte *lockItem(MpalHandle handle);
	static void unlockItem(MpalHandle handle);
};

// defines
#define globalAlloc(flags, size)    MemoryManager::alloc(size, flags)
#define globalAllocate(flags, size) MemoryManager::allocate(size, flags)
#define globalFree(handle)          MemoryManager::freeBlock(handle)
#define globalDestroy(handle)       MemoryManager::destroyItem(handle)
#define globalLock(handle)          MemoryManager::lockItem(handle)
#define globalUnlock(handle)        MemoryManager::unlockItem(handle)
#define globalSize(handle)          MemoryManager::getSize(handle)

#define GMEM_FIXED 1
#define GMEM_MOVEABLE 2
#define GMEM_ZEROINIT 4

const uint32 BLOCK_ID = 0x12345678;

} // end of namespace MPAL

} // end of namespace Tony

#endif
