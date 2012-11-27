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

#include "dreamweb/dreamweb.h"

namespace DreamWeb {

void DreamWebEngine::turnPathOn(uint8 param) {
	findOrMake(param, 0xff, _roomNum + 100);
	PathNode *roomsPaths = getRoomsPaths()->nodes;
	if (param == 0xff)
		return;
	roomsPaths[param].on = 0xff;
}

void DreamWebEngine::turnPathOff(uint8 param) {
	findOrMake(param, 0x00, _roomNum + 100);
	PathNode *roomsPaths = getRoomsPaths()->nodes;
	if (param == 0xff)
		return;
	roomsPaths[param].on = 0x00;
}

void DreamWebEngine::turnAnyPathOn(uint8 param, uint8 room) {
	findOrMake(param, 0xff, room + 100);
	_pathData[room].nodes[param].on = 0xff;
}

void DreamWebEngine::turnAnyPathOff(uint8 param, uint8 room) {
	findOrMake(param, 0x00, room + 100);
	_pathData[room].nodes[param].on = 0x00;
}

RoomPaths *DreamWebEngine::getRoomsPaths() {
	return &_pathData[_roomNum];
}

void DreamWebEngine::faceRightWay() {
	PathNode *paths = getRoomsPaths()->nodes;
	uint8 dir = paths[_mansPath].dir;
	_turnToFace = dir;
	_leaveDirection = dir;
}

void DreamWebEngine::setWalk() {
	if (_linePointer != 254) {
		// Already walking
		_finalDest = _pointersPath;
	} else if (_pointersPath == _mansPath) {
		// Can't walk
		faceRightWay();
	} else if (_vars._watchMode == 1) {
		// Holding reel
		_vars._destAfterHold = _pointersPath;
		_vars._watchMode = 2;
	} else if (_vars._watchMode == 2) {
		// Can't walk
	} else {
		_destination = _pointersPath;
		_finalDest = _pointersPath;
		if (_mouseButton != 2 || _commandType == 3) {
			autoSetWalk();
		} else {
			_walkAndExam = 1;
			_walkExamType = _commandType;
			_walkExamNum = _command;
			autoSetWalk();
		}
	}
}

void DreamWebEngine::autoSetWalk() {
	if (_finalDest == _mansPath)
		return;
	const RoomPaths *roomsPaths = getRoomsPaths();
	checkDest(roomsPaths);
	_lineStartX = roomsPaths->nodes[_mansPath].x - 12;
	_lineStartY = roomsPaths->nodes[_mansPath].y - 12;
	_lineEndX = roomsPaths->nodes[_destination].x - 12;
	_lineEndY = roomsPaths->nodes[_destination].y - 12;
	bresenhams();
	if (_lineDirection != 0) {
		_linePointer = _lineLength - 1;
		_lineDirection = 1;
		return;
	}
	_linePointer = 0;
}

void DreamWebEngine::checkDest(const RoomPaths *roomsPaths) {
	const PathSegment *segments = roomsPaths->segments;
	const uint8 tmp = _mansPath << 4;
	uint8 destination = _destination;
	for (size_t i = 0; i < 24; ++i) {
		if ((segments[i].b0 & 0xf0) == tmp &&
		    (segments[i].b0 & 0x0f) == _destination) {
			_destination = segments[i].b1 & 0x0f;
			return;
		}

		if (((segments[i].b0 & 0x0f) << 4) == tmp &&
		    ((segments[i].b0 & 0xf0) >> 4) == _destination) {
			destination = segments[i].b1 & 0x0f;
		}
	}
	_destination = destination;
}

void DreamWebEngine::findXYFromPath() {
	const PathNode *roomsPaths = getRoomsPaths()->nodes;
	_ryanX = roomsPaths[_mansPath].x - 12;
	_ryanY = roomsPaths[_mansPath].y - 12;
}

bool DreamWebEngine::checkIfPathIsOn(uint8 index) {
	RoomPaths *roomsPaths = getRoomsPaths();
	uint8 pathOn = roomsPaths->nodes[index].on;
	return pathOn == 0xff;
}

void DreamWebEngine::bresenhams() {
	workoutFrames();
	Common::Point *lineData = &_lineData[0];
	int16 startX = (int16)_lineStartX;
	int16 startY = (int16)_lineStartY;
	int16 endX = (int16)_lineEndX;
	int16 endY = (int16)_lineEndY;

	if (endX == startX) {
		uint16 deltaY;
		int8 y;
		if (endY < startY) {
			deltaY = startY - endY;
			y = (int8)endY;
			_lineDirection = 1;
		} else {
			deltaY = endY - startY;
			y = (int8)startY;
			_lineDirection = 0;
		}
		++deltaY;
		int8 x = (int8)startX;
		_lineLength = deltaY;
		for (; deltaY; --deltaY) {
			lineData->x = x;
			lineData->y = y;
			++lineData;
			++y;
		}
		return;
	}
	uint16 deltaX;
	if (endX < startX) {
		deltaX = startX - endX;
		SWAP(startX, endX);
		SWAP(startY, endY);
		_lineStartX = (uint16)startX;
		_lineStartY = (uint16)startY;
		_lineEndX = (uint16)endX;
		_lineEndY = (uint16)endY;
		_lineDirection = 1;
	} else {
		deltaX = endX - startX;
		_lineDirection = 0;
	}

	int16 increment;
	if (endY == startY) {
		int8 x = (int8)startX;
		int8 y = (int8)startY;
		++deltaX;
		_lineLength = deltaX;
		for (; deltaX; --deltaX) {
			lineData->x = x;
			lineData->y = y;
			++lineData;
			++x;
		}
		return;
	}
	uint16 deltaY;
	if (startY > endY) {
		deltaY = startY - endY;
		increment = -1;
	} else {
		deltaY = endY - startY;
		increment = 1;
	}

	uint16 delta1, delta2;
	byte lineRoutine;

	if (deltaY > deltaX) {
		lineRoutine = 1;
		delta1 = deltaY;
		delta2 = deltaX;
	} else {
		lineRoutine = 0;
		delta1 = deltaX;
		delta2 = deltaY;
	}

	uint16 increment1 = delta2 * 2;
	uint16 increment2 = delta2 * 2 - delta1 * 2;
	int16 remainder = delta2 * 2 - delta1;
	++delta1;
	int8 x = (int8)startX;
	int8 y = (int8)startY;
	_lineLength = delta1;
	if (lineRoutine != 1) {
		for (; delta1; --delta1) {
			lineData->x = x;
			lineData->y = y;
			++lineData;
			++x;
			if (remainder < 0) {
				remainder += increment1;
			} else {
				remainder += increment2;
				y += increment;
			}
		}
	} else {
		for (; delta1; --delta1) {
			lineData->x = x;
			lineData->y = y;
			++lineData;
			y += increment;
			if (remainder < 0) {
				remainder += increment1;
			} else {
				remainder += increment2;
				++x;
			}
		}
	}
}

void DreamWebEngine::workoutFrames() {
	byte tmp;
	int diffx, diffy;

	// We have to use signed arithmetic here because these values can
	// be slightly negative when walking off-screen
	int lineStartX = (int16)_lineStartX;
	int lineStartY = (int16)_lineStartY;
	int lineEndX = (int16)_lineEndX;
	int lineEndY = (int16)_lineEndY;


	diffx = ABS(lineStartX - lineEndX);
	diffy = ABS(lineStartY - lineEndY);

	if (diffx < diffy) {
		tmp = 2;
		if (diffx >= (diffy >> 1))
			tmp = 1;
	} else {
		// tendstohoriz
		tmp = 0;
		if (diffy >= (diffx >> 1))
			tmp = 1;
	}

	if (lineStartX >= lineEndX) {
		// isinleft
		if (lineStartY < lineEndY) {
			if (tmp != 1)
				tmp ^= 2;
			tmp += 4;
		} else {
			// topleft
			tmp += 6;
		}
	} else {
		// isinright
		if (lineStartY < lineEndY) {
			tmp += 2;
		} else {
			// botright
			if (tmp != 1)
				tmp ^= 2;
		}
	}

	_turnToFace = tmp & 7;
	_turnDirection = 0;
}

byte DreamWebEngine::findFirstPath(byte x, byte y) {
	PathNode *paths = _pathData[_roomNum].nodes;

	for (uint8 index = 0; index < 12; index++) {
		if (paths[index].x1 == 0xff && paths[index].y1 == 0xff)
			continue; // "nofirst"

		if (x < paths[index].x1 || y < paths[index].y1)
			continue; // "nofirst"

		if (x >= paths[index].x2 || y >= paths[index].y2)
			continue; // "nofirst"

		return paths[index].on; // "gotfirst"
	}

	return 0;
}

byte DreamWebEngine::findPathOfPoint(byte x, byte y) {
	PathNode *paths = _pathData[_roomNum].nodes;

	for (uint8 index = 0; index < 12; index++) {
		if (paths[index].on != 0xff)
			continue; // "flunkedit"

		if (paths[index].x1 == 0xff && paths[index].y1 == 0xff)
			continue; // "flunkedit"

		if (x < paths[index].x1 || y < paths[index].y1)
			continue; // "flunkedit"

		if (x >= paths[index].x2 || y >= paths[index].y2)
			continue; // "flunkedit"

		return index; // "gotvalidpath"
	}

	return 0xff;
}

} // End of namespace DreamWeb
