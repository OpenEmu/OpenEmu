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

#include "kyra/kyra_lok.h"
#include "kyra/animator_lok.h"
#include "kyra/resource.h"
#include "kyra/sound.h"
#include "kyra/timer.h"

#include "common/savefile.h"

namespace Kyra {

Common::Error KyraEngine_LoK::loadGameState(int slot) {
	const char *fileName = getSavegameFilename(slot);

	SaveHeader header;
	Common::InSaveFile *in = openSaveForReading(fileName, header);
	if (!in)
		return _saveFileMan->getError();

	if (header.originalSave) {
		// no support for original savefile in Kyrandia 1 (yet)
		delete in;
		return Common::kUnknownError;
	}

	snd_playSoundEffect(0x0A);
	snd_playWanderScoreViaMap(0, 1);

	// unloading the current voice file should fix some problems with voices
	if (_currentRoom != 0xFFFF && _flags.isTalkie) {
		char file[32];
		assert(_currentRoom < _roomTableSize);
		int tableId = _roomTable[_currentRoom].nameIndex;
		assert(tableId < _roomFilenameTableSize);
		strcpy(file, _roomFilenameTable[tableId]);
		strcat(file, ".VRM");
		_res->unloadPakFile(file);
	}

	for (int i = 0; i < 11; i++) {
		_characterList[i].sceneId = in->readUint16BE();
		_characterList[i].height = in->readByte();
		_characterList[i].facing = in->readByte();
		_characterList[i].currentAnimFrame = in->readUint16BE();
		//_characterList[i].unk6 = in->readUint32BE();
		in->read(_characterList[i].inventoryItems, 10);
		_characterList[i].x1 = in->readSint16BE();
		_characterList[i].y1 = in->readSint16BE();
		_characterList[i].x2 = in->readSint16BE();
		_characterList[i].y2 = in->readSint16BE();
		//_characterList[i].field_20 = in->readUint16BE();
		//_characterList[i].field_23 = in->readUint16BE();
	}

	_marbleVaseItem = in->readSint16BE();
	_itemInHand = (int8)in->readByte();

	for (int i = 0; i < 4; ++i)
		_birthstoneGemTable[i] = in->readByte();
	for (int i = 0; i < 3; ++i)
		_idolGemsTable[i] = in->readByte();
	for (int i = 0; i < 3; ++i)
		_foyerItemTable[i] = in->readByte();
	_cauldronState = in->readByte();
	for (int i = 0; i < 2; ++i)
		_crystalState[i] = in->readByte();

	_brandonStatusBit = in->readUint16BE();
	_brandonStatusBit0x02Flag = in->readByte();
	_brandonStatusBit0x20Flag = in->readByte();
	in->read(_brandonPoisonFlagsGFX, 256);
	_brandonInvFlag = in->readSint16BE();
	_poisonDeathCounter = in->readByte();
	_animator->_brandonDrawFrame = in->readUint16BE();

	_timer->loadDataFromFile(*in, header.version);

	memset(_flagsTable, 0, sizeof(_flagsTable));
	uint32 flagsSize = in->readUint32BE();
	assert(flagsSize <= sizeof(_flagsTable));
	in->read(_flagsTable, flagsSize);

	for (int i = 0; i < _roomTableSize; ++i) {
		for (int item = 0; item < 12; ++item) {
			_roomTable[i].itemsTable[item] = kItemNone;
			_roomTable[i].itemsXPos[item] = 0xFFFF;
			_roomTable[i].itemsYPos[item] = 0xFF;
			_roomTable[i].needInit[item] = 0;
		}
	}

	uint16 sceneId = 0;

	while (true) {
		sceneId = in->readUint16BE();
		if (sceneId == 0xFFFF)
			break;
		assert(sceneId < _roomTableSize);
		_roomTable[sceneId].nameIndex = in->readByte();

		for (int i = 0; i < 12; i++) {
			_roomTable[sceneId].itemsTable[i] = in->readByte();
			_roomTable[sceneId].itemsXPos[i] = in->readUint16BE();
			_roomTable[sceneId].itemsYPos[i] = in->readUint16BE();
			_roomTable[sceneId].needInit[i] = in->readByte();
		}
	}
	if (header.version >= 3) {
		_lastMusicCommand = in->readSint16BE();
		if (_lastMusicCommand != -1)
			snd_playWanderScoreViaMap(_lastMusicCommand, 1);
	}

	// Version 4 stored settings in the savegame. As of version 5, they are
	// handled by the config manager.

	if (header.version == 4) {
		in->readByte(); // Text speed
		in->readByte(); // Walk speed
		in->readByte(); // Music
		in->readByte(); // Sound
		in->readByte(); // Voice
	}

	if (header.version >= 7) {
		_curSfxFile = in->readByte();

		// In the first version when this entry was introduced,
		// it wasn't made sure that _curSfxFile was initialized
		// so if it's out of bounds we just set it to 0.
		if (_flags.platform == Common::kPlatformFMTowns) {
			if (!_sound->hasSoundFile(_curSfxFile))
				_curSfxFile = 0;
			_sound->loadSoundFile(_curSfxFile);
		}
	}

	loadMainScreen(8);

	if (queryGameFlag(0x2D)) {
		_screen->loadBitmap("AMULET3.CPS", 10, 10, 0);
		if (!queryGameFlag(0xF1)) {
			for (int i = 0x55; i <= 0x5A; ++i) {
				if (queryGameFlag(i))
					seq_createAmuletJewel(i - 0x55, 10, 1, 1);
			}
		}

		_screen->copyRegion(8, 8, 8, 8, 304, 212, 10, 0);
	}

	setHandItem(_itemInHand);

	// Will-O-Wisp uses a different shape size than Brandon's usual
	// shape, thus we need to setup the correct size depending on
	// his state over here. This fixes graphics glitches when loading
	// saves, where Brandon is transformed into the Will-O-Wisp.
	if (_brandonStatusBit & 2)
		_animator->setBrandonAnimSeqSize(5, 48);
	else
		_animator->setBrandonAnimSeqSize(3, 48);

	redrawInventory(0);

	// Original hardcoded Brandon position for certain scenes:
	// - SceneId 7 ("A ruined bridge") and flag 0x39 set, which seems
	//  to indicate that Herman is still in the scene.
	// - SceneId 2 ("Inside the temple") and flag 0x2D not set, which
	//  indicates that the amulet is not obtained yet and thus Brynn
	//  is still inside the temple
	if (_currentCharacter->sceneId == 7 && queryGameFlag(0x39)) {
		_currentCharacter->x1 = 282;
		_currentCharacter->y1 = 108;
		_currentCharacter->facing = 5;
	} else if (_currentCharacter->sceneId == 2 && !queryGameFlag(0x2D)) {
		_currentCharacter->x1 = 294;
		_currentCharacter->y1 = 132;
		_currentCharacter->facing = 5;
	}

	_brandonPosX = _currentCharacter->x2 = _currentCharacter->x1;
	_brandonPosY = _currentCharacter->y2 = _currentCharacter->y1;

	// We need to reset the "_noDrawShapesFlag" flag of Animator_LoK
	// over here. Else in certain cases restoring an savegame might
	// result in no shapes being drawn at all. See bug report
	// #2868581 "KYRA1: Invisible Brandon" for an example of this.
	_animator->_noDrawShapesFlag = 0;

	enterNewScene(_currentCharacter->sceneId, _currentCharacter->facing, 0, 0, 1);

	_animator->animRefreshNPC(0);
	_animator->restoreAllObjectBackgrounds();
	_animator->preserveAnyChangedBackgrounds();
	_animator->prepDrawAllObjects();
	_animator->copyChangedObjectsForward(0);
	_screen->copyRegion(8, 8, 8, 8, 304, 128, 2, 0);
	_screen->updateScreen();

	setMousePos(_currentCharacter->x1, _currentCharacter->y1);

	if (in->err() || in->eos()) {
		warning("Load failed ('%s', '%s').", fileName, header.description.c_str());
		return Common::kUnknownError;
	} else {
		debugC(1, kDebugLevelMain, "Loaded savegame '%s.'", header.description.c_str());
	}

	// We didn't explicitly set the walk speed, but it's saved as part of
	// the _timers array, so we need to re-sync it with _configWalkspeed.
	setWalkspeed(_configWalkspeed);

	delete in;
	return Common::kNoError;
}

Common::Error KyraEngine_LoK::saveGameStateIntern(int slot, const char *saveName, const Graphics::Surface *thumb) {
	const char *fileName = getSavegameFilename(slot);

	if (shouldQuit())
		return Common::kNoError;

	Common::OutSaveFile *out = openSaveForWriting(fileName, saveName, thumb);
	if (!out)
		return _saveFileMan->getError();

	for (int i = 0; i < 11; i++) {
		out->writeUint16BE(_characterList[i].sceneId);
		out->writeByte(_characterList[i].height);
		out->writeByte(_characterList[i].facing);
		out->writeUint16BE(_characterList[i].currentAnimFrame);
		//out->writeUint32BE(_characterList[i].unk6);
		out->write(_characterList[i].inventoryItems, 10);
		out->writeSint16BE(_characterList[i].x1);
		out->writeSint16BE(_characterList[i].y1);
		out->writeSint16BE(_characterList[i].x2);
		out->writeSint16BE(_characterList[i].y2);
		//out->writeUint16BE(_characterList[i].field_20);
		//out->writeUint16BE(_characterList[i].field_23);
	}

	out->writeSint16BE(_marbleVaseItem);
	out->writeByte(_itemInHand);

	for (int i = 0; i < 4; ++i)
		out->writeByte(_birthstoneGemTable[i]);
	for (int i = 0; i < 3; ++i)
		out->writeByte(_idolGemsTable[i]);
	for (int i = 0; i < 3; ++i)
		out->writeByte(_foyerItemTable[i]);
	out->writeByte(_cauldronState);
	for (int i = 0; i < 2; ++i)
		out->writeByte(_crystalState[i]);

	out->writeUint16BE(_brandonStatusBit);
	out->writeByte(_brandonStatusBit0x02Flag);
	out->writeByte(_brandonStatusBit0x20Flag);
	out->write(_brandonPoisonFlagsGFX, 256);
	out->writeSint16BE(_brandonInvFlag);
	out->writeByte(_poisonDeathCounter);
	out->writeUint16BE(_animator->_brandonDrawFrame);

	_timer->saveDataToFile(*out);

	out->writeUint32BE(sizeof(_flagsTable));
	out->write(_flagsTable, sizeof(_flagsTable));

	for (uint16 i = 0; i < _roomTableSize; i++) {
		out->writeUint16BE(i);
		out->writeByte(_roomTable[i].nameIndex);
		for (int a = 0; a < 12; a++) {
			out->writeByte(_roomTable[i].itemsTable[a]);
			out->writeUint16BE(_roomTable[i].itemsXPos[a]);
			out->writeUint16BE(_roomTable[i].itemsYPos[a]);
			out->writeByte(_roomTable[i].needInit[a]);
		}
	}
	// room table terminator
	out->writeUint16BE(0xFFFF);

	out->writeSint16BE(_lastMusicCommand);

	out->writeByte(_curSfxFile);

	out->finalize();

	// check for errors
	if (out->err()) {
		warning("Can't write file '%s'. (Disk full?)", fileName);
		return Common::kUnknownError;
	} else {
		debugC(1, kDebugLevelMain, "Saved game '%s.'", saveName);
	}

	delete out;
	return Common::kNoError;
}
} // End of namespace Kyra
