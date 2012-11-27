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

#ifndef TOON_PATH_H
#define TOON_PATH_H

#include "common/array.h"
#include "common/rect.h"

#include "toon/toon.h"

namespace Toon {

// binary heap system for fast A*
class PathFindingHeap {
public:
	PathFindingHeap();
	~PathFindingHeap();

	void push(int16 x, int16 y, uint16 weight);
	void pop(int16 *x, int16 *y, uint16 *weight);
	void init(int32 size);
	void clear();
	void unload();
	uint32 getCount() { return _count; }

private:
	struct HeapDataGrid {
		int16 _x, _y;
		uint16 _weight;
	};

	HeapDataGrid *_data;

	uint32 _size;
	uint32 _count;
};

class PathFinding {
public:
	PathFinding();
	~PathFinding();

	void init(Picture *mask);

	bool findPath(int16 x, int16 y, int16 destX, int16 destY);
	bool findClosestWalkingPoint(int16 xx, int16 yy, int16 *fxx, int16 *fyy, int16 origX = -1, int16 origY = -1);
	bool isWalkable(int16 x, int16 y);
	bool isLikelyWalkable(int16 x, int16 y);
	bool lineIsWalkable(int16 x, int16 y, int16 x2, int16 y2);
	void walkLine(int16 x, int16 y, int16 x2, int16 y2);

	void resetBlockingRects() { _numBlockingRects = 0; }
	void addBlockingRect(int16 x1, int16 y1, int16 x2, int16 y2);
	void addBlockingEllipse(int16 x1, int16 y1, int16 w, int16 h);

	uint32 getPathNodeCount() const { return _tempPath.size(); }
	int16 getPathNodeX(uint32 nodeId) const { return _tempPath[(_tempPath.size() - 1) - nodeId].x; }
	int16 getPathNodeY(uint32 nodeId) const { return _tempPath[(_tempPath.size() - 1) - nodeId].y; }

private:
	static const uint8 kMaxBlockingRects = 16;

	Picture *_currentMask;

	PathFindingHeap *_heap;

	uint16 *_sq;
	int16 _width;
	int16 _height;

	Common::Array<Common::Point> _tempPath;

	int16 _blockingRects[kMaxBlockingRects][5];
	uint8 _numBlockingRects;
};

} // End of namespace Toon

#endif
