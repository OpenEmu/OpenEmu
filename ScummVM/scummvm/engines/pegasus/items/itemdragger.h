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

#ifndef PEGASUS_ITEMS_ITEMDRAGGER_H
#define PEGASUS_ITEMS_ITEMDRAGGER_H

#include "pegasus/elements.h"
#include "pegasus/input.h"

namespace Pegasus {

// TODO: Merge SpriteDragger and ItemDragger

class Hotspot;
class Sprite;

class SpriteDragger : public Tracker {
public:
	SpriteDragger();
	virtual ~SpriteDragger() {}

	void setDragSprite(Sprite *);
	Sprite *getDragSprite() { return _draggingSprite; }

	void setDragConstraints(const Common::Rect &, const Common::Rect &);
	void getDragConstraints(Common::Rect &, Common::Rect &) const;

	void startTracking(const Input &);
	void continueTracking(const Input&);

	Hotspot *getLastHotspot() const { return _lastHotspot; }

protected:
	virtual void enterHotspot(Hotspot *) {}
	virtual void exitHotspot(Hotspot *) {}

	Sprite *_draggingSprite;
	Common::Point _startPoint, _lastPoint, _dragOffset;
	Common::Point _startRawPoint, _lastRawPoint;
	Common::Rect _rawLimitRect;
	Common::Rect _limitRect;
	Common::Rect _slopRect;
	Hotspot *_lastHotspot;

	// This is a replica of QuickDraw's PinPointInRect function
	void pinPointInRect(const Common::Rect &, Common::Point &);
};

class PegasusEngine;

class ItemDragger : public SpriteDragger {
public:
	ItemDragger(PegasusEngine *);
	virtual ~ItemDragger() {}

	void setHighlightBounds();
	void startTracking(const Input &);
	void stopTracking(const Input &);
	bool stopTrackingInput(const Input &);

protected:
	virtual void enterHotspot(Hotspot *);
	virtual void exitHotspot(Hotspot *);

	PegasusEngine *_owner;
	DropHighlight _inventoryHighlight;
	Hotspot _inventoryDropSpot;
	DropHighlight _biochipHighlight;
	Hotspot _biochipDropSpot;
};

} // End of namespace Pegasus

#endif
