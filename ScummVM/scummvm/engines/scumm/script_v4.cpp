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

#include "scumm/scumm_v4.h"
#include "scumm/object.h"

namespace Scumm {

#define OPCODE(i, x)	_opcodes[i]._OPCODE(ScummEngine_v4, x)

void ScummEngine_v4::setupOpcodes() {
	ScummEngine_v5::setupOpcodes();

	OPCODE(0x25, o5_drawObject);
	OPCODE(0x45, o5_drawObject);
	OPCODE(0x65, o5_drawObject);
	OPCODE(0xa5, o5_drawObject);
	OPCODE(0xc5, o5_drawObject);
	OPCODE(0xe5, o5_drawObject);

	OPCODE(0x50, o4_pickupObject);
	OPCODE(0xd0, o4_pickupObject);

	OPCODE(0x5c, o4_oldRoomEffect);
	OPCODE(0xdc, o4_oldRoomEffect);

	OPCODE(0x0f, o4_ifState);
	OPCODE(0x4f, o4_ifState);
	OPCODE(0x8f, o4_ifState);
	OPCODE(0xcf, o4_ifState);

	OPCODE(0x2f, o4_ifNotState);
	OPCODE(0x6f, o4_ifNotState);
	OPCODE(0xaf, o4_ifNotState);
	OPCODE(0xef, o4_ifNotState);

	OPCODE(0xa7, o4_saveLoadVars);

	OPCODE(0x22, o4_saveLoadGame);
	OPCODE(0xa2, o4_saveLoadGame);

	// Disable some opcodes which are unused in v4.
	_opcodes[0x3b].setProc(0, 0);
	_opcodes[0x4c].setProc(0, 0);
	_opcodes[0xbb].setProc(0, 0);
}

void ScummEngine_v4::o4_ifState() {
	int a = getVarOrDirectWord(PARAM_1);
	int b = getVarOrDirectByte(PARAM_2);

	// WORKAROUND bug #3306145 (also occurs in original): Some old versions of
	// Indy3 sometimes fail to allocate IQ points correctly. To quote:
	// "About the points error leaving Castle Brunwald: It seems to "reversed"!
	// When you get caught, free yourself and escape, you get 25 IQ points even
	// though you're not supposed to. However if you escape WITHOUT getting
	// caught, you get 0 IQ points (supposed to get 25 IQ points)."
	// This workaround is meant to address that.
	if (_game.id == GID_INDY3 && a == 367 &&
	    vm.slot[_currentScript].number == 363 && _currentRoom == 25) {
		b = 0;
	}

	jumpRelative(getState(a) == b);
}

void ScummEngine_v4::o4_ifNotState() {
	int a = getVarOrDirectWord(PARAM_1);
	int b = getVarOrDirectByte(PARAM_2);

	jumpRelative(getState(a) != b);
}

void ScummEngine_v4::o4_pickupObject() {
	int obj = getVarOrDirectWord(PARAM_1);

	if (obj < 1) {
		error("pickupObjectOld received invalid index %d (script %d)", obj, vm.slot[_currentScript].number);
	}

	if (getObjectIndex(obj) == -1)
		return;

	if (whereIsObject(obj) == WIO_INVENTORY)	// Don't take an object twice
		return;

	// debug(0, "adding %d from %d to inventoryOld", obj, _currentRoom);
	addObjectToInventory(obj, _roomResource);
	markObjectRectAsDirty(obj);
	putOwner(obj, VAR(VAR_EGO));
	putClass(obj, kObjectClassUntouchable, 1);
	putState(obj, 1);
	clearDrawObjectQueue();
	runInventoryScript(1);
}

void ScummEngine_v4::o4_oldRoomEffect() {
	int a;

	_opcode = fetchScriptByte();
	if ((_opcode & 0x1F) == 3) {
		a = getVarOrDirectWord(PARAM_1);

		if (_game.platform == Common::kPlatformFMTowns && _game.version == 3) {
			if (a == 4) {
				_textSurface.fillRect(Common::Rect(0, 0, _textSurface.w * _textSurfaceMultiplier, _textSurface.h * _textSurfaceMultiplier), 0);
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
				if (_townsScreen)
					_townsScreen->clearLayer(1);
#endif
				return;
			}
		}

		if (a) {
			_switchRoomEffect = (byte)(a & 0xFF);
			_switchRoomEffect2 = (byte)(a >> 8);
		} else {
			fadeIn(_newEffect);
		}
	}
}

void ScummEngine_v4::o4_saveLoadVars() {
	if (fetchScriptByte() == 1)
		saveVars();
	else
		loadVars();
}

enum StringIds {
	// The string IDs used by Indy3 to store the episode resp. series IQ points.
	// Note that we save the episode IQ points but load the series IQ points,
	// which matches the original Indy3 save/load code. See also the notes
	// on Feature Request #1666521.
	STRINGID_IQ_EPISODE = 7,
	STRINGID_IQ_SERIES = 9,
	// The string IDs of the first savegame name, used as an offset to determine
	// the IDs of all savenames.
	// Loom is the only game whose savenames start with a different ID.
	STRINGID_SAVENAME1 = 10,
	STRINGID_SAVENAME1_LOOM = 9
};

void ScummEngine_v4::saveVars() {
	int a, b;

	while ((_opcode = fetchScriptByte()) != 0) {
		switch (_opcode & 0x1F) {
		case 0x01: // write a range of variables
			getResultPos();
			a = _resultVarNumber;
			getResultPos();
			b = _resultVarNumber;
			debug(0, "stub saveVars: vars %d -> %d", a, b);
			break;
		case 0x02: // write a range of string variables
			a = getVarOrDirectByte(PARAM_1);
			b = getVarOrDirectByte(PARAM_2);

			if (a == STRINGID_IQ_EPISODE && b == STRINGID_IQ_EPISODE) {
				if (_game.id == GID_INDY3) {
					saveIQPoints();
				}
				break;
			}
			// FIXME: changing savegame-names not supported
			break;
		case 0x03: // open file
			a = resStrLen(_scriptPointer);
			strncpy(_saveLoadVarsFilename, (const char *)_scriptPointer, a);
			_saveLoadVarsFilename[a] = '\0';
			_scriptPointer += a + 1;
			break;
		case 0x04:
			return;
		case 0x1F: // close file
			_saveLoadVarsFilename[0] = '\0';
			return;
		}
	}
}

void ScummEngine_v4::loadVars() {
	int a, b;

	while ((_opcode = fetchScriptByte()) != 0) {
		switch (_opcode & 0x1F) {
		case 0x01: // read a range of variables
			getResultPos();
			a = _resultVarNumber;
			getResultPos();
			b = _resultVarNumber;
			debug(0, "stub loadVars: vars %d -> %d", a, b);
			break;
		case 0x02: // read a range of string variables
			a = getVarOrDirectByte(PARAM_1);
			b = getVarOrDirectByte(PARAM_2);

			int slot;
			int slotSize;
			byte* slotContent;
			int savegameId;
			bool avail_saves[100];

			if (a == STRINGID_IQ_SERIES && b == STRINGID_IQ_SERIES) {
				// Zak256 loads the IQ script-slot but does not use it -> ignore it
				if (_game.id == GID_INDY3) {
					byte *ptr = getResourceAddress(rtString, STRINGID_IQ_SERIES);
					if (ptr) {
						int size = getResourceSize(rtString, STRINGID_IQ_SERIES);
						loadIQPoints(ptr, size);
					}
				}
				break;
			}

			listSavegames(avail_saves, ARRAYSIZE(avail_saves));
			for (slot = a; slot <= b; ++slot) {
				slotSize = getResourceSize(rtString, slot);
				slotContent = getResourceAddress(rtString, slot);

				// load savegame names
				savegameId = slot - a + 1;
				Common::String name;
				if (avail_saves[savegameId] && getSavegameName(savegameId, name)) {
					int pos;
					const char *ptr = name.c_str();
					// slotContent ends with {'\0','@'} -> max. length = slotSize-2
					for (pos = 0; pos < slotSize - 2; ++pos) {
						if (!ptr[pos])
							break;
						// replace special characters
						if (ptr[pos] >= 32 && ptr[pos] <= 122 && ptr[pos] != 64)
							slotContent[pos] = ptr[pos];
						else
							slotContent[pos] = '_';
					}
					slotContent[pos] = '\0';
				} else {
					slotContent[0] = '\0';
				}
			}
			break;
		case 0x03: // open file
			a = resStrLen(_scriptPointer);
			strncpy(_saveLoadVarsFilename, (const char *)_scriptPointer, a);
			_saveLoadVarsFilename[a] = '\0';
			_scriptPointer += a + 1;
			break;
		case 0x04:
			return;
		case 0x1F: // close file
			_saveLoadVarsFilename[0] = '\0';
			return;
		}
	}
}

/**
 * IQ Point calculation for Indy3.
 * The scripts that perform this task are
 * - script-9 (save/load dialog initialization, loads room 14),
 * - room-14-204 (load series IQ string),
 * - room-14-205 (save series IQ string),
 * - room-14-206 (calculate series IQ string).
 * Unfortunately script-9 contains lots of GUI stuff so calling this script
 * directly is not possible. The other scripts depend on script-9.
 */
void ScummEngine_v4::updateIQPoints() {
	int seriesIQ;
	// IQString[0..72] corresponds to each puzzle's IQ.
	// IQString[73] indicates that the IQ-file was loaded successfully and is always 0 when
	// the IQ is calculated, hence it will be ignored here.
	const int NUM_PUZZLES = 73;
	byte seriesIQString[NUM_PUZZLES];
	byte *episodeIQString;
	int episodeIQStringSize;

	// load string with IQ points given per puzzle in any savegame
	// IMPORTANT: the resource string STRINGID_IQ_SERIES is only valid while
	// the original save/load dialog is executed, so do not use it here.
	memset(seriesIQString, 0, sizeof(seriesIQString));
	loadIQPoints(seriesIQString, sizeof(seriesIQString));

	// string with IQ points given per puzzle in current savegame
	episodeIQString = getResourceAddress(rtString, STRINGID_IQ_EPISODE);
	if (!episodeIQString)
		return;
	episodeIQStringSize = getResourceSize(rtString, STRINGID_IQ_EPISODE);
	if (episodeIQStringSize < NUM_PUZZLES)
		return;

	// merge episode and series IQ strings and calculate series IQ
	seriesIQ = 0;
	// iterate over puzzles
	for (int i = 0; i < NUM_PUZZLES; ++i) {
		byte puzzleIQ = seriesIQString[i];
		// if puzzle is solved copy points to episode string
		if (puzzleIQ > 0)
			episodeIQString[i] = puzzleIQ;
		// add puzzle's IQ-points to series IQ
		seriesIQ += episodeIQString[i];
	}
	_scummVars[245] = seriesIQ;

	// save series IQ string
	saveIQPoints();
}

void ScummEngine_v4::saveIQPoints() {
	// save Indy3 IQ-points
	Common::OutSaveFile *file;
	Common::String filename = _targetName + ".iq";

	file = _saveFileMan->openForSaving(filename);
	if (file != NULL) {
		byte *ptr = getResourceAddress(rtString, STRINGID_IQ_EPISODE);
		if (ptr) {
			int size = getResourceSize(rtString, STRINGID_IQ_EPISODE);
			file->write(ptr, size);
		}
		delete file;
	}
}

void ScummEngine_v4::loadIQPoints(byte *ptr, int size) {
	// load Indy3 IQ-points
	Common::InSaveFile *file;
	Common::String filename = _targetName + ".iq";

	file = _saveFileMan->openForLoading(filename);
	if (file != NULL) {
		byte *tmp = (byte *)malloc(size);
		int nread = file->read(tmp, size);
		if (nread == size) {
			memcpy(ptr, tmp, size);
		}
		free(tmp);
		delete file;
	}
}

void ScummEngine_v4::o4_saveLoadGame() {
	getResultPos();
	byte slot;
	byte a = getVarOrDirectByte(PARAM_1);
	byte result = 0;

	if ((_game.id == GID_MANIAC && _game.version <= 1) || (_game.id == GID_ZAK && _game.platform == Common::kPlatformC64)) {
		// Convert V0/V1 load/save screen (they support only one savegame per disk)
		// 1 Load
		// 2 Save
		slot = 1;
		if (a == 1)
			_opcode = 0x40;
		else if ((a == 2) || (_game.platform == Common::kPlatformNES))
			_opcode = 0x80;
	} else {
		slot = a & 0x1F;
		// Slot numbers in older games start with 0, in newer games with 1
		if (_game.version <= 2)
			slot++;
		_opcode = a & 0xE0;
	}

	switch (_opcode) {
	case 0x00: // num slots available
		result = 100;
		break;
	case 0x20: // drive
		if (_game.version <= 3) {
			// 0 = ???
			// [1,2] = disk drive [A:,B:]
			// 3 = hard drive
			result = 3;
		} else {
			// set current drive
			result = 1;
		}
		break;
	case 0x40: // load
		if (loadState(slot, false))
			result = 3; // sucess
		else
			result = 5; // failed to load
		break;
	case 0x80: // save
		if (_game.version <= 3) {
			char name[32];
			if (_game.version <= 2) {
				// use generic name
				sprintf(name, "Game %c", 'A'+slot-1);
			} else {
				// use name entered by the user
				char* ptr;
				int firstSlot = (_game.id == GID_LOOM) ? STRINGID_SAVENAME1_LOOM : STRINGID_SAVENAME1;
				ptr = (char *)getStringAddress(slot + firstSlot - 1);
				strncpy(name, ptr, sizeof(name));
			}

			if (savePreparedSavegame(slot, name))
				result = 0;
			else
				result = 2;
		} else {
			result = 2; // failed to save
		}
		break;
	case 0xC0: // test if save exists
		{
		Common::InSaveFile *file;
		bool avail_saves[100];

		listSavegames(avail_saves, ARRAYSIZE(avail_saves));
		Common::String filename = makeSavegameName(slot, false);
		if (avail_saves[slot] && (file = _saveFileMan->openForLoading(filename))) {
			result = 6; // save file exists
			delete file;
		} else
			result = 7; // save file does not exist
		}
		break;
	default:
		error("o4_saveLoadGame: unknown subopcode %d", _opcode);
	}

	setResult(result);
}

} // End of namespace Scumm
