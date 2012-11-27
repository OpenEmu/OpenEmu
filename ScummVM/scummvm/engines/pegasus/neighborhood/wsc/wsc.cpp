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
#include "pegasus/items/biochips/opticalchip.h"
#include "pegasus/items/biochips/shieldchip.h"
#include "pegasus/neighborhood/wsc/wsc.h"

namespace Pegasus {

static const CanMoveForwardReason kCantMoveWatchingDiagnosis = kCantMoveLastReason + 1;

static const CanTurnReason kCantTurnWatchingDiagnosis = kCantTurnLastReason + 1;
static const CanTurnReason kCantTurnWatchingAnalysis = kCantTurnWatchingDiagnosis + 1;
static const CanTurnReason kCantTurnInMoleculeGame = kCantTurnWatchingAnalysis + 1;

static const TimeScale kMoleculesMovieScale = 600;
static const TimeValue kMoleculeLoopTime = 4 * kMoleculesMovieScale;
static const TimeValue kMoleculeFailTime = 2 * kMoleculesMovieScale;

enum {
	kMoleculeLoop0Time = 0,
	kMoleculeFail0Time = kMoleculeLoop0Time + kMoleculeLoopTime,
	kMoleculeLoop1Time = kMoleculeFail0Time + kMoleculeFailTime,
	kMoleculeFail1Time = kMoleculeLoop1Time + kMoleculeLoopTime,
	kMoleculeLoop2Time = kMoleculeFail1Time + kMoleculeFailTime,
	kMoleculeFail2Time = kMoleculeLoop2Time + kMoleculeLoopTime,
	kMoleculeLoop3Time = kMoleculeFail2Time + kMoleculeFailTime,
	kMoleculeFail3Time = kMoleculeLoop3Time + kMoleculeLoopTime,
	kMoleculeLoop4Time = kMoleculeFail3Time + kMoleculeFailTime,
	kMoleculeFail4Time = kMoleculeLoop4Time + kMoleculeLoopTime,
	kMoleculeLoop5Time = kMoleculeFail4Time + kMoleculeFailTime,
	kMoleculeFail5Time = kMoleculeLoop5Time + kMoleculeLoopTime,
	kMoleculeLoop6Time = kMoleculeFail5Time + kMoleculeFailTime
};

static const TimeValue s_moleculeLoopTimes[] = {
	kMoleculeLoop0Time,
	kMoleculeLoop1Time,
	kMoleculeLoop2Time,
	kMoleculeLoop3Time,
	kMoleculeLoop4Time,
	kMoleculeLoop5Time,
	kMoleculeLoop6Time
};

static const TimeValue s_moleculeFailTimes[] = {
	kMoleculeFail0Time,
	kMoleculeFail1Time,
	kMoleculeFail2Time,
	kMoleculeFail3Time,
	kMoleculeFail4Time,
	kMoleculeFail5Time
};

static const int16 kAuditoriumAngleOffset = 5;

static const int kPlasmaEnergyWithShield = kMaxJMPEnergy * 10 / 100;
static const int kPlasmaEnergyNoShield = kMaxJMPEnergy * 20 / 100;

static const int kTimerEventPlasmaHit = 0;
static const int kTimerEventPlayerGawkingAtRobot = 1;
static const int kTimerEventPlayerGawkingAtRobot2 = 2;

static const TimeValue kWSCMolecule1In = 0;
static const TimeValue kWSCMolecule1Out = 937;

static const TimeValue kWSCMolecule2In = 937;
static const TimeValue kWSCMolecule2Out = 1864;

static const TimeValue kWSCMolecule3In = 1864;
static const TimeValue kWSCMolecule3Out = 2790;

static const TimeValue kWSCClick1In = 2790;
static const TimeValue kWSCClick1Out = 2890;

static const TimeValue kWSCClick2In = 2890;
static const TimeValue kWSCClick2Out = 3059;

static const TimeValue kWSCClick3In = 3059;
static const TimeValue kWSCClick3Out = 3156;

static const TimeValue kWSCFlashlightClickIn = 3156;
static const TimeValue kWSCFlashlightClickOut = 3211;

static const TimeValue kWSCBumpIntoWallIn = 3211;
static const TimeValue kWSCBumpIntoWallOut = 3514;

static const TimeValue kWSCCantTransportIn = 3514;
static const TimeValue kWSCCantTransportOut = 7791;

static const TimeValue kHernandezNotHomeIn = 7791;
static const TimeValue kHernandezNotHomeOut = 10199;

static const TimeValue kWashingtonNotHomeIn = 10199;
static const TimeValue kWashingtonNotHomeOut = 12649;

static const TimeValue kSullivanNotHomeIn = 12649;
static const TimeValue kSullivanNotHomeOut = 15031;

static const TimeValue kNakamuraNotHomeIn = 15031;
static const TimeValue kNakamuraNotHomeOut = 17545;

static const TimeValue kGrailisNotHomeIn = 17545;
static const TimeValue kGrailisNotHomeOut = 19937;

static const TimeValue kTheriaultNotHomeIn = 19937;
static const TimeValue kTheriaultNotHomeOut = 22395;

static const TimeValue kGlennerNotHomeIn = 22395;
static const TimeValue kGlennerNotHomeOut = 24770;

static const TimeValue kSinclairNotHomeIn = 24770;
static const TimeValue kSinclairNotHomeOut = 27328;

static const TimeValue kWSCLabClosedIn = 27328;
static const TimeValue kWSCLabClosedOut = 28904;

static const TimeValue kSlidingDoorCloseIn = 28904;
static const TimeValue kSlidingDoorCloseOut = 29295;

static const TimeValue kSlimyDoorCloseIn = 29295;
static const TimeValue kSlimyDoorCloseOut = 29788;

static const TimeValue kPaging1In = 29788;
static const TimeValue kPaging1Out = 32501;

static const TimeValue kPaging2In = 32501;
static const TimeValue kPaging2Out = 34892;

static const TimeValue kCheckInIn = 34892;
static const TimeValue kCheckInOut = 37789;

static const TimeValue kDrinkAntidoteIn = 37789;
static const TimeValue kDrinkAntidoteOut = 39725;

static const TimeScale kWSCMovieScale = 600;
static const TimeScale kWSCFramesPerSecond = 15;
static const TimeScale kWSCFrameDuration = 40;

// Alternate IDs.
static const AlternateID kAltWSCNormal = 0;
static const AlternateID kAltWSCTookMachineGun = 1;
static const AlternateID kAltWSCW0ZDoorOpen = 2;
static const AlternateID kAltWSCPeopleAtW19North = 3;

// Room IDs.
static const RoomID kWSC02 = 1;
static const RoomID kWSC03 = 4;
static const RoomID kWSC04 = 5;
static const RoomID kWSC06 = 6;
static const RoomID kWSC07 = 7;
static const RoomID kWSC08 = 8;
static const RoomID kWSC09 = 9;
static const RoomID kWSC10 = 10;
static const RoomID kWSC11 = 11;
static const RoomID kWSC13 = 12;
static const RoomID kWSC14 = 13;
static const RoomID kWSC15 = 14;
static const RoomID kWSC16 = 15;
static const RoomID kWSC17 = 16;
static const RoomID kWSC18 = 17;
static const RoomID kWSC19 = 18;
static const RoomID kWSC20 = 19;
static const RoomID kWSC21 = 20;
static const RoomID kWSC22 = 21;
static const RoomID kWSC23 = 22;
static const RoomID kWSC24 = 23;
static const RoomID kWSC25 = 24;
static const RoomID kWSC26 = 25;
static const RoomID kWSC27 = 26;
static const RoomID kWSC28 = 27;
static const RoomID kWSC29 = 28;
static const RoomID kWSC31 = 29;
static const RoomID kWSC32 = 30;
static const RoomID kWSC33 = 31;
static const RoomID kWSC34 = 32;
static const RoomID kWSC35 = 33;
static const RoomID kWSC36 = 34;
static const RoomID kWSC37 = 35;
static const RoomID kWSC38 = 36;
static const RoomID kWSC39 = 37;
static const RoomID kWSC40 = 38;
static const RoomID kWSC41 = 39;
static const RoomID kWSC42 = 40;
static const RoomID kWSC43 = 41;
static const RoomID kWSC44 = 42;
static const RoomID kWSC45 = 43;
static const RoomID kWSC46 = 44;
static const RoomID kWSC47 = 45;
static const RoomID kWSC48 = 46;
static const RoomID kWSC49 = 47;
static const RoomID kWSC50 = 48;
static const RoomID kWSC52 = 49;
static const RoomID kWSC53 = 50;
static const RoomID kWSC54 = 51;
static const RoomID kWSC55 = 52;
static const RoomID kWSC56 = 53;
static const RoomID kWSC57 = 54;
static const RoomID kWSC58 = 55;
static const RoomID kWSC60 = 56;
static const RoomID kWSC60East = 57;
static const RoomID kWSC60North = 58;
static const RoomID kWSC61 = 59;
static const RoomID kWSC61South = 60;
static const RoomID kWSC61West = 61;
static const RoomID kWSC63 = 63;
static const RoomID kWSC64 = 64;
static const RoomID kWSC65 = 65;
static const RoomID kWSC65Screen = 66;
static const RoomID kWSC66 = 67;
static const RoomID kWSC67 = 68;
static const RoomID kWSC68 = 69;
static const RoomID kWSC69 = 70;
static const RoomID kWSC70 = 71;
static const RoomID kWSC71 = 72;
static const RoomID kWSC72 = 73;
static const RoomID kWSC73 = 74;
static const RoomID kWSC74 = 75;
static const RoomID kWSC75 = 76;
static const RoomID kWSC0Z = 77;
static const RoomID kWSC76 = 78;
static const RoomID kWSC77 = 79;
static const RoomID kWSC78 = 80;
static const RoomID kWSC79 = 81;
static const RoomID kWSC80 = 82;
static const RoomID kWSC81 = 83;
static const RoomID kWSC82 = 84;
static const RoomID kWSC83 = 85;
static const RoomID kWSC84 = 86;
static const RoomID kWSC85 = 87;
static const RoomID kWSC86 = 88;
static const RoomID kWSC87 = 89;
static const RoomID kWSC88 = 90;
static const RoomID kWSC89 = 91;
static const RoomID kWSC90 = 92;
static const RoomID kWSC91 = 93;
static const RoomID kWSC92 = 94;
static const RoomID kWSC93 = 95;
static const RoomID kWSC94 = 96;
static const RoomID kWSC95 = 97;
static const RoomID kWSC96 = 98;
static const RoomID kWSC97 = 99;
static const RoomID kWSC98 = 100;
static const RoomID kWSCDeathRoom = 101;

// Hot Spot Activation IDs.
static const HotSpotActivationID kActivationZoomedInToAnalyzer = 1;
static const HotSpotActivationID kActivationShotByRobot = 2;
static const HotSpotActivationID kActivationWarnedAboutPoison = 3;
static const HotSpotActivationID kActivationMorphScreenOff = 4;
static const HotSpotActivationID kActivationReadyForMorph = 5;
static const HotSpotActivationID kActivationMorphLooping = 6;
static const HotSpotActivationID kActivationMorphInterrupted = 7;
static const HotSpotActivationID kActivationW03NorthOff = 8;
static const HotSpotActivationID kActivationW03NorthReadyForInstructions = 9;
static const HotSpotActivationID kActivationW03NorthSawInstructions = 10;
static const HotSpotActivationID kActivationW03NorthInGame = 11;
static const HotSpotActivationID kActivationReadyForSynthesis = 12;
static const HotSpotActivationID kActivationSynthesizerLooping = 13;
static const HotSpotActivationID kActivationReadyForMap = 14;
static const HotSpotActivationID kActivationSinclairOfficeLocked = 15;
static const HotSpotActivationID kActivationW58SouthDoorLocked = 16;
static const HotSpotActivationID kActivationW61SouthOff = 17;
static const HotSpotActivationID kActivationW61SouthOn = 18;
static const HotSpotActivationID kActivationW61MessagesOff = 19;
static const HotSpotActivationID kActivationW61MessagesOn = 20;
static const HotSpotActivationID kActivationWSCRobotHeadOpen = 21;
static const HotSpotActivationID kActivationRobotTurning = 22;
static const HotSpotActivationID kActivationRobotDead = 23;
static const HotSpotActivationID kActivationRobotGone = 24;

// Hot Spot IDs.
static const HotSpotID kWSCDropDartSpotID = 5000;
static const HotSpotID kWSCTurnOnAnalyzerSpotID = 5001;
static const HotSpotID kWSCAnalyzerScreenSpotID = 5002;
static const HotSpotID kWSCSpinRobotSpotID = 5003;
static const HotSpotID kWSC01YesSpotID = 5004;
static const HotSpotID kWSC01NoSpotID = 5005;
static const HotSpotID kWSC01AcknowledgeWarningSpotID = 5006;
static const HotSpotID kWSC02SouthMorphSpotID = 5007;
static const HotSpotID kWSC02SouthMessagesSpotID = 5008;
static const HotSpotID kWSC02SouthMorphOutSpotID = 5009;
static const HotSpotID kWSC02ActivateMorphScreenSpotID = 5010;
static const HotSpotID kWSC02SouthStartMorphSpotID = 5011;
static const HotSpotID kWSC02SouthInterruptMorphSpotID = 5012;
static const HotSpotID kWSC02SouthMorphFinishedSpotID = 5013;
static const HotSpotID kWSC02SouthTakeArgonSpotID = 5014;
static const HotSpotID kWSC02SouthMessagesOutSpotID = 5015;
static const HotSpotID kWSC02SouthTakeNitrogenSpotID = 5016;
static const HotSpotID kWSC02SouthPlayMessagesSpotID = 5017;
static const HotSpotID kWSC03NorthActivateScreenSpotID = 5018;
static const HotSpotID kWSC03NorthBuildMoleculeSpotID = 5019;
static const HotSpotID kWSC03NorthProceedSpotID = 5020;
static const HotSpotID kWSC03NorthMolecule1SpotID = 5021;
static const HotSpotID kWSC03NorthMolecule2SpotID = 5022;
static const HotSpotID kWSC03NorthMolecule3SpotID = 5023;
static const HotSpotID kWSC03NorthMolecule4SpotID = 5024;
static const HotSpotID kWSC03NorthMolecule5SpotID = 5025;
static const HotSpotID kWSC03NorthMolecule6SpotID = 5026;
static const HotSpotID kWSC03SouthActivateSynthesizerSpotID = 5027;
static const HotSpotID kWSC03SouthPickUpAntidoteSpotID = 5028;
static const HotSpotID kWSC07SouthMapSpotID = 5029;
static const HotSpotID kW42EastUnlockDoorSpotID = 5030;
static const HotSpotID kW56NorthMapSpotID = 5031;
static const HotSpotID kW58SouthPryDoorSpotID = 5032;
static const HotSpotID kWSC60EastSpotID = 5033;
static const HotSpotID kWSC60NorthSpotID = 5034;
static const HotSpotID kWSC60EastOutSpotID = 5035;
static const HotSpotID kWSC60NorthOutSpotID = 5036;
static const HotSpotID kWSC61EastSpotID = 5037;
static const HotSpotID kWSC61SouthSpotID = 5038;
static const HotSpotID kW61SouthMachineGunSpotID = 5039;
static const HotSpotID kW61SouthDropMachineGunSpotID = 5040;
static const HotSpotID kWSC61WestSpotID = 5041;
static const HotSpotID kWSC61SouthOutSpotID = 5042;
static const HotSpotID kW61SouthActivateSpotID = 5043;
static const HotSpotID kW61SmartAlloysSpotID = 5044;
static const HotSpotID kW61MorphingSpotID = 5045;
static const HotSpotID kW61TimeBendingSpotID = 5046;
static const HotSpotID kWSC61WestOutSpotID = 5047;
static const HotSpotID kW61TurnOnMessagesSpotID = 5048;
static const HotSpotID kW61WhiteMessageSpotID = 5049;
static const HotSpotID kW61WalchekMessageSpotID = 5050;
static const HotSpotID kWSC65SouthScreenSpotID = 5051;
static const HotSpotID kWSC65SouthScreenOutSpotID = 5052;
static const HotSpotID kW98RetinalChipSpotID = 5053;
static const HotSpotID kW98MapChipSpotID = 5054;
static const HotSpotID kW98OpticalChipSpotID = 5055;
static const HotSpotID kW98DropArgonSpotID = 5056;
static const HotSpotID kW98GrabCableSpotID = 5057;
static const HotSpotID kW98OpenRobotSpotID = 5058;
static const HotSpotID kW98StunGunSpotID = 5059;

// Extra sequence IDs.
static const ExtraID kWSCArrivalFromTSA = 0;
static const ExtraID kWSCShotByRobot = 1;
static const ExtraID kWSCDartScan1 = 2;
static const ExtraID kWSCDartScan2 = 3;
static const ExtraID kWSCDartScanNo = 4;
static const ExtraID kWSCDartScan3 = 5;
static const ExtraID kWSCAnalyzerPowerUp = 6;
static const ExtraID kWSCAnalyzerPowerUpWithDart = 7;
static const ExtraID kWSCDropDartIntoAnalyzer = 8;
static const ExtraID kWSCAnalyzeDart = 9;
static const ExtraID kWSCZoomOutFromAnalyzer = 10;
static const ExtraID kWSCSpinRobot = 11;
static const ExtraID kWSC02MorphZoomNoArgon = 12;
static const ExtraID kWSC02MessagesZoomNoNitrogen = 13;
static const ExtraID kWSC02ZoomOutNoArgon = 14;
static const ExtraID kWSC02TurnOnMorphScreen = 15;
static const ExtraID kWSC02DropToMorphExperiment = 16;
static const ExtraID kWSC02MorphLoop = 17;
static const ExtraID kWSC02MorphInterruption = 18;
static const ExtraID kWSC02MorphFinished = 19;
static const ExtraID kWSC02TurnOffMorphScreen = 20;
static const ExtraID kWSC02SouthViewNoArgon = 21;
static const ExtraID kMessagesMovedToOffice = 22;
static const ExtraID kMessagesOff = 23;
static const ExtraID kMessagesZoomOutNoNitrogen = 24;
static const ExtraID kMessagesMovedToOfficeNoNitrogen = 25;
static const ExtraID kMessagesOffNoNitrogen = 26;
static const ExtraID kMessagesViewNoNitrogen = 27;
static const ExtraID kMessagesViewMachineOnNoNitrogen = 28;
static const ExtraID kW03NorthActivate = 29;
static const ExtraID kW03NorthGetData = 30;
static const ExtraID kW03NorthInstructions = 31;
static const ExtraID kW03NorthPrepMolecule1 = 32;
static const ExtraID kW03NorthPrepMolecule2 = 33;
static const ExtraID kW03NorthPrepMolecule3 = 34;
static const ExtraID kW03NorthFinishSynthesis = 35;
static const ExtraID kW03SouthCreateAntidote = 36;
static const ExtraID kW03SouthAntidoteLoop = 37;
static const ExtraID kW03SouthDeactivate = 38;
static const ExtraID kW03SouthViewNoAntidote = 39;
static const ExtraID kWSC07SouthMap = 40;
static const ExtraID kW17WestPeopleCrossing = 41;
static const ExtraID kW17WestPeopleCrossingView = 42;
static const ExtraID kW21SouthPeopleCrossing = 43;
static const ExtraID kW24SouthPeopleCrossing = 44;
static const ExtraID kW34EastPeopleCrossing = 45;
static const ExtraID kW36WestPeopleCrossing = 46;
static const ExtraID kW38NorthPeopleCrossing = 47;
static const ExtraID kW46SouthPeopleCrossing = 48;
static const ExtraID kW49NorthPeopleCrossing = 49;
static const ExtraID kW49NorthPeopleCrossingView = 50;
static const ExtraID kWSC56SouthMap = 51;
static const ExtraID kNerdAtTheDoor1 = 52;
static const ExtraID kNerdAtTheDoor2 = 53;
static const ExtraID kW61SouthZoomInNoGun = 54;
static const ExtraID kW61Brochure = 55;
static const ExtraID kW61SouthScreenOnWithGun = 56;
static const ExtraID kW61SouthScreenOffWithGun = 57;
static const ExtraID kW61SouthSmartAlloysWithGun = 58;
static const ExtraID kW61SouthMorphingWithGun = 59;
static const ExtraID kW61SouthTimeBendingWithGun = 60;
static const ExtraID kW61SouthZoomOutNoGun = 61;
static const ExtraID kW61SouthScreenOnNoGun = 62;
static const ExtraID kW61SouthScreenOffNoGun = 63;
static const ExtraID kW61SouthSmartAlloysNoGun = 64;
static const ExtraID kW61SouthMorphingNoGun = 65;
static const ExtraID kW61SouthTimeBendingNoGun = 66;
static const ExtraID kW61MessagesOn = 67;
static const ExtraID kW61MessagesOff = 68;
static const ExtraID kW61WhiteMessage = 69;
static const ExtraID kW61WalchekMessage = 70;
static const ExtraID kW61WalchekEasterEgg1 = 71;
static const ExtraID kW62SouthPlasmaRobotAppears = 72;
static const ExtraID kW62ZoomToRobot = 73;
static const ExtraID kW62ZoomOutFromRobot = 74;
static const ExtraID kW62PlasmaDodgeSurvive = 75;
static const ExtraID kW62PlasmaDodgeDie = 76;
static const ExtraID kW65SouthSinclairLecture = 77;
static const ExtraID kW73WestPeopleCrossing = 78;
static const ExtraID kW73WestPeopleCrossingView = 79;
static const ExtraID kW0ZSpottedByWomen = 80;
static const ExtraID kW95RobotShoots = 81;
static const ExtraID kW98MorphsToRobot = 82;
static const ExtraID kW98RobotShoots = 83;
static const ExtraID kW98RobotShocked = 84;
static const ExtraID kW98RobotGassed = 85;
static const ExtraID kW98RobotHeadOpensDark = 86;
static const ExtraID kW98RobotHead000Dark = 87;
static const ExtraID kW98RobotHead001Dark = 88;
static const ExtraID kW98RobotHead010Dark = 89;
static const ExtraID kW98RobotHead011Dark = 90;
static const ExtraID kW98RobotHead100Dark = 91;
static const ExtraID kW98RobotHead101Dark = 92;
static const ExtraID kW98RobotHead110Dark = 93;
static const ExtraID kW98RobotHead111Dark = 94;
static const ExtraID kW98RobotHeadClosesDark = 95;
static const ExtraID kW98WestViewWithGunDark = 96;
static const ExtraID kW98WestViewNoGunDark = 97;
static const ExtraID kW98RobotHeadOpensLight = 98;
static const ExtraID kW98RobotHead000Light = 99;
static const ExtraID kW98RobotHead001Light = 100;
static const ExtraID kW98RobotHead010Light = 101;
static const ExtraID kW98RobotHead011Light = 102;
static const ExtraID kW98RobotHead100Light = 103;
static const ExtraID kW98RobotHead101Light = 104;
static const ExtraID kW98RobotHead110Light = 105;
static const ExtraID kW98RobotHead111Light = 106;
static const ExtraID kW98RobotHeadClosesLight = 107;
static const ExtraID kW98WestViewWithGunLight = 108;
static const ExtraID kW98WestViewNoGunLight = 109;

static const CoordType kMoleculesMovieLeft = kNavAreaLeft + 112;
static const CoordType kMoleculesMovieTop = kNavAreaTop + 40;

WSC::WSC(InputHandler *nextHandler, PegasusEngine *owner) : Neighborhood(nextHandler, owner, "WSC", kWSCID),
		_moleculesMovie(kNoDisplayElement) {
	setIsItemTaken(kArgonCanister);
	setIsItemTaken(kSinclairKey);
	setIsItemTaken(kNitrogenCanister);
	setIsItemTaken(kPoisonDart);
	setIsItemTaken(kAntidote);
	setIsItemTaken(kMachineGun);
	setIsItemTaken(kStunGun);

	GameState.setTakenItemID(kArgonPickup, GameState.isTakenItemID(kArgonCanister) &&
			GameState.isTakenItemID(kSinclairKey));
}

uint16 WSC::getDateResID() const {
	return kDate2310ID;
}

void WSC::init() {
	Neighborhood::init();

	_cachedZoomSpot = 0;
	_argonSprite = 0;

	// HACK: Fix the drag item for picking up the Sinclair Key Card
	HotspotInfoTable::Entry *entry = findHotspotEntry(kWSC02SouthTakeArgonSpotID);
	entry->hotspotItem = kArgonPickup;
}

void WSC::flushGameState() {
	g_energyMonitor->saveCurrentEnergyValue();
}

void WSC::start() {
	if (g_energyMonitor) {
		g_energyMonitor->stopEnergyDraining();
		g_energyMonitor->restoreLastEnergyValue();
		_vm->resetEnergyDeathReason();
		g_energyMonitor->startEnergyDraining();
	}

	if (!GameState.getWSCDidPlasmaDodge())
		forceStridingStop(kWSC58, kSouth, kAltWSCNormal);

	Neighborhood::start();
}

class PryDoorMessage : public AIPlayMessageAction {
public:
	PryDoorMessage() : AIPlayMessageAction("Images/AI/WSC/XW59SD3", false) {}

protected:
	virtual void performAIAction(AIRule *);
};

void PryDoorMessage::performAIAction(AIRule *rule) {
	if (((PegasusEngine *)g_engine)->playerHasItemID(kShieldBiochip)
			&& ((PegasusEngine *)g_engine)->getCurrentBiochip()->getObjectID() != kShieldBiochip)
		AIPlayMessageAction::performAIAction(rule);
}

void WSC::setUpAIRules() {
	Neighborhood::setUpAIRules();

	if (g_AIArea) {
		AIPlayMessageAction *messageAction = new AIPlayMessageAction("Images/AI/WSC/XW1WB1", false);
		AILastExtraCondition *extraCondition = new AILastExtraCondition(kWSCDartScan1);
		AIRule *rule = new AIRule(extraCondition, messageAction);
		g_AIArea->addAIRule(rule);

		messageAction = new AIPlayMessageAction("Images/AI/Globals/XGLOB5A", false);
		AILocationCondition *locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kWSC06, kNorth));
		rule = new AIRule(locCondition, messageAction);
		g_AIArea->addAIRule(rule);

		messageAction = new AIPlayMessageAction("Images/AI/Globals/XGLOB5A", false);
		locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kWSC10, kWest));
		rule = new AIRule(locCondition, messageAction);
		g_AIArea->addAIRule(rule);

		messageAction = new AIPlayMessageAction("Images/AI/Globals/XGLOB5A", false);
		locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kWSC28, kWest));
		rule = new AIRule(locCondition, messageAction);
		g_AIArea->addAIRule(rule);

		messageAction = new AIPlayMessageAction("Images/AI/Globals/XGLOB5A", false);
		locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kWSC49, kWest));
		rule = new AIRule(locCondition, messageAction);
		g_AIArea->addAIRule(rule);

		messageAction = new AIPlayMessageAction("Images/AI/Globals/XGLOB5A", false);
		locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kWSC65, kSouth));
		rule = new AIRule(locCondition, messageAction);
		g_AIArea->addAIRule(rule);

		messageAction = new AIPlayMessageAction("Images/AI/Globals/XGLOB5A", false);
		locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kWSC73, kSouth));
		rule = new AIRule(locCondition, messageAction);
		g_AIArea->addAIRule(rule);

		messageAction = new AIPlayMessageAction("Images/AI/Globals/XGLOB5A", false);
		locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kWSC79, kWest));
		rule = new AIRule(locCondition, messageAction);
		g_AIArea->addAIRule(rule);

		messageAction = new AIPlayMessageAction("Images/AI/WSC/XW59SD1", false);
		locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kWSC58, kSouth));
		rule = new AIRule(locCondition, messageAction);
		g_AIArea->addAIRule(rule);

		PryDoorMessage *pryDoorMessage = new PryDoorMessage();
		AIDoorOpenedCondition *doorCondition = new AIDoorOpenedCondition(MakeRoomView(kWSC58, kSouth));
		rule = new AIRule(doorCondition, pryDoorMessage);
		g_AIArea->addAIRule(rule);

		messageAction = new AIPlayMessageAction("Images/AI/WSC/XW61E", false);
		AIHasItemCondition *itemCondition = new AIHasItemCondition(kMachineGun);
		rule = new AIRule(itemCondition, messageAction);
		g_AIArea->addAIRule(rule);

		messageAction = new AIPlayMessageAction("Images/AI/Globals/XGLOB1E", false);
		locCondition = new AILocationCondition(1);
		locCondition->addLocation(MakeRoomView(kWSC95, kWest));
		rule = new AIRule(locCondition, messageAction);
		g_AIArea->addAIRule(rule);
	}
}

Common::String WSC::getBriefingMovie() {
	return "Images/AI/WSC/XWO";
}

Common::String WSC::getEnvScanMovie() {
	RoomID room = GameState.getCurrentRoom();

	if (room >= kWSC01 && room <= kWSC04)
		return "Images/AI/WSC/XWE1";
	else if (room >= kWSC06 && room <= kWSC58)
		return "Images/AI/WSC/XWE2";
	else if (room >= kWSC60 && room <= kWSC61West)
		return "Images/AI/WSC/XWE3";
	else if (room >= kWSC64 && room <= kWSC98)
		return "Images/AI/WSC/XWE4";

	return "Images/AI/WSC/XWE5";
}

uint WSC::getNumHints() {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kWSC10, kWest):
	case MakeRoomView(kWSC28, kWest):
	case MakeRoomView(kWSC49, kWest):
	case MakeRoomView(kWSC65, kSouth):
	case MakeRoomView(kWSC75, kSouth):
	case MakeRoomView(kWSC79, kWest):
		return 2;
	case MakeRoomView(kWSC02, kSouth):
		if (_vm->getEnergyDeathReason() == kDeathDidntStopPoison &&
				!_privateFlags.getFlag(kWSCPrivateInMoleculeGameFlag) &&
				!GameState.getWSCDesignedAntidote())
			return 3;
		else if (!GameState.getScoringGotNitrogenCanister() ||
				!GameState.getScoringGotSinclairKey())
			return 1;
		break;
	case MakeRoomView(kWSC03, kNorth):
		// WORKAROUND: The original game is missing the first two hint movies and
		// just plays nothing in its stead. We'll just return that we have one
		// hint available.
		if (inSynthesizerGame())
			return 1;

		// fall through
	case MakeRoomView(kWSC01, kNorth):
	case MakeRoomView(kWSC01, kSouth):
	case MakeRoomView(kWSC01, kEast):
	case MakeRoomView(kWSC01, kWest):
	case MakeRoomView(kWSC02, kNorth):
	case MakeRoomView(kWSC02, kEast):
	case MakeRoomView(kWSC02, kWest):
	case MakeRoomView(kWSC02Morph, kNorth):
	case MakeRoomView(kWSC02Morph, kEast):
	case MakeRoomView(kWSC02Morph, kWest):
	case MakeRoomView(kWSC02Messages, kNorth):
	case MakeRoomView(kWSC02Messages, kEast):
	case MakeRoomView(kWSC02Messages, kWest):
	case MakeRoomView(kWSC03, kSouth):
	case MakeRoomView(kWSC03, kEast):
	case MakeRoomView(kWSC03, kWest):
	case MakeRoomView(kWSC04, kNorth):
	case MakeRoomView(kWSC04, kSouth):
	case MakeRoomView(kWSC04, kEast):
	case MakeRoomView(kWSC04, kWest):
		if (_vm->getEnergyDeathReason() == kDeathDidntStopPoison &&
				!_privateFlags.getFlag(kWSCPrivateInMoleculeGameFlag) &&
				!GameState.getWSCDesignedAntidote())
			return 3;
		break;
	case MakeRoomView(kWSC02Messages, kSouth):
		if (_vm->getEnergyDeathReason() == kDeathDidntStopPoison &&
				!_privateFlags.getFlag(kWSCPrivateInMoleculeGameFlag) &&
				!GameState.getWSCDesignedAntidote())
			return 3;
		else if (!GameState.getScoringGotNitrogenCanister())
			return 1;
		break;
	case MakeRoomView(kWSC02Morph, kSouth):
		if (_vm->getEnergyDeathReason() == kDeathDidntStopPoison &&
				!_privateFlags.getFlag(kWSCPrivateInMoleculeGameFlag) &&
				!GameState.getWSCDesignedAntidote())
			return 3;
		else if (!GameState.getScoringGotSinclairKey())
			return 1;
		break;
	case MakeRoomView(kWSC42, kEast):
		if (!GameState.isCurrentDoorOpen())
			return 1;
		break;
	case MakeRoomView(kWSC58, kSouth):
		if (GameState.isCurrentDoorOpen()) {
			if (GameState.getWSCDidPlasmaDodge())
				return 0;
			else
				return 1;
		} else if (_vm->playerHasItemID(kCrowbar))
			return 2;

		return 3;
	case MakeRoomView(kWSC61, kEast):
		if (!GameState.getScoringSawBrochure())
			return 1;
		break;
	case MakeRoomView(kWSC61, kSouth):
		if (!GameState.getScoringSawSinclairEntry1() ||
				!GameState.getScoringSawSinclairEntry2() ||
				!GameState.getScoringSawSinclairEntry3())
			return 1;
		break;
	case MakeRoomView(kWSC98, kWest):
		if (getCurrentActivation() == kActivationRobotTurning)
			return 1;
		break;
	}

	return 0;
}

Common::String WSC::getHintMovie(uint hintNum) {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kWSC10, kWest):
	case MakeRoomView(kWSC28, kWest):
	case MakeRoomView(kWSC49, kWest):
	case MakeRoomView(kWSC65, kSouth):
	case MakeRoomView(kWSC75, kSouth):
	case MakeRoomView(kWSC79, kWest):
		if (hintNum == 1)
			return "Images/AI/Globals/XGLOB5B";

		return "Images/AI/Globals/XGLOB5C";
	case MakeRoomView(kWSC02, kSouth):
		if (_vm->getEnergyDeathReason() == kDeathDidntStopPoison &&
				!_privateFlags.getFlag(kWSCPrivateInMoleculeGameFlag) &&
				!GameState.getWSCDesignedAntidote())
			return Common::String::format("Images/AI/WSC/XWPH%d", hintNum);

		return "Images/AI/Globals/XGLOB1C";
	case MakeRoomView(kWSC61, kEast):
	case MakeRoomView(kWSC61, kSouth):
		return "Images/AI/Globals/XGLOB1C";
	case MakeRoomView(kWSC42, kEast):
		if (_vm->playerHasItemID(kSinclairKey))
			return "Images/AI/Globals/XGLOB1A";

		return "Images/AI/Globals/XGLOB2C";
	case MakeRoomView(kWSC58, kSouth):
		switch (hintNum) {
		case 1:
			if (GameState.isCurrentDoorOpen()) {
				// Only get here if we haven't done the plasma dodge game...
				if (_vm->playerHasItemID(kShieldBiochip))
					return "Images/AI/Globals/XGLOB1A";
				else
					return "Images/AI/Globals/XGLOB3F";
			} else if (_vm->playerHasItemID(kCrowbar)) {
				return "Images/AI/Globals/XGLOB1A";
			}

			return "Images/AI/Globals/XGLOB1B";
		case 2:
			// Only get here if the door is still locked...
			if (_vm->playerHasItemID(kCrowbar))
				return "Images/AI/WSC/XW59SD2";

			return "Images/AI/Globals/XGLOB2D";
		case 3:
			// Only get here if the door is still locked and we don't have the
			// crowbar...
			return "Images/AI/WSC/XW59SD2";
		}
		break;
	case MakeRoomView(kWSC03, kNorth):
		// WORKAROUND: The original game is missing the first two hint movies and
		// just plays nothing in its stead. We just make it the first hint.
		if (inSynthesizerGame())
			return "Images/AI/WSC/XW03NH3";

		// fall through
	case MakeRoomView(kWSC01, kNorth):
	case MakeRoomView(kWSC01, kSouth):
	case MakeRoomView(kWSC01, kEast):
	case MakeRoomView(kWSC01, kWest):
	case MakeRoomView(kWSC02, kNorth):
	case MakeRoomView(kWSC02, kEast):
	case MakeRoomView(kWSC02, kWest):
	case MakeRoomView(kWSC02Morph, kNorth):
	case MakeRoomView(kWSC02Morph, kEast):
	case MakeRoomView(kWSC02Morph, kWest):
	case MakeRoomView(kWSC02Messages, kNorth):
	case MakeRoomView(kWSC02Messages, kEast):
	case MakeRoomView(kWSC02Messages, kWest):
	case MakeRoomView(kWSC03, kSouth):
	case MakeRoomView(kWSC03, kEast):
	case MakeRoomView(kWSC03, kWest):
	case MakeRoomView(kWSC04, kNorth):
	case MakeRoomView(kWSC04, kSouth):
	case MakeRoomView(kWSC04, kEast):
	case MakeRoomView(kWSC04, kWest):
		// analyzer hint
		return Common::String::format("Images/AI/WSC/XWPH%d", hintNum);
	case MakeRoomView(kWSC02Messages, kSouth):
	case MakeRoomView(kWSC02Morph, kSouth):
		if (_vm->getEnergyDeathReason() == kDeathDidntStopPoison &&
				!_privateFlags.getFlag(kWSCPrivateInMoleculeGameFlag) &&
				!GameState.getWSCDesignedAntidote())
			// analyzer hint
			return Common::String::format("Images/AI/WSC/XWPH%d", hintNum);

		return "Images/AI/Globals/XGLOB1C";
	case MakeRoomView(kWSC98, kWest):
		return "Images/AI/WSC/XW98WH2";
	}

	return "";
}

void WSC::prepareForAIHint(const Common::String &movieName) {
	if (movieName == "Images/AI/WSC/XW98WH2" && isEventTimerRunning())
		pauseTimer();
}

void WSC::cleanUpAfterAIHint(const Common::String &movieName) {
	if (movieName == "Images/AI/WSC/XW98WH2" && isEventTimerRunning())
		resumeTimer();
}

bool WSC::okayToJump() {
	if (GameState.getWSCPoisoned()) {
		die(kDeathDidntStopPoison);
		return false;
	}

	bool result = Neighborhood::okayToJump();
	if (!result)
		playSpotSoundSync(kWSCCantTransportIn, kWSCCantTransportOut);

	return result;
}

TimeValue WSC::getViewTime(const RoomID room, const DirectionConstant direction) {
	ExtraID viewExtra = 0xffffffff;
	ExtraTable::Entry extra;

	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kWSC01, kWest):
		if (!GameState.getWSCSeenTimeStream()) {
			getExtraEntry(kWSCArrivalFromTSA, extra);
			return extra.movieStart;
		} else if (GameState.getWSCPoisoned() && !GameState.getWSCAnsweredAboutDart()) {
			viewExtra = kWSCDartScan1;
		}
		break;
	case MakeRoomView(kWSC02Morph, kSouth):
		if (GameState.isTakenItemID(kArgonPickup) || GameState.isTakenItemID(kArgonCanister))
			viewExtra = kWSC02SouthViewNoArgon;
		break;
	case MakeRoomView(kWSC02Messages, kSouth):
		if (GameState.isTakenItemID(kNitrogenCanister)) {
			if (_privateFlags.getFlag(kWSCPrivateLabMessagesOpenFlag))
				viewExtra = kMessagesViewMachineOnNoNitrogen;
			else
				viewExtra = kMessagesViewNoNitrogen;
		}
		break;
	case MakeRoomView(kWSC03, kSouth):
		if (_privateFlags.getFlag(kWSCDraggingAntidoteFlag))
			viewExtra = kW03SouthViewNoAntidote;
		break;
	case MakeRoomView(kWSC17, kWest):
		if (_privateFlags.getFlag(kWSCPrivateNeedPeopleAt17WestFlag))
			viewExtra = kW17WestPeopleCrossingView;
		break;
	case MakeRoomView(kWSC49, kNorth):
		if (_privateFlags.getFlag(kWSCPrivateNeedPeopleAt49NorthFlag))
			viewExtra = kW49NorthPeopleCrossingView;
		break;
	case MakeRoomView(kWSC73, kWest):
		if (_privateFlags.getFlag(kWSCPrivateNeedPeopleAt73WestFlag))
			viewExtra = kW73WestPeopleCrossingView;
		break;
	case MakeRoomView(kWSC98, kWest):
		if (GameState.getWSCRobotDead()) {
			if (GameState.getWSCRobotGone()) {
				if (GameState.isTakenItemID(kStunGun)) {
					if (GameState.getWSCCatwalkDark())
						viewExtra = kW98WestViewNoGunDark;
					else
						viewExtra = kW98WestViewNoGunLight;
				} else {
					if (GameState.getWSCCatwalkDark())
						viewExtra = kW98WestViewWithGunDark;
					else
						viewExtra = kW98WestViewWithGunLight;
				}
			} else if (_privateFlags.getFlag(kWSCPrivateRobotHeadOpenFlag)) {
				if (GameState.getWSCCatwalkDark())
					viewExtra = kW98RobotHead111Dark;
				else
					viewExtra = kW98RobotHead111Light;

				if (_privateFlags.getFlag(kWSCPrivateGotRetScanChipFlag))
					viewExtra -= 1;
				if (_privateFlags.getFlag(kWSCPrivateGotMapChipFlag))
					viewExtra -= 2;
				if (_privateFlags.getFlag(kWSCPrivateGotOpticalChipFlag))
					viewExtra -= 4;
			} else if (GameState.getWSCRobotDead()) {
				// Should only happen on loading a saved game, so it can take its time.
				if (GameState.getWSCCatwalkDark())
					viewExtra = kW98RobotShocked;
				else
					viewExtra = kW98RobotGassed;
			}
		}
		break;
	}

	if (viewExtra != 0xffffffff) {
		getExtraEntry(viewExtra, extra);
		return extra.movieEnd - 1;
	}

	return Neighborhood::getViewTime(room, direction);
}

void WSC::findSpotEntry(const RoomID room, const DirectionConstant direction, SpotFlags flags, SpotTable::Entry &spotEntry) {
	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kWSC58, kSouth):
	case MakeRoomView(kWSC79, kWest):
		if ((flags & kSpotOnTurnMask) != 0) {
			spotEntry.clear();
			return;
		}
		break;
	}

	Neighborhood::findSpotEntry(room, direction, flags, spotEntry);
}

void WSC::getZoomEntry(const HotSpotID id, ZoomTable::Entry &zoomEntry) {
	Neighborhood::getZoomEntry(id, zoomEntry);

	ExtraTable::Entry extra;
	ExtraID zoomExtra = 0xffffffff;

	switch (id) {
	case kWSC02SouthMessagesSpotID:
		if (GameState.isTakenItemID(kNitrogenCanister))
			zoomExtra = kWSC02MessagesZoomNoNitrogen;
		break;
	case kWSC02SouthMessagesOutSpotID:
		if (GameState.isTakenItemID(kNitrogenCanister))
			zoomExtra = kMessagesZoomOutNoNitrogen;
		break;
	case kWSC02SouthMorphSpotID:
		if (GameState.isTakenItemID(kArgonCanister))
			zoomExtra = kWSC02MorphZoomNoArgon;
		break;
	case kWSC02SouthMorphOutSpotID:
		if (GameState.isTakenItemID(kArgonCanister))
			zoomExtra = kWSC02ZoomOutNoArgon;
		break;
	case kWSC61SouthSpotID:
		if (GameState.isTakenItemID(kMachineGun))
			zoomExtra = kW61SouthZoomInNoGun;
		break;
	case kWSC61SouthOutSpotID:
		if (GameState.isTakenItemID(kMachineGun))
			zoomExtra = kW61SouthZoomOutNoGun;
		break;
	}

	if (zoomExtra != 0xffffffff) {
		getExtraEntry(zoomExtra, extra);
		zoomEntry.movieStart = extra.movieStart;
		zoomEntry.movieEnd = extra.movieEnd;
	}
}

void WSC::getExtraEntry(const uint32 id, ExtraTable::Entry &extraEntry) {
	switch (id) {
	case kWSCZoomOutFromAnalyzer:
		Neighborhood::getExtraEntry(kWSCZoomOutFromAnalyzer, extraEntry);
		extraEntry.movieEnd = extraEntry.movieStart + 14 * kWSCFrameDuration;
		break;
	case kW61WalchekMessage:
		if (GameState.getEasterEgg())
			Neighborhood::getExtraEntry(kW61WalchekEasterEgg1, extraEntry);
		else
			Neighborhood::getExtraEntry(id, extraEntry);
		break;
	case kW61SouthScreenOnWithGun:
		if (GameState.isTakenItemID(kMachineGun))
			Neighborhood::getExtraEntry(id, extraEntry);
		else
			Neighborhood::getExtraEntry(kW61SouthScreenOnNoGun, extraEntry);
		break;
	case kW61SouthSmartAlloysWithGun:
		if (GameState.isTakenItemID(kMachineGun))
			Neighborhood::getExtraEntry(id, extraEntry);
		else
			Neighborhood::getExtraEntry(kW61SouthSmartAlloysNoGun, extraEntry);
		break;
	case kW61SouthMorphingWithGun:
		if (GameState.isTakenItemID(kMachineGun))
			Neighborhood::getExtraEntry(id, extraEntry);
		else
			Neighborhood::getExtraEntry(kW61SouthMorphingNoGun, extraEntry);
		break;
	case kW61SouthTimeBendingWithGun:
		if (GameState.isTakenItemID(kMachineGun))
			Neighborhood::getExtraEntry(id, extraEntry);
		else
			Neighborhood::getExtraEntry(kW61SouthTimeBendingNoGun, extraEntry);
		break;
	case kW98RobotHeadOpensLight:
		if (GameState.getWSCCatwalkDark())
			Neighborhood::getExtraEntry(kW98RobotHeadOpensDark, extraEntry);
		else
			Neighborhood::getExtraEntry(id, extraEntry);
		break;
	default:
		Neighborhood::getExtraEntry(id, extraEntry);
		break;
	}
}

CanMoveForwardReason WSC::canMoveForward(ExitTable::Entry &entry) {
	if (GameState.getCurrentRoomAndView() == MakeRoomView(kWSC01, kWest) &&
			getCurrentActivation() != kActivateHotSpotAlways)
		return kCantMoveWatchingDiagnosis;

	return Neighborhood::canMoveForward(entry);
}

// Also add cases here for compound analyzer...
CanTurnReason WSC::canTurn(TurnDirection turnDirection, DirectionConstant &nextDir) {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kWSC01, kWest):
		if (getCurrentActivation() != kActivateHotSpotAlways)
			return kCantTurnWatchingDiagnosis;
		break;
	case MakeRoomView(kWSC01, kEast):
		if (getCurrentActivation() != kActivateHotSpotAlways)
			return kCantTurnWatchingAnalysis;
		break;
	case MakeRoomView(kWSC03, kNorth):
		if (_privateFlags.getFlag(kWSCPrivateInMoleculeGameFlag))
			return kCantTurnInMoleculeGame;
		break;
	}

	return Neighborhood::canTurn(turnDirection, nextDir);
}

CanOpenDoorReason WSC::canOpenDoor(DoorTable::Entry &entry) {
	switch (GameState.getCurrentRoom()) {
	case kWSC42:
		if (!_privateFlags.getFlag(kWSCPrivateSinclairOfficeOpenFlag))
			return kCantOpenLocked;
		break;
	case kWSC58:
		if (!_privateFlags.getFlag(kWSCPrivate58SouthOpenFlag))
			return kCantOpenLocked;
		break;
	}

	return Neighborhood::canOpenDoor(entry);
}

void WSC::bumpIntoWall() {
	requestSpotSound(kWSCBumpIntoWallIn, kWSCBumpIntoWallOut, kFilterAllInput, 0);
	Neighborhood::bumpIntoWall();
}

void WSC::closeDoorOffScreen(const RoomID room, const DirectionConstant) {
	Item *keyCard;

	switch (room) {
	case kWSC58:
	case kWSC62:
	case kWSC63:
	case kWSC64:
	case kWSC85:
	case kWSC86:
	case kWSC88:
	case kWSC89:
		playSpotSoundSync(kSlidingDoorCloseIn, kSlidingDoorCloseOut);
		break;
	case kWSC81:
	case kWSC82:
	case kWSC92:
	case kWSC93:
		keyCard = _vm->getAllItems().findItemByID(kKeyCard);
		if (keyCard->getItemState() == kFlashlightOn && (GameState.getCurrentRoom() == kWSC81 ||
				GameState.getCurrentRoom() == kWSC93)) {
			keyCard->setItemState(kFlashlightOff);
			playSpotSoundSync(kWSCFlashlightClickIn, kWSCFlashlightClickOut);
		} else if (keyCard->getItemState() == kFlashlightOff && (GameState.getCurrentRoom() == kWSC82 ||
				GameState.getCurrentRoom() == kWSC92)) {
			keyCard->setItemState(kFlashlightOn);
			playSpotSoundSync(kWSCFlashlightClickIn, kWSCFlashlightClickOut);
		}

		playSpotSoundSync(kSlimyDoorCloseIn, kSlimyDoorCloseOut);
		break;
	default:
		playSpotSoundSync(kSlimyDoorCloseIn, kSlimyDoorCloseOut);
		break;
	}
}

void WSC::cantMoveThatWay(CanMoveForwardReason reason) {
	if (reason != kCantMoveWatchingDiagnosis)
		Neighborhood::cantMoveThatWay(reason);
}

void WSC::cantOpenDoor(CanOpenDoorReason reason) {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kWSC22, kWest):
		playSpotSoundSync(kNakamuraNotHomeIn, kNakamuraNotHomeOut);
		break;
	case MakeRoomView(kWSC23, kEast):
		playSpotSoundSync(kHernandezNotHomeIn, kHernandezNotHomeOut);
		break;
	case MakeRoomView(kWSC26, kWest):
		playSpotSoundSync(kGrailisNotHomeIn, kGrailisNotHomeOut);
		break;
	case MakeRoomView(kWSC27, kEast):
		playSpotSoundSync(kWashingtonNotHomeIn, kWashingtonNotHomeOut);
		break;
	case MakeRoomView(kWSC32, kWest):
		playSpotSoundSync(kTheriaultNotHomeIn, kTheriaultNotHomeOut);
		break;
	case MakeRoomView(kWSC33, kEast):
		playSpotSoundSync(kSullivanNotHomeIn, kSullivanNotHomeOut);
		break;
	case MakeRoomView(kWSC41, kWest):
		playSpotSoundSync(kGlennerNotHomeIn, kGlennerNotHomeOut);
		break;
	case MakeRoomView(kWSC42, kEast):
		playSpotSoundSync(kSinclairNotHomeIn, kSinclairNotHomeOut);
		break;
	case MakeRoomView(kWSC15, kWest):
	case MakeRoomView(kWSC25, kWest):
	case MakeRoomView(kWSC33, kWest):
	case MakeRoomView(kWSC41, kEast):
	case MakeRoomView(kWSC46, kWest):
		playSpotSoundSync(kWSCLabClosedIn, kWSCLabClosedOut);
		break;
	default:
		Neighborhood::cantOpenDoor(reason);
		break;
	}
}

void WSC::doorOpened() {
	Neighborhood::doorOpened();

	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kWSC42, kEast):
		_vm->addItemToInventory((InventoryItem *)_vm->getAllItems().findItemByID(kSinclairKey));
		break;
	case MakeRoomView(kWSC58, kSouth):
		GameState.setScoringUsedCrowBarInWSC();
		_vm->addItemToInventory((InventoryItem *)_vm->getAllItems().findItemByID(kCrowbar));
		break;
	case MakeRoomView(kWSC06, kNorth):
	case MakeRoomView(kWSC79, kWest):
		die(kDeathArrestedInWSC);
		break;
	case MakeRoomView(kWSC60, kWest):
		if (_vm->itemInInventory(kMachineGun))
			startExtraSequence(kNerdAtTheDoor2, kExtraCompletedFlag, kFilterNoInput);
		else if (!GameState.getWSCSeenNerd())
			startExtraSequence(kNerdAtTheDoor1, kExtraCompletedFlag, kFilterNoInput);
		break;
	case MakeRoomView(kWSC95, kWest):
		GameState.setScoringOpenedCatwalk();
		scheduleEvent(kGawkAtRobotTime, 1, kTimerEventPlayerGawkingAtRobot);
		break;
	}
}

void WSC::turnLeft() {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kWSC17, kNorth):
		if (!_privateFlags.getFlag(kWSCPrivateSeenPeopleAt17WestFlag) && _vm->getRandomNumber(2) == 0)
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt17WestFlag, true);
		break;
	case MakeRoomView(kWSC49, kEast):
		if (!_privateFlags.getFlag(kWSCPrivateSeenPeopleAt49NorthFlag) && _vm->getRandomNumber(2) == 0)
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt49NorthFlag, true);
		break;
	case MakeRoomView(kWSC73, kNorth):
		if (!_privateFlags.getFlag(kWSCPrivateSeenPeopleAt73WestFlag) && _vm->getRandomNumber(2) == 0)
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt73WestFlag, true);
		break;
	case MakeRoomView(kWSC73, kWest):
		if (!GameState.getWSCBeenAtWSC93())
			setCurrentAlternate(kAltWSCW0ZDoorOpen);
		break;
	case MakeRoomView(kWSC95, kWest):
		cancelEvent();
		break;
	}

	Neighborhood::turnLeft();
}

void WSC::turnRight() {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kWSC17, kSouth):
		if (!_privateFlags.getFlag(kWSCPrivateSeenPeopleAt17WestFlag) && _vm->getRandomNumber(2) == 0)
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt17WestFlag, true);
		break;
	case MakeRoomView(kWSC49, kWest):
		if (!_privateFlags.getFlag(kWSCPrivateSeenPeopleAt49NorthFlag) && _vm->getRandomNumber(2) == 0)
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt49NorthFlag, true);
		break;
	case MakeRoomView(kWSC73, kSouth):
		if (!_privateFlags.getFlag(kWSCPrivateSeenPeopleAt73WestFlag) && _vm->getRandomNumber(2) == 0)
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt73WestFlag, true);
		break;
	case MakeRoomView(kWSC73, kEast):
		if (!GameState.getWSCBeenAtWSC93())
			setCurrentAlternate(kAltWSCW0ZDoorOpen);
		break;
	case MakeRoomView(kWSC95, kWest):
		cancelEvent();
		break;
	}

	Neighborhood::turnRight();
}

void WSC::moveForward() {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kWSC19, kNorth):
		if (!_privateFlags.getFlag(kWSCPrivateSeenPeopleAt19NorthFlag))
			setCurrentAlternate(kAltWSCPeopleAtW19North);
		break;
	case MakeRoomView(kWSC95, kWest):
		cancelEvent();
		break;
	}

	Neighborhood::moveForward();
}

void WSC::zoomTo(const Hotspot *hotspot) {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kWSC02Messages, kSouth):
		if (_privateFlags.getFlag(kWSCPrivateLabMessagesOpenFlag)) {
			_cachedZoomSpot = hotspot;
			if (GameState.isTakenItemID(kNitrogenCanister))
				startExtraSequence(kMessagesOffNoNitrogen, kExtraCompletedFlag, kFilterNoInput);
			else
				startExtraSequence(kMessagesOff, kExtraCompletedFlag, kFilterNoInput);
			return;
		}
		break;
	case MakeRoomView(kWSC61West, kWest):
		if (GameState.getWSCOfficeMessagesOpen()) {
			_cachedZoomSpot = hotspot;
			startExtraSequence(kW61MessagesOff, kExtraCompletedFlag, kFilterNoInput);
			return;
		}
		break;
	case MakeRoomView(kWSC61South, kSouth):
		if (_privateFlags.getFlag(kWSCPrivateOfficeLogOpenFlag)) {
			_cachedZoomSpot = hotspot;
			if (GameState.isTakenItemID(kMachineGun))
				startExtraSequence(kW61SouthScreenOffNoGun, kExtraCompletedFlag, kFilterNoInput);
			else
				startExtraSequence(kW61SouthScreenOffWithGun, kExtraCompletedFlag, kFilterNoInput);
			return;
		}
		break;
	}

	Neighborhood::zoomTo(hotspot);
}

void WSC::startExtraSequence(const ExtraID extraID, const NotificationFlags flags, const InputBits interruptionFilter) {
	if (extraID == kW61Brochure)
		loadLoopSound1("");

	Neighborhood::startExtraSequence(extraID, flags, interruptionFilter);
}

int16 WSC::getStaticCompassAngle(const RoomID room, const DirectionConstant dir) {
	int16 angle = Neighborhood::getStaticCompassAngle(room, dir);

	switch (room) {
	case kWSC02Messages:
		angle -= 50;
		break;
	case kWSC02Morph:
		angle += 5;
		break;
	case kWSC60East:
		angle -= 10;
		break;
	case kWSC66:
		angle -= kAuditoriumAngleOffset;
		break;
	case kWSC67:
		angle += kAuditoriumAngleOffset;
		break;
	case kWSC68:
		angle -= kAuditoriumAngleOffset * 2;
		break;
	case kWSC69:
		angle += kAuditoriumAngleOffset * 2;
		break;
	case kWSC70:
		angle -= kAuditoriumAngleOffset * 3;
		break;
	case kWSC71:
		angle += kAuditoriumAngleOffset * 3;
		break;
	case kWSC72:
		if (dir == kEast || dir == kWest)
			angle -= kAuditoriumAngleOffset * 4;
		break;
	case kWSC73:
		if (dir == kEast || dir == kWest)
			angle += kAuditoriumAngleOffset * 4;
		break;
	}

	return angle;
}

void WSC::getExitCompassMove(const ExitTable::Entry &exitEntry, FaderMoveSpec &compassMove) {
	Neighborhood::getExitCompassMove(exitEntry, compassMove);

	if (exitEntry.room == kWSC65 && exitEntry.direction == kSouth) {
		compassMove.insertFaderKnot(exitEntry.movieStart + 100 * kWSCFrameDuration, 180);
		compassMove.insertFaderKnot(exitEntry.movieStart + 108 * kWSCFrameDuration, 150);
		compassMove.insertFaderKnot(exitEntry.movieEnd, 150);
	}
}

void WSC::getExtraCompassMove(const ExtraTable::Entry &entry, FaderMoveSpec &compassMove) {
	switch (entry.extra) {
	case kW61Brochure:
		compassMove.insertFaderKnot(entry.movieStart + 15 * kWSCFrameDuration, 85);
		compassMove.insertFaderKnot(entry.movieEnd - 15 * kWSCFrameDuration, 85);
		compassMove.insertFaderKnot(entry.movieEnd, 90);
		break;
	default:
		Neighborhood::getExtraCompassMove(entry, compassMove);
		break;
	}
}

void WSC::loadAmbientLoops() {
	RoomID room = GameState.getCurrentRoom();

	if (room >= kWSC01 && room <= kWSC04) {
		if (GameState.getWSCSeenTimeStream())
			loadLoopSound1("Sounds/World Science Center/WLabLoop.22K.AIFF", 0x100 / 2);
	} else if ((room >= kWSC06 && room <= kWSC58) || (room >= kWSC62 && room <= kWSC63))
		loadLoopSound1("Sounds/World Science Center/Organic Walls.22K.AIFF", 0x100 / 2);
	else if (room >= kWSC82 && room <= kWSC92)
		loadLoopSound1("Sounds/World Science Center/Creature Feature.22K.AIFF");
	else if ((room >= kWSC60 && room <= kWSC61West) || (room >= kWSC64 && room <= kWSC81) ||
			(room >= kWSC93 && room <= kWSC97))
		loadLoopSound1("Sounds/World Science Center/The Other Side.22K.AIFF", 0x100 / 12);
	else if (room == kWSC98)
		loadLoopSound1("Sounds/World Science Center/WCatLoop.22K.AIFF");
}

void WSC::checkContinuePoint(const RoomID room, const DirectionConstant direction) {
	switch (MakeRoomView(room, direction)) {
	case MakeRoomView(kWSC07, kNorth):
	case MakeRoomView(kWSC11, kSouth):
	case MakeRoomView(kWSC13, kSouth):
	case MakeRoomView(kWSC13, kWest):
	case MakeRoomView(kWSC16, kWest):
	case MakeRoomView(kWSC17, kEast):
	case MakeRoomView(kWSC19, kWest):
	case MakeRoomView(kWSC28, kNorth):
	case MakeRoomView(kWSC28, kSouth):
	case MakeRoomView(kWSC28, kEast):
	case MakeRoomView(kWSC28, kWest):
	case MakeRoomView(kWSC29, kNorth):
	case MakeRoomView(kWSC29, kSouth):
	case MakeRoomView(kWSC29, kEast):
	case MakeRoomView(kWSC29, kWest):
	case MakeRoomView(kWSC40, kEast):
	case MakeRoomView(kWSC42, kEast):
	case MakeRoomView(kWSC49, kWest):
	case MakeRoomView(kWSC49, kNorth):
	case MakeRoomView(kWSC50, kNorth):
	case MakeRoomView(kWSC55, kEast):
	case MakeRoomView(kWSC65, kSouth):
	case MakeRoomView(kWSC65, kEast):
	case MakeRoomView(kWSC65, kWest):
	case MakeRoomView(kWSC72, kEast):
	case MakeRoomView(kWSC72, kSouth):
	case MakeRoomView(kWSC73, kWest):
	case MakeRoomView(kWSC73, kSouth):
	case MakeRoomView(kWSC79, kWest):
	case MakeRoomView(kWSC81, kEast):
	case MakeRoomView(kWSC93, kNorth):
	case MakeRoomView(kWSC95, kWest):
		makeContinuePoint();
		break;
	case MakeRoomView(kWSC58, kSouth):
		if (!GameState.getWSCDidPlasmaDodge())
			makeContinuePoint();
		break;
	case MakeRoomView(kWSC60, kWest):
		if (_vm->playerHasItemID(kMachineGun))
			makeContinuePoint();
		break;
	}
}

void WSC::arriveAt(const RoomID room, const DirectionConstant dir) {
	switch (MakeRoomView(room, dir)) {
	case MakeRoomView(kWSC60, kNorth):
	case MakeRoomView(kWSC60, kSouth):
	case MakeRoomView(kWSC60, kEast):
	case MakeRoomView(kWSC60, kWest):
	case MakeRoomView(kWSC60East, kNorth):
	case MakeRoomView(kWSC60East, kSouth):
	case MakeRoomView(kWSC60East, kEast):
	case MakeRoomView(kWSC60East, kWest):
	case MakeRoomView(kWSC60North, kNorth):
	case MakeRoomView(kWSC60North, kSouth):
	case MakeRoomView(kWSC60North, kEast):
	case MakeRoomView(kWSC60North, kWest):
	case MakeRoomView(kWSC61, kNorth):
	case MakeRoomView(kWSC61, kSouth):
	case MakeRoomView(kWSC61, kEast):
	case MakeRoomView(kWSC61, kWest):
	case MakeRoomView(kWSC61South, kNorth):
	case MakeRoomView(kWSC61South, kSouth):
	case MakeRoomView(kWSC61South, kEast):
	case MakeRoomView(kWSC61South, kWest):
	case MakeRoomView(kWSC61West, kNorth):
	case MakeRoomView(kWSC61West, kSouth):
	case MakeRoomView(kWSC61West, kEast):
	case MakeRoomView(kWSC61West, kWest):
		if (GameState.isTakenItemID(kMachineGun))
			setCurrentAlternate(kAltWSCTookMachineGun);
		else
			setCurrentAlternate(kAltWSCNormal);
		break;
	case MakeRoomView(kWSC73, kSouth):
	case MakeRoomView(kWSC75, kNorth):
	case MakeRoomView(kWSC75, kSouth):
	case MakeRoomView(kWSC75, kEast):
	case MakeRoomView(kWSC75, kWest):
		if (!GameState.getWSCBeenAtWSC93())
			setCurrentAlternate(kAltWSCW0ZDoorOpen);
		break;
	}

	Neighborhood::arriveAt(room, dir);

	switch (MakeRoomView(room, dir)) {
	case MakeRoomView(kWSC01, kWest):
		if (!GameState.getWSCSeenTimeStream()) {
			requestExtraSequence(kWSCArrivalFromTSA, kExtraCompletedFlag, kFilterNoInput);
			requestExtraSequence(kWSCShotByRobot, 0, kFilterNoInput);
			requestExtraSequence(kWSCDartScan1, kExtraCompletedFlag, kFilterNoInput);
		} else if (GameState.getWSCPoisoned() && !GameState.getWSCAnsweredAboutDart()) {
			setCurrentActivation(kActivationShotByRobot);
		}
		break;
	case MakeRoomView(kWSC01, kEast):
		if (GameState.getWSCDartInAnalyzer())
			requestExtraSequence(kWSCDropDartIntoAnalyzer, kExtraCompletedFlag, kFilterNoInput);
		break;
	case MakeRoomView(kWSC02Morph, kSouth):
		setCurrentActivation(kActivationMorphScreenOff);
		break;
	case MakeRoomView(kWSC03, kNorth):
		setCurrentActivation(kActivationW03NorthOff);
		break;
	case MakeRoomView(kWSC03, kSouth):
		if (GameState.getWSCDesignedAntidote() && !GameState.getWSCPickedUpAntidote())
			setCurrentActivation(kActivationReadyForSynthesis);
		break;
	case MakeRoomView(kWSC16, kNorth):
		if (getCurrentAlternate() == kAltWSCPeopleAtW19North) {
			setCurrentAlternate(kAltWSCNormal);
			_privateFlags.setFlag(kWSCPrivateSeenPeopleAt19NorthFlag, true);
		}
		break;
	case MakeRoomView(kWSC07, kSouth):
	case MakeRoomView(kWSC56, kNorth):
		setCurrentActivation(kActivationReadyForMap);
		break;
	case MakeRoomView(kWSC42, kWest):
		setCurrentAlternate(kAltWSCNormal);
		break;
	case MakeRoomView(kWSC42, kEast):
		_privateFlags.setFlag(kWSCPrivateSinclairOfficeOpenFlag, false);
		setCurrentActivation(kActivationSinclairOfficeLocked);
		break;
	case MakeRoomView(kWSC58, kSouth):
		setCurrentActivation(kActivationW58SouthDoorLocked);
		_privateFlags.setFlag(kWSCPrivate58SouthOpenFlag, false);
		break;
	case MakeRoomView(kWSC60, kEast):
		GameState.setScoringEnteredSinclairOffice();
		break;
	case MakeRoomView(kWSC61West, kWest):
		setCurrentActivation(kActivationW61MessagesOff);
		break;
	case MakeRoomView(kWSC61South, kSouth):
		setCurrentActivation(kActivationW61SouthOff);
		break;
	case MakeRoomView(kWSC62, kSouth):
		if (!GameState.getWSCDidPlasmaDodge()) {
			g_AIArea->lockAIOut();
			loadLoopSound1("Sounds/World Science Center/Plasma Rock.22K.AIFF");
			requestExtraSequence(kW62SouthPlasmaRobotAppears, 0, kFilterNoInput);
			requestExtraSequence(kW62ZoomToRobot, 0, kFilterNoInput);
			requestExtraSequence(kW62ZoomOutFromRobot, kExtraCompletedFlag, kFilterNoInput);
		}
		break;
	case MakeRoomView(kWSC65Screen, kSouth):
		if (!GameState.getWSCSeenSinclairLecture()) {
			GameState.setWSCSeenSinclairLecture(true);
			startExtraSequence(kW65SouthSinclairLecture, kExtraCompletedFlag, kFilterAllInput);
		}
		break;
	case MakeRoomView(kWSC66, kWest):
	case MakeRoomView(kWSC67, kEast):
		if (!GameState.getWSCHeardPage2()) {
			playSpotSoundSync(kPaging2In, kPaging2Out);
			GameState.setWSCHeardPage2(true);
		}
	case MakeRoomView(kWSC10, kNorth):
	case MakeRoomView(kWSC26, kSouth):
	case MakeRoomView(kWSC72, kWest):
	case MakeRoomView(kWSC83, kWest):
		if (!GameState.getWSCHeardCheckIn()) {
			playSpotSoundSync(kCheckInIn, kCheckInOut);
			GameState.setWSCHeardCheckIn(true);
		}
		break;
	case MakeRoomView(kWSC0Z, kSouth):
		if (getCurrentAlternate() == kAltWSCW0ZDoorOpen)
			turnLeft();
		break;
	case MakeRoomView(kWSC93, kEast):
		GameState.setWSCBeenAtWSC93(true);
		break;
	case MakeRoomView(kWSC98, kWest):
		if (!GameState.getWSCRobotDead()) {
			scheduleEvent(kGawkAtRobotTime2, 1, kTimerEventPlayerGawkingAtRobot2);
			setCurrentActivation(kActivationRobotTurning);
			if (g_AIArea)
				g_AIArea->checkMiddleArea();
		} else if (!GameState.getWSCRobotGone()) {
			setCurrentActivation(kActivationRobotDead);
		} else {
			if (GameState.getWSCCatwalkDark()) {
				//	Change the gun hot spot...
				_vm->getAllHotspots().setHotspotRect(kW98StunGunSpotID, Common::Rect(181 + kNavAreaLeft,
						99 + kNavAreaTop,372 + kNavAreaLeft, 149 + kNavAreaTop));
			}
			setCurrentActivation(kActivationRobotGone);
		}
		break;
	case MakeRoomView(kWSCDeathRoom, kNorth):
	case MakeRoomView(kWSCDeathRoom, kSouth):
	case MakeRoomView(kWSCDeathRoom, kEast):
	case MakeRoomView(kWSCDeathRoom, kWest):
		die(kDeathArrestedInWSC);
		break;
	}

	checkPeopleCrossing();
	setUpPoison();
}

void WSC::turnTo(const DirectionConstant direction) {
	Neighborhood::turnTo(direction);

	switch (MakeRoomView(GameState.getCurrentRoom(), direction)) {
	case MakeRoomView(kWSC01, kNorth):
	case MakeRoomView(kWSC01, kSouth):
		GameState.setWSCAnalyzerOn(false);
		break;
	case MakeRoomView(kWSC03, kNorth):
		setCurrentActivation(kActivationW03NorthOff);
		break;
	case MakeRoomView(kWSC03, kSouth):
		if (GameState.getWSCDesignedAntidote() && !GameState.getWSCPickedUpAntidote())
			setCurrentActivation(kActivationReadyForSynthesis);
		break;
	case MakeRoomView(kWSC07, kSouth):
	case MakeRoomView(kWSC56, kNorth):
		setCurrentActivation(kActivationReadyForMap);
		break;
	case MakeRoomView(kWSC18, kSouth):
	case MakeRoomView(kWSC57, kEast):
	case MakeRoomView(kWSC75, kEast):
	case MakeRoomView(kWSC90, kSouth):
		if (!GameState.getWSCHeardCheckIn()) {
			playSpotSoundSync(kCheckInIn, kCheckInOut);
			GameState.setWSCHeardCheckIn(true);
		}
		break;
	case MakeRoomView(kWSC56, kSouth):
		if (!GameState.getWSCHeardPage1()) {
			playSpotSoundSync(kPaging1In, kPaging1Out);
			GameState.setWSCHeardPage1(true);
		}
		// clone2727 says: This falls through?!??! WTF?
	case MakeRoomView(kWSC42, kEast):
		_privateFlags.setFlag(kWSCPrivateSinclairOfficeOpenFlag, false);
		setCurrentActivation(kActivationSinclairOfficeLocked);
		break;
	case MakeRoomView(kWSC58, kSouth):
		setCurrentActivation(kActivationW58SouthDoorLocked);
		_privateFlags.setFlag(kWSCPrivate58SouthOpenFlag, false);
		break;
	case MakeRoomView(kWSC73, kWest):
		setCurrentAlternate(kAltWSCNormal);
		break;
	case MakeRoomView(kWSC0Z, kEast):
		if (getCurrentAlternate() == kAltWSCW0ZDoorOpen)
			startExtraSequence(kW0ZSpottedByWomen, kExtraCompletedFlag, kFilterNoInput);
		break;
	}

	checkPeopleCrossing();
}

void WSC::receiveNotification(Notification *notification, const NotificationFlags flags) {
	int32 currentEnergy;
	Item *item;

	if (flags & kExtraCompletedFlag) {
		_interruptionFilter = kFilterAllInput;

		switch (_lastExtra) {
		case kWSCArrivalFromTSA:
			GameState.setWSCSeenTimeStream(true);
			loadAmbientLoops();
			break;
		case kWSCDartScan1:
			setCurrentActivation(kActivationShotByRobot);
			GameState.setWSCPoisoned(true);
			setUpPoison();
			makeContinuePoint();
			break;
		case kWSCDartScan2:
			_vm->addItemToInventory((InventoryItem *)_vm->getAllItems().findItemByID(kPoisonDart));
			GameState.setScoringRemovedDart();
			GameState.setWSCRemovedDart(true);
			setUpPoison();
			g_AIArea->playAIMovie(kRightAreaSignature, "Images/AI/WSC/XW1WB2", false, kHintInterruption);
			// Fall through...
		case kWSCDartScanNo:
			GameState.setWSCAnsweredAboutDart(true);
			startExtraSequence(kWSCDartScan3, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kWSCDartScan3:
			setCurrentActivation(kActivateHotSpotAlways);
			break;
		case kWSCAnalyzerPowerUp:
		case kWSCAnalyzerPowerUpWithDart:
			GameState.setWSCAnalyzerOn(true);
			break;
		case kWSCDropDartIntoAnalyzer:
			setCurrentActivation(kActivationZoomedInToAnalyzer);
			break;
		case kWSCAnalyzeDart:
			GameState.setWSCAnalyzedDart(true);
			GameState.setScoringAnalyzedDart();
			break;
		case kWSCZoomOutFromAnalyzer:
			setCurrentActivation(kActivateHotSpotAlways);
			GameState.setWSCAnalyzerOn(false);
			GameState.setWSCDartInAnalyzer(false);
			updateViewFrame();
			break;
		case kMessagesMovedToOffice:
		case kMessagesMovedToOfficeNoNitrogen:
			_privateFlags.setFlag(kWSCPrivateLabMessagesOpenFlag, true);
			GameState.setScoringPlayedWithMessages();
			break;
		case kMessagesOff:
		case kMessagesOffNoNitrogen:
			_privateFlags.setFlag(kWSCPrivateLabMessagesOpenFlag, false);
			if (_cachedZoomSpot) {
				zoomTo(_cachedZoomSpot);
				_cachedZoomSpot = 0;
			}
			break;
		case kWSC02TurnOnMorphScreen:
			setCurrentActivation(kActivationReadyForMorph);
			break;
		case kWSC02DropToMorphExperiment:
			loopExtraSequence(kWSC02MorphLoop, kExtraCompletedFlag);
			setCurrentActivation(kActivationMorphLooping);
			break;
		case kWSC02MorphLoop:
			if (_privateFlags.getFlag(kWSCPrivateInterruptedMorphFlag))
				startExtraSequence(kWSC02MorphInterruption, kExtraCompletedFlag, kFilterNoInput);
			else
				scheduleNavCallBack(kExtraCompletedFlag);
			break;
		case kWSC02MorphInterruption:
			setCurrentActivation(kActivationMorphInterrupted);
			GameState.setScoringSawMorphExperiment();
			break;
		case kWSC02TurnOffMorphScreen:
			setCurrentActivation(kActivationMorphScreenOff);
			GameState.setWSCSawMorph(true);
			break;
		case kW03NorthActivate:
			if (GameState.getWSCAnalyzedDart() && !GameState.getWSCDesignedAntidote())
				startExtraSequence(kW03NorthGetData, kExtraCompletedFlag, kFilterNoInput);
			else
				setCurrentActivation(kActivateHotSpotAlways);
			break;
		case kW03NorthGetData:
			setCurrentActivation(kActivationW03NorthReadyForInstructions);
			break;
		case kW03NorthInstructions:
			setCurrentActivation(kActivationW03NorthSawInstructions);
			break;
		case kW03NorthPrepMolecule1:
			setUpMoleculeGame();
			break;
		case kW03NorthPrepMolecule2:
		case kW03NorthPrepMolecule3:
			nextMoleculeGameLevel();
			break;
		case kW03NorthFinishSynthesis:
			setCurrentActivation(kActivateHotSpotAlways);
			_privateFlags.setFlag(kWSCPrivateInMoleculeGameFlag, false);
			GameState.setWSCDesignedAntidote(true);
			GameState.setScoringBuiltAntidote();
			break;
		case kW03SouthCreateAntidote:
			setCurrentActivation(kActivationSynthesizerLooping);
			loopExtraSequence(kW03SouthAntidoteLoop);
			break;
		case kW03SouthDeactivate:
			setCurrentActivation(kActivateHotSpotAlways);
			break;
		case kWSC07SouthMap:
		case kWSC56SouthMap:
			setCurrentActivation(kActivateHotSpotAlways);
			GameState.setScoringSawWSCDirectory();
			break;
		case kNerdAtTheDoor1:
			GameState.setWSCSeenNerd(true);
			break;
		case kNerdAtTheDoor2:
			die(kDeathArrestedInWSC);
			break;
		case kW61Brochure:
			GameState.setScoringSawBrochure();
			loadAmbientLoops();
			break;
		case kW61SouthSmartAlloysWithGun:
		case kW61SouthSmartAlloysNoGun:
			GameState.setScoringSawSinclairEntry1();
			break;
		case kW61SouthMorphingWithGun:
		case kW61SouthMorphingNoGun:
			GameState.setScoringSawSinclairEntry2();
			break;
		case kW61SouthTimeBendingWithGun:
		case kW61SouthTimeBendingNoGun:
			GameState.setScoringSawSinclairEntry3();
			break;
		case kW61MessagesOn:
			GameState.setWSCOfficeMessagesOpen(true);
			setCurrentActivation(kActivationW61MessagesOn);
			break;
		case kW61MessagesOff:
			GameState.setWSCOfficeMessagesOpen(false);
			setCurrentActivation(kActivationW61MessagesOff);
			if (_cachedZoomSpot) {
				zoomTo(_cachedZoomSpot);
				_cachedZoomSpot = 0;
			}
			break;
		case kW61SouthScreenOnWithGun:
		case kW61SouthScreenOnNoGun:
			_privateFlags.setFlag(kWSCPrivateOfficeLogOpenFlag, true);
			setCurrentActivation(kActivationW61SouthOn);
			break;
		case kW61SouthScreenOffWithGun:
		case kW61SouthScreenOffNoGun:
			_privateFlags.setFlag(kWSCPrivateOfficeLogOpenFlag, false);
			setCurrentActivation(kActivationW61SouthOff);
			if (_cachedZoomSpot) {
				zoomTo(_cachedZoomSpot);
				_cachedZoomSpot = 0;
			}
			break;
		case kW62ZoomOutFromRobot:
			// Handle action queue before starting new movie sequences.
			Neighborhood::receiveNotification(notification, flags);
			_energyDrainRate = g_energyMonitor->getEnergyDrainRate();
			g_energyMonitor->setEnergyDrainRate(0);
			currentEnergy = g_energyMonitor->getCurrentEnergy();
			_vm->setEnergyDeathReason(kDeathHitByPlasma);

			if (GameState.getShieldOn())
				currentEnergy -= kPlasmaEnergyWithShield;
			else
				currentEnergy -= kPlasmaEnergyNoShield;

			if (currentEnergy <= 0)
				startExtraSequence(kW62PlasmaDodgeDie, kExtraCompletedFlag, kFilterNoInput);
			else
				startExtraSequence(kW62PlasmaDodgeSurvive, kExtraCompletedFlag, kFilterNoInput);

			scheduleEvent(kPlasmaImpactTime, kOneTickPerSecond, kTimerEventPlasmaHit);
			break;
		case kW62PlasmaDodgeDie:
			g_energyMonitor->setEnergyValue(0);
			break;
		case kW62PlasmaDodgeSurvive:
			if (GameState.getShieldOn()) {
				g_shield->setItemState(kShieldNormal);
				g_energyMonitor->drainEnergy(kPlasmaEnergyWithShield);
			} else {
				g_energyMonitor->drainEnergy(kPlasmaEnergyNoShield);
			}

			g_energyMonitor->setEnergyDrainRate(_energyDrainRate);
			g_AIArea->unlockAI();
			GameState.setScoringFinishedPlasmaDodge();
			GameState.setWSCDidPlasmaDodge(true);
			restoreStriding(kWSC58, kSouth, kAltWSCNormal);
			loadAmbientLoops();
			break;
		case kW0ZSpottedByWomen:
			die(kDeathArrestedInWSC);
			break;
		case kW17WestPeopleCrossing:
			_privateFlags.setFlag(kWSCPrivateSeenPeopleAt17WestFlag, true);
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt17WestFlag, false);
			break;
		case kW21SouthPeopleCrossing:
			_privateFlags.setFlag(kWSCPrivateSeenPeopleAt21SouthFlag, true);
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt21SouthFlag, true);
			break;
		case kW24SouthPeopleCrossing:
			_privateFlags.setFlag(kWSCPrivateSeenPeopleAt24SouthFlag, true);
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt24SouthFlag, true);
			break;
		case kW34EastPeopleCrossing:
			_privateFlags.setFlag(kWSCPrivateSeenPeopleAt34EastFlag, true);
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt34EastFlag, true);
			break;
		case kW36WestPeopleCrossing:
			_privateFlags.setFlag(kWSCPrivateSeenPeopleAt36WestFlag, true);
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt36WestFlag, true);
			break;
		case kW38NorthPeopleCrossing:
			_privateFlags.setFlag(kWSCPrivateSeenPeopleAt38NorthFlag, true);
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt38NorthFlag, true);
			break;
		case kW46SouthPeopleCrossing:
			_privateFlags.setFlag(kWSCPrivateSeenPeopleAt46SouthFlag, true);
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt46SouthFlag, true);
			break;
		case kW49NorthPeopleCrossing:
			_privateFlags.setFlag(kWSCPrivateSeenPeopleAt49NorthFlag, true);
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt49NorthFlag, false);
			break;
		case kW73WestPeopleCrossing:
			_privateFlags.setFlag(kWSCPrivateSeenPeopleAt73WestFlag, true);
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt73WestFlag, false);
			break;
		case kW95RobotShoots:
		case kW98RobotShoots:
			die(kDeathShotOnCatwalk);
			break;
		case kW98MorphsToRobot:
			if (_argonSprite) {
				delete _argonSprite; _argonSprite = 0;
				startExtraSequence(kW98RobotGassed, kExtraCompletedFlag, kFilterNoInput);
			} else if (_privateFlags.getFlag(kWSCPrivateClickedCatwalkCableFlag)) {
				startExtraSequence(kW98RobotShocked, kExtraCompletedFlag, kFilterNoInput);
			} else {
				startExtraSequence(kW98RobotShoots, kExtraCompletedFlag, kFilterNoInput);
			}
			break;
		case kW98RobotShocked:
			GameState.setWSCCatwalkDark(true);
			// Change the gun hot spot...
			_vm->getAllHotspots().setHotspotRect(kW98StunGunSpotID, Common::Rect(181 + kNavAreaLeft, 99 + kNavAreaTop,
					372 + kNavAreaLeft, 149 + kNavAreaTop));
			setCurrentActivation(kActivationRobotDead);
			GameState.setWSCRobotDead(true);
			GameState.setScoringStoppedWSCRobot();

			// Video is not present
			//g_AIArea->playAIMovie(kRightAreaSignature, "Images/AI/WSC/XN59WD", false, kWarningInterruption);
			break;
		case kW98RobotGassed:
			item = (Item *)_vm->getAllItems().findItemByID(kArgonCanister);
			_vm->addItemToInventory((InventoryItem *)item);
			setCurrentActivation(kActivationRobotDead);
			GameState.setWSCRobotDead(true);
			GameState.setScoringStoppedWSCRobot();

			// Video is not present
			//g_AIArea->playAIMovie(kRightAreaSignature, "Images/AI/WSC/XN59WD", false, kWarningInterruption);
			break;
		case kW98RobotHeadOpensLight:
		case kW98RobotHeadOpensDark:
			setCurrentActivation(kActivationWSCRobotHeadOpen);
			_privateFlags.setFlag(kWSCPrivateRobotHeadOpenFlag, true);
			break;
		case kW98RobotHeadClosesDark:
		case kW98RobotHeadClosesLight:
			setCurrentActivation(kActivationRobotGone);
			_privateFlags.setFlag(kWSCPrivateRobotHeadOpenFlag, false);
			GameState.setWSCRobotGone(true);
			break;
		}
	}

	Neighborhood::receiveNotification(notification, flags);
	g_AIArea->checkMiddleArea();
}

void WSC::timerExpired(const uint32 event) {
	switch (event) {
	case kTimerEventPlasmaHit:
		if (GameState.getShieldOn())
			g_shield->setItemState(kShieldPlasma);
		break;
	case kTimerEventPlayerGawkingAtRobot:
		startExtraSequence(kW95RobotShoots, kExtraCompletedFlag, kFilterNoInput);
		break;
	case kTimerEventPlayerGawkingAtRobot2:
		startExtraSequence(kW98MorphsToRobot, kExtraCompletedFlag, kFilterAllInput);
		break;
	}
}

void WSC::setUpMoleculeGame() {
	_privateFlags.setFlag(kWSCPrivateInMoleculeGameFlag, true);
	setCurrentActivation(kActivationW03NorthInGame);
	initOneMovie(&_moleculesMovie, "Images/World Science Center/Molecules.movie",
			kWSCMoleculesMovieOrder, kMoleculesMovieLeft, kMoleculesMovieTop, true);
	_moleculesMovie.redrawMovieWorld();
	_moleculeBin.initMoleculeBin();
	_moleculeGameLevel = 0;
	nextMoleculeGameLevel();
}

void WSC::nextMoleculeGameLevel() {
	_moleculeGameLevel++;

	for (byte i = 0; i < 6; ++i)
		_levelArray[i] = i;

	_vm->shuffleArray((int32 *)_levelArray, 6);
	_moleculeBin.setBinLayout(_levelArray);
	startMoleculeGameLevel();
}

void WSC::startMoleculeGameLevel() {
	_moleculeBin.resetBin();
	_numCorrect = 0;
	_moleculesMovie.stop();
	_moleculesMovie.setFlags(0);
	_moleculesMovie.setSegment(s_moleculeLoopTimes[0], s_moleculeLoopTimes[0] + kMoleculeLoopTime);
	_moleculesMovie.setTime(s_moleculeLoopTimes[0]);
	_moleculesMovie.setFlags(kLoopTimeBase);
	_moleculesMovie.show();

	switch (_moleculeGameLevel) {
	case 1:
		playSpotSoundSync(kWSCMolecule1In, kWSCMolecule1Out);
		break;
	case 2:
		playSpotSoundSync(kWSCMolecule2In, kWSCMolecule2Out);
		break;
	case 3:
		playSpotSoundSync(kWSCMolecule3In, kWSCMolecule3Out);
		break;
	}

	_moleculesMovie.start();
}

void WSC::moleculeGameClick(const HotSpotID id) {
	uint32 molecule = id - kWSC03NorthMolecule1SpotID;

	_moleculeBin.highlightMolecule(molecule);
	_moleculeBin.selectMolecule(molecule);

	if (molecule == _levelArray[_numCorrect]) {
		playSpotSoundSync(kWSCClick2In, kWSCClick2Out);
		_numCorrect++;
		_moleculesMovie.stop();
		_moleculesMovie.setFlags(0);

		TimeValue time = _moleculesMovie.getTime();
		_moleculesMovie.setSegment(s_moleculeLoopTimes[_numCorrect], s_moleculeLoopTimes[_numCorrect] + kMoleculeLoopTime);
		_moleculesMovie.setTime(s_moleculeLoopTimes[_numCorrect] + time - s_moleculeLoopTimes[_numCorrect - 1]);

		if (_numCorrect == 6) {
			_moleculesMovie.start();

			while (_moleculesMovie.isRunning()) {
				_vm->checkCallBacks();
				_vm->refreshDisplay();
				_vm->_system->delayMillis(10);
			}

			_moleculesMovie.stop();
			_moleculesMovie.hide();

			switch (_moleculeGameLevel) {
			case 1:
				startExtraSequence(kW03NorthPrepMolecule2, kExtraCompletedFlag, kFilterNoInput);
				break;
			case 2:
				startExtraSequence(kW03NorthPrepMolecule3, kExtraCompletedFlag, kFilterNoInput);
				break;
			case 3:
				_moleculesMovie.releaseMovie();
				_moleculeBin.cleanUpMoleculeBin();
				requestExtraSequence(kW03NorthFinishSynthesis, kExtraCompletedFlag, kFilterNoInput);
				break;
			}
		} else {
			_moleculesMovie.setFlags(kLoopTimeBase);
			_moleculesMovie.start();
		}
	} else {
		// FAIL
		playSpotSoundSync(kWSCClick3In, kWSCClick3Out);

		_moleculesMovie.stop();
		_moleculesMovie.setFlags(0);
		_moleculesMovie.start();

		while (_moleculesMovie.isRunning()) {
			_vm->checkCallBacks();
			_vm->refreshDisplay();
			_vm->_system->delayMillis(10);
		}

		_moleculesMovie.stop();
		_moleculesMovie.setFlags(0);
		_moleculesMovie.setSegment(s_moleculeFailTimes[_numCorrect], s_moleculeFailTimes[_numCorrect] + kMoleculeFailTime);
		_moleculesMovie.setTime(s_moleculeFailTimes[_numCorrect]);
		_moleculesMovie.start();


		while (_moleculesMovie.isRunning()) {
			_vm->checkCallBacks();
			_vm->refreshDisplay();
			_vm->_system->delayMillis(10);
		}

		_moleculesMovie.stop();
		startMoleculeGameLevel();
	}
}

void WSC::activateOneHotspot(HotspotInfoTable::Entry &entry, Hotspot *hotspot) {
	Neighborhood::activateOneHotspot(entry, hotspot);

	Item *argonCanister;

	switch (hotspot->getObjectID()) {
	case kWSCTurnOnAnalyzerSpotID:
		if (GameState.getWSCAnalyzerOn())
			hotspot->setInactive();
		break;
	case kWSC02SouthTakeArgonSpotID:
		if (!GameState.getWSCSawMorph() || GameState.isTakenItemID(kArgonCanister))
			hotspot->setInactive();
		break;
	case kWSC02ActivateMorphScreenSpotID:
		if (GameState.getWSCSawMorph())
			hotspot->setInactive();
		break;
	case kWSC03NorthMolecule1SpotID:
	case kWSC03NorthMolecule2SpotID:
	case kWSC03NorthMolecule3SpotID:
	case kWSC03NorthMolecule4SpotID:
	case kWSC03NorthMolecule5SpotID:
	case kWSC03NorthMolecule6SpotID:
		if (_moleculeBin.isMoleculeHighlighted(hotspot->getObjectID() - kWSC03NorthMolecule1SpotID))
			hotspot->setInactive();
		break;
	case kWSC03SouthPickUpAntidoteSpotID:
		if (getCurrentActivation() == kActivationSynthesizerLooping)
			hotspot->setActive();
		break;
	case kW98DropArgonSpotID:
		argonCanister = _vm->getAllItems().findItemByID(kArgonCanister);
		if (argonCanister->getItemState() != kArgonFull)
			hotspot->setInactive();
		break;
	}
}

void WSC::activateHotspots() {
	Neighborhood::activateHotspots();

	if (GameState.getCurrentRoomAndView() == MakeRoomView(kWSC98, kWest) && _privateFlags.getFlag(kWSCPrivateRobotHeadOpenFlag)) {
		if (_privateFlags.getFlag(kWSCPrivateGotRetScanChipFlag))
			_vm->getAllHotspots().deactivateOneHotspot(kW98RetinalChipSpotID);
		else
			_vm->getAllHotspots().activateOneHotspot(kW98RetinalChipSpotID);

		if (_privateFlags.getFlag(kWSCPrivateGotMapChipFlag))
			_vm->getAllHotspots().deactivateOneHotspot(kW98MapChipSpotID);
		else
			_vm->getAllHotspots().activateOneHotspot(kW98MapChipSpotID);

		if (_privateFlags.getFlag(kWSCPrivateGotOpticalChipFlag))
			_vm->getAllHotspots().deactivateOneHotspot(kW98OpticalChipSpotID);
		else
			_vm->getAllHotspots().activateOneHotspot(kW98OpticalChipSpotID);
	}
}

void WSC::clickInHotspot(const Input &input, const Hotspot *clickedSpot) {
	if (JMPPPInput::isEasterEggModifierInput(input))
		GameState.setEasterEgg(true);

	if (clickedSpot) {
		switch (clickedSpot->getObjectID()) {
		case kWSCAnalyzerScreenSpotID:
			requestExtraSequence(kWSCAnalyzeDart, kExtraCompletedFlag, kFilterNoInput);
			requestExtraSequence(kWSCZoomOutFromAnalyzer, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kWSC02SouthPlayMessagesSpotID:
			if (GameState.isTakenItemID(kNitrogenCanister)) {
				if (_lastExtra == (uint32)kMessagesMovedToOfficeNoNitrogen)
					startExtraSequence(kMessagesOffNoNitrogen, kExtraCompletedFlag, kFilterNoInput);
				else
					startExtraSequence(kMessagesMovedToOfficeNoNitrogen, kExtraCompletedFlag, kFilterNoInput);
			} else {
				if (_lastExtra == (uint32)kMessagesMovedToOffice)
					startExtraSequence(kMessagesOff, kExtraCompletedFlag, kFilterNoInput);
				else
					startExtraSequence(kMessagesMovedToOffice, kExtraCompletedFlag, kFilterNoInput);
			}
			break;
		case kWSC02SouthInterruptMorphSpotID:
			_privateFlags.setFlag(kWSCPrivateInterruptedMorphFlag, true);
			break;
		case kWSC02SouthMorphFinishedSpotID:
			requestExtraSequence(kWSC02MorphFinished, 0, kFilterNoInput);
			requestExtraSequence(kWSC02TurnOffMorphScreen, kExtraCompletedFlag, kFilterNoInput);
			break;
		case kWSC03NorthMolecule1SpotID:
		case kWSC03NorthMolecule2SpotID:
		case kWSC03NorthMolecule3SpotID:
		case kWSC03NorthMolecule4SpotID:
		case kWSC03NorthMolecule5SpotID:
		case kWSC03NorthMolecule6SpotID:
			moleculeGameClick(clickedSpot->getObjectID());
			break;
		case kW98GrabCableSpotID:
			if (isEventTimerRunning()) {
				cancelEvent();
				startExtraSequence(kW98MorphsToRobot, kExtraCompletedFlag, kFilterAllInput);
			}

			_privateFlags.setFlag(kWSCPrivateClickedCatwalkCableFlag, true);
			break;
		default:
			Neighborhood::clickInHotspot(input, clickedSpot);
			break;
		}
	} else {
		Neighborhood::clickInHotspot(input, clickedSpot);
	}

	GameState.setEasterEgg(false);
}

void WSC::dropItemIntoRoom(Item *item, Hotspot *dropSpot) {
	CoordType h, v;

	switch (item->getObjectID()) {
	case kPoisonDart:
		Neighborhood::dropItemIntoRoom(item, dropSpot);
		GameState.setWSCDartInAnalyzer(true);
		if (dropSpot && dropSpot->getObjectID() == kWSCDropDartSpotID) {
			if (!GameState.getWSCAnalyzerOn())
				requestExtraSequence(kWSCAnalyzerPowerUpWithDart, kExtraCompletedFlag, kFilterNoInput);

			requestExtraSequence(kWSCDropDartIntoAnalyzer, kExtraCompletedFlag, kFilterNoInput);
		}
		break;
	case kAntidote:
		_privateFlags.setFlag(kWSCDraggingAntidoteFlag, false);
		Neighborhood::dropItemIntoRoom(item, dropSpot);
		loopExtraSequence(kW03SouthAntidoteLoop);
		break;
	case kSinclairKey:
		Neighborhood::dropItemIntoRoom(item, dropSpot);
		_privateFlags.setFlag(kWSCPrivateSinclairOfficeOpenFlag, true);
		openDoor();
		break;
	case kCrowbar:
		Neighborhood::dropItemIntoRoom(item, dropSpot);
		_privateFlags.setFlag(kWSCPrivate58SouthOpenFlag, true);
		openDoor();
		break;
	case kMachineGun:
		setCurrentAlternate(kAltWSCNormal);
		Neighborhood::dropItemIntoRoom(item, dropSpot);
		break;
	case kArgonCanister:
		item->setItemState(kArgonEmpty);
		_argonSprite = item->getDragSprite(0);
		_argonSprite->setCurrentFrameIndex(1);
		_argonSprite->setDisplayOrder(kDragSpriteOrder);
		dropSpot->getCenter(h, v);
		_argonSprite->centerElementAt(h, v);
		_argonSprite->startDisplaying();
		_argonSprite->show();

		if (isEventTimerRunning()) {
			cancelEvent();
			startExtraSequence(kW98MorphsToRobot, kExtraCompletedFlag, kFilterAllInput);
		}
		break;
	case kRetinalScanBiochip:
		_privateFlags.setFlag(kWSCPrivateGotRetScanChipFlag, false);
		Neighborhood::dropItemIntoRoom(item, dropSpot);
		break;
	case kMapBiochip:
		_privateFlags.setFlag(kWSCPrivateGotMapChipFlag, false);
		Neighborhood::dropItemIntoRoom(item, dropSpot);
		break;
	case kOpticalBiochip:
		_privateFlags.setFlag(kWSCPrivateGotOpticalChipFlag, false);
		Neighborhood::dropItemIntoRoom(item, dropSpot);
		break;
	default:
		Neighborhood::dropItemIntoRoom(item, dropSpot);
		break;
	}
}

void WSC::takeItemFromRoom(Item *item) {
	switch (item->getObjectID()) {
	case kAntidote:
		_privateFlags.setFlag(kWSCDraggingAntidoteFlag, true);
		Neighborhood::takeItemFromRoom(item);
		break;
	case kMachineGun:
		setCurrentAlternate(kAltWSCTookMachineGun);
		Neighborhood::takeItemFromRoom(item);
		break;
	case kRetinalScanBiochip:
		_privateFlags.setFlag(kWSCPrivateGotRetScanChipFlag, true);
		Neighborhood::takeItemFromRoom(item);
		break;
	case kMapBiochip:
		_privateFlags.setFlag(kWSCPrivateGotMapChipFlag, true);
		Neighborhood::takeItemFromRoom(item);
		break;
	case kOpticalBiochip:
		_privateFlags.setFlag(kWSCPrivateGotOpticalChipFlag, true);
		Neighborhood::takeItemFromRoom(item);
		break;
	default:
		Neighborhood::takeItemFromRoom(item);
		break;
	}
}

Hotspot *WSC::getItemScreenSpot(Item *item, DisplayElement *element) {
	HotSpotID destSpotID;

	switch (item->getObjectID()) {
	case kNitrogenCanister:
		destSpotID = kWSC02SouthTakeNitrogenSpotID;
		break;
	case kArgonPickup:
		destSpotID = kWSC02SouthTakeArgonSpotID;
		break;
	case kAntidote:
		destSpotID = kWSC03SouthPickUpAntidoteSpotID;
		break;
	case kMachineGun:
		destSpotID = kW61SouthMachineGunSpotID;
		break;
	case kRetinalScanBiochip:
		destSpotID = kW98RetinalChipSpotID;
		break;
	case kMapBiochip:
		destSpotID = kW98MapChipSpotID;
		break;
	case kOpticalBiochip:
		destSpotID = kW98OpticalChipSpotID;
		break;
	default:
		destSpotID = kNoHotSpotID;
		break;
	}

	if (destSpotID == kNoHotSpotID)
		return Neighborhood::getItemScreenSpot(item, element);

	return _vm->getAllHotspots().findHotspotByID(destSpotID);
}

void WSC::pickedUpItem(Item *item) {
	switch (item->getObjectID()) {
	case kAntidote:
		if (!GameState.getWSCPickedUpAntidote()) {
			GameState.setWSCPoisoned(false);
			GameState.setWSCRemovedDart(false);
			GameState.setWSCPickedUpAntidote(true);
			_privateFlags.setFlag(kWSCDraggingAntidoteFlag, false);
			playSpotSoundSync(kDrinkAntidoteIn, kDrinkAntidoteOut);
			setUpPoison();
			startExtraSequence(kW03SouthDeactivate, kExtraCompletedFlag, kFilterNoInput);
		}
		break;
	case kArgonPickup:
		_vm->removeItemFromInventory((InventoryItem *)item);
		item = (Item *)_vm->getAllItems().findItemByID(kArgonCanister);
		_vm->addItemToInventory((InventoryItem *)item);
		item = (Item *)_vm->getAllItems().findItemByID(kSinclairKey);
		_vm->addItemToInventory((InventoryItem *)item);
		_vm->getAllHotspots().setHotspotRect(kWSC02SouthMorphOutSpotID,
				Common::Rect(kNavAreaLeft, kNavAreaTop, 512 + kNavAreaLeft, 256 + kNavAreaTop));
		break;
	case kArgonCanister:
		GameState.setScoringGotArgonCanister();
		break;
	case kSinclairKey:
		GameState.setScoringGotSinclairKey();
		break;
	case kNitrogenCanister:
		GameState.setScoringGotNitrogenCanister();
		break;
	case kRetinalScanBiochip:
		if (_privateFlags.getFlag(kWSCPrivateGotMapChipFlag) && _privateFlags.getFlag(kWSCPrivateGotOpticalChipFlag)) {
			if (GameState.getWSCCatwalkDark())
				startExtraSequence(kW98RobotHeadClosesDark, kExtraCompletedFlag, kFilterNoInput);
			else
				startExtraSequence(kW98RobotHeadClosesLight, kExtraCompletedFlag, kFilterNoInput);
		}
		break;
	case kMapBiochip:
		if (_privateFlags.getFlag(kWSCPrivateGotRetScanChipFlag) && _privateFlags.getFlag(kWSCPrivateGotOpticalChipFlag)) {
			if (GameState.getWSCCatwalkDark())
				startExtraSequence(kW98RobotHeadClosesDark, kExtraCompletedFlag, kFilterNoInput);
			else
				startExtraSequence(kW98RobotHeadClosesLight, kExtraCompletedFlag, kFilterNoInput);
		}
		break;
	case kOpticalBiochip:
		g_opticalChip->addMercury();
		GameState.setScoringGotWSCOpMemChip();
		if (_privateFlags.getFlag(kWSCPrivateGotRetScanChipFlag) && _privateFlags.getFlag(kWSCPrivateGotMapChipFlag)) {
			if (GameState.getWSCCatwalkDark())
				startExtraSequence(kW98RobotHeadClosesDark, kExtraCompletedFlag, kFilterNoInput);
			else
				startExtraSequence(kW98RobotHeadClosesLight, kExtraCompletedFlag, kFilterNoInput);
		}
		break;
	case kStunGun:
		GameState.setWSCFinished(true);

		if (!GameState.getWSCCatwalkDark())
			GameState.setScoringWSCGandhi();

		recallToTSASuccess();
		break;
	}
}

void WSC::checkPeopleCrossing() {
	switch (GameState.getCurrentRoomAndView()) {
	case MakeRoomView(kWSC17, kWest):
		if (_privateFlags.getFlag(kWSCPrivateNeedPeopleAt17WestFlag))
			startExtraSequence(kW17WestPeopleCrossing, kExtraCompletedFlag, kFilterNoInput);
		break;
	case MakeRoomView(kWSC21, kSouth):
		if (_privateFlags.getFlag(kWSCPrivateNeedPeopleAt21SouthFlag))
			startExtraSequence(kW21SouthPeopleCrossing, kExtraCompletedFlag, kFilterNoInput);
		break;
	case MakeRoomView(kWSC24, kSouth):
		if (_privateFlags.getFlag(kWSCPrivateNeedPeopleAt24SouthFlag))
			startExtraSequence(kW24SouthPeopleCrossing, kExtraCompletedFlag, kFilterNoInput);
		break;
	case MakeRoomView(kWSC34, kEast):
		if (_privateFlags.getFlag(kWSCPrivateNeedPeopleAt34EastFlag))
			startExtraSequence(kW34EastPeopleCrossing, kExtraCompletedFlag, kFilterNoInput);
		break;
	case MakeRoomView(kWSC36, kWest):
		if (_privateFlags.getFlag(kWSCPrivateNeedPeopleAt36WestFlag))
			startExtraSequence(kW36WestPeopleCrossing, kExtraCompletedFlag, kFilterNoInput);
		break;
	case MakeRoomView(kWSC38, kNorth):
		if (_privateFlags.getFlag(kWSCPrivateNeedPeopleAt38NorthFlag))
			startExtraSequence(kW38NorthPeopleCrossing, kExtraCompletedFlag, kFilterNoInput);
		break;
	case MakeRoomView(kWSC46, kSouth):
		if (_privateFlags.getFlag(kWSCPrivateNeedPeopleAt46SouthFlag))
			startExtraSequence(kW46SouthPeopleCrossing, kExtraCompletedFlag, kFilterNoInput);
		break;
	case MakeRoomView(kWSC49, kNorth):
		if (_privateFlags.getFlag(kWSCPrivateNeedPeopleAt49NorthFlag))
			startExtraSequence(kW49NorthPeopleCrossing, kExtraCompletedFlag, kFilterNoInput);
		break;
	case MakeRoomView(kWSC73, kWest):
		if (_privateFlags.getFlag(kWSCPrivateNeedPeopleAt73WestFlag))
			startExtraSequence(kW73WestPeopleCrossing, kExtraCompletedFlag, kFilterNoInput);
		break;
	default:
		if (!_privateFlags.getFlag(kWSCPrivateSeenPeopleAt21SouthFlag) && _vm->getRandomNumber(2) == 0) {
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt21SouthFlag, true);
			forceStridingStop(kWSC18, kSouth, kAltWSCNormal);
		} else {
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt21SouthFlag, false);
			restoreStriding(kWSC18, kSouth, kAltWSCNormal);
		}

		if (!_privateFlags.getFlag(kWSCPrivateSeenPeopleAt19NorthFlag) && _vm->getRandomNumber(2) == 0) {
			forceStridingStop(kWSC22, kNorth, kAltWSCNormal);
		} else {
			restoreStriding(kWSC22, kNorth, kAltWSCNormal);
		}

		if (!_privateFlags.getFlag(kWSCPrivateSeenPeopleAt24SouthFlag) && _vm->getRandomNumber(2) == 0) {
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt24SouthFlag, true);
			forceStridingStop(kWSC22, kSouth, kAltWSCNormal);
		} else {
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt24SouthFlag, false);
			restoreStriding(kWSC22, kSouth, kAltWSCNormal);
		}

		if (!_privateFlags.getFlag(kWSCPrivateSeenPeopleAt34EastFlag) && _vm->getRandomNumber(2) == 0) {
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt34EastFlag, true);
			forceStridingStop(kWSC28, kEast, kAltWSCNormal);
		} else {
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt34EastFlag, false);
			restoreStriding(kWSC28, kEast, kAltWSCNormal);
		}

		if (!_privateFlags.getFlag(kWSCPrivateSeenPeopleAt36WestFlag) && _vm->getRandomNumber(2) == 0) {
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt36WestFlag, true);
			forceStridingStop(kWSC40, kWest, kAltWSCNormal);
		} else {
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt36WestFlag, false);
			restoreStriding(kWSC40, kWest, kAltWSCNormal);
		}

		if (!_privateFlags.getFlag(kWSCPrivateSeenPeopleAt38NorthFlag) && _vm->getRandomNumber(2) == 0) {
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt38NorthFlag, true);
			forceStridingStop(kWSC42, kNorth, kAltWSCNormal);
		} else {
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt38NorthFlag, false);
			restoreStriding(kWSC42, kNorth, kAltWSCNormal);
		}

		if (!_privateFlags.getFlag(kWSCPrivateSeenPeopleAt46SouthFlag) && _vm->getRandomNumber(2) == 0) {
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt46SouthFlag, true);
			forceStridingStop(kWSC44, kSouth, kAltWSCNormal);
		} else {
			_privateFlags.setFlag(kWSCPrivateNeedPeopleAt46SouthFlag, false);
			restoreStriding(kWSC44, kSouth, kAltWSCNormal);
		}
		break;
	}
}

void WSC::setUpPoison() {
	if (GameState.getWSCPoisoned()) {
		if (GameState.getWSCRemovedDart()) {
			if (g_energyMonitor->getEnergyDrainRate() != kWSCPoisonEnergyDrainNoDart) {
				g_energyMonitor->setEnergyDrainRate(kWSCPoisonEnergyDrainNoDart);
				_vm->setEnergyDeathReason(kDeathDidntStopPoison);
			}
		} else {
			if (g_energyMonitor->getEnergyDrainRate() != kWSCPoisonEnergyDrainWithDart) {
				g_energyMonitor->setEnergyDrainRate(kWSCPoisonEnergyDrainWithDart);
				_vm->setEnergyDeathReason(kDeathDidntStopPoison);
			}
		}
	} else if (g_energyMonitor->getEnergyDrainRate() != kEnergyDrainNormal) {
		g_energyMonitor->setEnergyDrainRate(kEnergyDrainNormal);
		_vm->resetEnergyDeathReason();
	}
}

bool WSC::inSynthesizerGame() {
	return _moleculesMovie.isMovieValid();
}

bool WSC::canSolve() {
	return (inSynthesizerGame() || (GameState.getCurrentRoom() == kWSC98 && !GameState.getWSCRobotDead()));
}

void WSC::doSolve() {
	if (inSynthesizerGame()) {
		_moleculesMovie.releaseMovie();
		_moleculeBin.cleanUpMoleculeBin();
		requestExtraSequence(kW03NorthFinishSynthesis, kExtraCompletedFlag, kFilterNoInput);
	} else if (GameState.getCurrentRoom() == kWSC98 && !GameState.getWSCRobotDead()) {
		cancelEvent();
		startExtraSequence(kW98RobotShocked, kExtraCompletedFlag, kFilterNoInput);
	}
}

Common::String WSC::getNavMovieName() {
	return "Images/World Science Center/WSC.movie";
}

Common::String WSC::getSoundSpotsName() {
	return "Sounds/World Science Center/WSC Spots";
}

} // End of namespace Pegasus
