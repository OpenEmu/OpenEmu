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

#include "kyra/kyra_v2.h"

#include "common/system.h"

namespace Kyra {

void KyraEngine_v2::freeSceneAnims() {
	for (int i = 0; i < ARRAYSIZE(_sceneAnims); ++i)
		_sceneAnims[i].flags = 0;

	for (int i = 0; i < ARRAYSIZE(_sceneAnimMovie); ++i) {
		if (_sceneAnimMovie[i])
			_sceneAnimMovie[i]->close();
	}
}

void KyraEngine_v2::updateSpecialSceneScripts() {
	uint32 nextTime = _system->getMillis() + _tickLength;
	const int startScript = _lastProcessedSceneScript;

	while (_system->getMillis() <= nextTime) {
		if (_sceneSpecialScriptsTimer[_lastProcessedSceneScript] <= _system->getMillis() &&
		        !_specialSceneScriptState[_lastProcessedSceneScript]) {
			_specialSceneScriptRunFlag = true;

			while (_specialSceneScriptRunFlag && _sceneSpecialScriptsTimer[_lastProcessedSceneScript] <= _system->getMillis()) {
				if (!_emc->run(&_sceneSpecialScripts[_lastProcessedSceneScript]))
					_specialSceneScriptRunFlag = false;
			}
		}

		if (!_emc->isValid(&_sceneSpecialScripts[_lastProcessedSceneScript])) {
			_emc->start(&_sceneSpecialScripts[_lastProcessedSceneScript], _desc.firstAnimSceneScript + _lastProcessedSceneScript);
			_specialSceneScriptRunFlag = false;
		}

		++_lastProcessedSceneScript;
		if (_lastProcessedSceneScript >= 10)
			_lastProcessedSceneScript = 0;

		if (_lastProcessedSceneScript == startScript)
			return;
	}
}

void KyraEngine_v2::runSceneScript6() {
	_emc->init(&_sceneScriptState, &_sceneScriptData);

	_sceneScriptState.regs[0] = _mainCharacter.sceneId;
	_sceneScriptState.regs[1] = _mouseX;
	_sceneScriptState.regs[2] = _mouseY;
	_sceneScriptState.regs[4] = _itemInHand;

	_emc->start(&_sceneScriptState, 6);
	while (_emc->isValid(&_sceneScriptState))
		_emc->run(&_sceneScriptState);
}

#pragma mark - pathfinder

int KyraEngine_v2::findWay(int x, int y, int toX, int toY, int *moveTable, int moveTableSize) {
	x &= ~3; toX &= ~3;
	y &= ~1; toY &= ~1;
	int size = KyraEngine_v1::findWay(x, y, toX, toY, moveTable, moveTableSize);

	if (size && !_smoothingPath) {
		_smoothingPath = true;
		int temp = pathfinderInitPositionTable(moveTable);
		temp = pathfinderInitPositionIndexTable(temp, x, y);
		pathfinderFinializePath(moveTable, temp, x, y, moveTableSize);
		_smoothingPath = false;
	}

	return _smoothingPath ? size : getMoveTableSize(moveTable);
}

bool KyraEngine_v2::directLinePassable(int x, int y, int toX, int toY) {
	Screen *scr = screen();

	while (x != toX || y != toY) {
		int facing = getFacingFromPointToPoint(x, y, toX, toY);
		x += _addXPosTable[facing];
		y += _addYPosTable[facing];
		if (!scr->getShapeFlag1(x, y))
			return false;
	}

	return true;
}

int KyraEngine_v2::pathfinderInitPositionTable(int *moveTable) {
	bool breakLoop = false;
	int *moveTableCur = moveTable;
	int oldEntry = *moveTableCur, curEntry = *moveTableCur;
	int oldX = 0, newX = 0, oldY = 0, newY = 0;
	int lastEntry = 0;
	lastEntry = pathfinderAddToPositionTable(lastEntry, 0, 0);

	while (*moveTableCur != 8) {
		oldEntry = curEntry;

		while (true) {
			curEntry = *moveTableCur;
			if (curEntry >= 0 && curEntry <= 7)
				break;

			if (curEntry == 8) {
				breakLoop = true;
				break;
			} else {
				++moveTableCur;
			}
		}

		if (breakLoop)
			break;

		oldX = newX;
		oldY = newY;

		newX += _addXPosTable[curEntry];
		newY += _addYPosTable[curEntry];

		int temp = ABS(curEntry - oldEntry);
		if (temp > 4) {
			temp = 8 - temp;
		}

		if (temp > 1 || oldEntry != curEntry)
			lastEntry = pathfinderAddToPositionTable(lastEntry, oldX, oldY);

		++moveTableCur;
	}

	lastEntry = pathfinderAddToPositionTable(lastEntry, newX, newY);
	_pathfinderPositionTable[lastEntry * 2 + 0] = -1;
	_pathfinderPositionTable[lastEntry * 2 + 1] = -1;
	return lastEntry;
}

int KyraEngine_v2::pathfinderAddToPositionTable(int index, int v1, int v2) {
	_pathfinderPositionTable[index << 1] = v1;
	_pathfinderPositionTable[(index << 1) + 1] = v2;
	++index;
	if (index >= 199)
		--index;
	return index;
}

int KyraEngine_v2::pathfinderInitPositionIndexTable(int tableLen, int x, int y) {
	int x1 = 0, y1 = 0;
	int x2 = 0, y2 = 0;
	int lastEntry = 0;
	int index2 = tableLen - 1, index1 = 0;
	while (index2 > index1) {
		x1 = _pathfinderPositionTable[index1 * 2 + 0] + x;
		y1 = _pathfinderPositionTable[index1 * 2 + 1] + y;
		x2 = _pathfinderPositionTable[index2 * 2 + 0] + x;
		y2 = _pathfinderPositionTable[index2 * 2 + 1] + y;

		if (directLinePassable(x1, y1, x2, y2)) {
			lastEntry = pathfinderAddToPositionIndexTable(lastEntry, index2);
			if (tableLen - 1 == index2)
				break;
			index1 = index2;
			index2 = tableLen - 1;
		} else if (index1 + 1 == index2) {
			lastEntry = pathfinderAddToPositionIndexTable(lastEntry, index2);
			index1 = index2;
			index2 = tableLen - 1;
		} else {
			--index2;
		}
	}
	return lastEntry;
}

int KyraEngine_v2::pathfinderAddToPositionIndexTable(int index, int v) {
	_pathfinderPositionIndexTable[index] = v;
	++index;
	if (index >= 199)
		--index;
	return index;
}

void KyraEngine_v2::pathfinderFinializePath(int *moveTable, int tableLen, int x, int y, int moveTableSize) {
	int x1 = 0, y1 = 0;
	int x2 = 0, y2 = 0;
	int index1 = 0, index2 = 0;
	int sizeLeft = moveTableSize;
	for (int i = 0; i < tableLen; ++i) {
		index2 = _pathfinderPositionIndexTable[i];
		x1 = _pathfinderPositionTable[index1 * 2 + 0] + x;
		y1 = _pathfinderPositionTable[index1 * 2 + 1] + y;
		x2 = _pathfinderPositionTable[index2 * 2 + 0] + x;
		y2 = _pathfinderPositionTable[index2 * 2 + 1] + y;

		int wayLen = findWay(x1, y1, x2, y2, moveTable, sizeLeft);
		moveTable += wayLen;
		sizeLeft -= wayLen; // unlike the original we want to be sure that the size left is correct
		index1 = index2;
	}
}

} // End of namespace Kyra
