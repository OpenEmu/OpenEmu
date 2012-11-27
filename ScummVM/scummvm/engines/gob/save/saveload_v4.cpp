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

SaveLoad_v4::SaveFile SaveLoad_v4::_saveFiles[] = {
	{   "cat.inf", kSaveModeSave, 0, "savegame"},
	{  "save.tmp", kSaveModeSave, 0, "current screen properties"},
	{ "save0.tmp", kSaveModeSave, 0, "savegame screen properties"}, // Slot 0
	{ "save1.tmp", kSaveModeSave, 0, "savegame screen properties"}, // Slot 1
	{ "save2.tmp", kSaveModeSave, 0, "savegame screen properties"}, // Slot 2
	{ "save3.tmp", kSaveModeSave, 0, "savegame screen properties"}, // Slot 3
	{ "save4.tmp", kSaveModeSave, 0, "savegame screen properties"}, // Slot 4
	{ "save5.tmp", kSaveModeSave, 0, "savegame screen properties"}, // Slot 5
	{ "save6.tmp", kSaveModeSave, 0, "savegame screen properties"}, // Slot 6
	{ "save7.tmp", kSaveModeSave, 0, "savegame screen properties"}, // Slot 7
	{ "save8.tmp", kSaveModeSave, 0, "savegame screen properties"}, // Slot 8
	{ "save9.tmp", kSaveModeSave, 0, "savegame screen properties"}  // Slot 9
};


SaveLoad_v4::GameHandler::File::File(GobEngine *vm, const char *base) :
	SlotFileIndexed(vm, SaveLoad_v4::kSlotCount, base, "s") {
}

SaveLoad_v4::GameHandler::File::File(const File &file) :
	SlotFileIndexed(file._vm, file._slotCount, file._base, file._ext) {
}

SaveLoad_v4::GameHandler::File::~File() {
}

int SaveLoad_v4::GameHandler::File::getSlot(int32 offset) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return ((offset - (kPropsSize + kIndexSize)) / varSize);
}

int SaveLoad_v4::GameHandler::File::getSlotRemainder(int32 offset) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return ((offset - (kPropsSize + kIndexSize)) % varSize);
}


SaveLoad_v4::GameHandler::GameHandler(GobEngine *vm, const char *target) : SaveHandler(vm) {
	_firstSize = true;
	memset(_props, 0, kPropsSize);
	memset(_index, 0, kIndexSize);
	_hasIndex = false;

	_slotFile = new File(vm, target);

	_lastSlot = -1;

	_writer = 0;
	_reader = 0;
}

SaveLoad_v4::GameHandler::~GameHandler() {
	delete _slotFile;
	delete _reader;
	delete _writer;
}

int SaveLoad_v4::GameHandler::getLastSlot() const {
	return _lastSlot;
}

int32 SaveLoad_v4::GameHandler::getSize() {
	// Fake an empty save file for the very first query, to get clear properties
	if (_firstSize) {
		_firstSize = false;
		return -1;
	}

	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return _slotFile->tallyUpFiles(varSize, kPropsSize + kIndexSize);
}

bool SaveLoad_v4::GameHandler::load(int16 dataVar, int32 size, int32 offset) {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return false;

	if (size == 0) {
		// Indicator to load all variables
		dataVar = 0;
		size = varSize;
	}

	if (((uint32) offset) < kPropsSize) {
		// Global properties

		debugC(3, kDebugSaveLoad, "Loading global properties");

		if (((uint32) (offset + size)) > kPropsSize) {
			warning("Wrong global properties list size (%d, %d)", size, offset);
			return false;
		}

		_vm->_inter->_variables->copyFrom(dataVar, _props + offset, size);

	} else if (((uint32) offset) == kPropsSize) {
		// Save index

		if (((uint32) size) != kIndexSize) {
			warning("Requested index has wrong size (%d)", size);
			return false;
		}

		// Create/Fake the index
		buildIndex(_vm->_inter->_variables->getAddressOff8(dataVar));

	} else {
		// Save slot, whole variable block

		uint32 slot = _slotFile->getSlot(offset);
		int slotRem = _slotFile->getSlotRemainder(offset);

		debugC(2, kDebugSaveLoad, "Loading from slot %d", slot);

		if ((slot >= kSlotCount) || (slotRem != 0) ||
		    (dataVar != 0) || (((uint32) size) != varSize)) {

			warning("Invalid saving procedure (%d, %d, %d, %d, %d)",
					dataVar, size, offset, slot, slotRem);
			return false;
		}

		_hasIndex = false;

		if (!createReader(slot))
			return false;

		SavePartInfo info(kSlotNameLength, (uint32) _vm->getGameType(), 0,
				_vm->getEndianness(), varSize);
		SavePartVars vars(_vm, varSize);

		if (!_reader->readPart(0, &info))
			return false;
		if (!_reader->readPart(1, &vars))
			return false;

		// Get all variables
		if (!vars.writeInto(0, 0, varSize))
			return false;

		_lastSlot = slot;
	}

	return true;
}

bool SaveLoad_v4::GameHandler::save(int16 dataVar, int32 size, int32 offset) {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return false;

	if (size == 0) {
		// Indicator to load all variables
		dataVar = 0;
		size = varSize;
	}

	if (((uint32) offset) < kPropsSize) {
		// Global properties

		debugC(3, kDebugSaveLoad, "Saving global properties");

		if (((uint32) (offset + size)) > kPropsSize) {
			warning("Wrong global properties list size (%d, %d)", size, offset);
			return false;
		}

		_vm->_inter->_variables->copyTo(dataVar, _props + offset, size);

	} else if (((uint32) offset) == kPropsSize) {
		// Save index

		if (((uint32) size) != kIndexSize) {
			warning("Requested index has wrong size (%d)", size);
			return false;
		}

		// Just copy the index into our buffer
		_vm->_inter->_variables->copyTo(dataVar, _index, kIndexSize);
		_hasIndex = true;

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

		// An index is needed for the save slot description
		if (!_hasIndex) {
			warning("No index written yet");
			return false;
		}

		_hasIndex = false;

		if (!createWriter(slot))
			return false;

		SavePartInfo info(kSlotNameLength, (uint32) _vm->getGameType(), 0,
				_vm->getEndianness(), varSize);
		SavePartVars vars(_vm, varSize);

		// Write the description
		info.setDesc(_index + (slot * kSlotNameLength), kSlotNameLength);
		// Write all variables
		if (!vars.readFrom(0, 0, varSize))
			return false;

		if (!_writer->writePart(0, &info))
			return false;
		if (!_writer->writePart(1, &vars))
			return false;

		_lastSlot = slot;
	}

	return true;
}

bool SaveLoad_v4::GameHandler::saveScreenProps(int slot, const byte *props) {
	if (!createWriter(slot))
		return false;

	SavePartMem mem(256000);

	if (!mem.readFrom(props, 0, 256000))
		return false;

	return _writer->writePart(2, &mem);
}

bool SaveLoad_v4::GameHandler::loadScreenProps(int slot, byte *props) {
	if (!createReader(slot))
		return false;

	SavePartMem mem(256000);

	if (!_reader->readPart(2, &mem))
		return false;

	if (!mem.writeInto(props, 0, 256000))
		return false;

	return true;
}

void SaveLoad_v4::GameHandler::buildIndex(byte *buffer) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return;

	SavePartInfo info(kSlotNameLength, (uint32) _vm->getGameType(),
			0, _vm->getEndianness(), varSize);

	SaveConverter_v4 converter(_vm);

	_slotFile->buildIndex(buffer, info, &converter);

	// 400 bytes index + 800 bytes 0
	memset(buffer + 400, 0, 800);
}

bool SaveLoad_v4::GameHandler::createReader(int slot) {
	// If slot < 0, just check if a reader exists
	if (slot < 0)
		return (_reader != 0);

	if (!_reader || (_reader->getSlot() != ((uint32) slot))) {
		Common::String slotFile = _slotFile->build(slot);

		if (slotFile.empty())
			return false;

		delete _reader;

		SaveConverter_v4 converter(_vm, slotFile);
		if (converter.isOldSave()) {
			// Old save, plug the converter in
			if (!converter.load()) {
				return false;
			}

			_reader = new SaveReader(3, slot, converter);

		} else
			_reader = new SaveReader(3, slot, slotFile);

		if (!_reader->load()) {
			delete _reader;
			_reader = 0;
			return false;
		}
	}

	return true;
}

bool SaveLoad_v4::GameHandler::createWriter(int slot) {
	// If slot < 0, just check if a writer exists
	if (slot < 0)
		return (_writer != 0);

	if (!_writer || (_writer->getSlot() != ((uint32) slot))) {
		Common::String slotFile = _slotFile->build(slot);

		if (slotFile.empty())
			return false;

		delete _writer;
		_writer = new SaveWriter(3, slot, slotFile);
	}

	return true;
}


SaveLoad_v4::CurScreenPropsHandler::CurScreenPropsHandler(GobEngine *vm) :
	SaveHandler(vm) {

	_props = new byte[256000];
	memset(_props, 0, 256000);
}

SaveLoad_v4::CurScreenPropsHandler::~CurScreenPropsHandler() {
	delete[] _props;
}

int32 SaveLoad_v4::CurScreenPropsHandler::getSize() {
	return 256000;
}

bool SaveLoad_v4::CurScreenPropsHandler::load(int16 dataVar,
		int32 size, int32 offset) {

	// Using a sprite as a buffer
	if (size <= 0)
		return true;

	if ((offset < 0) || (size + offset) > 256000) {
		warning("Invalid size (%d) or offset (%d)", size, offset);
		return false;
	}

	debugC(3, kDebugSaveLoad, "Loading screen properties (%d, %d, %d)",
			dataVar, size, offset);

	_vm->_inter->_variables->copyFrom(dataVar, _props + offset, size);

	return true;
}

bool SaveLoad_v4::CurScreenPropsHandler::save(int16 dataVar,
		int32 size, int32 offset) {

	// Using a sprite as a buffer
	if (size <= 0)
		return true;

	if ((offset < 0) || (size + offset) > 256000) {
		warning("Invalid size (%d) or offset (%d)", size, offset);
		return false;
	}

	debugC(3, kDebugSaveLoad, "Saving screen properties (%d, %d, %d)",
			dataVar, size, offset);

	_vm->_inter->_variables->copyTo(dataVar, _props + offset, size);

	return true;
}


SaveLoad_v4::ScreenPropsHandler::File::File(const SaveLoad_v4::GameHandler::File &file,
		uint32 slot) : SaveLoad_v4::GameHandler::File(file) {

	_slot = slot;
}

SaveLoad_v4::ScreenPropsHandler::File::~File() {
}

int SaveLoad_v4::ScreenPropsHandler::File::getSlot(int32 offset) const {
	return _slot;
}

int SaveLoad_v4::ScreenPropsHandler::File::getSlotRemainder(int32 offset) const {
	return 0;
}


SaveLoad_v4::ScreenPropsHandler::ScreenPropsHandler(GobEngine *vm, uint32 slot,
		CurScreenPropsHandler *curProps, GameHandler *gameHandler) : SaveHandler(vm) {

	_slot = slot;
	_curProps = curProps;
	_gameHandler = gameHandler;

	_file = new File(*_gameHandler->_slotFile, _slot);
}

SaveLoad_v4::ScreenPropsHandler::~ScreenPropsHandler() {
	delete _file;
}

int32 SaveLoad_v4::ScreenPropsHandler::getSize() {
	if (_file->exists(_slot))
		return 256000;

	return 0;
}

bool SaveLoad_v4::ScreenPropsHandler::load(int16 dataVar, int32 size, int32 offset) {
	if (size != -5) {
		warning("Invalid saving procedure (%d, %d, %d)", dataVar, size, offset);
		return false;
	}

	int slot = _gameHandler->getLastSlot();
	if (slot == -1)
		slot = _file->getSlot(offset);

	return _gameHandler->loadScreenProps(slot, _curProps->_props);
}

bool SaveLoad_v4::ScreenPropsHandler::save(int16 dataVar, int32 size, int32 offset) {
	if (size != -5) {
		warning("Invalid saving procedure (%d, %d, %d)", dataVar, size, offset);
		return false;
	}

	int slot = _gameHandler->getLastSlot();
	if (slot == -1)
		slot = _file->getSlot(offset);

	return _gameHandler->saveScreenProps(slot, _curProps->_props);
}


SaveLoad_v4::SaveLoad_v4(GobEngine *vm, const char *targetName) :
		SaveLoad(vm) {

	_gameHandler = new GameHandler(vm, targetName);
	_curProps    = new CurScreenPropsHandler(vm);
	for (int i = 0; i < 10; i++)
		_props[i] = new ScreenPropsHandler(vm, i, _curProps, _gameHandler);

	_saveFiles[0].handler = _gameHandler;
	_saveFiles[1].handler = _curProps;
	for (int i = 0; i < 10; i++)
		_saveFiles[i + 2].handler = _props[i];
}

SaveLoad_v4::~SaveLoad_v4() {
	delete _gameHandler;
	delete _curProps;
	for (int i = 0; i < 10; i++)
		delete _props[i];
}

const SaveLoad_v4::SaveFile *SaveLoad_v4::getSaveFile(const char *fileName) const {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveLoad_v4::SaveFile *SaveLoad_v4::getSaveFile(const char *fileName) {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveHandler *SaveLoad_v4::getHandler(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->handler;

	return 0;
}

const char *SaveLoad_v4::getDescription(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->description;

	return 0;
}

SaveLoad::SaveMode SaveLoad_v4::getSaveMode(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->mode;

	return kSaveModeNone;
}

} // End of namespace Gob
