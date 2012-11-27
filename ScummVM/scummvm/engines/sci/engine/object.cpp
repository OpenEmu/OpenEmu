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


#include "sci/engine/kernel.h"
#include "sci/engine/object.h"
#include "sci/engine/seg_manager.h"

namespace Sci {

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

void Object::init(byte *buf, reg_t obj_pos, bool initVariables) {
	byte *data = buf + obj_pos.getOffset();
	_baseObj = data;
	_pos = obj_pos;

	if (getSciVersion() <= SCI_VERSION_1_LATE) {
		_variables.resize(READ_LE_UINT16(data + kOffsetSelectorCounter));
		_baseVars = (const uint16 *)(_baseObj + _variables.size() * 2);
		_methodCount = READ_LE_UINT16(data + READ_LE_UINT16(data + kOffsetFunctionArea) - 2);
		for (int i = 0; i < _methodCount * 2 + 2; ++i) {
			_baseMethod.push_back(READ_SCI11ENDIAN_UINT16(data + READ_LE_UINT16(data + kOffsetFunctionArea) + i * 2));
		}
	} else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1) {
		_variables.resize(READ_SCI11ENDIAN_UINT16(data + 2));
		_baseVars = (const uint16 *)(buf + READ_SCI11ENDIAN_UINT16(data + 4));
		_methodCount = READ_SCI11ENDIAN_UINT16(buf + READ_SCI11ENDIAN_UINT16(data + 6));
		for (int i = 0; i < _methodCount * 2 + 3; ++i) {
			_baseMethod.push_back(READ_SCI11ENDIAN_UINT16(buf + READ_SCI11ENDIAN_UINT16(data + 6) + i * 2));
		}
	} else if (getSciVersion() == SCI_VERSION_3) {
		initSelectorsSci3(buf);
	}

	if (initVariables) {
		if (getSciVersion() <= SCI_VERSION_2_1) {
			for (uint i = 0; i < _variables.size(); i++)
				_variables[i] = make_reg(0, READ_SCI11ENDIAN_UINT16(data + (i * 2)));
		} else {
			_infoSelectorSci3 = make_reg(0, READ_SCI11ENDIAN_UINT16(_baseObj + 10));
		}
	}
}

const Object *Object::getClass(SegManager *segMan) const {
	return isClass() ? this : segMan->getObject(getSuperClassSelector());
}

int Object::locateVarSelector(SegManager *segMan, Selector slc) const {
	const byte *buf = 0;
	uint varnum = 0;

	if (getSciVersion() <= SCI_VERSION_2_1) {
		const Object *obj = getClass(segMan);
		varnum = getSciVersion() <= SCI_VERSION_1_LATE ? getVarCount() : obj->getVariable(1).toUint16();
		buf = (const byte *)obj->_baseVars;
	} else if (getSciVersion() == SCI_VERSION_3) {
		varnum = _variables.size();
		buf = (const byte *)_baseVars;
	}

	for (uint i = 0; i < varnum; i++)
		if (READ_SCI11ENDIAN_UINT16(buf + (i << 1)) == slc) // Found it?
			return i; // report success

	return -1; // Failed
}

bool Object::relocateSci0Sci21(SegmentId segment, int location, size_t scriptSize) {
	return relocateBlock(_variables, getPos().getOffset(), segment, location, scriptSize);
}

bool Object::relocateSci3(SegmentId segment, uint32 location, int offset, size_t scriptSize) {
	assert(_propertyOffsetsSci3);

	for (uint i = 0; i < _variables.size(); ++i) {
		if (location == _propertyOffsetsSci3[i]) {
			_variables[i].setSegment(segment);
			_variables[i].incOffset(offset);
			return true;
		}
	}

	return false;
}

int Object::propertyOffsetToId(SegManager *segMan, int propertyOffset) const {
	int selectors = getVarCount();

	if (propertyOffset < 0 || (propertyOffset >> 1) >= selectors) {
		error("Applied propertyOffsetToId to invalid property offset %x (property #%d not in [0..%d])",
		          propertyOffset, propertyOffset >> 1, selectors - 1);
		return -1;
	}

	if (getSciVersion() < SCI_VERSION_1_1) {
		const byte *selectoroffset = ((const byte *)(_baseObj)) + kOffsetSelectorSegment + selectors * 2;
		return READ_SCI11ENDIAN_UINT16(selectoroffset + propertyOffset);
	} else {
		const Object *obj = this;
		if (!isClass())
			obj = segMan->getObject(getSuperClassSelector());

		return READ_SCI11ENDIAN_UINT16((const byte *)obj->_baseVars + propertyOffset);
	}
}

void Object::initSpecies(SegManager *segMan, reg_t addr) {
	uint16 speciesOffset = getSpeciesSelector().getOffset();

	if (speciesOffset == 0xffff)		// -1
		setSpeciesSelector(NULL_REG);	// no species
	else
		setSpeciesSelector(segMan->getClassAddress(speciesOffset, SCRIPT_GET_LOCK, addr.getSegment()));
}

void Object::initSuperClass(SegManager *segMan, reg_t addr) {
	uint16 superClassOffset = getSuperClassSelector().getOffset();

	if (superClassOffset == 0xffff)			// -1
		setSuperClassSelector(NULL_REG);	// no superclass
	else
		setSuperClassSelector(segMan->getClassAddress(superClassOffset, SCRIPT_GET_LOCK, addr.getSegment()));
}

bool Object::initBaseObject(SegManager *segMan, reg_t addr, bool doInitSuperClass) {
	const Object *baseObj = segMan->getObject(getSpeciesSelector());

	if (baseObj) {
		uint originalVarCount = _variables.size();

		if (_variables.size() != baseObj->getVarCount())
			_variables.resize(baseObj->getVarCount());
		// Copy base from species class, as we need its selector IDs
		_baseObj = baseObj->_baseObj;
		if (doInitSuperClass)
			initSuperClass(segMan, addr);

		if (_variables.size() != originalVarCount) {
			// These objects are probably broken.
			// An example is 'witchCage' in script 200 in KQ5 (#3034714),
			// but also 'girl' in script 216 and 'door' in script 22.
			// In LSL3 a number of sound objects trigger this right away.
			// SQ4-floppy's bug #3037938 also seems related.

			// The effect is that a number of its method selectors may be
			// treated as variable selectors, causing unpredictable effects.
			int objScript = segMan->getScript(_pos.getSegment())->getScriptNumber();

			// We have to do a little bit of work to get the name of the object
			// before any relocations are done.
			reg_t nameReg = getNameSelector();
			const char *name;
			if (nameReg.isNull()) {
				name = "<no name>";
			} else {
				nameReg.setSegment(_pos.getSegment());
				name = segMan->derefString(nameReg);
				if (!name)
					name = "<invalid name>";
			}

			debugC(kDebugLevelVM, "Object %04x:%04x (name %s, script %d) "
			        "varnum doesn't match baseObj's: obj %d, base %d",
			        PRINT_REG(_pos), name, objScript,
			        originalVarCount, baseObj->getVarCount());

#if 0
			// We enumerate the methods selectors which could be hidden here
			if (getSciVersion() <= SCI_VERSION_2_1) {
				const SegmentRef objRef = segMan->dereference(baseObj->_pos);
				assert(objRef.isRaw);
				uint segBound = objRef.maxSize/2 - baseObj->getVarCount();
				const byte* buf = (const byte *)baseObj->_baseVars;
				if (!buf) {
					// While loading this may happen due to objects being loaded
					// out of order, and we can't proceed then, unfortunately.
					segBound = 0;
				}
				for (uint i = baseObj->getVarCount();
				         i < originalVarCount && i < segBound; ++i) {
					uint16 slc = READ_SCI11ENDIAN_UINT16(buf + 2*i);
					// Skip any numbers which happen to be varselectors too
					bool found = false;
					for (uint j = 0; j < baseObj->getVarCount() && !found; ++j)
						found = READ_SCI11ENDIAN_UINT16(buf + 2*j) == slc;
					if (found) continue;
					// Skip any selectors which aren't method selectors,
					// so couldn't be mistaken for varselectors
					if (lookupSelector(segMan, _pos, slc, 0, 0) != kSelectorMethod) continue;
					warning("    Possibly affected selector: %02x (%s)", slc,
					        g_sci->getKernel()->getSelectorName(slc).c_str());
				}
			}
#endif
		}

		return true;
	}

	return false;
}

const int EXTRA_GROUPS = 3;

void Object::initSelectorsSci3(const byte *buf) {
	const byte *groupInfo = _baseObj + 16;
	const byte *selectorBase = groupInfo + EXTRA_GROUPS * 32 * 2;
	int groups = g_sci->getKernel()->getSelectorNamesSize()/32;
	int methods, properties;

	if (g_sci->getKernel()->getSelectorNamesSize() % 32)
		++groups;

	methods = properties = 0;

	// Selectors are divided into groups of 32, of which the first
	// two selectors are always reserved (because their storage
	// space is used by the typeMask).
	// We don't know beforehand how many methods and properties
	// there are, so we count them first.
	for (int groupNr = 0; groupNr < groups; ++groupNr) {
		byte groupLocation = groupInfo[groupNr];
		const byte *seeker = selectorBase + groupLocation * 32 * 2;

		if (groupLocation != 0)	{
			// This object actually has selectors belonging to this group
			int typeMask = READ_SCI11ENDIAN_UINT32(seeker);

			for (int bit = 2; bit < 32; ++bit) {
				int value = READ_SCI11ENDIAN_UINT16(seeker + bit * 2);
				if (typeMask & (1 << bit)) { // Property
					++properties;
				} else if (value != 0xffff) { // Method
					++methods;
				} else {
					// Undefined selector
				}

			}
		}
	}

	_variables.resize(properties);
	uint16 *propertyIds = (uint16 *)malloc(sizeof(uint16) * properties);
//	uint16 *methodOffsets = (uint16 *)malloc(sizeof(uint16) * 2 * methods);
	uint32 *propertyOffsets = (uint32 *)malloc(sizeof(uint32) * properties);
	int propertyCounter = 0;
	int methodCounter = 0;

	// Go through the whole thing again to get the property values
	// and method pointers
	for (int groupNr = 0; groupNr < groups; ++groupNr) {
		byte groupLocation = groupInfo[groupNr];
		const byte *seeker = selectorBase + groupLocation * 32 * 2;

		if (groupLocation != 0)	{
			// This object actually has selectors belonging to this group
			int typeMask = READ_SCI11ENDIAN_UINT32(seeker);
			int groupBaseId = groupNr * 32;

			for (int bit = 2; bit < 32; ++bit) {
				int value = READ_SCI11ENDIAN_UINT16(seeker + bit * 2);
				if (typeMask & (1 << bit)) { // Property

					// FIXME: We really shouldn't be doing endianness
					// conversion here; instead, propertyIds should be converted
					// to a Common::Array, like _baseMethod already is
					// This interim solution fixes playing SCI3 PC games
					// on Big Endian platforms

					WRITE_SCI11ENDIAN_UINT16(&propertyIds[propertyCounter],
					                         groupBaseId + bit);
					_variables[propertyCounter] = make_reg(0, value);
					uint32 propertyOffset = (seeker + bit * 2) - buf;
					propertyOffsets[propertyCounter] = propertyOffset;
					++propertyCounter;
				} else if (value != 0xffff) { // Method
					_baseMethod.push_back(groupBaseId + bit);
					_baseMethod.push_back(value + READ_SCI11ENDIAN_UINT32(buf));
//					methodOffsets[methodCounter] = (seeker + bit * 2) - buf;
					++methodCounter;
				} else {
					// Undefined selector
				}

			}
		}
	}

	_speciesSelectorSci3 = make_reg(0, READ_SCI11ENDIAN_UINT16(_baseObj + 4));
	_superClassPosSci3 = make_reg(0, READ_SCI11ENDIAN_UINT16(_baseObj + 8));

	_baseVars = propertyIds;
	_methodCount = methods;
	_propertyOffsetsSci3 = propertyOffsets;
	//_methodOffsetsSci3 = methodOffsets;
}

} // End of namespace Sci
