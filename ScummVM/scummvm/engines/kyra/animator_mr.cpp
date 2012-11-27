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
#include "kyra/resource.h"
#include "kyra/wsamovie.h"

#include "common/system.h"

namespace Kyra {

void KyraEngine_MR::restorePage3() {
	screen()->copyBlockToPage(2, 0, 0, 320, 200, _gamePlayBuffer);
}

void KyraEngine_MR::clearAnimObjects() {
	for (int i = 0; i < 67; ++i)
		_animObjects[i].enabled = false;

	_animObjects[0].index = 0;
	_animObjects[0].type = 0;
	_animObjects[0].enabled = true;
	_animObjects[0].specialRefresh = 1;
	_animObjects[0].flags = 0x800;
	_animObjects[0].width = 57;
	_animObjects[0].height = 91;
	_animObjects[0].width2 = 4;
	_animObjects[0].height2 = 10;

	for (int i = 1; i < 17; ++i) {
		_animObjects[i].index = i;
		_animObjects[i].type = 2;
		_animObjects[i].flags = 0;
		_animObjects[i].enabled = false;
		_animObjects[i].needRefresh = 0;
		_animObjects[i].specialRefresh = 1;
	}

	for (int i = 17; i <= 66; ++i) {
		_animObjects[i].index = i;
		_animObjects[i].type = 1;
		_animObjects[i].specialRefresh = 1;
		_animObjects[i].flags = 0x800;
		_animObjects[i].width = 24;
		_animObjects[i].height = 20;
		_animObjects[i].width2 = 0;
		_animObjects[i].height2 = 0;
	}
}

void KyraEngine_MR::animSetupPaletteEntry(AnimObj *anim) {
	int layer = _screen->getLayer(anim->xPos1, anim->yPos1) - 1;
	int16 count = 0;
	for (int i = 0; i < 3; ++i)
		count += _sceneDatPalette[layer*3+i];
	count /= 3;
	count *= -1;
	count = MAX<int16>(0, MIN<int16>(count, 10));
	anim->palette = count / 3;
}

void KyraEngine_MR::drawAnimObjects() {
	for (AnimObj *curObject = _animList; curObject; curObject = curObject->nextObject) {
		if (!curObject->enabled)
			continue;

		int x = curObject->xPos2 - (_screen->getScreenDim(2)->sx << 3);
		int y = curObject->yPos2 - _screen->getScreenDim(2)->sy;
		int layer = 7;

		if (curObject->flags & 0x800) {
			if (!curObject->specialRefresh)
				layer = 0;
			else
				layer = getDrawLayer(curObject->xPos1, curObject->yPos1);
		}

		if (curObject->index)
			drawSceneAnimObject(curObject, x, y, layer);
		else
			drawCharacterAnimObject(curObject, x, y, layer);
	}
}

void KyraEngine_MR::drawSceneAnimObject(AnimObj *obj, int x, int y, int layer) {
	if (obj->type == 1) {
		if (obj->shapeIndex1 == 0xFFFF)
			return;
		int scale = getScale(obj->xPos1, obj->yPos1);
		_screen->drawShape(2, getShapePtr(obj->shapeIndex1), x, y, 2, obj->flags | 0x104, _paletteOverlay, obj->palette, layer, scale, scale);
	} else {
		if (obj->shapePtr) {
			_screen->drawShape(2, obj->shapePtr, x, y, 2, obj->flags, 7);
		} else {
			if (obj->shapeIndex3 == 0xFFFF || obj->animNum == 0xFFFF)
				return;
			uint16 flags = 0x4000;
			if (obj->flags & 0x800)
				flags |= 0x8000;
			x = obj->xPos2 - _sceneAnimMovie[obj->animNum]->xAdd();
			y = obj->yPos2 - _sceneAnimMovie[obj->animNum]->yAdd();
			_sceneAnimMovie[obj->animNum]->displayFrame(obj->shapeIndex3, 2, x, y, flags | layer, 0, 0);
		}
	}
}

void KyraEngine_MR::drawCharacterAnimObject(AnimObj *obj, int x, int y, int layer) {
	if (_drawNoShapeFlag)
		return;

	if (_mainCharacter.animFrame < 9)
		_mainCharacter.animFrame = 87;

	if (obj->shapeIndex1 == 0xFFFF || _mainCharacter.animFrame == 87)
		return;

	_screen->drawShape(2, getShapePtr(421), _mainCharacter.x3, _mainCharacter.y3, 2, obj->flags | 0x304, _paletteOverlay, 3, layer, _charScale, _charScale);
	uint8 *shape = getShapePtr(_mainCharacter.animFrame);
	if (shape)
		_screen->drawShape(2, shape, x, y, 2, obj->flags | 4, layer, _charScale, _charScale);
}

void KyraEngine_MR::refreshAnimObjects(int force) {
	for (AnimObj *curObject = _animList; curObject; curObject = curObject->nextObject) {
		if (!curObject->enabled)
			continue;
		if (!curObject->needRefresh && !force)
			continue;

		const int scale = (curObject->index == 0) ? _charScale : 0;

		int x = curObject->xPos2 - curObject->width2;
		if (scale)
			x -= (0x100 - scale) >> 4;

		if (x < 0)
			x = 0;
		if (x >= 320)
			x = 319;

		int y = curObject->yPos2 - curObject->height2;
		if (scale)
			y -= (0x100 - scale) >> 3;
		if (y < 0)
			y = 0;
		if (y >= 187)
			y = 186;

		int width = curObject->width + curObject->width2 + 8;
		int height = curObject->height + curObject->height2*2;
		if (width + x > 320)
			width -= width + x - 322;

		const int maxY = _inventoryState ? 143 : 187;
		if (height + y > maxY)
			height -= height + y - (maxY + 1);

		if (height > 0) {
			_screen->copyRegion(x, y, x, y, width, height, 2, 0, Screen::CR_NO_P_CHECK);
		}

		curObject->needRefresh = false;
	}
}

void KyraEngine_MR::updateItemAnimations() {
	bool nextFrame = false;

	if (_itemAnimDefinition[0].itemIndex == -1)
		return;

	const ItemAnimDefinition *s = &_itemAnimDefinition[_nextAnimItem];
	ActiveItemAnim *a = &_activeItemAnim[_nextAnimItem];
	_nextAnimItem = (_nextAnimItem + 1) % 10;

	if (_system->getMillis() < a->nextFrameTime)
		return;

	uint16 shpIdx = s->frames[a->currentFrame].index + 248;
	if (s->itemIndex == _mouseState && s->itemIndex == _itemInHand && _screen->isMouseVisible()) {
		nextFrame = true;
		_screen->setMouseCursor(12, 19, getShapePtr(shpIdx));
	}

	if (_inventoryState) {
		for (int i = 0; i < 10; i++) {
			if (s->itemIndex == _mainCharacter.inventory[i]) {
				nextFrame = true;
				_screen->drawShape(2, getShapePtr(422 + i), 9, 0, 0, 0);
				_screen->drawShape(2, getShapePtr(shpIdx), 9, 0, 0, 0);
				_screen->copyRegion(9, 0, _inventoryX[i], _inventoryY[i], 24, 20, 2, 0, Screen::CR_NO_P_CHECK);
			}
		}
	}

	_screen->updateScreen();

	for (int i = 17; i < 66; i++) {
		AnimObj *animObject = &_animObjects[i];
		if (animObject->shapeIndex2 == s->itemIndex + 248) {
			animObject->shapePtr = getShapePtr(shpIdx);
			animObject->shapeIndex1 = shpIdx;
			animObject->needRefresh = true;
			nextFrame = true;
		}
	}

	if (nextFrame) {
		a->nextFrameTime = _system->getMillis() + (s->frames[a->currentFrame].delay * _tickLength);
		a->currentFrame = (a->currentFrame + 1) % s->numFrames;
	}
}

void KyraEngine_MR::updateCharacterAnim(int charId) {
	AnimObj *obj = &_animObjects[0];
	obj->needRefresh = true;
	obj->flags &= ~1;
	obj->xPos1 = _mainCharacter.x1;
	obj->yPos1 = _mainCharacter.y1;
	obj->shapePtr = getShapePtr(_mainCharacter.animFrame);
	obj->shapeIndex1 = obj->shapeIndex2 = _mainCharacter.animFrame;

	int shapeOffsetX = 0, shapeOffsetY = 0;
	if (_mainCharacter.animFrame >= 50 && _mainCharacter.animFrame <= 87) {
		shapeOffsetX = _malcolmShapeXOffset;
		shapeOffsetY = _malcolmShapeYOffset;
	} else {
		shapeOffsetX = _animShapeXAdd;
		shapeOffsetY = _animShapeYAdd;
	}

	obj->xPos2 = _mainCharacter.x1;
	obj->yPos2 = _mainCharacter.y1;
	_charScale = getScale(_mainCharacter.x1, _mainCharacter.y1);
	obj->xPos2 += (shapeOffsetX * _charScale) >> 8;
	obj->yPos2 += (shapeOffsetY * _charScale) >> 8;
	_mainCharacter.x3 = _mainCharacter.x1 - (_charScale >> 4) - 1;
	_mainCharacter.y3 = _mainCharacter.y1 - (_charScale >> 6) - 1;
	if (_charBackUpWidth2 == -1) {
		obj->width2 = 4;
		obj->height2 = 10;
	}

	for (int i = 1; i <= 16; ++i) {
		if (_animObjects[i].enabled && _animObjects[i].specialRefresh)
			_animObjects[i].needRefresh = true;
	}

	_animList = deleteAnimListEntry(_animList, obj);
	if (_animList)
		_animList = addToAnimListSorted(_animList, obj);
	else
		_animList = initAnimList(_animList, obj);

	if (!_loadingState)
		updateCharPal(1);
}

void KyraEngine_MR::updateSceneAnim(int anim, int newFrame) {
	AnimObj *animObject = &_animObjects[1+anim];
	if (!animObject->enabled)
		return;

	animObject->needRefresh = true;

	if (_sceneAnims[anim].flags & 2)
		animObject->flags |= 1;
	else
		animObject->flags &= ~1;

	if (_sceneAnims[anim].flags & 4) {
		animObject->shapePtr = _sceneShapes[newFrame];
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

	if (_sceneAnims[anim].flags & 0x20) {
		_animList = deleteAnimListEntry(_animList, animObject);
		if (!_animList)
			_animList = initAnimList(_animList, animObject);
		else
			_animList = addToAnimListSorted(_animList, animObject);
	}
}

void KyraEngine_MR::setupSceneAnimObject(int animId, uint16 flags, int x, int y, int x2, int y2, int w,
										int h, int unk10, int specialSize, int unk14, int shape, const char *filename) {
	restorePage3();
	SceneAnim &anim = _sceneAnims[animId];
	anim.flags = flags;
	anim.x = x;
	anim.y = y;
	anim.x2 = x2;
	anim.y2 = y2;
	anim.width = w;
	anim.height = h;
	anim.specialSize = specialSize;
	anim.shapeIndex = shape;
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

	AnimObj *obj = &_animObjects[1+animId];
	obj->enabled = true;
	obj->needRefresh = true;

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
		obj->animNum = animId;
	} else {
		obj->shapeIndex3 = 0xFFFF;
		obj->animNum = 0xFFFF;
	}

	obj->xPos3 = obj->xPos2 = anim.x2;
	obj->yPos3 = obj->yPos2 = anim.y2;
	obj->width = anim.width;
	obj->height = anim.height;
	obj->width2 = obj->height2 = anim.specialSize;

	if (_animList)
		_animList = addToAnimListSorted(_animList, obj);
	else
		_animList = initAnimList(_animList, obj);
}

void KyraEngine_MR::removeSceneAnimObject(int anim, int refresh) {
	AnimObj *obj = &_animObjects[anim+1];
	restorePage3();
	obj->shapeIndex3 = 0xFFFF;
	obj->animNum = 0xFFFF;
	obj->needRefresh = true;

	if (refresh)
		refreshAnimObjectsIfNeed();

	obj->enabled = false;
	_animList = deleteAnimListEntry(_animList, obj);
	_sceneAnimMovie[anim]->close();
}

void KyraEngine_MR::setCharacterAnimDim(int w, int h) {
	restorePage3();
	_charBackUpWidth = _animObjects[0].width;
	_charBackUpWidth2 = _animObjects[0].width2;
	_charBackUpHeight = _animObjects[0].height;
	_charBackUpHeight2 = _animObjects[0].height2;

	_animObjects[0].width2 = (w - _charBackUpWidth) / 2;
	_animObjects[0].height2 = h - _charBackUpHeight;
	_animObjects[0].width = w;
	_animObjects[0].height = h;
}

void KyraEngine_MR::resetCharacterAnimDim() {
	restorePage3();
	_animObjects[0].width2 = _charBackUpWidth2;
	_animObjects[0].height2 = _charBackUpHeight2;
	_animObjects[0].width = _charBackUpWidth;
	_animObjects[0].height = _charBackUpHeight;
	_charBackUpWidth2 = _charBackUpHeight2 = -1;
	_charBackUpWidth = _charBackUpHeight = -1;
}

void KyraEngine_MR::showIdleAnim() {
	if (_mainCharacter.sceneId == 20 || _mainCharacter.sceneId == 21
			|| _mainCharacter.sceneId == 12 || _mainCharacter.sceneId == 11)
		return;

	if (_mainCharacter.animFrame == 87)
		return;

	if (!_nextIdleType && !talkObjectsInCurScene()) {
		randomSceneChat();
	} else {
		static const char *const facingTable[] = {
			"A", "R", "R", "FR", "FX", "FL", "L", "L"
		};

		Common::String filename = Common::String::format( "MI0%s%.02d.EMC", facingTable[_mainCharacter.facing], _characterShapeFile);

		if (_res->exists(filename.c_str()))
			runAnimationScript(filename.c_str(), 1, 1, 1, 1);
	}

	_nextIdleType = !_nextIdleType;
}

} // End of namespace Kyra
