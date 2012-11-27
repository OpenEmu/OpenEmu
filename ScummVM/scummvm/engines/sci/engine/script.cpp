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

#include "sci/sci.h"
#include "sci/resource.h"
#include "sci/util.h"
#include "sci/engine/features.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/script.h"

#include "common/util.h"

namespace Sci {

Script::Script() : SegmentObj(SEG_TYPE_SCRIPT), _buf(NULL) {
	freeScript();
}

Script::~Script() {
	freeScript();
}

void Script::freeScript() {
	_nr = 0;

	free(_buf);
	_buf = NULL;
	_bufSize = 0;
	_scriptSize = 0;
	_heapStart = NULL;
	_heapSize = 0;

	_exportTable = NULL;
	_numExports = 0;
	_synonyms = NULL;
	_numSynonyms = 0;

	_localsOffset = 0;
	_localsSegment = 0;
	_localsBlock = NULL;
	_localsCount = 0;

	_lockers = 1;
	_markedAsDeleted = false;
	_objects.clear();
}

void Script::load(int script_nr, ResourceManager *resMan) {
	freeScript();

	Resource *script = resMan->findResource(ResourceId(kResourceTypeScript, script_nr), 0);
	if (!script)
		error("Script %d not found", script_nr);

	_nr = script_nr;
	_bufSize = _scriptSize = script->size;

	if (getSciVersion() == SCI_VERSION_0_EARLY) {
		_bufSize += READ_LE_UINT16(script->data) * 2;
	} else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1) {
		// In SCI1.1 - SCI2.1, the heap was in a separate space from the script. We append
		// it to the end of the script, and adjust addressing accordingly.
		// However, since we address the heap with a 16-bit pointer, the
		// combined size of the stack and the heap must be 64KB. So far this has
		// worked for SCI11, SCI2 and SCI21 games. SCI3 games use a different
		// script format, and theoretically they can exceed the 64KB boundary
		// using relocation.
		Resource *heap = resMan->findResource(ResourceId(kResourceTypeHeap, script_nr), 0);
		_bufSize += heap->size;
		_heapSize = heap->size;

		// Ensure that the start of the heap resource can be word-aligned.
		if (script->size & 2) {
			_bufSize++;
			_scriptSize++;
		}

		// As mentioned above, the script and the heap together should not exceed 64KB
		if (script->size + heap->size > 65535)
			error("Script and heap sizes combined exceed 64K. This means a fundamental "
					"design bug was made regarding SCI1.1 and newer games.\n"
					"Please report this error to the ScummVM team");
	} else if (getSciVersion() == SCI_VERSION_3) {
		// Check for scripts over 64KB. These won't work with the current 16-bit address
		// scheme. We need an overlaying mechanism, or a mechanism to split script parts
		// in different segments to handle these. For now, simply stop when such a script
		// is found.
		//
		// Known large SCI 3 scripts are:
		// Lighthouse: 9, 220, 270, 351, 360, 490, 760, 765, 800
		// LSL7: 240, 511, 550
		// Phantasmagoria 2: none (hooray!)
		// RAMA: 70
		//
		// TODO: Remove this once such a mechanism is in place
		if (script->size > 65535)
			error("TODO: SCI script %d is over 64KB - it's %d bytes long. This can't "
			      "be handled at the moment, thus stopping", script_nr, script->size);
	}

	uint extraLocalsWorkaround = 0;
	if (g_sci->getGameId() == GID_FANMADE && _nr == 1 && script->size == 11140) {
		// WORKAROUND: Script 1 in Ocean Battle doesn't have enough locals to
		// fit the string showing how many shots are left (a nasty script bug,
		// corrupting heap memory). We add 10 more locals so that it has enough
		// space to use as the target for its kFormat operation. Fixes bug
		// #3059871.
		extraLocalsWorkaround = 10;
	}
	_bufSize += extraLocalsWorkaround * 2;

	_buf = (byte *)malloc(_bufSize);
	assert(_buf);

	assert(_bufSize >= script->size);
	memcpy(_buf, script->data, script->size);

	// Check scripts for matching signatures and patch those, if found
	matchSignatureAndPatch(_nr, _buf, script->size);

	if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1) {
		Resource *heap = resMan->findResource(ResourceId(kResourceTypeHeap, _nr), 0);
		assert(heap != 0);

		_heapStart = _buf + _scriptSize;

		assert(_bufSize - _scriptSize >= heap->size);
		memcpy(_heapStart, heap->data, heap->size);
	}

	if (getSciVersion() <= SCI_VERSION_1_LATE) {
		_exportTable = (const uint16 *)findBlockSCI0(SCI_OBJ_EXPORTS);
		if (_exportTable) {
			_numExports = READ_SCI11ENDIAN_UINT16(_exportTable + 1);
			_exportTable += 3;	// skip header plus 2 bytes (_exportTable is a uint16 pointer)
		}
		_synonyms = findBlockSCI0(SCI_OBJ_SYNONYMS);
		if (_synonyms) {
			_numSynonyms = READ_SCI11ENDIAN_UINT16(_synonyms + 2) / 4;
			_synonyms += 4;	// skip header
		}
		const byte* localsBlock = findBlockSCI0(SCI_OBJ_LOCALVARS);
		if (localsBlock) {
			_localsOffset = localsBlock - _buf + 4;
			_localsCount = (READ_LE_UINT16(_buf + _localsOffset - 2) - 4) >> 1;	// half block size
		}
	} else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1) {
		if (READ_LE_UINT16(_buf + 1 + 5) > 0) {	// does the script have an export table?
			_exportTable = (const uint16 *)(_buf + 1 + 5 + 2);
			_numExports = READ_SCI11ENDIAN_UINT16(_exportTable - 1);
		}
		_localsOffset = _scriptSize + 4;
		_localsCount = READ_SCI11ENDIAN_UINT16(_buf + _localsOffset - 2);
	} else if (getSciVersion() == SCI_VERSION_3) {
		_localsCount = READ_LE_UINT16(_buf + 12);
		_exportTable = (const uint16 *)(_buf + 22);
		_numExports = READ_LE_UINT16(_buf + 20);
		// SCI3 local variables always start dword-aligned
		if (_numExports % 2)
			_localsOffset = 22 + _numExports * 2;
		else
			_localsOffset = 24 + _numExports * 2;
	}

	// WORKAROUND: Increase locals, if needed (check above)
	_localsCount += extraLocalsWorkaround;

	if (getSciVersion() == SCI_VERSION_0_EARLY) {
		// SCI0 early
		// Old script block. There won't be a localvar block in this case.
		// Instead, the script starts with a 16 bit int specifying the
		// number of locals we need; these are then allocated and zeroed.
		_localsCount = READ_LE_UINT16(_buf);
		_localsOffset = -_localsCount * 2; // Make sure it's invalid
	} else {
		// SCI0 late and newer
		// Does the script actually have locals? If not, set the locals offset to 0
		if (!_localsCount)
			_localsOffset = 0;

		if (_localsOffset + _localsCount * 2 + 1 >= (int)_bufSize) {
			error("Locals extend beyond end of script: offset %04x, count %d vs size %d", _localsOffset, _localsCount, (int)_bufSize);
			//_localsCount = (_bufSize - _localsOffset) >> 1;
		}
	}
}

const byte *Script::getSci3ObjectsPointer() {
	const byte *ptr = 0;

	// SCI3 local variables always start dword-aligned
	if (_numExports % 2)
		ptr = _buf + 22 + _numExports * 2;
	else
		ptr = _buf + 24 + _numExports * 2;

	// SCI3 object structures always start dword-aligned
	if (_localsCount % 2)
		ptr += 2 + _localsCount * 2;
	else
		ptr += _localsCount * 2;

	return ptr;
}

Object *Script::getObject(uint16 offset) {
	if (_objects.contains(offset))
		return &_objects[offset];
	else
		return 0;
}

const Object *Script::getObject(uint16 offset) const {
	if (_objects.contains(offset))
		return &_objects[offset];
	else
		return 0;
}

Object *Script::scriptObjInit(reg_t obj_pos, bool fullObjectInit) {
	if (getSciVersion() < SCI_VERSION_1_1 && fullObjectInit)
		obj_pos.incOffset(8);	// magic offset (SCRIPT_OBJECT_MAGIC_OFFSET)

	if (obj_pos.getOffset() >= _bufSize)
		error("Attempt to initialize object beyond end of script");

	// Get the object at the specified position and init it. This will
	// automatically "allocate" space for it in the _objects map if necessary.
	Object *obj = &_objects[obj_pos.getOffset()];
	obj->init(_buf, obj_pos, fullObjectInit);

	return obj;
}

// This helper function is used by Script::relocateLocal and Object::relocate
// Duplicate in segment.cpp and script.cpp
static bool relocateBlock(Common::Array<reg_t> &block, int block_location, SegmentId segment, int location, size_t scriptSize) {
	int rel = location - block_location;

	if (rel < 0)
		return false;

	uint idx = rel >> 1;

	if (idx >= block.size())
		return false;

	if (rel & 1) {
		error("Attempt to relocate odd variable #%d.5e (relative to %04x)\n", idx, block_location);
		return false;
	}
	block[idx].setSegment(segment); // Perform relocation
	if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1)
		block[idx].incOffset(scriptSize);

	return true;
}

int Script::relocateOffsetSci3(uint32 offset) {
	int relocStart = READ_LE_UINT32(_buf + 8);
	int relocCount = READ_LE_UINT16(_buf + 18);
	const byte *seeker = _buf + relocStart;

	for (int i = 0; i < relocCount; ++i) {
		if (READ_SCI11ENDIAN_UINT32(seeker) == offset) {
			// TODO: Find out what UINT16 at (seeker + 8) means
			return READ_SCI11ENDIAN_UINT16(_buf + offset) + READ_SCI11ENDIAN_UINT32(seeker + 4);
		}
		seeker += 10;
	}

	return -1;
}

bool Script::relocateLocal(SegmentId segment, int location) {
	if (_localsBlock)
		return relocateBlock(_localsBlock->_locals, _localsOffset, segment, location, _scriptSize);
	else
		return false;
}

void Script::relocateSci0Sci21(reg_t block) {
	const byte *heap = _buf;
	uint16 heapSize = (uint16)_bufSize;
	uint16 heapOffset = 0;

	if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1) {
		heap = _heapStart;
		heapSize = (uint16)_heapSize;
		heapOffset = _scriptSize;
	}

	if (block.getOffset() >= (uint16)heapSize ||
		READ_SCI11ENDIAN_UINT16(heap + block.getOffset()) * 2 + block.getOffset() >= (uint16)heapSize)
	    error("Relocation block outside of script");

	int count = READ_SCI11ENDIAN_UINT16(heap + block.getOffset());
	int exportIndex = 0;
	int pos = 0;

	for (int i = 0; i < count; i++) {
		pos = READ_SCI11ENDIAN_UINT16(heap + block.getOffset() + 2 + (exportIndex * 2)) + heapOffset;
		// This occurs in SCI01/SCI1 games where usually one export value is
		// zero. It seems that in this situation, we should skip the export and
		// move to the next one, though the total count of valid exports remains
		// the same
		if (!pos) {
			exportIndex++;
			pos = READ_SCI11ENDIAN_UINT16(heap + block.getOffset() + 2 + (exportIndex * 2)) + heapOffset;
			if (!pos)
				error("Script::relocate(): Consecutive zero exports found");
		}

		// In SCI0-SCI1, script local variables, objects and code are relocated.
		// We only relocate locals and objects here, and ignore relocation of
		// code blocks. In SCI1.1 and newer versions, only locals and objects
		// are relocated.
		if (!relocateLocal(block.getSegment(), pos)) {
			// Not a local? It's probably an object or code block. If it's an
			// object, relocate it.
			const ObjMap::iterator end = _objects.end();
			for (ObjMap::iterator it = _objects.begin(); it != end; ++it)
				if (it->_value.relocateSci0Sci21(block.getSegment(), pos, _scriptSize))
					break;
		}

		exportIndex++;
	}
}

void Script::relocateSci3(reg_t block) {
	const byte *relocStart = _buf + READ_SCI11ENDIAN_UINT32(_buf + 8);
	//int count = _bufSize - READ_SCI11ENDIAN_UINT32(_buf + 8);

	ObjMap::iterator it;
	for (it = _objects.begin(); it != _objects.end(); ++it) {
		const byte *seeker = relocStart;
		while (seeker < _buf + _bufSize) {
			// TODO: Find out what UINT16 at (seeker + 8) means
			it->_value.relocateSci3(block.getSegment(),
						READ_SCI11ENDIAN_UINT32(seeker),
						READ_SCI11ENDIAN_UINT32(seeker + 4),
						_scriptSize);
			seeker += 10;
		}
	}
}

void Script::incrementLockers() {
	assert(!_markedAsDeleted);
	_lockers++;
}

void Script::decrementLockers() {
	if (_lockers > 0)
		_lockers--;
}

int Script::getLockers() const {
	return _lockers;
}

void Script::setLockers(int lockers) {
	assert(lockers == 0 || !_markedAsDeleted);
	_lockers = lockers;
}

uint32 Script::validateExportFunc(int pubfunct, bool relocSci3) {
	bool exportsAreWide = (g_sci->_features->detectLofsType() == SCI_VERSION_1_MIDDLE);

	if (_numExports <= pubfunct) {
		error("validateExportFunc(): pubfunct is invalid");
		return 0;
	}

	if (exportsAreWide)
		pubfunct *= 2;

	uint32 offset;

	if (getSciVersion() != SCI_VERSION_3) {
		offset = READ_SCI11ENDIAN_UINT16(_exportTable + pubfunct);
	} else {
		if (!relocSci3)
			offset = READ_SCI11ENDIAN_UINT16(_exportTable + pubfunct) + getCodeBlockOffsetSci3();
		else
			offset = relocateOffsetSci3(pubfunct * 2 + 22);
	}

	// Check if the offset found points to a second export table (e.g. script 912
	// in Camelot and script 306 in KQ4). Such offsets are usually small (i.e. < 10),
	// thus easily distinguished from actual code offsets.
	// This only makes sense for SCI0-SCI1, as the export table in SCI1.1+ games
	// is located at a specific address, thus findBlockSCI0() won't work.
	// Fixes bugs #3039785 and #3037595.
	if (offset < 10 && getSciVersion() <= SCI_VERSION_1_LATE) {
		const uint16 *secondExportTable = (const uint16 *)findBlockSCI0(SCI_OBJ_EXPORTS, 0);

		if (secondExportTable) {
			secondExportTable += 3;	// skip header plus 2 bytes (secondExportTable is a uint16 pointer)
			offset = READ_SCI11ENDIAN_UINT16(secondExportTable + pubfunct);
		}
	}

	// Note that it's perfectly normal to return a zero offset, especially in
	// SCI1.1 and newer games. Examples include script 64036 in Torin's Passage,
	// script 64908 in the demo of RAMA and script 1013 in KQ6 floppy.

	if (offset >= _bufSize)
		error("Invalid export function pointer");

	return offset;
}

byte *Script::findBlockSCI0(int type, int startBlockIndex) {
	byte *buf = _buf;
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);
	int blockIndex = 0;

	if (oldScriptHeader)
		buf += 2;

	do {
		int blockType = READ_LE_UINT16(buf);

		if (blockType == 0)
			break;
		if (blockType == type && blockIndex > startBlockIndex)
			return buf;

		int blockSize = READ_LE_UINT16(buf + 2);
		assert(blockSize > 0);
		buf += blockSize;
		blockIndex++;
	} while (1);

	return NULL;
}

// memory operations

void Script::mcpyInOut(int dst, const void *src, size_t n) {
	if (_buf) {
		assert(dst + n <= _bufSize);
		memcpy(_buf + dst, src, n);
	}
}

bool Script::isValidOffset(uint16 offset) const {
	return offset < _bufSize;
}

SegmentRef Script::dereference(reg_t pointer) {
	if (pointer.getOffset() > _bufSize) {
		error("Script::dereference(): Attempt to dereference invalid pointer %04x:%04x into script segment (script size=%d)",
				  PRINT_REG(pointer), (uint)_bufSize);
		return SegmentRef();
	}

	SegmentRef ret;
	ret.isRaw = true;
	ret.maxSize = _bufSize - pointer.getOffset();
	ret.raw = _buf + pointer.getOffset();
	return ret;
}

LocalVariables *Script::allocLocalsSegment(SegManager *segMan) {
	if (!getLocalsCount()) { // No locals
		return NULL;
	} else {
		LocalVariables *locals;

		if (_localsSegment) {
			locals = (LocalVariables *)segMan->getSegment(_localsSegment, SEG_TYPE_LOCALS);
			if (!locals || locals->getType() != SEG_TYPE_LOCALS || locals->script_id != getScriptNumber())
				error("Invalid script locals segment while allocating locals");
		} else
			locals = (LocalVariables *)segMan->allocSegment(new LocalVariables(), &_localsSegment);

		_localsBlock = locals;
		locals->script_id = getScriptNumber();
		locals->_locals.resize(getLocalsCount());

		return locals;
	}
}

void Script::initializeLocals(SegManager *segMan) {
	LocalVariables *locals = allocLocalsSegment(segMan);
	if (locals) {
		if (getSciVersion() > SCI_VERSION_0_EARLY) {
			const byte *base = (const byte *)(_buf + getLocalsOffset());

			for (uint16 i = 0; i < getLocalsCount(); i++)
				locals->_locals[i] = make_reg(0, READ_SCI11ENDIAN_UINT16(base + i * 2));
		} else {
			// In SCI0 early, locals are set at run time, thus zero them all here
			for (uint16 i = 0; i < getLocalsCount(); i++)
				locals->_locals[i] = NULL_REG;
		}
	}
}

void Script::syncLocalsBlock(SegManager *segMan) {
	_localsBlock = (_localsSegment == 0) ? NULL : (LocalVariables *)(segMan->getSegment(_localsSegment, SEG_TYPE_LOCALS));
}

void Script::initializeClasses(SegManager *segMan) {
	const byte *seeker = 0;
	uint16 mult = 0;

	if (getSciVersion() <= SCI_VERSION_1_LATE) {
		seeker = findBlockSCI0(SCI_OBJ_CLASS);
		mult = 1;
	} else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1) {
		seeker = _heapStart + 4 + READ_SCI11ENDIAN_UINT16(_heapStart + 2) * 2;
		mult = 2;
	} else if (getSciVersion() == SCI_VERSION_3) {
		seeker = getSci3ObjectsPointer();
		mult = 1;
	}

	if (!seeker)
		return;

	uint16 marker;
	bool isClass = false;
	uint32 classpos;
	int16 species = 0;

	while (true) {
		// In SCI0-SCI1, this is the segment type. In SCI11, it's a marker (0x1234)
		marker = READ_SCI11ENDIAN_UINT16(seeker);
		classpos = seeker - _buf;

		if (getSciVersion() <= SCI_VERSION_1_LATE && !marker)
			break;

		if (getSciVersion() >= SCI_VERSION_1_1 && marker != SCRIPT_OBJECT_MAGIC_NUMBER)
			break;

		if (getSciVersion() <= SCI_VERSION_1_LATE) {
			isClass = (marker == SCI_OBJ_CLASS);
			if (isClass)
				species = READ_SCI11ENDIAN_UINT16(seeker + 12);
			classpos += 12;
		} else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1) {
			isClass = (READ_SCI11ENDIAN_UINT16(seeker + 14) & kInfoFlagClass);	// -info- selector
			species = READ_SCI11ENDIAN_UINT16(seeker + 10);
		} else if (getSciVersion() == SCI_VERSION_3) {
			isClass = (READ_SCI11ENDIAN_UINT16(seeker + 10) & kInfoFlagClass);
			species = READ_SCI11ENDIAN_UINT16(seeker + 4);
		}

		if (isClass) {
			// WORKAROUNDs for off-by-one script errors
			if (species == (int)segMan->classTableSize()) {
				if (g_sci->getGameId() == GID_LSL2 && g_sci->isDemo())
					segMan->resizeClassTable(species + 1);
				else if (g_sci->getGameId() == GID_LSL3 && !g_sci->isDemo() && _nr == 500)
					segMan->resizeClassTable(species + 1);
				else if (g_sci->getGameId() == GID_SQ3 && !g_sci->isDemo() && _nr == 93)
					segMan->resizeClassTable(species + 1);
				else if (g_sci->getGameId() == GID_SQ3 && !g_sci->isDemo() && _nr == 99)
					segMan->resizeClassTable(species + 1);
			}

			if (species < 0 || species >= (int)segMan->classTableSize())
				error("Invalid species %d(0x%x) unknown max %d(0x%x) while instantiating script %d\n",
						  species, species, segMan->classTableSize(), segMan->classTableSize(), _nr);

			SegmentId segmentId = segMan->getScriptSegment(_nr);
			segMan->setClassOffset(species, make_reg(segmentId, classpos));
		}

		seeker += READ_SCI11ENDIAN_UINT16(seeker + 2) * mult;
	}
}

void Script::initializeObjectsSci0(SegManager *segMan, SegmentId segmentId) {
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

	// We need to make two passes, as the objects in the script might be in the
	// wrong order (e.g. in the demo of Iceman) - refer to bug #3034713
	for (int pass = 1; pass <= 2; pass++) {
		const byte *seeker = _buf + (oldScriptHeader ? 2 : 0);

		do {
			uint16 objType = READ_SCI11ENDIAN_UINT16(seeker);
			if (!objType)
				break;

			switch (objType) {
			case SCI_OBJ_OBJECT:
			case SCI_OBJ_CLASS:
				{
					reg_t addr = make_reg(segmentId, seeker - _buf + 4);
					Object *obj = scriptObjInit(addr);
					obj->initSpecies(segMan, addr);

					if (pass == 2) {
						if (!obj->initBaseObject(segMan, addr)) {
							if ((_nr == 202 || _nr == 764) && g_sci->getGameId() == GID_KQ5) {
								// WORKAROUND: Script 202 of KQ5 French and German
								// (perhaps Spanish too?) has an invalid object.
								// This is non-fatal. Refer to bugs #3035396 and
								// #3150767.
								// Same happens with script 764, it seems to
								// contain junk towards its end.
								_objects.erase(addr.toUint16() - SCRIPT_OBJECT_MAGIC_OFFSET);
							} else {
								error("Failed to locate base object for object at %04X:%04X", PRINT_REG(addr));
							}
						}
					}
				}
				break;

			default:
				break;
			}

			seeker += READ_SCI11ENDIAN_UINT16(seeker + 2);
		} while ((uint32)(seeker - _buf) < getScriptSize() - 2);
	}

	byte *relocationBlock = findBlockSCI0(SCI_OBJ_POINTERS);
	if (relocationBlock)
		relocateSci0Sci21(make_reg(segmentId, relocationBlock - getBuf() + 4));
}

void Script::initializeObjectsSci11(SegManager *segMan, SegmentId segmentId) {
	const byte *seeker = _heapStart + 4 + READ_SCI11ENDIAN_UINT16(_heapStart + 2) * 2;

	while (READ_SCI11ENDIAN_UINT16(seeker) == SCRIPT_OBJECT_MAGIC_NUMBER) {
		reg_t reg = make_reg(segmentId, seeker - _buf);
		Object *obj = scriptObjInit(reg);

		// Copy base from species class, as we need its selector IDs
		obj->setSuperClassSelector(
			segMan->getClassAddress(obj->getSuperClassSelector().getOffset(), SCRIPT_GET_LOCK, 0));

		// If object is instance, get -propDict- from class and set it for this
		// object. This is needed for ::isMemberOf() to work.
		// Example test case - room 381 of sq4cd - if isMemberOf() doesn't work,
		// talk-clicks on the robot will act like clicking on ego
		if (!obj->isClass()) {
			reg_t classObject = obj->getSuperClassSelector();
			const Object *classObj = segMan->getObject(classObject);
			obj->setPropDictSelector(classObj->getPropDictSelector());
		}

		// Set the -classScript- selector to the script number.
		// FIXME: As this selector is filled in at run-time, it is likely
		// that it is supposed to hold a pointer. The Obj::isKindOf method
		// uses this selector together with -propDict- to compare classes.
		// For the purpose of Obj::isKindOf, using the script number appears
		// to be sufficient.
		obj->setClassScriptSelector(make_reg(0, _nr));

		seeker += READ_SCI11ENDIAN_UINT16(seeker + 2) * 2;
	}

	relocateSci0Sci21(make_reg(segmentId, READ_SCI11ENDIAN_UINT16(_heapStart)));
}

void Script::initializeObjectsSci3(SegManager *segMan, SegmentId segmentId) {
	const byte *seeker = getSci3ObjectsPointer();

	while (READ_SCI11ENDIAN_UINT16(seeker) == SCRIPT_OBJECT_MAGIC_NUMBER) {
		reg_t reg = make_reg(segmentId, seeker - _buf);
		Object *obj = scriptObjInit(reg);

		obj->setSuperClassSelector(segMan->getClassAddress(obj->getSuperClassSelector().getOffset(), SCRIPT_GET_LOCK, 0));
		seeker += READ_SCI11ENDIAN_UINT16(seeker + 2);
	}

	relocateSci3(make_reg(segmentId, 0));
}

void Script::initializeObjects(SegManager *segMan, SegmentId segmentId) {
	if (getSciVersion() <= SCI_VERSION_1_LATE)
		initializeObjectsSci0(segMan, segmentId);
	else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1)
		initializeObjectsSci11(segMan, segmentId);
	else if (getSciVersion() == SCI_VERSION_3)
		initializeObjectsSci3(segMan, segmentId);
}

reg_t Script::findCanonicAddress(SegManager *segMan, reg_t addr) const {
	addr.setOffset(0);
	return addr;
}

void Script::freeAtAddress(SegManager *segMan, reg_t addr) {
	/*
		debugC(kDebugLevelGC, "[GC] Freeing script %04x:%04x", PRINT_REG(addr));
		if (_localsSegment)
			debugC(kDebugLevelGC, "[GC] Freeing locals %04x:0000", _localsSegment);
	*/

	if (_markedAsDeleted)
		segMan->deallocateScript(_nr);
}

Common::Array<reg_t> Script::listAllDeallocatable(SegmentId segId) const {
	const reg_t r = make_reg(segId, 0);
	return Common::Array<reg_t>(&r, 1);
}

Common::Array<reg_t> Script::listAllOutgoingReferences(reg_t addr) const {
	Common::Array<reg_t> tmp;
	if (addr.getOffset() <= _bufSize && addr.getOffset() >= (uint)-SCRIPT_OBJECT_MAGIC_OFFSET && offsetIsObject(addr.getOffset())) {
		const Object *obj = getObject(addr.getOffset());
		if (obj) {
			// Note all local variables, if we have a local variable environment
			if (_localsSegment)
				tmp.push_back(make_reg(_localsSegment, 0));

			for (uint i = 0; i < obj->getVarCount(); i++)
				tmp.push_back(obj->getVariable(i));
		} else {
			error("Request for outgoing script-object reference at %04x:%04x failed", PRINT_REG(addr));
		}
	} else {
		/*		warning("Unexpected request for outgoing script-object references at %04x:%04x", PRINT_REG(addr));*/
		/* Happens e.g. when we're looking into strings */
	}
	return tmp;
}

Common::Array<reg_t> Script::listObjectReferences() const {
	Common::Array<reg_t> tmp;

	// Locals, if present
	if (_localsSegment)
		tmp.push_back(make_reg(_localsSegment, 0));

	// All objects (may be classes, may be indirectly reachable)
	ObjMap::iterator it;
	const ObjMap::iterator end = _objects.end();
	for (it = _objects.begin(); it != end; ++it) {
		tmp.push_back(it->_value.getPos());
	}

	return tmp;
}

bool Script::offsetIsObject(uint16 offset) const {
	return (READ_SCI11ENDIAN_UINT16((const byte *)_buf + offset + SCRIPT_OBJECT_MAGIC_OFFSET) == SCRIPT_OBJECT_MAGIC_NUMBER);
}

} // End of namespace Sci
