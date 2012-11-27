/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef	SWORD2_MEMORY_H
#define	SWORD2_MEMORY_H

enum {
	MAX_MEMORY_BLOCKS = 999
};

namespace Sword2 {

struct MemBlock {
	int16 id;
	int16 uid;
	byte *ptr;
	uint32 size;
};

class MemoryManager {
private:
	Sword2Engine *_vm;

	MemBlock *_memBlocks;
	MemBlock **_memBlockIndex;
	int16 _numBlocks;

	uint32 _totAlloc;

	int16 *_idStack;
	int16 _idStackPtr;

	int16 findExactPointerInIndex(byte *ptr);
	int16 findPointerInIndex(byte *ptr);
	int16 findInsertionPointInIndex(byte *ptr);

public:
	MemoryManager(Sword2Engine *vm);
	~MemoryManager();

	int16 getNumBlocks() { return _numBlocks; }
	uint32 getTotAlloc() { return _totAlloc; }
	MemBlock *getMemBlocks() { return _memBlocks; }

	int32 encodePtr(byte *ptr);
	byte *decodePtr(int32 n);

	byte *memAlloc(uint32 size, int16 uid);
	void memFree(byte *ptr);
};

} // End of namespace Sword2

#endif
