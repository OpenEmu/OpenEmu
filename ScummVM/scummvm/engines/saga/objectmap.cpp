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

// Object map / Object click-area module

// Polygon Hit Test code ( HitTestPoly() ) adapted from code (C) Eric Haines
// appearing in Graphics Gems IV, "Point in Polygon Strategies."
// p. 24-46, code: p. 34-45

#include "saga/saga.h"

#include "saga/gfx.h"
#include "saga/console.h"
#include "saga/font.h"
#include "saga/interface.h"
#include "saga/objectmap.h"
#include "saga/actor.h"
#include "saga/scene.h"
#include "saga/isomap.h"
#ifdef SAGA_DEBUG
#include "saga/render.h"
#endif

namespace Saga {

void HitZone::load(SagaEngine *vm, Common::MemoryReadStreamEndian *readStream, int index, int sceneNumber) {
	_index = index;
	_flags = readStream->readByte();
	_clickAreas.resize(readStream->readByte());
	_rightButtonVerb = readStream->readByte();
	readStream->readByte(); // pad
	_nameIndex = readStream->readUint16();
	_scriptNumber = readStream->readUint16();

	for (ClickAreas::iterator i = _clickAreas.begin(); i != _clickAreas.end(); ++i) {
		i->resize(readStream->readUint16LE());

		assert(!i->empty());

		for (ClickArea::iterator j = i->begin(); j != i->end(); ++j) {
			j->x = readStream->readSint16();
			j->y = readStream->readSint16();

			// WORKAROUND: bug #1259608: "ITE: Riff ignores command in Ferret merchant center"
			// Apparently ITE Mac version has bug in game data. Both ObjectMap and ActionMap
			// for exit area are little taller (y = 123) and thus Riff goes to exit
			// when clicked on barrel of nails.
			if (vm->getGameId() == GID_ITE) {
				if (sceneNumber == 18 && index == 0 && (i == _clickAreas.begin()) && (j == i->begin()) && j->y == 123) {
					j->y = 129;
				}
			}
		}
	}
}

bool HitZone::getSpecialPoint(Point &specialPoint) const {
	for (ClickAreas::const_iterator i = _clickAreas.begin(); i != _clickAreas.end(); ++i) {
		if (i->size() == 1) {
			specialPoint = (*i)[0];
			return true;
		}
	}
	return false;
}

bool HitZone::hitTest(const Point &testPoint) {
	const Point *points;
	uint pointsCount;

	if (_flags & kHitZoneEnabled) {
		for (ClickAreas::const_iterator i = _clickAreas.begin(); i != _clickAreas.end(); ++i) {
			pointsCount = i->size();
			if (pointsCount < 2) {
				continue;
			}
			points = &i->front();
			if (pointsCount == 2) {
				// Hit-test a box region
				if ((testPoint.x >= points[0].x) &&
					(testPoint.x <= points[1].x) &&
					(testPoint.y >= points[0].y) &&
					(testPoint.y <= points[1].y)) {
						return true;
					}
			} else {
				// Hit-test a polygon
				if (hitTestPoly(points, pointsCount, testPoint)) {
					return true;
				}
			}
		}
	}
	return false;
}

#ifdef SAGA_DEBUG
void HitZone::draw(SagaEngine *vm, int color) {
	int pointsCount, j;
	Location location;
	HitZone::ClickArea tmpPoints;
	const Point *points;
	Point specialPoint1;
	Point specialPoint2;

	for (ClickAreas::const_iterator i = _clickAreas.begin(); i != _clickAreas.end(); ++i) {
		pointsCount = i->size();
		points = &i->front();
		if (vm->_scene->getFlags() & kSceneFlagISO) {
			tmpPoints.resize(pointsCount);
			for (j = 0; j < pointsCount; j++) {
				location.u() = points[j].x;
				location.v() = points[j].y;
				location.z = 0;
				vm->_isoMap->tileCoordsToScreenPoint(location, tmpPoints[j]);
			}
			points = &tmpPoints.front();
		}

		if (pointsCount == 2) {
			// 2 points represent a box
			vm->_gfx->drawFrame(points[0], points[1], color);
		} else {
			if (pointsCount > 2) {
				// Otherwise draw a polyline
				// Do a full refresh so that the polyline can be shown
				vm->_render->setFullRefresh(true);
				vm->_gfx->drawPolyLine(points, pointsCount, color);
			}
		}
	}
	if (getSpecialPoint(specialPoint1)) {
		specialPoint2 = specialPoint1;
		specialPoint1.x--;
		specialPoint1.y--;
		specialPoint2.x++;
		specialPoint2.y++;
		vm->_gfx->drawFrame(specialPoint1, specialPoint2, color);
	}
}
#endif

// Loads an object map resource ( objects ( clickareas ( points ) ) )
void ObjectMap::load(const ByteArray &resourceData) {

	if (!_hitZoneList.empty()) {
		error("ObjectMap::load _hitZoneList not empty");
	}

	if (resourceData.empty()) {
		return;
	}

	if (resourceData.size() < 4) {
		error("ObjectMap::load wrong resourceLength");
	}

	ByteArrayReadStreamEndian readS(resourceData, _vm->isBigEndian());

	_hitZoneList.resize(readS.readUint16());

	int idx = 0;
	for (HitZoneArray::iterator i = _hitZoneList.begin(); i != _hitZoneList.end(); ++i) {
		i->load(_vm, &readS, idx++, _vm->_scene->currentSceneNumber());
	}
}

void ObjectMap::clear() {
	_hitZoneList.clear();
}

#ifdef SAGA_DEBUG
void ObjectMap::draw(const Point& testPoint, int color, int color2) {
	int hitZoneIndex;
	Common::String txtBuf;
	Point pickPoint;
	Point textPoint;
	Location pickLocation;
	pickPoint = testPoint;
	if (_vm->_scene->getFlags() & kSceneFlagISO) {
		assert(_vm->_actor->_protagonist);
		pickPoint.y -= _vm->_actor->_protagonist->_location.z;
		_vm->_isoMap->screenPointToTileCoords(pickPoint, pickLocation);
		pickLocation.toScreenPointUV(pickPoint);
	}

	hitZoneIndex = hitTest(pickPoint);

	for (HitZoneArray::iterator i = _hitZoneList.begin(); i != _hitZoneList.end(); ++i) {
		i->draw(_vm, (hitZoneIndex == i->getIndex()) ? color2 : color);
	}

	if (hitZoneIndex != -1) {
		txtBuf = Common::String::format("hitZone %d", hitZoneIndex);
		textPoint.x = 2;
		textPoint.y = 2;
		_vm->_font->textDraw(kKnownFontSmall, txtBuf.c_str(), textPoint, kITEColorBrightWhite, kITEColorBlack, kFontOutline);
	}
}
#endif

int ObjectMap::hitTest(const Point& testPoint) {

	// Loop through all scene objects
	for (HitZoneArray::iterator i = _hitZoneList.begin(); i != _hitZoneList.end(); ++i) {
		if (i->hitTest(testPoint)) {
			return i->getIndex();
		}
	}

	return -1;
}

void ObjectMap::cmdInfo() {
	_vm->_console->DebugPrintf("%d zone(s) loaded.\n\n", _hitZoneList.size());
}

} // End of namespace Saga
