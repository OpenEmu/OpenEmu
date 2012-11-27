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


#include "queen/grid.h"

#include "queen/display.h"
#include "queen/logic.h"
#include "queen/queen.h"

#include "common/debug.h"

namespace Queen {

Grid::Grid(QueenEngine *vm)
	: _vm(vm) {
	memset(_zones, 0, sizeof(_zones));
}

Grid::~Grid() {
	delete[] _objMax;
	delete[] _areaMax;
	delete[] _area;
	delete[] _objectBox;
}

void Grid::readDataFrom(uint16 numObjects, uint16 numRooms, byte *&ptr) {
	uint16 i, j;

	_numRoomAreas = numRooms;

	_objMax  = new int16[_numRoomAreas + 1];
	_areaMax = new int16[_numRoomAreas + 1];
	_area    = new Area[_numRoomAreas + 1][MAX_AREAS_NUMBER];

	_objMax[0] = 0;
	_areaMax[0] = 0;
	memset(&_area[0], 0, sizeof(Area) * MAX_AREAS_NUMBER);
	for (i = 1; i <= _numRoomAreas; i++) {
		_objMax[i] = (int16)READ_BE_INT16(ptr); ptr += 2;
		_areaMax[i] = (int16)READ_BE_INT16(ptr); ptr += 2;
		memset(&_area[i][0], 0, sizeof(Area));
		for (j = 1; j <= _areaMax[i]; j++) {
			assert(j < MAX_AREAS_NUMBER);
			_area[i][j].readFromBE(ptr);
		}
	}

	_objectBox = new Box[numObjects + 1];
	memset(&_objectBox[0], 0, sizeof(Box));
	for (i = 1; i <= numObjects; i++) {
		_objectBox[i].readFromBE(ptr);
	}
}

void Grid::setZone(GridScreen screen, uint16 zoneNum, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	debug(9, "Grid::setZone(%d, %d, (%d,%d), (%d,%d))", screen, zoneNum, x1, y1, x2, y2);
	assert(zoneNum < MAX_ZONES_NUMBER);
	ZoneSlot *pzs = &_zones[screen][zoneNum];
	pzs->valid = true;
	pzs->box.x1 = x1;
	pzs->box.y1 = y1;
	pzs->box.x2 = x2;
	pzs->box.y2 = y2;
}

void Grid::setZone(GridScreen screen, uint16 zoneNum, const Box &box) {
	debug(9, "Grid::setZone(%d, %d, (%d,%d), (%d,%d))", screen, zoneNum, box.x1, box.y1, box.x2, box.y2);
	assert(zoneNum < MAX_ZONES_NUMBER);
	ZoneSlot *pzs = &_zones[screen][zoneNum];
	pzs->valid = true;
	pzs->box = box;
}

uint16 Grid::findZoneForPos(GridScreen screen, uint16 x, uint16 y) const {
	debug(9, "Logic::findZoneForPos(%d, (%d,%d))", screen, x, y);
	int i;
	if (screen == GS_PANEL) {
		y -= ROOM_ZONE_HEIGHT;
	}
	for (i = 1; i < MAX_ZONES_NUMBER; ++i) {
		const ZoneSlot *pzs = &_zones[screen][i];
		if (pzs->valid && pzs->box.contains(x, y)) {
			return i;
		}
	}
	return 0;
}

uint16 Grid::findAreaForPos(GridScreen screen, uint16 x, uint16 y) const {
	uint16 room = _vm->logic()->currentRoom();
	uint16 zoneNum = findZoneForPos(screen, x, y);
	if (zoneNum <= _objMax[room]) {
		zoneNum = 0;
	} else {
		zoneNum -= _objMax[room];
	}
	return zoneNum;
}

void Grid::clear(GridScreen screen) {
	debug(9, "Grid::clear(%d)", screen);
	for (int i = 1; i < MAX_ZONES_NUMBER; ++i) {
		_zones[screen][i].valid = false;
	}
}

void Grid::setupNewRoom(uint16 room, uint16 firstRoomObjNum) {
	debug(9, "Grid::setupNewRoom()");
	clear(GS_ROOM);

	uint16 i;
	uint16 zoneNum;

	// setup objects zones
	uint16 maxObjRoom = _objMax[room];
	zoneNum = 1;
	for (i = firstRoomObjNum + 1; i <= firstRoomObjNum + maxObjRoom; ++i) {
		if (_vm->logic()->objectData(i)->name != 0) {
			if (room == 41 && i == 303) {

				// WORKAROUND bug #1599009: In the room 41, the bounding box of the
				// stairs (object 303) doesn't match with the room picture. With the
				// original box dimensions, Joe could walk "above" the stairs, giving
				// the impression of floating in the air.
				// To fix this, the bounding box is set relative to the position of
				// the cabinet (object 295).

				uint16 y1 = _objectBox[295].y2 + 1;
				setZone(GS_ROOM, zoneNum, _objectBox[i].x1, y1, _objectBox[i].x2, _objectBox[i].y2);
			} else {
				setZone(GS_ROOM, zoneNum, _objectBox[i]);
			}
		}
		++zoneNum;
	}

	// setup room zones (areas)
	uint16 maxAreaRoom = _areaMax[room];
	for (zoneNum = 1; zoneNum <= maxAreaRoom; ++zoneNum) {
		setZone(GS_ROOM, maxObjRoom + zoneNum, _area[room][zoneNum].box);
	}
}

void Grid::setupPanel() {
	for (int i = 0; i <= 7; ++i) {
		uint16 x = i * 20;
		setZone(GS_PANEL, i + 1, x, 10, x + 19, 49);
	}

	// inventory scrolls
	setZone(GS_PANEL,  9, 160, 10, 179, 29);
	setZone(GS_PANEL, 10, 160, 30, 179, 49);

	// inventory items
	setZone(GS_PANEL, 11, 180, 10, 213, 49);
	setZone(GS_PANEL, 12, 214, 10, 249, 49);
	setZone(GS_PANEL, 13, 250, 10, 284, 49);
	setZone(GS_PANEL, 14, 285, 10, 320, 49);
}

void Grid::drawZones() {
	for (int i = 1; i < MAX_ZONES_NUMBER; ++i) {
		const ZoneSlot *pzs = &_zones[GS_ROOM][i];
		if (pzs->valid) {
			const Box *b = &pzs->box;
			_vm->display()->drawBox(b->x1, b->y1, b->x2, b->y2, 3);
		}
	}
}

const Box *Grid::zone(GridScreen screen, uint16 index) const {
	const ZoneSlot *zs = &_zones[screen][index];
	assert(zs->valid);
	return &zs->box;
}

Verb Grid::findVerbUnderCursor(int16 cursorx, int16 cursory) const {
	static const Verb pv[] = {
		VERB_NONE,
		VERB_OPEN,
		VERB_CLOSE,
		VERB_MOVE,
		VERB_GIVE,
		VERB_LOOK_AT,
		VERB_PICK_UP,
		VERB_TALK_TO,
		VERB_USE,
		VERB_SCROLL_UP,
		VERB_SCROLL_DOWN,
		VERB_INV_1,
		VERB_INV_2,
		VERB_INV_3,
		VERB_INV_4,
	};
	return pv[findZoneForPos(GS_PANEL, cursorx, cursory)];
}

uint16 Grid::findObjectUnderCursor(int16 cursorx, int16 cursory) const {
	uint16 roomObj = 0;
	if (cursory < ROOM_ZONE_HEIGHT) {
		int16 x = cursorx + _vm->display()->horizontalScroll();
		roomObj = findZoneForPos(GS_ROOM, x, cursory);
	}
	return roomObj;
}

uint16 Grid::findObjectNumber(uint16 zoneNum) const {
	// l.316-327 select.c
	uint16 room = _vm->logic()->currentRoom();
	uint16 obj = zoneNum;
	uint16 objectMax = _objMax[room];
	debug(9, "Grid::findObjectNumber(%X, %X)", zoneNum, objectMax);
	if (zoneNum > objectMax) {
		// this is an area box, check for associated object
		obj =  _area[room][zoneNum - objectMax].object;
		if (obj != 0) {
			// there is an object, get its number
			obj -= _vm->logic()->currentRoomData();
		}
	}
	return obj;
}

uint16 Grid::findScale(uint16 x, uint16 y) const {
	uint16 room = _vm->logic()->currentRoom();
	uint16 scale = 100;
	uint16 areaNum = findAreaForPos(GS_ROOM, x, y);
	if (areaNum != 0) {
		scale = _area[room][areaNum].calcScale(y);
	}
	return scale;
}

void Grid::saveState(byte *&ptr) {
	uint16 i, j;
	for (i = 1; i <= _numRoomAreas; ++i) {
		for (j = 1; j <= _areaMax[i]; ++j) {
			_area[i][j].writeToBE(ptr);
		}
	}
}

void Grid::loadState(uint32 ver, byte *&ptr) {
	uint16 i, j;
	for (i = 1; i <= _numRoomAreas; ++i) {
		for (j = 1; j <= _areaMax[i]; ++j) {
			_area[i][j].readFromBE(ptr);
		}
	}
}

} // End of namespace Queen
