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

#include "common/endian.h"

#include "sci/sci.h"
#include "sci/engine/kernel.h"
#include "sci/engine/features.h"
#include "sci/engine/object.h"
#include "sci/engine/script.h"	// for SCI_OBJ_EXPORTS and SCI_OBJ_SYNONYMS
#include "sci/engine/segment.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/state.h"

namespace Sci {

//#define GC_DEBUG // Debug garbage collection
//#define GC_DEBUG_VERBOSE // Debug garbage verbosely

SegmentObj *SegmentObj::createSegmentObj(SegmentType type) {
	SegmentObj *mem = 0;
	switch (type) {
	case SEG_TYPE_SCRIPT:
		mem = new Script();
		break;
	case SEG_TYPE_CLONES:
		mem = new CloneTable();
		break;
	case SEG_TYPE_LOCALS:
		mem = new LocalVariables();
		break;
	case SEG_TYPE_STACK:
		mem = new DataStack();
		break;
	case SEG_TYPE_HUNK:
		mem = new HunkTable();
		break;
	case SEG_TYPE_LISTS:
		mem = new ListTable();
		break;
	case SEG_TYPE_NODES:
		mem = new NodeTable();
		break;
	case SEG_TYPE_DYNMEM:
		mem = new DynMem();
		break;
#ifdef ENABLE_SCI32
	case SEG_TYPE_ARRAY:
		mem = new ArrayTable();
		break;
	case SEG_TYPE_STRING:
		mem = new StringTable();
		break;
#endif
	default:
		error("Unknown SegmentObj type %d", type);
		break;
	}

	assert(mem);
	assert(mem->_type == type);
	return mem;
}

SegmentRef SegmentObj::dereference(reg_t pointer) {
	error("Error: Trying to dereference pointer %04x:%04x to inappropriate segment",
		          PRINT_REG(pointer));
	return SegmentRef();
}

//-------------------- clones --------------------

Common::Array<reg_t> CloneTable::listAllOutgoingReferences(reg_t addr) const {
	Common::Array<reg_t> tmp;
//	assert(addr.segment == _segId);

	if (!isValidEntry(addr.getOffset())) {
		error("Unexpected request for outgoing references from clone at %04x:%04x", PRINT_REG(addr));
	}

	const Clone *clone = &(_table[addr.getOffset()]);

	// Emit all member variables (including references to the 'super' delegate)
	for (uint i = 0; i < clone->getVarCount(); i++)
		tmp.push_back(clone->getVariable(i));

	// Note that this also includes the 'base' object, which is part of the script and therefore also emits the locals.
	tmp.push_back(clone->getPos());
	//debugC(kDebugLevelGC, "[GC] Reporting clone-pos %04x:%04x", PRINT_REG(clone->pos));

	return tmp;
}

void CloneTable::freeAtAddress(SegManager *segMan, reg_t addr) {
#ifdef GC_DEBUG
	Object *victim_obj = &(_table[addr.getOffset()]);

	if (!(victim_obj->_flags & OBJECT_FLAG_FREED))
		warning("[GC] Clone %04x:%04x not reachable and not freed (freeing now)", PRINT_REG(addr));
#ifdef GC_DEBUG_VERBOSE
	else
		warning("[GC-DEBUG] Clone %04x:%04x: Freeing", PRINT_REG(addr));

	warning("[GC] Clone had pos %04x:%04x", PRINT_REG(victim_obj->pos));
#endif
#endif

	freeEntry(addr.getOffset());
}


//-------------------- locals --------------------

SegmentRef LocalVariables::dereference(reg_t pointer) {
	SegmentRef ret;
	ret.isRaw = false;	// reg_t based data!
	ret.maxSize = (_locals.size() - pointer.getOffset() / 2) * 2;

	if (pointer.getOffset() & 1) {
		ret.maxSize -= 1;
		ret.skipByte = true;
	}

	if (ret.maxSize > 0) {
		ret.reg = &_locals[pointer.getOffset() / 2];
	} else {
		if ((g_sci->getEngineState()->currentRoomNumber() == 160 ||
			 g_sci->getEngineState()->currentRoomNumber() == 220)
			&& g_sci->getGameId() == GID_LAURABOW2) {
			// WORKAROUND: Happens in two places during the intro of LB2CD, both
			// from kMemory(peek):
			// - room 160: Heap 160 has 83 local variables (0-82), and the game
			//   asks for variables at indices 83 - 90 too.
			// - room 220: Heap 220 has 114 local variables (0-113), and the
			//   game asks for variables at indices 114-120 too.
		} else {
			error("LocalVariables::dereference: Offset at end or out of bounds %04x:%04x", PRINT_REG(pointer));
		}
		ret.reg = 0;
	}
	return ret;
}

reg_t LocalVariables::findCanonicAddress(SegManager *segMan, reg_t addr) const {
	// Reference the owning script
	SegmentId owner_seg = segMan->getScriptSegment(script_id);
	assert(owner_seg > 0);
	return make_reg(owner_seg, 0);
}

Common::Array<reg_t> LocalVariables::listAllOutgoingReferences(reg_t addr) const {
	Common::Array<reg_t> tmp;
	for (uint i = 0; i < _locals.size(); i++)
		tmp.push_back(_locals[i]);

	return tmp;
}


//-------------------- stack --------------------

SegmentRef DataStack::dereference(reg_t pointer) {
	SegmentRef ret;
	ret.isRaw = false;	// reg_t based data!
	ret.maxSize = (_capacity - pointer.getOffset() / 2) * 2;

	if (pointer.getOffset() & 1) {
		ret.maxSize -= 1;
		ret.skipByte = true;
	}

	ret.reg = &_entries[pointer.getOffset() / 2];
	return ret;
}

Common::Array<reg_t> DataStack::listAllOutgoingReferences(reg_t object) const {
	Common::Array<reg_t> tmp;
	for (int i = 0; i < _capacity; i++)
		tmp.push_back(_entries[i]);

	return tmp;
}

//-------------------- lists --------------------

Common::Array<reg_t> ListTable::listAllOutgoingReferences(reg_t addr) const {
	Common::Array<reg_t> tmp;
	if (!isValidEntry(addr.getOffset())) {
		error("Invalid list referenced for outgoing references: %04x:%04x", PRINT_REG(addr));
	}

	const List *list = &(_table[addr.getOffset()]);

	tmp.push_back(list->first);
	tmp.push_back(list->last);
	// We could probably get away with just one of them, but
	// let's be conservative here.

	return tmp;
}

//-------------------- nodes --------------------

Common::Array<reg_t> NodeTable::listAllOutgoingReferences(reg_t addr) const {
	Common::Array<reg_t> tmp;
	if (!isValidEntry(addr.getOffset())) {
		error("Invalid node referenced for outgoing references: %04x:%04x", PRINT_REG(addr));
	}
	const Node *node = &(_table[addr.getOffset()]);

	// We need all four here. Can't just stick with 'pred' OR 'succ' because node operations allow us
	// to walk around from any given node
	tmp.push_back(node->pred);
	tmp.push_back(node->succ);
	tmp.push_back(node->key);
	tmp.push_back(node->value);

	return tmp;
}

//-------------------- dynamic memory --------------------

SegmentRef DynMem::dereference(reg_t pointer) {
	SegmentRef ret;
	ret.isRaw = true;
	ret.maxSize = _size - pointer.getOffset();
	ret.raw = _buf + pointer.getOffset();
	return ret;
}

#ifdef ENABLE_SCI32

SegmentRef ArrayTable::dereference(reg_t pointer) {
	SegmentRef ret;
	ret.isRaw = false;
	ret.maxSize = _table[pointer.getOffset()].getSize() * 2;
	ret.reg = _table[pointer.getOffset()].getRawData();
	return ret;
}

void ArrayTable::freeAtAddress(SegManager *segMan, reg_t sub_addr) {
	_table[sub_addr.getOffset()].destroy();
	freeEntry(sub_addr.getOffset());
}

Common::Array<reg_t> ArrayTable::listAllOutgoingReferences(reg_t addr) const {
	Common::Array<reg_t> tmp;
	if (!isValidEntry(addr.getOffset())) {
		error("Invalid array referenced for outgoing references: %04x:%04x", PRINT_REG(addr));
	}

	const SciArray<reg_t> *array = &(_table[addr.getOffset()]);

	for (uint32 i = 0; i < array->getSize(); i++) {
		reg_t value = array->getValue(i);
		if (value.getSegment() != 0)
			tmp.push_back(value);
	}

	return tmp;
}

Common::String SciString::toString() const {
	if (_type != 3)
		error("SciString::toString(): Array is not a string");

	Common::String string;
	for (uint32 i = 0; i < _size && _data[i] != 0; i++)
		string += _data[i];

	return string;
}

void SciString::fromString(const Common::String &string) {
	if (_type != 3)
		error("SciString::fromString(): Array is not a string");

	setSize(string.size() + 1);

	for (uint32 i = 0; i < string.size(); i++)
		_data[i] = string[i];

	_data[string.size()] = 0;
}

SegmentRef StringTable::dereference(reg_t pointer) {
	SegmentRef ret;
	ret.isRaw = true;
	ret.maxSize = _table[pointer.getOffset()].getSize();
	ret.raw = (byte *)_table[pointer.getOffset()].getRawData();
	return ret;
}

#endif

} // End of namespace Sci
