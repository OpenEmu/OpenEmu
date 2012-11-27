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

// The hash map (associative array) implementation in this file is
// based on the PyDict implementation of CPython.

#ifndef COMMON_HASHMAP_H
#define COMMON_HASHMAP_H

/**
 * @def DEBUG_HASH_COLLISIONS
 * Enable the following #define if you want to check how many collisions the
 * code produces (many collisions indicate either a bad hash function, or a
 * hash table that is too small).
 */
//#define DEBUG_HASH_COLLISIONS

/**
 * @def USE_HASHMAP_MEMORY_POOL
 * Enable the following define to let HashMaps use a memory pool for the
 nodes they contain. * This increases memory usage, but also can improve
 speed quite a bit.
 */
#define USE_HASHMAP_MEMORY_POOL


#include "common/func.h"

#ifdef DEBUG_HASH_COLLISIONS
#include "common/debug.h"
#endif

#ifdef USE_HASHMAP_MEMORY_POOL
#include "common/memorypool.h"
#endif



namespace Common {

// The sgi IRIX MIPSpro Compiler has difficulties with nested templates.
// This and the other __sgi conditionals below work around these problems.
// The Intel C++ Compiler suffers from the same problems.
#if (defined(__sgi) && !defined(__GNUC__)) || defined(__INTEL_COMPILER)
template<class T> class IteratorImpl;
#endif


/**
 * HashMap<Key,Val> maps objects of type Key to objects of type Val.
 * For each used Key type, we need an "size_type hashit(Key,size_type)" function
 * that computes a hash for the given Key object and returns it as an
 * an integer from 0 to hashsize-1, and also an "equality functor".
 * that returns true if if its two arguments are to be considered
 * equal. Also, we assume that "=" works on Val objects for assignment.
 *
 * If aa is an HashMap<Key,Val>, then space is allocated each time aa[key] is
 * referenced, for a new key. If the object is const, then an assertion is
 * triggered instead. Hence if you are not sure whether a key is contained in
 * the map, use contains() first to check for its presence.
 */
template<class Key, class Val, class HashFunc = Hash<Key>, class EqualFunc = EqualTo<Key> >
class HashMap {
public:
	typedef uint size_type;

private:

	typedef HashMap<Key, Val, HashFunc, EqualFunc> HM_t;

	struct Node {
		const Key _key;
		Val _value;
		explicit Node(const Key &key) : _key(key), _value() {}
		Node() : _key(), _value() {}
	};

	enum {
		HASHMAP_PERTURB_SHIFT = 5,
		HASHMAP_MIN_CAPACITY = 16,

		// The quotient of the next two constants controls how much the
		// internal storage of the hashmap may fill up before being
		// increased automatically.
		// Note: the quotient of these two must be between and different
		// from 0 and 1.
		HASHMAP_LOADFACTOR_NUMERATOR = 2,
		HASHMAP_LOADFACTOR_DENOMINATOR = 3,

		HASHMAP_MEMORYPOOL_SIZE = HASHMAP_MIN_CAPACITY * HASHMAP_LOADFACTOR_NUMERATOR / HASHMAP_LOADFACTOR_DENOMINATOR
	};

#ifdef USE_HASHMAP_MEMORY_POOL
	ObjectPool<Node, HASHMAP_MEMORYPOOL_SIZE> _nodePool;
#endif

	Node **_storage;	///< hashtable of size arrsize.
	size_type _mask;		///< Capacity of the HashMap minus one; must be a power of two of minus one
	size_type _size;
	size_type _deleted; ///< Number of deleted elements (_dummyNodes)

	HashFunc _hash;
	EqualFunc _equal;

	/** Default value, returned by the const getVal. */
	const Val _defaultVal;

	/** Dummy node, used as marker for erased objects. */
	#define HASHMAP_DUMMY_NODE	((Node *)1)

#ifdef DEBUG_HASH_COLLISIONS
	mutable int _collisions, _lookups, _dummyHits;
#endif

	Node *allocNode(const Key &key) {
#ifdef USE_HASHMAP_MEMORY_POOL
		return new (_nodePool) Node(key);
#else
		return new Node(key);
#endif
	}

	void freeNode(Node *node) {
		if (node && node != HASHMAP_DUMMY_NODE)
#ifdef USE_HASHMAP_MEMORY_POOL
			_nodePool.deleteChunk(node);
#else
			delete node;
#endif
	}

	void assign(const HM_t &map);
	size_type lookup(const Key &key) const;
	size_type lookupAndCreateIfMissing(const Key &key);
	void expandStorage(size_type newCapacity);

#if !defined(__sgi) || defined(__GNUC__)
	template<class T> friend class IteratorImpl;
#endif

	/**
	 * Simple HashMap iterator implementation.
	 */
	template<class NodeType>
	class IteratorImpl {
		friend class HashMap;
#if (defined(__sgi) && !defined(__GNUC__)) || defined(__INTEL_COMPILER)
		template<class T> friend class Common::IteratorImpl;
#else
		template<class T> friend class IteratorImpl;
#endif
	protected:
		typedef const HashMap hashmap_t;

		size_type _idx;
		hashmap_t *_hashmap;

	protected:
		IteratorImpl(size_type idx, hashmap_t *hashmap) : _idx(idx), _hashmap(hashmap) {}

		NodeType *deref() const {
			assert(_hashmap != 0);
			assert(_idx <= _hashmap->_mask);
			Node *node = _hashmap->_storage[_idx];
			assert(node != 0);
			assert(node != HASHMAP_DUMMY_NODE);
			return node;
		}

	public:
		IteratorImpl() : _idx(0), _hashmap(0) {}
		template<class T>
		IteratorImpl(const IteratorImpl<T> &c) : _idx(c._idx), _hashmap(c._hashmap) {}

		NodeType &operator*() const { return *deref(); }
		NodeType *operator->() const { return deref(); }

		bool operator==(const IteratorImpl &iter) const { return _idx == iter._idx && _hashmap == iter._hashmap; }
		bool operator!=(const IteratorImpl &iter) const { return !(*this == iter); }

		IteratorImpl &operator++() {
			assert(_hashmap);
			do {
				_idx++;
			} while (_idx <= _hashmap->_mask && (_hashmap->_storage[_idx] == 0 || _hashmap->_storage[_idx] == HASHMAP_DUMMY_NODE));
			if (_idx > _hashmap->_mask)
				_idx = (size_type)-1;

			return *this;
		}

		IteratorImpl operator++(int) {
			IteratorImpl old = *this;
			operator ++();
			return old;
		}
	};

public:
	typedef IteratorImpl<Node> iterator;
	typedef IteratorImpl<const Node> const_iterator;

	HashMap();
	HashMap(const HM_t &map);
	~HashMap();

	HM_t &operator=(const HM_t &map) {
		if (this == &map)
			return *this;

		// Remove the previous content and ...
		clear();
		delete[] _storage;
		// ... copy the new stuff.
		assign(map);
		return *this;
	}

	bool contains(const Key &key) const;

	Val &operator[](const Key &key);
	const Val &operator[](const Key &key) const;

	Val &getVal(const Key &key);
	const Val &getVal(const Key &key) const;
	const Val &getVal(const Key &key, const Val &defaultVal) const;
	void setVal(const Key &key, const Val &val);

	void clear(bool shrinkArray = 0);

	void erase(iterator entry);
	void erase(const Key &key);

	size_type size() const { return _size; }

	iterator	begin() {
		// Find and return the first non-empty entry
		for (size_type ctr = 0; ctr <= _mask; ++ctr) {
			if (_storage[ctr] && _storage[ctr] != HASHMAP_DUMMY_NODE)
				return iterator(ctr, this);
		}
		return end();
	}
	iterator	end() {
		return iterator((size_type)-1, this);
	}

	const_iterator	begin() const {
		// Find and return the first non-empty entry
		for (size_type ctr = 0; ctr <= _mask; ++ctr) {
			if (_storage[ctr] && _storage[ctr] != HASHMAP_DUMMY_NODE)
				return const_iterator(ctr, this);
		}
		return end();
	}
	const_iterator	end() const {
		return const_iterator((size_type)-1, this);
	}

	iterator	find(const Key &key) {
		size_type ctr = lookup(key);
		if (_storage[ctr])
			return iterator(ctr, this);
		return end();
	}

	const_iterator	find(const Key &key) const {
		size_type ctr = lookup(key);
		if (_storage[ctr])
			return const_iterator(ctr, this);
		return end();
	}

	// TODO: insert() method?

	bool empty() const {
		return (_size == 0);
	}
};

//-------------------------------------------------------
// HashMap functions

/**
 * Base constructor, creates an empty hashmap.
 */
template<class Key, class Val, class HashFunc, class EqualFunc>
HashMap<Key, Val, HashFunc, EqualFunc>::HashMap()
//
// We have to skip _defaultVal() on PS2 to avoid gcc 3.2.2 ICE
//
#ifdef __PLAYSTATION2__
	{
#else
	: _defaultVal() {
#endif
	_mask = HASHMAP_MIN_CAPACITY - 1;
	_storage = new Node *[HASHMAP_MIN_CAPACITY];
	assert(_storage != NULL);
	memset(_storage, 0, HASHMAP_MIN_CAPACITY * sizeof(Node *));

	_size = 0;
	_deleted = 0;

#ifdef DEBUG_HASH_COLLISIONS
	_collisions = 0;
	_lookups = 0;
	_dummyHits = 0;
#endif
}

/**
 * Copy constructor, creates a full copy of the given hashmap.
 * We must provide a custom copy constructor as we use pointers
 * to heap buffers for the internal storage.
 */
template<class Key, class Val, class HashFunc, class EqualFunc>
HashMap<Key, Val, HashFunc, EqualFunc>::HashMap(const HM_t &map) :
	_defaultVal() {
#ifdef DEBUG_HASH_COLLISIONS
	_collisions = 0;
	_lookups = 0;
	_dummyHits = 0;
#endif
	assign(map);
}

/**
 * Destructor, frees all used memory.
 */
template<class Key, class Val, class HashFunc, class EqualFunc>
HashMap<Key, Val, HashFunc, EqualFunc>::~HashMap() {
	for (size_type ctr = 0; ctr <= _mask; ++ctr)
	  freeNode(_storage[ctr]);

	delete[] _storage;
#ifdef DEBUG_HASH_COLLISIONS
	extern void updateHashCollisionStats(int, int, int, int, int);
	updateHashCollisionStats(_collisions, _dummyHits, _lookups, _mask+1, _size);
#endif
}

/**
 * Internal method for assigning the content of another HashMap
 * to this one.
 *
 * @note We do *not* deallocate the previous storage here -- the caller is
 *       responsible for doing that!
 */
template<class Key, class Val, class HashFunc, class EqualFunc>
void HashMap<Key, Val, HashFunc, EqualFunc>::assign(const HM_t &map) {
	_mask = map._mask;
	_storage = new Node *[_mask+1];
	assert(_storage != NULL);
	memset(_storage, 0, (_mask+1) * sizeof(Node *));

	// Simply clone the map given to us, one by one.
	_size = 0;
	_deleted = 0;
	for (size_type ctr = 0; ctr <= _mask; ++ctr) {
		if (map._storage[ctr] == HASHMAP_DUMMY_NODE) {
			_storage[ctr] = HASHMAP_DUMMY_NODE;
			_deleted++;
		} else if (map._storage[ctr] != NULL) {
			_storage[ctr] = allocNode(map._storage[ctr]->_key);
			_storage[ctr]->_value = map._storage[ctr]->_value;
			_size++;
		}
	}
	// Perform a sanity check (to help track down hashmap corruption)
	assert(_size == map._size);
	assert(_deleted == map._deleted);
}


template<class Key, class Val, class HashFunc, class EqualFunc>
void HashMap<Key, Val, HashFunc, EqualFunc>::clear(bool shrinkArray) {
	for (size_type ctr = 0; ctr <= _mask; ++ctr) {
		freeNode(_storage[ctr]);
		_storage[ctr] = NULL;
	}

#ifdef USE_HASHMAP_MEMORY_POOL
	_nodePool.freeUnusedPages();
#endif

	if (shrinkArray && _mask >= HASHMAP_MIN_CAPACITY) {
		delete[] _storage;

		_mask = HASHMAP_MIN_CAPACITY;
		_storage = new Node *[HASHMAP_MIN_CAPACITY];
		assert(_storage != NULL);
		memset(_storage, 0, HASHMAP_MIN_CAPACITY * sizeof(Node *));
	}

	_size = 0;
	_deleted = 0;
}

template<class Key, class Val, class HashFunc, class EqualFunc>
void HashMap<Key, Val, HashFunc, EqualFunc>::expandStorage(size_type newCapacity) {
	assert(newCapacity > _mask+1);

#ifndef NDEBUG
	const size_type old_size = _size;
#endif
	const size_type old_mask = _mask;
	Node **old_storage = _storage;

	// allocate a new array
	_size = 0;
	_deleted = 0;
	_mask = newCapacity - 1;
	_storage = new Node *[newCapacity];
	assert(_storage != NULL);
	memset(_storage, 0, newCapacity * sizeof(Node *));

	// rehash all the old elements
	for (size_type ctr = 0; ctr <= old_mask; ++ctr) {
		if (old_storage[ctr] == NULL || old_storage[ctr] == HASHMAP_DUMMY_NODE)
			continue;

		// Insert the element from the old table into the new table.
		// Since we know that no key exists twice in the old table, we
		// can do this slightly better than by calling lookup, since we
		// don't have to call _equal().
		const size_type hash = _hash(old_storage[ctr]->_key);
		size_type idx = hash & _mask;
		for (size_type perturb = hash; _storage[idx] != NULL && _storage[idx] != HASHMAP_DUMMY_NODE; perturb >>= HASHMAP_PERTURB_SHIFT) {
			idx = (5 * idx + perturb + 1) & _mask;
		}

		_storage[idx] = old_storage[ctr];
		_size++;
	}

	// Perform a sanity check: Old number of elements should match the new one!
	// This check will fail if some previous operation corrupted this hashmap.
	assert(_size == old_size);

	delete[] old_storage;

	return;
}

template<class Key, class Val, class HashFunc, class EqualFunc>
typename HashMap<Key, Val, HashFunc, EqualFunc>::size_type HashMap<Key, Val, HashFunc, EqualFunc>::lookup(const Key &key) const {
	const size_type hash = _hash(key);
	size_type ctr = hash & _mask;
	for (size_type perturb = hash; ; perturb >>= HASHMAP_PERTURB_SHIFT) {
		if (_storage[ctr] == NULL)
			break;
		if (_storage[ctr] == HASHMAP_DUMMY_NODE) {
#ifdef DEBUG_HASH_COLLISIONS
			_dummyHits++;
#endif
		} else if (_equal(_storage[ctr]->_key, key))
			break;

		ctr = (5 * ctr + perturb + 1) & _mask;

#ifdef DEBUG_HASH_COLLISIONS
		_collisions++;
#endif
	}

#ifdef DEBUG_HASH_COLLISIONS
	_lookups++;
	debug("collisions %d, dummies hit %d, lookups %d, ratio %f in HashMap %p; size %d num elements %d",
		_collisions, _dummyHits, _lookups, ((double) _collisions / (double)_lookups),
		(const void *)this, _mask+1, _size);
#endif

	return ctr;
}

template<class Key, class Val, class HashFunc, class EqualFunc>
typename HashMap<Key, Val, HashFunc, EqualFunc>::size_type HashMap<Key, Val, HashFunc, EqualFunc>::lookupAndCreateIfMissing(const Key &key) {
	const size_type hash = _hash(key);
	size_type ctr = hash & _mask;
	const size_type NONE_FOUND = _mask + 1;
	size_type first_free = NONE_FOUND;
	bool found = false;
	for (size_type perturb = hash; ; perturb >>= HASHMAP_PERTURB_SHIFT) {
		if (_storage[ctr] == NULL)
			break;
		if (_storage[ctr] == HASHMAP_DUMMY_NODE) {
#ifdef DEBUG_HASH_COLLISIONS
			_dummyHits++;
#endif
			if (first_free != _mask + 1)
				first_free = ctr;
		} else if (_equal(_storage[ctr]->_key, key)) {
			found = true;
			break;
		}

		ctr = (5 * ctr + perturb + 1) & _mask;

#ifdef DEBUG_HASH_COLLISIONS
		_collisions++;
#endif
	}

#ifdef DEBUG_HASH_COLLISIONS
	_lookups++;
	debug("collisions %d, dummies hit %d, lookups %d, ratio %f in HashMap %p; size %d num elements %d",
		_collisions, _dummyHits, _lookups, ((double) _collisions / (double)_lookups),
		(const void *)this, _mask+1, _size);
#endif

	if (!found && first_free != _mask + 1)
		ctr = first_free;

	if (!found) {
		if (_storage[ctr])
			_deleted--;
		_storage[ctr] = allocNode(key);
		assert(_storage[ctr] != NULL);
		_size++;

		// Keep the load factor below a certain threshold.
		// Deleted nodes are also counted
		size_type capacity = _mask + 1;
		if ((_size + _deleted) * HASHMAP_LOADFACTOR_DENOMINATOR >
		        capacity * HASHMAP_LOADFACTOR_NUMERATOR) {
			capacity = capacity < 500 ? (capacity * 4) : (capacity * 2);
			expandStorage(capacity);
			ctr = lookup(key);
			assert(_storage[ctr] != NULL);
		}
	}

	return ctr;
}


template<class Key, class Val, class HashFunc, class EqualFunc>
bool HashMap<Key, Val, HashFunc, EqualFunc>::contains(const Key &key) const {
	size_type ctr = lookup(key);
	return (_storage[ctr] != NULL);
}

template<class Key, class Val, class HashFunc, class EqualFunc>
Val &HashMap<Key, Val, HashFunc, EqualFunc>::operator[](const Key &key) {
	return getVal(key);
}

template<class Key, class Val, class HashFunc, class EqualFunc>
const Val &HashMap<Key, Val, HashFunc, EqualFunc>::operator[](const Key &key) const {
	return getVal(key);
}

template<class Key, class Val, class HashFunc, class EqualFunc>
Val &HashMap<Key, Val, HashFunc, EqualFunc>::getVal(const Key &key) {
	size_type ctr = lookupAndCreateIfMissing(key);
	assert(_storage[ctr] != NULL);
	return _storage[ctr]->_value;
}

template<class Key, class Val, class HashFunc, class EqualFunc>
const Val &HashMap<Key, Val, HashFunc, EqualFunc>::getVal(const Key &key) const {
	return getVal(key, _defaultVal);
}

template<class Key, class Val, class HashFunc, class EqualFunc>
const Val &HashMap<Key, Val, HashFunc, EqualFunc>::getVal(const Key &key, const Val &defaultVal) const {
	size_type ctr = lookup(key);
	if (_storage[ctr] != NULL)
		return _storage[ctr]->_value;
	else
		return defaultVal;
}

template<class Key, class Val, class HashFunc, class EqualFunc>
void HashMap<Key, Val, HashFunc, EqualFunc>::setVal(const Key &key, const Val &val) {
	size_type ctr = lookupAndCreateIfMissing(key);
	assert(_storage[ctr] != NULL);
	_storage[ctr]->_value = val;
}

template<class Key, class Val, class HashFunc, class EqualFunc>
void HashMap<Key, Val, HashFunc, EqualFunc>::erase(iterator entry) {
	// Check whether we have a valid iterator
	assert(entry._hashmap == this);
	const size_type ctr = entry._idx;
	assert(ctr <= _mask);
	Node * const node = _storage[ctr];
	assert(node != NULL);
	assert(node != HASHMAP_DUMMY_NODE);

	// If we remove a key, we replace it with a dummy node.
	freeNode(node);
	_storage[ctr] = HASHMAP_DUMMY_NODE;
	_size--;
	_deleted++;
}

template<class Key, class Val, class HashFunc, class EqualFunc>
void HashMap<Key, Val, HashFunc, EqualFunc>::erase(const Key &key) {

	size_type ctr = lookup(key);
	if (_storage[ctr] == NULL)
		return;

	// If we remove a key, we replace it with a dummy node.
	freeNode(_storage[ctr]);
	_storage[ctr] = HASHMAP_DUMMY_NODE;
	_size--;
	_deleted++;
	return;
}

#undef HASHMAP_DUMMY_NODE

}	// End of namespace Common

#endif
