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

// Isometric level module

#include "saga/saga.h"
#include "saga/gfx.h"
#include "saga/scene.h"
#include "saga/isomap.h"
#include "saga/render.h"

namespace Saga {

enum MaskRules {
	kMaskRuleNever = 0,
	kMaskRuleAlways,
	kMaskRuleUMIN,
	kMaskRuleUMID,
	kMaskRuleUMAX,
	kMaskRuleVMIN,
	kMaskRuleVMID,
	kMaskRuleVMAX,
	kMaskRuleYMIN,
	kMaskRuleYMID,
	kMaskRuleYMAX,
	kMaskRuleUVMAX,
	kMaskRuleUVMIN,
	kMaskRuleUorV,
	kMaskRuleUandV
};


static const IsoMap::TilePoint normalDirTable[8] = {
	{ 1, 1, 0, SAGA_DIAG_NORMAL_COST},
	{ 1, 0, 0, SAGA_STRAIGHT_NORMAL_COST},
	{ 1,-1, 0, SAGA_DIAG_NORMAL_COST},
	{ 0,-1, 0, SAGA_STRAIGHT_NORMAL_COST},
	{-1,-1, 0, SAGA_DIAG_NORMAL_COST},
	{-1, 0, 0, SAGA_STRAIGHT_NORMAL_COST},
	{-1, 1, 0, SAGA_DIAG_NORMAL_COST},
	{ 0, 1, 0, SAGA_STRAIGHT_NORMAL_COST},
};

static const IsoMap::TilePoint easyDirTable[8] = {
	{ 1, 1, 0, SAGA_DIAG_EASY_COST},
	{ 1, 0, 0, SAGA_STRAIGHT_EASY_COST},
	{ 1,-1, 0, SAGA_DIAG_EASY_COST},
	{ 0,-1, 0, SAGA_STRAIGHT_EASY_COST},
	{-1,-1, 0, SAGA_DIAG_EASY_COST},
	{-1, 0, 0, SAGA_STRAIGHT_EASY_COST},
	{-1, 1, 0, SAGA_DIAG_EASY_COST},
	{ 0, 1, 0, SAGA_STRAIGHT_EASY_COST},
};

static const IsoMap::TilePoint hardDirTable[8] = {
	{ 1, 1, 0, SAGA_DIAG_HARD_COST},
	{ 1, 0, 0, SAGA_STRAIGHT_HARD_COST},
	{ 1,-1, 0, SAGA_DIAG_HARD_COST},
	{ 0,-1, 0, SAGA_STRAIGHT_HARD_COST},
	{-1,-1, 0, SAGA_DIAG_HARD_COST},
	{-1, 0, 0, SAGA_STRAIGHT_HARD_COST},
	{-1, 1, 0, SAGA_DIAG_HARD_COST},
	{ 0, 1, 0, SAGA_STRAIGHT_HARD_COST},
};

static const int16 directions[8][2] = {
	{	16,		16},
	{	16,		0},
	{	16,		-16},
	{	0,		-16},
	{	-16,	-16},
	{	-16,	0},
	{	-16,	16},
	{	0,		16}
};

IsoMap::IsoMap(SagaEngine *vm) : _vm(vm) {
	_viewScroll.x = (128 - 8) * 16;
	_viewScroll.y = (128 - 8) * 16 - 64;
	_viewDiff = 1;
}

void IsoMap::loadImages(const ByteArray &resourceData) {
	IsoTileData *tileData;
	uint16 i;
	size_t offsetDiff;

	if (resourceData.empty()) {
		error("IsoMap::loadImages wrong resourceLength");
	}


	ByteArrayReadStreamEndian readS(resourceData, _vm->isBigEndian());
	readS.readUint16(); // skip
	i = readS.readUint16();
	i = i / SAGA_ISOTILEDATA_LEN;
	_tilesTable.resize(i);
	Common::Array<size_t> tempOffsets;
	tempOffsets.resize(_tilesTable.size());
	readS.seek(0);


	for (i = 0; i < _tilesTable.size(); i++) {
		tileData = &_tilesTable[i];
		tileData->height = readS.readByte();
		tileData->attributes = readS.readSByte();
		tempOffsets[i] = readS.readUint16();
		tileData->terrainMask = readS.readUint16();
		tileData->FGDBGDAttr = readS.readByte();
		readS.readByte(); //skip
	}

	offsetDiff = readS.pos();

	_tileData.resize(resourceData.size() - offsetDiff);
	memcpy(_tileData.getBuffer(), resourceData.getBuffer() + offsetDiff, _tileData.size());

	for (i = 0; i < _tilesTable.size(); i++) {
		_tilesTable[i].tilePointer = _tileData.getBuffer() + tempOffsets[i] - offsetDiff;
	}
}

void IsoMap::loadPlatforms(const ByteArray &resourceData) {
	TilePlatformData *tilePlatformData;
	uint16 i, x, y;

	if (resourceData.empty()) {
		error("IsoMap::loadPlatforms wrong resourceLength");
	}

	ByteArrayReadStreamEndian readS(resourceData, _vm->isBigEndian());

	i = resourceData.size() / SAGA_TILEPLATFORMDATA_LEN;
	_tilePlatformList.resize(i);

	for (i = 0; i < _tilePlatformList.size(); i++) {
		tilePlatformData = &_tilePlatformList[i];
		tilePlatformData->metaTile = readS.readSint16();
		tilePlatformData->height = readS.readSint16();
		tilePlatformData->highestPixel = readS.readSint16();
		tilePlatformData->vBits = readS.readByte();
		tilePlatformData->uBits = readS.readByte();
		for (x = 0; x < SAGA_PLATFORM_W; x++) {
			for (y = 0; y < SAGA_PLATFORM_W; y++) {
				tilePlatformData->tiles[x][y] = readS.readSint16();
			}
		}
	}

}

void IsoMap::loadMap(const ByteArray &resourceData) {
	uint16 x, y;

	if (resourceData.size() != SAGA_TILEMAP_LEN) {
		error("IsoMap::loadMap wrong resource length %d", resourceData.size());
	}

	ByteArrayReadStreamEndian readS(resourceData, _vm->isBigEndian());
	_tileMap.edgeType = readS.readByte();
	readS.readByte(); //skip

	for (x = 0; x < SAGA_TILEMAP_W; x++) {
		for (y = 0; y < SAGA_TILEMAP_H; y++) {
			_tileMap.tilePlatforms[x][y] = readS.readSint16();
		}
	}

}

void IsoMap::loadMetaTiles(const ByteArray &resourceData) {
	MetaTileData *metaTileData;
	uint16 i, j;

	if (resourceData.empty()) {
		error("IsoMap::loadMetaTiles wrong resourceLength");
	}

	ByteArrayReadStreamEndian readS(resourceData, _vm->isBigEndian());
	i = resourceData.size() / SAGA_METATILEDATA_LEN;
	_metaTileList.resize(i);

	for (i = 0; i < _metaTileList.size(); i++) {
		metaTileData = &_metaTileList[i];
		metaTileData->highestPlatform = readS.readUint16();
		metaTileData->highestPixel = readS.readUint16();
		for (j = 0; j < SAGA_MAX_PLATFORM_H; j++) {
			metaTileData->stack[j] = readS.readSint16();
		}
	}
}

void IsoMap::loadMulti(const ByteArray &resourceData) {
	MultiTileEntryData *multiTileEntryData;
	uint16 i;
	int16 offsetDiff;

	if (resourceData.size() < 2) {
		error("IsoMap::loadMetaTiles wrong resourceLength");
	}

	ByteArrayReadStreamEndian readS(resourceData, _vm->isBigEndian());
	i = readS.readUint16();
	_multiTable.resize(i);

	for (i = 0; i < _multiTable.size(); i++) {
		multiTileEntryData = &_multiTable[i];
		readS.readUint32();//skip
		multiTileEntryData->offset = readS.readSint16();
		multiTileEntryData->u = readS.readByte();
		multiTileEntryData->v = readS.readByte();
		multiTileEntryData->h = readS.readByte();
		multiTileEntryData->uSize = readS.readByte();
		multiTileEntryData->vSize = readS.readByte();
		multiTileEntryData->numStates = readS.readByte();
		multiTileEntryData->currentState = readS.readByte();
		readS.readByte();//skip
	}

	offsetDiff = (readS.pos() - 2);

	for (i = 0; i < _multiTable.size(); i++) {
		_multiTable[i].offset -= offsetDiff;
	}

	uint16 multiDataCount = (readS.size() - readS.pos()) / 2;

	_multiTableData.resize(multiDataCount);
	for (i = 0; i < _multiTableData.size(); i++) {
		_multiTableData[i] = readS.readSint16();
	}
}

void IsoMap::clear() {
	_tilesTable.clear();
	_tilePlatformList.clear();
	_metaTileList.clear();
	_multiTable.clear();
	_tileData.clear();
	_multiTableData.clear();
}

void IsoMap::adjustScroll(bool jump) {
	Point playerPoint;
	Point minScrollPos;
	Point maxScrollPos;


	tileCoordsToScreenPoint(_vm->_actor->_centerActor->_location, playerPoint);

	if (_vm->_scene->currentSceneResourceId() == ITE_SCENE_OVERMAP) {
		_mapPosition.x = (playerPoint.x + _viewScroll.x) * 30 / 100 - (381);
		_mapPosition.y = (playerPoint.y + _viewScroll.y) * 30 / 100 - (342);
	}

	if (_vm->_actor->_centerActor != _vm->_actor->_protagonist) {
		playerPoint.y -= 24;
	}
	playerPoint.y -= 28;

	playerPoint.x += _viewScroll.x - _vm->getDisplayInfo().width/2;
	playerPoint.y += _viewScroll.y - _vm->_scene->getHeight()/2;

	minScrollPos.x = playerPoint.x - SAGA_SCROLL_LIMIT_X1;
	minScrollPos.y = playerPoint.y - SAGA_SCROLL_LIMIT_Y1;

	maxScrollPos.x = playerPoint.x + SAGA_SCROLL_LIMIT_X1;
	maxScrollPos.y = playerPoint.y + SAGA_SCROLL_LIMIT_Y2;

	if (jump) {
		if (_viewScroll.y < minScrollPos.y) {
			_viewScroll.y = minScrollPos.y;
		}
		if (_viewScroll.y > maxScrollPos.y) {
			_viewScroll.y = maxScrollPos.y;
		}
		if (_viewScroll.x < minScrollPos.x) {
			_viewScroll.x = minScrollPos.x;
		}
		if (_viewScroll.x > maxScrollPos.x) {
			_viewScroll.x = maxScrollPos.x;
		}
	} else {
		_viewScroll.y = smoothSlide(_viewScroll.y, minScrollPos.y, maxScrollPos.y);
		_viewScroll.x = smoothSlide(_viewScroll.x, minScrollPos.x, maxScrollPos.x);
	}

	if (_vm->_scene->currentSceneResourceId() == ITE_SCENE_OVERMAP) {
		ObjectData *obj;
		uint16 objectId;
		objectId = _vm->_actor->objIndexToId(ITE_OBJ_MAP);
		obj = _vm->_actor->getObj(objectId);
		if (obj->_sceneNumber != ITE_SCENE_INV) {
			_viewScroll.x = 1552 + 8;
			_viewScroll.y = 1456 + 8;
		}
	}
}

int16 IsoMap::findMulti(int16 tileIndex, int16 absU, int16 absV, int16 absH) {
	MultiTileEntryData *multiTileEntryData;
	int16 ru;
	int16 rv;
	int16 mu;
	int16 mv;
	int16 state;
	uint16 i, offset;
	int16 *tiles;

	ru = (tileIndex >> 13) & 0x03;
	rv = (tileIndex >> 11) & 0x03;
	mu = absU - ru;
	mv = absV - rv;

	tileIndex = 0;
	for (i = 0; i < _multiTable.size(); i++) {
		multiTileEntryData = &_multiTable[i];

		if ((multiTileEntryData->u == mu) &&
			(multiTileEntryData->v == mv) &&
			(multiTileEntryData->h == absH)) {
			state = multiTileEntryData->currentState;

			offset = (ru + state * multiTileEntryData->uSize) * multiTileEntryData->vSize + rv;
			offset *= sizeof(int16);
			offset += multiTileEntryData->offset;
			if (offset + sizeof(int16) > _multiTableData.size() * sizeof(int16)) {
				error("wrong multiTileEntryData->offset");
			}
			tiles = (int16 *)((byte *)&_multiTableData.front() + offset);
			tileIndex = *tiles;
			if (tileIndex >= 256) {
				warning("something terrible happened");
				return 1;
			}
			return tileIndex;
		}
	}

	return 1;
}

void IsoMap::draw() {
	_tileClip = _vm->_scene->getSceneClip();
	_vm->_gfx->drawRect(_tileClip, 0);
	drawTiles(NULL);
}

void IsoMap::setMapPosition(int x, int y) {
	_mapPosition.x = x;
	_mapPosition.y = y;
}

void IsoMap::drawSprite(SpriteList &spriteList, int spriteNumber, const Location &location, const Point &screenPosition, int scale) {
	int width;
	int height;
	int xAlign;
	int yAlign;
	const byte *spriteBuffer;
	Point spritePointer;

	_vm->_sprite->getScaledSpriteBuffer(spriteList, spriteNumber, scale, width, height, xAlign, yAlign, spriteBuffer);

	spritePointer.x = screenPosition.x + xAlign;
	spritePointer.y = screenPosition.y + yAlign;

	_tileClip.left = CLIP<int>(spritePointer.x, 0, _vm->getDisplayInfo().width);
	_tileClip.right = CLIP<int>(spritePointer.x + width, 0, _vm->getDisplayInfo().width);
	_tileClip.top = CLIP<int>(spritePointer.y, 0, _vm->_scene->getHeight());
	_tileClip.bottom = CLIP<int>(spritePointer.y + height, 0, _vm->_scene->getHeight());

	_vm->_sprite->drawClip(spritePointer, width, height, spriteBuffer, true);
	drawTiles(&location);
}


void IsoMap::drawTiles(const Location *location) {
	Point view1;
	Point fineScroll;
	Point tileScroll;
	Point metaTileY;
	Point metaTileX;
	int16 u0, v0,
		  u1, v1,
		  u2, v2,
		  uc, vc;
	uint16 metaTileIndex;
	Location rLocation;
	int16 workAreaWidth;
	int16 workAreaHeight;

	tileScroll.x = _viewScroll.x >> 4;
	tileScroll.y = _viewScroll.y >> 4;

	fineScroll.x = _viewScroll.x & 0xf;
	fineScroll.y = _viewScroll.y & 0xf;

	view1.x = tileScroll.x - (8 * SAGA_TILEMAP_W);
	view1.y = (8 * SAGA_TILEMAP_W) - tileScroll.y;

	u0 = ((view1.y + 64) * 2 + view1.x) >> 4;
	v0 = ((view1.y + 64) * 2 - view1.x) >> 4;

	metaTileY.x = (u0 - v0) * 128 - (view1.x * 16 + fineScroll.x);
	metaTileY.y = (view1.y * 16 - fineScroll.y) - (u0 + v0) * 64;

	workAreaWidth = _vm->getDisplayInfo().width + 128;
	workAreaHeight = _vm->_scene->getHeight() + 128 + 80;

	for (u1 = u0, v1 = v0; metaTileY.y < workAreaHeight; u1--, v1--) {
		metaTileX = metaTileY;

		for (u2 = u1, v2 = v1; metaTileX.x < workAreaWidth; u2++, v2--, metaTileX.x += 256) {

			uc = u2 & (SAGA_TILEMAP_W - 1);
			vc = v2 & (SAGA_TILEMAP_W - 1);

			if (uc != u2 || vc != v2) {
				metaTileIndex = 0;
				switch ( _tileMap.edgeType) {
				case kEdgeTypeBlack:
					continue;
				case kEdgeTypeFill0:
					break;
				case kEdgeTypeFill1:
					metaTileIndex = 1;
					break;
				case kEdgeTypeRpt:
					uc = CLIP<int16>(u2, 0, SAGA_TILEMAP_W - 1);
					vc = CLIP<int16>(v2, 0, SAGA_TILEMAP_W - 1);
					metaTileIndex = _tileMap.tilePlatforms[uc][vc];
					break;
				case kEdgeTypeWrap:
					metaTileIndex = _tileMap.tilePlatforms[uc][vc];
					break;
				}
			} else {
				metaTileIndex = _tileMap.tilePlatforms[uc][vc];
			}

			if (location != NULL) {
				rLocation.u() = location->u() - (u2 << 7);
				rLocation.v() = location->v() - (v2 << 7);
				rLocation.z = location->z;
				drawSpriteMetaTile(metaTileIndex, metaTileX, rLocation, u2 << 3, v2 << 3);
			} else {
				drawMetaTile(metaTileIndex, metaTileX, u2 << 3, v2 << 3);
			}
		}

		metaTileY.y += 64;

		metaTileX = metaTileY;

		metaTileX.x -= 128;

		for (u2 = u1 - 1, v2 = v1; metaTileX.x < workAreaWidth; u2++, v2--, metaTileX.x += 256) {

			uc = u2 & (SAGA_TILEMAP_W - 1);
			vc = v2 & (SAGA_TILEMAP_W - 1);

			if (uc != u2 || vc != v2) {
				metaTileIndex = 0;
				switch ( _tileMap.edgeType) {
				case kEdgeTypeBlack:
					continue;
				case kEdgeTypeFill0:
					break;
				case kEdgeTypeFill1:
					metaTileIndex = 1;
					break;
				case kEdgeTypeRpt:
					uc = CLIP<int16>(u2, 0, SAGA_TILEMAP_W - 1);
					vc = CLIP<int16>(v2, 0, SAGA_TILEMAP_W - 1);
					metaTileIndex = _tileMap.tilePlatforms[uc][vc];
					break;
				case kEdgeTypeWrap:
					metaTileIndex = _tileMap.tilePlatforms[uc][vc];
					break;
				}
			} else {
				metaTileIndex = _tileMap.tilePlatforms[uc][vc];
			}

			if (location != NULL) {
				rLocation.u() = location->u() - (u2 << 7);
				rLocation.v() = location->v() - (v2 << 7);
				rLocation.z = location->z;
				drawSpriteMetaTile(metaTileIndex, metaTileX, rLocation, u2 << 3, v2 << 3);
			} else {
				drawMetaTile(metaTileIndex, metaTileX, u2 << 3, v2 << 3);
			}
		}
		metaTileY.y += 64;
	}

}

void IsoMap::drawSpriteMetaTile(uint16 metaTileIndex, const Point &point, Location &location, int16 absU, int16 absV) {
	MetaTileData * metaTile;
	uint16 high;
	int16 platformIndex;
	Point platformPoint;
	platformPoint = point;

	if (_metaTileList.size() <= metaTileIndex) {
		error("IsoMap::drawMetaTile wrong metaTileIndex");
	}

	metaTile = &_metaTileList[metaTileIndex];

	if (metaTile->highestPlatform > 18) {
		metaTile->highestPlatform = 0;
	}

	for (high = 0; high <= metaTile->highestPlatform; high++, platformPoint.y -= 8, location.z -= 8) {
		assert(SAGA_MAX_PLATFORM_H > high);
		platformIndex = metaTile->stack[high];

		if (platformIndex >= 0) {
			drawSpritePlatform(platformIndex, platformPoint, location, absU, absV, high);
		}
	}
}

void IsoMap::drawMetaTile(uint16 metaTileIndex, const Point &point, int16 absU, int16 absV) {
	MetaTileData * metaTile;
	uint16 high;
	int16 platformIndex;
	Point platformPoint;
	platformPoint = point;

	if (_metaTileList.size() <= metaTileIndex) {
		error("IsoMap::drawMetaTile wrong metaTileIndex");
	}

	metaTile = &_metaTileList[metaTileIndex];

	if (metaTile->highestPlatform > 18) {
		metaTile->highestPlatform = 0;
	}

	for (high = 0; high <= metaTile->highestPlatform; high++, platformPoint.y -= 8) {
		assert(SAGA_MAX_PLATFORM_H > high);
		platformIndex = metaTile->stack[high];

		if (platformIndex >= 0) {
			drawPlatform(platformIndex, platformPoint, absU, absV, high);
		}
	}
}

void IsoMap::drawSpritePlatform(uint16 platformIndex, const Point &point, const Location &location, int16 absU, int16 absV, int16 absH) {
	TilePlatformData *tilePlatform;
	int16 u, v;
	Point s;
	Point s0;
	uint16 tileIndex;
	Location copyLocation(location);

	if (_tilePlatformList.size() <= platformIndex) {
		error("IsoMap::drawPlatform wrong platformIndex");
	}

	tilePlatform = &_tilePlatformList[platformIndex];

	if ((point.y <= _tileClip.top) || (point.y - SAGA_MAX_TILE_H - SAGA_PLATFORM_W * SAGA_TILE_NOMINAL_H >= _tileClip.bottom)) {
		return;
	}

	s0 = point;
	s0.y -= (((SAGA_PLATFORM_W - 1) + (SAGA_PLATFORM_W - 1)) * 8);

	for (v = SAGA_PLATFORM_W - 1,
		copyLocation.v() = location.v() - ((SAGA_PLATFORM_W - 1) << 4);
		v >= 0 && s0.y - SAGA_MAX_TILE_H < _tileClip.bottom && s0.x - 128 < _tileClip.right;
		v--, copyLocation.v() += 16, s0.x += 16, s0.y += 8) {

		if ((tilePlatform->vBits & (1 << v)) == 0) {
			continue;
		}

		if (s0.x + 128 + 32 < _tileClip.left) {
			continue;
		}

		s = s0;

		for (u = SAGA_PLATFORM_W - 1,
			copyLocation.u() = location.u() - ((SAGA_PLATFORM_W - 1) << 4);
			 u >= 0 && s.x + 32 > _tileClip.left && s.y - SAGA_MAX_TILE_H < _tileClip.bottom;
			 u--, copyLocation.u() += 16, s.x -= 16, s.y += 8) {
			if (s.x < _tileClip.right && s.y > _tileClip.top) {

				tileIndex = tilePlatform->tiles[u][v];
				if (tileIndex != 0) {
					if (tileIndex & SAGA_MULTI_TILE) {
						tileIndex = findMulti(tileIndex, absU + u, absV + v, absH);
					}

					drawTile(tileIndex, s, &copyLocation);
				}
			}
		}
	}
}

void IsoMap::drawPlatform(uint16 platformIndex, const Point &point, int16 absU, int16 absV, int16 absH) {
	TilePlatformData *tilePlatform;
	int16 u, v;
	Point s;
	Point s0;
	uint16 tileIndex;

	if (_tilePlatformList.size() <= platformIndex) {
		error("IsoMap::drawPlatform wrong platformIndex");
	}

	tilePlatform = &_tilePlatformList[platformIndex];

	if ((point.y <= _tileClip.top) || (point.y - SAGA_MAX_TILE_H - SAGA_PLATFORM_W * SAGA_TILE_NOMINAL_H >= _tileClip.bottom)) {
		return;
	}

	s0 = point;
	s0.y -= (((SAGA_PLATFORM_W - 1) + (SAGA_PLATFORM_W - 1)) * 8);

	for (v = SAGA_PLATFORM_W - 1;
		v >= 0 && s0.y - SAGA_MAX_TILE_H < _tileClip.bottom && s0.x - 128 < _tileClip.right;
		v--, s0.x += 16, s0.y += 8) {

		if ((tilePlatform->vBits & (1 << v)) == 0) {
			continue;
		}

		if (s0.x + 128 + 32 < _tileClip.left) {
			continue;
		}

		s = s0;

		for (u = SAGA_PLATFORM_W - 1;
			u >= 0 && s.x + 32 > _tileClip.left && s.y - SAGA_MAX_TILE_H < _tileClip.bottom;
			u--, s.x -= 16, s.y += 8) {
			if (s.x < _tileClip.right && s.y > _tileClip.top) {

				tileIndex = tilePlatform->tiles[u][v];
				if (tileIndex > 1) {
					if (tileIndex & SAGA_MULTI_TILE) {
						tileIndex = findMulti(tileIndex, absU + u, absV + v, absH);
					}

					drawTile(tileIndex, s, NULL);
				}
			}
		}
	}
}

#define THRESH0			0
#define THRESH8			8
#define THRESH16		16

void IsoMap::drawTile(uint16 tileIndex, const Point &point, const Location *location) {
	const byte *tilePointer;
	const byte *readPointer;
	byte *drawPointer;
	Point drawPoint;
	int height;
	int widthCount = 0;
	int row, col, count, lowBound;
	int bgRunCount;
	int fgRunCount;

	if (tileIndex >= _tilesTable.size()) {
		error("IsoMap::drawTile wrong tileIndex");
	}


	if (point.x + SAGA_ISOTILE_WIDTH < _tileClip.left) {
		return;
	}

	if (point.x - SAGA_ISOTILE_WIDTH >= _tileClip.right) {
		return;
	}

	tilePointer = _tilesTable[tileIndex].tilePointer;
	height = _tilesTable[tileIndex].height;

	if ((height <= 8) || (height > 64)) {
		return;
	}

	drawPoint = point;
	drawPoint.y -= height;

	if (drawPoint.y >= _tileClip.bottom) {
		return;
	}

	if (location != NULL) {
		if (location->z <= -16) {
			if (location->z <= -48) {
				if (location->u() < -THRESH8 || location->v() < -THRESH8) {
					return;
				}
			} else {
				if (location->u() < THRESH0 || location->v() < THRESH0) {
					return;
				}
			}
		} else {
			if (location->z >= 16) {
				return;
			} else {
				switch (_tilesTable[tileIndex].GetMaskRule()) {
				case kMaskRuleNever:
					return;
				case kMaskRuleAlways:
					break;
				case kMaskRuleUMIN:
					if (location->u() < THRESH0) {
						return;
					}
					break;
				case kMaskRuleUMID:
					if (location->u() < THRESH8) {
						return;
					}
					break;
				case kMaskRuleUMAX:
					if (location->u() < THRESH16) {
						return;
					}
					break;
				case kMaskRuleVMIN:
					if (location->v() < THRESH0) {
						return;
					}
					break;
				case kMaskRuleVMID:
					if (location->v() < THRESH8) {
						return;
					}
					break;
				case kMaskRuleVMAX:
					if (location->v() < THRESH16) {
						return;
					}
					break;
				case kMaskRuleYMIN:
					if (location->uv() < THRESH0 * 2) {
						return;
					}
					break;
				case kMaskRuleYMID:
					if (location->uv() < THRESH8 * 2) {
						return;
					}
					break;
				case kMaskRuleYMAX:
					if (location->uv() < THRESH16 * 2) {
						return;
					}
					break;
				case kMaskRuleUVMAX:
					if (location->u() < THRESH16 && location->v() < THRESH16) {
						return;
					}
					break;
				case kMaskRuleUVMIN:
					if (location->u() < THRESH0 || location->v() < THRESH0) {
						return;
					}
					break;
				case kMaskRuleUorV:
					if (location->u() < THRESH8 && location->v() < THRESH8) {
						return;
					}
					break;
				case kMaskRuleUandV:
					if (location->u() < THRESH8 || location->v() < THRESH8) {
						return;
					}
					break;
				}
			}
		}
	}

	readPointer = tilePointer;
	lowBound = MIN((int)(drawPoint.y + height), (int)_tileClip.bottom);
	for (row = drawPoint.y; row < lowBound; row++) {
		widthCount = 0;
		if (row >= _tileClip.top) {
			drawPointer = _vm->_gfx->getBackBufferPixels() + drawPoint.x + (row * _vm->_gfx->getBackBufferPitch());
			col = drawPoint.x;
			for (;;) {
				bgRunCount = *readPointer++;
				widthCount += bgRunCount;
				if (widthCount >= SAGA_ISOTILE_WIDTH) {
					break;
				}

				drawPointer += bgRunCount;
				col += bgRunCount;
				fgRunCount = *readPointer++;
				widthCount += fgRunCount;

				count = 0;
				int colDiff = _tileClip.left - col;
				if (colDiff > 0) {
					if (colDiff > fgRunCount) {
						colDiff = fgRunCount;
					}
					count = colDiff;
					col += colDiff;
				}

				colDiff = _tileClip.right - col;
				if (colDiff > 0) {
					int countDiff = fgRunCount - count;
					if (colDiff > countDiff) {
						colDiff = countDiff;
					}
					if (colDiff > 0) {
						byte *dst = (byte *)(drawPointer + count);
						assert(_vm->_gfx->getBackBufferPixels() <= dst);
						assert((_vm->_gfx->getBackBufferPixels() + (_vm->getDisplayInfo().width * _vm->getDisplayInfo().height)) >= (byte *)(dst + colDiff));
						memcpy(dst, (readPointer + count), colDiff);
						col += colDiff;
					}
				}

				readPointer += fgRunCount;
				drawPointer += fgRunCount;
			}
		} else {
			for (;;) {
				bgRunCount = *readPointer++;
				widthCount += bgRunCount;
				if (widthCount >= SAGA_ISOTILE_WIDTH) {
					break;
				}

				fgRunCount = *readPointer++;
				widthCount += fgRunCount;

				readPointer += fgRunCount;
			}
		}
	}

	// Compute dirty rect
	int rectX = MAX<int>(drawPoint.x, 0);
	int rectY = MAX<int>(drawPoint.y, 0);
	int rectX2 = MIN<int>(drawPoint.x + SAGA_ISOTILE_WIDTH, _tileClip.right);
	int rectY2 = lowBound;
	_vm->_render->addDirtyRect(Common::Rect(rectX, rectY, rectX2, rectY2));
}

bool IsoMap::checkDragonPoint(int16 u, int16 v, uint16 direction) {
	DragonPathCell *pathCell;

	if ((u < 1) || (u >= SAGA_DRAGON_SEARCH_DIAMETER - 1) || (v < 1) || (v >= SAGA_DRAGON_SEARCH_DIAMETER - 1)) {
			return false;
	}

	pathCell = _dragonSearchArray.getPathCell(u, v);

	if (pathCell->visited) {
		return false;
	}

	pathCell->visited = 1;
	pathCell->direction = direction;
	return true;
}

void IsoMap::pushDragonPoint(int16 u, int16 v, uint16 direction) {
	DragonTilePoint *tilePoint;
	DragonPathCell *pathCell;

	if ((u < 1) || (u >= SAGA_DRAGON_SEARCH_DIAMETER - 1) || (v < 1) || (v >= SAGA_DRAGON_SEARCH_DIAMETER - 1)) {
			return;
	}

	pathCell = _dragonSearchArray.getPathCell(u, v);

	if (pathCell->visited) {
		return;
	}

	tilePoint = _dragonSearchArray.getQueue(_queueCount);
	_queueCount++;
	if (_queueCount >= SAGA_SEARCH_QUEUE_SIZE) {
		_queueCount = 0;
	}

	tilePoint->u = u;
	tilePoint->v = v;
	tilePoint->direction = direction;

	pathCell->visited = 1;
	pathCell->direction = direction;
}

void IsoMap::pushPoint(int16 u, int16 v, uint16 cost, uint16 direction) {
	int16 upper;
	int16 lower;
	int16 mid;
	TilePoint *tilePoint;
	PathCell *pathCell;

	upper = _queueCount;
	lower = 0;

	if ((u < 1) || (u >= SAGA_SEARCH_DIAMETER - 1) || (v < 1) || (v >= SAGA_SEARCH_DIAMETER - 1)) {
			return;
	}

	pathCell = _searchArray.getPathCell(u, v);

	if ((pathCell->visited) && (pathCell->cost <= cost)) {
		return;
	}

	if (_queueCount >= SAGA_SEARCH_QUEUE_SIZE) {
		return;
	}

	while (1) {
		mid = (upper + lower) / 2;
		tilePoint = _searchArray.getQueue(mid);

		if (upper <= lower) {
			break;
		}

		if (cost < tilePoint->cost) {
			lower = mid + 1;
		} else {
			upper = mid;
		}
	}

	if (mid < _queueCount) {
		memmove(tilePoint + 1, tilePoint, (_queueCount - mid) * sizeof (*tilePoint));
	}
	_queueCount++;

	tilePoint->u = u;
	tilePoint->v = v;
	tilePoint->cost = cost;
	tilePoint->direction = direction;

	pathCell->visited = 1;
	pathCell->direction = direction;
	pathCell->cost = cost;
}

int16 IsoMap::getTileIndex(int16 u, int16 v, int16 z) {
	int16 mtileU;
	int16 mtileV;
	int16 uc;
	int16 vc;
	int16 u0;
	int16 v0;
	int16 platformIndex;
	int16 metaTileIndex;

	mtileU = u >> 3;
	mtileV = v >> 3;
	uc = mtileU & (SAGA_TILEMAP_W - 1);
	vc = mtileV & (SAGA_TILEMAP_W - 1);
	u0 = u & (SAGA_PLATFORM_W - 1);
	v0 = v & (SAGA_PLATFORM_W - 1);

	if ((uc != mtileU) || (vc != mtileV)) {
		metaTileIndex = 0;
		switch ( _tileMap.edgeType) {
		case kEdgeTypeBlack:
			return 0;
		case kEdgeTypeFill0:
			break;
		case kEdgeTypeFill1:
			metaTileIndex = 1;
			break;
		case kEdgeTypeRpt:
			uc = CLIP<int16>(mtileU, 0, SAGA_TILEMAP_W - 1);
			vc = CLIP<int16>(mtileV, 0, SAGA_TILEMAP_W - 1);
			metaTileIndex = _tileMap.tilePlatforms[uc][vc];
			break;
		case kEdgeTypeWrap:
			metaTileIndex = _tileMap.tilePlatforms[uc][vc];
			break;
		}
	} else {
		metaTileIndex = _tileMap.tilePlatforms[uc][vc];
	}

	if (_metaTileList.size() <= (uint)metaTileIndex) {
		error("IsoMap::getTile wrong metaTileIndex");
	}

	platformIndex = _metaTileList[metaTileIndex].stack[z];
	if (platformIndex < 0) {
		return 0;
	}

	if (_tilePlatformList.size() <= (uint)platformIndex) {
		error("IsoMap::getTile wrong platformIndex");
	}

	return _tilePlatformList[platformIndex].tiles[u0][v0];
}

IsoTileData *IsoMap::getTile(int16 u, int16 v, int16 z) {
	int16 tileIndex;

	tileIndex = getTileIndex(u, v, z);

	if (tileIndex == 0) {
		return NULL;
	}

	if (tileIndex & SAGA_MULTI_TILE) {
		tileIndex = findMulti(tileIndex, u, v, z);
	}

	return &_tilesTable[tileIndex];
}

void IsoMap::testPossibleDirections(int16 u, int16 v, uint16 terraComp[8], int skipCenter) {
	IsoTileData *tile;
	uint16 fgdMask;
	uint16 bgdMask;
	uint16 mask;


	memset(terraComp, 0, 8 * sizeof(uint16));

#define FILL_MASK(index, testMask)		\
	if (mask & testMask) {				\
		terraComp[index] |= fgdMask;	\
	}									\
	if (~mask & testMask) {				\
		terraComp[index] |= bgdMask;	\
	}

#define TEST_TILE_PROLOG(offsetU, offsetV)						\
	tile = getTile(u + offsetU, v + offsetV , _platformHeight);	\
	if (tile != NULL) {											\
		fgdMask = tile->GetFGDMask();							\
		bgdMask = tile->GetBGDMask();							\
		mask = tile->terrainMask;

#define TEST_TILE_EPILOG(index)									\
	} else {													\
		if (_vm->_actor->_protagonist->_location.z > 0) {		\
			terraComp[index] = SAGA_IMPASSABLE;					\
		}														\
	}

#define TEST_TILE_END	}

	TEST_TILE_PROLOG(0, 0)
		if (skipCenter) {
			if ((mask & 0x0660) && (fgdMask & SAGA_IMPASSABLE)) {
				fgdMask = 0;
			}
			if ((~mask & 0x0660) && (bgdMask & SAGA_IMPASSABLE)) {
				bgdMask = 0;
			}
		}

		FILL_MASK(0, 0xcc00)
		FILL_MASK(1, 0x6600)
		FILL_MASK(2, 0x3300)
		FILL_MASK(3, 0x0330)
		FILL_MASK(4, 0x0033)
		FILL_MASK(5, 0x0066)
		FILL_MASK(6, 0x00cc)
		FILL_MASK(7, 0x0cc0)
	TEST_TILE_END

	TEST_TILE_PROLOG(1, 1)
		FILL_MASK(0, 0x0673)
	TEST_TILE_EPILOG(0)


	TEST_TILE_PROLOG(1, 0)
		FILL_MASK(0, 0x0008)
		FILL_MASK(1, 0x0666)
		FILL_MASK(2, 0x0001)
	TEST_TILE_EPILOG(1)


	TEST_TILE_PROLOG(1, -1)
		FILL_MASK(2, 0x06ec)
	TEST_TILE_EPILOG(2)

	TEST_TILE_PROLOG(0, 1)
		FILL_MASK(0, 0x1000)
		FILL_MASK(7, 0x0770)
		FILL_MASK(6, 0x0001)
	TEST_TILE_EPILOG(7)


	TEST_TILE_PROLOG(0, -1)
		FILL_MASK(2, 0x8000)
		FILL_MASK(3, 0x0ee0)
		FILL_MASK(4, 0x0008)
	TEST_TILE_EPILOG(3)


	TEST_TILE_PROLOG(-1, 1)
		FILL_MASK(6, 0x3670)
	TEST_TILE_EPILOG(6)


	TEST_TILE_PROLOG(-1, 0)
		FILL_MASK(6, 0x8000)
		FILL_MASK(5, 0x6660)
		FILL_MASK(4, 0x1000)
	TEST_TILE_EPILOG(5)

	TEST_TILE_PROLOG(-1, -1)
		FILL_MASK(4, 0xce60)
	TEST_TILE_EPILOG(4)
}

void IsoMap::placeOnTileMap(const Location &start, Location &result, int16 distance, uint16 direction) {
	int16 bestDistance;
	int16 bestU;
	int16 bestV;
	int16 uBase;
	int16 vBase;
	int16 u;
	int16 v;
	TilePoint tilePoint;
	uint16 dir;
	int16 dist;
	uint16 terraComp[8];
	const TilePoint *tdir;
	uint16 terrainMask;

	bestDistance = 0;


	uBase = (start.u() >> 4) - SAGA_SEARCH_CENTER;
	vBase = (start.v() >> 4) - SAGA_SEARCH_CENTER;

	bestU = SAGA_SEARCH_CENTER;
	bestV = SAGA_SEARCH_CENTER;

	_platformHeight = _vm->_actor->_protagonist->_location.z / 8;

	memset( &_searchArray, 0, sizeof(_searchArray));

	for (ActorDataArray::const_iterator actor = _vm->_actor->_actors.begin(); actor != _vm->_actor->_actors.end(); ++actor) {
		if (!actor->_inScene) continue;

		u = (actor->_location.u() >> 4) - uBase;
		v = (actor->_location.v() >> 4) - vBase;
		if ((u >= 0) && (u < SAGA_SEARCH_DIAMETER) &&
			(v >= 0) && (v < SAGA_SEARCH_DIAMETER) &&
			((u != SAGA_SEARCH_CENTER) || (v != SAGA_SEARCH_CENTER))) {
			_searchArray.getPathCell(u, v)->visited = 1;
		}
	}

	_queueCount = 0;
	pushPoint(SAGA_SEARCH_CENTER, SAGA_SEARCH_CENTER, 0, 0);

	while (_queueCount > 0) {

		_queueCount--;
		tilePoint = *_searchArray.getQueue(_queueCount);


		dist = ABS(tilePoint.u - SAGA_SEARCH_CENTER) + ABS(tilePoint.v - SAGA_SEARCH_CENTER);

		if (dist > bestDistance) {
			bestU = tilePoint.u;
			bestV = tilePoint.v;
			bestDistance = dist;

			if (dist >= distance) {
				break;
			}
		}

		testPossibleDirections(uBase + tilePoint.u, vBase + tilePoint.v, terraComp, 0);


		for (dir = 0; dir < 8; dir++) {
			terrainMask = terraComp[dir];

			if (terrainMask & SAGA_IMPASSABLE) {
				continue;
			}

			if (dir == direction) {
				tdir = &easyDirTable[ dir ];
			} else {
				if (dir + 1 == direction || dir - 1 == direction) {
					tdir = &normalDirTable[ dir ];
				} else {
					tdir = &hardDirTable[ dir ];
				}
			}

			pushPoint(tilePoint.u + tdir->u,tilePoint.v + tdir->v, tilePoint.cost + tdir->cost, dir);
		}
	}

	result.u() = ((uBase + bestU) << 4) + 8;
	result.v() = ((vBase + bestV) << 4) + 8;
}

bool IsoMap::findNearestChasm(int16 &u0, int16 &v0, uint16 &direction) {
	int16 u, v;
	uint16 i;
	u = u0;
	v = v0;

	for (i = 1; i < 5; i++) {
		if (getTile( u - i, v, 6) == NULL) {
			u0 = u - i - 1;
			v0 = v;
			direction = kDirDownLeft;
			return true;
		}

		if (getTile( u, v - i, 6) == NULL) {
			u0 = u;
			v0 = v - i - 1;
			direction = kDirDownRight;
			return true;
		}

		if (getTile( u - i, v - i, 6) == NULL) {
			u0 = u - i - 1;
			v0 = v - i - 1;
			direction = kDirDown;
			return true;
		}

		if (getTile( u + i, v - i, 6) == NULL) {
			u0 = u + i + 1;
			v0 = v - i - 1;
			direction = kDirDownRight;
			return true;
		}

		if (getTile( u - i, v + i, 6) == NULL) {
			u0 = u + i + 1;
			v0 = v - i - 1;
			direction = kDirLeft;
			return true;
		}
	}

	for (i = 1; i < 5; i++) {
		if (getTile( u + i, v, 6) == NULL) {
			u0 = u + i + 1;
			v0 = v;
			direction = kDirUpRight;
			return true;
		}

		if (getTile( u, v + i, 6) == NULL) {
			u0 = u;
			v0 = v + i + 1;
			direction = kDirUpLeft;
			return true;
		}

		if (getTile( u + i, v + i, 6) == NULL) {
			u0 = u + i + 1;
			v0 = v + i + 1;
			direction = kDirUp;
			return true;
		}
	}
	return false;
}

void IsoMap::findDragonTilePath(ActorData* actor,const Location &start, const Location &end, uint16 initialDirection) {
	byte *res;
	int i;
	int16 u;
	int16 v;
	int16 u1;
	int16 v1;
	uint16 dir;

	int16 bestDistance;
	int16 bestU;
	int16 bestV;

	int16 uBase;
	int16 vBase;
	int16 uFinish;
	int16 vFinish;
	DragonPathCell *pcell;
	IsoTileData *tile;
	uint16 mask;
	DragonTilePoint *tilePoint;

	int16 dist;
	bool first;

	bestDistance = SAGA_DRAGON_SEARCH_DIAMETER;
	bestU = SAGA_DRAGON_SEARCH_CENTER,
	bestV = SAGA_DRAGON_SEARCH_CENTER;

	uBase = (start.u() >> 4) - SAGA_DRAGON_SEARCH_CENTER;
	vBase = (start.v() >> 4) - SAGA_DRAGON_SEARCH_CENTER;
	uFinish = (end.u() >> 4) - uBase;
	vFinish = (end.v() >> 4) - vBase;

	_platformHeight = _vm->_actor->_protagonist->_location.z / 8;

	memset( &_dragonSearchArray, 0, sizeof(_dragonSearchArray));

	for (u = 0; u < SAGA_DRAGON_SEARCH_DIAMETER; u++) {
		for (v = 0; v < SAGA_DRAGON_SEARCH_DIAMETER; v++) {

			pcell = _dragonSearchArray.getPathCell(u, v);

			u1 = uBase + u;
			v1 = vBase + v;

			if ((u1 > 127) || (u1 < 48) || (v1 > 127) || (v1 < 0)) {
				pcell->visited = 1;
				continue;
			}

			tile = getTile(u1, v1, _platformHeight);
			if (tile != NULL) {
				mask = tile->terrainMask;
				if (((mask != 0) && (tile->GetFGDAttr() >= kTerrBlock)) ||
					((mask != 0xFFFF) && (tile->GetBGDAttr() >= kTerrBlock))) {
					pcell->visited = 1;
				}
			} else {
				pcell->visited = 1;
			}
		}
	}

	first = true;
	_queueCount = _readCount = 0;
	pushDragonPoint( SAGA_DRAGON_SEARCH_CENTER, SAGA_DRAGON_SEARCH_CENTER, initialDirection);

	while (_queueCount != _readCount) {

		tilePoint = _dragonSearchArray.getQueue(_readCount++);
		if (_readCount >= SAGA_SEARCH_QUEUE_SIZE) {
			_readCount = 0;
		}


		dist = ABS(tilePoint->u - uFinish) + ABS(tilePoint->v - vFinish);

		if (dist < bestDistance) {

			bestU = tilePoint->u;
			bestV = tilePoint->v;
			bestDistance = dist;
			if (dist == 0) {
				break;
			}
		}

		switch (tilePoint->direction) {
			case kDirUpRight:
				if (checkDragonPoint( tilePoint->u + 1, tilePoint->v + 0, kDirUpRight)) {
					pushDragonPoint( tilePoint->u + 2, tilePoint->v + 0, kDirUpRight);
					pushDragonPoint( tilePoint->u + 1, tilePoint->v + 1, kDirUpLeft);
					pushDragonPoint( tilePoint->u + 1, tilePoint->v - 1, kDirDownRight);
				}
				break;
			case kDirDownRight:
				if (checkDragonPoint( tilePoint->u + 0, tilePoint->v - 1, kDirDownRight)) {
					pushDragonPoint( tilePoint->u + 0, tilePoint->v - 2, kDirDownRight);
					pushDragonPoint( tilePoint->u + 1, tilePoint->v - 1, kDirUpRight);
					pushDragonPoint( tilePoint->u - 1, tilePoint->v - 1, kDirDownLeft);
				}
				break;
			case kDirDownLeft:
				if (checkDragonPoint( tilePoint->u - 1, tilePoint->v + 0, kDirDownLeft)) {
					pushDragonPoint( tilePoint->u - 2, tilePoint->v + 0, kDirDownLeft);
					pushDragonPoint( tilePoint->u - 1, tilePoint->v - 1, kDirDownRight);
					pushDragonPoint( tilePoint->u - 1, tilePoint->v + 1, kDirUpLeft);
				}
				break;
			case kDirUpLeft:
				if (checkDragonPoint( tilePoint->u + 0, tilePoint->v + 1, kDirUpLeft)) {
					pushDragonPoint( tilePoint->u + 0, tilePoint->v + 2, kDirUpLeft);
					pushDragonPoint( tilePoint->u - 1, tilePoint->v + 1, kDirDownLeft);
					pushDragonPoint( tilePoint->u + 1, tilePoint->v + 1, kDirUpRight);
				}
				break;
		}

		if (first && (_queueCount == _readCount)) {
			pushDragonPoint( tilePoint->u + 1, tilePoint->v + 0, kDirUpRight);
			pushDragonPoint( tilePoint->u + 0, tilePoint->v - 1, kDirDownRight);
			pushDragonPoint( tilePoint->u - 1, tilePoint->v + 0, kDirDownLeft);
			pushDragonPoint( tilePoint->u + 0, tilePoint->v + 1, kDirUpLeft);
		}
		first = false;
	}

	res = &_pathDirections[SAGA_MAX_PATH_DIRECTIONS];
	i = 0;
	while ((bestU != SAGA_DRAGON_SEARCH_CENTER) || (bestV != SAGA_DRAGON_SEARCH_CENTER)) {
		pcell = _dragonSearchArray.getPathCell(bestU, bestV);

		*--res = pcell->direction;
		i++;
		if (i >= SAGA_MAX_PATH_DIRECTIONS) {
			break;
		}

		dir = (pcell->direction + 4) & 0x07;

		bestU += normalDirTable[dir].u;
		bestV += normalDirTable[dir].v;
	}

/*	if (i > 64) {
		i = 64;
	}*/

	actor->_walkStepsCount = i;
	if (i) {
		actor->_tileDirections.resize(i);
		memcpy(&actor->_tileDirections.front(), res, i);
	}

}

void IsoMap::findTilePath(ActorData* actor, const Location &start, const Location &end) {
	int i;
	int16 u;
	int16 v;
	int16 bestDistance;
	int16 bestU;
	int16 bestV;

	int16 uBase;
	int16 vBase;
	int16 uFinish;
	int16 vFinish;

	TilePoint tilePoint;
	uint16 dir;
	int16 dist;
	uint16 terraComp[8];
	const TilePoint *tdir;
	uint16 terrainMask;
	const PathCell *pcell;
	byte *res;


	bestDistance = SAGA_SEARCH_DIAMETER;
	bestU = SAGA_SEARCH_CENTER,
	bestV = SAGA_SEARCH_CENTER;

	uBase = (start.u() >> 4) - SAGA_SEARCH_CENTER;
	vBase = (start.v() >> 4) - SAGA_SEARCH_CENTER;
	uFinish = (end.u() >> 4) - uBase;
	vFinish = (end.v() >> 4) - vBase;

	_platformHeight = _vm->_actor->_protagonist->_location.z / 8;



	memset( &_searchArray, 0, sizeof(_searchArray));

	if (!(actor->_actorFlags & kActorNoCollide) &&
		(_vm->_scene->currentSceneResourceId() != ITE_SCENE_OVERMAP)) {
			for (ActorDataArray::const_iterator other = _vm->_actor->_actors.begin(); other != _vm->_actor->_actors.end(); ++other) {
				if (!other->_inScene) continue;
				if (other->_id == actor->_id) continue;

				u = (other->_location.u() >> 4) - uBase;
				v = (other->_location.v() >> 4) - vBase;
				if ((u >= 1) && (u < SAGA_SEARCH_DIAMETER) &&
					(v >= 1) && (v < SAGA_SEARCH_DIAMETER) &&
					((u != SAGA_SEARCH_CENTER) || (v != SAGA_SEARCH_CENTER))) {
						_searchArray.getPathCell(u, v)->visited = 1;
					}
			}
		}

	_queueCount = 0;
	pushPoint(SAGA_SEARCH_CENTER, SAGA_SEARCH_CENTER, 0, 0);


	while (_queueCount > 0) {

		_queueCount--;
		tilePoint = *_searchArray.getQueue(_queueCount);

		if (tilePoint.cost > 100 && actor == _vm->_actor->_protagonist) continue;

		dist = ABS(tilePoint.u - uFinish) + ABS(tilePoint.v - vFinish);

		if (dist < bestDistance) {
			bestU = tilePoint.u;
			bestV = tilePoint.v;
			bestDistance = dist;

			if (dist == 0) {
				break;
			}
		}

		testPossibleDirections(uBase + tilePoint.u, vBase + tilePoint.v, terraComp,
			(tilePoint.u == SAGA_SEARCH_CENTER && tilePoint.v == SAGA_SEARCH_CENTER));

		for (dir = 0; dir < 8; dir++) {
			terrainMask = terraComp[dir];

			if (terrainMask & SAGA_IMPASSABLE) {
				continue;
			} else {
				if (terrainMask & (1 << kTerrRough)) {
					tdir = &hardDirTable[ dir ];
				} else {
					if (terrainMask & (1 << kTerrNone)) {
						tdir = &normalDirTable[ dir ];
					} else {
						tdir = &easyDirTable[ dir ];
					}
				}
			}


			pushPoint(tilePoint.u + tdir->u, tilePoint.v + tdir->v, tilePoint.cost + tdir->cost, dir);
		}
	}

	res = &_pathDirections[SAGA_MAX_PATH_DIRECTIONS];
	i = 0;
	while ((bestU != SAGA_SEARCH_CENTER) || (bestV != SAGA_SEARCH_CENTER)) {
		pcell = _searchArray.getPathCell(bestU, bestV);

		*--res = pcell->direction;
		i++;
		if (i >= SAGA_MAX_PATH_DIRECTIONS) {
			break;
		}

		dir = (pcell->direction + 4) & 0x07;

		bestU += normalDirTable[dir].u;
		bestV += normalDirTable[dir].v;
	}

/*	if (i > 64) {
		i = 64;
	}*/

	actor->_walkStepsCount = i;
	if (i) {
		actor->_tileDirections.resize(i);
		memcpy(&actor->_tileDirections.front(), res, i);
	}
}

void IsoMap::setTileDoorState(int doorNumber, int doorState) {
	MultiTileEntryData *multiTileEntryData;

	if ((doorNumber < 0) || ((uint)doorNumber >= _multiTable.size())) {
		error("setTileDoorState: doorNumber >= _multiTable.size()");
	}

	multiTileEntryData = &_multiTable[doorNumber];
	multiTileEntryData->currentState = doorState;
}

bool IsoMap::nextTileTarget(ActorData* actor) {
	uint16 dir;

	if (actor->_walkStepIndex >= actor->_walkStepsCount) {
		return false;
	}


	actor->_actionDirection = dir = actor->_tileDirections[actor->_walkStepIndex++];

	actor->_partialTarget.u() =
		(actor->_location.u() & ~0x0f) + 8 + directions[dir][0];

	actor->_partialTarget.v() =
		(actor->_location.v() & ~0x0f) + 8 + directions[dir][1];


	if (dir == 0) {
		actor->_facingDirection = kDirUp;
	} else {
		if (dir == 4) {
			actor->_facingDirection = kDirDown;
		} else {
			if (dir < 4) {
				actor->_facingDirection = kDirRight;
			} else {
				actor->_facingDirection = kDirLeft;
			}
		}
	}

	return true;
}

void IsoMap::screenPointToTileCoords(const Point &position, Location &location) {
	Point mPos(position);
	int x,y;

	if (_vm->_scene->currentSceneResourceId() == ITE_SCENE_OVERMAP){
		if (mPos.y < 16) {
			mPos.y = 16;
		}
	}

	x = mPos.x + _viewScroll.x - (128 * SAGA_TILEMAP_W) - 16;
	y = mPos.y + _viewScroll.y - (128 * SAGA_TILEMAP_W) + _vm->_actor->_protagonist->_location.z;

	location.u() = (x - y * 2) >> 1;
	location.v() = - (x + y * 2) >> 1;
	location.z = _vm->_actor->_protagonist->_location.z;
}

} // End of namespace Saga
