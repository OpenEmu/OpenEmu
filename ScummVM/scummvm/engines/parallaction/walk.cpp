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

#include "parallaction/exec.h"
#include "parallaction/parallaction.h"
#include "parallaction/walk.h"

namespace Parallaction {

enum {
	WALK_LEFT = 0,
	WALK_RIGHT = 1,
	WALK_DOWN = 2,
	WALK_UP = 3
};

struct WalkFrames {
	int16 stillFrame[4];
	int16 firstWalkFrame[4];
	int16 numWalkFrames[4];
	int16 frameRepeat[4];
};

WalkFrames _char20WalkFrames_NS = {
	{  0,  7, 14, 17 },
	{  1,  8, 15, 18 },
	{  6,  6,  2,  2 },
	{  2,  2,  4,  4 }
};

WalkFrames _char24WalkFrames_NS = {
	{  0,  9, 18, 21 },
	{  1, 10, 19, 22 },
	{  8,  8,  2,  2 },
	{  2,  2,  4,  4 }
};

static int getPathWidth() {
	if (!g_vm->_gfx->_backgroundInfo->_path) {
		warning("getPathWidth() _path is NULL!");
		return 0;
	} else
		return g_vm->_gfx->_backgroundInfo->_path->w;
}

static int getPathHeight() {
	if (!g_vm->_gfx->_backgroundInfo->_path) {
		warning("getPathHeight() _path is NULL!");
		return 0;
	} else
		return g_vm->_gfx->_backgroundInfo->_path->h;
}

static bool isPathClear(uint16 x, uint16 y) {
	if (!g_vm->_gfx->_backgroundInfo->_path) {
		warning("isPathClear() _path is NULL!");
		return false;
	} else
		return (g_vm->_gfx->_backgroundInfo->_path->getValue(x, y) ? true : false);
}

// adjusts position towards nearest walkable point
void PathWalker_NS::correctPathPoint(Common::Point &to) {
	if (isPathClear(to.x, to.y)) return;

	int maxX = getPathWidth();
	int maxY = getPathHeight();

	int16 right = to.x;
	int16 left = to.x;
	do {
		right++;
	} while ((right < maxX) && !isPathClear(right, to.y));
	do {
		left--;
	} while ((left > 0) && !isPathClear(left, to.y));
	right = (right == maxX) ? 1000 : right - to.x;
	left = (left == 0) ? 1000 : to.x - left;

	int16 top = to.y;
	int16 bottom = to.y;
	do {
		top--;
	} while ((top > 0) && !isPathClear(to.x, top));
	do {
		bottom++;
	} while ((bottom < maxY) && !isPathClear(to.x, bottom));
	top = (top == 0) ? 1000 : to.y - top;
	bottom = (bottom == maxY) ? 1000 : bottom - to.y;

	int16 closeX = (right >= left) ? left : right;
	int16 closeY = (top >= bottom) ? bottom : top;
	int16 close = (closeX >= closeY) ? closeY : closeX;
	if (close == right) {
		to.x += right;
	} else
	if (close == left) {
		to.x -= left;
	} else
	if (close == top) {
		to.y -= top;
	} else
	if (close == bottom) {
		to.y += bottom;
	}
}

uint32 PathWalker_NS::buildSubPath(const Common::Point& pos, const Common::Point& stop) {
	uint32 v28 = 0;
	uint32 v2C = 0;
	uint32 v34 = pos.sqrDist(stop);				// square distance from current position and target
	uint32 v30 = v34;

	_subPath.clear();

	Common::Point v20(pos);

	while (true) {

		PointList::iterator nearest = _vm->_location._walkPoints.end();
		PointList::iterator locNode = _vm->_location._walkPoints.begin();

		// scans location path nodes searching for the nearest Node
		// which can't be farther than the target position
		// otherwise no _closest_node is selected
		while (locNode != _vm->_location._walkPoints.end()) {

			Common::Point v8 = *locNode;
			v2C = v8.sqrDist(stop);
			v28 = v8.sqrDist(v20);

			if (v2C < v34 && v28 < v30) {
				v30 = v28;
				nearest = locNode;
			}

			locNode++;
		}

		if (nearest == _vm->_location._walkPoints.end()) break;

		v20 = *nearest;
		v34 = v30 = v20.sqrDist(stop);

		_subPath.push_back(*nearest);
	}

	return v34;
}

//	x, y: mouse click (foot) coordinates
void PathWalker_NS::buildPath(AnimationPtr a, uint16 x, uint16 y) {
	debugC(1, kDebugWalk, "PathBuilder::buildPath to (%i, %i)", x, y);

	_a = a;

	_walkPath.clear();

	Common::Point to(x, y);
	correctPathPoint(to);
	debugC(1, kDebugWalk, "found closest path point at (%i, %i)", to.x, to.y);

	Common::Point v48(to);
	Common::Point v44(to);

	uint16 v38 = walkFunc1(to, v44);
	if (v38 == 1) {
		// destination directly reachable
		debugC(1, kDebugWalk, "direct move to (%i, %i)", to.x, to.y);
		_walkPath.push_back(v48);
		return;
	}

	// path is obstructed: look for alternative
	_walkPath.push_back(v48);
	Common::Point pos;
	_a->getFoot(pos);

	uint32 v34 = buildSubPath(pos, v48);
	if (v38 != 0 && v34 > v38) {
		// no alternative path (gap?)
		_walkPath.clear();
		_walkPath.push_back(v44);
		return;
	}
	_walkPath.insert(_walkPath.begin(), _subPath.begin(), _subPath.end());

	buildSubPath(pos, *_walkPath.begin());
	_walkPath.insert(_walkPath.begin(), _subPath.begin(), _subPath.end());
}

//	x,y : top left coordinates
//
//	0 : Point not reachable
//	1 : Point reachable in a straight line
//	other values: square distance to target (point not reachable in a straight line)
uint16 PathWalker_NS::walkFunc1(const Common::Point &to, Common::Point& node) {
	Common::Point arg(to);

	Common::Point v4;

	Common::Point foot;
	_a->getFoot(foot);

	Common::Point v8(foot);

	while (foot != arg) {

		if (foot.x < to.x && isPathClear(foot.x + 1, foot.y)) foot.x++;
		if (foot.x > to.x && isPathClear(foot.x - 1, foot.y)) foot.x--;
		if (foot.y < to.y && isPathClear(foot.x, foot.y + 1)) foot.y++;
		if (foot.y > to.y && isPathClear(foot.x, foot.y - 1)) foot.y--;

		if (foot == v8 && foot != arg) {
			// foot couldn't move and still away from target

			v4 = foot;

			while (foot != arg) {

				if (foot.x < to.x && !isPathClear(foot.x + 1, foot.y)) foot.x++;
				if (foot.x > to.x && !isPathClear(foot.x - 1, foot.y)) foot.x--;
				if (foot.y < to.y && !isPathClear(foot.x, foot.y + 1)) foot.y++;
				if (foot.y > to.y && !isPathClear(foot.x, foot.y - 1)) foot.y--;

				if (foot == v8 && foot != arg)
					return 0;

				v8 = foot;
			}

			node = v4;
			return v4.sqrDist(to);
		}

		v8 = foot;
	}

	// there exists an unobstructed path
	return 1;
}

void PathWalker_NS::clipMove(Common::Point& pos, const Common::Point& to) {
	if ((pos.x < to.x) && (pos.x < getPathWidth()) && isPathClear(pos.x + 2, pos.y)) {
		pos.x = (pos.x + 2 < to.x) ? pos.x + 2 : to.x;
	}

	if ((pos.x > to.x) && (pos.x > 0) && isPathClear(pos.x - 2, pos.y)) {
		pos.x = (pos.x - 2 > to.x) ? pos.x - 2 : to.x;
	}

	if ((pos.y < to.y) && (pos.y < getPathHeight()) && isPathClear(pos.x, pos.y + 2)) {
		pos.y = (pos.y + 2 <= to.y) ? pos.y + 2 : to.y;
	}

	if ((pos.y > to.y) && (pos.y > 0) && isPathClear(pos.x, pos.y - 2)) {
		pos.y = (pos.y - 2 >= to.y) ? pos.y - 2 : to.y;
	}
}

void PathWalker_NS::checkDoor(const Common::Point &foot) {
	ZonePtr z = _vm->hitZone(kZoneDoor, foot.x, foot.y);
	if (z) {
		if ((z->_flags & kFlagsClosed) == 0) {
			_vm->_location._startPosition = z->u._doorStartPos;
			_vm->_location._startFrame = z->u._doorStartFrame;
			_vm->scheduleLocationSwitch(z->u._doorLocation.c_str());
			_vm->_zoneTrap.reset();
		} else {
			_vm->_cmdExec->run(z->_commands, z);
		}
	}

	z = _vm->hitZone(kZoneTrap, foot.x, foot.y);
	if (z) {
		_vm->setLocationFlags(kFlagsEnter);
		_vm->_cmdExec->run(z->_commands, z);
		_vm->clearLocationFlags(kFlagsEnter);
		_vm->_zoneTrap = z;
	} else
	if (_vm->_zoneTrap) {
		_vm->setLocationFlags(kFlagsExit);
		_vm->_cmdExec->run(_vm->_zoneTrap->_commands, _vm->_zoneTrap);
		_vm->clearLocationFlags(kFlagsExit);
		_vm->_zoneTrap.reset();
	}
}

void PathWalker_NS::finalizeWalk() {
	g_engineFlags &= ~kEngineWalking;

	Common::Point foot;
	_a->getFoot(foot);
	checkDoor(foot);

	_walkPath.clear();
}

void PathWalker_NS::walk() {
	if ((g_engineFlags & kEngineWalking) == 0) {
		return;
	}

	Common::Point curPos;
	_a->getFoot(curPos);

	// update target, if previous was reached
	PointList::iterator it = _walkPath.begin();
	if (it != _walkPath.end()) {
		if (*it == curPos) {
			debugC(1, kDebugWalk, "walk reached node (%i, %i)", (*it).x, (*it).y);
			it = _walkPath.erase(it);
		}
	}

	// advance character towards the target
	Common::Point targetPos;
	if (it == _walkPath.end()) {
		debugC(1, kDebugWalk, "walk reached last node");
		finalizeWalk();
		targetPos = curPos;
	} else {
		// targetPos is saved to help setting character direction
		targetPos = *it;

		Common::Point newPos(curPos);
		clipMove(newPos, targetPos);
		_a->setFoot(newPos);

		if (newPos == curPos) {
			debugC(1, kDebugWalk, "walk was blocked by an unforeseen obstacle");
			finalizeWalk();
			targetPos = newPos;	// when walking is interrupted, targetPos must be hacked so that a still frame can be selected
		}
	}

	// targetPos is used to select the direction (and the walkFrame) of a character,
	// since it doesn't cause the sudden changes in orientation that newPos would.
	// Since newPos is 'adjusted' according to walkable areas, an imaginary line drawn
	// from curPos to newPos is prone to abrutply change in direction, thus making the
	// code select 'too different' frames when walking diagonally against obstacles,
	// and yielding an annoying shaking effect in the character.
	updateDirection(curPos, targetPos);
}

void PathWalker_NS::updateDirection(const Common::Point& pos, const Common::Point& to) {
	Common::Point dist(to.x - pos.x, to.y - pos.y);
	WalkFrames *frames = (_a->getFrameNum() == 20) ? &_char20WalkFrames_NS : &_char24WalkFrames_NS;

	_step++;

	if (dist.x == 0 && dist.y == 0) {
		_a->setF(frames->stillFrame[_direction]);
		return;
	}

	if (dist.x < 0)
		dist.x = -dist.x;
	if (dist.y < 0)
		dist.y = -dist.y;

	_direction = (dist.x > dist.y) ? ((to.x > pos.x) ? WALK_LEFT : WALK_RIGHT) : ((to.y > pos.y) ? WALK_DOWN : WALK_UP);
	_a->setF(frames->firstWalkFrame[_direction] + (_step / frames->frameRepeat[_direction]) % frames->numWalkFrames[_direction]);
}

PathWalker_NS::PathWalker_NS(Parallaction *vm) : _direction(WALK_DOWN), _step(0), _vm(vm) {
}

bool PathWalker_BR::directPathExists(const Common::Point &from, const Common::Point &to) {
	Common::Point copy(from);
	Common::Point p(copy);

	while (p != to) {
		if (p.x < to.x && isPathClear(p.x + 1, p.y)) p.x++;
		if (p.x > to.x && isPathClear(p.x - 1, p.y)) p.x--;
		if (p.y < to.y && isPathClear(p.x, p.y + 1)) p.y++;
		if (p.y > to.y && isPathClear(p.x, p.y - 1)) p.y--;

		if (p == copy && p != to)
			return false;

		copy = p;
	}

	return true;
}

void PathWalker_BR::setCharacterPath(AnimationPtr a, uint16 x, uint16 y) {
	_character._a = a;
	_character._first = true;
	_character._stillWalkingTowardsNode = true;
	_character._walkDelay = 0;
	buildPath(_character, x, y);
	_character._active = true;
}

void PathWalker_BR::setFollowerPath(AnimationPtr a, uint16 x, uint16 y) {
	_follower._a = a;
	_follower._first = true;
	_follower._stillWalkingTowardsNode = true;
	_follower._walkDelay = 5;
	buildPath(_follower, x - 50, y);
	_follower._active = true;
}

void PathWalker_BR::stopFollower() {
	if (_follower._active) {
		uint32 frame = _follower._a->getF();
		_follower._a->setF((frame/9) * 9);
	}
	_follower._a.reset();
	_follower._active = false;
}

void PathWalker_BR::buildPath(State &s, uint16 x, uint16 y) {
	Common::Point foot;
	s._a->getFoot(foot);

	debugC(1, kDebugWalk, "buildPath: try to build path from (%i, %i) to (%i, %i)", foot.x, foot.y, x, y);
	s._walkPath.clear();

	// look for easy path first
	Common::Point dest(x, y);
	if (directPathExists(foot, dest)) {
		s._walkPath.push_back(dest);
		debugC(3, kDebugWalk, "buildPath: direct path found");
		return;
	}

	// look for short circuit cases
	ZonePtr z0 = _vm->hitZone(kZonePath, x, y);
	if (!z0) {
		s._walkPath.push_back(dest);
		debugC(3, kDebugWalk, "buildPath: corner case 0 (%i nodes)", s._walkPath.size());
		return;
	}
	ZonePtr z1 = _vm->hitZone(kZonePath, foot.x, foot.y);
	if (!z1 || z1 == z0) {
		s._walkPath.push_back(dest);
		debugC(3, kDebugWalk, "buildPath: corner case 1 (%i nodes)", s._walkPath.size());
		return;
	}

	// build complex path
	int id = atoi(z0->_name);

	if (z1->u._pathLists[id].empty()) {
		s._walkPath.clear();
		debugC(3, kDebugWalk, "buildPath: no path found");
		// If no path, trigger finalize and stop of walking...
		s._stillWalkingTowardsNode = false;
		return;
	}

	PointList::iterator b = z1->u._pathLists[id].begin();
	PointList::iterator e = z1->u._pathLists[id].end();
	for ( ; b != e; ++b) {
		s._walkPath.push_front(*b);
	}
	s._walkPath.push_back(dest);
	debugC(3, kDebugWalk, "buildPath: complex path (%i nodes)", s._walkPath.size());
}

void PathWalker_BR::finalizeWalk(State &s) {
	g_engineFlags &= ~kEngineWalking;

	Common::Point foot;
	_character._a->getFoot(foot);

	ZonePtr z = _vm->hitZone(kZoneDoor, foot.x, foot.y);
	if (z && ((z->_flags & kFlagsClosed) == 0)) {
		_vm->_location._startPosition = z->u._doorStartPos; // foot pos
		_vm->_location._startFrame = z->u._doorStartFrame;

		// TODO: implement working follower. Must find out a location in which the code is
		// used and which is stable enough.
		if (_follower._active) {
			_vm->_location._followerStartPosition = z->u._doorStartPos2_br;	// foot pos
			_vm->_location._followerStartFrame = z->u._doorStartFrame2_br;
		} else {
			_vm->_location._followerStartPosition.x = -1000;
			_vm->_location._followerStartPosition.y = -1000;
			_vm->_location._followerStartFrame = 0;
		}

		_vm->scheduleLocationSwitch(z->u._doorLocation.c_str());
		_vm->_cmdExec->run(z->_commands, z);
	}

#if 0
	// TODO: Input::walkTo must be extended to support destination frame in addition to coordinates
	if (g_engineFlags & FINAL_WALK_FRAME) {	// this flag is set in readInput()
		g_engineFlags &= ~FINAL_WALK_FRAME;
		_ch._a->_frame = _moveToF;	// from readInput()...
	} else {
		_ch._a->_frame = _dirFrame;	// from walk()
	}
	_ch._a->setFoot(foot);
#endif

	s._a->setF(s._dirFrame);	// temporary solution

	s._active = false;
}

void PathWalker_BR::walk() {
	if ((g_engineFlags & kEngineWalking) == 0) {
		return;
	}

	doWalk(_character);
	doWalk(_follower);

	Common::Point pos, foot;
	_vm->_gfx->getScrollPos(pos);
	_character._a->getFoot(foot);

	int32 dx = 0, dy = 0;
	if (foot.x > pos.x + 600) {
		dx = 78*4;
	} else
	if (foot.x < pos.x + 40) {
		dx = -78*4;
	}

	if (foot.y > pos.y + 350) {
		dy = 100;
	} else
	if (foot.y < pos.y + 80) {
		dy = -100;
	}

	_vm->_gfx->initiateScroll(dx, dy);
}

void PathWalker_BR::checkTrap(const Common::Point &p) {
	ZonePtr z = _vm->hitZone(kZoneTrap, p.x, p.y);

	if (z && z != _vm->_zoneTrap) {
		if (z->_flags & kFlagsIsAnimation) {
			z->_flags |= kFlagsActing;
		} else {
			_vm->setLocationFlags(kFlagsExit);
			_vm->_cmdExec->run(z->_commands, z);
			_vm->clearLocationFlags(kFlagsExit);
		}
	}

	if (_vm->_zoneTrap && _vm->_zoneTrap != z) {
		if (_vm->_zoneTrap->_flags & kFlagsIsAnimation) {
			_vm->_zoneTrap->_flags &= ~kFlagsActing;
		} else {
			_vm->setLocationFlags(kFlagsEnter);
			_vm->_cmdExec->run(_vm->_zoneTrap->_commands, _vm->_zoneTrap);
			_vm->clearLocationFlags(kFlagsEnter);
		}
	}

	_vm->_zoneTrap = z;
}

void PathWalker_BR::doWalk(State &s) {
	if (!s._active) {
		return;
	}

	if (s._walkDelay > 0) {
		s._walkDelay--;
		if (s._walkDelay == 0 && s._a->_scriptName) {
			// stop script and reset
			s._a->_flags &= ~kFlagsActing;
//			_vm->_programExec->resetProgram(s._a->_scriptName);
		}
		return;
	}

	if (!s._stillWalkingTowardsNode) {
		if (!s._walkPath.empty())
			s._walkPath.erase(s._walkPath.begin());

		if (s._walkPath.empty()) {
			finalizeWalk(s);
			debugC(3, kDebugWalk, "PathWalker_BR::doWalk, walk completed (no more nodes)");
			return;
		} else {
			debugC(3, kDebugWalk, "PathWalker_BR::doWalk, reached a walkpath node, %i left", s._walkPath.size());
		}
	}

	s._a->getFoot(s._startFoot);

	uint scale = _vm->_location.getScale(s._startFoot.y);
	int xStep = (scale * 16) / 100 + 1;
	int yStep = (scale * 10) / 100 + 1;

	/* WORKAROUND: in the balloon scene, the position of the balloon (which is implemented as a
	Character) is controlled by the user (for movement, via this walking code) and by the scripts
	(to simulate the balloon floating in the air, in a neverending loop that alters the position
	coordinates).
	When the two step sizes are equal in magnitude and opposite in direction, then the walk code
	enters an infinite loop without giving control back to the user (this happens quite frequently
	when navigating the balloon near the borders of the screen, where the calculated step is
	forcibly small because of clipping). Since the "floating" script (part1/scripts/mongolo.scr)
	uses increments of 3 for both x and y, we tweak the calculated steps accordingly here. */
	if (xStep == 3) xStep--;
	if (yStep == 3) yStep--;

	debugC(9, kDebugWalk, "calculated step: (%i, %i)", xStep, yStep);

	s._stillWalkingTowardsNode = false;
	s._step++;
	s._step %= 8;

	int maxX = _vm->_gfx->_backgroundInfo->width;
	int minX = 0;
	int maxY = _vm->_gfx->_backgroundInfo->height;
	int minY = 0;

	int walkFrame = s._step;
	s._dirFrame = 0;
	Common::Point newpos(s._startFoot), delta;

	assert (!s._walkPath.empty());
	Common::Point p(*s._walkPath.begin());

	if (s._startFoot.y < p.y && (s._startFoot.y + yStep) < maxY && isPathClear(s._startFoot.x, s._startFoot.y + yStep)) {
		if (yStep + s._startFoot.y <= p.y) {
			s._stillWalkingTowardsNode = true;
			delta.y = yStep;
			newpos.y = yStep + s._startFoot.y;
		} else {
			delta.y = p.y - s._startFoot.y;
			newpos.y = p.y;
		}
		s._dirFrame = 9;
	} else
	if (s._startFoot.y > p.y && (s._startFoot.y - yStep) > minY && isPathClear(s._startFoot.x, s._startFoot.y - yStep)) {
		if (s._startFoot.y - yStep >= p.y) {
			s._stillWalkingTowardsNode = true;
			delta.y = yStep;
			newpos.y = s._startFoot.y - yStep;
		} else {
			delta.y = s._startFoot.y - p.y;
			newpos.y = p.y;
		}
		s._dirFrame = 0;
	}

	if (s._startFoot.x < p.x && (s._startFoot.x + xStep) < maxX && isPathClear(s._startFoot.x + xStep, s._startFoot.y)) {
		if (s._startFoot.x + xStep <= p.x) {
			s._stillWalkingTowardsNode = true;
			delta.x = xStep;
			newpos.x = xStep + s._startFoot.x;
		} else {
			delta.x = p.x - s._startFoot.x;
			newpos.x = p.x;
		}
		if (delta.y < delta.x) {
			s._dirFrame = 18;	// right
		}
	} else
	if (s._startFoot.x > p.x && (s._startFoot.x - xStep) > minX && isPathClear(s._startFoot.x - xStep, s._startFoot.y)) {
		if (s._startFoot.x - xStep >= p.x) {
			s._stillWalkingTowardsNode = true;
			delta.x = xStep;
			newpos.x = s._startFoot.x - xStep;
		} else {
			delta.x = s._startFoot.x - p.x;
			newpos.x = p.x;
		}
		if (delta.y < delta.x) {
			s._dirFrame = 27;	// left
		}
	}

	debugC(9, kDebugWalk, "foot (%i, %i) dest (%i, %i) deltas = %i/%i ", s._startFoot.x, s._startFoot.y, p.x, p.y, delta.x, delta.y);

	if (s._stillWalkingTowardsNode) {
		debugC(9, kDebugWalk, "PathWalker_BR::doWalk, foot moved from (%i, %i) to (%i, %i)", s._startFoot.x, s._startFoot.y, newpos.x, newpos.y);
		s._a->setF(walkFrame + s._dirFrame + 1);
		s._startFoot.x = newpos.x;
		s._startFoot.y = newpos.y;
		s._a->setFoot(s._startFoot);
		s._a->setZ(newpos.y);
	}

	if (s._stillWalkingTowardsNode || !s._walkPath.empty()) {
		Common::Point p2;
		s._a->getFoot(p2);
		checkTrap(p2);
		debugC(3, kDebugWalk, "PathWalker_BR::doWalk, stepped to (%i, %i)", p2.x, p2.y);
	} else {
		debugC(3, kDebugWalk, "PathWalker_BR::doWalk, case 2");
		finalizeWalk(s);
	}
}

PathWalker_BR::PathWalker_BR(Parallaction *vm) : _vm(vm) {
	_character._active = false;
	_character._step = 0;
	_follower._active = false;
	_follower._step = 0;
}

} // namespace Parallaction
