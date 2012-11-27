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

#include "agi/agi.h"
#include "common/random.h"

namespace Agi {

int AgiEngine::checkStep(int delta, int step) {
	return (-step >= delta) ? 0 : (step <= delta) ? 2 : 1;
}

int AgiEngine::checkBlock(int x, int y) {
	if (x <= _game.block.x1 || x >= _game.block.x2)
		return false;

	if (y <= _game.block.y1 || y >= _game.block.y2)
		return false;

	return true;
}

void AgiEngine::changePos(VtEntry *v) {
	int b, x, y;
	int dx[9] = { 0, 0, 1, 1, 1, 0, -1, -1, -1 };
	int dy[9] = { 0, -1, -1, 0, 1, 1, 1, 0, -1 };

	x = v->xPos;
	y = v->yPos;
	b = checkBlock(x, y);

	x += v->stepSize * dx[v->direction];
	y += v->stepSize * dy[v->direction];

	if (checkBlock(x, y) == b) {
		v->flags &= ~fMotion;
	} else {
		v->flags |= fMotion;
		v->direction = 0;
		if (isEgoView(v))
			_game.vars[vEgoDir] = 0;
	}
}

void AgiEngine::motionWander(VtEntry *v) {
	if (v->parm1--) {
		if (~v->flags & fDidntMove)
			return;
	}

	v->direction = _rnd->getRandomNumber(8);

	if (isEgoView(v)) {
		_game.vars[vEgoDir] = v->direction;
		while (v->parm1 < 6) {
			v->parm1 = _rnd->getRandomNumber(50);	// huh?
		}
	}
}

void AgiEngine::motionFollowEgo(VtEntry *v) {
	int egoX, egoY;
	int objX, objY;
	int dir;

	egoX = _game.viewTable[0].xPos + _game.viewTable[0].xSize / 2;
	egoY = _game.viewTable[0].yPos;

	objX = v->xPos + v->xSize / 2;
	objY = v->yPos;

	// Get direction to reach ego
	dir = getDirection(objX, objY, egoX, egoY, v->parm1);

	// Already at ego coordinates
	if (dir == 0) {
		v->direction = 0;
		v->motion = kMotionNormal;
		setflag(v->parm2, true);
		return;
	}

	if (v->parm3 == 0xff) {
		v->parm3 = 0;
	} else if (v->flags & fDidntMove) {
		int d;

		while ((v->direction = _rnd->getRandomNumber(8)) == 0) {
		}

		d = (ABS(egoY - objY) + ABS(egoX - objX)) / 2;

		if (d < v->stepSize) {
			v->parm3 = v->stepSize;
			return;
		}

		while ((v->parm3 = _rnd->getRandomNumber(d)) < v->stepSize) {
		}
		return;
	}

	if (v->parm3 != 0) {
		int k;

		// DF: this is ugly and I dont know why this works, but
		// other line does not! (watcom complained about lvalue)
		//
		// if (((int8)v->parm3 -= v->step_size) < 0)
		//      v->parm3 = 0;

		k = v->parm3;
		k -= v->stepSize;
		v->parm3 = k;

		if ((int8) v->parm3 < 0)
			v->parm3 = 0;
	} else {
		v->direction = dir;
	}
}

void AgiEngine::motionMoveObj(VtEntry *v) {
	v->direction = getDirection(v->xPos, v->yPos, v->parm1, v->parm2, v->stepSize);

	// Update V6 if ego
	if (isEgoView(v))
		_game.vars[vEgoDir] = v->direction;

	if (v->direction == 0)
		inDestination(v);
}

void AgiEngine::checkMotion(VtEntry *v) {
	switch (v->motion) {
	case kMotionNormal:
		break;
	case kMotionWander:
		motionWander(v);
		break;
	case kMotionFollowEgo:
		motionFollowEgo(v);
		break;
	case kMotionMoveObj:
		motionMoveObj(v);
		break;
	}

	if ((_game.block.active && (~v->flags & fIgnoreBlocks)) && v->direction)
		changePos(v);
}

/*
 * Public functions
 */

/**
 *
 */
void AgiEngine::checkAllMotions() {
	VtEntry *v;

	for (v = _game.viewTable; v < &_game.viewTable[MAX_VIEWTABLE]; v++) {
		if ((v->flags & (fAnimated | fUpdate | fDrawn)) == (fAnimated | fUpdate | fDrawn)
				&& v->stepTimeCount == 1) {
			checkMotion(v);
		}
	}
}

/**
 * Check if given entry is at destination point.
 * This function is used to updated the flags of an object with move.obj
 * type motion that * has reached its final destination coordinates.
 * @param  v  Pointer to view table entry
 */
void AgiEngine::inDestination(VtEntry *v) {
	if (v->motion == kMotionMoveObj) {
		v->stepSize = v->parm3;
		setflag(v->parm4, true);
	}
	v->motion = kMotionNormal;
	if (isEgoView(v))
		_game.playerControl = true;
}

/**
 * Wrapper for static function motion_moveobj().
 * This function is used by cmd_move_object() in the first motion cycle
 * after setting the motion mode to kMotionMoveObj.
 * @param  v  Pointer to view table entry
 */
void AgiEngine::moveObj(VtEntry *v) {
	motionMoveObj(v);
}

/**
 * Get direction from motion coordinates
 * This function gets the motion direction from the current and previous
 * object coordinates and the step size.
 * @param  x0  Original x coordinate of the object
 * @param  y0  Original y coordinate of the object
 * @param  x   x coordinate of the object
 * @param  y   y coordinate of the object
 * @param  s   step size
 */
int AgiEngine::getDirection(int x0, int y0, int x, int y, int s) {
	int dirTable[9] = { 8, 1, 2, 7, 0, 3, 6, 5, 4 };
	return dirTable[checkStep(x - x0, s) + 3 * checkStep(y - y0, s)];
}

} // End of namespace Agi
