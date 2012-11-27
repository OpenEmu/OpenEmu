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

#include "common/system.h"

namespace Kyra {

int KyraEngine_HoF::checkItemCollision(int x, int y) {
	int itemPos = -1, yPos = -1;

	for (int i = 0; i < 30; ++i) {
		const ItemDefinition &curItem = _itemList[i];

		if (curItem.id == kItemNone || curItem.sceneId != _mainCharacter.sceneId)
			continue;

		int itemX1 = curItem.x - 8 - 3;
		int itemX2 = curItem.x + 7 + 3;

		if (x < itemX1 || x > itemX2)
			continue;

		int itemY1 = curItem.y - _itemHtDat[curItem.id] - 3;
		int itemY2 = curItem.y + 3;

		if (y < itemY1 || y > itemY2)
			continue;

		if (curItem.y >= yPos) {
			itemPos = i;
			yPos = curItem.y;
		}
	}

	return itemPos;
}

void KyraEngine_HoF::updateWaterFlasks() {
	for (int i = 22; i < 24; i++) {
		if (_itemInHand == i)
			setHandItem(i - 1);

		for (int ii = 0; ii < 20; ii++) {
			if (_mainCharacter.inventory[ii] == i) {
				_mainCharacter.inventory[ii]--;
				if (ii < 10) {
					clearInventorySlot(ii, 0);
					_screen->drawShape(0, getShapePtr(i + 63), _inventoryX[ii], _inventoryY[ii], 0, 0);
				}
			}
		}

		for (int ii = 0; ii < 30; ii++) {
			if (_itemList[ii].id == i)
				_itemList[ii].id--;
		}
	}
}

bool KyraEngine_HoF::dropItem(int unk1, Item item, int x, int y, int unk2) {
	if (_mouseState <= -1)
		return false;

	bool success = processItemDrop(_mainCharacter.sceneId, item, x, y, unk1, unk2);
	if (!success) {
		snd_playSoundEffect(0x0d);
		if (countAllItems() >= 30)
			showMessageFromCCode(5, 0x84, 0);
	}

	return success;
}

bool KyraEngine_HoF::processItemDrop(uint16 sceneId, Item item, int x, int y, int unk1, int unk2) {
	int itemPos = checkItemCollision(x, y);

	if (unk1)
		itemPos = -1;

	if (itemPos >= 0) {
		exchangeMouseItem(itemPos);
		return false;
	}

	int freeItemSlot = -1;

	if (unk1 != 3) {
		for (int i = 0; i < 30; ++i) {
			if (_itemList[i].id == kItemNone) {
				freeItemSlot = i;
				break;
			}
		}
	}

	if (freeItemSlot == -1)
		return false;

	if (sceneId != _mainCharacter.sceneId) {
		_itemList[freeItemSlot].x = x;
		_itemList[freeItemSlot].y = y;
		_itemList[freeItemSlot].id = item;
		_itemList[freeItemSlot].sceneId = sceneId;
		return true;
	}

	int itemHeight = _itemHtDat[item];

	// no idea why it's '&&' here and not single checks for x and y
	if (x == -1 && y == -1) {
		x = _rnd.getRandomNumberRng(0x10, 0x130);
		y = _rnd.getRandomNumberRng(0x10, 0x87);
	}

	int posX = x, posY = y;
	int itemX = -1, itemY = -1;
	bool needRepositioning = true;

	while (needRepositioning) {
		if ((_screen->getDrawLayer(posX, posY) <= 1 && _screen->getDrawLayer2(posX, posY, itemHeight) <= 1 && isDropable(posX, posY)) || posY == 136) {
			int posX2 = posX, posX3 = posX;
			bool repositioning = true;

			while (repositioning) {
				if (isDropable(posX3, posY) && _screen->getDrawLayer(posX3, posY) < 7 && checkItemCollision(posX3, posY) == -1) {
					itemX = posX3;
					itemY = posY;
					needRepositioning = false;
					repositioning = false;
				}

				if (isDropable(posX2, posY) && _screen->getDrawLayer(posX2, posY) < 7 && checkItemCollision(posX2, posY) == -1) {
					itemX = posX2;
					itemY = posY;
					needRepositioning = false;
					repositioning = false;
				}

				if (repositioning) {
					posX3 = MAX(posX3 - 2, 16);
					posX2 = MIN(posX2 + 2, 304);

					if (posX3 <= 16 && posX2 >= 304)
						repositioning = false;
				}
			}
		}

		if (posY == 136)
			needRepositioning = false;
		else
			posY = MIN(posY + 2, 136);
	}

	if (itemX == -1 || itemY == -1)
		return false;

	if (unk1 == 3) {
		_itemList[freeItemSlot].x = itemX;
		_itemList[freeItemSlot].y = itemY;
		return true;
	} else if (unk1 == 2) {
		itemDropDown(x, y, itemX, itemY, freeItemSlot, item);
	}

	if (!unk1)
		removeHandItem();

	itemDropDown(x, y, itemX, itemY, freeItemSlot, item);

	if (!unk1 && unk2) {
		int itemStr = 3;
		if (_lang == 1)
			itemStr = getItemCommandStringDrop(item);
		updateCommandLineEx(item+54, itemStr, 0xD6);
	}

	return true;
}

void KyraEngine_HoF::itemDropDown(int startX, int startY, int dstX, int dstY, int itemSlot, Item item) {
	uint8 *itemShape = getShapePtr(item + 64);

	if (startX == dstX && startY == dstY) {
		if (_layerFlagTable[_screen->getLayer(dstX, dstY)] && item != 13) {
			updateCharFacing();
			snd_playSoundEffect(0x2d);
			removeHandItem();
			objectChat(getTableString(0xFF, _cCodeBuffer, 1), 0, 0x83, 0xFF);
		} else {
			_itemList[itemSlot].x = dstX;
			_itemList[itemSlot].y = dstY;
			_itemList[itemSlot].id = item;
			_itemList[itemSlot].sceneId = _mainCharacter.sceneId;
			snd_playSoundEffect(0x0c);
			addItemToAnimList(itemSlot);
		}
	} else {
		_screen->hideMouse();

		if (startY <= dstY) {
			int speed = 2;
			int curY = startY;
			int curX = startX - 8;

			backUpGfxRect24x24(curX, curY-16);
			while (curY < dstY) {
				restoreGfxRect24x24(curX, curY-16);

				curY = MIN(curY + speed, dstY);
				++speed;

				backUpGfxRect24x24(curX, curY-16);
				uint32 endDelay = _system->getMillis() + _tickLength;

				_screen->drawShape(0, itemShape, curX, curY-16, 0, 0);
				_screen->updateScreen();

				delayUntil(endDelay, false, true);
			}

			if (dstX != dstY || (dstY - startY > 16)) {
				snd_playSoundEffect(0x69);
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
					x = (curX >> 4) - 8;
					y = curY - 16;

					restoreGfxRect24x24(x, y);
					curY = MIN(curY + speed, dstY);
					curX += speedX;
					++speed;

					x = (curX >> 4) - 8;
					y = curY - 16;
					backUpGfxRect24x24(x, y);

					uint16 endDelay = _system->getMillis() + _tickLength;
					_screen->drawShape(0, itemShape, x, y, 0, 0);
					_screen->updateScreen();

					delayUntil(endDelay, false, true);
				}

				restoreGfxRect24x24(x, y);
			} else {
				restoreGfxRect24x24(curX, curY-16);
			}
		}

		if (_layerFlagTable[_screen->getLayer(dstX, dstY)] && item != 13) {
			updateCharFacing();
			snd_playSoundEffect(0x2d);
			removeHandItem();
			_screen->showMouse();
			objectChat(getTableString(0xFF, _cCodeBuffer, 1), 0, 0x83, 0xFF);
		} else {
			_itemList[itemSlot].x = dstX;
			_itemList[itemSlot].y = dstY;
			_itemList[itemSlot].id = item;
			_itemList[itemSlot].sceneId = _mainCharacter.sceneId;
			snd_playSoundEffect(0x0c);
			addItemToAnimList(itemSlot);
			_screen->showMouse();
		}
	}
}

void KyraEngine_HoF::exchangeMouseItem(int itemPos) {
	deleteItemAnimEntry(itemPos);

	int itemId = _itemList[itemPos].id;
	_itemList[itemPos].id = _itemInHand;
	_itemInHand = itemId;

	addItemToAnimList(itemPos);
	snd_playSoundEffect(0x0b);
	setMouseCursor(_itemInHand);
	int str2 = 7;

	if (_lang == 1)
		str2 = getItemCommandStringPickUp(itemId);

	updateCommandLineEx(itemId + 54, str2, 0xD6);

	runSceneScript6();
}

bool KyraEngine_HoF::pickUpItem(int x, int y) {
	int itemPos = checkItemCollision(x, y);

	if (itemPos <= -1)
		return false;

	if (_itemInHand >= 0) {
		exchangeMouseItem(itemPos);
	} else {
		deleteItemAnimEntry(itemPos);
		int itemId = _itemList[itemPos].id;
		_itemList[itemPos].id = kItemNone;
		snd_playSoundEffect(0x0b);
		setMouseCursor(itemId);
		int str2 = 7;

		if (_lang == 1)
			str2 = getItemCommandStringPickUp(itemId);

		updateCommandLineEx(itemId + 54, str2, 0xD6);
		_itemInHand = itemId;

		runSceneScript6();
	}

	return true;
}

bool KyraEngine_HoF::isDropable(int x, int y) {
	if (x < 14 || x > 304 || y < 14 || y > 136)
		return false;

	x -= 8;
	y -= 1;

	for (int xpos = x; xpos < x + 16; ++xpos) {
		if (_screen->getShapeFlag1(xpos, y) == 0)
			return false;
	}

	return true;
}

int KyraEngine_HoF::getItemCommandStringDrop(Item item) {
	assert(item >= 0 && item < _itemStringMapSize);
	int stringId = _itemStringMap[item];

	static const int dropStringIds[] = {
		0x2D, 0x103, 0x003, 0x106
	};
	assert(stringId < ARRAYSIZE(dropStringIds));

	return dropStringIds[stringId];
}

int KyraEngine_HoF::getItemCommandStringPickUp(Item item) {
	assert(item >= 0 && item < _itemStringMapSize);
	int stringId = _itemStringMap[item];

	static const int pickUpStringIds[] = {
		0x02B, 0x102, 0x007, 0x105
	};
	assert(stringId < ARRAYSIZE(pickUpStringIds));

	return pickUpStringIds[stringId];
}

int KyraEngine_HoF::getItemCommandStringInv(Item item) {
	assert(item >= 0 && item < _itemStringMapSize);
	int stringId = _itemStringMap[item];

	static const int pickUpStringIds[] = {
		0x02C, 0x104, 0x008, 0x107
	};
	assert(stringId < ARRAYSIZE(pickUpStringIds));

	return pickUpStringIds[stringId];
}

bool KyraEngine_HoF::itemIsFlask(Item item) {
	for (int i = 0; _flaskTable[i] != kItemNone; ++i) {
		if (_flaskTable[i] == item)
			return true;
	}

	return false;
}

void KyraEngine_HoF::setMouseCursor(Item item) {
	int shape = 0;
	int hotX = 1;
	int hotY = 1;

	if (item != kItemNone) {
		hotX = 8;
		hotY = 15;
		shape = item+64;
	}

	_screen->setMouseCursor(hotX, hotY, getShapePtr(shape));
}

} // End of namespace Kyra
