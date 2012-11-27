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

SaveLoad_Geisha::SaveFile SaveLoad_Geisha::_saveFiles[] = {
	{"save.inf", kSaveModeSave, 0, "savegame"}
};


SaveLoad_Geisha::GameHandler::File::File(GobEngine *vm, const Common::String &base) :
	SlotFileIndexed(vm, SaveLoad_Geisha::kSlotCount, base, "s") {

}

SaveLoad_Geisha::GameHandler::File::~File() {
}

int SaveLoad_Geisha::GameHandler::File::getSlot(int32 offset) const {
	return 0;
}

int SaveLoad_Geisha::GameHandler::File::getSlotRemainder(int32 offset) const {
	return 0;
}


SaveLoad_Geisha::GameHandler::GameHandler(GobEngine *vm, const Common::String &target) :
	SaveHandler(vm), _file(vm, target) {

}

SaveLoad_Geisha::GameHandler::~GameHandler() {
}

int32 SaveLoad_Geisha::GameHandler::getSize() {
	if (_file.getSlotMax() == 0)
		return -1;

	return SaveLoad_Geisha::kSaveFileSize;
}

bool SaveLoad_Geisha::GameHandler::load(int16 dataVar, int32 size, int32 offset) {
	if ((size != 0) || (offset != 0)) {
		warning("Invalid loading procedure: %d, %d, %d", dataVar, size, offset);
		return false;
	}

	memset(_vm->_inter->_variables->getAddressOff8(dataVar), 0, SaveLoad_Geisha::kSaveFileSize);

	for (uint32 slot = 0; slot < SaveLoad_Geisha::kSlotCount;
	     slot++, dataVar += SaveLoad_Geisha::kSlotSize) {

		if (!_file.exists(slot))
			continue;

		Common::String slotFile = _file.build(slot);
		if (slotFile.empty())
			return false;

		SaveReader reader(2, slot, slotFile);
		if (!reader.load()) {
			warning("Save slot %d contains corrupted save", slot);
			continue;
		}

		SavePartInfo info(20, (uint32) _vm->getGameType(), 0,
				_vm->getEndianness(), _vm->_inter->_variables->getSize());
		SavePartVars vars(_vm, SaveLoad_Geisha::kSlotSize);

		if (!reader.readPart(0, &info) || !reader.readPart(1, &vars)) {
			warning("Save slot %d contains corrupted save", slot);
			continue;
		}

			if (!vars.writeInto(dataVar, 0, SaveLoad_Geisha::kSlotSize)) {
			warning("Save slot %d contains corrupted save", slot);
			continue;
		}
	}

	return true;
}

bool SaveLoad_Geisha::GameHandler::save(int16 dataVar, int32 size, int32 offset) {
	if (((uint32)size != SaveLoad_Geisha::kSaveFileSize) || (offset != 0)) {
		warning("Invalid saving procedure: %d, %d, %d", dataVar, size, offset);
		return false;
	}

	for (uint32 slot = 0; slot < SaveLoad_Geisha::kSlotCount;
	     slot++, dataVar += SaveLoad_Geisha::kSlotSize) {

		const byte *slotData = _vm->_inter->_variables->getAddressOff8(dataVar);

		// Check of the slot's data is empty
		bool empty = true;
		for (uint32 j = 0; j < SaveLoad_Geisha::kSlotSize; j++) {
			if (slotData[j] != 0) {
				empty = false;
				break;
			}
		}

		// Don't save empty slots
		if (empty)
			continue;

		Common::String slotFile = _file.build(slot);
		if (slotFile.empty())
			return false;

		SaveWriter writer(2, slot, slotFile);

		SavePartInfo info(20, (uint32) _vm->getGameType(), 0,
				_vm->getEndianness(), _vm->_inter->_variables->getSize());
		SavePartVars vars(_vm, SaveLoad_Geisha::kSlotSize);

		info.setDesc(Common::String::format("Geisha, slot %d", slot).c_str());
		if (!vars.readFrom(dataVar, 0, SaveLoad_Geisha::kSlotSize))
			return false;

		if (!writer.writePart(0, &info))
			return false;
		if (!writer.writePart(1, &vars))
			return false;
	}

	return true;
}


SaveLoad_Geisha::SaveLoad_Geisha(GobEngine *vm, const char *targetName) :
		SaveLoad(vm) {

	_saveFiles[0].handler = new GameHandler(vm, targetName);
}

SaveLoad_Geisha::~SaveLoad_Geisha() {
	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		delete _saveFiles[i].handler;
}

const SaveLoad_Geisha::SaveFile *SaveLoad_Geisha::getSaveFile(const char *fileName) const {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveLoad_Geisha::SaveFile *SaveLoad_Geisha::getSaveFile(const char *fileName) {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveHandler *SaveLoad_Geisha::getHandler(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->handler;

	return 0;
}

const char *SaveLoad_Geisha::getDescription(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->description;

	return 0;
}

SaveLoad::SaveMode SaveLoad_Geisha::getSaveMode(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->mode;

	return kSaveModeNone;
}

} // End of namespace Gob
