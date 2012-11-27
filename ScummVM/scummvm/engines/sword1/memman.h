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
 */

#ifndef SWORD1_MEMMAN_H
#define SWORD1_MEMMAN_H

#include "common/scummsys.h"

namespace Sword1 {

struct MemHandle {
	void *data;
	uint32 size;
	uint32 refCount;
	uint16 cond;
	MemHandle *next, *prev;
};
// mem conditions:
#define MEM_FREED       0
#define MEM_CAN_FREE    1
#define MEM_DONT_FREE   2

#define MAX_ALLOC (6*1024*1024) // max amount of mem we want to alloc().

class MemMan {
public:
	MemMan();
	~MemMan();
	void alloc(MemHandle *bsMem, uint32 pSize, uint16 pCond = MEM_DONT_FREE);
	void setCondition(MemHandle *bsMem, uint16 pCond);
	void freeNow(MemHandle *bsMem);
	void initHandle(MemHandle *bsMem);
	void flush();
private:
	void addToFreeList(MemHandle *bsMem);
	void removeFromFreeList(MemHandle *bsMem);
	void checkMemoryUsage();
	uint32 _alloced;  //currently allocated memory
	MemHandle *_memListFree;
	MemHandle *_memListFreeEnd;
};

} // End of namespace Sword1

#endif //MEMMAN_H
