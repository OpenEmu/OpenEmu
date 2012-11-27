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
#include "kyra/wsamovie.h"

#include "common/system.h"

namespace Kyra {

void KyraEngine_HoF::restorePage3() {
	screen()->copyBlockToPage(2, 0, 0, 320, 144, _gamePlayBuffer);
}

void KyraEngine_HoF::clearAnimObjects() {
	_animObjects[0].index = 0;
	_animObjects[0].type = 0;
	_animObjects[0].enabled = 1;
	_animObjects[0].flags = 0x800;
	_animObjects[0].width = 32;
	_animObjects[0].height = 49;
	_animObjects[0].width2 = 4;
	_animObjects[0].height2 = 10;

	for (int i = 1; i < 11; ++i) {
		_animObjects[i].index = i;
		_animObjects[i].type = 2;
	}

	for (int i = 11; i <= 40; ++i) {
		_animObjects[i].index = i;
		_animObjects[i].type = 1;
		_animObjects[i].flags = 0x800;
		_animObjects[i].width = 16;
		_animObjects[i].height = 16;
	}
}

void KyraEngine_HoF::drawAnimObjects() {
	for (AnimObj *curObject = _animList; curObject; curObject = curObject->nextObject) {
		if (!curObject->enabled)
			continue;

		int x = curObject->xPos2 - (_screen->getScreenDim(2)->sx << 3);
		int y = curObject->yPos2 - _screen->getScreenDim(2)->sy;
		int layer = 7;

		if (curObject->flags & 0x800) {
			if (curObject->animFlags)
				layer = 0;
			else
				layer = getDrawLayer(curObject->xPos1, curObject->yPos1);
		}
		curObject->flags |= 0x800;

		if (curObject->index)
			drawSceneAnimObject(curObject, x, y, layer);
		else
			drawCharacterAnimObject(curObject, x, y, layer);
	}
}

void KyraEngine_HoF::refreshAnimObjects(int force) {
	for (AnimObj *curObject = _animList; curObject; curObject = curObject->nextObject) {
		if (!curObject->enabled)
			continue;
		if (!curObject->needRefresh && !force)
			continue;

		int x = curObject->xPos2 - curObject->width2;
		if (x < 0)
			x = 0;
		if (x >= 320)
			x = 319;
		int y = curObject->yPos2 - curObject->height2;
		if (y < 0)
			y = 0;
		if (y >= 143)
			y = 142;

		int width = curObject->width + curObject->width2 + 8;
		int height = curObject->height + curObject->height2*2;
		if (width + x > 320)
			width -= width + x - 322;
		if (height + y > 143)
			height -= height + y - 144;

		_screen->copyRegion(x, y, x, y, width, height, 2, 0, Screen::CR_NO_P_CHECK);

		curObject->needRefresh = false;
	}
}

void KyraEngine_HoF::updateItemAnimations() {
	bool nextFrame = false;

	if (_itemAnimDefinition[0].itemIndex == -1 || _inventorySaved)
		return;

	const ItemAnimDefinition *s = &_itemAnimDefinition[_nextAnimItem];
	ActiveItemAnim *a = &_activeItemAnim[_nextAnimItem];
	_nextAnimItem = (_nextAnimItem + 1) % _itemAnimDefinitionSize;

	if (_system->getMillis() < a->nextFrameTime)
		return;

	uint16 shpIdx = s->frames[a->currentFrame].index + 64;
	if (s->itemIndex == _mouseState && s->itemIndex == _itemInHand && _screen->isMouseVisible()) {
		nextFrame = true;
		_screen->setMouseCursor(8, 15, getShapePtr(shpIdx));
	}

	for (int i = 0; i < 10; i++) {
		if (s->itemIndex == _mainCharacter.inventory[i]) {
			nextFrame = true;
			_screen->drawShape(2, getShapePtr(240 + i), 304, 184, 0, 0);
			_screen->drawShape(2, getShapePtr(shpIdx), 304, 184, 0, 0);
			_screen->copyRegion(304, 184, _inventoryX[i], _inventoryY[i], 16, 16, 2, 0);
		}
	}

	_screen->updateScreen();

	for (int i = 11; i < 40; i++) {
		AnimObj *animObject = &_animObjects[i];
		if (animObject->shapeIndex2 == s->itemIndex + 64) {
			if (s->itemIndex == 121) {
				int f = findItem(_mainCharacter.sceneId, 121);
				int nx = _itemList[f].x - 4;
				if (nx > 12) {
					if (lineIsPassable(nx, _itemList[f].y)) {
						animObject->xPos2 -= 4;
						_itemList[f].x -= 4;
					}
				}
			}
			animObject->shapePtr = getShapePtr(shpIdx);
			animObject->shapeIndex1 = shpIdx;
			animObject->needRefresh = 1;
			nextFrame = true;
		}
	}

	if (nextFrame) {
		a->nextFrameTime = _system->getMillis() + (s->frames[a->currentFrame].delay * _tickLength);
		a->currentFrame = (a->currentFrame + 1) % s->numFrames;
	}
}

void KyraEngine_HoF::updateCharFacing() {
	if (_mainCharacter.x1 > _mouseX)
		_mainCharacter.facing = 5;
	else
		_mainCharacter.facing = 3;

	_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
	updateCharacterAnim(0);
	refreshAnimObjectsIfNeed();
}

void KyraEngine_HoF::updateCharacterAnim(int) {
	Character *c = &_mainCharacter;
	AnimObj *animState = _animObjects;

	animState->needRefresh = 1;
	animState->specialRefresh = 1;

	if (c->facing >= 1 && c->facing <= 3)
		animState->flags |= 1;
	else if (c->facing >= 5 && c->facing <= 7)
		animState->flags &= ~1;

	animState->xPos2 = animState->xPos1 = c->x1;
	animState->yPos2 = animState->yPos1 = c->y1;
	animState->shapePtr = getShapePtr(c->animFrame);
	animState->shapeIndex1 = animState->shapeIndex2 = c->animFrame;

	int xAdd = _shapeDescTable[c->animFrame-9].xAdd;
	int yAdd = _shapeDescTable[c->animFrame-9].yAdd;

	_charScale = getScale(c->x1, c->y1);

	animState->xPos2 += (xAdd * _charScale) >> 8;
	animState->yPos2 += (yAdd * _charScale) >> 8;
	animState->width2 = 8;
	animState->height2 = 10;

	_animList = deleteAnimListEntry(_animList, animState);
	if (_animList)
		_animList = addToAnimListSorted(_animList, animState);
	else
		_animList = initAnimList(_animList, animState);

	updateCharPal(1);
}

void KyraEngine_HoF::updateSceneAnim(int anim, int newFrame) {
	AnimObj *animObject = &_animObjects[1+anim];
	if (!animObject->enabled)
		return;

	animObject->needRefresh = 1;
	animObject->specialRefresh = 1;
	animObject->flags = 0;

	if (_sceneAnims[anim].flags & 2)
		animObject->flags |= 0x800;
	else
		animObject->flags &= ~0x800;

	if (_sceneAnims[anim].flags & 4)
		animObject->flags |= 1;
	else
		animObject->flags &= ~1;

	if (_sceneAnims[anim].flags & 0x20) {
		animObject->shapePtr = _sceneShapeTable[newFrame];
		animObject->shapeIndex2 = 0xFFFF;
		animObject->shapeIndex3 = 0xFFFF;
		animObject->animNum = 0xFFFF;
	} else {
		animObject->shapePtr = 0;
		animObject->shapeIndex3 = newFrame;
		animObject->animNum = anim;
	}

	animObject->xPos1 = _sceneAnims[anim].x;
	animObject->yPos1 = _sceneAnims[anim].y;
	animObject->xPos2 = _sceneAnims[anim].x2;
	animObject->yPos2 = _sceneAnims[anim].y2;

	if (_sceneAnims[anim].flags & 2) {
		_animList = deleteAnimListEntry(_animList, animObject);
		if (!_animList)
			_animList = initAnimList(_animList, animObject);
		else
			_animList = addToAnimListSorted(_animList, animObject);
	}
}

void KyraEngine_HoF::drawSceneAnimObject(AnimObj *obj, int x, int y, int layer) {
	if (obj->type == 1) {
		if (obj->shapeIndex1 == 0xFFFF)
			return;
		int scale = getScale(obj->xPos1, obj->yPos1);
		_screen->drawShape(2, getShapePtr(obj->shapeIndex1), x, y, 2, obj->flags | 4, layer, scale, scale);
		return;
	}

	if (obj->shapePtr) {
		_screen->drawShape(2, obj->shapePtr, x, y, 2, obj->flags, layer);
	} else {
		if (obj->shapeIndex3 == 0xFFFF || obj->animNum == 0xFFFF)
			return;

		int flags = 0x4000;
		if (obj->flags & 0x800)
			flags |= 0x8000;

		if (_sceneAnims[obj->animNum].wsaFlag) {
			x = y = 0;
		} else {
			x = obj->xPos2;
			y = obj->yPos2;
		}

		_sceneAnimMovie[obj->animNum]->displayFrame(obj->shapeIndex3, 2, x, y, int(flags | layer), 0, 0);
	}
}

void KyraEngine_HoF::drawCharacterAnimObject(AnimObj *obj, int x, int y, int layer) {
	if (_drawNoShapeFlag || obj->shapeIndex1 == 0xFFFF)
		return;
	_screen->drawShape(2, getShapePtr(obj->shapeIndex1), x, y, 2, obj->flags | 4, layer, _charScale, _charScale);
}

void KyraEngine_HoF::setCharacterAnimDim(int w, int h) {
	restorePage3();

	_animObj0Width = _animObjects[0].width;
	_animObj0Height = _animObjects[0].height;

	_animObjects[0].width = w;
	_animObjects[0].height = h;
}

void KyraEngine_HoF::resetCharacterAnimDim() {
	restorePage3();

	_animObjects[0].width = _animObj0Width;
	_animObjects[0].height = _animObj0Height;
}

} // End of namespace Kyra
