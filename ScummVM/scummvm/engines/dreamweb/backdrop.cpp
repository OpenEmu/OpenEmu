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

void DreamWebEngine::doBlocks() {
	uint16 dstOffset = _mapAdY * 320 + _mapAdX;
	uint16 mapOffset = _mapY * kMapWidth + _mapX;
	const uint8 *mapData = _mapData + mapOffset;
	uint8 *dstBuffer = workspace() + dstOffset;

	for (size_t i = 0; i < 10; ++i) {
		for (size_t j = 0; j < 11; ++j) {
			uint16 blockType = mapData[j];
			if (blockType != 0) {
				uint8 *dst = dstBuffer + i * 320 * 16 + j * 16;
				const uint8 *block = _backdropBlocks + blockType * 256;
				for (size_t k = 0; k < 4; ++k) {
					memcpy(dst, block, 16);
					block += 16;
					dst += 320;
				}
				for (size_t k = 0; k < 12; ++k) {
					memcpy(dst, block, 16);
					memset(dst + 16, 0xdf, 4);
					block += 16;
					dst += 320;
				}
				dst += 4;
				memset(dst, 0xdf, 16);
				dst += 320;
				memset(dst, 0xdf, 16);
				dst += 320;
				memset(dst, 0xdf, 16);
				dst += 320;
				memset(dst, 0xdf, 16);
			}
		}
		mapData += kMapWidth;
	}
}

uint8 DreamWebEngine::getXAd(const uint8 *setData, uint8 *result) {
	uint8 v0 = setData[0];
	uint8 v1 = setData[1];
	uint8 v2 = setData[2];
	if (v0 != 0)
		return 0;
	if (v1 < _mapX)
		return 0;
	v1 -= _mapX;
	if (v1 >= 11)
		return 0;
	*result = (v1 << 4) | v2;
	return 1;
}

uint8 DreamWebEngine::getYAd(const uint8 *setData, uint8 *result) {
	uint8 v0 = setData[3];
	uint8 v1 = setData[4];
	if (v0 < _mapY)
		return 0;
	v0 -= _mapY;
	if (v0 >= 10)
		return 0;
	*result = (v0 << 4) | v1;
	return 1;
}

uint8 DreamWebEngine::getMapAd(const uint8 *setData, uint16 *x, uint16 *y) {
	uint8 xad, yad;
	if (getXAd(setData, &xad) == 0)
		return 0;
	*x = xad;
	if (getYAd(setData, &yad) == 0)
		return 0;
	*y = yad;
	return 1;
}

void DreamWebEngine::calcFrFrame(const Frame &frame, uint8 *width, uint8 *height, uint16 x, uint16 y, ObjPos *objPos) {
	*width = frame.width;
	*height = frame.height;

	objPos->xMin = (x + frame.x) & 0xff;
	objPos->yMin = (y + frame.y) & 0xff;
	objPos->xMax = objPos->xMin + frame.width;
	objPos->yMax = objPos->yMin + frame.height;
}

void DreamWebEngine::makeBackOb(SetObject *objData, uint16 x, uint16 y) {
	if (_vars._newObs == 0)
		return;
	uint8 priority = objData->priority;
	uint8 type = objData->type;
	Sprite *sprite = makeSprite(x, y, false, &_setFrames);

	sprite->_objData = objData;
	if (priority == 255)
		priority = 0;
	sprite->priority = priority;
	sprite->type = type;
	sprite->delay = 0;
	sprite->animFrame = 0;
}

void DreamWebEngine::showAllObs() {
	_setList.clear();

	const GraphicsFile &frameBase = _setFrames;
	for (size_t i = 0; i < 128; ++i) {
		SetObject *setEntry = &_setDat[i];
		uint16 x, y;
		if (getMapAd(setEntry->mapad, &x, &y) == 0)
			continue;
		uint8 currentFrame = setEntry->frames[0];
		if (currentFrame == 0xff)
			continue;
		uint8 width, height;
		ObjPos objPos;
		calcFrFrame(frameBase._frames[currentFrame], &width, &height, x, y, &objPos);
		setEntry->index = setEntry->frames[0];
		if ((setEntry->type == 0) && (setEntry->priority != 5) && (setEntry->priority != 6)) {
			x += _mapAdX;
			y += _mapAdY;
			showFrame(frameBase, x, y, currentFrame, 0);
		} else
			makeBackOb(setEntry, x, y);

		objPos.index = i;
		_setList.push_back(objPos);
	}
}

static bool addAlong(const MapFlag *mapFlags) {
	for (size_t i = 0; i < 11; ++i) {
		if (mapFlags[i]._flag != 0)
			return true;
	}
	return false;
}

static bool addLength(const MapFlag *mapFlags) {
	for (size_t i = 0; i < 10; ++i) {
		if (mapFlags[11 * i]._flag != 0)
			return true;
	}
	return false;
}

void DreamWebEngine::getDimension(uint8 *mapXstart, uint8 *mapYstart, uint8 *mapXsize, uint8 *mapYsize) {
	uint8 yStart = 0;
	while (! addAlong(_mapFlags + 11 * yStart))
		++yStart;

	uint8 xStart = 0;
	while (! addLength(_mapFlags + xStart))
		++xStart;

	uint8 yEnd = 10;
	while (! addAlong(_mapFlags + 11 * (yEnd - 1)))
		--yEnd;

	uint8 xEnd = 11;
	while (! addLength(_mapFlags + (xEnd - 1)))
		--xEnd;

	*mapXstart = xStart;
	*mapYstart = yStart;
	*mapXsize = xEnd - xStart;
	*mapYsize = yEnd - yStart;
	_mapXStart = xStart << 4;
	_mapYStart = yStart << 4;
	_mapXSize = *mapXsize << 4;
	_mapYSize = *mapYsize << 4;
}

void DreamWebEngine::calcMapAd() {
	uint8 mapXstart, mapYstart;
	uint8 mapXsize, mapYsize;
	getDimension(&mapXstart, &mapYstart, &mapXsize, &mapYsize);
	_mapAdX = _mapOffsetX - 8 * (mapXsize + 2 * mapXstart - 11);
	_mapAdY = _mapOffsetY - 8 * (mapYsize + 2 * mapYstart - 10);
}

void DreamWebEngine::showAllFree() {
	const unsigned int count = 80;

	_freeList.clear();

	const DynObject *freeObjects = _freeDat;
	const GraphicsFile &frameBase = _freeFrames;
	for (size_t i = 0; i < count; ++i) {
		uint16 x, y;
		uint8 mapAd = getMapAd(freeObjects[i].mapad, &x, &y);
		if (mapAd != 0) {
			uint8 width, height;
			ObjPos objPos;
			uint16 currentFrame = 3 * i;
			calcFrFrame(frameBase._frames[currentFrame], &width, &height, x, y, &objPos);
			if ((width != 0) || (height != 0)) {
				x += _mapAdX;
				y += _mapAdY;
				assert(currentFrame < 256);
				showFrame(frameBase, x, y, currentFrame, 0);
				objPos.index = i;
				_freeList.push_back(objPos);
			}
		}
	}
}

void DreamWebEngine::drawFlags() {
	MapFlag *mapFlag = _mapFlags;
	uint16 mapOffset = _mapY * kMapWidth + _mapX;
	const uint8 *mapData = _mapData + mapOffset;

	for (size_t i = 0; i < 10; ++i) {
		for (size_t j = 0; j < 11; ++j) {
			uint8 tile = mapData[i * kMapWidth + j];
			mapFlag->_flag = _backdropFlags[tile]._flag;
			mapFlag->_flagEx = _backdropFlags[tile]._flagEx;
			mapFlag->_type = tile;
			mapFlag++;
		}
	}
}

void DreamWebEngine::showAllEx() {
	const unsigned int count = 100;

	_exList.clear();

	DynObject *objects = _exData;
	const GraphicsFile &frameBase = _exFrames;
	for (size_t i = 0; i < count; ++i) {
		DynObject *object = objects + i;
		if (object->mapad[0] == 0xff)
			continue;
		if (object->currentLocation != _realLocation)
			continue;
		uint16 x, y;
		if (getMapAd(object->mapad, &x, &y) == 0)
			continue;
		uint8 width, height;
		ObjPos objPos;
		uint16 currentFrame = 3 * i;
		calcFrFrame(frameBase._frames[currentFrame], &width, &height, x, y, &objPos);
		if ((width != 0) || (height != 0)) {
			assert(currentFrame < 256);
			showFrame(frameBase, x + _mapAdX, y + _mapAdY, currentFrame, 0);
			objPos.index = i;
			_exList.push_back(objPos);
		}
	}
}

} // End of namespace DreamWeb
