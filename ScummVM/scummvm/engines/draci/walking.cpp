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

#include "common/memstream.h"

#include "draci/draci.h"
#include "draci/animation.h"
#include "draci/game.h"
#include "draci/walking.h"
#include "draci/sprite.h"

namespace Draci {

void WalkingMap::load(const byte *data, uint length) {
	Common::MemoryReadStream mapReader(data, length);

	_realWidth = mapReader.readUint16LE();
	_realHeight = mapReader.readUint16LE();
	_deltaX = mapReader.readUint16LE();
	_deltaY = mapReader.readUint16LE();
	_mapWidth = mapReader.readUint16LE();
	_mapHeight = mapReader.readUint16LE();
	_byteWidth = mapReader.readUint16LE();

	// Set the data pointer to raw map data
	_data = data + mapReader.pos();
}

bool WalkingMap::getPixel(int x, int y) const {
	const byte *pMapByte = _data + _byteWidth * y + x / 8;
	return *pMapByte & (1 << x % 8);
}

bool WalkingMap::isWalkable(const Common::Point &p) const {
	// Convert to map pixels
	return getPixel(p.x / _deltaX, p.y / _deltaY);
}

Sprite *WalkingMap::newOverlayFromMap(byte color) const {
	// HACK: Create a visible overlay from the walking map so we can test it
	byte *wlk = new byte[_realWidth * _realHeight];
	memset(wlk, 255, _realWidth * _realHeight);

	for (int i = 0; i < _mapWidth; ++i) {
		for (int j = 0; j < _mapHeight; ++j) {
			if (getPixel(i, j)) {
				drawOverlayRectangle(Common::Point(i, j), color, wlk);
			}
		}
	}

	Sprite *ov = new Sprite(_realWidth, _realHeight, wlk, 0, 0, false);
	// ov has taken the ownership of wlk.

	return ov;
}

/**
 * @brief For a given point, find a nearest walkable point on the walking map
 *
 * @param startX    x coordinate of the point
 * @param startY    y coordinate of the point
 *
 * @return A Common::Point representing the nearest walkable point
 */
Common::Point WalkingMap::findNearestWalkable(int startX, int startY) const {
	// The dimension of the screen.
	const Common::Rect searchRect(0, 0, _realWidth, _realHeight);

	// Consider circles with radii gradually rising from 0 to the length of
	// the longest edge on the screen.  For each radius, probe all points
	// on the circle and return the first walkable one.  Go through angles
	// [0, 45 degrees] and probe all 8 reflections of each point.
	for (int radius = 0; radius < searchRect.width() + searchRect.height(); radius += _deltaX) {

		// The position of the point on the circle.
		int x = 0;
		int y = radius;

		// Variables for computing the points on the circle
		int prediction = 1 - radius;
		int dx = 3;
		int dy = 2 * radius - 2;

		// Walk until we reach the 45-degree angle.
		while (x <= y) {

			// The place where, eventually, the result coordinates will be stored
			Common::Point final;

			// Auxilliary array of multiplicative coefficients for reflecting points.
			static const int kSigns[] = { 1, -1 };

			// Check all 8 reflections of the basic point.
			for (uint i = 0; i < 2; ++i) {
				final.y = startY + y * kSigns[i];

				for (uint j = 0; j < 2; ++j) {
					final.x = startX + x * kSigns[j];

					// If the current point is walkable, return it
					if (searchRect.contains(final.x, final.y) && isWalkable(final)) {
						return final;
					}
				}
			}
			for (uint i = 0; i < 2; ++i) {
				final.y = startY + x * kSigns[i];

				for (uint j = 0; j < 2; ++j) {
					final.x = startX + y * kSigns[j];

					// If the current point is walkable, return it
					if (searchRect.contains(final.x, final.y) && isWalkable(final)) {
						return final;
					}
				}
			}

			// Walk along the circle to the next point: the
			// X-coordinate moves to the right, and the
			// Y-coordinate may move to the bottom if the predictor
			// says so.
			if (prediction >= 0) {
				prediction -= dy;
				dy -= 2 * _deltaX;
				y -= _deltaX;
			}
			prediction += dx;
			dx += 2 * _deltaX;
			x += _deltaX;
		}
	}

	// The destination point is unreachable.
	return Common::Point(-1, -1);
}

// We don't use Common::Point due to using static initialization.
const int WalkingMap::kDirections[][2] = { {0, -1}, {0, +1}, {-1, 0}, {+1, 0} };

bool WalkingMap::findShortestPath(Common::Point p1, Common::Point p2, WalkingPath *path) const {
	// Round the positions to map squares.
	p1.x /= _deltaX;
	p2.x /= _deltaX;
	p1.y /= _deltaY;
	p2.y /= _deltaY;

	// Allocate buffers for breadth-first search.  The buffer of points for
	// exploration should be large enough.
	const int bufSize = 4 * _realHeight;
	int8 *cameFrom = new int8[_mapWidth * _mapHeight];
	Common::Point *toSearch = new Common::Point[bufSize];

	// Insert the starting point as a single seed.
	int toRead = 0, toWrite = 0;
	memset(cameFrom, -1, _mapWidth * _mapHeight);	// -1 = not found yet
	cameFrom[p1.y * _mapWidth + p1.x] = 0;
	toSearch[toWrite++] = p1;

	// Search until we empty the whole buffer (not found) or find the
	// destination point.
	while (toRead != toWrite) {
		const Common::Point &here = toSearch[toRead];
		const int from = cameFrom[here.y * _mapWidth + here.x];
		if (here == p2) {
			break;
		}
		// Look into all 4 directions in a particular order depending
		// on the direction we came to this point from.  This is to
		// ensure that among many paths of the same length, the one
		// with the smallest number of turns is preferred.
		for (int addDir = 0; addDir < 4; ++addDir) {
			const int probeDirection = (from + addDir) % 4;
			const int x = here.x + kDirections[probeDirection][0];
			const int y = here.y + kDirections[probeDirection][1];
			if (x < 0 || x >= _mapWidth || y < 0 || y >= _mapHeight) {
				continue;
			}
			// If this point is walkable and we haven't seen it
			// yet, record how we have reached it and insert it
			// into the round buffer for exploration.
			if (getPixel(x, y) && cameFrom[y * _mapWidth + x] == -1) {
				cameFrom[y * _mapWidth + x] = probeDirection;
				toSearch[toWrite++] = Common::Point(x, y);
				toWrite %= bufSize;
			}
		}
		++toRead;
		toRead %= bufSize;
	}

	// The path doesn't exist.
	if (toRead == toWrite) {
		delete[] cameFrom;
		delete[] toSearch;
		return false;
	}

	// Trace the path back and store it.  Count the path length, resize the
	// output array, and then track the pack from the end.
	path->clear();
	for (int pass = 0; pass < 2; ++pass) {
		Common::Point p = p2;
		int index = 0;
		while (1) {
			++index;
			if (pass == 1) {
				(*path)[path->size() - index] = p;
			}
			if (p == p1) {
				break;
			}
			const int from = cameFrom[p.y * _mapWidth + p.x];
			p.x -= kDirections[from][0];
			p.y -= kDirections[from][1];
		}
		if (pass == 0) {
			path->resize(index);
		}
	}

	delete[] cameFrom;
	delete[] toSearch;
	return true;
}

void WalkingMap::obliquePath(const WalkingPath& path, WalkingPath *obliquedPath) {
	// Prune the path to only contain vertices where the direction is changing.
	obliquedPath->clear();
	if (path.empty()) {
		return;
	}
	obliquedPath->push_back(path[0]);
	uint index = 1;
	while (index < path.size()) {
		// index1 points to the last vertex inserted into the
		// simplified path.
		uint index1 = index - 1;
		// Probe the vertical direction.  Notice that the shortest path
		// never turns by 180 degrees and therefore it is sufficient to
		// test that the X coordinates are equal.
		while (index < path.size() && path[index].x == path[index1].x) {
			++index;
		}
		if (index - 1 > index1) {
			index1 = index - 1;
			obliquedPath->push_back(path[index1]);
		}
		// Probe the horizontal direction.
		while (index < path.size() && path[index].y == path[index1].y) {
			++index;
		}
		if (index - 1 > index1) {
			index1 = index - 1;
			obliquedPath->push_back(path[index1]);
		}
	}

	// Repeatedly oblique the path until it cannot be improved.  This
	// process is finite, because after each success the number of vertices
	// goes down.
	while (managedToOblique(obliquedPath)) {}
}

Sprite *WalkingMap::newOverlayFromPath(const WalkingPath &path, byte color) const {
	// HACK: Create a visible overlay from the walking map so we can test it
	byte *wlk = new byte[_realWidth * _realHeight];
	memset(wlk, 255, _realWidth * _realHeight);

	for (uint segment = 1; segment < path.size(); ++segment) {
		const Common::Point &v1 = path[segment-1];
		const Common::Point &v2 = path[segment];
		const int steps = pointsBetween(v1, v2);
		// Draw only points in the interval [v1, v2).  These half-open
		// half-closed intervals connect all the way to the last point.
		for (int step = 0; step < steps; ++step) {
			drawOverlayRectangle(interpolate(v1, v2, step, steps), color, wlk);
		}
	}
	// Draw the last point.  This works also when the path has no segment,
	// but just one point.
	if (path.size() > 0) {
		const Common::Point &vLast = path[path.size()-1];
		drawOverlayRectangle(vLast, color, wlk);
	}

	Sprite *ov = new Sprite(_realWidth, _realHeight, wlk, 0, 0, false);
	// ov has taken the ownership of wlk.

	return ov;
}

void WalkingMap::drawOverlayRectangle(const Common::Point &p, byte color, byte *buf) const {
	for (int i = 0; i < _deltaX; ++i) {
		for (int j = 0; j < _deltaY; ++j) {
			buf[(p.y * _deltaY + j) * _realWidth + (p.x * _deltaX + i)] = color;
		}
	}
}

int WalkingMap::pointsBetween(const Common::Point &p1, const Common::Point &p2) {
	return MAX(ABS(p2.x - p1.x), ABS(p2.y - p1.y));
}

Common::Point WalkingMap::interpolate(const Common::Point &p1, const Common::Point &p2, int i, int n) {
	const int x = (p1.x * (n-i) + p2.x * i + n/2) / n;
	const int y = (p1.y * (n-i) + p2.y * i + n/2) / n;
	return Common::Point(x, y);
}

bool WalkingMap::lineIsCovered(const Common::Point &p1, const Common::Point &p2) const {
	const int steps = pointsBetween(p1, p2);
	for (int step = 0; step <= steps; ++step) {
		Common::Point p = interpolate(p1, p2, step, steps);
		if (!getPixel(p.x, p.y)) {
			return false;
		}
	}
	return true;
}

bool WalkingMap::managedToOblique(WalkingPath *path) const {
	bool improved = false;

	// Making the path oblique works as follows.  If the path has at least
	// 3 remaining vertices, we try to oblique the L-shaped path between
	// them.  First we try to connect the 1st and 3rd vertex directly (if
	// all points on the line between them are walkable) and then we try to
	// walk on both edges towards the 2nd vertex in parallel and try to
	// find a shortcut (replacing the 2nd vertex by this mid-point).  If
	// either of those attempts succeeds, we have shortned the path.  We
	// update the path vertices and continue with the next segment.
	for (uint head = 2; head < path->size(); ++head) {
		const Common::Point &v1 = (*path)[head-2];
		const Common::Point &v2 = (*path)[head-1];
		const Common::Point &v3 = (*path)[head];
		const int points12 = pointsBetween(v1, v2);
		const int points32 = pointsBetween(v3, v2);
		// Find the first point p on each edge [v1, v2] and [v3, v2]
		// such that the edge [p, the third vertex] is covered.
		// Ideally we would like p \in {v1, v3} and the closer the
		// better.  The last point p = v2 should always succeed.
		int first12, first32;
		for (first12 = 0; first12 < points12; ++first12) {
			Common::Point midPoint = interpolate(v1, v2, first12, points12);
			if (lineIsCovered(midPoint, v3)) {
				break;
			}
		}
		if (first12 == 0) {
			// Can completely remove the vertex.  Head stays the
			// same after -- and ++.
			path->remove_at(--head);
			improved = true;
			continue;
		}
		for (first32 = 0; first32 < points32; ++first32) {
			Common::Point midPoint = interpolate(v3, v2, first32, points32);
			if (lineIsCovered(midPoint, v1)) {
				break;
			}
		}
		if (first12 < points12 && first32 >= points32 + MIN(first12 - points12, 0)) {
			// There is such a point on the first edge and the
			// second edge has either not succeeded or we gain more
			// by cutting this edge than the other one.
			(*path)[head-1] = interpolate(v1, v2, first12, points12);
			// After replacing the 2nd vertex, let head move on.
		} else if (first32 < points32) {
			(*path)[head-1] = interpolate(v3, v2, first32, points32);
		}
	}
	return improved;
}

void WalkingState::stopWalking() {
	_path.clear();
	_callback = NULL;
}

void WalkingState::startWalking(const Common::Point &p1, const Common::Point &p2,
	const Common::Point &mouse, SightDirection dir,
	const Common::Point &delta, const WalkingPath& path) {
	_path = path;
	_mouse = mouse;
	_dir = dir;

	if (!_path.size()) {
		_path.push_back(p1);
	}
	if (_path.size() == 1 && p2 != p1) {
		// Although the first and last point belong to the same
		// rectangle and therefore the computed path is of length 1,
		// they are different pixels.
		_path.push_back(p2);
	}
	debugC(2, kDraciWalkingDebugLevel, "Starting walking [%d,%d] -> [%d,%d] with %d vertices",
		p1.x, p1.y, p2.x, p2.y, _path.size());

	// The first and last point are available with pixel accurracy.
	_path[0] = p1;
	_path[_path.size() - 1] = p2;
	// The intermediate points are given with map granularity; convert them
	// to pixels.
	for (uint i = 1; i < _path.size() - 1; ++i) {
		_path[i].x *= delta.x;
		_path[i].y *= delta.y;
	}

	// Remember the initial dragon's direction.
	const GameObject *dragon = _vm->_game->getObject(kDragonObject);
	_startingDirection = static_cast<Movement> (dragon->playingAnim());

	// Going to start with the first segment.
	_segment = 0;
	_lastAnimPhase = -1;
	_turningFinished = false;
	turnForTheNextSegment();
}

void WalkingState::setCallback(const GPL2Program *program, uint16 offset) {
	_callback = program;
	_callbackOffset = offset;
}

void WalkingState::callback() {
	if (!_callback) {
		return;
	}
	debugC(2, kDraciWalkingDebugLevel, "Calling walking callback");

	const GPL2Program &originalCallback = *_callback;
	_callback = NULL;
	_vm->_script->runWrapper(originalCallback, _callbackOffset, true, false);
}

bool WalkingState::continueWalkingOrClearPath() {
	const bool stillWalking = continueWalking();
	if (!stillWalking) {
		_path.clear();
	}
	return stillWalking;
}

bool WalkingState::continueWalking() {
	const GameObject *dragon = _vm->_game->getObject(kDragonObject);
	const Movement movement = static_cast<Movement> (dragon->playingAnim());

	if (_turningFinished) {
		// When a turning animation has finished, heroAnimationFinished() callback
		// gets called, which sets this flag to true.  It's important
		// to not start walking right away in the callback, because
		// that would disturb the data structures of the animation
		// manager.
		_turningFinished = false;
		return walkOnNextEdge();
	}

	// If the current segment is the last one, we have reached the
	// destination and are already facing in the right direction ===>
	// return false.  The code should, however, get here only if the path
	// has just 1 vertex and startWalking() leaves the path open.
	// Finishing and nontrivial path will get caught earlier.
	if (_segment >= _path.size()) {
		return false;
	}

	// Read the dragon's animation's current phase.  Determine if it has
	// changed from the last time.  If not, wait until it has.
	Animation *anim = dragon->_anim[movement];
	const int animPhase = anim->currentFrameNum();
	const bool wasUpdated = animPhase != _lastAnimPhase;
	if (!wasUpdated) {
		debugC(4, kDraciWalkingDebugLevel, "Waiting for an animation phase change: still %d", animPhase);
		return true;
	}

	if (isTurningMovement(movement)) {
		// If the current animation is a turning animation, wait a bit more.
		debugC(3, kDraciWalkingDebugLevel, "Continuing turning for edge %d with phase %d", _segment, animPhase);
		_lastAnimPhase = animPhase;
		return true;
	}

	// We are walking in the middle of an edge.  The animation phase has
	// just changed.

	// Read the position of the hero from the animation object, and adjust
	// it to the current edge.
	const Common::Point prevHero = _vm->_game->getHeroPosition();
	_vm->_game->positionHeroAsAnim(anim);
	const Common::Point curHero = _vm->_game->getHeroPosition();
	Common::Point adjustedHero = curHero;
	const bool reachedEnd = alignHeroToEdge(_path[_segment-1], _path[_segment], prevHero, &adjustedHero);
	if (reachedEnd && _segment >= _path.size() - 1) {
		// We don't want the dragon to jump around if we repeatedly
		// click on the same pixel.  Let him always end where desired.
		debugC(2, kDraciWalkingDebugLevel, "Adjusting position to the final node");
		adjustedHero = _path[_segment];
	}

	debugC(3, kDraciWalkingDebugLevel, "Continuing walking on edge %d: phase %d and position+=[%d,%d]->[%d,%d] adjusted to [%d,%d]",
		_segment-1, animPhase, curHero.x - prevHero.x, curHero.y - prevHero.y, curHero.x, curHero.y, adjustedHero.x, adjustedHero.y);

	// Update the hero position to the adjusted one.  The animation number
	// is not changing, so this will just move the sprite and return the
	// current frame number.
	_vm->_game->setHeroPosition(adjustedHero);
	_lastAnimPhase = _vm->_game->playHeroAnimation(movement);

	// If the hero has reached the end of the edge, start transition to the
	// next phase.  This will increment _segment, either immediately (if no
	// transition is needed) or in the callback (after the transition is
	// done).  If the hero has arrived at a slightly different point due to
	// animated sprites, adjust the path so that the animation can smoothly
	// continue.
	if (reachedEnd) {
		if (adjustedHero != _path[_segment]) {
			debugC(2, kDraciWalkingDebugLevel, "Adjusting node %d of the path [%d,%d]->[%d,%d]",
				_segment, _path[_segment].x, _path[_segment].y, adjustedHero.x, adjustedHero.y);
			_path[_segment] = adjustedHero;
		}
		return turnForTheNextSegment();
	}

	return true;
}

bool WalkingState::alignHeroToEdge(const Common::Point &p1, const Common::Point &p2, const Common::Point &prevHero, Common::Point *hero) {
	const Movement movement = animationForDirection(p1, p2);
	const Common::Point p2Diff(p2.x - p1.x, p2.y - p1.y);
	if (p2Diff.x == 0 && p2Diff.y == 0) {
		debugC(2, kDraciWalkingDebugLevel, "Adjusted walking edge has zero length");
		// Due to changing the path vertices on the fly, this can happen.
		return true;
	}
	bool reachedEnd;
	if (movement == kMoveLeft || movement == kMoveRight) {
		reachedEnd = movement == kMoveLeft ? hero->x <= p2.x : hero->x >= p2.x;
		hero->y += hero->x * p2Diff.y / p2Diff.x - prevHero.x * p2Diff.y / p2Diff.x;
	} else {
		reachedEnd = movement == kMoveUp ? hero->y <= p2.y : hero->y >= p2.y;
		hero->x += hero->y * p2Diff.x / p2Diff.y - prevHero.y * p2Diff.x / p2Diff.y;
	}
	return reachedEnd;
}

bool WalkingState::turnForTheNextSegment() {
	const GameObject *dragon = _vm->_game->getObject(kDragonObject);
	const Movement currentAnim = static_cast<Movement> (dragon->playingAnim());
	const Movement wantAnim = directionForNextPhase();
	Movement transition = transitionBetweenAnimations(currentAnim, wantAnim);

	debugC(2, kDraciWalkingDebugLevel, "Turning for edge %d", _segment);

	if (transition == kMoveUndefined) {
		// Start the next segment right away as if the turning has just finished.
		return walkOnNextEdge();
	} else {
		// Otherwise start the transition and wait until the Animation
		// class calls heroAnimationFinished() as a callback, leading
		// to calling walkOnNextEdge() in the next phase.
		assert(isTurningMovement(transition));
		_lastAnimPhase = _vm->_game->playHeroAnimation(transition);
		Animation *anim = dragon->_anim[transition];
		anim->registerCallback(&Animation::tellWalkingState);

		debugC(2, kDraciWalkingDebugLevel, "Starting turning animation %d with phase %d", transition, _lastAnimPhase);
		return true;
	}
}

void WalkingState::heroAnimationFinished() {
	debugC(2, kDraciWalkingDebugLevel, "Turning callback called");
	_turningFinished = true;

	// We don't need to clear the callback to safer doNothing, because
	// nobody ever plays this animation directly.  It is only played by
	// turnForTheNextSegment() and then the same callback needs to be
	// activated again.
}

bool WalkingState::walkOnNextEdge() {
	// The hero is turned well for the next line segment or for facing the
	// target direction.  It is also standing on the right spot thanks to
	// the entry condition for turnForTheNextSegment().

	// Start the desired next animation and retrieve the current animation
	// phase.
	// Don't use any callbacks, because continueWalking() will decide the
	// end on its own and after walking is done callbacks shouldn't be
	// called either.  It wouldn't make much sense anyway, since the
	// walking/staying/talking animations are cyclic.
	Movement nextAnim = directionForNextPhase();
	_lastAnimPhase = _vm->_game->playHeroAnimation(nextAnim);

	debugC(2, kDraciWalkingDebugLevel, "Turned for edge %d, starting animation %d with phase %d", _segment, nextAnim, _lastAnimPhase);

	if (++_segment < _path.size()) {
		// We are on an edge: track where the hero is on this edge.
		int length = WalkingMap::pointsBetween(_path[_segment-1], _path[_segment]);
		debugC(2, kDraciWalkingDebugLevel, "Next edge %d has length %d", _segment-1, length);
		return true;
	} else {
		// Otherwise we are done.  continueWalking() will return false next time.
		debugC(2, kDraciWalkingDebugLevel, "We have walked the whole path");
		return false;
	}
}

Movement WalkingState::animationForDirection(const Common::Point &here, const Common::Point &there) {
	const int dx = there.x - here.x;
	const int dy = there.y - here.y;
	if (ABS(dx) >= ABS(dy)) {
		return dx >= 0 ? kMoveRight : kMoveLeft;
	} else {
		return dy >= 0 ? kMoveDown : kMoveUp;
	}
}

Movement WalkingState::directionForNextPhase() const {
	if (_segment >= _path.size() - 1) {
		return animationForSightDirection(_dir, _path[_path.size()-1], _mouse, _path, _startingDirection);
	} else {
		return animationForDirection(_path[_segment], _path[_segment+1]);
	}
}

Movement WalkingState::transitionBetweenAnimations(Movement previous, Movement next) {
	switch (next) {
	case kMoveUp:
		switch (previous) {
		case kMoveLeft:
		case kStopLeft:
		case kSpeakLeft:
			return kMoveLeftUp;
		case kMoveRight:
		case kStopRight:
		case kSpeakRight:
			return kMoveRightUp;
		default:
			return kMoveUndefined;
		}
	case kMoveDown:
		switch (previous) {
		case kMoveLeft:
		case kStopLeft:
		case kSpeakLeft:
			return kMoveLeftDown;
		case kMoveRight:
		case kStopRight:
		case kSpeakRight:
			return kMoveRightDown;
		default:
			return kMoveUndefined;
		}
	case kMoveLeft:
		switch (previous) {
		case kMoveDown:
			return kMoveDownLeft;
		case kMoveUp:
			return kMoveUpLeft;
		case kMoveRight:
		case kStopRight:
		case kSpeakRight:
			return kMoveRightLeft;
		default:
			return kMoveUndefined;
		}
	case kMoveRight:
		switch (previous) {
		case kMoveDown:
			return kMoveDownRight;
		case kMoveUp:
			return kMoveUpRight;
		case kMoveLeft:
		case kStopLeft:
		case kSpeakLeft:
			return kMoveLeftRight;
		default:
			return kMoveUndefined;
		}
	case kStopLeft:
		switch (previous) {
		case kMoveUp:
			return kMoveUpStopLeft;
		case kMoveRight:
		case kStopRight:
		case kSpeakRight:
			return kMoveRightLeft;
		default:
			return kMoveUndefined;
		}
	case kStopRight:
		switch (previous) {
		case kMoveUp:
			return kMoveUpStopRight;
		case kMoveLeft:
		case kStopLeft:
		case kSpeakLeft:
			return kMoveLeftRight;
		default:
			return kMoveUndefined;
		}
	default:
		return kMoveUndefined;
	}
}

Movement WalkingState::animationForSightDirection(SightDirection dir, const Common::Point &hero, const Common::Point &mouse, const WalkingPath &path, Movement startingDirection) {
	switch (dir) {
	case kDirectionLeft:
		return kStopLeft;
	case kDirectionRight:
		return kStopRight;
	case kDirectionMouse:
		if (mouse.x < hero.x) {
			return kStopLeft;
		} else if (mouse.x > hero.x) {
			return kStopRight;
		}
		// fall-through here intentional
	default: {
		// Find the last horizontal direction on the path.
		int i = path.size() - 1;
		while (i >= 0 && path[i].x == hero.x) {
			--i;
		}
		if (i >= 0) {
			return path[i].x < hero.x ? kStopRight : kStopLeft;
		} else {
			// Avoid changing the direction when no walking has
			// been done.  Preserve the original direction.
			return (startingDirection == kMoveLeft || startingDirection == kStopLeft || startingDirection == kSpeakLeft)
				? kStopLeft : kStopRight;
		}
	}
	}
}

} // End of namespace Draci
