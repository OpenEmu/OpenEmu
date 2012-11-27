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
#include "pegasus/neighborhood/norad/subcontrolroom.h"
#include "pegasus/neighborhood/norad/alpha/ecrmonitor.h"
#include "pegasus/neighborhood/norad/alpha/fillingstation.h"
#include "pegasus/neighborhood/norad/alpha/noradalpha.h"

namespace Pegasus {

const uint32 NoradAlpha::_noradAlphaClawExtras[22] = {
	kN22ClawFromAToB,
	kN22ClawALoop,
	kN22ClawAPinch,
	kN22ClawACounterclockwise,
	kN22ClawAClockwise,
	kN22ClawFromBToA,
	kN22ClawFromBToC,
	kN22ClawFromBToD,
	kN22ClawBLoop,
	kN22ClawBPinch,
	kN22ClawBCounterclockwise,
	kN22ClawBClockwise,
	kN22ClawFromCToB,
	kN22ClawCLoop,
	kN22ClawCPinch,
	kN22ClawCCounterclockwise,
	kN22ClawCClockwise,
	kN22ClawFromDToB,
	kN22ClawDLoop,
	kN22ClawDPinch,
	kN22ClawDCounterclockwise,
	kN22ClawDClockwise
};

NoradAlpha::NoradAlpha(InputHandler *nextHandler, PegasusEngine *owner) : Norad(nextHandler, owner, "Norad Alpha", kNoradAlphaID) {
	_elevatorUpRoomID = kNorad11South;
	_elevatorDownRoomID = kNorad12South;
	_elevatorUpSpotID = kNorad12ElevatorUpSpotID;
	_elevatorDownSpotID = kNorad11ElevatorDownSpotID;

	_subRoomEntryRoom1 = kNorad10;
	_subRoomEntryDir1 = kEast;
	_subRoomEntryRoom2 = kNorad21;
	_subRoomEntryDir2 = kWest;
	_upperPressureDoorRoom = kNorad10East;
	_lowerPressureDoorRoom = kNorad21West;

	_upperPressureDoorUpSpotID = kAlphaUpperPressureDoorUpSpotID;
	_upperPressureDoorDownSpotID = kAlphaUpperPressureDoorDownSpotID;
	_upperPressureDoorAbortSpotID = kNorad10EastOutSpotID;

	_lowerPressureDoorUpSpotID = kAlphaLowerPressureDoorUpSpotID;
	_lowerPressureDoorDownSpotID = kAlphaLowerPressureDoorDownSpotID;
	_lowerPressureDoorAbortSpotID = kNorad21WestOutSpotID;

	_pressureSoundIn = kPressureDoorIntro1In;
	_pressureSoundOut = kPressureDoorIntro1Out;
	_equalizeSoundIn = kPressureDoorIntro2In;
	_equalizeSoundOut = kPressureDoorIntro2Out;
	_accessDeniedIn = kAlphaAccessDeniedIn;
	_accessDeniedOut = kAlphaAccessDeniedOut;

	_platformRoom = kNorad19West;
	_subControlRoom = kNorad22West;

	_subPrepFailed = false;

	setIsItemTaken(kGasCanister);
}

void NoradAlpha::init() {
	Norad::init();

	Hotspot *hotspot = _vm->getAllHotspots().findHotspotByID(kN01GasCanisterSpotID);
	hotspot->setMaskedHotspotFlags(kPickUpItemSpotFlag, kPickUpItemSpotFlag);
	HotspotInfoTable::Entry *hotspotEntry = findHotspotEntry(kN01GasCanisterSpotID);
	hotspotEntry->hotspotItem = kGasCanister;

	hotspot = _vm->getAllHotspots().findHotspotByID(kN01ArgonCanisterSpotID);
	hotspot->setMaskedHotspotFlags(kPickUpItemSpotFlag, kPickUpItemSpotFlag);
	hotspotEntry = findHotspotEntry(kN01ArgonCanisterSpotID);
	hotspotEntry->hotspotItem = kArgonCanister;

	hotspot = _vm->getAllHotspots().findHotspotByID(kN01NitrogenCanisterSpotID);
	hotspot->setMaskedHotspotFlags(kPickUpItemSpotFlag, kPickUpItemSpotFlag);
	hotspotEntry = findHotspotEntry(kN01NitrogenCanisterSpotID);
	hotspotEntry->hotspotItem = kNitrogenCanister;

	hotspot = _vm->getAllHotspots().findHotspotByID(kN01AirMaskSpotID);
	hotspot->setMaskedHotspotFlags(kPickUpItemSpotFlag, kPickUpItemSpotFlag);
	hotspotEntry = findHotspotEntry(kN01AirMaskSpotID);
	hotspotEntry->hotspotItem = kAirMask;

	hotspot = _vm->getAllHotspots().findHotspotByID(kN01GasOutletSpotID);
	hotspot->setMaskedHotspotFlags(kDropItemSpotFlag, kDropItemSpotFlag);
}

void NoradAlpha::start() {
	if (g_energyMonitor) {
		g_energyMonitor->stopEnergyDraining();
		g_energyMonitor->restoreLastEnergyValue();
		_vm->resetEnergyDeathReason();
		g_energyMonitor->startEnergyDraining();
	}

	NeighborhoodID itemNeighborhood;
	RoomID itemRoom;
	DirectionConstant itemDirection;

	Item *item = (Item *)_vm->getAllItems().findItemByID(kGasCanister);
	item->getItemRoom(itemNeighborhood, itemRoom, itemDirection);

	if (itemNeighborhood == getObjectID()) {
		_fillingStationItem = item;
	} else {
		item = (Item *)_vm->getAllItems().findItemByID(kAirMask);
		item->getItemRoom(itemNeighborhood, itemRoom, itemDirection);

		if (itemNeighborhood == getObjectID()) {
			_fillingStationItem = item;
		} else {
			item = (Item *)_vm->getAllItems().findItemByID(kNitrogenCanister);
			item->getItemRoom(itemNeighborhood, itemRoom, itemDirection);

			if (itemNeighborhood == getObjectID()) {
				_fillingStationItem = item;
			} else {
				item = (Item *)_vm->getAllItems().findItemByID(kArgonCanister);
				item->getItemRoom(itemNeighborhood, itemRoom, itemDirection);
				if (itemNeighborhood == getObjectID())
					_fillingStationItem = item;
				else
					_fillingStationItem = 0;
			}
		}
	}

	if (!GameState.getNoradGassed())
		forceStridingStop(kNorad03, kEast, kAltNoradAlphaNormal);

	GameState.setNoradArrivedFromSub(false);
	Norad::start();
}

void NoradAlpha::setUpAIRules() {
	Neighborhood::setUpAIRules();

	if (g_AIArea) {
		AIPlayMessageAction *messageAction = new AIPlayMessageAction("Images/AI/Norad/XN01WD1", false);
		AIHasItemCondition *hasGasCanisterCondition = new AIHasItemCondition(kGasCanister);
		AIRule *rule = new AIRule(hasGasCanisterCondition, messageAction);
		g_AIArea->addAIRule(rule);
	}
}

bool NoradAlpha::okayToJump() {
	bool result = Neighborhood::okayToJump();

	if (!result)
		playSpotSoundSync(kAlphaCantTransportIn, kAlphaCantTransportOut);

	return result;
}

void NoradAlpha::getExtraCompassMove(const ExtraTable::Entry &entry, FaderMoveSpec &compassMove) {
	if (entry.extra == kNorad19ExitToSub) {
		compassMove.makeTwoKnotFaderSpec(kNoradAlphaMovieScale, entry.movieStart, 270 + kSubPlatformCompassAngle,
				entry.movieEnd, 90 + 20 + 360);
		compassMove.insertFaderKnot(entry.movieStart + 10 * kNoradAlphaFrameDuration, 270 + kSubPlatformCompassAngle);
		compassMove.insertFaderKnot(entry.movieStart + 29 * kNoradAlphaFrameDuration, 270 + kSubPlatformCompassAngle + 20);
		compassMove.insertFaderKnot(entry.movieStart + 52 * kNoradAlphaFrameDuration, 270 + kSubPlatformCompassAngle + 20);
		compassMove.insertFaderKnot(entry.movieStart + 84 * kNoradAlphaFrameDuration, 360 + 90);
		compassMove.insertFaderKnot(entry.movieStart + 198 * kNoradAlphaFrameDuration, 360 + 90);
		compassMove.insertFaderKnot(entry.movieStart + 270 * kNoradAlphaFrameDuration, 360 + 90 + 15);
		compassMove.insertFaderKnot(entry.movieStart + 280 * kNoradAlphaFrameDuration, 360 + 90 + 20);
	} else {
		Norad::getExtraCompassMove(entry, compassMove);
	}
}

void NoradAlpha::playClawMonitorIntro() {
	playSpotSoundSync(kLoadClawIntroIn, kLoadClawIntroOut);
}

GameInteraction *NoradAlpha::makeInteraction(const InteractionID interactionID) {
	switch (interactionID) {
	case kNoradECRMonitorInteractionID:
		return new NoradAlphaECRMonitor(this);
	case kNoradFillingStationInteractionID:
		return new NoradAlphaFillingStation(this);
	}

	return Norad::makeInteraction(interactionID);
}

void NoradAlpha::loadAmbientLoops() {
	// clone2727 would like to point out that the following comment does not quite
	// match the code logic below

/*
	Logic:

	loop sound 1:
		if gassed,
			play warning loop of some sort
		else
			play nothing
	loop sound 2:
		if gassed and not wearing air mask
			if in ECR
				play breathing water loop
			else
				play breathing
		else
			if in ECR
				play water loop
			if at N07 north
				play unmanned loop
*/

	if (!GameState.getNoradSeenTimeStream())
		return;

	RoomID room = GameState.getCurrentRoom();
	if (GameState.getNoradGassed()) {
		if (room >= kNorad11 && room <= kNorad19West)
			loadLoopSound1("Sounds/Norad/NEW SUB AMB.22K.AIFF", kNoradWarningVolume * 3);
		else if (room >= kNorad21 && room <= kNorad22West)
			loadLoopSound1("Sounds/Norad/SUB CONTRL LOOP.22K.AIFF", kNoradWarningVolume * 3);
		else
			loadLoopSound1("Sounds/Norad/WARNING LOOP.22K.AIFF", kNoradWarningVolume);
	} else {
		loadLoopSound1("");
	}

	if (GameState.getNoradGassed() && !g_airMask->isAirFilterOn()) {
		if (room >= kNorad01 && room <= kNorad01West) {
			loadLoopSound2("Sounds/Norad/Breathing Water.22K.AIFF", kNoradSuckWindVolume);
		} else if (room == kNorad02) {
			if (GameState.isCurrentDoorOpen())
				loadLoopSound2("Sounds/Norad/Breathing Water.22K.AIFF", kNoradSuckWindVolume);
			else
				loadLoopSound2("Sounds/Norad/SUCKING WIND.22K.AIFF", kNoradSuckWindVolume, 0, 0);
		} else {
			loadLoopSound2("Sounds/Norad/SUCKING WIND.22K.AIFF", kNoradSuckWindVolume, 0, 0);
		}
	} else {
		if (room >= kNorad01 && room <= kNorad01West) {
			loadLoopSound2("Sounds/Norad/WATER FLOWING.AIFF", 0x100 / 2);
		} else if (room == kNorad02) {
			if (GameState.isCurrentDoorOpen())
				loadLoopSound2("Sounds/Norad/WATER FLOWING.AIFF", 0x100 / 2);
			else
				loadLoopSound2("");
		} else {
			loadLoopSound2("");
		}
	}

}

void NoradAlpha::checkContinuePoint(const RoomID room, const DirectionConstant direction) {
	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kNorad02, kEast):
	case MakeRoomView(kNorad06, kEast):
	case MakeRoomView(kNorad11, kEast):
	case MakeRoomView(kNorad15, kEast):
	case MakeRoomView(kNorad19, kWest):
	case MakeRoomView(kNorad21, kSouth):
		makeContinuePoint();
		break;
	}
}

void NoradAlpha::arriveAt(const RoomID room, const DirectionConstant direction) {
	Norad::arriveAt(room, direction);

	switch (GameState.getCurrentRoom()) {
	case kNorad01:
		arriveAtNorad01();
		break;
	case kNorad01East:
		arriveAtNorad01East();
		break;
	case kNorad01West:
		arriveAtNorad01West();
		break;
	case kNorad04:
		arriveAtNorad04();
		break;
	case kNorad07North:
		GameState.setScoringSawUnconsciousOperator(true);
		break;
	case kNorad11:
		GameState.setScoringWentThroughPressureDoor(true);
		break;
	case kNorad22:
		arriveAtNorad22();
		break;
	}
}

void NoradAlpha::arriveAtNorad01() {
	if (!GameState.getNoradSeenTimeStream() && GameState.getCurrentDirection() == kSouth) {
		GameState.setNoradN22MessagePlayed(false);
		requestExtraSequence(kNoradArriveFromTSA, kExtraCompletedFlag, kFilterNoInput);
		// You are no match for me, human.
		requestExtraSequence(kNorad01RobotTaunt, kExtraCompletedFlag, kFilterNoInput);
	}
}

void NoradAlpha::arriveAtNorad01East() {
	GameState.setScoringSawSecurityMonitor(true);
	newInteraction(kNoradECRMonitorInteractionID);
}

void NoradAlpha::arriveAtNorad01West() {
	newInteraction(kNoradFillingStationInteractionID);
}

void NoradAlpha::arriveAtNorad04() {
	if (GameState.getCurrentDirection() == kEast && !GameState.getNoradGassed())
		playDeathExtra(kNorad04EastDeath, kDeathWokeUpNorad);
}

void NoradAlpha::arriveAtNorad22() {
	if (!GameState.getNoradN22MessagePlayed() && GameState.getCurrentDirection() == kSouth) {
		startExtraSequence(kNorad22SouthIntro, kExtraCompletedFlag, kFilterNoInput);
		GameState.setNoradN22MessagePlayed(true);
	}
}

void NoradAlpha::bumpIntoWall() {
	requestSpotSound(kAlphaBumpIntoWallIn, kAlphaBumpIntoWallOut, kFilterNoInput, 0);
	Neighborhood::bumpIntoWall();
}

void NoradAlpha::receiveNotification(Notification *notification, const NotificationFlags flags) {
	if ((flags & kExtraCompletedFlag) != 0) {
		switch (_lastExtra) {
		case kNoradArriveFromTSA:
			GameState.setNoradSeenTimeStream(true);
			loadAmbientLoops();
			break;
		case kNorad01RobotTaunt:
			g_AIArea->playAIMovie(kRightAreaSignature, "Images/AI/Norad/XN01SB", false, kWarningInterruption);
			_interruptionFilter = kFilterAllInput;
			makeContinuePoint();
			break;
		}
	}

	Norad::receiveNotification(notification, flags);

	if ((flags & kExtraCompletedFlag) != 0) {
		switch (_lastExtra) {
		case kNorad22SouthIntro:
			loopExtraSequence(kNorad22SouthReply);
			playSpotSoundSync(kN22ReplyIn, kN22ReplyOut);
			startExtraSequence(kNorad22SouthFinish, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kNorad22SouthFinish:
			_interruptionFilter = kFilterAllInput;
			// Force ArriveAt to do its thing...
			GameState.setCurrentRoom(kNorad21);
			arriveAt(kNorad22, kSouth);
			break;
		}
	}

	g_AIArea->checkMiddleArea();
}

void NoradAlpha::getZoomEntry(const HotSpotID spotID, ZoomTable::Entry &entry) {
	Norad::getZoomEntry(spotID, entry);

	ExtraTable::Entry extra;

	if (spotID == kNorad01GasSpotID) {
		if (_fillingStationItem) {
			if (_fillingStationItem->getObjectID() == kGasCanister) {
				getExtraEntry(kNorad01ZoomInWithGasCanister, extra);
				entry.movieStart = extra.movieStart;
				entry.movieEnd = extra.movieEnd;
			} else {
				entry.clear();
			}
		}
	} else if (spotID == kNorad01GasOutSpotID) {
		if (_fillingStationItem) {
			if (_fillingStationItem->getObjectID() == kGasCanister) {
				getExtraEntry(kNorad01ZoomOutWithGasCanister, extra);
				entry.movieStart = extra.movieStart;
				entry.movieEnd = extra.movieEnd;
			} else {
				entry.clear();
			}
		}
	}
}

TimeValue NoradAlpha::getViewTime(const RoomID room, const DirectionConstant direction) {
	ExtraTable::Entry entry;

	if (room == kNorad01 && direction == kSouth && !GameState.getNoradSeenTimeStream()) {
		getExtraEntry(kNoradArriveFromTSA, entry);
		return entry.movieStart;
	}

	if (room == kNorad01 && direction == kWest) {
		if (!_fillingStationItem) {
			return Norad::getViewTime(room, direction);
		} else {
			getExtraEntry(kN01WGasCanister, entry);
			return entry.movieStart;
		}
	} else if (room == kNorad01West && direction == kWest) {
		uint32 extraID = 0xffffffff;
		if (_fillingStationItem) {
			switch (_fillingStationItem->getObjectID()) {
			case kArgonCanister:
				if (GameState.getNoradFillingStationOn())
					extraID = kN01WZArgonCanisterLit;
				else
					extraID = kN01WZArgonCanisterDim;
				break;
			case kGasCanister:
				if (GameState.getNoradFillingStationOn())
					extraID = kN01WZGasCanisterLit;
				else
					extraID = kN01WZGasCanisterDim;
				break;
			case kAirMask:
				if (GameState.getNoradFillingStationOn())
					extraID = kN01WZAirMaskLit;
				else
					extraID = kN01WZAirMaskDim;
				break;
			case kNitrogenCanister:
				if (GameState.getNoradFillingStationOn())
					extraID = kN01WZNitrogenCanisterLit;
				else
					extraID = kN01WZNitrogenCanisterDim;
				break;
			default:
				// Should never happen.
				break;
			}
		} else if (GameState.getNoradFillingStationOn()) {
			extraID = kN01WZEmptyLit;
		}

		if (extraID == 0xffffffff) {
			return Norad::getViewTime(room, direction);
		} else {
			getExtraEntry(extraID, entry);
			return entry.movieStart;
		}
	}

	return Norad::getViewTime(room, direction);
}

void NoradAlpha::turnOnFillingStation() {
	if (GameState.getCurrentRoom() == kNorad01West && !GameState.getNoradFillingStationOn()) {
		GameState.setNoradFillingStationOn(true);
		updateViewFrame();
	}
}

void NoradAlpha::turnOffFillingStation() {
	if (GameState.getCurrentRoom() == kNorad01West && GameState.getNoradFillingStationOn()) {
		GameState.setNoradFillingStationOn(false);
		updateViewFrame();
	}
}

void NoradAlpha::activateHotspots() {
	Norad::activateHotspots();

	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kNorad01West, kWest):
		if (_vm->getDragType() == kDragInventoryUse) {
			if (!_fillingStationItem) {
				ItemID itemID = _vm->getDraggingItem()->getObjectID();
				if (itemID == kArgonCanister || itemID == kGasCanister || itemID == kAirMask ||
						itemID == kNitrogenCanister)
					_vm->getAllHotspots().activateOneHotspot(kN01GasOutletSpotID);
			}
		} else {
			HotSpotID spotID;

			if (_fillingStationItem) {
				switch (_fillingStationItem->getObjectID()) {
				case kArgonCanister:
					spotID = kN01ArgonCanisterSpotID;
					_vm->getAllHotspots().deactivateOneHotspot(kNorad01GasOutSpotID);
					break;
				case kGasCanister:
					spotID = kN01GasCanisterSpotID;
					break;
				case kAirMask:
					spotID = kN01AirMaskSpotID;
					_vm->getAllHotspots().deactivateOneHotspot(kNorad01GasOutSpotID);
					break;
				case kNitrogenCanister:
					spotID = kN01NitrogenCanisterSpotID;
					_vm->getAllHotspots().deactivateOneHotspot(kNorad01GasOutSpotID);
					break;
				default:
					// Should never happen.
					spotID = kNoHotSpotID;
					break;
				}
				_vm->getAllHotspots().activateOneHotspot(spotID);
			}
		}
		break;
	case MakeRoomView(kNorad10, kEast):
		if (GameState.isCurrentDoorOpen())
			_vm->getAllHotspots().deactivateOneHotspot(kNorad10DoorSpotID);
		break;
	case MakeRoomView(kNorad21, kWest):
		if (GameState.isCurrentDoorOpen())
			_vm->getAllHotspots().deactivateOneHotspot(kNorad21WestSpotID);
		break;
	}
}

void NoradAlpha::clickInHotspot(const Input &input, const Hotspot *cursorSpot) {
	Norad::clickInHotspot(input, cursorSpot);

	if (_vm->getDragType() == kDragInventoryUse) {
		if (GameState.getCurrentRoomAndView() == MakeRoomView(kNorad01West, kWest)) {
			Item *item = _vm->getDraggingItem();
			if (item->getObjectID() == kAirMask || item->getObjectID() == kArgonCanister ||
					item->getObjectID() == kNitrogenCanister || item->getObjectID() == kGasCanister) {
				HotspotInfoTable::Entry *hotspotEntry = findHotspotEntry(kN01GasOutletSpotID);
				hotspotEntry->hotspotItem = item->getObjectID();
			}
		}
	}
}

void NoradAlpha::takeItemFromRoom(Item *item) {
	if (GameState.getCurrentRoom() == kNorad01West) {
		if (_fillingStationItem == item) {
			_fillingStationItem = 0;
			GameState.setNoradGassed(false);
			loadAmbientLoops();
			((NoradAlphaFillingStation *)_currentInteraction)->newFillingItem(0);
			forceStridingStop(kNorad03, kEast, kAltNoradAlphaNormal);
		}
	}

	Norad::takeItemFromRoom(item);
}

void NoradAlpha::dropItemIntoRoom(Item *item, Hotspot *droppedSpot) {
	if (GameState.getCurrentRoom() == kNorad01West) {
		if (!_fillingStationItem) {
			_fillingStationItem = item;
			((NoradAlphaFillingStation *)_currentInteraction)->newFillingItem(item);
		}
	}

	Norad::dropItemIntoRoom(item, droppedSpot);
}

void NoradAlpha::getClawInfo(HotSpotID &outSpotID, HotSpotID &prepSpotID, HotSpotID &clawControlSpotID, HotSpotID &pinchClawSpotID,
		HotSpotID &moveClawDownSpotID, HotSpotID &moveClawRightSpotID, HotSpotID &moveClawLeftSpotID, HotSpotID &moveClawUpSpotID,
		HotSpotID &clawCCWSpotID, HotSpotID &clawCWSpotID, uint32 &clawPosition, const uint32 *&clawExtraIDs) {
	outSpotID = kNorad22MonitorOutSpotID;
	prepSpotID = kNorad22LaunchPrepSpotID;
	clawControlSpotID = kNorad22ClawControlSpotID;
	pinchClawSpotID = kNorad22ClawPinchSpotID;
	moveClawDownSpotID = kNorad22ClawDownSpotID;
	moveClawRightSpotID = kNorad22ClawRightSpotID;
	moveClawLeftSpotID = kNorad22ClawLeftSpotID;
	moveClawUpSpotID = kNorad22ClawUpSpotID;
	clawCCWSpotID = kNorad22ClawCCWSpotID;
	clawCWSpotID = kNorad22ClawCWSpotID;
	clawPosition = kClawAtD;
	clawExtraIDs = _noradAlphaClawExtras;
}

Hotspot *NoradAlpha::getItemScreenSpot(Item *item, DisplayElement *element) {
	switch (item->getObjectID()) {
	case kGasCanister:
		return _vm->getAllHotspots().findHotspotByID(kN01GasCanisterSpotID);
	case kAirMask:
		return _vm->getAllHotspots().findHotspotByID(kN01AirMaskSpotID);
	case kArgonCanister:
		return _vm->getAllHotspots().findHotspotByID(kN01ArgonCanisterSpotID);
	case kNitrogenCanister:
		return _vm->getAllHotspots().findHotspotByID(kN01NitrogenCanisterSpotID);
	}

	return Norad::getItemScreenSpot(item, element);
}

Common::String NoradAlpha::getEnvScanMovie() {
	Common::String movieName = Neighborhood::getEnvScanMovie();

	if (movieName.empty()) {
		RoomID room = GameState.getCurrentRoom();
		if (room >= kNorad01 && room <= kNorad01West)
			return "Images/AI/Norad/XNE1";
		else if ((room >= kNorad02 && room <= kNorad19West))
			return "Images/AI/Norad/XNE2";

		return "Images/AI/Norad/XNE3";
	}

	return movieName;
}

uint NoradAlpha::getNumHints() {
	uint numHints = Neighborhood::getNumHints();

	if (numHints == 0) {
		switch (GameState.getCurrentRoomAndView()) {
		case MakeRoomView(kNorad01, kNorth):
		case MakeRoomView(kNorad01, kSouth):
		case MakeRoomView(kNorad01, kEast):
		case MakeRoomView(kNorad01, kWest):
		case MakeRoomView(kNorad01East, kEast):
		case MakeRoomView(kNorad01West, kWest):
			if (GameState.getNoradGassed()) {
				if (g_airMask->isAirFilterOn())
					numHints = 0;
				else
					numHints = 3;
			} else {
				numHints = 2;
			}
			break;
		case MakeRoomView(kNorad19West, kWest):
			if (getSubPrepFailed() && GameState.getNoradSubPrepState() != kSubPrepped)
				numHints = 1;
			break;
		case MakeRoomView(kNorad22, kWest):
			numHints = 1;
			break;
		}
	}

	return numHints;
}

Common::String NoradAlpha::getHintMovie(uint hintNum) {
	Common::String movieName = Neighborhood::getHintMovie(hintNum);

	if (movieName.empty()) {
		switch (GameState.getCurrentRoomAndView()) {
		case MakeRoomView(kNorad01, kNorth):
		case MakeRoomView(kNorad01, kSouth):
		case MakeRoomView(kNorad01, kEast):
		case MakeRoomView(kNorad01, kWest):
		case MakeRoomView(kNorad01East, kEast):
		case MakeRoomView(kNorad01West, kWest):
			switch (hintNum) {
			case 1:
				if (GameState.getNoradGassed())
					return "Images/AI/Norad/XN01SW";

				return "Images/AI/Norad/XN01WD2";
			case 2:
				if (GameState.getNoradGassed()) {
					if (_vm->playerHasItemID(kAirMask))
						// Mask must not be on if we get here...
						return "Images/AI/Globals/XGLOB1A";

					return "Images/AI/Globals/XGLOB3D";
				}

				return "Images/AI/Globals/XGLOB5C";
			case 3:
				return "Images/AI/Norad/XN01SH";
			}
			break;
		case MakeRoomView(kNorad19West, kWest):
			return "Images/AI/Norad/XN19NH";
		case MakeRoomView(kNorad22, kWest):
			return "Images/AI/Globals/XGLOB1C";
		}
	}

	return movieName;
}

void NoradAlpha::closeDoorOffScreen(const RoomID room, const DirectionConstant) {
	switch (room) {
	case kNorad12:
	case kNorad13:
	case kNorad18:
	case kNorad19:
		playSpotSoundSync(kAlphaElevatorDoorCloseIn, kAlphaElevatorDoorCloseOut);
		break;
	default:
		playSpotSoundSync(kAlphaRegDoorCloseIn, kAlphaRegDoorCloseOut);
		break;
	}
}

void NoradAlpha::findSpotEntry(const RoomID room, const DirectionConstant direction, SpotFlags flags, SpotTable::Entry &spotEntry) {
	if (room == kNorad01 && direction == kSouth)
		spotEntry.clear();
	else
		Norad::findSpotEntry(room, direction, flags, spotEntry);
}

bool NoradAlpha::canSolve() {
	return Norad::canSolve() || getHintMovie(1) == "Images/AI/Norad/XN01SW";
}

void NoradAlpha::doSolve() {
	Norad::doSolve();

	if (getHintMovie(1) == "Images/AI/Norad/XN01SW") {
		_vm->addItemToInventory(g_airMask);
		g_airMask->putMaskOn();
	}
}

Common::String NoradAlpha::getNavMovieName() {
	return "Images/Norad Alpha/Norad Alpha.movie";
}

Common::String NoradAlpha::getSoundSpotsName() {
	return "Sounds/Norad/Norad Alpha Spots";
}

} // End of namespace Pegasus
