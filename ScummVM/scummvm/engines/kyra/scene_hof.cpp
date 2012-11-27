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

#include "kyra/kyra_hof.h"
#include "kyra/sound.h"
#include "kyra/resource.h"

#include "common/system.h"

namespace Kyra {

void KyraEngine_HoF::enterNewScene(uint16 newScene, int facing, int unk1, int unk2, int unk3) {
	if (_newChapterFile != _currentTalkFile) {
		_currentTalkFile = _newChapterFile;
		if (_flags.isTalkie) {
			showMessageFromCCode(265, 150, 0);
			_screen->updateScreen();
			openTalkFile(_currentTalkFile);
		}
		showMessage(0, 207);
		_screen->updateScreen();
	}

	_screen->hideMouse();

	if (!unk3) {
		updateWaterFlasks();
		displayInvWsaLastFrame();
	}

	if (unk1) {
		int x = _mainCharacter.x1;
		int y = _mainCharacter.y1;

		switch (facing) {
		case 0:
			y -= 6;
			break;

		case 2:
			x = 335;
			break;

		case 4:
			y = 147;
			break;

		case 6:
			x = -16;
			break;

		default:
			break;
		}

		moveCharacter(facing, x, y);
	}

	// TODO: Check how the original handled sfx still playing
	_sound->stopAllSoundEffects();

	bool newSoundFile = false;
	uint32 waitTime = 0;
	if (_sceneList[newScene].sound != _lastMusicCommand) {
		newSoundFile = true;
		waitTime = _system->getMillis() + 1000;
		_sound->beginFadeOut();
	}

	_chatAltFlag = false;

	if (!unk3) {
		_emc->init(&_sceneScriptState, &_sceneScriptData);
		_emc->start(&_sceneScriptState, 5);
		while (_emc->isValid(&_sceneScriptState))
			_emc->run(&_sceneScriptState);
	}

	Common::for_each(_wsaSlots, ARRAYEND(_wsaSlots), Common::mem_fun(&WSAMovie_v2::close));
	_specialExitCount = 0;
	memset(_specialExitTable, -1, sizeof(_specialExitTable));

	_mainCharacter.sceneId = newScene;
	_sceneList[newScene].flags &= ~1;
	loadScenePal();
	unloadScene();
	loadSceneMsc();

	SceneDesc &scene = _sceneList[newScene];
	_sceneExit1 = scene.exit1;
	_sceneExit2 = scene.exit2;
	_sceneExit3 = scene.exit3;
	_sceneExit4 = scene.exit4;

	if (newSoundFile) {
		if (_sound->getMusicType() == Sound::kAdLib) {
			while (_sound->isPlaying())
				_system->delayMillis(10);
		} else {
			while (waitTime > _system->getMillis())
				_system->delayMillis(10);
		}
		snd_loadSoundFile(_sceneList[newScene].sound);
	}

	startSceneScript(unk3);

	if (_overwriteSceneFacing) {
		facing = _mainCharacter.facing;
		_overwriteSceneFacing = false;
	}

	enterNewSceneUnk1(facing, unk2, unk3);

	setTimer1DelaySecs(-1);
	_sceneScriptState.regs[3] = 1;
	enterNewSceneUnk2(unk3);
	_screen->showMouse();
	_unk5 = 0;
	setNextIdleAnimTimer();

	_currentScene = newScene;
}

void KyraEngine_HoF::enterNewSceneUnk1(int facing, int unk1, int unk2) {
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
		if (y2 >= 141)
			y2 = 139;
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
			y2 = 147;
			break;

		case 2:
			x2 = -16;
			break;

		case 4:
			y2 = y - 4;
			break;

		case 6:
			x2 = 335;
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

	if (!unk2)
		snd_playWanderScoreViaMap(_sceneList[_mainCharacter.sceneId].sound, 0);

	if (unk1 && !unk2 && _mainCharacter.animFrame != 32)
		moveCharacter(facing, x, y);
}

void KyraEngine_HoF::enterNewSceneUnk2(int unk1) {
	_savedMouseState = -1;

	if (_flags.isTalkie) {
		if (_mainCharX == -1 && _mainCharY == -1 && _mainCharacter.sceneId != 61 &&
			!queryGameFlag(0x1F1) && !queryGameFlag(0x192) && !queryGameFlag(0x193) &&
			_mainCharacter.sceneId != 70 && !queryGameFlag(0x159) && _mainCharacter.sceneId != 37) {
			_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
			updateCharacterAnim(0);
			refreshAnimObjectsIfNeed();
		}
	} else if (_mainCharX != -1 && _mainCharY != -1) {
		if (_characterFrameTable[_mainCharacter.facing] == 25)
			_mainCharacter.facing = 5;
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
		updateCharacterAnim(0);
		refreshAnimObjectsIfNeed();
	}

	if (!unk1) {
		runSceneScript4(0);
		zanthSceneStartupChat();
	}

	_unk4 = 0;
	_savedMouseState = -1;
}

int KyraEngine_HoF::trySceneChange(int *moveTable, int unk1, int updateChar) {
	bool running = true;
	bool unkFlag = false;
	int8 updateType = -1;
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
			if (skipFlag()) {
				resetSkipFlag(false);
				running = false;
				_unk4 = 1;
			}
		}

		if (!unkFlag || !running)
			continue;

		int ret = 0;
		if (moveTable == moveTableStart || moveTable[1] == 8)
			ret = updateCharPos(0);
		else
			ret = updateCharPos(moveTable);

		if (ret)
			++moveTable;

		++updateType;
		if (!updateType) {
			update();
		} else if (updateType == 1) {
			refreshAnimObjectsIfNeed();
			updateType = -1;
		}

		delay(10);
	}

	if (updateChar)
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];

	updateCharacterAnim(0);
	refreshAnimObjectsIfNeed();

	return changedScene;
}

int KyraEngine_HoF::checkSceneChange() {
	SceneDesc &curScene = _sceneList[_mainCharacter.sceneId];
	int charX = _mainCharacter.x1, charY = _mainCharacter.y1;
	int facing = 0;
	int process = 0;

	if (_screen->getLayer(charX, charY) == 1 && _savedMouseState == -6) {
		facing = 0;
		process = 1;
	} else if (charX >= 316 && _savedMouseState == -5) {
		facing = 2;
		process = 1;
	} else if (charY >= 142 && _savedMouseState == -4) {
		facing = 4;
		process = 1;
	} else if (charX <= 4 && _savedMouseState == -3) {
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

void KyraEngine_HoF::unloadScene() {
	_emc->unload(&_sceneScriptData);
	freeSceneShapePtrs();
	freeSceneAnims();
}

void KyraEngine_HoF::loadScenePal() {
	uint16 sceneId = _mainCharacter.sceneId;
	_screen->copyPalette(1, 0);

	char filename[14];
	strcpy(filename, _sceneList[sceneId].filename1);
	strcat(filename, ".COL");
	_screen->loadBitmap(filename, 3, 3, 0);
	_screen->getPalette(1).copy(_screen->getCPagePtr(3), 0, 128);
	_screen->getPalette(1).fill(0, 1, 0);
	memcpy(_scenePal, _screen->getCPagePtr(3)+336, 432);
}

void KyraEngine_HoF::loadSceneMsc() {
	uint16 sceneId = _mainCharacter.sceneId;
	char filename[14];
	strcpy(filename, _sceneList[sceneId].filename1);
	strcat(filename, ".MSC");
	_screen->loadBitmap(filename, 3, 5, 0);
}

void KyraEngine_HoF::startSceneScript(int unk1) {
	uint16 sceneId = _mainCharacter.sceneId;
	char filename[14];

	strcpy(filename, _sceneList[sceneId].filename1);
	if (sceneId == 68 && (queryGameFlag(0x1BC) || queryGameFlag(0x1BD)))
		strcpy(filename, "DOORX");
	strcat(filename, ".CPS");

	_screen->loadBitmap(filename, 3, 3, 0);
	resetScaleTable();
	_useCharPal = false;
	memset(_charPalTable, 0, sizeof(_charPalTable));
	memset(_layerFlagTable, 0, sizeof(_layerFlagTable));
	memset(_specialSceneScriptState, 0, sizeof(_specialSceneScriptState));

	_sceneEnterX1 = 160;
	_sceneEnterY1 = 0;
	_sceneEnterX2 = 296;
	_sceneEnterY2 = 72;
	_sceneEnterX3 = 160;
	_sceneEnterY3 = 128;
	_sceneEnterX4 = 24;
	_sceneEnterY4 = 72;

	_sceneCommentString = "Undefined scene comment string!";
	_emc->init(&_sceneScriptState, &_sceneScriptData);

	strcpy(filename, _sceneList[sceneId].filename1);
	strcat(filename, ".");
	strcat(filename, _scriptLangExt[(_flags.platform == Common::kPlatformPC && !_flags.isTalkie) ? 0 : _lang]);

	_res->exists(filename, true);
	_emc->load(filename, &_sceneScriptData, &_opcodes);
	runSceneScript7();

	_emc->start(&_sceneScriptState, 0);
	_sceneScriptState.regs[0] = sceneId;
	_sceneScriptState.regs[5] = unk1;
	while (_emc->isValid(&_sceneScriptState))
		_emc->run(&_sceneScriptState);

	memcpy(_gamePlayBuffer, _screen->getCPagePtr(3), 46080);

	for (int i = 0; i < 10; ++i) {
		_emc->init(&_sceneSpecialScripts[i], &_sceneScriptData);
		_emc->start(&_sceneSpecialScripts[i], i+8);
		_sceneSpecialScriptsTimer[i] = 0;
	}

	_sceneEnterX1 &= ~3;
	_sceneEnterX2 &= ~3;
	_sceneEnterX3 &= ~3;
	_sceneEnterX4 &= ~3;
	_sceneEnterY1 &= ~1;
	_sceneEnterY2 &= ~1;
	_sceneEnterY3 &= ~1;
	_sceneEnterY4 &= ~1;
}

void KyraEngine_HoF::runSceneScript2() {
	_emc->init(&_sceneScriptState, &_sceneScriptData);
	_sceneScriptState.regs[4] = _itemInHand;
	_emc->start(&_sceneScriptState, 2);

	while (_emc->isValid(&_sceneScriptState))
		_emc->run(&_sceneScriptState);
}

void KyraEngine_HoF::runSceneScript4(int unk1) {
	_sceneScriptState.regs[4] = _itemInHand;
	_sceneScriptState.regs[5] = unk1;

	_emc->start(&_sceneScriptState, 4);
	while (_emc->isValid(&_sceneScriptState))
		_emc->run(&_sceneScriptState);
}

void KyraEngine_HoF::runSceneScript7() {
	int oldPage = _screen->_curPage;
	_screen->_curPage = 2;

	_emc->start(&_sceneScriptState, 7);
	while (_emc->isValid(&_sceneScriptState))
		_emc->run(&_sceneScriptState);

	_screen->_curPage = oldPage;
}

void KyraEngine_HoF::initSceneAnims(int unk1) {
	for (int i = 0; i < 41; ++i)
		_animObjects[i].enabled = 0;

	bool animInit = false;

	AnimObj *animState = &_animObjects[0];

	if (_mainCharacter.animFrame != 32)
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];

	animState->enabled = 1;
	animState->xPos1 = _mainCharacter.x1;
	animState->yPos1 = _mainCharacter.y1;
	animState->shapePtr = getShapePtr(_mainCharacter.animFrame);
	animState->shapeIndex1 = animState->shapeIndex2 = _mainCharacter.animFrame;

	int frame = _mainCharacter.animFrame - 9;
	int shapeX = _shapeDescTable[frame].xAdd;
	int shapeY = _shapeDescTable[frame].yAdd;

	animState->xPos2 = _mainCharacter.x1;
	animState->yPos2 = _mainCharacter.y1;

	_charScale = getScale(_mainCharacter.x1, _mainCharacter.y1);

	int shapeXScaled = (shapeX * _charScale) >> 8;
	int shapeYScaled = (shapeY * _charScale) >> 8;

	animState->xPos2 += shapeXScaled;
	animState->yPos2 += shapeYScaled;
	animState->xPos3 = animState->xPos2;
	animState->yPos3 = animState->yPos2;
	animState->needRefresh = 1;
	animState->specialRefresh = 1;

	_animList = 0;

	AnimObj *charAnimState = animState;

	for (int i = 0; i < 10; ++i) {
		animState = &_animObjects[i+1];
		animState->enabled = 0;
		animState->needRefresh = 0;
		animState->specialRefresh = 0;

		if (_sceneAnims[i].flags & 1) {
			animState->enabled = 1;
			animState->needRefresh = 1;
			animState->specialRefresh = 1;
		}

		animState->animFlags = _sceneAnims[i].flags & 8;

		if (_sceneAnims[i].flags & 2)
			animState->flags = 0x800;
		else
			animState->flags = 0;

		if (_sceneAnims[i].flags & 4)
			animState->flags |= 1;

		animState->xPos1 = _sceneAnims[i].x;
		animState->yPos1 = _sceneAnims[i].y;

		if (_sceneAnims[i].flags & 0x20)
			animState->shapePtr = _sceneShapeTable[_sceneAnims[i].shapeIndex];
		else
			animState->shapePtr = 0;

		if (_sceneAnims[i].flags & 0x40) {
			animState->shapeIndex3 = _sceneAnims[i].shapeIndex;
			animState->animNum = i;
		} else {
			animState->shapeIndex3 = 0xFFFF;
			animState->animNum = 0xFFFF;
		}

		animState->shapeIndex2 = 0xFFFF;

		animState->xPos3 = animState->xPos2 = _sceneAnims[i].x2;
		animState->yPos3 = animState->yPos2 = _sceneAnims[i].y2;
		animState->width = _sceneAnims[i].width;
		animState->height = _sceneAnims[i].height;
		animState->width2 = animState->height2 = _sceneAnims[i].specialSize;

		if (_sceneAnims[i].flags & 1) {
			if (animInit) {
				_animList = addToAnimListSorted(_animList, animState);
			} else {
				_animList = initAnimList(_animList, animState);
				animInit = true;
			}
		}
	}

	if (animInit) {
		_animList = addToAnimListSorted(_animList, charAnimState);
	} else {
		_animList = initAnimList(_animList, charAnimState);
		animInit = true;
	}

	for (int i = 0; i < 30; ++i) {
		animState = &_animObjects[i+11];

		uint16 shapeIndex = _itemList[i].id;
		if (shapeIndex == 0xFFFF || _itemList[i].sceneId != _mainCharacter.sceneId) {
			animState->enabled = 0;
			animState->needRefresh = 0;
			animState->specialRefresh = 0;
		} else {
			animState->xPos1 = _itemList[i].x;
			animState->yPos1 = _itemList[i].y;
			animState->shapePtr = getShapePtr(64+shapeIndex);
			animState->shapeIndex1 = animState->shapeIndex2 = shapeIndex+64;

			animState->xPos2 = _itemList[i].x;
			animState->yPos2 = _itemList[i].y;
			int objectScale = getScale(animState->xPos2, animState->yPos2);

			const uint8 *shape = getShapePtr(animState->shapeIndex1);
			animState->xPos2 -= (_screen->getShapeScaledWidth(shape, objectScale) >> 1);
			animState->yPos2 -= (_screen->getShapeScaledHeight(shape, objectScale) >> 1);
			animState->xPos3 = animState->xPos2;
			animState->yPos3 = animState->yPos2;

			animState->enabled = 1;
			animState->needRefresh = 1;
			animState->specialRefresh = 1;

			if (animInit) {
				_animList = addToAnimListSorted(_animList, animState);
			} else {
				_animList = initAnimList(_animList, animState);
				animInit = true;
			}
		}
	}

	_animObjects[0].specialRefresh = 1;
	_animObjects[0].needRefresh = 1;

	for (int i = 1; i < 41; ++i) {
		if (_animObjects[i].enabled) {
			_animObjects[i].needRefresh = 1;
			_animObjects[i].specialRefresh = 1;
		}
	}

	restorePage3();
	drawAnimObjects();
	_screen->hideMouse();
	initSceneScreen(unk1);
	_screen->showMouse();
	refreshAnimObjects(0);
}

void KyraEngine_HoF::initSceneScreen(int unk1) {
	if (_unkSceneScreenFlag1) {
		_screen->copyRegion(0, 0, 0, 0, 320, 144, 2, 0, Screen::CR_NO_P_CHECK);
		return;
	}

	if (_noScriptEnter) {
		_screen->getPalette(0).fill(0, 128, 0);
		_screen->setScreenPalette(_screen->getPalette(0));
	}

	_screen->copyRegion(0, 0, 0, 0, 320, 144, 2, 0, Screen::CR_NO_P_CHECK);

	if (_noScriptEnter) {
		_screen->setScreenPalette(_screen->getPalette(1));
		_screen->getPalette(0).copy(_screen->getPalette(1), 0, 128);
	}

	updateCharPal(0);

	_emc->start(&_sceneScriptState, 3);
	_sceneScriptState.regs[5] = unk1;
	while (_emc->isValid(&_sceneScriptState))
		_emc->run(&_sceneScriptState);
}

void KyraEngine_HoF::freeSceneShapePtrs() {
	for (int i = 0; i < ARRAYSIZE(_sceneShapeTable); ++i)
		delete[] _sceneShapeTable[i];
	memset(_sceneShapeTable, 0, sizeof(_sceneShapeTable));
}

void KyraEngine_HoF::fadeScenePal(int srcIndex, int delayTime) {
	_screen->getPalette(0).copy(_scenePal, srcIndex << 4, 16, 112);
	_screen->fadePalette(_screen->getPalette(0), delayTime, &_updateFunctor);
}

#pragma mark -
#pragma mark - Pathfinder
#pragma mark -

bool KyraEngine_HoF::lineIsPassable(int x, int y) {
	static const int widthTable[] = { 1, 1, 1, 1, 1, 2, 4, 6, 8 };

	if (_pathfinderFlag & 2) {
		if (x >= 320)
			return false;
	}

	if (_pathfinderFlag & 4) {
		if (y >= 144)
			return false;
	}

	if (_pathfinderFlag & 8) {
		if (x < 0)
			return false;
	}

	if (y > 143)
		return false;

	int unk1 = widthTable[getScale(x, y) >> 5];

	if (y < 0)
		y = 0;
	x -= unk1 >> 1;
	if (x < 0)
		x = 0;
	int x2 = x + unk1;
	if (x2 > 320)
		x2 = 320;

	for (;x < x2; ++x)
		if (!_screen->getShapeFlag1(x, y))
			return false;

	return true;
}

} // End of namespace Kyra
