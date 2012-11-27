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

#include "kyra/kyra_v2.h"
#include "kyra/screen_v2.h"

namespace Kyra {

void KyraEngine_v2::allocAnimObjects(int actors, int anims, int items) {
	_animObjects = new AnimObj[actors + anims + items];
	assert(_animObjects);

	memset(_animObjects, 0, sizeof(AnimObj) * (actors + anims + items));

	_animActor = _animObjects;
	_animAnims = _animObjects + actors;
	_animItems = _animObjects + actors + anims;
}

KyraEngine_v2::AnimObj *KyraEngine_v2::initAnimList(AnimObj *list, AnimObj *entry) {
	entry->nextObject = list;
	return entry;
}

KyraEngine_v2::AnimObj *KyraEngine_v2::addToAnimListSorted(AnimObj *list, AnimObj *add) {
	add->nextObject = 0;

	if (!list)
		return add;

	if (add->yPos1 <= list->yPos1) {
		add->nextObject = list;
		return add;
	}

	AnimObj *cur = list;
	AnimObj *prev = list;
	while (add->yPos1 > cur->yPos1) {
		AnimObj *temp = cur->nextObject;
		if (!temp)
			break;
		prev = cur;
		cur = temp;
	}

	if (add->yPos1 <= cur->yPos1) {
		prev->nextObject = add;
		add->nextObject = cur;
	} else {
		cur->nextObject = add;
		add->nextObject = 0;
	}
	return list;
}

KyraEngine_v2::AnimObj *KyraEngine_v2::deleteAnimListEntry(AnimObj *list, AnimObj *entry) {
	if (!list)
		return 0;

	AnimObj *old = 0;
	AnimObj *cur = list;

	while (true) {
		if (cur == entry)
			break;
		if (!cur->nextObject)
			break;
		old = cur;
		cur = cur->nextObject;
	}

	if (cur != entry)
		return list;

	if (cur == list) {
		if (!cur->nextObject)
			return 0;
		cur = cur->nextObject;
		return cur;
	}

	if (!cur->nextObject) {
		if (!old)
			return 0;
		old->nextObject = 0;
		return list;
	}

	if (cur != entry)
		return list;

	old->nextObject = entry->nextObject;
	return list;
}

void KyraEngine_v2::refreshAnimObjectsIfNeed() {
	for (AnimObj *curEntry = _animList; curEntry; curEntry = curEntry->nextObject) {
		if (curEntry->enabled && curEntry->needRefresh) {
			restorePage3();
			drawAnimObjects();
			refreshAnimObjects(0);
			screen()->updateScreen();
			return;
		}
	}
}

void KyraEngine_v2::flagAnimObjsForRefresh() {
	for (AnimObj *curEntry = _animList; curEntry; curEntry = curEntry->nextObject)
		curEntry->needRefresh = 1;
}

void KyraEngine_v2::flagAnimObjsSpecialRefresh() {
	for (AnimObj *curEntry = _animList; curEntry; curEntry = curEntry->nextObject)
		curEntry->specialRefresh = 1;
}

void KyraEngine_v2::addItemToAnimList(int item) {
	assert(item >= 0 && item < _itemListSize);

	restorePage3();

	AnimObj *animObj = _animItems + item;

	animObj->enabled = 1;
	animObj->needRefresh = 1;

	int itemId = _itemList[item].id;

	animObj->xPos2 = animObj->xPos1 = _itemList[item].x;
	animObj->yPos2 = animObj->yPos1 = _itemList[item].y;

	animObj->shapePtr = getShapePtr(itemId + _desc.itemShapeStart);
	animSetupPaletteEntry(animObj);
	animObj->shapeIndex2 = animObj->shapeIndex1 = itemId + _desc.itemShapeStart;

	int scaleY, scaleX;
	scaleY = scaleX = getScale(animObj->xPos1, animObj->yPos1);

	uint8 *shapePtr = getShapePtr(itemId + _desc.itemShapeStart);
	animObj->xPos3 = (animObj->xPos2 -= (screen_v2()->getShapeScaledWidth(shapePtr, scaleX) >> 1));
	animObj->yPos3 = (animObj->yPos2 -= screen_v2()->getShapeScaledHeight(shapePtr, scaleY));

	animObj->width2 = animObj->height2 = 0;

	_animList = addToAnimListSorted(_animList, animObj);
	animObj->needRefresh = 1;
}

void KyraEngine_v2::deleteItemAnimEntry(int item) {
	assert(item < _itemListSize);

	AnimObj *animObj = _animItems + item;

	restorePage3();

	animObj->shapePtr = 0;
	animObj->shapeIndex1 = 0xFFFF;
	animObj->shapeIndex2 = 0xFFFF;
	animObj->needRefresh = 1;

	refreshAnimObjectsIfNeed();

	animObj->enabled = 0;
	_animList = deleteAnimListEntry(_animList, animObj);
}

} // End of namespace Kyra
