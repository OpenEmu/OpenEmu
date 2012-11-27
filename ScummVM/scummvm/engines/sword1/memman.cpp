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


#include "sword1/memman.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Sword1 {

MemMan::MemMan() {
	_alloced = 0;
	_memListFree = _memListFreeEnd = NULL;
}

MemMan::~MemMan() {
	flush();
	if (_alloced)
		warning("deleting MemMan, still %d bytes alloced", _alloced);
}

void MemMan::alloc(MemHandle *bsMem, uint32 pSize, uint16 pCond) {
	_alloced += pSize;
	bsMem->data = (void *)malloc(pSize);
	if (!bsMem->data)
		error("MemMan::alloc(): Can't alloc %d bytes of memory.", pSize);
	bsMem->cond = pCond;
	bsMem->size = pSize;
	if (pCond == MEM_CAN_FREE) {
		warning("%d Bytes alloced as FREEABLE.", pSize); // why should one want to alloc mem if it can be freed?
		addToFreeList(bsMem);
	} else if (bsMem->next || bsMem->prev) // it's in our _freeAble list, remove it from there
		removeFromFreeList(bsMem);
	checkMemoryUsage();
}

void MemMan::freeNow(MemHandle *bsMem) {
	if (bsMem->cond != MEM_FREED) {
		_alloced -= bsMem->size;
		removeFromFreeList(bsMem);
		free(bsMem->data);
		bsMem->cond = MEM_FREED;
	}
}

void MemMan::setCondition(MemHandle *bsMem, uint16 pCond) {
	if ((pCond == MEM_FREED) || (pCond > MEM_DONT_FREE))
		error("MemMan::setCondition: program tried to set illegal memory condition");
	if (bsMem->cond != pCond) {
		bsMem->cond = pCond;
		if (pCond == MEM_DONT_FREE)
			removeFromFreeList(bsMem);
		else if (pCond == MEM_CAN_FREE)
			addToFreeList(bsMem);
	}
}

void MemMan::flush() {
	while (_memListFree) {
		free(_memListFreeEnd->data);
		_memListFreeEnd->data = NULL;
		_memListFreeEnd->cond = MEM_FREED;
		_alloced -= _memListFreeEnd->size;
		removeFromFreeList(_memListFreeEnd);
	}
	if (_alloced)
		warning("MemMan::flush: Something's wrong: still %d bytes alloced", _alloced);
}

void MemMan::checkMemoryUsage() {
	while ((_alloced > MAX_ALLOC) && _memListFree) {
		free(_memListFreeEnd->data);
		_memListFreeEnd->data = NULL;
		_memListFreeEnd->cond = MEM_FREED;
		_alloced -= _memListFreeEnd->size;
		removeFromFreeList(_memListFreeEnd);
	}
}

void MemMan::addToFreeList(MemHandle *bsMem) {
	if (bsMem->next || bsMem->prev) {
		warning("addToFreeList: mem block is already in freeList");
		return;
	}
	bsMem->prev = NULL;
	bsMem->next = _memListFree;
	if (bsMem->next)
		bsMem->next->prev = bsMem;
	_memListFree = bsMem;
	if (!_memListFreeEnd)
		_memListFreeEnd = _memListFree;
}

void MemMan::removeFromFreeList(MemHandle *bsMem) {
	if (_memListFree == bsMem)
		_memListFree = bsMem->next;
	if (_memListFreeEnd == bsMem)
		_memListFreeEnd = bsMem->prev;

	if (bsMem->next)
		bsMem->next->prev = bsMem->prev;
	if (bsMem->prev)
		bsMem->prev->next = bsMem->next;
	bsMem->next = bsMem->prev = NULL;
}

void MemMan::initHandle(MemHandle *bsMem) {
	memset(bsMem, 0, sizeof(MemHandle));
}

} // End of namespace Sword1
