/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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
#include "common/stream.h"

#include "pegasus/compass.h"
#include "pegasus/cursor.h"
#include "pegasus/energymonitor.h"
#include "pegasus/gamestate.h"
#include "pegasus/graphics.h"
#include "pegasus/input.h"
#include "pegasus/interaction.h"
#include "pegasus/interface.h"
#include "pegasus/pegasus.h"
#include "pegasus/ai/ai_area.h"
#include "pegasus/items/biochips/mapchip.h"
#include "pegasus/neighborhood/neighborhood.h"
#include "pegasus/neighborhood/tsa/fulltsa.h"
#include "pegasus/neighborhood/tsa/tinytsa.h"

namespace Pegasus {

StriderCallBack::StriderCallBack(Neighborhood *neighborhood) {
	_neighborhood = neighborhood;
}

void StriderCallBack::callBack() {
	_neighborhood->checkStriding();
}

static const TimeValue kStridingSlop = 39;

Neighborhood *g_neighborhood = 0;

Neighborhood::Neighborhood(InputHandler *nextHandler, PegasusEngine *vm, const Common::String &resName, NeighborhoodID id)
		: InputHandler(nextHandler), IDObject(id), _vm(vm), _resName(resName), _navMovie(kNavMovieID), _stridingCallBack(this),
		_neighborhoodNotification(kNeighborhoodNotificationID, (NotificationManager *)vm), _pushIn(kNoDisplayElement),
		_turnPush(kTurnPushID), _croppedMovie(kCroppedMovieID) {
	GameState.setOpenDoorLocation(kNoRoomID, kNoDirection);
	_currentAlternate = 0;
	_currentActivation = kActivateHotSpotAlways;
	_interruptionFilter = kFilterAllInput;
	allowInput(true);
	resetLastExtra();
	g_neighborhood = this;
	_currentInteraction = 0;
	_doneWithInteraction = false;
	_croppedMovie.setDisplayOrder(kCroppedMovieLayer);
}

Neighborhood::~Neighborhood() {
	for (HotspotIterator it = _neighborhoodHotspots.begin(); it != _neighborhoodHotspots.end(); it++)
		_vm->getAllHotspots().remove(*it);

	_neighborhoodHotspots.deleteHotspots();
	g_neighborhood = 0;

	loadLoopSound1("");
	loadLoopSound2("");
	newInteraction(kNoInteractionID);

	if (g_AIArea)
		g_AIArea->removeAllRules();
}

void Neighborhood::init() {
	_neighborhoodNotification.notifyMe(this, kNeighborhoodFlags, kNeighborhoodFlags);
	_navMovieCallBack.setNotification(&_neighborhoodNotification);
	_turnPushCallBack.setNotification(&_neighborhoodNotification);
	_delayCallBack.setNotification(&_neighborhoodNotification);
	_spotSoundCallBack.setNotification(&_neighborhoodNotification);

	debug(0, "Loading '%s' neighborhood resources", _resName.c_str());

	Common::SeekableReadStream *stream = _vm->_resFork->getResource(_doorTable.getResTag(), _resName);
	if (!stream)
		error("Failed to load doors");
	_doorTable.loadFromStream(stream);
	delete stream;

	stream = _vm->_resFork->getResource(_exitTable.getResTag(), _resName);
	if (!stream)
		error("Failed to load exits");
	_exitTable.loadFromStream(stream);
	delete stream;

	stream = _vm->_resFork->getResource(_extraTable.getResTag(), _resName);
	if (!stream)
		error("Failed to load extras");
	_extraTable.loadFromStream(stream);
	delete stream;

	stream = _vm->_resFork->getResource(_hotspotInfoTable.getResTag(), _resName);
	if (!stream)
		error("Failed to load hotspot info");
	_hotspotInfoTable.loadFromStream(stream);
	delete stream;

	stream = _vm->_resFork->getResource(_spotTable.getResTag(), _resName);
	if (!stream)
		error("Failed to load spots");
	_spotTable.loadFromStream(stream);
	delete stream;

	stream = _vm->_resFork->getResource(_turnTable.getResTag(), _resName);
	if (!stream)
		error("Failed to load turns");
	_turnTable.loadFromStream(stream);
	delete stream;

	stream = _vm->_resFork->getResource(_viewTable.getResTag(), _resName);
	if (!stream)
		error("Failed to load views");
	_viewTable.loadFromStream(stream);
	delete stream;

	stream = _vm->_resFork->getResource(_zoomTable.getResTag(), _resName);
	if (!stream)
		error("Failed to load zooms");
	_zoomTable.loadFromStream(stream);
	delete stream;

	createNeighborhoodSpots();

	_navMovie.initFromMovieFile(getNavMovieName());
	_navMovie.setVolume(_vm->getSoundFXLevel());

	Common::String soundSpotsName = getSoundSpotsName();
	if (soundSpotsName.empty()) {
		_spotSounds.disposeSound();
	} else {
		_spotSounds.initFromQuickTime(getSoundSpotsName());
		_spotSounds.setVolume(_vm->getSoundFXLevel());
	}

	_navMovie.setDisplayOrder(kNavMovieOrder);
	_navMovie.startDisplaying();

	Common::Rect bounds;
	_navMovie.getBounds(bounds);
	_pushIn.allocateSurface(bounds);

	_turnPush.setInAndOutElements(&_pushIn, &_navMovie);
	_turnPush.setDisplayOrder(kTurnPushOrder);
	_turnPush.startDisplaying();
	_navMovieCallBack.initCallBack(&_navMovie, kCallBackAtExtremes);
	_stridingCallBack.initCallBack(&_navMovie, kCallBackAtTime);
	_turnPushCallBack.initCallBack(&_turnPush, kCallBackAtExtremes);
	_delayCallBack.initCallBack(&_delayTimer, kCallBackAtExtremes);
	_spotSoundCallBack.initCallBack(&_spotSounds, kCallBackAtExtremes);

	setUpAIRules();

	if (g_compass)
		g_compass->setFaderValue(getStaticCompassAngle(GameState.getCurrentRoom(), GameState.getCurrentDirection()));

	_soundLoop1.attachFader(&_loop1Fader);
	_soundLoop2.attachFader(&_loop2Fader);
	startIdling();
}

void Neighborhood::start() {
	GameState.setCurrentRoom(GameState.getLastRoom());
	GameState.setCurrentDirection(GameState.getLastDirection());
	arriveAt(GameState.getNextRoom(), GameState.getNextDirection());
}

void Neighborhood::receiveNotification(Notification *, const NotificationFlags flags) {
	if ((flags & (kNeighborhoodMovieCompletedFlag | kTurnCompletedFlag)) != 0 && g_AIArea)
		g_AIArea->unlockAI();
	if (flags & kMoveForwardCompletedFlag)
		arriveAt(GameState.getNextRoom(), GameState.getNextDirection());
	if (flags & kTurnCompletedFlag)
		turnTo(GameState.getNextDirection());
	if (flags & kSpotCompletedFlag)
		spotCompleted();
	if (flags & kDoorOpenCompletedFlag)
		doorOpened();
	if (flags & kActionRequestCompletedFlag)
		popActionQueue();
	if (flags & kDeathExtraCompletedFlag)
		die(_extraDeathReason);
}

void Neighborhood::arriveAt(const RoomID room, const DirectionConstant direction) {
	if (g_map)
		g_map->moveToMapLocation(GameState.getCurrentNeighborhood(), room, direction);

	GameState.setCurrentNeighborhood(getObjectID());

	_currentActivation = kActivateHotSpotAlways;
	_interruptionFilter = kFilterAllInput;

	if (room != GameState.getCurrentRoom() || direction != GameState.getCurrentDirection()) {
		GameState.setCurrentRoom(room);
		GameState.setCurrentDirection(direction);
		loadAmbientLoops();
		activateCurrentView(room, direction, kSpotOnArrivalMask);
	} else {
		loadAmbientLoops();
		showViewFrame(getViewTime(GameState.getCurrentRoom(), GameState.getCurrentDirection()));
	}

	if (GameState.getOpenDoorRoom() != kNoRoomID) {
		// Arriving always closes a door.
		loadAmbientLoops();
		closeDoorOffScreen(GameState.getOpenDoorRoom(), GameState.getOpenDoorDirection());
		GameState.setOpenDoorLocation(kNoRoomID, kNoDirection);
	}

	if (g_compass)
		g_compass->setFaderValue(getStaticCompassAngle(GameState.getCurrentRoom(), GameState.getCurrentDirection()));

	if (g_AIArea)
		g_AIArea->checkMiddleArea();

	checkContinuePoint(room, direction);
}

// These functions can be overridden to tweak the exact frames used.

void Neighborhood::getExitEntry(const RoomID room, const DirectionConstant direction, ExitTable::Entry &entry) {
	entry = _exitTable.findEntry(room, direction, _currentAlternate);

	if (entry.isEmpty())
		entry = _exitTable.findEntry(room, direction, kNoAlternateID);
}

TimeValue Neighborhood::getViewTime(const RoomID room, const DirectionConstant direction) {
	if (GameState.getOpenDoorRoom() == room && GameState.getOpenDoorDirection() == direction) {
		// If we get here, the door entry for this location must exist.
		DoorTable::Entry doorEntry = _doorTable.findEntry(room, direction, _currentAlternate);

		if (doorEntry.isEmpty())
			doorEntry = _doorTable.findEntry(room, direction, kNoAlternateID);

		return doorEntry.movieEnd - 1;
	}

	ViewTable::Entry viewEntry = _viewTable.findEntry(room, direction, _currentAlternate);

	if (viewEntry.isEmpty())
		viewEntry = _viewTable.findEntry(room, direction, kNoAlternateID);

	return viewEntry.time;
}

void Neighborhood::getDoorEntry(const RoomID room, const DirectionConstant direction, DoorTable::Entry &doorEntry) {
	doorEntry = _doorTable.findEntry(room, direction, _currentAlternate);

	if (doorEntry.isEmpty())
		doorEntry = _doorTable.findEntry(room, direction, kNoAlternateID);
}

DirectionConstant Neighborhood::getTurnEntry(const RoomID room, const DirectionConstant direction, const TurnDirection turnDirection) {
	TurnTable::Entry turnEntry = _turnTable.findEntry(room, direction, turnDirection, _currentAlternate);

	if (turnEntry.isEmpty())
		turnEntry = _turnTable.findEntry(room, direction, turnDirection, kNoAlternateID);

	return turnEntry.endDirection;
}

void Neighborhood::findSpotEntry(const RoomID room, const DirectionConstant direction, SpotFlags flags, SpotTable::Entry &spotEntry) {
	spotEntry = _spotTable.findEntry(room, direction, flags, _currentAlternate);

	if (spotEntry.isEmpty())
		spotEntry = _spotTable.findEntry(room, direction, flags, kNoAlternateID);
}

void Neighborhood::getZoomEntry(const HotSpotID id, ZoomTable::Entry &zoomEntry) {
	zoomEntry = _zoomTable.findEntry(id);
}

void Neighborhood::getHotspotEntry(const HotSpotID id, HotspotInfoTable::Entry &hotspotEntry) {
	hotspotEntry = _hotspotInfoTable.findEntry(id);
}

void Neighborhood::getExtraEntry(const uint32 id, ExtraTable::Entry &extraEntry) {
	extraEntry = _extraTable.findEntry(id);
}

/////////////////////////////////////////////
//
// "Can" functions: Called to see whether or not a user is allowed to do something

CanMoveForwardReason Neighborhood::canMoveForward(ExitTable::Entry &entry) {
	DoorTable::Entry door;

	getExitEntry(GameState.getCurrentRoom(), GameState.getCurrentDirection(), entry);
	getDoorEntry(GameState.getCurrentRoom(), GameState.getCurrentDirection(), door);

	// Fixed this so that doors that don't lead anywhere can be opened, but not walked
	// through.
	if (door.flags & kDoorPresentMask) {
		if (GameState.isCurrentDoorOpen()) {
			if (entry.exitRoom == kNoRoomID)
				return kCantMoveBlocked;
			else
				return kCanMoveForward;
		} else if (door.flags & kDoorLockedMask) {
			return kCantMoveDoorLocked;
		} else {
			return kCantMoveDoorClosed;
		}
	} else if (entry.exitRoom == kNoRoomID) {
		return kCantMoveBlocked;
	}

	return kCanMoveForward;
}

CanTurnReason Neighborhood::canTurn(TurnDirection turnDirection, DirectionConstant &nextDir) {
	nextDir = getTurnEntry(GameState.getCurrentRoom(), GameState.getCurrentDirection(), turnDirection);

	if (nextDir == kNoDirection)
		return kCantTurnNoTurn;

	return kCanTurn;
}

CanOpenDoorReason Neighborhood::canOpenDoor(DoorTable::Entry &entry) {
	getDoorEntry(GameState.getCurrentRoom(), GameState.getCurrentDirection(), entry);

	if (entry.flags & kDoorPresentMask) {
		if (GameState.isCurrentDoorOpen())
			return kCantOpenAlreadyOpen;

		if (entry.flags & kDoorLockedMask)
			return kCantOpenLocked;

		return kCanOpenDoor;
	}

	return kCantOpenNoDoor;
}

void Neighborhood::createNeighborhoodSpots() {
	Common::SeekableReadStream *hotspotList = _vm->_resFork->getResource(MKTAG('H', 'S', 'L', 's'), _resName);
	if (!hotspotList)
		error("Could not load neighborhood hotspots");

	uint32 hotspotCount = hotspotList->readUint32BE();

	while (hotspotCount--) {
		uint16 id = hotspotList->readUint16BE();
		uint32 flags = hotspotList->readUint32BE();
		uint32 rgnSize = hotspotList->readUint32BE();

		int32 startPos = hotspotList->pos();

		debug(0, "Hotspot %d:", id);
		Region region(hotspotList);

		hotspotList->seek(startPos + rgnSize);

		Hotspot *hotspot = new Hotspot(id);
		hotspot->setHotspotFlags(flags);
		hotspot->setArea(region);

		_vm->getAllHotspots().push_back(hotspot);
		_neighborhoodHotspots.push_back(hotspot);
	}

	delete hotspotList;
}

void Neighborhood::popActionQueue() {
	if (!_actionQueue.empty()) {
		QueueRequest topRequest = _actionQueue.pop();

		switch (topRequest.requestType) {
		case kNavExtraRequest:
			_navMovie.stop();
			break;
		case kSpotSoundRequest:
			_spotSounds.stopSound();
			break;
		case kDelayRequest:
			_delayTimer.stop();
			break;
		}

		serviceActionQueue();
	}
}

void Neighborhood::serviceActionQueue() {
	if (!_actionQueue.empty()) {
		QueueRequest &topRequest = _actionQueue.front();

		if (!topRequest.playing) {
			topRequest.playing = true;
			switch (topRequest.requestType) {
			case kNavExtraRequest:
				startExtraSequence(topRequest.extra, topRequest.flags, topRequest.interruptionFilter);
				break;
			case kSpotSoundRequest:
				_spotSounds.stopSound();
				_spotSounds.playSoundSegment(topRequest.start, topRequest.stop);
				_interruptionFilter = topRequest.interruptionFilter;
				_spotSoundCallBack.setCallBackFlag(topRequest.flags);
				_spotSoundCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
				break;
			case kDelayRequest:
				_delayTimer.stop();
				_delayCallBack.setCallBackFlag(topRequest.flags);
				_delayTimer.setSegment(0, topRequest.start, topRequest.stop);
				_delayTimer.setTime(0);
				_delayCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
				_interruptionFilter = topRequest.interruptionFilter;
				_delayTimer.start();
				break;
			}
		}
	} else {
		_interruptionFilter = kFilterAllInput;
	}
}

void Neighborhood::requestAction(const QueueRequestType requestType, const ExtraID extra, const TimeValue in, const TimeValue out,
		const InputBits interruptionFilter, const NotificationFlags flags) {

	QueueRequest request;

	request.requestType = requestType;
	request.extra = extra;
	request.start = in;
	request.stop = out;
	request.interruptionFilter = interruptionFilter;
	request.playing = false;
	request.flags = flags | kActionRequestCompletedFlag;
	request.notification = &_neighborhoodNotification;
	_actionQueue.push(request);
	if (_actionQueue.size() == 1)
		serviceActionQueue();
}

void Neighborhood::requestExtraSequence(const ExtraID whichExtra, const NotificationFlags flags, const InputBits interruptionFilter) {
	requestAction(kNavExtraRequest, whichExtra, 0, 0, interruptionFilter, flags);
}

void Neighborhood::requestSpotSound(const TimeValue in, const TimeValue out, const InputBits interruptionFilter, const NotificationFlags flags) {
	requestAction(kSpotSoundRequest, 0xFFFFFFFF, in, out, interruptionFilter, flags);
}

void Neighborhood::playSpotSoundSync(const TimeValue in, const TimeValue out) {
	// Let the action queue play out first...
	while (!actionQueueEmpty()) {
		_vm->checkCallBacks();
		_vm->refreshDisplay();
		_vm->checkNotifications();
		_vm->_system->delayMillis(10);
	}

	_spotSounds.stopSound();
	_spotSounds.playSoundSegment(in, out);

	while (_spotSounds.isPlaying()) {
		_vm->checkCallBacks();
		_vm->refreshDisplay();
		_vm->_system->delayMillis(10);
	}
}

void Neighborhood::requestDelay(const TimeValue delayDuration, const TimeScale delayScale, const InputBits interruptionFilter, const NotificationFlags flags) {
	requestAction(kDelayRequest, 0xFFFFFFFF, delayDuration, delayScale, interruptionFilter, flags);
}

bool operator==(const QueueRequest &arg1, const QueueRequest &arg2) {
	return arg1.requestType == arg2.requestType && arg1.extra == arg2.extra &&
			arg1.start == arg2.start && arg1.stop == arg2.stop;
}

bool operator!=(const QueueRequest &arg1, const QueueRequest &arg2) {
	return !operator==(arg1, arg2);
}

Common::String Neighborhood::getBriefingMovie() {
	if (_currentInteraction)
		return _currentInteraction->getBriefingMovie();

	return Common::String();
}

Common::String Neighborhood::getEnvScanMovie() {
	if (_currentInteraction)
		return _currentInteraction->getEnvScanMovie();

	return Common::String();
}

uint Neighborhood::getNumHints() {
	if (_currentInteraction)
		return _currentInteraction->getNumHints();

	return 0;
}

Common::String Neighborhood::getHintMovie(uint hintNum) {
	if (_currentInteraction)
		return _currentInteraction->getHintMovie(hintNum);

	return Common::String();
}

bool Neighborhood::canSolve() {
	if (_currentInteraction)
		return _currentInteraction->canSolve();

	return false;
}

void Neighborhood::doSolve() {
	if (_currentInteraction)
		_currentInteraction->doSolve();
}

bool Neighborhood::okayToJump() {
	return !_vm->playerHasItemID(kGasCanister) && !_vm->playerHasItemID(kMachineGun);
}

AirQuality Neighborhood::getAirQuality(const RoomID) {
	return kAirQualityGood;
}

void Neighborhood::checkStriding() {
	if (stillMoveForward()) {
		ExitTable::Entry nextExit;
		getExitEntry(GameState.getNextRoom(), GameState.getNextDirection(), nextExit);
		keepStriding(nextExit);
	} else {
		stopStriding();
	}
}

bool Neighborhood::stillMoveForward() {
	Input input;

	InputHandler::readInputDevice(input);
	return input.upButtonAnyDown();
}

void Neighborhood::keepStriding(ExitTable::Entry &nextExitEntry) {
	FaderMoveSpec compassMove;

	if (g_map)
		g_map->moveToMapLocation(GameState.getCurrentNeighborhood(), GameState.getNextRoom(), GameState.getNextDirection());

	if (g_compass)
		getExitCompassMove(nextExitEntry, compassMove);

	GameState.setCurrentRoom(GameState.getNextRoom());
	GameState.setCurrentDirection(GameState.getNextDirection());
	GameState.setNextRoom(nextExitEntry.exitRoom);
	GameState.setNextDirection(nextExitEntry.exitDirection);

	if (nextExitEntry.movieEnd == nextExitEntry.exitEnd)
		scheduleNavCallBack(kNeighborhoodMovieCompletedFlag | kMoveForwardCompletedFlag);
	else
		scheduleStridingCallBack(nextExitEntry.movieEnd - kStridingSlop, kStrideCompletedFlag);

	if (g_compass)
		g_compass->startFader(compassMove);
}

void Neighborhood::stopStriding() {
	_navMovie.stop();
	_neighborhoodNotification.setNotificationFlags(kNeighborhoodMovieCompletedFlag |
			kMoveForwardCompletedFlag, kNeighborhoodMovieCompletedFlag | kMoveForwardCompletedFlag);
}

// Compass support
int16 Neighborhood::getStaticCompassAngle(const RoomID, const DirectionConstant dir) {
	// North, south, east, west
	static const int16 compassAngles[] = { 0, 180, 90, 270 };
	return compassAngles[dir];
}

void Neighborhood::getExitCompassMove(const ExitTable::Entry &exitEntry, FaderMoveSpec &compassMove) {
	int32 startAngle = getStaticCompassAngle(exitEntry.room, exitEntry.direction);
	int32 stopAngle = getStaticCompassAngle(exitEntry.exitRoom, exitEntry.exitDirection);

	if (startAngle > stopAngle) {
		if (stopAngle + 180 < startAngle)
			stopAngle += 360;
	} else {
		if (startAngle + 180 < stopAngle)
			startAngle += 360;
	}

	compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), exitEntry.movieStart, startAngle, exitEntry.movieEnd, stopAngle);
}

void Neighborhood::scheduleNavCallBack(NotificationFlags flags) {
	_navMovieCallBack.cancelCallBack();

	if (flags != 0) {
		_navMovieCallBack.setCallBackFlag(flags);
		_navMovieCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
	}
}

void Neighborhood::scheduleStridingCallBack(const TimeValue strideStop, NotificationFlags flags) {
	_stridingCallBack.cancelCallBack();

	if (flags != 0)
		_stridingCallBack.scheduleCallBack(kTriggerTimeFwd, strideStop, _navMovie.getScale());
}

void Neighborhood::moveNavTo(const CoordType h, const CoordType v) {
	CoordType oldH, oldV;
	_navMovie.getLocation(oldH, oldV);

	CoordType offH = h - oldH;
	CoordType offV = v - oldV;

	_navMovie.moveElementTo(h, v);
	_turnPush.moveElementTo(h, v);

	if (offH != 0 || offV != 0)
		for (HotspotList::iterator it = _neighborhoodHotspots.begin(); it != _neighborhoodHotspots.end(); it++)
			if ((*it)->getHotspotFlags() & kNeighborhoodSpotFlag)
				(*it)->moveSpot(offH, offV);
}

void Neighborhood::activateHotspots() {
	InputHandler::activateHotspots();

	for (HotspotInfoTable::iterator it = _hotspotInfoTable.begin(); it != _hotspotInfoTable.end(); it++) {
		HotspotInfoTable::Entry entry = *it;

		if (entry.hotspotRoom == GameState.getCurrentRoom() && entry.hotspotDirection == GameState.getCurrentDirection()
				&& (entry.hotspotActivation == _currentActivation || entry.hotspotActivation == kActivateHotSpotAlways)) {
			Hotspot *hotspot = _vm->getAllHotspots().findHotspotByID(entry.hotspot);
			if (hotspot)
				activateOneHotspot(entry, hotspot);
		}
	}
}

void Neighborhood::clickInHotspot(const Input &input, const Hotspot *clickedSpot) {
	HotSpotFlags flags = clickedSpot->getHotspotFlags();

	if ((flags & (kPickUpItemSpotFlag | kPickUpBiochipSpotFlag)) != 0) {
		ItemID itemID = kNoItemID;

		for (HotspotInfoTable::iterator it = _hotspotInfoTable.begin(); it != _hotspotInfoTable.end(); it++) {
			if (it->hotspot == clickedSpot->getObjectID()) {
				itemID = it->hotspotItem;
				break;
			}
		}

		if (itemID != kNoItemID) {
			Item *draggingItem = _vm->getAllItems().findItemByID(itemID);

			if (draggingItem) {
				takeItemFromRoom(draggingItem);

				if ((flags & kPickUpItemSpotFlag) != 0)
					_vm->dragItem(input, draggingItem, kDragInventoryPickup);
				else
					_vm->dragItem(input, draggingItem, kDragBiochipPickup);
			}
		}
	} else {
		// Check other flags here?
		if ((flags & kZoomSpotFlags) != 0) {
			zoomTo(clickedSpot);
		} else if ((flags & kPlayExtraSpotFlag) != 0) {
			HotspotInfoTable::Entry hotspotEntry;
			getHotspotEntry(clickedSpot->getObjectID(), hotspotEntry);
			startExtraSequence(hotspotEntry.hotspotExtra, kExtraCompletedFlag, kFilterNoInput);
		} else if ((flags & kOpenDoorSpotFlag) != 0) {
			openDoor();
		} else {
			InputHandler::clickInHotspot(input, clickedSpot);
		}
	}
}

void Neighborhood::cantMoveThatWay(CanMoveForwardReason reason) {
	switch (reason) {
	case kCantMoveDoorClosed:
	case kCantMoveDoorLocked:
		openDoor();
		break;
	case kCantMoveBlocked:
		zoomUpOrBump();
		break;
	default:
		bumpIntoWall();
		break;
	}
}

void Neighborhood::cantOpenDoor(CanOpenDoorReason) {
	bumpIntoWall();
}

void Neighborhood::turnTo(const DirectionConstant direction) {
	if (g_map)
		g_map->moveToMapLocation(GameState.getCurrentNeighborhood(), GameState.getCurrentRoom(), direction);

	// clone2727 says: Is this necessary?
	_vm->_gfx->setCurSurface(_navMovie.getSurface());
	_pushIn.copyToCurrentPort();
	_vm->_gfx->setCurSurface(_vm->_gfx->getWorkArea());

	// Added 2/10/97. Shouldn't this be here? Shouldn't we set the current activation to
	// always when turning to a new view?
	_currentActivation = kActivateHotSpotAlways;

	_interruptionFilter = kFilterAllInput;

	if (direction != GameState.getCurrentDirection()) {
		GameState.setCurrentDirection(direction);
		activateCurrentView(GameState.getCurrentRoom(), direction, kSpotOnTurnMask);
	} else {
		showViewFrame(getViewTime(GameState.getCurrentRoom(), GameState.getCurrentDirection()));
	}

	if (GameState.getOpenDoorRoom() != kNoRoomID) {
		// Turning always closes a door.
		loadAmbientLoops();
		closeDoorOffScreen(GameState.getOpenDoorRoom(), GameState.getOpenDoorDirection());
		GameState.setOpenDoorLocation(kNoRoomID, kNoDirection);
	}

	if (g_AIArea)
		g_AIArea->checkMiddleArea();

	checkContinuePoint(GameState.getCurrentRoom(), direction);

	_vm->_cursor->hideUntilMoved();
}

void Neighborhood::spotCompleted() {
	_interruptionFilter = kFilterAllInput;
	showViewFrame(getViewTime(GameState.getCurrentRoom(), GameState.getCurrentDirection()));
}

void Neighborhood::doorOpened() {
	_interruptionFilter = kFilterAllInput;

	// 2/23/97
	// Fixes funny bug with doors that are opened by dropping things on them...
	setCurrentActivation(kActivateHotSpotAlways);

	GameState.setOpenDoorLocation(GameState.getCurrentRoom(), GameState.getCurrentDirection());

	SpotTable::Entry entry;
	findSpotEntry(GameState.getCurrentRoom(), GameState.getCurrentDirection(), kSpotOnDoorOpenMask, entry);

	if (entry.dstFlags & kSpotOnDoorOpenMask) {
		startSpotOnceOnly(entry.movieStart, entry.movieEnd);
	} else {
		findSpotEntry(GameState.getCurrentRoom(), GameState.getCurrentDirection(), kSpotOnDoorOpenMask | kSpotLoopsMask, entry);

		if (entry.dstFlags & kSpotOnDoorOpenMask)
			startSpotLoop(entry.movieStart, entry.movieEnd);
	}

	loadAmbientLoops();

	if (g_map)
		g_map->moveToMapLocation(GameState.getCurrentNeighborhood(), GameState.getNextRoom(), GameState.getNextDirection());

	if (g_AIArea)
		g_AIArea->checkMiddleArea();
}

void Neighborhood::moveForward() {
	ExitTable::Entry exitEntry;
	CanMoveForwardReason moveReason = canMoveForward(exitEntry);

	if (moveReason == kCanMoveForward)
		startExitMovie(exitEntry);
	else
		cantMoveThatWay(moveReason);
}

void Neighborhood::turn(const TurnDirection turnDirection) {
	DirectionConstant nextDir;
	CanTurnReason turnReason = canTurn(turnDirection, nextDir);

	if (turnReason == kCanTurn)
		startTurnPush(turnDirection, getViewTime(GameState.getCurrentRoom(), nextDir), nextDir);
	else
		cantTurnThatWay(turnReason);
}

void Neighborhood::turnLeft() {
	turn(kTurnLeft);
}

void Neighborhood::turnRight() {
	turn(kTurnRight);
}

void Neighborhood::turnUp() {
	turn(kTurnUp);
}

void Neighborhood::turnDown() {
	turn(kTurnDown);
}

void Neighborhood::openDoor() {
	DoorTable::Entry door;
	CanOpenDoorReason doorReason = canOpenDoor(door);

	if (doorReason == kCanOpenDoor)
		startDoorOpenMovie(door.movieStart, door.movieEnd);
	else
		cantOpenDoor(doorReason);
}

void Neighborhood::zoomTo(const Hotspot *hotspot) {
	ZoomTable::Entry zoomEntry;
	getZoomEntry(hotspot->getObjectID(), zoomEntry);
	if (!zoomEntry.isEmpty())
		startZoomMovie(zoomEntry);
}

void Neighborhood::updateViewFrame() {
	showViewFrame(getViewTime(GameState.getCurrentRoom(), GameState.getCurrentDirection()));
}

void Neighborhood::startSpotLoop(TimeValue startTime, TimeValue stopTime, NotificationFlags flags) {
	_turnPush.hide();
	startMovieSequence(startTime, stopTime, flags, true, kFilterAllInput);
}

void Neighborhood::showViewFrame(TimeValue viewTime) {
	if ((int32)viewTime >= 0) {
		_turnPush.hide();
		_navMovie.stop();
		_navMovie.setFlags(0);
		_navMovie.setSegment(0, _navMovie.getDuration());
		_navMovie.setTime(viewTime);

		Common::Rect pushBounds;
		_turnPush.getBounds(pushBounds);

		_navMovie.moveElementTo(pushBounds.left, pushBounds.top);
		_navMovie.show();
		_navMovie.redrawMovieWorld();
	}
}

void Neighborhood::startExtraSequence(const ExtraID extraID, const NotificationFlags flags, const InputBits interruptionFilter) {
	ExtraTable::Entry entry;
	getExtraEntry(extraID, entry);

	if (entry.movieStart != 0xffffffff)
		playExtraMovie(entry, flags, interruptionFilter);
}

bool Neighborhood::startExtraSequenceSync(const ExtraID extraID, const InputBits interruptionFilter) {
	InputDevice.waitInput(interruptionFilter);
	return prepareExtraSync(extraID) && waitMovieFinish(&_navMovie, interruptionFilter);
}

void Neighborhood::loopExtraSequence(const uint32 extraID, NotificationFlags flags) {
	ExtraTable::Entry entry;
	getExtraEntry(extraID, entry);

	if (entry.movieStart != 0xffffffff) {
		_lastExtra = extraID;
		startSpotLoop(entry.movieStart, entry.movieEnd, flags);
	}
}

bool Neighborhood::navMoviePlaying() {
	return _navMovie.isRunning();
}

void Neighborhood::playDeathExtra(ExtraID extra, DeathReason deathReason) {
	_extraDeathReason = deathReason;
	startExtraSequence(extra, kDeathExtraCompletedFlag, kFilterNoInput);
}

void Neighborhood::die(const DeathReason deathReason) {
	loadLoopSound1("");
	loadLoopSound2("");
	_vm->die(deathReason);
}

void Neighborhood::setSoundFXLevel(const uint16 fxLevel) {
	if (_navMovie.isSurfaceValid())
		_navMovie.setVolume(fxLevel);
	if (_spotSounds.isSoundLoaded())
		_spotSounds.setVolume(fxLevel);
	if (_currentInteraction)
		_currentInteraction->setSoundFXLevel(fxLevel);
}

void Neighborhood::setAmbienceLevel(const uint16 ambientLevel) {
	if (_soundLoop1.isSoundLoaded())
		_loop1Fader.setMasterVolume(_vm->getAmbienceLevel());
	if (_soundLoop2.isSoundLoaded())
		_loop2Fader.setMasterVolume(_vm->getAmbienceLevel());
	if (_currentInteraction)
		_currentInteraction->setAmbienceLevel(ambientLevel);
}

// Force the exit taken from (room, direction, alternate) to come to a stop.
void Neighborhood::forceStridingStop(const RoomID room, const DirectionConstant direction, const AlternateID alternate) {
	ExitTable::Entry entry = _exitTable.findEntry(room, direction, alternate);

	if (entry.movieStart != 0xffffffff) {
		TimeValue strideStop = entry.exitEnd;
		TimeValue exitStop = entry.movieEnd;

		if (strideStop != exitStop) {
			for (ExitTable::iterator it = _exitTable.begin(); it != _exitTable.end(); it++) {
				entry = *it;

				if (entry.exitEnd == strideStop && entry.movieEnd <= exitStop) {
					entry.exitEnd = exitStop;
					*it = entry;
				}
			}
		}
	}
}

// Restore the exit taken from (room, direction, alternate) to stride.
void Neighborhood::restoreStriding(const RoomID room, const DirectionConstant direction, const AlternateID alternate) {
	ExitTable::Entry entry = _exitTable.findEntry(room, direction, alternate);

	if (entry.movieStart != 0xffffffff) {
		TimeValue strideStop = entry.exitEnd;
		TimeValue exitStop = entry.movieEnd;

		if (strideStop != entry.originalEnd) {
			for (ExitTable::iterator it = _exitTable.begin(); it != _exitTable.end(); it++) {
				entry = *it;

				if (entry.exitEnd == strideStop && entry.movieEnd <= exitStop) {
					entry.exitEnd = entry.originalEnd;
					*it = entry;
				}
			}
		}
	}
}

HotspotInfoTable::Entry *Neighborhood::findHotspotEntry(const HotSpotID id) {
	for (HotspotInfoTable::iterator it = _hotspotInfoTable.begin(); it != _hotspotInfoTable.end(); it++)
		if (it->hotspot == id)
			return &(*it);

	return 0;
}

void Neighborhood::hideNav() {
	_isRunning = _navMovie.isRunning();
	_navMovie.stop();
	_navMovie.hide();
	_turnPush.stopFader();
	_turnPush.hide();
}

void Neighborhood::showNav() {
	_navMovie.show();
	_turnPush.hide();
	if (_isRunning)
		_navMovie.start();
}

void Neighborhood::startExitMovie(const ExitTable::Entry &exitEntry) {
	FaderMoveSpec compassMove;

	if (g_compass)
		getExitCompassMove(exitEntry, compassMove);

	GameState.setNextRoom(exitEntry.exitRoom);
	GameState.setNextDirection(exitEntry.exitDirection);

	if (exitEntry.movieEnd == exitEntry.exitEnd) // Just a walk.
		startMovieSequence(exitEntry.movieStart, exitEntry.movieEnd, kMoveForwardCompletedFlag, kFilterNoInput, false);
	else // We're stridin'!
		startMovieSequence(exitEntry.movieStart, exitEntry.exitEnd, kStrideCompletedFlag, kFilterNoInput, false, exitEntry.movieEnd);

	if (g_compass)
		g_compass->startFader(compassMove);
}

void Neighborhood::startZoomMovie(const ZoomTable::Entry &zoomEntry) {
	FaderMoveSpec compassMove;

	if (g_compass)
		getZoomCompassMove(zoomEntry, compassMove);

	GameState.setNextRoom(zoomEntry.room);
	GameState.setNextDirection(zoomEntry.direction);

	startMovieSequence(zoomEntry.movieStart, zoomEntry.movieEnd, kMoveForwardCompletedFlag, kFilterNoInput, false);

	if (g_compass)
		g_compass->startFader(compassMove);
}

void Neighborhood::startDoorOpenMovie(const TimeValue startTime, const TimeValue stopTime) {
	startMovieSequence(startTime, stopTime, kDoorOpenCompletedFlag, kFilterNoInput, false);
}

void Neighborhood::startTurnPush(const TurnDirection turnDirection, const TimeValue newView, const DirectionConstant nextDir) {
	if (g_AIArea)
		g_AIArea->lockAIOut();

	_vm->_cursor->hide();

	GameState.setNextDirection(nextDir);

	_interruptionFilter = kFilterNoInput;
	_turnPush.stopFader();

	// Set up callback.
	_turnPushCallBack.setCallBackFlag(kTurnCompletedFlag);
	_turnPushCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);

	// Stop nav movie.
	_navMovie.stop();
	_navMovie.setFlags(0);

	// Set segment of nav movie to whole movie, so that subsequent initFromMovieFrame
	// will work.
	_navMovie.setSegment(0, _navMovie.getDuration());

	_pushIn.initFromMovieFrame(_navMovie.getMovie(), newView);

	_navMovie.hide();

	switch (turnDirection) {
	case kTurnLeft:
		_turnPush.setSlideDirection(kSlideRightMask);
		break;
	case kTurnRight:
		_turnPush.setSlideDirection(kSlideLeftMask);
		break;
	case kTurnUp:
		_turnPush.setSlideDirection(kSlideDownMask);
		break;
	case kTurnDown:
		_turnPush.setSlideDirection(kSlideUpMask);
		break;
	}

	_turnPush.show();

	FaderMoveSpec moveSpec;
	moveSpec.makeTwoKnotFaderSpec(60, 0, 0, 15, 1000);
	_turnPush.startFader(moveSpec);

	if (g_compass) {
		_turnPush.pauseFader();

		int32 startAngle = getStaticCompassAngle(GameState.getCurrentRoom(), GameState.getCurrentDirection());
		int32 stopAngle = getStaticCompassAngle(GameState.getCurrentRoom(), nextDir);

		if (turnDirection == kTurnLeft) {
			if (startAngle < stopAngle)
				startAngle += 360;
		} else {
			if (stopAngle < startAngle)
				stopAngle += 360;
		}

		FaderMoveSpec turnSpec;
		_turnPush.getCurrentFaderMove(turnSpec);

		FaderMoveSpec compassMove;
		compassMove.makeTwoKnotFaderSpec(turnSpec.getFaderScale(), turnSpec.getNthKnotTime(0), startAngle, turnSpec.getNthKnotTime(1), stopAngle);
		g_compass->startFader(compassMove);
	}

	_turnPushCallBack.cancelCallBack();
	_turnPush.continueFader();

	do {
		_vm->checkCallBacks();
		_vm->refreshDisplay();
		_vm->_system->delayMillis(10);
	} while (_turnPush.isFading());

	_turnPush.stopFader();
	_neighborhoodNotification.setNotificationFlags(kTurnCompletedFlag, kTurnCompletedFlag);
}

void Neighborhood::playExtraMovie(const ExtraTable::Entry &extraEntry, const NotificationFlags flags, const InputBits interruptionInput) {
	FaderMoveSpec compassMove;

	if (g_compass)
		getExtraCompassMove(extraEntry, compassMove);

	_lastExtra = extraEntry.extra;
	_turnPush.hide();
	startMovieSequence(extraEntry.movieStart, extraEntry.movieEnd, flags, false, interruptionInput);

	if (g_compass)
		g_compass->startFader(compassMove);
}

void Neighborhood::activateCurrentView(const RoomID room, const DirectionConstant direction, SpotFlags flag) {
	SpotTable::Entry entry;
	findSpotEntry(room, direction, flag, entry);

	if (entry.dstFlags & flag) {
		startSpotOnceOnly(entry.movieStart, entry.movieEnd);
	} else {
		findSpotEntry(room, direction, flag | kSpotLoopsMask, entry);

		if (entry.dstFlags & flag)
			startSpotLoop(entry.movieStart, entry.movieEnd);
		else
			showViewFrame(getViewTime(room, direction));
	}
}

void Neighborhood::activateOneHotspot(HotspotInfoTable::Entry &entry, Hotspot *hotspot) {
	switch (_vm->getDragType()) {
	case kDragInventoryUse:
		if ((hotspot->getHotspotFlags() & kDropItemSpotFlag) != 0 &&
				_vm->getDraggingItem()->getObjectID() == entry.hotspotItem)
			hotspot->setActive();
		break;
	case kDragInventoryPickup:
	case kDragBiochipPickup:
		// Do nothing -- neighborhoods activate no hot spots in this case...
		break;
	default:
		if ((hotspot->getHotspotFlags() & kPickUpBiochipSpotFlag) != 0) {
			Item *item = _vm->getAllItems().findItemByID(entry.hotspotItem);
			if (item &&	item->getItemNeighborhood() == getObjectID())
				hotspot->setActive();
		} else {
			HotSpotFlags flags = hotspot->getHotspotFlags();

			if ((flags & kNeighborhoodSpotFlag) != 0) {
				if (flags & kOpenDoorSpotFlag) {
					if (!GameState.isCurrentDoorOpen())
						hotspot->setActive();
				} else if ((flags & (kZoomSpotFlags | kClickSpotFlag | kPlayExtraSpotFlag)) != 0) {
					hotspot->setActive();
				} else if ((flags & kPickUpItemSpotFlag) != 0) {
					// Changed this 2/19/96
					// Should only light up this hot spot if the item's taken flag is not
					// set. It's not based on neighborhood ID since that can be reset by the
					// destroying process.

					if (!GameState.isTakenItemID(entry.hotspotItem))
						hotspot->setActive();
				}
			}
		}
		break;
	}
}

void Neighborhood::startSpotOnceOnly(TimeValue startTime, TimeValue stopTime) {
	_turnPush.hide();
	startMovieSequence(startTime, stopTime, kSpotCompletedFlag, kFilterNoInput, false);
}

void Neighborhood::startMovieSequence(const TimeValue startTime, const TimeValue stopTime, NotificationFlags flags, bool loopSequence,
		const InputBits interruptionInput, const TimeValue strideStop) {
	if (!loopSequence && g_AIArea)
		g_AIArea->lockAIOut();

	_interruptionFilter = interruptionInput;

	// Stop the movie before doing anything else
	_navMovie.stop();

	Common::Rect pushBounds;
	_turnPush.getBounds(pushBounds);

	_navMovie.moveElementTo(pushBounds.left, pushBounds.top);
	_navMovie.show();
	_navMovie.setFlags(0);
	_navMovie.setSegment(startTime, stopTime);
	_navMovie.setTime(startTime);

	if (loopSequence)
		_navMovie.setFlags(kLoopTimeBase);
	else
		flags |= kNeighborhoodMovieCompletedFlag;

	if (strideStop != 0xffffffff)
		// Subtract a little slop from the striding stop time to keep from "pumping" at the
		// end of a walk.
		// 40 is one frame (scale == 600, 15 fps).
		scheduleStridingCallBack(strideStop - kStridingSlop, flags);
	else
		scheduleNavCallBack(flags);

	_navMovie.start();
}

void Neighborhood::throwAwayInterface() {
	_doorTable.clear();
	_exitTable.clear();
	_extraTable.clear();
	_hotspotInfoTable.clear();
	_spotTable.clear();
	_turnTable.clear();
	_viewTable.clear();
	_zoomTable.clear();

	_navMovie.stopDisplaying();
	_navMovie.releaseMovie();
	_pushIn.deallocateSurface();
	_turnPush.stopDisplaying();
	_turnPush.setInAndOutElements(0, 0);
	_turnPush.disposeAllCallBacks();

	for (HotspotList::iterator it = _neighborhoodHotspots.begin(); it != _neighborhoodHotspots.end(); it++)
		_vm->getAllHotspots().remove(*it);

	_neighborhoodHotspots.deleteHotspots();
	_spotSounds.disposeSound();
	_delayTimer.disposeAllCallBacks();

	if (g_AIArea) {
		g_AIArea->saveAIState();
		g_AIArea->removeAllRules();
	}

	if (_currentInteraction)
		newInteraction(kNoInteractionID);

	_croppedMovie.releaseMovie();

	loadLoopSound1("");
	loadLoopSound2("");

	if (g_energyMonitor) {
		g_energyMonitor->stopEnergyDraining();
		g_energyMonitor->saveCurrentEnergyValue();
	}

	delete g_interface;
}

bool Neighborhood::prepareExtraSync(const ExtraID extraID) {
	ExtraTable::Entry extraEntry;
	FaderMoveSpec compassMove;

	if (g_compass) {
		getExtraEntry(extraID, extraEntry);
		getExtraCompassMove(extraEntry, compassMove);
	}

	ExtraTable::Entry entry;
	getExtraEntry(extraID, entry);
	bool result;

	if (entry.movieStart != 0xffffffff) {
		_turnPush.hide();

		// Stop the movie before doing anything else
		_navMovie.stop();

		Common::Rect pushBounds;
		_turnPush.getBounds(pushBounds);
		_navMovie.moveElementTo(pushBounds.left, pushBounds.top);

		_navMovie.show();
		_navMovie.setFlags(0);
		_navMovie.setSegment(entry.movieStart, entry.movieEnd);
		_navMovie.setTime(entry.movieStart);
		_navMovie.start();
		result = true;
	} else {
		result = false;
	}

	if (result && g_compass)
		g_compass->startFader(compassMove);

	return result;
}

bool Neighborhood::waitMovieFinish(Movie *movie, const InputBits interruptionFilter) {
	Input input;
	bool result = true;
	bool saveAllowed = _vm->swapSaveAllowed(false);
	bool openAllowed = _vm->swapLoadAllowed(false);

	while (movie->isRunning()) {
		InputDevice.getInput(input, interruptionFilter);

		if (input.anyInput() || _vm->shouldQuit()) {
			result = false;
			break;
		}

		_vm->checkCallBacks();
		_vm->refreshDisplay();
		_vm->_system->delayMillis(10);
	}

	movie->stop();
	_vm->swapSaveAllowed(saveAllowed);
	_vm->swapLoadAllowed(openAllowed);

	return result;
}

InputBits Neighborhood::getInputFilter() {
	return _interruptionFilter & InputHandler::getInputFilter();
}

void Neighborhood::getZoomCompassMove(const ZoomTable::Entry &zoomEntry, FaderMoveSpec &compassMove) {
	int32 startAngle = getStaticCompassAngle(GameState.getCurrentRoom(), GameState.getCurrentDirection());
	int32 stopAngle = getStaticCompassAngle(zoomEntry.room, zoomEntry.direction);

	if (startAngle > stopAngle) {
		if (stopAngle + 180 < startAngle)
			stopAngle += 360;
	} else {
		if (startAngle + 180 < stopAngle)
			startAngle += 360;
	}

	compassMove.makeTwoKnotFaderSpec(_navMovie.getScale(), zoomEntry.movieStart, startAngle, zoomEntry.movieEnd, stopAngle);
}

void Neighborhood::getExtraCompassMove(const ExtraTable::Entry &, FaderMoveSpec &compassMove) {
	compassMove.makeOneKnotFaderSpec(g_compass->getFaderValue());
}

void Neighborhood::setUpAIRules() {
	// Set up default rules here:
	// --  Energy warning rules.

	if (g_AIArea) {
		g_AIArea->forceAIUnlocked();

		if (!_vm->isDemo() && (getObjectID() == kPrehistoricID || getObjectID() == kNoradAlphaID ||
				getObjectID() == kNoradDeltaID || getObjectID() == kMarsID || getObjectID() == kWSCID)) {

			AIEnergyMonitorCondition *condition50 = new AIEnergyMonitorCondition(kWorriedEnergy);
			AIPlayMessageAction *message = new AIPlayMessageAction("Images/AI/Globals/XGLOB4A", false);
			AIRule *rule50 = new AIRule(condition50, message);

			AIEnergyMonitorCondition *condition25 = new AIEnergyMonitorCondition(kNervousEnergy);
			AICompoundAction *compound = new AICompoundAction();
			message = new AIPlayMessageAction("Images/AI/Globals/XGLOB4B", false);
			compound->addAction(message);
			AIDeactivateRuleAction *deactivate = new AIDeactivateRuleAction(rule50);
			compound->addAction(deactivate);
			AIRule *rule25 = new AIRule(condition25, compound);

			AIEnergyMonitorCondition *condition5 = new AIEnergyMonitorCondition(kPanicStrickenEnergy);
			compound = new AICompoundAction();
			message = new AIPlayMessageAction("Images/AI/Globals/XGLOB4C", false);
			compound->addAction(message);
			deactivate = new AIDeactivateRuleAction(rule50);
			compound->addAction(deactivate);
			deactivate = new AIDeactivateRuleAction(rule25);
			compound->addAction(deactivate);
			AIRule *rule5 = new AIRule(condition5, compound);

			g_AIArea->addAIRule(rule5);
			g_AIArea->addAIRule(rule25);
			g_AIArea->addAIRule(rule50);
		}
	}
}

GameInteraction *Neighborhood::makeInteraction(const InteractionID interactionID) {
	if (interactionID == kNoInteractionID)
		return 0;

	return new GameInteraction(interactionID, this);
}

void Neighborhood::newInteraction(const InteractionID interactionID) {
	GameInteraction *interaction = makeInteraction(interactionID);
	_doneWithInteraction = false;

	if (_currentInteraction) {
		_currentInteraction->stopInteraction();
		delete _currentInteraction;
	}

	_currentInteraction = interaction;

	if (_currentInteraction)
		_currentInteraction->startInteraction();

	if (g_AIArea)
		g_AIArea->checkMiddleArea();
}

void Neighborhood::bumpIntoWall() {
	_vm->_gfx->shakeTheWorld(15, 30);
}

void Neighborhood::zoomUpOrBump() {
	Hotspot *zoomSpot = 0;

	for (HotspotList::iterator it = _vm->getAllHotspots().begin(); it != _vm->getAllHotspots().end(); it++) {
		Hotspot *hotspot = *it;

		if ((hotspot->getHotspotFlags() & (kNeighborhoodSpotFlag | kZoomInSpotFlag)) == (kNeighborhoodSpotFlag | kZoomInSpotFlag)) {
			HotspotInfoTable::Entry *entry = findHotspotEntry(hotspot->getObjectID());

			if (entry && entry->hotspotRoom == GameState.getCurrentRoom() && entry->hotspotDirection == GameState.getCurrentDirection()) {
				if (zoomSpot) {
					zoomSpot = 0;
					break;
				} else {
					zoomSpot = hotspot;
				}
			}
		}
	}

	if (zoomSpot)
		zoomTo(zoomSpot);
	else
		bumpIntoWall();
}

void Neighborhood::loadLoopSound1(const Common::String &soundName, uint16 volume, TimeValue fadeOut, TimeValue fadeIn, TimeScale fadeScale) {
	FaderMoveSpec faderMove;

	if (!loop1Loaded(soundName)) {
		_loop1SoundString = soundName;

		if (_soundLoop1.isSoundLoaded()) {
			faderMove.makeTwoKnotFaderSpec(fadeScale, 0, _loop1Fader.getFaderValue(), fadeOut, 0);
			_loop1Fader.startFaderSync(faderMove);
		}

		if (!_loop1SoundString.empty()) {
			_soundLoop1.initFromAIFFFile(_loop1SoundString);
			_soundLoop1.loopSound();
			_loop1Fader.setMasterVolume(_vm->getAmbienceLevel());
			_loop1Fader.setFaderValue(0);
			faderMove.makeTwoKnotFaderSpec(fadeScale, 0, 0, fadeIn, volume);
			_loop1Fader.startFaderSync(faderMove);
		} else {
			_soundLoop1.disposeSound();
		}
	} else if (_loop1Fader.getFaderValue() != volume) {
		faderMove.makeTwoKnotFaderSpec(fadeScale, 0, _loop1Fader.getFaderValue(), fadeIn, volume);
		_loop1Fader.startFaderSync(faderMove);
	}
}

void Neighborhood::loadLoopSound2(const Common::String &soundName, uint16 volume, TimeValue fadeOut, TimeValue fadeIn, TimeScale fadeScale) {
	FaderMoveSpec faderMove;

	if (!loop2Loaded(soundName)) {
		_loop2SoundString = soundName;

		if (_soundLoop2.isSoundLoaded()) {
			faderMove.makeTwoKnotFaderSpec(fadeScale, 0, _loop2Fader.getFaderValue(), fadeOut, 0);
			_loop2Fader.startFaderSync(faderMove);
		}

		if (!_loop2SoundString.empty()) {
			_soundLoop2.initFromAIFFFile(_loop2SoundString);
			_soundLoop2.loopSound();
			_loop2Fader.setMasterVolume(_vm->getAmbienceLevel());
			_loop2Fader.setFaderValue(0);
			faderMove.makeTwoKnotFaderSpec(fadeScale, 0, 0, fadeIn, volume);
			_loop2Fader.startFaderSync(faderMove);
		} else {
			_soundLoop2.disposeSound();
		}
	} else if (_loop2Fader.getFaderValue() != volume) {
		faderMove.makeTwoKnotFaderSpec(fadeScale, 0, _loop2Fader.getFaderValue(), fadeIn, volume);
		_loop2Fader.startFaderSync(faderMove);
	}
}

void Neighborhood::takeItemFromRoom(Item *item) {
	item->setItemRoom(kNoNeighborhoodID, kNoRoomID, kNoDirection);
	// Also set the taken item flag. Do this before updating the view frame.
	GameState.setTakenItem(item, true);
	updateViewFrame();
}

void Neighborhood::dropItemIntoRoom(Item *item, Hotspot *) {
	item->setItemRoom(getObjectID(), GameState.getCurrentRoom(), GameState.getCurrentDirection());
	// Also set the taken item flag. Do this before updating the view frame.
	GameState.setTakenItem(item, false);
	updateViewFrame();
}

void Neighborhood::makeContinuePoint() {
	_vm->makeContinuePoint();
}

void Neighborhood::startLoop1Fader(const FaderMoveSpec &faderMove) {
	_loop1Fader.startFader(faderMove);
}

void Neighborhood::startLoop2Fader(const FaderMoveSpec &faderMove) {
	_loop2Fader.startFader(faderMove);
}

// *** Revised 6/13/96 to use the last frame of the extra sequence.
//     Necessary for Cinepak buildup.
void Neighborhood::showExtraView(uint32 extraID) {
	ExtraTable::Entry entry;
	getExtraEntry(extraID, entry);

	if (entry.movieEnd != 0xffffffff)
		showViewFrame(entry.movieEnd - 1);
}

void Neighborhood::startExtraLongSequence(const uint32 firstExtra, const uint32 lastExtra, NotificationFlags flags,
		const InputBits interruptionFilter) {
	ExtraTable::Entry firstEntry, lastEntry;
	getExtraEntry(firstExtra, firstEntry);

	if (firstEntry.movieStart != 0xffffffff) {
		getExtraEntry(lastExtra, lastEntry);
		_lastExtra = firstExtra;
		_turnPush.hide();
		startMovieSequence(firstEntry.movieStart, lastEntry.movieEnd, flags, kFilterNoInput, interruptionFilter);
	}
}

void Neighborhood::openCroppedMovie(const Common::String &movieName, CoordType left, CoordType top) {
	if (_croppedMovie.isMovieValid())
		closeCroppedMovie();

	_croppedMovie.initFromMovieFile(movieName);
	_croppedMovie.moveElementTo(left, top);
	_croppedMovie.startDisplaying();
	_croppedMovie.show();
}

void Neighborhood::loopCroppedMovie(const Common::String &movieName, CoordType left, CoordType top) {
	openCroppedMovie(movieName, left, top);
	_croppedMovie.redrawMovieWorld();
	_croppedMovie.setFlags(kLoopTimeBase);
	_croppedMovie.start();
}

void Neighborhood::closeCroppedMovie() {
	_croppedMovie.releaseMovie();
}

void Neighborhood::playCroppedMovieOnce(const Common::String &movieName, CoordType left, CoordType top, const InputBits interruptionFilter) {
	openCroppedMovie(movieName, left, top);
	_croppedMovie.redrawMovieWorld();
	_croppedMovie.start();

	InputBits oldInterruptionFilter = _interruptionFilter;
	if (oldInterruptionFilter != kFilterNoInput)
		_interruptionFilter = kFilterNoInput;

	bool saveAllowed = _vm->swapSaveAllowed(false);
	bool openAllowed = _vm->swapLoadAllowed(false);

	Input input;
	while (_croppedMovie.isRunning() && !_vm->shouldQuit()) {
		_vm->processShell();
		InputDevice.getInput(input, interruptionFilter);
		if (input.anyInput() || _vm->saveRequested() || _vm->loadRequested() || _vm->shouldQuit())
			break;
		_vm->_system->delayMillis(10);
	}

	if (oldInterruptionFilter != kFilterNoInput)
		_interruptionFilter = oldInterruptionFilter;

	closeCroppedMovie();
	_vm->swapSaveAllowed(saveAllowed);
	_vm->swapLoadAllowed(openAllowed);
}

void Neighborhood::playMovieSegment(Movie *movie, TimeValue startTime, TimeValue stopTime) {
	TimeValue oldStart, oldStop;
	movie->getSegment(oldStart, oldStop);

	if (stopTime == 0xffffffff)
		stopTime = movie->getDuration();

	movie->setSegment(startTime, stopTime);
	movie->setTime(startTime);
	movie->start();

	while (movie->isRunning()) {
		_vm->checkCallBacks();
		_vm->refreshDisplay();
		_vm->_system->delayMillis(10);
	}

	movie->stop();
	movie->setSegment(oldStart, oldStop);
}

void Neighborhood::recallToTSASuccess() {
	if (GameState.allTimeZonesFinished())
		_vm->jumpToNewEnvironment(kFullTSAID, kTSA37, kNorth);
	else
		_vm->jumpToNewEnvironment(kTinyTSAID, kTinyTSA37, kNorth);
}

void Neighborhood::recallToTSAFailure() {
	_vm->jumpToNewEnvironment(kTinyTSAID, kTinyTSA37, kNorth);
}

void Neighborhood::handleInput(const Input &input, const Hotspot *cursorSpot) {
	if (_vm->getGameMode() == kModeNavigation) {
		if (input.upButtonAnyDown())
			upButton(input);
		else if (input.downButtonAnyDown())
			downButton(input);
		else if (input.leftButtonAnyDown())
			leftButton(input);
		else if (input.rightButtonAnyDown())
			rightButton(input);
	}

	InputHandler::handleInput(input, cursorSpot);
}

void Neighborhood::setHotspotFlags(const HotSpotID id, const HotSpotFlags flags) {
	Hotspot *hotspot = _vm->getAllHotspots().findHotspotByID(id);
	hotspot->setMaskedHotspotFlags(flags, flags);
}

void Neighborhood::setIsItemTaken(const ItemID id) {
	GameState.setTakenItemID(id, _vm->playerHasItemID(id));
}

void Neighborhood::upButton(const Input &) {
	moveForward();
}

void Neighborhood::leftButton(const Input &) {
	turnLeft();
}

void Neighborhood::rightButton(const Input &) {
	turnRight();
}

void Neighborhood::downButton(const Input &) {
	if (_inputHandler->wantsCursor()) {
		_vm->getAllHotspots().deactivateAllHotspots();
		_inputHandler->activateHotspots();

		for (HotspotList::iterator it = _vm->getAllHotspots().begin(); it != _vm->getAllHotspots().end(); it++) {
			Hotspot *hotspot = *it;

			if (hotspot->isSpotActive() && (hotspot->getHotspotFlags() & (kNeighborhoodSpotFlag | kZoomOutSpotFlag)) == (kNeighborhoodSpotFlag | kZoomOutSpotFlag)) {
				HotspotInfoTable::Entry *entry = findHotspotEntry(hotspot->getObjectID());

				if (entry && entry->hotspotRoom == GameState.getCurrentRoom() && entry->hotspotDirection == GameState.getCurrentDirection()) {
					Input scratch;
					_inputHandler->clickInHotspot(scratch, hotspot);
					return;
				}
			}
		}
	}
}

void Neighborhood::initOnePicture(Picture *picture, const Common::String &pictureName, DisplayOrder order, CoordType left, CoordType top, bool show) {
	picture->initFromPICTFile(pictureName);
	picture->setDisplayOrder(order);
	picture->moveElementTo(left, top);
	picture->startDisplaying();
	if (show)
		picture->show();
}

void Neighborhood::initOneMovie(Movie *movie, const Common::String &movieName, DisplayOrder order, CoordType left, CoordType top, bool show) {
	movie->initFromMovieFile(movieName);
	movie->setDisplayOrder(order);
	movie->moveElementTo(left, top);
	movie->startDisplaying();

	if (show)
		movie->show();

	movie->redrawMovieWorld();
}

void Neighborhood::reinstateMonocleInterface() {
	_vm->_gfx->disableErase();

	_vm->createInterface();

	if (g_AIArea)
		setNextHandler(g_AIArea);

	init();

	moveNavTo(kNavAreaLeft, kNavAreaTop);

	if (g_interface)
		g_interface->setDate(getDateResID());

	if (g_AIArea)
		g_AIArea->restoreAIState();
}

void Neighborhood::useIdleTime() {
	if (_doneWithInteraction) {
		newInteraction(kNoInteractionID);
		loadAmbientLoops();
	}
}

void Neighborhood::timerFunction() {
	timerExpired(getTimerEvent());
}

void Neighborhood::scheduleEvent(const TimeValue time, const TimeScale scale, const uint32 eventType) {
	_eventTimer.stopFuse();
	_eventTimer.primeFuse(time, scale);
	_timerEvent = eventType;
	_eventTimer.setFunctor(new Common::Functor0Mem<void, Neighborhood>(this, &Neighborhood::timerFunction));
	_eventTimer.lightFuse();
}

void Neighborhood::cancelEvent() {
	_eventTimer.stopFuse();
}

void Neighborhood::pauseTimer() {
	_eventTimer.pauseFuse();
}

void Neighborhood::resumeTimer() {
	// NOTE: The original calls pauseFuse() here, which causes a bug with the robot
	// in WSC on the catwalk, causing him never to come after you if you don't act
	// against him.
	_eventTimer.resumeFuse();
}

bool Neighborhood::timerPaused() {
	return _eventTimer.isFusePaused();
}

} // End of namespace Pegasus
