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

#ifndef QUEEN_WALK_H
#define QUEEN_WALK_H

#include "common/util.h"
#include "queen/structs.h"

namespace Queen {

struct MovePersonAnim {
	int16 firstFrame;
	int16 lastFrame;
	Direction facing;

	void set(int16 ff, int16 lf, Direction dir) {
		firstFrame = ff;
		lastFrame = lf;
		facing = dir;
	}
};

struct WalkData {
	int16 dx, dy;
	const Area *area;
	uint16 areaNum;
	MovePersonAnim anim;
};

struct MovePersonData {
	const char *name;
	int16 walkLeft1, walkLeft2;
	int16 walkRight1, walkRight2;
	int16 walkBack1, walkBack2;
	int16 walkFront1, walkFront2;
	uint16 frontStandingFrame;
	uint16 backStandingFrame;
	uint16 animSpeed;
	uint16 moveSpeed;
};

class QueenEngine;

class Walk {
public:

	Walk(QueenEngine *vm);

	int16 moveJoe(int direction, int16 endx, int16 endy, bool inCutaway);
	int16 movePerson(const Person *pp, int16 endx, int16 endy, uint16 curImage, int direction);

	void stopJoe();
	void stopPerson(uint16 bobNum);

	enum {
		MAX_WALK_DATA = 16
	};

private:

	void animateJoePrepare();
	void animateJoe();

	void animatePersonPrepare(const MovePersonData *mpd, int direction);
	void animatePerson(const MovePersonData *mpd, uint16 image, uint16 bobNum, uint16 bankNum, int direction);

	/// compute transition coordinate
	static int16 calcC(int16 c1, int16 c2, int16 c3, int16 c4, int16 lastc);

	/// find area for position
	int16 findAreaPosition(int16 *x, int16 *y, bool recalibrate);

	/// find an area not already struck
	uint16 findFreeArea(uint16 area) const;

	/// return true if the area is already on the walking path
	bool isAreaStruck(uint16 area) const;

	/// calculates the path list from oldArea to newArea
	bool calcPath(uint16 oldArea, uint16 newArea);

	/// resets path computed in calcPath()
	void initWalkData();

	/// add an area to the path
	void incWalkData(int16 px, int16 py, int16 x, int16 y, uint16 area);

	/// compute path (and populates _walkData) from current position to the new one
	bool calc(uint16 oldPos, uint16 newPos, int16 oldx, int16 oldy, int16 x, int16 y);


	/// areas for current room
	const Area *_roomArea;

	/// number of areas for current room
	uint16 _roomAreaCount;

	/// walking steps
	WalkData _walkData[MAX_WALK_DATA];

	/// number of walking steps
	uint16 _walkDataCount;

	uint16 _areaStrike[MAX_WALK_DATA];
	uint16 _areaStrikeCount;

	uint16 _areaList[MAX_WALK_DATA];
	uint16 _areaListCount;

	/// set if stopJoe() is called
	bool _joeInterrupted;

	/// set if handleSpecialArea() is called
	bool _joeMoveBlock;

	QueenEngine *_vm;

	/// persons walking animation data
	static const MovePersonData _moveData[];
};

} // End of namespace Queen

#endif
