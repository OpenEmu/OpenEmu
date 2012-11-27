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
#include "gob/inter.h"
#include "gob/variables.h"

namespace Gob {

SaveLoad_Playtoons::SaveFile SaveLoad_Playtoons::_saveFiles[] = {
	{   "did.inf", kSaveModeSave,   0, 0}, // Purpose ignored at the moment, intensively used to save things.
	{   "dan.itk", kSaveModeNone,   0, 0}, // Playtoons CK detection file
	{   "cat.inf", kSaveModeNone,   0, 0},
	{ "titre.009", kSaveModeIgnore, 0, 0}, // Playtoons theoritical title files that are checked for nothing
	{ "titre.010", kSaveModeIgnore, 0, 0},
	{ "titre.011", kSaveModeIgnore, 0, 0},
	{ "titre.012", kSaveModeIgnore, 0, 0},
	{ "titre.013", kSaveModeIgnore, 0, 0},
	{ "titre.014", kSaveModeIgnore, 0, 0},
	{ "titre.015", kSaveModeIgnore, 0, 0},
	{ "titre.016", kSaveModeIgnore, 0, 0},
	{ "titre.017", kSaveModeIgnore, 0, 0},
	{ "titre.018", kSaveModeIgnore, 0, 0},
	{ "titre.019", kSaveModeIgnore, 0, 0},
	{ "titre.020", kSaveModeIgnore, 0, 0},
	{ "titre.021", kSaveModeIgnore, 0, 0},
	{ "titre.022", kSaveModeIgnore, 0, 0},
	{ "titre.023", kSaveModeIgnore, 0, 0},
	{ "titre.024", kSaveModeIgnore, 0, 0},
	{ "titre.025", kSaveModeIgnore, 0, 0},
	{ "titre.026", kSaveModeIgnore, 0, 0},
	{ "titre.027", kSaveModeIgnore, 0, 0},
	{ "titre.028", kSaveModeIgnore, 0, 0},
	{ "titre.029", kSaveModeIgnore, 0, 0},
	{ "titre.030", kSaveModeIgnore, 0, 0},
	{ "titre.031", kSaveModeIgnore, 0, 0},
	{ "titre.032", kSaveModeIgnore, 0, 0},
	{ "titre.033", kSaveModeIgnore, 0, 0},
	{ "titre.034", kSaveModeIgnore, 0, 0},
	{ "titre.035", kSaveModeIgnore, 0, 0},
	{ "titre.036", kSaveModeIgnore, 0, 0},
	{ "titre.037", kSaveModeIgnore, 0, 0},
	{ "titre.038", kSaveModeIgnore, 0, 0},
	{ "titre.039", kSaveModeIgnore, 0, 0},
};

SaveLoad_Playtoons::GameHandler::File::File(GobEngine *vm, const char *base) :
	SlotFileIndexed(vm, SaveLoad_Playtoons::kSlotCount, base, "s") {
}

SaveLoad_Playtoons::GameHandler::File::~File() {
}

int SaveLoad_Playtoons::GameHandler::File::getSlot(int32 offset) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return ((offset - (kPropsSize + kIndexSize)) / varSize);
}

int SaveLoad_Playtoons::GameHandler::File::getSlotRemainder(int32 offset) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return ((offset - (kPropsSize + kIndexSize)) % varSize);
}


SaveLoad_Playtoons::GameHandler::GameHandler(GobEngine *vm, const char *target) : SaveHandler(vm) {
	memset(_props, 0, kPropsSize);
	memset(_index, 0, kIndexSize);

	_tempSpriteHandler = new TempSpriteHandler(vm);
	_slotFile = new File(vm, target);
}

SaveLoad_Playtoons::GameHandler::~GameHandler() {
	delete _slotFile;
	delete _tempSpriteHandler;
}

int32 SaveLoad_Playtoons::GameHandler::getSize() {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return _slotFile->tallyUpFiles(varSize, kPropsSize + kIndexSize);
}

bool SaveLoad_Playtoons::GameHandler::load(int16 dataVar, int32 size, int32 offset) {
	uint32 varSize;

	if (size < 0) {
		// Load a temporary sprite
		debugC(2, kDebugSaveLoad, "Loading temporary sprite %d at pos %d", size, offset);
		_tempSpriteHandler->load(dataVar, size, offset);
		return true;
	}

	varSize = SaveHandler::getVarSize(_vm);
	if (varSize == 0)
		return false;

	if (size == 0) {
		// Indicator to load all variables
		dataVar = 0;
		size = varSize;
	}

	if (((uint32) offset) < kPropsSize) {
		// Properties

		if (((uint32) (offset + size)) > kPropsSize) {
			warning("Wrong index size (%d, %d)", size, offset);
			return false;
		}

		_vm->_inter->_variables->copyFrom(dataVar, _props + offset, size);

	} else if (((uint32) offset) < kPropsSize + kIndexSize) {
		// Save index

		if (((uint32) size) != kIndexSize) {
			warning("Wrong index size (%d, %d)", size, offset);
			return false;
		}

		buildIndex(_vm->_inter->_variables->getAddressOff8(dataVar));

	} else {
		// Save slot, whole variable block

		uint32 slot = _slotFile->getSlot(offset);
		int slotRem = _slotFile->getSlotRemainder(offset);

		debugC(2, kDebugSaveLoad, "Loading from slot %d", slot);

		if ((slot >= kSlotCount) || (slotRem != 0) ||
		    (dataVar != 0) || (((uint32) size) != varSize)) {

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
		SavePartVars vars(_vm, varSize);

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
		if (!vars.writeInto(0, 0, varSize)) {
			delete reader;
			return false;
		}

		delete reader;
	}

	return true;
}

bool SaveLoad_Playtoons::GameHandler::save(int16 dataVar, int32 size, int32 offset) {
	uint32 varSize;

	if (size < 0) {
		// Save a temporary sprite
		debugC(2, kDebugSaveLoad, "Saving temporary sprite %d at pos %d", size, offset);
		_tempSpriteHandler->save(dataVar, size, offset);
		return true;
	}

	varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return false;

	if (size == 0) {
		// Indicator to save all variables
		dataVar = 0;
		size = varSize;
	}

	if (((uint32) offset) < kPropsSize) {
		// Properties

		if (((uint32) (offset + size)) > kPropsSize) {
			warning("Wrong index size (%d, %d)", size, offset);
			return false;
		}

		_vm->_inter->_variables->copyTo(dataVar, _props + offset, size);

	}  else if (((uint32) offset) < kPropsSize + kIndexSize) {
		// Save index

		if (((uint32) size) != kIndexSize) {
			warning("Wrong index size (%d, %d)", size, offset);
			return false;
		}

		// Just copy the index into our buffer
		_vm->_inter->_variables->copyTo(dataVar, _index, kIndexSize);

	} else {
		// Save slot, whole variable block

		uint32 slot = _slotFile->getSlot(offset);
		int slotRem = _slotFile->getSlotRemainder(offset);

		debugC(2, kDebugSaveLoad, "Saving to slot %d", slot);

		if ((slot >= kSlotCount) || (slotRem != 0) ||
		    (dataVar != 0) || (((uint32) size) != varSize)) {

			warning("Invalid saving procedure (%d, %d, %d, %d, %d)",
					dataVar, size, offset, slot, slotRem);
			return false;
		}

		Common::String slotFile = _slotFile->build(slot);

		SaveWriter writer(2, slot, slotFile);
		SavePartInfo info(kSlotNameLength, (uint32) _vm->getGameType(), 0,
				_vm->getEndianness(), varSize);
		SavePartVars vars(_vm, varSize);

		// Write the description
		info.setDesc(_index + (slot * kSlotNameLength), kSlotNameLength);
		// Write all variables
		if (!vars.readFrom(0, 0, varSize))
			return false;

		if (!writer.writePart(0, &info))
			return false;
		if (!writer.writePart(1, &vars))
			return false;
	}

	return true;
}

void SaveLoad_Playtoons::GameHandler::buildIndex(byte *buffer) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return;

	SavePartInfo info(kSlotNameLength, (uint32) _vm->getGameType(),
			0, _vm->getEndianness(), varSize);

	_slotFile->buildIndex(buffer, info, 0);
}


SaveLoad_Playtoons::SaveLoad_Playtoons(GobEngine *vm, const char *targetName) :
		SaveLoad(vm) {

	_gameHandler = new GameHandler(vm, targetName);

	_saveFiles[0].handler = _gameHandler;
}

SaveLoad_Playtoons::~SaveLoad_Playtoons() {
	delete _gameHandler;
}

const SaveLoad_Playtoons::SaveFile *SaveLoad_Playtoons::getSaveFile(const char *fileName) const {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveLoad_Playtoons::SaveFile *SaveLoad_Playtoons::getSaveFile(const char *fileName) {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveHandler *SaveLoad_Playtoons::getHandler(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->handler;

	return 0;
}

const char *SaveLoad_Playtoons::getDescription(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->description;

	return 0;
}

SaveLoad::SaveMode SaveLoad_Playtoons::getSaveMode(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->mode;

	return kSaveModeNone;
}

} // End of namespace Gob
