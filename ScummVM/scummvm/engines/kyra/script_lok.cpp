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
#include "kyra/sprites.h"
#include "kyra/wsamovie.h"
#include "kyra/animator_lok.h"
#include "kyra/text.h"
#include "kyra/timer.h"
#include "kyra/sound.h"

#include "common/system.h"

namespace Kyra {

int KyraEngine_LoK::o1_magicInMouseItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_magicInMouseItem(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	magicInMouseItem(stackPos(0), stackPos(1), -1);
	return 0;
}

int KyraEngine_LoK::o1_characterSays(EMCState *script) {
	resetSkipFlag();
	if (_flags.isTalkie) {
		debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_characterSays(%p) (%d, '%s', %d, %d)", (const void *)script, stackPos(0), stackPosString(1), stackPos(2), stackPos(3));
		characterSays(stackPos(0), stackPosString(1), stackPos(2), stackPos(3));
	} else {
		debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_characterSays(%p) ('%s', %d, %d)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2));
		const char *string = stackPosString(0);

		if ((_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98) && _flags.lang == Common::JA_JPN) {
			static const uint8 townsString1[] = {
				0x83, 0x75, 0x83, 0x89, 0x83, 0x93, 0x83, 0x83, 0x93, 0x81,
				0x41, 0x82, 0xDC, 0x82, 0xBD, 0x97, 0x88, 0x82, 0xBD, 0x82,
				0xCC, 0x82, 0xA9, 0x81, 0x48, 0x00, 0x00, 0x00
			};
			static const uint8 townsString2[] = {
				0x83, 0x75, 0x83, 0x89, 0x83, 0x93, 0x83, 0x5C, 0x83, 0x93,
				0x81, 0x41, 0x82, 0xDC, 0x82, 0xBD, 0x97, 0x88, 0x82, 0xBD,
				0x82, 0xCC, 0x82, 0xA9, 0x81, 0x48, 0x00, 0x00
			};

			if (strncmp((const char *)townsString1, string, sizeof(townsString1)) == 0)
				string = (const char *)townsString2;
		}

		characterSays(-1, string, stackPos(1), stackPos(2));
	}

	return 0;
}

int KyraEngine_LoK::o1_delay(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_delay(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	if (stackPos(1)) {
		warning("STUB: special o1_delay");
		// delete this after correct implementing
		delayWithTicks(stackPos(0));
	} else {
		delayWithTicks(stackPos(0));
	}
	return 0;
}

int KyraEngine_LoK::o1_drawSceneAnimShape(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_drawSceneAnimShape(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	_screen->drawShape(stackPos(4), _sprites->_sceneShapes[stackPos(0)], stackPos(1), stackPos(2), 0, (stackPos(3) != 0) ? 1 : 0);
	return 0;
}

int KyraEngine_LoK::o1_runNPCScript(EMCState *script) {
	warning("STUB: o1_runNPCScript");
	return 0;
}

int KyraEngine_LoK::o1_setSpecialExitList(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setSpecialExitList(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8), stackPos(9));

	for (int i = 0; i < 10; ++i)
		_exitList[i] = stackPos(i);
	_exitListPtr = _exitList;

	return 0;
}

int KyraEngine_LoK::o1_walkPlayerToPoint(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_walkPlayerToPoint(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));

	int normalTimers = stackPos(2);
	if (!normalTimers) {
		_timer->disable(19);
		_timer->disable(14);
		_timer->disable(18);
	}

	int reinitScript = handleSceneChange(stackPos(0), stackPos(1), stackPos(2), stackPos(3));

	if (!normalTimers) {
		_timer->enable(19);
		_timer->enable(14);
		_timer->enable(18);
	}

	if (reinitScript)
		_emc->init(script, script->dataPtr);

	if (_sceneChangeState) {
		_sceneChangeState = 0;
		return 1;
	}
	return 0;
}

int KyraEngine_LoK::o1_dropItemInScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_dropItemInScene(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	int item = stackPos(0);
	int xpos = stackPos(1);
	int ypos = stackPos(2);

	byte freeItem = findFreeItemInScene(_currentCharacter->sceneId);
	if (freeItem != 0xFF) {
		int sceneId = _currentCharacter->sceneId;
		Room *room = &_roomTable[sceneId];
		room->itemsXPos[freeItem] = xpos;
		room->itemsYPos[freeItem] = ypos;
		room->itemsTable[freeItem] = item;

		_animator->animAddGameItem(freeItem, sceneId);
		_animator->updateAllObjectShapes();
	} else {
		if (item == 43)
			placeItemInGenericMapScene(item, 0);
		else
			placeItemInGenericMapScene(item, 1);
	}
	return 0;
}

int KyraEngine_LoK::o1_drawAnimShapeIntoScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_drawAnimShapeIntoScene(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	_animator->restoreAllObjectBackgrounds();
	int shape = stackPos(0);
	int xpos = stackPos(1);
	int ypos = stackPos(2);
	int flags = (stackPos(3) != 0) ? 1 : 0;
	_screen->drawShape(2, _sprites->_sceneShapes[shape], xpos, ypos, 0, flags);
	_screen->drawShape(0, _sprites->_sceneShapes[shape], xpos, ypos, 0, flags);
	_animator->flagAllObjectsForBkgdChange();
	_animator->preserveAnyChangedBackgrounds();
	_animator->flagAllObjectsForRefresh();
	_animator->updateAllObjectShapes();
	return 0;
}

int KyraEngine_LoK::o1_savePageToDisk(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_savePageToDisk(%p) ('%s', %d)", (const void *)script, stackPosString(0), stackPos(1));
	_screen->savePageToDisk(stackPosString(0), stackPos(1));
	return 0;
}

int KyraEngine_LoK::o1_sceneAnimOn(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_sceneAnimOn(%p) (%d)", (const void *)script, stackPos(0));
	_sprites->_anims[stackPos(0)].play = true;
	return 0;
}

int KyraEngine_LoK::o1_sceneAnimOff(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_sceneAnimOff(%p) (%d)", (const void *)script, stackPos(0));
	_sprites->_anims[stackPos(0)].play = false;
	return 0;
}

int KyraEngine_LoK::o1_getElapsedSeconds(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_getElapsedSeconds(%p) ()", (const void *)script);
	return _system->getMillis() / 1000;
}

int KyraEngine_LoK::o1_mouseIsPointer(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_mouseIsPointer(%p) ()", (const void *)script);
	return (_itemInHand == kItemNone);
}

int KyraEngine_LoK::o1_runSceneAnimUntilDone(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_runSceneAnimUntilDone(%p) (%d)", (const void *)script, stackPos(0));
	_screen->hideMouse();
	_animator->restoreAllObjectBackgrounds();
	_sprites->_anims[stackPos(0)].play = true;
	_animator->sprites()[stackPos(0)].active = 1;
	_animator->flagAllObjectsForBkgdChange();
	_animator->preserveAnyChangedBackgrounds();
	while (_sprites->_anims[stackPos(0)].play) {
		_sprites->updateSceneAnims();
		_animator->updateAllObjectShapes();
		delay(10);
	}
	_animator->restoreAllObjectBackgrounds();
	_screen->showMouse();
	return 0;
}

int KyraEngine_LoK::o1_fadeSpecialPalette(EMCState *script) {
	if (_flags.platform == Common::kPlatformAmiga) {
		debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_fadeSpecialPalette(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
		if (_currentCharacter->sceneId != 45) {
			if (stackPos(0) == 13) {
				_screen->copyPalette(0, 12);
				_screen->setScreenPalette(_screen->getPalette(0));
			}
		} else {
			setupZanthiaPalette(stackPos(0));
			_screen->getPalette(0).copy(_screen->getPalette(4), 12, 1);
			_screen->fadePalette(_screen->getPalette(0), 2);
		}
	} else {
		debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_fadeSpecialPalette(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
		_screen->fadeSpecialPalette(stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	}
	return 0;
}

int KyraEngine_LoK::o1_phaseInSameScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_phaseInSameScene(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	transcendScenes(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_LoK::o1_setScenePhasingFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setScenePhasingFlag(%p) ()", (const void *)script);
	_scenePhasingFlag = 1;
	return 1;
}

int KyraEngine_LoK::o1_resetScenePhasingFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_resetScenePhasingFlag(%p) ()", (const void *)script);
	_scenePhasingFlag = 0;
	return 0;
}

int KyraEngine_LoK::o1_queryScenePhasingFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_queryScenePhasingFlag(%p) ()", (const void *)script);
	return _scenePhasingFlag;
}

int KyraEngine_LoK::o1_sceneToDirection(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_sceneToDirection(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	assert(stackPos(0) < _roomTableSize);
	Room *curRoom = &_roomTable[stackPos(0)];
	uint16 returnValue = 0xFFFF;
	switch (stackPos(1)) {
	case 0:
		returnValue = curRoom->northExit;
		break;

	case 2:
		returnValue = curRoom->eastExit;
		break;

	case 4:
		returnValue = curRoom->southExit;
		break;

	case 6:
		returnValue = curRoom->westExit;
		break;

	default:
		break;
	}
	if (returnValue == 0xFFFF)
		return -1;
	return returnValue;
}

int KyraEngine_LoK::o1_setBirthstoneGem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setBirthstoneGem(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	int index = stackPos(0);
	if (index < 4 && index >= 0) {
		_birthstoneGemTable[index] = stackPos(1);
		return 1;
	}
	return 0;
}

int KyraEngine_LoK::o1_placeItemInGenericMapScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_placeItemInGenericMapScene(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	placeItemInGenericMapScene(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_LoK::o1_setBrandonStatusBit(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setBrandonStatusBit(%p) (%d)", (const void *)script, stackPos(0));
	_brandonStatusBit |= stackPos(0);
	return 0;
}

int KyraEngine_LoK::o1_delaySecs(EMCState *script) {
	if (_flags.isTalkie && speechEnabled()) {
		debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_voiceDelay(%p) (%d)", (const void *)script, stackPos(0));
		if (stackPos(0) == 0) {
			snd_voiceWaitForFinish(true);
		} else if (stackPos(0) < 0) {
			uint32 time = ABS(stackPos(0)) * _tickLength;
			delay(time, true);
		}
	} else {
		debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_delaySecs(%p) (%d)", (const void *)script, stackPos(0));
		if (stackPos(0) >= 0 && !skipFlag())
			delay(stackPos(0) * 1000, true);
	}

	resetSkipFlag();
	return 0;
}

int KyraEngine_LoK::o1_getCharacterScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_getCharacterScene(%p) (%d)", (const void *)script, stackPos(0));
	return _characterList[stackPos(0)].sceneId;
}

int KyraEngine_LoK::o1_runNPCSubscript(EMCState *script) {
	warning("STUB: o1_runNPCSubscript");
	return 0;
}

int KyraEngine_LoK::o1_magicOutMouseItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_magicOutMouseItem(%p) (%d)", (const void *)script, stackPos(0));
	magicOutMouseItem(stackPos(0), -1);
	return 0;
}

int KyraEngine_LoK::o1_internalAnimOn(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_internalAnimOn(%p) (%d)", (const void *)script, stackPos(0));
	_animator->sprites()[stackPos(0)].active = 1;
	return 0;
}

int KyraEngine_LoK::o1_forceBrandonToNormal(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_forceBrandonToNormal(%p) ()", (const void *)script);
	checkAmuletAnimFlags();
	return 0;
}

int KyraEngine_LoK::o1_poisonDeathNow(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_poisonDeathNow(%p) ()", (const void *)script);
	seq_poisonDeathNow(1);
	return 0;
}

int KyraEngine_LoK::o1_setScaleMode(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setScaleMode(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	int len = stackPos(0);
	int setValue1 = stackPos(1);
	int start2 = stackPos(2);
	int setValue2 = stackPos(3);
	for (int i = 0; i < len; ++i)
		_scaleTable[i] = setValue1;
	int temp = setValue2 - setValue1;
	int temp2 = start2 - len;
	for (int i = len, offset = 0; i < start2; ++i, ++offset)
		_scaleTable[i] = (offset * temp) / temp2 + setValue1;
	for (int i = start2; i < 145; ++i)
		_scaleTable[i] = setValue2;
	_scaleMode = 1;
	return _scaleMode;
}

int KyraEngine_LoK::o1_openWSAFile(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_openWSAFile(%p) ('%s', %d, %d, %d)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2), stackPos(3));

	const char *filename = stackPosString(0);
	int wsaIndex = stackPos(1);

	_movieObjects[wsaIndex]->open(filename, (stackPos(3) != 0) ? 1 : 0, 0);
	assert(_movieObjects[wsaIndex]->opened());

	return 0;
}

int KyraEngine_LoK::o1_closeWSAFile(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_closeWSAFile(%p) (%d)", (const void *)script, stackPos(0));

	int wsaIndex = stackPos(0);
	if (_movieObjects[wsaIndex])
		_movieObjects[wsaIndex]->close();

	return 0;
}

int KyraEngine_LoK::o1_runWSAFromBeginningToEnd(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_runWSAFromBeginningToEnd(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));

	_screen->hideMouse();

	bool running = true;

	int xpos = stackPos(0);
	int ypos = stackPos(1);
	int waitTime = stackPos(2);
	int wsaIndex = stackPos(3);
	int worldUpdate = stackPos(4);
	int wsaFrame = 0;

	while (running) {
		const uint32 continueTime = waitTime * _tickLength + _system->getMillis();

		_movieObjects[wsaIndex]->displayFrame(wsaFrame++, 0, xpos, ypos, 0, 0, 0);
		if (wsaFrame >= _movieObjects[wsaIndex]->frames())
			running = false;

		delayUntil(continueTime, false, worldUpdate != 0);
	}

	_screen->showMouse();

	return 0;
}

int KyraEngine_LoK::o1_displayWSAFrame(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_displayWSAFrame(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	int frame = stackPos(0);
	int xpos = stackPos(1);
	int ypos = stackPos(2);
	int waitTime = stackPos(3);
	int wsaIndex = stackPos(4);
	_screen->hideMouse();
	const uint32 continueTime = waitTime * _tickLength + _system->getMillis();
	_movieObjects[wsaIndex]->displayFrame(frame, 0, xpos, ypos, 0, 0, 0);
	delayUntil(continueTime, false, true);
	_screen->showMouse();
	return 0;
}

int KyraEngine_LoK::o1_enterNewScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_enterNewScene(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	enterNewScene(stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	return 0;
}

int KyraEngine_LoK::o1_setSpecialEnterXAndY(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setSpecialEnterXAndY(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_brandonPosX = stackPos(0);
	_brandonPosY = stackPos(1);
	if (_brandonPosX + 1 == 0 && _brandonPosY + 1 == 0)
		_currentCharacter->currentAnimFrame = 88;
	return 0;
}

int KyraEngine_LoK::o1_runWSAFrames(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_runWSAFrames(%p) (%d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	int xpos = stackPos(0);
	int ypos = stackPos(1);
	int delayTime = stackPos(2);
	int startFrame = stackPos(3);
	int endFrame = stackPos(4);
	int wsaIndex = stackPos(5);
	_screen->hideMouse();
	for (; startFrame <= endFrame; ++startFrame) {
		const uint32 nextRun = _system->getMillis() + delayTime * _tickLength;
		_movieObjects[wsaIndex]->displayFrame(startFrame, 0, xpos, ypos, 0, 0, 0);
		delayUntil(nextRun, false, true);
	}
	_screen->showMouse();
	return 0;
}

int KyraEngine_LoK::o1_popBrandonIntoScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_popBrandonIntoScene(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	int changeScaleMode = stackPos(3);
	int xpos = (int16)(stackPos(0) & 0xFFFC);
	int ypos = (int16)(stackPos(1) & 0xFFFE);
	int facing = stackPos(2);
	_currentCharacter->x1 = _currentCharacter->x2 = xpos;
	_currentCharacter->y1 = _currentCharacter->y2 = ypos;
	_currentCharacter->facing = facing;
	_currentCharacter->currentAnimFrame = 7;
	int xOffset = _defaultShapeTable[0].xOffset;
	int yOffset = _defaultShapeTable[0].yOffset;
	int width = _defaultShapeTable[0].w << 3;
	int height = _defaultShapeTable[0].h;
	Animator_LoK::AnimObject *curAnim = _animator->actors();

	if (changeScaleMode) {
		curAnim->x1 = _currentCharacter->x1;
		curAnim->y1 = _currentCharacter->y1;
		_animator->_brandonScaleY = _scaleTable[_currentCharacter->y1];
		_animator->_brandonScaleX = _animator->_brandonScaleY;

		int animWidth = _animator->fetchAnimWidth(curAnim->sceneAnimPtr, _animator->_brandonScaleX) >> 1;
		int animHeight = _animator->fetchAnimHeight(curAnim->sceneAnimPtr, _animator->_brandonScaleY);

		animWidth = (xOffset * animWidth) /  width;
		animHeight = (yOffset * animHeight) / height;

		curAnim->x2 = curAnim->x1 += animWidth;
		curAnim->y2 = curAnim->y1 += animHeight;
	} else {
		curAnim->x2 = curAnim->x1 = _currentCharacter->x1 + xOffset;
		curAnim->y2 = curAnim->y1 = _currentCharacter->y1 + yOffset;
	}

	int scaleModeBackup = _scaleMode;
	if (changeScaleMode)
		_scaleMode = 1;

	_animator->animRefreshNPC(0);
	_animator->preserveAllBackgrounds();
	_animator->prepDrawAllObjects();
	_animator->copyChangedObjectsForward(0);

	_scaleMode = scaleModeBackup;

	return 0;
}

int KyraEngine_LoK::o1_restoreAllObjectBackgrounds(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_restoreAllObjectBackgrounds(%p) (%d)", (const void *)script, stackPos(0));
	int disable = stackPos(0);
	int activeBackup = 0;
	if (disable) {
		activeBackup = _animator->actors()->active;
		_animator->actors()->active = 0;
	}
	_animator->restoreAllObjectBackgrounds();
	if (disable)
		_animator->actors()->active = activeBackup;
	return 0;
}

int KyraEngine_LoK::o1_setCustomPaletteRange(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setCustomPaletteRange(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	if (_flags.platform == Common::kPlatformAmiga) {
		if (_currentCharacter->sceneId == 45) {
			setupZanthiaPalette(stackPos(0));
		} else if (stackPos(0) == 29) {
			_screen->copyPalette(0, 11);
		} else if (stackPos(0) == 13) {
			_screen->copyPalette(0, 12);
		}
	} else {
		if (!_specialPalettes[stackPos(0)])
			warning("KyraEngine_LoK::o1_setCustomPaletteRange(): Trying to use missing special palette %d", stackPos(0));
		else
			_screen->getPalette(1).copy(_specialPalettes[stackPos(0)], 0, stackPos(2), stackPos(1));
	}
	return 0;
}

int KyraEngine_LoK::o1_loadPageFromDisk(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_loadPageFromDisk(%p) ('%s', %d)", (const void *)script, stackPosString(0), stackPos(1));
	_screen->loadPageFromDisk(stackPosString(0), stackPos(1));
	return 0;
}

int KyraEngine_LoK::o1_customPrintTalkString(EMCState *script) {
	if (_flags.isTalkie) {
		debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_customPrintTalkString(%p) (%d, '%s', %d, %d, %d)", (const void *)script, stackPos(0), stackPosString(1), stackPos(2), stackPos(3), stackPos(4) & 0xFF);

		if (speechEnabled()) {
			snd_voiceWaitForFinish();
			snd_playVoiceFile(stackPos(0));
		}

		resetSkipFlag();
		if (textEnabled())
			_text->printTalkTextMessage(stackPosString(1), stackPos(2), stackPos(3), stackPos(4) & 0xFF, 0, 2);
	} else {
		debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_customPrintTalkString(%p) ('%s', %d, %d, %d)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2), stackPos(3) & 0xFF);
		resetSkipFlag();
		_text->printTalkTextMessage(stackPosString(0), stackPos(1), stackPos(2), stackPos(3) & 0xFF, 0, 2);
	}
	_screen->updateScreen();
	return 0;
}

int KyraEngine_LoK::o1_restoreCustomPrintBackground(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_restoreCustomPrintBackground(%p) ()", (const void *)script);
	_text->restoreTalkTextMessageBkgd(2, 0);
	return 0;
}

int KyraEngine_LoK::o1_getCharacterX(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_getCharacterX(%p) (%d)", (const void *)script, stackPos(0));
	return _characterList[stackPos(0)].x1;
}

int KyraEngine_LoK::o1_getCharacterY(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_getCharacterY(%p) (%d)", (const void *)script, stackPos(0));
	return _characterList[stackPos(0)].y1;
}

int KyraEngine_LoK::o1_setCharacterFacing(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setCharacterFacing(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	int character = stackPos(0);
	int facing = stackPos(1);
	int newAnimFrame = stackPos(2);

	_animator->restoreAllObjectBackgrounds();
	if (newAnimFrame != -1)
		_characterList[character].currentAnimFrame = newAnimFrame;
	_characterList[character].facing = facing;
	_animator->animRefreshNPC(character);
	_animator->preserveAllBackgrounds();
	_animator->prepDrawAllObjects();
	_animator->copyChangedObjectsForward(0);

	return 0;
}

int KyraEngine_LoK::o1_copyWSARegion(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_copyWSARegion(%p) (%d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	int xpos = stackPos(0);
	int ypos = stackPos(1);
	int width = stackPos(2);
	int height = stackPos(3);
	int srcPage = stackPos(4);
	int dstPage = stackPos(5);
	_screen->copyRegion(xpos, ypos, xpos, ypos, width, height, srcPage, dstPage);
	return 0;
}

int KyraEngine_LoK::o1_printText(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_printText(%p) ('%s', %d, %d, 0x%X, 0x%X)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	if (_flags.lang == Common::JA_JPN && stackPos(3) == 7)
		_screen->printText(stackPosString(0), stackPos(1), stackPos(2), 0, 0x80);
	else
		_screen->printText(stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	_screen->updateScreen();
	return 0;
}

int KyraEngine_LoK::o1_loadSoundFile(EMCState *script) {
	warning("STUB: o1_loadSoundFile");
	return 0;
}

int KyraEngine_LoK::o1_displayWSAFrameOnHidPage(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_displayWSAFrameOnHidPage(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	int frame = stackPos(0);
	int xpos = stackPos(1);
	int ypos = stackPos(2);
	int waitTime = stackPos(3);
	int wsaIndex = stackPos(4);

	_screen->hideMouse();
	const uint32 continueTime = waitTime * _tickLength + _system->getMillis();
	_movieObjects[wsaIndex]->displayFrame(frame, 2, xpos, ypos, 0, 0, 0);
	delayUntil(continueTime, false, true);
	_screen->showMouse();

	return 0;
}

int KyraEngine_LoK::o1_displayWSASequentialFrames(EMCState *script) {
	if (_flags.isTalkie)
		debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_displayWSASequentialFrames(%p) (%d, %d, %d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7));
	else
		debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_displayWSASequentialFrames(%p) (%d, %d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6));
	int startFrame = stackPos(0);
	int endFrame = stackPos(1);
	int xpos = stackPos(2);
	int ypos = stackPos(3);
	int waitTime = stackPos(4);
	int wsaIndex = stackPos(5);
	int maxTime = stackPos(6);

	if (_flags.isTalkie) {
		int specialTime = stackPos(7);
		if (specialTime) {
			uint32 voiceTime = snd_getVoicePlayTime();
			if (voiceTime) {
				int displayFrames = ABS(endFrame - startFrame) + 1;
				displayFrames *= maxTime;
				assert(displayFrames != 0);

				bool voiceSync = false;

				if (specialTime < 0) {
					voiceSync = true;
					specialTime = ABS(specialTime);
				}

				voiceTime *= specialTime;
				voiceTime /= 100;

				if (voiceSync) {
					uint32 voicePlayedTime = _sound->voicePlayedTime(_speechHandle);
					if (voicePlayedTime >= voiceTime)
						voiceTime = 0;
					else
						voiceTime -= voicePlayedTime;
				}

				waitTime = voiceTime / displayFrames;
				waitTime /= _tickLength;
			}
		}
	}

	if (maxTime - 1 <= 0)
		maxTime = 1;

	// WORKAROUND for bug #1498221 "KYRA1: Glitches when meeting Zanthia".
	// The original did not do a forced screen update after displaying a WSA
	// frame while we have to do it, which makes Brandon disappear for a short
	// moment. That is not supposed to happen. So we're not updating the
	// screen for this special case.
	// This is only an issue for the CD version, but since the floppy version
	// does not use the specified paramaeters like these, it is safe to enable
	// it for all versions.
	if (startFrame == 18 && endFrame == 18 && waitTime == 10 && wsaIndex == 0 && _currentRoom == 45) {
		_movieObjects[wsaIndex]->displayFrame(18, 0, xpos, ypos, 0, 0, 0);
		// We call delayMillis manually here to avoid the screen getting
		// updated.
		_system->delayMillis(waitTime * _tickLength);
		return 0;
	}

	int curTime = 0;
	_screen->hideMouse();
	while (curTime < maxTime) {
		if (endFrame >= startFrame) {
			int frame = startFrame;
			while (endFrame >= frame) {
				const uint32 continueTime = waitTime * _tickLength + _system->getMillis();
				_movieObjects[wsaIndex]->displayFrame(frame, 0, xpos, ypos, 0, 0, 0);
				delayUntil(continueTime, false, true);
				++frame;
			}
		} else {
			int frame = startFrame;
			while (endFrame <= frame) {
				const uint32 continueTime = waitTime * _tickLength + _system->getMillis();
				_movieObjects[wsaIndex]->displayFrame(frame, 0, xpos, ypos, 0, 0, 0);
				delayUntil(continueTime, false, true);
				--frame;
			}
		}

		if (skipFlag())
			break;
		else
			++curTime;
	}
	_screen->showMouse();

	return 0;
}

int KyraEngine_LoK::o1_refreshCharacter(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_refreshCharacter(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	int character = stackPos(0);
	int animFrame = stackPos(1);
	int newFacing = stackPos(2);
	int updateShapes = stackPos(3);
	_characterList[character].currentAnimFrame = animFrame;
	if (newFacing != -1)
		_characterList[character].facing = newFacing;
	_animator->animRefreshNPC(character);
	if (updateShapes)
		_animator->updateAllObjectShapes();
	return 0;
}

int KyraEngine_LoK::o1_internalAnimOff(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_internalAnimOff(%p) (%d)", (const void *)script, stackPos(0));
	_animator->sprites()[stackPos(0)].active = 0;
	return 0;
}

int KyraEngine_LoK::o1_changeCharactersXAndY(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_changeCharactersXAndY(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	Character *ch = &_characterList[stackPos(0)];
	int16 x = stackPos(1);
	int16 y = stackPos(2);
	if (x != -1 && y != -1) {
		x &= 0xFFFC;
		y &= 0xFFFE;
	}
	_animator->restoreAllObjectBackgrounds();
	ch->x1 = ch->x2 = x;
	ch->y1 = ch->y2 = y;
	_animator->preserveAllBackgrounds();
	return 0;
}

int KyraEngine_LoK::o1_clearSceneAnimatorBeacon(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_clearSceneAnimatorBeacon(%p) ()", (const void *)script);
	_sprites->_sceneAnimatorBeaconFlag = 0;
	return 0;
}

int KyraEngine_LoK::o1_querySceneAnimatorBeacon(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_querySceneAnimatorBeacon(%p) ()", (const void *)script);
	return _sprites->_sceneAnimatorBeaconFlag;
}

int KyraEngine_LoK::o1_refreshSceneAnimator(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_refreshSceneAnimator(%p) ()", (const void *)script);
	_sprites->updateSceneAnims();
	_animator->updateAllObjectShapes();
	return 0;
}

int KyraEngine_LoK::o1_placeItemInOffScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_placeItemInOffScene(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	int item = stackPos(0);
	int xpos = stackPos(1);
	int ypos = stackPos(2);
	int sceneId = stackPos(3);

	byte freeItem = findFreeItemInScene(sceneId);
	if (freeItem != 0xFF) {
		assert(sceneId < _roomTableSize);
		Room *room = &_roomTable[sceneId];

		room->itemsTable[freeItem] = item;
		room->itemsXPos[freeItem] = xpos;
		room->itemsYPos[freeItem] = ypos;
	}
	return 0;
}

int KyraEngine_LoK::o1_wipeDownMouseItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_wipeDownMouseItem(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	_screen->hideMouse();
	wipeDownMouseItem(stackPos(1), stackPos(2));
	removeHandItem();
	_screen->showMouse();
	return 0;
}

int KyraEngine_LoK::o1_placeCharacterInOtherScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_placeCharacterInOtherScene(%p) (%d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	int id = stackPos(0);
	int sceneId = stackPos(1);
	int xpos = (int16)(stackPos(2) & 0xFFFC);
	int ypos = (int16)(stackPos(3) & 0xFFFE);
	int facing = stackPos(4);
	int animFrame = stackPos(5);

	_characterList[id].sceneId = sceneId;
	_characterList[id].x1 = _characterList[id].x2 = xpos;
	_characterList[id].y1 = _characterList[id].y2 = ypos;
	_characterList[id].facing = facing;
	_characterList[id].currentAnimFrame = animFrame;
	return 0;
}

int KyraEngine_LoK::o1_getKey(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_getKey(%p) ()", (const void *)script);

	while (true) {
		delay(10);

		if (skipFlag())
			break;
	}

	resetSkipFlag();
	return 0;
}

int KyraEngine_LoK::o1_specificItemInInventory(EMCState *script) {
	warning("STUB: o1_specificItemInInventory");
	return 0;
}

int KyraEngine_LoK::o1_popMobileNPCIntoScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_popMobileNPCIntoScene(%p) (%d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), (int16)(stackPos(4) & 0xFFFC), (int8)(stackPos(5) & 0xFE));
	int character = stackPos(0);
	int sceneId = stackPos(1);
	int animFrame = stackPos(2);
	int facing = stackPos(3);
	int16 xpos = (int16)(stackPos(4) & 0xFFFC);
	int8 ypos = (int16)(stackPos(5) & 0xFFFE);
	Character *curChar = &_characterList[character];

	curChar->sceneId = sceneId;
	curChar->currentAnimFrame = animFrame;
	curChar->facing = facing;
	curChar->x1 = curChar->x2 = xpos;
	curChar->y1 = curChar->y2 = ypos;

	_animator->animAddNPC(character);
	_animator->updateAllObjectShapes();
	return 0;
}

int KyraEngine_LoK::o1_mobileCharacterInScene(EMCState *script) {
	warning("STUB: o1_mobileCharacterInScene");
	return 0;
}

int KyraEngine_LoK::o1_hideMobileCharacter(EMCState *script) {
	warning("STUB: o1_hideMobileCharacter");
	return 0;
}

int KyraEngine_LoK::o1_unhideMobileCharacter(EMCState *script) {
	warning("STUB: o1_unhideMobileCharacter");
	return 0;
}

int KyraEngine_LoK::o1_setCharacterLocation(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setCharacterLocation(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	Character *ch = &_characterList[stackPos(0)];
	Animator_LoK::AnimObject *animObj = &_animator->actors()[stackPos(0)];
	int newScene = stackPos(1);
	if (_currentCharacter->sceneId == ch->sceneId) {
		if (_currentCharacter->sceneId != newScene)
			animObj->active = 0;
	} else if (_currentCharacter->sceneId == newScene) {
		if (_currentCharacter->sceneId != ch->sceneId)
			animObj->active = 1;
	}

	ch->sceneId = stackPos(1);
	return 0;
}

int KyraEngine_LoK::o1_walkCharacterToPoint(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_walkCharacterToPoint(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	int character = stackPos(0);
	int toX = stackPos(1);
	int toY = stackPos(2);
	_pathfinderFlag2 = 1;
	uint32 nextFrame;
	int findWayReturn = findWay(_characterList[character].x1, _characterList[character].y1, toX, toY, _movFacingTable, 150);
	_pathfinderFlag2 = 0;

	if (_lastFindWayRet < findWayReturn)
		_lastFindWayRet = findWayReturn;
	if (findWayReturn == 0x7D00 || findWayReturn == 0)
		return 0;

	int *curPos = _movFacingTable;
	bool running = true;
	while (running) {
		bool forceContinue = false;
		switch (*curPos) {
		case 0:
			_characterList[character].facing = 2;
			break;

		case 1:
			_characterList[character].facing = 1;
			break;

		case 2:
			_characterList[character].facing = 0;
			break;

		case 3:
			_characterList[character].facing = 7;
			break;

		case 4:
			_characterList[character].facing = 6;
			break;

		case 5:
			_characterList[character].facing = 5;
			break;

		case 6:
			_characterList[character].facing = 4;
			break;

		case 7:
			_characterList[character].facing = 3;
			break;

		case 8:
			running = 0;
			break;

		default:
			++curPos;
			forceContinue = true;
		}

		if (forceContinue || !running)
			continue;

		setCharacterPosition(character, 0);
		++curPos;

		delayUntil(nextFrame = _timer->getDelay(5 + character) * _tickLength + _system->getMillis(), true, true);
	}
	return 0;
}

int KyraEngine_LoK::o1_specialEventDisplayBrynnsNote(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_specialEventDisplayBrynnsNote(%p) ()", (const void *)script);
	_screen->hideMouse();
	_screen->savePageToDisk("HIDPAGE.TMP", 2);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);
	if (_flags.isTalkie) {
		if (_flags.lang == Common::EN_ANY || _flags.lang == Common::IT_ITA)
			_screen->loadBitmap("NOTEENG.CPS", 3, 3, 0);
		else if (_flags.lang == Common::FR_FRA)
			_screen->loadBitmap("NOTEFRE.CPS", 3, 3, 0);
		else if (_flags.lang == Common::DE_DEU)
			_screen->loadBitmap("NOTEGER.CPS", 3, 3, 0);
	} else {
		_screen->loadBitmap("NOTE.CPS", 3, 3, 0);
	}
	_screen->copyRegion(63, 8, 63, 8, 194, 128, 2, 0);
	_screen->updateScreen();
	_screen->showMouse();

	if (_flags.platform != Common::kPlatformAmiga && !_flags.isTalkie && _flags.lang != Common::JA_JPN)
		_screen->setFont(Screen::FID_6_FNT);
	return 0;
}

int KyraEngine_LoK::o1_specialEventRemoveBrynnsNote(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_specialEventRemoveBrynnsNote(%p) ()", (const void *)script);
	_screen->hideMouse();
	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->loadPageFromDisk("HIDPAGE.TMP", 2);
	_screen->updateScreen();
	_screen->showMouse();

	if (_flags.platform != Common::kPlatformAmiga && !_flags.isTalkie && _flags.lang != Common::JA_JPN)
		_screen->setFont(Screen::FID_8_FNT);
	return 0;
}

int KyraEngine_LoK::o1_setLogicPage(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setLogicPage(%p) (%d)", (const void *)script, stackPos(0));
	_screen->_curPage = stackPos(0);
	return stackPos(0);
}

int KyraEngine_LoK::o1_fatPrint(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_fatPrint(%p) ('%s', %d, %d, %d, %d, %d)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));

	// Workaround for bug #1582672 ("KYRA1: Text crippled and drawn wrong")
	// I'm not sure how the original handles this, since it seems to call
	// printText also, maybe it fails somewhere inside...
	// TODO: fix the reason for this workaround
	if (_currentRoom == 117)
		return 0;
	_text->printText(stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	return 0;
}

int KyraEngine_LoK::o1_preserveAllObjectBackgrounds(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_preserveAllObjectBackgrounds(%p) ()", (const void *)script);
	_animator->preserveAllBackgrounds();
	return 0;
}

int KyraEngine_LoK::o1_updateSceneAnimations(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_updateSceneAnimations(%p) (%d)", (const void *)script, stackPos(0));
	int times = stackPos(0);
	while (times--) {
		_sprites->updateSceneAnims();
		_animator->updateAllObjectShapes();
	}
	return 0;
}

int KyraEngine_LoK::o1_sceneAnimationActive(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_sceneAnimationActive(%p) (%d)", (const void *)script, stackPos(0));
	return _sprites->_anims[stackPos(0)].play;
}

int KyraEngine_LoK::o1_setCharacterMovementDelay(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setCharacterMovementDelay(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_timer->setDelay(stackPos(0) + 5, stackPos(1));
	return 0;
}

int KyraEngine_LoK::o1_getCharacterFacing(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_getCharacterFacing(%p) (%d)", (const void *)script, stackPos(0));
	return _characterList[stackPos(0)].facing;
}

int KyraEngine_LoK::o1_bkgdScrollSceneAndMasksRight(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_bkgdScrollSceneAndMasksRight(%p) (%d)", (const void *)script, stackPos(0));
	_screen->copyBackgroundBlock(stackPos(0), 2, 0);
	_screen->copyBackgroundBlock2(stackPos(0));
	// update the whole screen
	_screen->copyRegion(7, 7, 7, 7, 305, 129, 3, 0);
	// Don't do a screen update here, see bug #1910180 "KYRA1: Screen 'flash'"
	// it would cause to draw the screen with a wrong palette and thus look
	// strange for the user. Since this opcode should be just called on scene
	// initialization anyway, there should be no problem with not updating the
	// screen right now.
	return 0;
}

int KyraEngine_LoK::o1_dispelMagicAnimation(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_dispelMagicAnimation(%p) ()", (const void *)script);
	seq_dispelMagicAnimation();
	return 0;
}

int KyraEngine_LoK::o1_findBrightestFireberry(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_findBrightestFireberry(%p) ()", (const void *)script);
	if (_currentCharacter->sceneId >= 187 && _currentCharacter->sceneId <= 198)
		return 29;

	// The following rooms are only a "A fireberry bush" scene in the CD version
	// of Kyrandia 1. In all other versions they are a usual dark cave, thus we do only
	// return a glow value of "29" over here, when we are running a CD version.
	if (_flags.isTalkie) {
		if (_currentCharacter->sceneId == 133 || _currentCharacter->sceneId == 137 ||
		        _currentCharacter->sceneId == 165 || _currentCharacter->sceneId == 173)
			return 29;
	}

	if (_itemInHand == 28)
		return 28;

	int brightestFireberry = 107;
	if (_itemInHand >= 29 && _itemInHand <= 33)
		brightestFireberry = _itemInHand;
	for (int i = 0; i < 10; ++i) {
		uint8 item = _currentCharacter->inventoryItems[i];
		if (item == 0xFF)
			continue;
		if (item == 28)
			return 28;
		if (item >= 29 && item <= 33) {
			if (item < brightestFireberry)
				brightestFireberry = item;
		}
	}
	assert(_currentCharacter->sceneId < _roomTableSize);
	Room *curRoom = &_roomTable[_currentCharacter->sceneId];
	for (int i = 0; i < 12; ++i) {
		uint8 item = curRoom->itemsTable[i];
		if (item == 0xFF)
			continue;
		if (item == 28)
			return 28;
		if (item >= 29 && item <= 33) {
			if (item < brightestFireberry)
				brightestFireberry = item;
		}
	}
	if (brightestFireberry == 107)
		return -1;
	return brightestFireberry;
}

int KyraEngine_LoK::o1_setFireberryGlowPalette(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setFireberryGlowPalette(%p) (%d)", (const void *)script, stackPos(0));

	if (_flags.platform == Common::kPlatformAmiga) {
		int palIndex = 0;

		switch (stackPos(0)) {
		case -1:
			// The original seemed to draw some lines on page 2 here, which looks strange...
			//if (!(_brandonStatusBit & 2))
			//  warning("Unimplemented case for o1_setFireberryGlowPalette");
			palIndex = 9;
			break;

		case 30:
			palIndex = 7;
			break;

		case 31:
			palIndex = 8;
			break;

		case 32:
		case 33:
			palIndex = 9;
			break;

	case 28: case 29: default:
			palIndex = 6;
		}

		if (_brandonStatusBit & 2) {
			if (_currentCharacter->sceneId < 187 || _currentCharacter->sceneId > 198)
				palIndex = 10;
		}

		_screen->copyPalette(0, palIndex);
	} else {
		int palIndex = 0;

		switch (stackPos(0)) {
		case 0x1E:
			palIndex = 9;
			break;

		case 0x1F:
			palIndex = 10;
			break;

		case 0x20:
			palIndex = 11;
			break;

		case 0x21:
		case -1:
			palIndex = 12;
			break;

		default:
			palIndex = 8;
		}

		if (_brandonStatusBit & 2) {
			if (_currentCharacter->sceneId != 133 && _currentCharacter->sceneId != 137 &&
			        _currentCharacter->sceneId != 165 && _currentCharacter->sceneId != 173 &&
			        (_currentCharacter->sceneId < 187 || _currentCharacter->sceneId > 198)) {
				palIndex = 14;
			}
		}

		_screen->getPalette(1).copy(_specialPalettes[palIndex], 0, 15, 228);
	}

	return 0;
}

int KyraEngine_LoK::o1_drinkPotionAnimation(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_drinkPotionAnimation(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	seq_playDrinkPotionAnim(stackPos(0), stackPos(1), stackPos(2));
	return 0;
}

int KyraEngine_LoK::o1_makeAmuletAppear(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_makeAmuletAppear(%p) ()", (const void *)script);
	Movie *amulet = createWSAMovie();
	assert(amulet);
	amulet->open("AMULET.WSA", 1, 0);

	if (amulet->opened()) {
		assert(_amuleteAnim);
		_screen->hideMouse();
		snd_playSoundEffect(0x70);
		for (int i = 0; _amuleteAnim[i] != 0xFF; ++i) {
			const uint32 nextTime = _system->getMillis() + 5 * _tickLength;

			uint8 code = _amuleteAnim[i];
			if (code == 3 || code == 7)
				snd_playSoundEffect(0x71);

			if (code == 5)
				snd_playSoundEffect(0x72);

			if (code == 14)
				snd_playSoundEffect(0x73);

			amulet->displayFrame(code, 0, 224, 152, 0, 0, 0);
			delayUntil(nextTime, false, true);
		}
		_screen->showMouse();
	}

	delete amulet;
	setGameFlag(0x2D);
	return 0;
}

int KyraEngine_LoK::o1_drawItemShapeIntoScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_drawItemShapeIntoScene(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	int item = stackPos(0);
	int x = stackPos(1);
	int y = stackPos(2);
	int flags = stackPos(3);
	int onlyHidPage = stackPos(4);

	if (flags)
		flags = 1;

	if (onlyHidPage) {
		_screen->drawShape(2, _shapes[216 + item], x, y, 0, flags);
	} else {
		_animator->restoreAllObjectBackgrounds();
		_screen->drawShape(2, _shapes[216 + item], x, y, 0, flags);
		_screen->drawShape(0, _shapes[216 + item], x, y, 0, flags);
		_animator->flagAllObjectsForBkgdChange();
		_animator->preserveAnyChangedBackgrounds();
		_animator->flagAllObjectsForRefresh();
		_animator->updateAllObjectShapes();
	}
	return 0;
}

int KyraEngine_LoK::o1_setCharacterCurrentFrame(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setCharacterCurrentFrame(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_characterList[stackPos(0)].currentAnimFrame = stackPos(1);
	return 0;
}

int KyraEngine_LoK::o1_waitForConfirmationMouseClick(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_waitForConfirmationMouseClick(%p) ()", (const void *)script);

	_eventList.clear();
	while (true) {
		updateMousePointer();
		_sprites->updateSceneAnims();
		_animator->updateAllObjectShapes();

		updateInput();

		int input = checkInput(0, false) & 0xFF;
		removeInputTop();
		if (input == 200)
			break;

		delay(10);
	}

	script->regs[1] = _mouseX;
	script->regs[2] = _mouseY;

	return 0;
}

int KyraEngine_LoK::o1_pageFlip(EMCState *script) {
	warning("STUB: o1_pageFlip");
	return 0;
}

int KyraEngine_LoK::o1_setSceneFile(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setSceneFile(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	setSceneFile(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_LoK::o1_getItemInMarbleVase(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_getItemInMarbleVase(%p) ()", (const void *)script);
	return _marbleVaseItem;
}

int KyraEngine_LoK::o1_setItemInMarbleVase(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setItemInMarbleVase(%p) (%d)", (const void *)script, stackPos(0));
	_marbleVaseItem = stackPos(0);
	return 0;
}

int KyraEngine_LoK::o1_addItemToInventory(EMCState *script) {
	warning("STUB: o1_addItemToInventory");
	return 0;
}

int KyraEngine_LoK::o1_intPrint(EMCState *script) {
	warning("STUB: o1_intPrint");
	return 0;
}

int KyraEngine_LoK::o1_shakeScreen(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_shakeScreen(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	int waitTicks = stackPos(1);
	int times = stackPos(0);

	for (int i = 0; i < times; ++i) {
		_screen->shakeScreen(1);
		delay(waitTicks * _tickLength);
	}

	return 0;
}

int KyraEngine_LoK::o1_createAmuletJewel(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_createAmuletJewel(%p) (%d)", (const void *)script, stackPos(0));
	seq_createAmuletJewel(stackPos(0), 0, 0, 0);
	return 0;
}

int KyraEngine_LoK::o1_setSceneAnimCurrXY(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setSceneAnimCurrXY(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	_sprites->_anims[stackPos(0)].x = stackPos(1);
	_sprites->_anims[stackPos(0)].y = stackPos(2);
	return 0;
}

int KyraEngine_LoK::o1_poisonBrandonAndRemaps(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_poisonBrandonAndRemaps(%p) ()", (const void *)script);
	setBrandonPoisonFlags(1);
	return 0;
}

int KyraEngine_LoK::o1_fillFlaskWithWater(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_fillFlaskWithWater(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	seq_fillFlaskWithWater(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_LoK::o1_getCharacterMovementDelay(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_getCharacterMovementDelay(%p) (%d)", (const void *)script, stackPos(0));
	return _timer->getDelay(stackPos(0) + 5);
}

int KyraEngine_LoK::o1_getBirthstoneGem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_getBirthstoneGem(%p) (%d)", (const void *)script, stackPos(0));
	if (stackPos(0) < 4)
		return _birthstoneGemTable[stackPos(0)];
	return 0;
}

int KyraEngine_LoK::o1_queryBrandonStatusBit(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_queryBrandonStatusBit(%p) (%d)", (const void *)script, stackPos(0));
	if (_brandonStatusBit & stackPos(0))
		return 1;
	return 0;
}

int KyraEngine_LoK::o1_playFluteAnimation(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_playFluteAnimation(%p) ()", (const void *)script);
	seq_playFluteAnimation();
	return 0;
}

int KyraEngine_LoK::o1_playWinterScrollSequence(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_playWinterScrollSequence(%p) (%d)", (const void *)script, stackPos(0));
	if (!stackPos(0))
		seq_winterScroll2();
	else
		seq_winterScroll1();
	return 0;
}

int KyraEngine_LoK::o1_getIdolGem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_getIdolGem(%p) (%d)", (const void *)script, stackPos(0));
	return _idolGemsTable[stackPos(0)];
}

int KyraEngine_LoK::o1_setIdolGem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setIdolGem(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_idolGemsTable[stackPos(0)] = stackPos(1);
	return 0;
}

int KyraEngine_LoK::o1_totalItemsInScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_totalItemsInScene(%p) (%d)", (const void *)script, stackPos(0));
	return countItemsInScene(stackPos(0));
}

int KyraEngine_LoK::o1_restoreBrandonsMovementDelay(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_restoreBrandonsMovementDelay(%p) ()", (const void *)script);
	setWalkspeed(_configWalkspeed);
	return 0;
}

int KyraEngine_LoK::o1_setEntranceMouseCursorTrack(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setEntranceMouseCursorTrack(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	_entranceMouseCursorTracks[0] = stackPos(0);
	_entranceMouseCursorTracks[1] = stackPos(1);
	_entranceMouseCursorTracks[2] = stackPos(0) + stackPos(2) - 1;
	_entranceMouseCursorTracks[3] = stackPos(1) + stackPos(3) - 1;
	_entranceMouseCursorTracks[4] = stackPos(4);
	return 0;
}

int KyraEngine_LoK::o1_itemAppearsOnGround(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_itemAppearsOnGround(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	processItemDrop(_currentCharacter->sceneId, stackPos(0), stackPos(1), stackPos(2), 2, 0);
	return 0;
}

int KyraEngine_LoK::o1_setNoDrawShapesFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setNoDrawShapesFlag(%p) (%d)", (const void *)script, stackPos(0));
	_animator->_noDrawShapesFlag = stackPos(0);
	return 0;
}

int KyraEngine_LoK::o1_fadeEntirePalette(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_fadeEntirePalette(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	int cmd = stackPos(0);

	int fadePal = 0;

	if (_flags.platform == Common::kPlatformAmiga) {
		if (cmd == 0) {
			_screen->getPalette(2).clear();
			fadePal = 2;
			_screen->copyPalette(4, 0);
		} else if (cmd == 1) {
			fadePal = 0;
			_screen->copyPalette(0, 4);
		} else if (cmd == 2) {
			fadePal = 0;
			_screen->getPalette(2).clear();
		}
	} else {
		if (cmd == 0) {
			fadePal = 2;
			_screen->getPalette(2).clear();
			_screen->copyPalette(3, 0);
		} else if (cmd == 1) {
			//fadePal = 3;
			warning("unimplemented o1_fadeEntirePalette function");
			return 0;
		} else if (cmd == 2) {
			_screen->getPalette(2).clear();
			_screen->copyPalette(0, 1);
			fadePal = 0;
		}
	}

	_screen->fadePalette(_screen->getPalette(fadePal), stackPos(1));
	return 0;
}

int KyraEngine_LoK::o1_itemOnGroundHere(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_itemOnGroundHere(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	assert(stackPos(0) < _roomTableSize);
	Room *curRoom = &_roomTable[stackPos(0)];
	for (int i = 0; i < 12; ++i) {
		if (curRoom->itemsTable[i] == stackPos(1))
			return 1;
	}
	return 0;
}

int KyraEngine_LoK::o1_queryCauldronState(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_queryCauldronState(%p) ()", (const void *)script);
	return _cauldronState;
}

int KyraEngine_LoK::o1_setCauldronState(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setCauldronState(%p) (%d)", (const void *)script, stackPos(0));
	_cauldronState = stackPos(0);
	return _cauldronState;
}

int KyraEngine_LoK::o1_queryCrystalState(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_queryCrystalState(%p) (%d)", (const void *)script, stackPos(0));
	if (!stackPos(0))
		return _crystalState[0];
	else if (stackPos(0) == 1)
		return _crystalState[1];
	return -1;
}

int KyraEngine_LoK::o1_setCrystalState(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setCrystalState(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	if (!stackPos(0))
		_crystalState[0] = stackPos(1);
	else if (stackPos(0) == 1)
		_crystalState[1] = stackPos(1);
	return stackPos(1);
}

int KyraEngine_LoK::o1_setPaletteRange(EMCState *script) {
	warning("STUB: o1_setPaletteRange");
	return 0;
}

int KyraEngine_LoK::o1_shrinkBrandonDown(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_shrinkBrandonDown(%p) (%d)", (const void *)script, stackPos(0));
	int delayTime = stackPos(0);
	checkAmuletAnimFlags();
	int scaleValue = _scaleTable[_currentCharacter->y1];
	int scale = 0;

	if (_scaleMode)
		scale = scaleValue;
	else
		scale = 256;

	int scaleModeBackUp = _scaleMode;
	_scaleMode = 1;
	int scaleEnd = scale >> 1;
	for (; scaleEnd <= scale; --scale) {
		_scaleTable[_currentCharacter->y1] = scale;
		_animator->animRefreshNPC(0);
		delayWithTicks(1);
	}
	delayWithTicks(delayTime); // XXX
	_scaleTable[_currentCharacter->y1] = scaleValue;
	_scaleMode = scaleModeBackUp;
	return 0;
}

int KyraEngine_LoK::o1_growBrandonUp(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_growBrandonUp(%p) ()", (const void *)script);
	int scaleValue = _scaleTable[_currentCharacter->y1];
	int scale = 0;
	if (_scaleMode)
		scale = scaleValue;
	else
		scale = 256;

	int scaleModeBackUp = _scaleMode;
	_scaleMode = 1;
	for (int curScale = scale >> 1; curScale <= scale; ++curScale) {
		_scaleTable[_currentCharacter->y1] = curScale;
		_animator->animRefreshNPC(0);
		delayWithTicks(1);
	}
	_scaleTable[_currentCharacter->y1] = scaleValue;
	_scaleMode = scaleModeBackUp;
	return 0;
}

int KyraEngine_LoK::o1_setBrandonScaleXAndY(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setBrandonScaleXAndY(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_animator->_brandonScaleX = stackPos(0);
	_animator->_brandonScaleY = stackPos(1);
	return 0;
}

int KyraEngine_LoK::o1_resetScaleMode(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_resetScaleMode(%p) ()", (const void *)script);
	_scaleMode = 0;
	return 0;
}

int KyraEngine_LoK::o1_getScaleDepthTableValue(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_getScaleDepthTableValue(%p) (%d)", (const void *)script, stackPos(0));
	assert(stackPos(0) < ARRAYSIZE(_scaleTable));
	return _scaleTable[stackPos(0)];
}

int KyraEngine_LoK::o1_setScaleDepthTableValue(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setScaleDepthTableValue(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	assert(stackPos(0) < ARRAYSIZE(_scaleTable));
	_scaleTable[stackPos(0)] = stackPos(1);
	return stackPos(1);
}

int KyraEngine_LoK::o1_message(EMCState *script) {
	if (_flags.isTalkie) {
		debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_message(%p) (%d, '%s', %d)", (const void *)script, stackPos(0), stackPosString(1), stackPos(2));
		drawSentenceCommand(stackPosString(1), stackPos(2));
	} else {
		debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_message(%p) ('%s', %d)", (const void *)script, stackPosString(0), stackPos(1));
		drawSentenceCommand(stackPosString(0), stackPos(1));
	}

	return 0;
}

int KyraEngine_LoK::o1_checkClickOnNPC(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_checkClickOnNPC(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	return checkForNPCScriptRun(stackPos(0), stackPos(1));
}

int KyraEngine_LoK::o1_getFoyerItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_getFoyerItem(%p) (%d)", (const void *)script, stackPos(0));
	assert(stackPos(0) < ARRAYSIZE(_foyerItemTable));
	return _foyerItemTable[stackPos(0)];
}

int KyraEngine_LoK::o1_setFoyerItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setFoyerItem(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	assert(stackPos(0) < ARRAYSIZE(_foyerItemTable));
	_foyerItemTable[stackPos(0)] = stackPos(1);
	return stackPos(1);
}

int KyraEngine_LoK::o1_setNoItemDropRegion(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setNoItemDropRegion(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	addToNoDropRects(stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	return 0;
}

int KyraEngine_LoK::o1_walkMalcolmOn(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_walkMalcolmOn(%p) ()", (const void *)script);
	if (!_malcolmFlag)
		_malcolmFlag = 1;
	return 0;
}

int KyraEngine_LoK::o1_passiveProtection(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_passiveProtection(%p) ()", (const void *)script);
	return 1;
}

int KyraEngine_LoK::o1_setPlayingLoop(EMCState *script) {
	warning("STUB: o1_setPlayingLoop");
	return 0;
}

int KyraEngine_LoK::o1_brandonToStoneSequence(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_brandonToStoneSequence(%p) ()", (const void *)script);
	seq_brandonToStone();
	return 0;
}

int KyraEngine_LoK::o1_brandonHealingSequence(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_brandonHealingSequence(%p) ()", (const void *)script);
	seq_brandonHealing();
	return 0;
}

int KyraEngine_LoK::o1_protectCommandLine(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_protectCommandLine(%p) (%d)", (const void *)script, stackPos(0));
	return stackPos(0);
}

int KyraEngine_LoK::o1_pauseMusicSeconds(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_pauseMusicSeconds(%p) ()", (const void *)script);
	// if music disabled
	//     return
	delay(stackPos(0) * 1000, true);
	return 0;
}

int KyraEngine_LoK::o1_resetMaskRegion(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_resetMaskRegion(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	_screen->fillRect(stackPos(1), stackPos(2), stackPos(1) + stackPos(3), stackPos(2) + stackPos(4), 0, 5);
	return 0;
}

int KyraEngine_LoK::o1_setPaletteChangeFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_setPaletteChangeFlag(%p) (%d)", (const void *)script, stackPos(0));
	_paletteChanged = stackPos(0);
	return _paletteChanged;
}

int KyraEngine_LoK::o1_vocUnload(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_vocUnload(%p) ()", (const void *)script);
	// this should unload all voc files (not needed)
	return 0;
}

int KyraEngine_LoK::o1_vocLoad(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_vocLoad(%p) (%d)", (const void *)script, stackPos(0));
	// this should load the specified voc file (not needed)
	return 0;
}

int KyraEngine_LoK::o1_dummy(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_LoK::o1_dummy(%p) ()", (const void *)script);
	return 0;
}

#pragma mark -

typedef Common::Functor1Mem<EMCState *, int, KyraEngine_LoK> OpcodeV1;
#define SetOpcodeTable(x) table = &x;
#define Opcode(x) table->push_back(new OpcodeV1(this, &KyraEngine_LoK::x))
void KyraEngine_LoK::setupOpcodeTable() {
	Common::Array<const Opcode *> *table = 0;

	_opcodes.reserve(157);
	SetOpcodeTable(_opcodes);
	// 0x00
	Opcode(o1_magicInMouseItem);
	Opcode(o1_characterSays);
	Opcode(o1_delay);
	Opcode(o1_drawSceneAnimShape);
	// 0x04
	Opcode(o1_queryGameFlag);
	Opcode(o1_setGameFlag);
	Opcode(o1_resetGameFlag);
	Opcode(o1_runNPCScript);
	// 0x08
	Opcode(o1_setSpecialExitList);
	Opcode(o1_blockInWalkableRegion);
	Opcode(o1_blockOutWalkableRegion);
	Opcode(o1_walkPlayerToPoint);
	// 0x0c
	Opcode(o1_dropItemInScene);
	Opcode(o1_drawAnimShapeIntoScene);
	Opcode(o1_setHandItem);
	Opcode(o1_savePageToDisk);
	// 0x10
	Opcode(o1_sceneAnimOn);
	Opcode(o1_sceneAnimOff);
	Opcode(o1_getElapsedSeconds);
	Opcode(o1_mouseIsPointer);
	// 0x14
	Opcode(o1_removeHandItem);
	Opcode(o1_runSceneAnimUntilDone);
	Opcode(o1_fadeSpecialPalette);
	Opcode(o1_playSoundEffect);
	// 0x18
	Opcode(o1_playWanderScoreViaMap);
	Opcode(o1_phaseInSameScene);
	Opcode(o1_setScenePhasingFlag);
	Opcode(o1_resetScenePhasingFlag);
	// 0x1c
	Opcode(o1_queryScenePhasingFlag);
	Opcode(o1_sceneToDirection);
	Opcode(o1_setBirthstoneGem);
	Opcode(o1_placeItemInGenericMapScene);
	// 0x20
	Opcode(o1_setBrandonStatusBit);
	Opcode(o1_delaySecs);
	Opcode(o1_getCharacterScene);
	Opcode(o1_runNPCSubscript);
	// 0x24
	Opcode(o1_magicOutMouseItem);
	Opcode(o1_internalAnimOn);
	Opcode(o1_forceBrandonToNormal);
	Opcode(o1_poisonDeathNow);
	// 0x28
	Opcode(o1_setScaleMode);
	Opcode(o1_openWSAFile);
	Opcode(o1_closeWSAFile);
	Opcode(o1_runWSAFromBeginningToEnd);
	// 0x2c
	Opcode(o1_displayWSAFrame);
	Opcode(o1_enterNewScene);
	Opcode(o1_setSpecialEnterXAndY);
	Opcode(o1_runWSAFrames);
	// 0x30
	Opcode(o1_popBrandonIntoScene);
	Opcode(o1_restoreAllObjectBackgrounds);
	Opcode(o1_setCustomPaletteRange);
	Opcode(o1_loadPageFromDisk);
	// 0x34
	Opcode(o1_customPrintTalkString);
	Opcode(o1_restoreCustomPrintBackground);
	Opcode(o1_hideMouse);
	Opcode(o1_showMouse);
	// 0x38
	Opcode(o1_getCharacterX);
	Opcode(o1_getCharacterY);
	Opcode(o1_setCharacterFacing);
	Opcode(o1_copyWSARegion);
	// 0x3c
	Opcode(o1_printText);
	Opcode(o1_getRand);
	Opcode(o1_loadSoundFile);
	Opcode(o1_displayWSAFrameOnHidPage);
	// 0x40
	Opcode(o1_displayWSASequentialFrames);
	Opcode(o1_refreshCharacter);
	Opcode(o1_internalAnimOff);
	Opcode(o1_changeCharactersXAndY);
	// 0x44
	Opcode(o1_clearSceneAnimatorBeacon);
	Opcode(o1_querySceneAnimatorBeacon);
	Opcode(o1_refreshSceneAnimator);
	Opcode(o1_placeItemInOffScene);
	// 0x48
	Opcode(o1_wipeDownMouseItem);
	Opcode(o1_placeCharacterInOtherScene);
	Opcode(o1_getKey);
	Opcode(o1_specificItemInInventory);
	// 0x4c
	Opcode(o1_popMobileNPCIntoScene);
	Opcode(o1_mobileCharacterInScene);
	Opcode(o1_hideMobileCharacter);
	Opcode(o1_unhideMobileCharacter);
	// 0x50
	Opcode(o1_setCharacterLocation);
	Opcode(o1_walkCharacterToPoint);
	Opcode(o1_specialEventDisplayBrynnsNote);
	Opcode(o1_specialEventRemoveBrynnsNote);
	// 0x54
	Opcode(o1_setLogicPage);
	Opcode(o1_fatPrint);
	Opcode(o1_preserveAllObjectBackgrounds);
	Opcode(o1_updateSceneAnimations);
	// 0x58
	Opcode(o1_sceneAnimationActive);
	Opcode(o1_setCharacterMovementDelay);
	Opcode(o1_getCharacterFacing);
	Opcode(o1_bkgdScrollSceneAndMasksRight);
	// 0x5c
	Opcode(o1_dispelMagicAnimation);
	Opcode(o1_findBrightestFireberry);
	Opcode(o1_setFireberryGlowPalette);
	Opcode(o1_setDeathHandler);
	// 0x60
	Opcode(o1_drinkPotionAnimation);
	Opcode(o1_makeAmuletAppear);
	Opcode(o1_drawItemShapeIntoScene);
	Opcode(o1_setCharacterCurrentFrame);
	// 0x64
	Opcode(o1_waitForConfirmationMouseClick);
	Opcode(o1_pageFlip);
	Opcode(o1_setSceneFile);
	Opcode(o1_getItemInMarbleVase);
	// 0x68
	Opcode(o1_setItemInMarbleVase);
	Opcode(o1_addItemToInventory);
	Opcode(o1_intPrint);
	Opcode(o1_shakeScreen);
	// 0x6c
	Opcode(o1_createAmuletJewel);
	Opcode(o1_setSceneAnimCurrXY);
	Opcode(o1_poisonBrandonAndRemaps);
	Opcode(o1_fillFlaskWithWater);
	// 0x70
	Opcode(o1_getCharacterMovementDelay);
	Opcode(o1_getBirthstoneGem);
	Opcode(o1_queryBrandonStatusBit);
	Opcode(o1_playFluteAnimation);
	// 0x74
	Opcode(o1_playWinterScrollSequence);
	Opcode(o1_getIdolGem);
	Opcode(o1_setIdolGem);
	Opcode(o1_totalItemsInScene);
	// 0x78
	Opcode(o1_restoreBrandonsMovementDelay);
	Opcode(o1_setMousePos);
	Opcode(o1_getMouseState);
	Opcode(o1_setEntranceMouseCursorTrack);
	// 0x7c
	Opcode(o1_itemAppearsOnGround);
	Opcode(o1_setNoDrawShapesFlag);
	Opcode(o1_fadeEntirePalette);
	Opcode(o1_itemOnGroundHere);
	// 0x80
	Opcode(o1_queryCauldronState);
	Opcode(o1_setCauldronState);
	Opcode(o1_queryCrystalState);
	Opcode(o1_setCrystalState);
	// 0x84
	Opcode(o1_setPaletteRange);
	Opcode(o1_shrinkBrandonDown);
	Opcode(o1_growBrandonUp);
	Opcode(o1_setBrandonScaleXAndY);
	// 0x88
	Opcode(o1_resetScaleMode);
	Opcode(o1_getScaleDepthTableValue);
	Opcode(o1_setScaleDepthTableValue);
	Opcode(o1_message);
	// 0x8c
	Opcode(o1_checkClickOnNPC);
	Opcode(o1_getFoyerItem);
	Opcode(o1_setFoyerItem);
	Opcode(o1_setNoItemDropRegion);
	// 0x90
	Opcode(o1_walkMalcolmOn);
	Opcode(o1_passiveProtection);
	Opcode(o1_setPlayingLoop);
	Opcode(o1_brandonToStoneSequence);
	// 0x94
	Opcode(o1_brandonHealingSequence);
	Opcode(o1_protectCommandLine);
	Opcode(o1_pauseMusicSeconds);
	Opcode(o1_resetMaskRegion);
	// 0x98
	Opcode(o1_setPaletteChangeFlag);
	Opcode(o1_fillRect);
	Opcode(o1_vocUnload);
	Opcode(o1_vocLoad);
	// 0x9c
	Opcode(o1_dummy);
}
#undef Opcode

} // End of namespace Kyra
