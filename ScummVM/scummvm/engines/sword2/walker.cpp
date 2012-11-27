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

// WALKER.CPP by James (14nov96)

// Functions for moving megas about the place & also for keeping tabs on them



#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/resman.h"
#include "sword2/router.h"
#include "sword2/screen.h"

namespace Sword2 {

void Router::setStandbyCoords(int16 x, int16 y, uint8 dir) {
	assert(dir <= 7);

	_standbyX = x;
	_standbyY = y;
	_standbyDir = dir;
}

/**
 * Work out direction from start to dest.
 */

// Used in whatTarget(); not valid for all megas
#define	diagonalx 36
#define	diagonaly 8

int Router::whatTarget(int startX, int startY, int destX, int destY) {
	int deltaX = destX - startX;
	int deltaY = destY - startY;

	// 7 0 1
	// 6   2
	// 5 4 3

	// Flat route

	if (ABS(deltaY) * diagonalx < ABS(deltaX) * diagonaly / 2)
		return (deltaX > 0) ? 2 : 6;

	// Vertical route

	if (ABS(deltaY) * diagonalx / 2 > ABS(deltaX) * diagonaly)
		return (deltaY > 0) ? 4 : 0;

	// Diagonal route

	if (deltaX > 0)
		return (deltaY > 0) ? 3 : 1;

	return (deltaY > 0) ? 5 : 7;
}

/**
 * Walk meta to (x,y,dir). Set RESULT to 0 if it succeeded. Otherwise, set
 * RESULT to 1. Return true if the mega has finished walking.
 */

int Router::doWalk(byte *ob_logic, byte *ob_graph, byte *ob_mega, byte *ob_walkdata, int16 target_x, int16 target_y, uint8 target_dir) {
	ObjectLogic obLogic(ob_logic);
	ObjectGraphic obGraph(ob_graph);
	ObjectMega obMega(ob_mega);

	// If this is the start of the walk, calculate the route.

	if (obLogic.getLooping() == 0) {
		// If we're already there, don't even bother allocating
		// memory and calling the router, just quit back & continue
		// the script! This avoids an embarassing mega stand frame
		// appearing for one cycle when we're already in position for
		// an anim eg. repeatedly clicking on same object to repeat
		// an anim - no mega frame will appear in between runs of the
		// anim.

		if (obMega.getFeetX() == target_x && obMega.getFeetY() == target_y && obMega.getCurDir() == target_dir) {
			_vm->_logic->writeVar(RESULT, 0);
			return IR_CONT;
		}

		assert(target_dir <= 8);

		obMega.setWalkPc(0);

		// Set up mem for _walkData in route_slots[] & set mega's
		// 'route_slot_id' accordingly
		allocateRouteMem();

		int32 route = routeFinder(ob_mega, ob_walkdata, target_x, target_y, target_dir);

		// 0 = can't make route to target
		// 1 = created route
		// 2 = zero route but may need to turn

		if (route != 1 && route != 2) {
			freeRouteMem();
			_vm->_logic->writeVar(RESULT, 1);
			return IR_CONT;
		}

		// Walk is about to start

		obMega.setIsWalking(1);
		obLogic.setLooping(1);
		obGraph.setAnimResource(obMega.getMegasetRes());
	} else if (_vm->_logic->readVar(EXIT_FADING) && _vm->_screen->getFadeStatus() == RDFADE_BLACK) {
		// Double clicked an exit, and the screen has faded down to
		// black. Ok, that's it. Back to script and change screen.

		// We have to clear te EXIT_CLICK_ID variable in case there's a
		// walk instruction on the new screen, or it'd be cut short.

		freeRouteMem();

		obLogic.setLooping(0);
		obMega.setIsWalking(0);
		_vm->_logic->writeVar(EXIT_CLICK_ID, 0);
		_vm->_logic->writeVar(RESULT, 0);

		return IR_CONT;
	}

	// Get pointer to walkanim & current frame position

	WalkData *walkAnim = getRouteMem();
	int32 walk_pc = obMega.getWalkPc();

	// If stopping the walk early, overwrite the next step with a
	// slow-out, then finish

	if (_vm->_logic->checkEventWaiting() && walkAnim[walk_pc].step == 0 && walkAnim[walk_pc + 1].step == 1) {
		// At the beginning of a step
		earlySlowOut(ob_mega, ob_walkdata);
	}

	// Get new frame of walk

	obGraph.setAnimPc(walkAnim[walk_pc].frame);
	obMega.setCurDir(walkAnim[walk_pc].dir);
	obMega.setFeetX(walkAnim[walk_pc].x);
	obMega.setFeetY(walkAnim[walk_pc].y);

	// Is the NEXT frame is the end-marker (512) of the walk sequence?

	if (walkAnim[walk_pc + 1].frame != 512) {
		// No, it wasn't. Increment the walk-anim frame number and
		// come back next cycle.
		obMega.setWalkPc(obMega.getWalkPc() + 1);
		return IR_REPEAT;
	}

	// We have reached the end-marker, which means we can return to the
	// script just as the final (stand) frame of the walk is set.

	freeRouteMem();
	obLogic.setLooping(0);
	obMega.setIsWalking(0);

	// If George's walk has been interrupted to run a new action script for
	// instance or Nico's walk has been interrupted by player clicking on
	// her to talk

	// There used to be code here for checking if two megas were colliding,
	// but it had been commented out, and it was only run if a function
	// that always returned zero returned non-zero.

	if (_vm->_logic->checkEventWaiting()) {
		_vm->_logic->startEvent();
		_vm->_logic->writeVar(RESULT, 1);
		return IR_TERMINATE;
	}

	_vm->_logic->writeVar(RESULT, 0);

	// CONTINUE the script so that RESULT can be checked! Also, if an anim
	// command follows the fnWalk command, the 1st frame of the anim (which
	// is always a stand frame itself) can replace the final stand frame of
	// the walk, to hide the slight difference between the shrinking on the
	// mega frames and the pre-shrunk anim start-frame.

	return IR_CONT;
}

/**
 * Walk mega to start position of anim
 */

int Router::walkToAnim(byte *ob_logic, byte *ob_graph, byte *ob_mega, byte *ob_walkdata, uint32 animRes) {
	int16 target_x = 0;
	int16 target_y = 0;
	uint8 target_dir = 0;

	// Walkdata is needed for earlySlowOut if player clicks elsewhere
	// during the walk.

	// If this is the start of the walk, read anim file to get start coords

	ObjectLogic obLogic(ob_logic);

	if (obLogic.getLooping() == 0) {
		byte *anim_file = _vm->_resman->openResource(animRes);
		AnimHeader anim_head;

		anim_head.read(_vm->fetchAnimHeader(anim_file));

		target_x = anim_head.feetStartX;
		target_y = anim_head.feetStartY;
		target_dir = anim_head.feetStartDir;

		_vm->_resman->closeResource(animRes);

		// If start coords not yet set in anim header, use the standby
		// coords (which should be set beforehand in the script).

		if (target_x == 0 && target_y == 0) {
			target_x = _standbyX;
			target_y = _standbyY;
			target_dir = _standbyDir;
		}

		assert(target_dir <= 7);
	}

	return doWalk(ob_logic, ob_graph, ob_mega, ob_walkdata, target_x, target_y, target_dir);
}

/**
 * Route to the left or right hand side of target id, if possible.
 */

int Router::walkToTalkToMega(byte *ob_logic, byte *ob_graph, byte *ob_mega, byte *ob_walkdata, uint32 megaId, uint32 separation) {
	ObjectMega obMega(ob_mega);

	int16 target_x = 0;
	int16 target_y = 0;
	uint8 target_dir = 0;

	// If this is the start of the walk, calculate the route.

	ObjectLogic obLogic(ob_logic);

	if (obLogic.getLooping() == 0) {
		assert(_vm->_resman->fetchType(megaId) == GAME_OBJECT);

		// Call the base script. This is the graphic/mouse service
		// call, and will set _engineMega to the ObjectMega of mega we
		// want to route to.

		_vm->_logic->runResScript(megaId, 3);

		ObjectMega targetMega(_vm->_logic->getEngineMega());

		// Stand exactly beside the mega, ie. at same y-coord
		target_y = targetMega.getFeetY();

		int scale = obMega.calcScale();
		int mega_separation = (separation * scale) / 256;

		debug(4, "Target is at (%d, %d), separation %d", targetMega.getFeetX(), targetMega.getFeetY(), mega_separation);

		if (targetMega.getFeetX() < obMega.getFeetX()) {
			// Target is left of us, so aim to stand to their
			// right. Face down_left

			target_x = targetMega.getFeetX() + mega_separation;
			target_dir = 5;
		} else {
			// Ok, must be right of us so aim to stand to their
			// left. Face down_right.

			target_x = targetMega.getFeetX() - mega_separation;
			target_dir = 3;
		}
	}

	return doWalk(ob_logic, ob_graph, ob_mega, ob_walkdata, target_x, target_y, target_dir);
}
/**
 * Turn mega to the specified direction. Just needs to call doWalk() with
 * current feet coords, so router can produce anim of turn frames.
 */

int Router::doFace(byte *ob_logic, byte *ob_graph, byte *ob_mega, byte *ob_walkdata, uint8 target_dir) {
	int16 target_x = 0;
	int16 target_y = 0;

	// If this is the start of the turn, get the mega's current feet
	// coords + the required direction

	ObjectLogic obLogic(ob_logic);

	if (obLogic.getLooping() == 0) {
		assert(target_dir <= 7);

		ObjectMega obMega(ob_mega);

		target_x = obMega.getFeetX();
		target_y = obMega.getFeetY();
	}

	return doWalk(ob_logic, ob_graph, ob_mega, ob_walkdata, target_x, target_y, target_dir);
}

/**
 * Turn mega to face point (x,y) on the floor
 */

int Router::faceXY(byte *ob_logic, byte *ob_graph, byte *ob_mega, byte *ob_walkdata, int16 target_x, int16 target_y) {
	uint8 target_dir = 0;

	// If this is the start of the turn, get the mega's current feet
	// coords + the required direction

	ObjectLogic obLogic(ob_logic);

	if (obLogic.getLooping() == 0) {
		ObjectMega obMega(ob_mega);

		target_dir = whatTarget(obMega.getFeetX(), obMega.getFeetY(), target_x, target_y);
	}

	return doFace(ob_logic, ob_graph, ob_mega, ob_walkdata, target_dir);
}

/**
 * Turn mega to face another mega.
 */

int Router::faceMega(byte *ob_logic, byte *ob_graph, byte *ob_mega, byte *ob_walkdata, uint32 megaId) {
	uint8 target_dir = 0;

	// If this is the start of the walk, decide where to walk to.

	ObjectLogic obLogic(ob_logic);

	if (obLogic.getLooping() == 0) {
		assert(_vm->_resman->fetchType(megaId) == GAME_OBJECT);

		// Call the base script. This is the graphic/mouse service
		// call, and will set _engineMega to the ObjectMega of mega we
		// want to turn to face.

		_vm->_logic->runResScript(megaId, 3);

		ObjectMega obMega(ob_mega);
		ObjectMega targetMega(_vm->_logic->getEngineMega());

		target_dir = whatTarget(obMega.getFeetX(), obMega.getFeetY(), targetMega.getFeetX(), targetMega.getFeetY());
	}

	return doFace(ob_logic, ob_graph, ob_mega, ob_walkdata, target_dir);
}

/**
 * Stand mega at (x,y,dir)
 * Sets up the graphic object, but also needs to set the new 'current_dir' in
 * the mega object, so the router knows in future
 */

void Router::standAt(byte *ob_graph, byte *ob_mega, int32 x, int32 y, int32 dir) {
	assert(dir >= 0 && dir <= 7);

	ObjectGraphic obGraph(ob_graph);
	ObjectMega obMega(ob_mega);

	// Set up the stand frame & set the mega's new direction

	obMega.setFeetX(x);
	obMega.setFeetY(y);
	obMega.setCurDir(dir);

	// Mega-set animation file
	obGraph.setAnimResource(obMega.getMegasetRes());

	// Dir + first stand frame (always frame 96)
	obGraph.setAnimPc(dir + 96);
}

/**
 * Stand mega at end position of anim
 */

void Router::standAfterAnim(byte *ob_graph, byte *ob_mega, uint32 animRes) {
	byte *anim_file = _vm->_resman->openResource(animRes);
	AnimHeader anim_head;

	anim_head.read(_vm->fetchAnimHeader(anim_file));

	int32 x = anim_head.feetEndX;
	int32 y = anim_head.feetEndY;
	int32 dir = anim_head.feetEndDir;

	_vm->_resman->closeResource(animRes);

	// If start coords not available either use the standby coords (which
	// should be set beforehand in the script)

	if (x == 0 && y == 0) {
		x = _standbyX;
		y = _standbyY;
		dir = _standbyDir;
	}

	standAt(ob_graph, ob_mega, x, y, dir);
}

void Router::standAtAnim(byte *ob_graph, byte *ob_mega, uint32 animRes) {
	byte *anim_file = _vm->_resman->openResource(animRes);
	AnimHeader anim_head;

	anim_head.read(_vm->fetchAnimHeader(anim_file));

	int32 x = anim_head.feetStartX;
	int32 y = anim_head.feetStartY;
	int32 dir = anim_head.feetStartDir;

	_vm->_resman->closeResource(animRes);

	// If start coords not available use the standby coords (which should
	// be set beforehand in the script)

	if (x == 0 && y == 0) {
		x = _standbyX;
		y = _standbyY;
		dir = _standbyDir;
	}

	standAt(ob_graph, ob_mega, x, y, dir);
}

} // End of namespace Sword2
