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

#include "pegasus/energymonitor.h"
#include "pegasus/gamestate.h"
#include "pegasus/pegasus.h"
#include "pegasus/ai/ai_area.h"
#include "pegasus/items/inventory/airmask.h"
#include "pegasus/neighborhood/norad/constants.h"
#include "pegasus/neighborhood/norad/norad.h"
#include "pegasus/neighborhood/norad/noradelevator.h"
#include "pegasus/neighborhood/norad/pressuredoor.h"
#include "pegasus/neighborhood/norad/subcontrolroom.h"
#include "pegasus/neighborhood/norad/subplatform.h"

namespace Pegasus {

const NotificationFlags kDoneWithPressureDoorNotification = 1;

const NotificationFlags kNoradNotificationFlags = kDoneWithPressureDoorNotification;

// This class handles everything that Norad Alpha and Delta have in common, such as
// oxygen mask usage, the elevator and the pressure doors.

Norad::Norad(InputHandler *nextHandler, PegasusEngine *vm, const Common::String &resName, NeighborhoodID id) :
		Neighborhood(nextHandler, vm, resName, id), _noradNotification(kNoradNotificationID, vm) {
	_elevatorUpSpotID = kNoHotSpotID;
	_elevatorDownSpotID = kNoHotSpotID;
	_elevatorUpRoomID = kNoHotSpotID;
	_elevatorDownRoomID = kNoHotSpotID;

	_subRoomEntryRoom1 = kNoRoomID;
	_subRoomEntryDir1 = kNoDirection;
	_subRoomEntryRoom2 = kNoRoomID;
	_subRoomEntryDir2 = kNoDirection;
	_upperPressureDoorRoom = kNoRoomID;
	_lowerPressureDoorRoom = kNoRoomID;

	_upperPressureDoorUpSpotID = kNoHotSpotID;
	_upperPressureDoorDownSpotID = kNoHotSpotID;
	_upperPressureDoorAbortSpotID = kNoHotSpotID;

	_lowerPressureDoorUpSpotID = kNoHotSpotID;
	_lowerPressureDoorDownSpotID = kNoHotSpotID;
	_lowerPressureDoorAbortSpotID = kNoHotSpotID;

	_pressureSoundIn = 0xffffffff;
	_pressureSoundOut = 0xffffffff;
	_equalizeSoundIn = 0xffffffff;
	_equalizeSoundOut = 0xffffffff;
	_accessDeniedIn = 0xffffffff;
	_accessDeniedOut = 0xffffffff;

	_platformRoom = kNoRoomID;
	_subControlRoom = kNoRoomID;

	_doneWithPressureDoor = false;

	_noradNotification.notifyMe(this, kNoradNotificationFlags, kNoradNotificationFlags);
}

GameInteraction *Norad::makeInteraction(const InteractionID interactionID) {
	PressureDoor *pressureDoor;
	SubControlRoom *subControl;

	switch (interactionID) {
	case kNoradElevatorInteractionID:
		return new NoradElevator(this, _elevatorUpRoomID, _elevatorDownRoomID, _elevatorUpSpotID, _elevatorDownSpotID);
	case kNoradPressureDoorInteractionID:
		if (GameState.getCurrentRoom() == _upperPressureDoorRoom)
			pressureDoor = new PressureDoor(this, true, _upperPressureDoorUpSpotID, _upperPressureDoorDownSpotID,
					_upperPressureDoorAbortSpotID, _pressureSoundIn, _pressureSoundOut, _equalizeSoundIn, _equalizeSoundOut);
		else
			pressureDoor = new PressureDoor(this, false, _lowerPressureDoorUpSpotID, _lowerPressureDoorDownSpotID,
					_lowerPressureDoorAbortSpotID, _pressureSoundIn, _pressureSoundOut, _equalizeSoundIn, _equalizeSoundOut);

		if (GameState.getCurrentRoom() == kNorad59West && playingAgainstRobot())
			pressureDoor->playAgainstRobot();

		return pressureDoor;
	case kNoradSubControlRoomInteractionID:
		subControl = new SubControlRoom(this);

		if (GameState.getCurrentRoom() == kNorad60West && playingAgainstRobot())
			subControl->playAgainstRobot();

		return subControl;
	case kNoradSubPlatformInteractionID:
		return new SubPlatform(this);
	default:
		return 0;
	}
}

void Norad::flushGameState() {
	g_energyMonitor->saveCurrentEnergyValue();
}

void Norad::start() {
	setUpAirMask();
	Neighborhood::start();
}

void Norad::activateHotspots() {
	Neighborhood::activateHotspots();

	RoomID room = GameState.getCurrentRoom();
	if (room == _elevatorUpRoomID)
		_neighborhoodHotspots.activateOneHotspot(_elevatorDownSpotID);
	else if (room == _elevatorDownRoomID)
		_neighborhoodHotspots.activateOneHotspot(_elevatorUpSpotID);
}

void Norad::arriveAt(const RoomID room, const DirectionConstant direction) {
	Neighborhood::arriveAt(room, direction);

	if (GameState.getCurrentRoom() == _elevatorUpRoomID || GameState.getCurrentRoom() == _elevatorDownRoomID)
		arriveAtNoradElevator();
	else if (GameState.getCurrentRoom() == _upperPressureDoorRoom)
		arriveAtUpperPressureDoorRoom();
	else if (GameState.getCurrentRoom() == _lowerPressureDoorRoom)
		arriveAtLowerPressureDoorRoom();
	else if (GameState.getCurrentRoom() == _platformRoom)
		arriveAtSubPlatformRoom();
	else if (GameState.getCurrentRoom() == _subControlRoom)
		arriveAtSubControlRoom();

	if (_doneWithPressureDoor) {
		_doneWithPressureDoor = false;
		openDoor();
	}
}

void Norad::arriveAtNoradElevator() {
	if (_currentInteraction)
		_currentInteraction->startOverInteraction();
	else
		newInteraction(kNoradElevatorInteractionID);
}

void Norad::arriveAtUpperPressureDoorRoom() {
	newInteraction(kNoradPressureDoorInteractionID);
}

void Norad::arriveAtLowerPressureDoorRoom() {
	newInteraction(kNoradPressureDoorInteractionID);
}

void Norad::arriveAtSubPlatformRoom() {
	newInteraction(kNoradSubPlatformInteractionID);
}

void Norad::arriveAtSubControlRoom() {
	newInteraction(kNoradSubControlRoomInteractionID);
}

int16 Norad::getStaticCompassAngle(const RoomID room, const DirectionConstant dir) {
	int16 result = Neighborhood::getStaticCompassAngle(room, dir);

	if (room == _elevatorUpRoomID || room == _elevatorDownRoomID)
		result += kElevatorCompassAngle;
	else if (room == _platformRoom)
		result += kSubPlatformCompassAngle;
	else if (room == _subControlRoom)
		result += kSubControlCompassAngle;

	return result;
}

CanOpenDoorReason Norad::canOpenDoor(DoorTable::Entry &entry) {
	if (((GameState.getCurrentRoom() == _subRoomEntryRoom1 && GameState.getCurrentDirection() == _subRoomEntryDir1) ||
			(GameState.getCurrentRoom() == _subRoomEntryRoom2 && GameState.getCurrentDirection() == _subRoomEntryDir2)) &&
			GameState.getNoradSubRoomPressure() != kNormalSubRoomPressure)
		return kCantOpenBadPressure;

	return Neighborhood::canOpenDoor(entry);
}

void Norad::cantOpenDoor(CanOpenDoorReason reason) {
	if (reason == kCantOpenBadPressure)
		playSpotSoundSync(_pressureSoundIn, _pressureSoundOut);
	else
		playSpotSoundSync(_accessDeniedIn, _accessDeniedOut);
}

void Norad::startExitMovie(const ExitTable::Entry &exitEntry) {
	if (GameState.getCurrentRoom() == _elevatorUpRoomID) {
		if (exitEntry.exitRoom != _elevatorDownRoomID)
			newInteraction(kNoInteractionID);
	} else if (GameState.getCurrentRoom() == _elevatorDownRoomID) {
		if (exitEntry.exitRoom != _elevatorUpRoomID)
			newInteraction(kNoInteractionID);
	} else {
		newInteraction(kNoInteractionID);
	}

	Neighborhood::startExitMovie(exitEntry);
}

void Norad::startZoomMovie(const ZoomTable::Entry &zoomEntry) {
	newInteraction(kNoInteractionID);
	Neighborhood::startZoomMovie(zoomEntry);
}

void Norad::upButton(const Input &input) {
	if (GameState.getCurrentRoom() != _elevatorUpRoomID && GameState.getCurrentRoom() != _elevatorDownRoomID)
		Neighborhood::upButton(input);
}

void Norad::setUpAirMask() {
	_airMaskCallBack.setNotification(&_neighborhoodNotification);
	_airMaskCallBack.initCallBack(&_airMaskTimer, kCallBackAtExtremes);
	_airMaskCallBack.setCallBackFlag(kAirTimerExpiredFlag);
	_neighborhoodNotification.notifyMe(this, kAirTimerExpiredFlag, kAirTimerExpiredFlag);
	_airMaskCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
	_airMaskTimer.setScale(1);
	_airMaskTimer.setSegment(0, kNoradAirMaskTimeLimit);
	checkAirMask();
}

void Norad::checkAirMask() {
	if (g_airMask && g_airMask->isAirFilterOn()) {
		_airMaskTimer.stop();
	} else if (GameState.getNoradGassed() && !_airMaskTimer.isRunning()) {
		_airMaskTimer.setTime(0);
		_airMaskTimer.start();
	}

	loadAmbientLoops();
}

void Norad::receiveNotification(Notification *notification, const NotificationFlags flags) {
	if (notification == &_neighborhoodNotification && (flags & kAirTimerExpiredFlag) != 0)
		((PegasusEngine *)g_engine)->die(kDeathGassedInNorad);

	Neighborhood::receiveNotification(notification, flags);

	if (notification == &_noradNotification) {
		// Must be kDoneWithPressureDoorNotification...
		Input scratch;
		_doneWithPressureDoor = true;
		downButton(scratch);
	}
}

uint16 Norad::getDateResID() const {
	return kDate2112ID;
}

Common::String Norad::getBriefingMovie() {
	return "Images/AI/Norad/XNO";
}

void Norad::pickedUpItem(Item *item) {
	Neighborhood::pickedUpItem(item);
	g_AIArea->checkMiddleArea();
}

void Norad::doneWithPressureDoor() {
	_noradNotification.setNotificationFlags(kDoneWithPressureDoorNotification, kDoneWithPressureDoorNotification);
}

} // End of namespace Pegasus
