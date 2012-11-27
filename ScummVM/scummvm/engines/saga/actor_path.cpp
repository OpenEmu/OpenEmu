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

#include "saga/saga.h"

#include "saga/actor.h"
#include "saga/objectmap.h"
#include "saga/scene.h"

namespace Saga {

static const PathDirectionData pathDirectionLUT[8][3] = {
	{ { 0,  0, -1 }, { 7, -1, -1 }, { 4,  1, -1 } },
	{ { 1,  1,  0 }, { 4,  1, -1 }, { 5,  1,  1 } },
	{ { 2,  0,  1 }, { 5,  1,  1 }, { 6, -1,  1 } },
	{ { 3, -1,  0 }, { 6, -1,  1 }, { 7, -1, -1 } },
	{ { 0,  0, -1 }, { 1,  1,  0 }, { 4,  1, -1 } },
	{ { 1,  1,  0 }, { 2,  0,  1 }, { 5,  1,  1 } },
	{ { 2,  0,  1 }, { 3, -1,  0 }, { 6, -1,  1 } },
	{ { 3, -1,  0 }, { 0,  0, -1 }, { 7, -1, -1 } }
};

static const int pathDirectionLUT2[8][2] = {
	{  0, -1 },
	{  1,  0 },
	{  0,  1 },
	{ -1,  0 },
	{  1, -1 },
	{  1,  1 },
	{ -1,  1 },
	{ -1, -1 }
};

inline int16 int16Compare(int16 i1, int16 i2) {
	return ((i1) > (i2) ? 1 : ((i1) < (i2) ? -1 : 0));
}

inline int16 quickDistance(const Point &point1, const Point &point2, int16 compressX) {
	Point delta;
	delta.x = ABS(point1.x - point2.x) / compressX;
	delta.y = ABS(point1.y - point2.y);
	return ((delta.x < delta.y) ? (delta.y + delta.x / 2) : (delta.x + delta.y / 2));
}

inline void calcDeltaS(const Point &point1, const Point &point2, Point &delta, Point &s) {

	delta.x = point2.x - point1.x;
	if (delta.x == 0) {
		s.x = 0;
	} else {
		if (delta.x > 0) {
			s.x = 1;
		} else {
			s.x = -1;
			delta.x = -delta.x;
		}
	}


	delta.y = point2.y - point1.y;
	if (delta.y == 0) {
		s.y = 0;
	} else {
		if (delta.y > 0) {
			s.y = 1;
		} else {
			s.y = -1;
			delta.y = -delta.y;
		}
	}
}

void Actor::findActorPath(ActorData *actor, const Point &fromPoint, const Point &toPoint) {
	Point iteratorPoint;
	Point bestPoint;
	int maskType;
	int i;
	Rect intersect;

#ifdef ACTOR_DEBUG
	_debugPointsCount = 0;
#endif

	// WORKAROUND for bug #3360396. Path finding in IHNM is a bit buggy
	// compared to the original, which occasionally leads to the player
	// leaving the room instead of interacting with an object. So far, no
	// one has figured out how to fix this properly. As a temporary [*]
	// solution, we try to fix this on a case-by-case basis.
	//
	// The workaround is to assume that the player wants to stay in the
	// room, unless he or she explicitly clicked on an exit zone.
	//
	// [*] And there is nothing more permanent than a temporary solution...

	bool pathFindingWorkaround = false;

	if (_vm->getGameId() == GID_IHNM) {
		int chapter = _vm->_scene->currentChapterNumber();
		int scene = _vm->_scene->currentSceneNumber();

		// Ellen, in the room with the monitors.
		if (chapter == 3 && scene == 54)
			pathFindingWorkaround = true;

		// Nimdok in the recovery room
		if (chapter == 4 && scene == 71)
			pathFindingWorkaround = true;
	}

	int hitZoneIndex;
	const HitZone *hitZone;
	bool restrictToRoom = false;

	if (pathFindingWorkaround) {
		restrictToRoom = true;
		hitZoneIndex = _vm->_scene->_actionMap->hitTest(toPoint);
		if (hitZoneIndex != -1) {
			hitZone = _vm->_scene->_actionMap->getHitZone(hitZoneIndex);
			if (hitZone->getFlags() & kHitZoneExit) {
				restrictToRoom = false;
			}
		}
	}

	actor->_walkStepsCount = 0;
	if (fromPoint == toPoint) {
		actor->addWalkStepPoint(toPoint);
		return;
	}

	for (iteratorPoint.y = 0; iteratorPoint.y < _yCellCount; iteratorPoint.y++) {
		for (iteratorPoint.x = 0; iteratorPoint.x < _xCellCount; iteratorPoint.x++) {
			if (_vm->_scene->validBGMaskPoint(iteratorPoint)) {
				maskType = _vm->_scene->getBGMaskType(iteratorPoint);
				setPathCell(iteratorPoint, _vm->_scene->getDoorState(maskType) ? kPathCellBarrier : kPathCellEmpty);
				if (restrictToRoom) {
					hitZoneIndex = _vm->_scene->_actionMap->hitTest(iteratorPoint);
					if (hitZoneIndex != -1) {
						hitZone = _vm->_scene->_actionMap->getHitZone(hitZoneIndex);
						if (hitZone->getFlags() & kHitZoneExit) {
							setPathCell(iteratorPoint, kPathCellBarrier);
						}
					}
				}
			} else {
				setPathCell(iteratorPoint, kPathCellBarrier);
			}
		}
	}

	for (i = 0; i < _barrierCount; i++) {
		intersect.left = MAX(_pathRect.left, _barrierList[i].left);
		intersect.top = MAX(_pathRect.top, _barrierList[i].top);
		intersect.right = MIN(_pathRect.right, _barrierList[i].right);
		intersect.bottom = MIN(_pathRect.bottom, _barrierList[i].bottom);

		for (iteratorPoint.y = intersect.top; iteratorPoint.y < intersect.bottom; iteratorPoint.y++) {
			for (iteratorPoint.x = intersect.left; iteratorPoint.x < intersect.right; iteratorPoint.x++) {
				setPathCell(iteratorPoint, kPathCellBarrier);
			}
		}
	}

#ifdef ACTOR_DEBUG
	for (iteratorPoint.y = 0; iteratorPoint.y < _yCellCount; iteratorPoint.y++) {
		for (iteratorPoint.x = 0; iteratorPoint.x < _xCellCount; iteratorPoint.x++) {
			if (getPathCell(iteratorPoint) == kPathCellBarrier) {
				addDebugPoint(iteratorPoint, 24);
			}
		}
	}
#endif

	if (scanPathLine(fromPoint, toPoint)) {
		actor->addWalkStepPoint(fromPoint);
		actor->addWalkStepPoint(toPoint);
		return;
	}

	i = fillPathArray(fromPoint, toPoint, bestPoint);

	if (fromPoint == bestPoint) {
		actor->addWalkStepPoint(bestPoint);
		return;
	}

	if (i == 0) {
		error("fillPathArray returns zero");
	}

	setActorPath(actor, fromPoint, bestPoint);
}

bool Actor::scanPathLine(const Point &point1, const Point &point2) {
	Point point;
	Point delta;
	Point s;
	Point fDelta;
	int16 errterm;

	calcDeltaS(point1, point2, delta, s);
	point = point1;

	fDelta.x = delta.x * 2;
	fDelta.y = delta.y * 2;

	if (delta.y > delta.x) {

		errterm = fDelta.x - delta.y;

		while (delta.y > 0) {
			while (errterm >= 0) {
				point.x += s.x;
				errterm -= fDelta.y;
			}

			point.y += s.y;
			errterm += fDelta.x;

			if (!validPathCellPoint(point)) {
				return false;
			}
			if (getPathCell(point) == kPathCellBarrier) {
				return false;
			}
			delta.y--;
		}
	} else {

		errterm = fDelta.y - delta.x;

		while (delta.x > 0) {
			while (errterm >= 0) {
				point.y += s.y;
				errterm -= fDelta.x;
			}

			point.x += s.x;
			errterm += fDelta.y;

			if (!validPathCellPoint(point)) {
				return false;
			}
			if (getPathCell(point) == kPathCellBarrier) {
				return false;
			}
			delta.x--;
		}
	}
	return true;
}

int Actor::fillPathArray(const Point &fromPoint, const Point &toPoint, Point &bestPoint) {
	int bestRating;
	int currentRating;
	Point bestPath;
	int pointCounter;
	const PathDirectionData *samplePathDirection;
	Point nextPoint;
	int directionCount;
	int16 compressX = (_vm->getGameId() == GID_ITE) ? 2 : 1;

	Common::List<PathDirectionData> pathDirectionQueue;

	pointCounter = 0;
	bestRating = quickDistance(fromPoint, toPoint, compressX);
	bestPath = fromPoint;

	for (int8 startDirection = 0; startDirection < 4; startDirection++) {
		PathDirectionData tmp = { startDirection, fromPoint.x, fromPoint.y };
		pathDirectionQueue.push_back(tmp);
	}

	if (validPathCellPoint(fromPoint)) {
		setPathCell(fromPoint, kDirUp);

#ifdef ACTOR_DEBUG
		addDebugPoint(fromPoint, 24+36);
#endif
	}

	while (!pathDirectionQueue.empty()) {
		PathDirectionData curPathDirection = pathDirectionQueue.front();
		pathDirectionQueue.pop_front();
		for (directionCount = 0; directionCount < 3; directionCount++) {
			samplePathDirection = &pathDirectionLUT[curPathDirection.direction][directionCount];
			nextPoint = Point(curPathDirection.x, curPathDirection.y);
			nextPoint.x += samplePathDirection->x;
			nextPoint.y += samplePathDirection->y;

			if (!validPathCellPoint(nextPoint)) {
				continue;
			}

			if (getPathCell(nextPoint) != kPathCellEmpty) {
				continue;
			}

			setPathCell(nextPoint, samplePathDirection->direction);

#ifdef ACTOR_DEBUG
			addDebugPoint(nextPoint, samplePathDirection->direction + 96);
#endif
			PathDirectionData tmp = {
				samplePathDirection->direction,
				nextPoint.x, nextPoint.y };
			pathDirectionQueue.push_back(tmp);
			++pointCounter;
			if (nextPoint == toPoint) {
				bestPoint = toPoint;
				return pointCounter;
			}
			currentRating = quickDistance(nextPoint, toPoint, compressX);
			if (currentRating < bestRating) {
				bestRating = currentRating;
				bestPath = nextPoint;
			}
		}
	}

	bestPoint = bestPath;
	return pointCounter;
}

void Actor::setActorPath(ActorData *actor, const Point &fromPoint, const Point &toPoint) {
	Point nextPoint;
	int8 direction;

	_pathList[0] = toPoint;
	nextPoint = toPoint;

	_pathListIndex = 0;
	while (!(nextPoint == fromPoint)) {
		direction = getPathCell(nextPoint);
		if ((direction < 0) || (direction >= 8)) {
			error("Actor::setActorPath error direction 0x%X", direction);
		}
		nextPoint.x -= pathDirectionLUT2[direction][0];
		nextPoint.y -= pathDirectionLUT2[direction][1];
		++_pathListIndex;
		if (_pathListIndex >= _pathList.size()) {
			_pathList.push_back(nextPoint);
		} else {
			_pathList[_pathListIndex] = nextPoint;
		}

#ifdef ACTOR_DEBUG
		addDebugPoint(nextPoint, 0x8a);
#endif
	}

	pathToNode();
	removeNodes();
	nodeToPath();
	removePathPoints();

	for (uint i = 0; i < _pathNodeList.size(); i++) {
		actor->addWalkStepPoint(_pathNodeList[i].point);
	}
}

void Actor::pathToNode() {
	Point point1, point2, delta;
	int direction;
	int i;

	direction = 0;

	_pathNodeList.clear();
	_pathNodeList.push_back(PathNode(_pathList[_pathListIndex]));

	for (i = _pathListIndex; i > 0; i--) {
		point1 = _pathList[i];
		point2 = _pathList[i - 1];
		if (direction == 0) {
			delta.x = int16Compare(point2.x, point1.x);
			delta.y = int16Compare(point2.y, point1.y);
			direction++;
		}
		if ((point1.x + delta.x != point2.x) || (point1.y + delta.y != point2.y)) {
			_pathNodeList.push_back(PathNode(point1));
			direction--;
			i++;
		}
	}
	_pathNodeList.push_back(PathNode(_pathList[0]));
}

uint pathLine(PointList &pointList, uint idx, const Point &point1, const Point &point2) {
	Point point;
	Point delta;
	Point tempPoint;
	Point s;
	int16 errterm;
	uint res;

	calcDeltaS(point1, point2, delta, s);

	point = point1;

	tempPoint.x = delta.x * 2;
	tempPoint.y = delta.y * 2;

	if (delta.y > delta.x) {

		errterm = tempPoint.x - delta.y;
		res = delta.y;

		while (delta.y > 0) {
			while (errterm >= 0) {
				point.x += s.x;
				errterm -= tempPoint.y;
			}

			point.y += s.y;
			errterm += tempPoint.x;

			if (idx >= pointList.size()) {
				pointList.push_back(point);
			} else {
				pointList[idx] = point;
			}
			++idx;
			delta.y--;
		}
	} else {

		errterm = tempPoint.y - delta.x;
		res = delta.x;

		while (delta.x > 0) {
			while (errterm >= 0) {
				point.y += s.y;
				errterm -= tempPoint.x;
			}

			point.x += s.x;
			errterm += tempPoint.y;

			if (idx >= pointList.size()) {
				pointList.push_back(point);
			} else {
				pointList[idx] = point;
			}
			++idx;
			delta.x--;
		}
	}
	return res;
}

void Actor::nodeToPath() {
	uint i;
	Point point1, point2;

	for (i = 0; i < _pathList.size(); i++) {
		_pathList[i].x = _pathList[i].y = PATH_NODE_EMPTY;
	}

	_pathListIndex = 1;
	_pathList[0] = _pathNodeList[0].point;
	_pathNodeList[0].link = 0;
	for (i = 0; i < _pathNodeList.size() - 1; i++) {
		point1 = _pathNodeList[i].point;
		point2 = _pathNodeList[i + 1].point;
		_pathListIndex += pathLine(_pathList, _pathListIndex, point1, point2);
		_pathNodeList[i + 1].link = _pathListIndex - 1;
	}
	_pathListIndex--;
	_pathNodeList.back().link = _pathListIndex;

}

void Actor::removeNodes() {
	uint i, j, k;

	// If there are only two nodes, nothing to be done
	if (_pathNodeList.size() <= 2)
		return;

	// If there is a direct connection between the first and last node, we can
	// remove all nodes in between and are done.
	if (scanPathLine(_pathNodeList.front().point, _pathNodeList.back().point)) {
		_pathNodeList[1] = _pathNodeList.back();
		_pathNodeList.resize(2);
	}

	if (_pathNodeList.size() <= 4)
		return;

	// Scan the nodes in reverse order, and look for a node which can be
	// directly reached from the first node. If we find any, skip directly
	// from the first node to that node (by marking all nodes in between as
	// empty).
	for (i = _pathNodeList.size() - 2; i > 1; i--) {
		if (_pathNodeList[i].point.x == PATH_NODE_EMPTY) {
			continue;
		}

		if (scanPathLine(_pathNodeList.front().point, _pathNodeList[i].point)) {
			for (j = 1; j < i; j++) {
				_pathNodeList[j].point.x = PATH_NODE_EMPTY;
			}
			break;
		}
	}

	// Now do the reverse: Find the first node which is directly connected
	// to the end node; if we find any, skip over all nodes in between.
	for (i = 1; i < _pathNodeList.size() - 2; i++) {
		if (_pathNodeList[i].point.x == PATH_NODE_EMPTY) {
			continue;
		}

		if (scanPathLine(_pathNodeList.back().point, _pathNodeList[i].point)) {
			for (j = i + 1; j < _pathNodeList.size() - 1; j++) {
				_pathNodeList[j].point.x = PATH_NODE_EMPTY;
			}
			break;
		}
	}
	condenseNodeList();

	// Finally, try arbitrary combinations of non-adjacent nodes and see
	// if we can skip over any of them.
	for (i = 1; i < _pathNodeList.size() - 2; i++) {
		if (_pathNodeList[i].point.x == PATH_NODE_EMPTY) {
			continue;
		}
		for (j = i + 2; j < _pathNodeList.size() - 1; j++) {
			if (_pathNodeList[j].point.x == PATH_NODE_EMPTY) {
				continue;
			}

			if (scanPathLine(_pathNodeList[i].point, _pathNodeList[j].point)) {
				for (k = i + 1; k < j; k++) {
					_pathNodeList[k].point.x = PATH_NODE_EMPTY;
				}
			}
		}
	}
	condenseNodeList();
}

/** Remove all empty nodes from _pathNodeList. */
void Actor::condenseNodeList() {
	uint i, j, count;

	count = _pathNodeList.size();
	for (i = 1; i < _pathNodeList.size() - 1; i++) {
		if (_pathNodeList[i].point.x == PATH_NODE_EMPTY) {
			j = i + 1;
			while (_pathNodeList[j].point.x == PATH_NODE_EMPTY) {
				j++;
			}
			_pathNodeList[i] = _pathNodeList[j];
			count = i + 1;
			_pathNodeList[j].point.x = PATH_NODE_EMPTY;
			if (j == _pathNodeList.size() - 1) {
				break;
			}
		}
	}
	_pathNodeList.resize(count);
}

void Actor::removePathPoints() {
	uint i, j, l;
	int start;
	int end;
	Point point1, point2;

	if (_pathNodeList.size() <= 2)
		return;

	PathNodeList newPathNodeList;

	// Add the first node
	newPathNodeList.push_back(_pathNodeList.front());

	// Process all nodes between the first and the last.
	for (i = 1; i < _pathNodeList.size() - 1; i++) {
		newPathNodeList.push_back(_pathNodeList[i]);

		for (j = 5; j > 0; j--) {
			start = _pathNodeList[i].link - j;
			end = _pathNodeList[i].link + j;

			if (start < 0 || end > (int)_pathListIndex) {
				continue;
			}

			point1 = _pathList[start];
			point2 = _pathList[end];
			if ((point1.x == PATH_NODE_EMPTY) || (point2.x == PATH_NODE_EMPTY)) {
				continue;
			}

			if (scanPathLine(point1, point2)) {
				for (l = 1; l < newPathNodeList.size(); l++) {
					if (start <= newPathNodeList[l].link) {
						newPathNodeList.resize(l + 1);
						newPathNodeList.back().point = point1;
						newPathNodeList.back().link = start;
						newPathNodeList.resize(l + 2);
						break;
					}
				}
				newPathNodeList.back().point = point2;
				newPathNodeList.back().link = end;

				for (int k = start + 1; k < end; k++) {
					_pathList[k].x = PATH_NODE_EMPTY;
				}
				break;
			}
		}
	}

	// Add the last node
	newPathNodeList.push_back(_pathNodeList.back());

	// Copy newPathNodeList into _pathNodeList, skipping any duplicate points
	_pathNodeList.clear();
	for (i = 0; i < newPathNodeList.size(); i++) {
		if (((newPathNodeList.size() - 1) == i) || (newPathNodeList[i].point != newPathNodeList[i + 1].point)) {
			_pathNodeList.push_back(newPathNodeList[i]);
		}
	}
}

#ifdef ACTOR_DEBUG
void Actor::drawPathTest() {
	uint i;

	for (i = 0; i < _debugPointsCount; i++) {
		_vm->_gfx->setPixelColor(_debugPoints[i].point.x, _debugPoints[i].point.y, _debugPoints[i].color);
	}
}
#endif

} // End of namespace Saga
