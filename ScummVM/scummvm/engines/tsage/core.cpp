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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "common/config-manager.h"
#include "common/util.h"
#include "engines/engine.h"
#include "graphics/palette.h"
#include "tsage/tsage.h"
#include "tsage/core.h"
#include "tsage/dialogs.h"
#include "tsage/events.h"
#include "tsage/scenes.h"
#include "tsage/staticres.h"
#include "tsage/globals.h"
#include "tsage/sound.h"
#include "tsage/blue_force/blueforce_logic.h"
#include "tsage/ringworld2/ringworld2_logic.h"

namespace TsAGE {

// The engine uses ScumMVM screen buffering, so all logic is hardcoded to use pane buffer 0
#define CURRENT_PANENUM 0

/*--------------------------------------------------------------------------*/

InvObject::InvObject(int sceneNumber, int rlbNum, int cursorNum, CursorType cursorId, const Common::String description) :
		_sceneNumber(sceneNumber), _rlbNum(rlbNum), _cursorNum(cursorNum), _cursorId(cursorId),
		_description(description) {
	_displayResNum = 3;
	_iconResNum = 5;

	// Decode the image for the inventory item to get it's display bounds
	uint size;
	byte *imgData = g_resourceManager->getSubResource(_displayResNum, _rlbNum, _cursorNum, &size);
	GfxSurface s = surfaceFromRes(imgData);
	_bounds = s.getBounds();

	DEALLOCATE(imgData);
}

InvObject::InvObject(int visage, int strip, int frame) {
	assert(g_vm->getGameID() == GType_BlueForce);
	_visage = visage;
	_strip = strip;
	_frame = frame;
	_sceneNumber = 0;
	_iconResNum = 10;
}

InvObject::InvObject(int strip, int frame) {
	assert(g_vm->getGameID() == GType_Ringworld2);
	_strip = strip;
	_frame = frame;

	_visage = 7;
	_sceneNumber = 0;
	_iconResNum = 10;
}

void InvObject::setCursor() {
	if (g_vm->getGameID() != GType_Ringworld) {
		// All other games
		_cursorId = (CursorType)BF_GLOBALS._inventory->indexOf(this);
		g_globals->_events.setCursor(_cursorId);
	} else {
		// Ringworld cursor handling
		g_globals->_events._currentCursor = _cursorId;

		if (_iconResNum != -1) {
			GfxSurface s = surfaceFromRes(_iconResNum, _rlbNum, _cursorNum);

			Graphics::Surface src = s.lockSurface();
			g_globals->_events.setCursor(src, s._transColor, s._centroid, _cursorId);
		}
	}
}

bool InvObject::inInventory() const {
	return _sceneNumber == ((g_vm->getGameID() != GType_Ringworld2) ? 1 : g_globals->_player._characterIndex);
}

/*--------------------------------------------------------------------------*/

InvObjectList::InvObjectList() {
	_selectedItem = NULL;
}

void InvObjectList::synchronize(Serializer &s) {
	SavedObject::synchronize(s);
	SYNC_POINTER(_selectedItem);
}

int InvObjectList::indexOf(InvObject *obj) const {
	int idx = 0;
	SynchronizedList<InvObject *>::const_iterator i;

	for (i = _itemList.begin(); i != _itemList.end(); ++i, ++idx) {
		if ((*i) == obj)
			return idx;
	}

	return -1;
}

InvObject *InvObjectList::getItem(int objectNum) {
	SynchronizedList<InvObject *>::const_iterator i = _itemList.begin();
	while (objectNum-- > 0)
		++i;

	return *i;
}

int InvObjectList::getObjectScene(int objectNum) {
	InvObject *obj = getItem(objectNum);
	return obj->_sceneNumber;
}

/*--------------------------------------------------------------------------*/

void EventHandler::dispatch() {
	if (_action) _action->dispatch();
}

void EventHandler::setAction(Action *action, EventHandler *endHandler, ...) {
	if (_action) {
		_action->_endHandler = NULL;
		_action->remove();
	}

	_action = action;
	if (action) {
		va_list va;
		va_start(va, endHandler);
		_action->attached(this, endHandler, va);
		va_end(va);
	}
}

/*--------------------------------------------------------------------------*/

Action::Action() {
	_actionIndex = 0;
	_owner = NULL;
	_endHandler = NULL;
	_attached = false;
}

void Action::synchronize(Serializer &s) {
	EventHandler::synchronize(s);
	if (s.getVersion() == 1)
		remove();

	SYNC_POINTER(_owner);
	s.syncAsSint32LE(_actionIndex);
	s.syncAsSint32LE(_delayFrames);
	s.syncAsUint32LE(_startFrame);
	s.syncAsByte(_attached);
	SYNC_POINTER(_endHandler);
}

void Action::remove() {
	if (_action)
		_action->remove();

	if (_owner) {
		_owner->_action = NULL;
		_owner = NULL;
	} else {
		g_globals->_sceneManager.removeAction(this);
	}

	_attached = false;
	if (_endHandler)
		_endHandler->signal();
}

void Action::process(Event &event) {
	if (_action)
		_action->process(event);
}

void Action::dispatch() {
	if (_action)
		_action->dispatch();

	if (_delayFrames) {
		uint32 frameNumber = g_globals->_events.getFrameNumber();

		if (frameNumber >= _startFrame) {
			_delayFrames -= frameNumber - _startFrame;
			_startFrame = frameNumber;
			if (_delayFrames <= 0) {
				_delayFrames = 0;
				signal();
			}
		}
	}
}

void Action::attached(EventHandler *newOwner, EventHandler *endHandler, va_list va) {
	_actionIndex = 0;
	_delayFrames = 0;
	_startFrame = g_globals->_events.getFrameNumber();
	_owner = newOwner;
	_endHandler = endHandler;
	_attached = true;
	signal();
}

void Action::setDelay(int numFrames) {
	_delayFrames = numFrames;
	_startFrame = g_globals->_events.getFrameNumber();
}

/*--------------------------------------------------------------------------*/

ObjectMover::~ObjectMover() {
	if (_sceneObject->_mover == this)
		_sceneObject->_mover = NULL;
}

void ObjectMover::synchronize(Serializer &s) {
	EventHandler::synchronize(s);

	s.syncAsSint16LE(_destPosition.x); s.syncAsSint16LE(_destPosition.y);
	s.syncAsSint16LE(_moveDelta.x); s.syncAsSint16LE(_moveDelta.y);
	s.syncAsSint16LE(_moveSign.x); s.syncAsSint16LE(_moveSign.y);
	s.syncAsSint32LE(_minorDiff);
	s.syncAsSint32LE(_majorDiff);
	s.syncAsSint32LE(_changeCtr);
	SYNC_POINTER(_action);
	SYNC_POINTER(_sceneObject);
}

void ObjectMover::remove() {
	if (_sceneObject->_mover == this)
		_sceneObject->_mover = NULL;

	delete this;
}

void ObjectMover::dispatch() {
	Common::Point currPos = _sceneObject->_position;
	int yDiff = _sceneObject->_yDiff;

	if (dontMove())
		return;

	_sceneObject->_regionIndex = 0;
	if (_moveDelta.x >= _moveDelta.y) {
		int xAmount = _moveSign.x * _sceneObject->_moveDiff.x * _sceneObject->_percent / 100;
		if (!xAmount)
			xAmount = _moveSign.x;
		currPos.x += xAmount;

		int yAmount = ABS(_destPosition.y - currPos.y);
		int yChange = _majorDiff / ABS(xAmount);
		int ySign;

		if (!yChange)
			ySign = _moveSign.y;
		else {
			int v = yAmount / yChange;
			_changeCtr += yAmount % yChange;
			if (_changeCtr >= yChange) {
				++v;
				_changeCtr -= yChange;
			}

			ySign = _moveSign.y * v;
		}

		currPos.y += ySign;
		_majorDiff -= ABS(xAmount);

	} else {
		int yAmount = _moveSign.y * _sceneObject->_moveDiff.y * _sceneObject->_percent / 100;
		if (!yAmount)
			yAmount = _moveSign.y;
		currPos.y += yAmount;

		int xAmount = ABS(_destPosition.x - currPos.x);
		int xChange = _majorDiff / ABS(yAmount);
		int xSign;

		if (!xChange)
			xSign = _moveSign.x;
		else {
			int v = xAmount / xChange;
			_changeCtr += xAmount % xChange;
			if (_changeCtr >= xChange) {
				++v;
				_changeCtr -= xChange;
			}

			xSign = _moveSign.x * v;
		}

		currPos.x += xSign;
		_majorDiff -= ABS(yAmount);
	}

	_sceneObject->_regionIndex = _sceneObject->checkRegion(currPos);
	if (!_sceneObject->_regionIndex) {
		_sceneObject->setPosition(currPos, yDiff);
		_sceneObject->getHorizBounds();

		if (dontMove()) {
			_sceneObject->_position = _destPosition;
			endMove();
		}
	} else {
		endMove();
	}
}

void ObjectMover::setup(const Common::Point &destPos) {
	_sceneObject->calcAngle(destPos);

	if ((_sceneObject->_objectWrapper) && !(_sceneObject->_flags & OBJFLAG_SUPPRESS_DISPATCH)) {
		if (g_vm->getGameID() == GType_Ringworld)
			_sceneObject->_objectWrapper->dispatch();
		else
			_sceneObject->updateAngle(destPos);
	}

	// Get the difference
	int diffX = destPos.x - _sceneObject->_position.x;
	int diffY = destPos.y - _sceneObject->_position.y;
	int xSign = (diffX < 0) ? -1 : (diffX > 0 ? 1 : 0);
	int ySign = (diffY < 0) ? -1 : (diffY > 0 ? 1 : 0);
	diffX = ABS(diffX);
	diffY = ABS(diffY);

	if (diffX < diffY) {
		_minorDiff = diffX / 2;
		_majorDiff = diffY;
	} else {
		_minorDiff = diffY / 2;
		_majorDiff = diffX;
	}

	// Set the destination position
	_destPosition = destPos;
	_moveDelta = Common::Point(diffX, diffY);
	_moveSign = Common::Point(xSign, ySign);
	_changeCtr = 0;

	if (!diffX && !diffY)
		// Object is already at the correct destination
		endMove();
}

bool ObjectMover::dontMove() const {
	return (_majorDiff <= 0);
}

void ObjectMover::endMove() {
	EventHandler *actionP = _action;
	remove();

	if (actionP)
		actionP->signal();
}

/*--------------------------------------------------------------------------*/

ObjectMover2::ObjectMover2() : ObjectMover() {
	_destObject = NULL;
}

void ObjectMover2::synchronize(Serializer &s) {
	ObjectMover::synchronize(s);

	SYNC_POINTER(_destObject);
	s.syncAsSint32LE(_minArea);
	s.syncAsSint32LE(_maxArea);
}

void ObjectMover2::dispatch() {
	int area = _sceneObject->getSpliceArea(_destObject);
	if (area > _maxArea) {
		// Setup again for the new destination
		setup(_destObject->_position);
		ObjectMover::dispatch();
	} else if (area >= _minArea) {
		// Keep dispatching
		ObjectMover::dispatch();
	} else {
		// Within minimum, so end move
		endMove();
	}
}

void ObjectMover2::startMove(SceneObject *sceneObj, va_list va) {
	// Set up fields
	_sceneObject = sceneObj;

	_minArea = va_arg(va, int);
	_maxArea = va_arg(va, int);
	_destObject = va_arg(va, SceneObject *);

	setup(_destObject->_position);
}

void ObjectMover2::endMove() {
	_sceneObject->_regionIndex = 0x40;
}

/*--------------------------------------------------------------------------*/

void ObjectMover3::dispatch() {
	int area = _sceneObject->getSpliceArea(_destObject);
	if (area <= _minArea) {
		endMove();
	} else {
		setup(_destObject->_position);
		ObjectMover::dispatch();
	}
}

void ObjectMover3::startMove(SceneObject *sceneObj, va_list va) {
	_sceneObject = sceneObj;
	_destObject = va_arg(va, SceneObject *);
	_minArea = va_arg(va, int);
	_action = va_arg(va, Action *);

	setup(_destObject->_position);
}

void ObjectMover3::endMove() {
	ObjectMover::endMove();
}

/*--------------------------------------------------------------------------*/

void NpcMover::startMove(SceneObject *sceneObj, va_list va) {
	_sceneObject = sceneObj;

	Common::Point *destPos = va_arg(va, Common::Point *);
	_action = va_arg(va, Action *);

	setup(*destPos);
}

/*--------------------------------------------------------------------------*/

void PlayerMover::synchronize(Serializer &s) {
	NpcMover::synchronize(s);

	s.syncAsSint16LE(_finalDest.x); s.syncAsSint16LE(_finalDest.y);
	s.syncAsSint32LE(_routeIndex);

	for (int i = 0; i < MAX_ROUTE_SIZE; ++i) {
		s.syncAsSint16LE(_routeList[i].x); s.syncAsSint16LE(_routeList[i].y);
	}
}

void PlayerMover::startMove(SceneObject *sceneObj, va_list va) {
	_sceneObject = sceneObj;
	Common::Point *pt = va_arg(va, Common::Point *);
	_finalDest = *pt;
	_action = va_arg(va, Action *);

	setDest(_finalDest);
}

void PlayerMover::endMove() {
	while (++_routeIndex != 0) {
		if ((_routeList[_routeIndex].x == ROUTE_END_VAL) ||
			(_routeList[_routeIndex].y == ROUTE_END_VAL) ||
			(_sceneObject->_regionIndex)) {
			// Movement route is completely finished
			ObjectMover::endMove();
			return;
		}

		if ((_routeList[_routeIndex].x != _sceneObject->_position.x) ||
			(_routeList[_routeIndex].y != _sceneObject->_position.y))
			break;
	}

	// Set up the new interim destination along the route
	g_globals->_walkRegions._routeEnds.moveSrc = g_globals->_walkRegions._routeEnds.moveDest;
	g_globals->_walkRegions._routeEnds.moveDest = _routeList[_routeIndex];
	setup(_routeList[_routeIndex]);
	dispatch();
}

void PlayerMover::setDest(const Common::Point &destPos) {
	_routeList[0] = _sceneObject->_position;

	if (g_globals->_walkRegions._resNum == -1) {
		// Scene has no walk regions defined, so player can walk anywhere directly
		_routeList[0] = destPos;
		_routeList[1] = Common::Point(ROUTE_END_VAL, ROUTE_END_VAL);
	} else {
		// Figure out a path to the destination (or as close as possible to it)
		pathfind(_routeList, _sceneObject->_position, destPos, g_globals->_walkRegions._routeEnds);
	}

	_routeIndex = 0;
	g_globals->_walkRegions._routeEnds.moveSrc = _sceneObject->_position;
	g_globals->_walkRegions._routeEnds.moveDest = _routeList[0];
	setup(_routeList[0]);
}

#define REGION_LIST_SIZE 20

void PlayerMover::pathfind(Common::Point *routeList, Common::Point srcPos, Common::Point destPos, RouteEnds routeEnds) {
	Common::List<int> regionIndexes;
	RouteEnds tempRouteEnds;
	int routeRegions[REGION_LIST_SIZE];
	Common::Point objPos;

	// Get the region the source is in
	int srcRegion = g_globals->_walkRegions.indexOf(srcPos);
	if (srcRegion == -1) {
		srcRegion = findClosestRegion(srcPos, regionIndexes);
	}

	// Main loop for building up the path
	routeRegions[0] = 0;
	while (!routeRegions[0]) {
		// Check the destination region
		int destRegion = g_globals->_walkRegions.indexOf(destPos, &regionIndexes);

		if ((srcRegion == -1) && (destRegion == -1)) {
			// Both source and destination are outside walkable areas
		} else if (srcRegion == -1) {
			// Source is outside walkable areas
			tempRouteEnds = routeEnds;
			objPos = _sceneObject->_position;

			Common::Point newPos;
			findLinePoint(&tempRouteEnds, &objPos, 1, &newPos);
			int srcId = g_globals->_walkRegions.indexOf(newPos);

			if (srcId == -1) {
				tempRouteEnds.moveDest = tempRouteEnds.moveSrc;
				tempRouteEnds.moveSrc = routeEnds.moveDest;

				findLinePoint(&tempRouteEnds, &objPos, 1, &newPos);
				srcRegion = g_globals->_walkRegions.indexOf(newPos);

				if (srcRegion == -1)
					srcRegion = checkMover(srcPos, destPos);
			}

		} else if (destRegion == -1) {
			// Destination is outside walkable areas
			destRegion = findClosestRegion(destPos, regionIndexes);
			if (destRegion == -1) {
				// No further route found, so end it
				*routeList++ = srcPos;
				break;
			} else {
				_finalDest = destPos;
			}
		}

		if (srcRegion == destRegion) {
			*routeList++ = (srcRegion == -1) ? srcPos : destPos;
			break;
		}

		bool tempVar; // This is used only as internal state for the function.
		calculateRestOfRoute(routeRegions, srcRegion, destRegion, tempVar);

		// Empty route?
		if (!routeRegions[0]) {
			regionIndexes.push_back(destRegion);
			continue;
		}

		// field 0 holds the start, and field 1 holds the destination
		WRField18 &currSrcField = g_globals->_walkRegions._field18[0];
		WRField18 &currDestField = g_globals->_walkRegions._field18[1];

		currSrcField._pt1 = srcPos;
		currSrcField._pt2 = srcPos;
		currDestField._pt1 = destPos;
		currDestField._pt2 = destPos;

		int tempList[REGION_LIST_SIZE];
		tempList[0] = 0;
		int endIndex = 0;
		int idx = 1;

		// Find the indexes for each entry in the found route.
		do {
			int breakEntry = routeRegions[idx];
			int breakEntry2 = routeRegions[idx + 1];

			int listIndex = 0;
			while (g_globals->_walkRegions._idxList[g_globals->_walkRegions[breakEntry]._idxListIndex + listIndex] !=
					breakEntry2)
				++listIndex;

			tempList[idx] = g_globals->_walkRegions._idxList2[g_globals->_walkRegions[breakEntry]._idxList2Index
				+ listIndex];

			++endIndex;
		} while (routeRegions[++idx] != destRegion);

		tempList[idx] = 1;
		for (int listIndex = 1; listIndex <= endIndex; ++listIndex) {
			int thisIdx = tempList[listIndex];
			int nextIdx = tempList[listIndex + 1];

			WRField18 &thisField = g_globals->_walkRegions._field18[thisIdx];
			WRField18 &nextField = g_globals->_walkRegions._field18[nextIdx];

			if (sub_F8E5_calculatePoint(currSrcField._pt1, nextField._pt1,
					thisField._pt1, thisField._pt2) &&
				sub_F8E5_calculatePoint(currSrcField._pt1, nextField._pt2,
					thisField._pt1, thisField._pt2))
				continue;

			Common::Point tempPt;
			if (sub_F8E5_calculatePoint(currSrcField._pt1, currDestField._pt1,
					thisField._pt1, thisField._pt2, &tempPt)) {
				// Add point to the route list
				currSrcField._pt1 = tempPt;
				*routeList++ = tempPt;
			} else {
				int dist1 =
					(findDistance(currSrcField._pt1, thisField._pt1) << 1) +
					(findDistance(thisField._pt1, currDestField._pt1) << 1) +
					findDistance(thisField._pt1, nextField._pt1) +
					findDistance(thisField._pt1, nextField._pt2);

				int dist2 =
					(findDistance(currSrcField._pt1, thisField._pt2) << 1) +
					(findDistance(thisField._pt2, currDestField._pt2) << 1) +
					findDistance(thisField._pt2, nextField._pt1) +
					findDistance(thisField._pt2, nextField._pt2);

				// Do 1 step of movement, storing the new position in objPos.
				if (dist1 < dist2) {
					doStepsOfNpcMovement(thisField._pt1, thisField._pt2, 1, objPos);
				} else {
					doStepsOfNpcMovement(thisField._pt2, thisField._pt1, 1, objPos);
				}

				// Update the current position.
				currSrcField._pt1 = objPos;
				*routeList++ = objPos;
			}
		}

		// Add in the route entry
		*routeList++ = currDestField._pt1;
	}

	// Mark the end of the path
	*routeList = Common::Point(ROUTE_END_VAL, ROUTE_END_VAL);
}

int PlayerMover::regionIndexOf(const Common::Point &pt) {
	for (uint idx = 0; idx < g_globals->_walkRegions._regionList.size(); ++idx) {
		if (g_globals->_walkRegions._regionList[idx].contains(pt))
			return idx + 1;
	}

	return 0;
}

int PlayerMover::findClosestRegion(Common::Point &pt, const Common::List<int> &indexList) {
	int newY = pt.y;
	int result = 0;

	for (int idx = 1; idx < SCREEN_WIDTH; ++idx, newY += idx) {
		int newX = pt.x + idx;
		result = regionIndexOf(newX, pt.y);

		if ((result == 0) || contains(indexList, result)) {
			newY = pt.y + idx;
			result = regionIndexOf(newX, newY);

			if ((result == 0) || contains(indexList, result)) {
				newX -= idx;
				result = regionIndexOf(newX, newY);

				if ((result == 0) || contains(indexList, result)) {
					newX -= idx;
					result = regionIndexOf(newX, newY);

					if ((result == 0) || contains(indexList, result)) {
						newY -= idx;
						result = regionIndexOf(newX, newY);

						if ((result == 0) || contains(indexList, result)) {
							newY -= idx;
							result = regionIndexOf(newX, newY);

							if ((result == 0) || contains(indexList, result)) {
								newX += idx;
								result = regionIndexOf(newX, newY);

								if ((result == 0) || contains(indexList, result)) {
									newX += idx;
									result = regionIndexOf(newX, newY);

									if ((result == 0) || contains(indexList, result)) {
										continue;
									}
								}
							}
						}
					}
				}
			}
		}

		// Found an index
		pt.x = newX;
		pt.y = newY;
		return result;
	}

	return (result == 0) ? -1 : result;
}

Common::Point *PlayerMover::findLinePoint(RouteEnds *routeEnds, Common::Point *objPos, int length, Common::Point *outPos) {
	int xp = objPos->x + (((routeEnds->moveDest.y - routeEnds->moveSrc.y) * 9) / 8);
	int yp = objPos->y - (((routeEnds->moveDest.x - routeEnds->moveSrc.x) * 8) / 9);

	int xDiff = xp - objPos->x;
	int yDiff = yp - objPos->y;
	int xDirection = (xDiff == 0) ? 0 : ((xDiff < 0) ? 1 : -1);
	int yDirection = (yDiff == 0) ? 0 : ((yDiff < 0) ? 1 : -1);
	xDiff = ABS(xDiff);
	yDiff = ABS(yDiff);
	int majorChange = MAX(xDiff, yDiff) / 2;

	int outX = objPos->x;
	int outY = objPos->y;

	while (length-- > 0) {
		if (xDiff < yDiff) {
			outY += yDirection;
			majorChange += xDiff;
			if (majorChange > yDiff) {
				majorChange -= yDiff;
				outX += xDirection;
			}
		} else {
			outX += xDirection;
			majorChange += yDiff;
			if (majorChange > xDiff) {
				majorChange -= xDiff;
				outY += yDirection;
			}
		}
	}

	outPos->x = outX;
	outPos->y = outY;
	return outPos;
}

int PlayerMover::checkMover(Common::Point &srcPos, const Common::Point &destPos) {
	int regionIndex = 0;
	Common::Point objPos = _sceneObject->_position;
	uint32 regionBitList = _sceneObject->_regionBitList;
	_sceneObject->_regionBitList = 0;

	_sceneObject->_position.x = srcPos.x;
	_sceneObject->_position.y = srcPos.y;
	_sceneObject->_mover = NULL;

	NpcMover *mover = new NpcMover();
	_sceneObject->addMover(mover, &destPos, NULL);

	// Handle automatic movement of the player until a walkable region is reached,
	// or the end point of the movement is
	do {
		_sceneObject->_mover->dispatch();

		// Scan walk regions for point
		for (uint idx = 0; idx < g_globals->_walkRegions._regionList.size(); ++idx) {
			if (g_globals->_walkRegions[idx].contains(_sceneObject->_position)) {
				regionIndex = idx + 1;
				srcPos = _sceneObject->_position;
				break;
			}
		}
	} while ((regionIndex == 0) && (_sceneObject->_mover) && !g_vm->shouldQuit());

	_sceneObject->_position = objPos;
	_sceneObject->_regionBitList = regionBitList;

	if (_sceneObject->_mover)
		_sceneObject->_mover->remove();

	_sceneObject->_mover = this;
	return regionIndex;
}

void PlayerMover::doStepsOfNpcMovement(const Common::Point &srcPos, const Common::Point &destPos, int numSteps, Common::Point &ptOut) {
	Common::Point objPos = _sceneObject->_position;
	_sceneObject->_position = srcPos;
	uint32 regionBitList = _sceneObject->_regionBitList;
	_sceneObject->_position = srcPos;
	_sceneObject->_mover = NULL;

	NpcMover *mover = new NpcMover();
	_sceneObject->addMover(mover, &destPos, NULL);

	while ((numSteps > 0) && ((_sceneObject->_position.x != destPos.x) || (_sceneObject->_position.y != destPos.y))) {
		_sceneObject->_mover->dispatch();
		--numSteps;
	}

	ptOut = _sceneObject->_position;
	_sceneObject->_position = objPos;
	_sceneObject->_regionBitList = regionBitList;

	if (_sceneObject->_mover)
		_sceneObject->_mover->remove();

	_sceneObject->_mover = this;
}

int PlayerMover::calculateRestOfRoute(int *routeList, int srcRegion, int destRegion, bool &foundRoute) {
	// Make a copy of the provided route. The first entry is the size.
	int tempList[REGION_LIST_SIZE + 1];
	foundRoute = false;
	for (int idx = 0; idx <= *routeList; ++idx)
		tempList[idx] = routeList[idx];

	if (*routeList == REGION_LIST_SIZE)
		// Sequence too long
		return 32000;

	int regionIndex;
	for (regionIndex = 1; regionIndex <= *tempList; ++regionIndex) {
		if (routeList[regionIndex] == srcRegion)
			// Current path returns to original source region, so don't allow it
			return 32000;
	}

	WalkRegion &srcWalkRegion = g_globals->_walkRegions[srcRegion];
	int distance;
	if (!routeList[0]) {
		// The route is empty (new route).
		distance = 0;
	} else {
		// Find the distance from the last region in the route.
		WalkRegion &region = g_globals->_walkRegions[routeList[*routeList]];
		distance = findDistance(srcWalkRegion._pt, region._pt);
	}

	// Add the srcRegion to the end of the route.
	tempList[++*tempList] = srcRegion;
	int ourListSize = *tempList;

	if (srcRegion == destRegion) {
		// We made a route to the destination; copy that route and return.
		foundRoute = true;
		for (int idx = ourListSize; idx <= *tempList; ++idx) {
			routeList[idx] = tempList[idx];
			++*routeList;
		}
		return distance;
	} else {
		// Find the first connected region leading to our destination.
		int foundIndex = 0;
		int idx = 0;
		int currDest;
		while ((currDest = g_globals->_walkRegions._idxList[srcWalkRegion._idxListIndex + idx]) != 0) {
			if (currDest == destRegion) {
				foundIndex = idx;
				break;
			}

			++idx;
		}

		// Check every connected region until we find a route to the destination (or we have no more to check).
		int bestDistance = 31990;
		while (((currDest = g_globals->_walkRegions._idxList[srcWalkRegion._idxListIndex + foundIndex]) != 0) && (!foundRoute)) {
			// Only check the region if it isn't in the list of explicitly disabled regions
			if (!contains(g_globals->_walkRegions._disabledRegions, (int)currDest)) {
				int newDistance = calculateRestOfRoute(tempList, currDest, destRegion, foundRoute);

				if ((newDistance <= bestDistance) || foundRoute) {
					// We found a shorter possible route, or one leading to the destination.

					// Overwrite the route with this new one.
					routeList[0] = ourListSize - 1;

					for (int i = ourListSize; i <= tempList[0]; ++i) {
						routeList[i] = tempList[i];
						++routeList[0];
					}

					bestDistance = newDistance;
				}

				// Truncate our local list to the size it was before the call.
				tempList[0] = ourListSize;
			}

			++foundIndex;
		}

		foundRoute = false;
		return bestDistance + distance;
	}
}

int PlayerMover::findDistance(const Common::Point &pt1, const Common::Point &pt2) {
	int diff = ABS(pt1.x - pt2.x);
	double xx = diff * diff;
	diff = ABS(pt1.y - pt2.y);
	double yy = diff * 8.0 / 7.0;
	yy *= yy;

	return (int)sqrt(xx + yy);
}

// Calculate intersection of the line segments pt1-pt2 and pt3-pt4.
// Return true if they intersect, and return the intersection in ptOut.
bool PlayerMover::sub_F8E5_calculatePoint(const Common::Point &pt1, const Common::Point &pt2, const Common::Point &pt3,
						  const Common::Point &pt4, Common::Point *ptOut) {
	double diffX1 = pt2.x - pt1.x;
	double diffY1 = pt2.y - pt1.y;
	double diffX2 = pt4.x - pt3.x;
	double diffY2 = pt4.y - pt3.y;
	double ratio1 = 0.0, ratio2 = 0.0;
	double adjustedY1 = 0.0, adjustedY2 = 0.0;

	// Calculate the ratios between the X and Y points.
	if (diffX1 != 0.0) {
		ratio1 = diffY1 / diffX1;
		adjustedY1 = pt1.y - (pt1.x * ratio1);
	}
	if (diffX2 != 0.0) {
		ratio2 = diffY2 / diffX2;
		adjustedY2 = pt3.y - (pt3.x * ratio2);
	}

	if (ratio1 == ratio2)
		return false;

	double xPos, yPos;
	if (diffX1 == 0) {
		if (diffX2 == 0)
			return false;

		xPos = pt1.x;
		yPos = ratio2 * xPos + adjustedY2;
	} else {
		xPos = (diffX2 == 0) ? pt3.x : (adjustedY2 - adjustedY1) / (ratio1 - ratio2);
		yPos = ratio1 * xPos + adjustedY1;
	}

	// This is our candidate point, which we must check for validity.
	Common::Point tempPt((int)(xPos + 0.5), (int)(yPos + 0.5));

	// Is tempPt inside the second bounds?
	if (!((tempPt.x >= pt3.x) && (tempPt.x <= pt4.x)))
		if (!((tempPt.x >= pt4.x) && (tempPt.x <= pt3.x)))
			return false;
	if (!((tempPt.y >= pt3.y) && (tempPt.y <= pt4.y)))
		if (!((tempPt.y >= pt4.y) && (tempPt.y <= pt3.y)))
			return false;

	// Is tempPt inside the first bounds?
	if (!((tempPt.x >= pt1.x) && (tempPt.x <= pt2.x)))
		if (!((tempPt.x >= pt2.x) && (tempPt.x <= pt1.x)))
			return false;
	if (!((tempPt.y >= pt1.y) && (tempPt.y <= pt2.y)))
		if (!((tempPt.y >= pt2.y) && (tempPt.y <= pt1.y)))
			return false;

	if (ptOut)
		*ptOut = tempPt;
	return true;
}

/*--------------------------------------------------------------------------*/

void PlayerMover2::synchronize(Serializer &s) {
	if (s.getVersion() >= 2)
		PlayerMover::synchronize(s);
	SYNC_POINTER(_destObject);
	s.syncAsSint16LE(_maxArea);
	s.syncAsSint16LE(_minArea);
}

void PlayerMover2::dispatch() {
	int total = _sceneObject->getSpliceArea(_destObject);

	if (total <= _minArea)
		endMove();
	else {
		setDest(_destObject->_position);
		ObjectMover::dispatch();
	}
}

void PlayerMover2::startMove(SceneObject *sceneObj, va_list va) {
	_sceneObject = sceneObj;
	_maxArea = va_arg(va, int);
	_minArea = va_arg(va, int);
	_destObject = va_arg(va, SceneObject *);

	PlayerMover::setDest(_destObject->_position);
}

void PlayerMover2::endMove() {
	_sceneObject->_regionIndex = 0x40;
}

/*--------------------------------------------------------------------------*/

PaletteModifier::PaletteModifier() {
	_scenePalette = NULL;
	_action = NULL;
}

/*--------------------------------------------------------------------------*/

PaletteModifierCached::PaletteModifierCached(): PaletteModifier() {
	_step = 0;
	_percent = 0;
}

void PaletteModifierCached::setPalette(ScenePalette *palette, int step) {
	_scenePalette = palette;
	_step = step;
	_percent = 100;
}

void PaletteModifierCached::synchronize(Serializer &s) {
	PaletteModifier::synchronize(s);

	s.syncAsByte(_step);
	s.syncAsSint32LE(_percent);
}

/*--------------------------------------------------------------------------*/

PaletteRotation::PaletteRotation() : PaletteModifierCached() {
	_percent = 0;
	_delayCtr = 0;
	_frameNumber = g_globals->_events.getFrameNumber();
	_idxChange = 1;
	_countdown = 0;
}

void PaletteRotation::synchronize(Serializer &s) {
	PaletteModifierCached::synchronize(s);

	s.syncAsSint32LE(_delayCtr);
	s.syncAsUint32LE(_frameNumber);
	s.syncAsSint32LE(_currIndex);
	s.syncAsSint32LE(_start);
	s.syncAsSint32LE(_end);
	s.syncAsSint32LE(_rotationMode);
	s.syncAsSint32LE(_duration);
	s.syncBytes(&_palette[0], 256 * 3);

	if (g_vm->getGameID() == GType_Ringworld2) {
		s.syncAsSint16LE(_idxChange);
		s.syncAsSint16LE(_countdown);
	}
}

void PaletteRotation::signal() {
	if (_countdown > 0) {
		--_countdown;
		return;
	}

	if (_delayCtr) {
		uint32 frameNumber = g_globals->_events.getFrameNumber();

		if (frameNumber >= _frameNumber) {
			_delayCtr -= frameNumber - _frameNumber;
			_frameNumber = frameNumber;

			if (_delayCtr < 0)
				_delayCtr = 0;
		}
	}

	if (_delayCtr)
		return;
	_delayCtr = _percent;
	if (_step)
		return;

	bool flag = true;
	switch (_rotationMode) {
	case -1:
		_currIndex -= _idxChange;
		if (_currIndex < _start) {
			flag = decDuration();
			if (flag)
				_currIndex = _end - 1;
		}
		break;
	case 1:
		_currIndex += _idxChange;
		if (_currIndex >= _end) {
			flag = decDuration();
			if (flag)
				_currIndex = _start;
		}
		break;
	case 2:
		_currIndex += _idxChange;
		if (_currIndex >= _end) {
			flag = decDuration();
			if (flag) {
				_currIndex = _end - 2;
				_rotationMode = 3;
			}
		}
		break;
	case 3:
		_currIndex -= _idxChange;
		if (_currIndex < _start) {
			flag = decDuration();
			if (flag) {
				_currIndex = _start + 1;
				_rotationMode = 2;
			}
		}
		break;
	}

	if (flag) {
		int count2 = _currIndex - _start;
		int count = _end - _currIndex;
		g_system->getPaletteManager()->setPalette((const byte *)&_palette[_currIndex * 3], _start, count);

		if (count2) {
			g_system->getPaletteManager()->setPalette((const byte *)&_palette[_start * 3], _start + count, count2);
		}
	}
}

void PaletteRotation::remove() {
	Action *action = _action;

	if (_idxChange)
		g_system->getPaletteManager()->setPalette((const byte *)&_palette[_start * 3], _start, _end - _start);

	_scenePalette->_listeners.remove(this);

	delete this;
	if (action)
		action->signal();
}

void PaletteRotation::set(ScenePalette *palette, int start, int end, int rotationMode, int duration, Action *action) {
	_duration = duration;
	_step = false;
	_action = action;
	_scenePalette = palette;

	Common::copy(&palette->_palette[0], &palette->_palette[256 * 3], &_palette[0]);

	_start = start;
	_end = end + 1;
	_rotationMode = rotationMode;

	switch (_rotationMode) {
	case -1:
	case 3:
		_currIndex = _end;
		break;
	default:
		_currIndex = _start;
		break;
	}
}

bool PaletteRotation::decDuration() {
	if (_duration) {
		if (--_duration == 0) {
			remove();
			return false;
		}
	}
	return true;
}

void PaletteRotation::setDelay(int amount) {
	_percent = _delayCtr = amount;
}

/*--------------------------------------------------------------------------*/

void PaletteFader::synchronize(Serializer &s) {
	PaletteModifierCached::synchronize(s);

	s.syncAsSint16LE(_step);
	s.syncAsSint16LE(_percent);
	s.syncBytes(&_palette[0], 256 * 3);
}

void PaletteFader::signal() {
	_percent -= _step;
	if (_percent > 0) {
		_scenePalette->fade((byte *)_palette, true /* 256 */, _percent);
	} else {
		remove();
	}
}

void PaletteFader::remove() {
	// Save of a copy of the object's action, since it will be used after the object is destroyed
	Action *action = _action;

	Common::copy(&_palette[0], &_palette[256 * 3], &_scenePalette->_palette[0]);
	_scenePalette->refresh();
	_scenePalette->_listeners.remove(this);
	delete this;

	if (action)
		action->signal();
}

void PaletteFader::setPalette(ScenePalette *palette, int step) {
	if (step < 0) {
		// Reverse step means moving from dest palette to source, so swap the two palettes
		byte tempPal[256 * 3];
		Common::copy(&palette->_palette[0], &palette->_palette[256 * 3], &tempPal[0]);
		Common::copy(&this->_palette[0], &this->_palette[256 * 3], &palette->_palette[0]);
		Common::copy(&tempPal[0], &tempPal[256 * 3], &this->_palette[0]);

		step = -step;
	}

	PaletteModifierCached::setPalette(palette, step);
}

/*--------------------------------------------------------------------------*/

ScenePalette::ScenePalette() {
	// Set a default gradiant range
	byte *palData = &_palette[0];
	for (int idx = 0; idx < 256; ++idx) {
		*palData++ = idx;
		*palData++ = idx;
		*palData++ = idx;
	}

	_field412 = 0;
}

ScenePalette::~ScenePalette() {
	clearListeners();
}

ScenePalette::ScenePalette(int paletteNum) {
	loadPalette(paletteNum);
}

bool ScenePalette::loadPalette(int paletteNum) {
	byte *palData = g_resourceManager->getResource(RES_PALETTE, paletteNum, 0, true);
	if (!palData)
		return false;

	int palStart = READ_LE_UINT16(palData);
	int palSize = READ_LE_UINT16(palData + 2);
	assert(palSize <= 256);

	byte *destP = &_palette[palStart * 3];
	byte *srcP = palData + 6;

	Common::copy(&srcP[0], &srcP[palSize * 3], destP);

	DEALLOCATE(palData);
	return true;
}

/**
 * Loads a palette from the passed raw data block
 */
void ScenePalette::loadPalette(const byte *pSrc, int start, int count) {
	Common::copy(pSrc, pSrc + count * 3, &_palette[start * 3]);
}

void ScenePalette::refresh() {
	// Set indexes for standard colors to closest color in the palette
	_colors.background = indexOf(255, 255, 255);	// White background
	_colors.foreground = indexOf(0, 0, 0);			// Black foreground
	_redColor = indexOf(180, 0, 0);				// Red-ish
	_greenColor = indexOf(0, 180, 0);				// Green-ish
	_blueColor = indexOf(0, 0, 180);				// Blue-ish
	_aquaColor = indexOf(0, 180, 180);				// Aqua
	_purpleColor = indexOf(180, 0, 180);			// Purple
	_limeColor = indexOf(180, 180, 0);				// Lime

	// Refresh the palette
	g_system->getPaletteManager()->setPalette((const byte *)&_palette[0], 0, 256);
}

/**
 * Loads a section of the palette into the game palette
 */
void ScenePalette::setPalette(int index, int count) {
	g_system->getPaletteManager()->setPalette((const byte *)&_palette[index * 3], index, count);
}

/**
 * Get a palette entry
 */
void ScenePalette::getEntry(int index, uint *r, uint *g, uint *b) {
	*r = _palette[index * 3];
	*g = _palette[index * 3 + 1];
	*b = _palette[index * 3 + 2];
}

/**
 * Set a palette entry
 */
void ScenePalette::setEntry(int index, uint r, uint g, uint b) {
	_palette[index * 3] = r;
	_palette[index * 3 + 1] = g;
	_palette[index * 3 + 2] = b;
}

/**
 * Returns the palette index with the closest matching color to that specified
 * @param r			R component
 * @param g			G component
 * @param b			B component
 * @param threshold	Closeness threshold.
 * @param start		Starting index
 * @param count		Number of indexes to scan
 * @remarks	A threshold may be provided to specify how close the matching color must be
 */
uint8 ScenePalette::indexOf(uint r, uint g, uint b, int threshold, int start, int count) {
	int palIndex = -1;
	byte *palData = &_palette[0];

	for (int i = start; i < (start + count); ++i) {
		byte ir = *palData++;
		byte ig = *palData++;
		byte ib = *palData++;
		int rDiff = abs(ir - (int)r);
		int gDiff = abs(ig - (int)g);
		int bDiff = abs(ib - (int)b);

		int idxThreshold = rDiff * rDiff + gDiff * gDiff + bDiff * bDiff;
		if (idxThreshold < threshold) {
			threshold = idxThreshold;
			palIndex = i;
		}
	}

	return palIndex;
}

/**
 * Loads the specified range of the palette with the current system palette
 * @param start		Start index
 * @param count		Number of palette entries
 */
void ScenePalette::getPalette(int start, int count) {
	g_system->getPaletteManager()->grabPalette((byte *)&_palette[start], start, count);
}

void ScenePalette::signalListeners() {
	SynchronizedList<PaletteModifier *>::iterator i = _listeners.begin();
	while (i != _listeners.end()) {
		PaletteModifier *obj = *i;
		++i;
		obj->signal();
	}
}

void ScenePalette::clearListeners() {
	SynchronizedList<PaletteModifier *>::iterator i = _listeners.begin();
	while (i != _listeners.end()) {
		PaletteModifier *obj = *i;
		++i;
		obj->remove();
	}
}

void ScenePalette::fade(const byte *adjustData, bool fullAdjust, int percent) {
	byte tempPalette[256 * 3];

	// Ensure the percent adjustment is within 0 - 100%
	percent = CLIP(percent, 0, 100);

	for (int palIndex = 0; palIndex < 256; ++palIndex) {
		const byte *srcP = (const byte *)&_palette[palIndex * 3];
		byte *destP = &tempPalette[palIndex * 3];

		for (int rgbIndex = 0; rgbIndex < 3; ++rgbIndex, ++srcP, ++destP) {
			*destP = *srcP - ((*srcP - adjustData[rgbIndex]) * (100 - percent)) / 100;
		}

		if (fullAdjust)
			adjustData += 3;
	}

	// Set the altered pale4tte
	g_system->getPaletteManager()->setPalette((const byte *)&tempPalette[0], 0, 256);
	GLOBALS._screenSurface.updateScreen();
}

PaletteRotation *ScenePalette::addRotation(int start, int end, int rotationMode, int duration, Action *action) {
	PaletteRotation *obj = new PaletteRotation();

	if ((rotationMode == 2) || (rotationMode == 3))
		duration <<= 1;

	obj->set(this, start, end, rotationMode, duration, action);
	_listeners.push_back(obj);
	return obj;
}

PaletteFader *ScenePalette::addFader(const byte *arrBufferRGB, int palSize, int step, Action *action) {
	PaletteFader *fader = new PaletteFader();
	fader->_action = action;
	for (int i = 0; i < 256 * 3; i += 3) {
		fader->_palette[i] = *(arrBufferRGB + 0);
		fader->_palette[i + 1] = *(arrBufferRGB + 1);
		fader->_palette[i + 2] = *(arrBufferRGB + 2);

		if (palSize > 1)
			arrBufferRGB += 3;
	}

	fader->setPalette(this, step);
	g_globals->_scenePalette._listeners.push_back(fader);
	return fader;
}


void ScenePalette::changeBackground(const Rect &bounds, FadeMode fadeMode) {
	ScenePalette tempPalette;

	if (g_globals->_sceneManager._hasPalette) {
		if ((fadeMode == FADEMODE_GRADUAL) || (fadeMode == FADEMODE_IMMEDIATE)) {
			// Fade out any active palette
			tempPalette.getPalette();
			uint32 adjustData = 0;

			for (int percent = 100; percent >= 0; percent -= 5) {
				if (fadeMode == FADEMODE_IMMEDIATE)
					percent = 0;
				tempPalette.fade((byte *)&adjustData, false, percent);
				g_system->delayMillis(10);
			}
		} else {
			g_globals->_scenePalette.refresh();
			g_globals->_sceneManager._hasPalette = false;
		}
	}

	Rect tempRect = bounds;
	if (g_vm->getGameID() != GType_Ringworld)
		tempRect.setHeight(T2_GLOBALS._interfaceY);

	g_globals->_screenSurface.copyFrom(g_globals->_sceneManager._scene->_backSurface,
		tempRect, Rect(0, 0, tempRect.width(), tempRect.height()), NULL);

	for (SynchronizedList<PaletteModifier *>::iterator i = tempPalette._listeners.begin(); i != tempPalette._listeners.end(); ++i)
		delete *i;
	tempPalette._listeners.clear();
}

void ScenePalette::synchronize(Serializer &s) {
	if (s.getVersion() >= 2)
		SavedObject::synchronize(s);
	if (s.getVersion() >= 5)
		_listeners.synchronize(s);

	s.syncBytes(_palette, 256 * 3);
	s.syncAsSint32LE(_colors.foreground);
	s.syncAsSint32LE(_colors.background);

	s.syncAsSint32LE(_field412);
	s.syncAsByte(_redColor);
	s.syncAsByte(_greenColor);
	s.syncAsByte(_blueColor);
	s.syncAsByte(_aquaColor);
	s.syncAsByte(_purpleColor);
	s.syncAsByte(_limeColor);
}

/*--------------------------------------------------------------------------*/

void SceneItem::synchronize(Serializer &s) {
	EventHandler::synchronize(s);

	_bounds.synchronize(s);
	s.syncString(_msg);
	s.syncAsSint32LE(_fieldE);
	s.syncAsSint32LE(_field10);
	s.syncAsSint16LE(_position.x); s.syncAsSint32LE(_position.y);
	s.syncAsSint16LE(_yDiff);
	s.syncAsSint32LE(_sceneRegionId);
}

void SceneItem::remove() {
	g_globals->_sceneItems.remove(this);
}

bool SceneItem::startAction(CursorType action, Event &event) {
	if (g_vm->getGameID() == GType_Ringworld) {
		doAction(action);
		return true;
	} else if ((action == CURSOR_LOOK) || (action == CURSOR_USE) || (action == CURSOR_TALK) ||
			(action < CURSOR_LOOK)) {
		doAction(action);
		return true;
	} else {
		return false;
	}
}

void SceneItem::doAction(int action) {
	const char *msg = NULL;

	if (g_vm->getGameID() == GType_Ringworld2) {
		Event dummyEvent;
		((Ringworld2::SceneExt *)GLOBALS._sceneManager._scene)->display((CursorType)action, dummyEvent);
	} else {
		switch ((int)action) {
		case CURSOR_LOOK:
			msg = LOOK_SCENE_HOTSPOT;
			break;
		case CURSOR_USE:
			msg = USE_SCENE_HOTSPOT;
			break;
		case CURSOR_TALK:
			msg = TALK_SCENE_HOTSPOT;
			break;
		case 0x1000:
			msg = SPECIAL_SCENE_HOTSPOT;
			break;
		default:
			msg = DEFAULT_SCENE_HOTSPOT;
			break;
		}

		GUIErrorMessage(msg);
	}
}

bool SceneItem::contains(const Common::Point &pt) {
	const Rect &sceneBounds = g_globals->_sceneManager._scene->_sceneBounds;

	if (_sceneRegionId == 0)
		return _bounds.contains(pt.x + sceneBounds.left, pt.y + sceneBounds.top);
	else
		return g_globals->_sceneRegions.indexOf(Common::Point(pt.x + sceneBounds.left,
			pt.y + sceneBounds.top)) == _sceneRegionId;
}

void SceneItem::display(int resNum, int lineNum, ...) {
	Common::String msg = (!resNum || (resNum == -1)) ? Common::String() :
		g_resourceManager->getMessage(resNum, lineNum);

	if ((g_vm->getGameID() != GType_Ringworld) && T2_GLOBALS._uiElements._active)
		T2_GLOBALS._uiElements.hide();

	if (g_globals->_sceneObjects->contains(&g_globals->_sceneText)) {
		g_globals->_sceneText.remove();
		g_globals->_sceneObjects->draw();
	}

	Common::Point pos(160, 100);
	Rect textRect;
	int maxWidth = 120;
	bool keepOnscreen = false;
	bool centerText = g_vm->getGameID() != GType_BlueForce;
	Common::List<int> playList;

	if (resNum != 0) {
		va_list va;
		va_start(va, lineNum);

		if (resNum == -1)
			msg = Common::String(va_arg(va, const char *));

		if (g_vm->getGameID() == GType_Ringworld2) {
			// Pre-process the string for any sound information
			while (msg.hasPrefix("!")) {
				msg.deleteChar(0);
				playList.push_back(atoi(msg.c_str()));

				while (!msg.empty() && (*msg.c_str() >= '0' && *msg.c_str() <= '9'))
					msg.deleteChar(0);
			}
		}

		int mode;
		do {
			// Get next instruction
			mode = va_arg(va, int);

			switch (mode) {
			case SET_WIDTH:
				// Set width
				maxWidth = va_arg(va, int);
				g_globals->_sceneText._width = maxWidth;
				break;
			case SET_X:
				// Set the X Position
				pos.x = va_arg(va, int);
				break;
			case SET_Y:
				// Set the Y Position
				pos.y = va_arg(va, int);
				break;
			case SET_FONT:
				// Set the font number
				g_globals->_sceneText._fontNumber = va_arg(va, int);
				g_globals->gfxManager()._font.setFontNumber(g_globals->_sceneText._fontNumber);
				break;
			case SET_BG_COLOR: {
				// Set the background color
				int bgColor = va_arg(va, int);
				g_globals->gfxManager()._font._colors.background = bgColor;
				if (!bgColor)
					g_globals->gfxManager().setFillFlag(false);
				break;
			}
			case SET_FG_COLOR:
				// Set the foreground color
				g_globals->_sceneText._color1 = va_arg(va, int);
				g_globals->gfxManager()._font._colors.foreground = g_globals->_sceneText._color1;
				break;
			case SET_KEEP_ONSCREEN:
				// Suppresses immediate display
				keepOnscreen = va_arg(va, int) != 0;
				break;
			case SET_EXT_BGCOLOR: {
				// Set secondary bg color
				int v = va_arg(va, int);
				g_globals->_sceneText._color2 = v;
				g_globals->gfxManager()._font._colors2.background = v;
				break;
			}
			case SET_EXT_FGCOLOR: {
				// Set secondary fg color
				int v = va_arg(va, int);
				g_globals->_sceneText._color3 = v;
				g_globals->gfxManager()._font._colors.foreground = v;
				break;
			}
			case SET_POS_MODE:
				// Set whether a custom x/y is used
				centerText = va_arg(va, int) != 0;
				break;
			case SET_TEXT_MODE:
				// Set the text mode
				g_globals->_sceneText._textMode = (TextAlign)va_arg(va, int);
				break;
			default:
				break;
			}
		} while (mode != LIST_END);

		va_end(va);
	}

	if (resNum) {
		// Get required bounding size
		GfxFont font;
		font.setFontNumber(g_globals->_sceneText._fontNumber);
		font.getStringBounds(msg.c_str(), textRect, maxWidth);

		// Center the text at the specified position, and then constrain it to be-
		textRect.center(pos.x, pos.y);
		textRect.contain(g_globals->gfxManager()._bounds);

		if (centerText) {
			g_globals->_sceneText._color1 = g_globals->_sceneText._color2;
			g_globals->_sceneText._color2 = 0;
			g_globals->_sceneText._color3 = 0;
		}

		g_globals->_sceneText.setup(msg);
		if (centerText) {
			g_globals->_sceneText.setPosition(Common::Point(
				g_globals->_sceneManager._scene->_sceneBounds.left + textRect.left,
				g_globals->_sceneManager._scene->_sceneBounds.top + textRect.top), 0);
		} else {
			g_globals->_sceneText.setPosition(pos, 0);
		}

		g_globals->_sceneText.fixPriority(255);
		g_globals->_sceneObjects->draw();
	}

	// Unless the flag is set to keep the message on-screen, show it until a mouse or keypress, then remove it
	if (!keepOnscreen && !msg.empty()) {
		Event event;

		// Keep event on-screen until a mouse or keypress
		while (!g_vm->shouldQuit() && !g_globals->_events.getEvent(event,
				EVENT_BUTTON_DOWN | EVENT_KEYPRESS)) {
			GLOBALS._screenSurface.updateScreen();
			g_system->delayMillis(10);
		}

		// For Return to Ringworld, play the voice overs in sequence
		if ((g_vm->getGameID() == GType_Ringworld2) && !playList.empty() && !R2_GLOBALS._playStream.isPlaying()) {
			R2_GLOBALS._playStream.play(*playList.begin(), NULL);
			playList.pop_front();
		}

		g_globals->_sceneText.remove();
	}

	if ((g_vm->getGameID() != GType_Ringworld) && T2_GLOBALS._uiElements._active) {
		// Show user interface
		T2_GLOBALS._uiElements.show();

		// Re-show the cursor
		BF_GLOBALS._events.setCursor(BF_GLOBALS._events.getCursor());
	}
}

void SceneItem::display2(int resNum, int lineNum) {
	switch (g_vm->getGameID()) {
	case GType_BlueForce:
		display(resNum, lineNum, SET_WIDTH, 312,
			SET_X, 4 + GLOBALS._sceneManager._scene->_sceneBounds.left,
			SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
			SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
			SET_EXT_FGCOLOR, 13, LIST_END);
		break;
	case GType_Ringworld2:
		display(resNum, lineNum, SET_WIDTH, 280, SET_X, 160, SET_Y, 20, SET_POS_MODE, ALIGN_CENTER,
			SET_EXT_BGCOLOR, 60, LIST_END);
		break;
	default:
		display(resNum, lineNum, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
		break;
	}
}

void SceneItem::display(const Common::String &msg) {
	assert(g_vm->getGameID() == GType_BlueForce);

	display(-1, -1, msg.c_str(),
		SET_WIDTH, 312,
		SET_X, 4 + GLOBALS._sceneManager._scene->_sceneBounds.left,
		SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
		SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
		SET_EXT_FGCOLOR, 13, LIST_END);
}

/*--------------------------------------------------------------------------*/

SceneHotspot::SceneHotspot(): SceneItem() {
	_lookLineNum = _useLineNum = _talkLineNum = 0;
}

void SceneHotspot::synchronize(Serializer &s) {
	SceneItem::synchronize(s);

	if (g_vm->getGameID() == GType_Ringworld2) {
		// In R2R, the following fields were moved into the SceneItem class
		s.syncAsSint16LE(_resNum);
		s.syncAsSint16LE(_lookLineNum);
		s.syncAsSint16LE(_useLineNum);
		s.syncAsSint16LE(_talkLineNum);
	}
}

bool SceneHotspot::startAction(CursorType action, Event &event) {
	switch (g_vm->getGameID()) {
	case GType_BlueForce: {
		BlueForce::SceneExt *scene = (BlueForce::SceneExt *)BF_GLOBALS._sceneManager._scene;
		assert(scene);
		return scene->display(action);
	}
	case GType_Ringworld2: {
		switch (action) {
		case CURSOR_LOOK:
			if (_lookLineNum != -1) {
				SceneItem::display2(_resNum, _lookLineNum);
				return true;
			}
			break;
		case CURSOR_USE:
			if (_useLineNum != -1) {
				SceneItem::display2(_resNum, _useLineNum);
				return true;
			}
			break;
		case CURSOR_TALK:
			if (_talkLineNum != -1) {
				SceneItem::display2(_resNum, _talkLineNum);
				return true;
			}
			break;
		default:
			break;
		}

		return ((Ringworld2::SceneExt *)GLOBALS._sceneManager._scene)->display(action, event);
	}
	default:
		return SceneItem::startAction(action, event);
	}
}

void SceneHotspot::doAction(int action) {
	switch ((int)action) {
	case CURSOR_LOOK:
		if (g_vm->getGameID() == GType_BlueForce)
			SceneItem::display(LOOK_SCENE_HOTSPOT);
		else
			display(1, 0, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
		break;
	case CURSOR_USE:
		if (g_vm->getGameID() == GType_BlueForce)
			SceneItem::display(USE_SCENE_HOTSPOT);
		else
			display(1, 5, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
		break;
	case CURSOR_TALK:
		if (g_vm->getGameID() == GType_BlueForce)
			SceneItem::display(TALK_SCENE_HOTSPOT);
		else
			display(1, 15, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
		break;
	case CURSOR_WALK:
		break;
	default:
		if (g_vm->getGameID() == GType_BlueForce)
			SceneItem::display(DEFAULT_SCENE_HOTSPOT);
		else
			display(2, action, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
		break;
	}
}

void SceneHotspot::setDetails(int ys, int xs, int ye, int xe, const int resnum, const int lookLineNum, const int useLineNum) {
	setBounds(ys, xe, ye, xs);
	_resNum = resnum;
	_lookLineNum = lookLineNum;
	_useLineNum = useLineNum;
	_talkLineNum = -1;
	g_globals->_sceneItems.addItems(this, NULL);
}

void SceneHotspot::setDetails(const Rect &bounds, int resNum, int lookLineNum, int talkLineNum, int useLineNum, int mode, SceneItem *item) {
	setBounds(bounds);
	_resNum = resNum;
	_lookLineNum = lookLineNum;
	_talkLineNum = talkLineNum;
	_useLineNum = useLineNum;

	switch (mode) {
	case 2:
		g_globals->_sceneItems.push_front(this);
		break;
	case 4:
		g_globals->_sceneItems.addBefore(item, this);
		break;
	case 5:
		g_globals->_sceneItems.addAfter(item, this);
		break;
	default:
		g_globals->_sceneItems.push_back(this);
		break;
	}
}

void SceneHotspot::setDetails(int sceneRegionId, int resNum, int lookLineNum, int talkLineNum, int useLineNum, int mode) {
	_sceneRegionId = sceneRegionId;
	_resNum = resNum;
	_lookLineNum = lookLineNum;
	_talkLineNum = talkLineNum;
	_useLineNum = useLineNum;

	// Handle adding hotspot to scene items list as necessary
	switch (mode) {
	case 2:
		GLOBALS._sceneItems.push_front(this);
		break;
	case 3:
		break;
	default:
		GLOBALS._sceneItems.push_back(this);
		break;
	}
}

void SceneHotspot::setDetails(int resNum, int lookLineNum, int talkLineNum, int useLineNum, int mode, SceneItem *item) {
	_resNum = resNum;
	_lookLineNum = lookLineNum;
	_talkLineNum = talkLineNum;
	_useLineNum = useLineNum;

	switch (mode) {
	case 2:
		g_globals->_sceneItems.push_front(this);
		break;
	case 4:
		g_globals->_sceneItems.addBefore(item, this);
		break;
	case 5:
		g_globals->_sceneItems.addAfter(item, this);
		break;
	default:
		g_globals->_sceneItems.push_back(this);
		break;
	}
}

void SceneHotspot::setDetails(int resNum, int lookLineNum, int talkLineNum, int useLineNum) {
	_resNum = resNum;
	_lookLineNum = lookLineNum;
	_talkLineNum = talkLineNum;
	_useLineNum = useLineNum;
}

/*--------------------------------------------------------------------------*/

void SceneObjectWrapper::setSceneObject(SceneObject *so) {
	_sceneObject = so;
	so->_strip = 1;
	so->_flags |= OBJFLAG_PANES;
}

void SceneObjectWrapper::synchronize(Serializer &s) {
	EventHandler::synchronize(s);
	SYNC_POINTER(_sceneObject);
}

void SceneObjectWrapper::remove() {
	delete this;
}

void SceneObjectWrapper::dispatch() {
	if (g_vm->getGameID() == GType_Ringworld)
		check();
}

void SceneObjectWrapper::check() {
	_visageImages.setVisage(_sceneObject->_visage);
	int visageCount = _visageImages.getFrameCount();
	int angle = _sceneObject->_angle;
	int strip = _sceneObject->_strip;

	if (visageCount == 4) {
		if ((angle > 314) || (angle < 45))
			strip = 4;
		if ((angle > 44) && (angle < 135))
			strip = 1;
		if ((angle >= 135) && (angle < 225))
			strip = 3;
		if ((angle >= 225) && (angle < 315))
			strip = 2;
	} else if (visageCount == 8) {
		if ((angle > 330) || (angle < 30))
			strip = 4;
		if ((angle >= 30) && (angle < 70))
			strip = 7;
		if ((angle >= 70) && (angle < 110))
			strip = 1;
		if ((angle >= 110) && (angle < 150))
			strip = 5;
		if ((angle >= 150) && (angle < 210))
			strip = 3;
		if ((angle >= 210) && (angle < 250))
			strip = 6;
		if ((angle >= 250) && (angle < 290))
			strip = 2;
		if ((angle >= 290) && (angle < 331))
			strip = 8;
	}

	if (strip > visageCount)
		strip = visageCount;

	_sceneObject->setStrip(strip);
}

/*--------------------------------------------------------------------------*/

SceneObject::SceneObject() : SceneHotspot() {
	_endAction = NULL;
	_mover = NULL;
	_objectWrapper = NULL;
	_flags = 0;
	_walkStartFrame = 0;
	_animateMode = ANIM_MODE_NONE;
	_updateStartFrame = 0;
	_moveDiff.x = 5;
	_moveDiff.y = 3;
	_numFrames = 10;
	_moveRate = 10;
	_regionBitList = 0;
	_sceneRegionId = 0;
	_percent = 100;
	_flags |= OBJFLAG_PANES;
	_priority = 0;

	_frameChange = 0;
	_visage = 0;
	_strip = 0;
	_frame = 0;
	_effect = 0;
	_shade = _shade2 = 0;
	_linkedActor = NULL;

	_field8A = Common::Point(0, 0);
}

SceneObject::SceneObject(const SceneObject &so) : SceneHotspot() {
	*this = so;
	if (_objectWrapper)
		// Create a fresh object wrapper for this object
		_objectWrapper = new SceneObjectWrapper();
}

SceneObject::~SceneObject() {
	delete _mover;
	delete _objectWrapper;
}

int SceneObject::getNewFrame() {
	int frameNum = _frame + _frameChange;

	if (_frameChange > 0) {
		if (frameNum > getFrameCount()) {
			frameNum = 1;
			if (_animateMode == ANIM_MODE_1)
				++frameNum;
		}
	} else if (frameNum < 1) {
		frameNum = getFrameCount();
	}

	return frameNum;
}

int SceneObject::getFrameCount() {
	_visageImages.setVisage(_visage, _strip);
	return _visageImages.getFrameCount();
}

void SceneObject::animEnded() {
	_animateMode = ANIM_MODE_NONE;
	if (_endAction)
		_endAction->signal();
}

int SceneObject::changeFrame() {
	int frameNum = _frame;
	uint32 mouseCtr = g_globals->_events.getFrameNumber();

	if ((_updateStartFrame <= mouseCtr) || (_animateMode == ANIM_MODE_1)) {
		if (_numFrames > 0) {
			int v = 60 / _numFrames;
			_updateStartFrame = mouseCtr + v;

			frameNum = getNewFrame();
		}
	}

	return frameNum;
}

void SceneObject::setPosition(const Common::Point &p, int yDiff) {
	_position = p;
	_yDiff = yDiff;
	_flags |= OBJFLAG_PANES;
}

void SceneObject::setZoom(int percent) {
	assert((percent >= -1) && (percent < 999));
	if (percent != _percent) {
		_percent = percent;
		_flags |= OBJFLAG_PANES;
	}
}

void SceneObject::updateZoom() {
	changeZoom(_percent);
}

void SceneObject::changeZoom(int percent) {
	if (percent == -1)
		_flags &= ~OBJFLAG_ZOOMED;
	else {
		_flags |= OBJFLAG_ZOOMED;
		setZoom(percent);
	}
}

void SceneObject::setStrip(int stripNum) {
	if (stripNum != _strip) {
		_strip = stripNum;
		_flags |= OBJFLAG_PANES;
	}
}

void SceneObject::setStrip2(int stripNum) {
	if (stripNum == -1)
		_flags &= ~OBJFLAG_SUPPRESS_DISPATCH;
	else {
		_flags |= OBJFLAG_SUPPRESS_DISPATCH;
		setStrip(stripNum);
	}
}

void SceneObject::setFrame(int frameNum) {
	if (frameNum != _frame) {
		_frame = frameNum;
		_flags |= OBJFLAG_PANES;
	}
}

void SceneObject::setFrame2(int frameNum) {
	if (frameNum != -1) {
		_flags |= OBJFLAG_NO_UPDATES;
		setFrame(frameNum);
	} else {
		_flags &= ~OBJFLAG_NO_UPDATES;
	}
}

void SceneObject::setPriority(int priority) {
	if (priority != _priority) {
		_priority = priority;
		_flags |= OBJFLAG_PANES;
	}
}

void SceneObject::fixPriority(int priority) {
	if (priority == -1) {
		_flags &= ~OBJFLAG_FIXED_PRIORITY;
	} else {
		_flags |= OBJFLAG_FIXED_PRIORITY;
		setPriority(priority);
	}
}

void SceneObject::setVisage(int visage) {
	if (visage != _visage) {
		_visage = visage;
		_flags |= OBJFLAG_PANES;
	}
}

void SceneObject::setObjectWrapper(SceneObjectWrapper *objWrapper) {
	if (_objectWrapper)
		_objectWrapper->remove();
	_objectWrapper = objWrapper;
	if (objWrapper)
		objWrapper->setSceneObject(this);
}

void SceneObject::addMover(ObjectMover *mover, ...) {
	if (_mover)
		_mover->remove();
	_mover = mover;

	if (mover) {
		// Set up the assigned mover
		_walkStartFrame = g_globals->_events.getFrameNumber();
		if (_moveRate != 0)
			_walkStartFrame = 60 / _moveRate;

		// Signal the mover that movement is beginning
		va_list va;
		va_start(va, mover);
		mover->startMove(this, va);
		va_end(va);
	}
}

void SceneObject::getHorizBounds() {
	Rect tempRect;

	GfxSurface frame = getFrame();
	tempRect.resize(frame, _position.x, _position.y - _yDiff, _percent);

	_xs = tempRect.left;
	_xe = tempRect.right;
}

int SceneObject::getRegionIndex() {
	return g_globals->_sceneRegions.indexOf(_position);
}

int SceneObject::checkRegion(const Common::Point &pt) {
	Rect tempRect;
	int regionIndex = 0;

	// Temporarily change the position
	Common::Point savedPos = _position;
	_position = pt;

	int regIndex = g_globals->_sceneRegions.indexOf(pt);
	if (_regionBitList & (1 << regIndex))
		regionIndex = regIndex;

	// Restore position
	_position = savedPos;

	// Get the object's frame bounds
	GfxSurface frame = getFrame();
	tempRect.resize(frame, _position.x, _position.y - _yDiff, _percent);

	int yPos, newY;
	if ((_position.y - _yDiff) <= (pt.y - _yDiff)) {
		yPos = _position.y - _yDiff;
		newY = pt.y;
	} else {
		yPos = pt.y - _yDiff;
		newY = _position.y;
	}
	newY -= _yDiff;

	SynchronizedList<SceneObject *>::iterator i;
	for (i = g_globals->_sceneObjects->begin(); (regionIndex == 0) && (i != g_globals->_sceneObjects->end()); ++i) {
		if ((*i) && ((*i)->_flags & OBJFLAG_CHECK_REGION)) {
			int objYDiff = (*i)->_position.y - _yDiff;
			if ((objYDiff >= yPos) && (objYDiff <= newY) &&
				((*i)->_xs < tempRect.right) && ((*i)->_xe > tempRect.left)) {
				// Found index
				regionIndex = (*i)->_regionIndex;
				break;
			}
		}
	}

	return regionIndex;
}

void SceneObject::animate(AnimateMode animMode, ...) {
	_animateMode = animMode;
	_updateStartFrame = g_globals->_events.getFrameNumber();
	if (_numFrames)
		_updateStartFrame += 60 / _numFrames;

	va_list va;
	va_start(va, animMode);

	switch (_animateMode) {
	case ANIM_MODE_NONE:
		_endAction = NULL;
		break;

	case ANIM_MODE_1:
		_frameChange = 1;
		_field2E = _position;
		_endAction = 0;
		break;

	case ANIM_MODE_2:
		_frameChange = 1;
		_endAction = NULL;
		break;

	case ANIM_MODE_3:
		_frameChange = -1;
		_endAction = NULL;
		break;

	case ANIM_MODE_4:
		_endFrame = va_arg(va, int);
		_frameChange = va_arg(va, int);
		_endAction = va_arg(va, Action *);
		if (_endFrame == _frame)
			setFrame(getNewFrame());
		break;

	case ANIM_MODE_5:
		_frameChange = 1;
		_endFrame = getFrameCount();
		_endAction = va_arg(va, Action *);
		if (_endFrame == _frame)
			setFrame(getNewFrame());
		break;

	case ANIM_MODE_6:
		_frameChange = -1;
		_endAction = va_arg(va, Action *);
		_endFrame = 1;
		if (_frame == _endFrame)
			setFrame(getNewFrame());
		break;

	case ANIM_MODE_7:
		_endFrame = va_arg(va, int);
		_endAction = va_arg(va, Action *);
		_frameChange = 1;
		break;

	case ANIM_MODE_8:
	case ANIM_MODE_9:
		_field68 = va_arg(va, int);
		_endAction = va_arg(va, Action *);
		_frameChange = 1;
		_endFrame = getFrameCount();
		if (_frame == _endFrame)
			setFrame(getNewFrame());
		break;
	}
}

SceneObject *SceneObject::clone() const {
	SceneObject *obj = new SceneObject(*this);
	return obj;
}

void SceneObject::copy(SceneObject *src) {
	*this = *src;

	_objectWrapper = NULL;
	_mover = NULL;
	_endAction = NULL;
}

void SceneObject::checkAngle(const SceneObject *obj) {
	checkAngle(obj->_position);
}

void SceneObject::checkAngle(const Common::Point &pt) {
	int angleAmount = GfxManager::getAngle(_position, pt);
	if (angleAmount != -1) {
		_angle = angleAmount;

		if (_animateMode == ANIM_MODE_9)
			_angle = (angleAmount + 180) % 360;
	}

	if (_objectWrapper && (g_vm->getGameID() == GType_Ringworld))
		_objectWrapper->dispatch();
}

void SceneObject::hide() {
	_flags |= OBJFLAG_HIDE;
	if (_flags & OBJFLAG_HIDING)
		_flags |= OBJFLAG_PANES;
}

void SceneObject::show() {
	if (_flags & OBJFLAG_HIDE) {
		_flags &= ~OBJFLAG_HIDE;
		_flags |= OBJFLAG_PANES;
	}
}

int SceneObject::getSpliceArea(const SceneObject *obj) {
	int xd = ABS(_position.x - obj->_position.x);
	int yd = ABS(_position.y - obj->_position.y);

	return (xd * xd + yd) / 2;
}

void SceneObject::synchronize(Serializer &s) {
	SceneHotspot::synchronize(s);

	s.syncAsUint32LE(_updateStartFrame);
	s.syncAsUint32LE(_walkStartFrame);
	s.syncAsSint16LE(_field2E.x); s.syncAsSint16LE(_field2E.y);
	s.syncAsSint16LE(_percent);
	s.syncAsSint16LE(_priority);
	s.syncAsSint16LE(_angle);
	s.syncAsUint32LE(_flags);
	s.syncAsSint16LE(_xs);
	s.syncAsSint16LE(_xe);
	_paneRects[0].synchronize(s);
	_paneRects[1].synchronize(s);
	s.syncAsSint32LE(_visage);
	SYNC_POINTER(_objectWrapper);
	s.syncAsSint32LE(_strip);
	SYNC_ENUM(_animateMode, AnimateMode);
	s.syncAsSint32LE(_frame);
	s.syncAsSint32LE(_endFrame);
	s.syncAsSint32LE(_field68);
	s.syncAsSint32LE(_frameChange);
	s.syncAsSint32LE(_numFrames);
	s.syncAsSint32LE(_regionIndex);
	SYNC_POINTER(_mover);
	s.syncAsSint16LE(_moveDiff.x); s.syncAsSint16LE(_moveDiff.y);
	s.syncAsSint32LE(_moveRate);
	if (g_vm->getGameID() == GType_Ringworld2) {
		s.syncAsSint16LE(_field8A.x);
		s.syncAsSint16LE(_field8A.y);
	}
	SYNC_POINTER(_endAction);
	s.syncAsUint32LE(_regionBitList);

	if (g_vm->getGameID() == GType_Ringworld2) {
		s.syncAsSint16LE(_effect);
		s.syncAsSint16LE(_shade);
		s.syncAsSint16LE(_shade2);
		SYNC_POINTER(_linkedActor);
	}
}

void SceneObject::postInit(SceneObjectList *OwnerList) {
	if (!OwnerList)
		OwnerList = g_globals->_sceneObjects;

	if (!OwnerList->contains(this)) {
		_percent = 100;
		_priority = 255;
		_flags = 4;
		_visage = 0;
		_strip = 1;
		_frame = 1;
		_objectWrapper = NULL;
		_animateMode = ANIM_MODE_NONE;
		_endAction = 0;
		_mover = NULL;
		_yDiff = 0;
		_moveDiff.x = 5;
		_moveDiff.y = 3;
		_moveRate = 10;
		_regionIndex = 0x40;
		_numFrames = 10;
		_regionBitList = 0;

		OwnerList->push_back(this);
		_flags |= OBJFLAG_PANES;
	}
}

void SceneObject::remove() {
	SceneItem::remove();
	if (g_globals->_sceneObjects->contains(this))
		// For objects in the object list, flag the object for removal in the next drawing, so that
		// the drawing code has a chance to restore the area previously covered by the object
		_flags |= OBJFLAG_PANES | OBJFLAG_REMOVE | OBJFLAG_HIDE;
	else
		// Not in the list, so immediately remove the object
		removeObject();
}

void SceneObject::dispatch() {
	if (g_vm->getGameID() == GType_Ringworld2) {
		if (_shade != _shade2)
			_flags |= OBJFLAG_PANES;
		_shade2 = _shade;
	}

	uint32 currTime = g_globals->_events.getFrameNumber();
	if (_action)
		_action->dispatch();

	if (_mover && (_walkStartFrame <= currTime)) {
		if (_moveRate) {
			int frameInc = 60 / _moveRate;
			_walkStartFrame = currTime + frameInc;
		}
		_mover->dispatch();
	}

	if (!(_flags & OBJFLAG_NO_UPDATES)) {
		switch (_animateMode) {
		case ANIM_MODE_1:
			if (isNoMover())
				setFrame(1);
			else if ((_field2E.x != _position.x) || (_field2E.y != _position.y)) {
				setFrame(changeFrame());
				_field2E = _position;

			}
			break;

		case ANIM_MODE_2:
		case ANIM_MODE_3:
			setFrame(changeFrame());

			break;
		case ANIM_MODE_4:
		case ANIM_MODE_5:
		case ANIM_MODE_6:
			if (_frame == _endFrame)
				animEnded();
			else
				setFrame(changeFrame());
			break;

		case ANIM_MODE_7:
			if (changeFrame() != _frame) {
				// Pick a new random frame
				int frameNum = 0;
				do {
					int count = getFrameCount();
					frameNum = g_globals->_randomSource.getRandomNumber(count - 1);
				} while (frameNum == _frame);

				setFrame(frameNum);
				if (_endFrame) {
					if (--_endFrame == 0)
						animEnded();
				}
			}
			break;

		case ANIM_MODE_8:
			if (_frame == _endFrame) {
				if (_frameChange != -1) {
					_frameChange = -1;
					_endFrame = 1;

					setFrame(changeFrame());
				} else if (!_field68 || (--_field68 > 0)) {
					_frameChange = 1;
					_endFrame = getFrameCount();

					setFrame(changeFrame());
				} else {
					animEnded();
				}
			} else {
				setFrame(changeFrame());
			}
			break;

		case ANIM_MODE_9:
			if (_frame == _endFrame) {
				if (_frameChange != -1) {
					_frameChange = -1;
					_strip = ((_strip - 1) ^ 1) + 1;
					_endFrame = 1;
				} else if ((_field68 == 0) || (--_field68 != 0)) {
					_frameChange = 1;
					_endFrame = getFrameCount();

					setFrame(changeFrame());
				} else {
					animEnded();
				}
			} else {
				setFrame(changeFrame());
			}
			break;

		default:
			break;
		}
	}

	// Handle updating the zoom and/or priority
	if (!(_flags & OBJFLAG_ZOOMED)) {
		int yp = CLIP((int)_position.y, 0, 255);
		setZoom(g_globals->_sceneManager._scene->_zoomPercents[yp]);
	}
	if (!(_flags & OBJFLAG_FIXED_PRIORITY)) {
		setPriority(_position.y);
	}

	if (g_vm->getGameID() == GType_Ringworld2) {
		if (_linkedActor) {
			_linkedActor->setPosition(_position);
			_linkedActor->setStrip(_strip);
			_linkedActor->setFrame(_frame);
		}

		if ((_effect == 1) && (getRegionIndex() < 11))
			_shade = 0;
	}
}

void SceneObject::calcAngle(const Common::Point &pt) {
	int newAngle = GfxManager::getAngle(_position, pt);
	if (newAngle != -1)
		_angle = newAngle;
}

void SceneObject::removeObject() {
	g_globals->_sceneItems.remove(this);
	g_globals->_sceneObjects->remove(this);

	if (_objectWrapper) {
		_objectWrapper->remove();
		_objectWrapper = NULL;
	}
	if (_mover) {
		_mover->remove();
		_mover = NULL;
	}
	if (_flags & OBJFLAG_CLONED)
		// Cloned temporary object, so delete it
		delete this;
}

GfxSurface SceneObject::getFrame() {
	_visageImages.setVisage(_visage, _strip);
	return _visageImages.getFrame(_frame);
}

void SceneObject::reposition() {
	GfxSurface frame = getFrame();
	_bounds.resize(frame, _position.x, _position.y - _yDiff, _percent);
	_xs = _bounds.left;
	_xe = _bounds.right;
}

/**
 * Draws an object into the scene
 */
void SceneObject::draw() {
	Rect destRect = _bounds;
	destRect.translate(-g_globals->_sceneManager._scene->_sceneBounds.left,
		-g_globals->_sceneManager._scene->_sceneBounds.top);
	Region *priorityRegion = g_globals->_sceneManager._scene->_priorities.find(_priority);
	GfxSurface frame = getFrame();
	g_globals->gfxManager().copyFrom(frame, destRect, priorityRegion);
}

/**
 * Refreshes the background around the area of a scene object prior to it's being redrawn,
 * in case it is moving
 */
void SceneObject::updateScreen() {
	Rect srcRect = _paneRects[CURRENT_PANENUM];
	const Rect &sceneBounds = g_globals->_sceneManager._scene->_sceneBounds;
	srcRect.left = (srcRect.left / 4) * 4;
	srcRect.right = ((srcRect.right + 3) / 4) * 4;
	srcRect.clip(g_globals->_sceneManager._scene->_sceneBounds);

	if (g_vm->getGameID() != GType_Ringworld) {
		if (T2_GLOBALS._uiElements._visible)
			srcRect.bottom = MIN<int16>(srcRect.bottom, T2_GLOBALS._interfaceY);
	}

	if (srcRect.isValidRect()) {
		Rect destRect  = srcRect;
		destRect.translate(-sceneBounds.left, -sceneBounds.top);
		srcRect.translate(-g_globals->_sceneOffset.x, -g_globals->_sceneOffset.y);

		g_globals->_screenSurface.copyFrom(g_globals->_sceneManager._scene->_backSurface, srcRect, destRect);
	}
}

void SceneObject::updateAngle(const Common::Point &pt) {
	checkAngle(pt);
	if (_objectWrapper)
		_objectWrapper->check();
}

void SceneObject::changeAngle(int angle) {
	_angle = angle;
	if (_objectWrapper)
		_objectWrapper->check();
}

void SceneObject::setup(int visage, int stripFrameNum, int frameNum, int posX, int posY, int priority) {
	postInit();
	setVisage(visage);
	setStrip(stripFrameNum);
	setFrame(frameNum);
	setPosition(Common::Point(posX, posY), 0);
	fixPriority(priority);
}

void SceneObject::setup(int visage, int stripFrameNum, int frameNum) {
	postInit();
	setVisage(visage);
	setStrip(stripFrameNum);
	setFrame(frameNum);
}

/*--------------------------------------------------------------------------*/

void BackgroundSceneObject::postInit(SceneObjectList *OwnerList) {
	SceneObjectList dummyList;
	SceneObjectList *pList = !g_globals->_sceneManager._scene ? &dummyList :
		&g_globals->_sceneManager._scene->_bgSceneObjects;

	SceneObject::postInit(pList);
}

void BackgroundSceneObject::draw() {
	assert(g_globals->_sceneManager._scene);
	Rect destRect = _bounds;
	destRect.translate(-g_globals->_sceneManager._scene->_sceneBounds.left,
		-g_globals->_sceneManager._scene->_sceneBounds.top);
	Region *priorityRegion = g_globals->_sceneManager._scene->_priorities.find(_priority);
	GfxSurface frame = getFrame();
	g_globals->_sceneManager._scene->_backSurface.copyFrom(frame, destRect, priorityRegion);
}

void BackgroundSceneObject::setup2(int visage, int stripFrameNum, int frameNum, int posX, int posY, int priority, int32 arg10) {
	warning("TODO: Implement properly BackgroundSceneObject::setup2()");
	postInit();
	setVisage(visage);
	setStrip(stripFrameNum);
	setFrame(frameNum);
	setPosition(Common::Point(posX, posY), 0);
	fixPriority(priority);
}

void BackgroundSceneObject::proc27() {
	warning("STUB: BackgroundSceneObject::proc27()");
}

/*--------------------------------------------------------------------------*/

void SceneObjectList::draw() {
	Common::Array<SceneObject *> objList;
	int paneNum = 0;
	int xAmount = 0, yAmount = 0;

	if (_objList.size() == 0) {
		// Alternate draw mode

		if (g_globals->_paneRefreshFlag[paneNum] == 1) {
			// Load the background
			g_globals->_sceneManager._scene->refreshBackground(0, 0);

			Rect tempRect = g_globals->_sceneManager._scene->_sceneBounds;
			tempRect.translate(-g_globals->_sceneOffset.x, -g_globals->_sceneOffset.y);
			ScenePalette::changeBackground(tempRect, g_globals->_sceneManager._fadeMode);
		} else {
			g_globals->_paneRegions[CURRENT_PANENUM].draw();
		}

		g_globals->_paneRegions[CURRENT_PANENUM].setRect(0, 0, 0, 0);
		g_globals->_sceneManager.fadeInIfNecessary();

	} else {
		// If there is a scroll follower, check whether it has moved off-screen
		if (g_globals->_scrollFollower) {
			const Rect &scrollerRect = g_globals->_sceneManager._scrollerRect;
			Common::Point objPos(
				g_globals->_scrollFollower->_position.x - g_globals->_sceneManager._scene->_sceneBounds.left,
				g_globals->_scrollFollower->_position.y - g_globals->_sceneManager._scene->_sceneBounds.top);
			int loadCount = 0;

			if (objPos.x >= scrollerRect.right) {
				xAmount = 8;
				loadCount = 20;
			}
			if (objPos.x < scrollerRect.left) {
				xAmount = -8;
				loadCount = 20;
			}
			if (objPos.y >= scrollerRect.bottom) {
				yAmount = 2;
				loadCount = 25;
			}
			if (objPos.y < scrollerRect.top) {
				yAmount = -2;
				loadCount = 25;
			}

			if (loadCount > 0)
				g_globals->_sceneManager.setBgOffset(Common::Point(xAmount, yAmount), loadCount);
		}

		if (g_globals->_sceneManager._sceneLoadCount > 0) {
			--g_globals->_sceneManager._sceneLoadCount;
			g_globals->_sceneManager._scene->loadBackground(g_globals->_sceneManager._sceneBgOffset.x,
				g_globals->_sceneManager._sceneBgOffset.y);
		}

		// Set up the flag mask
		uint32 flagMask = (paneNum == 0) ? OBJFLAG_PANE_0 : OBJFLAG_PANE_1;

		// Initial loop to set up object list and update object position, priority, and flags
		for (SynchronizedList<SceneObject *>::iterator i = g_globals->_sceneObjects->begin();
				i != g_globals->_sceneObjects->end(); ++i) {
			SceneObject *obj = *i;
			objList.push_back(obj);

			if (!(obj->_flags & OBJFLAG_HIDE))
				obj->_flags &= ~OBJFLAG_HIDING;

			// Reposition the bounds of the object to match the desired position
			obj->reposition();

			// Handle updating object priority
			if (!(obj->_flags & OBJFLAG_FIXED_PRIORITY)) {
				obj->_priority = MIN((int)obj->_position.y,
					(int)g_globals->_sceneManager._scene->_backgroundBounds.bottom - 1);
			}

			if ((g_globals->_paneRefreshFlag[paneNum] != 0) || !g_globals->_paneRegions[paneNum].empty()) {
				obj->_flags |= flagMask;
			}
		}

		// Check for any intersections, and then sort the object list by priority
		checkIntersection(objList, objList.size(), CURRENT_PANENUM);
		sortList(objList);

		if (g_globals->_paneRefreshFlag[paneNum] == 1) {
			// Load the background
			g_globals->_sceneManager._scene->refreshBackground(0, 0);
		}

		g_globals->_sceneManager._scene->_sceneBounds.left &= ~3;
		g_globals->_sceneManager._scene->_sceneBounds.right &= ~3;
		g_globals->_sceneOffset.x &= ~3;

		if (g_globals->_paneRefreshFlag[paneNum] != 0) {
			// Change the background
			Rect tempRect = g_globals->_sceneManager._scene->_sceneBounds;
			tempRect.translate(-g_globals->_sceneOffset.x, -g_globals->_sceneOffset.y);
			ScenePalette::changeBackground(tempRect, g_globals->_sceneManager._fadeMode);
		} else {
			for (uint objIndex = 0; objIndex < objList.size(); ++objIndex) {
				SceneObject *obj = objList[objIndex];

				if ((obj->_flags & flagMask) && obj->_paneRects[paneNum].isValidRect())
					obj->updateScreen();
			}

			g_globals->_paneRegions[paneNum].draw();
		}

		g_globals->_paneRegions[paneNum].setRect(0, 0, 0, 0);

		// FIXME: Currently, removing objects causes screen flickers when the removed object intersects
		// another drawn object, since the background is briefly redrawn over the object. For now, I'm
		// using a forced jump back to redraw objects. In the long term, I should figure out how the
		// original game does this properly
		bool redrawFlag = true;
		while (redrawFlag) {
			redrawFlag = false;

			// Main draw loop
			for (uint objIndex = 0; objIndex < objList.size(); ++objIndex) {
				SceneObject *obj = objList[objIndex];

				if ((obj->_flags & flagMask) && !(obj->_flags & OBJFLAG_HIDE)) {
					obj->_paneRects[paneNum] = obj->_bounds;
					obj->draw();
				}
			}

			// Update the palette
			g_globals->_sceneManager.fadeInIfNecessary();
			g_globals->_sceneManager._loadMode = 0;
			g_globals->_paneRefreshFlag[paneNum] = 0;

			// Loop through the object list, removing any objects and refreshing the screen as necessary
			for (uint objIndex = 0; objIndex < objList.size() && !redrawFlag; ++objIndex) {
				SceneObject *obj = objList[objIndex];

				if (obj->_flags & OBJFLAG_HIDE)
					obj->_flags |= OBJFLAG_HIDING;
				obj->_flags &= ~flagMask;
				if (obj->_flags & OBJFLAG_REMOVE) {
					obj->_flags |= OBJFLAG_PANES;

					checkIntersection(objList, objIndex, CURRENT_PANENUM);

					obj->updateScreen();
					obj->removeObject();

					objList.remove_at(objIndex);
					redrawFlag = true;
				}
			}
		}
	}
}

void SceneObjectList::checkIntersection(Common::Array<SceneObject *> &ObjList, uint ObjIndex, int PaneNum) {
	uint32 flagMask = (PaneNum == 0) ? OBJFLAG_PANE_0 : OBJFLAG_PANE_1;
	SceneObject *obj = (ObjIndex == ObjList.size()) ? NULL : ObjList[ObjIndex];
	Rect rect1;

	for (uint idx = 0; idx < ObjList.size(); ++idx) {
		SceneObject *currObj = ObjList[idx];

		if (ObjIndex == ObjList.size()) {
			if (currObj->_flags & flagMask)
				checkIntersection(ObjList, idx, PaneNum);
		} else if (idx != ObjIndex) {
			Rect &paneRect = obj->_paneRects[PaneNum];
			Rect objBounds = currObj->_bounds;
			if (paneRect.isValidRect())
				objBounds.extend(paneRect);

			Rect objBounds2 = currObj->_bounds;
			if (paneRect.isValidRect())
				objBounds2.extend(paneRect);

			objBounds.left &= ~3;
			objBounds.right += 3;
			objBounds.right &= ~3;
			objBounds2.left &= ~3;
			objBounds2.right += 3;
			objBounds2.right &= ~3;

			if (objBounds.intersects(objBounds2) && !(currObj->_flags & flagMask)) {
				currObj->_flags |= flagMask;
				checkIntersection(ObjList, idx, PaneNum);
			}
		}
	}
}

struct SceneObjectLess {
	bool operator()(const SceneObject *x, const SceneObject *y) const {
		if (y->_priority > x->_priority)
			return true;
		else if ((y->_priority == x->_priority) && (y->_position.y > x->_position.y))
			return true;
		else if ((y->_priority == x->_priority) && (y->_position.y == x->_position.y) &&
				 (y->_yDiff > x->_yDiff))
			return true;

		return false;
	}
};

void SceneObjectList::sortList(Common::Array<SceneObject *> &ObjList) {
	Common::sort(ObjList.begin(), ObjList.end(), SceneObjectLess());
}

void SceneObjectList::activate() {
	SceneObjectList *objectList = g_globals->_sceneObjects;
	g_globals->_sceneObjects = this;
	g_globals->_sceneObjects_queue.push_front(this);

	// Flag all the objects as modified
	SynchronizedList<SceneObject *>::iterator i;
	for (i = begin(); i != end(); ++i) {
		(*i)->_flags |= OBJFLAG_PANES;
	}

	// Replicate all existing objects on the old object list
	for (i = objectList->begin(); i != objectList->end(); ++i) {
		SceneObject *sceneObj = (*i)->clone();
		sceneObj->_flags |= OBJFLAG_HIDE | OBJFLAG_REMOVE | OBJFLAG_CLONED;
		push_front(sceneObj);
	}
}

void SceneObjectList::deactivate() {
	if (g_globals->_sceneObjects_queue.size() <= 1)
		return;

	SceneObjectList *objectList = *g_globals->_sceneObjects_queue.begin();
	g_globals->_sceneObjects_queue.pop_front();
	g_globals->_sceneObjects = *g_globals->_sceneObjects_queue.begin();

	SynchronizedList<SceneObject *>::iterator i;
	for (i = objectList->begin(); i != objectList->end(); ++i) {
		if (!((*i)->_flags & OBJFLAG_CLONED)) {
			SceneObject *sceneObj = (*i)->clone();
			sceneObj->_flags |= OBJFLAG_HIDE | OBJFLAG_REMOVE | OBJFLAG_CLONED;
			g_globals->_sceneObjects->push_front(sceneObj);
		}
	}
}

void SceneObjectList::synchronize(Serializer &s) {
	if (s.getVersion() >= 2)
		SavedObject::synchronize(s);
	_objList.synchronize(s);
}

/*--------------------------------------------------------------------------*/

SceneText::SceneText() : SceneObject() {
	_fontNumber = 2;
	_width = 160;
	_textMode = ALIGN_LEFT;
	_color1 = 0;
	_color2 = 0;
	_color3 = 0;
}

SceneText::~SceneText() {
}

void SceneText::setup(const Common::String &msg) {
	GfxManager gfxMan(_textSurface);
	gfxMan.activate();
	Rect textRect;

	if ((g_vm->getGameID() != GType_Ringworld) && g_globals->_sceneObjects->contains(this) &&
			(_flags & OBJFLAG_REMOVE)) {
		// Trying to setup a SceneText scheduled to be removed, so remove it now
		_bounds.expandPanes();
		this->removeObject();
		g_globals->_sceneObjects->remove(this);
	}

	gfxMan._font.setFontNumber(_fontNumber);
	gfxMan._font._colors.foreground = _color1;
	gfxMan._font._colors2.background = _color2;
	gfxMan._font._colors2.foreground = _color3;

	gfxMan.getStringBounds(msg.c_str(), textRect, _width);
	_bounds.setWidth(textRect.width());
	_bounds.setHeight(textRect.height());

	// Set up a new blank surface to hold the text
	_textSurface.create(textRect.width(), textRect.height());
	_textSurface._transColor = 0xff;
	_textSurface.fillRect(textRect, _textSurface._transColor);

	// Write the text to the surface
	gfxMan._bounds = textRect;
	gfxMan._font.writeLines(msg.c_str(), textRect, _textMode);

	// Do post-init, which adds this SceneText object to the scene
	postInit();
	gfxMan.deactivate();
}

void SceneText::synchronize(Serializer &s) {
	SceneObject::synchronize(s);

	s.syncAsSint16LE(_fontNumber);
	s.syncAsSint16LE(_width);
	s.syncAsSint16LE(_color1);
	s.syncAsSint16LE(_color2);
	s.syncAsSint16LE(_color3);
	SYNC_ENUM(_textMode, TextAlign);

	if (s.getVersion() >= 5)
		_textSurface.synchronize(s);
}

void SceneText::updateScreen() {
	// FIXME: Hack for Blue Force to handle not refreshing the screen if the user interface
	// has been re-activated after showing some scene text
	if ((g_vm->getGameID() == GType_Ringworld) || (_bounds.top < UI_INTERFACE_Y) ||
			!T2_GLOBALS._uiElements._visible)
		SceneObject::updateScreen();
}

/*--------------------------------------------------------------------------*/

Visage::Visage() {
	_resNum = -1;
	_rlbNum = -1;
	_data = NULL;
	_flipHoriz = false;
}

Visage::Visage(const Visage &v) {
	_resNum = v._resNum;
	_rlbNum = v._rlbNum;
	_data = v._data;
	if (_data)
		g_vm->_memoryManager.incLocks(_data);
}

Visage &Visage::operator=(const Visage &s) {
	_resNum = s._resNum;
	_rlbNum = s._rlbNum;
	_data = s._data;
	if (_data)
		g_vm->_memoryManager.incLocks(_data);

	return *this;
}

void Visage::setVisage(int resNum, int rlbNum) {
	if ((_resNum != resNum) || (_rlbNum != rlbNum)) {
		_resNum = resNum;
		_rlbNum = rlbNum;
		DEALLOCATE(_data);

		if (g_vm->getGameID() == GType_Ringworld) {
			// In Ringworld, we immediately get the data
			_data = g_resourceManager->getResource(RES_VISAGE, resNum, rlbNum);
		} else {
			// Games after Ringworld have an extra indirection via the visage index file
			byte *indexData = g_resourceManager->getResource(RES_VISAGE, resNum, 9999);
			if (rlbNum == 9999) {
				_data = indexData;
			} else {
				if (rlbNum == 0)
					rlbNum = 1;

				// Check how many slots there are
				uint16 count = READ_LE_UINT16(indexData);
				if (rlbNum > count)
					rlbNum = count;

				// Get the flags/rlbNum to use
				uint32 v = READ_LE_UINT32(indexData + (rlbNum - 1) * 4 + 2);
				int flags = v >> 30;

				if (flags & 3) {
					rlbNum = (int)(v & 0xff);
				}
				_flipHoriz = flags & 1;

				_data = g_resourceManager->getResource(RES_VISAGE, resNum, rlbNum);
			}
		}

		assert(_data);
	}
}

Visage::~Visage() {
	DEALLOCATE(_data);
}

GfxSurface Visage::getFrame(int frameNum) {
	int numFrames = READ_LE_UINT16(_data);
	if (frameNum > numFrames)
		frameNum = numFrames;
	if (frameNum > 0)
		--frameNum;

	int offset = READ_LE_UINT32(_data + 2 + frameNum * 4);
	byte *frameData = _data + offset;

	GfxSurface result = surfaceFromRes(frameData);
	if (_flipHoriz) flip(result);
	return result;
}

int Visage::getFrameCount() const {
	return READ_LE_UINT16(_data);
}

void Visage::flip(GfxSurface &gfxSurface) {
	Graphics::Surface s = gfxSurface.lockSurface();

	for (int y = 0; y < s.h; ++y) {
		// Flip the line
		byte *lineP = (byte *)s.getBasePtr(0, y);
		for (int x = 0; x < (s.w / 2); ++x)
			SWAP(lineP[x], lineP[s.w - x - 1]);
	}

	gfxSurface.unlockSurface();
}

/*--------------------------------------------------------------------------*/

Player::Player(): SceneObject() {
	_canWalk = false;
	_enabled = false;
	_uiEnabled = false;
	_field8C = 0;

	// Return to Ringworld specific fields
	_characterIndex = R2_NONE;

	for (int i = 0; i < MAX_CHARACTERS; ++i) {
		_characterScene[i] = 0;
		_characterStrip[i] = 0;
		_characterFrame[i] = 0;
		_oldCharacterScene[i] = 0;
	}
}

void Player::postInit(SceneObjectList *OwnerList) {
	SceneObject::postInit();

	_canWalk = true;
	_uiEnabled = true;
	_percent = 100;
	_field8C = 10;

	if  (g_vm->getGameID() != GType_Ringworld2)
	{
		_moveDiff.x = 4;
		_moveDiff.y = 2;
	}
	else
	{
		_moveDiff.x = 3;
		_moveDiff.y = 2;
		_effect = 1;
		_shade = 0;

		setObjectWrapper(new SceneObjectWrapper());
		setPosition(_characterPos[_characterIndex]);
		setStrip(_characterStrip[_characterIndex]);
		setFrame(_characterFrame[_characterIndex]);
		_characterScene[_characterIndex] = GLOBALS._sceneManager._sceneNumber;
	}
}

void Player::disableControl() {
	_canWalk = false;
	_uiEnabled = false;
	g_globals->_events.setCursor(CURSOR_NONE);
	_enabled = false;

	if ((g_vm->getGameID() != GType_Ringworld) && T2_GLOBALS._uiElements._active)
		T2_GLOBALS._uiElements.hide();
}

void Player::enableControl() {
	CursorType cursor;

	_canWalk = true;
	_uiEnabled = true;
	_enabled = true;

	switch (g_vm->getGameID()) {
	case GType_BlueForce:
	case GType_Ringworld2:
		cursor = g_globals->_events.getCursor();
		g_globals->_events.setCursor(cursor);

		if (T2_GLOBALS._uiElements._active)
			T2_GLOBALS._uiElements.show();
		break;

	default:
		// Ringworld
		g_globals->_events.setCursor(CURSOR_WALK);

		switch (g_globals->_events.getCursor()) {
		case CURSOR_WALK:
		case CURSOR_LOOK:
		case CURSOR_USE:
		case CURSOR_TALK:
			g_globals->_events.setCursor(g_globals->_events.getCursor());
			break;
		default:
			g_globals->_events.setCursor(CURSOR_WALK);
			break;
		}
		break;
	}
}

void Player::disableControl(CursorType cursorId, CursorType objectId) {
	if (cursorId != -1)
		R2_GLOBALS._events.setCursor(cursorId);
	else if (objectId != CURSOR_NONE)
		R2_GLOBALS._events.setCursor(objectId);

	disableControl();
}

void Player::enableControl(CursorType cursorId, CursorType objectId) {
	enableControl();

	if (cursorId != -1)
		R2_GLOBALS._events.setCursor(cursorId);
	else if (objectId != CURSOR_NONE)
		R2_GLOBALS._events.setCursor(objectId);
}

void Player::process(Event &event) {
	if ((g_vm->getGameID() != GType_Ringworld) && _action)
		_action->process(event);

	if (!event.handled && (event.eventType == EVENT_BUTTON_DOWN) &&
			(g_globals->_events.getCursor() == CURSOR_WALK) && g_globals->_player._canWalk &&
			(_position != event.mousePos) && g_globals->_sceneObjects->contains(this)) {

		if ((g_vm->getGameID() != GType_Ringworld) && !BF_GLOBALS._player._enabled)
			return;

		PlayerMover *newMover = new PlayerMover();
		Common::Point destPos(event.mousePos.x + g_globals->_sceneManager._scene->_sceneBounds.left,
			event.mousePos.y + g_globals->_sceneManager._scene->_sceneBounds.top);

		addMover(newMover, &destPos, NULL);
		event.handled = true;
	}
}

void Player::synchronize(Serializer &s) {
	SceneObject::synchronize(s);

	s.syncAsByte(_canWalk);
	s.syncAsByte(_uiEnabled);
	s.syncAsSint16LE(_field8C);

	if (g_vm->getGameID() != GType_Ringworld)
		s.syncAsByte(_enabled);

	if (g_vm->getGameID() == GType_Ringworld2) {
		s.syncAsSint16LE(_characterIndex);
		for (int i = 0; i < MAX_CHARACTERS; ++i) {
			s.syncAsSint16LE(_characterScene[i]);
			s.syncAsSint16LE(_oldCharacterScene[i]);
			s.syncAsSint16LE(_characterPos[i].x);
			s.syncAsSint16LE(_characterPos[i].y);
			s.syncAsSint16LE(_characterStrip[i]);
			s.syncAsSint16LE(_characterFrame[i]);
		}
	}
}

/*--------------------------------------------------------------------------*/

Region::Region(int resNum, int rlbNum, ResourceType ctlType) {
	_regionId = rlbNum;

	byte *regionData = g_resourceManager->getResource(ctlType, resNum, rlbNum);
	assert(regionData);

	load(regionData);

	DEALLOCATE(regionData);
}

Region::Region(int regionId, const byte *regionData) {
	_regionId = regionId;
	load(regionData);
}

void Region::load(const byte *regionData) {
	// Set the region bounds
	_bounds.top = READ_LE_UINT16(regionData + 6);
	_bounds.left = READ_LE_UINT16(regionData + 8);
	_bounds.bottom = READ_LE_UINT16(regionData + 10);
	_bounds.right = READ_LE_UINT16(regionData + 12);

	// Special handling for small size regions
	_regionSize = READ_LE_UINT16(regionData);
	if (_regionSize == 14)
		// No line slices
		return;

	// Set up the line slices
	for (int y = 0; y < (_regionSize == 22 ? 1 : _bounds.height()); ++y) {
		int slicesCount = READ_LE_UINT16(regionData + 16 + y * 4);
		int slicesOffset = READ_LE_UINT16(regionData + 14 + y * 4);
		assert(slicesCount < 100);
		LineSliceSet sliceSet;
		sliceSet.load(slicesCount, regionData + 14 + slicesOffset);

		_ySlices.push_back(sliceSet);
	}
}

/**
 * Returns true if the given region contains the specified point
 * @param pt	Specified position
 */
bool Region::contains(const Common::Point &pt) {
	// First check if the point falls inside the overall bounding rectangle
	if (!_bounds.contains(pt) || _ySlices.empty())
		return false;

	// Get the correct Y line to use
	const LineSliceSet &line = getLineSlices(pt.y);

	// Loop through the horizontal slice list to see if the point falls in one
	for (uint idx = 0; idx < line.items.size(); ++idx) {
		if ((pt.x >= line.items[idx].xs) && (pt.x < line.items[idx].xe))
			return true;
	}

	return false;
}

/**
 * Returns true if the given region is empty
 */
bool Region::empty() const {
	return !_bounds.isValidRect() && (_regionSize == 14);
}

void Region::clear() {
	_bounds.set(0, 0, 0, 0);
	_regionId = 0;
	_regionSize = 0;
}

void Region::setRect(const Rect &r) {
	setRect(r.left, r.top, r.right, r.bottom);
}

void Region::setRect(int xs, int ys, int xe, int ye) {
	bool validRect = (ys < ye) && (xs < xe);
	_ySlices.clear();

	if (!validRect) {
		_regionSize = 14;
		_bounds.set(0, 0, 0, 0);
	} else {
		_regionSize = 22;
		_bounds.set(xs, ys, xe, ye);

		LineSliceSet sliceSet;
		sliceSet.load2(1, xs, xe);

		_ySlices.push_back(sliceSet);
	}
}

const LineSliceSet &Region::getLineSlices(int yp) {
	return _ySlices[(_regionSize == 22) ? 0 : yp - _bounds.top];
}

LineSliceSet Region::sectPoints(int yp, const LineSliceSet &sliceSet) {
	if ((yp < _bounds.top) || (yp >= _bounds.bottom))
		return LineSliceSet();

	const LineSliceSet &ySet = getLineSlices(yp);
	return mergeSlices(sliceSet, ySet);
}

LineSliceSet Region::mergeSlices(const LineSliceSet &set1, const LineSliceSet &set2) {
	LineSliceSet result;

	uint set1Index = 0, set2Index = 0;

	while ((set1Index < set1.items.size()) && (set2Index < set2.items.size())) {
		if (set1.items[set1Index].xe <= set2.items[set2Index].xs) {
			++set1Index;
		} else if (set2.items[set2Index].xe <= set1.items[set1Index].xs) {
			++set2Index;
		} else {
			bool set1Flag = set1.items[set1Index].xs >= set2.items[set2Index].xs;
			const LineSlice &slice = set1Flag ? set1.items[set1Index] : set2.items[set2Index];

			result.add(slice.xs, MIN(set1.items[set1Index].xe, set2.items[set2Index].xe));
			if (set1Flag)
				++set1Index;
			else
				++set2Index;
		}
	}

	return result;
}

/**
 * Copies the background covered by the given region to the screen surface
 */
void Region::draw() {
	Rect &sceneBounds = g_globals->_sceneManager._scene->_sceneBounds;

	for (int yp = sceneBounds.top; yp < sceneBounds.bottom; ++yp) {
		// Generate a line slice set
		LineSliceSet tempSet;
		tempSet.add(sceneBounds.left, sceneBounds.right);
		LineSliceSet newSet = sectPoints(yp, tempSet);

		// Loop through the calculated slices
		for (uint idx = 0; idx < newSet.items.size(); ++idx) {
			Rect rect1(newSet.items[idx].xs, yp, newSet.items[idx].xe, yp + 1);
			rect1.left &= ~3;
			rect1.right = (rect1.right + 3) & ~3;

			Rect rect2 = rect1;
			rect1.translate(-g_globals->_sceneOffset.x, -g_globals->_sceneOffset.y);
			rect2.translate(-sceneBounds.left, -sceneBounds.top);

			g_globals->gfxManager().getSurface().copyFrom(g_globals->_sceneManager._scene->_backSurface,
				rect1, rect2);
		}
	}
}

void Region::uniteLine(int yp, LineSliceSet &sliceSet) {
	// First expand the bounds as necessary to fit in the row
	if (_ySlices.empty()) {
		_bounds = Rect(sliceSet.items[0].xs, yp, sliceSet.items[sliceSet.items.size() - 1].xe, yp + 1);
		_ySlices.push_back(LineSliceSet());
	}
	while (yp < _bounds.top) {
		_ySlices.insert_at(0, LineSliceSet());
		--_bounds.top;
	}
	while (yp >= _bounds.bottom) {
		_ySlices.push_back(LineSliceSet());
		++_bounds.bottom;
	}

	// Merge the existing line set into the line
	LineSliceSet &destSet = _ySlices[yp - _bounds.top];
	for (uint srcIndex = 0; srcIndex < sliceSet.items.size(); ++srcIndex) {
		LineSlice &srcSlice = sliceSet.items[srcIndex];

		// Check if overlaps existing slices
		uint destIndex = 0;
		while (destIndex < destSet.items.size()) {
			LineSlice &destSlice = destSet.items[destIndex];
			if (((srcSlice.xs >= destSlice.xs) && (srcSlice.xs <= destSlice.xe)) ||
				((srcSlice.xe >= destSlice.xs) && (srcSlice.xe <= destSlice.xe)) ||
				((srcSlice.xs < destSlice.xs) && (srcSlice.xe > destSlice.xe))) {
				// Intersecting, so merge them
				destSlice.xs = MIN(srcSlice.xs, destSlice.xs);
				destSlice.xe = MAX(srcSlice.xe, destSlice.xe);
				break;
			}
			++destIndex;
		}
		if (destIndex == destSet.items.size()) {
			// No intersecting region found, so add it to the list
			destSet.items.push_back(srcSlice);
		}
	}

	// Check whether to expand the left/bounds bounds
	if (destSet.items[0].xs < _bounds.left)
		_bounds.left = destSet.items[0].xs;
	if (destSet.items[destSet.items.size() - 1].xe > _bounds.right)
		_bounds.right = destSet.items[destSet.items.size() - 1].xe;
}

void Region::uniteRect(const Rect &rect) {
	for (int yp = rect.top; yp < rect.bottom; ++yp) {
		LineSliceSet sliceSet;
		sliceSet.add(rect.left, rect.right);
		uniteLine(yp, sliceSet);
	}
}

/*--------------------------------------------------------------------------*/

void SceneRegions::load(int sceneNum) {
	clear();
	bool altRegions = g_vm->getFeatures() & GF_ALT_REGIONS;
	byte *regionData = g_resourceManager->getResource(RES_CONTROL, sceneNum, altRegions ? 1 : 9999, true);

	if (regionData) {
		int regionCount = READ_LE_UINT16(regionData);
		for (int regionCtr = 0; regionCtr < regionCount; ++regionCtr) {
			int regionId = READ_LE_UINT16(regionData + regionCtr * 6 + 2);

			if (altRegions) {
				// Load data from within this resource
				uint32 dataOffset = READ_LE_UINT32(regionData + regionCtr * 6 + 4);
				push_back(Region(regionId, regionData + dataOffset));
			} else {
				// Load region from a separate resource
				push_back(Region(sceneNum, regionId));
			}
		}

		DEALLOCATE(regionData);
	}
}

int SceneRegions::indexOf(const Common::Point &pt) {
	for (SceneRegions::iterator i = begin(); i != end(); ++i) {
		if ((*i).contains(pt))
			return (*i)._regionId;
	}

	return 0;
}

/*--------------------------------------------------------------------------*/

void SceneItemList::addItems(SceneItem *first, ...) {
	va_list va;
	va_start(va, first);

	SceneItem *p = first;
	while (p) {
		push_back(p);
		p = va_arg(va, SceneItem *);
	}
}

/*--------------------------------------------------------------------------*/

RegionSupportRec WalkRegion::_processList[PROCESS_LIST_SIZE];

void RegionSupportRec::process() {
	if (_xDiff < _yDiff) {
		_halfDiff += _xDiff;
		if (_halfDiff > _yDiff) {
			_halfDiff -= _yDiff;
			_xp += _xDirection;
		}
	} else {
		do {
			_xp += _xDirection;
			_halfDiff += _yDiff;
		} while (_halfDiff <= _xDiff);
		_halfDiff -= _xDiff;
	}
	--_yDiff2;
}

/*--------------------------------------------------------------------------*/

void WalkRegion::loadRegion(byte *dataP, int size) {
	// First clear the region
	clear();

	// Decode the data for the region
	int dataCount, regionHeight;
	loadProcessList(dataP, size, dataCount, regionHeight);

	int processIndex = 0, idx2 = 0, count;
	for (int yp = _processList[0]._yp; yp < regionHeight; ++yp) {
		process3(yp, dataCount, processIndex, idx2);
		process4(yp, processIndex, idx2, count);

		loadRecords(yp, count, processIndex);
	}
}

void WalkRegion::loadProcessList(byte *dataP, int dataSize, int &dataIndex, int &regionHeight) {
	dataIndex = 0;
	int x1 = READ_LE_UINT16(dataP + (dataSize - 1) * 4);
	int y1 = READ_LE_UINT16(dataP + (dataSize - 1) * 4 + 2);
	regionHeight = y1;

	for (int idx = 0; idx < dataSize; ++idx) {
		int xp = READ_LE_UINT16(dataP + idx * 4);
		int yp = READ_LE_UINT16(dataP + idx * 4 + 2);
		if (yp != y1) {
			/*
			 * Commented out: v doesn't seem to be used
			int v;
			if (idx == (dataSize - 1))
				v = READ_LE_UINT16(dataP + 2);
			else
				v = process1(idx, dataP, dataSize);
			*/
			process2(dataIndex, x1, y1, xp, yp);
			++dataIndex;
		}

		// Keep regionHeight as the maximum of any y
		if (yp > regionHeight)
			regionHeight = yp;

		x1 = xp;
		y1 = yp;
	}
}

int WalkRegion::process1(int idx, byte *dataP, int dataSize) {
	int idx2 = idx + 1;
	if (idx2 == dataSize)
		idx2 = 0;

	while (READ_LE_UINT16(dataP + idx2 * 4 + 2) == READ_LE_UINT16(dataP + idx * 4 + 2)) {
		if (idx2 == (dataSize - 1))
			idx2 = 0;
		else
			++idx2;
	}

	return READ_LE_UINT16(dataP + idx2 * 4 + 2);
}

void WalkRegion::process2(int dataIndex, int x1, int y1, int x2, int y2) {
	int xDiff = ABS(x2 - x1);
	int yDiff = ABS(y2 - y1);
	int halfDiff = MAX(xDiff, yDiff) / 2;
	int yMax = MIN(y1, y2);

	while (dataIndex && (_processList[dataIndex - 1]._yp > yMax)) {
		_processList[dataIndex] = _processList[dataIndex - 1];
		--dataIndex;
	}
	_processList[dataIndex]._yp = yMax;

	_processList[dataIndex]._xp = (y1 >= y2) ? x2 : x1;
	_processList[dataIndex]._xDiff = xDiff;
	_processList[dataIndex]._yDiff = yDiff;
	_processList[dataIndex]._halfDiff = halfDiff;

	int xTemp = (y1 >= y2) ? x1 - x2 : x2 - x1;
	_processList[dataIndex]._xDirection = (xTemp == 0) ? 0 : ((xTemp < 0) ? -1 : 1);
	_processList[dataIndex]._yDiff2 = yDiff;
}

void WalkRegion::process3(int yp, int dataCount, int &idx1, int &idx2) {
	while ((idx2 < (dataCount - 1)) && (_processList[idx2 + 1]._yp <= yp))
		++idx2;
	while (!_processList[idx1]._yDiff2)
		++idx1;
}

void WalkRegion::process4(int yp, int idx1, int idx2, int &count) {
	count = 0;
	for (int idx = idx1; idx <= idx2; ++idx) {
		if (_processList[idx]._yDiff2 > 0)
			++count;
		process5(idx, idx1);
	}
}

void WalkRegion::process5(int idx1, int idx2) {
	while ((idx1 > idx2) && (_processList[idx1 - 1]._xp > _processList[idx1]._xp)) {
		SWAP(_processList[idx1], _processList[idx1 - 1]);
		--idx1;
	}
}

void WalkRegion::loadRecords(int yp, int size, int processIndex) {
	LineSliceSet sliceSet;
	int sliceCount =  size / 2;

	for (int idx = 0; idx < sliceCount; ++idx, ++processIndex) {
		while (!_processList[processIndex]._yDiff2)
			++processIndex;

		int sliceXs = _processList[processIndex]._xp;
		_processList[processIndex].process();

		do {
			++processIndex;
		} while (!_processList[processIndex]._yDiff2);

		int sliceXe = _processList[processIndex]._xp;
		_processList[processIndex].process();

		sliceSet.items.push_back(LineSlice(sliceXs, sliceXe));
	}

	uniteLine(yp, sliceSet);
}

/*--------------------------------------------------------------------------*/

void WRField18::load(byte *data) {
	_pt1.x = READ_LE_UINT16(data);
	_pt1.y = READ_LE_UINT16(data + 2);
	_pt2.x = READ_LE_UINT16(data + 4);
	_pt2.y = READ_LE_UINT16(data + 6);
	_v = READ_LE_UINT16(data + 8);
}

/*--------------------------------------------------------------------------*/

void WalkRegions::clear() {
	_regionList.clear();
	_field18.clear();
	_idxList.clear();
	_idxList2.clear();
	_disabledRegions.clear();
}

void WalkRegions::load(int sceneNum) {
	clear();
	_resNum = sceneNum;

	if (g_vm->getFeatures() & GF_ALT_REGIONS) {
		loadRevised();
	} else {
		loadOriginal();
	}
}

/**
 * This version handles loading walk regions for Ringworld floppy version and Demo #1
 */
void WalkRegions::loadOriginal() {
	byte *regionData = g_resourceManager->getResource(RES_WALKRGNS, _resNum, 1, true);
	if (!regionData) {
		// No data, so return
		_resNum = -1;
		return;
	}

	byte *dataP;
	int dataSize;

	// Load the field 18 list
	dataP = g_resourceManager->getResource(RES_WALKRGNS, _resNum, 2);
	dataSize = g_vm->_memoryManager.getSize(dataP);
	assert(dataSize % 10 == 0);

	byte *p = dataP;
	for (int idx = 0; idx < (dataSize / 10); ++idx, p += 10) {
		WRField18 rec;
		rec.load(p);
		_field18.push_back(rec);
	}

	DEALLOCATE(dataP);

	// Load the idx list
	dataP = g_resourceManager->getResource(RES_WALKRGNS, _resNum, 3);
	dataSize = g_vm->_memoryManager.getSize(dataP);
	assert(dataSize % 2 == 0);

	p = dataP;
	for (int idx = 0; idx < (dataSize / 2); ++idx, p += 2)
		_idxList.push_back(READ_LE_UINT16(p));

	DEALLOCATE(dataP);

	// Load the secondary idx list
	dataP = g_resourceManager->getResource(RES_WALKRGNS, _resNum, 4);
	dataSize = g_vm->_memoryManager.getSize(dataP);
	assert(dataSize % 2 == 0);

	p = dataP;
	for (int idx = 0; idx < (dataSize / 2); ++idx, p += 2)
		_idxList2.push_back(READ_LE_UINT16(p));

	DEALLOCATE(dataP);

	// Handle the loading of the actual regions themselves
	dataP = g_resourceManager->getResource(RES_WALKRGNS, _resNum, 5);

	byte *pWalkRegion = regionData + 16;
	byte *srcP = dataP;
	for (; (int16)READ_LE_UINT16(pWalkRegion) != -20000; pWalkRegion += 16) {
		WalkRegion wr;

		// Set the Walk region specific fields
		wr._pt.x = (int16)READ_LE_UINT16(pWalkRegion);
		wr._pt.y = (int16)READ_LE_UINT16(pWalkRegion + 2);
		wr._idxListIndex = READ_LE_UINT32(pWalkRegion + 4);
		wr._idxList2Index = READ_LE_UINT32(pWalkRegion + 8);

		// Read in the region data
		int size = READ_LE_UINT16(srcP);
		srcP += 2;
		wr.loadRegion(srcP, size);

		srcP += size * 4;
		_regionList.push_back(wr);
	}

	DEALLOCATE(dataP);
	DEALLOCATE(regionData);
}

/**
 * This version handles loading walk regions for Ringworld CD version and Demo #2. Given it's the newer
 * version, it may also be used by future game titles
 */
void WalkRegions::loadRevised() {
	byte *regionData = g_resourceManager->getResource(RES_WALKRGNS, _resNum, 2, true);
	if (!regionData) {
		// No data, so return
		_resNum = -1;
		return;
	}

	byte *data1P = regionData + READ_LE_UINT32(regionData);
	byte *data2P = regionData + READ_LE_UINT32(regionData + 4);
	byte *data3P = regionData + READ_LE_UINT32(regionData + 8);
	byte *data4P = regionData + READ_LE_UINT32(regionData + 12);
	byte *regionOffset = regionData + 16;
	int dataSize;

	// Load the field 18 list
	dataSize = READ_LE_UINT32(regionData + 8) - READ_LE_UINT32(regionData + 4);
	assert(dataSize % 10 == 0);

	byte *p = data2P;
	for (int idx = 0; idx < (dataSize / 10); ++idx, p += 10) {
		WRField18 rec;
		rec.load(p);
		_field18.push_back(rec);
	}

	// Load the idx list
	dataSize = READ_LE_UINT32(regionData + 12) - READ_LE_UINT32(regionData + 8);
	assert(dataSize % 2 == 0);

	p = data3P;
	for (int idx = 0; idx < (dataSize / 2); ++idx, p += 2)
		_idxList.push_back(READ_LE_UINT16(p));

	// Load the secondary idx list
	dataSize = READ_LE_UINT32(regionData + 16) - READ_LE_UINT32(regionData + 12);
	assert(dataSize % 2 == 0);

	p = data4P;
	for (int idx = 0; idx < (dataSize / 2); ++idx, p += 2)
		_idxList2.push_back(READ_LE_UINT16(p));

	// Handle the loading of the actual regions themselves
	byte *pWalkRegion = data1P + 16;
	for (; (int16)READ_LE_UINT16(pWalkRegion) != -20000; pWalkRegion += 16, regionOffset += 4) {
		WalkRegion wr;
		byte *srcP = regionData + READ_LE_UINT32(regionOffset);

		// Set the Walk region specific fields
		wr._pt.x = (int16)READ_LE_UINT16(pWalkRegion);
		wr._pt.y = (int16)READ_LE_UINT16(pWalkRegion + 2);
		wr._idxListIndex = READ_LE_UINT32(pWalkRegion + 4);
		wr._idxList2Index = READ_LE_UINT32(pWalkRegion + 8);

		// Read in the region data
		wr._regionId = 0;
		wr.load(srcP);

		_regionList.push_back(wr);
	}

	DEALLOCATE(regionData);
}

/**
 * Returns the index of the walk region that contains the given point
 * @param pt		Point to locate
 * @param indexList	List of region indexes that should be ignored
 */
int WalkRegions::indexOf(const Common::Point &pt, const Common::List<int> *indexList) {
	for (uint idx = 0; idx < _regionList.size(); ++idx) {
		if ((!indexList || !contains(*indexList, int(idx + 1))) && _regionList[idx].contains(pt))
			return idx + 1;
	}

	return -1;
}

void WalkRegions::synchronize(Serializer &s) {
	// Synchronise the list of disabled regions as a list of values terminated with a '-1'
	int regionId = 0;
	if (s.isLoading()) {
		_disabledRegions.clear();

		s.syncAsSint16LE(regionId);
		while (regionId != -1) {
			_disabledRegions.push_back(regionId);
			s.syncAsSint16LE(regionId);
		}
	} else {
		Common::List<int>::iterator i;
		for (i = _disabledRegions.begin(); i != _disabledRegions.end(); ++i) {
			regionId = *i;
			s.syncAsSint16LE(regionId);
		}

		regionId = -1;
		s.syncAsSint16LE(regionId);
	}
}

void WalkRegions::disableRegion(int regionId) {
	if (!contains(_disabledRegions, regionId))
		_disabledRegions.push_back(regionId);
}

void WalkRegions::enableRegion(int regionId) {
	_disabledRegions.remove(regionId);
}


/*--------------------------------------------------------------------------*/

void ScenePriorities::load(int resNum) {
	_resNum = resNum;
	clear();

	bool altMode = (g_vm->getFeatures() & GF_ALT_REGIONS) != 0;
	byte *regionData = g_resourceManager->getResource(RES_PRIORITY, resNum, altMode ? 1 : 9999, true);
	if (!regionData)
		return;

	int regionCount = READ_LE_UINT16(regionData);
	for (int regionCtr = 0; regionCtr < regionCount; ++regionCtr) {
		if (altMode) {
			// Region data is embedded within the resource
			uint16 regionId = READ_LE_UINT16(regionData + regionCtr * 6 + 2);
			uint32 dataOffset = READ_LE_UINT32(regionData + regionCtr * 6 + 4);
			push_back(Region(regionId, regionData + dataOffset));
		} else {
			// The data contains the index of another resource containing the region data
			int rlbNum = READ_LE_UINT16(regionData + regionCtr * 6 + 2);

			push_back(Region(resNum, rlbNum, RES_PRIORITY));
		}
	}

	DEALLOCATE(regionData);
}

Region *ScenePriorities::find(int priority) {
	// If no priority regions are loaded, then return the placeholder region
	if (empty()) {
		if (g_vm->getGameID() == GType_Ringworld)
			return &_defaultPriorityRegion;
		return NULL;
	}

	if (priority > 255)
		priority = 255;

	// Loop through the regions to find the closest for the given priority level
	int minRegionId = 9998;
	Region *region = NULL;
	for (ScenePriorities::iterator i = begin(); i != end(); ++i) {
		Region *r = &(*i);
		int regionId = r->_regionId;

		if ((regionId > priority) && (regionId < minRegionId)) {
			minRegionId = regionId;
			region = r;
		}
	}

	assert(region);
	return region;
}

/*--------------------------------------------------------------------------*/

void FloatSet::add(double v1, double v2, double v3) {
	_float1 += v1;
	_float2 += v2;
	_float3 += v3;
}

void FloatSet::proc1(double v) {
	double diff = (cos(v) * _float1) - (sin(v) * _float2);
	_float2 = (sin(v) * _float1) + (cos(v) * _float2);
	_float1 = diff;
}

double FloatSet::sqrt(FloatSet &floatSet) {
	double f1Diff = _float1 - floatSet._float1;
	double f2Diff = _float2 - floatSet._float2;
	double f3Diff = _float3 - floatSet._float3;

	return ::sqrt(f1Diff * f1Diff + f2Diff * f2Diff + f3Diff * f3Diff);
}

/*--------------------------------------------------------------------------*/

GameHandler::GameHandler() : EventHandler() {
	_nextWaitCtr = 1;
	_waitCtr.setCtr(1);
	_field14 = 10;
}

GameHandler::~GameHandler() {
	if (g_globals)
		g_globals->_game->removeHandler(this);
}

void GameHandler::execute() {
	if (_waitCtr.decCtr() == 0) {
		_waitCtr.setCtr(_nextWaitCtr);
		dispatch();
	}
}

void GameHandler::synchronize(Serializer &s) {
	if (s.getVersion() >= 2)
		EventHandler::synchronize(s);

	_lockCtr.synchronize(s);
	_waitCtr.synchronize(s);
	s.syncAsSint16LE(_nextWaitCtr);
	s.syncAsSint16LE(_field14);
}

/*--------------------------------------------------------------------------*/

SceneHandler::SceneHandler() {
	_saveGameSlot = -1;
	_loadGameSlot = -1;
	_prevFrameNumber = 0;
}

void SceneHandler::registerHandler() {
	postInit();
	g_globals->_game->addHandler(this);
}

uint32 SceneHandler::getFrameDifference() {
	return GLOBALS._events.getFrameNumber() - _prevFrameNumber;
}

void SceneHandler::postInit(SceneObjectList *OwnerList) {
	_delayTicks = 2;

	g_globals->_scenePalette.loadPalette(0);
	g_globals->_scenePalette.refresh();

	g_globals->_soundManager.postInit();
	g_globals->_soundManager.buildDriverList(true);
	g_globals->_soundManager.installConfigDrivers();

	g_globals->_game->start();
}

void SceneHandler::process(Event &event) {
	// Main keypress handler
	if (!event.handled) {
		g_globals->_game->processEvent(event);

		if (event.eventType == EVENT_KEYPRESS)
			g_globals->_events.setCursorFromFlag();
	}

	// Check for displaying right-click dialog
	if ((event.eventType == EVENT_BUTTON_DOWN) && (event.btnState == BTNSHIFT_RIGHT) &&
			g_globals->_player._uiEnabled &&
			((g_vm->getGameID() != GType_Ringworld2) || (R2_GLOBALS._sceneManager._sceneNumber != 1330))) {
		g_globals->_game->rightClick();

		event.handled = true;
		return;
	}

	// If there is an active scene, pass the event to it
	if (g_globals->_sceneManager._scene)
		g_globals->_sceneManager._scene->process(event);

	if (!event.handled) {
		// Separate check for F5 - Save key
		if ((event.eventType == EVENT_KEYPRESS) && (event.kbd.keycode == Common::KEYCODE_F5)) {
			// F5 - Save
			g_globals->_game->saveGame();
			event.handled = true;
			g_globals->_events.setCursorFromFlag();
		}

		// Check for debugger
		if ((event.eventType == EVENT_KEYPRESS) && (event.kbd.keycode == Common::KEYCODE_d) &&
			(event.kbd.flags & Common::KBD_CTRL)) {
			// Attach to the debugger
			g_vm->_debugger->attach();
			g_vm->_debugger->onFrame();
		}

		if ((event.eventType == EVENT_KEYPRESS) && g_globals->_player._enabled && g_globals->_player._canWalk) {
			// Keyboard shortcuts for different actions
			switch (event.kbd.keycode) {
			case Common::KEYCODE_w:
				g_globals->_events.setCursor(CURSOR_WALK);
				event.handled = true;
				break;
			case Common::KEYCODE_l:
				g_globals->_events.setCursor(CURSOR_LOOK);
				event.handled = true;
				break;
			case Common::KEYCODE_u:
				g_globals->_events.setCursor(CURSOR_USE);
				event.handled = true;
				break;
			case Common::KEYCODE_t:
				g_globals->_events.setCursor(CURSOR_TALK);
				event.handled = true;
				break;
			default:
				break;
			}
		}

		// Mouse press handling
		bool enabled = (g_vm->getGameID() != GType_Ringworld) ? g_globals->_player._enabled :
			g_globals->_player._uiEnabled;
		if (enabled && (event.eventType == EVENT_BUTTON_DOWN) && !g_globals->_sceneItems.empty()) {
			// Check if the mouse is on the player
			if (g_globals->_player.contains(event.mousePos)) {
				playerAction(event);
				if (event.handled)
					return;
			}

			// Scan the item list to find one the mouse is within
			SynchronizedList<SceneItem *>::iterator i;
			for (i = g_globals->_sceneItems.begin(); i != g_globals->_sceneItems.end(); ++i) {
				if ((*i)->contains(event.mousePos)) {
					// Pass the action to the item
					bool handled = (*i)->startAction(g_globals->_events.getCursor(), event);
					if (!handled)
						// Item wasn't handled, keep scanning
						continue;

					if ((g_vm->getGameID() == GType_Ringworld) || (g_globals->_events.getCursor() == CURSOR_9999)) {
						event.handled = g_globals->_events.getCursor() != CURSOR_WALK;

						if (g_globals->_player._uiEnabled && g_globals->_player._canWalk &&
								(g_globals->_events.getCursor() != CURSOR_LOOK)) {
							g_globals->_events.setCursor(CURSOR_WALK);
						} else if (g_globals->_player._canWalk && (g_globals->_events.getCursor() != CURSOR_LOOK)) {
							g_globals->_events.setCursor(CURSOR_WALK);
						} else if (g_globals->_player._uiEnabled && (g_globals->_events.getCursor() != CURSOR_LOOK)) {
							g_globals->_events.setCursor(CURSOR_USE);
						}

						if (g_vm->getGameID() != GType_Ringworld)
							event.handled = true;
					} else if (g_vm->getGameID() != GType_Ringworld) {
						event.handled = true;
					}
					break;
				}
			}

			// Handle any fallback text display
			processEnd(event);
		}

		// Handle player processing
		g_globals->_player.process(event);
	}
}

void SceneHandler::dispatch() {
	// Handle game saving and loading
	if (_saveGameSlot != -1) {
		int saveSlot = _saveGameSlot;
		_saveGameSlot = -1;
		Common::Error err = g_saver->save(saveSlot, _saveName);
		// FIXME: Make use of the description string in err to enhance
		// the error reported to the user.
		if (err.getCode() != Common::kNoError)
			GUIErrorMessage(SAVE_ERROR_MSG);
	}
	if (_loadGameSlot != -1) {
		int loadSlot = _loadGameSlot;
		_loadGameSlot = -1;
		g_saver->restore(loadSlot);
		g_globals->_events.setCursorFromFlag();
	}

	g_globals->_soundManager.dispatch();
	g_globals->_scenePalette.signalListeners();

	// Dispatch to any objects registered in the scene
	g_globals->_sceneObjects->recurse(SceneHandler::dispatchObject);

	// If a scene is active, then dispatch to it
	if (g_globals->_sceneManager._scene)
		g_globals->_sceneManager._scene->dispatch();

	// Not actually used
	//_eventListeners.forEach(SceneHandler::handleListener);

	// Handle pending events
	Event event;
	if (g_globals->_events.getEvent(event)) {
		// Process pending events
		do {
			process(event);
		} while (g_globals->_events.getEvent(event));
	} else if (g_vm->getGameID() != GType_Ringworld) {
		// For Blue Force, 'none' events need to be generated in the absence of any
		event.eventType = EVENT_NONE;
		event.mousePos = g_globals->_events._mousePos;
		process(event);
	}

	// Handle drawing the contents of the scene
	if ((g_vm->getGameID() != GType_Ringworld2) || (R2_GLOBALS._animationCtr == 0)) {
		if (g_globals->_sceneManager._scene)
			g_globals->_sceneObjects->draw();
	}

	// Check to see if any scene change is required
	g_globals->_sceneManager.checkScene();

	// Signal the ScummVM debugger
	g_vm->_debugger->onFrame();

	// Delay between frames
	g_globals->_events.delay(_delayTicks);
}

void SceneHandler::dispatchObject(EventHandler *obj) {
	obj->dispatch();
}

void SceneHandler::saveListener(Serializer &ser) {
}

} // End of namespace TsAGE
