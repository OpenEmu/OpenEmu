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
#include "pegasus/neighborhood/neighborhood.h"
#include "pegasus/neighborhood/caldoria/caldoria.h"
#include "pegasus/neighborhood/caldoria/caldoriamirror.h"

namespace Pegasus {

CaldoriaMirror::CaldoriaMirror(Neighborhood *owner) : GameInteraction(kCaldoriaMirrorInteractionID, owner) {
}

void CaldoriaMirror::openInteraction() {
	_neighborhoodNotification = _owner->getNeighborhoodNotification();
	_neighborhoodNotification->notifyMe(this, kExtraCompletedFlag, kExtraCompletedFlag);
}

void CaldoriaMirror::initInteraction() {
	_owner->setCurrentActivation(kActivateMirrorReady);
	_owner->startExtraSequence(kCaBathroomGreeting, kExtraCompletedFlag, kFilterNoInput);
}

void CaldoriaMirror::closeInteraction() {
	_neighborhoodNotification->cancelNotification(this);
}

void CaldoriaMirror::handleInput(const Input &input, const Hotspot *cursorSpot) {
	if (_owner->getLastExtra() == (uint32)kCaBathroomAgencyStandard || !input.anyDirectionInput())
		GameInteraction::handleInput(input, cursorSpot);
}

void CaldoriaMirror::activateHotspots() {
	GameInteraction::activateHotspots();

	switch (_owner->getLastExtra()) {
	case kCaBathroomGreeting:
	case kCaBathroomBodyFat:
	case kCaBathroomRetrothrash:
	case kCaBathroomGeoWave:
		g_allHotspots.activateOneHotspot(kCaBathroomMirrorSpotID);
		g_allHotspots.deactivateOneHotspot(kCaHairStyle1SpotID);
		g_allHotspots.deactivateOneHotspot(kCaHairStyle2SpotID);
		g_allHotspots.deactivateOneHotspot(kCaHairStyle3SpotID);
		break;
	case kCaBathroomStylistIntro:
	case kCaBathroomRetrothrashReturn:
	case kCaBathroomGeoWaveReturn:
		g_allHotspots.activateOneHotspot(kCaHairStyle1SpotID);
		g_allHotspots.activateOneHotspot(kCaHairStyle2SpotID);
		g_allHotspots.activateOneHotspot(kCaHairStyle3SpotID);
		g_allHotspots.deactivateOneHotspot(kCaBathroomMirrorSpotID);
		break;
	}
}

void CaldoriaMirror::clickInHotspot(const Input &input, const Hotspot *spot) {
	switch (spot->getObjectID()) {
	case kCaBathroomMirrorSpotID:
		switch (_owner->getLastExtra()) {
		case kCaBathroomGreeting:
			_owner->startExtraSequence(kCaBathroomBodyFat, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kCaBathroomBodyFat:
			_owner->startExtraSequence(kCaBathroomStylistIntro, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kCaBathroomRetrothrash:
			_owner->startExtraSequence(kCaBathroomRetrothrashReturn, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kCaBathroomGeoWave:
			_owner->startExtraSequence(kCaBathroomGeoWaveReturn, kExtraCompletedFlag, kFilterNoInput);
			break;
		}
		break;
	case kCaHairStyle1SpotID:
		_owner->startExtraSequence(kCaBathroomRetrothrash, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kCaHairStyle2SpotID:
		_owner->startExtraSequence(kCaBathroomAgencyStandard, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kCaHairStyle3SpotID:
		_owner->startExtraSequence(kCaBathroomGeoWave, kExtraCompletedFlag, kFilterNoInput);
		break;
	default:
		GameInteraction::clickInHotspot(input, spot);
		break;
	}
}

void CaldoriaMirror::receiveNotification(Notification *, const NotificationFlags) {
	switch (_owner->getLastExtra()) {
	case kCaBathroomRetrothrash:
	case kCaBathroomGeoWave:
		_owner->setCurrentActivation(kActivateMirrorReady);
		break;
	case kCaBathroomStylistIntro:
	case kCaBathroomRetrothrashReturn:
	case kCaBathroomGeoWaveReturn:
		_owner->setCurrentActivation(kActivateStylistReady);
		break;
	case kCaBathroomAgencyStandard:
		_owner->setCurrentActivation(kActivateHotSpotAlways);
		_owner->requestDeleteCurrentInteraction();
		GameState.setScoringFixedHair(true);
		GameState.setCaldoriaDoneHygiene(true);
		break;
	}

	allowInput(true);
}

} // End of namespace Pegasus
