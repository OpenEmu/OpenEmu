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

#include "common/endian.h"
#include "common/savefile.h"
#include "common/substream.h"
#include "common/system.h"

#include "kyra/kyra_mr.h"
#include "kyra/timer.h"

namespace Kyra {

Common::Error KyraEngine_MR::saveGameStateIntern(int slot, const char *saveName, const Graphics::Surface *thumb) {
	const char *fileName = getSavegameFilename(slot);

	Common::OutSaveFile *out = openSaveForWriting(fileName, saveName, thumb);
	if (!out)
		return _saveFileMan->getError();

	_timer->saveDataToFile(*out);

	out->writeUint32BE(sizeof(_flagsTable));
	out->write(_flagsTable, sizeof(_flagsTable));

	out->writeSint16BE(_lastMusicCommand);
	out->writeByte(_currentChapter);
	out->writeByte(_characterShapeFile);
	out->writeByte(_album.curPage);
	out->writeSint16BE(_score);
	out->writeSint16BE(_scoreMax);
	out->writeByte(_malcolmsMood);
	for (int i = 0; i < 30; ++i)
		out->write(_conversationState[i], 30);
	out->write(_newSceneDlgState, 40);
	for (int i = 0; i < 100; ++i)
		out->writeSint16BE(_hiddenItems[i]);
	out->write(_scoreFlagTable, 26);

	out->writeUint16BE(_mainCharacter.sceneId);
	out->writeSint16BE(_mainCharacter.dlgIndex);
	out->writeByte(_mainCharacter.height);
	out->writeByte(_mainCharacter.facing);
	out->writeUint16BE(_mainCharacter.animFrame);
	out->writeByte(_mainCharacter.walkspeed);
	for (int i = 0; i < 10; ++i)
		out->writeUint16BE(_mainCharacter.inventory[i]);
	out->writeSint16BE(_mainCharacter.x1);
	out->writeSint16BE(_mainCharacter.y1);
	out->writeSint16BE(_mainCharacter.x2);
	out->writeSint16BE(_mainCharacter.y2);
	out->writeSint16BE(_mainCharacter.x3);
	out->writeSint16BE(_mainCharacter.y3);

	for (int i = 0; i < 50; ++i) {
		out->writeSint16BE(_itemList[i].id);
		out->writeUint16BE(_itemList[i].sceneId);
		out->writeSint16BE(_itemList[i].x);
		out->writeSint16BE(_itemList[i].y);
	}

	for (int i = 0; i < 88; ++i) {
		out->write(_talkObjectList[i].filename, 13);
		out->writeByte(_talkObjectList[i].sceneAnim);
		out->writeByte(_talkObjectList[i].sceneScript);
		out->writeSint16BE(_talkObjectList[i].x);
		out->writeSint16BE(_talkObjectList[i].y);
		out->writeByte(_talkObjectList[i].color);
		out->writeByte(_talkObjectList[i].sceneId);
	}

	for (int i = 0; i < 98; ++i) {
		out->write(_sceneList[i].filename1, 10);
		out->write(_sceneList[i].filename2, 10);
		out->writeUint16BE(_sceneList[i].exit1);
		out->writeUint16BE(_sceneList[i].exit2);
		out->writeUint16BE(_sceneList[i].exit3);
		out->writeUint16BE(_sceneList[i].exit4);
		out->writeByte(_sceneList[i].flags);
		out->writeByte(_sceneList[i].sound);
	}

	out->writeSint16BE(_itemInHand);
	out->writeUint16BE(_sceneExit1);
	out->writeUint16BE(_sceneExit2);
	out->writeUint16BE(_sceneExit3);
	out->writeUint16BE(_sceneExit4);

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

Common::Error KyraEngine_MR::loadGameState(int slot) {
	const char *fileName = getSavegameFilename(slot);

	SaveHeader header;
	Common::InSaveFile *saveFile = openSaveForReading(fileName, header);
	if (!saveFile) {
		showMessageFromCCode(17, 0xB3, 0);
		snd_playSoundEffect(0x0D, 0xC8);
		return Common::kUnknownError;
	}

	if (header.originalSave)
		warning("Trying to load savegame from original interpreter, while this is possible, it is not officially supported");

	if (_inventoryState) {
		updateCharacterAnim(0);
		restorePage3();
		drawAnimObjects();
		_inventoryState = true;
		refreshAnimObjects(0);
		hideInventory();
	}

	_deathHandler = -1;
	if (!_unkSceneScreenFlag1)
		_lastMusicCommand = -1;

	int curShapes = _characterShapeFile;

	Common::SeekableSubReadStreamEndian in(saveFile, saveFile->pos(), saveFile->size(), !header.originalSave, DisposeAfterUse::YES);

	_screen->hideMouse();

	if (!header.originalSave) {
		_timer->loadDataFromFile(in, header.version);

		uint32 flagsSize = in.readUint32BE();
		assert(flagsSize <= sizeof(_flagsTable));
		in.read(_flagsTable, flagsSize);
	}

	_lastMusicCommand = in.readSint16();
	_currentChapter = in.readByte();
	_characterShapeFile = in.readByte();

	if (header.version >= 12 || header.originalSave)
		_album.curPage = in.readByte();
	if (header.originalSave)
		in.readByte();

	_score = in.readSint16();
	_scoreMax = in.readSint16();
	_malcolmsMood = in.readByte();

	if (header.originalSave)
		in.seek(8, SEEK_CUR);

	for (int i = 0; i < 30; ++i)
		in.read(_conversationState[i], 30);

	if (!header.originalSave) {
		in.read(_newSceneDlgState, 40);
	} else {
		for (int i = 0; i < 40; ++i)
			_newSceneDlgState[i] = in.readUint16();
	}

	for (int i = 0; i < 100; ++i)
		_hiddenItems[i] = in.readSint16();

	if (header.originalSave)
		in.read(_flagsTable, 69);
	in.read(_scoreFlagTable, 26);

	_mainCharacter.sceneId = in.readUint16();
	_mainCharacter.dlgIndex = in.readSint16();
	_mainCharacter.height = in.readByte();
	_mainCharacter.facing = in.readByte();
	_mainCharacter.animFrame = in.readUint16();
	if (!header.originalSave) {
		_mainCharacter.walkspeed = in.readByte();
	} else {
		in.seek(2, SEEK_CUR);
		_mainCharacter.walkspeed = in.readUint32();
	}
	for (int i = 0; i < 10; ++i)
		_mainCharacter.inventory[i] = in.readUint16();
	_mainCharacter.x1 = in.readSint16();
	_mainCharacter.y1 = in.readSint16();
	_mainCharacter.x2 = in.readSint16();
	_mainCharacter.y2 = in.readSint16();
	_mainCharacter.x3 = in.readSint16();
	_mainCharacter.y3 = in.readSint16();

	for (int i = 0; i < 50; ++i) {
		_itemList[i].id = in.readSint16();
		_itemList[i].sceneId = in.readUint16();
		_itemList[i].x = in.readSint16();
		_itemList[i].y = in.readSint16();
		if (header.version <= 9 || header.originalSave)
			in.readUint16();
	}

	for (int i = 0; i < 88; ++i) {
		in.read(_talkObjectList[i].filename, 13);
		_talkObjectList[i].sceneAnim = in.readByte();
		_talkObjectList[i].sceneScript = in.readByte();
		_talkObjectList[i].x = in.readSint16();
		_talkObjectList[i].y = in.readSint16();
		_talkObjectList[i].color = in.readByte();
		if (header.version >= 13 || header.originalSave)
			_talkObjectList[i].sceneId = in.readByte();
	}

	for (int i = 0; i < 98; ++i) {
		if (!header.originalSave) {
			in.read(_sceneList[i].filename1, 10);
		} else {
			in.read(_sceneList[i].filename1, 9);
			_sceneList[i].filename1[9] = 0;
		}

		if (!header.originalSave) {
			in.read(_sceneList[i].filename2, 10);
		} else {
			in.read(_sceneList[i].filename2, 9);
			_sceneList[i].filename2[9] = 0;
		}

		_sceneList[i].exit1 = in.readUint16();
		_sceneList[i].exit2 = in.readUint16();
		_sceneList[i].exit3 = in.readUint16();
		_sceneList[i].exit4 = in.readUint16();
		_sceneList[i].flags = in.readByte();
		_sceneList[i].sound = in.readByte();
	}

	_itemInHand = in.readSint16();

	if (header.originalSave) {
		uint32 currentTime = _system->getMillis();

		for (int i = 0; i < 6; ++i)
			_timer->setDelay(i, in.readSint32LE());

		for (int i = 0; i < 6; ++i) {
			if (in.readUint16LE())
				_timer->enable(i);
			else
				_timer->disable(i);
		}

		for (int i = 0; i < 6; ++i)
			_timer->setNextRun(i, currentTime + (in.readUint32LE() * _tickLength));

		_timer->resetNextRun();
	}

	_sceneExit1 = in.readUint16();
	_sceneExit2 = in.readUint16();
	_sceneExit3 = in.readUint16();
	_sceneExit4 = in.readUint16();

	if (saveFile->err() || saveFile->eos()) {
		warning("Load failed ('%s', '%s').", fileName, header.description.c_str());
		return Common::kUnknownError;
	} else {
		debugC(1, kDebugLevelMain, "Loaded savegame '%s.'", header.description.c_str());
	}

	_loadingState = true;
	updateCharacterAnim(0);
	_loadingState = false;

	if (curShapes != _characterShapeFile)
		loadCharacterShapes(_characterShapeFile);

	_mainCharX = _mainCharacter.x2 = _mainCharacter.x1;
	_mainCharY = _mainCharacter.y2 = _mainCharacter.y1;
	_mainCharacter.facing = 4;
	_badConscienceShown = false;
	_badConsciencePosition = false;
	_goodConscienceShown = false;
	_goodConsciencePosition = false;

	enterNewScene(_mainCharacter.sceneId, _mainCharacter.facing, 0, 0, 1);
	setHandItem(_itemInHand);

	if (_lastMusicCommand >= 0 && !_unkSceneScreenFlag1)
		snd_playWanderScoreViaMap(_lastMusicCommand, 1);
	else if (_lastMusicCommand == -1)
		snd_playWanderScoreViaMap(28, 1);

	while (!_screen->isMouseVisible())
		_screen->showMouse();

	setCommandLineRestoreTimer(7);
	_shownMessage = " ";
	_restoreCommandLine = false;

	// We didn't explicitly set the walk speed, but it's saved as part of
	// the _timers array, so we need to re-sync it with _configWalkspeed.
	setWalkspeed(_configWalkspeed);

	return Common::kNoError;
}

} // End of namespace Kyra
