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
#include "kyra/timer.h"

#include "common/system.h"

namespace Kyra {

void KyraEngine_MR::removeTrashItems() {
	for (int i = 0; _trashItemList[i] != kItemNone; ++i) {
		for (int item = findItem(_trashItemList[i]); item != -1; item = findItem(_trashItemList[i])) {
			if (_itemList[item].sceneId != _mainCharacter.sceneId)
				resetItem(item);
			else
				break;
		}
	}
}

int KyraEngine_MR::findFreeInventorySlot() {
	for (int i = 0; i < 10; ++i) {
		if (_mainCharacter.inventory[i] == kItemNone)
			return i;
	}
	return -1;
}

int KyraEngine_MR::checkItemCollision(int x, int y) {
	int itemIndex = -1;
	int maxItemY = -1;

	for (int i = 0; i < 50; ++i) {
		if (_itemList[i].id == kItemNone || _itemList[i].sceneId != _mainCharacter.sceneId)
			continue;

		const int x1 = _itemList[i].x - 11;
		const int x2 = _itemList[i].x + 10;

		if (x < x1 || x > x2)
			continue;

		const int y1 = _itemList[i].y - _itemBuffer1[_itemList[i].id] - 3;
		const int y2 = _itemList[i].y + 3;

		if (y < y1 || y > y2)
			continue;

		if (_itemList[i].y >= maxItemY) {
			itemIndex = i;
			maxItemY = _itemList[i].y;
		}
	}

	return itemIndex;
}

void KyraEngine_MR::setMouseCursor(Item item) {
	int shape = 0;
	int hotX = 1;
	int hotY = 1;

	if (item != kItemNone) {
		hotX = 12;
		hotY = 19;
		shape = item+248;
	}

	_mouseState = item;
	if ((int16)item >= 0)
		_screen->setMouseCursor(hotX, hotY, getShapePtr(shape));
}

void KyraEngine_MR::setItemMouseCursor() {
	_mouseState = _itemInHand;
	if (_itemInHand == kItemNone)
		_screen->setMouseCursor(0, 0, _gameShapes[0]);
	else
		_screen->setMouseCursor(12, 19, _gameShapes[_itemInHand+248]);
}

bool KyraEngine_MR::dropItem(int unk1, Item item, int x, int y, int unk2) {
	if (_mouseState <= -1)
		return false;

	if (processItemDrop(_mainCharacter.sceneId, item, x, y, unk1, unk2))
		return true;

	snd_playSoundEffect(13, 200);

	if (countAllItems() >= 50) {
		removeTrashItems();
		if (processItemDrop(_mainCharacter.sceneId, item, x, y, unk1, unk2))
			return true;

		if (countAllItems() >= 50)
			showMessageFromCCode(14, 0xB3, 0);
	}

	if (!_chatText)
		snd_playSoundEffect(13, 200);
	return false;
}

bool KyraEngine_MR::processItemDrop(uint16 sceneId, Item item, int x, int y, int unk1, int unk2) {
	int itemPos = checkItemCollision(x, y);

	if (unk1)
		itemPos = -1;

	if (itemPos >= 0) {
		exchangeMouseItem(itemPos, 1);
		return true;
	}

	int freeItemSlot = -1;

	if (unk2 != 3) {
		for (int i = 0; i < 50; ++i) {
			if (_itemList[i].id == kItemNone) {
				freeItemSlot = i;
				break;
			}
		}
	}

	if (freeItemSlot < 0)
		return false;

	if (_mainCharacter.sceneId != sceneId) {
		_itemList[freeItemSlot].x = x;
		_itemList[freeItemSlot].y = y;
		_itemList[freeItemSlot].id = item;
		_itemList[freeItemSlot].sceneId = sceneId;
		return true;
	}

	int itemHeight = _itemBuffer1[item];

	// no idea why it's '&&' here and not single checks for x and y
	if (x == -1 && y == -1) {
		x = _rnd.getRandomNumberRng(0x18, 0x128);
		y = _rnd.getRandomNumberRng(0x14, 0x87);
	}

	int posX = x, posY = y;
	int itemX = -1, itemY = -1;
	bool needRepositioning = true;

	while (needRepositioning) {
		if ((_screen->getDrawLayer(posX, posY) <= 1 && _screen->getDrawLayer2(posX, posY, itemHeight) <= 1 && isDropable(posX, posY)) || posY == 187) {
			int posX2 = posX, posX3 = posX;
			bool repositioning = true;

			while (repositioning) {
				if (isDropable(posX3, posY) && _screen->getDrawLayer2(posX3, posY, itemHeight) < 7 && checkItemCollision(posX3, posY) == -1) {
					itemX = posX3;
					itemY = posY;
					needRepositioning = false;
					repositioning = false;
				}

				if (isDropable(posX2, posY) && _screen->getDrawLayer2(posX2, posY, itemHeight) < 7 && checkItemCollision(posX2, posY) == -1) {
					itemX = posX2;
					itemY = posY;
					needRepositioning = false;
					repositioning = false;
				}

				if (repositioning) {
					posX3 = MAX(posX3 - 2, 24);
					posX2 = MIN(posX2 + 2, 296);

					if (posX3 <= 24 && posX2 >= 296)
						repositioning = false;
				}
			}
		}

		if (posY == 187)
			needRepositioning = false;
		else
			posY = MIN(posY + 2, 187);
	}

	if (itemX == -1 || itemY == -1)
		return false;

	if (unk1 == 3) {
		_itemList[freeItemSlot].x = itemX;
		_itemList[freeItemSlot].y = itemY;
		return true;
	} else if (unk1 == 2) {
		itemDropDown(x, y, itemX, itemY, freeItemSlot, item, 0);
	}

	itemDropDown(x, y, itemX, itemY, freeItemSlot, item, (unk1 == 0) ? 1 : 0);

	if (!unk1 && unk2) {
		int itemStr = 1;
		if (_lang == 1)
			itemStr = getItemCommandStringDrop(item);
		updateItemCommand(item, itemStr, 0xFF);
	}

	return true;
}

void KyraEngine_MR::itemDropDown(int startX, int startY, int dstX, int dstY, int itemSlot, Item item, int remove) {
	if (startX == dstX && startY == dstY) {
		_itemList[itemSlot].x = dstX;
		_itemList[itemSlot].y = dstY;
		_itemList[itemSlot].id = item;
		_itemList[itemSlot].sceneId = _mainCharacter.sceneId;
		snd_playSoundEffect(0x0C, 0xC8);
		addItemToAnimList(itemSlot);
	} else {
		uint8 *itemShape = getShapePtr(item + 248);
		_screen->hideMouse();

		if (startY <= dstY) {
			int speed = 2;
			int curY = startY;
			int curX = startX - 12;

			backUpGfxRect32x32(curX, curY-16);
			while (curY < dstY) {
				restoreGfxRect32x32(curX, curY-16);

				curY = MIN(curY + speed, dstY);
				++speed;

				backUpGfxRect32x32(curX, curY-16);
				uint32 endDelay = _system->getMillis() + _tickLength;

				_screen->drawShape(0, itemShape, curX, curY-16, 0, 0);
				_screen->updateScreen();

				delayUntil(endDelay);
			}
			restoreGfxRect32x32(curX, curY-16);

			if (dstX != dstY || (dstY - startY > 16)) {
				snd_playSoundEffect(0x11, 0xC8);
				speed = MAX(speed, 6);
				int speedX = ((dstX - startX) << 4) / speed;
				int origSpeed = speed;
				speed >>= 1;

				if (dstY - startY <= 8)
					speed >>= 1;

				speed = -speed;

				curX = startX << 4;

				int x = 0, y = 0;
				while (--origSpeed) {
					curY = MIN(curY + speed, dstY);
					curX += speedX;
					++speed;

					x = (curX >> 4) - 8;
					y = curY - 16;
					backUpGfxRect32x32(x, y);

					uint16 endDelay = _system->getMillis() + _tickLength;
					_screen->drawShape(0, itemShape, x, y, 0, 0);
					_screen->updateScreen();

					restoreGfxRect32x32(x, y);

					delayUntil(endDelay);
				}

				restoreGfxRect32x32(x, y);
			}
		}

		_itemList[itemSlot].x = dstX;
		_itemList[itemSlot].y = dstY;
		_itemList[itemSlot].id = item;
		_itemList[itemSlot].sceneId = _mainCharacter.sceneId;
		snd_playSoundEffect(0x0C, 0xC8);
		addItemToAnimList(itemSlot);
		_screen->showMouse();
	}

	if (remove)
		removeHandItem();
}

void KyraEngine_MR::exchangeMouseItem(int itemPos, int runScript) {
	if (itemListMagic(_itemInHand, itemPos))
		return;

	if (_itemInHand == 43) {
		removeHandItem();
		return;
	}

	deleteItemAnimEntry(itemPos);

	Item itemId = _itemList[itemPos].id;
	_itemList[itemPos].id = _itemInHand;
	_itemInHand = itemId;

	addItemToAnimList(itemPos);
	snd_playSoundEffect(0x0B, 0xC8);
	setMouseCursor(_itemInHand);
	int str2 = 0;

	if (_lang == 1)
		str2 = getItemCommandStringPickUp(itemId);

	updateItemCommand(itemId, str2, 0xFF);

	if (runScript)
		runSceneScript6();
}

bool KyraEngine_MR::pickUpItem(int x, int y, int runScript) {
	int itemPos = checkItemCollision(x, y);

	if (itemPos <= -1)
		return false;

	if (_itemInHand >= 0) {
		exchangeMouseItem(itemPos, runScript);
	} else {
		deleteItemAnimEntry(itemPos);
		Item itemId = _itemList[itemPos].id;
		_itemList[itemPos].id = kItemNone;
		snd_playSoundEffect(0x0B, 0xC8);
		setMouseCursor(itemId);
		int itemString = 0;

		if (_lang == 1)
			itemString = getItemCommandStringPickUp(itemId);

		updateItemCommand(itemId, itemString, 0xFF);
		_itemInHand = itemId;

		if (runScript)
			runSceneScript6();
	}

	return true;
}

bool KyraEngine_MR::isDropable(int x, int y) {
	if (y < 14 || y > 187)
		return false;

	x -= 12;

	for (int xpos = x; xpos < x + 24; ++xpos) {
		if (_screen->getShapeFlag1(xpos, y) == 0)
			return false;
	}

	return true;
}

bool KyraEngine_MR::itemListMagic(Item handItem, int itemSlot) {
	Item item = _itemList[itemSlot].id;

	if (_currentChapter == 1 && handItem == 3 && item == 3 && queryGameFlag(0x76)) {
		eelScript();
		return true;
	} else if ((handItem == 6 || handItem == 7) && item == 2) {
		int animObjIndex = -1;
		for (int i = 17; i <= 66; ++i) {
			if (_animObjects[i].shapeIndex2 == 250)
				animObjIndex = i;
		}

		assert(animObjIndex != -1);

		snd_playSoundEffect(0x93, 0xC8);
		for (int i = 109; i <= 141; ++i) {
			_animObjects[animObjIndex].shapeIndex1 = i+248;
			_animObjects[animObjIndex].needRefresh = true;
			delay(1*_tickLength, true);
		}

		deleteItemAnimEntry(itemSlot);
		_itemList[itemSlot].id = kItemNone;
		return true;
	}

	if (_mainCharacter.sceneId == 51 && queryGameFlag(0x19B) && !queryGameFlag(0x19C)
		&& ((item == 63 && handItem == 56) || (item == 56 && handItem == 63))) {

		if (queryGameFlag(0x1AC)) {
			setGameFlag(0x19C);
			setGameFlag(0x1AD);
		} else {
			setGameFlag(0x1AE);
		}

		_timer->setCountdown(12, 1);
		_timer->enable(12);
	}

	for (int i = 0; _itemMagicTable[i] != 0xFF; i += 4) {
		if (_itemMagicTable[i+0] != handItem || (int8)_itemMagicTable[i+1] != item)
			continue;

		uint8 resItem = _itemMagicTable[i+2];
		uint8 newItem = _itemMagicTable[i+3];

		snd_playSoundEffect(0x0F, 0xC8);

		_itemList[itemSlot].id = (int8)resItem;

		deleteItemAnimEntry(itemSlot);
		addItemToAnimList(itemSlot);

		if (newItem == 0xFE)
			removeHandItem();
		else if (newItem != 0xFF)
			setHandItem(newItem);

		if (_lang != 1)
			updateItemCommand(resItem, 3, 0xFF);

		// Unlike the original we give points for when combining with scene items
		if (resItem == 7) {
			updateScore(35, 100);
			delay(60*_tickLength, true);
		}

		return true;
	}

	return false;
}

bool KyraEngine_MR::itemInventoryMagic(Item handItem, int invSlot) {
	Item item = _mainCharacter.inventory[invSlot];

	if (_currentChapter == 1 && handItem == 3 && item == 3 && queryGameFlag(0x76)) {
		eelScript();
		return true;
	} else if ((handItem == 6 || handItem == 7) && item == 2) {
		_screen->hideMouse();
		snd_playSoundEffect(0x93, 0xC8);
		for (int i = 109; i <= 141; ++i) {
			_mainCharacter.inventory[invSlot] = i;
			_screen->drawShape(2, getShapePtr(invSlot+422), 0, 144, 0, 0);
			_screen->drawShape(2, getShapePtr(i+248), 0, 144, 0, 0);
			_screen->copyRegion(0, 144, _inventoryX[invSlot], _inventoryY[invSlot], 24, 20, 2, 0, Screen::CR_NO_P_CHECK);
			_screen->updateScreen();
			delay(1*_tickLength, true);
		}

		_mainCharacter.inventory[invSlot] = kItemNone;
		clearInventorySlot(invSlot, 0);
		_screen->showMouse();
		return true;
	}

	for (int i = 0; _itemMagicTable[i] != 0xFF; i += 4) {
		if (_itemMagicTable[i+0] != handItem || _itemMagicTable[i+1] != item)
			continue;

		uint8 resItem = _itemMagicTable[i+2];
		uint8 newItem = _itemMagicTable[i+3];

		snd_playSoundEffect(0x0F, 0xC8);

		_mainCharacter.inventory[invSlot] = (int8)resItem;

		clearInventorySlot(invSlot, 0);
		drawInventorySlot(0, resItem, invSlot);

		if (newItem == 0xFE)
			removeHandItem();
		else if (newItem != 0xFF)
			setHandItem(newItem);

		if (_lang != 1)
			updateItemCommand(resItem, 3, 0xFF);

		// Unlike the original we give points for every language
		if (resItem == 7) {
			updateScore(35, 100);
			delay(60*_tickLength, true);
		}

		return true;
	}

	return false;
}

int KyraEngine_MR::getItemCommandStringDrop(uint16 item) {
	assert(item < _itemStringMapSize);
	int stringId = _itemStringMap[item];
	return _itemStringDrop[stringId];
}

int KyraEngine_MR::getItemCommandStringPickUp(uint16 item) {
	assert(item < _itemStringMapSize);
	int stringId = _itemStringMap[item];
	return _itemStringPickUp[stringId];
}

int KyraEngine_MR::getItemCommandStringInv(uint16 item) {
	assert(item < _itemStringMapSize);
	int stringId = _itemStringMap[item];
	return _itemStringInv[stringId];
}

} // End of namespace Kyra
