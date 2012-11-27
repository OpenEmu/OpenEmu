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

#include "common/debug.h"

#include "toon/path.h"

namespace Toon {

PathFindingHeap::PathFindingHeap() {
	_count = 0;
	_size = 0;
	_data = NULL;
}

PathFindingHeap::~PathFindingHeap() {
	free(_data);
}

void PathFindingHeap::init(int32 size) {
	debugC(1, kDebugPath, "init(%d)", size);
	_size = size;

	free(_data);
	_data = (HeapDataGrid *)malloc(sizeof(HeapDataGrid) * _size);
	memset(_data, 0, sizeof(HeapDataGrid) * _size);
	_count = 0;
}

void PathFindingHeap::unload() {
	_count = 0;
	_size = 0;
	free(_data);
	_data = NULL;
}

void PathFindingHeap::clear() {
	debugC(1, kDebugPath, "clear()");

	_count = 0;
	memset(_data, 0, sizeof(HeapDataGrid) * _size);
}

void PathFindingHeap::push(int16 x, int16 y, uint16 weight) {
	debugC(2, kDebugPath, "push(%d, %d, %d)", x, y, weight);

	if (_count == _size) {
		// Increase size by 50%
		uint32 newSize = _size + (_size / 2) + 1;
		HeapDataGrid *newData;

		newData = (HeapDataGrid *)realloc(_data, sizeof(HeapDataGrid) * newSize);
		if (newData == NULL) {
			warning("Aborting attempt to push onto PathFindingHeap at maximum size: %d", _count);
			return;
		}

		memset(newData + _size, 0, sizeof(HeapDataGrid) * (newSize - _size));
		_data = newData;
		_size = newSize;
	}

	_data[_count]._x = x;
	_data[_count]._y = y;
	_data[_count]._weight = weight;
	_count++;

	uint32 lMax = _count - 1;
	uint32 lT = 0;

	while (true) {
		if (lMax <= 0)
			break;
		lT = (lMax - 1) / 2;

		if (_data[lT]._weight > _data[lMax]._weight) {
			HeapDataGrid temp;
			temp = _data[lT];
			_data[lT] = _data[lMax];
			_data[lMax] = temp;
			lMax = lT;
		} else {
			break;
		}
	}
}

void PathFindingHeap::pop(int16 *x, int16 *y, uint16 *weight) {
	debugC(2, kDebugPath, "pop(x, y, weight)");

	if (!_count) {
		warning("Attempt to pop empty PathFindingHeap!");
		return;
	}

	*x = _data[0]._x;
	*y = _data[0]._y;
	*weight = _data[0]._weight;

	_data[0] = _data[--_count];
	if (!_count)
		return;

	uint32 lMin = 0;
	uint32 lT = 0;

	while (true) {
		lT = (lMin * 2) + 1;
		if (lT < _count) {
			if (lT < _count - 1) {
				if (_data[lT + 1]._weight < _data[lT]._weight)
					lT++;
			}
			if (_data[lT]._weight <= _data[lMin]._weight) {
				HeapDataGrid temp;
				temp = _data[lMin];
				_data[lMin] = _data[lT];
				_data[lT] = temp;

				lMin = lT;
			} else {
				break;
			}
		} else {
			break;
		}
	}
}

PathFinding::PathFinding() {
	_width = 0;
	_height = 0;
	_heap = new PathFindingHeap();
	_sq = NULL;
	_numBlockingRects = 0;
}

PathFinding::~PathFinding(void) {
	if (_heap)
		_heap->unload();
	delete _heap;
	delete[] _sq;
}

void PathFinding::init(Picture *mask) {
	debugC(1, kDebugPath, "init(mask)");

	_width = mask->getWidth();
	_height = mask->getHeight();
	_currentMask = mask;
	_heap->unload();
	_heap->init(500);
	delete[] _sq;
	_sq = new uint16[_width * _height];
}

bool PathFinding::isLikelyWalkable(int16 x, int16 y) {
	for (uint8 i = 0; i < _numBlockingRects; i++) {
		if (_blockingRects[i][4] == 0) {
			if (x >= _blockingRects[i][0] && x <= _blockingRects[i][2] && y >= _blockingRects[i][1] && y < _blockingRects[i][3])
				return false;
		} else {
			int16 dx = abs(_blockingRects[i][0] - x);
			int16 dy = abs(_blockingRects[i][1] - y);
			if ((dx << 8) / _blockingRects[i][2] < (1 << 8) && (dy << 8) / _blockingRects[i][3] < (1 << 8)) {
				return false;
			}
		}
	}
	return true;
}

bool PathFinding::isWalkable(int16 x, int16 y) {
	debugC(2, kDebugPath, "isWalkable(%d, %d)", x, y);

	return (_currentMask->getData(x, y) & 0x1f) > 0;
}

bool PathFinding::findClosestWalkingPoint(int16 xx, int16 yy, int16 *fxx, int16 *fyy, int16 origX, int16 origY) {
	debugC(1, kDebugPath, "findClosestWalkingPoint(%d, %d, fxx, fyy, %d, %d)", xx, yy, origX, origY);

	int32 currentFound = -1;
	int32 dist = -1;
	int32 dist2 = -1;

	if (origX == -1)
		origX = xx;
	if (origY == -1)
		origY = yy;

	for (int16 y = 0; y < _height; y++) {
		for (int16 x = 0; x < _width; x++) {
			if (isWalkable(x, y) && isLikelyWalkable(x, y)) {
				int32 ndist = (x - xx) * (x - xx) + (y - yy) * (y - yy);
				int32 ndist2 = (x - origX) * (x - origX) + (y - origY) * (y - origY);
				if (currentFound < 0 || ndist < dist || (ndist == dist && ndist2 < dist2)) {
					dist = ndist;
					dist2 = ndist2;
					currentFound = y * _width + x;
				}
			}
		}
	}

	if (currentFound != -1) {
		*fxx = currentFound % _width;
		*fyy = currentFound / _width;
		return true;
	} else {
		*fxx = 0;
		*fyy = 0;
		return false;
	}
}

void PathFinding::walkLine(int16 x, int16 y, int16 x2, int16 y2) {
	uint32 bx = x << 16;
	int32 dx = x2 - x;
	uint32 by = y << 16;
	int32 dy = y2 - y;
	uint32 adx = abs(dx);
	uint32 ady = abs(dy);
	int32 t = 0;
	if (adx <= ady)
		t = ady;
	else
		t = adx;

	int32 cdx = (dx << 16) / t;
	int32 cdy = (dy << 16) / t;

	_tempPath.clear();
	for (int32 i = t; i > 0; i--) {
		_tempPath.insert_at(0, Common::Point(bx >> 16, by >> 16));
		bx += cdx;
		by += cdy;
	}

	_tempPath.insert_at(0, Common::Point(x2, y2));
}

bool PathFinding::lineIsWalkable(int16 x, int16 y, int16 x2, int16 y2) {
	uint32 bx = x << 16;
	int32 dx = x2 - x;
	uint32 by = y << 16;
	int32 dy = y2 - y;
	uint32 adx = abs(dx);
	uint32 ady = abs(dy);
	int32 t = 0;
	if (adx <= ady)
		t = ady;
	else
		t = adx;

	int32 cdx = (dx << 16) / t;
	int32 cdy = (dy << 16) / t;

	for (int32 i = t; i > 0; i--) {
		if (!isWalkable(bx >> 16, by >> 16))
			return false;
		bx += cdx;
		by += cdy;
	}
	return true;
}

bool PathFinding::findPath(int16 x, int16 y, int16 destx, int16 desty) {
	debugC(1, kDebugPath, "findPath(%d, %d, %d, %d)", x, y, destx, desty);

	if (x == destx && y == desty) {
		_tempPath.clear();
		return true;
	}

	// ignore path finding if the character is outside the screen
	if (x < 0 || x > 1280 || y < 0 || y > 400 || destx < 0 || destx > 1280 || desty < 0 || desty > 400) {
		_tempPath.clear();
		return true;
	}

	// first test direct line
	if (lineIsWalkable(x, y, destx, desty)) {
		walkLine(x, y, destx, desty);
		return true;
	}

	// no direct line, we use the standard A* algorithm
	memset(_sq , 0, _width * _height * sizeof(uint16));
	_heap->clear();
	int16 curX = x;
	int16 curY = y;
	uint16 curWeight = 0;

	_sq[curX + curY *_width] = 1;
	_heap->push(curX, curY, abs(destx - x) + abs(desty - y));

	while (_heap->getCount()) {
		_heap->pop(&curX, &curY, &curWeight);
		int32 curNode = curX + curY * _width;

		int16 endX = MIN<int16>(curX + 1, _width - 1);
		int16 endY = MIN<int16>(curY + 1, _height - 1);
		int16 startX = MAX<int16>(curX - 1, 0);
		int16 startY = MAX<int16>(curY - 1, 0);
		bool next = false;

		for (int16 px = startX; px <= endX && !next; px++) {
			for (int16 py = startY; py <= endY && !next; py++) {
				if (px != curX || py != curY) {
					uint16 wei = abs(px - curX) + abs(py - curY);

					if (isWalkable(px, py)) { // walkable ?
						int32 curPNode = px + py * _width;
						uint32 sum = _sq[curNode] + wei * (1 + (isLikelyWalkable(px, py) ? 5 : 0));
						if (sum > (uint32)0xFFFF) {
							warning("PathFinding::findPath sum exceeds maximum representable!");
							sum = (uint32)0xFFFF;
						}
						if (_sq[curPNode] > sum || !_sq[curPNode]) {
							_sq[curPNode] = sum;
							uint32 newWeight = _sq[curPNode] + abs(destx - px) + abs(desty - py);
							if (newWeight > (uint32)0xFFFF) {
								warning("PathFinding::findPath newWeight exceeds maximum representable!");
								newWeight = (uint16)0xFFFF;
							}
							_heap->push(px, py, newWeight);
							if (!newWeight)
								next = true; // we found it !
						}
					}
				}
			}
		}
	}

	// let's see if we found a result !
	if (!_sq[destx + desty * _width]) {
		// didn't find anything
		_tempPath.clear();
		return false;
	}

	curX = destx;
	curY = desty;

	Common::Array<Common::Point> retPath;
	retPath.push_back(Common::Point(curX, curY));

	uint16 bestscore = _sq[destx + desty * _width];

	bool retVal = false;
	while (true) {
		int16 bestX = -1;
		int16 bestY = -1;

		int16 endX = MIN<int16>(curX + 1, _width - 1);
		int16 endY = MIN<int16>(curY + 1, _height - 1);
		int16 startX = MAX<int16>(curX - 1, 0);
		int16 startY = MAX<int16>(curY - 1, 0);

		for (int16 px = startX; px <= endX; px++) {
			for (int16 py = startY; py <= endY; py++) {
				if (px != curX || py != curY) {
					int32 PNode = px + py * _width;
					if (_sq[PNode] && (isWalkable(px, py))) {
						if (_sq[PNode] < bestscore) {
							bestscore = _sq[PNode];
							bestX = px;
							bestY = py;
						}
					}
				}
			}
		}

		if (bestX < 0 || bestY < 0)
			break;

		retPath.push_back(Common::Point(bestX, bestY));

		if ((bestX == x && bestY == y)) {
			_tempPath.clear();
			for (uint32 i = 0; i < retPath.size(); i++)
				_tempPath.push_back(retPath[i]);

			retVal = true;
			break;
		}

		curX = bestX;
		curY = bestY;
	}

	return retVal;
}

void PathFinding::addBlockingRect(int16 x1, int16 y1, int16 x2, int16 y2) {
	debugC(1, kDebugPath, "addBlockingRect(%d, %d, %d, %d)", x1, y1, x2, y2);
	if (_numBlockingRects >= kMaxBlockingRects) {
		warning("Maximum number of %d Blocking Rects reached!", kMaxBlockingRects);
		return;
	}

	_blockingRects[_numBlockingRects][0] = x1;
	_blockingRects[_numBlockingRects][1] = y1;
	_blockingRects[_numBlockingRects][2] = x2;
	_blockingRects[_numBlockingRects][3] = y2;
	_blockingRects[_numBlockingRects][4] = 0;
	_numBlockingRects++;
}

void PathFinding::addBlockingEllipse(int16 x1, int16 y1, int16 w, int16 h) {
	debugC(1, kDebugPath, "addBlockingEllipse(%d, %d, %d, %d)", x1, y1, w, h);
	if (_numBlockingRects >= kMaxBlockingRects) {
		warning("Maximum number of %d Blocking Rects reached!", kMaxBlockingRects);
		return;
	}

	_blockingRects[_numBlockingRects][0] = x1;
	_blockingRects[_numBlockingRects][1] = y1;
	_blockingRects[_numBlockingRects][2] = w;
	_blockingRects[_numBlockingRects][3] = h;
	_blockingRects[_numBlockingRects][4] = 1;
	_numBlockingRects++;
}

} // End of namespace Toon
