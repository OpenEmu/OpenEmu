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

#include "kyra/kyra_mr.h"
#include "kyra/screen_mr.h"
#include "kyra/sound_digital.h"
#include "kyra/resource.h"

#include "common/system.h"

namespace Kyra {

void KyraEngine_MR::enterNewScene(uint16 sceneId, int facing, int unk1, int unk2, int unk3) {
	++_enterNewSceneLock;
	_screen->hideMouse();

	showMessage(0, 0xF0, 0xF0);
	if (_inventoryState)
		hideInventory();

	if (_currentChapter != _currentTalkFile) {
		_currentTalkFile = _currentChapter;
		openTalkFile(_currentTalkFile);
	}

	if (unk1) {
		int x = _mainCharacter.x1;
		int y = _mainCharacter.y1;

		switch (facing) {
		case 0:
			y -= 6;
			break;

		case 2:
			x = 343;
			break;

		case 4:
			y = 191;
			break;

		case 6:
			x = -24;
			break;
		}

		moveCharacter(facing, x, y);
	}

	uint32 waitUntilTimer = 0;
	if (_lastMusicCommand != _sceneList[sceneId].sound) {
		fadeOutMusic(60);
		waitUntilTimer = _system->getMillis() + 60 * _tickLength;
	}

	_chatAltFlag = false;

	if (!unk3) {
		_emc->init(&_sceneScriptState, &_sceneScriptData);
		_emc->start(&_sceneScriptState, 5);
		while (_emc->isValid(&_sceneScriptState))
			_emc->run(&_sceneScriptState);
	}

	_specialExitCount = 0;
	Common::fill(_specialExitTable, ARRAYEND(_specialExitTable), 0xFFFF);

	_mainCharacter.sceneId = sceneId;
	_sceneList[sceneId].flags &= ~1;
	unloadScene();

	for (int i = 0; i < 4; ++i) {
		if (i != _musicSoundChannel && i != _fadeOutMusicChannel)
			_soundDigital->stopSound(i);
	}
	_fadeOutMusicChannel = -1;
	loadScenePal();

	if (queryGameFlag(0x1D9)) {
		char filename[20];
		if (queryGameFlag(0x20D)) {
			resetGameFlag(0x20D);
			strcpy(filename, "COW1_");
		} else if (queryGameFlag(0x20E)) {
			resetGameFlag(0x20E);
			strcpy(filename, "COW2_");
		} else if (queryGameFlag(0x20F)) {
			resetGameFlag(0x20F);
			strcpy(filename, "COW3_");
		} else if (queryGameFlag(0x20C)) {
			resetGameFlag(0x20C);
			strcpy(filename, "BOAT");
		} else if (queryGameFlag(0x210)) {
			resetGameFlag(0x210);
			strcpy(filename, "JUNG");
		}

		playVQA(filename);

		resetGameFlag(0x1D9);
	}

	loadSceneMsc();
	_sceneExit1 = _sceneList[sceneId].exit1;
	_sceneExit2 = _sceneList[sceneId].exit2;
	_sceneExit3 = _sceneList[sceneId].exit3;
	_sceneExit4 = _sceneList[sceneId].exit4;

	while (_system->getMillis() < waitUntilTimer)
		_system->delayMillis(10);

	initSceneScript(unk3);

	if (_overwriteSceneFacing) {
		facing = _mainCharacter.facing;
		_overwriteSceneFacing = false;
	}

	enterNewSceneUnk1(facing, unk2, unk3);
	setCommandLineRestoreTimer(-1);
	_sceneScriptState.regs[3] = 1;
	enterNewSceneUnk2(unk3);
	if (queryGameFlag(0)) {
		_showOutro = true;
		_runFlag = false;
	} else {
		if (!--_enterNewSceneLock)
			_unk5 = 0;

		setNextIdleAnimTimer();

		if (_itemInHand < 0) {
			_itemInHand = kItemNone;
			_mouseState = kItemNone;
			_screen->setMouseCursor(0, 0, _gameShapes[0]);
		}

		Common::Point pos = getMousePos();
		if (pos.y > 187)
			setMousePos(pos.x, 179);
	}
	_screen->showMouse();

	_currentScene = sceneId;
}

void KyraEngine_MR::enterNewSceneUnk1(int facing, int unk1, int unk2) {
	int x = 0, y = 0;
	int x2 = 0, y2 = 0;
	bool needProc = true;

	if (_mainCharX == -1 && _mainCharY == -1) {
		switch (facing+1) {
		case 1: case 2: case 8:
			x2 = _sceneEnterX3;
			y2 = _sceneEnterY3;
			break;

		case 3:
			x2 = _sceneEnterX4;
			y2 = _sceneEnterY4;
			break;

		case 4: case 5: case 6:
			x2 = _sceneEnterX1;
			y2 = _sceneEnterY1;
			break;

		case 7:
			x2 = _sceneEnterX2;
			y2 = _sceneEnterY2;
			break;

		default:
			x2 = y2 = -1;
		}

		if (x2 >= 316)
			x2 = 312;
		if (y2 >= 185)
			y2 = 183;
		if (x2 <= 4)
			x2 = 8;
	}

	if (_mainCharX >= 0) {
		x = x2 = _mainCharX;
		needProc = false;
	}

	if (_mainCharY >= 0) {
		y = y2 = _mainCharY;
		needProc = false;
	}

	_mainCharX = _mainCharY = -1;

	if (unk1 && needProc) {
		x = x2;
		y = y2;

		switch (facing) {
		case 0:
			y2 = 191;
			break;

		case 2:
			x2 = -24;
			break;

		case 4:
			y2 = y - 4;
			break;

		case 6:
			x2 = 343;
			break;

		default:
			break;
		}
	}

	x2 &= ~3;
	x &= ~3;
	y2 &= ~1;
	y &= ~1;

	_mainCharacter.facing = facing;
	_mainCharacter.x1 = _mainCharacter.x2 = x2;
	_mainCharacter.y1 = _mainCharacter.y2 = y2;
	initSceneAnims(unk2);

	if (_mainCharacter.sceneId == 9 && !_soundDigital->isPlaying(_musicSoundChannel))
		snd_playWanderScoreViaMap(_sceneList[_mainCharacter.sceneId].sound, 0);
	if (!unk2)
		snd_playWanderScoreViaMap(_sceneList[_mainCharacter.sceneId].sound, 0);

	if (unk1 && !unk2 && _mainCharacter.animFrame != 87)
		moveCharacter(facing, x, y);
}

void KyraEngine_MR::enterNewSceneUnk2(int unk1) {
	_savedMouseState = -1;
	if (_mainCharX == -1 && _mainCharY == -1 && !unk1) {
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
		updateCharacterAnim(0);
		refreshAnimObjectsIfNeed();
	}

	if (!unk1) {
		runSceneScript4(0);
		malcolmSceneStartupChat();
	}

	_unk4 = 0;
	_savedMouseState = -1;
}

void KyraEngine_MR::unloadScene() {
	delete[] _sceneStrings;
	_sceneStrings = 0;
	_emc->unload(&_sceneScriptData);
	freeSceneShapes();
	freeSceneAnims();
}

void KyraEngine_MR::freeSceneShapes() {
	for (uint i = 0; i < ARRAYSIZE(_sceneShapes); ++i) {
		delete[] _sceneShapes[i];
		_sceneShapes[i] = 0;
	}
}

void KyraEngine_MR::loadScenePal() {
	char filename[16];
	_screen->copyPalette(2, 0);
	strcpy(filename, _sceneList[_mainCharacter.sceneId].filename1);
	strcat(filename, ".COL");

	_screen->loadBitmap(filename, 3, 3, 0);
	_screen->getPalette(2).copy(_screen->getCPagePtr(3), 0, 144);
	_screen->getPalette(2).fill(0, 1, 0);

	for (int i = 144; i <= 167; ++i) {
		uint8 *palette = _screen->getPalette(2).getData() + i * 3;
		palette[0] = palette[2] = 63;
		palette[1] = 0;
	}

	_screen->generateOverlay(_screen->getPalette(2), _paletteOverlay, 0xF0, 0x19);

	_screen->getPalette(2).copy(_costPalBuffer, _characterShapeFile * 24, 24, 144);
}

void KyraEngine_MR::loadSceneMsc() {
	char filename[16];
	strcpy(filename, _sceneList[_mainCharacter.sceneId].filename1);
	strcat(filename, ".MSC");

	_res->exists(filename, true);
	Common::SeekableReadStream *stream = _res->createReadStream(filename);
	assert(stream);
	int16 minY = 0, height = 0;
	minY = stream->readSint16LE();
	height = stream->readSint16LE();
	delete stream;
	stream = 0;
	_maskPageMinY = minY;
	_maskPageMaxY = minY + height - 1;

	_screen->setShapePages(5, 3, _maskPageMinY, _maskPageMaxY);

	_screen->loadBitmap(filename, 5, 5, 0, true);

	// HACK
	uint8 *data = new uint8[320*200];
	_screen->copyRegionToBuffer(5, 0, 0, 320, 200, data);
	_screen->clearPage(5);
	_screen->copyBlockToPage(5, 0, _maskPageMinY, 320, height, data);
	delete[] data;

}

void KyraEngine_MR::initSceneScript(int unk1) {
	const SceneDesc &scene = _sceneList[_mainCharacter.sceneId];

	char filename[16];
	strcpy(filename, scene.filename1);
	strcat(filename, ".DAT");

	_res->exists(filename, true);
	Common::SeekableReadStream *stream = _res->createReadStream(filename);
	assert(stream);
	stream->seek(2, SEEK_CUR);

	byte scaleTable[15];
	stream->read(scaleTable, 15);
	stream->read(_sceneDatPalette, 45);
	stream->read(_sceneDatLayerTable, 15);
	int16 shapesCount = stream->readSint16LE();

	for (int i = 0; i < 15; ++i)
		_scaleTable[i] = (uint16(scaleTable[i]) << 8) / 100;

	if (shapesCount > 0) {
		strcpy(filename, scene.filename1);
		strcat(filename, "9.CPS");
		_screen->loadBitmap(filename, 3, 3, 0);
		int pageBackUp = _screen->_curPage;
		_screen->_curPage = 2;
		for (int i = 0; i < shapesCount; ++i) {
			int16 x = stream->readSint16LE();
			int16 y = stream->readSint16LE();
			int16 w = stream->readSint16LE();
			int16 h = stream->readSint16LE();
			_sceneShapeDescs[i].drawX = stream->readSint16LE();
			_sceneShapeDescs[i].drawY = stream->readSint16LE();
			_sceneShapes[i] = _screen->encodeShape(x, y, w, h, 0);
			assert(_sceneShapes[i]);
		}
		_screen->_curPage = pageBackUp;
	}
	delete stream;
	stream = 0;

	strcpy(filename, scene.filename1);
	strcat(filename, ".CPS");
	_screen->loadBitmap(filename, 3, 3, 0);

	Common::fill(_specialSceneScriptState, ARRAYEND(_specialSceneScriptState), false);
	_sceneEnterX1 = 160;
	_sceneEnterY1 = 0;
	_sceneEnterX2 = 296;
	_sceneEnterY2 = 93;
	_sceneEnterX3 = 160;
	_sceneEnterY3 = 171;
	_sceneEnterX4 = 24;
	_sceneEnterY4 = 93;
	_sceneMinX = 0;
	_sceneMaxX = 319;

	_emc->init(&_sceneScriptState, &_sceneScriptData);
	strcpy(filename, scene.filename2);
	strcat(filename, ".EMC");
	_res->exists(filename, true);
	_emc->load(filename, &_sceneScriptData, &_opcodes);

	strcpy(filename, scene.filename2);
	strcat(filename, ".");
	loadLanguageFile(filename, _sceneStrings);

	runSceneScript8();
	_emc->start(&_sceneScriptState, 0);
	_sceneScriptState.regs[0] = _mainCharacter.sceneId;
	_sceneScriptState.regs[5] = unk1;
	while (_emc->isValid(&_sceneScriptState))
		_emc->run(&_sceneScriptState);

	_screen->copyRegionToBuffer(3, 0, 0, 320, 200, _gamePlayBuffer);

	for (int i = 0; i < 10; ++i) {
		_emc->init(&_sceneSpecialScripts[i], &_sceneScriptData);
		_emc->start(&_sceneSpecialScripts[i], i+9);
		_sceneSpecialScriptsTimer[i] = 0;
	}

	_sceneEnterX1 &= ~3;
	_sceneEnterY1 &= ~1;
	_sceneEnterX2 &= ~3;
	_sceneEnterY2 &= ~1;
	_sceneEnterX3 &= ~3;
	_sceneEnterY3 &= ~1;
	_sceneEnterX4 &= ~3;
	_sceneEnterY4 &= ~1;
}

void KyraEngine_MR::initSceneAnims(int unk1) {
	for (int i = 0; i < 67; ++i)
		_animObjects[i].enabled = false;

	AnimObj *obj = &_animObjects[0];

	if (_mainCharacter.animFrame != 87 && !unk1)
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];

	obj->enabled = true;
	obj->xPos1 = _mainCharacter.x1;
	obj->yPos1 = _mainCharacter.y1;
	obj->shapePtr = getShapePtr(_mainCharacter.animFrame);
	obj->shapeIndex2 = obj->shapeIndex1 = _mainCharacter.animFrame;
	obj->xPos2 = _mainCharacter.x1;
	obj->yPos2 = _mainCharacter.y1;
	_charScale = getScale(_mainCharacter.x1, _mainCharacter.y1);
	obj->xPos3 = obj->xPos2 += (_malcolmShapeXOffset * _charScale) >> 8;
	obj->yPos3 = obj->yPos2 += (_malcolmShapeYOffset * _charScale) >> 8;
	_mainCharacter.x3 = _mainCharacter.x1 - (_charScale >> 4) - 1;
	_mainCharacter.y3 = _mainCharacter.y1 - (_charScale >> 6) - 1;
	obj->needRefresh = true;
	_animList = 0;

	for (int i = 0; i < 16; ++i) {
		const SceneAnim &anim = _sceneAnims[i];
		obj = &_animObjects[i+1];
		obj->enabled = false;
		obj->needRefresh = false;

		if (anim.flags & 1) {
			obj->enabled = true;
			obj->needRefresh = true;
		}

		obj->specialRefresh = (anim.flags & 0x20) ? 1 : 0;
		obj->flags = (anim.flags & 0x10) ? 0x800 : 0;
		if (anim.flags & 2)
			obj->flags |= 1;

		obj->xPos1 = anim.x;
		obj->yPos1 = anim.y;

		if ((anim.flags & 4) && anim.shapeIndex != -1)
			obj->shapePtr = _sceneShapes[anim.shapeIndex];
		else
			obj->shapePtr = 0;

		if (anim.flags & 8) {
			obj->shapeIndex3 = anim.shapeIndex;
			obj->animNum = i;
		} else {
			obj->shapeIndex3 = 0xFFFF;
			obj->animNum = 0xFFFF;
		}

		obj->xPos3 = obj->xPos2 = anim.x2;
		obj->yPos3 = obj->yPos2 = anim.y2;
		obj->width = anim.width;
		obj->height = anim.height;
		obj->width2 = obj->height2 = anim.specialSize;

		if (anim.flags & 1) {
			if (_animList)
				_animList = addToAnimListSorted(_animList, obj);
			else
				_animList = initAnimList(_animList, obj);
		}
	}

	if (_animList)
		_animList = addToAnimListSorted(_animList, &_animObjects[0]);
	else
		_animList = initAnimList(_animList, &_animObjects[0]);

	for (int i = 0; i < 50; ++i) {
		obj = &_animObjects[i+17];
		const ItemDefinition &item = _itemList[i];
		if (item.id != kItemNone && item.sceneId == _mainCharacter.sceneId) {
			obj->xPos1 = item.x;
			obj->yPos1 = item.y;
			animSetupPaletteEntry(obj);
			obj->shapePtr = 0;
			obj->shapeIndex1 = obj->shapeIndex2 = item.id + 248;
			obj->xPos2 = item.x;
			obj->yPos2 = item.y;

			int scale = getScale(obj->xPos1, obj->yPos1);
			const uint8 *shape = getShapePtr(obj->shapeIndex1);
			obj->xPos3 = obj->xPos2 -= (_screen->getShapeScaledWidth(shape, scale) >> 1);
			obj->yPos3 = obj->yPos2 -= _screen->getShapeScaledHeight(shape, scale) - 1;
			obj->enabled = true;
			obj->needRefresh = true;

			if (_animList)
				_animList = addToAnimListSorted(_animList, obj);
			else
				_animList = initAnimList(_animList, obj);
		} else {
			obj->enabled = false;
			obj->needRefresh = false;
		}
	}

	for (int i = 0; i < 67; ++i)
		_animObjects[i].needRefresh = _animObjects[i].enabled;

	restorePage3();
	drawAnimObjects();
	_screen->hideMouse();
	initSceneScreen(unk1);
	_screen->showMouse();
	refreshAnimObjects(0);
}

void KyraEngine_MR::initSceneScreen(int unk1) {
	_screen->copyBlockToPage(2, 0, 188, 320, 12, _interfaceCommandLine);

	if (_unkSceneScreenFlag1) {
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);
		return;
	}

	if (_noScriptEnter) {
		_screen->getPalette(0).fill(0, 144, 0);
		if (!_wasPlayingVQA)
			_screen->setScreenPalette(_screen->getPalette(0));
	}

	_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);

	if (_noScriptEnter) {
		if (!_wasPlayingVQA)
			_screen->setScreenPalette(_screen->getPalette(2));
		_screen->getPalette(0).copy(_screen->getPalette(2), 0, 144);
		if (_wasPlayingVQA) {
			_screen->fadeFromBlack(0x3C);
			_wasPlayingVQA = false;
		}
	}

	updateCharPal(0);

	if (!_menuDirectlyToLoad) {
		_emc->start(&_sceneScriptState, 3);
		_sceneScriptState.regs[5] = unk1;

		while (_emc->isValid(&_sceneScriptState))
			_emc->run(&_sceneScriptState);
	}
}

int KyraEngine_MR::trySceneChange(int *moveTable, int unk1, int updateChar) {
	bool running = true;
	bool unkFlag = false;
	int changedScene = 0;
	const int *moveTableStart = moveTable;
	_unk4 = 0;

	while (running && !shouldQuit()) {
		if (*moveTable >= 0 && *moveTable <= 7) {
			_mainCharacter.facing = getOppositeFacingDirection(*moveTable);
			unkFlag = true;
		} else {
			if (*moveTable == 8) {
				running = false;
			} else {
				++moveTable;
				unkFlag = false;
			}
		}

		if (checkSceneChange()) {
			running = false;
			changedScene = 1;
		}

		if (unk1) {
			// Notice that we can't use KyraEngine_MR's skipFlag handling
			// here, since Kyra3 allows disabling of skipFlag support
			if (KyraEngine_v2::skipFlag()) {
				resetSkipFlag(false);
				running = false;
				_unk4 = 1;
			}
		}

		if (!unkFlag || !running)
			continue;

		int ret = 0;
		if (moveTable == moveTableStart || moveTable[1] == 8)
			ret = updateCharPos(0, 0);
		else
			ret = updateCharPos(moveTable, 0);

		if (ret)
			++moveTable;

		delay(10, true);
	}

	if (updateChar)
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];

	updateCharacterAnim(0);
	refreshAnimObjectsIfNeed();

	return changedScene;
}

int KyraEngine_MR::checkSceneChange() {
	const SceneDesc &curScene = _sceneList[_mainCharacter.sceneId];
	int charX = _mainCharacter.x1, charY = _mainCharacter.y1;
	int facing = 0;
	int process = 0;

	if (_screen->getLayer(charX, charY) == 1 && _savedMouseState == -7) {
		facing = 0;
		process = 1;
	} else if (charX >= 316 && _savedMouseState == -6) {
		facing = 2;
		process = 1;
	} else if (charY >= 186 && _savedMouseState == -5) {
		facing = 4;
		process = 1;
	} else if (charX <= 4 && _savedMouseState == -4) {
		facing = 6;
		process = 1;
	}

	if (!process)
		return 0;

	uint16 newScene = 0xFFFF;
	switch (facing) {
	case 0:
		newScene = curScene.exit1;
		break;

	case 2:
		newScene = curScene.exit2;
		break;

	case 4:
		newScene = curScene.exit3;
		break;

	case 6:
		newScene = curScene.exit4;
		break;

	default:
		newScene = _mainCharacter.sceneId;
	}

	if (newScene == 0xFFFF)
		return 0;

	enterNewScene(newScene, facing, 1, 1, 0);
	return 1;
}
int KyraEngine_MR::runSceneScript1(int x, int y) {
	if (y > 187 && _savedMouseState > -4)
		return 0;
	if (_deathHandler >= 0)
		return 0;

	_emc->init(&_sceneScriptState, &_sceneScriptData);
	_sceneScriptState.regs[1] = x;
	_sceneScriptState.regs[2] = y;
	_sceneScriptState.regs[3] = 0;
	_sceneScriptState.regs[4] = _itemInHand;

	_emc->start(&_sceneScriptState, 1);
	while (_emc->isValid(&_sceneScriptState))
		_emc->run(&_sceneScriptState);

	return _sceneScriptState.regs[3];
}

int KyraEngine_MR::runSceneScript2() {
	_sceneScriptState.regs[1] = _mouseX;
	_sceneScriptState.regs[2] = _mouseY;
	_sceneScriptState.regs[3] = 0;
	_sceneScriptState.regs[4] = _itemInHand;

	_emc->start(&_sceneScriptState, 2);
	while (_emc->isValid(&_sceneScriptState))
		_emc->run(&_sceneScriptState);

	return _sceneScriptState.regs[3];
}

void KyraEngine_MR::runSceneScript4(int unk1) {
	_sceneScriptState.regs[4] = _itemInHand;
	_sceneScriptState.regs[5] = unk1;
	_sceneScriptState.regs[3] = 0;
	_noStartupChat = false;

	_emc->start(&_sceneScriptState, 4);
	while (_emc->isValid(&_sceneScriptState))
		_emc->run(&_sceneScriptState);

	if (_sceneScriptState.regs[3])
		_noStartupChat = true;
}

void KyraEngine_MR::runSceneScript8() {
	_emc->start(&_sceneScriptState, 8);
	while (_emc->isValid(&_sceneScriptState))
		_emc->run(&_sceneScriptState);
}

bool KyraEngine_MR::lineIsPassable(int x, int y) {
	static const uint8 widthTable[] = { 1, 1, 1, 1, 1, 2, 4, 6, 8 };

	if ((_pathfinderFlag & 2) && x >= 320)
		return false;
	if ((_pathfinderFlag & 4) && y >= 188)
		return false;
	if ((_pathfinderFlag & 8) && x < 0)
		return false;
	if (y > 187)
		return false;

	uint width = widthTable[getScale(x, y) >> 5];

	if (y < 0)
		y = 0;
	x -= width >> 1;
	if (x < 0)
		x = 0;
	int x2 = x + width;
	if (x2 > 320)
		x2 = 320;

	for (; x < x2; ++x) {
		if (y < _maskPageMinY || y > _maskPageMaxY)
			return false;

		if (!_screen->getShapeFlag1(x, y))
			return false;
	}

	return true;
}

} // End of namespace Kyra
