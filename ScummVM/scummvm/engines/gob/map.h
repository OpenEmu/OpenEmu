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

#ifndef GOB_MAP_H
#define GOB_MAP_H

#include "gob/mult.h"

namespace Gob {

enum RelativeDirection {
	kRelDirNone  = 0 ,

	kRelDirLeft  = (1 << 0),
	kRelDirUp    = (1 << 1),
	kRelDirRight = (1 << 2),
	kRelDirDown  = (1 << 3),

	kRelDirLeftUp    = kRelDirLeft  | kRelDirUp,
	kRelDirLeftDown  = kRelDirLeft  | kRelDirDown,
	kRelDirRightUp   = kRelDirRight | kRelDirUp,
	kRelDirRightDown = kRelDirRight | kRelDirDown
};

// The same numeric values are also used for the arrow keys.
enum Direction {
	kDirNone = 0x0000,
	kDirNW   = 0x4700,
	kDirN    = 0x4800,
	kDirNE   = 0x4900,
	kDirW    = 0x4B00,
	kDirE    = 0x4D00,
	kDirSW   = 0x4F00,
	kDirS    = 0x5000,
	kDirSE   = 0x5100
};

struct WayPoint {
	int16 x;
	int16 y;
	int16 notWalkable;
};

struct ItemPos {
	int8 x;
	int8 y;
	int8 orient;
};


class Map {
public:
	int16 _nearestWayPoint;
	int16 _nearestDest;

	int16 _curGoblinX;
	int16 _curGoblinY;
	int16 _destX;
	int16 _destY;

	bool _mapUnknownBool;

	ItemPos _itemPoses[40];
	char _sourceFile[15];

	Map(GobEngine *vm);
	virtual ~Map();

	uint8 getVersion() const;

	int16 getMapWidth() const;
	int16 getMapHeight() const;

	int16 getScreenWidth() const;
	int16 getScreenHeight() const;

	int16 getTilesWidth() const;
	int16 getTilesHeight() const;

	bool hasBigTiles() const;

	int8 getPass(int x, int y, int width = -1) const;
	void setPass(int x, int y, int8 pass, int width = -1);

	const WayPoint &getWayPoint(int n) const;

	void findNearestWalkable(int16 &gobDestX, int16 &gobDestY,
		int16 mouseX, int16 mouseY);

	int16 getItem(int x, int y) const;
	void setItem(int x, int y, int16 item);
	void placeItem(int16 x, int16 y, int16 id);

	Direction getDirection(int16 x0, int16 y0, int16 x1, int16 y1);

	int16 checkDirectPath(Mult::Mult_Object *obj, int16 x0,
			int16 y0, int16 x1, int16 y1);
	int16 checkLongPath(int16 x0, int16 y0,
			int16 x1, int16 y1, int16 i0, int16 i1);

	void loadMapsInitGobs();

	virtual void loadMapObjects(const char *avjFile) = 0;
	virtual void findNearestToGob(Mult::Mult_Object *obj) = 0;
	virtual void findNearestToDest(Mult::Mult_Object *obj) = 0;
	virtual void optimizePoints(Mult::Mult_Object *obj, int16 x, int16 y) = 0;

protected:
	GobEngine *_vm;

	bool _loadFromAvo;

	uint8 _mapVersion;

	int16 _mapWidth;
	int16 _mapHeight;

	int16 _screenWidth;
	int16 _screenHeight;

	int16 _tilesWidth;
	int16 _tilesHeight;

	bool _bigTiles;

	int16 _passWidth;
	int8 *_passMap; // [y * _mapWidth + x], getPass(x, y);

	int16 _wayPointCount;
	WayPoint *_wayPoints;

	int16 **_itemsMap; // [y][x]

	int16 findNearestWayPoint(int16 x, int16 y);

private:
	// Move the x and y values according to the direction
	void moveDirection(Direction dir, int16 &x, int16 &y);
};

class Map_v1 : public Map {
public:
	virtual void loadMapObjects(const char *avjFile);
	virtual void findNearestToGob(Mult::Mult_Object *obj);
	virtual void findNearestToDest(Mult::Mult_Object *obj);
	virtual void optimizePoints(Mult::Mult_Object *obj, int16 x, int16 y);

	Map_v1(GobEngine *vm);
	virtual ~Map_v1();

protected:
	void init();
	void loadSounds(Common::SeekableReadStream &data);
	void loadGoblins(Common::SeekableReadStream &data, uint32 gobsPos);
	void loadObjects(Common::SeekableReadStream &data, uint32 objsPos);
	void loadItemToObject(Common::SeekableReadStream &data);
};

class Map_v2 : public Map_v1 {
public:
	virtual void loadMapObjects(const char *avjFile);
	virtual void findNearestToGob(Mult::Mult_Object *obj);
	virtual void findNearestToDest(Mult::Mult_Object *obj);
	virtual void optimizePoints(Mult::Mult_Object *obj, int16 x, int16 y);

	Map_v2(GobEngine *vm);
	virtual ~Map_v2();

protected:
	void loadGoblinStates(Common::SeekableReadStream &data, int index);
};

} // End of namespace Gob

#endif // GOB_MAP_H
