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

#ifndef LURE_MEMORY_H
#define LURE_MEMORY_H


#include "common/system.h"
#include "common/str.h"

namespace Lure {

class MemoryBlock {
private:
	byte *_data;
	uint32 _size;
public:
	MemoryBlock(uint32 size);
	MemoryBlock(MemoryBlock *src);
	~MemoryBlock();

	byte *data() { return _data; }
	uint32 size() { return _size; }

	void empty();
	void setBytes(int c, size_t startIndex, size_t num);
	void copyFrom(MemoryBlock *src);
	void copyFrom(MemoryBlock *src, uint32 srcPos, uint32 destPos, uint32 srcLen);
	void copyFrom(const byte *src, uint32 srcPos, uint32 destPos, uint32 srcLen);
	void reallocate(uint32 size);
};

class Memory {
public:
	static MemoryBlock *allocate(uint32 size);
	static MemoryBlock *duplicate(MemoryBlock *src);
	static void *alloc(uint32 size);
	static void dealloc(void *block);
};

} // end of namspace Lure

#endif
