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

// Isometric level module - private header

#ifndef SAGA_ISOMAP_H
#define SAGA_ISOMAP_H

#include "saga/actor.h"

namespace Saga {

#define ITE_OBJ_MAP 14

#define SAGA_ISOTILEDATA_LEN 8
#define SAGA_ISOTILE_WIDTH 32
#define SAGA_ISOTILE_BASEHEIGHT 15
#define SAGA_TILE_NOMINAL_H 16
#define SAGA_MAX_TILE_H 64

#define SAGA_TILEPLATFORMDATA_LEN 136
#define SAGA_PLATFORM_W 8
#define SAGA_MAX_PLATFORM_H 16

#define SAGA_TILEMAP_LEN 514
#define SAGA_TILEMAP_W 16
#define SAGA_TILEMAP_H 16

#define SAGA_METATILEDATA_LEN 36

#define SAGA_MULTI_TILE (1 << 15)

#define SAGA_SCROLL_LIMIT_X1 32
#define SAGA_SCROLL_LIMIT_X2 64
#define SAGA_SCROLL_LIMIT_Y1 8
#define SAGA_SCROLL_LIMIT_Y2 32

#define SAGA_DRAGON_SEARCH_CENTER     24
#define SAGA_DRAGON_SEARCH_DIAMETER   (SAGA_DRAGON_SEARCH_CENTER * 2)

#define SAGA_SEARCH_CENTER     15
#define SAGA_SEARCH_DIAMETER   (SAGA_SEARCH_CENTER * 2)
#define SAGA_SEARCH_QUEUE_SIZE 128
#define SAGA_IMPASSABLE                              ((1 << kTerrBlock) | (1 << kTerrWater))

#define SAGA_STRAIGHT_NORMAL_COST			4
#define SAGA_DIAG_NORMAL_COST				6

#define SAGA_STRAIGHT_EASY_COST				2
#define SAGA_DIAG_EASY_COST					3

#define SAGA_STRAIGHT_HARD_COST				9
#define SAGA_DIAG_HARD_COST					10
#define SAGA_MAX_PATH_DIRECTIONS			256

enum TerrainTypes {
	kTerrNone	= 0,
	kTerrPath	= 1,
	kTerrRough	= 2,
	kTerrBlock	= 3,
	kTerrWater	= 4,
	kTerrLast	= 5
};

enum TileMapEdgeType {
	kEdgeTypeBlack	= 0,
	kEdgeTypeFill0	= 1,
	kEdgeTypeFill1	= 2,
	kEdgeTypeRpt	= 3,
	kEdgeTypeWrap	= 4
};

struct IsoTileData {
	byte height;
	int8 attributes;
	byte *tilePointer;
	uint16 terrainMask;
	byte FGDBGDAttr;
	int8 GetMaskRule() const {
		return attributes & 0x0F;
	}
	byte GetFGDAttr() const {
		return FGDBGDAttr >> 4;
	}
	byte GetBGDAttr() const {
		return FGDBGDAttr & 0x0F;
	}
	uint16 GetFGDMask() const {
		return 1 << GetFGDAttr();
	}
	uint16 GetBGDMask() const {
		return 1 << GetBGDAttr();
	}
};

struct TilePlatformData {
	int16 metaTile;
	int16 height;
	int16 highestPixel;
	byte vBits;
	byte uBits;
	int16 tiles[SAGA_PLATFORM_W][SAGA_PLATFORM_W];
};

struct TileMapData {
	byte edgeType;
	int16 tilePlatforms[SAGA_TILEMAP_W][SAGA_TILEMAP_H];
};

struct MetaTileData {
	uint16 highestPlatform;
	uint16 highestPixel;
	int16 stack[SAGA_MAX_PLATFORM_H];
};

struct MultiTileEntryData {
	int16 offset;
	byte u;
	byte v;
	byte h;
	byte uSize;
	byte vSize;
	byte numStates;
	byte currentState;
};





class IsoMap {
public:
	IsoMap(SagaEngine *vm);
	~IsoMap() {
	}
	void loadImages(const ByteArray &resourceData);
	void loadMap(const ByteArray &resourceData);
	void loadPlatforms(const ByteArray &resourceData);
	void loadMetaTiles(const ByteArray &resourceData);
	void loadMulti(const ByteArray &resourceData);
	void clear();
	void draw();
	void drawSprite(SpriteList &spriteList, int spriteNumber, const Location &location, const Point &screenPosition, int scale);
	void adjustScroll(bool jump);
	void tileCoordsToScreenPoint(const Location &location, Point &position) {
		position.x = location.u() - location.v() + (128 * SAGA_TILEMAP_W) - _viewScroll.x + 16;
		position.y = -(location.uv() >> 1) + (128 * SAGA_TILEMAP_W) - _viewScroll.y - location.z;
	}
	void screenPointToTileCoords(const Point &position, Location &location);
	void placeOnTileMap(const Location &start, Location &result, int16 distance, uint16 direction);
	void findDragonTilePath(ActorData* actor, const Location &start, const Location &end, uint16 initialDirection);
	bool findNearestChasm(int16 &u0, int16 &v0, uint16 &direction);
	void findTilePath(ActorData* actor, const Location &start, const Location &end);
	bool nextTileTarget(ActorData* actor);
	void setTileDoorState(int doorNumber, int doorState);
	Point getMapPosition() { return _mapPosition; }
	void setMapPosition(int x, int y);
	int16 getTileIndex(int16 u, int16 v, int16 z);

private:
	void drawTiles(const Location *location);
	void drawMetaTile(uint16 metaTileIndex, const Point &point, int16 absU, int16 absV);
	void drawSpriteMetaTile(uint16 metaTileIndex, const Point &point, Location &location, int16 absU, int16 absV);
	void drawPlatform(uint16 platformIndex, const Point &point, int16 absU, int16 absV, int16 absH);
	void drawSpritePlatform(uint16 platformIndex, const Point &point, const Location &location, int16 absU, int16 absV, int16 absH);
	void drawTile(uint16 tileIndex, const Point &point, const Location *location);
	int16 smoothSlide(int16 value, int16 min, int16 max) {
		if (value < min) {
			if (value < min - 100 || value > min - 4) {
				value = min;
			} else {
				value += 4;
			}
		} else {
			if (value > max) {
				if (value > max + 100 || value < max + 4) {
					value = max;
				} else {
					value -= 4;
				}
			}
		}
		return value;
	}
	int16 findMulti(int16 tileIndex, int16 absU, int16 absV, int16 absH);
	void pushPoint(int16 u, int16 v, uint16 cost, uint16 direction);
	void pushDragonPoint(int16 u, int16 v, uint16 direction);
	bool checkDragonPoint(int16 u, int16 v, uint16 direction);
	void testPossibleDirections(int16 u, int16 v, uint16 terraComp[8], int skipCenter);
	IsoTileData *getTile(int16 u, int16 v, int16 z);


	ByteArray _tileData;
	Common::Array<IsoTileData> _tilesTable;

	Common::Array<TilePlatformData> _tilePlatformList;
	Common::Array<MetaTileData> _metaTileList;

	Common::Array<MultiTileEntryData> _multiTable;
	Common::Array<int16> _multiTableData;

	TileMapData _tileMap;

	Point _mapPosition;

// path finding stuff
	uint16 _platformHeight;

	struct DragonPathCell {
		uint8 visited:1,direction:3;
	};
	struct DragonTilePoint {
		int8 u, v;
		uint8 direction:4;
	};
	struct PathCell {
		uint16 visited:1,direction:3,cost:12;
	};

public:
	struct TilePoint {
		int8 u, v;
		uint16 direction:4,cost:12;
	};

private:
	struct DragonSearchArray {
		DragonPathCell cell[SAGA_DRAGON_SEARCH_DIAMETER][SAGA_DRAGON_SEARCH_DIAMETER];
		DragonTilePoint queue[SAGA_SEARCH_QUEUE_SIZE];
		DragonTilePoint *getQueue(uint16 i) {
			assert(i < SAGA_SEARCH_QUEUE_SIZE);
			return &queue[i];
		}
		DragonPathCell *getPathCell(uint16 u, uint16 v) {
			assert((u < SAGA_DRAGON_SEARCH_DIAMETER) && (v < SAGA_DRAGON_SEARCH_DIAMETER));
			return &cell[u][v];
		}
	};
	struct SearchArray {
		PathCell cell[SAGA_SEARCH_DIAMETER][SAGA_SEARCH_DIAMETER];
		TilePoint queue[SAGA_SEARCH_QUEUE_SIZE];
		TilePoint *getQueue(uint16 i) {
			assert(i < SAGA_SEARCH_QUEUE_SIZE);
			return &queue[i];
		}
		PathCell *getPathCell(uint16 u, uint16 v) {
			assert((u < SAGA_SEARCH_DIAMETER) && (v < SAGA_SEARCH_DIAMETER));
			return &cell[u][v];
		}
	};

	int16 _queueCount;
	int16 _readCount;
	SearchArray _searchArray;
	DragonSearchArray _dragonSearchArray;
	byte _pathDirections[SAGA_MAX_PATH_DIRECTIONS];


	int _viewDiff;
	Point _viewScroll;
	Rect _tileClip;

	SagaEngine *_vm;
};

} // End of namespace Saga

#endif
