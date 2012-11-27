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

#include "common/scummsys.h"

#if defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER)

#include "backends/plugins/elf/memory-manager.h"
#include "common/debug.h"
#include "common/util.h"
#include <malloc.h>

namespace Common {
DECLARE_SINGLETON(ELFMemoryManager);
}

ELFMemoryManager::ELFMemoryManager() :
	_heap(0), _heapSize(0), _heapAlign(0),
	_trackAllocs(false), _measuredSize(0), _measuredAlign(0),
	_bytesAllocated(0) {}

ELFMemoryManager::~ELFMemoryManager() {
	free(_heap);
	_heap = 0;
}

void ELFMemoryManager::trackPlugin(bool value) {
	assert(!_heap);

	if (value == _trackAllocs)
		return;

	_trackAllocs = value;

	if (_trackAllocs) {	// start measuring
		// start tracking allocations
		_measuredAlign = 0;

	} else {	// we're done measuring
		// get the total allocated size
		uint32 measuredSize = _allocList.back().end() - _allocList.front().start;

		_heapSize = MAX(_heapSize, measuredSize);
		_heapAlign = MAX(_heapAlign, _measuredAlign);

		_allocList.clear();
		_bytesAllocated = 0;	// reset

		debug(2, "measured a plugin of size %d. Max size %d. Max align %d", measuredSize, _heapSize, _heapAlign);
	}
}

void ELFMemoryManager::trackAlloc(uint32 align, uint32 size) {
	if (!_measuredAlign)
		_measuredAlign = align;

	// use the allocate function to simulate allocation
	allocateOnHeap(align, size);
}

void ELFMemoryManager::allocateHeap() {
	// The memory manager should only allocate once
	assert (!_heap);
	assert (_heapSize);

	// clear the list
	_allocList.clear();
	_bytesAllocated = 0;

	debug(2, "ELFMemoryManager: allocating %d bytes aligned at %d as the \
			plugin heap", _heapSize, _heapAlign);

	// prepare the heap
	if (_heapAlign)
		_heap = ::memalign(_heapAlign, _heapSize);
	else
		_heap = ::malloc(_heapSize);

	assert(_heap);
}

void *ELFMemoryManager::pluginAllocate(uint32 size) {
	if (_heap) {
		return pluginAllocate(sizeof(void *), size);
	}
	return ::malloc(size);
}

void *ELFMemoryManager::pluginAllocate(uint32 align, uint32 size) {
	if (_heap) {
		return allocateOnHeap(align, size);
	}
	return ::memalign(align, size);
}

void ELFMemoryManager::pluginDeallocate(void *ptr) {
	if (_heap) {
		return deallocateFromHeap(ptr);
	}
	return ::free(ptr);
}

// Allocate space for the request in our heap
void *ELFMemoryManager::allocateOnHeap(uint32 align, uint32 size) {
	byte *lastAddress = (byte *)_heap;

	// We can't allow allocations smaller than sizeof(Allocation). This could
	// only be from non-plugin allocations and would cause infinite recursion
	// when allocating our Allocation in the list.
	if (size <= sizeof(Allocation))
		return 0;

	Common::List<Allocation>::iterator i;
	for (i = _allocList.begin(); i != _allocList.end(); i++) {
		if (i->start - lastAddress > (int)size)
			break;
		lastAddress = i->end();
		// align to desired alignment
		if (align) {
			lastAddress = (byte *)( ((uint32)lastAddress + align - 1) & ~(align - 1) );
		}
	}

	// Check if we exceeded our heap limit
	// We skip this case if we're only tracking allocations
	if (!_trackAllocs && ((uint32)lastAddress + size > (uint32)_heap + _heapSize)) {
		debug(2, "failed to find space to allocate %d bytes", size);
		return 0;
	}

	_allocList.insert(i, Allocation(lastAddress, size));
	_bytesAllocated += size;

	debug(7, "ELFMemoryManager: allocated %d bytes at %p. Total %d bytes",
		size, lastAddress, _bytesAllocated);

	return lastAddress;
}

void ELFMemoryManager::deallocateFromHeap(void *ptr) {
	Common::List<Allocation>::iterator i;
	for (i = _allocList.begin(); i != _allocList.end(); i++) {
		if (i->start == ptr) {
			_bytesAllocated -= (*i).size;

			debug(7, "ELFMemoryManager: freed %d bytes at %p. Total %d bytes",
				(*i).size, (*i).start, _bytesAllocated);

			_allocList.erase(i);
			break;
		}
	}
}

#endif /* defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER) */
