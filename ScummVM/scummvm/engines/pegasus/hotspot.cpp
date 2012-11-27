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

#include "common/stream.h"

#include "pegasus/hotspot.h"

namespace Pegasus {

Region::Region(Common::ReadStream *stream) {
	uint16 length = stream->readUint16BE();

	assert(length >= 10);

	_bounds.top = stream->readUint16BE();
	_bounds.left = stream->readUint16BE();
	_bounds.bottom = stream->readUint16BE();
	_bounds.right = stream->readUint16BE();

	_bounds.debugPrint(0, "Bounds:");

	if (length == 10)
		return;

	length -= 10;

	while (length > 0) {
		Vector v;
		v.y = stream->readUint16BE();
		length -= 2;

		if (v.y == 0x7fff)
			break;

		debug(0, "y: %d", v.y);

		// Normalize y to _bounds
		v.y -= _bounds.top;

		while (length > 0) {
			Run run;
			run.start = stream->readUint16BE();
			length -= 2;

			if (run.start == 0x7fff)
				break;

			run.end = stream->readUint16BE();
			length -= 2;

			debug(0, "\t[%d, %d)", run.start, run.end);

			// Normalize to _bounds
			run.start -= _bounds.left;
			run.end -= _bounds.left;

			v.push_back(run);
		}

		_vectors.push_back(v);
	}
}

Region::Region(const Common::Rect &rect) {
	_bounds = rect;
}

bool Region::pointInRegion(const Common::Point &point) const {
	if (!_bounds.contains(point))
		return false;

	bool pixelActive = false;

	// Normalize the points to _bounds
	uint16 x = point.x - _bounds.left;
	uint16 y = point.y - _bounds.top;

	for (Common::List<Vector>::const_iterator v = _vectors.begin(); v != _vectors.end(); v++) {
		if (v->y > y)
			return pixelActive;

		for (Vector::const_iterator run = v->begin(); run != v->end(); run++) {
			if (x >= run->start && x < run->end) {
				pixelActive = !pixelActive;
				break;
			}
		}
	}

	// the case if the region is just a rect
	return true;
}

void Region::moveTo(CoordType h, CoordType v) {
	_bounds.moveTo(h, v);
}

void Region::moveTo(const Common::Point &point) {
	_bounds.moveTo(point);
}

void Region::translate(CoordType h, CoordType v) {
	_bounds.translate(h, v);
}

void Region::translate(const Common::Point &point) {
	_bounds.translate(point.x, point.y);
}

void Region::getCenter(CoordType &h, CoordType &v) const {
	h = (_bounds.left + _bounds.right) / 2;
	v = (_bounds.top + _bounds.bottom) / 2;
}

void Region::getCenter(Common::Point &point) const {
	getCenter(point.x, point.y);
}

Hotspot::Hotspot(const HotSpotID id) : IDObject(id) {
	_spotFlags = kNoHotSpotFlags;
	_spotActive = false;
}

Hotspot::~Hotspot() {
}

void Hotspot::setArea(const Common::Rect &area) {
	_spotArea = Region(area);
}

void Hotspot::setArea(const CoordType left, const CoordType top, const CoordType right, const CoordType bottom) {
	_spotArea = Region(Common::Rect(left, top, right, bottom));
}

void Hotspot::getBoundingBox(Common::Rect &r) const {
	r = _spotArea.getBoundingBox();
}

void Hotspot::getCenter(Common::Point &pt) const {
	_spotArea.getCenter(pt);
}

void Hotspot::getCenter(CoordType &h, CoordType &v) const {
	_spotArea.getCenter(h, v);
}

void Hotspot::setActive() {
	_spotActive = true;
}

void Hotspot::setInactive() {
	_spotActive = false;
}

void Hotspot::setHotspotFlags(const HotSpotFlags flags) {
	_spotFlags = flags;
}

void Hotspot::setMaskedHotspotFlags(const HotSpotFlags flags, const HotSpotFlags mask) {
	_spotFlags = (_spotFlags & ~mask) | flags;
}

bool Hotspot::isSpotActive() const {
	return _spotActive;
}

void Hotspot::moveSpotTo(const CoordType h, const CoordType v) {
	_spotArea.moveTo(h, v);
}

void Hotspot::moveSpotTo(const Common::Point pt) {
	_spotArea.moveTo(pt);
}

void Hotspot::moveSpot(const CoordType h, const CoordType v) {
	_spotArea.translate(h, v);
}

void Hotspot::moveSpot(const Common::Point pt) {
	_spotArea.translate(pt.x, pt.y);
}

bool Hotspot::pointInSpot(const Common::Point where) const {
	return _spotActive && _spotArea.pointInRegion(where);
}

HotSpotFlags Hotspot::getHotspotFlags() const {
	return _spotFlags;
}

HotspotList::HotspotList() {
}

HotspotList::~HotspotList() {
	// TODO: Should this call deleteHotspots()?
}

void HotspotList::deleteHotspots() {
	for (HotspotIterator it = begin(); it != end(); it++)
		delete *it;

	clear();
}

Hotspot *HotspotList::findHotspot(const Common::Point where) {
	for (HotspotIterator it = begin(); it != end(); it++)
		if ((*it)->pointInSpot(where))
			return *it;

	return 0;
}

HotSpotID HotspotList::findHotspotID(const Common::Point where) {
	Hotspot *hotspot = findHotspot(where);
	return hotspot ? hotspot->getObjectID() : kNoHotSpotID;
}

Hotspot *HotspotList::findHotspotByID(const HotSpotID id) {
	for (HotspotIterator it = begin(); it != end(); it++)
		if ((*it)->getObjectID() == id)
			return *it;

	return 0;
}

Hotspot *HotspotList::findHotspotByMask(const HotSpotFlags flags) {
	for (HotspotIterator it = begin(); it != end(); it++)
		if (((*it)->getHotspotFlags() & flags) == flags)
			return *it;

	return 0;
}

void HotspotList::activateMaskedHotspots(const HotSpotFlags flags) {
	for (HotspotIterator it = begin(); it != end(); it++)
		if (flags == kNoHotSpotFlags || ((*it)->getHotspotFlags() & flags) != 0)
			(*it)->setActive();
}

void HotspotList::deactivateAllHotspots() {
	for (HotspotIterator it = begin(); it != end(); it++)
		(*it)->setInactive();
}

void HotspotList::deactivateMaskedHotspots(const HotSpotFlags flags) {
	for (HotspotIterator it = begin(); it != end(); it++)
		if (((*it)->getHotspotFlags() & flags) != 0)
			(*it)->setInactive();
}

void HotspotList::activateOneHotspot(const HotSpotID id) {
	for (HotspotIterator it = begin(); it != end(); it++) {
		if ((*it)->getObjectID() == id) {
			(*it)->setActive();
			return;
		}
	}
}

void HotspotList::deactivateOneHotspot(const HotSpotID id) {
	for (HotspotIterator it = begin(); it != end(); it++) {
		if ((*it)->getObjectID() == id) {
			(*it)->setInactive();
			return;
		}
	}
}

void HotspotList::removeOneHotspot(const HotSpotID id) {
	for (HotspotIterator it = begin(); it != end(); it++) {
		if ((*it)->getObjectID() == id) {
			erase(it);
			return;
		}
	}
}

void HotspotList::removeMaskedHotspots(const HotSpotFlags flags) {
	if (flags != kNoHotSpotFlags) {
		for (HotspotIterator it = begin(); it != end(); ) {
			if (((*it)->getHotspotFlags() & flags) != 0)
				it = erase(it);
			else
				it++;
		}
	} else {
		clear();
	}
}

void HotspotList::setHotspotRect(const HotSpotID id, const Common::Rect &r) {
	Hotspot *hotspot = findHotspotByID(id);
	if (hotspot)
		hotspot->setArea(r);
}

void HotspotList::getHotspotRect(const HotSpotID id, Common::Rect &r) {
	Hotspot *hotspot = findHotspotByID(id);
	if (hotspot)
		hotspot->getBoundingBox(r);
}

} // End of namespace Pegasus
