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
#include "pegasus/neighborhood/neighborhood.h"
#include "pegasus/neighborhood/caldoria/caldoria.h"
#include "pegasus/neighborhood/caldoria/caldoriamessages.h"

namespace Pegasus {

static const NotificationFlags kMessageDoneFlag = 1;

CaldoriaMessages::CaldoriaMessages(Neighborhood *owner, const NotificationID id, NotificationManager *manager) :
		GameInteraction(kCaldoriaMessagesInteractionID, owner), Notification(id, manager), _messageMovie(kCaldoriaMessagesID) {
}

void CaldoriaMessages::openInteraction() {
	_neighborhoodNotification = GameInteraction::_owner->getNeighborhoodNotification();
	_neighborhoodNotification->notifyMe(this, kExtraCompletedFlag, kExtraCompletedFlag);
	_messageCallBack.setNotification(this);
	notifyMe(this, kMessageDoneFlag, kMessageDoneFlag);
	_messageCallBack.setCallBackFlag(kMessageDoneFlag);
	_messageNumber = 1;
}

void CaldoriaMessages::initInteraction() {
	GameInteraction::_owner->startExtraSequence(kCaBedroomVidPhone, kExtraCompletedFlag, kFilterNoInput);
}

void CaldoriaMessages::closeInteraction() {
	cancelNotification(this);
	_neighborhoodNotification->cancelNotification(this);
}

void CaldoriaMessages::receiveNotification(Notification *notification, const NotificationFlags) {
	if (notification == _neighborhoodNotification) {
		switch (GameInteraction::_owner->getLastExtra()) {
		case kCaBedroomVidPhone:
			GameInteraction::_owner->showExtraView(kCaBedroomMessage1);
			break;
		case kCaBedroomMessage1:
			play1Message(1);
			break;
		case kCaBedroomMessage2:
			play1Message(2);
			break;
		}
	} else {
		_messageCallBack.releaseCallBack();
		_messageMovie.releaseMovie();

		uint32 extraID = (_messageNumber == 1) ? kCaBedroomMessage1 : kCaBedroomMessage2;
		GameInteraction::_owner->showExtraView(extraID);
		allowInput(true);
	}
}

void CaldoriaMessages::clickInHotspot(const Input &input, const Hotspot *spot) {
	uint32 extraID;

	switch (spot->getObjectID()) {
	case kCaBedroomVidPhoneActivationSpotID:
		extraID = (_messageNumber == 1) ? kCaBedroomMessage1 : kCaBedroomMessage2;
		GameInteraction::_owner->startExtraSequence(extraID, kExtraCompletedFlag, kFilterNoInput);
		break;
	default:
		GameInteraction::clickInHotspot(input, spot);
		break;
	}
}

void CaldoriaMessages::play1Message(uint messageNumber) {
	if (messageNumber == 1) {
		_messageMovie.initFromMovieFile("Images/Caldoria/A12NVA.movie");
		_messageNumber = 2;
	} else {
		_messageMovie.initFromMovieFile("Images/Caldoria/A12NVB.movie");
		_messageNumber = 1;
		GameState.setCaldoriaSeenMessages(true);
	}

	_messageMovie.moveElementTo(kCaldoriaMessageLeft, kCaldoriaMessageTop);
	_messageMovie.setDisplayOrder(kCaldoriaMessagesOrder);
	_messageMovie.startDisplaying();
	_messageCallBack.initCallBack(&_messageMovie, kCallBackAtExtremes);
	_messageCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
	allowInput(false);
	_messageMovie.show();
	_messageMovie.redrawMovieWorld();
	_messageMovie.start();
}

} // End of namespace Pegasus
