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

#include "common/memstream.h"

#include "gob/gob.h"
#include "gob/map.h"
#include "gob/dataio.h"
#include "gob/goblin.h"
#include "gob/mult.h"
#include "gob/sound/sound.h"

namespace Gob {

Map_v1::Map_v1(GobEngine *vm) : Map(vm) {
}

Map_v1::~Map_v1() {
}

void Map_v1::init() {
	if (_passMap || _itemsMap)
		return;

	_passWidth = 26;
	_mapWidth  = 26;
	_mapHeight = 28;

	_passMap = new int8[_mapHeight * _mapWidth];
	memset(_passMap, 0, _mapHeight * _mapWidth * sizeof(int8));

	_itemsMap = new int16*[_mapHeight];
	 for (int i = 0; i < _mapHeight; i++) {
		_itemsMap[i] = new int16[_mapWidth];
		memset(_itemsMap[i], 0, _mapWidth * sizeof(int16));
	}

	_wayPointCount = 40;
	_wayPoints = new WayPoint[40];
	memset(_wayPoints, 0, sizeof(WayPoint));
}

void Map_v1::loadMapObjects(const char *avjFile) {
	char avoName[128];
	byte *dataBuf;
	int16 tmp;
	int32 flag;
	int16 gobDataCount;
	int16 objDataCount;
	uint32 gobsPos;
	uint32 objsPos;

	strcpy(avoName, _sourceFile);
	strcat(avoName, ".avo");

	int32 size;
	dataBuf = _vm->_dataIO->getFile(avoName, size);
	if (!dataBuf) {
		dataBuf = _vm->_dataIO->getFile(avjFile, size);
		_loadFromAvo = false;
	} else
		_loadFromAvo = true;

	Common::MemoryReadStream mapData(dataBuf, 4294967295U);

	init();

	if (_loadFromAvo) {
		mapData.read(_passMap, _mapHeight * _mapWidth);

		for (int y = 0; y < _mapHeight; y++)
			for (int x = 0; x < _mapWidth; x++)
				_itemsMap[y][x] = mapData.readSByte();

		for (int i = 0; i < 40; i++) {
			_wayPoints[i].x = mapData.readUint16LE();
			_wayPoints[i].y = mapData.readUint16LE();
		}

		for (int i = 0; i < 20; i++) {
			_itemPoses[i].x      = mapData.readByte();
			_itemPoses[i].y      = mapData.readByte();
			_itemPoses[i].orient = mapData.readByte();
		}
	}

	mapData.skip(32 + 76 + 4 + 20);

	for (int i = 0; i < 3; i++) {
		tmp = mapData.readUint16LE();
		mapData.skip(tmp * 14);
	}

	loadSounds(mapData);

	mapData.skip(4 + 24);

	gobDataCount = mapData.readUint16LE();
	objDataCount = mapData.readUint16LE();

	gobsPos = mapData.pos();
	Common::MemoryReadStream gobsData(dataBuf + gobsPos, 4294967295U);
	mapData.skip(gobDataCount * 8);

	objsPos = mapData.pos();
	Common::MemoryReadStream objsData(dataBuf + objsPos, 4294967295U);
	mapData.skip(objDataCount * 8);

	loadGoblins(mapData, gobsPos);
	loadObjects(mapData, objsPos);

	tmp = mapData.readUint16LE();
	for (int i = 0; i < tmp; i++) {
		mapData.skip(30);

		flag = mapData.readSint32LE();
		mapData.skip(56);

		if (flag != 0)
			mapData.skip(30);
	}

	mapData.skip(50);
	loadItemToObject(mapData);

	delete[] dataBuf;
}

void Map_v1::loadSounds(Common::SeekableReadStream &data) {
	int16 count;
	char buf[19];
	char sndNames[20][14];

	count = data.readUint16LE();

	for (int i = 0; i < count; i++) {
		data.read(buf, 14);
		buf[14] = 0;
		strcat(buf, ".SND");
		strcpy(sndNames[i], buf);
	}

	_vm->_sound->sampleLoad(&_vm->_goblin->_soundData[14], SOUND_SND, "diamant1.snd");

	for (int i = 0; i < count; i++) {
		if (!_vm->_dataIO->hasFile(sndNames[i]))
			continue;

		_vm->_sound->sampleLoad(&_vm->_goblin->_soundData[i], SOUND_SND, sndNames[i]);
	}
}

void Map_v1::loadGoblins(Common::SeekableReadStream &data, uint32 gobsPos) {
	Goblin::Gob_State *pState;
	uint32 tmpStateData[40 * 6];
	uint32 tmpPos;

	_vm->_goblin->_gobsCount = data.readUint16LE();
	for (int i = 0; i < _vm->_goblin->_gobsCount; i++) {
		int linesCount = (i == 3) ? 70 : 40;

		_vm->_goblin->_goblins[i] = new Goblin::Gob_Object;
		memset(_vm->_goblin->_goblins[i], 0, sizeof(Goblin::Gob_Object));

		tmpPos = data.pos();
		data.seek(gobsPos);
		_vm->_goblin->_goblins[i]->xPos = data.readUint16LE();
		_vm->_goblin->_goblins[i]->yPos = data.readUint16LE();
		_vm->_goblin->_goblins[i]->order = data.readUint16LE();
		_vm->_goblin->_goblins[i]->state = data.readUint16LE();
		gobsPos = data.pos();
		data.seek(tmpPos);

		_vm->_goblin->_goblins[i]->stateMach =
				new Goblin::Gob_StateLine[linesCount];
		for (int state = 0; state < linesCount; ++state)
			for (int col = 0; col < 6; ++col)
				_vm->_goblin->_goblins[i]->stateMach[state][col] = 0;

		for (int state = 0; state < 40; ++state)
			for (int col = 0; col < 6; ++col)
				tmpStateData[state * 6 + col] = data.readUint32LE();

		data.skip(160);
		_vm->_goblin->_goblins[i]->multObjIndex = data.readByte();
		data.skip(1);

		_vm->_goblin->_goblins[i]->realStateMach =
			_vm->_goblin->_goblins[i]->stateMach;
		for (int state = 0; state < 40; state++) {
			for (int col = 0; col < 6; col++) {
				if (tmpStateData[state * 6 + col] == 0) {
					_vm->_goblin->_goblins[i]->stateMach[state][col] = 0;
					continue;
				}

				Goblin::Gob_State *tmpState = new Goblin::Gob_State;
				memset(tmpState, 0, sizeof(Goblin::Gob_State));

				_vm->_goblin->_goblins[i]->stateMach[state][col] = tmpState;

				tmpState->animation = data.readUint16LE();
				tmpState->layer = data.readUint16LE();
				data.skip(8);
				tmpState->unk0 = data.readUint16LE();
				tmpState->unk1 = data.readUint16LE();

				data.skip(2);
				if (data.readUint32LE() == 0) {
					data.skip(2);
					tmpState->sndItem = -1;
				} else
					tmpState->sndItem = data.readUint16LE();

				tmpState->freq = data.readUint16LE();
				tmpState->repCount = data.readUint16LE();
				tmpState->sndFrame = data.readUint16LE();
			}
		}
	}

	pState = new Goblin::Gob_State;
	memset(pState, 0, sizeof(Goblin::Gob_State));
	_vm->_goblin->_goblins[0]->stateMach[39][0] = pState;
	pState->layer = 98;
	pState->sndItem = -1;

	pState = new Goblin::Gob_State;
	memset(pState, 0, sizeof(Goblin::Gob_State));
	_vm->_goblin->_goblins[1]->stateMach[39][0] = pState;
	pState->layer = 99;
	pState->sndItem = -1;

	pState = new Goblin::Gob_State;
	memset(pState, 0, sizeof(Goblin::Gob_State));
	_vm->_goblin->_goblins[2]->stateMach[39][0] = pState;
	pState->layer = 100;
	pState->sndItem = -1;

	_vm->_goblin->_goblins[2]->stateMach[10][0]->sndFrame = 13;
	_vm->_goblin->_goblins[2]->stateMach[11][0]->sndFrame = 13;
	_vm->_goblin->_goblins[2]->stateMach[28][0]->sndFrame = 13;
	_vm->_goblin->_goblins[2]->stateMach[29][0]->sndFrame = 13;

	_vm->_goblin->_goblins[1]->stateMach[10][0]->sndFrame = 13;
	_vm->_goblin->_goblins[1]->stateMach[11][0]->sndFrame = 13;

	for (int state = 40; state < 70; state++) {
		pState = new Goblin::Gob_State;
		memset(pState, 0, sizeof(Goblin::Gob_State));
		_vm->_goblin->_goblins[3]->stateMach[state][0] = pState;
		_vm->_goblin->_goblins[3]->stateMach[state][1] = 0;

		pState->animation = 9;
		pState->layer = state - 40;
		pState->sndItem = -1;
	}
}

void Map_v1::loadObjects(Common::SeekableReadStream &data, uint32 objsPos) {
	Goblin::Gob_State *pState;
	uint32 tmpStateData[40 * 6];
	uint32 tmpPos;

	_vm->_goblin->_objCount = data.readUint16LE();
	for (int i = 0; i < _vm->_goblin->_objCount; i++) {
		_vm->_goblin->_objects[i] = new Goblin::Gob_Object;
		memset(_vm->_goblin->_objects[i], 0, sizeof(Goblin::Gob_Object));

		tmpPos = data.pos();
		data.seek(objsPos);
		_vm->_goblin->_objects[i]->xPos = data.readUint16LE();
		_vm->_goblin->_objects[i]->yPos = data.readUint16LE();
		_vm->_goblin->_objects[i]->order = data.readUint16LE();
		_vm->_goblin->_objects[i]->state = data.readUint16LE();
		objsPos = data.pos();
		data.seek(tmpPos);

		_vm->_goblin->_objects[i]->stateMach = new Goblin::Gob_StateLine[40];
		for (int state = 0; state < 40; ++state) {
			for (int col = 0; col < 6; ++col) {
				_vm->_goblin->_objects[i]->stateMach[state][col] = 0;
				tmpStateData[state * 6 + col] = data.readUint32LE();
			}
		}

		data.skip(160);
		_vm->_goblin->_objects[i]->multObjIndex = data.readByte();
		data.skip(1);

		_vm->_goblin->_objects[i]->realStateMach =
			_vm->_goblin->_objects[i]->stateMach;
		for (int state = 0; state < 40; state++) {
			for (int col = 0; col < 6; col++) {
				if (tmpStateData[state * 6 + col] == 0) {
					_vm->_goblin->_objects[i]->stateMach[state][col] = 0;
					continue;
				}

				Goblin::Gob_State *tmpState = new Goblin::Gob_State;
				memset(tmpState, 0, sizeof(Goblin::Gob_State));
				_vm->_goblin->_objects[i]->stateMach[state][col] = tmpState;

				tmpState->animation = data.readUint16LE();
				tmpState->layer = data.readUint16LE();
				data.skip(8);
				tmpState->unk0 = data.readUint16LE();
				tmpState->unk1 = data.readUint16LE();

				data.skip(2);
				if (data.readUint32LE() == 0) {
					data.skip(2);
					tmpState->sndItem = -1;
				} else
					tmpState->sndItem = data.readUint16LE();

				tmpState->freq = data.readUint16LE();
				tmpState->repCount = data.readUint16LE();
				tmpState->sndFrame = data.readUint16LE();
			}
		}
	}

	_vm->_goblin->_objects[10] = new Goblin::Gob_Object;
	memset(_vm->_goblin->_objects[10], 0, sizeof(Goblin::Gob_Object));

	_vm->_goblin->_objects[10]->stateMach = new Goblin::Gob_StateLine[40];
	for (int state = 0; state < 40; ++state)
		for (int col = 0; col < 6; ++col)
			_vm->_goblin->_objects[10]->stateMach[state][col] = 0;

	pState = new Goblin::Gob_State;
	memset(pState, 0, sizeof(Goblin::Gob_State));
	_vm->_goblin->_objects[10]->stateMach[0][0] = pState;

	pState->animation = 9;
	pState->layer = 27;
	pState->sndItem = -1;

	_vm->_goblin->placeObject(_vm->_goblin->_objects[10], 1, 0, 0, 0, 0);

	_vm->_goblin->_objects[10]->realStateMach =
		_vm->_goblin->_objects[10]->stateMach;
	_vm->_goblin->_objects[10]->type = 1;
	_vm->_goblin->_objects[10]->unk14 = 1;
}

void Map_v1::loadItemToObject(Common::SeekableReadStream &data) {
	int16 count;

	if (data.readUint16LE() == 0)
		return;

	data.skip(1456);
	count = data.readUint16LE();
	for (int i = 0; i < count; i++) {
		data.skip(20);
		_vm->_goblin->_itemToObject[i] = data.readUint16LE();
		data.skip(5);
	}
}

void Map_v1::optimizePoints(Mult::Mult_Object *obj, int16 x, int16 y) {
	if (_nearestWayPoint < _nearestDest) {
		for (int i = _nearestWayPoint; i <= _nearestDest; i++) {
			if (checkDirectPath(0, _curGoblinX, _curGoblinY,
				_wayPoints[i].x, _wayPoints[i].y) == 1)
				_nearestWayPoint = i;
		}
	} else if (_nearestWayPoint > _nearestDest) {
		for (int i = _nearestWayPoint; i >= _nearestDest; i--) {
			if (checkDirectPath(0, _curGoblinX, _curGoblinY,
				_wayPoints[i].x, _wayPoints[i].y) == 1)
				_nearestWayPoint = i;
		}
	}
}

void Map_v1::findNearestToGob(Mult::Mult_Object *obj) {
	int16 wayPoint = findNearestWayPoint(_curGoblinX, _curGoblinY);

	if (wayPoint != -1)
		_nearestWayPoint = wayPoint;
}

void Map_v1::findNearestToDest(Mult::Mult_Object *obj) {
	int16 wayPoint = findNearestWayPoint(_destX, _destY);

	if (wayPoint != -1)
		_nearestDest = wayPoint;
}

} // End of namespace Gob
