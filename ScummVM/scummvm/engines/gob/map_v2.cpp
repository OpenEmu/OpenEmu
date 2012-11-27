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

#include "common/stream.h"

#include "gob/gob.h"
#include "gob/map.h"
#include "gob/global.h"
#include "gob/goblin.h"
#include "gob/inter.h"
#include "gob/game.h"
#include "gob/script.h"
#include "gob/resources.h"
#include "gob/mult.h"

namespace Gob {

Map_v2::Map_v2(GobEngine *vm) : Map_v1(vm) {
	_screenHeight = 200;
}

Map_v2::~Map_v2() {
	_passMap = 0;
}

void Map_v2::loadMapObjects(const char *avjFile) {
	uint8 wayPointsCount;
	uint16 var;
	int16 id;
	int16 mapWidth, mapHeight;
	int16 tmp;
	byte *variables;
	uint32 tmpPos;
	uint32 passPos;

	var = _vm->_game->_script->readVarIndex();
	variables = _vm->_inter->_variables->getAddressOff8(var);

	id = _vm->_game->_script->readInt16();

	if (((uint16) id) >= 65520) {
		switch ((uint16) id) {
			case 65530:
				for (int i = 0; i < _mapWidth * _mapHeight; i++)
					_passMap[i] -= READ_VARO_UINT8(var + i);
				break;
			case 65531:
				for (int i = 0; i < _mapWidth * _mapHeight; i++)
					_passMap[i] += READ_VARO_UINT8(var + i);
				break;
			case 65532:
				for (int i = 0; i < _mapWidth * _mapHeight; i++)
					WRITE_VARO_UINT8(var + i, 0x00);
				break;
			case 65533: {
				int index = READ_VARO_UINT16(var);
				// _vm->_mult->_objects[index].field_6E = 0;
				// _vm->_mult->_objects[index].field_6A = variables;
				warning("Map_v2::loadMapObjects(): ID == 65533 (%d)", index);
				break;
			}
			case 65534:
				_tilesWidth     = READ_VARO_UINT8(var);
				_tilesHeight    = READ_VARO_UINT8(var + 1);
				_mapWidth       = READ_VARO_UINT8(var + 2);
				_mapHeight      = READ_VARO_UINT8(var + 3);
				_mapUnknownBool = READ_VARO_UINT8(var + 4) ? true : false;
				if (_mapUnknownBool)
					warning("Map_v2::loadMapObjects(): _mapUnknownBool == true");
				break;
			case 65535:
				_passMap = (int8 *)_vm->_inter->_variables->getAddressOff8(var);
				break;
			default:
				warning("Map_v2::loadMapObjects(): ID == %d", (uint16) id);
				break;
		}
		return;
	}

	Resource *resource = _vm->_game->_resources->getResource(id);
	if (!resource)
		return;

	Common::SeekableReadStream &mapData = *resource->stream();

	_mapVersion = mapData.readByte();
	if (_mapVersion == 4) {
		_screenWidth = 640;
		_screenHeight = 400;
	} else if (_mapVersion == 3) {
		_passWidth = 65;
		_screenWidth = 640;
		_screenHeight = 200;
	} else {
		_passWidth = 40;
		_screenWidth = 320;
		_screenHeight = 200;
	}

	_wayPointCount = mapData.readByte();
	_tilesWidth = mapData.readSint16LE();
	_tilesHeight = mapData.readSint16LE();

	_bigTiles = !(_tilesHeight & 0xFF00);
	_tilesHeight &= 0xFF;

	if (_mapVersion == 4) {
		_screenWidth = mapData.readSint16LE();
		_screenHeight = mapData.readSint16LE();
	}

	_mapWidth = _screenWidth / _tilesWidth;
	_mapHeight = _screenHeight / _tilesHeight;

	passPos = mapData.pos();
	mapData.skip(_mapWidth * _mapHeight);

	if (resource->getData()[0] == 1)
		wayPointsCount = _wayPointCount = 40;
	else
		wayPointsCount = _wayPointCount == 0 ? 1 : _wayPointCount;

	delete[] _wayPoints;
	_wayPoints = new WayPoint[wayPointsCount];
	for (int i = 0; i < _wayPointCount; i++) {
		_wayPoints[i].x = mapData.readSByte();
		_wayPoints[i].y = mapData.readSByte();
		_wayPoints[i].notWalkable = mapData.readSByte();
	}

	if (_mapVersion == 4) {
		_mapWidth  = VAR(17);
		_passWidth = _mapWidth;
	}

	// In the original asm, this writes byte-wise into the variables-array
	tmpPos = mapData.pos();
	mapData.seek(passPos);
	if ((variables != 0) &&
	    (variables != _vm->_inter->_variables->getAddressOff8(0))) {

		_passMap = (int8 *)variables;
		mapHeight = _screenHeight / _tilesHeight;
		mapWidth = _screenWidth / _tilesWidth;

		for (int i = 0; i < mapHeight; i++) {
			for (int j = 0; j < mapWidth; j++)
				setPass(j, i, mapData.readSByte());
			_vm->_inter->_variables->getAddressOff8(var + i * _passWidth);
		}
	}
	mapData.seek(tmpPos);

	tmp = mapData.readSint16LE();
	mapData.skip(tmp * 14);
	tmp = mapData.readSint16LE();
	mapData.skip(tmp * 14 + 28);
	tmp = mapData.readSint16LE();
	mapData.skip(tmp * 14);

	_vm->_goblin->_gobsCount = tmp;
	for (int i = 0; i < _vm->_goblin->_gobsCount; i++)
		loadGoblinStates(mapData, i);

	_vm->_goblin->_soundSlotsCount = _vm->_game->_script->readInt16();
	for (int i = 0; i < _vm->_goblin->_soundSlotsCount; i++)
		_vm->_goblin->_soundSlots[i] = _vm->_inter->loadSound(1);

	delete resource;
}

void Map_v2::loadGoblinStates(Common::SeekableReadStream &data, int index) {
	Mult::Mult_GobState *statesPtr;
	Mult::Mult_GobState *gobState;
	int8 indices[102];
	uint8 statesCount;
	uint8 dataCount;
	int16 state;
	uint32 tmpPos;

	memset(indices, -1, 101);
	_vm->_mult->_objects[index].goblinStates = new Mult::Mult_GobState*[101];
	memset(_vm->_mult->_objects[index].goblinStates, 0,
			101 * sizeof(Mult::Mult_GobState *));

	data.read(indices, 100);
	tmpPos = data.pos();
	statesCount = 0;
	for (int i = 0; i < 100; i++) {
		if (indices[i] != -1) {
			statesCount++;
			data.skip(4);
			dataCount = data.readByte();
			statesCount += dataCount;
			data.skip(dataCount * 9);
		}
	}

	data.seek(tmpPos);

	statesPtr = new Mult::Mult_GobState[statesCount];
	_vm->_mult->_objects[index].goblinStates[0] = statesPtr;
	for (int i = 0; i < 100; i++) {
		state = indices[i];
		if (state != -1) {
			_vm->_mult->_objects[index].goblinStates[state] = statesPtr++;
			gobState = _vm->_mult->_objects[index].goblinStates[state];

			gobState[0].animation = data.readSint16LE();
			gobState[0].layer = data.readSint16LE();
			dataCount = data.readByte();
			gobState[0].dataCount = dataCount;
			for (uint8 j = 1; j <= dataCount; j++) {
				data.skip(1);
				gobState[j].sndItem = data.readSByte();
				data.skip(1);
				gobState[j].sndFrame = data.readByte();
				data.skip(1);
				gobState[j].freq = data.readSint16LE();
				gobState[j].repCount = data.readSByte();
				gobState[j].speaker = data.readByte();
				statesPtr++;
			}
		}
	}
}

void Map_v2::findNearestToGob(Mult::Mult_Object *obj) {
	int16 wayPoint = findNearestWayPoint(obj->goblinX, obj->goblinY);

	if (wayPoint != -1)
		obj->nearestWayPoint = wayPoint;
}

void Map_v2::findNearestToDest(Mult::Mult_Object *obj) {
	int16 wayPoint = findNearestWayPoint(obj->destX, obj->destY);

	if (wayPoint != -1)
		obj->nearestDest = wayPoint;
}

void Map_v2::optimizePoints(Mult::Mult_Object *obj, int16 x, int16 y) {
	if (!_wayPoints)
		return;

	if (obj->nearestWayPoint < obj->nearestDest) {

		for (int i = obj->nearestWayPoint; i <= obj->nearestDest; i++) {
			if (checkDirectPath(obj, x, y, _wayPoints[i].x, _wayPoints[i].y) == 1)
				obj->nearestWayPoint = i;
		}

	} else {

		for (int i = obj->nearestWayPoint; i >= obj->nearestDest; i--) {
			if (_wayPoints[i].notWalkable == 1)
				break;

			if (checkDirectPath(obj, x, y, _wayPoints[i].x, _wayPoints[i].y) == 1)
				obj->nearestWayPoint = i;
		}

	}

}

} // End of namespace Gob
