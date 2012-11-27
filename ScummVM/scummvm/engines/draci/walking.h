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

#ifndef DRACI_WALKING_H
#define DRACI_WALKING_H

#include "common/array.h"
#include "common/rect.h"

namespace Draci {

class Sprite;

typedef Common::Array<Common::Point> WalkingPath;

class WalkingMap {
public:
	WalkingMap() : _realWidth(0), _realHeight(0), _deltaX(1), _deltaY(1),
		_mapWidth(0), _mapHeight(0), _byteWidth(0), _data(NULL) { }

	void load(const byte *data, uint length);

	bool getPixel(int x, int y) const;
	bool isWalkable(const Common::Point &p) const;

	Sprite *newOverlayFromMap(byte color) const;
	Common::Point findNearestWalkable(int x, int y) const;

	bool findShortestPath(Common::Point p1, Common::Point p2, WalkingPath *path) const;
	void obliquePath(const WalkingPath& path, WalkingPath *obliquedPath);
	Sprite *newOverlayFromPath(const WalkingPath &path, byte color) const;
	Common::Point getDelta() const { return Common::Point(_deltaX, _deltaY); }

	static int pointsBetween(const Common::Point &p1, const Common::Point &p2);
	static Common::Point interpolate(const Common::Point &p1, const Common::Point &p2, int i, int n);

private:
	int _realWidth, _realHeight;
	int _deltaX, _deltaY;
	int _mapWidth, _mapHeight;
	int _byteWidth;

	// We don't own the pointer.  It points to the BArchive cache for this room.
	const byte *_data;

	// 4 possible directions to walk from a pixel.
	static const int kDirections[][2];

	void drawOverlayRectangle(const Common::Point &p, byte color, byte *buf) const;
	bool lineIsCovered(const Common::Point &p1, const Common::Point &p2) const;

	// Returns true if the number of vertices on the path was decreased.
	bool managedToOblique(WalkingPath *path) const;
};

/*
 * Enumerates the directions the dragon can look into when arrived.
 */
enum SightDirection {
	kDirectionLast, kDirectionMouse, kDirectionUnknown,
	kDirectionRight, kDirectionLeft, kDirectionIntelligent
};

/**
  * Enumerates the animations for the dragon's movement.
  */
enum Movement {
	kMoveUndefined = -1,
	kMoveDown, kMoveUp, kMoveRight, kMoveLeft,

	kFirstTurning,
	kMoveRightDown = kFirstTurning, kMoveRightUp, kMoveLeftDown, kMoveLeftUp,
	kMoveDownRight, kMoveUpRight, kMoveDownLeft, kMoveUpLeft,
	kMoveLeftRight, kMoveRightLeft, kMoveUpStopLeft, kMoveUpStopRight,
	kLastTurning = kMoveUpStopRight,

	kSpeakRight, kSpeakLeft, kStopRight, kStopLeft,

	kFirstTemporaryAnimation
};

class DraciEngine;
struct GPL2Program;

class WalkingState {
public:
	explicit WalkingState(DraciEngine *vm) : _vm(vm) { stopWalking(); }
	~WalkingState() {}

	void stopWalking();
	void startWalking(const Common::Point &p1, const Common::Point &p2,
		const Common::Point &mouse, SightDirection dir,
		const Common::Point &delta, const WalkingPath& path);
	const WalkingPath& getPath() const { return _path; }

	void setCallback(const GPL2Program *program, uint16 offset);
	void callback();

	bool isActive() const { return _path.size() > 0; }

	// Advances the hero along the path and changes animation accordingly.
	// Walking MUST be active when calling this method.  When the hero has
	// arrived to the target, returns false, but leaves the callback
	// untouched (the caller must call it).
	// The second variant also clears the path when returning false.
	bool continueWalking();
	bool continueWalkingOrClearPath();

	// Called when the hero's turning animation has finished.
	void heroAnimationFinished();

	// Returns the hero's animation corresponding to looking into given
	// direction.  The direction can be smart and in that case this
	// function needs to know the whole last path, the current position of
	// the hero, or the mouse position.
	static Movement animationForSightDirection(SightDirection dir, const Common::Point &hero, const Common::Point &mouse, const WalkingPath &path, Movement startingDirection);

private:
	DraciEngine *_vm;

	WalkingPath _path;
	Common::Point _mouse;
	SightDirection _dir;
	Movement _startingDirection;

	uint _segment;		// Index of the path vertex we are currently going to / rotation on
	int _lastAnimPhase;
	bool _turningFinished;

	const GPL2Program *_callback;
	uint16 _callbackOffset;

	// Initiates turning of the dragon into the direction for the next
	// segment / after walking.  Returns false when there is nothing left
	// to do and walking is done.
	bool turnForTheNextSegment();

	// Starts walking on the next edge.  Returns false if we are already at
	// the final vertex and walking is done.
	bool walkOnNextEdge();

	// Return one of the 4 animations kMove{Down,Up,Right,Left}
	// corresponding to walking from here to there.
	static Movement animationForDirection(const Common::Point &here, const Common::Point &there);

	// Returns the desired facing direction to begin the next phase of the
	// walk.  It's either a direction for the given edge or the desired
	// final direction.
	Movement directionForNextPhase() const;

	// Returns either animation that needs to be played between given two
	// animations (e.g., kMoveRightDown after kMoveRight and before
	// kMoveDown), or kMoveUndefined if none animation is to be played.
	static Movement transitionBetweenAnimations(Movement previous, Movement next);

	static bool isTurningMovement(Movement m) {
		return m >= kFirstTurning && m <= kLastTurning;
	}

	// Projects hero to the given edge.  Returns true when hero has reached
	// at least p2.  prevHero is passed so that we can compute how much to
	// adjust in the other-than-walking direction.
	static bool alignHeroToEdge(const Common::Point &p1, const Common::Point &p2, const Common::Point &prevHero, Common::Point *hero);
};

} // End of namespace Draci

#endif // DRACI_WALKING_H
