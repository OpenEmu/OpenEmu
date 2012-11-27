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

#include "pegasus/gamestate.h"
#include "pegasus/pegasus.h"
#include "pegasus/neighborhood/norad/constants.h"
#include "pegasus/neighborhood/norad/norad.h"
#include "pegasus/neighborhood/norad/noradelevator.h"

namespace Pegasus {

// Norad elevator PICTs:
static const ResIDType kElevatorLabelID = 200;
static const ResIDType kElevatorButtonsID = 201;
static const ResIDType kElevatorDownOnID = 202;
static const ResIDType kElevatorUpOnID = 203;

NoradElevator::NoradElevator(Neighborhood *handler, const RoomID upRoom, const RoomID downRoom,
		const HotSpotID upHotspot, const HotSpotID downHotspot) : GameInteraction(kNoradElevatorInteractionID, handler),
		_elevatorControls(kNoradElevatorControlsID), _elevatorNotification(kNoradElevatorNotificationID, ((PegasusEngine *)g_engine)) {
	_timerExpired = false;
	_upRoom = upRoom;
	_downRoom = downRoom;
	_upHotspot = upHotspot;
	_downHotspot = downHotspot;
}

void NoradElevator::openInteraction() {
	SpriteFrame *frame = new SpriteFrame();
	frame->initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kElevatorLabelID, true);
	_elevatorControls.addFrame(frame, 0, 0);

	frame = new SpriteFrame();
	frame->initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kElevatorButtonsID, true);
	_elevatorControls.addFrame(frame, 0, 0);

	frame = new SpriteFrame();
	frame->initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kElevatorDownOnID, true);
	_elevatorControls.addFrame(frame, 0, 0);

	frame = new SpriteFrame();
	frame->initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kElevatorUpOnID, true);
	_elevatorControls.addFrame(frame, 0, 0);

	_elevatorControls.setCurrentFrameIndex(0);
	_elevatorControls.setDisplayOrder(kElevatorControlsOrder);

	Common::Rect r;
	frame->getSurfaceBounds(r);
	r.moveTo(kNoradAlphaElevatorControlsLeft, kNoradAlphaElevatorControlsTop);

	_elevatorControls.setBounds(r);
	_elevatorControls.startDisplaying();
	_elevatorControls.show();
}

void NoradElevator::initInteraction() {
	_elevatorTimer.setScale(2);
	_elevatorTimer.setSegment(0, 1);
	_elevatorCallBack.initCallBack(&_elevatorTimer, kCallBackAtExtremes);
	_elevatorCallBack.setCallBackFlag(1);
	_elevatorCallBack.setNotification(&_elevatorNotification);
	_elevatorNotification.notifyMe(this, 1, 1);
	_elevatorCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
	_elevatorTimer.start();
}

void NoradElevator::closeInteraction() {
	_elevatorControls.stopDisplaying();
	_elevatorControls.discardFrames();
	_elevatorCallBack.releaseCallBack();
}

void NoradElevator::resetInteraction() {
	_elevatorControls.setCurrentFrameIndex(1);
}

void NoradElevator::activateHotspots() {
	GameInteraction::activateHotspots();

	if (_timerExpired) {
		if (GameState.getCurrentRoom() == _upRoom)
			g_allHotspots.activateOneHotspot(_downHotspot);
		else if (GameState.getCurrentRoom() == _downRoom)
			g_allHotspots.activateOneHotspot(_upHotspot);
	}
}

void NoradElevator::clickInHotspot(const Input &input, const Hotspot *spot) {
	HotSpotID id = spot->getObjectID();

	if (id == _upHotspot || id == _downHotspot) {
		g_neighborhood->moveForward();
		if (id == _downHotspot)
			_elevatorControls.setCurrentFrameIndex(2);
		else
			_elevatorControls.setCurrentFrameIndex(3);
	} else {
		GameInteraction::clickInHotspot(input, spot);
	}
}

void NoradElevator::receiveNotification(Notification *, const NotificationFlags) {
	_elevatorControls.setCurrentFrameIndex(1);
	_timerExpired = true;
}

} // End of namespace Pegasus
