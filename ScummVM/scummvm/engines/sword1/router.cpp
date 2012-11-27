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

#include "common/debug.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "sword1/router.h"
#include "sword1/swordres.h"
#include "sword1/sworddefs.h"
#include "sword1/objectman.h"
#include "sword1/resman.h"

namespace Sword1 {

/****************************************************************************
 *    JROUTER.C             polygon router with modular walks
 *                          using a tree of modules
 *                          21 july 94
 *    3  november 94
 *    System currently works by scanning grid data and coming up with a ROUTE
 *    as a series of way points(nodes), the smoothest eight directional PATH
 *      through these nodes is then found, and a WALK created to fit the PATH.
 *
 *      Two funtions are called by the user, RouteFinder creates a route as a
 *      module list, HardWalk creates an animation list from the module list.
 *      The split is only provided to allow the possibility of turning the
 *      autorouter over two game cycles.
 ****************************************************************************
 *
 *    Routine timings on osborne 486
 *
 *      Read floor resource (file already loaded)    112 pixels
 *
 *      Read mega resource (file already loaded)     112 pixels
 *
 *
 *
 ****************************************************************************
 *
 *    Modified 12 Oct 95
 *
 *      Target Points within 1 pixel of a line are ignored ???
 *
 *      Modules split into  Points within 1 pixel of a line are ignored ???
 *
 ****************************************************************************/

#define NO_DIRECTIONS    8
#define SLOW_IN          3
#define SLOW_OUT         7
#define ROUTE_END_FLAG 255

Router::Router(ObjectMan *pObjMan, ResMan *pResMan) {
	_objMan = pObjMan;
	_resMan = pResMan;
	_nNodes = _nBars = 0;
	_playerTargetX = _playerTargetY = _playerTargetDir = _playerTargetStance = 0;
	_diagonalx = _diagonaly = 0;
	_slidyWalkAnimatorState = false;
}

/*
 *    CODE
 */

int32 Router::routeFinder(int32 id, Object *megaObject, int32 x, int32 y, int32 dir) {
	/*********************************************************************
	 * RouteFinder.C        polygon router with modular walks
	 *                      21 august 94
	 *                      3  november 94
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
	 * return   0 = failed to find a route
	 *
	 *          1 = found a route
	 *
	 *          2 = mega already at target
	 *
	 *********************************************************************/

	int32 routeFlag = 0;
	int32 solidFlag = 0;
	WalkData *walkAnim;

	megaId = id;

	LoadWalkResources(megaObject, x, y, dir);

	walkAnim = megaObject->o_route;

	_framesPerStep = _nWalkFrames / 2;
	_framesPerChar = _nWalkFrames * NO_DIRECTIONS;

	// offset pointers added Oct 30 95 JPS
	standFrames = _framesPerChar;
	turnFramesLeft = standFrames;
	turnFramesRight = standFrames;
	walkFramesLeft = 0;
	walkFramesRight = 0;
	slowInFrames = 0;
	slowOutFrames = 0;

	if (megaId == GEORGE) {
		turnFramesLeft = 3 * _framesPerChar + NO_DIRECTIONS + 2 * SLOW_IN + 4 * SLOW_OUT;
		turnFramesRight = 3 * _framesPerChar + NO_DIRECTIONS + 2 * SLOW_IN + 4 * SLOW_OUT + NO_DIRECTIONS;
		walkFramesLeft = _framesPerChar + NO_DIRECTIONS;
		walkFramesRight = 2 * _framesPerChar + NO_DIRECTIONS;
		slowInFrames = 3 * _framesPerChar + NO_DIRECTIONS;
		slowOutFrames = 3 * _framesPerChar + NO_DIRECTIONS + 2 * SLOW_IN;
	} else if (megaId == NICO) {
		turnFramesLeft = _framesPerChar + NO_DIRECTIONS;
		turnFramesRight = _framesPerChar + 2 * NO_DIRECTIONS;
		walkFramesLeft = 0;
		walkFramesRight = 0;
		slowInFrames = 0;
		slowOutFrames = 0;
	}

	// **************************************************************************
	// All route data now loaded start finding a route
	// **************************************************************************
	// **************************************************************************
	// check if we can get a route through the floor        changed 12 Oct95 JPS
	// **************************************************************************

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

		if (_targetDir == NO_DIRECTIONS) {
			// can end facing ANY direction (ie. exact end
			// position not vital) - so use SOLID walk to
			// avoid sliding to exact position

			solidPath();
			solidFlag = solidWalkAnimator(walkAnim);
		}

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

	return routeFlag;   // send back null route
}

int32 Router::getRoute() {
	/*********************************************************************
	 * GetRoute.C               extract a path from walk grid
	 *                          12 october 94
	 *
	 * GetRoute currently works by scanning grid data and coming up with
	 * a ROUTE as a series of way points(nodes).
	 *
	 * static routeData _route[O_ROUTE_SIZE];
	 *
	 * return   0 = failed to find a route
	 *
	 *      1 = found a route
	 *
	 *      2 = mega already at target
	 *
	 *      3 = failed to find a route because target was on a line
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

	if (dirS == 0 || dirS == 4) {   // vert and diag
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
	case 0:     // halfsquare, diagonal, halfsquare
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
	case 1:     // square, diagonal
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
	case 2:     // diagonal square
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
	default:    // halfdiagonal, square, halfdiagonal
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

void Router::slidyWalkAnimator(WalkData *walkAnim) {
	/*********************************************************************
	 * Skidding every where HardWalk creates an animation that exactly
	 * fits the smoothPath and uses foot slipping to fit whole steps into
	 * the route
	 *
	 *  Parameters: georgeg, mouseg
	 *  Returns:    rout
	 *
	 * produce a module list from the line data
	 *********************************************************************/

	int32 p;
	int32 lastDir;
	int32 lastRealDir;
	int32 currentDir;
	int32 turnDir;
	int32 scale;
	int32 step;
	int32 module;
	int32 moduleEnd;
	int32 moduleX;
	int32 moduleY;
	int32 module16X = 0;
	int32 module16Y = 0;
	int32 stepX;
	int32 stepY;
	int32 errorX;
	int32 errorY;
	int32 lastErrorX;
	int32 lastErrorY;
	int32 lastCount;
	int32 stepCount;
	int32 frameCount;
	int32 frames;
	int32 frame;

	// start at the begining for a change
	p = 0;
	lastDir = _modularPath[0].dir;
	currentDir = _modularPath[1].dir;

	if (currentDir == NO_DIRECTIONS)
		currentDir = lastDir;

	moduleX = _startX;
	moduleY = _startY;
	module16X = moduleX << 16;
	module16Y = moduleY << 16;
	stepCount = 0;

	//****************************************************************************
	// SLIDY
	// START THE WALK WITH THE FIRST STANDFRAME THIS MAY CAUSE A DELAY
	// BUT IT STOPS THE PLAYER MOVING FOR COLLISIONS ARE DETECTED
	//****************************************************************************
	module = _framesPerChar + lastDir;
	walkAnim[stepCount].frame = module;
	walkAnim[stepCount].step = 0;
	walkAnim[stepCount].dir = lastDir;
	walkAnim[stepCount].x = moduleX;
	walkAnim[stepCount].y = moduleY;
	stepCount += 1;

	//****************************************************************************
	// SLIDY
	// TURN TO START THE WALK
	//****************************************************************************
	// rotate if we need to
	if (lastDir != currentDir) {
		// get the direction to turn
		turnDir = currentDir - lastDir;
		if (turnDir < 0)
			turnDir += NO_DIRECTIONS;

		if (turnDir > 4)
			turnDir = -1;
		else if (turnDir > 0)
			turnDir = 1;

		// rotate to new walk direction
		// for george and nico put in a head turn at the start
		if ((megaId == GEORGE) || (megaId == NICO)) {
			if (turnDir < 0) {  // new frames for turn frames   29oct95jps
				module = turnFramesLeft + lastDir;
			} else {
				module = turnFramesRight + lastDir;
			}
			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}

		// rotate till were facing new dir then go back 45 degrees
		while (lastDir != currentDir) {
			lastDir += turnDir;
			if (turnDir < 0) {  // new frames for turn frames   29oct95jps
				if (lastDir < 0)
					lastDir += NO_DIRECTIONS;
				module = turnFramesLeft + lastDir;
			} else {
				if (lastDir > 7)
					lastDir -= NO_DIRECTIONS;
				module = turnFramesRight + lastDir;
			}
			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}
		// the back 45 degrees bit
		stepCount -= 1;// step back one because new head turn for george takes us past the new dir
	}
	// his head is in the right direction
	lastRealDir = currentDir;

	//****************************************************************************
	// SLIDY
	// THE WALK
	//****************************************************************************

	_slidyWalkAnimatorState = !_slidyWalkAnimatorState;

	lastCount = stepCount;
	lastDir = 99;// this ensures that we don't put in turn frames for the start
	currentDir = 99;// this ensures that we don't put in turn frames for the start
	do {
		while (_modularPath[p].num == 0) {
			p = p + 1;
			if (currentDir != 99)
				lastRealDir = currentDir;
			lastDir = currentDir;
			lastCount = stepCount;
		}
		//calculate average amount to lose in each step on the way to the next _node
		currentDir = _modularPath[p].dir;
		if (currentDir < NO_DIRECTIONS) {
			module = currentDir * _framesPerStep * 2 + _slidyWalkAnimatorState * _framesPerStep;
			_slidyWalkAnimatorState = !_slidyWalkAnimatorState;
			moduleEnd = module + _framesPerStep;
			step = 0;
			scale = (_scaleA * moduleY + _scaleB);
			do {
				module16X += _dx[module] * scale;
				module16Y += _dy[module] * scale;
				moduleX = module16X >> 16;
				moduleY = module16Y >> 16;
				walkAnim[stepCount].frame = module;
				walkAnim[stepCount].step = step;
				walkAnim[stepCount].dir = currentDir;
				walkAnim[stepCount].x = moduleX;
				walkAnim[stepCount].y = moduleY;
				stepCount += 1;
				step += 1;
				module += 1;
			} while (module < moduleEnd);
			stepX = _modX[_modularPath[p].dir];
			stepY = _modY[_modularPath[p].dir];
			errorX = _modularPath[p].x - moduleX;
			errorX = errorX * stepX;
			errorY = _modularPath[p].y - moduleY;
			errorY = errorY * stepY;
			if ((errorX < 0) || (errorY < 0)) {
				_modularPath[p].num = 0;    // the end of the path
				// okay those last steps took us past our target but do we want to scoot or moonwalk
				frames = stepCount - lastCount;
				errorX = _modularPath[p].x - walkAnim[stepCount - 1].x;
				errorY = _modularPath[p].y - walkAnim[stepCount - 1].y;

				if (frames > _framesPerStep) {
					lastErrorX = _modularPath[p].x - walkAnim[stepCount - 7].x;
					lastErrorY = _modularPath[p].y - walkAnim[stepCount - 7].y;
					if (stepX == 0) {
						if (3 * ABS(lastErrorY) < ABS(errorY)) { //the last stop was closest
							stepCount -= _framesPerStep;
							_slidyWalkAnimatorState = !_slidyWalkAnimatorState;
						}
					} else {
						if (3 * ABS(lastErrorX) < ABS(errorX)) { //the last stop was closest
							stepCount -= _framesPerStep;
							_slidyWalkAnimatorState = !_slidyWalkAnimatorState;
						}
					}
				}
				errorX = _modularPath[p].x - walkAnim[stepCount - 1].x;
				errorY = _modularPath[p].y - walkAnim[stepCount - 1].y;
				// okay we've reached the end but we still have an error
				if (errorX != 0) {
					frameCount = 0;
					frames = stepCount - lastCount;
					do {
						frameCount += 1;
						walkAnim[lastCount + frameCount - 1].x += errorX * frameCount / frames;
					} while (frameCount < frames);
				}
				if (errorY != 0) {
					frameCount = 0;
					frames = stepCount - lastCount;
					do {
						frameCount += 1;
						walkAnim[lastCount + frameCount - 1].y += errorY * frameCount / frames;
					} while (frameCount < frames);
				}
				// Now is the time to put in the turn frames for the last turn
				if (frames < _framesPerStep)
					currentDir = 99;// this ensures that we don't put in turn frames for this walk or the next
				if (currentDir != 99)
					lastRealDir = currentDir;
				// check each turn condition in turn
				if (((lastDir != 99) && (currentDir != 99)) && (megaId == GEORGE)) { // only for george
					lastDir = currentDir - lastDir;//1 and -7 going right -1 and 7 going left
					if (((lastDir == -1) || (lastDir == 7)) || ((lastDir == -2) || (lastDir == 6))) {
						// turn at the end of the last walk
						frame = lastCount - _framesPerStep;
						do {
							walkAnim[frame].frame += 104;//turning left
							frame += 1;
						} while (frame < lastCount);
					}
					if (((lastDir == 1) || (lastDir == -7)) || ((lastDir == 2) || (lastDir == -6))) {
						// turn at the end of the current walk
						frame = lastCount - _framesPerStep;
						do {
							walkAnim[frame].frame += 200; //was 60 now 116
							frame += 1;
						} while (frame < lastCount);
					}
					lastDir = currentDir;
				}
				// all turns checked

				lastCount = stepCount;
				moduleX = walkAnim[stepCount - 1].x;
				moduleY = walkAnim[stepCount - 1].y;
				module16X = moduleX << 16;
				module16Y = moduleY << 16;
			}
		}
	} while (_modularPath[p].dir < NO_DIRECTIONS);



	if (lastRealDir == 99) {
		error("SlidyWalkAnimatorlast direction error");
	}
	//****************************************************************************
	// SLIDY
	// TURNS TO END THE WALK ?
	//****************************************************************************

	// We've done the walk now put in any turns at the end


	if (_targetDir == NO_DIRECTIONS) {  // stand in the last direction
		module = standFrames + lastRealDir;
		_targetDir = lastRealDir;
		walkAnim[stepCount].frame = module;
		walkAnim[stepCount].step = 0;
		walkAnim[stepCount].dir = lastRealDir;
		walkAnim[stepCount].x = moduleX;
		walkAnim[stepCount].y = moduleY;
		stepCount += 1;
	}
	if (_targetDir == 9) {
		if (stepCount == 0) {
			module = _framesPerChar + lastRealDir;
			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastRealDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}
	} else if (_targetDir != lastRealDir) { // rotate to _targetDir
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
		if ((megaId == GEORGE) || (megaId == NICO)) {
			if (turnDir < 0) {  // new frames for turn frames   29oct95jps
				module = turnFramesLeft + lastDir;
			} else {
				module = turnFramesRight + lastDir;
			}
			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastRealDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}

		// rotate if we need to
		while (lastRealDir != _targetDir) {
			lastRealDir += turnDir;
			if (turnDir < 0) {  // new frames for turn frames   29oct95jps
				if (lastRealDir < 0)
					lastRealDir += NO_DIRECTIONS;
				module = turnFramesLeft + lastRealDir;
			} else {
				if (lastRealDir > 7)
					lastRealDir -= NO_DIRECTIONS;
				module = turnFramesRight + lastRealDir;
			}
			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastRealDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}
		module = standFrames + lastRealDir;
		walkAnim[stepCount - 1].frame = module;
	} else { // just stand at the end
		module = standFrames + lastRealDir;
		walkAnim[stepCount].frame = module;
		walkAnim[stepCount].step = 0;
		walkAnim[stepCount].dir = lastRealDir;
		walkAnim[stepCount].x = moduleX;
		walkAnim[stepCount].y = moduleY;
		stepCount += 1;
	}

	walkAnim[stepCount].frame = 512;
	stepCount += 1;
	walkAnim[stepCount].frame = 512;
	stepCount += 1;
	walkAnim[stepCount].frame = 512;
	//Tdebug("RouteFinder RouteSize is %d", stepCount);
	return;
}

// ****************************************************************************
// * THE SOLID PATH ROUTINES
// ****************************************************************************

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
	int32 lastDir;
	int32 currentDir;
	int32 turnDir;
	int32 scale;
	int32 step;
	int32 module;
	int32 moduleX;
	int32 moduleY;
	int32 module16X;
	int32 module16Y;
	int32 errorX;
	int32 errorY;
	int32 moduleEnd;
	int32 slowStart;
	int32 stepCount;
	int32 lastCount;
	int32 frame;

	// start at the begining for a change
	lastDir = _modularPath[0].dir;
	currentDir = _modularPath[1].dir;
	module =    _framesPerChar + lastDir;
	moduleX = _startX;
	moduleY = _startY;
	module16X = moduleX << 16;
	module16Y = moduleY << 16;
	slowStart = 0;
	stepCount = 0;

	//****************************************************************************
	// SOLID
	// START THE WALK WITH THE FIRST STANDFRAME THIS MAY CAUSE A DELAY
	// BUT IT STOPS THE PLAYER MOVING FOR COLLISIONS ARE DETECTED
	//****************************************************************************
	walkAnim[stepCount].frame = module;
	walkAnim[stepCount].step = 0;
	walkAnim[stepCount].dir = lastDir;
	walkAnim[stepCount].x = moduleX;
	walkAnim[stepCount].y = moduleY;
	stepCount += 1;

	//****************************************************************************
	// SOLID
	// TURN TO START THE WALK
	//****************************************************************************
	// rotate if we need to
	if (lastDir != currentDir) {
		// get the direction to turn
		turnDir = currentDir - lastDir;
		if (turnDir < 0)
			turnDir += NO_DIRECTIONS;

		if (turnDir > 4)
			turnDir = -1;
		else if (turnDir > 0)
			turnDir = 1;

		// rotate to new walk direction
		// for george and nico put in a head turn at the start
		if ((megaId == GEORGE) || (megaId == NICO)) {
			if (turnDir < 0) {  // new frames for turn frames   29oct95jps
				module = turnFramesLeft + lastDir;
			} else {
				module = turnFramesRight + lastDir;
			}
			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}

		// rotate till were facing new dir then go back 45 degrees
		while (lastDir != currentDir) {
			lastDir += turnDir;
			if (turnDir < 0) {  // new frames for turn frames   29oct95jps
				if (lastDir < 0)
					lastDir += NO_DIRECTIONS;
				module = turnFramesLeft + lastDir;
			} else {
				if (lastDir > 7)
					lastDir -= NO_DIRECTIONS;
				module = turnFramesRight + lastDir;
			}
			walkAnim[stepCount].frame = module;
			walkAnim[stepCount].step = 0;
			walkAnim[stepCount].dir = lastDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}
		// the back 45 degrees bit
		stepCount -= 1;// step back one because new head turn for george takes us past the new dir
	}

	//****************************************************************************
	// SOLID
	// THE SLOW IN
	//****************************************************************************

	// do start frames if its george and left or right
	if (megaId == GEORGE) {
		if (_modularPath[1].num > 0) {
			if (currentDir == 2) { // only for george
				slowStart = 1;
				walkAnim[stepCount].frame = 296;
				walkAnim[stepCount].step = 0;
				walkAnim[stepCount].dir = currentDir;
				walkAnim[stepCount].x = moduleX;
				walkAnim[stepCount].y = moduleY;
				stepCount += 1;
				walkAnim[stepCount].frame = 297;
				walkAnim[stepCount].step = 0;
				walkAnim[stepCount].dir = currentDir;
				walkAnim[stepCount].x = moduleX;
				walkAnim[stepCount].y = moduleY;
				stepCount += 1;
				walkAnim[stepCount].frame = 298;
				walkAnim[stepCount].step = 0;
				walkAnim[stepCount].dir = currentDir;
				walkAnim[stepCount].x = moduleX;
				walkAnim[stepCount].y = moduleY;
				stepCount += 1;
			} else if (currentDir == 6) { // only for george
				slowStart = 1;
				walkAnim[stepCount].frame = 299;
				walkAnim[stepCount].step = 0;
				walkAnim[stepCount].dir = currentDir;
				walkAnim[stepCount].x = moduleX;
				walkAnim[stepCount].y = moduleY;
				stepCount += 1;
				walkAnim[stepCount].frame = 300;
				walkAnim[stepCount].step = 0;
				walkAnim[stepCount].dir = currentDir;
				walkAnim[stepCount].x = moduleX;
				walkAnim[stepCount].y = moduleY;
				stepCount += 1;
				walkAnim[stepCount].frame = 301;
				walkAnim[stepCount].step = 0;
				walkAnim[stepCount].dir = currentDir;
				walkAnim[stepCount].x = moduleX;
				walkAnim[stepCount].y = moduleY;
				stepCount += 1;
			}
		}
	}
	//****************************************************************************
	// SOLID
	// THE WALK
	//****************************************************************************

	if (currentDir > 4)
		left = 1;
	else
		left = 0;

	lastCount = stepCount;
	lastDir = 99;// this ensures that we don't put in turn frames for the start
	currentDir = 99;// this ensures that we don't put in turn frames for the start

	int32 p;

	for (p = 1; _modularPath[p].dir < NO_DIRECTIONS; ++p) {
		while (_modularPath[p].num > 0) {
			currentDir = _modularPath[p].dir;
			if (currentDir < NO_DIRECTIONS) {

				module = currentDir * _framesPerStep * 2 + left * _framesPerStep;
				left = !left;
				moduleEnd = module + _framesPerStep;
				step = 0;
				scale = (_scaleA * moduleY + _scaleB);
				do {
					module16X += _dx[module] * scale;
					module16Y += _dy[module] * scale;
					moduleX = module16X >> 16;
					moduleY = module16Y >> 16;
					walkAnim[stepCount].frame = module;
					walkAnim[stepCount].step = step;
					walkAnim[stepCount].dir = currentDir;
					walkAnim[stepCount].x = moduleX;
					walkAnim[stepCount].y = moduleY;
					stepCount += 1;
					module += 1;
					step += 1;
				} while (module < moduleEnd);
				errorX = _modularPath[p].x - moduleX;
				errorX = errorX * _modX[_modularPath[p].dir];
				errorY = _modularPath[p].y - moduleY;
				errorY = errorY * _modY[_modularPath[p].dir];
				if ((errorX < 0) || (errorY < 0)) {
					_modularPath[p].num = 0;
					stepCount -= _framesPerStep;
					left = !left;
					// Okay this is the end of a section
					moduleX = walkAnim[stepCount - 1].x;
					moduleY = walkAnim[stepCount - 1].y;
					module16X = moduleX << 16;
					module16Y = moduleY << 16;
					_modularPath[p].x = moduleX;
					_modularPath[p].y = moduleY;
					// Now is the time to put in the turn frames for the last turn
					if ((stepCount - lastCount) < _framesPerStep) { // no step taken
						currentDir = 99;// this ensures that we don't put in turn frames for this walk or the next
						if (slowStart == 1) { // clean up if a slow in but no walk
							stepCount -= 3;
							lastCount -= 3;
							slowStart = 0;
						}
					}
					// check each turn condition in turn
					if (((lastDir != 99) && (currentDir != 99)) && (megaId == GEORGE)) { // only for george
						lastDir = currentDir - lastDir;//1 and -7 going right -1 and 7 going left
						if (((lastDir == -1) || (lastDir == 7)) || ((lastDir == -2) || (lastDir == 6))) {
							// turn at the end of the last walk
							frame = lastCount - _framesPerStep;
							do {
								walkAnim[frame].frame += 104;//turning left
								frame += 1;
							} while (frame < lastCount);
						}
						if (((lastDir == 1) || (lastDir == -7)) || ((lastDir == 2) || (lastDir == -6))) {
							// turn at the end of the current walk
							frame = lastCount - _framesPerStep;
							do {
								walkAnim[frame].frame += 200; //was 60 now 116
								frame += 1;
							} while (frame < lastCount);
						}
					}
					// all turns checked
					lastCount = stepCount;
				}
			}
		}
		lastDir = currentDir;
		slowStart = 0; //can only be valid first time round
	}

	//****************************************************************************
	// SOLID
	// THE SLOW OUT
	//****************************************************************************

	if ((currentDir == 2) && (megaId == GEORGE)) { // only for george
		// place stop frames here
		// slowdown at the end of the last walk
		frame = lastCount - _framesPerStep;
		if (walkAnim[frame].frame == 24) {
			do {
				walkAnim[frame].frame += 278;//stopping right
				frame += 1;
			} while (frame < lastCount);
			walkAnim[stepCount].frame = 308;
			walkAnim[stepCount].step = 7;
			walkAnim[stepCount].dir = currentDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		} else if (walkAnim[frame].frame == 30) {
			do {
				walkAnim[frame].frame += 279;//stopping right
				frame += 1;
			} while (frame < lastCount);
			walkAnim[stepCount].frame = 315;
			walkAnim[stepCount].step = 7;
			walkAnim[stepCount].dir = currentDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}
	} else if ((currentDir == 6) && (megaId == GEORGE)) { // only for george
		// place stop frames here
		// slowdown at the end of the last walk
		frame = lastCount - _framesPerStep;
		if (walkAnim[frame].frame == 72) {
			do {
				walkAnim[frame].frame += 244;//stopping left
				frame += 1;
			} while (frame < lastCount);
			walkAnim[stepCount].frame = 322;
			walkAnim[stepCount].step = 7;
			walkAnim[stepCount].dir = currentDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		} else if (walkAnim[frame].frame == 78) {
			do {
				walkAnim[frame].frame += 245;//stopping left
				frame += 1;
			} while (frame < lastCount);
			walkAnim[stepCount].frame = 329;
			walkAnim[stepCount].step = 7;
			walkAnim[stepCount].dir = currentDir;
			walkAnim[stepCount].x = moduleX;
			walkAnim[stepCount].y = moduleY;
			stepCount += 1;
		}
	}
	module = _framesPerChar + _modularPath[p - 1].dir;
	walkAnim[stepCount].frame = module;
	walkAnim[stepCount].step = 0;
	walkAnim[stepCount].dir = _modularPath[p - 1].dir;
	walkAnim[stepCount].x = moduleX;
	walkAnim[stepCount].y = moduleY;
	stepCount += 1;

	walkAnim[stepCount].frame = 512;
	stepCount += 1;
	walkAnim[stepCount].frame = 512;
	stepCount += 1;
	walkAnim[stepCount].frame = 512;

	//****************************************************************************
	// SOLID
	// NO END TURNS
	//****************************************************************************

	debug(5, "routeFinder RouteSize is %d", stepCount);
	// now check the route

	for (int i = 0; i < p - 1; ++i) {
		if (!check(_modularPath[i].x, _modularPath[i].y, _modularPath[i + 1].x, _modularPath[i + 1].y))
			p = 0;
	}

	if (p != 0) {
		_targetDir = _modularPath[p - 1].dir;
		if (checkTarget(moduleX, moduleY) == 3) {
			// new target on a line
			p = 0;
			debug(5, "Solid walk target was on a line %d %d", moduleX, moduleY);
		}
	}

	return p;
}

// ****************************************************************************
// * THE SCAN ROUTINES
// ****************************************************************************

bool Router::scan(int32 level) {
	/*********************************************************************
	 * Called successively from routeFinder until no more changes take
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
		step1 = check(x1 , y1, x1, y1 + ldy);
		if (step1 != 0) {
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
					if (step3 != 0) {
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

// ****************************************************************************
// * CHECK ROUTINES
// ****************************************************************************

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

	int32 co = (y1 * dirx) - (x1 * diry);       // new line equation

	for (int i = 0; i < _nBars && linesCrossed; i++) {
		// skip if not on module
		if (xmax >= _bars[i].xmin && xmin <= _bars[i].xmax && ymax >= _bars[i].ymin && ymin <= _bars[i].ymax) {
			// Okay, it's a valid line. Calculate an intercept. Wow
			// but all this arithmetic we must have loads of time

			// slope it he slope between the two lines
			int32 slope = (_bars[i].dx * diry) - (_bars[i].dy * dirx);
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

// ****************************************************************************
// * THE SETUP ROUTINES
// ****************************************************************************

int32 Router::LoadWalkResources(Object *megaObject, int32 x, int32 y, int32 dir) {
	WalkGridHeader floorHeader;
	int32 i;
	uint8 *fPolygrid;
	uint8 *fMegaWalkData;

	int32 floorId;
	int32 walkGridResourceId;
	Object *floorObject;

	int32 cnt;
	uint32 cntu;

	// load in floor grid for current mega

	floorId = megaObject->o_place;

	//floorObject = (object *)Lock_object(floorId);
	floorObject = _objMan->fetchObject(floorId);
	walkGridResourceId = floorObject->o_resource;
	//Unlock_object(floorId);

	//ResOpen(walkGridResourceId);          // mouse wiggle
	//fPolygrid = ResLock(walkGridResourceId);          // mouse wiggle
	fPolygrid = (uint8 *)_resMan->openFetchRes(walkGridResourceId);


	fPolygrid += sizeof(Header);
	memcpy(&floorHeader, fPolygrid, sizeof(WalkGridHeader));
	fPolygrid += sizeof(WalkGridHeader);
	_nBars = _resMan->getUint32(floorHeader.numBars);

	if (_nBars >= O_GRID_SIZE) {
#ifdef DEBUG //check for id > number in file,
		error("RouteFinder Error too many _bars %d", _nBars);
#endif
		_nBars = 0;
	}

	_nNodes = _resMan->getUint32(floorHeader.numNodes) + 1; //array starts at 0 begins at a start _node has nnodes nodes and a target _node

	if (_nNodes >= O_GRID_SIZE) {
#ifdef DEBUG //check for id > number in file,
		error("RouteFinder Error too many nodes %d", _nNodes);
#endif
		_nNodes = 0;
	}

	/*memmove(&_bars[0],fPolygrid,_nBars*sizeof(BarData));
	fPolygrid += _nBars*sizeof(BarData);//move pointer to start of _node data*/
	for (cnt = 0; cnt < _nBars; cnt++) {
		_bars[cnt].x1   = _resMan->readUint16(fPolygrid); fPolygrid += 2;
		_bars[cnt].y1   = _resMan->readUint16(fPolygrid); fPolygrid += 2;
		_bars[cnt].x2   = _resMan->readUint16(fPolygrid); fPolygrid += 2;
		_bars[cnt].y2   = _resMan->readUint16(fPolygrid); fPolygrid += 2;
		_bars[cnt].xmin = _resMan->readUint16(fPolygrid); fPolygrid += 2;
		_bars[cnt].ymin = _resMan->readUint16(fPolygrid); fPolygrid += 2;
		_bars[cnt].xmax = _resMan->readUint16(fPolygrid); fPolygrid += 2;
		_bars[cnt].ymax = _resMan->readUint16(fPolygrid); fPolygrid += 2;
		_bars[cnt].dx   = _resMan->readUint16(fPolygrid); fPolygrid += 2;
		_bars[cnt].dy   = _resMan->readUint16(fPolygrid); fPolygrid += 2;
		_bars[cnt].co   = _resMan->readUint32(fPolygrid); fPolygrid += 4;
	}

	/*j = 1;// leave _node 0 for start _node
	do {
	    memmove(&_node[j].x,fPolygrid,2*sizeof(int16));
	    fPolygrid += 2*sizeof(int16);
	    j ++;
	} while (j < _nNodes);//array starts at 0*/
	for (cnt = 1; cnt < _nNodes; cnt++) {
		_node[cnt].x = _resMan->readUint16(fPolygrid); fPolygrid += 2;
		_node[cnt].y = _resMan->readUint16(fPolygrid); fPolygrid += 2;
	}

	//ResUnlock(walkGridResourceId);            // mouse wiggle
	//ResClose(walkGridResourceId);         // mouse wiggle
	_resMan->resClose(walkGridResourceId);


	// floor grid loaded

	// copy the mega structure into the local variables for use in all subroutines

	_startX = megaObject->o_xcoord;
	_startY = megaObject->o_ycoord;
	_startDir = megaObject->o_dir;
	_targetX = x;
	_targetY = y;
	_targetDir = dir;

	_scaleA = megaObject->o_scale_a;
	_scaleB = megaObject->o_scale_b;

	//ResOpen(megaObject->o_mega_resource);         // mouse wiggle
	//fMegaWalkData = ResLock(megaObject->o_mega_resource);         // mouse wiggle
	fMegaWalkData = (uint8 *)_resMan->openFetchRes(megaObject->o_mega_resource);
	// Apparently this resource is in little endian in both the Mac and the PC version

	_nWalkFrames = fMegaWalkData[0];
	_nTurnFrames = fMegaWalkData[1];
	fMegaWalkData += 2;
	for (cnt = 0; cnt < NO_DIRECTIONS * (_nWalkFrames + 1 + _nTurnFrames); cnt++) {
		_dx[cnt] = (int32)_resMan->readLEUint32(fMegaWalkData);
		fMegaWalkData += 4;
	}
	for (cnt = 0; cnt < NO_DIRECTIONS * (_nWalkFrames + 1 + _nTurnFrames); cnt++) {
		_dy[cnt] = (int32)_resMan->readLEUint32(fMegaWalkData);
		fMegaWalkData += 4;
	}
	/*memmove(&_dx[0],fMegaWalkData,NO_DIRECTIONS*(_nWalkFrames+1+_nTurnFrames)*sizeof(int32));
	fMegaWalkData += NO_DIRECTIONS*(_nWalkFrames+1+_nTurnFrames)*sizeof(int32);
	memmove(&_dy[0],fMegaWalkData,NO_DIRECTIONS*(_nWalkFrames+1+_nTurnFrames)*sizeof(int32));
	fMegaWalkData += NO_DIRECTIONS*(_nWalkFrames+1+_nTurnFrames)*sizeof(int32);*/

	for (cntu = 0; cntu < NO_DIRECTIONS; cntu++) {
		_modX[cntu] = (int32)_resMan->readLEUint32(fMegaWalkData);
		fMegaWalkData += 4;
	}
	for (cntu = 0; cntu < NO_DIRECTIONS; cntu++) {
		_modY[cntu] = (int32)_resMan->readLEUint32(fMegaWalkData);
		fMegaWalkData += 4;
	}
	/*memmove(&_modX[0],fMegaWalkData,NO_DIRECTIONS*sizeof(int32));
	fMegaWalkData += NO_DIRECTIONS*sizeof(int32);
	memmove(&_modY[0],fMegaWalkData,NO_DIRECTIONS*sizeof(int32));
	fMegaWalkData += NO_DIRECTIONS*sizeof(int32);*/

	//ResUnlock(megaObject->o_mega_resource);           // mouse wiggle
	//ResClose(megaObject->o_mega_resource);            // mouse wiggle
	_resMan->resClose(megaObject->o_mega_resource);

	_diagonalx =  _modX[3]; //36
	_diagonaly =  _modY[3]; //8

	// mega data ready

	// finish setting grid by putting mega _node at begining
	// and target _node at end  and reset current values
	_node[0].x = _startX;
	_node[0].y = _startY;
	_node[0].level = 1;
	_node[0].prev = 0;
	_node[0].dist = 0;
	i = 1;
	do {
		_node[i].level = 0;
		_node[i].prev = 0;
		_node[i].dist = 9999;
		i = i + 1;
	} while (i < _nNodes);
	_node[_nNodes].x = _targetX;
	_node[_nNodes].y = _targetY;
	_node[_nNodes].level = 0;
	_node[_nNodes].prev = 0;
	_node[_nNodes].dist = 9999;

	return 1;
}

// ****************************************************************************
// * THE ROUTE EXTRACTOR
// ****************************************************************************

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

	// extract the route from the _node data
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

#define DIAGONALX 36
#define DIAGONALY 8
int whatTarget(int32 startX, int32 startY, int32 destX, int32 destY) {
	int tar_dir;
	//setting up
	int deltaX = destX - startX;
	int deltaY = destY - startY;
	int signX = (deltaX > 0);
	int signY = (deltaY > 0);
	int slope;

	if ((ABS(deltaY) * DIAGONALX) < (ABS(deltaX) * DIAGONALY / 2))
		slope = 0;// its flat
	else if ((ABS(deltaY) * DIAGONALX / 2) > (ABS(deltaX) * DIAGONALY))
		slope = 2;// its vertical
	else
		slope = 1;// its diagonal

	if (slope == 0) { //flat
		if (signX == 1) // going right
			tar_dir = 2;
		else
			tar_dir = 6;
	} else if (slope == 2) { //vertical
		if (signY == 1) // going down
			tar_dir = 4;
		else
			tar_dir = 0;
	} else if (signX == 1) { //right diagonal
		if (signY == 1) // going down
			tar_dir = 3;
		else
			tar_dir = 1;
	} else { //left diagonal
		if (signY == 1) // going down
			tar_dir = 5;
		else
			tar_dir = 7;
	}
	return tar_dir;
}

void Router::setPlayerTarget(int32 x, int32 y, int32 dir, int32 stance) {
	_playerTargetX = x;
	_playerTargetY = y;
	_playerTargetDir = dir;
	_playerTargetStance = stance;
}

} // End of namespace Sword1
