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

#ifndef PEGASUS_GAMESTATE_H
#define PEGASUS_GAMESTATE_H

#include "common/singleton.h"
#include "common/util.h"

#include "pegasus/types.h"
#include "pegasus/util.h"
#include "pegasus/items/item.h"

namespace Common {
	class Error;
	class ReadStream;
	class WriteStream;
}

namespace Pegasus {

// The only things saved in here are things which get written out to a saved game file...

enum {
	kGlobalWalkthroughFlag,
	kGlobalShieldOnFlag,
	kGlobalEasterEggFlag,
	kGlobalBeenToWSCFlag,
	kGlobalBeenToMarsFlag,
	kGlobalBeenToNoradFlag,
	kGlobalWSCFinishedFlag,
	kGlobalMarsFinishedFlag,
	kGlobalNoradFinishedFlag,
	kNumGlobalFlags
};

enum {
	kScoringSawINNFlag,
	kScoringTookShowerFlag,
	kScoringFixedHairFlag,
	kScoringGotKeyCardFlag,
	kScoringReadPaperFlag,
	kScoringLookThroughTelescopeFlag,
	kScoringSawCaldoriaKioskFlag,
	kScoringGoToTSAFlag,
	kScoringEnterTSAFlag,
	kScoringSawBust1Flag,
	kScoringSawBust2Flag,
	kScoringSawBust3Flag,
	kScoringSawBust4Flag,
	kScoringSawBust5Flag,
	kScoringSawBust6Flag,
	kScoringSawTheoryFlag,
	kScoringSawBackgroundFlag,
	kScoringSawProcedureFlag,
	kScoringGotJourneymanKeyFlag,
	kScoringGotPegasusBiochipFlag,
	kScoringGotBiosuitFlag,
	kScoringGoToPrehistoricFlag,
	kScoringPutLogInReaderFlag,
	kScoringSawCaldoriaNormalFlag,
	kScoringSawCaldoriaAlteredFlag,
	kScoringSawNoradNormalFlag,
	kScoringSawNoradAlteredFlag,
	kScoringSawMarsNormalFlag,
	kScoringSawMarsAlteredFlag,
	kScoringSawWSCNormalFlag,
	kScoringSawWSCAlteredFlag,
	kScoringWentToReadyRoom2Flag,
	kScoringWentAfterSinclairFlag,
	kScoringUsedCardBombFlag,
	kScoringShieldedCardBombFlag,
	kScoringStunnedSinclairFlag,
	kScoringDisarmedNukeFlag,

	kScoringThrewBreakerFlag,
	kScoringExtendedBridgeFlag,
	kScoringGotHistoricalLogFlag,
	kScoringFinishedPrehistoricFlag,

	kScoringThrownByRobotFlag,
	kScoringGotMarsCardFlag,
	kScoringSawMarsKioskFlag,
	kScoringSawTransportMapFlag,
	kScoringGotCrowBarFlag,
	kScoringTurnedOnTransportFlag,
	kScoringGotOxygenMaskFlag,
	kScoringAvoidedRobotFlag,
	kScoringActivatedPlatformFlag,
	kScoringUsedLiquidNitrogenFlag,
	kScoringUsedCrowBarFlag,
	kScoringFoundCardBombFlag,
	kScoringDisarmedCardBombFlag,
	kScoringGotCardBombFlag,
	kScoringThreadedMazeFlag,
	kScoringThreadedGearRoomFlag,
	kScoringEnteredShuttleFlag,
	kScoringEnteredLaunchTubeFlag,
	kScoringStoppedRobotsShuttleFlag,
	kScoringGotMarsOpMemChipFlag,
	kScoringFinishedMarsFlag,

	kScoringSawSecurityMonitorFlag,
	kScoringFilledOxygenCanisterFlag,
	kScoringFilledArgonCanisterFlag,
	kScoringSawUnconsciousOperatorFlag,
	kScoringWentThroughPressureDoorFlag,
	kScoringPreppedSubFlag,
	kScoringEnteredSubFlag,
	kScoringExitedSubFlag,
	kScoringSawRobotAt54NorthFlag,
	kScoringPlayedWithClawFlag,
	kScoringUsedRetinalChipFlag,
	kScoringFinishedGlobeGameFlag,
	kScoringStoppedNoradRobotFlag,
	kScoringGotNoradOpMemChipFlag,
	kScoringFinishedNoradFlag,

	kScoringRemovedDartFlag,
	kScoringAnalyzedDartFlag,
	kScoringBuiltAntidoteFlag,
	kScoringGotSinclairKeyFlag,
	kScoringGotArgonCanisterFlag,
	kScoringGotNitrogenCanisterFlag,
	kScoringPlayedWithMessagesFlag,
	kScoringSawMorphExperimentFlag,
	kScoringEnteredSinclairOfficeFlag,
	kScoringSawBrochureFlag,
	kScoringSawSinclairEntry1Flag,
	kScoringSawSinclairEntry2Flag,
	kScoringSawSinclairEntry3Flag,
	kScoringSawWSCDirectoryFlag,
	kScoringUsedCrowBarInWSCFlag,
	kScoringFinishedPlasmaDodgeFlag,
	kScoringOpenedCatwalkFlag,
	kScoringStoppedWSCRobotFlag,
	kScoringGotWSCOpMemChipFlag,
	kScoringFinishedWSCFlag,

	kScoringMarsGandhiFlag,
	kScoringNoradGandhiFlag,
	kScoringWSCGandhiFlag,

	kNumScoringFlags
};

enum {
	kCaldoriaSeenPullbackFlag,
	kCaldoriaMadeOJFlag,
	kCaldoriaWokenUpFlag,
	kCaldoriaDidRecalibrationFlag,
	kCaldoriaSeenSinclairInElevatorFlag,
	kCaldoriaINNAnnouncingFlag,
	kCaldoriaSeenINNFlag,
	kCaldoriaSeenMessagesFlag,
	kCaldoriaSinclairShotFlag,
	kCaldoriaBombDisarmedFlag,
	kCaldoriaRoofDoorOpenFlag,
	kCaldoriaDoneHygieneFlag,
	kCaldoriaSawVoiceAnalysisFlag,
	kCaldoriaDoorBombedFlag,
	kCaldoriaGunAimedFlag,
	kNumCaldoriaFlags
};

enum {
	kCaldoriaNoFuseRunning,
	kCaldoriaDoorBombFuseRunning,
	kCaldoriaSinclairFuseRunning
};

enum {
	kTSAIDedAtDoorFlag,
	kTSA0BZoomedInFlag,
	kTSAFrontDoorUnlockedOutsideFlag,
	kTSAFrontDoorUnlockedInsideFlag,
	kTSASeenRobotGreetingFlag,
	kTSASeenTheoryFlag,
	kTSASeenBackgroundFlag,
	kTSASeenProcedureFlag,
	kTSASeenAgent3AtDoorFlag,
	kTSACommandCenterLockedFlag,
	kTSASeenCaldoriaNormalFlag,
	kTSASeenCaldoriaAlteredFlag,
	kTSASeenNoradNormalFlag,
	kTSASeenNoradAlteredFlag,
	kTSASeenMarsNormalFlag,
	kTSASeenMarsAlteredFlag,
	kTSASeenWSCNormalFlag,
	kTSASeenWSCAlteredFlag,
	kTSABiosuitOnFlag,
	kNumTSAFlags
};

enum {
	kPrehistoricTriedToExtendBridgeFlag,
	kPrehistoricSeenTimeStreamFlag,
	kPrehistoricSeenFlyer1Flag,
	kPrehistoricSeenFlyer2Flag,
	kPrehistoricSeenBridgeZoomFlag,
	kPrehistoricBreakerThrownFlag,
	kNumPrehistoricFlags
};

enum {
	kNoradSeenTimeStreamFlag,
	kNoradGassedFlag,
	kNoradFillingStationOnFlag,
	kNoradN22MessagePlayedFlag,
	kNoradArrivedFromSubFlag,
	kNoradWaitingForLaserFlag,
	kNoradRetScanGoodFlag,
	kNoradPlayedGlobeGameFlag,
	kNoradBeatRobotWithClawFlag,
	kNoradBeatRobotWithDoorFlag,
	kNumNoradFlags
};

enum {
	kMarsSeenTimeStreamFlag,
	kMarsHeardUpperPodMessageFlag,
	kMarsRobotThrownPlayerFlag,
	kMarsHeardCheckInMessageFlag,
	kMarsPodAtUpperPlatformFlag,
	kMarsSeenThermalScanFlag,
	kMarsArrivedBelowFlag,
	kMarsSeenRobotAtReactorFlag,
	kMarsAvoidedReactorRobotFlag,
	kMarsInAirlockFlag,
	kMarsAirlockOpenFlag,
	kMarsMaskOnFillerFlag,
	kMarsLockFrozenFlag,
	kMarsLockBrokenFlag,
	kMarsMazeDoorPair1Flag,
	kMarsMazeDoorPair2Flag,
	kMarsMazeDoorPair3Flag,
	kMarsSawRobotLeaveFlag,
	kMarsSecurityDownFlag,
	kMarsHitRobotWithCannonFlag,
	kMarsReadyForShuttleTransportFlag,
	kMarsFinishedCanyonChaseFlag,
	kMarsThreadedMazeFlag,
	kNumMarsFlags
};

enum {
	kWSCSeenTimeStreamFlag,
	kWSCPoisonedFlag,
	kWSCAnsweredAboutDartFlag,
	kWSCRemovedDartFlag,
	kWSCAnalyzerOnFlag,
	kWSCDartInAnalyzerFlag,
	kWSCAnalyzedDartFlag,
	kWSCSawMorphFlag,
	kWSCDesignedAntidoteFlag,
	kWSCPickedUpAntidoteFlag,
	kWSCOfficeMessagesOpenFlag,
	kWSCSeenNerdFlag,
	kWSCHeardPage1Flag,
	kWSCHeardPage2Flag,
	kWSCHeardCheckInFlag,
	kWSCDidPlasmaDodgeFlag,
	kWSCSeenSinclairLectureFlag,
	kWSCBeenAtWSC93Flag,
	kWSCCatwalkDarkFlag,
	kWSCRobotDeadFlag,
	kWSCRobotGoneFlag,
	kNumWSCFlags
};

class GameStateManager : public Common::Singleton<GameStateManager> {
public:
	GameStateManager() { resetGameState(); }

	// Base game state
	Common::Error writeGameState(Common::WriteStream *stream);
	Common::Error readGameState(Common::ReadStream *stream);

	void resetGameState();

	void getCurrentLocation(NeighborhoodID &neighborhood, RoomID &room, DirectionConstant &direction);
	void setCurrentLocation(const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant direction);

	NeighborhoodID getCurrentNeighborhood();
	void setCurrentNeighborhood(const NeighborhoodID neighborhood);
	RoomID getCurrentRoom();
	void setCurrentRoom(const RoomID room);
	DirectionConstant getCurrentDirection();
	void setCurrentDirection(const DirectionConstant direction);

	RoomViewID getCurrentRoomAndView();

	void getNextLocation(NeighborhoodID &neighborhood, RoomID &room, DirectionConstant &direction);
	void setNextLocation(const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant direction);

	NeighborhoodID getNextNeighborhood();
	void setNextNeighborhood(const NeighborhoodID neighborhood);
	RoomID getNextRoom();
	void setNextRoom(const RoomID room);
	DirectionConstant getNextDirection();
	void setNextDirection(const DirectionConstant direction);

	void getLastLocation(NeighborhoodID &neighborhood, RoomID &room, DirectionConstant &direction);
	void setLastLocation(const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant direction);

	NeighborhoodID getLastNeighborhood();
	void setLastNeighborhood(const NeighborhoodID neighborhood);
	RoomID getLastRoom();
	void setLastRoom(const RoomID room);
	DirectionConstant getLastDirection();
	void setLastDirection(const DirectionConstant direction);

	RoomViewID getLastRoomAndView();

	void getOpenDoorLocation(RoomID &room, DirectionConstant &direction);
	void setOpenDoorLocation(const RoomID room, const DirectionConstant direction);
	RoomID getOpenDoorRoom();
	void setOpenDoorRoom(const RoomID room);
	DirectionConstant getOpenDoorDirection();
	void setOpenDoorDirection(const DirectionConstant direction);

	RoomViewID getDoorOpenRoomAndView();

	bool isCurrentDoorOpen();

	// Pegasus Prime

	// Scoring...
	// Scoring "Set" functions.
	// Caldoria/TSA scoring
	void setScoringSawINN(const bool = true);
	void setScoringTookShower(const bool = true);
	void setScoringFixedHair(const bool = true);
	void setScoringGotKeyCard(const bool = true);
	void setScoringReadPaper(const bool = true);
	void setScoringLookThroughTelescope(const bool = true);
	void setScoringSawCaldoriaKiosk(const bool = true);
	void setScoringGoToTSA(const bool = true);
	void setScoringEnterTSA(const bool = true);
	void setScoringSawBust1(const bool = true);
	void setScoringSawBust2(const bool = true);
	void setScoringSawBust3(const bool = true);
	void setScoringSawBust4(const bool = true);
	void setScoringSawBust5(const bool = true);
	void setScoringSawBust6(const bool = true);
	void setScoringSawTheory(const bool = true);
	void setScoringSawBackground(const bool = true);
	void setScoringSawProcedure(const bool = true);
	void setScoringGotJourneymanKey(const bool = true);
	void setScoringGotPegasusBiochip(const bool = true);
	void setScoringGotBiosuit(const bool = true);
	void setScoringGoToPrehistoric(const bool = true);
	void setScoringPutLogInReader(const bool = true);
	void setScoringSawCaldoriaNormal(const bool = true);
	void setScoringSawCaldoriaAltered(const bool = true);
	void setScoringSawNoradNormal(const bool = true);
	void setScoringSawNoradAltered(const bool = true);
	void setScoringSawMarsNormal(const bool = true);
	void setScoringSawMarsAltered(const bool = true);
	void setScoringSawWSCNormal(const bool = true);
	void setScoringSawWSCAltered(const bool = true);
	void setScoringWentToReadyRoom2(const bool = true);
	void setScoringWentAfterSinclair(const bool = true);
	void setScoringUsedCardBomb(const bool = true);
	void setScoringShieldedCardBomb(const bool = true);
	void setScoringStunnedSinclair(const bool = true);
	void setScoringDisarmedNuke(const bool = true);

	// Prehistoric scoring
	void setScoringThrewBreaker(const bool = true);
	void setScoringExtendedBridge(const bool = true);
	void setScoringGotHistoricalLog(const bool = true);
	void setScoringFinishedPrehistoric(const bool = true);

	// Mars scoring
	void setScoringThrownByRobot(const bool = true);
	void setScoringGotMarsCard(const bool = true);
	void setScoringSawMarsKiosk(const bool = true);
	void setScoringSawTransportMap(const bool = true);
	void setScoringGotCrowBar(const bool = true);
	void setScoringTurnedOnTransport(const bool = true);
	void setScoringGotOxygenMask(const bool = true);
	void setScoringAvoidedRobot(const bool = true);
	void setScoringActivatedPlatform(const bool = true);
	void setScoringUsedLiquidNitrogen(const bool = true);
	void setScoringUsedCrowBar(const bool = true);
	void setScoringFoundCardBomb(const bool = true);
	void setScoringDisarmedCardBomb(const bool = true);
	void setScoringGotCardBomb(const bool = true);
	void setScoringThreadedMaze(const bool = true);
	void setScoringThreadedGearRoom(const bool = true);
	void setScoringEnteredShuttle(const bool = true);
	void setScoringEnteredLaunchTube(const bool = true);
	void setScoringStoppedRobotsShuttle(const bool = true);
	void setScoringGotMarsOpMemChip(const bool = true);
	void setScoringFinishedMars(const bool = true);

	// Norad scoring
	void setScoringSawSecurityMonitor(const bool = true);
	void setScoringFilledOxygenCanister(const bool = true);
	void setScoringFilledArgonCanister(const bool = true);
	void setScoringSawUnconsciousOperator(const bool = true);
	void setScoringWentThroughPressureDoor(const bool = true);
	void setScoringPreppedSub(const bool = true);
	void setScoringEnteredSub(const bool = true);
	void setScoringExitedSub(const bool = true);
	void setScoringSawRobotAt54North(const bool = true);
	void setScoringPlayedWithClaw(const bool = true);
	void setScoringUsedRetinalChip(const bool = true);
	void setScoringFinishedGlobeGame(const bool = true);
	void setScoringStoppedNoradRobot(const bool = true);
	void setScoringGotNoradOpMemChip(const bool = true);
	void setScoringFinishedNorad(const bool = true);

	// WSC scoring
	void setScoringRemovedDart(const bool = true);
	void setScoringAnalyzedDart(const bool = true);
	void setScoringBuiltAntidote(const bool = true);
	void setScoringGotSinclairKey(const bool = true);
	void setScoringGotArgonCanister(const bool = true);
	void setScoringGotNitrogenCanister(const bool = true);
	void setScoringPlayedWithMessages(const bool = true);
	void setScoringSawMorphExperiment(const bool = true);
	void setScoringEnteredSinclairOffice(const bool = true);
	void setScoringSawBrochure(const bool = true);
	void setScoringSawSinclairEntry1(const bool = true);
	void setScoringSawSinclairEntry2(const bool = true);
	void setScoringSawSinclairEntry3(const bool = true);
	void setScoringSawWSCDirectory(const bool = true);
	void setScoringUsedCrowBarInWSC(const bool = true);
	void setScoringFinishedPlasmaDodge(const bool = true);
	void setScoringOpenedCatwalk(const bool = true);
	void setScoringStoppedWSCRobot(const bool = true);
	void setScoringGotWSCOpMemChip(const bool = true);
	void setScoringFinishedWSC(const bool = true);

	// Gandhi scoring
	void setScoringMarsGandhi(const bool = true);
	void setScoringNoradGandhi(const bool = true);
	void setScoringWSCGandhi(const bool = true);

	// Scoring "Get" functions.
	bool getScoringSawINN();
	bool getScoringTookShower();
	bool getScoringFixedHair();
	bool getScoringGotKeyCard();
	bool getScoringReadPaper();
	bool getScoringLookThroughTelescope();
	bool getScoringSawCaldoriaKiosk();
	bool getScoringGoToTSA();
	bool getScoringEnterTSA();
	bool getScoringSawBust1();
	bool getScoringSawBust2();
	bool getScoringSawBust3();
	bool getScoringSawBust4();
	bool getScoringSawBust5();
	bool getScoringSawBust6();
	bool getScoringSawTheory();
	bool getScoringSawBackground();
	bool getScoringSawProcedure();
	bool getScoringGotJourneymanKey();
	bool getScoringGotPegasusBiochip();
	bool getScoringGotBiosuit();
	bool getScoringGoToPrehistoric();
	bool getScoringPutLogInReader();
	bool getScoringSawCaldoriaNormal();
	bool getScoringSawCaldoriaAltered();
	bool getScoringSawNoradNormal();
	bool getScoringSawNoradAltered();
	bool getScoringSawMarsNormal();
	bool getScoringSawMarsAltered();
	bool getScoringSawWSCNormal();
	bool getScoringSawWSCAltered();
	bool getScoringWentToReadyRoom2();
	bool getScoringWentAfterSinclair();
	bool getScoringUsedCardBomb();
	bool getScoringShieldedCardBomb();
	bool getScoringStunnedSinclair();
	bool getScoringDisarmedNuke();
	bool getScoringThrewBreaker();
	bool getScoringExtendedBridge();
	bool getScoringGotHistoricalLog();
	bool getScoringFinishedPrehistoric();
	bool getScoringThrownByRobot();
	bool getScoringGotMarsCard();
	bool getScoringSawMarsKiosk();
	bool getScoringSawTransportMap();
	bool getScoringGotCrowBar();
	bool getScoringTurnedOnTransport();
	bool getScoringGotOxygenMask();
	bool getScoringAvoidedRobot();
	bool getScoringActivatedPlatform();
	bool getScoringUsedLiquidNitrogen();
	bool getScoringUsedCrowBar();
	bool getScoringFoundCardBomb();
	bool getScoringDisarmedCardBomb();
	bool getScoringGotCardBomb();
	bool getScoringThreadedMaze();
	bool getScoringThreadedGearRoom();
	bool getScoringEnteredShuttle();
	bool getScoringEnteredLaunchTube();
	bool getScoringStoppedRobotsShuttle();
	bool getScoringGotMarsOpMemChip();
	bool getScoringFinishedMars();
	bool getScoringSawSecurityMonitor();
	bool getScoringFilledOxygenCanister();
	bool getScoringFilledArgonCanister();
	bool getScoringSawUnconsciousOperator();
	bool getScoringWentThroughPressureDoor();
	bool getScoringPreppedSub();
	bool getScoringEnteredSub();
	bool getScoringExitedSub();
	bool getScoringSawRobotAt54North();
	bool getScoringPlayedWithClaw();
	bool getScoringUsedRetinalChip();
	bool getScoringFinishedGlobeGame();
	bool getScoringStoppedNoradRobot();
	bool getScoringGotNoradOpMemChip();
	bool getScoringFinishedNorad();
	bool getScoringRemovedDart();
	bool getScoringAnalyzedDart();
	bool getScoringBuiltAntidote();
	bool getScoringGotSinclairKey();
	bool getScoringGotArgonCanister();
	bool getScoringGotNitrogenCanister();
	bool getScoringPlayedWithMessages();
	bool getScoringSawMorphExperiment();
	bool getScoringEnteredSinclairOffice();
	bool getScoringSawBrochure();
	bool getScoringSawSinclairEntry1();
	bool getScoringSawSinclairEntry2();
	bool getScoringSawSinclairEntry3();
	bool getScoringSawWSCDirectory();
	bool getScoringUsedCrowBarInWSC();
	bool getScoringFinishedPlasmaDodge();
	bool getScoringOpenedCatwalk();
	bool getScoringStoppedWSCRobot();
	bool getScoringGotWSCOpMemChip();
	bool getScoringFinishedWSC();
	bool getScoringMarsGandhi();
	bool getScoringNoradGandhi();
	bool getScoringWSCGandhi();

	GameScoreType getCaldoriaTSAScore();
	GameScoreType getPrehistoricScore();
	GameScoreType getMarsScore();
	GameScoreType getNoradScore();
	GameScoreType getWSCScore();
	GameScoreType getGandhiScore();
	GameScoreType getTotalScore();

	void writeCaldoriaState(Common::WriteStream *stream);
	void readCaldoriaState(Common::ReadStream *stream);
	void resetCaldoriaState();

	void writeTSAState(Common::WriteStream *stream);
	void readTSAState(Common::ReadStream *stream);
	void resetTSAState();

	void writePrehistoricState(Common::WriteStream *stream);
	void readPrehistoricState(Common::ReadStream *stream);
	void resetPrehistoricState();

	void writeNoradState(Common::WriteStream *stream);
	void readNoradState(Common::ReadStream *stream);
	void resetNoradState();

	void writeMarsState(Common::WriteStream *stream);
	void readMarsState(Common::ReadStream *stream);
	void resetMarsState();

	void writeWSCState(Common::WriteStream *stream);
	void readWSCState(Common::ReadStream *stream);
	void resetWSCState();

	// Globals.
	void setWalkthroughMode(bool);
	bool getWalkthroughMode();
	void setShieldOn(bool);
	bool getShieldOn();
	void setEasterEgg(bool);
	bool getEasterEgg();
	void setBeenToWSC(bool value);
	bool getBeenToWSC();
	void setBeenToMars(bool value);
	bool getBeenToMars();
	void setBeenToNorad(bool value);
	bool getBeenToNorad();
	void setWSCFinished(bool);
	bool getWSCFinished();
	void setMarsFinished(bool);
	bool getMarsFinished();
	void setNoradFinished(bool);
	bool getNoradFinished();
	bool allTimeZonesFinished();
	void setTakenItemID(ItemID, bool);
	bool isTakenItemID(ItemID);
	void setTakenItem(Item *, bool);
	bool isTakenItem(Item *);

	// Caldoria
	void setCaldoriaFuseTimeLimit(const TimeValue);
	TimeValue getCaldoriaFuseTimeLimit();
	void setCaldoriaSeenPullback(bool);
	bool getCaldoriaSeenPullback();
	void setCaldoriaMadeOJ(bool);
	bool getCaldoriaMadeOJ();
	void setCaldoriaWokenUp(bool);
	bool getCaldoriaWokenUp();
	void setCaldoriaDidRecalibration(bool);
	bool getCaldoriaDidRecalibration();
	void setCaldoriaSeenSinclairInElevator(bool);
	bool getCaldoriaSeenSinclairInElevator();
	void setCaldoriaINNAnnouncing(bool);
	bool getCaldoriaINNAnnouncing();
	void setCaldoriaSeenINN(bool);
	bool getCaldoriaSeenINN();
	void setCaldoriaSeenMessages(bool);
	bool getCaldoriaSeenMessages();
	void setCaldoriaSinclairShot(bool);
	bool getCaldoriaSinclairShot();
	void setCaldoriaBombDisarmed(bool);
	bool getCaldoriaBombDisarmed();
	void setCaldoriaRoofDoorOpen(bool);
	bool getCaldoriaRoofDoorOpen();
	void setCaldoriaDoneHygiene(bool);
	bool getCaldoriaDoneHygiene();
	void setCaldoriaSawVoiceAnalysis(bool);
	bool getCaldoriaSawVoiceAnalysis();
	void setCaldoriaDoorBombed(bool);
	bool getCaldoriaDoorBombed();
	void setCaldoriaGunAimed(bool);
	bool getCaldoriaGunAimed();

	// TSA
	void setRipTimerTime(TimeValue);
	TimeValue getRipTimerTime();
	void setTSAFuseTimeLimit(TimeValue);
	TimeValue getTSAFuseTimeLimit();
	void setT0BMonitorMode(byte);
	byte getT0BMonitorMode();
	void setTSAState(byte);
	byte getTSAState();
	void setT0BMonitorStart(TimeValue);
	TimeValue getT0BMonitorStart();
	void setTSAIDedAtDoor(bool);
	bool getTSAIDedAtDoor();
	void setTSA0BZoomedIn(bool);
	bool getTSA0BZoomedIn();
	void setTSAFrontDoorUnlockedOutside(bool);
	bool getTSAFrontDoorUnlockedOutside();
	void setTSAFrontDoorUnlockedInside(bool);
	bool getTSAFrontDoorUnlockedInside();
	void setTSASeenRobotGreeting(bool);
	bool getTSASeenRobotGreeting();
	void setTSASeenTheory(bool);
	bool getTSASeenTheory();
	void setTSASeenBackground(bool);
	bool getTSASeenBackground();
	void setTSASeenProcedure(bool);
	bool getTSASeenProcedure();
	void setTSASeenAgent3AtDoor(bool);
	bool getTSASeenAgent3AtDoor();
	void setTSACommandCenterLocked(bool);
	bool getTSACommandCenterLocked();
	void setTSASeenCaldoriaNormal(bool);
	bool getTSASeenCaldoriaNormal();
	void setTSASeenCaldoriaAltered(bool);
	bool getTSASeenCaldoriaAltered();
	void setTSASeenNoradNormal(bool);
	bool getTSASeenNoradNormal();
	void setTSASeenNoradAltered(bool);
	bool getTSASeenNoradAltered();
	void setTSASeenMarsNormal(bool);
	bool getTSASeenMarsNormal();
	void setTSASeenMarsAltered(bool);
	bool getTSASeenMarsAltered();
	void setTSASeenWSCNormal(bool);
	bool getTSASeenWSCNormal();
	void setTSASeenWSCAltered(bool);
	bool getTSASeenWSCAltered();
	void setTSABiosuitOn(bool);
	bool getTSABiosuitOn();

	// Prehistoric
	void setPrehistoricTriedToExtendBridge(bool);
	bool getPrehistoricTriedToExtendBridge();
	void setPrehistoricSeenTimeStream(bool);
	bool getPrehistoricSeenTimeStream();
	void setPrehistoricSeenFlyer1(bool);
	bool getPrehistoricSeenFlyer1();
	void setPrehistoricSeenFlyer2(bool);
	bool getPrehistoricSeenFlyer2();
	void setPrehistoricSeenBridgeZoom(bool);
	bool getPrehistoricSeenBridgeZoom();
	void setPrehistoricBreakerThrown(bool);
	bool getPrehistoricBreakerThrown();

	// Norad
	void setNoradSeenTimeStream(bool);
	bool getNoradSeenTimeStream();
	void setNoradGassed(bool);
	bool getNoradGassed();
	void setNoradFillingStationOn(bool);
	bool getNoradFillingStationOn();
	void setNoradN22MessagePlayed(bool);
	bool getNoradN22MessagePlayed();
	void setNoradPlayedGlobeGame(bool);
	bool getNoradPlayedGlobeGame();
	void setNoradBeatRobotWithClaw(bool);
	bool getNoradBeatRobotWithClaw();
	void setNoradBeatRobotWithDoor(bool);
	bool getNoradBeatRobotWithDoor();
	void setNoradRetScanGood(bool);
	bool getNoradRetScanGood();
	void setNoradWaitingForLaser(bool);
	bool getNoradWaitingForLaser();
	void setNoradSubRoomPressure(uint16);
	uint16 getNoradSubRoomPressure();
	void setNoradSubPrepState(NoradSubPrepState);
	NoradSubPrepState getNoradSubPrepState();
	void setNoradArrivedFromSub(bool);
	bool getNoradArrivedFromSub();

	// Mars
	void setMarsSeenTimeStream(bool);
	bool getMarsSeenTimeStream();
	void setMarsHeardUpperPodMessage(bool);
	bool getMarsHeardUpperPodMessage();
	void setMarsRobotThrownPlayer(bool);
	bool getMarsRobotThrownPlayer();
	void setMarsHeardCheckInMessage(bool);
	bool getMarsHeardCheckInMessage();
	void setMarsPodAtUpperPlatform(bool);
	bool getMarsPodAtUpperPlatform();
	void setMarsSeenThermalScan(bool);
	bool getMarsSeenThermalScan();
	void setMarsArrivedBelow(bool);
	bool getMarsArrivedBelow();
	void setMarsSeenRobotAtReactor(bool);
	bool getMarsSeenRobotAtReactor();
	void setMarsAvoidedReactorRobot(bool);
	bool getMarsAvoidedReactorRobot();
	void setMarsInAirlock(bool);
	bool getMarsInAirlock();
	void setMarsAirlockOpen(bool);
	bool getMarsAirlockOpen();
	void setMarsMaskOnFiller(bool);
	bool getMarsMaskOnFiller();
	void setMarsLockFrozen(bool);
	bool getMarsLockFrozen();
	void setMarsLockBroken(bool);
	bool getMarsLockBroken();
	void setMarsMazeDoorPair1(bool);
	bool getMarsMazeDoorPair1();
	void setMarsMazeDoorPair2(bool);
	bool getMarsMazeDoorPair2();
	void setMarsMazeDoorPair3(bool);
	bool getMarsMazeDoorPair3();
	void setMarsSawRobotLeave(bool);
	bool getMarsSawRobotLeave();
	void setMarsSecurityDown(bool);
	bool getMarsSecurityDown();
	void setMarsFinishedCanyonChase(bool);
	bool getMarsFinishedCanyonChase();
	void setMarsThreadedMaze(bool);
	bool getMarsThreadedMaze();
	void setMarsHitRobotWithCannon(bool);
	bool getMarsHitRobotWithCannon();
	void setMarsReadyForShuttleTransport(bool);
	bool getMarsReadyForShuttleTransport();

	// WSC
	void setWSCSeenTimeStream(bool);
	bool getWSCSeenTimeStream();
	void setWSCPoisoned(bool);
	bool getWSCPoisoned();
	void setWSCAnsweredAboutDart(bool);
	bool getWSCAnsweredAboutDart();
	void setWSCRemovedDart(bool);
	bool getWSCRemovedDart();
	void setWSCAnalyzerOn(bool);
	bool getWSCAnalyzerOn();
	void setWSCDartInAnalyzer(bool);
	bool getWSCDartInAnalyzer();
	void setWSCAnalyzedDart(bool);
	bool getWSCAnalyzedDart();
	void setWSCSawMorph(bool);
	bool getWSCSawMorph();
	void setWSCDesignedAntidote(bool);
	bool getWSCDesignedAntidote();
	void setWSCPickedUpAntidote(bool);
	bool getWSCPickedUpAntidote();
	void setWSCOfficeMessagesOpen(bool);
	bool getWSCOfficeMessagesOpen();
	void setWSCSeenNerd(bool);
	bool getWSCSeenNerd();
	void setWSCHeardPage1(bool);
	bool getWSCHeardPage1();
	void setWSCHeardPage2(bool);
	bool getWSCHeardPage2();
	void setWSCHeardCheckIn(bool);
	bool getWSCHeardCheckIn();
	void setWSCDidPlasmaDodge(bool);
	bool getWSCDidPlasmaDodge();
	void setWSCSeenSinclairLecture(bool);
	bool getWSCSeenSinclairLecture();
	void setWSCBeenAtWSC93(bool);
	bool getWSCBeenAtWSC93();
	void setWSCCatwalkDark(bool);
	bool getWSCCatwalkDark();
	void setWSCRobotDead(bool);
	bool getWSCRobotDead();
	void setWSCRobotGone(bool);
	bool getWSCRobotGone();

protected:
	friend class Common::Singleton<SingletonBaseType>;

private:
	// Base
	NeighborhoodID _currentNeighborhood;
	RoomID _currentRoom;
	DirectionConstant _currentDirection;
	NeighborhoodID _nexNeighborhoodID;
	RoomID _nextRoomID;
	DirectionConstant _nextDirection;
	NeighborhoodID _lastNeighborhood;
	RoomID _lastRoom;
	DirectionConstant _lastDirection;
	RoomID _openDoorRoom;
	DirectionConstant _openDoorDirection;

	// Pegasus Prime
	FlagsArray<byte, kNumGlobalFlags> _globalFlags;
	FlagsArray<byte, kNumScoringFlags> _scoringFlags;
	FlagsArray<uint32, kNumItems> _itemTakenFlags;

	FlagsArray<byte, kNumCaldoriaFlags> _caldoriaFlags;
	TimeValue _caldoriaFuseTimeLimit;

	TimeValue _TSARipTimerTime;
	TimeValue _TSAFuseTimeLimit;
	byte _TSAState;
	byte _T0BMonitorMode;
	TimeValue _T0BMonitorStart;
	FlagsArray<byte, kNumTSAFlags> _TSAFlags;

	FlagsArray<byte, kNumPrehistoricFlags> _prehistoricFlags;

	FlagsArray<byte, kNumNoradFlags> _noradFlags;
	uint16 _noradSubRoomPressure;
	NoradSubPrepState _noradSubPrepState;

	FlagsArray<byte, kNumMarsFlags> _marsFlags;

	FlagsArray<byte, kNumWSCFlags> _WSCFlags;
};

} // End of namespace Pegasus

#define GameState (::Pegasus::GameStateManager::instance())

#endif
