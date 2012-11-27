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

#include "lure/memory.h"
#include "common/file.h"
#include "common/textconsole.h"

namespace Lure {

MemoryBlock *Memory::allocate(uint32 size) {
	MemoryBlock *block = new MemoryBlock(size);
	return block;
}

MemoryBlock *Memory::duplicate(MemoryBlock *src) {
	MemoryBlock *block = new MemoryBlock(src);
	return block;
}

void *Memory::alloc(uint32 size) {
	return malloc(size);
}

void Memory::dealloc(void *block) {
	free(block);
}

/*--------------------------------------------------------------------------*/

MemoryBlock::MemoryBlock(uint32 size1) {
	_data = (uint8 *) malloc(size1);
	if (!_data) error ("Failed allocating memory block");
	_size = size1;
}

MemoryBlock::MemoryBlock(MemoryBlock *src) {
	_size = src->size();
	_data = (uint8 *) malloc(_size);
	if (!_data) error ("Failed allocating memory block");
	memcpy(_data, src->data(), _size);
}

MemoryBlock::~MemoryBlock() {
	free(_data);
}

void MemoryBlock::empty() {
	::memset(_data, 0, _size);
}

void MemoryBlock::setBytes(int c, size_t startIndex, size_t num) {
	byte *p = _data + startIndex;
	::memset(p, c, num);
}

void MemoryBlock::copyFrom(MemoryBlock *src) {
	copyFrom(src, 0, 0, src->size());
}

void MemoryBlock::copyFrom(MemoryBlock *src, uint32 srcPos, uint32 destPos, uint32 srcLen) {
	if ((srcPos + srcLen > src->size()) || (destPos + srcLen > size()))
		error("Memory block overrun in block copy");

	uint8 *pDest = _data + destPos;
	uint8 *pSrc = src->data() + srcPos;
	memcpy(pDest, pSrc, srcLen);
}

void MemoryBlock::copyFrom(const byte *src, uint32 srcPos, uint32 destPos, uint32 srcLen) {
	byte *pDest = _data + destPos;
	const byte *pSrc = src + srcPos;
	memcpy(pDest, pSrc, srcLen);
}

void MemoryBlock::reallocate(uint32 size1) {
	_size = size1;

	byte *tmp = (byte *) realloc(_data, size1);
	if (!tmp)
		error ("[MemoryBlock::reallocate] Failed reallocating memory block");

	_data = tmp;
}

} // End of namespace Lure
