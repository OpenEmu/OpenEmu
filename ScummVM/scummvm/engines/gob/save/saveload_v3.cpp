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

SaveLoad_v3::SaveFile SaveLoad_v3::_saveFiles[] = {
	{    "cat.inf", kSaveModeSave  , 0, "savegame"},
	{    "ima.inf", kSaveModeSave  , 0, "screenshot"},
	{  "intro.$$$", kSaveModeSave  , 0, "temporary sprite"},
	{   "bloc.inf", kSaveModeSave  , 0, "notes"},
	{   "prot",     kSaveModeIgnore, 0, 0},
	{ "config",     kSaveModeIgnore, 0, 0}
};


SaveLoad_v3::GameHandler::File::File(GobEngine *vm, const char *base) :
	SlotFileIndexed(vm, SaveLoad_v3::kSlotCount, base, "s") {
}

SaveLoad_v3::GameHandler::File::File(const File &file) :
	SlotFileIndexed(file._vm, file._slotCount, file._base, file._ext) {
}

SaveLoad_v3::GameHandler::File::~File() {
}

int SaveLoad_v3::GameHandler::File::getSlot(int32 offset) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return ((offset - (kPropsSize + kIndexSize)) / varSize);
}

int SaveLoad_v3::GameHandler::File::getSlotRemainder(int32 offset) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return ((offset - (kPropsSize + kIndexSize)) % varSize);
}


SaveLoad_v3::GameHandler::GameHandler(GobEngine *vm, const char *target,
		bool usesScreenshots) : SaveHandler(vm) {

	_slotFile = new File(vm, target);

	_usesScreenshots = usesScreenshots;

	_firstSize = true;
	memset(_props, 0, kPropsSize);
	memset(_index, 0, kIndexSize);
	_hasIndex = false;

	_writer = 0;
	_reader = 0;
}

SaveLoad_v3::GameHandler::~GameHandler() {
	delete _slotFile;
	delete _reader;
	delete _writer;
}

int32 SaveLoad_v3::GameHandler::getSize() {
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

bool SaveLoad_v3::GameHandler::load(int16 dataVar, int32 size, int32 offset) {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return false;

	if (size == 0) {
		// Indicator to load all variables
		dataVar = 0;
		size = varSize;
	}

	if (((uint32) offset) < kPropsSize) {
		// Global properties, like joker usage

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

	}

	return true;
}

bool SaveLoad_v3::GameHandler::save(int16 dataVar, int32 size, int32 offset) {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return false;

	if (size == 0) {
		// Indicator to save all variables
		dataVar = 0;
		size = varSize;
	}

	if (((uint32) offset) < kPropsSize) {
		// Global properties, like joker usage

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

	}

	return true;
}

bool SaveLoad_v3::GameHandler::saveScreenshot(int slot,
		const SavePartSprite *screenshot) {

	if (!createWriter(slot))
		return false;

	return _writer->writePart(2, screenshot);
}

bool SaveLoad_v3::GameHandler::loadScreenshot(int slot,
		SavePartSprite *screenshot) {

	if (!createReader(slot))
		return false;

	return _reader->readPart(2, screenshot);
}

void SaveLoad_v3::GameHandler::buildIndex(byte *buffer) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return;

	SavePartInfo info(40, (uint32) _vm->getGameType(), 0, _vm->getEndianness(), varSize);

	SaveConverter_v3 converter(_vm);

	_slotFile->buildIndex(buffer, info, &converter);
}

bool SaveLoad_v3::GameHandler::createReader(int slot) {
	// If slot < 0, just check if a reader exists
	if (slot < 0)
		return (_reader != 0);

	if (!_reader || (_reader->getSlot() != ((uint32) slot))) {
		Common::String slotFile = _slotFile->build(slot);

		if (slotFile.empty())
			return false;

		delete _reader;

		SaveConverter_v3 converter(_vm, slotFile);
		if (converter.isOldSave()) {
			// Old save, plug the converter in
			if (!converter.load()) {
				return false;
			}

			_reader = new SaveReader(_usesScreenshots ? 3 : 2, slot, converter);

		} else
			_reader = new SaveReader(_usesScreenshots ? 3 : 2, slot, slotFile);

		if (!_reader->load()) {
			delete _reader;
			_reader = 0;
			return false;
		}
	}

	return true;
}

bool SaveLoad_v3::GameHandler::createWriter(int slot) {
	// If slot < 0, just check if a writer exists
	if (slot < 0)
		return (_writer != 0);

	if (!_writer || (_writer->getSlot() != ((uint32) slot))) {
		Common::String slotFile = _slotFile->build(slot);

		if (slotFile.empty())
			return false;

		delete _writer;
		_writer = new SaveWriter(_usesScreenshots ? 3 : 2, slot, slotFile);
	}

	return true;
}


SaveLoad_v3::ScreenshotHandler::File::File(const SaveLoad_v3::GameHandler::File &file,
		uint32 shotSize, uint32 shotIndexSize) : SaveLoad_v3::GameHandler::File(file) {

	_shotSize = shotSize;
	_shotIndexSize = shotIndexSize;
}

SaveLoad_v3::ScreenshotHandler::File::~File() {
}

int SaveLoad_v3::ScreenshotHandler::File::getSlot(int32 offset) const {
	return ((offset - _shotIndexSize) / _shotSize);
}

int SaveLoad_v3::ScreenshotHandler::File::getSlotRemainder(int32 offset) const {
	return ((offset - _shotIndexSize) % _shotSize);
}

void SaveLoad_v3::ScreenshotHandler::File::buildScreenshotIndex(byte *buffer) const {
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in;

	for (uint32 i = 0; i < _slotCount; i++, buffer++) {
		Common::String slotFile = build(i);

		if (!slotFile.empty() && ((in = saveMan->openForLoading(slotFile)))) {
			delete in;
			*buffer = 1;
		} else
			*buffer = 0;
	}
}


SaveLoad_v3::ScreenshotHandler::ScreenshotHandler(GobEngine *vm,
		GameHandler *gameHandler, ScreenshotType sShotType) : TempSpriteHandler(vm) {

	assert(gameHandler);

	_gameHandler = gameHandler;
	_sShotType   = sShotType;

	_shotSize =      (_sShotType == kScreenshotTypeLost) ? 4768 : 19968;
	_shotIndexSize = (_sShotType == kScreenshotTypeLost) ?   50 :    80;

	_file = new File(*_gameHandler->_slotFile, _shotSize, _shotIndexSize);

	memset(_index, 0, 80);
}

SaveLoad_v3::ScreenshotHandler::~ScreenshotHandler() {
	delete _file;
}

int32 SaveLoad_v3::ScreenshotHandler::getSize() {
	return _file->tallyUpFiles(_shotSize, _shotIndexSize);
}

bool SaveLoad_v3::ScreenshotHandler::load(int16 dataVar, int32 size, int32 offset) {
	if (offset < _shotIndexSize) {
		// Screenshot index list

		if ((size + offset) > _shotIndexSize) {
			warning("Wrong screenshot index offset (%d, %d)", size, offset);
			return false;
		}

		if (_sShotType == kScreenshotTypeGob3) {
			// Create/Fake the index
			_file->buildScreenshotIndex(_index + 40);
			// The last 10 bytes are 0
			memset(_index + 70, 0, 10);
		} else if (_sShotType == kScreenshotTypeLost) {
			// Create/Fake the index
			_file->buildScreenshotIndex(_index);
			// The last byte is 0
			_index[30] = 0;
		}

		_vm->_inter->_variables->copyFrom(dataVar, _index + offset, size);

	} else {
		// Screenshot

		uint32 slot = _file->getSlot(offset);
		int slotRem = _file->getSlotRemainder(offset);

		if ((slot >= kSlotCount) || (slotRem != 0))
			return false;

		if (!TempSpriteHandler::createFromSprite(dataVar, size, offset))
			return false;

		if (!_gameHandler->loadScreenshot(slot, _sprite))
			return false;

		if (!TempSpriteHandler::load(dataVar, size, offset))
			return false;
	}

	return true;
}

bool SaveLoad_v3::ScreenshotHandler::save(int16 dataVar, int32 size, int32 offset) {
	if (offset < _shotIndexSize) {
		// Screenshot index list

		if ((size + offset) > _shotIndexSize) {
			warning("Wrong screenshot index offset (%d, %d)", size, offset);
			return false;
		}

		_vm->_inter->_variables->copyTo(dataVar, _index + offset, size);

	} else {
		// Screenshot

		if (!TempSpriteHandler::save(dataVar, size, offset))
			return false;

		uint32 slot = _file->getSlot(offset);
		int slotRem = _file->getSlotRemainder(offset);

		if ((slot >= kSlotCount) || (slotRem != 0))
			return false;

		return _gameHandler->saveScreenshot(slot, _sprite);
	}

	return true;
}


SaveLoad_v3::SaveLoad_v3(GobEngine *vm, const char *targetName, ScreenshotType sShotType) :
		SaveLoad(vm) {

	_sShotType = sShotType;

	// The Amiga version doesn't use screenshots
	if (_vm->getPlatform() == Common::kPlatformAmiga) {
		_gameHandler = new GameHandler(vm, targetName, false);
		_screenshotHandler = 0;
	} else {
		_gameHandler = new GameHandler(vm, targetName, true);
		_screenshotHandler = new ScreenshotHandler(vm, _gameHandler, sShotType);
	}

	_tempSpriteHandler = new TempSpriteHandler(vm);
	_notesHandler = new NotesHandler(2560, vm, targetName);

	_saveFiles[0].handler = _gameHandler;
	_saveFiles[1].handler = _screenshotHandler;
	_saveFiles[2].handler = _tempSpriteHandler;
	_saveFiles[3].handler = _notesHandler;
}

SaveLoad_v3::~SaveLoad_v3() {
	delete _screenshotHandler;
	delete _gameHandler;
	delete _notesHandler;
	delete _tempSpriteHandler;
}

const SaveLoad_v3::SaveFile *SaveLoad_v3::getSaveFile(const char *fileName) const {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveLoad_v3::SaveFile *SaveLoad_v3::getSaveFile(const char *fileName) {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveHandler *SaveLoad_v3::getHandler(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->handler;

	return 0;
}

const char *SaveLoad_v3::getDescription(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->description;

	return 0;
}

SaveLoad::SaveMode SaveLoad_v3::getSaveMode(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->mode;

	return kSaveModeNone;
}

} // End of namespace Gob
