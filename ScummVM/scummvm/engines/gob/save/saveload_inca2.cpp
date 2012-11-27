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
#include "gob/global.h"
#include "gob/inter.h"

namespace Gob {

SaveLoad_Inca2::SaveFile SaveLoad_Inca2::_saveFiles[] = {
	{"speak.inf", kSaveModeExists, 0, 0}, // Exists = speech enabled
	{"voice.inf", kSaveModeSave  , 0, 0}, // Contains the language of the voices
	{"intro.$$$", kSaveModeSave  , 0, "temporary sprite"},
	{  "cat.inf", kSaveModeSave  , 0, "savegame"},
	{  "ima.inf", kSaveModeSave  , 0, "screenshot"},
};


SaveLoad_Inca2::GameHandler::File::File(GobEngine *vm, const char *base) :
	SlotFileIndexed(vm, SaveLoad_Inca2::kSlotCount, base, "s") {
}

SaveLoad_Inca2::GameHandler::File::File(const File &file) :
	SlotFileIndexed(file._vm, file._slotCount, file._base, file._ext) {
}

SaveLoad_Inca2::GameHandler::File::~File() {
}

int SaveLoad_Inca2::GameHandler::File::getSlot(int32 offset) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return offset - kPropsSize;
}

int SaveLoad_Inca2::GameHandler::File::getSlotRemainder(int32 offset) const {
	return 0;
}


SaveLoad_Inca2::GameHandler::GameHandler(GobEngine *vm, const char *target) :
	SaveHandler(vm) {

	_slotFile = new File(vm, target);

	memset(_props, 0x00, kPropsSize);
	memset(_props, 0x20, 10);

	_props[43] = 0x01;
	_props[79] = 0x03;

	buildIndex();

	_writer = 0;
	_reader = 0;
}

SaveLoad_Inca2::GameHandler::~GameHandler() {
	delete _slotFile;
	delete _reader;
	delete _writer;
}

int32 SaveLoad_Inca2::GameHandler::getSize() {
	return _slotFile->tallyUpFiles(1, kPropsSize);
}

bool SaveLoad_Inca2::GameHandler::load(int16 dataVar, int32 size, int32 offset) {
	if (((uint32) offset) < kPropsSize) {
		// Global properties, like joker usage

		debugC(3, kDebugSaveLoad, "Loading global properties");

		if (((uint32) (offset + size)) > kPropsSize) {
			warning("Wrong global properties list size (%d, %d)", size, offset);
			return false;
		}

		if (((uint32) (offset + size)) >= kPropsSize)
			buildIndex();

		_vm->_inter->_variables->copyFrom(dataVar, _props + offset, size);

	} else {
		int32 slot = offset - kPropsSize;

		if ((size != 1) || (slot < 0) || ((uint32)slot >= kSlotCount)) {
			warning("Invalid loading procedure (%d, %d, %d)", dataVar, size, offset);
			return false;
		}

		WRITE_VARO_UINT8(dataVar, 0);
		if (!createReader(slot))
			return true;

		SavePartInfo info(0, (uint32) _vm->getGameType(), 0, _vm->getEndianness(), 1);
		SavePartVars vars(_vm, 1);

		if (!_reader->readPart(0, &info))
			return true;
		if (!_reader->readPart(1, &vars))
			return true;

		// Read the save point number
		if (!vars.writeInto(dataVar, 0, 1)) {
			WRITE_VARO_UINT8(dataVar, 0);
			return true;
		}
	}

	return true;
}

bool SaveLoad_Inca2::GameHandler::save(int16 dataVar, int32 size, int32 offset) {
	if (((uint32) offset) < kPropsSize) {
		// Global properties, like joker usage

		debugC(3, kDebugSaveLoad, "Saving global properties");

		if (((uint32) (offset + size)) > kPropsSize) {
			warning("Wrong global properties list size (%d, %d)", size, offset);
			return false;
		}

		_vm->_inter->_variables->copyTo(dataVar, _props + offset, size);

	} else {
		// Save point flag

		int32 slot = offset - kPropsSize;

		if ((size != 1) || (slot < 0) || ((uint32)slot >= kSlotCount)) {
			warning("Invalid saving procedure (%d, %d, %d)", dataVar, size, offset);
			return false;
		}

		if (!createWriter(slot))
			return false;

		SavePartInfo info(0, (uint32) _vm->getGameType(), 0, _vm->getEndianness(), 1);
		SavePartVars vars(_vm, 1);

		// Write the save point number
		if (!vars.readFrom(dataVar, 0, 1))
			return false;

		if (!_writer->writePart(0, &info))
			return false;
		if (!_writer->writePart(1, &vars))
			return false;
	}

	return true;
}

bool SaveLoad_Inca2::GameHandler::saveScreenshot(int slot,
		const SavePartSprite *screenshot) {

	if (!createWriter(slot))
		return false;

	return _writer->writePart(2, screenshot);
}

bool SaveLoad_Inca2::GameHandler::loadScreenshot(int slot,
		SavePartSprite *screenshot) {

	if (!createReader(slot))
		return false;

	return _reader->readPart(2, screenshot);
}

void SaveLoad_Inca2::GameHandler::buildIndex() {
	_props[499] = _slotFile->getSlotMax();
}

bool SaveLoad_Inca2::GameHandler::createReader(int slot) {
	// If slot < 0, just check if a reader exists
	if (slot < 0)
		return (_reader != 0);

	if (!_reader || (_reader->getSlot() != ((uint32) slot))) {
		Common::String slotFile = _slotFile->build(slot);

		if (slotFile.empty())
			return false;

		delete _reader;

		_reader = new SaveReader(3, slot, slotFile);

		if (!_reader->load()) {
			delete _reader;
			_reader = 0;
			return false;
		}
	}

	return true;
}

bool SaveLoad_Inca2::GameHandler::createWriter(int slot) {
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


SaveLoad_Inca2::ScreenshotHandler::File::File(const SaveLoad_Inca2::GameHandler::File &file) : SaveLoad_Inca2::GameHandler::File(file) {

}

SaveLoad_Inca2::ScreenshotHandler::File::~File() {
}

int SaveLoad_Inca2::ScreenshotHandler::File::getSlot(int32 offset) const {
	return (offset - 80) / 15168;
}

int SaveLoad_Inca2::ScreenshotHandler::File::getSlotRemainder(int32 offset) const {
	return (offset - 80) % 15168;
}

void SaveLoad_Inca2::ScreenshotHandler::File::buildScreenshotIndex(byte *buffer) const {
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in;

	for (uint32 i = 0; i < 40; i++, buffer++) {
		Common::String slotFile = build(i);

		if (!slotFile.empty() && ((in = saveMan->openForLoading(slotFile)))) {
			delete in;
			*buffer = 1;
		} else
			*buffer = 0;
	}
}


SaveLoad_Inca2::ScreenshotHandler::ScreenshotHandler(GobEngine *vm,
		GameHandler *gameHandler) : TempSpriteHandler(vm) {

	assert(gameHandler);

	_gameHandler = gameHandler;

	_file = new File(*_gameHandler->_slotFile);

	memset(_index, 0, 80);
}

SaveLoad_Inca2::ScreenshotHandler::~ScreenshotHandler() {
	delete _file;
}

int32 SaveLoad_Inca2::ScreenshotHandler::getSize() {
	return _file->tallyUpFiles(15168, 80);
}

bool SaveLoad_Inca2::ScreenshotHandler::load(int16 dataVar, int32 size, int32 offset) {
	if (offset < 80) {
		// Screenshot index list

		if ((size + offset) > 80) {
			warning("Wrong screenshot index offset (%d, %d)", size, offset);
			return false;
		}

		// Create/Fake the index
		_file->buildScreenshotIndex(_index + 40);

		_vm->_inter->_variables->copyFrom(dataVar, _index + offset, size);

	} else {
		// Screenshot

		uint32 slot = _file->getSlot(offset);
		int slotRem = _file->getSlotRemainder(offset);

		if ((slot >= kSlotCount) || (slotRem != 0)) {
			warning("Invalid screenshot loading procedure (%d, %d, %d, %d, %d)",
					dataVar, size, offset, slot, slotRem);
			return false;
		}

		if (!TempSpriteHandler::createFromSprite(dataVar, size, offset))
			return false;

		if (!_gameHandler->loadScreenshot(slot, _sprite))
			return false;

		if (!TempSpriteHandler::load(dataVar, size, offset))
			return false;
	}

	return true;
}

bool SaveLoad_Inca2::ScreenshotHandler::save(int16 dataVar, int32 size, int32 offset) {
	if (offset < 80) {
		// Index, we'll ignore that

	} else {
		// Screenshot

		uint32 slot = _file->getSlot(offset);
		int slotRem = _file->getSlotRemainder(offset);

		if ((slot >= kSlotCount) || (slotRem != 0)) {
			warning("Invalid screenshot saving procedure (%d, %d, %d, %d, %d)",
					dataVar, size, offset, slot, slotRem);
			return false;
		}

		if (!TempSpriteHandler::save(dataVar, size, offset))
			return false;

		return _gameHandler->saveScreenshot(slot, _sprite);
	}

	return true;
}


SaveLoad_Inca2::VoiceHandler::VoiceHandler(GobEngine *vm) : SaveHandler(vm) {
}

SaveLoad_Inca2::VoiceHandler::~VoiceHandler() {
}

int32 SaveLoad_Inca2::VoiceHandler::getSize() {
	return 1;
}

bool SaveLoad_Inca2::VoiceHandler::load(int16 dataVar, int32 size, int32 offset) {
	if ((size != 1) || (offset != 0)) {
		warning("Invalid voice language loading?!? (%d, %d, %d)", dataVar, size, offset);
		return false;
	}

	WRITE_VARO_UINT8(dataVar, _vm->_global->_language);
	return true;
}

bool SaveLoad_Inca2::VoiceHandler::save(int16 dataVar, int32 size, int32 offset) {
	return false;
}


SaveLoad_Inca2::SaveLoad_Inca2(GobEngine *vm, const char *targetName) : SaveLoad(vm) {
	_voiceHandler      = new VoiceHandler(vm);
	_tempSpriteHandler = new TempSpriteHandler(vm);
	_gameHandler       = new GameHandler(vm, targetName);
	_screenshotHandler = new ScreenshotHandler(vm, _gameHandler);

	_saveFiles[1].handler = _voiceHandler;
	_saveFiles[2].handler = _tempSpriteHandler;
	_saveFiles[3].handler = _gameHandler;
	_saveFiles[4].handler = _screenshotHandler;
}

SaveLoad_Inca2::~SaveLoad_Inca2() {
	delete _voiceHandler;
}

const SaveLoad_Inca2::SaveFile *SaveLoad_Inca2::getSaveFile(const char *fileName) const {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveLoad_Inca2::SaveFile *SaveLoad_Inca2::getSaveFile(const char *fileName) {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveHandler *SaveLoad_Inca2::getHandler(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->handler;

	return 0;
}

const char *SaveLoad_Inca2::getDescription(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->description;

	return 0;
}

SaveLoad::SaveMode SaveLoad_Inca2::getSaveMode(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->mode;

	return kSaveModeNone;
}

} // End of namespace Gob
