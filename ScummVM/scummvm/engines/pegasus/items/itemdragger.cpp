/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#include "pegasus/elements.h"
#include "pegasus/hotspot.h"
#include "pegasus/pegasus.h"
#include "pegasus/items/itemdragger.h"

namespace Pegasus {

SpriteDragger::SpriteDragger() {
	_draggingSprite = 0;
	_limitRect = Common::Rect(-30000, -30000, 30000, 30000);
	_slopRect = Common::Rect(-30000, -30000, 30000, 30000);
	_dragOffset.x = 0;
	_dragOffset.y = 0;
	_lastHotspot = 0;
}

void SpriteDragger::setDragSprite(Sprite *newSprite) {
	if (!isTracking())
		_draggingSprite = newSprite;
}

void SpriteDragger::setDragConstraints(const Common::Rect &limitRect, const Common::Rect &slopRect) {
	if (!isTracking()) {
		_rawLimitRect = limitRect;
		_slopRect = slopRect;
	}
}

void SpriteDragger::getDragConstraints(Common::Rect &limitRect, Common::Rect &slopRect) const {
	limitRect = _rawLimitRect;
	slopRect = _slopRect;
}

void SpriteDragger::startTracking(const Input &input) {
	if (_draggingSprite) {
		Tracker::startTracking(input);

		if (isTracking()) {
			input.getInputLocation(_startPoint);
			_lastRawPoint = _startRawPoint = _startPoint;

			Common::Rect r;
			_draggingSprite->getBounds(r);
			_dragOffset.x = _startPoint.x - r.left;
			_dragOffset.y = _startPoint.y - r.top;

			_limitRect = _rawLimitRect;
			_limitRect.left += _dragOffset.x;
			_limitRect.top += _dragOffset.y;
			_limitRect.right -= r.width() - _dragOffset.x;
			_limitRect.bottom -= r.height() - _dragOffset.y;
			pinPointInRect(_limitRect, _startPoint);

			_lastPoint = _startPoint;
			if (_startPoint != _startRawPoint) {
				Common::Point pt = _startPoint - _dragOffset;
				_draggingSprite->moveElementTo(pt.x, pt.y);
			}

			_lastHotspot = g_allHotspots.findHotspot(_lastRawPoint);
			if (_lastHotspot)
				enterHotspot(_lastHotspot);
		}
	}
}

void SpriteDragger::continueTracking(const Input &input) {
	if (_draggingSprite) {
		Common::Point rawPoint;
		input.getInputLocation(rawPoint);

		if (!_slopRect.contains(rawPoint))
			rawPoint = _startRawPoint;

		if (rawPoint != _lastRawPoint) {
			Common::Point newPoint = rawPoint;
			pinPointInRect(_limitRect, newPoint);
			newPoint -= _dragOffset;

			if (newPoint != _lastPoint) {
				_draggingSprite->moveElementTo(newPoint.x, newPoint.y);
				_lastPoint = newPoint;
			}

			Hotspot *newHotspot = g_allHotspots.findHotspot(rawPoint);
			if (newHotspot != _lastHotspot) {
				if (_lastHotspot)
					exitHotspot(_lastHotspot);
				if (newHotspot)
					enterHotspot(newHotspot);
				_lastHotspot = newHotspot;
			}

			_lastRawPoint = rawPoint;
		}
	}
}

void SpriteDragger::pinPointInRect(const Common::Rect &r, Common::Point &pt) {
	pt.x = CLIP<int>(pt.x, r.left, r.right - 1);
	pt.y = CLIP<int>(pt.y, r.top, r.bottom - 1);
}

ItemDragger::ItemDragger(PegasusEngine *owner) : _inventoryDropSpot(kInventoryDropSpotID), _biochipDropSpot(kBiochipDropSpotID),
		_inventoryHighlight(kInventoryDropHighlightID), _biochipHighlight(kBiochipDropHighlightID) {
	_owner = owner;

	Common::Rect r(kInventoryDropLeft, kInventoryDropTop, kInventoryDropRight, kInventoryDropBottom);
	_inventoryDropSpot.setArea(r);
	_inventoryDropSpot.setHotspotFlags(kDropItemSpotFlag);
	g_allHotspots.push_back(&_inventoryDropSpot);

	r = Common::Rect(kBiochipDropLeft, kBiochipDropTop, kBiochipDropRight, kBiochipDropBottom);
	_biochipDropSpot.setArea(r);
	_biochipDropSpot.setHotspotFlags(kDropBiochipSpotFlag);
	g_allHotspots.push_back(&_biochipDropSpot);
}

void ItemDragger::startTracking(const Input &input) {
	_inventoryHighlight.setDisplayOrder(kInventoryHiliteOrder);
	_inventoryHighlight.startDisplaying();

	_biochipHighlight.setDisplayOrder(kBiochipHiliteOrder);
	_biochipHighlight.startDisplaying();

	SpriteDragger::startTracking(input);
}

void ItemDragger::stopTracking(const Input &input) {
	SpriteDragger::stopTracking(input);
	_inventoryHighlight.hide();
	_biochipHighlight.hide();
	_inventoryHighlight.stopDisplaying();
	_biochipHighlight.stopDisplaying();
	_owner->dragTerminated(input);
}

bool ItemDragger::stopTrackingInput(const Input &input) {
	return !JMPPPInput::isDraggingInput(input);
}

void ItemDragger::enterHotspot(Hotspot *spot) {
	if (spot->getObjectID() == kInventoryDropSpotID)
		_inventoryHighlight.show();
	else if (spot->getObjectID() == kBiochipDropSpotID)
		_biochipHighlight.show();
	else if ((spot->getHotspotFlags() & kDropItemSpotFlag) != 0)
		_draggingSprite->setCurrentFrameIndex(1);
}

void ItemDragger::exitHotspot(Hotspot *spot) {
	if (spot->getObjectID() == kInventoryDropSpotID)
		_inventoryHighlight.hide();
	else if (spot->getObjectID() == kBiochipDropSpotID)
		_biochipHighlight.hide();
	else if ((spot->getHotspotFlags() & kDropItemSpotFlag) != 0)
		_draggingSprite->setCurrentFrameIndex(0);
}

void ItemDragger::setHighlightBounds() {
	uint32 color = g_system->getScreenFormat().RGBToColor(0x48, 0x80, 0xD8);
	_inventoryHighlight.setBounds(Common::Rect(76, 334, 172, 430));
	_inventoryHighlight.setHighlightColor(color);
	_biochipHighlight.setBounds(Common::Rect(364, 334, 460, 430));
	_biochipHighlight.setHighlightColor(color);
}

} // End of namespace Pegasus
