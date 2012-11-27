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

#include "common/savefile.h"
#include "common/textconsole.h"

#include "tucker/tucker.h"

namespace Tucker {

enum {
	kCurrentGameStateVersion = 1
};

Common::String generateGameStateFileName(const char *target, int slot, bool prefixOnly) {
	Common::String name(target);
	if (prefixOnly) {
		name += ".*";
	} else {
		name += Common::String::format(".%d", slot);
	}
	return name;
}

static void saveOrLoadInt(Common::WriteStream &stream, int &i) {
	stream.writeSint32LE(i);
}

static void saveOrLoadInt(Common::ReadStream &stream, int &i) {
	i = stream.readSint32LE();
}

template<class S>
void TuckerEngine::saveOrLoadGameStateData(S &s) {
	for (int i = 0; i < kFlagsTableSize; ++i) {
		saveOrLoadInt(s, _flagsTable[i]);
	}
	for (int i = 0; i < 40; ++i) {
		saveOrLoadInt(s, _inventoryObjectsList[i]);
	}
	for (int i = 0; i < 50; ++i) {
		saveOrLoadInt(s, _inventoryItemsState[i]);
	}
	for (int i = 0; i < 50; ++i) {
		saveOrLoadInt(s, _panelObjectsOffsetTable[i]);
	}
	saveOrLoadInt(s, _mainSpritesBaseOffset);
	saveOrLoadInt(s, _selectedObject.xPos);
	saveOrLoadInt(s, _selectedObject.yPos);
	saveOrLoadInt(s, _locationNum);
	saveOrLoadInt(s, _xPosCurrent);
	saveOrLoadInt(s, _yPosCurrent);
	saveOrLoadInt(s, _inventoryObjectsCount);
	saveOrLoadInt(s, _inventoryObjectsOffset);
}

Common::Error TuckerEngine::loadGameState(int num) {
	Common::Error ret = Common::kNoError;
	Common::String gameStateFileName = generateGameStateFileName(_targetName.c_str(), num);
	Common::InSaveFile *f = _saveFileMan->openForLoading(gameStateFileName);
	if (f) {
		uint16 version = f->readUint16LE();
		if (version < kCurrentGameStateVersion) {
			warning("Unsupported gamestate version %d (slot %d)", version, num);
		} else {
			f->skip(2);
			saveOrLoadGameStateData(*f);
			if (f->err() || f->eos()) {
				warning("Can't read file '%s'", gameStateFileName.c_str());
				ret = Common::kReadingFailed;
			} else {
				_nextLocationNum = _locationNum;
				setBlackPalette();
				loadBudSpr(0);
				_forceRedrawPanelItems = true;
			}
		}
		delete f;
	}
	return ret;
}

Common::Error TuckerEngine::saveGameState(int num, const Common::String &description) {
	Common::Error ret = Common::kNoError;
	Common::String gameStateFileName = generateGameStateFileName(_targetName.c_str(), num);
	Common::OutSaveFile *f = _saveFileMan->openForSaving(gameStateFileName);
	if (f) {
		f->writeUint16LE(kCurrentGameStateVersion);
		f->writeUint16LE(0);
		saveOrLoadGameStateData(*f);
		f->finalize();
		if (f->err()) {
			warning("Can't write file '%s'", gameStateFileName.c_str());
			ret = Common::kWritingFailed;
		}
		delete f;
	}
	return ret;
}


bool TuckerEngine::canLoadGameStateCurrently() {
	return !_player && _cursorType < 2;
}

bool TuckerEngine::canSaveGameStateCurrently() {
	return !_player && _cursorType < 2;
}

} // namespace Tucker
