/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "common/memstream.h"
#include "common/textconsole.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/logic.h"
#include "sword2/resman.h"
#include "sword2/router.h"
#include "sword2/screen.h"

namespace Sword2 {

// ---------------------------------------------------------------------------
// ROUTER.CPP by James
//
// A rehash of Jeremy's original jrouter.c, containing low-level system
// routines for calculating routes between points inside a walk-grid, and
// constructing walk animations from mega-sets.
//
// jrouter.c underwent 2 major reworks from the original:
// (1)	Restructured to allow more flexibility in the mega-sets, ie. more info
//      taken from the walk-data
//	- the new George & Nico mega-sets & walk-data were then tested &
//        tweaked in the Sword1 system
// (2)	Updated for the new Sword2 system, ie. new object structures
//	- now compatible with Sword2, the essential code already having been
//        tested
//
// ---------------------------------------------------------------------------

/****************************************************************************
 *    JROUTER.C				polygon router with modular walks
 *							using a tree of modules
 *							21 july 94
 *  3  november 94
 *  System currently works by scanning grid data and coming up with a	ROUTE
 *  as a series of way points(nodes), the smoothest eight directional PATH
 *	through these nodes is then found, and a WALK created to fit the PATH.
 *
 *	Two funtions are called by the user, RouteFinder creates a route as a
 *	module list, HardWalk creates an animation list from the module list.
 *	The split is only provided to allow the possibility of turning the
 *	autorouter over two game cycles.
 ****************************************************************************
 *
 *	Routine timings on osborne 486
 *
 *	Read floor resource (file already loaded)	 112 pixels
 *
 *	Read mega resource (file already loaded)	 112 pixels
 *
 *
 *
 ****************************************************************************
 *
 *  Modified 12 Oct 95
 *
 *	Target Points within 1 pixel of a line are ignored ???
 *
 *	Modules split into  Points within 1 pixel of a line are ignored ???
 *
 ****************************************************************************
 *
 *  TOTALLY REHASHED BY JAMES FOR NEW MEGAS USING OLD SYSTEM
 *  THEN REINCARNATED BY JAMES FOR NEW MEGAS USING NEW SYSTEM
 *
 ****************************************************************************/

//----------------------------------------------------------
// (4) WALK-GRID FILES
//----------------------------------------------------------
// a walk-grid file consists of:
//
// standard file header
// walk-grid file header
// walk-grid data

// Walk-Grid Header - taken directly from old "header.h" in STD_INC

struct WalkGridHeader {
	int32 numBars;		// number of bars on the floor
	int32 numNodes;		// number of nodes
};

uint8 Router::returnSlotNo(uint32 megaId) {
	if (_vm->_logic->readVar(ID) == CUR_PLAYER_ID) {
		// George (8)
		return 0;
	} else {
		// One of Nico's mega id's
		return 1;
	}
}

void Router::allocateRouteMem() {
	uint8 slotNo;

	// Player character always always slot 0, while the other mega
	// (normally Nico) always uses slot 1
	// Better this way, so that if mega object removed from memory while
	// in middle of route, the old route will be safely cleared from
	// memory just before they create a new one

	slotNo = returnSlotNo(_vm->_logic->readVar(ID));

	// if this slot is already used, then it can't be needed any more
	// because this id is creating a new route!

	if (_routeSlots[slotNo])
		freeRouteMem();

	_routeSlots[slotNo] = (WalkData *)malloc(sizeof(WalkData) * O_WALKANIM_SIZE);

	// 12000 bytes were used for this in Sword1 mega compacts, based on
	// 20 bytes per 'WalkData' frame
	// ie. allowing for 600 frames including end-marker
	// Now 'WalkData' is 8 bytes, so 8*600 = 4800 bytes.
	// Note that a 600 frame walk lasts about 48 seconds!
	// (600fps / 12.5s = 48s)

	// mega keeps note of which slot contains the pointer to it's walk
	// animation mem block
	// +1 so that '0' can mean "not walking"
	// megaObject->route_slot_id = slotNo + 1;
}

WalkData *Router::getRouteMem() {
	uint8 slotNo = returnSlotNo(_vm->_logic->readVar(ID));

	return (WalkData *)_routeSlots[slotNo];
}

void Router::freeRouteMem() {
	uint8 slotNo = returnSlotNo(_vm->_logic->readVar(ID));

	free(_routeSlots[slotNo]);
	_routeSlots[slotNo] = NULL;
}

void Router::freeAllRouteMem() {
	for (int i = 0; i < TOTAL_ROUTE_SLOTS; i++) {
		free(_routeSlots[i]);
		_routeSlots[i] = NULL;
	}
}

int32 Router::routeFinder(byte *ob_mega, byte *ob_walkdata, int32 x, int32 y, int32 dir) {
	/*********************************************************************
	 * RouteFinder.C		polygon router with modular walks
	 *						21 august 94
	 *						3  november 94
	 * routeFinder creates a list of modules that enables HardWalk to
	 * create an animation list.
	 *
	 * routeFinder currently works by scanning grid data and coming up
	 * with a ROUTE as a series of way points(nodes), the smoothest eight
	 * directional PATH through these nodes is then found, this
	 * information is made available to HardWalk for a WALK to be created
	 * to fit the PATH.
	 *
	 * 30 november 94 return values modified
	 *
	 * return	0 = failed to find a route
	 *
	 *			1 = found a route
	 *
	 *			2 = mega already at target
	 *
	 *********************************************************************/

	int32 routeFlag = 0;
	int32 solidFlag = 0;
	WalkData *walkAnim;

	// megaId = id;

	setUpWalkGrid(ob_mega, x, y, dir);
	loadWalkData(ob_walkdata);

	walkAnim = getRouteMem();

	// All route data now loaded start finding a route

	// Check if we can get a route through the floor. changed 12 Oct95 JPS

	routeFlag = getRoute();

	switch (routeFlag) {
	case 2:
		// special case for zero length route

		// if target direction specified as any
		if (_targetDir > 7)
			_targetDir = _startDir;

		// just a turn on the spot is required set an end module for
		// the route let the animator deal with it
		// modularPath is normally set by extractRoute

		_modularPath[0].dir = _startDir;
		_modularPath[0].num = 0;
		_modularPath[0].x = _startX;
		_modularPath[0].y = _startY;
		_modularPath[1].dir = _targetDir;
		_modularPath[1].num = 0;
		_modularPath[1].x = _startX;
		_modularPath[1].y = _startY;
		_modularPath[2].dir = 9;
		_modularPath[2].num = ROUTE_END_FLAG;

		slidyWalkAnimator(walkAnim);
		routeFlag = 2;
		break;
	case 1:
		// A normal route. Convert the route to an exact path
		smoothestPath();

		// The Route had waypoints and direction options

		// The Path is an exact set of lines in 8 directions that
		// reach the target.

		// The path is in module format, but steps taken in each
		// direction are not accurate

		// if target dir = 8 then the walk isn't linked to an anim so
		// we can create a route without sliding and miss the exact
		// target

#ifndef FORCE_SLIDY
		if (_targetDir == 8) {
			// can end facing ANY direction (ie. exact end
			// position not vital) - so use SOLID walk to
			// avoid sliding to exact position

			solidPath();
			solidFlag = solidWalkAnimator(walkAnim);
		}
#endif

		if (!solidFlag) {
			// if we failed to create a SOLID route, do a SLIDY
			// one instead

			slidyPath();
			slidyWalkAnimator(walkAnim);
		}

		break;
	default:
		// Route didn't reach target so assume point was off the floor
		// routeFlag = 0;
		break;
	}

	return routeFlag;	// send back null route
}

int32 Router::getRoute() {
	/*********************************************************************
	 * GetRoute.C				extract a path from walk grid
	 *							12 october 94
	 *
	 * GetRoute currently works by scanning grid data and coming up with
	 * a ROUTE as a series of way points(nodes).
	 *
	 * static routeData _route[O_ROUTE_SIZE];
	 *
	 * return	0 = failed to find a route
	 *
	 *		1 = found a route
	 *
	 *		2 = mega already at target
	 *
	 *		3 = failed to find a route because target was on a line
	 *
	 *********************************************************************/

	int32 routeGot = 0;

	if (_startX == _targetX && _startY == _targetY)
		routeGot = 2;
	else {
		// 'else' added by JEL (23jan96) otherwise 'routeGot' affected
		// even when already set to '2' above - causing some 'turns'
		// to walk downwards on the spot

		// returns 3 if target on a line ( +- 1 pixel )
		routeGot = checkTarget(_targetX, _targetY);
	}

	if (routeGot == 0) {
		// still looking for a route check if target is within a pixel
		// of a line

		// scan through the nodes linking each node to its nearest
		// neighbor until no more nodes change

		// This is the routine that finds a route using scan()

		int32 level = 1;

		while (scan(level))
			level++;

		// Check to see if the route reached the target

		if (_node[_nNodes].dist < 9999) {
			// it did so extract the route as nodes and the
			// directions to go between each node

			routeGot = 1;
			extractRoute();

			// route.X,route.Y and route.Dir now hold all the
			// route infomation with the target dir or route
			// continuation
		}
	}

	return routeGot;
}

// THE SLIDY PATH ROUTINES

int32 Router::smoothestPath() {
	// This is the second big part of the route finder and the the only
	// bit that tries to be clever (the other bits are clever).
	//
	// This part of the autorouter creates a list of modules from a set of
	// lines running across the screen. The task is complicated by two
	// things:
	//
	// Firstly in choosing a route through the maze of nodes the routine
	// tries to minimise the amount of each individual turn avoiding 90
	// degree and greater turns (where possible) and reduces the total
	// number of turns (subject to two 45 degree turns being better than
	// one 90 degree turn).
	//
	// Secondly when walking in a given direction the number of steps
	// required to reach the end of that run is not calculated accurately.
	// This is because I was unable to derive a function to relate number
	// of steps taken between two points to the shrunken step size

	int i;
	int32 steps = 0;
	int32 lastDir;
	int32 tempturns[4];
	int32 turns[4];
	const int32 turntable[NO_DIRECTIONS] = { 0, 1, 3, 5, 7, 5, 3, 1 };

	// route.X route.Y and route.Dir start at far end

	_smoothPath[0].x = _startX;
	_smoothPath[0].y = _startY;
	_smoothPath[0].dir = _startDir;
	_smoothPath[0].num = 0;

	lastDir = _startDir;

	// for each section of the route

	for (int p = 0; p < _routeLength; p++) {
		int32 dirS = _route[p].dirS;
		int32 dirD = _route[p].dirD;
		int32 nextDirS = _route[p + 1].dirS;
		int32 nextDirD = _route[p + 1].dirD;

		// Check directions into and out of a pair of nodes going in
		int32 dS = dirS - lastDir;
		if (dS < 0)
			dS = dS + NO_DIRECTIONS;

		int32 dD = dirD - lastDir;
		if (dD < 0)
			dD = dD + NO_DIRECTIONS;

		// coming out
		int32 dSS = dirS - nextDirS;
		if (dSS < 0)
			dSS = dSS + NO_DIRECTIONS;

		int32 dDD = dirD - nextDirD;
		if (dDD < 0)
			dDD = dDD + NO_DIRECTIONS;

		int32 dSD = dirS - nextDirD;
		if (dSD < 0)
			dSD = dSD + NO_DIRECTIONS;

		int32 dDS = dirD - nextDirS;
		if (dDS < 0)
			dDS = dDS + NO_DIRECTIONS;

		// Determine the amount of turning involved in each possible path
		dS = turntable[dS];
		dD = turntable[dD];
		dSS = turntable[dSS];
		dDD = turntable[dDD];
		dSD = turntable[dSD];
		dDS = turntable[dDS];

		// get the best path out ie assume next section uses best direction
		if (dSD < dSS)
			dSS = dSD;
		if (dDS < dDD)
			dDD = dDS;

		// Rate each option. Split routes look crap so weight against them
		tempturns[0] = dS + dSS + 3;
		turns[0] = 0;
		tempturns[1] = dS + dDD;
		turns[1] = 1;
		tempturns[2] = dD + dSS;
		turns[2] = 2;
		tempturns[3] = dD + dDD + 3;
		turns[3] = 3;

		// set up turns as a sorted array of the turn values
		for (i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				if (tempturns[j] > tempturns[j + 1]) {
					SWAP(turns[j], turns[j + 1]);
					SWAP(tempturns[j], tempturns[j + 1]);
				}
			}
		}

		// best option matched in order of the priority we would like
		// to see on the screen but each option must be checked to see
		// if it can be walked

		int32 options = newCheck(1, _route[p].x, _route[p].y, _route[p + 1].x, _route[p + 1].y);

		assert(options);

		for (i = 0; i < 4; ++i) {
			int32 opt = 1 << turns[i];
			if (options & opt) {
				smoothCheck(steps, turns[i], p, dirS, dirD);
				break;
			}
		}

		assert(i < 4);

		// route.X route.Y route.dir and bestTurns start at far end
	}

	// best turns will end heading as near as possible to target dir rest
	// is down to anim for now

	_smoothPath[steps].dir = 9;
	_smoothPath[steps].num = ROUTE_END_FLAG;
	return 1;
}

void Router::smoothCheck(int32 &k, int32 best, int32 p, int32 dirS, int32 dirD) {
	/*********************************************************************
	 * Slip sliding away
	 * This path checker checks to see if a walk that exactly follows the
	 * path would be valid. This should be inherently true for atleast one
	 * of the turn options.
	 * No longer checks the data it only creates the smoothPath array JPS
	 *********************************************************************/

	int32 dsx, dsy;
	int32 ddx, ddy;
	int32 ss0, ss1, ss2;
	int32 sd0, sd1, sd2;

	if (p == 0)
		k = 1;

	int32 x = _route[p].x;
	int32 y = _route[p].y;
	int32 x2 = _route[p + 1].x;
	int32 y2 = _route[p + 1].y;
	int32 ldx = x2 - x;
	int32 ldy = y2 - y;
	int32 dirX = 1;
	int32 dirY = 1;

	if (ldx < 0) {
		ldx = -ldx;
		dirX = -1;
	}

	if (ldy < 0) {
		ldy = -ldy;
		dirY = -1;
	}

	// set up sd0-ss2 to reflect possible movement in each direction

	if (dirS == 0 || dirS == 4) {	// vert and diag
		ddx = ldx;
		ddy = (ldx * _diagonaly) / _diagonalx;
		dsy = ldy - ddy;
		ddx = ddx * dirX;
		ddy = ddy * dirY;
		dsy = dsy * dirY;
		dsx = 0;

		sd0 = (ddx + _modX[dirD] / 2) / _modX[dirD];
		ss0 = (dsy + _modY[dirS] / 2) / _modY[dirS];
		sd1 = sd0 / 2;
		ss1 = ss0 / 2;
		sd2 = sd0 - sd1;
		ss2 = ss0 - ss1;
	} else {
		ddy = ldy;
		ddx = (ldy * _diagonalx) / _diagonaly;
		dsx = ldx - ddx;
		ddy = ddy * dirY;
		ddx = ddx * dirX;
		dsx = dsx * dirX;
		dsy = 0;

		sd0 = (ddy + _modY[dirD] / 2) / _modY[dirD];
		ss0 = (dsx + _modX[dirS] / 2) / _modX[dirS];
		sd1 = sd0 / 2;
		ss1 = ss0 / 2;
		sd2 = sd0 - sd1;
		ss2 = ss0 - ss1;
	}

	switch (best) {
	case 0:		// halfsquare, diagonal, halfsquare
		_smoothPath[k].x = x + dsx / 2;
		_smoothPath[k].y = y + dsy / 2;
		_smoothPath[k].dir = dirS;
		_smoothPath[k].num = ss1;
		k++;

		_smoothPath[k].x = x + dsx / 2 + ddx;
		_smoothPath[k].y = y + dsy / 2 + ddy;
		_smoothPath[k].dir = dirD;
		_smoothPath[k].num = sd0;
		k++;

		_smoothPath[k].x = x + dsx + ddx;
		_smoothPath[k].y = y + dsy + ddy;
		_smoothPath[k].dir = dirS;
		_smoothPath[k].num = ss2;
		k++;

		break;
	case 1:		// square, diagonal
		_smoothPath[k].x = x + dsx;
		_smoothPath[k].y = y + dsy;
		_smoothPath[k].dir = dirS;
		_smoothPath[k].num = ss0;
		k++;

		_smoothPath[k].x = x2;
		_smoothPath[k].y = y2;
		_smoothPath[k].dir = dirD;
		_smoothPath[k].num = sd0;
		k++;

		break;
	case 2:		// diagonal square
		_smoothPath[k].x = x + ddx;
		_smoothPath[k].y = y + ddy;
		_smoothPath[k].dir = dirD;
		_smoothPath[k].num = sd0;
		k++;

		_smoothPath[k].x = x2;
		_smoothPath[k].y = y2;
		_smoothPath[k].dir = dirS;
		_smoothPath[k].num = ss0;
		k++;

		break;
	default:	// halfdiagonal, square, halfdiagonal
		_smoothPath[k].x = x + ddx / 2;
		_smoothPath[k].y = y + ddy / 2;
		_smoothPath[k].dir = dirD;
		_smoothPath[k].num = sd1;
		k++;

		_smoothPath[k].x = x + dsx + ddx / 2;
		_smoothPath[k].y = y + dsy + ddy / 2;
		_smoothPath[k].dir = dirS;
		_smoothPath[k].num = ss0;
		k++;

		_smoothPath[k].x = x2;
		_smoothPath[k].y = y2;
		_smoothPath[k].dir = dirD;
		_smoothPath[k].num = sd2;
		k++;

		break;
	}
}

void Router::slidyPath() {
	/*********************************************************************
	 * slidyPath creates a path based on part steps with no sliding to get
	 * as near as possible to the target without any sliding this routine
	 * is intended for use when just clicking about.
	 *
	 * produce a module list from the line data
	 *********************************************************************/

	int32 smooth = 1;
	int32 slidy = 1;

	// strip out the short sections

	_modularPath[0].x = _smoothPath[0].x;
	_modularPath[0].y = _smoothPath[0].y;
	_modularPath[0].dir = _smoothPath[0].dir;
	_modularPath[0].num = 0;

	while (_smoothPath[smooth].num < ROUTE_END_FLAG) {
		int32 scale = _scaleA * _smoothPath[smooth].y + _scaleB;
		int32 deltaX = _smoothPath[smooth].x - _modularPath[slidy - 1].x;
		int32 deltaY = _smoothPath[smooth].y - _modularPath[slidy - 1].y;
		// quarter a step minimum
		int32 stepX = (scale * _modX[_smoothPath[smooth].dir]) >> 19;
		int32 stepY = (scale * _modY[_smoothPath[smooth].dir]) >> 19;

		if (ABS(deltaX) >= ABS(stepX) && ABS(deltaY) >= ABS(stepY)) {
			_modularPath[slidy].x = _smoothPath[smooth].x;
			_modularPath[slidy].y = _smoothPath[smooth].y;
			_modularPath[slidy].dir = _smoothPath[smooth].dir;
			_modularPath[slidy].num = 1;
			slidy++;
		}
		smooth++;
	}

	// in case the last bit had no steps

	if (slidy > 1) {
		_modularPath[slidy - 1].x = _smoothPath[smooth - 1].x;
		_modularPath[slidy - 1].y = _smoothPath[smooth - 1].y;
	}

	// set up the end of the walk

	_modularPath[slidy].x = _smoothPath[smooth - 1].x;
	_modularPath[slidy].y = _smoothPath[smooth - 1].y;
	_modularPath[slidy].dir = _targetDir;
	_modularPath[slidy].num = 0;
	slidy++;

	_modularPath[slidy].x = _smoothPath[smooth - 1].x;
	_modularPath[slidy].y = _smoothPath[smooth - 1].y;
	_modularPath[slidy].dir = 9;
	_modularPath[slidy].num = ROUTE_END_FLAG;
}

// SLOW IN

bool Router::addSlowInFrames(WalkData *walkAnim) {
	if (_walkData.usingSlowInFrames && _modularPath[1].num > 0) {
		for (int slowInFrameNo = 0; slowInFrameNo < _walkData.nSlowInFrames[_currentDir]; slowInFrameNo++) {
			walkAnim[_stepCount].frame = _firstSlowInFrame[_currentDir] + slowInFrameNo;
			walkAnim[_stepCount].step = 0;
			walkAnim[_stepCount].dir = _currentDir;
			walkAnim[_stepCount].x = _moduleX;
			walkAnim[_stepCount].y = _moduleY;
			_stepCount++;
		}
		return true;
	}

	return false;
}

void Router::earlySlowOut(byte *ob_mega, byte *ob_walkdata) {
	int32 slowOutFrameNo;
	int32 walk_pc;
	WalkData *walkAnim;

	ObjectMega obMega(ob_mega);

	debug(5, "EARLY SLOW-OUT");

	loadWalkData(ob_walkdata);

	debug(5, "********************************");
	debug(5, "_framesPerStep = %d", _framesPerStep);
	debug(5, "_numberOfSlowOutFrames = %d", _numberOfSlowOutFrames);
	debug(5, "_firstWalkingTurnLeftFrame = %d", _firstWalkingTurnLeftFrame);
	debug(5, "_firstWalkingTurnRightFrame = %d", _firstWalkingTurnRightFrame);
	debug(5, "_firstSlowOutFrame = %d", _firstSlowOutFrame);
	debug(5, "********************************");

	walk_pc = obMega.getWalkPc();

	walkAnim = getRouteMem();

	// if this mega does actually have slow-out frames
	if (_walkData.usingSlowOutFrames) {
		// overwrite the next step (half a cycle) of the walk
		// (ie .step - 0..5)

		do {
			debug(5, "STEP NUMBER: walkAnim[%d].step = %d", walk_pc, walkAnim[walk_pc].step);
			debug(5, "ORIGINAL FRAME: walkAnim[%d].frame = %d", walk_pc, walkAnim[walk_pc].frame);

			// map from existing walk frame across to correct
			// frame number of slow-out - remember, there may be
			// more slow-out frames than walk-frames!

			if (walkAnim[walk_pc].frame >= _firstWalkingTurnRightFrame) {
				// if it's a walking turn-right, rather than a
				// normal step, then map it to a normal step
				// frame first

				walkAnim[walk_pc].frame -= _firstWalkingTurnRightFrame;
				debug(5, "MAPPED TO WALK: walkAnim[%d].frame = %d  (walking turn-right frame --> walk frame)", walk_pc, walkAnim[walk_pc].frame);
			} else if (walkAnim[walk_pc].frame >= _firstWalkingTurnLeftFrame) {
				// if it's a walking turn-left, rather than a
				// normal step, then map it to a normal step
				// frame first

				walkAnim[walk_pc].frame -= _firstWalkingTurnLeftFrame;
				debug(5, "MAPPED TO WALK: walkAnim[%d].frame = %d  (walking turn-left frame --> walk frame)", walk_pc, walkAnim[walk_pc].frame);
			}

			walkAnim[walk_pc].frame += _firstSlowOutFrame + ((walkAnim[walk_pc].frame / _framesPerStep) * (_numberOfSlowOutFrames - _framesPerStep));
			walkAnim[walk_pc].step = 0;
			debug(5, "SLOW-OUT FRAME: walkAnim[%d].frame = %d",walk_pc, walkAnim[walk_pc].frame);
			walk_pc++;
		} while (walkAnim[walk_pc].step > 0);

		// add stationary frame(s) (OPTIONAL)

		for (slowOutFrameNo = _framesPerStep; slowOutFrameNo < _numberOfSlowOutFrames; slowOutFrameNo++) {
			walkAnim[walk_pc].frame = walkAnim[walk_pc - 1].frame + 1;
			debug(5, "EXTRA FRAME: walkAnim[%d].frame = %d", walk_pc, walkAnim[walk_pc].frame);
			walkAnim[walk_pc].step = 0;
			walkAnim[walk_pc].dir = walkAnim[walk_pc - 1].dir;
			walkAnim[walk_pc].x = walkAnim[walk_pc - 1].x;
			walkAnim[walk_pc].y = walkAnim[walk_pc - 1].y;
			walk_pc++;
		}
	} else {
		// this mega doesn't have slow-out frames
		// stand in current direction

		walkAnim[walk_pc].frame = _firstStandFrame + walkAnim[walk_pc - 1].dir;
		walkAnim[walk_pc].step = 0;
		walkAnim[walk_pc].dir = walkAnim[walk_pc - 1].dir;
		walkAnim[walk_pc].x = walkAnim[walk_pc - 1].x;
		walkAnim[walk_pc].y = walkAnim[walk_pc - 1].y;
		walk_pc++;
	}

	// end of sequence
	walkAnim[walk_pc].frame	= 512;

	// so that this doesn't happen again while 'george_walking' is still
	// '2'
	walkAnim[walk_pc].step = 99;
}

// SLOW OUT

void Router::addSlowOutFrames(WalkData *walkAnim) {
	int32 slowOutFrameNo;

	// if the mega did actually walk, we overwrite the last step (half a
	// cycle) with slow-out frames + add any necessary stationary frames

	if (_walkData.usingSlowOutFrames && _lastCount >= _framesPerStep) {
		// place stop frames here
		// slowdown at the end of the last walk

		slowOutFrameNo = _lastCount - _framesPerStep;

		debug(5, "SLOW OUT: slowOutFrameNo(%d) = _lastCount(%d) - _framesPerStep(%d)", slowOutFrameNo, _lastCount, _framesPerStep);

		// overwrite the last step (half a cycle) of the walk

		do {
			// map from existing walk frame across to correct
			// frame number of slow-out - remember, there may be
			// more slow-out frames than walk-frames!

			walkAnim[slowOutFrameNo].frame += _firstSlowOutFrame + ((walkAnim[slowOutFrameNo].frame / _framesPerStep) * (_numberOfSlowOutFrames - _framesPerStep));

			// because no longer a normal walk-step
			walkAnim[slowOutFrameNo].step = 0;

			debug(5, "walkAnim[%d].frame = %d",slowOutFrameNo,walkAnim[slowOutFrameNo].frame);
			slowOutFrameNo++;
		} while (slowOutFrameNo < _lastCount);

		// add stationary frame(s) (OPTIONAL)

		for (slowOutFrameNo = _framesPerStep; slowOutFrameNo < _numberOfSlowOutFrames; slowOutFrameNo++) {
			walkAnim[_stepCount].frame = walkAnim[_stepCount - 1].frame + 1;

			debug(5, "EXTRA FRAMES: walkAnim[%d].frame = %d", _stepCount, walkAnim[_stepCount].frame);

			walkAnim[_stepCount].step = 0;
			walkAnim[_stepCount].dir = walkAnim[_stepCount - 1].dir;
			walkAnim[_stepCount].x = walkAnim[_stepCount - 1].x;
			walkAnim[_stepCount].y = walkAnim[_stepCount - 1].y;
			_stepCount++;
		}
	}
}

void Router::slidyWalkAnimator(WalkData *walkAnim) {
	/*********************************************************************
	 * Skidding every where HardWalk creates an animation that exactly
	 * fits the smoothPath and uses foot slipping to fit whole steps into
	 * the route
	 *
	 *	Parameters:	georgeg, mouseg
	 *	Returns:	rout
	 *
	 * produce a module list from the line data
	 *********************************************************************/

	int32 left;
	int32 p;
	int32 lastDir;
	int32 lastRealDir;
	int32 turnDir;
	int32 scale;
	int32 step;
	int32 module;
	int32 moduleEnd;
	int32 module16X;
	int32 module16Y;
	int32 stepX;
	int32 stepY;
	int32 errorX;
	int32 errorY;
	int32 lastErrorX;
	int32 lastErrorY;
	int32 frameCount;
	int32 frames;

	p = 0;
	lastDir = _modularPath[0].dir;
	_currentDir = _modularPath[1].dir;

	if (_currentDir == NO_DIRECTIONS)
		_currentDir = lastDir;

	_moduleX = _startX;
	_moduleY = _startY;
	module16X = _moduleX << 16;
	module16Y = _moduleY << 16;
	_stepCount = 0;

	// START THE WALK WITH THE FIRST STANDFRAME THIS MAY CAUSE A DELAY
	// BUT IT STOPS THE PLAYER MOVING FOR COLLISIONS ARE DETECTED

	debug(5, "SLIDY: STARTING THE WALK");

	module = _framesPerChar + lastDir;
	walkAnim[_stepCount].frame = module;
	walkAnim[_stepCount].step = 0;
	walkAnim[_stepCount].dir = lastDir;
	walkAnim[_stepCount].x = _moduleX;
	walkAnim[_stepCount].y = _moduleY;
	_stepCount++;

	// TURN TO START THE WALK

	debug(5, "SLIDY: TURNING TO START THE WALK");
	// rotate if we need to

	if (lastDir != _currentDir) {
		// get the direction to turn
		turnDir = _currentDir - lastDir;
		if (turnDir < 0)
			turnDir += NO_DIRECTIONS;

		if (turnDir > 4)
			turnDir = -1;
		else if (turnDir > 0)
			turnDir = 1;

		// rotate to new walk direction
		// for george and nico put in a head turn at the start

		if (_walkData.usingStandingTurnFrames) {
			// new frames for turn frames	29oct95jps
			if (turnDir < 0)
				module = _firstStandingTurnLeftFrame + lastDir;
			else
				module = _firstStandingTurnRightFrame + lastDir;

			walkAnim[_stepCount].frame = module;
			walkAnim[_stepCount].step = 0;
			walkAnim[_stepCount].dir = lastDir;
			walkAnim[_stepCount].x = _moduleX;
			walkAnim[_stepCount].y = _moduleY;
			_stepCount++;
		}

		// rotate till were facing new dir then go back 45 degrees
		while (lastDir != _currentDir) {
			lastDir += turnDir;

			// new frames for turn frames	29oct95jps
			if (turnDir < 0) {
				if (lastDir < 0)
					lastDir += NO_DIRECTIONS;
				module = _firstStandingTurnLeftFrame + lastDir;
			} else {
				if (lastDir > 7)
					lastDir -= NO_DIRECTIONS;
				module = _firstStandingTurnRightFrame + lastDir;
			}

			walkAnim[_stepCount].frame = module;
			walkAnim[_stepCount].step = 0;
			walkAnim[_stepCount].dir = lastDir;
			walkAnim[_stepCount].x = _moduleX;
			walkAnim[_stepCount].y = _moduleY;
			_stepCount++;
		}

		// the back 45 degrees bit
		// step back one because new head turn for george takes us
		// past the new dir
		_stepCount--;
	}

	// his head is in the right direction
	lastRealDir = _currentDir;

	// SLIDY: THE SLOW IN

	addSlowInFrames(walkAnim);

	// THE WALK

	debug(5, "SLIDY: THE WALK");

	// start the walk on the left or right leg, depending on how the
	// slow-in frames were drawn

	// (0 = left; 1 = right)

	if (_walkData.leadingLeg[_currentDir] == 0) {
		// start the walk on the left leg (ie. at beginning of the
		// first step of the walk cycle)
		left = 0;
	} else {
		// start the walk on the right leg (ie. at beginning of the
		// second step of the walk cycle)
		left = 1;
	}

	_lastCount = _stepCount;

	// this ensures that we don't put in turn frames for the start
	lastDir = 99;

	// this ensures that we don't put in turn frames for the start
	_currentDir = 99;

	do {
		assert(_stepCount < O_WALKANIM_SIZE);
		while (_modularPath[p].num == 0) {
			p++;
			if (_currentDir != 99)
				lastRealDir = _currentDir;
			lastDir = _currentDir;
			_lastCount = _stepCount;
		}

		// calculate average amount to lose in each step on the way
		// to the next node

		_currentDir = _modularPath[p].dir;

		if (_currentDir < NO_DIRECTIONS) {
			module = _currentDir * _framesPerStep * 2 + left * _framesPerStep;

			left = !left;

			moduleEnd = module + _framesPerStep;
			step = 0;
			scale = (_scaleA * _moduleY + _scaleB);

			do {
				module16X += _walkData.dx[module] * scale;
				module16Y += _walkData.dy[module] * scale;
				_moduleX = module16X >> 16;
				_moduleY = module16Y >> 16;
				walkAnim[_stepCount].frame = module;
				walkAnim[_stepCount].step = step;	// normally 0,1,2,3,4,5,0,1,2,etc
				walkAnim[_stepCount].dir = _currentDir;
				walkAnim[_stepCount].x = _moduleX;
				walkAnim[_stepCount].y = _moduleY;
				_stepCount++;
				step++;
				module++;
			} while (module < moduleEnd);

			stepX = _modX[_modularPath[p].dir];
			stepY = _modY[_modularPath[p].dir];
			errorX = _modularPath[p].x - _moduleX;
			errorX = errorX * stepX;
			errorY = _modularPath[p].y - _moduleY;
			errorY = errorY * stepY;

			if (errorX < 0 || errorY < 0) {
				_modularPath[p].num = 0;	// the end of the path

				// okay those last steps took us past our
				// target but do we want to scoot or moonwalk

				frames = _stepCount - _lastCount;
				errorX = _modularPath[p].x - walkAnim[_stepCount - 1].x;
				errorY = _modularPath[p].y - walkAnim[_stepCount - 1].y;

				if (frames > _framesPerStep) {
					lastErrorX = _modularPath[p].x - walkAnim[_stepCount - 7].x;
					lastErrorY = _modularPath[p].y - walkAnim[_stepCount - 7].y;

					if (stepX == 0) {
						if (3 * ABS(lastErrorY) < ABS(errorY)) {
							// the last stop was
							// closest
							_stepCount -= _framesPerStep;
							left = !left;
						}
					} else {
						if (3 * ABS(lastErrorX) < ABS(errorX)) {
							//the last stop was
							// closest
							_stepCount -= _framesPerStep;
							left = !left;
						}
					}
				}

				errorX = _modularPath[p].x - walkAnim[_stepCount-1].x;
				errorY = _modularPath[p].y - walkAnim[_stepCount-1].y;

				// okay we've reached the end but we still
				// have an error

				if (errorX != 0) {
					frameCount = 0;
					frames = _stepCount - _lastCount;

					do {
						frameCount++;
						walkAnim[_lastCount + frameCount - 1].x += errorX * frameCount / frames;
					} while (frameCount < frames);
				}

				if (errorY != 0) {
					frameCount = 0;
					frames = _stepCount - _lastCount;
					do {
						frameCount++;
						walkAnim[_lastCount + frameCount - 1].y += errorY * frameCount / frames;
					} while (frameCount < frames);
				}

				// Now is the time to put in the turn frames
				// for the last turn

				if (frames < _framesPerStep) {
					// this ensures that we don't put in
					// turn frames for this walk or the
					// next
					_currentDir = 99;
				}

				if (_currentDir != 99)
					lastRealDir = _currentDir;

				// check each turn condition in turn

				 // only for george
				if (lastDir != 99 && _currentDir != 99 && _walkData.usingWalkingTurnFrames) {
					// 1 and -7 going right -1 and 7 going
					// left
					lastDir = _currentDir - lastDir;

					if (lastDir == -1 || lastDir == 7 || lastDir == -2 || lastDir == 6) {
						// turn at the end of the last
						// walk

						_frame = _lastCount - _framesPerStep;
						do {
							// turning left
							walkAnim[_frame].frame += _firstWalkingTurnLeftFrame;
							_frame++;
						} while (_frame < _lastCount);
					} else if (lastDir == 1 || lastDir == -7 || lastDir == 2 || lastDir == -6) {
						// turn at the end of the
						// current walk

						_frame = _lastCount - _framesPerStep;
						do {
							// turning right
							walkAnim[_frame].frame += _firstWalkingTurnRightFrame;
							_frame++;
						} while (_frame < _lastCount);
					}
					lastDir = _currentDir;
				}

				// all turns checked

				_lastCount = _stepCount;
				_moduleX = walkAnim[_stepCount - 1].x;
				_moduleY = walkAnim[_stepCount - 1].y;
				module16X = _moduleX << 16;
				module16Y = _moduleY << 16;
			}
		}
	} while (_modularPath[p].dir < NO_DIRECTIONS);

#ifdef SWORD2_DEBUG
	if (lastRealDir == 99)
		error("slidyWalkAnimatorlast direction error");
#endif

	// THE SLOW OUT
	addSlowOutFrames(walkAnim);

	// TURNS TO END THE WALK ?

	// We've done the walk now put in any turns at the end

	if (_targetDir == 8) {
		// ANY direction -> stand in the last direction

		module = _firstStandFrame + lastRealDir;
		_targetDir = lastRealDir;
		walkAnim[_stepCount].frame = module;
		walkAnim[_stepCount].step = 0;
		walkAnim[_stepCount].dir = lastRealDir;
		walkAnim[_stepCount].x = _moduleX;
		walkAnim[_stepCount].y = _moduleY;
		_stepCount++;
	}

	if (_targetDir == 9) {
		// 'stance' was non-zero
		if (_stepCount == 0) {
			module = _framesPerChar + lastRealDir;
			walkAnim[_stepCount].frame = module;
			walkAnim[_stepCount].step = 0;
			walkAnim[_stepCount].dir = lastRealDir;
			walkAnim[_stepCount].x = _moduleX;
			walkAnim[_stepCount].y = _moduleY;
			_stepCount++;
		}
	} else if (_targetDir != lastRealDir) {
		// rotate to target direction
		turnDir = _targetDir - lastRealDir;
		if (turnDir < 0)
			turnDir += NO_DIRECTIONS;

		if (turnDir > 4)
			turnDir = -1;
		else if (turnDir > 0)
			turnDir = 1;

		// rotate to target direction
		// for george and nico put in a head turn at the start

		if (_walkData.usingStandingTurnFrames) {
			// new frames for turn frames	29oct95jps
			if (turnDir < 0)
				module = _firstStandingTurnLeftFrame + lastDir;
			else
				module = _firstStandingTurnRightFrame + lastDir;

			walkAnim[_stepCount].frame = module;
			walkAnim[_stepCount].step = 0;
			walkAnim[_stepCount].dir = lastRealDir;
			walkAnim[_stepCount].x = _moduleX;
			walkAnim[_stepCount].y = _moduleY;
			_stepCount++;
		}

		// rotate if we need to

		while (lastRealDir != _targetDir) {
			lastRealDir += turnDir;

			// new frames for turn frames	29oct95jps
			if (turnDir < 0) {
				if (lastRealDir < 0)
					lastRealDir += NO_DIRECTIONS;
				module = _firstStandingTurnLeftFrame + lastRealDir;
			} else {
				if (lastRealDir > 7)
					lastRealDir -= NO_DIRECTIONS;
				module = _firstStandingTurnRightFrame + lastRealDir;
			}

			walkAnim[_stepCount].frame = module;
			walkAnim[_stepCount].step = 0;
			walkAnim[_stepCount].dir = lastRealDir;
			walkAnim[_stepCount].x = _moduleX;
			walkAnim[_stepCount].y = _moduleY;
			_stepCount++;
		}

		module = _firstStandFrame + lastRealDir;
		walkAnim[_stepCount - 1].frame = module;
	} else {
		// just stand at the end
		module = _firstStandFrame + lastRealDir;
		walkAnim[_stepCount].frame = module;
		walkAnim[_stepCount].step = 0;
		walkAnim[_stepCount].dir = lastRealDir;
		walkAnim[_stepCount].x = _moduleX;
		walkAnim[_stepCount].y = _moduleY;
		_stepCount++;
	}

	walkAnim[_stepCount].frame = 512;
	walkAnim[_stepCount].step = 99;
	_stepCount++;

	walkAnim[_stepCount].frame = 512;
	walkAnim[_stepCount].step = 99;
	_stepCount++;

	walkAnim[_stepCount].frame = 512;
	walkAnim[_stepCount].step = 99;

	// write all the frames to "debug.txt"
	debug(5, "THE WALKDATA:");

	for (_frame = 0; _frame <= _stepCount; _frame++)
		debug(5, "walkAnim[%d].frame=%d", _frame, walkAnim[_frame].frame);

	debug(5, "routeFinder RouteSize is %d", _stepCount);
	return;
}

#ifndef FORCE_SLIDY

// THE SOLID PATH ROUTINES

void Router::solidPath() {
	/*********************************************************************
	 * SolidPath creates a path based on whole steps with no sliding to
	 * get as near as possible to the target without any sliding this
	 * routine is currently unused, but is intended for use when just
	 * clicking about.
	 *
	 * produce a module list from the line data
	 *********************************************************************/

	int32 smooth;
	int32 solid;
	int32 scale;
	int32 stepX;
	int32 stepY;
	int32 deltaX;
	int32 deltaY;

	// strip out the short sections

	solid = 1;
	smooth = 1;
	_modularPath[0].x = _smoothPath[0].x;
	_modularPath[0].y = _smoothPath[0].y;
	_modularPath[0].dir = _smoothPath[0].dir;
	_modularPath[0].num = 0;

	do {
		scale = _scaleA * _smoothPath[smooth].y + _scaleB;
		deltaX = _smoothPath[smooth].x - _modularPath[solid - 1].x;
		deltaY = _smoothPath[smooth].y - _modularPath[solid - 1].y;
		stepX = _modX[_smoothPath[smooth].dir];
		stepY = _modY[_smoothPath[smooth].dir];
		stepX = stepX * scale;
		stepY = stepY * scale;
		stepX = stepX >> 16;
		stepY = stepY >> 16;

		if (ABS(deltaX) >= ABS(stepX) && ABS(deltaY) >= ABS(stepY)) {
			_modularPath[solid].x = _smoothPath[smooth].x;
			_modularPath[solid].y = _smoothPath[smooth].y;
			_modularPath[solid].dir = _smoothPath[smooth].dir;
			_modularPath[solid].num = 1;
			solid++;
		}

		smooth++;
	} while (_smoothPath[smooth].num < ROUTE_END_FLAG);

	// in case the last bit had no steps

	if (solid == 1) {
		// there were no paths so put in a dummy end
		solid = 2;
		_modularPath[1].dir = _smoothPath[0].dir;
		_modularPath[1].num = 0;
	}

	_modularPath[solid - 1].x = _smoothPath[smooth - 1].x;
	_modularPath[solid - 1].y = _smoothPath[smooth - 1].y;

	// set up the end of the walk
	_modularPath[solid].x = _smoothPath[smooth - 1].x;
	_modularPath[solid].y = _smoothPath[smooth - 1].y;
	_modularPath[solid].dir = 9;
	_modularPath[solid].num = ROUTE_END_FLAG;
}

int32 Router::solidWalkAnimator(WalkData *walkAnim) {
	/*********************************************************************
	 * SolidWalk creates an animation based on whole steps with no sliding
	 * to get as near as possible to the target without any sliding. This
	 * routine is is intended for use when just clicking about.
	 *
	 * produce a module list from the line data
	 *
	 * returns 0 if solid route not found
	 *********************************************************************/

	int32 left;
	int32 turnDir;
	int32 scale;
	int32 step;
	int32 errorX;
	int32 errorY;
	int32 moduleEnd;
	bool slowStart = false;

	// start at the beginning for a change

	int32 lastDir = _modularPath[0].dir;
	int32 module = _framesPerChar + lastDir;

	_currentDir = _modularPath[1].dir;
	_moduleX = _startX;
	_moduleY = _startY;
	_stepCount = 0;

	int32 module16X = _moduleX << 16;
	int32 module16Y = _moduleY << 16;

	// START THE WALK WITH THE FIRST STANDFRAME THIS MAY CAUSE A DELAY
	// BUT IT STOPS THE PLAYER MOVING FOR COLLISIONS ARE DETECTED

	debug(5, "SOLID: STARTING THE WALK");
	walkAnim[_stepCount].frame = module;
	walkAnim[_stepCount].step = 0;
	walkAnim[_stepCount].dir = lastDir;
	walkAnim[_stepCount].x = _moduleX;
	walkAnim[_stepCount].y = _moduleY;
	_stepCount++;

	// TURN TO START THE WALK

	debug(5, "SOLID: TURNING TO START THE WALK");

	// rotate if we need to

	if (lastDir != _currentDir) {
		// get the direction to turn
		turnDir = _currentDir - lastDir;
		if (turnDir < 0)
			turnDir += NO_DIRECTIONS;

		if (turnDir > 4)
			turnDir = -1;
		else if (turnDir > 0)
			turnDir = 1;

		// rotate to new walk direction
		// for george and nico put in a head turn at the start

		if (_walkData.usingStandingTurnFrames) {
			// new frames for turn frames	29oct95jps
			if (turnDir < 0)
				module = _firstStandingTurnLeftFrame + lastDir;
			else
				module = _firstStandingTurnRightFrame + lastDir;

			walkAnim[_stepCount].frame = module;
			walkAnim[_stepCount].step = 0;
			walkAnim[_stepCount].dir = lastDir;
			walkAnim[_stepCount].x = _moduleX;
			walkAnim[_stepCount].y = _moduleY;
			_stepCount++;
		}

		// rotate till were facing new dir then go back 45 degrees

		while (lastDir != _currentDir) {
			lastDir += turnDir;

			// new frames for turn frames
			if (turnDir < 0) {
				if (lastDir < 0)
					lastDir += NO_DIRECTIONS;
				module = _firstStandingTurnLeftFrame + lastDir;
			} else {
				if (lastDir > 7)
					lastDir -= NO_DIRECTIONS;
				module = _firstStandingTurnRightFrame + lastDir;
			}

			walkAnim[_stepCount].frame = module;
			walkAnim[_stepCount].step = 0;
			walkAnim[_stepCount].dir = lastDir;
			walkAnim[_stepCount].x = _moduleX;
			walkAnim[_stepCount].y = _moduleY;
			_stepCount++;
		}

		// the back 45 degrees bit
		// step back one because new head turn for george takes us
		// past the new dir

		_stepCount--;
	}

	// THE SLOW IN

	slowStart = addSlowInFrames(walkAnim);

	// THE WALK

	debug(5, "SOLID: THE WALK");

	// start the walk on the left or right leg, depending on how the
	// slow-in frames were drawn

	// (0 = left; 1 = right)
	if (_walkData.leadingLeg[_currentDir] == 0) {
		// start the walk on the left leg (ie. at beginning of the
		// first step of the walk cycle)
		left = 0;
	} else {
		// start the walk on the right leg (ie. at beginning of the
		// second step of the walk cycle)
		left = 1;
	}

	_lastCount = _stepCount;

	// this ensures that we don't put in turn frames for the start
	lastDir = 99;

	// this ensures that we don't put in turn frames for the start
	_currentDir = 99;

	int32 p;

	 for (p = 1; _modularPath[p].dir < NO_DIRECTIONS; ++p) {
		while (_modularPath[p].num > 0) {
			_currentDir = _modularPath[p].dir;
			if (_currentDir < NO_DIRECTIONS) {
				module = _currentDir * _framesPerStep * 2 + left * _framesPerStep;

				left = !left;

				moduleEnd = module + _framesPerStep;
				step = 0;
				scale = (_scaleA * _moduleY + _scaleB);

				do {
					module16X += _walkData.dx[module] * scale;
					module16Y += _walkData.dy[module] * scale;
					_moduleX = module16X >> 16;
					_moduleY = module16Y >> 16;
					walkAnim[_stepCount].frame = module;
					walkAnim[_stepCount].step = step;	// normally 0,1,2,3,4,5,0,1,2,etc
					walkAnim[_stepCount].dir = _currentDir;
					walkAnim[_stepCount].x = _moduleX;
					walkAnim[_stepCount].y = _moduleY;
					_stepCount++;
					module++;
					step++;
				} while (module < moduleEnd);

				errorX = _modularPath[p].x - _moduleX;
				errorX = errorX * _modX[_modularPath[p].dir];
				errorY = _modularPath[p].y - _moduleY;
				errorY = errorY * _modY[_modularPath[p].dir];

				if (errorX < 0 || errorY < 0) {
					_modularPath[p].num = 0;
					_stepCount -= _framesPerStep;

					left = !left;

					// Okay this is the end of a section

					_moduleX = walkAnim[_stepCount - 1].x;
					_moduleY = walkAnim[_stepCount - 1].y;
					module16X = _moduleX << 16;
					module16Y = _moduleY << 16;
					_modularPath[p].x = _moduleX;
					_modularPath[p].y = _moduleY;

					// Now is the time to put in the turn
					// frames for the last turn

					if (_stepCount - _lastCount < _framesPerStep) {
						// no step taken

						// clean up if a slow in but no
						// walk

						if (slowStart) {
							_stepCount -= _walkData.nSlowInFrames[_currentDir];
							_lastCount -= _walkData.nSlowInFrames[_currentDir];
							slowStart = false;
						}

						// this ensures that we don't
						// put in turn frames for this
						// walk or the next

						_currentDir = 99;
					}

					// check each turn condition in turn
					if (lastDir != 99 && _currentDir != 99 && _walkData.usingWalkingTurnFrames) {
						// only for george
						// 1 and -7 going right -1 and
						// 7 going left

						lastDir = _currentDir - lastDir;

						if (lastDir == -1 || lastDir == 7 || lastDir == -2 || lastDir == 6) {
							// turn at the end of
							// the last walk

							_frame = _lastCount - _framesPerStep;

							do {
								// turning left
								walkAnim[_frame].frame += _firstWalkingTurnLeftFrame;
								_frame++;
							} while (_frame < _lastCount);
						} else if (lastDir == 1 || lastDir == -7 || lastDir == 2 || lastDir == -6) {
							// turn at the end of
							// the current walk

							_frame = _lastCount - _framesPerStep;
							do {
								// turning right
								walkAnim[_frame].frame += _firstWalkingTurnRightFrame;
								_frame++;
							} while (_frame < _lastCount);
						}
					}

					// all turns checked
					_lastCount = _stepCount;
				}
			}
		}
		lastDir = _currentDir;

		// can only be valid first time round
		slowStart = false;
	}

	// THE SLOW OUT

	addSlowOutFrames(walkAnim);

	module = _framesPerChar + _modularPath[p - 1].dir;
	walkAnim[_stepCount].frame = module;
	walkAnim[_stepCount].step = 0;
	walkAnim[_stepCount].dir = _modularPath[p - 1].dir;
	walkAnim[_stepCount].x = _moduleX;
	walkAnim[_stepCount].y = _moduleY;
	_stepCount++;

	walkAnim[_stepCount].frame = 512;
	walkAnim[_stepCount].step = 99;
	_stepCount++;

	walkAnim[_stepCount].frame = 512;
	walkAnim[_stepCount].step = 99;
	_stepCount++;

	walkAnim[_stepCount].frame = 512;
	walkAnim[_stepCount].step = 99;

	debug(5, "THE WALKDATA:");

	for (_frame = 0; _frame <= _stepCount; _frame++)
		debug(5, "walkAnim[%d].frame=%d", _frame, walkAnim[_frame].frame);

	// NO END TURNS

	debug(5, "routeFinder RouteSize is %d", _stepCount);
	// now check the route

	for (int i = 0; i < p - 1; ++i) {
		if (!check(_modularPath[i].x, _modularPath[i].y, _modularPath[i + 1].x, _modularPath[i + 1].y))
			p = 0;
	}

	if (p != 0) {
		_targetDir = _modularPath[p - 1].dir;
		if (checkTarget(_moduleX, _moduleY) == 3) {
			// new target on a line
			p = 0;
			debug(5, "Solid walk target was on a line %d %d", _moduleX, _moduleY);
		}
	}

	return p;
}
#endif

// THE SCAN ROUTINES

bool Router::scan(int32 level) {
	/*********************************************************************
	 * Called successively from routeFinder	until no more changes take
	 * place in the grid array, ie he best path has been found
	 *
	 * Scans through every point in the node array and checks if there is
	 * a route between each point and if this route gives a new route.
	 *
	 * This routine could probably halve its processing time if it doubled
	 * up on the checks after each route check
	 *
	 *********************************************************************/

	int32 x1, y1, x2, y2;
	int32 distance;
	bool changed = false;

	// For all the nodes that have new values and a distance less than
	// enddist, ie dont check for new routes from a point we checked
	// before or from a point that is already further away than the best
	// route so far.

	for (int i = 0; i < _nNodes; i++) {
		if (_node[i].dist < _node[_nNodes].dist && _node[i].level == level) {
			x1 = _node[i].x;
			y1 = _node[i].y;

			for (int j = _nNodes; j > 0; j--) {
				if (_node[j].dist > _node[i].dist) {
					x2 = _node[j].x;
					y2 = _node[j].y;

					if (ABS(x2 - x1) > 4.5 * ABS(y2 - y1))
						distance = (8 * ABS(x2 - x1) + 18 * ABS(y2 - y1)) / (54 * 8) + 1;
					else
						distance = (6 * ABS(x2 - x1) + 36 * ABS(y2 - y1)) / (36 * 14) + 1;

					if (distance + _node[i].dist < _node[_nNodes].dist && distance + _node[i].dist < _node[j].dist) {
						if (newCheck(0, x1, y1, x2, y2)) {
							_node[j].level = level + 1;
							_node[j].dist = distance + _node[i].dist;
							_node[j].prev = i;
							changed = true;
						}
					}
				}
			}
		}
	}

	return changed;
}

int32 Router::newCheck(int32 status, int32 x1, int32 y1, int32 x2, int32 y2) {
	/*********************************************************************
	 * newCheck routine checks if the route between two points can be
	 * achieved without crossing any of the bars in the Bars array.
	 *
	 * newCheck differs from check in that that 4 route options are
	 * considered corresponding to actual walked routes.
	 *
	 * Note distance doesnt take account of shrinking ???
	 *
	 * Note Bars array must be properly calculated ie min max dx dy co
	 *********************************************************************/

	int32 ldx;
	int32 ldy;
	int32 dlx;
	int32 dly;
	int32 dirX;
	int32 dirY;
	int32 step1;
	int32 step2;
	int32 step3;
	int32 steps;
	int32 options;

	steps = 0;
	options = 0;
	ldx = x2 - x1;
	ldy = y2 - y1;
	dirX = 1;
	dirY = 1;

	if (ldx < 0) {
		ldx = -ldx;
		dirX = -1;
	}

	if (ldy < 0) {
		ldy = -ldy;
		dirY = -1;
	}

	// make the route options

	if (_diagonaly * ldx > _diagonalx * ldy) {
		// dir  = 1,2 or 2,3 or 5,6 or 6,7

		dly = ldy;
		dlx = (ldy * _diagonalx) / _diagonaly;
		ldx = ldx - dlx;
		dlx = dlx * dirX;
		dly = dly * dirY;
		ldx = ldx * dirX;
		ldy = 0;

		// options are square, diagonal a code 1 route
		step1 = check(x1, y1, x1 + ldx, y1);
		if (step1 != 0) {
			step2 = check(x1 + ldx, y1, x2, y2);
			if (step2 != 0) {
				steps = step1 + step2;
				options |= 2;
			}
		}

		// diagonal, square a code 2 route
		if (steps == 0 || status == 1) {
			step1 = check(x1, y1, x1 + dlx, y1 + dly);
			if (step1 != 0) {
				step2 = check(x1 + dlx, y2, x2, y2);
				if (step2 != 0) {
					steps = step1 + step2;
					options |= 4;
				}
			}
		}

		// halfsquare, diagonal, halfsquare a code 0 route
		if (steps == 0 || status == 1) {
			step1 = check(x1, y1, x1 + ldx / 2, y1);
			if (step1 != 0) {
				step2 = check(x1 + ldx / 2, y1, x1 + ldx / 2 + dlx, y2);
				if (step2 != 0) {
					step3 = check(x1 + ldx / 2 + dlx, y2, x2, y2);
					if (step3 != 0)	{
						steps = step1 + step2 + step3;
						options |= 1;
					}
				}
			}
		}

		// halfdiagonal, square, halfdiagonal a code 3 route
		if (steps == 0 || status == 1) {
			step1 = check(x1, y1, x1 + dlx / 2, y1 + dly / 2);
			if (step1 != 0) {
				step2 = check(x1 + dlx / 2, y1 + dly / 2, x1 + ldx + dlx / 2, y1 + dly / 2);
				if (step2 != 0) {
					step3 = check(x1 + ldx + dlx / 2, y1 + dly / 2, x2, y2);
					if (step3 != 0) {
						steps = step1 + step2 + step3;
						options |= 8;
					}
				}
			}
		}
	} else {
		// dir  = 7,0 or 0,1 or 3,4 or 4,5

		dlx = ldx;
		dly = (ldx * _diagonaly) / _diagonalx;
		ldy = ldy - dly;
		dlx = dlx * dirX;
		dly = dly * dirY;
		ldy = ldy * dirY;
		ldx = 0;

		// options are square, diagonal a code 1 route
		step1 = check(x1 ,y1, x1, y1 + ldy);
		if (step1 != 0)	{
			step2 = check(x1, y1 + ldy, x2, y2);
			if (step2 != 0) {
				steps = step1 + step2;
				options |= 2;
			}
		}

		// diagonal, square a code 2 route
		if (steps == 0 || status == 1) {
			step1 = check(x1, y1, x2, y1 + dly);
			if (step1 != 0) {
				step2 = check(x2, y1 + dly, x2, y2);
				if (step2 != 0) {
					steps = step1 + step2;
					options |= 4;
				}
			}
		}

		// halfsquare, diagonal, halfsquare a code 0 route
		if (steps == 0 || status == 1) {
			step1 = check(x1, y1, x1, y1 + ldy / 2);
			if (step1 != 0) {
				step2 = check(x1, y1 + ldy / 2, x2, y1 + ldy / 2 + dly);
				if (step2 != 0) {
					step3 = check(x2, y1 + ldy / 2 + dly, x2, y2);
					if (step3 != 0) {
						steps = step1 + step2 + step3;
						options |= 1;
					}
				}
			}
		}

		// halfdiagonal, square, halfdiagonal a code 3 route
		if (steps == 0 || status == 1) {
			step1 = check(x1, y1, x1 + dlx / 2, y1 + dly / 2);
			if (step1 != 0) {
				step2 = check(x1 + dlx / 2, y1 + dly / 2, x1 + dlx / 2, y1 + ldy + dly / 2);
				if (step2 != 0) {
					step3 = check(x1 + dlx / 2, y1 + ldy + dly / 2, x2, y2);
					if (step3 != 0)	{
						steps = step1 + step2 + step3;
						options |= 8;
					}
				}
			}
		}
	}

	if (status == 0)
		status = steps;
	else
		status = options;

	return status;
}

// CHECK ROUTINES

bool Router::check(int32 x1, int32 y1, int32 x2, int32 y2) {
	// call the fastest line check for the given line
	// returns true if line didn't cross any bars

	if (x1 == x2 && y1 == y2)
		return true;

	if (x1 == x2)
		return vertCheck(x1, y1, y2);

	if (y1 == y2)
		return horizCheck(x1, y1, x2);

	return lineCheck(x1, y1, x2, y2);
}

bool Router::lineCheck(int32 x1, int32 y1, int32 x2, int32 y2) {
	bool linesCrossed = true;

	int32 xmin = MIN(x1, x2);
	int32 xmax = MAX(x1, x2);
	int32 ymin = MIN(y1, y2);
	int32 ymax = MAX(y1, y2);

	// Line set to go one step in chosen direction so ignore if it hits
	// anything

	int32 dirx = x2 - x1;
	int32 diry = y2 - y1;

	int32 co = (y1 * dirx) - (x1 * diry);		// new line equation

	for (int i = 0; i < _nBars && linesCrossed; i++) {
		// skip if not on module
		if (xmax >= _bars[i].xmin && xmin <= _bars[i].xmax && ymax >= _bars[i].ymin && ymin <= _bars[i].ymax) {
			// Okay, it's a valid line. Calculate an intercept. Wow
			// but all this arithmetic we must have loads of time

			// slope it he slope between the two lines
			int32 slope = (_bars[i].dx * diry) - (_bars[i].dy *dirx);
			// assuming parallel lines don't cross
			if (slope != 0) {
				// calculate x intercept and check its on both
				// lines
				int32 xc = ((_bars[i].co * dirx) - (co * _bars[i].dx)) / slope;

				// skip if not on module
				if (xc >= xmin - 1 && xc <= xmax + 1) {
					// skip if not on line
					if (xc >= _bars[i].xmin - 1 && xc <= _bars[i].xmax + 1) {
						int32 yc = ((_bars[i].co * diry) - (co * _bars[i].dy)) / slope;

						// skip if not on module
						if (yc >= ymin - 1 && yc <= ymax + 1) {
							// skip if not on line
							if (yc >= _bars[i].ymin - 1 && yc <= _bars[i].ymax + 1) {
								linesCrossed = false;
							}
						}
					}
				}
			}
		}
	}

	return linesCrossed;
}

bool Router::horizCheck(int32 x1, int32 y, int32 x2) {
	bool linesCrossed = true;

	int32 xmin = MIN(x1, x2);
	int32 xmax = MAX(x1, x2);

	// line set to go one step in chosen direction so ignore if it hits
	// anything

	for (int i = 0; i < _nBars && linesCrossed; i++) {
		// skip if not on module
		if (xmax >= _bars[i].xmin && xmin <= _bars[i].xmax && y >= _bars[i].ymin && y <= _bars[i].ymax) {
			// Okay, it's a valid line calculate an intercept. Wow
			// but all this arithmetic we must have loads of time

			if (_bars[i].dy == 0)
				linesCrossed = false;
			else {
				int32 ldy = y - _bars[i].y1;
				int32 xc = _bars[i].x1 + (_bars[i].dx * ldy) / _bars[i].dy;
				// skip if not on module
				if (xc >= xmin - 1 && xc <= xmax + 1)
					linesCrossed = false;
			}
		}
	}

	return linesCrossed;
}

bool Router::vertCheck(int32 x, int32 y1, int32 y2) {
	bool linesCrossed = true;

	int32 ymin = MIN(y1, y2);
	int32 ymax = MAX(y1, y2);

	// Line set to go one step in chosen direction so ignore if it hits
	// anything

	for (int i = 0; i < _nBars && linesCrossed; i++) {
		// skip if not on module
		if (x >= _bars[i].xmin && x <= _bars[i].xmax && ymax >= _bars[i].ymin && ymin <= _bars[i].ymax) {
			// Okay, it's a valid line calculate an intercept. Wow
			// but all this arithmetic we must have loads of time

			// both lines vertical and overlap in x and y so they
			// cross

			if (_bars[i].dx == 0)
				linesCrossed = false;
			else {
				int32 ldx = x - _bars[i].x1;
				int32 yc = _bars[i].y1 + (_bars[i].dy * ldx) / _bars[i].dx;
				// the intercept overlaps
				if (yc >= ymin - 1 && yc <= ymax + 1)
					linesCrossed = false;
			}
		}
	}

	return linesCrossed;
}

int32 Router::checkTarget(int32 x, int32 y) {
	int32 onLine = 0;

	int32 xmin = x - 1;
	int32 xmax = x + 1;
	int32 ymin = y - 1;
	int32 ymax = y + 1;

	// check if point +- 1 is on the line
	// so ignore if it hits anything

	for (int i = 0; i < _nBars && onLine == 0; i++) {
		// overlapping line
		if (xmax >= _bars[i].xmin && xmin <= _bars[i].xmax && ymax >= _bars[i].ymin && ymin <= _bars[i].ymax) {
			int32 xc, yc;

			// okay this line overlaps the target calculate an y intercept for x

			// vertical line so we know it overlaps y
			if (_bars[i].dx == 0)
				yc = 0;
			else {
				int ldx = x - _bars[i].x1;
				yc = _bars[i].y1 + (_bars[i].dy * ldx) / _bars[i].dx;
			}

			// overlapping point for y
			if (yc >= ymin && yc <= ymax) {
				// target on a line so drop out
				onLine = 3;
				debug(5, "RouteFail due to target on a line %d %d", x, y);
			} else {
				// vertical line so we know it overlaps y
				if (_bars[i].dy == 0)
					xc = 0;
				else {
					int32 ldy = y - _bars[i].y1;
					xc = _bars[i].x1 + (_bars[i].dx * ldy) / _bars[i].dy;
				}

				// skip if not on module
				if (xc >= xmin && xc <= xmax) {
					// target on a line so drop out
					onLine = 3;
					debug(5, "RouteFail due to target on a line %d %d", x, y);
				}
			}
		}
	}

	return onLine;
}

// THE SETUP ROUTINES

void Router::loadWalkData(byte *ob_walkdata) {
	uint16 firstFrameOfDirection;
	uint16 walkFrameNo;
	uint32 frameCounter = 0; // starts at frame 0 of mega set
	int i;

	_walkData.read(ob_walkdata);

	// 0 = not using slow out frames; non-zero = using that many frames
	// for each leading leg for each direction

	_numberOfSlowOutFrames = _walkData.usingSlowOutFrames;

	for (i = 0; i < NO_DIRECTIONS; i++) {
		firstFrameOfDirection = i * _walkData.nWalkFrames;

		_modX[i] = 0;
		_modY[i] = 0;

		for (walkFrameNo = firstFrameOfDirection; walkFrameNo < firstFrameOfDirection + _walkData.nWalkFrames / 2; walkFrameNo++) {
			// eg. _modX[0] is the sum of the x-step sizes for the
			// first half of the walk cycle for direction 0
			_modX[i] += _walkData.dx[walkFrameNo];
			_modY[i] += _walkData.dy[walkFrameNo];
		}
	}

	_diagonalx = _modX[3];
	_diagonaly = _modY[3];

	// interpret the walk data

	_framesPerStep = _walkData.nWalkFrames / 2;
	_framesPerChar = _walkData.nWalkFrames * NO_DIRECTIONS;

	// offset pointers added Oct 30 95 JPS
	// mega id references removed 16sep96 by JEL

	// WALK FRAMES
	// start on frame 0

	frameCounter += _framesPerChar;

	// STAND FRAMES
	// stand frames come after the walk frames
	// one stand frame for each direction

	_firstStandFrame = frameCounter;
	frameCounter += NO_DIRECTIONS;

	// STANDING TURN FRAMES - OPTIONAL!
	// standing turn-left frames come after the slow-out frames
	// one for each direction
	// standing turn-left frames come after the standing turn-right frames
	// one for each direction

	if (_walkData.usingStandingTurnFrames) {
		_firstStandingTurnLeftFrame = frameCounter;
		frameCounter += NO_DIRECTIONS;

		_firstStandingTurnRightFrame = frameCounter;
		frameCounter += NO_DIRECTIONS;
	} else {
		// refer instead to the normal stand frames
		_firstStandingTurnLeftFrame = _firstStandFrame;
		_firstStandingTurnRightFrame = _firstStandFrame;
	}

	// WALKING TURN FRAMES - OPTIONAL!
	// walking left-turn frames come after the stand frames
	// walking right-turn frames come after the walking left-turn frames

	if (_walkData.usingWalkingTurnFrames) {
		_firstWalkingTurnLeftFrame = frameCounter;
		frameCounter += _framesPerChar;

		_firstWalkingTurnRightFrame = frameCounter;
		frameCounter += _framesPerChar;
	} else {
		_firstWalkingTurnLeftFrame = 0;
		_firstWalkingTurnRightFrame = 0;
	}

	// SLOW-IN FRAMES - OPTIONAL!
	// slow-in frames come after the walking right-turn frames

	if (_walkData.usingSlowInFrames) {
		// Make note of frame number of first slow-in frame for each
		// direction. There may be a different number of slow-in
		// frames in each direction

		for (i = 0; i < NO_DIRECTIONS; i++) {
			_firstSlowInFrame[i] = frameCounter;
			frameCounter += _walkData.nSlowInFrames[i];
		}
	}

	// SLOW-OUT FRAMES - OPTIONAL!
	// slow-out frames come after the slow-in frames

	if (_walkData.usingSlowOutFrames)
		_firstSlowOutFrame = frameCounter;
}

// THE ROUTE EXTRACTOR

void Router::extractRoute() {
	/*********************************************************************
	 * extractRoute gets route from the node data after a full scan, route
	 * is written with just the basic way points and direction options for
	 * heading to the next point.
	 *********************************************************************/

	int32 prev;
	int32 prevx;
	int32 prevy;
	int32 last;
	int32 point;
	int32 dirx;
	int32 diry;
	int32 dir;
	int32 ldx;
	int32 ldy;
	int32 p;

	// extract the route from the node data

	prev = _nNodes;
	last = prev;
	point = O_ROUTE_SIZE - 1;
	_route[point].x = _node[last].x;
	_route[point].y = _node[last].y;

	do {
		point--;
		prev = _node[last].prev;
		prevx = _node[prev].x;
		prevy = _node[prev].y;
		_route[point].x = prevx;
		_route[point].y = prevy;
		last = prev;
	} while (prev > 0);

	// now shuffle route down in the buffer
	_routeLength = 0;

	do {
		_route[_routeLength].x = _route[point].x;
		_route[_routeLength].y = _route[point].y;
		point++;
		_routeLength++;
	} while (point < O_ROUTE_SIZE);

	_routeLength--;

	// okay the route exists as a series point now put in some directions
	for (p = 0; p < _routeLength; ++p) {
		ldx = _route[p + 1].x - _route[p].x;
		ldy = _route[p + 1].y - _route[p].y;
		dirx = 1;
		diry = 1;

		if (ldx < 0) {
			ldx = -ldx;
			dirx = -1;
		}

		if (ldy < 0) {
			ldy = -ldy;
			diry = -1;
		}

		if (_diagonaly * ldx > _diagonalx * ldy) {
			// dir  = 1,2 or 2,3 or 5,6 or 6,7

			// 2 or 6
			dir = 4 - 2 * dirx;
			_route[p].dirS = dir;

			// 1, 3, 5 or 7
			dir = dir + diry * dirx;
			_route[p].dirD = dir;
		} else {
			// dir  = 7,0 or 0,1 or 3,4 or 4,5

			// 0 or 4
			dir = 2 + 2 * diry;
			_route[p].dirS = dir;

			// 2 or 6
			dir = 4 - 2 * dirx;

			// 1, 3, 5 or 7
			dir = dir + diry * dirx;
			_route[p].dirD = dir;
		}
	}

	// set the last dir to continue previous route unless specified
	if (_targetDir == NO_DIRECTIONS) {
		// ANY direction
		_route[p].dirS = _route[p - 1].dirS;
		_route[p].dirD = _route[p - 1].dirD;
	} else {
		_route[p].dirS = _targetDir;
		_route[p].dirD = _targetDir;
	}

	return;
}

void Router::setUpWalkGrid(byte *ob_mega, int32 x, int32 y, int32 dir) {
	ObjectMega obMega(ob_mega);

	// get walk grid file + extra grid into 'bars' & 'node' arrays
	loadWalkGrid();

	// copy the mega structure into the local variables for use in all
	// subroutines

	_startX = obMega.getFeetX();
	_startY = obMega.getFeetY();
	_startDir = obMega.getCurDir();
	_targetX = x;
	_targetY = y;
	_targetDir = dir;

	_scaleA = obMega.getScaleA();
	_scaleB = obMega.getScaleB();

	// mega's current position goes into first node

	_node[0].x = _startX;
	_node[0].y = _startY;
	_node[0].level = 1;
	_node[0].prev = 0;
	_node[0].dist = 0;

	// reset other nodes

	for (int i = 1; i < _nNodes; i++) {
		_node[i].level = 0;
		_node[i].prev = 0;
		_node[i].dist = 9999;
	}

	// target position goes into final node
	_node[_nNodes].x = _targetX;
	_node[_nNodes].y = _targetY;
	_node[_nNodes].level = 0;
	_node[_nNodes].prev = 0;
	_node[_nNodes].dist = 9999;
}

void Router::plotWalkGrid() {
	int32 i;

	// get walk grid file + extra grid into 'bars' & 'node' arrays
	loadWalkGrid();

	// lines

	for (i = 0; i < _nBars; i++)
		_vm->_screen->drawLine(_bars[i].x1, _bars[i].y1, _bars[i].x2, _bars[i].y2, 254);

	// nodes

	// leave node 0 for start node
	for (i = 1; i < _nNodes; i++)
		plotCross(_node[i].x, _node[i].y, 184);
}

void Router::plotCross(int16 x, int16 y, uint8 color) {
	_vm->_screen->drawLine(x - 1, y - 1, x + 1, y + 1, color);
	_vm->_screen->drawLine(x + 1, y - 1, x - 1, y + 1, color);
}

void Router::loadWalkGrid() {
	WalkGridHeader floorHeader;
	byte *fPolygrid;
	uint16 fPolygridLen;

	_nBars	= 0;	// reset counts
	_nNodes	= 1;	// leave node 0 for start-node

	// STATIC GRIDS (added/removed by object logics)

	// go through walkgrid list
	for (int i = 0; i < MAX_WALKGRIDS; i++) {
		if (_walkGridList[i]) {
			int j;

			// open walk grid file
			fPolygrid = _vm->_resman->openResource(_walkGridList[i]);
			fPolygridLen = _vm->_resman->fetchLen(_walkGridList[i]);

			Common::MemoryReadStream readS(fPolygrid, fPolygridLen);

			readS.seek(ResHeader::size());

			floorHeader.numBars = readS.readSint32LE();
			floorHeader.numNodes = readS.readSint32LE();

			// check that we're not going to exceed the max
			// allowed in the complete walkgrid arrays

			assert(_nBars + floorHeader.numBars < O_GRID_SIZE);
			assert(_nNodes + floorHeader.numNodes < O_GRID_SIZE);

			// lines

			for (j = 0; j < floorHeader.numBars; j++) {
				_bars[_nBars + j].x1 = readS.readSint16LE();
				_bars[_nBars + j].y1 = readS.readSint16LE();
				_bars[_nBars + j].x2 = readS.readSint16LE();
				_bars[_nBars + j].y2 = readS.readSint16LE();
				_bars[_nBars + j].xmin = readS.readSint16LE();
				_bars[_nBars + j].ymin = readS.readSint16LE();
				_bars[_nBars + j].xmax = readS.readSint16LE();
				_bars[_nBars + j].ymax = readS.readSint16LE();
				_bars[_nBars + j].dx = readS.readSint16LE();
				_bars[_nBars + j].dy = readS.readSint16LE();
				_bars[_nBars + j].co = readS.readSint32LE();
			}

			// nodes

			// leave node 0 for start node
			for (j = 0; j < floorHeader.numNodes; j++) {
				_node[_nNodes + j].x = readS.readSint16LE();
				_node[_nNodes + j].y = readS.readSint16LE();
			}

			// close walk grid file
			_vm->_resman->closeResource(_walkGridList[i]);

			// increment counts of total bars & nodes in whole
			// walkgrid

			_nBars += floorHeader.numBars;
			_nNodes	+= floorHeader.numNodes;
		}
	}
}

void Router::clearWalkGridList() {
	memset(_walkGridList, 0, sizeof(_walkGridList));
}

// called from fnAddWalkGrid

void Router::addWalkGrid(int32 gridResource) {
	int i;
	// First, scan the list to see if this grid is already included

	for (i = 0; i < MAX_WALKGRIDS; i++) {
		if (_walkGridList[i] == gridResource)
			return;
	}

	// Scan the list for a free slot

	for (i = 0; i < MAX_WALKGRIDS; i++) {
		if (_walkGridList[i] == 0) {
			_walkGridList[i] = gridResource;
			return;
		}
	}

	error("_walkGridList[] full");
}

// called from fnRemoveWalkGrid

void Router::removeWalkGrid(int32 gridResource) {
	for (int i = 0; i < MAX_WALKGRIDS; i++) {
		if (_walkGridList[i] == gridResource) {
			// If we've found it in the list, reset entry to zero.
			// Otherwise just ignore the request.
			_walkGridList[i] = 0;
			break;
		}
	}
}

} // End of namespace Sword2
