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

#include "gob/save/saveload.h"
#include "gob/save/saveconverter.h"
#include "gob/inter.h"
#include "gob/variables.h"

namespace Gob {

SaveLoad_Fascination::SaveFile SaveLoad_Fascination::_saveFiles[] = {
	{ "cat.cat",    kSaveModeSave, -1, 0, "savegame catalog"},
	{ "save0.inf",  kSaveModeSave,  0, 0, "savegame"},
	{ "save1.inf",  kSaveModeSave,  1, 0, "savegame"},
	{ "save2.inf",  kSaveModeSave,  2, 0, "savegame"},
	{ "save3.inf",  kSaveModeSave,  3, 0, "savegame"},
	{ "save4.inf",  kSaveModeSave,  4, 0, "savegame"},
	{ "save5.inf",  kSaveModeSave,  5, 0, "savegame"},
	{ "save6.inf",  kSaveModeSave,  6, 0, "savegame"},
	{ "save7.inf",  kSaveModeSave,  7, 0, "savegame"},
	{ "save8.inf",  kSaveModeSave,  8, 0, "savegame"},
	{ "save9.inf",  kSaveModeSave,  9, 0, "savegame"},
	{ "save10.inf", kSaveModeSave, 10, 0, "savegame"},
	{ "save11.inf", kSaveModeSave, 11, 0, "savegame"},
	{ "save12.inf", kSaveModeSave, 12, 0, "savegame"},
	{ "save13.inf", kSaveModeSave, 13, 0, "savegame"},
	{ "save14.inf", kSaveModeSave, 14, 0, "savegame"},
};


SaveLoad_Fascination::GameHandler::File::File(GobEngine *vm, const char *base) :
	SlotFileIndexed(vm, SaveLoad_Fascination::kSlotCount, base, "s") {
}

SaveLoad_Fascination::GameHandler::File::~File() {
}

int SaveLoad_Fascination::GameHandler::File::getSlot(int32 offset) const {
	return ((offset - kIndexSize) / 320);
}

int SaveLoad_Fascination::GameHandler::File::getSlotRemainder(int32 offset) const {
	return ((offset - kIndexSize) % 320);
}


SaveLoad_Fascination::GameHandler::GameHandler(GobEngine *vm, const char *target,
		int slot, byte *index, bool *hasIndex) : SaveHandler(vm) {

	_index    = index;
	_hasIndex = hasIndex;

	_slot = slot;

	_slotFile = new File(vm, target);
}

SaveLoad_Fascination::GameHandler::~GameHandler() {
	delete _slotFile;
}

int32 SaveLoad_Fascination::GameHandler::getSize() {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return _slotFile->tallyUpFiles(varSize, kIndexSize);
}

bool SaveLoad_Fascination::GameHandler::load(int16 dataVar, int32 size, int32 offset) {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return false;

	if (size == 0) {
		// Indicator to load all variables
		dataVar = 0;
		size = varSize;
	}

	if ((offset == 0) && (_slot == -1)) {
		// Save index

		if (((uint32) size) != kIndexSize) {
			warning("Requested index has wrong size (%d)", size);
			return false;
		}

		// Create/Fake the index
		buildIndex(_vm->_inter->_variables->getAddressOff8(dataVar));

	} else {
		// Load slot

		uint32 slot = _slot;
		int slotRem = 0;
		if (_slot == -1) {
			slot    = _slotFile->getSlot(offset);
			slotRem = _slotFile->getSlotRemainder(offset);
		}

		debugC(2, kDebugSaveLoad, "Loading from slot %d", slot);

		if ((slot >= kSlotCount) || (slotRem != 0) || (size != 320)) {
			warning("Invalid loading procedure (%d, %d, %d, %d, %d)",
					dataVar, size, offset, slot, slotRem);
			return false;
		}

		Common::String slotFile = _slotFile->build(slot);

		SaveReader *reader = 0;

		// New save, load directly
		reader = new SaveReader(2, slot, slotFile);

		SavePartInfo info(kSlotNameLength, (uint32) _vm->getGameType(), 0,
				_vm->getEndianness(), varSize);
		SavePartVars vars(_vm, size);

		if (!reader->load()) {
			delete reader;
			return false;
		}

		if (!reader->readPart(0, &info)) {
			delete reader;
			return false;
		}
		if (!reader->readPart(1, &vars)) {
			delete reader;
			return false;
		}

		// Get all variables
		if (!vars.writeInto(dataVar, 0, size)) {
			delete reader;
			return false;
		}

		delete reader;
	}

	return true;
}

bool SaveLoad_Fascination::GameHandler::save(int16 dataVar, int32 size, int32 offset) {
	if ((_slot == -1) && (offset == 0) && (size == 5400))
		// Initialize empty file
		return true;

	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return false;

	if (size == 0) {
		// Indicator to save all variables
		dataVar = 0;
		size = varSize;
	}

	if ((offset == 0) && (_slot == -1)) {
		// Save index

		if (((uint32) size) != kIndexSize) {
			warning("Requested index has wrong size (%d)", size);
			return false;
		}

		// Just copy the index into our buffer
		_vm->_inter->_variables->copyTo(dataVar, _index, kIndexSize);
		*_hasIndex = true;

	} else {
		// Save slot

		uint32 slot = _slot;
		int slotRem = 0;
		if (_slot == -1) {
			slot    = _slotFile->getSlot(offset);
			slotRem = _slotFile->getSlotRemainder(offset);
		}

		debugC(2, kDebugSaveLoad, "Saving to slot %d", slot);

		if ((slot >= kSlotCount) || (slotRem != 0) || (size != 320)) {
			warning("Invalid saving procedure (%d, %d, %d, %d, %d)",
					dataVar, size, offset, slot, slotRem);
			return false;
		}

		// An index is needed for the save slot description
		if (!*_hasIndex) {
			warning("No index written yet");
			return false;
		}

		*_hasIndex = false;

		Common::String slotFile = _slotFile->build(slot);

		SaveWriter writer(2, slot, slotFile);
		SavePartInfo info(kSlotNameLength, (uint32) _vm->getGameType(), 0,
				_vm->getEndianness(), varSize);
		SavePartVars vars(_vm, size);

		// Write the description
		info.setDesc(_index + (slot * kSlotNameLength), kSlotNameLength);
		// Write all variables
		if (!vars.readFrom(dataVar, 0, size))
			return false;

		if (!writer.writePart(0, &info))
			return false;
		if (!writer.writePart(1, &vars))
			return false;
	}

	return true;
}

void SaveLoad_Fascination::GameHandler::buildIndex(byte *buffer) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return;

	SavePartInfo info(kSlotNameLength, (uint32) _vm->getGameType(),
			0, _vm->getEndianness(), varSize);

	_slotFile->buildIndex(buffer, info, 0);
}


SaveLoad_Fascination::SaveLoad_Fascination(GobEngine *vm, const char *targetName) :
		SaveLoad(vm) {

	memset(_index, 0, kIndexSize);
	_hasIndex = false;

	for (int i = 0; i < 16; i++)
		_saveFiles[i].handler = new GameHandler(vm, targetName, _saveFiles[i].slot, _index, &_hasIndex);
}

SaveLoad_Fascination::~SaveLoad_Fascination() {
	for (int i = 0; i < 16; i++)
		delete _saveFiles[i].handler;
}

const SaveLoad_Fascination::SaveFile *SaveLoad_Fascination::getSaveFile(const char *fileName) const {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveLoad_Fascination::SaveFile *SaveLoad_Fascination::getSaveFile(const char *fileName) {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveHandler *SaveLoad_Fascination::getHandler(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->handler;

	return 0;
}

const char *SaveLoad_Fascination::getDescription(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->description;

	return 0;
}

SaveLoad::SaveMode SaveLoad_Fascination::getSaveMode(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->mode;

	return kSaveModeNone;
}

} // End of namespace Gob
