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

SaveLoad_v6::SaveFile SaveLoad_v6::_saveFiles[] = {
	{    "cat.inf", kSaveModeSave,   0, "savegame"   }, // Save file
	{  "cata1.inf", kSaveModeSave,   0, "autosave"   }, // Autosave file
	{    "mdo.def", kSaveModeExists, 0, 0            },
	{  "no_cd.txt", kSaveModeExists, 0, 0            },
	{   "vide.inf", kSaveModeIgnore, 0, 0            },
	{"fenetre.txt", kSaveModeIgnore, 0, 0            },
	{  "music.txt", kSaveModeIgnore, 0, 0            },
	{  "cata2.inf", kSaveModeSave,   0, "temp save"  },
	{  "cata3.inf", kSaveModeSave,   0, "temp save"  },
	{  "cata2.000", kSaveModeSave,   0, "extra save" }, // Slot 00
	{  "cata2.001", kSaveModeSave,   0, "extra save" }, // Slot 01
	{  "cata2.002", kSaveModeSave,   0, "extra save" }, // Slot 02
	{  "cata2.003", kSaveModeSave,   0, "extra save" }, // Slot 03
	{  "cata2.004", kSaveModeSave,   0, "extra save" }, // Slot 04
	{  "cata2.005", kSaveModeSave,   0, "extra save" }, // Slot 05
	{  "cata2.006", kSaveModeSave,   0, "extra save" }, // Slot 06
	{  "cata2.007", kSaveModeSave,   0, "extra save" }, // Slot 07
	{  "cata2.008", kSaveModeSave,   0, "extra save" }, // Slot 08
	{  "cata2.009", kSaveModeSave,   0, "extra save" }, // Slot 09
	{  "cata2.010", kSaveModeSave,   0, "extra save" }, // Slot 10
	{  "cata2.011", kSaveModeSave,   0, "extra save" }, // Slot 11
	{  "cata2.012", kSaveModeSave,   0, "extra save" }, // Slot 12
	{  "cata2.013", kSaveModeSave,   0, "extra save" }, // Slot 13
	{  "cata2.014", kSaveModeSave,   0, "extra save" }, // Slot 14
	{  "cata2.015", kSaveModeSave,   0, "extra save" }, // Slot 15
	{  "cata2.016", kSaveModeSave,   0, "extra save" }, // Slot 16
	{  "cata2.017", kSaveModeSave,   0, "extra save" }, // Slot 17
	{  "cata2.018", kSaveModeSave,   0, "extra save" }, // Slot 18
	{  "cata2.019", kSaveModeSave,   0, "extra save" }, // Slot 19
	{  "cata2.020", kSaveModeSave,   0, "extra save" }, // Slot 20
	{  "cata2.021", kSaveModeSave,   0, "extra save" }, // Slot 21
	{  "cata2.022", kSaveModeSave,   0, "extra save" }, // Slot 22
	{  "cata2.023", kSaveModeSave,   0, "extra save" }, // Slot 23
	{  "cata2.024", kSaveModeSave,   0, "extra save" }, // Slot 24
	{  "cata2.025", kSaveModeSave,   0, "extra save" }, // Slot 25
	{  "cata2.026", kSaveModeSave,   0, "extra save" }, // Slot 26
	{  "cata2.027", kSaveModeSave,   0, "extra save" }, // Slot 27
	{  "cata2.028", kSaveModeSave,   0, "extra save" }, // Slot 28
	{  "cata2.029", kSaveModeSave,   0, "extra save" }, // Slot 29
	{  "cata2.030", kSaveModeSave,   0, "extra save" }, // Slot 30
	{  "cata2.031", kSaveModeSave,   0, "extra save" }, // Slot 31
	{  "cata2.032", kSaveModeSave,   0, "extra save" }, // Slot 32
	{  "cata2.033", kSaveModeSave,   0, "extra save" }, // Slot 33
	{  "cata2.034", kSaveModeSave,   0, "extra save" }, // Slot 34
	{  "cata2.035", kSaveModeSave,   0, "extra save" }, // Slot 35
	{  "cata2.036", kSaveModeSave,   0, "extra save" }, // Slot 36
	{  "cata2.037", kSaveModeSave,   0, "extra save" }, // Slot 37
	{  "cata2.038", kSaveModeSave,   0, "extra save" }, // Slot 38
	{  "cata2.039", kSaveModeSave,   0, "extra save" }, // Slot 39
	{  "cata2.040", kSaveModeSave,   0, "extra save" }, // Slot 40
	{  "cata2.041", kSaveModeSave,   0, "extra save" }, // Slot 41
	{  "cata2.042", kSaveModeSave,   0, "extra save" }, // Slot 42
	{  "cata2.043", kSaveModeSave,   0, "extra save" }, // Slot 43
	{  "cata2.044", kSaveModeSave,   0, "extra save" }, // Slot 44
	{  "cata2.045", kSaveModeSave,   0, "extra save" }, // Slot 45
	{  "cata2.046", kSaveModeSave,   0, "extra save" }, // Slot 46
	{  "cata2.047", kSaveModeSave,   0, "extra save" }, // Slot 47
	{  "cata2.048", kSaveModeSave,   0, "extra save" }, // Slot 48
	{  "cata2.049", kSaveModeSave,   0, "extra save" }, // Slot 49
	{  "cata2.050", kSaveModeSave,   0, "extra save" }, // Slot 50
	{  "cata2.051", kSaveModeSave,   0, "extra save" }, // Slot 51
	{  "cata2.052", kSaveModeSave,   0, "extra save" }, // Slot 52
	{  "cata2.053", kSaveModeSave,   0, "extra save" }, // Slot 53
	{  "cata2.054", kSaveModeSave,   0, "extra save" }, // Slot 54
	{  "cata2.055", kSaveModeSave,   0, "extra save" }, // Slot 55
	{  "cata2.056", kSaveModeSave,   0, "extra save" }, // Slot 56
	{  "cata2.057", kSaveModeSave,   0, "extra save" }, // Slot 57
	{  "cata2.058", kSaveModeSave,   0, "extra save" }, // Slot 58
	{  "cata2.059", kSaveModeSave,   0, "extra save" }, // Slot 59
	{  "cata3.000", kSaveModeSave,   0, "extra save" }, // Slot 00
	{  "cata3.001", kSaveModeSave,   0, "extra save" }, // Slot 01
	{  "cata3.002", kSaveModeSave,   0, "extra save" }, // Slot 02
	{  "cata3.003", kSaveModeSave,   0, "extra save" }, // Slot 03
	{  "cata3.004", kSaveModeSave,   0, "extra save" }, // Slot 04
	{  "cata3.005", kSaveModeSave,   0, "extra save" }, // Slot 05
	{  "cata3.006", kSaveModeSave,   0, "extra save" }, // Slot 06
	{  "cata3.007", kSaveModeSave,   0, "extra save" }, // Slot 07
	{  "cata3.008", kSaveModeSave,   0, "extra save" }, // Slot 08
	{  "cata3.009", kSaveModeSave,   0, "extra save" }, // Slot 09
	{  "cata3.010", kSaveModeSave,   0, "extra save" }, // Slot 10
	{  "cata3.011", kSaveModeSave,   0, "extra save" }, // Slot 11
	{  "cata3.012", kSaveModeSave,   0, "extra save" }, // Slot 12
	{  "cata3.013", kSaveModeSave,   0, "extra save" }, // Slot 13
	{  "cata3.014", kSaveModeSave,   0, "extra save" }, // Slot 14
	{  "cata3.015", kSaveModeSave,   0, "extra save" }, // Slot 15
	{  "cata3.016", kSaveModeSave,   0, "extra save" }, // Slot 16
	{  "cata3.017", kSaveModeSave,   0, "extra save" }, // Slot 17
	{  "cata3.018", kSaveModeSave,   0, "extra save" }, // Slot 18
	{  "cata3.019", kSaveModeSave,   0, "extra save" }, // Slot 19
	{  "cata3.020", kSaveModeSave,   0, "extra save" }, // Slot 20
	{  "cata3.021", kSaveModeSave,   0, "extra save" }, // Slot 21
	{  "cata3.022", kSaveModeSave,   0, "extra save" }, // Slot 22
	{  "cata3.023", kSaveModeSave,   0, "extra save" }, // Slot 23
	{  "cata3.024", kSaveModeSave,   0, "extra save" }, // Slot 24
	{  "cata3.025", kSaveModeSave,   0, "extra save" }, // Slot 25
	{  "cata3.026", kSaveModeSave,   0, "extra save" }, // Slot 26
	{  "cata3.027", kSaveModeSave,   0, "extra save" }, // Slot 27
	{  "cata3.028", kSaveModeSave,   0, "extra save" }, // Slot 28
	{  "cata3.029", kSaveModeSave,   0, "extra save" }, // Slot 29
	{  "cata3.030", kSaveModeSave,   0, "extra save" }, // Slot 30
	{  "cata3.031", kSaveModeSave,   0, "extra save" }, // Slot 31
	{  "cata3.032", kSaveModeSave,   0, "extra save" }, // Slot 32
	{  "cata3.033", kSaveModeSave,   0, "extra save" }, // Slot 33
	{  "cata3.034", kSaveModeSave,   0, "extra save" }, // Slot 34
	{  "cata3.035", kSaveModeSave,   0, "extra save" }, // Slot 35
	{  "cata3.036", kSaveModeSave,   0, "extra save" }, // Slot 36
	{  "cata3.037", kSaveModeSave,   0, "extra save" }, // Slot 37
	{  "cata3.038", kSaveModeSave,   0, "extra save" }, // Slot 38
	{  "cata3.039", kSaveModeSave,   0, "extra save" }, // Slot 39
	{  "cata3.040", kSaveModeSave,   0, "extra save" }, // Slot 40
	{  "cata3.041", kSaveModeSave,   0, "extra save" }, // Slot 41
	{  "cata3.042", kSaveModeSave,   0, "extra save" }, // Slot 42
	{  "cata3.043", kSaveModeSave,   0, "extra save" }, // Slot 43
	{  "cata3.044", kSaveModeSave,   0, "extra save" }, // Slot 44
	{  "cata3.045", kSaveModeSave,   0, "extra save" }, // Slot 45
	{  "cata3.046", kSaveModeSave,   0, "extra save" }, // Slot 46
	{  "cata3.047", kSaveModeSave,   0, "extra save" }, // Slot 47
	{  "cata3.048", kSaveModeSave,   0, "extra save" }, // Slot 48
	{  "cata3.049", kSaveModeSave,   0, "extra save" }, // Slot 49
	{  "cata3.050", kSaveModeSave,   0, "extra save" }, // Slot 50
	{  "cata3.051", kSaveModeSave,   0, "extra save" }, // Slot 51
	{  "cata3.052", kSaveModeSave,   0, "extra save" }, // Slot 52
	{  "cata3.053", kSaveModeSave,   0, "extra save" }, // Slot 53
	{  "cata3.054", kSaveModeSave,   0, "extra save" }, // Slot 54
	{  "cata3.055", kSaveModeSave,   0, "extra save" }, // Slot 55
	{  "cata3.056", kSaveModeSave,   0, "extra save" }, // Slot 56
	{  "cata3.057", kSaveModeSave,   0, "extra save" }, // Slot 57
	{  "cata3.058", kSaveModeSave,   0, "extra save" }, // Slot 58
	{  "cata3.059", kSaveModeSave,   0, "extra save" }, // Slot 59
	{  "intro.0xx", kSaveModeSave,   0, "temp sprite"}, // Autosave sprite
	{  "intro.000", kSaveModeSave,   0, "temp sprite"}, // Slot 00
	{  "intro.001", kSaveModeSave,   0, "temp sprite"}, // Slot 01
	{  "intro.002", kSaveModeSave,   0, "temp sprite"}, // Slot 02
	{  "intro.003", kSaveModeSave,   0, "temp sprite"}, // Slot 03
	{  "intro.004", kSaveModeSave,   0, "temp sprite"}, // Slot 04
	{  "intro.005", kSaveModeSave,   0, "temp sprite"}, // Slot 05
	{  "intro.006", kSaveModeSave,   0, "temp sprite"}, // Slot 06
	{  "intro.007", kSaveModeSave,   0, "temp sprite"}, // Slot 07
	{  "intro.008", kSaveModeSave,   0, "temp sprite"}, // Slot 08
	{  "intro.009", kSaveModeSave,   0, "temp sprite"}, // Slot 09
	{  "intro.010", kSaveModeSave,   0, "temp sprite"}, // Slot 10
	{  "intro.011", kSaveModeSave,   0, "temp sprite"}, // Slot 11
	{  "intro.012", kSaveModeSave,   0, "temp sprite"}, // Slot 12
	{  "intro.013", kSaveModeSave,   0, "temp sprite"}, // Slot 13
	{  "intro.014", kSaveModeSave,   0, "temp sprite"}, // Slot 14
	{  "intro.015", kSaveModeSave,   0, "temp sprite"}, // Slot 15
	{  "intro.016", kSaveModeSave,   0, "temp sprite"}, // Slot 16
	{  "intro.017", kSaveModeSave,   0, "temp sprite"}, // Slot 17
	{  "intro.018", kSaveModeSave,   0, "temp sprite"}, // Slot 18
	{  "intro.019", kSaveModeSave,   0, "temp sprite"}, // Slot 19
	{  "intro.020", kSaveModeSave,   0, "temp sprite"}, // Slot 20
	{  "intro.021", kSaveModeSave,   0, "temp sprite"}, // Slot 21
	{  "intro.022", kSaveModeSave,   0, "temp sprite"}, // Slot 22
	{  "intro.023", kSaveModeSave,   0, "temp sprite"}, // Slot 23
	{  "intro.024", kSaveModeSave,   0, "temp sprite"}, // Slot 24
	{  "intro.025", kSaveModeSave,   0, "temp sprite"}, // Slot 25
	{  "intro.026", kSaveModeSave,   0, "temp sprite"}, // Slot 26
	{  "intro.027", kSaveModeSave,   0, "temp sprite"}, // Slot 27
	{  "intro.028", kSaveModeSave,   0, "temp sprite"}, // Slot 28
	{  "intro.029", kSaveModeSave,   0, "temp sprite"}, // Slot 29
	{  "intro.030", kSaveModeSave,   0, "temp sprite"}, // Slot 30
	{  "intro.031", kSaveModeSave,   0, "temp sprite"}, // Slot 31
	{  "intro.032", kSaveModeSave,   0, "temp sprite"}, // Slot 32
	{  "intro.033", kSaveModeSave,   0, "temp sprite"}, // Slot 33
	{  "intro.034", kSaveModeSave,   0, "temp sprite"}, // Slot 34
	{  "intro.035", kSaveModeSave,   0, "temp sprite"}, // Slot 35
	{  "intro.036", kSaveModeSave,   0, "temp sprite"}, // Slot 36
	{  "intro.037", kSaveModeSave,   0, "temp sprite"}, // Slot 37
	{  "intro.038", kSaveModeSave,   0, "temp sprite"}, // Slot 38
	{  "intro.039", kSaveModeSave,   0, "temp sprite"}, // Slot 39
	{  "intro.040", kSaveModeSave,   0, "temp sprite"}, // Slot 40
	{  "intro.041", kSaveModeSave,   0, "temp sprite"}, // Slot 41
	{  "intro.042", kSaveModeSave,   0, "temp sprite"}, // Slot 42
	{  "intro.043", kSaveModeSave,   0, "temp sprite"}, // Slot 43
	{  "intro.044", kSaveModeSave,   0, "temp sprite"}, // Slot 44
	{  "intro.045", kSaveModeSave,   0, "temp sprite"}, // Slot 45
	{  "intro.046", kSaveModeSave,   0, "temp sprite"}, // Slot 46
	{  "intro.047", kSaveModeSave,   0, "temp sprite"}, // Slot 47
	{  "intro.048", kSaveModeSave,   0, "temp sprite"}, // Slot 48
	{  "intro.049", kSaveModeSave,   0, "temp sprite"}, // Slot 49
	{  "intro.050", kSaveModeSave,   0, "temp sprite"}, // Slot 50
	{  "intro.051", kSaveModeSave,   0, "temp sprite"}, // Slot 51
	{  "intro.052", kSaveModeSave,   0, "temp sprite"}, // Slot 52
	{  "intro.053", kSaveModeSave,   0, "temp sprite"}, // Slot 53
	{  "intro.054", kSaveModeSave,   0, "temp sprite"}, // Slot 54
	{  "intro.055", kSaveModeSave,   0, "temp sprite"}, // Slot 55
	{  "intro.056", kSaveModeSave,   0, "temp sprite"}, // Slot 56
	{  "intro.057", kSaveModeSave,   0, "temp sprite"}, // Slot 57
	{  "intro.058", kSaveModeSave,   0, "temp sprite"}, // Slot 58
	{  "intro.059", kSaveModeSave,   0, "temp sprite"}  // Slot 59
};


SaveLoad_v6::SpriteHandler::SpriteHandler(GobEngine *vm) : TempSpriteHandler(vm) {
}

SaveLoad_v6::SpriteHandler::~SpriteHandler() {
}

bool SaveLoad_v6::SpriteHandler::set(SaveReader *reader, uint32 part) {
	if (!TempSpriteHandler::create(624, 272, true))
		return false;

	return reader->readPart(part, _sprite);
}

bool SaveLoad_v6::SpriteHandler::get(SaveWriter *writer, uint32 part) {
	if (getSize() < 0)
		if (!TempSpriteHandler::create(624, 272, true))
			return false;

	return writer->writePart(part, _sprite);
}


SaveLoad_v6::GameHandler::File::File(GobEngine *vm, const char *base) :
	SlotFileIndexed(vm, SaveLoad_v6::kSlotCount, base, "s") {
}

SaveLoad_v6::GameHandler::File::~File() {
}

int SaveLoad_v6::GameHandler::File::getSlot(int32 offset) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return ((offset - (kPropsSize + kIndexSize)) / varSize);
}

int SaveLoad_v6::GameHandler::File::getSlotRemainder(int32 offset) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return ((offset - (kPropsSize + kIndexSize)) % varSize);
}


SaveLoad_v6::GameHandler::GameHandler(GobEngine *vm, const char *target,
		SpriteHandler &spriteHandler) : SaveHandler(vm), _spriteHandler(&spriteHandler),
		_reader(0), _writer(0), _hasExtra(false) {

	memset(_props, 0, kPropsSize);
	memset(_index, 0, kIndexSize);

	_slotFile = new File(vm, target);
}

SaveLoad_v6::GameHandler::~GameHandler() {
	delete _slotFile;

	delete _reader;
	delete _writer;
}

int32 SaveLoad_v6::GameHandler::getSize() {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return -1;

	return _slotFile->tallyUpFiles(varSize, kPropsSize + kIndexSize);
}

bool SaveLoad_v6::GameHandler::load(int16 dataVar, int32 size, int32 offset) {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return false;

	if (size == 0) {
		// Indicator to load all variables
		dataVar = 0;
		size = varSize;
	}

	if (((uint32) offset) < kPropsSize) {
		// Properties

		refreshProps();

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

		if (!createReader(slot))
			return false;

		SavePartInfo info(kSlotNameLength, (uint32) _vm->getGameType(), 0,
				_vm->getEndianness(), varSize);
		SavePartVars vars(_vm, varSize);

		if (!_reader->load())
			return false;

		if (!_reader->readPart(0, &info))
			return false;
		if (!_reader->readPart(1, &vars))
			return false;

		// Get all variables
		if (!vars.writeInto(0, 0, varSize))
			return false;

		if (!_spriteHandler->set(_reader, 4))
			return false;
	}

	return true;
}

bool SaveLoad_v6::GameHandler::save(int16 dataVar, int32 size, int32 offset) {
	uint32 varSize = SaveHandler::getVarSize(_vm);

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

		refreshProps();

		// If that screen doesn't save any extra temp saves, write a dummy
		if (_writer && (size == 40) && (offset == 0)) {
			if (!_hasExtra) {
				SavePartMem  mem(1);
				SavePartVars vars(_vm, varSize);

				uint8 extraSaveNumber = 0;
				if (!mem.readFrom(&extraSaveNumber, 0, 1))
					return false;
				if (!vars.readFrom(0, 0, varSize))
					return false;

				if (!_writer->writePart(2, &mem))
					return false;
				if (!_writer->writePart(3, &vars))
					return false;
			}
		}

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

		_hasExtra = false;

		uint32 slot = _slotFile->getSlot(offset);
		int slotRem = _slotFile->getSlotRemainder(offset);

		debugC(2, kDebugSaveLoad, "Saving to slot %d", slot);

		if ((slot >= kSlotCount) || (slotRem != 0) ||
		    (dataVar != 0) || (((uint32) size) != varSize)) {

			warning("Invalid saving procedure (%d, %d, %d, %d, %d)",
					dataVar, size, offset, slot, slotRem);
			return false;
		}

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

		if (!_spriteHandler->get(_writer, 4))
			return false;
	}

	return true;
}

uint8 SaveLoad_v6::GameHandler::getExtraID(int slot) {
	if (!_reader || (_reader->getSlot() != (uint32)slot))
		return 0;

	SavePartMem mem(1);
	if (!_reader->readPart(2, &mem))
		return 0;

	uint8 extraSaveNumber;
	if (!mem.writeInto(&extraSaveNumber, 0, 1))
		return 0;

	return extraSaveNumber;
}

bool SaveLoad_v6::GameHandler::loadExtra(int slot, uint8 id,
		int16 dataVar, int32 size, int32 offset) {

	if (!_reader || (_reader->getSlot() != (uint32)slot))
		return false;

	SavePartMem mem(1);
	if (!_reader->readPart(2, &mem))
		return false;

	uint8 extraSaveNumber;
	if (!mem.writeInto(&extraSaveNumber, 0, 1))
		return false;

	if (extraSaveNumber != id)
		return false;

	uint32 varSize = SaveHandler::getVarSize(_vm);

	SavePartVars vars(_vm, varSize);
	if (!_reader->readPart(3, &vars))
		return false;

	if (!vars.writeInto(0, 0, varSize))
		return false;

	return true;
}

bool SaveLoad_v6::GameHandler::saveExtra(int slot, uint8 id,
		int16 dataVar, int32 size, int32 offset) {

	if (!_writer || (_writer->getSlot() != (uint32)slot))
		return false;

	uint32 varSize = SaveHandler::getVarSize(_vm);

	SavePartMem  mem(1);
	SavePartVars vars(_vm, varSize);

	if (!mem.readFrom(&id, 0, 1))
		return false;
	if (!vars.readFrom(0, 0, varSize))
		return false;

	if (!_writer->writePart(2, &mem))
		return false;
	if (!_writer->writePart(3, &vars))
		return false;

	_hasExtra = true;

	return true;
}

void SaveLoad_v6::GameHandler::buildIndex(byte *buffer) const {
	uint32 varSize = SaveHandler::getVarSize(_vm);

	if (varSize == 0)
		return;

	SavePartInfo info(kSlotNameLength, (uint32) _vm->getGameType(),
			0, _vm->getEndianness(), varSize);

	_slotFile->buildIndex(buffer, info, 0, true);
}

void SaveLoad_v6::GameHandler::refreshProps() {
	uint32 maxSlot = _slotFile->getSlotMax();

	memset(_props + 40, 0xFF, 40);          // Joker
	_props[159] = 0x03;                     // # of joker unused
	WRITE_LE_UINT32(_props + 160, maxSlot); // # of saves
}

bool SaveLoad_v6::GameHandler::createReader(int slot) {
	// If slot < 0, just check if a reader exists
	if (slot < 0)
		return (_reader != 0);

	if (!_reader || (_reader->getSlot() != ((uint32) slot))) {
		Common::String slotFile = _slotFile->build(slot);

		if (slotFile.empty())
			return false;

		delete _reader;

		_reader = new SaveReader(5, slot, slotFile);
		if (!_reader->load()) {
			delete _reader;
			_reader = 0;
			return false;
		}
	}

	return true;
}

bool SaveLoad_v6::GameHandler::createWriter(int slot) {
	// If slot < 0, just check if a writer exists
	if (slot < 0)
		return (_writer != 0);

	if (!_writer || (_writer->getSlot() != ((uint32) slot))) {
		Common::String slotFile = _slotFile->build(slot);

		if (slotFile.empty())
			return false;

		delete _writer;
		_writer = new SaveWriter(5, slot, slotFile);
	}

	return true;
}


SaveLoad_v6::AutoHandler::File::File(GobEngine *vm, const Common::String &base) :
	SlotFileStatic(vm, base, "aut") {
}

SaveLoad_v6::AutoHandler::File::~File() {
}


SaveLoad_v6::AutoHandler::AutoHandler(GobEngine *vm, const Common::String &target) :
	SaveHandler(vm), _file(vm, target) {
}

SaveLoad_v6::AutoHandler::~AutoHandler() {
}

int32 SaveLoad_v6::AutoHandler::getSize() {
	Common::String fileName = _file.build();
	if (fileName.empty())
		return -1;

	SaveReader reader(1, 0, fileName);
	SaveHeader header;

	if (!reader.load())
		return -1;

	if (!reader.readPartHeader(0, &header))
		return -1;

	// Return the part's size
	return header.getSize() + 2900;
}

bool SaveLoad_v6::AutoHandler::load(int16 dataVar, int32 size, int32 offset) {
	uint32 varSize = SaveHandler::getVarSize(_vm);
	if (varSize == 0)
		return false;

	if ((size != 0) || (offset != 2900)) {
		warning("Invalid autoloading procedure (%d, %d, %d)", dataVar, size, offset);
		return false;
	}

	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

	SaveReader reader(1, 0, fileName);
	SaveHeader header;
	SavePartVars vars(_vm, varSize);

	if (!reader.load())
		return false;

	if (!reader.readPartHeader(0, &header))
		return false;

	if (header.getSize() != varSize) {
		warning("Autosave mismatch (%d, %d)", header.getSize(), varSize);
		return false;
	}

	if (!reader.readPart(0, &vars))
		return false;

	if (!vars.writeInto(0, 0, varSize))
		return false;

	return true;
}

bool SaveLoad_v6::AutoHandler::save(int16 dataVar, int32 size, int32 offset) {
	uint32 varSize = SaveHandler::getVarSize(_vm);
	if (varSize == 0)
		return false;

	if ((size != 0) || (offset != 2900)) {
		warning("Invalid autosaving procedure (%d, %d, %d)", dataVar, size, offset);
		return false;
	}

	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

	SaveWriter writer(1, 0, fileName);
	SavePartVars vars(_vm, varSize);

	if (!vars.readFrom(0, 0, varSize))
		return false;

	return writer.writePart(0, &vars);
}


SaveLoad_v6::AutoSpriteHandler::File::File(GobEngine *vm, const Common::String &base) :
	SlotFileStatic(vm, base, "asp") {
}

SaveLoad_v6::AutoSpriteHandler::File::~File() {
}


SaveLoad_v6::AutoSpriteHandler::AutoSpriteHandler(GobEngine *vm,
		const Common::String &target) : TempSpriteHandler(vm), _file(vm, target) {

}

SaveLoad_v6::AutoSpriteHandler::~AutoSpriteHandler() {
}

int32 SaveLoad_v6::AutoSpriteHandler::getSize() {
	Common::InSaveFile *file = _file.openRead();
	if (!file)
		return -1;

	delete file;
	return 1;
}

bool SaveLoad_v6::AutoSpriteHandler::load(int16 dataVar, int32 size, int32 offset) {
	if (offset != 0) {
		warning("Invalid autosprite saving procedure (%d, %d, %d)", dataVar, size, offset);
		return false;
	}

	if (!TempSpriteHandler::create(624, 272, true))
		return false;

	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

	SaveReader reader(1, 0, fileName);
	if (!reader.load())
		return false;

	if (!reader.readPart(0, _sprite))
		return false;

	return TempSpriteHandler::load(dataVar, size, offset);
}

bool SaveLoad_v6::AutoSpriteHandler::save(int16 dataVar, int32 size, int32 offset) {
	if (!TempSpriteHandler::save(dataVar, size, offset))
		return false;

	if (offset != 0) {
		warning("Invalid autosprite saving procedure (%d, %d, %d)", dataVar, size, offset);
		return false;
	}

	Common::String fileName = _file.build();
	if (fileName.empty())
		return false;

	SaveWriter writer(1, 0, fileName);

	return writer.writePart(0, _sprite);
}


SaveLoad_v6::TempHandler::TempHandler(GobEngine *vm) : SaveHandler(vm),
	_empty(true), _size(0), _data(0) {
}

SaveLoad_v6::TempHandler::~TempHandler() {
	delete[] _data;
}

int32 SaveLoad_v6::TempHandler::getSize() {
	if (_empty)
		return -1;

	return _size + 2900;
}

bool SaveLoad_v6::TempHandler::load(int16 dataVar, int32 size, int32 offset) {
	if (_empty || (_size == 0) || !_data)
		return false;

	if ((size != 0) || (offset != 2900)) {
		warning("Invalid temp loading procedure (%d, %d, %d)", dataVar, size, offset);
		return false;
	}

	_vm->_inter->_variables->copyFrom(0, _data, _size);

	return true;
}

bool SaveLoad_v6::TempHandler::save(int16 dataVar, int32 size, int32 offset) {
	if ((size != 0) || (offset != 2900)) {
		warning("Invalid temp saving procedure (%d, %d, %d)", dataVar, size, offset);
		return false;
	}

	delete[] _data;

	_size = SaveHandler::getVarSize(_vm);
	_data = new byte[_size];

	_vm->_inter->_variables->copyTo(0, _data, _size);

	_empty = false;

	return true;
}

bool SaveLoad_v6::TempHandler::deleteFile() {
	delete[] _data;

	_empty = true;
	_size  = 0;
	_data  = 0;

	return true;
}


SaveLoad_v6::ExtraHandler::ExtraHandler(GobEngine *vm, GameHandler &game,
		uint8 id, int slot) : SaveHandler(vm), _game(&game), _id(id), _slot(slot) {

}

SaveLoad_v6::ExtraHandler::~ExtraHandler() {
}

int32 SaveLoad_v6::ExtraHandler::getSize() {
	if (_game->getExtraID(_slot) != _id)
		return -1;

	return SaveHandler::getVarSize(_vm) + 2900;
}

bool SaveLoad_v6::ExtraHandler::load(int16 dataVar, int32 size, int32 offset) {
	return _game->loadExtra(_slot, _id, dataVar, size, offset);
}

bool SaveLoad_v6::ExtraHandler::save(int16 dataVar, int32 size, int32 offset) {
	return _game->saveExtra(_slot, _id, dataVar, size, offset);
}


SaveLoad_v6::SaveLoad_v6(GobEngine *vm, const char *targetName) :
		SaveLoad(vm) {

	_spriteHandler     = new SpriteHandler(vm);
	_gameHandler       = new GameHandler(vm, targetName, *_spriteHandler);
	_autoHandler       = new AutoHandler(vm, targetName);
	_autoSpriteHandler = new AutoSpriteHandler(vm, targetName);

	_tmpHandler[0] = new TempHandler(vm);
	_tmpHandler[1] = new TempHandler(vm);

	_saveFiles[0].handler = _gameHandler;
	_saveFiles[1].handler = _autoHandler;

	_saveFiles[7].handler = _tmpHandler[0];
	_saveFiles[8].handler = _tmpHandler[1];

	for (int i = 0; i < 60; i++)
		_saveFiles[ 9 + i].handler =
			_extraHandler[     i] = new ExtraHandler(_vm, *_gameHandler, 2, i);
	for (int i = 0; i < 60; i++)
		_saveFiles[69 + i].handler =
			_extraHandler[60 + i] = new ExtraHandler(_vm, *_gameHandler, 3, i);

	_saveFiles[129].handler = _autoSpriteHandler;

	for (int i = 0; i < 60; i++)
		_saveFiles[130 + i].handler = _spriteHandler;
}

SaveLoad_v6::~SaveLoad_v6() {
	for (int i = 0; i < 120; i++)
		delete _extraHandler[i];

	delete _tmpHandler[0];
	delete _tmpHandler[1];
	delete _autoSpriteHandler;
	delete _autoHandler;
	delete _gameHandler;
	delete _spriteHandler;
}

const SaveLoad_v6::SaveFile *SaveLoad_v6::getSaveFile(const char *fileName) const {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveLoad_v6::SaveFile *SaveLoad_v6::getSaveFile(const char *fileName) {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveHandler *SaveLoad_v6::getHandler(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->handler;

	return 0;
}

const char *SaveLoad_v6::getDescription(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->description;

	return 0;
}

SaveLoad::SaveMode SaveLoad_v6::getSaveMode(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->mode;

	return kSaveModeNone;
}

} // End of namespace Gob
