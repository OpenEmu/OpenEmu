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
#include "pegasus/items/biochips/aichip.h"
#include "pegasus/items/biochips/opticalchip.h"
#include "pegasus/neighborhood/mars/constants.h"
#include "pegasus/neighborhood/norad/constants.h"
#include "pegasus/neighborhood/tsa/tinytsa.h"
#include "pegasus/neighborhood/wsc/wsc.h"

namespace Pegasus {

static const int16 kCompassShift = 30;

static const TimeScale kTinyTSAMovieScale = 600;
static const TimeScale kTinyTSAFramesPerSecond = 15;
static const TimeScale kTinyTSAFrameDuration = 40;

// Alternate IDs.
static const AlternateID kAltTinyTSANormal = 0;

// Hot Spot Activation IDs.
static const HotSpotActivationID kActivationTinyTSAJumpToNorad = 1;
static const HotSpotActivationID kActivationTinyTSAJumpToMars = 2;
static const HotSpotActivationID kActivationTinyTSAJumpToWSC = 3;
static const HotSpotActivationID kActivationTinyTSAReadyForJumpMenu = 4;
static const HotSpotActivationID kActivationTinyTSAMainJumpMenu = 5;

// Hot Spot IDs.
static const HotSpotID kTinyTSA37NorthJumpToNoradSpotID = 5000;
static const HotSpotID kTinyTSA37NorthCancelNoradSpotID = 5001;
static const HotSpotID kTinyTSA37NorthJumpToMarsSpotID = 5002;
static const HotSpotID kTinyTSA37NorthCancelMarsSpotID = 5003;
static const HotSpotID kTinyTSA37NorthJumpToWSCSpotID = 5004;
static const HotSpotID kTinyTSA37NorthCancelWSCSpotID = 5005;
static const HotSpotID kTinyTSA37NorthJumpMenuSpotID = 5006;
static const HotSpotID kTinyTSA37NorthNoradMenuSpotID = 5007;
static const HotSpotID kTinyTSA37NorthMarsMenuSpotID = 5008;
static const HotSpotID kTinyTSA37NorthWSCMenuSpotID = 5009;

// Extra sequence IDs.
static const ExtraID kTinyTSA37PegasusDepart = 0;
static const ExtraID kTinyTSA37TimeJumpToPegasus = 1;
static const ExtraID kTinyTSA37RecallToDownload = 2;
static const ExtraID kTinyTSA37ExitHilited = 3;
static const ExtraID kTinyTSA37ExitToHorse = 4;
static const ExtraID kTinyTSA37JumpMenu000 = 5;
static const ExtraID kTinyTSA37JumpMenu001 = 6;
static const ExtraID kTinyTSA37JumpMenu010 = 7;
static const ExtraID kTinyTSA37JumpMenu011 = 8;
static const ExtraID kTinyTSA37JumpMenu100 = 9;
static const ExtraID kTinyTSA37JumpMenu101 = 10;
static const ExtraID kTinyTSA37JumpMenu110 = 11;
static const ExtraID kTinyTSA37JumpMenu111 = 12;
static const ExtraID kTinyTSA37JumpToWSCMenu = 13;
static const ExtraID kTinyTSA37CancelWSC = 14;
static const ExtraID kTinyTSA37JumpToWSC = 15;
static const ExtraID kTinyTSA37WSCToAI5 = 16;
static const ExtraID kTinyTSA37PegasusAI5 = 17;
static const ExtraID kTinyTSA37AI5ToWSC = 18;
static const ExtraID kTinyTSA37WSCToDepart = 19;
static const ExtraID kTinyTSA37JumpToMarsMenu = 20;
static const ExtraID kTinyTSA37CancelMars = 21;
static const ExtraID kTinyTSA37JumpToMars = 22;
static const ExtraID kTinyTSA37MarsToAI6 = 23;
static const ExtraID kTinyTSA37PegasusAI6 = 24;
static const ExtraID kTinyTSA37AI6ToMars = 25;
static const ExtraID kTinyTSA37MarsToDepart = 26;
static const ExtraID kTinyTSA37JumpToNoradMenu = 27;
static const ExtraID kTinyTSA37CancelNorad = 28;
static const ExtraID kTinyTSA37JumpToNorad = 29;
static const ExtraID kTinyTSA37NoradToAI7 = 30;
static const ExtraID kTinyTSA37PegasusAI7 = 31;
static const ExtraID kTinyTSA37AI7ToNorad = 32;
static const ExtraID kTinyTSA37NoradToDepart = 33;
static const ExtraID kTinyTSA37EnvironmentalScan = 34;
static const ExtraID kTinyTSA37DownloadToMainMenu = 35;
static const ExtraID kTinyTSA37DownloadToOpMemReview = 36;
static const ExtraID kTinyTSA37OpMemReviewToMainMenu = 37;

TinyTSA::TinyTSA(InputHandler *nextHandler, PegasusEngine *owner) : Neighborhood(nextHandler, owner, "Tiny TSA", kTinyTSAID) {
}

void TinyTSA::start() {
	g_energyMonitor->stopEnergyDraining();
	Neighborhood::start();
}

Common::String TinyTSA::getBriefingMovie() {
	Common::String movieName = Neighborhood::getBriefingMovie();

	if (movieName.empty()) {
		switch (getCurrentActivation()) {
		case kActivationTinyTSAJumpToNorad:
			g_AIChip->showBriefingClicked();
			startExtraSequenceSync(kTinyTSA37PegasusAI7, kHintInterruption);
			startExtraSequenceSync(kTinyTSA37AI7ToNorad, kFilterNoInput);
			g_AIChip->clearClicked();
			movieName = "";
			break;
		case kActivationTinyTSAJumpToMars:
			g_AIChip->showBriefingClicked();
			startExtraSequenceSync(kTinyTSA37PegasusAI6, kHintInterruption);
			startExtraSequenceSync(kTinyTSA37AI6ToMars, kFilterNoInput);
			g_AIChip->clearClicked();
			movieName = "";
			break;
		case kActivationTinyTSAJumpToWSC:
			g_AIChip->showBriefingClicked();
			startExtraSequenceSync(kTinyTSA37PegasusAI5, kHintInterruption);
			startExtraSequenceSync(kTinyTSA37AI5ToWSC, kFilterNoInput);
			g_AIChip->clearClicked();
			movieName = "";
			break;
		default:
			movieName = "Images/AI/TSA/XT04";
			break;
		}
	}

	return movieName;
}

Common::String TinyTSA::getEnvScanMovie() {
	Common::String movieName = Neighborhood::getEnvScanMovie();

	if (movieName.empty()) {
		g_AIChip->showEnvScanClicked();
		startExtraSequenceSync(kTinyTSA37EnvironmentalScan, kHintInterruption);

		switch (getCurrentActivation()) {
		case kActivationTinyTSAJumpToNorad:
			startExtraSequenceSync(kTinyTSA37AI7ToNorad, kFilterNoInput);
			showExtraView(kTinyTSA37JumpToNoradMenu);
			break;
		case kActivationTinyTSAJumpToMars:
			startExtraSequenceSync(kTinyTSA37AI6ToMars, kFilterNoInput);
			showExtraView(kTinyTSA37JumpToMarsMenu);
			break;
		case kActivationTinyTSAJumpToWSC:
			startExtraSequenceSync(kTinyTSA37AI5ToWSC, kFilterNoInput);
			showExtraView(kTinyTSA37JumpToWSCMenu);
			break;
		default:
			showMainJumpMenu();
			break;
		}

		g_AIChip->clearClicked();
	}

	return movieName;
}

void TinyTSA::loadAmbientLoops() {
	loadLoopSound1("Sounds/TSA/T01NAE.NEW.22K.AIFF");
}

int16 TinyTSA::getStaticCompassAngle(const RoomID room, const DirectionConstant dir) {
	return Neighborhood::getStaticCompassAngle(room, dir) - kCompassShift;
}

uint16 TinyTSA::getDateResID() const {
	return kDate2318ID;
}

InputBits TinyTSA::getInputFilter() {
	// Can't move forward...
	return Neighborhood::getInputFilter() & ~(kFilterUpButton | kFilterUpAuto);
}

void TinyTSA::clickInHotspot(const Input &input, const Hotspot *clickedSpot) {
	if (clickedSpot) {
		switch (clickedSpot->getObjectID()) {
		case kTinyTSA37NorthJumpMenuSpotID:
			// This hotspot isn't accessable from Tiny TSA
			warning("jump menu spot");
			return;
		case kTinyTSA37NorthJumpToNoradSpotID:
			GameState.setTSAState(kPlayerOnWayToNorad);
			requestExtraSequence(kTinyTSA37JumpToNorad, 0, kFilterNoInput);
			if (!GameState.getBeenToNorad()) {
				requestExtraSequence(kTinyTSA37NoradToAI7, 0, kFilterNoInput);
				requestExtraSequence(kTinyTSA37PegasusAI7, 0, kFilterNoInput);
				requestExtraSequence(kTinyTSA37AI7ToNorad, 0, kFilterNoInput);
				GameState.setBeenToNorad(true);
			}

			requestExtraSequence(kTinyTSA37NoradToDepart, 0, kFilterNoInput);
			requestExtraSequence(kTinyTSA37PegasusDepart, kExtraCompletedFlag, kFilterNoInput);
			return;
		case kTinyTSA37NorthJumpToMarsSpotID:
			GameState.setTSAState(kPlayerOnWayToMars);
			requestExtraSequence(kTinyTSA37JumpToMars, 0, kFilterNoInput);
			if (!GameState.getBeenToMars()) {
				requestExtraSequence(kTinyTSA37MarsToAI6, 0, kFilterNoInput);
				requestExtraSequence(kTinyTSA37PegasusAI6, 0, kFilterNoInput);
				requestExtraSequence(kTinyTSA37AI6ToMars, 0, kFilterNoInput);
				GameState.setBeenToMars(true);
			}

			requestExtraSequence(kTinyTSA37MarsToDepart, 0, kFilterNoInput);
			requestExtraSequence(kTinyTSA37PegasusDepart, kExtraCompletedFlag, kFilterNoInput);
			return;
		case kTinyTSA37NorthJumpToWSCSpotID:
			GameState.setTSAState(kPlayerOnWayToWSC);
			requestExtraSequence(kTinyTSA37JumpToWSC, 0, kFilterNoInput);
			if (!GameState.getBeenToWSC()) {
				requestExtraSequence(kTinyTSA37WSCToAI5, 0, kFilterNoInput);
				requestExtraSequence(kTinyTSA37PegasusAI5, 0, kFilterNoInput);
				requestExtraSequence(kTinyTSA37AI5ToWSC, 0, kFilterNoInput);
				GameState.setBeenToWSC(true);
			}

			requestExtraSequence(kTinyTSA37WSCToDepart, 0, kFilterNoInput);
			requestExtraSequence(kTinyTSA37PegasusDepart, kExtraCompletedFlag, kFilterNoInput);
			return;
		}
	}

	Neighborhood::clickInHotspot(input, clickedSpot);
}

void TinyTSA::showMainJumpMenu() {
	ExtraID jumpMenuView = kTinyTSA37JumpMenu000;

	if (GameState.getNoradFinished())
		jumpMenuView += 4;
	if (GameState.getMarsFinished())
		jumpMenuView += 2;
	if (GameState.getWSCFinished())
		jumpMenuView += 1;

	showExtraView(jumpMenuView);
	setCurrentActivation(kActivationTinyTSAMainJumpMenu);
}

void TinyTSA::checkContinuePoint(const RoomID, const DirectionConstant) {
	makeContinuePoint();
}

void TinyTSA::arriveAt(const RoomID room, const DirectionConstant direction) {
	Neighborhood::arriveAt(room, direction);

	switch (GameState.getTSAState()) {
	case kPlayerOnWayToNorad:
	case kPlayerOnWayToMars:
	case kPlayerOnWayToWSC:
		startExtraSequence(kTinyTSA37TimeJumpToPegasus, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kPlayerLockedInPegasus:
		showMainJumpMenu();
		break;
	}
}

void TinyTSA::receiveNotification(Notification *notification, const NotificationFlags flags) {
	ExtraID lastExtra = _lastExtra;

	Neighborhood::receiveNotification(notification, flags);

	if ((flags & kExtraCompletedFlag) != 0) {
		// Only allow input if we're not in the middle of series of queue requests.
		if (actionQueueEmpty())
			_interruptionFilter = kFilterAllInput;

		switch (lastExtra) {
		case kTinyTSA37PegasusDepart:
			_vm->setLastEnergyValue(kFullEnergy);

			switch (GameState.getTSAState()) {
			case kPlayerOnWayToNorad:
				_vm->jumpToNewEnvironment(kNoradAlphaID, kNorad01, kSouth);
				GameState.setNoradSeenTimeStream(false);
				GameState.setNoradGassed(true);
				GameState.setNoradFillingStationOn(false);
				GameState.setNoradN22MessagePlayed(false);
				GameState.setNoradPlayedGlobeGame(false);
				GameState.setNoradBeatRobotWithClaw(false);
				GameState.setNoradBeatRobotWithDoor(false);
				GameState.setNoradRetScanGood(false);
				GameState.setNoradWaitingForLaser(false);
				GameState.setNoradSubRoomPressure(9);
				GameState.setNoradSubPrepState(kSubNotPrepped);
				break;
			case kPlayerOnWayToMars:
				_vm->jumpToNewEnvironment(kMarsID, kMars0A, kNorth);
				GameState.setMarsSeenTimeStream(false);
				GameState.setMarsHeardUpperPodMessage(false);
				GameState.setMarsRobotThrownPlayer(false);
				GameState.setMarsHeardCheckInMessage(false);
				GameState.setMarsPodAtUpperPlatform(false);
				GameState.setMarsSeenThermalScan(false);
				GameState.setMarsArrivedBelow(false);
				GameState.setMarsSeenRobotAtReactor(false);
				GameState.setMarsAvoidedReactorRobot(false);
				GameState.setMarsLockFrozen(false);
				GameState.setMarsLockBroken(false);
				GameState.setMarsSecurityDown(false);
				GameState.setMarsAirlockOpen(false);
				GameState.setMarsReadyForShuttleTransport(false);
				GameState.setMarsFinishedCanyonChase(false);
				GameState.setMarsThreadedMaze(false);
				break;
			case kPlayerOnWayToWSC:
				_vm->jumpToNewEnvironment(kWSCID, kWSC01, kWest);
				GameState.setWSCSeenTimeStream(false);
				GameState.setWSCPoisoned(false);
				GameState.setWSCAnsweredAboutDart(false);
				GameState.setWSCDartInAnalyzer(false);
				GameState.setWSCRemovedDart(false);
				GameState.setWSCAnalyzerOn(false);
				GameState.setWSCAnalyzedDart(false);
				GameState.setWSCPickedUpAntidote(false);
				GameState.setWSCSawMorph(false);
				GameState.setWSCDesignedAntidote(false);
				GameState.setWSCOfficeMessagesOpen(false);
				GameState.setWSCSeenNerd(false);
				GameState.setWSCHeardPage1(false);
				GameState.setWSCHeardPage2(false);
				GameState.setWSCHeardCheckIn(false);
				GameState.setWSCDidPlasmaDodge(false);
				GameState.setWSCSeenSinclairLecture(false);
				GameState.setWSCBeenAtWSC93(false);
				GameState.setWSCCatwalkDark(false);
				GameState.setWSCRobotDead(false);
				GameState.setWSCRobotGone(false);
				break;
			};
			break;
		case kTinyTSA37TimeJumpToPegasus:
			if (g_energyMonitor)
				g_energyMonitor->stopEnergyDraining();

			switch (GameState.getTSAState()) {
			case kPlayerOnWayToNorad:
				arriveFromNorad();
				break;
			case kPlayerOnWayToMars:
				arriveFromMars();
				break;
			case kPlayerOnWayToWSC:
				arriveFromWSC();
				break;
			default:
				break;
			}
			break;
		case kTinyTSA37DownloadToOpMemReview:
			switch (GameState.getTSAState()) {
			case kPlayerOnWayToNorad:
				g_opticalChip->playOpMemMovie(kPoseidonSpotID);
				break;
			case kPlayerOnWayToMars:
				g_opticalChip->playOpMemMovie(kAriesSpotID);
				break;
			case kPlayerOnWayToWSC:
				g_opticalChip->playOpMemMovie(kMercurySpotID);
				break;
			}

			requestExtraSequence(kTinyTSA37OpMemReviewToMainMenu, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kTinyTSA37DownloadToMainMenu:
		case kTinyTSA37OpMemReviewToMainMenu:
			GameState.setTSAState(kPlayerLockedInPegasus);
			showMainJumpMenu();
			makeContinuePoint();
			break;
		case kTinyTSA37JumpToNoradMenu:
			setCurrentActivation(kActivationTinyTSAJumpToNorad);
			break;
		case kTinyTSA37JumpToMarsMenu:
			setCurrentActivation(kActivationTinyTSAJumpToMars);
			break;
		case kTinyTSA37JumpToWSCMenu:
			setCurrentActivation(kActivationTinyTSAJumpToWSC);
			break;
		case kTinyTSA37CancelNorad:
		case kTinyTSA37CancelMars:
		case kTinyTSA37CancelWSC:
			showMainJumpMenu();
			break;
		}
	}

	g_AIArea->checkMiddleArea();
}

void TinyTSA::arriveFromNorad() {
	requestExtraSequence(kTinyTSA37RecallToDownload, 0, kFilterNoInput);

	if (GameState.getNoradFinished() && !GameState.getScoringFinishedNorad()) {
		GameState.setScoringFinishedNorad();
		requestExtraSequence(kTinyTSA37DownloadToOpMemReview, kExtraCompletedFlag, kFilterNoInput);
	} else {
		requestExtraSequence(kTinyTSA37DownloadToMainMenu, kExtraCompletedFlag, kFilterNoInput);
	}
}

void TinyTSA::arriveFromMars() {
	requestExtraSequence(kTinyTSA37RecallToDownload, 0, kFilterNoInput);

	if (GameState.getMarsFinished() && !GameState.getScoringFinishedMars()) {
		GameState.setScoringFinishedMars();
		requestExtraSequence(kTinyTSA37DownloadToOpMemReview, kExtraCompletedFlag, kFilterNoInput);
	} else {
		requestExtraSequence(kTinyTSA37DownloadToMainMenu, kExtraCompletedFlag, kFilterNoInput);
	}
}

void TinyTSA::arriveFromWSC() {
	requestExtraSequence(kTinyTSA37RecallToDownload, 0, kFilterNoInput);

	if (GameState.getWSCFinished() && !GameState.getScoringFinishedWSC()) {
		GameState.setScoringFinishedWSC();
		requestExtraSequence(kTinyTSA37DownloadToOpMemReview, kExtraCompletedFlag, kFilterNoInput);
	} else {
		requestExtraSequence(kTinyTSA37DownloadToMainMenu, kExtraCompletedFlag, kFilterNoInput);
	}
}

Common::String TinyTSA::getNavMovieName() {
	return "Images/TSA/Tiny TSA.movie";
}

} // End of namespace Pegasus
