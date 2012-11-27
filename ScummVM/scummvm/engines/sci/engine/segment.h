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

#ifndef SCI_ENGINE_SEGMENT_H
#define SCI_ENGINE_SEGMENT_H

#include "common/serializer.h"

#include "sci/engine/object.h"
#include "sci/engine/vm.h"
#include "sci/engine/vm_types.h"	// for reg_t
#include "sci/util.h"

namespace Sci {

struct SegmentRef {
	bool isRaw;	///< true if data is raw, false if it is a reg_t sequence
	union {
		byte *raw;
		reg_t *reg;
	};
	int maxSize;	///< number of available bytes

	// FIXME: Perhaps a generic 'offset' is more appropriate here
	bool skipByte; ///< true if referencing the 2nd data byte of *reg, false otherwise

	// TODO: Add this?
	//reg_t pointer;	// Original pointer

	// TODO: Add this?
	//SegmentType type;

	SegmentRef() : isRaw(true), raw(0), maxSize(0), skipByte(false) {}

	bool isValid() const { return (isRaw ? raw != 0 : reg != 0); }
};


enum SegmentType {
	SEG_TYPE_INVALID = 0,
	SEG_TYPE_SCRIPT = 1,
	SEG_TYPE_CLONES = 2,
	SEG_TYPE_LOCALS = 3,
	SEG_TYPE_STACK = 4,
	// 5 used to be system strings,	now obsolete
	SEG_TYPE_LISTS = 6,
	SEG_TYPE_NODES = 7,
	SEG_TYPE_HUNK = 8,
	SEG_TYPE_DYNMEM = 9,
	// 10 used to be string fragments, now obsolete

#ifdef ENABLE_SCI32
	SEG_TYPE_ARRAY = 11,
	SEG_TYPE_STRING = 12,
#endif

	SEG_TYPE_MAX // For sanity checking
};

struct SegmentObj : public Common::Serializable {
	SegmentType _type;

public:
	static SegmentObj *createSegmentObj(SegmentType type);

public:
	SegmentObj(SegmentType type) : _type(type) {}
	virtual ~SegmentObj() {}

	inline SegmentType getType() const { return _type; }

	/**
	 * Check whether the given offset into this memory object is valid,
	 * i.e., suitable for passing to dereference.
	 */
	virtual bool isValidOffset(uint16 offset) const = 0;

	/**
	 * Dereferences a raw memory pointer.
	 * @param reg	reference to dereference
	 * @return		the data block referenced
	 */
	virtual SegmentRef dereference(reg_t pointer);

	/**
	 * Finds the canonic address associated with sub_reg.
	 * Used by the garbage collector.
	 *
	 * For each valid address a, there exists a canonic address c(a) such that c(a) = c(c(a)).
	 * This address "governs" a in the sense that deallocating c(a) will deallocate a.
	 *
	 * @param sub_addr		base address whose canonic address is to be found
	 */
	virtual reg_t findCanonicAddress(SegManager *segMan, reg_t sub_addr) const { return sub_addr; }

	/**
	 * Deallocates all memory associated with the specified address.
	 * Used by the garbage collector.
	 * @param sub_addr		address (within the given segment) to deallocate
	 */
	virtual void freeAtAddress(SegManager *segMan, reg_t sub_addr) {}

	/**
	 * Iterates over and reports all addresses within the segment.
	 * Used by the garbage collector.
	 * @return a list of addresses within the segment
	 */
	virtual Common::Array<reg_t> listAllDeallocatable(SegmentId segId) const {
		return Common::Array<reg_t>();
	}

	/**
	 * Iterates over all references reachable from the specified object.
	 * Used by the garbage collector.
	 * @param  object	object (within the current segment) to analyze
	 * @return a list of outgoing references within the object
	 *
	 * @note This function may also choose to report numbers (segment 0) as adresses
	 */
	virtual Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const {
		return Common::Array<reg_t>();
	}
};

struct LocalVariables : public SegmentObj {
	int script_id; /**< Script ID this local variable block belongs to */
	Common::Array<reg_t> _locals;

public:
	LocalVariables(): SegmentObj(SEG_TYPE_LOCALS), script_id(0) { }

	virtual bool isValidOffset(uint16 offset) const {
		return offset < _locals.size() * 2;
	}
	virtual SegmentRef dereference(reg_t pointer);
	virtual reg_t findCanonicAddress(SegManager *segMan, reg_t sub_addr) const;
	virtual Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const;

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};

/** Data stack */
struct DataStack : SegmentObj {
	int _capacity; /**< Number of stack entries */
	reg_t *_entries;

public:
	DataStack() : SegmentObj(SEG_TYPE_STACK), _capacity(0), _entries(NULL) { }
	~DataStack() {
		free(_entries);
		_entries = NULL;
	}

	virtual bool isValidOffset(uint16 offset) const {
		return offset < _capacity * 2;
	}
	virtual SegmentRef dereference(reg_t pointer);
	virtual reg_t findCanonicAddress(SegManager *segMan, reg_t addr) const {
		return make_reg(addr.getSegment(), 0);
	}
	virtual Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const;

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};

enum {
	CLONE_USED = -1,
	CLONE_NONE = -1
};

typedef Object Clone;

struct Node {
	reg_t pred; /**< Predecessor node */
	reg_t succ; /**< Successor node */
	reg_t key;
	reg_t value;
}; /* List nodes */

struct List {
	reg_t first;
	reg_t last;
};

struct Hunk {
	void *mem;
	unsigned int size;
	const char *type;
};

template<typename T>
struct SegmentObjTable : public SegmentObj {
	typedef T value_type;
	struct Entry : public T {
		int next_free; /* Only used for free entries */
	};
	enum { HEAPENTRY_INVALID = -1 };


	int first_free; /**< Beginning of a singly linked list for entries */
	int entries_used; /**< Statistical information */

	Common::Array<Entry> _table;

public:
	SegmentObjTable(SegmentType type) : SegmentObj(type) {
		initTable();
	}

	void initTable() {
		entries_used = 0;
		first_free = HEAPENTRY_INVALID;
		_table.clear();
	}

	int allocEntry() {
		entries_used++;
		if (first_free != HEAPENTRY_INVALID) {
			int oldff = first_free;
			first_free = _table[oldff].next_free;

			_table[oldff].next_free = oldff;
			return oldff;
		} else {
			uint newIdx = _table.size();
			_table.push_back(Entry());
			_table[newIdx].next_free = newIdx;	// Tag as 'valid'
			return newIdx;
		}
	}

	virtual bool isValidOffset(uint16 offset) const {
		return isValidEntry(offset);
	}

	bool isValidEntry(int idx) const {
		return idx >= 0 && (uint)idx < _table.size() && _table[idx].next_free == idx;
	}

	virtual void freeEntry(int idx) {
		if (idx < 0 || (uint)idx >= _table.size())
			::error("Table::freeEntry: Attempt to release invalid table index %d", idx);

		_table[idx].next_free = first_free;
		first_free = idx;
		entries_used--;
	}

	virtual Common::Array<reg_t> listAllDeallocatable(SegmentId segId) const {
		Common::Array<reg_t> tmp;
		for (uint i = 0; i < _table.size(); i++)
			if (isValidEntry(i))
				tmp.push_back(make_reg(segId, i));
		return tmp;
	}
};


/* CloneTable */
struct CloneTable : public SegmentObjTable<Clone> {
	CloneTable() : SegmentObjTable<Clone>(SEG_TYPE_CLONES) {}

	virtual void freeAtAddress(SegManager *segMan, reg_t sub_addr);
	virtual Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const;

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};


/* NodeTable */
struct NodeTable : public SegmentObjTable<Node> {
	NodeTable() : SegmentObjTable<Node>(SEG_TYPE_NODES) {}

	virtual void freeAtAddress(SegManager *segMan, reg_t sub_addr) {
		freeEntry(sub_addr.getOffset());
	}
	virtual Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const;

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};


/* ListTable */
struct ListTable : public SegmentObjTable<List> {
	ListTable() : SegmentObjTable<List>(SEG_TYPE_LISTS) {}

	virtual void freeAtAddress(SegManager *segMan, reg_t sub_addr) {
		freeEntry(sub_addr.getOffset());
	}
	virtual Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const;

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};


/* HunkTable */
struct HunkTable : public SegmentObjTable<Hunk> {
	HunkTable() : SegmentObjTable<Hunk>(SEG_TYPE_HUNK) {}
	virtual ~HunkTable() {
		for (uint i = 0; i < _table.size(); i++) {
			if (isValidEntry(i))
				freeEntryContents(i);
		}
	}

	void freeEntryContents(int idx) {
		free(_table[idx].mem);
		_table[idx].mem = 0;
	}

	virtual void freeEntry(int idx) {
		SegmentObjTable<Hunk>::freeEntry(idx);
		freeEntryContents(idx);
	}

	virtual void freeAtAddress(SegManager *segMan, reg_t sub_addr) {
		freeEntry(sub_addr.getOffset());
	}

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};


// Free-style memory
struct DynMem : public SegmentObj {
	int _size;
	Common::String _description;
	byte *_buf;

public:
	DynMem() : SegmentObj(SEG_TYPE_DYNMEM), _size(0), _buf(0) {}
	~DynMem() {
		free(_buf);
		_buf = NULL;
	}

	virtual bool isValidOffset(uint16 offset) const {
		return offset < _size;
	}
	virtual SegmentRef dereference(reg_t pointer);
	virtual reg_t findCanonicAddress(SegManager *segMan, reg_t addr) const {
		return make_reg(addr.getSegment(), 0);
	}
	virtual Common::Array<reg_t> listAllDeallocatable(SegmentId segId) const {
		const reg_t r = make_reg(segId, 0);
		return Common::Array<reg_t>(&r, 1);
	}

	virtual void saveLoadWithSerializer(Common::Serializer &ser);
};

#ifdef ENABLE_SCI32

template<typename T>
class SciArray {
public:
	SciArray() : _type(-1), _data(NULL), _size(0), _actualSize(0) { }

	SciArray(const SciArray<T> &array) {
		_type = array._type;
		_size = array._size;
		_actualSize = array._actualSize;
		_data = new T[_actualSize];
		assert(_data);
		memcpy(_data, array._data, _size * sizeof(T));
	}

	SciArray<T>& operator=(const SciArray<T> &array) {
		if (this == &array)
			return *this;

		delete[] _data;
		_type = array._type;
		_size = array._size;
		_actualSize = array._actualSize;
		_data = new T[_actualSize];
		assert(_data);
		memcpy(_data, array._data, _size * sizeof(T));

		return *this;
	}

	virtual ~SciArray() {
		destroy();
	}

	virtual void destroy() {
		delete[] _data;
		_data = NULL;
		_type = -1;
		_size = _actualSize = 0;
	}

	void setType(byte type) {
		if (_type >= 0)
			error("SciArray::setType(): Type already set");

		_type = type;
	}

	void setSize(uint32 size) {
		if (_type < 0)
			error("SciArray::setSize(): No type set");

		// Check if we don't have to do anything
		if (_size == size)
			return;

		// Check if we don't have to expand the array
		if (size <= _actualSize) {
			_size = size;
			return;
		}

		// So, we're going to have to create an array of some sort
		T *newArray = new T[size];
		memset(newArray, 0, size * sizeof(T));

		// Check if we never created an array before
		if (!_data) {
			_size = _actualSize = size;
			_data = newArray;
			return;
		}

		// Copy data from the old array to the new
		memcpy(newArray, _data, _size * sizeof(T));

		// Now set the new array to the old and set the sizes
		delete[] _data;
		_data = newArray;
		_size = _actualSize = size;
	}

	T getValue(uint16 index) const {
		if (index >= _size)
			error("SciArray::getValue(): %d is out of bounds (%d)", index, _size);

		return _data[index];
	}

	void setValue(uint16 index, T value) {
		if (index >= _size)
			error("SciArray::setValue(): %d is out of bounds (%d)", index, _size);

		_data[index] = value;
	}

	byte getType() const { return _type; }
	uint32 getSize() const { return _size; }
	T *getRawData() { return _data; }
	const T *getRawData() const { return _data; }

protected:
	int8 _type;
	T *_data;
	uint32 _size; // _size holds the number of entries that the scripts have requested
	uint32 _actualSize; // _actualSize is the actual numbers of entries allocated
};

class SciString : public SciArray<char> {
public:
	SciString() : SciArray<char>() { setType(3); }

	// We overload destroy to ensure the string type is 3 after destroying
	void destroy() { SciArray<char>::destroy(); _type = 3; }

	Common::String toString() const;
	void fromString(const Common::String &string);
};

struct ArrayTable : public SegmentObjTable<SciArray<reg_t> > {
	ArrayTable() : SegmentObjTable<SciArray<reg_t> >(SEG_TYPE_ARRAY) {}

	virtual void freeAtAddress(SegManager *segMan, reg_t sub_addr);
	virtual Common::Array<reg_t> listAllOutgoingReferences(reg_t object) const;

	void saveLoadWithSerializer(Common::Serializer &ser);
	SegmentRef dereference(reg_t pointer);
};

struct StringTable : public SegmentObjTable<SciString> {
	StringTable() : SegmentObjTable<SciString>(SEG_TYPE_STRING) {}

	virtual void freeAtAddress(SegManager *segMan, reg_t sub_addr) {
		_table[sub_addr.getOffset()].destroy();
		freeEntry(sub_addr.getOffset());
	}

	void saveLoadWithSerializer(Common::Serializer &ser);
	SegmentRef dereference(reg_t pointer);
};

#endif


} // End of namespace Sci

#endif // SCI_ENGINE_SEGMENT_H
