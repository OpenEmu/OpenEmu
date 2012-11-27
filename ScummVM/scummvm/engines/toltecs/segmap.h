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
 *
 */

#ifndef TOLTECS_SEGMAP_H
#define TOLTECS_SEGMAP_H

#include "common/array.h"

#include "toltecs/screen.h"

namespace Toltecs {

struct ScriptWalk {
	int16 y, x;
	int16 y1, x1, y2, x2;
	int16 yerror, xerror;
	int16 mulValue;
	int16 scaling;
};

struct SegmapMaskRect {
	int16 x, y;
	int16 width, height;
	int16 priority;
	Graphics::Surface *surface;
};

class SegmentMap {
public:
	SegmentMap(ToltecsEngine *vm);
	~SegmentMap();

	void load(byte *source);

	int16 findPathRectAtPoint(int16 x, int16 y);
	void adjustPathPoint(int16 &x, int16 &y);

	void findPath(int16 *pointsArray, int16 destX, int16 destY, int16 sourceX, int16 sourceY);

	int8 getScalingAtPoint(int16 x, int16 y);
	void getRgbModifiertAtPoint(int16 x, int16 y, int16 id, byte &r, byte &g, byte &b);

	void addMasksToRenderQueue();

//protected:
public: // for debugging purposes

	struct SegmapPathRect {
		int16 x1, y1, x2, y2;
	};

	struct SegmapInfoRect {
		int16 y, x;
		int16 height, width;
		byte id;
		byte a, b, c;
		inline bool isPointInside(int16 px, int16 py) {
			return py >= y && py <= y + height && px >= x && px <= x + width;
		}
	};

	struct PathPoint {
		int16 y, x;
	};

	typedef Common::Array<SegmapMaskRect> SegmapMaskRectArray;
	typedef Common::Array<SegmapPathRect> SegmapPathRectArray;
	typedef Common::Array<SegmapInfoRect> SegmapInfoRectArray;

	ToltecsEngine *_vm;

	SegmapMaskRectArray _maskRects;
	byte *_maskRectData;

	SegmapPathRectArray _pathRects;
	SegmapInfoRectArray _infoRects;

	int16 _deadEndPathRects[1000];
	uint _deadEndPathRectsCount;

	int16 _closedPathRects[1000];
	uint _closedPathRectsCount;

	PathPoint _pathNodes[1000];
	int16 _pathNodesCount;

	int16 findNextPathRect(int16 srcRectIndex, int16 destX, int16 destY);

	void loadSegmapMaskRectSurface(byte *maskData, SegmapMaskRect &maskRect);
	void freeSegmapMaskRectSurfaces();

};

} // End of namespace Toltecs

#endif /* TOLTECS_SEGMAP_H */
