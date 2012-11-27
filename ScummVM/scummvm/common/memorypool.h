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

#ifndef COMMON_MEMORYPOOL_H
#define COMMON_MEMORYPOOL_H

#include "common/scummsys.h"
#include "common/array.h"


namespace Common {

/**
 * This class provides a pool of memory 'chunks' of identical size.
 * The size of a chunk is determined when creating the memory pool.
 *
 * Using a memory pool may yield better performance and memory usage
 * when allocating and deallocating many memory blocks of equal size.
 * E.g. the Common::String class uses a memory pool for the refCount
 * variables (each the size of an int) it allocates for each string
 * instance.
 */
class MemoryPool {
protected:
	MemoryPool(const MemoryPool&);
	MemoryPool& operator=(const MemoryPool&);

	struct Page {
		void *start;
		size_t numChunks;
	};

	const size_t	_chunkSize;
	Array<Page>		_pages;
	void			*_next;
	size_t			_chunksPerPage;

	void	allocPage();
	void	addPageToPool(const Page &page);
	bool	isPointerInPage(void *ptr, const Page &page);

public:
	/**
	 * Constructor for a memory pool with the given chunk size.
	 * @param chunkSize		the chunk size of this memory pool
	 */
	explicit MemoryPool(size_t chunkSize);
	~MemoryPool();

	/**
	 * Allocate a new chunk from the memory pool.
	 */
	void	*allocChunk();
	/**
	 * Return a chunk to the memory pool. The given pointer must have
	 * been obtained from calling the allocChunk() method of the very
	 * same MemoryPool instance. Passing any other pointer (e.g. to
	 * a chunk from another MemoryPool, or a malloc'ed memory block)
	 * will lead to undefined behavior and may result in a crash (if
	 * you are lucky) or in silent data corruption.
	 */
	void	freeChunk(void *ptr);

	/**
	 * Perform garbage collection. The memory pool stores all the
	 * chunks it manages in memory 'pages' obtained via the classic
	 * memory allocation APIs (i.e. malloc/free). Ordinarily, once
	 * a page has been allocated, it won't be released again during
	 * the life time of the memory pool. The exception is when this
	 * method is called.
	 */
	void	freeUnusedPages();

	/**
	 * Return the chunk size used by this memory pool.
	 */
	size_t	getChunkSize() const { return _chunkSize; }
};

/**
 * This is a memory pool which already contains in itself some storage
 * space for a fixed number of chunks. Thus if the memory pool is only
 * lightly used, no malloc() calls have to be made at all.
 */
template<size_t CHUNK_SIZE, size_t NUM_INTERNAL_CHUNKS = 32>
class FixedSizeMemoryPool : public MemoryPool {
private:
	enum {
		REAL_CHUNK_SIZE = (CHUNK_SIZE + sizeof(void *) - 1) & (~(sizeof(void *) - 1))
	};

	byte	_storage[NUM_INTERNAL_CHUNKS * REAL_CHUNK_SIZE];
public:
	FixedSizeMemoryPool() : MemoryPool(CHUNK_SIZE) {
		assert(REAL_CHUNK_SIZE == _chunkSize);
		// Insert some static storage
		Page internalPage = { _storage, NUM_INTERNAL_CHUNKS };
		addPageToPool(internalPage);
	}
};

// Ensure NUM_INTERNAL_CHUNKS == 0 results in a compile error
template<size_t CHUNK_SIZE>
class FixedSizeMemoryPool<CHUNK_SIZE,0> : public MemoryPool {
public:
	FixedSizeMemoryPool() : MemoryPool(CHUNK_SIZE) {}
};

/**
 * A memory pool for C++ objects.
 */
template<class T, size_t NUM_INTERNAL_CHUNKS = 32>
class ObjectPool : public FixedSizeMemoryPool<sizeof(T), NUM_INTERNAL_CHUNKS> {
public:
	/**
	 * Return the memory chunk used as storage for the given object back
	 * to the pool, after calling its destructor.
	 */
	void deleteChunk(T *ptr) {
		ptr->~T();
		this->freeChunk(ptr);
	}
};

}	// End of namespace Common

/**
 * A custom placement new operator, using an arbitrary MemoryPool.
 *
 * This *should* work with all C++ implementations, but may not.
 *
 * For details on using placement new for custom allocators, see e.g.
 * <http://www.parashift.com/c++-faq-lite/dtors.html#faq-11.14>
 */
inline void *operator new(size_t nbytes, Common::MemoryPool &pool) {
	assert(nbytes <= pool.getChunkSize());
	return pool.allocChunk();
}

inline void operator delete(void *p, Common::MemoryPool &pool) {
	pool.freeChunk(p);
}

#endif
