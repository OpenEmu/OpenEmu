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
#include "pegasus/interface.h"
#include "pegasus/pegasus.h"
#include "pegasus/ai/ai_area.h"
#include "pegasus/items/biochips/opticalchip.h"
#include "pegasus/items/biochips/retscanchip.h"
#include "pegasus/items/inventory/airmask.h"
#include "pegasus/neighborhood/norad/constants.h"
#include "pegasus/neighborhood/norad/subcontrolroom.h"
#include "pegasus/neighborhood/norad/delta/globegame.h"
#include "pegasus/neighborhood/norad/delta/noraddelta.h"

namespace Pegasus {

const uint32 NoradDelta::_noradDeltaClawExtras[22] = {
	kN60ClawFromAToB,
	kN60ClawALoop,
	kN60ClawAPinch,
	kN60ClawACounterclockwise,
	kN60ClawAClockwise,
	kN60ClawFromBToA,
	kN60ClawFromBToC,
	kN60ClawFromBToD,
	kN60ClawBLoop,
	kN60ClawBPinch,
	kN60ClawBCounterclockwise,
	kN60ClawBClockwise,
	kN60ClawFromCToB,
	kN60ClawCLoop,
	kN60ClawCPinch,
	kN60ClawCCounterclockwise,
	kN60ClawCClockwise,
	kN60ClawFromDToB,
	kN60ClawDLoop,
	kN60ClawDPinch,
	kN60ClawDCounterclockwise,
	kN60ClawDClockwise
};

NoradDelta::NoradDelta(InputHandler *nextHandler, PegasusEngine *owner) : Norad(nextHandler, owner, "Norad Delta", kNoradDeltaID) {
	_elevatorUpRoomID = kNorad49South;
	_elevatorDownRoomID = kNorad48South;
	_elevatorUpSpotID = kNorad48ElevatorUpSpotID;
	_elevatorDownSpotID = kNorad49ElevatorDownSpotID;

	// Pressure door stuff.

	_subRoomEntryRoom1 = kNorad50;
	_subRoomEntryDir1 = kEast;
	_subRoomEntryRoom2 = kNorad59;
	_subRoomEntryDir2 = kWest;
	_upperPressureDoorRoom = kNorad50East;
	_lowerPressureDoorRoom = kNorad59West;

	_upperPressureDoorUpSpotID = kDeltaUpperPressureDoorUpSpotID;
	_upperPressureDoorDownSpotID = kDeltaUpperPressureDoorDownSpotID;
	_upperPressureDoorAbortSpotID = kNorad50DoorOutSpotID;

	_lowerPressureDoorUpSpotID = kDeltaLowerPressureDoorUpSpotID;
	_lowerPressureDoorDownSpotID = kDeltaLowerPressureDoorDownSpotID;
	_lowerPressureDoorAbortSpotID = kNorad59WestOutSpotID;

	_pressureSoundIn = kPressureDoorIntro1In;
	_pressureSoundOut = kPressureDoorIntro1Out;
	_equalizeSoundIn = kPressureDoorIntro2In;
	_equalizeSoundOut = kPressureDoorIntro2Out;
	_accessDeniedIn = kDeltaAccessDeniedIn;
	_accessDeniedOut = kDeltaAccessDeniedOut;

	GameState.setNoradSubPrepState(kSubDamaged);

	_subControlRoom = kNorad60West;
}

void NoradDelta::init() {
	Norad::init();

	// Little fix for the retinal scan zoom in spot...
	Hotspot *hotspot = _vm->getAllHotspots().findHotspotByID(kNorad68WestSpotID);
	hotspot->setMaskedHotspotFlags(kZoomInSpotFlag, kZoomInSpotFlag | kZoomOutSpotFlag);

	hotspot = _vm->getAllHotspots().findHotspotByID(kNorad79WestSpotID);
	hotspot->setMaskedHotspotFlags(kZoomInSpotFlag, kZoomInSpotFlag | kZoomOutSpotFlag);

	hotspot = _vm->getAllHotspots().findHotspotByID(kDelta59RobotShieldBiochipSpotID);
	hotspot->setMaskedHotspotFlags(kPickUpBiochipSpotFlag, kPickUpBiochipSpotFlag);
	HotspotInfoTable::Entry *hotspotEntry = findHotspotEntry(kDelta59RobotShieldBiochipSpotID);
	hotspotEntry->hotspotItem = kShieldBiochip;

	hotspot = _vm->getAllHotspots().findHotspotByID(kDelta59RobotOpMemBiochipSpotID);
	hotspot->setMaskedHotspotFlags(kPickUpBiochipSpotFlag, kPickUpBiochipSpotFlag);
	hotspotEntry = findHotspotEntry(kDelta59RobotOpMemBiochipSpotID);
	hotspotEntry->hotspotItem = kOpticalBiochip;

	hotspot = _vm->getAllHotspots().findHotspotByID(kDelta59RobotRetinalBiochipSpotID);
	hotspot->setMaskedHotspotFlags(kPickUpBiochipSpotFlag, kPickUpBiochipSpotFlag);
	hotspotEntry = findHotspotEntry(kDelta59RobotRetinalBiochipSpotID);
	hotspotEntry->hotspotItem = kRetinalScanBiochip;

	hotspot = _vm->getAllHotspots().findHotspotByID(kDelta60RobotShieldBiochipSpotID);
	hotspot->setMaskedHotspotFlags(kPickUpBiochipSpotFlag, kPickUpBiochipSpotFlag);
	hotspotEntry = findHotspotEntry(kDelta60RobotShieldBiochipSpotID);
	hotspotEntry->hotspotItem = kShieldBiochip;

	hotspot = _vm->getAllHotspots().findHotspotByID(kDelta60RobotOpMemBiochipSpotID);
	hotspot->setMaskedHotspotFlags(kPickUpBiochipSpotFlag, kPickUpBiochipSpotFlag);
	hotspotEntry = findHotspotEntry(kDelta60RobotOpMemBiochipSpotID);
	hotspotEntry->hotspotItem = kOpticalBiochip;

	hotspot = _vm->getAllHotspots().findHotspotByID(kDelta60RobotRetinalBiochipSpotID);
	hotspot->setMaskedHotspotFlags(kPickUpBiochipSpotFlag, kPickUpBiochipSpotFlag);
	hotspotEntry = findHotspotEntry(kDelta60RobotRetinalBiochipSpotID);
	hotspotEntry->hotspotItem = kRetinalScanBiochip;
}

void NoradDelta::start() {
	if (g_energyMonitor) {
		g_energyMonitor->stopEnergyDraining();
		g_energyMonitor->restoreLastEnergyValue();
		_vm->resetEnergyDeathReason();
		g_energyMonitor->startEnergyDraining();
	}

	Norad::start();
}

void NoradDelta::setUpAIRules() {
	Neighborhood::setUpAIRules();

	if (g_AIArea) {
		AIPlayMessageAction *messageAction = new AIPlayMessageAction("Images/AI/Norad/XN07NE", false);
		AILocationCondition *locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kNorad68, kWest));
		AIRule *rule = new AIRule(locCondition, messageAction);
		g_AIArea->addAIRule(rule);
	}
}

void NoradDelta::getExtraCompassMove(const ExtraTable::Entry &entry, FaderMoveSpec &compassMove) {
	switch (entry.extra) {
	case kArriveFromSubChase:
		compassMove.makeTwoKnotFaderSpec(kNoradDeltaMovieScale, entry.movieStart, 20, entry.movieEnd, 90);
		compassMove.insertFaderKnot(entry.movieStart + 25 * kNoradDeltaFrameDuration, 20);
		compassMove.insertFaderKnot(entry.movieStart + 94 * kNoradDeltaFrameDuration, 45);
		compassMove.insertFaderKnot(entry.movieStart + 101 * kNoradDeltaFrameDuration, 45);
		compassMove.insertFaderKnot(entry.movieStart + 146 * kNoradDeltaFrameDuration, 90 + 15);
		compassMove.insertFaderKnot(entry.movieStart + 189 * kNoradDeltaFrameDuration, 90 + 15);
		compassMove.insertFaderKnot(entry.movieStart + 204 * kNoradDeltaFrameDuration, 90 + 30);
		compassMove.insertFaderKnot(entry.movieStart + 214 * kNoradDeltaFrameDuration, 90 + 20);
		compassMove.insertFaderKnot(entry.movieStart + 222 * kNoradDeltaFrameDuration, 90 + 20);
		compassMove.insertFaderKnot(entry.movieStart + 228 * kNoradDeltaFrameDuration, 90 + 10);
		compassMove.insertFaderKnot(entry.movieStart + 245 * kNoradDeltaFrameDuration, 90 + 85);
		compassMove.insertFaderKnot(entry.movieStart + 262 * kNoradDeltaFrameDuration, 90 + 70);
		compassMove.insertFaderKnot(entry.movieStart + 273 * kNoradDeltaFrameDuration, 90 + 80);
		compassMove.insertFaderKnot(entry.movieStart + 287 * kNoradDeltaFrameDuration, 90);
		break;
	case kN60PlayerFollowsRobotToDoor:
		compassMove.makeTwoKnotFaderSpec(kNoradDeltaMovieScale, entry.movieStart, 270 + kSubControlCompassAngle,
				entry.movieEnd, 270 - 15);
		compassMove.insertFaderKnot(entry.movieStart + 280, 270 + kSubControlCompassAngle);
		compassMove.insertFaderKnot(entry.movieStart + 920, 360);
		compassMove.insertFaderKnot(entry.movieStart + 1840, 360);
		compassMove.insertFaderKnot(entry.movieStart + 2520, 270);
		compassMove.insertFaderKnot(entry.movieStart + 3760, 270);
		compassMove.insertFaderKnot(entry.movieStart + 4640, 270 + kSubControlCompassAngle);
		break;
	case kN59PlayerWins2:
		compassMove.makeTwoKnotFaderSpec(kNoradDeltaMovieScale, entry.movieStart, 270, entry.movieEnd, 280);
		compassMove.insertFaderKnot(entry.movieEnd - 1000, 270);
	default:
		Norad::getExtraCompassMove(entry, compassMove);
		break;
	}
}

GameInteraction *NoradDelta::makeInteraction(const InteractionID interactionID) {
	if (interactionID == kNoradGlobeGameInteractionID)
		return new GlobeGame(this);

	return Norad::makeInteraction(interactionID);
}

void NoradDelta::playClawMonitorIntro() {
	playSpotSoundSync(kLoadClawIntroIn, kLoadClawIntroOut);
}

void NoradDelta::getExitEntry(const RoomID room, const DirectionConstant direction, ExitTable::Entry &entry) {
	Norad::getExitEntry(room, direction, entry);

	if (room == kNorad61 && direction == kSouth)
		entry.movieStart += kNoradDeltaFrameDuration;
}

void NoradDelta::getZoomEntry(const HotSpotID id, ZoomTable::Entry &zoomEntry) {
	Norad::getZoomEntry(id, zoomEntry);

	if (id == kNorad59WestSpotID && GameState.getNoradPlayedGlobeGame()) {
		ExtraTable::Entry extraEntry;
		getExtraEntry(kN59ZoomWithRobot, extraEntry);
		zoomEntry.movieStart = extraEntry.movieStart;
		zoomEntry.movieEnd = extraEntry.movieEnd;
	}
}

void NoradDelta::loadAmbientLoops() {
/*
	Logic:

	loop sound 1:
		if room == kNorad79West
			if player globe game
				play kNoradGlobeLoop2SoundNum
			else
				play kNoradRedAlertLoopSoundNum
		else if room >= kNorad78 && room <= kNorad79
			play kNoradGlobeLoop2SoundNum
		else if gassed,
			if room >= kNorad41 && room <= kNorad49South
				play kNoradNewSubLoopSoundNum, kNoradWarningVolume
			else if room >= kNorad59 && room <= kNorad60West
				play kNoradSubControlLoopSoundNum, kNoradWarningVolume
			else
				play kNoradWarningLoopSoundNum, kNoradWarningVolume
		else
			play nothing
	loop sound 2:
		if gassed and not wearing air mask
			if room == kNorad54North
				play breathing unmanned loop
			else
				play breathing
		else
			if room == kNorad54North
				play unmanned loop
			else
				play nothing
*/

	if (GameState.getNoradArrivedFromSub()) {
		RoomID room = GameState.getCurrentRoom();

		if (room == kNorad79West) {
			if (_privateFlags.getFlag(kNoradPrivateFinishedGlobeGameFlag))
				loadLoopSound1("Sounds/Norad/GlobAmb2.22K.AIFF");
			else
				loadLoopSound1("Sounds/Norad/RedAlert.22K.AIFF");
		} else if (room >= kNorad78 && room <= kNorad79) {
			// clone2727 says: This looks like it should be loadLoopSound1...
			loadLoopSound2("Sounds/Norad/RedAlert.22K.AIFF");
		} else if (GameState.getNoradGassed()) {
			if (room >= kNorad41 && room <= kNorad49South)
				loadLoopSound1("Sounds/Norad/NEW SUB AMB.22K.AIFF", kNoradWarningVolume * 3);
			else if (room >= kNorad59 && room <= kNorad60West)
				loadLoopSound1("Sounds/Norad/SUB CONTRL LOOP.22K.AIFF", kNoradWarningVolume * 3);
			else
				loadLoopSound1("Sounds/Norad/WARNING LOOP.22K.AIFF", kNoradWarningVolume);
		} else {
			loadLoopSound1("");
		}

		if (GameState.getNoradGassed() && !g_airMask->isAirFilterOn()) {
			if (room == kNorad54North)
				loadLoopSound2("Sounds/Norad/Breathing Typing.22K.AIFF", 0x100 / 2);
			else
				loadLoopSound2("Sounds/Norad/SUCKING WIND.22K.AIFF", kNoradSuckWindVolume, 0, 0);
		} else {
			if (room == kNorad54North)
				loadLoopSound2("Sounds/Norad/N54NAS.22K.AIFF", 0x100 / 2);
			else
				loadLoopSound2("");
		}
	} else {
		// Start them off at zero...
		if (GameState.getNoradGassed())
			loadLoopSound1("Sounds/Norad/NEW SUB AMB.22K.AIFF", 0, 0, 0);
		if (!g_airMask->isAirFilterOn())
			loadLoopSound2("Sounds/Norad/SUCKING WIND.22K.AIFF", 0, 0, 0);
	}
}

void NoradDelta::checkContinuePoint(const RoomID room, const DirectionConstant direction) {
	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kNorad41, kEast):
	case MakeRoomView(kNorad49, kEast):
	case MakeRoomView(kNorad49, kWest):
	case MakeRoomView(kNorad61, kSouth):
	case MakeRoomView(kNorad68, kEast):
	case MakeRoomView(kNorad79, kWest):
		makeContinuePoint();
		break;
	}
}

void NoradDelta::arriveAt(const RoomID room, const DirectionConstant direction) {
	if (room != kNorad68)
		GameState.setNoradRetScanGood(false);

	Norad::arriveAt(room, direction);

	FaderMoveSpec loop1Spec, loop2Spec;
	ExtraTable::Entry entry;

	switch (room) {
	case kNorad41:
		if (direction == kEast && !GameState.getNoradArrivedFromSub()) {
			GameState.setNoradPlayedGlobeGame(false);

			GameState.setNoradBeatRobotWithClaw(false);
			GameState.setNoradBeatRobotWithDoor(false);
			GameState.setNoradRetScanGood(false);

			GameState.setScoringExitedSub(true);

			getExtraEntry(kArriveFromSubChase, entry);

			loop1Spec.makeTwoKnotFaderSpec(kNoradDeltaMovieScale, 0, 0, entry.movieEnd -
					entry.movieStart, kNoradWarningVolume);
			loop1Spec.insertFaderKnot(7320, 0);
			loop1Spec.insertFaderKnot(7880, kNoradWarningVolume);

			loop2Spec.makeTwoKnotFaderSpec(kNoradDeltaMovieScale, 0, 0, entry.movieEnd -
					entry.movieStart, kNoradSuckWindVolume);
			loop1Spec.insertFaderKnot(7320, 0);
			loop1Spec.insertFaderKnot(7880, kNoradSuckWindVolume);

			startExtraSequence(kArriveFromSubChase, kExtraCompletedFlag, kFilterNoInput);

			startLoop1Fader(loop1Spec);
			startLoop2Fader(loop2Spec);
		}
		break;
	case kNorad54North:
		GameState.setScoringSawRobotAt54North(true);
		break;
	case kNorad68:
		if (GameState.getNoradRetScanGood())
			openDoor();
		break;
	case kNorad68West:
		arriveAtNorad68West();
		break;
	case kNorad79West:
		arriveAtNorad79West();
		break;
	default:
		break;
	}
}

void NoradDelta::doorOpened() {
	Norad::doorOpened();
	GameState.setNoradRetScanGood(false);
}

void NoradDelta::arriveAtNorad68West() {
	playSpotSoundSync(kHoldForRetinalIn, kHoldForRetinalOut);

	BiochipItem *retScan = _vm->getCurrentBiochip();

	if (retScan != 0 && retScan->getObjectID() == kRetinalScanBiochip) {
		((RetScanChip *)retScan)->searchForLaser();
		succeedRetinalScan();
	} else {
		failRetinalScan();
	}
}

void NoradDelta::arriveAtNorad79West() {
	if (!GameState.getNoradPlayedGlobeGame())
		newInteraction(kNoradGlobeGameInteractionID);
}

void NoradDelta::bumpIntoWall() {
	requestSpotSound(kDeltaBumpIntoWallIn, kDeltaBumpIntoWallOut, kFilterNoInput, 0);
	Neighborhood::bumpIntoWall();
}

void NoradDelta::failRetinalScan() {
	startExtraSequence(kNoradDeltaRetinalScanBad, kExtraCompletedFlag, kFilterNoInput);
}

void NoradDelta::succeedRetinalScan() {
	startExtraSequence(kNoradDeltaRetinalScanGood, kExtraCompletedFlag, kFilterNoInput);
	GameState.setNoradRetScanGood(true);
	GameState.setScoringUsedRetinalChip(true);
}

void NoradDelta::getDoorEntry(const RoomID room, const DirectionConstant direction, DoorTable::Entry &entry) {
	Norad::getDoorEntry(room, direction, entry);

	if (room == kNorad68 && direction == kWest && !GameState.getNoradRetScanGood())
		entry.flags = kDoorPresentMask | kDoorLockedMask;
}

void NoradDelta::finishedGlobeGame() {
	GameState.setNoradPlayedGlobeGame(true);
	_privateFlags.setFlag(kNoradPrivateFinishedGlobeGameFlag, true);
	GameState.setScoringFinishedGlobeGame(true);
	loadAmbientLoops();
	g_AIArea->playAIMovie(kRightAreaSignature, "Images/AI/Norad/XN60WD1", false, kWarningInterruption);
}

bool NoradDelta::playingAgainstRobot() {
	return GameState.getNoradPlayedGlobeGame();
}

void NoradDelta::getClawInfo(HotSpotID &outSpotID, HotSpotID &prepSpotID, HotSpotID &clawControlSpotID, HotSpotID &pinchClawSpotID,
		HotSpotID &moveClawDownSpotID, HotSpotID &moveClawRightSpotID, HotSpotID &moveClawLeftSpotID, HotSpotID &moveClawUpSpotID,
		HotSpotID &clawCCWSpotID, HotSpotID &clawCWSpotID, uint32 &clawPosition, const uint32 *&clawExtraIDs) {
	outSpotID = kNorad60MonitorOutSpotID;
	prepSpotID = kNorad60LaunchPrepSpotID;
	clawControlSpotID = kNorad60ClawControlSpotID;
	pinchClawSpotID = kNorad60ClawPinchSpotID;
	moveClawDownSpotID = kNorad60ClawDownSpotID;
	moveClawRightSpotID = kNorad60ClawRightSpotID;
	moveClawLeftSpotID = kNorad60ClawLeftSpotID;
	moveClawUpSpotID = kNorad60ClawUpSpotID;
	clawCCWSpotID = kNorad60ClawCCWSpotID;
	clawCWSpotID = kNorad60ClawCWSpotID;
	clawPosition = kClawAtC;
	clawExtraIDs = _noradDeltaClawExtras;
}

void NoradDelta::playerBeatRobotWithDoor() {
	GameState.setNoradBeatRobotWithDoor(true);
	updateViewFrame();
	GameState.setScoringStoppedNoradRobot(true);
	g_AIArea->playAIMovie(kRightAreaSignature, "Images/AI/Norad/XN59WD", false, kWarningInterruption);
}

void NoradDelta::playerBeatRobotWithClaw() {
	GameState.setNoradBeatRobotWithClaw(true);
	updateViewFrame();
	GameState.setScoringStoppedNoradRobot(true);
	GameState.setScoringNoradGandhi(true);
	g_AIArea->playAIMovie(kRightAreaSignature, "Images/AI/Norad/XN59WD", false, kWarningInterruption);
}

TimeValue NoradDelta::getViewTime(const RoomID room, const DirectionConstant direction) {
	ExtraTable::Entry entry;

	if (room == kNorad41 && direction == kSouth && !GameState.getNoradArrivedFromSub()) {
		getExtraEntry(kArriveFromSubChase, entry);
		return entry.movieStart;
	}

	if (GameState.getNoradBeatRobotWithDoor()) {
		if (_privateFlags.getFlag(kNoradPrivateRobotHeadOpenFlag)) {
			uint32 extraID = kN59Biochips111;
			if (_privateFlags.getFlag(kNoradPrivateGotShieldChipFlag))
				extraID += 1;
			if (_privateFlags.getFlag(kNoradPrivateGotOpticalChipFlag))
				extraID += 2;
			if (_privateFlags.getFlag(kNoradPrivateGotRetScanChipFlag))
				extraID += 4;
			getExtraEntry(extraID, entry);
			return entry.movieStart;
		}

		getExtraEntry(kN59RobotHeadOpens, entry);
		return entry.movieStart;
	} else if (GameState.getNoradBeatRobotWithClaw()) {
		if (_privateFlags.getFlag(kNoradPrivateRobotHeadOpenFlag)) {
			uint32 extraID = kN60Biochips111;
			if (_privateFlags.getFlag(kNoradPrivateGotShieldChipFlag))
				extraID += 1;
			if (_privateFlags.getFlag(kNoradPrivateGotOpticalChipFlag))
				extraID += 2;
			if (_privateFlags.getFlag(kNoradPrivateGotRetScanChipFlag))
				extraID += 4;
			getExtraEntry(extraID, entry);
			return entry.movieStart;
		}

		getExtraEntry(kN60RobotHeadOpens, entry);
		return entry.movieStart;
	}

	return Norad::getViewTime(room, direction);
}

void NoradDelta::openDoor() {
	if (GameState.getCurrentRoom() == kNorad59 && GameState.getCurrentDirection() == kWest && GameState.getNoradPlayedGlobeGame()) {
		Input scratch;
		InputHandler::_inputHandler->clickInHotspot(scratch, _vm->getAllHotspots().findHotspotByID(kNorad59WestSpotID));
	} else {
		Norad::openDoor();
	}
}

void NoradDelta::activateHotspots() {
	Norad::activateHotspots();

	if (GameState.getCurrentRoom() == kNorad59West && GameState.getCurrentDirection() == kWest && GameState.getNoradBeatRobotWithDoor()) {
		_vm->getAllHotspots().deactivateOneHotspot(kNorad59WestOutSpotID);

		if (_privateFlags.getFlag(kNoradPrivateRobotHeadOpenFlag)) {
			if (!_privateFlags.getFlag(kNoradPrivateGotShieldChipFlag))
				_vm->getAllHotspots().activateOneHotspot(kDelta59RobotShieldBiochipSpotID);
			else
				_vm->getAllHotspots().deactivateOneHotspot(kDelta59RobotShieldBiochipSpotID);

			if (!_privateFlags.getFlag(kNoradPrivateGotOpticalChipFlag))
				_vm->getAllHotspots().activateOneHotspot(kDelta59RobotOpMemBiochipSpotID);
			else
				_vm->getAllHotspots().deactivateOneHotspot(kDelta59RobotOpMemBiochipSpotID);

			if (!_privateFlags.getFlag(kNoradPrivateGotRetScanChipFlag))
				_vm->getAllHotspots().activateOneHotspot(kDelta59RobotRetinalBiochipSpotID);
			else
				_vm->getAllHotspots().deactivateOneHotspot(kDelta59RobotRetinalBiochipSpotID);
		} else
			_vm->getAllHotspots().activateOneHotspot(kDelta59RobotHeadSpotID);
	} else if (GameState.getCurrentRoom() == kNorad60West && GameState.getCurrentDirection() == kWest &&
			GameState.getNoradBeatRobotWithClaw()) {
		_vm->getAllHotspots().deactivateOneHotspot(kNorad60MonitorOutSpotID);

		if (_privateFlags.getFlag(kNoradPrivateRobotHeadOpenFlag)) {
			if (!_privateFlags.getFlag(kNoradPrivateGotShieldChipFlag))
				_vm->getAllHotspots().activateOneHotspot(kDelta60RobotShieldBiochipSpotID);
			else
				_vm->getAllHotspots().deactivateOneHotspot(kDelta60RobotShieldBiochipSpotID);

			if (!_privateFlags.getFlag(kNoradPrivateGotOpticalChipFlag))
				_vm->getAllHotspots().activateOneHotspot(kDelta60RobotOpMemBiochipSpotID);
			else
				_vm->getAllHotspots().deactivateOneHotspot(kDelta60RobotOpMemBiochipSpotID);

			if (!_privateFlags.getFlag(kNoradPrivateGotRetScanChipFlag))
				_vm->getAllHotspots().activateOneHotspot(kDelta60RobotRetinalBiochipSpotID);
			else
				_vm->getAllHotspots().deactivateOneHotspot(kDelta60RobotRetinalBiochipSpotID);
		} else {
			_vm->getAllHotspots().activateOneHotspot(kDelta60RobotHeadSpotID);
		}
	} else if (GameState.getCurrentRoomAndView() == MakeRoomView(kNorad50, kEast)) {
		if (GameState.isCurrentDoorOpen())
			_vm->getAllHotspots().deactivateOneHotspot(kNorad50DoorSpotID);
	} else if (GameState.getCurrentRoomAndView() == MakeRoomView(kNorad59, kWest)) {
		if (GameState.isCurrentDoorOpen())
			_vm->getAllHotspots().deactivateOneHotspot(kNorad59WestSpotID);
	}
}

void NoradDelta::clickInHotspot(const Input &input, const Hotspot *clickedSpot) {
	switch (clickedSpot->getObjectID()) {
	case kDelta59RobotHeadSpotID:
		startExtraSequence(kN59RobotHeadOpens, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kDelta60RobotHeadSpotID:
		startExtraSequence(kN60RobotHeadOpens, kExtraCompletedFlag, kFilterNoInput);
		break;
	default:
		Norad::clickInHotspot(input, clickedSpot);
		break;
	}
}

void NoradDelta::receiveNotification(Notification *notification, const NotificationFlags flags) {
	Norad::receiveNotification(notification, flags);

	if ((flags & kExtraCompletedFlag) != 0) {
		RetScanChip *retScan;
		Input dummy;

		switch (_lastExtra) {
		case kArriveFromSubChase:
			GameState.setNoradArrivedFromSub(true);
			GameState.setCurrentRoom(kNoRoomID);
			GameState.setCurrentDirection(kNoDirection);
			arriveAt(kNorad41, kEast);
			break;
		case kN59RobotHeadOpens:
		case kN60RobotHeadOpens:
			_privateFlags.setFlag(kNoradPrivateRobotHeadOpenFlag, true);
			break;
		case kNoradDeltaRetinalScanBad:
			retScan = (RetScanChip *)_vm->getCurrentBiochip();
			retScan->setItemState(kNormalItem);
			playSpotSoundSync(kRetinalScanFailedIn, kRetinalScanFailedOut);
			downButton(dummy);
			break;
		case kNoradDeltaRetinalScanGood:
			retScan = (RetScanChip *)_vm->getCurrentBiochip();
			retScan->setItemState(kNormalItem);
			downButton(dummy);
			break;
		case kN59RobotDisappears:
		case kN60RobotDisappears:
			recallToTSASuccess();
			break;
		}

		_interruptionFilter = kFilterAllInput;
	}

	g_AIArea->checkMiddleArea();
}

void NoradDelta::pickedUpItem(Item *item) {
	switch (item->getObjectID()) {
	case kShieldBiochip:
		if (_privateFlags.getFlag(kNoradPrivateGotShieldChipFlag) &&
					_privateFlags.getFlag(kNoradPrivateGotRetScanChipFlag) &&
					_privateFlags.getFlag(kNoradPrivateGotOpticalChipFlag)) {
			GameState.setNoradFinished(true);

			if (GameState.getCurrentRoom() == kNorad59West)
				startExtraSequence(kN59RobotDisappears, kExtraCompletedFlag, kFilterNoInput);
			else
				startExtraSequence(kN60RobotDisappears, kExtraCompletedFlag, kFilterNoInput);
		}
		break;
	case kRetinalScanBiochip:
		if (_privateFlags.getFlag(kNoradPrivateGotShieldChipFlag) &&
				_privateFlags.getFlag(kNoradPrivateGotRetScanChipFlag) &&
				_privateFlags.getFlag(kNoradPrivateGotOpticalChipFlag)) {
			GameState.setNoradFinished(true);

			if (GameState.getCurrentRoom() == kNorad59West)
				startExtraSequence(kN59RobotDisappears, kExtraCompletedFlag, kFilterNoInput);
			else
				startExtraSequence(kN60RobotDisappears, kExtraCompletedFlag, kFilterNoInput);
		}
		break;
	case kOpticalBiochip:
		g_opticalChip->addPoseidon();
		GameState.setScoringGotNoradOpMemChip();

		if (_privateFlags.getFlag(kNoradPrivateGotShieldChipFlag) &&
				_privateFlags.getFlag(kNoradPrivateGotRetScanChipFlag) &&
				_privateFlags.getFlag(kNoradPrivateGotOpticalChipFlag)) {
			GameState.setNoradFinished(true);

			if (GameState.getCurrentRoom() == kNorad59West)
				startExtraSequence(kN59RobotDisappears, kExtraCompletedFlag, kFilterNoInput);
			else
				startExtraSequence(kN60RobotDisappears, kExtraCompletedFlag, kFilterNoInput);
		}
		break;
	}

	Norad::pickedUpItem(item);
}

void NoradDelta::takeItemFromRoom(Item *item) {
	switch (item->getObjectID()) {
	case kShieldBiochip:
		_privateFlags.setFlag(kNoradPrivateGotShieldChipFlag, true);
		break;
	case kRetinalScanBiochip:
		_privateFlags.setFlag(kNoradPrivateGotRetScanChipFlag, true);
		break;
	case kOpticalBiochip:
		_privateFlags.setFlag(kNoradPrivateGotOpticalChipFlag, true);
		break;
	}

	Norad::takeItemFromRoom(item);
}

void NoradDelta::dropItemIntoRoom(Item *item, Hotspot *hotspot) {
	switch (item->getObjectID()) {
	case kShieldBiochip:
		_privateFlags.setFlag(kNoradPrivateGotShieldChipFlag, false);
		break;
	case kOpticalBiochip:
		_privateFlags.setFlag(kNoradPrivateGotOpticalChipFlag, false);
		break;
	case kRetinalScanBiochip:
		_privateFlags.setFlag(kNoradPrivateGotRetScanChipFlag, false);
		break;
	}

	Norad::dropItemIntoRoom(item, hotspot);
}

Hotspot *NoradDelta::getItemScreenSpot(Item *item, DisplayElement *element) {
	HotSpotID id = kNoHotSpotID;

	switch (item->getObjectID()) {
	case kShieldBiochip:
		if (GameState.getNoradBeatRobotWithDoor())
			id = kDelta59RobotShieldBiochipSpotID;
		else
			id = kDelta60RobotShieldBiochipSpotID;
		break;
	case kOpticalBiochip:
		if (GameState.getNoradBeatRobotWithDoor())
			id = kDelta59RobotOpMemBiochipSpotID;
		else
			id = kDelta60RobotOpMemBiochipSpotID;
		break;
	case kRetinalScanBiochip:
		if (GameState.getNoradBeatRobotWithDoor())
			id = kDelta59RobotRetinalBiochipSpotID;
		else
			id = kDelta60RobotRetinalBiochipSpotID;
		break;
	}

	if (id != kNoHotSpotID)
		return _vm->getAllHotspots().findHotspotByID(id);

	return Norad::getItemScreenSpot(item, element);
}

Common::String NoradDelta::getEnvScanMovie() {
	return "Images/AI/Norad/XNE2";
}

uint NoradDelta::getNumHints() {
	uint numHints = Neighborhood::getNumHints();

	if (numHints == 0) {
		switch (GameState.getCurrentRoomAndView()) {
		case MakeRoomView(kNorad60, kWest):
			if (GameState.getNoradPlayedGlobeGame())
				numHints = 2;
			else
				numHints = 1;
			break;
		case MakeRoomView(kNorad59, kNorth):
		case MakeRoomView(kNorad59, kSouth):
		case MakeRoomView(kNorad59, kEast):
		case MakeRoomView(kNorad59, kWest):
		case MakeRoomView(kNorad60, kNorth):
		case MakeRoomView(kNorad60, kSouth):
		case MakeRoomView(kNorad60, kEast):
			if (GameState.getNoradPlayedGlobeGame())
				numHints = 2;
			break;
		case MakeRoomView(kNorad68, kWest):
			if (_vm->playerHasItemID(kRetinalScanBiochip)) {
				BiochipItem *retScan = _vm->getCurrentBiochip();
				if (retScan == 0 || retScan->getObjectID() != kRetinalScanBiochip)
					numHints = 2;
			} else if (!GameState.isCurrentDoorOpen()) {
				numHints = 2;
			}
			break;
		}
	}

	return numHints;
}

Common::String NoradDelta::getHintMovie(uint hintNum) {
	Common::String movieName = Neighborhood::getHintMovie(hintNum);

	if (movieName.empty()) {
		switch (GameState.getCurrentRoomAndView()) {
		case MakeRoomView(kNorad60, kWest):
			if (GameState.getNoradPlayedGlobeGame()) {
				if (hintNum == 1)
					return "Images/AI/Norad/XN60WD2";

				return "Images/AI/Norad/XN60WD3";
			}

			return "Images/AI/Globals/XGLOB1C";
		case MakeRoomView(kNorad59, kNorth):
		case MakeRoomView(kNorad59, kSouth):
		case MakeRoomView(kNorad59, kEast):
		case MakeRoomView(kNorad59, kWest):
		case MakeRoomView(kNorad60, kNorth):
		case MakeRoomView(kNorad60, kSouth):
		case MakeRoomView(kNorad60, kEast):
			if (hintNum == 1)
				return "Images/AI/Norad/XN60WD2";

			return "Images/AI/Norad/XN60WD3";
		case MakeRoomView(kNorad68, kWest):
			if (_vm->playerHasItemID(kRetinalScanBiochip)) {
				if (hintNum == 1)
					return "Images/AI/Globals/XGLOB1A";

				return "Images/AI/Globals/XGLOB1C";
			}

			if (hintNum == 1)
				return "Images/AI/Globals/XGLOB1B";

			return "Images/AI/Globals/XGLOB3B";
		}
	}

	return movieName;
}

void NoradDelta::closeDoorOffScreen(const RoomID room, const DirectionConstant) {
	switch (room) {
	case kNorad47:
	case kNorad48:
	case kNorad41:
	case kNorad42:
		playSpotSoundSync(kDeltaElevatorDoorCloseIn, kDeltaElevatorDoorCloseOut);
		break;
	default:
		playSpotSoundSync(kDeltaRegDoorCloseIn, kDeltaRegDoorCloseOut);
		break;
	}
}

bool NoradDelta::canSolve() {
	if (Norad::canSolve())
		return true;

	if (GameState.getCurrentRoomAndView() == MakeRoomView(kNorad68, kWest)) {
		BiochipItem *biochip = _vm->getCurrentBiochip();
		if (biochip != 0 && biochip->getObjectID() != kRetinalScanBiochip)
			return true;
	}

	return false;
}

void NoradDelta::doSolve() {
	Norad::doSolve();

	if (GameState.getCurrentRoomAndView() == MakeRoomView(kNorad68, kWest)) {
		if (!_vm->playerHasItemID(kRetinalScanBiochip))
			_vm->addItemToBiochips((BiochipItem *)_vm->getAllItems().findItemByID(kRetinalScanBiochip));

		BiochipItem *biochip = _vm->getCurrentBiochip();
		if (biochip != 0 && biochip->getObjectID() != kRetinalScanBiochip && g_interface)
			g_interface->setCurrentBiochipID(kRetinalScanBiochip);

		Hotspot *spot = _vm->getAllHotspots().findHotspotByID(kNorad68WestSpotID);
		Input scratch;
		InputHandler::_inputHandler->clickInHotspot(scratch, spot);
	}
}

Common::String NoradDelta::getSoundSpotsName() {
	return "Sounds/Norad/Norad Delta Spots";
}

Common::String NoradDelta::getNavMovieName() {
	return "Images/Norad Delta/Norad Delta.movie";
}

} // End of namespace Pegasus
