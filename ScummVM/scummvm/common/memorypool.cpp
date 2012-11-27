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

#include "common/memorypool.h"
#include "common/util.h"

namespace Common {

enum {
	INITIAL_CHUNKS_PER_PAGE = 8
};

static size_t adjustChunkSize(size_t chunkSize) {
	// You must at least fit the pointer in the node (technically unneeded considering the next rounding statement)
	chunkSize = MAX(chunkSize, sizeof(void *));
	// There might be an alignment problem on some platforms when trying to load a void* on a non natural boundary
	// so we round to the next sizeof(void *)
	chunkSize = (chunkSize + sizeof(void *) - 1) & (~(sizeof(void *) - 1));

	return chunkSize;
}


MemoryPool::MemoryPool(size_t chunkSize)
	: _chunkSize(adjustChunkSize(chunkSize)) {

	_next = NULL;

	_chunksPerPage = INITIAL_CHUNKS_PER_PAGE;
}

MemoryPool::~MemoryPool() {
#if 0
	freeUnusedPages();
	if (!_pages.empty())
		warning("Memory leak found in pool");
#endif

	for (size_t i = 0; i < _pages.size(); ++i)
		::free(_pages[i].start);
}

void MemoryPool::allocPage() {
	Page page;

	// Allocate a new page
	page.numChunks = _chunksPerPage;
	assert(page.numChunks * _chunkSize < 16*1024*1024);	// Refuse to allocate pages bigger than 16 MB

	page.start = ::malloc(page.numChunks * _chunkSize);
	assert(page.start);
	_pages.push_back(page);


	// Next time, we'll allocate a page twice as big as this one.
	_chunksPerPage *= 2;

	// Add the page to the pool of free chunk
	addPageToPool(page);
}

void MemoryPool::addPageToPool(const Page &page) {
	// Add all chunks of the new page to the linked list (pool) of free chunks
	void *current = page.start;
	for (size_t i = 1; i < page.numChunks; ++i) {
		void *next = (byte *)current + _chunkSize;
		*(void **)current = next;

		current = next;
	}

	// Last chunk points to the old _next
	*(void **)current = _next;

	// From now on, the first free chunk is the first chunk of the new page
	_next = page.start;
}

void *MemoryPool::allocChunk() {
	// No free chunks left? Allocate a new page
	if (!_next)
		allocPage();

	assert(_next);
	void *result = _next;
	_next = *(void **)result;
	return result;
}

void MemoryPool::freeChunk(void *ptr) {
	// Add the chunk back to (the start of) the list of free chunks
	*(void **)ptr = _next;
	_next = ptr;
}

// Technically not compliant C++ to compare unrelated pointers. In practice...
bool MemoryPool::isPointerInPage(void *ptr, const Page &page) {
	return (ptr >= page.start) && (ptr < (char *)page.start + page.numChunks * _chunkSize);
}

void MemoryPool::freeUnusedPages() {
	//std::sort(_pages.begin(), _pages.end());
	Array<size_t> numberOfFreeChunksPerPage;
	numberOfFreeChunksPerPage.resize(_pages.size());
	for (size_t i = 0; i < numberOfFreeChunksPerPage.size(); ++i) {
		numberOfFreeChunksPerPage[i] = 0;
	}

	// Compute for each page how many chunks in it are still in use.
	void *iterator = _next;
	while (iterator) {
		// TODO: This should be a binary search (requiring us to keep _pages sorted)
		for (size_t i = 0; i < _pages.size(); ++i) {
			if (isPointerInPage(iterator, _pages[i])) {
				++numberOfFreeChunksPerPage[i];
				break;
			}
		}

		iterator = *(void **)iterator;
	}

	// Free all pages which are not in use.
	size_t freedPagesCount = 0;
	for (size_t i = 0; i < _pages.size(); ++i)  {
		if (numberOfFreeChunksPerPage[i] == _pages[i].numChunks) {
			// Remove all chunks of this page from the list of free chunks
			void **iter2 = &_next;
			while (*iter2) {
				if (isPointerInPage(*iter2, _pages[i]))
					*iter2 = **(void ***)iter2;
				else
					iter2 = *(void ***)iter2;
			}

			::free(_pages[i].start);
			++freedPagesCount;
			_pages[i].start = NULL;
		}
	}

//	debug("freed %d pages out of %d", (int)freedPagesCount, (int)_pages.size());

	// Remove all now unused pages
	size_t newSize = 0;
	for (size_t i = 0; i < _pages.size(); ++i) {
		if (_pages[i].start != NULL) {
			if (newSize != i)
				_pages[newSize] = _pages[i];
			++newSize;
		}
	}
	_pages.resize(newSize);

	// Reset _chunksPerPage
	_chunksPerPage = INITIAL_CHUNKS_PER_PAGE;
	for (size_t i = 0; i < _pages.size(); ++i) {
		if (_chunksPerPage < _pages[i].numChunks)
			_chunksPerPage = _pages[i].numChunks;
	}
}

} // End of namespace Common
