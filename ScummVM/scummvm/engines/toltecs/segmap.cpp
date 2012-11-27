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

#include "toltecs/toltecs.h"
#include "toltecs/render.h"
#include "toltecs/segmap.h"

namespace Toltecs {

SegmentMap::SegmentMap(ToltecsEngine *vm) : _vm(vm) {
}

SegmentMap::~SegmentMap() {
	freeSegmapMaskRectSurfaces();
}

void SegmentMap::load(byte *source) {

	freeSegmapMaskRectSurfaces();
	_maskRects.clear();
	_pathRects.clear();
	_infoRects.clear();

	// Load mask rects
	byte *maskData = source + 2;
	uint16 maskSize = READ_LE_UINT16(source);
	source += 2;
	uint16 maskRectCount = READ_LE_UINT16(source);
	source += 2;
	uint16 maskRectDataSize = maskRectCount * 12 + 2;

	debug(0, "SegmentMap::load() maskRectCount = %d", maskRectCount);

	for (uint16 i = 0; i < maskRectCount; i++) {
		SegmapMaskRect maskRect;
		int16 maskOffset;
		maskRect.y = READ_LE_UINT16(source);
		maskRect.x = READ_LE_UINT16(source + 2);
		maskRect.height = READ_LE_UINT16(source + 4);
		maskRect.width = READ_LE_UINT16(source + 6);
		maskOffset = READ_LE_UINT16(source + 8);
		maskRect.priority = READ_LE_UINT16(source + 10);
		loadSegmapMaskRectSurface(maskData + maskOffset, maskRect);

		debug(0, "SegmentMap::load() (%d, %d, %d, %d, %04X, %d)",
			maskRect.x, maskRect.y, maskRect.width, maskRect.height, maskOffset, maskRect.priority);

		source += 12;
		_maskRects.push_back(maskRect);
	}

	source += maskSize - maskRectDataSize;

	// Load path rects

	source += 2; // skip rects array size

	uint16 pathRectCount = READ_LE_UINT16(source);
	source += 2;

	debug(0, "SegmentMap::load() pathRectCount = %d", pathRectCount);

	for (uint16 i = 0; i < pathRectCount; i++) {
		SegmapPathRect pathRect;
		pathRect.y1 = READ_LE_UINT16(source);
		pathRect.x1 = READ_LE_UINT16(source + 2);
		pathRect.y2 = pathRect.y1 + READ_LE_UINT16(source + 4);
		pathRect.x2 = pathRect.x1 + READ_LE_UINT16(source + 6);

		debug(0, "SegmentMap::load() (%d, %d, %d, %d)", pathRect.x1, pathRect.y1, pathRect.x2, pathRect.y2);

		source += 8;
		_pathRects.push_back(pathRect);
	}

	// Load info rects

	source += 2; // skip rects array size

	uint16 infoRectCount = READ_LE_UINT16(source);
	source += 2;
	debug(0, "SegmentMap::load() infoRectCount = %d", infoRectCount);
	for (uint16 i = 0; i < infoRectCount; i++) {
		SegmapInfoRect infoRect;
		infoRect.y = READ_LE_UINT16(source);
		infoRect.x = READ_LE_UINT16(source + 2);
		infoRect.height = READ_LE_UINT16(source + 4);
		infoRect.width = READ_LE_UINT16(source + 6);
		infoRect.id = source[8];
		infoRect.a = source[9];
		infoRect.b = source[10];
		infoRect.c = source[11];

		debug(0, "SegmentMap::load() (%d, %d, %d, %d) (%d, %d, %d, %d)",
			infoRect.x, infoRect.y, infoRect.width, infoRect.height,
			infoRect.id, (int8)infoRect.a, (int8)infoRect.b, (int8)infoRect.c);

		source += 12;
		_infoRects.push_back(infoRect);
	}

	// TODO Other stuff


}

int16 SegmentMap::findPathRectAtPoint(int16 x, int16 y) {
	for (int16 rectIndex = 0; rectIndex < (int16)_pathRects.size(); rectIndex++) {
		if (y >= _pathRects[rectIndex].y1 && y <= _pathRects[rectIndex].y2 &&
			x >= _pathRects[rectIndex].x1 && x <= _pathRects[rectIndex].x2) {
			return rectIndex;
		}
	}
	return -1;
}

void SegmentMap::adjustPathPoint(int16 &x, int16 &y) {

	if (findPathRectAtPoint(x, y) != -1)
		return;

	uint32 minDistance = 0xFFFFFFFF, distance;
	int16 adjustedX = 0, adjustedY = 0, x2, y2;

	for (int16 rectIndex = 0; rectIndex < (int16)_pathRects.size(); rectIndex++) {

		if (x >= _pathRects[rectIndex].x1 && x < _pathRects[rectIndex].x2) {
			x2 = x;
		} else if (ABS(x - _pathRects[rectIndex].x1) >= ABS(x - _pathRects[rectIndex].x2)) {
			x2 = _pathRects[rectIndex].x2;
		} else {
			x2 = _pathRects[rectIndex].x1;
		}

		if (ABS(y - _pathRects[rectIndex].y1) >= ABS(y - _pathRects[rectIndex].y2)) {
			y2 = _pathRects[rectIndex].y2;
		} else {
			y2 = _pathRects[rectIndex].y1;
		}

		distance = ABS(y - y2) + ABS(x - x2);
		if (distance < minDistance) {
			if (x >= _pathRects[rectIndex].x1 && x <= _pathRects[rectIndex].x2) {
				adjustedX = x;
			} else {
				adjustedX = x2;
			}
			if (y >= _pathRects[rectIndex].y1 && y <= _pathRects[rectIndex].y2) {
				adjustedY = y;
			} else {
				adjustedY = y2;
			}
			minDistance = distance;
		}

	}

	x = adjustedX;
	y = adjustedY;

}

int16 SegmentMap::findNextPathRect(int16 srcRectIndex, int16 destX, int16 destY) {

	int16 result;
	uint16 minDistance, distance;
	int16 x1, y1, x2, y2;
	int16 xmin, xmax, ymax, ymin;

	result = -1;
	minDistance = 0xFFFF;

	x1 = _pathRects[srcRectIndex].x1;
	y1 = _pathRects[srcRectIndex].y1;
	x2 = _pathRects[srcRectIndex].x2;
	y2 = _pathRects[srcRectIndex].y2;

	for (int16 rectIndex = 0; rectIndex < (int16)_pathRects.size(); rectIndex++) {

		int16 nodeX = -1, nodeY = -1;

		// Check if the current rectangle is connected to the source rectangle
		if (x1 == _pathRects[rectIndex].x2 && y1 < _pathRects[rectIndex].y2 && y2 > _pathRects[rectIndex].y1) {
			nodeX = x1;
		} else if (x2 == _pathRects[rectIndex].x1 && y1 < _pathRects[rectIndex].y2 && y2 > _pathRects[rectIndex].y1) {
			nodeX = x2 - 1;
		} else if (y1 == _pathRects[rectIndex].y2 && x1 < _pathRects[rectIndex].x2 && x2 > _pathRects[rectIndex].x1) {
			nodeY = y1;
		} else if (y2 == _pathRects[rectIndex].y1 && x1 < _pathRects[rectIndex].x2 && x2 > _pathRects[rectIndex].x1) {
			nodeY = y2 - 1;
		} else
			continue;

		if (nodeX == -1) {
			xmin = MAX<int16>(x1, _pathRects[rectIndex].x1);
			xmax = MIN<int16>(x2, _pathRects[rectIndex].x2) - 1;
			if (destX > xmin && destX < xmax) {
				nodeX = destX;
			} else if (ABS(destX - xmin) >= ABS(destX - xmax)) {
				nodeX = xmax - 1;
			} else {
				nodeX = xmin;
			}
		}

		if (nodeY == -1) {
			ymin = MAX<int16>(y1, _pathRects[rectIndex].y1);
			ymax = MIN<int16>(y2, _pathRects[rectIndex].y2) - 1;
			if (destY > ymin && destY < ymax) {
				nodeY = destY;
			} else if (ABS(destY - ymin) >= ABS(destY - ymax)) {
				nodeY = ymax - 1;
			} else {
				nodeY = ymin;
			}
		}

		distance = ABS(destX - nodeX) + ABS(destY - nodeY);

		for (uint i = 0; i < _closedPathRectsCount; i++) {
			if (rectIndex == _closedPathRects[i]) {
				distance = minDistance;
				break;
			}
		}

		for (uint i = 0; i < _deadEndPathRectsCount; i++) {
			if (rectIndex == _deadEndPathRects[i]) {
				distance = minDistance;
				break;
			}
		}

		if (distance < minDistance) {
			result = rectIndex;
			minDistance = distance;
			_pathNodes[_pathNodesCount].x = nodeX;
			_pathNodes[_pathNodesCount].y = nodeY;
		}

	}

	return result;
}

struct LineData {
	int pitch;
	byte *surf;
};

void plotProc(int x, int y, int color, void *data) {
	LineData *ld = (LineData *)data;
	ld->surf[x + y * ld->pitch] = color;
}

void SegmentMap::findPath(int16 *pointsArray, int16 destX, int16 destY, int16 sourceX, int16 sourceY) {

	int16 currentRectIndex, destRectIndex;
	int16 pointsCount;

	debug(0, "SegmentMap::findPath(fromX: %d; fromY: %d; toX: %d; toY: %d)", sourceX, sourceY, destX, destY);

	_deadEndPathRectsCount = 0;
	_closedPathRectsCount = 0;
	_pathNodesCount = 0;

	pointsCount = 2;

	adjustPathPoint(sourceX, sourceY);
	currentRectIndex = findPathRectAtPoint(sourceX, sourceY);

	adjustPathPoint(destX, destY);
	destRectIndex = findPathRectAtPoint(destX, destY);

	if (currentRectIndex != -1) {
		if (destRectIndex != currentRectIndex) {
			while (1) {
				do {
					_closedPathRects[_closedPathRectsCount++] = currentRectIndex;
					currentRectIndex = findNextPathRect(currentRectIndex, destX, destY);
					_pathNodesCount++;
				} while (currentRectIndex != -1 && currentRectIndex != destRectIndex);
				if (currentRectIndex != -1 && currentRectIndex == destRectIndex)
					break;
				_deadEndPathRects[_deadEndPathRectsCount++] = _closedPathRects[--_closedPathRectsCount];
				_pathNodesCount -= 2;
				currentRectIndex = _closedPathRects[--_closedPathRectsCount];
			}
			for (int16 i = 0; i < _pathNodesCount; i++) {
				pointsArray[pointsCount++] = TO_LE_16(_pathNodes[i].y);
				pointsArray[pointsCount++] = TO_LE_16(_pathNodes[i].x);
			}
		}

		pointsArray[pointsCount++] = TO_LE_16(destY);
		pointsArray[pointsCount++] = TO_LE_16(destX);

		pointsArray[0] = 0;
		pointsArray[1] = TO_LE_16(_pathNodesCount + 1);
	}

	debug(0, "SegmentMap::findPath() count = %d", FROM_LE_16(pointsArray[1]));

#if 0 // DEBUG: Draw the path we found
	int sx = sourceX, sy = sourceY;
	LineData ld;
	ld.pitch = _vm->_sceneWidth;
	ld.surf = _vm->_screen->_backScreen;
	for (int16 i = 0; i < FROM_LE_16(pointsArray[1]) * 2; i+=2) {
		const int x = FROM_LE_16(pointsArray[3+i]);
		const int y = FROM_LE_16(pointsArray[2+1]);
		debug(0, "x = %d; y = %d", x, y);
		Graphics::drawLine(sx, sy, x, y, 0xFF, plotProc, &ld);
		sx = x;
		sy = y;
	}
#endif

}

int8 SegmentMap::getScalingAtPoint(int16 x, int16 y) {
	int8 scaling = 0;
	for (uint i = 0; i < _infoRects.size(); i++) {
		if (_infoRects[i].id == 0 && _infoRects[i].isPointInside(x, y)) {
			int8 topScaling = (int8)_infoRects[i].b;
			int8 bottomScaling = (int8)_infoRects[i].c;
			if (y - _infoRects[i].y != 0) {
				scaling = (ABS(y - _infoRects[i].y) * (bottomScaling - topScaling) / _infoRects[i].height) + topScaling;
			}
		}
	}
	debug(0, "SegmentMap::getScalingAtPoint(%d, %d) %d", x, y, scaling);
	return scaling;
}

void SegmentMap::getRgbModifiertAtPoint(int16 x, int16 y, int16 id, byte &r, byte &g, byte &b) {
	r = 0;
	g = 0;
	b = 0;
	for (uint i = 0; i < _infoRects.size(); i++) {
		if (_infoRects[i].id == id && _infoRects[i].isPointInside(x, y)) {
			r = _infoRects[i].a;
			g = _infoRects[i].b;
			b = _infoRects[i].c;
		}
	}
	debug(0, "SegmentMap::getRgbModifiertAtPoint() r: %d; g: %d; b: %d", r, g, b);
}

void SegmentMap::loadSegmapMaskRectSurface(byte *maskData, SegmapMaskRect &maskRect) {

	maskRect.surface = new Graphics::Surface();
	maskRect.surface->create(maskRect.width, maskRect.height, Graphics::PixelFormat::createFormatCLUT8());

	byte *backScreen = _vm->_screen->_backScreen + maskRect.x + (maskRect.y * _vm->_sceneWidth);
	byte *dest = (byte *)maskRect.surface->getBasePtr(0, 0);

	for (int16 h = 0; h < maskRect.height; h++) {
		int16 w = maskRect.width;
		while (w > 0) {
			byte mask = *maskData++;
			byte count = mask & 0x7F;
			if (mask & 0x80)
				memcpy(dest, backScreen, count);
			else
				memset(dest, 0xFF, count);
			w -= count;
			dest += count;
			backScreen += count;
		}
		backScreen += _vm->_sceneWidth - maskRect.width;
	}

}

void SegmentMap::freeSegmapMaskRectSurfaces() {
	for (uint i = 0; i < _maskRects.size(); i++) {
		delete _maskRects[i].surface;
	}
}

void SegmentMap::addMasksToRenderQueue() {
	for (uint i = 0; i < _maskRects.size(); i++) {
		_vm->_screen->_renderQueue->addMask(_maskRects[i]);
	}
}

} // End of namespace Toltecs
