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
 *
 */

#include "common/system.h"

#include "toltecs/toltecs.h"
#include "toltecs/render.h"
#include "toltecs/resource.h"

namespace Toltecs {

Common::Rect makeRect(int16 x, int16 y, int16 width, int16 height) {
	Common::Rect rect;
	rect.left = x;
	rect.top = y;
	rect.setWidth(width);
	rect.setHeight(height);
	return rect;
}

RenderQueue::RenderQueue(ToltecsEngine *vm) : _vm(vm) {
	_currQueue = new RenderQueueArray();
	_prevQueue = new RenderQueueArray();
	_updateUta = new MicroTileArray(640, 400);
}

RenderQueue::~RenderQueue() {
	delete _currQueue;
	delete _prevQueue;
	delete _updateUta;
}

void RenderQueue::addSprite(SpriteDrawItem &sprite) {

	RenderQueueItem item;
	item.type = kSprite;
	item.flags = kRefresh;
	item.rect = makeRect(sprite.x - _vm->_cameraX, sprite.y - _vm->_cameraY, sprite.width, sprite.height);
	item.priority = sprite.priority;

	item.sprite = sprite;
	item.sprite.x -= _vm->_cameraX;
	item.sprite.y -= _vm->_cameraY;

	// Add sprite sorted by priority
	RenderQueueArray::iterator iter = _currQueue->begin();
	while (iter != _currQueue->end() && (*iter).priority <= item.priority) {
		iter++;
	}
	_currQueue->insert(iter, item);

}

void RenderQueue::addText(int16 x, int16 y, byte color, uint fontResIndex, byte *text, int len) {

	Font font(_vm->_res->load(fontResIndex)->data);

	RenderQueueItem item;
	item.type = kText;
	item.flags = kRefresh;
	item.rect = makeRect(x, y, font.getTextWidth(text), font.getHeight());
	item.priority = 1000;

	item.text.color = color;
	item.text.fontResIndex = fontResIndex;
	item.text.text = text;
	item.text.len = len;

	_currQueue->push_back(item);

}

void RenderQueue::addMask(SegmapMaskRect &mask) {

	RenderQueueItem item;
	item.type = kMask;
	item.flags = kRefresh;
	item.rect = makeRect(mask.x - _vm->_cameraX, mask.y - _vm->_cameraY, mask.width, mask.height);
	item.priority = mask.priority;

	item.mask = mask;

	// Only add the mask if a sprite intersects its rect
	if (rectIntersectsItem(item.rect)) {
		RenderQueueArray::iterator iter = _currQueue->begin();
		while (iter != _currQueue->end() && (*iter).priority <= item.priority) {
			iter++;
		}
		_currQueue->insert(iter, item);
	}

}

void RenderQueue::update() {

	bool doFullRefresh = _vm->_screen->_fullRefresh;

	_updateUta->clear();

	if (!doFullRefresh) {

		for (RenderQueueArray::iterator iter = _currQueue->begin(); iter != _currQueue->end(); iter++) {
			RenderQueueItem *item = &(*iter);
			RenderQueueItem *prevItem = findItemInQueue(_prevQueue, *item);
			if (prevItem) {
				if (hasItemChanged(*prevItem, *item)) {
					item->flags = kRefresh;
					addDirtyRect(prevItem->rect);
				} else {
					item->flags = kUnchanged;
				}
			} else {
				item->flags = kRefresh;
			}
		}

		for (RenderQueueArray::iterator iter = _prevQueue->begin(); iter != _prevQueue->end(); iter++) {
			RenderQueueItem *prevItem = &(*iter);
			RenderQueueItem *item = findItemInQueue(_currQueue, *prevItem);
			if (!item) {
				prevItem->flags = kRemoved;
				addDirtyRect(prevItem->rect);
			}
		}

		restoreDirtyBackground();

		for (RenderQueueArray::iterator iter = _currQueue->begin(); iter != _currQueue->end(); iter++) {
			RenderQueueItem *item = &(*iter);
			if (item->flags != kUnchanged)
				invalidateItemsByRect(item->rect, item);
		}

	} else {
		byte *destp = _vm->_screen->_frontScreen;
		byte *srcp = _vm->_screen->_backScreen + _vm->_cameraX + _vm->_cameraY * _vm->_sceneWidth;
		int16 w = MIN<int16>(640, _vm->_sceneWidth);
		int16 h = MIN<int16>(400, _vm->_cameraHeight);
  		while (h--) {
			memcpy(destp, srcp, w);
			destp += 640;
			srcp += _vm->_sceneWidth;
		}
		_vm->_screen->_fullRefresh = false;
	}

	for (RenderQueueArray::iterator iter = _currQueue->begin(); iter != _currQueue->end(); iter++) {
		const RenderQueueItem *item = &(*iter);

		if (item->flags == kRefresh || doFullRefresh) {

			switch (item->type) {
			case kSprite:
				_vm->_screen->drawSprite(item->sprite);
				break;
			case kText:
				_vm->_screen->drawString(item->rect.left, item->rect.top, item->text.color, item->text.fontResIndex,
					item->text.text, item->text.len, NULL, true);
				break;
			case kMask:
				_vm->_screen->drawSurface(item->rect.left, item->rect.top, item->mask.surface);
				break;
			default:
				break;
			}

			if (!doFullRefresh)
				addDirtyRect(item->rect);

		}

	}

	if (doFullRefresh) {
		clear();
		_vm->_system->copyRectToScreen(_vm->_screen->_frontScreen, 640, 0, 0, 640, _vm->_cameraHeight);
	} else {
		updateDirtyRects();
	}

	SWAP(_currQueue, _prevQueue);
	_currQueue->clear();

}

void RenderQueue::clear() {
	_prevQueue->clear();
	_currQueue->clear();
}

bool RenderQueue::rectIntersectsItem(const Common::Rect &rect) {
	for (RenderQueueArray::iterator iter = _currQueue->begin(); iter != _currQueue->end(); iter++) {
		const RenderQueueItem *item = &(*iter);
		if (rect.intersects(item->rect))
			return true;
	}
	return false;
}

RenderQueueItem *RenderQueue::findItemInQueue(RenderQueueArray *queue, const RenderQueueItem &item) {
	/* This checks if the given item also exists in the previously drawn frame.
	   The state of the item (position, color etc) is handled elsewhere.
	*/
	for (RenderQueueArray::iterator iter = queue->begin(); iter != queue->end(); iter++) {
		RenderQueueItem *prevItem = &(*iter);
		if (prevItem->type == item.type) {
			switch (item.type) {
			case kSprite:
				if (prevItem->sprite.resIndex == item.sprite.resIndex &&
					prevItem->sprite.frameNum == item.sprite.frameNum)
					return prevItem;
				break;
			case kText:
				if (prevItem->text.text == item.text.text &&
					prevItem->text.len == item.text.len)
					return prevItem;
				break;
			case kMask:
				if (prevItem->mask.surface == item.mask.surface)
					return prevItem;
				break;
			}
		}
	}
	return NULL; // Not found
}

bool RenderQueue::hasItemChanged(const RenderQueueItem &item1, const RenderQueueItem &item2) {

	if (item1.type != item2.type)
		return true;

	if (item1.rect.left != item2.rect.left ||
		item1.rect.top != item2.rect.top ||
		item1.rect.right != item2.rect.right ||
		item1.rect.bottom != item2.rect.bottom)
		return true;

	if (item1.type == kText && item1.text.color != item2.text.color)
		return true;

	return false;
}

void RenderQueue::invalidateItemsByRect(const Common::Rect &rect, const RenderQueueItem *item) {
	for (RenderQueueArray::iterator iter = _currQueue->begin(); iter != _currQueue->end(); iter++) {
		RenderQueueItem *subItem = &(*iter);
		if (item != subItem &&
			subItem->flags == kUnchanged &&
			rect.intersects(subItem->rect)) {

			subItem->flags = kRefresh;
			invalidateItemsByRect(subItem->rect, subItem);
		}
	}
}

void RenderQueue::addDirtyRect(const Common::Rect &rect) {
	_updateUta->addRect(rect);
}

void RenderQueue::restoreDirtyBackground() {
	int n_rects = 0;
	Common::Rect *rects = _updateUta->getRectangles(&n_rects, 0, 0, 639, _vm->_cameraHeight - 1);
	for (int i = 0; i < n_rects; i++) {
		byte *destp = _vm->_screen->_frontScreen + rects[i].left + rects[i].top * 640;
		byte *srcp = _vm->_screen->_backScreen + (_vm->_cameraX + rects[i].left) + (_vm->_cameraY + rects[i].top) * _vm->_sceneWidth;
		int16 w = rects[i].width();
		int16 h = rects[i].height();
		while (h--) {
			memcpy(destp, srcp, w);
			destp += 640;
			srcp += _vm->_sceneWidth;
		}
		invalidateItemsByRect(rects[i], NULL);
	}
	delete[] rects;
}

void RenderQueue::updateDirtyRects() {
	int n_rects = 0;
	Common::Rect *rects = _updateUta->getRectangles(&n_rects, 0, 0, 639, _vm->_cameraHeight - 1);
	for (int i = 0; i < n_rects; i++) {
		_vm->_system->copyRectToScreen(_vm->_screen->_frontScreen + rects[i].left + rects[i].top * 640,
			640, rects[i].left, rects[i].top, rects[i].width(), rects[i].height());
	}
	delete[] rects;
}


} // End of namespace Toltecs
