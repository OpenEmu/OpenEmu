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
#include "kyra/timer.h"
#include "kyra/resource.h"
#include "kyra/sound.h"

#include "common/system.h"

namespace Kyra {

int KyraEngine_HoF::o2_setCharacterFacingRefresh(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_setCharacterFacingRefresh(%p) (-, %d, %d)", (const void *)script, stackPos(1), stackPos(2));
	int animFrame = stackPos(2);
	if (animFrame >= 0)
		_mainCharacter.animFrame = animFrame;
	_mainCharacter.facing = stackPos(1);
	updateCharacterAnim(0);
	refreshAnimObjectsIfNeed();
	return 0;
}

int KyraEngine_HoF::o2_setCharacterPos(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_setCharacterFacingRefresh(%p) (-, %d, %d)", (const void *)script, stackPos(1), stackPos(2));
	int x = stackPos(1);
	int y = stackPos(2);

	if (x != -1 && y != -1) {
		x &= ~3;
		y &= ~1;
	}

	restorePage3();
	_mainCharacter.x2 = _mainCharacter.x1 = x;
	_mainCharacter.y2 = _mainCharacter.y1 = y;
	return 0;
}

int KyraEngine_HoF::o2_defineObject(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_defineObject(%p) (%d, '%s', %d, %d, %d, %d)", (const void *)script,
			stackPos(0), stackPosString(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	TalkObject *object = &_talkObjectList[stackPos(0)];
	strcpy(object->filename, stackPosString(1));
	object->scriptId = stackPos(2);
	object->x = stackPos(3);
	object->y = stackPos(4);
	object->color = stackPos(5);
	return 0;
}

int KyraEngine_HoF::o2_refreshCharacter(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_refreshCharacter(%p) (-, %d, %d, %d)", (const void *)script, stackPos(1), stackPos(2), stackPos(3));
	int unk = stackPos(1);
	int facing = stackPos(2);
	int refresh = stackPos(3);
	if (facing >= 0)
		_mainCharacter.facing = facing;
	if (unk >= 0 && unk != 32)
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
	updateCharacterAnim(0);
	if (refresh)
		refreshAnimObjectsIfNeed();
	return 0;
}

int KyraEngine_HoF::o2_setSceneComment(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_setSceneComment(%p) ('%s')", (const void *)script, stackPosString(0));
	_sceneCommentString = stackPosString(0);
	return 0;
}

int KyraEngine_HoF::o2_setCharacterAnimFrame(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_setCharacterAnimFrame(%p) (-, %d, %d)", (const void *)script, stackPos(1), stackPos(2));
	int animFrame = stackPos(1);
	int updateAnim = stackPos(2);

	_mainCharacter.animFrame = animFrame;
	if (updateAnim)
		updateCharacterAnim(0);

	return 0;
}

int KyraEngine_HoF::o2_customCharacterChat(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_customCharacterChat(%p) ('%s', %d, %d, %d, %d)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	playVoice(_vocHigh, stackPos(4));
	_text->printCustomCharacterText(stackPosString(0), stackPos(1), stackPos(2), stackPos(3), 0, 2);
	return 0;
}

int KyraEngine_HoF::o2_soundFadeOut(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_soundFadeOut(%p) ()", (const void *)script);
	_sound->beginFadeOut();
	return 0;
}

int KyraEngine_HoF::o2_showChapterMessage(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_showChapterMessage(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	showChapterMessage(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_HoF::o2_restoreTalkTextMessageBkgd(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_restoreTalkTextMessageBkgd(%p) ()", (const void *)script);
	_text->restoreTalkTextMessageBkgd(2, 0);
	return 0;
}

int KyraEngine_HoF::o2_wsaClose(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_wsaClose(%p) (%d)", (const void *)script, stackPos(0));
	assert(stackPos(0) >= 0 && stackPos(0) < ARRAYSIZE(_wsaSlots));
	_wsaSlots[stackPos(0)]->close();
	return 0;
}

int KyraEngine_HoF::o2_meanWhileScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_meanWhileScene(%p) (%d)", (const void *)script, stackPos(0));
	static const uint8 jpSubtitle[] = { 0x88, 0xEA, 0x95, 0xFB, 0x81, 0x45, 0x81, 0x45, 0x81, 0x45 };
	const char *cpsfile = stackPosString(0);
	const char *palfile = stackPosString(1);

	_screen->loadBitmap(cpsfile, 3, 3, 0);
	_screen->copyPalette(2, 0);
	_screen->loadPalette(palfile, _screen->getPalette(2));
	_screen->fillRect(0, 0, 319, 199, 207);
	_screen->setScreenPalette(_screen->getPalette(2));
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0);
	if (!scumm_stricmp(cpsfile, "_MEANWIL.CPS") && _flags.lang == Common::JA_JPN) {
		Screen::FontId o = _screen->setFont(Screen::FID_SJIS_FNT);
		_screen->printText((const char *)jpSubtitle, 140, 176, 255, 132);
		_screen->setFont(o);
	}
	_screen->updateScreen();
	return 0;
}

int KyraEngine_HoF::o2_backUpScreen(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_backUpScreen(%p) (%d)", (const void *)script, stackPos(0));
	_screen->copyRegionToBuffer(stackPos(0), 0, 0, 320, 144, _screenBuffer);
	return 0;
}

int KyraEngine_HoF::o2_restoreScreen(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_restoreScreen(%p) (%d)", (const void *)script, stackPos(0));
	_screen->copyBlockToPage(stackPos(0), 0, 0, 320, 144, _screenBuffer);
	return 0;
}

int KyraEngine_HoF::o2_displayWsaFrame(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_displayWsaFrame(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d)", (const void *)script,
			stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8));
	int frame = stackPos(0);
	int x = stackPos(1);
	int y = stackPos(2);
	int waitTime = stackPos(3);
	int slot = stackPos(4);
	int copyParam = stackPos(5);
	int doUpdate = stackPos(6);
	int dstPage = stackPos(7);
	int backUp = stackPos(8);

	_screen->hideMouse();
	const uint32 endTime = _system->getMillis() + waitTime * _tickLength;
	_wsaSlots[slot]->displayFrame(frame, dstPage, x, y, copyParam | 0xC000, 0, 0);
	_screen->updateScreen();

	if (backUp)
		memcpy(_gamePlayBuffer, _screen->getCPagePtr(3), 46080);

	delayUntil(endTime, false, doUpdate != 0);
	_screen->showMouse();
	return 0;
}

int KyraEngine_HoF::o2_displayWsaSequentialFramesLooping(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_displayWsaSequentialFramesLooping(%p) (%d, %d, %d, %d, %d, %d, %d, %d)", (const void *)script,
			stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7));
	int startFrame = stackPos(0);
	int endFrame = stackPos(1);
	int x = stackPos(2);
	int y = stackPos(3);
	int waitTime = stackPos(4);
	int slot = stackPos(5);
	int maxTimes = stackPos(6);
	int copyFlags = stackPos(7);

	if (maxTimes > 1)
		maxTimes = 1;

	_screen->hideMouse();
	int curTime = 0;
	while (curTime < maxTimes) {
		if (startFrame < endFrame) {
			for (int i = startFrame; i <= endFrame; ++i) {
				const uint32 endTime = _system->getMillis() + waitTime * _tickLength;
				_wsaSlots[slot]->displayFrame(i, 0, x, y, 0xC000 | copyFlags, 0, 0);

				if (!skipFlag()) {
					_screen->updateScreen();
					delayUntil(endTime, false, true);
				}
			}
		} else {
			for (int i = startFrame; i >= endFrame; --i) {
				const uint32 endTime = _system->getMillis() + waitTime * _tickLength;
				_wsaSlots[slot]->displayFrame(i, 0, x, y, 0xC000 | copyFlags, 0, 0);

				if (!skipFlag()) {
					_screen->updateScreen();
					delayUntil(endTime, false, true);
				}
			}
		}

		++curTime;
	}
	resetSkipFlag();
	_screen->showMouse();
	return 0;
}

int KyraEngine_HoF::o2_wsaOpen(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_wsaOpen(%p) ('%s', %d)", (const void *)script, stackPosString(0), stackPos(1));
	assert(stackPos(1) >= 0 && stackPos(1) < ARRAYSIZE(_wsaSlots));
	_wsaSlots[stackPos(1)]->open(stackPosString(0), 1, 0);
	return 0;
}

int KyraEngine_HoF::o2_displayWsaSequentialFrames(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_displayWsaSequentialFrames(%p) (%d, %d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6));

	uint16 frameDelay = stackPos(2) * _tickLength;
	uint16 currentFrame = stackPos(3);
	uint16 lastFrame = stackPos(4);
	uint16 index = stackPos(5);
	uint16 copyParam = stackPos(6) | 0xc000;

	_screen->hideMouse();

	while (currentFrame <= lastFrame) {
		const uint32 endTime = _system->getMillis() + frameDelay;
		_wsaSlots[index]->displayFrame(currentFrame++, 0, stackPos(0), stackPos(1), copyParam, 0, 0);
		if (!skipFlag()) {
			_screen->updateScreen();
			delayUntil(endTime);
		}
	}

	resetSkipFlag();
	_screen->showMouse();

	return 0;
}

int KyraEngine_HoF::o2_displayWsaSequence(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_displayWsaSequence(%p) (%d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));

	const int frameDelay = stackPos(2) * _tickLength;
	const int index = stackPos(3);
	const bool doUpdate = (stackPos(4) != 0);
	const uint16 copyParam = stackPos(5) | 0xc000;

	_screen->hideMouse();

	int currentFrame = 0;
	const int lastFrame = _wsaSlots[index]->frames();

	while (currentFrame <= lastFrame) {
		const uint32 endTime = _system->getMillis() + frameDelay;
		_wsaSlots[index]->displayFrame(currentFrame++, 0, stackPos(0), stackPos(1), copyParam, 0, 0);
		if (!skipFlag()) {
			if (doUpdate)
				update();
			_screen->updateScreen();
			delayUntil(endTime);
		}
	}

	resetSkipFlag();
	_screen->showMouse();

	return 0;
}

int KyraEngine_HoF::o2_addItemToInventory(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_addItemToInventory(%p) (%d, -, %d)", (const void *)script, stackPos(0), stackPos(2));
	int slot = findFreeVisibleInventorySlot();
	if (slot != -1) {
		_mainCharacter.inventory[slot] = stackPos(0);
		if (stackPos(2))
			redrawInventory(0);
	}
	return slot;
}

int KyraEngine_HoF::o2_drawShape(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_drawShape(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));

	uint8 *shp = getShapePtr(stackPos(0) + 64);
	int x = stackPos(1);
	int y = stackPos(2);
	uint8 dsFlag = stackPos(3) & 0xff;
	uint8 modeFlag = stackPos(4) & 0xff;

	if (modeFlag) {
		_screen->drawShape(2, shp, x, y, 2, dsFlag ? 1 : 0);
	} else {
		restorePage3();
		_screen->drawShape(2, shp, x, y, 2, dsFlag ? 1 : 0);
		memcpy(_gamePlayBuffer, _screen->getCPagePtr(3), 46080);
		_screen->drawShape(0, shp, x, y, 2, dsFlag ? 1 : 0);

		flagAnimObjsForRefresh();
		flagAnimObjsSpecialRefresh();
		refreshAnimObjectsIfNeed();
	}

	return 0;
}

int KyraEngine_HoF::o2_addItemToCurScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_addItemToCurScene(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	const int16 id = stackPos(0);
	int x = stackPos(1);
	int y = stackPos(2);

	int freeItem = findFreeItem();
	x = MAX(14, x);
	x = MIN(304, x);
	y = MAX(14, y);
	y = MIN(136, y);
	if (freeItem >= 0) {
		_itemList[freeItem].id = id;
		_itemList[freeItem].x = x;
		_itemList[freeItem].y = y;
		_itemList[freeItem].sceneId = _mainCharacter.sceneId;
		addItemToAnimList(freeItem);
		refreshAnimObjectsIfNeed();
	}
	return 0;
}

int KyraEngine_HoF::o2_loadSoundFile(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_loadSoundFile(%p) (%d)", (const void *)script, stackPos(0));
	if (_flags.platform == Common::kPlatformPC)
		snd_loadSoundFile(stackPos(0));
	return 0;
}

int KyraEngine_HoF::o2_removeSlotFromInventory(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_removeSlotFromInventory(%p) (%d)", (const void *)script, stackPos(0));
	removeSlotFromInventory(stackPos(0));
	return 0;
}

int KyraEngine_HoF::o2_removeItemFromInventory(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_removeItemFromInventory(%p) (%d)", (const void *)script, stackPos(0));
	uint16 item = stackPos(0);
	int slot = -1;
	while ((slot = getInventoryItemSlot(item)) != -1)
		removeSlotFromInventory(slot);
	return 0;
}

int KyraEngine_HoF::o2_countItemInInventory(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_countItemInInventory(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	uint16 item = stackPos(1);
	int count = 0;

	for (int i = 0; i < 20; ++i) {
		if (_mainCharacter.inventory[i] == item)
			++count;
	}

	if ((stackPos(0) == 0) && _itemInHand == int16(item))
		++count;

	return count;
}

int KyraEngine_HoF::o2_countItemsInScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_countItemsInScene(%p) (%d)", (const void *)script, stackPos(0));
	int count = 0;
	for (int i = 0; i < 30; ++i) {
		if (_itemList[i].sceneId == stackPos(0) && _itemList[i].id != kItemNone)
			++count;
	}
	return count;
}

int KyraEngine_HoF::o2_wipeDownMouseItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_wipeDownMouseItem(%p) (-, %d, %d)", (const void *)script, stackPos(1), stackPos(2));
	_screen->hideMouse();
	const int x = stackPos(1) - 8;
	const int y = stackPos(2) - 15;

	if (_itemInHand >= 0) {
		backUpGfxRect32x32(x, y);
		uint8 *shape = getShapePtr(_itemInHand+64);
		for (int curY = y, height = 16; height > 0; height -= 2, curY += 2) {
			restoreGfxRect32x32(x, y);
			_screen->setNewShapeHeight(shape, height);
			uint32 waitTime = _system->getMillis() + _tickLength;
			_screen->drawShape(0, shape, x, curY, 0, 0);
			_screen->updateScreen();
			delayUntil(waitTime);
		}
		restoreGfxRect32x32(x, y);
		_screen->resetShapeHeight(shape);
	}

	_screen->showMouse();
	removeHandItem();

	return 0;
}

int KyraEngine_HoF::o2_getElapsedSecs(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_getElapsedSecs(%p) ()", (const void *)script);
	return _system->getMillis() / 1000;
}

int KyraEngine_HoF::o2_getTimerDelay(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_getTimerDelay(%p) (%d)", (const void *)script, stackPos(0));
	return _timer->getDelay(stackPos(0));
}

int KyraEngine_HoF::o2_delaySecs(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_delaySecs(%p) (%d)", (const void *)script, stackPos(0));
	delay(stackPos(0) * 1000, true);
	return 0;
}

int KyraEngine_HoF::o2_setTimerDelay(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_setTimerDelay(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_timer->setDelay(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_HoF::o2_setScaleTableItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_setScaleTableItem(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	setScaleTableItem(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_HoF::o2_setDrawLayerTableItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_setDrawLayerTableItem(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	setDrawLayerTableEntry(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_HoF::o2_setCharPalEntry(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_setCharPalEntry(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	setCharPalEntry(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_HoF::o2_loadZShapes(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_loadZShapes(%p) (%d)", (const void *)script, stackPos(0));
	loadCharacterShapes(stackPos(0));
	return 0;
}

int KyraEngine_HoF::o2_drawSceneShape(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_drawSceneShape(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1),
		stackPos(2), stackPos(3));

	int shape = stackPos(0);
	int x = stackPos(1);
	int y = stackPos(2);
	int flag = (stackPos(3) != 0) ? 1 : 0;

	restorePage3();

	_screen->drawShape(2, _sceneShapeTable[shape], x, y, 2, flag);

	memcpy(_gamePlayBuffer, _screen->getCPagePtr(3), 46080);

	_screen->drawShape(0, _sceneShapeTable[shape], x, y, 2, flag);

	flagAnimObjsSpecialRefresh();
	flagAnimObjsForRefresh();
	refreshAnimObjectsIfNeed();
	return 0;
}

int KyraEngine_HoF::o2_drawSceneShapeOnPage(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_drawSceneShapeOnPage(%p) (%d, %d, %d, %d, %d)", (const void *)script,
		stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	int shape = stackPos(0);
	int x = stackPos(1);
	int y = stackPos(2);
	int flag = stackPos(3);
	int drawPage = stackPos(4);

	_screen->drawShape(drawPage, _sceneShapeTable[shape], x, y, 2, flag ? 1 : 0);
	return 0;
}

int KyraEngine_HoF::o2_disableAnimObject(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_disableAnimObject(%p) (%d)", (const void *)script, stackPos(0));
	_animObjects[stackPos(0)+1].enabled = false;
	return 0;
}

int KyraEngine_HoF::o2_enableAnimObject(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_enableAnimObject(%p) (%d)", (const void *)script, stackPos(0));
	_animObjects[stackPos(0)+1].enabled = true;
	return 0;
}

int KyraEngine_HoF::o2_loadPalette384(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_loadPalette384(%p) ('%s')", (const void *)script, stackPosString(0));
	_screen->copyPalette(1, 0);
	_res->loadFileToBuf(stackPosString(0), _screen->getPalette(1).getData(), 384);
	return 0;
}

int KyraEngine_HoF::o2_setPalette384(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_setPalette384(%p) ()", (const void *)script);
	_screen->getPalette(0).copy(_screen->getPalette(1), 0, 128);
	_screen->setScreenPalette(_screen->getPalette(0));
	return 0;
}

int KyraEngine_HoF::o2_restoreBackBuffer(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_restoreBackBuffer(%p) (%d)", (const void *)script, stackPos(0));
	int disable = stackPos(0);
	int oldState = 0;

	if (disable) {
		oldState = _animObjects[0].enabled;
		_animObjects[0].enabled = 0;
	}

	restorePage3();

	if (disable)
		_animObjects[0].enabled = (oldState != 0);

	return 0;
}

int KyraEngine_HoF::o2_backUpInventoryGfx(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_backUpInventoryGfx(%p) ()", (const void *)script);
	_screen->copyRegionToBuffer(1, 0, 144, 320, 56, _screenBuffer);
	_inventorySaved = true;
	return 0;
}

int KyraEngine_HoF::o2_disableSceneAnim(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_disableSceneAnim(%p) (%d)", (const void *)script, stackPos(0));
	_sceneAnims[stackPos(0)].flags &= ~1;
	return 0;
}

int KyraEngine_HoF::o2_enableSceneAnim(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_enableSceneAnim(%p) (%d)", (const void *)script, stackPos(0));
	_sceneAnims[stackPos(0)].flags |= 1;
	return 0;
}

int KyraEngine_HoF::o2_restoreInventoryGfx(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_restoreInventoryGfx(%p) ()", (const void *)script);
	_screen->copyBlockToPage(1, 0, 144, 320, 56, _screenBuffer);
	_inventorySaved = false;
	return 0;
}

int KyraEngine_HoF::o2_setSceneAnimPos2(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_setSceneAnimPos2(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	_sceneAnims[stackPos(0)].x2 = stackPos(1);
	_sceneAnims[stackPos(0)].y2 = stackPos(2);
	return 0;
}

int KyraEngine_HoF::o2_fadeScenePal(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_fadeScenePal(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	fadeScenePal(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_HoF::o2_enterNewScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_enterNewScene(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0),
		stackPos(1), stackPos(2), stackPos(3), stackPos(4));

	int skipNpcScript = stackPos(3);
	enterNewScene(stackPos(0), stackPos(1), stackPos(2), skipNpcScript, stackPos(4));

	if (!skipNpcScript)
		runSceneScript4(0);

	_unk5 = 1;

	if (_mainCharX == -1 || _mainCharY == -1) {
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
		updateCharacterAnim(0);
	}

	return 0;
}

int KyraEngine_HoF::o2_switchScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_switchScene(%p) (%d)", (const void *)script, stackPos(0));
	setGameFlag(0x1EF);
	_mainCharX = _mainCharacter.x1;
	_mainCharY = _mainCharacter.y1;
	_noScriptEnter = false;
	enterNewScene(stackPos(0), _mainCharacter.facing, 0, 0, 0);
	_noScriptEnter = true;
	return 0;
}

int KyraEngine_HoF::o2_setPathfinderFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_setPathfinderFlag(%p) (%d)", (const void *)script, stackPos(0));
	_pathfinderFlag = stackPos(0);
	return 0;
}

int KyraEngine_HoF::o2_getSceneExitToFacing(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_getSceneExitToFacing(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	const int scene = stackPos(0);
	const int facing = stackPos(1);

	if (facing == 0)
		return (int16)_sceneList[scene].exit1;
	else if (facing == 2)
		return (int16)_sceneList[scene].exit2;
	else if (facing == 4)
		return (int16)_sceneList[scene].exit3;
	else if (facing == 6)
		return (int16)_sceneList[scene].exit4;
	return -1;
}

int KyraEngine_HoF::o2_setLayerFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_setLayerFlag(%p) (%d)", (const void *)script, stackPos(0));
	int layer = stackPos(0);
	if (layer >= 1 && layer <= 16)
		_layerFlagTable[layer] = 1;
	return 0;
}

int KyraEngine_HoF::o2_setZanthiaPos(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_setZanthiaPos(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_mainCharX = stackPos(0);
	_mainCharY = stackPos(1);

	if (_mainCharX == -1 && _mainCharY == -1)
		_mainCharacter.animFrame = 32;
	else
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];

	return 0;
}

int KyraEngine_HoF::o2_loadMusicTrack(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_loadMusicTrack(%p) (%d)", (const void *)script, stackPos(0));
	snd_loadSoundFile(stackPos(0));
	return 0;
}

int KyraEngine_HoF::o2_setSceneAnimPos(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_setSceneAnimPos(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	_sceneAnims[stackPos(0)].x = stackPos(1);
	_sceneAnims[stackPos(0)].y = stackPos(2);
	return 0;
}

int KyraEngine_HoF::o2_setCauldronState(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_setCauldronState(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	setCauldronState(stackPos(0), stackPos(1) != 0);
	clearCauldronTable();
	return 0;
}

int KyraEngine_HoF::o2_showItemString(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_showItemString(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	const int item = stackPos(0);

	int string = 0;
	if (stackPos(1) == 1) {
		if (_lang == 1)
			string = getItemCommandStringPickUp(item);
		else
			string = 7;
	} else {
		if (_lang == 1)
			string = getItemCommandStringInv(item);
		else
			string = 8;
	}

	updateCommandLineEx(item+54, string, 0xD6);
	return 0;
}

int KyraEngine_HoF::o2_isAnySoundPlaying(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_isAnySoundPlaying(%p) ()", (const void *)script);

	// WORKAROUND
	//
	// The input script function in the skull scene does busy wait
	// for the sound effect, which is played after completing the
	// song, to finish. To avoid too much CPU use, we add some slight
	// delay here.
	//
	// Also the Nintendo DS backend seems only to update the sound, when
	// either OSystem::updateScreen or OSystem::delayMillis is called.
	// So we have to call delay here, since otherwise the game would hang.
#ifndef __DS__
	if (_currentScene == 16 && _currentChapter == 1)
#endif
		delay(_tickLength);

	return _sound->voiceIsPlaying() ? 1 : 0;
}

int KyraEngine_HoF::o2_setDrawNoShapeFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_setDrawNoShapeFlag(%p) (%d)", (const void *)script, stackPos(0));
	_drawNoShapeFlag = (stackPos(0) != 0);
	return 0;
}

int KyraEngine_HoF::o2_setRunFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_setRunFlag(%p) (%d)", (const void *)script, stackPos(0));
	// this is usually just _runFlag, but since this is just used when the game should play the credits
	// we handle it a bit different :-)
	_showOutro = true;
	_runFlag = false;
	return 0;
}

int KyraEngine_HoF::o2_showLetter(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_showLetter(%p) (%d)", (const void *)script, stackPos(0));
	const int letter = stackPos(0);
	char filename[16];

	_screen->hideMouse();

	showMessage(0, 0xCF);
	displayInvWsaLastFrame();
	backUpPage0();

	_screen->copyPalette(2, 0);

	_screen->clearPage(3);
	_screen->loadBitmap("_NOTE.CPS", 3, 3, 0);

	sprintf(filename, "_NTEPAL%.1d.COL", letter+1);
	_screen->loadPalette(filename, _screen->getPalette(0));

	_screen->fadeToBlack(0x14);

	sprintf(filename, "LETTER%.1d.%s", letter, _languageExtension[_lang]);
	uint8 *letterBuffer = _res->fileData(filename, 0);
	if (!letterBuffer) {
		// some floppy versions use a TXT extension
		sprintf(filename, "LETTER%.1d.TXT", letter);
		letterBuffer = _res->fileData(filename, 0);
	}

	if (letterBuffer) {
		bookDecodeText(letterBuffer);
		bookPrintText(2, letterBuffer, 0xC, 0xA, 0x20);
	}

	_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->fadePalette(_screen->getPalette(0), 0x14);
	_screen->setMouseCursor(0, 0, getShapePtr(0));
	setMousePos(280, 160);

	_screen->showMouse();

	bool running = true;
	while (running) {
		int inputFlag = checkInput(0);
		removeInputTop();

		if (inputFlag == 198 || inputFlag == 199)
			running = false;

		_screen->updateScreen();
		_system->delayMillis(10);
	}

	_screen->hideMouse();
	_screen->fadeToBlack(0x14);
	restorePage0();
	_screen->copyPalette(0, 2);
	_screen->fadePalette(_screen->getPalette(0), 0x14);
	setHandItem(_itemInHand);
	_screen->showMouse();

	return 0;
}

int KyraEngine_HoF::o2_playFireflyScore(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_playFireflyScore(%p) ()", (const void *)script);
	if (_sound->getSfxType() == Sound::kAdLib || _sound->getSfxType() == Sound::kPCSpkr ||
			_sound->getSfxType() == Sound::kMidiMT32 || _sound->getSfxType() == Sound::kMidiGM) {
		snd_playWanderScoreViaMap(86, 1);
		return 1;
	} else {
		return 0;
	}
}

int KyraEngine_HoF::o2_encodeShape(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_encodeShape(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1),
		stackPos(2), stackPos(3), stackPos(4));
	_sceneShapeTable[stackPos(0)] = _screen->encodeShape(stackPos(1), stackPos(2), stackPos(3), stackPos(4), 2);
	return 0;
}

int KyraEngine_HoF::o2_defineSceneAnim(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_defineSceneAnim(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, '%s')", (const void *)script,
			stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8),
			stackPos(9), stackPos(10), stackPos(11), stackPosString(12));
	int animId = stackPos(0);
	SceneAnim &anim = _sceneAnims[animId];
	anim.flags = stackPos(1);
	anim.x = stackPos(2);
	anim.y = stackPos(3);
	anim.x2 = stackPos(4);
	anim.y2 = stackPos(5);
	anim.width = stackPos(6);
	anim.height = stackPos(7);
	anim.specialSize = stackPos(9);
	anim.shapeIndex = stackPos(11);
	if (stackPosString(12) != 0)
		strcpy(anim.filename, stackPosString(12));

	if (anim.flags & 0x40) {
		if (!_sceneAnimMovie[animId]->open(anim.filename, 1, 0))
			error("couldn't load '%s'", anim.filename);

		if (_sceneAnimMovie[animId]->xAdd() || _sceneAnimMovie[animId]->yAdd())
			anim.wsaFlag = 1;
		else
			anim.wsaFlag = 0;
	}

	return 0;
}

int KyraEngine_HoF::o2_updateSceneAnim(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_updateSceneAnim(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	updateSceneAnim(stackPos(0), stackPos(1));

	// HACK: Some animations are really too fast because of missing delay times.
	// Notice that the delay time is purely subjective set here, it could look
	// slower or maybe faster in the original, but at least this looks OK for
	// Raziel^.
	//
	// We know currently of some different animations where this happens.
	// - Where Marco is dangling from the flesh-eating plant (see bug
	//   #1923638 "HoF: Marco missing animation frames").
	// - After giving the ticket to the captain. He would move very fast
	//   (barely noticeable) onto the ship without this delay.
	// - The scene after giving the sandwitch to the guards in the city.
	//   (see bug #1926838 "HoF: Animation plays too fast")
	//   This scene script calls o2_delay though, but since this updates
	//   the scene animation scripts again there is no delay for the
	//   animation.
	// - When the sheriff enters the jail, either to lock you up or to throw
	//   away the key. (see bug #1926838 "HoF: Animation plays too fast").

	if ((stackPos(0) == 2 && _mainCharacter.sceneId == 3) ||
			(stackPos(0) == 3 && _mainCharacter.sceneId == 33) ||
			((stackPos(0) == 1 || stackPos(0) == 2) && _mainCharacter.sceneId == 19) ||
			((stackPos(0) == 1 || stackPos(0) == 2) && _mainCharacter.sceneId == 27))
		_sceneSpecialScriptsTimer[_lastProcessedSceneScript] = _system->getMillis() + _tickLength * 6;

	_specialSceneScriptRunFlag = false;
	return 0;
}

int KyraEngine_HoF::o2_addToSceneAnimPosAndUpdate(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_addToSceneAnimPosAndUpdate(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	const int anim = stackPos(0);
	_sceneAnims[anim].x2 += stackPos(1);
	_sceneAnims[anim].y2 += stackPos(2);
	if (_sceneAnims[anim].flags & 2) {
		_sceneAnims[anim].x += stackPos(1);
		_sceneAnims[anim].y += stackPos(2);
	}
	updateSceneAnim(anim, stackPos(3));
	_specialSceneScriptRunFlag = false;
	return 0;
}

int KyraEngine_HoF::o2_useItemOnMainChar(EMCState *script) {
	EMCState tmpScript;
	_emc->init(&tmpScript, &_npcScriptData);
	_emc->start(&tmpScript, 0);
	tmpScript.regs[4] = _itemInHand;
	tmpScript.regs[0] = _mainCharacter.sceneId;

	int oldVocH = _vocHigh;
	_vocHigh = 0x5a;

	while (_emc->isValid(&tmpScript))
		_emc->run(&tmpScript);

	_vocHigh = oldVocH;

	return 0;
}

int KyraEngine_HoF::o2_startDialogue(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_startDialogue(%p) (%d)", (const void *)script, stackPos(0));
	startDialogue(stackPos(0));
	return 0;
}

int KyraEngine_HoF::o2_addCauldronStateTableEntry(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_addCauldronStateTableEntry(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	return addToCauldronStateTable(stackPos(0), stackPos(1)) ? 1 : 0;
}

int KyraEngine_HoF::o2_setCountDown(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_setCountDown(%p) (%d)", (const void *)script, stackPos(0));
	_scriptCountDown = _system->getMillis() + stackPos(0) * _tickLength;
	return 0;
}

int KyraEngine_HoF::o2_getCountDown(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_getCountDown(%p)", (const void *)script);
	uint32 time = _system->getMillis();
	return (time > _scriptCountDown) ? 0 : (_scriptCountDown - time) / _tickLength;
}

int KyraEngine_HoF::o2_pressColorKey(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_pressColorKey(%p) (%d)", (const void *)script, stackPos(0));
	for (int i = 6; i; i--)
		_inputColorCode[i] = _inputColorCode[i - 1];
	_inputColorCode[0] = stackPos(0) & 0xff;
	for (int i = 0; i < 7; i++) {
		if (_presetColorCode[i] != _inputColorCode[6 - i])
			return _dbgPass;
	}
	return 1;
}

int KyraEngine_HoF::o2_objectChat(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_objectChat(%p) ('%s', %d)", (const void *)script, stackPosString(0), stackPos(1));
	if (_flags.isTalkie)
		warning("Unexpected call: o2_objectChat(%p) ('%s', %d)", (const void *)script, stackPosString(0), stackPos(1));
	else
		objectChat(stackPosString(0), stackPos(1));
	return 0;
}

int KyraEngine_HoF::o2_changeChapter(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_changeChapter(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	const int chapter = stackPos(0);
	const int scene = stackPos(1);

	resetItemList();

	_newChapterFile = chapter;
	runStartScript(chapter, 0);

	_mainCharacter.dlgIndex = 0;
	memset(_newSceneDlgState, 0, 32);

	static const int zShapeList[] = { 1, 2, 2, 2, 4 };
	assert(chapter > 1 && chapter <= ARRAYSIZE(zShapeList));
	loadCharacterShapes(zShapeList[chapter-1]);

	enterNewScene(scene, (chapter == 2) ? 2 : 0, 0, 0, 0);

	return 0;
}

int KyraEngine_HoF::o2_getColorCodeFlag1(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_getColorCodeFlag1(%p) ()", (const void *)script);
	return _colorCodeFlag1;
}

int KyraEngine_HoF::o2_setColorCodeFlag1(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_getColorCodeFlag1(%p) (%d)", (const void *)script, stackPos(0));
	_colorCodeFlag1 = stackPos(0);
	return 0;
}

int KyraEngine_HoF::o2_getColorCodeFlag2(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_getColorCodeFlag2(%p) ()", (const void *)script);
	return _colorCodeFlag2;
}

int KyraEngine_HoF::o2_setColorCodeFlag2(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_getColorCodeFlag2(%p) (%d)", (const void *)script, stackPos(0));
	_colorCodeFlag2 = stackPos(0);
	return 0;
}

int KyraEngine_HoF::o2_getColorCodeValue(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_getColorCodeValue(%p) (%d)", (const void *)script, stackPos(0));
	return _presetColorCode[stackPos(0)];
}

int KyraEngine_HoF::o2_setColorCodeValue(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_setColorCodeValue(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_presetColorCode[stackPos(0)] = stackPos(1) & 0xff;
	return stackPos(1) & 0xff;
}

int KyraEngine_HoF::o2_countItemInstances(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_countItemInstances(%p) (%d)", (const void *)script, stackPos(0));
	Item item = stackPos(0);

	int count = 0;
	for (int i = 0; i < 20; ++i) {
		if (_mainCharacter.inventory[i] == item)
			++count;
	}

	if (_itemInHand == item)
		++count;

	for (int i = 0; i < 30; ++i) {
		if (_itemList[i].id == item)
			++count;
	}

	if (_hiddenItems[0] == item && _newChapterFile == 1)
		++count;
	if (_hiddenItems[1] == item && _newChapterFile == 1)
		++count;
	if (_hiddenItems[2] == item && _newChapterFile == 2)
		++count;
	if (_hiddenItems[3] == item && _newChapterFile == 2)
		++count;
	if (_hiddenItems[4] == item && _newChapterFile == 1)
		++count;

	return count;
}

int KyraEngine_HoF::o2_removeItemFromScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_removeItemFromScene(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	const int scene = stackPos(0);
	const uint16 item = stackPos(1);
	for (int i = 0; i < 30; ++i) {
		if (_itemList[i].sceneId == scene && _itemList[i].id == item)
			_itemList[i].id = kItemNone;
	}
	return 0;
}

int KyraEngine_HoF::o2_initObject(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_initObject(%p) (%d)", (const void *)script, stackPos(0));
	initTalkObject(stackPos(0));
	return 0;
}

int KyraEngine_HoF::o2_npcChat(EMCState *script) {
	if (_flags.isTalkie) {
		debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_npcChat(%p) ('%s', %d, %d, %d)", (const void *)script, stackPosString(0), stackPos(1), _vocHigh, stackPos(2));
		npcChatSequence(stackPosString(0), stackPos(1), _vocHigh, stackPos(2));
	} else {
		debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_npcChat(%p) ('%s', %d)", (const void *)script, stackPosString(0), stackPos(1));
		npcChatSequence(stackPosString(0), stackPos(1));
	}
	return 0;
}

int KyraEngine_HoF::o2_deinitObject(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_deinitObject(%p) (%d)", (const void *)script, stackPos(0));
	deinitTalkObject(stackPos(0));
	return 0;
}

int KyraEngine_HoF::o2_playTimSequence(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_playTimSequence(%p) ('%s')", (const void *)script, stackPosString(0));
	playTim(stackPosString(0));
	return 0;
}

int KyraEngine_HoF::o2_makeBookOrCauldronAppear(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_makeBookOrCauldronAppear(%p) (%d)", (const void *)script, stackPos(0));
	seq_makeBookOrCauldronAppear(stackPos(0));
	return 0;
}

int KyraEngine_HoF::o2_resetInputColorCode(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_resetInputColorCode(%p)", (const void *)script);
	memset(_inputColorCode, 255, 7);
	return 0;
}

int KyraEngine_HoF::o2_mushroomEffect(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_mushroomEffect(%p)", (const void *)script);
	_screen->copyPalette(2, 0);

	for (int i = 1; i < 768; i += 3)
		_screen->getPalette(0)[i] = 0;
	snd_playSoundEffect(106);
	_screen->fadePalette(_screen->getPalette(0), 90, &_updateFunctor);
	_screen->copyPalette(0, 2);

	for (int i = 0; i < 768; i += 3) {
		_screen->getPalette(0)[i] = _screen->getPalette(0)[i + 1] = 0;
		_screen->getPalette(0)[i + 2] += (((int8)_screen->getPalette(0)[i + 2]) >> 1);
		if (_screen->getPalette(0)[i + 2] > 63)
			_screen->getPalette(0)[i + 2] = 63;
	}
	snd_playSoundEffect(106);
	_screen->fadePalette(_screen->getPalette(0), 90, &_updateFunctor);

	_screen->copyPalette(0, 2);
	_screen->fadePalette(_screen->getPalette(0), 30, &_updateFunctor);

	return 0;
}

int KyraEngine_HoF::o2_customChat(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_customChat(%p) ('%s', %d, %d)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2));
	strcpy((char *)_unkBuf500Bytes, stackPosString(0));
	_chatText = (char *)_unkBuf500Bytes;
	_chatObject = stackPos(1);

	_chatVocHigh = _chatVocLow = -1;
	objectChatInit(_chatText, _chatObject, _vocHigh, stackPos(2));
	playVoice(_vocHigh, stackPos(2));
	return 0;
}

int KyraEngine_HoF::o2_customChatFinish(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_customChatFinish(%p) ()", (const void *)script);
	_text->restoreScreen();
	_chatText = 0;
	_chatObject = -1;
	return 0;
}

int KyraEngine_HoF::o2_setupSceneAnimation(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_setupSceneAnimation(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, '%s')", (const void *)script,
		stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8), stackPos(9), stackPos(10), stackPos(11), stackPosString(12));
	const int index = stackPos(0);
	const uint16 flags = stackPos(1);

	restorePage3();

	SceneAnim &anim = _sceneAnims[index];
	anim.flags = flags;
	anim.x = stackPos(2);
	anim.y = stackPos(3);
	anim.x2 = stackPos(4);
	anim.y2 = stackPos(5);
	anim.width = stackPos(6);
	anim.height = stackPos(7);
	anim.specialSize = stackPos(9);
	anim.shapeIndex = stackPos(11);
	if (stackPosString(12))
		strcpy(anim.filename, stackPosString(12));

	if (flags & 0x40) {
		_sceneAnimMovie[index]->open(stackPosString(12), 0, 0);
		if (_sceneAnimMovie[index]->xAdd() || _sceneAnimMovie[index]->yAdd())
			anim.wsaFlag = 1;
		else
			anim.wsaFlag = 0;
	}

	AnimObj *obj = &_animObjects[1+index];
	obj->enabled = 1;
	obj->needRefresh = 1;
	obj->specialRefresh = 1;
	obj->animFlags = anim.flags & 8;

	if (anim.flags & 2)
		obj->flags = 0x800;
	else
		obj->flags = 0;

	if (anim.flags & 4)
		obj->flags |= 1;

	obj->xPos1 = anim.x;
	obj->yPos1 = anim.y;

	if ((anim.flags & 0x20) && anim.shapeIndex >= 0)
		obj->shapePtr = _sceneShapeTable[anim.shapeIndex];
	else
		obj->shapePtr = 0;

	if (anim.flags & 0x40) {
		obj->shapeIndex3 = anim.shapeIndex;
		obj->animNum = index;
	} else {
		obj->shapeIndex3 = 0xFFFF;
		obj->animNum = 0xFFFF;
	}

	obj->shapeIndex2 = 0xFFFF;
	obj->xPos2 = obj->xPos3 = anim.x2;
	obj->yPos2 = obj->yPos3 = anim.y2;
	obj->width = anim.width;
	obj->height = anim.height;
	obj->width2 = obj->height2 = anim.specialSize;

	_animList = addToAnimListSorted(_animList, obj);
	obj->needRefresh = 1;
	obj->specialRefresh = 1;
	return 0;
}

int KyraEngine_HoF::o2_stopSceneAnimation(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_stopSceneAnimation(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	const int index = stackPos(0);
	AnimObj &obj = _animObjects[1+index];
	restorePage3();
	obj.shapeIndex3 = 0xFFFF;
	obj.animNum = 0xFFFF;
	obj.needRefresh = 1;
	obj.specialRefresh = 1;
	if (stackPos(1))
		refreshAnimObjectsIfNeed();
	obj.enabled = 0;
	_animList = deleteAnimListEntry(_animList, &_animObjects[1+index]);

	if (_sceneAnimMovie[index]->opened())
		_sceneAnimMovie[index]->close();

	return 0;
}

int KyraEngine_HoF::o2_processPaletteIndex(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_processPaletteIndex(%p) (%d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	Palette &palette = _screen->getPalette(0);

	const int index = stackPos(0);
	const bool updatePalette = (stackPos(4) != 0);
	const int delayTime = stackPos(5);

	palette[index*3+0] = (stackPos(1) * 0x3F) / 100;
	palette[index*3+1] = (stackPos(2) * 0x3F) / 100;
	palette[index*3+2] = (stackPos(3) * 0x3F) / 100;

	if (updatePalette) {
		if (delayTime > 0)
			_screen->fadePalette(palette, delayTime, &_updateFunctor);
		else
			_screen->setScreenPalette(palette);
	}

	return 0;
}

int KyraEngine_HoF::o2_updateTwoSceneAnims(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_updateTwoSceneAnims(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	updateSceneAnim(stackPos(0), stackPos(1));
	updateSceneAnim(stackPos(2), stackPos(3));
	_specialSceneScriptRunFlag = false;
	return 0;
}

int KyraEngine_HoF::o2_getRainbowRoomData(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_getRainbowRoomData(%p) (%d)", (const void *)script, stackPos(0));
	return _rainbowRoomData[stackPos(0)];
}

int KyraEngine_HoF::o2_drawSceneShapeEx(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_drawSceneShapeEx(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	const int itemShape = stackPos(0) + 64;
	const int x = stackPos(1);
	const int y = stackPos(2);
	const bool skipFronUpdate = (stackPos(3) != 0);

	_screen->drawShape(2, _sceneShapeTable[6], x, y, 2, 0);
	_screen->drawShape(2, getShapePtr(itemShape), x+2, y+2, 2, 0);

	if (!skipFronUpdate) {
		_screen->copyRegion(x, y, x, y, 0x15, 0x14, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	}

	return 0;
}

int KyraEngine_HoF::o2_midiSoundFadeout(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_midiSoundFadeout(%p) ()", (const void *)script);
	if (!stackPos(0)) {
		if ((_sound->getMusicType() == Sound::kMidiMT32 || _sound->getMusicType() == Sound::kMidiGM) &&
			(_sound->getSfxType() == Sound::kMidiMT32 || _sound->getSfxType() == Sound::kMidiGM)) {
			_sound->beginFadeOut();
			delay(2000, true);
			_lastMusicCommand = -1;
		} else {
			return 0;
		}
	}

	return 1;
}

int KyraEngine_HoF::o2_getSfxDriver(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_getSfxDriver(%p) ()", (const void *)script);
	if (_sound->getSfxType() == Sound::kAdLib)
		return 1;
	else if (_sound->getSfxType() == Sound::kPCSpkr)
		return 4;
	else if (_sound->getSfxType() == Sound::kMidiMT32)
		return 6;
	else if (_sound->getSfxType() == Sound::kMidiGM)
		return 7;
	// TODO: find nice default value
	return 0;
}

int KyraEngine_HoF::o2_getVocSupport(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_getVocSupport(%p) ()", (const void *)script);
	// we always support VOC file playback
	return 1;
}

int KyraEngine_HoF::o2_getMusicDriver(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_getMusicDriver(%p) ()", (const void *)script);
	if (_sound->getMusicType() == Sound::kAdLib)
		return 1;
	else if (_sound->getMusicType() == Sound::kPCSpkr)
		return 4;
	else if (_sound->getMusicType() == Sound::kMidiMT32)
		return 6;
	else if (_sound->getMusicType() == Sound::kMidiGM)
		return 7;
	// TODO: find nice default value
	return 0;
}

int KyraEngine_HoF::o2_zanthiaChat(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_zanthiaChat(%p) ('%s', %d)", (const void *)script, stackPosString(0), stackPos(1));
	objectChat(stackPosString(0), 0, _vocHigh, stackPos(1));
	return 0;
}

int KyraEngine_HoF::o2_isVoiceEnabled(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_isVoiceEnabled(%p) ()", (const void *)script);
	return speechEnabled() ? 1 : 0;
}

int KyraEngine_HoF::o2_isVoicePlaying(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_isVoicePlaying(%p) ()", (const void *)script);
	return (snd_voiceIsPlaying() && !skipFlag()) ? 1 : 0;
}

int KyraEngine_HoF::o2_stopVoicePlaying(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_stopVoicePlaying(%p) ()", (const void *)script);
	snd_stopVoice();
	return 0;
}

int KyraEngine_HoF::o2_getGameLanguage(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_getGameLanguage(%p) ()", (const void *)script);
	return _lang;
}

int KyraEngine_HoF::o2_demoFinale(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_demoFinale(%p) ()", (const void *)script);
	if (!_flags.isDemo)
		return 0;

	int tmpSize;
	const char *const *strings = _staticres->loadStrings(k2IngameTlkDemoStrings, tmpSize);
	assert(strings);

	_screen->clearPage(0);
	_screen->loadPalette("THANKS.COL", _screen->getPalette(0));
	_screen->loadBitmap("THANKS.CPS", 3, 3, 0);
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0);

	_screen->_curPage = 0;
	int y = _lang == 1 ? 70 : 65;
	for (int i = 0; i < 6; i++)
		_text->printText(strings[i], _text->getCenterStringX(strings[i], 1, 319), y + i * 10, 255, 207, 0);

	_screen->setScreenPalette(_screen->getPalette(0));
	_screen->updateScreen();

	_eventList.clear();
	while (!skipFlag() && !shouldQuit())
		delay(10);

	_sound->beginFadeOut();
	_screen->fadeToBlack();

	_runFlag = 0;
	return 0;
}

int KyraEngine_HoF::o2_dummy(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2_dummy(%p) ()", (const void *)script);
	return 0;
}

#pragma mark -

int KyraEngine_HoF::o2a_setCharacterFrame(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::o2a_setCharacterFrame(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_animNewFrame = stackPos(0);
	_animDelayTime = stackPos(1);
	_animNeedUpdate = true;
	return 0;
}

#pragma mark -

int KyraEngine_HoF::t2_initChat(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::t2_initChat(%p, %p) (%d)", (const void *)tim, (const void *)param, param[0]);
	_chatText = (const char *)tim->text + READ_LE_UINT16(tim->text + (param[0] << 1));
	_chatObject = param[1];

	if (_flags.lang == Common::JA_JPN) {
		for (int i = 0; i < _ingameTimJpStrSize; i += 2) {
			if (!scumm_stricmp(_chatText, _ingameTimJpStr[i]))
				_chatText = _ingameTimJpStr[i + 1];
		}
	}

	objectChatInit(_chatText, _chatObject);
	return 0;
}

int KyraEngine_HoF::t2_updateSceneAnim(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::t2_updateSceneAnim(%p, %p) (%d, %d)", (const void *)tim, (const void *)param, param[0], param[1]);
	updateSceneAnim(param[1], param[0]);
	return 0;
}

int KyraEngine_HoF::t2_resetChat(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::t2_resetChat(%p, %p) ()", (const void *)tim, (const void *)param);
	_text->restoreScreen();
	_chatText = 0;
	_chatObject = -1;
	return 0;
}

int KyraEngine_HoF::t2_playSoundEffect(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_HoF::t2_playSoundEffect(%p, %p) (%d)", (const void *)tim, (const void *)param, param[0]);
	snd_playSoundEffect(*param);
	return 0;
}

#pragma mark -

typedef Common::Functor1Mem<EMCState *, int, KyraEngine_HoF> OpcodeV2;
#define SetOpcodeTable(x) table = &x;
#define Opcode(x) table->push_back(new OpcodeV2(this, &KyraEngine_HoF::x))
#define OpcodeUnImpl() table->push_back(new OpcodeV2(this, 0))

typedef Common::Functor2Mem<const TIM *, const uint16 *, int, KyraEngine_HoF> TIMOpcodeV2;
#define OpcodeTim(x) _timOpcodes.push_back(new TIMOpcodeV2(this, &KyraEngine_HoF::x))
#define OpcodeTimUnImpl() _timOpcodes.push_back(new TIMOpcodeV2(this, 0))

void KyraEngine_HoF::setupOpcodeTable() {
	Common::Array<const Opcode *> *table = 0;

	_opcodes.reserve(176);
	SetOpcodeTable(_opcodes);
	// 0x00
	Opcode(o2_setCharacterFacingRefresh);
	Opcode(o2_setCharacterPos);
	Opcode(o2_defineObject);
	Opcode(o2_refreshCharacter);
	// 0x04
	Opcode(o2_getCharacterX);
	Opcode(o2_getCharacterY);
	Opcode(o2_getCharacterFacing);
	Opcode(o2_getCharacterScene);
	// 0x08
	Opcode(o2_setSceneComment);
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(o2_setCharacterAnimFrame);
	// 0x0c
	Opcode(o2_setCharacterFacingOverwrite);
	Opcode(o2_trySceneChange);
	Opcode(o2_moveCharacter);
	Opcode(o2_customCharacterChat);
	// 0x10
	Opcode(o2_soundFadeOut);
	Opcode(o2_showChapterMessage);
	Opcode(o2_restoreTalkTextMessageBkgd);
	OpcodeUnImpl();
	// 0x14
	Opcode(o2_wsaClose);
	Opcode(o2_backUpScreen);
	Opcode(o2_restoreScreen);
	Opcode(o2_displayWsaFrame);
	// 0x18
	Opcode(o2_displayWsaSequentialFramesLooping);
	Opcode(o2_wsaOpen);
	Opcode(o2_displayWsaSequentialFrames);
	Opcode(o2_displayWsaSequence);
	// 0x1c
	Opcode(o2_addItemToInventory);
	Opcode(o2_drawShape);
	Opcode(o2_addItemToCurScene);
	Opcode(o2_dummy);	// the original used this opcode to limit the mouse range temporary,
						// since that is of no use and not really important we just use a dummy here
	// 0x20
	Opcode(o2_checkForItem);
	Opcode(o2_loadSoundFile);
	Opcode(o2_removeSlotFromInventory);
	Opcode(o2_defineItem);
	// 0x24
	Opcode(o2_removeItemFromInventory);
	Opcode(o2_countItemInInventory);
	Opcode(o2_countItemsInScene);
	Opcode(o1_queryGameFlag);
	// 0x28
	Opcode(o1_resetGameFlag);
	Opcode(o1_setGameFlag);
	Opcode(o1_setHandItem);
	Opcode(o1_removeHandItem);
	// 0x2c
	Opcode(o1_getMouseState);
	Opcode(o1_hideMouse);
	Opcode(o2_addSpecialExit);
	Opcode(o1_setMousePos);
	// 0x30
	Opcode(o1_showMouse);
	OpcodeUnImpl();
	Opcode(o2_wipeDownMouseItem);
	Opcode(o2_getElapsedSecs);
	// 0x34
	Opcode(o2_getTimerDelay);
	Opcode(o1_playSoundEffect);
	Opcode(o2_delaySecs);
	Opcode(o2_delay);
	// 0x38
	Opcode(o2_dummy);
	Opcode(o2_setTimerDelay);
	Opcode(o2_setScaleTableItem);
	Opcode(o2_setDrawLayerTableItem);
	// 0x3c
	Opcode(o2_setCharPalEntry);
	Opcode(o2_loadZShapes);
	Opcode(o2_drawSceneShape);
	Opcode(o2_drawSceneShapeOnPage);
	// 0x40
	Opcode(o2_disableAnimObject);
	Opcode(o2_enableAnimObject);
	Opcode(o2_dummy);
	Opcode(o2_loadPalette384);
	// 0x44
	Opcode(o2_setPalette384);
	Opcode(o2_restoreBackBuffer);
	Opcode(o2_backUpInventoryGfx);
	Opcode(o2_disableSceneAnim);
	// 0x48
	Opcode(o2_enableSceneAnim);
	Opcode(o2_restoreInventoryGfx);
	Opcode(o2_setSceneAnimPos2);
	Opcode(o2_update);
	// 0x4c
	OpcodeUnImpl();
	Opcode(o2_fadeScenePal);
	Opcode(o2_dummy);
	Opcode(o2_dummy);
	// 0x50
	Opcode(o2_enterNewScene);
	Opcode(o2_switchScene);
	Opcode(o2_getShapeFlag1);
	Opcode(o2_setPathfinderFlag);
	// 0x54
	Opcode(o2_getSceneExitToFacing);
	Opcode(o2_setLayerFlag);
	Opcode(o2_setZanthiaPos);
	Opcode(o2_loadMusicTrack);
	// 0x58
	Opcode(o1_playWanderScoreViaMap);
	Opcode(o1_playSoundEffect);
	Opcode(o2_setSceneAnimPos);
	Opcode(o1_blockInWalkableRegion);
	// 0x5c
	Opcode(o1_blockOutWalkableRegion);
	OpcodeUnImpl();
	Opcode(o2_setCauldronState);
	Opcode(o2_showItemString);
	// 0x60
	Opcode(o1_getRand);
	Opcode(o2_isAnySoundPlaying);
	Opcode(o1_setDeathHandler);
	Opcode(o2_setDrawNoShapeFlag);
	// 0x64
	Opcode(o2_setRunFlag);
	Opcode(o2_showLetter);
	OpcodeUnImpl();
	Opcode(o1_fillRect);
	// 0x68
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(o2_playFireflyScore);
	Opcode(o2_waitForConfirmationClick);
	// 0x6c
	Opcode(o2_encodeShape);
	Opcode(o2_defineRoomEntrance);
	Opcode(o2_runAnimationScript);
	Opcode(o2_setSpecialSceneScriptRunTime);
	// 0x70
	Opcode(o2_defineSceneAnim);
	Opcode(o2_updateSceneAnim);
	Opcode(o2_updateSceneAnim);
	Opcode(o2_addToSceneAnimPosAndUpdate);
	// 0x74
	Opcode(o2_useItemOnMainChar);
	Opcode(o2_startDialogue);
	Opcode(o2_randomSceneChat);
	Opcode(o2_setDlgIndex);
	// 0x78
	Opcode(o2_getDlgIndex);
	Opcode(o2_defineScene);
	Opcode(o2_addCauldronStateTableEntry);
	Opcode(o2_setCountDown);
	// 0x7c
	Opcode(o2_getCountDown);
	Opcode(o2_dummy);
	Opcode(o2_dummy);
	Opcode(o2_pressColorKey);
	// 0x80
	Opcode(o2_objectChat);
	Opcode(o2_changeChapter);
	Opcode(o2_getColorCodeFlag1);
	Opcode(o2_setColorCodeFlag1);
	// 0x84
	Opcode(o2_getColorCodeFlag2);
	Opcode(o2_setColorCodeFlag2);
	Opcode(o2_getColorCodeValue);
	Opcode(o2_setColorCodeValue);
	// 0x88
	Opcode(o2_countItemInstances);
	Opcode(o2_removeItemFromScene);
	Opcode(o2_initObject);
	Opcode(o2_npcChat);
	// 0x8c
	Opcode(o2_deinitObject);
	Opcode(o2_playTimSequence);
	Opcode(o2_makeBookOrCauldronAppear);
	Opcode(o2_setSpecialSceneScriptState);
	// 0x90
	Opcode(o2_clearSpecialSceneScriptState);
	Opcode(o2_querySpecialSceneScriptState);
	Opcode(o2_resetInputColorCode);
	Opcode(o2_setHiddenItemsEntry);
	// 0x94
	Opcode(o2_getHiddenItemsEntry);
	Opcode(o2_mushroomEffect);
	Opcode(o2_wsaClose);
	Opcode(o2_meanWhileScene);
	// 0x98
	Opcode(o2_customChat);
	Opcode(o2_customChatFinish);
	Opcode(o2_setupSceneAnimation);
	Opcode(o2_stopSceneAnimation);
	// 0x9c
	Opcode(o2_disableTimer);
	Opcode(o2_enableTimer);
	Opcode(o2_setTimerCountdown);
	Opcode(o2_processPaletteIndex);
	// 0xa0
	Opcode(o2_updateTwoSceneAnims);
	Opcode(o2_getRainbowRoomData);
	Opcode(o2_drawSceneShapeEx);
	Opcode(o2_midiSoundFadeout);
	// 0xa4
	Opcode(o2_getSfxDriver);
	Opcode(o2_getVocSupport);
	Opcode(o2_getMusicDriver);
	Opcode(o2_setVocHigh);
	// 0xa8
	Opcode(o2_getVocHigh);
	Opcode(o2_zanthiaChat);
	Opcode(o2_isVoiceEnabled);
	Opcode(o2_isVoicePlaying);
	// 0xac
	Opcode(o2_stopVoicePlaying);
	Opcode(o2_getGameLanguage);
	Opcode(o2_demoFinale);
	Opcode(o2_dummy);

	_opcodesAnimation.reserve(6);
	SetOpcodeTable(_opcodesAnimation);

	// 0x00
	Opcode(o2a_setAnimationShapes);
	Opcode(o2a_setCharacterFrame);
	Opcode(o1_playSoundEffect);
	Opcode(o2_fadeScenePal);
	// 0x04
	_flags.isTalkie ? Opcode(o2a_setResetFrame) : Opcode(o2_dummy);
	Opcode(o2_dummy);

	// ---- TIM opcodes

	_timOpcodes.reserve(4);
	// 0x00
	OpcodeTim(t2_initChat);
	OpcodeTim(t2_updateSceneAnim);
	OpcodeTim(t2_resetChat);
	OpcodeTim(t2_playSoundEffect);
}

} // End of namespace Kyra
