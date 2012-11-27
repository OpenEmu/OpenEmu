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

#ifndef QUEEN_GRID_H
#define QUEEN_GRID_H

#include "common/util.h"
#include "queen/structs.h"

namespace Queen {

enum GridScreen {
	GS_ROOM  = 0,
	GS_PANEL = 1,
	GS_COUNT = 2
};

class QueenEngine;

class Grid {
public:

	Grid(QueenEngine *vm);
	~Grid();

	//! read areas data from specified stream
	void readDataFrom(uint16 numObjects, uint16 numRooms, byte *&ptr);

	//! defines a new zone
	void setZone(GridScreen screen, uint16 zoneNum, uint16 x1, uint16 y1, uint16 x2, uint16 y2);

	//! defines a new zone
	void setZone(GridScreen screen, uint16 zoneNum, const Box &box);

	//! find the zone number containing the specified point
	uint16 findZoneForPos(GridScreen screen, uint16 x, uint16 y) const;

	//! find the area number containing the specified point
	uint16 findAreaForPos(GridScreen screen, uint16 x, uint16 y) const;

	//! clear the zones for current room
	void clear(GridScreen screen);

	//! setup objects zones for the specified room
	void setupNewRoom(uint16 room, uint16 firstRoomObjNum);

	//! setup panel zones
	void setupPanel();

	//! draw the zones for current room (debug only)
	void drawZones();

	//! retuns a reference to the specified zone
	const Box *zone(GridScreen screen, uint16 index) const;

	//! get the verb for the specified cursor position
	Verb findVerbUnderCursor(int16 cursorx, int16 cursory) const;

	//! get the object for the specified cursor position
	uint16 findObjectUnderCursor(int16 cursorx, int16 cursory) const;

	//! get the object for the specified zone number
	uint16 findObjectNumber(uint16 zoneNum) const;

	//! get scale for the specified position
	uint16 findScale(uint16 x, uint16 y) const;

	//! returns a reference to the specfied room area
	Area *area(int room, int num) const { return &_area[room][num]; }

	//! returns the number of areas in this room
	uint16 areaMax(int room) const { return _areaMax[room]; }

	//! returns the number of objects in this room
	uint16 objMax(int room) const { return _objMax[room]; }

	void saveState(byte *&ptr);
	void loadState(uint32 ver, byte *&ptr);

	enum {
		MAX_ZONES_NUMBER = 32,
		MAX_AREAS_NUMBER = 11
	};


private:

	struct ZoneSlot {
		bool valid;
		Box box;
	};

	//! current room zones
	ZoneSlot _zones[GS_COUNT][MAX_ZONES_NUMBER];

	//! number of objects for each room
	int16 *_objMax;

	//! number of areas for each room
	int16 *_areaMax;

	//! areas for each room
	Area (*_area)[MAX_AREAS_NUMBER];

	//! total number of room areas
	uint16 _numRoomAreas;

	//! box/zone for each objects
	Box *_objectBox;

	QueenEngine *_vm;
};


} // End of namespace Queen

#endif
