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

#ifndef SWORD1_ROUTER_H
#define SWORD1_ROUTER_H

#include "sword1/object.h"

namespace Sword1 {

#include "common/pack-start.h"  // START STRUCT PACKING

struct BarData {
	int16 x1;
	int16 y1;
	int16 x2;
	int16 y2;
	int16 xmin;
	int16 ymin;
	int16 xmax;
	int16 ymax;
	int16 dx;   // x2 - x1
	int16 dy;   // y2 - y1
	int32 co;   // co = (y1*dx) - (x1*dy) from an equation for a line y*dx = x*dy + co
} PACKED_STRUCT;

struct NodeData {
	int16 x;
	int16 y;
	int16 level;
	int16 prev;
	int16 dist;
} PACKED_STRUCT;

#include "common/pack-end.h"    // END STRUCT PACKING

struct FloorData {
	int32       nbars;
	BarData     *bars;
	int32       nnodes;
	NodeData    *node;
};

struct RouteData {
	int32 x;
	int32 y;
	int32 dirS;
	int32 dirD;
};

struct PathData {
	int32 x;
	int32 y;
	int32 dir;
	int32 num;
};

#define MAX_FRAMES_PER_CYCLE 16
#define NO_DIRECTIONS 8
#define MAX_FRAMES_PER_CHAR (MAX_FRAMES_PER_CYCLE * NO_DIRECTIONS)
#define ROUTE_END_FLAG 255

#define O_GRID_SIZE 200
#define O_ROUTE_SIZE 50

class ObjectMan;
class ResMan;
class Screen;

extern int whatTarget(int32 startX, int32 startY, int32 destX, int32 destY);

class Router {
public:
	Router(ObjectMan *pObjMan, ResMan *pResMan);
	int32 routeFinder(int32 id, Object *mega, int32 x, int32 y, int32 dir);
	void setPlayerTarget(int32 x, int32 y, int32 dir, int32 stance);

	// these should be private but are read by Screen for debugging:
	BarData   _bars[O_GRID_SIZE];
	NodeData  _node[O_GRID_SIZE];

	int32 _nBars;
	int32 _nNodes;

private:
	// when the player collides with another mega, we'll receive a ReRouteRequest here.
	// that's why we need to remember the player's target coordinates
	int32 _playerTargetX, _playerTargetY, _playerTargetDir, _playerTargetStance;

	ObjectMan *_objMan;
	ResMan *_resMan;

	int32 _startX, _startY, _startDir;
	int32 _targetX, _targetY, _targetDir;
	int32 _scaleA, _scaleB;

	int32 megaId;

	RouteData   _route[O_ROUTE_SIZE];
	PathData    _smoothPath[O_ROUTE_SIZE];
	PathData    _modularPath[O_ROUTE_SIZE];
	int32       _routeLength;

	int32       _framesPerStep, _framesPerChar;
	uint8       _nWalkFrames, _nTurnFrames;
	int32       _dx[NO_DIRECTIONS + MAX_FRAMES_PER_CHAR];
	int32       _dy[NO_DIRECTIONS + MAX_FRAMES_PER_CHAR];
	int32       _modX[NO_DIRECTIONS];
	int32       _modY[NO_DIRECTIONS];
	int32       _diagonalx, _diagonaly;
	int32       standFrames;
	int32       turnFramesLeft, turnFramesRight;
	int32       walkFramesLeft, walkFramesRight; // left/right walking turn
	int32       slowInFrames, slowOutFrames;

	bool        _slidyWalkAnimatorState;

	int32 LoadWalkResources(Object *mega, int32 x, int32 y, int32 dir);
	int32 getRoute();
	int32 checkTarget(int32 x, int32 y);

	bool scan(int32 level);
	int32 newCheck(int32 status, int32 x1, int32 x2, int32 y1, int32 y2);
	bool check(int32 x1, int32 y1, int32 x2, int32 y2);
	bool horizCheck(int32 x1, int32 y, int32 x2);
	bool vertCheck(int32 x, int32 y1, int32 y2);
	bool lineCheck(int32 x1, int32 y1, int32 x2, int32 y2);

	void extractRoute();

	void slidyPath();
	void slidyWalkAnimator(WalkData *walkAnim);

	int32 smoothestPath();
	void smoothCheck(int32 &steps, int32 best, int32 p, int32 dirS, int32 dirD);

	void solidPath();
	int32 solidWalkAnimator(WalkData *walkAnim);
};

} // End of namespace Sword1

#endif //BSROUTER_H
