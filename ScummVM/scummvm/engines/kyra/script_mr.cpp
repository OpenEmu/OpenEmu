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
#include "kyra/text_mr.h"
#include "kyra/resource.h"

#include "common/system.h"

namespace Kyra {

int KyraEngine_MR::o3_getMalcolmShapes(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_getMaloclmShapes(%p) ()", (const void *)script);
	return _characterShapeFile;
}

int KyraEngine_MR::o3_setCharacterPos(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_setCharacterPos(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	int x = stackPos(0);
	int y = stackPos(1);

	if (x != -1 && y != -1) {
		x &= ~3;
		y &= ~1;
	}

	_mainCharacter.x1 = _mainCharacter.x2 = x;
	_mainCharacter.y1 = _mainCharacter.y2 = y;

	return 0;
}

int KyraEngine_MR::o3_defineObject(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_defineObject(%p) (%d, '%s', %d, %d, %d, %d, %d, %d)", (const void *)script,
			stackPos(0), stackPosString(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7));
	TalkObject &obj = _talkObjectList[stackPos(0)];
	strcpy(obj.filename, stackPosString(1));
	obj.sceneAnim = stackPos(2);
	obj.sceneScript = stackPos(3);
	obj.x = stackPos(4);
	obj.y = stackPos(5);
	obj.color = stackPos(6);
	obj.sceneId = stackPos(7);
	return 0;
}

int KyraEngine_MR::o3_refreshCharacter(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_refreshCharacter(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	const int frame = stackPos(0);
	const int facing = stackPos(1);
	const bool updateNeed = stackPos(2) != 0;

	if (facing >= 0)
		_mainCharacter.facing = facing;

	if (frame >= 0 && frame != 87)
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
	else
		_mainCharacter.animFrame = 87;

	updateCharacterAnim(0);

	if (updateNeed)
		refreshAnimObjectsIfNeed();
	return 0;
}

int KyraEngine_MR::o3_getMalcolmsMood(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_getMalcolmsMood(%p) ()", (const void *)script);
	return _malcolmsMood;
}

int KyraEngine_MR::o3_getCharacterFrameFromFacing(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_getCharacterFrameFromFacing(%p) ()", (const void *)script);
	return _characterFrameTable[_mainCharacter.facing];
}

int KyraEngine_MR::o3_setCharacterFacing(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_setCharacterFacing(%p) (%d)", (const void *)script, stackPos(0));
	_mainCharacter.facing = stackPos(0);
	return 0;
}

int KyraEngine_MR::o3_showSceneFileMessage(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_showSceneFileMessage(%p) (%d)", (const void *)script, stackPos(0));
	showMessage((const char *)getTableEntry(_scenesFile, stackPos(0)), 0xFF, 0xF0);
	return 0;
}

int KyraEngine_MR::o3_setCharacterAnimFrameFromFacing(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_setCharacterAnimFrameFromFacing(%p) ()", (const void *)script);
	updateCharPal(0);
	_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
	updateCharacterAnim(0);
	refreshAnimObjectsIfNeed();
	return 0;
}

int KyraEngine_MR::o3_showBadConscience(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_showBadConscience(%p) ()", (const void *)script);
	showBadConscience();
	return 0;
}

int KyraEngine_MR::o3_hideBadConscience(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_hideBadConscience(%p) ()", (const void *)script);
	hideBadConscience();
	return 0;
}

int KyraEngine_MR::o3_showAlbum(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_showAlbum(%p) ()", (const void *)script);
	showAlbum();
	return 0;
}

int KyraEngine_MR::o3_setInventorySlot(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_setInventorySlot(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	const int slot = MAX<int16>(0, MIN<int16>(10, stackPos(0)));
	return (_mainCharacter.inventory[slot] = stackPos(1));
}

int KyraEngine_MR::o3_getInventorySlot(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_getInventorySlot(%p) (%d)", (const void *)script, stackPos(0));
	return _mainCharacter.inventory[stackPos(0)];
}

int KyraEngine_MR::o3_addItemToInventory(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_addItemToInventory(%p) (%d)", (const void *)script, stackPos(0));
	int slot = findFreeInventorySlot();
	if (slot >= 0) {
		_mainCharacter.inventory[slot] = stackPos(0);
		if (_inventoryState) {
			redrawInventory(0);
		}
	}
	return slot;
}

int KyraEngine_MR::o3_addItemToCurScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_addItemToCurScene(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	const uint16 item = stackPos(0);
	int x = stackPos(1);
	int y = stackPos(2);
	int itemSlot = findFreeItem();

	if (x < 20)
		x = 20;
	else if (x > 299)
		x = 299;

	if (y < 18)
		y = 18;
	else if (y > 187)
		y = 187;

	if (itemSlot >= 0) {
		_itemList[itemSlot].x = x;
		_itemList[itemSlot].y = y;
		_itemList[itemSlot].id = item;
		_itemList[itemSlot].sceneId = _mainCharacter.sceneId;
		addItemToAnimList(itemSlot);
		refreshAnimObjectsIfNeed();
	}

	return itemSlot;
}

int KyraEngine_MR::o3_objectChat(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_objectChat(%p) (%d)", (const void *)script, stackPos(0));
	int id = stackPos(0);
	const char *str = (const char *)getTableEntry(_useActorBuffer ? _actorFile : _sceneStrings, id);
	if (str) {
		objectChat(str, 0, _vocHigh, id);
		playStudioSFX(str);
	}
	return 0;
}

int KyraEngine_MR::o3_resetInventory(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_resetInventory(%p) ()", (const void *)script);
	memset(_mainCharacter.inventory, -1, sizeof(_mainCharacter.inventory));
	return 0;
}

int KyraEngine_MR::o3_removeInventoryItemInstances(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_removeInventoryItemInstances(%p) (%d)", (const void *)script, stackPos(0));
	const int item = stackPos(0);
	for (int i = 0; i < 10; ++i) {
		if (_mainCharacter.inventory[i] == item)
			_mainCharacter.inventory[i] = kItemNone;
	}
	return 0;
}

int KyraEngine_MR::o3_countInventoryItemInstances(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_countInventoryItemInstances(%p) (%d)", (const void *)script, stackPos(0));
	const int item = stackPos(0);
	int count = 0;

	for (int i = 0; i < 10; ++i) {
		if (_mainCharacter.inventory[i] == item)
			++count;
	}

	if (_itemInHand == item)
		++count;

	return count;
}

int KyraEngine_MR::o3_npcChatSequence(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_npcChatSequence(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	const int id = stackPos(0);
	const char *str = (const char *)getTableEntry(_sceneStrings, id);
	if (str)
		npcChatSequence(str, stackPos(1), _vocHigh, id);
	return 0;
}

int KyraEngine_MR::o3_badConscienceChat(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_badConscienceChat(%p) (%d)", (const void *)script, stackPos(0));
	int id = stackPos(0);
	const char *str = (const char *)getTableEntry(_useActorBuffer ? _actorFile : _sceneStrings, id);
	badConscienceChat(str, _vocHigh, id);
	return 0;
}

int KyraEngine_MR::o3_wipeDownMouseItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o3_wipeDownMouseItem(%p) (-, %d, %d)", (const void *)script, stackPos(1), stackPos(2));
	_screen->hideMouse();
	const int x = stackPos(1) - 12;
	const int y = stackPos(2) - 19;

	if (_itemInHand >= 0) {
		backUpGfxRect32x32(x, y);
		uint8 *shape = getShapePtr(_itemInHand+248);
		for (int curY = y, height = 20; height > 0; height -= 2, curY += 2) {
			restoreGfxRect32x32(x, y);
			_screen->setNewShapeHeight(shape, height);
			const uint32 waitTime = _system->getMillis() + _tickLength;
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

int KyraEngine_MR::o3_setMalcolmsMood(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_setMalcolmsMood(%p) (%d)", (const void *)script, stackPos(0));
	return (_malcolmsMood = stackPos(0));
}

int KyraEngine_MR::o3_updateScore(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_updateScore(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	return updateScore(stackPos(0), stackPos(1)) ? 1 : 0;
}

int KyraEngine_MR::o3_makeSecondChanceSave(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_makeSecondChanceSave(%p) ()", (const void *)script);
	saveGameStateIntern(999, "Autosave", 0);
	return 0;
}

int KyraEngine_MR::o3_setSceneFilename(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_setSceneFilename(%p) (%d, '%s')", (const void *)script, stackPos(0), stackPosString(1));
	strcpy(_sceneList[stackPos(0)].filename1, stackPosString(1));
	_sceneList[stackPos(0)].filename1[9] = 0;
	return 0;
}

int KyraEngine_MR::o3_removeItemsFromScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_removeItemsFromScene(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	const uint16 itemId = stackPos(0);
	const uint16 sceneId = stackPos(1);
	const bool allItems = (stackPos(2) != 0);

	int retValue = 0;

	for (int i = 0; i < 50; ++i) {
		if (_itemList[i].sceneId == sceneId && _itemList[i].id == itemId) {
			resetItem(i);
			retValue = 1;
			if (!allItems)
				return 1;
		}
	}

	return retValue;
}

int KyraEngine_MR::o3_disguiseMalcolm(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o3_disguiseMalcolm(%p) (%d)", (const void *)script, stackPos(0));
	loadCharacterShapes(stackPos(0));
	updateDlgIndex();
	return 0;
}

int KyraEngine_MR::o3_drawSceneShape(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o3_drawSceneShape(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));

	int shape = stackPos(0);
	int flag = (stackPos(1) != 0) ? 1 : 0;

	restorePage3();

	const int x = _sceneShapeDescs[shape].drawX;
	const int y = _sceneShapeDescs[shape].drawY;

	_screen->drawShape(2, _sceneShapes[shape], x, y, 2, flag);

	_screen->copyRegionToBuffer(3, 0, 0, 320, 200, _gamePlayBuffer);

	_screen->drawShape(0, _sceneShapes[shape], x, y, 2, flag);

	flagAnimObjsForRefresh();
	refreshAnimObjectsIfNeed();
	return 0;
}

int KyraEngine_MR::o3_drawSceneShapeOnPage(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_drawSceneShapeOnPage(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	const int shape = stackPos(0);

	int x = _sceneShapeDescs[shape].drawX;
	int y = _sceneShapeDescs[shape].drawY;
	_screen->drawShape(stackPos(2), _sceneShapes[shape], x, y, 2, (stackPos(1) != 0) ? 1 : 0);
	return 0;
}

int KyraEngine_MR::o3_checkInRect(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_checkInRect(%p) (%d, %d, %d, %d, %d, %d)", (const void *)script,
			stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	const int x1 = stackPos(0);
	const int y1 = stackPos(1);
	const int x2 = stackPos(2);
	const int y2 = stackPos(3);
	int x = stackPos(4), y = stackPos(5);
	if (_itemInHand >= 0) {
		const int8 *desc = &_itemBuffer2[_itemInHand*2];
		x -= 12;
		x += desc[0];
		y -= 19;
		y += desc[1];
	}

	if (x >= x1 && x <= x2 && y >= y1 && y <= y2)
		return 1;
	else
		return 0;
}

int KyraEngine_MR::o3_updateConversations(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_updateConversations(%p) (%d)", (const void *)script, stackPos(0));
	int dlgIndex = stackPos(0);
	switch (_currentChapter-2) {
	case 0:
		dlgIndex -= 34;
		break;

	case 1:
		dlgIndex -= 54;
		break;

	case 2:
		dlgIndex -= 55;
		break;

	case 3:
		dlgIndex -= 70;
		break;

	default:
		break;
	}

	int convs[4];
	Common::fill(convs, convs+4, -1);

	if (_currentChapter == 1) {
		switch (_mainCharacter.dlgIndex) {
		case 0:
			convs[0] = 6;
			convs[1] = 12;
			break;

		case 2:
			convs[0] = 8;
			convs[1] = 14;
			break;

		case 3:
			convs[0] = 9;
			convs[1] = 15;
			break;

		case 4:
			convs[0] = 10;
			convs[1] = 16;
			break;

		case 5:
			convs[0] = 11;
			convs[1] = 17;
			break;

		case 6:
			convs[0] = 0;
			convs[1] = 12;
			break;

		case 8:
			convs[0] = 2;
			convs[1] = 14;
			break;

		case 9:
			convs[0] = 3;
			convs[1] = 15;
			break;

		case 10:
			convs[0] = 4;
			convs[1] = 16;
			break;

		case 11:
			convs[0] = 5;
			convs[1] = 17;
			break;

		case 12:
			convs[0] = 0;
			convs[1] = 6;
			break;

		case 14:
			convs[0] = 2;
			convs[1] = 8;
			break;

		case 15:
			convs[0] = 3;
			convs[1] = 9;
			break;

		case 16:
			convs[0] = 4;
			convs[1] = 10;
			break;

		case 17:
			convs[0] = 5;
			convs[1] = 11;
			break;

		default:
			break;
		}
	} else if (_currentChapter == 2) {
		switch (_mainCharacter.dlgIndex) {
		case 0:
			convs[0] = 4;
			convs[1] = 8;
			convs[2] = 5;
			convs[3] = 9;
			break;

		case 1:
			convs[0] = 4;
			convs[1] = 8;
			convs[2] = 0;
			convs[3] = 5;
			break;

		case 2:
			convs[0] = 6;
			convs[2] = 11;
			break;

		case 3:
			convs[0] = 7;
			convs[2] = 12;
			break;

		case 4:
			convs[0] = 0;
			convs[1] = 8;
			convs[2] = 1;
			convs[3] = 9;
			break;

		case 5:
			convs[0] = 0;
			convs[1] = 8;
			convs[2] = 4;
			convs[3] = 1;
			break;

		case 6:
			convs[0] = 2;
			convs[1] = 10;
			break;

		case 7:
			convs[0] = 3;
			convs[1] = 11;
			break;

		case 8:
			convs[0] = 0;
			convs[1] = 4;
			convs[2] = 1;
			break;

		case 9:
			convs[0] = 0;
			convs[1] = 4;
			convs[2] = 0;
			convs[3] = 1;
			break;

		case 10:
			convs[0] = 2;
			convs[1] = 6;
			break;

		case 11:
			convs[0] = 3;
			convs[1] = 7;
			break;

		default:
			break;
		}
	} else if (_currentChapter == 4) {
		if (_malcolmsMood == 0) {
			convs[0] = _mainCharacter.dlgIndex - 10;
			convs[1] = _mainCharacter.dlgIndex - 5;
		} else if (_malcolmsMood == 1) {
			convs[0] = _mainCharacter.dlgIndex + 5;
			convs[1] = _mainCharacter.dlgIndex + 10;
		} else if (_malcolmsMood == 2) {
			convs[0] = _mainCharacter.dlgIndex - 5;
			convs[1] = _mainCharacter.dlgIndex + 5;
		}
	}

	for (int i = 0; i < 4; ++i) {
		if (convs[i] != -1)
			_conversationState[dlgIndex][convs[i]] = 0;
	}

	return 1;
}

int KyraEngine_MR::o3_removeItemSlot(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_removeItemSlot(%p) (%d)", (const void *)script, stackPos(0));
	deleteItemAnimEntry(stackPos(0));
	_itemList[stackPos(0)].id = kItemNone;
	return 1;
}

int KyraEngine_MR::o3_setSceneDim(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_setSceneDim(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_sceneMinX = stackPos(0);
	_sceneMaxX = stackPos(1);
	return 0;
}

int KyraEngine_MR::o3_setSceneAnimPosAndFrame(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_setSceneAnimPosAndFrame(%p) (%d, %d, %d, %d, %d, %d)", (const void *)script,
			stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	SceneAnim &anim = _sceneAnims[stackPos(0)];
	const int newX2 = stackPos(1);
	const int newY2 = stackPos(2);
	const int newX = stackPos(3);
	const int newY = stackPos(4);

	if (newX2 >= 0)
		anim.x2 = newX2;
	if (newY2 >= 0)
		anim.y2 = newY2;

	if (newX >= 0)
		anim.x = newX;
	else
		anim.x = anim.x2 + (anim.width >> 1);

	if (newY >= 0)
		anim.y = newY;
	else
		anim.y = anim.y2 + anim.height - 1;

	updateSceneAnim(stackPos(0), stackPos(5));
	_specialSceneScriptRunFlag = false;
	return 0;
}

int KyraEngine_MR::o3_removeItemInstances(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_removeItemInstances(%p) (%d)", (const void *)script, stackPos(0));
	const int16 item = stackPos(0);

	int deleted = 0;

	for (int i = 0; i < 10; ++i) {
		if (_mainCharacter.inventory[i] == item) {
			_mainCharacter.inventory[i] = kItemNone;
			++deleted;
		}
	}

	if (_itemInHand == item) {
		removeHandItem();
		++deleted;
	}

	for (int i = 0; i < 50; ++i) {
		if (_itemList[i].id == item) {
			_itemList[i].id = kItemNone;
			++deleted;
		}
	}

	return deleted;
}

int KyraEngine_MR::o3_disableInventory(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_disableInventory(%p) ()", (const void *)script);
	_enableInventory = false;
	return 0;
}

int KyraEngine_MR::o3_enableInventory(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_enableInventory(%p) ()", (const void *)script);
	_enableInventory = true;
	return 1;
}

int KyraEngine_MR::o3_enterNewScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_enterNewScene(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0),
		stackPos(1), stackPos(2), stackPos(3), stackPos(4));

	_screen->hideMouse();
	enterNewScene(stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));

	_unk5 = 1;

	if (_mainCharX == -1 || _mainCharY == -1) {
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
		updateCharacterAnim(0);
	}
	_screen->showMouse();

	return 0;
}

int KyraEngine_MR::o3_switchScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_switchScene(%p) (%d)", (const void *)script, stackPos(0));
	setGameFlag(1);
	_mainCharX = _mainCharacter.x1;
	_mainCharY = _mainCharacter.y1;
	_noScriptEnter = false;
	enterNewScene(stackPos(0), _mainCharacter.facing, 0, 0, 0);
	_noScriptEnter = true;
	return 0;
}

int KyraEngine_MR::o3_setMalcolmPos(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_setMalcolmPos(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_mainCharX = stackPos(0);
	_mainCharY = stackPos(1);

	if (_mainCharX == -1 && _mainCharY == -1)
		_mainCharacter.animFrame = 87;
	else
		_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];

	return 0;
}

int KyraEngine_MR::o3_stopMusic(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_stopMusic(%p) ()", (const void *)script);
	stopMusicTrack();
	return 0;
}

int KyraEngine_MR::o3_playSoundEffect(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_playSoundEffect(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	snd_playSoundEffect(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_MR::o3_getScore(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_getScore(%p) ()", (const void *)script);
	return _score;
}

int KyraEngine_MR::o3_daggerWarning(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_daggerWarning(%p) ()", (const void *)script);
	int selection = 1;

	_screen->hideMouse();
	_screen->copyRegionToBuffer(1, 0, 0, 320, 200, _screenBuffer);
	int curPageBackUp = _screen->_curPage;
	_screen->_curPage = 2;

	_screen->drawFilledBox(0, 0, 0x13F, 0xC7, 0xB4, 0xB3, 0xB6);
	_screen->drawFilledBox(0xF, 0xAA, 0x68, 0xBA, 0xB4, 0xB3, 0xB6);
	_screen->drawFilledBox(0x73, 0xAA, 0xCC, 0xBA, 0xB4, 0xB3, 0xB6);
	_screen->drawFilledBox(0xD6, 0xAA, 0x12F, 0xBA, 0xB4, 0xB3, 0xB6);

	int y = 15;
	for (int i = 100; i <= 107; ++i) {
		const char *str = (const char *)getTableEntry(_cCodeFile, i);
		int x = _text->getCenterStringX(str, 0, 0x13F);
		_text->printText(str, x, y, 0xFF, 0xF0, 0x00);
		y += 10;
	}
	y += 15;
	for (int i = 110; i <= 113; ++i) {
		const char *str = (const char *)getTableEntry(_cCodeFile, i);
		int x = _text->getCenterStringX(str, 0, 0x13F);
		_text->printText(str, x, y, 0xFF, 0xF0, 0x00);
		y += 10;
	}

	const char *str = 0;
	int x = 0;

	str = (const char *)getTableEntry(_cCodeFile, 120);
	x = _text->getCenterStringX(str, 0xF, 0x68);
	_text->printText(str, x, 174, 0xFF, 0xF0, 0x00);

	str = (const char *)getTableEntry(_cCodeFile, 121);
	x = _text->getCenterStringX(str, 0x73, 0xCC);
	_text->printText(str, x, 174, 0xFF, 0xF0, 0x00);

	str = (const char *)getTableEntry(_cCodeFile, 122);
	x = _text->getCenterStringX(str, 0xD6, 0x12F);
	_text->printText(str, x, 174, 0xFF, 0xF0, 0x00);

	_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0);
	_screen->updateScreen();

	_screen->_curPage = curPageBackUp;
	_screen->showMouse();

	while (!shouldQuit()) {
		int keys = checkInput(0);
		removeInputTop();

		if (keys == 198 || keys == 199) {
			if (_mouseX >= 15 && _mouseX <= 104 && _mouseY >= 170 && _mouseY <= 186) {
				selection = 1;
				break;
			} else if (_mouseX >= 115 && _mouseX <= 204 && _mouseY >= 170 && _mouseY <= 186) {
				selection = 2;
				break;
			} else if (_mouseX >= 214 && _mouseX <= 303 && _mouseY >= 170 && _mouseY <= 186) {
				selection = 3;
				break;
			}
		}

		delay(10);
	}

	restorePage3();
	_screen->copyBlockToPage(1, 0, 0, 320, 200, _screenBuffer);
	return selection;
}

int KyraEngine_MR::o3_blockOutWalkableRegion(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_blockOutWalkableRegion(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	const int x1 = stackPos(0);
	int y1 = stackPos(1);
	const int x2 = stackPos(2);
	int y2 = stackPos(3);

	if (y1 < _maskPageMinY)
		y1 = _maskPageMinY;
	if (y2 > _maskPageMaxY)
		y2 = _maskPageMaxY;

	_screen->blockOutRegion(x1, y1, x2-x1+1, y2-y1+1);
	return 0;
}

int KyraEngine_MR::o3_showSceneStringsMessage(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_showSceneStringsMessage(%p) (%d)", (const void *)script, stackPos(0));
	showMessage((const char *)getTableEntry(_sceneStrings, stackPos(0)), 0xFF, 0xF0);
	return 0;
}

int KyraEngine_MR::o3_showGoodConscience(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_showGoodConscience(%p) ()", (const void *)script);
	showGoodConscience();
	return 0;
}

int KyraEngine_MR::o3_goodConscienceChat(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_goodConscienceChat(%p) (%d)", (const void *)script, stackPos(0));
	int id = stackPos(0);
	const char *str = (const char *)getTableEntry(_useActorBuffer ? _actorFile : _sceneStrings, id);
	goodConscienceChat(str, _vocHigh, id);
	return 0;
}

int KyraEngine_MR::o3_hideGoodConscience(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_hideGoodConscience(%p) ()", (const void *)script);
	hideGoodConscience();
	return 0;
}

int KyraEngine_MR::o3_defineSceneAnim(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_defineSceneAnim(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, '%s')",
		(const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7),
		stackPos(8), stackPos(9), stackPos(10), stackPos(11), stackPosString(12));
	const int animId = stackPos(0);
	SceneAnim &anim = _sceneAnims[animId];

	uint16 flags = anim.flags = stackPos(1);
	int x = anim.x = stackPos(2);
	int y = anim.y = stackPos(3);
	int x2 = anim.x2 = stackPos(4);
	int y2 = anim.y2 = stackPos(5);
	int w = anim.width = stackPos(6);
	int h = anim.height = stackPos(7);
	anim.specialSize = stackPos(9);
	anim.shapeIndex = stackPos(11);
	const char *filename = stackPosString(12);

	if (filename)
		strcpy(anim.filename, filename);

	if (flags & 8) {
		_sceneAnimMovie[animId]->open(filename, 1, 0);
		if (_sceneAnimMovie[animId]->opened()) {
			anim.wsaFlag = 1;
			if (x2 == -1)
				x2 = _sceneAnimMovie[animId]->xAdd();
			if (y2 == -1)
				y2 = _sceneAnimMovie[animId]->yAdd();
			if (w == -1)
				w = _sceneAnimMovie[animId]->width();
			if (h == -1)
				h = _sceneAnimMovie[animId]->height();
			if (x == -1)
				x = (w >> 1) + x2;
			if (y == -1)
				y = y2 + h - 1;

			anim.x = x;
			anim.y = y;
			anim.x2 = x2;
			anim.y2 = y2;
			anim.width = w;
			anim.height = h;
		}
	}

	return 9;
}

int KyraEngine_MR::o3_updateSceneAnim(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_updateSceneAnim(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	updateSceneAnim(stackPos(0), stackPos(1));
	_specialSceneScriptRunFlag = false;
	return 0;
}

int KyraEngine_MR::o3_runActorScript(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_runActorScript(%p) ()", (const void *)script);
	EMCData data;
	EMCState state;
	memset(&data, 0, sizeof(data));
	memset(&state, 0, sizeof(state));

	_res->exists("_ACTOR.EMC", true);
	_emc->load("_ACTOR.EMC", &data, &_opcodes);
	_emc->init(&state, &data);
	_emc->start(&state, 0);

	state.regs[4] = _itemInHand;
	state.regs[0] = _mainCharacter.sceneId;

	int vocHigh = _vocHigh;
	_vocHigh = 200;
	_useActorBuffer = true;

	while (_emc->isValid(&state))
		_emc->run(&state);

	_useActorBuffer = false;
	_vocHigh = vocHigh;
	_emc->unload(&data);

	if (queryGameFlag(0x218)) {
		resetGameFlag(0x218);
		enterNewScene(78, -1, 0, 0, 0);
	}

	return 0;
}

int KyraEngine_MR::o3_doDialog(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_doDialog(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	doDialog(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_MR::o3_setConversationState(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_setConversationState(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	int id = stackPos(0);
	const int dlgIndex = stackPos(1);
	const int value = stackPos(2);

	switch (_currentChapter-2) {
	case 0:
		id -= 34;
		break;

	case 1:
		id -= 54;
		break;

	case 2:
		id -= 55;
		break;

	case 3:
		id -= 70;
		break;

	default:
		break;
	}

	return (_conversationState[id][dlgIndex] = value);
}

int KyraEngine_MR::o3_getConversationState(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_getConversationState(%p) (%d)", (const void *)script, stackPos(0));
	int id = stackPos(0);
	const int dlgIndex = _mainCharacter.dlgIndex;

	switch (_currentChapter-2) {
	case 0:
		id -= 34;
		break;

	case 1:
		id -= 54;
		break;

	case 2:
		id -= 55;
		break;

	case 3:
		id -= 70;
		break;

	default:
		break;
	}

	return _conversationState[id][dlgIndex];
}

int KyraEngine_MR::o3_changeChapter(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_changeChapter(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	changeChapter(stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	return 0;
}

int KyraEngine_MR::o3_countItemInstances(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_countItemInstances(%p) (%d)", (const void *)script, stackPos(0));
	int count = 0;
	const int16 item = stackPos(0);

	for (int i = 0; i < 10; ++i) {
		if (_mainCharacter.inventory[i] == item)
			++count;
	}

	if (_itemInHand == item)
		++count;

	for (int i = 0; i < 50; ++i) {
		if (_itemList[i].id == item)
			++count;
	}

	return count;
}

int KyraEngine_MR::o3_dialogStartScript(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_dialogStartScript(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	dialogStartScript(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_MR::o3_dialogEndScript(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_dialogEndScript(%p) (%d)", (const void *)script, stackPos(0));
	dialogEndScript(stackPos(0));
	return 0;
}

int KyraEngine_MR::o3_customChat(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_customChat(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	const int id = stackPos(0);
	const int object = stackPos(1);
	const char *str = (const char *)getTableEntry(_sceneStrings, id);

	if (!str)
		return 0;

	strcpy(_stringBuffer, str);
	_chatText = _stringBuffer;
	_chatObject = object;
	_chatVocHigh = _chatVocLow = -1;
	objectChatInit(_stringBuffer, object, _vocHigh, id);
	playVoice(_vocHigh, id);
	return 0;
}

int KyraEngine_MR::o3_customChatFinish(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_customChatFinish(%p) ()", (const void *)script);
	_text->restoreScreen();
	_chatText = 0;
	_chatObject = -1;
	return 0;
}

int KyraEngine_MR::o3_setupSceneAnimObject(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_setupSceneAnimObject(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %s)", (const void *)script,
			stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8), stackPos(9),
			stackPos(10), stackPos(11), stackPosString(12));
	setupSceneAnimObject(stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8),
						stackPos(9), stackPos(10), stackPos(11), stackPosString(12));
	return 0;
}

int KyraEngine_MR::o3_removeSceneAnimObject(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_removeSceneAnimObject(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	removeSceneAnimObject(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_MR::o3_dummy(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3_dummy(%p) ()", (const void *)script);
	return 0;
}

#pragma mark -

int KyraEngine_MR::o3a_setCharacterFrame(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3a_setCharacterFrame(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	static const uint8 frameTable[] = {
		0x58, 0xD8, 0xD8, 0x98, 0x78, 0x78, 0xB8, 0xB8
	};

	_animNewFrame = stackPos(0);
	if (_useFrameTable)
		_animNewFrame += frameTable[_mainCharacter.facing];

	_animDelayTime = stackPos(1);
	_animNeedUpdate = true;
	return 0;
}

int KyraEngine_MR::o3a_playSoundEffect(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3a_playSoundEffect(%p) (%d)", (const void *)script, stackPos(0));
	snd_playSoundEffect(stackPos(0), 200);
	return 0;
}

#pragma mark -

int KyraEngine_MR::o3d_updateAnim(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3d_updateAnim(%p) (%d)", (const void *)script, stackPos(0));
	if (_dialogSceneAnim >= 0)
		updateSceneAnim(_dialogSceneAnim, stackPos(0));
	return 0;
}

int KyraEngine_MR::o3d_delay(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3d_delay(%p) (%d)", (const void *)script, stackPos(0));
	delayUntil(_system->getMillis() + stackPos(0) * _tickLength, false, true);
	return 0;
}

#pragma mark -

typedef Common::Functor1Mem<EMCState *, int, KyraEngine_MR> OpcodeV3;
#define SetOpcodeTable(x) table = &x;
#define Opcode(x) table->push_back(new OpcodeV3(this, &KyraEngine_MR::x))
#define OpcodeUnImpl() table->push_back(new OpcodeV3(this, 0))
void KyraEngine_MR::setupOpcodeTable() {
	Common::Array<const Opcode *> *table = 0;

	_opcodes.reserve(176);
	SetOpcodeTable(_opcodes);
	// 0x00
	Opcode(o3_getMalcolmShapes);
	Opcode(o3_setCharacterPos);
	Opcode(o3_defineObject);
	Opcode(o3_refreshCharacter);
	// 0x04
	Opcode(o2_getCharacterX);
	Opcode(o2_getCharacterY);
	Opcode(o2_getCharacterFacing);
	Opcode(o2_getCharacterScene);
	// 0x08
	Opcode(o3_getMalcolmsMood);
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	Opcode(o3_getCharacterFrameFromFacing);
	// 0x0c
	Opcode(o2_setCharacterFacingOverwrite);
	Opcode(o2_trySceneChange);
	Opcode(o2_moveCharacter);
	Opcode(o3_setCharacterFacing);
	// 0x10
	OpcodeUnImpl();
	Opcode(o3_showSceneFileMessage);
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	// 0x14
	Opcode(o3_setCharacterAnimFrameFromFacing);
	Opcode(o3_showBadConscience);
	Opcode(o3_dummy);
	Opcode(o3_hideBadConscience);
	// 0x18
	OpcodeUnImpl();
	Opcode(o3_showAlbum);
	Opcode(o3_setInventorySlot);
	Opcode(o3_getInventorySlot);
	// 0x1c
	Opcode(o3_addItemToInventory);
	OpcodeUnImpl();
	Opcode(o3_addItemToCurScene);
	Opcode(o3_objectChat);
	// 0x20
	Opcode(o2_checkForItem);
	Opcode(o3_dummy);
	Opcode(o3_resetInventory);
	Opcode(o2_defineItem);
	// 0x24
	Opcode(o3_removeInventoryItemInstances);
	Opcode(o3_countInventoryItemInstances);
	Opcode(o3_npcChatSequence);
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
	Opcode(o3_badConscienceChat);
	Opcode(o3_wipeDownMouseItem);
	Opcode(o3_dummy);
	// 0x34
	Opcode(o3_setMalcolmsMood);
	Opcode(o3_playSoundEffect);
	Opcode(o3_dummy);
	Opcode(o2_delay);
	// 0x38
	Opcode(o3_updateScore);
	Opcode(o3_makeSecondChanceSave);
	Opcode(o3_setSceneFilename);
	OpcodeUnImpl();
	// 0x3c
	Opcode(o3_removeItemsFromScene);
	Opcode(o3_disguiseMalcolm);
	Opcode(o3_drawSceneShape);
	Opcode(o3_drawSceneShapeOnPage);
	// 0x40
	Opcode(o3_checkInRect);
	Opcode(o3_updateConversations);
	Opcode(o3_removeItemSlot);
	Opcode(o3_dummy);
	// 0x44
	Opcode(o3_dummy);
	Opcode(o3_setSceneDim);
	OpcodeUnImpl();
	Opcode(o3_dummy);
	// 0x48
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	Opcode(o3_setSceneAnimPosAndFrame);
	Opcode(o2_update);
	// 0x4c
	Opcode(o3_removeItemInstances);
	Opcode(o3_dummy);
	Opcode(o3_disableInventory);
	Opcode(o3_enableInventory);
	// 0x50
	Opcode(o3_enterNewScene);
	Opcode(o3_switchScene);
	Opcode(o2_getShapeFlag1);
	Opcode(o3_dummy);
	// 0x54
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	Opcode(o3_setMalcolmPos);
	Opcode(o3_stopMusic);
	// 0x58
	Opcode(o1_playWanderScoreViaMap);
	Opcode(o3_playSoundEffect);
	Opcode(o3_getScore);
	Opcode(o3_daggerWarning);
	// 0x5c
	Opcode(o3_blockOutWalkableRegion);
	Opcode(o3_dummy);
	Opcode(o3_showSceneStringsMessage);
	OpcodeUnImpl();
	// 0x60
	Opcode(o1_getRand);
	Opcode(o3_dummy);
	Opcode(o1_setDeathHandler);
	Opcode(o3_showGoodConscience);
	// 0x64
	Opcode(o3_goodConscienceChat);
	Opcode(o3_hideGoodConscience);
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	// 0x68
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	Opcode(o2_waitForConfirmationClick);
	// 0x6c
	Opcode(o3_dummy);
	Opcode(o2_defineRoomEntrance);
	Opcode(o2_runAnimationScript);
	Opcode(o2_setSpecialSceneScriptRunTime);
	// 0x70
	Opcode(o3_defineSceneAnim);
	Opcode(o3_dummy);
	Opcode(o3_updateSceneAnim);
	Opcode(o3_dummy);
	// 0x74
	Opcode(o3_runActorScript);
	Opcode(o3_doDialog);
	Opcode(o2_randomSceneChat);
	Opcode(o2_setDlgIndex);
	// 0x78
	Opcode(o2_getDlgIndex);
	Opcode(o2_defineScene);
	Opcode(o3_setConversationState);
	OpcodeUnImpl();
	// 0x7c
	OpcodeUnImpl();
	Opcode(o3_getConversationState);
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	// 0x80
	Opcode(o3_dummy);
	Opcode(o3_changeChapter);
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	// 0x84
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	// 0x88
	Opcode(o3_countItemInstances);
	Opcode(o3_dummy);
	Opcode(o3_dialogStartScript);
	Opcode(o3_dummy);
	// 0x8c
	Opcode(o3_dialogEndScript);
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	Opcode(o2_setSpecialSceneScriptState);
	// 0x90
	Opcode(o2_clearSpecialSceneScriptState);
	Opcode(o2_querySpecialSceneScriptState);
	Opcode(o3_dummy);
	Opcode(o2_setHiddenItemsEntry);
	// 0x94
	Opcode(o2_getHiddenItemsEntry);
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	OpcodeUnImpl();
	// 0x98
	Opcode(o3_customChat);
	Opcode(o3_customChatFinish);
	Opcode(o3_setupSceneAnimObject);
	Opcode(o3_removeSceneAnimObject);
	// 0x9c
	Opcode(o2_disableTimer);
	Opcode(o2_enableTimer);
	Opcode(o2_setTimerCountdown);
	OpcodeUnImpl();
	// 0xa0
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	Opcode(o3_dummy);
	// 0xa4
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(o2_setVocHigh);
	// 0xa8
	Opcode(o2_getVocHigh);
	OpcodeUnImpl();
	OpcodeUnImpl();
	OpcodeUnImpl();
	// 0xac
	OpcodeUnImpl();
	Opcode(o3_dummy);
	OpcodeUnImpl();
	Opcode(o3_dummy);

	_opcodesAnimation.reserve(8);
	SetOpcodeTable(_opcodesAnimation);
	// 0x00
	Opcode(o2a_setAnimationShapes);
	Opcode(o3a_setCharacterFrame);
	Opcode(o3a_playSoundEffect);
	Opcode(o3_dummy);
	// 0x04
	Opcode(o2a_setResetFrame);
	Opcode(o1_getRand);
	Opcode(o3_getMalcolmShapes);
	Opcode(o3_dummy);

	_opcodesDialog.reserve(5);
	SetOpcodeTable(_opcodesDialog);
	// 0x00
	Opcode(o3d_updateAnim);
	Opcode(o3d_delay);
	Opcode(o1_getRand);
	Opcode(o1_queryGameFlag);
	// 0x04
	Opcode(o3_dummy);
}

} // End of namespace Kyra
