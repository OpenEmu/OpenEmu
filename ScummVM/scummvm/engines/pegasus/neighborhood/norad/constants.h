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

#ifndef PEGASUS_NEIGHBORHOOD_NORAD_CONSTANTS_H
#define PEGASUS_NEIGHBORHOOD_NORAD_CONSTANTS_H

#include "pegasus/constants.h"

namespace Pegasus {

// Norad Alpha spot constants

static const TimeValue kAlphaBumpIntoWallIn = 0;
static const TimeValue kAlphaBumpIntoWallOut = 303;

static const TimeValue kAlphaAccessDeniedIn = 303;
static const TimeValue kAlphaAccessDeniedOut = 3045;

static const TimeValue kAlphaRegDoorCloseIn = 3045;
static const TimeValue kAlphaRegDoorCloseOut = 4476;

static const TimeValue kAlphaElevatorDoorCloseIn = 4476;
static const TimeValue kAlphaElevatorDoorCloseOut = 5071;

static const TimeValue kAlphaCantTransportIn = 5071;
static const TimeValue kAlphaCantTransportOut = 9348;

static const TimeValue kAlphaPressureDoorIntro1In = 9348;
static const TimeValue kAlphaPressureDoorIntro1Out = 11061;

static const TimeValue kAlphaPressureDoorIntro2In = 11061;
static const TimeValue kAlphaPressureDoorIntro2Out = 14098;

static const TimeValue kN22ReplyIn = 14098;
static const TimeValue kN22ReplyOut = 18442;

static const TimeValue kAlphaLoadClawIntroIn = 18442;
static const TimeValue kAlphaLoadClawIntroOut = 20698;

// Norad Delta spot constants

static const TimeValue kDeltaBumpIntoWallIn = 0;
static const TimeValue kDeltaBumpIntoWallOut = 303;

static const TimeValue kDeltaAccessDeniedIn = 303;
static const TimeValue kDeltaAccessDeniedOut = 3045;

static const TimeValue kDeltaRegDoorCloseIn = 3045;
static const TimeValue kDeltaRegDoorCloseOut = 4476;

static const TimeValue kDeltaElevatorDoorCloseIn = 4476;
static const TimeValue kDeltaElevatorDoorCloseOut = 5071;

static const TimeValue kPressureDoorIntro1In = 5071;
static const TimeValue kPressureDoorIntro1Out = 6784;

static const TimeValue kPressureDoorIntro2In = 6784;
static const TimeValue kPressureDoorIntro2Out = 9821;

static const TimeValue kLoadClawIntroIn = 9821;
static const TimeValue kLoadClawIntroOut = 12077;

static const TimeValue kHoldForRetinalIn = 12077;
static const TimeValue kHoldForRetinalOut = 14104;

static const TimeValue kRetinalScanFailedIn = 14104;
static const TimeValue kRetinalScanFailedOut = 17538;

static const TimeValue kAddisAbabaIn = 17538;
static const TimeValue kAddisAbabaOut = 19263;

static const TimeValue kBangkokIn = 19263;
static const TimeValue kBangkokOut = 20201;

static const TimeValue kBonnIn = 20201;
static const TimeValue kBonnOut = 20915;

static const TimeValue kDublinIn = 20915;
static const TimeValue kDublinOut = 21660;

static const TimeValue kHonoluluIn = 21660;
static const TimeValue kHonoluluOut = 22498;

static const TimeValue kMadridIn = 22498;
static const TimeValue kMadridOut = 23474;

static const TimeValue kReykjavikIn = 23474;
static const TimeValue kReykjavikOut = 24488;

static const TimeValue kSanAntonioIn = 24488;
static const TimeValue kSanAntonioOut = 25561;

static const TimeValue kSeoulIn = 25561;
static const TimeValue kSeoulOut = 26461;

static const TimeValue kSvortalskIn = 26461;
static const TimeValue kSvortalskOut = 27582;

static const TimeValue kSiloBeepIn = 27582;
static const TimeValue kSiloBeepOut = 27721;

static const TimeValue kAllSilosDeactivatedIn = 27721;
static const TimeValue kAllSilosDeactivatedOut = 28928;

static const TimeValue kGlobalLaunchOverrideIn = 28928;
static const TimeValue kGlobalLaunchOverrideOut = 30736;

static const TimeValue kLaunchSiloSelectedIn = 30736;
static const TimeValue kLaunchSiloSelectedOut = 31660;

static const TimeValue kLaunchToProceedIn = 31660;
static const TimeValue kLaunchToProceedOut = 32536;

static const TimeValue kMaximumDeactivationIn = 32536;
static const TimeValue kMaximumDeactivationOut = 34337;

static const TimeValue kMissileLaunchedIn = 34337;
static const TimeValue kMissileLaunchedOut = 35082;

static const TimeValue kNewLaunchSiloIn = 35082;
static const TimeValue kNewLaunchSiloOut = 36320;

static const TimeValue kStrikeAuthorizedIn = 36320;
static const TimeValue kStrikeAuthorizedOut = 37393;

static const TimeValue kPrimaryTargetIn = 37393;
static const TimeValue kPrimaryTargetOut = 38628;

static const TimeValue kSiloDeactivatedIn = 38628;
static const TimeValue kSiloDeactivatedOut = 39566;

static const TimeValue kStrikeCodeRejectedIn = 39566;
static const TimeValue kStrikeCodeRejectedOut = 41056;

static const TimeValue kToDeactivateIn = 41056;
static const TimeValue kToDeactivateOut = 46494;

static const TimeValue kTwoMinutesIn = 46494;
static const TimeValue kTwoMinutesOut = 47166;

static const TimeValue kOneMinuteIn = 47166;
static const TimeValue kOneMinuteOut = 47856;

static const TimeValue kFiftySecondsIn = 47856;
static const TimeValue kFiftySecondsOut = 48691;

static const TimeValue kFortySecondsIn = 48691;
static const TimeValue kFortySecondsOut = 49500;

static const TimeValue kThirtySecondsIn = 49500;
static const TimeValue kThirtySecondsOut = 50362;

static const TimeValue kTwentySecondsIn = 50362;
static const TimeValue kTwentySecondsOut = 51245;

static const TimeValue kTenSecondsIn = 51245;
static const TimeValue kTenSecondsOut = 52069;

static const TimeValue kGiveUpHumanIn = 52069;
static const TimeValue kGiveUpHumanOut = 55023;

static const TimeValue kIJustBrokeIn = 55023;
static const TimeValue kIJustBrokeOut = 59191;

static const TimeValue kTheOnlyGoodHumanIn = 59191;
static const TimeValue kTheOnlyGoodHumanOut = 62379;

static const TimeValue kYouAreRunningIn = 62379;
static const TimeValue kYouAreRunningOut = 64201;

static const TimeValue kYouCannotPossiblyIn = 64201;
static const TimeValue kYouCannotPossiblyOut = 65740;

static const TimeValue kYouWillFailIn = 65740;
static const TimeValue kYouWillFailOut = 67217;

static const CanOpenDoorReason kCantOpenBadPressure = kCantOpenLastReason + 1;

static const NotificationFlags kAirTimerExpiredFlag = kLastNeighborhoodNotificationFlag << 1;

static const uint16 kNoradWarningVolume = 0x100 / 3;
static const uint16 kNoradSuckWindVolume = 0x100 / 2;

static const int16 kElevatorCompassAngle = -40;
static const int16 kSubPlatformCompassAngle = 45;
static const int16 kSubControlCompassAngle = -10;

// Norad interactions.

static const InteractionID kNoradGlobeGameInteractionID = 0;
static const InteractionID kNoradECRMonitorInteractionID = 1;
static const InteractionID kNoradFillingStationInteractionID = 2;
static const InteractionID kNoradElevatorInteractionID = 3;
static const InteractionID kNoradPressureDoorInteractionID = 4;
static const InteractionID kNoradSubControlRoomInteractionID = 5;
static const InteractionID kNoradSubPlatformInteractionID = 6;

/////////////////////////////////////////////
//
// Norad Alpha

static const CoordType kECRSlideShowLeft = kNavAreaLeft + 78;
static const CoordType kECRSlideShowTop = kNavAreaTop + 1;

static const CoordType kECRPanLeft = kNavAreaLeft + 78 + 5;
static const CoordType kECRPanTop = kNavAreaTop + 1 + 4;
static const CoordType kECRPanRight = kECRPanLeft + 213;
static const CoordType kECRPanBottom = kECRPanTop + 241;

static const CoordType kNoradAlphaElevatorControlsLeft = kNavAreaLeft + 332;
static const CoordType kNoradAlphaElevatorControlsTop = kNavAreaTop + 127;

static const CoordType kNoradAlpha01LeftSideLeft = kNavAreaLeft + 0;
static const CoordType kNoradAlpha01LeftSideTop = kNavAreaTop + 0;

static const CoordType kNoradAlpha01RightSideLeft = kNavAreaLeft + 240;
static const CoordType kNoradAlpha01RightSideTop = kNavAreaTop + 12;

static const CoordType kNoradUpperLevelsLeft = kNavAreaLeft + 98;
static const CoordType kNoradUpperLevelsTop = kNavAreaTop + 31;

static const CoordType kNoradUpperTypeLeft = kNoradUpperLevelsLeft + 114;
static const CoordType kNoradUpperTypeTop = kNoradUpperLevelsTop + 8;

static const CoordType kNoradUpperUpLeft = kNavAreaLeft + 361;
static const CoordType kNoradUpperUpTop = kNavAreaTop + 32;

static const CoordType kNoradUpperDownLeft = kNavAreaLeft + 367;
static const CoordType kNoradUpperDownTop = kNavAreaTop + 66;

static const CoordType kNoradLowerLevelsLeft = kNavAreaLeft + 74;
static const CoordType kNoradLowerLevelsTop = kNavAreaTop + 157;

static const CoordType kNoradLowerTypeLeft = kNoradLowerLevelsLeft + 144;
static const CoordType kNoradLowerTypeTop = kNoradLowerLevelsTop + 9;

static const CoordType kNoradLowerUpLeft = kNavAreaLeft + 380;
static const CoordType kNoradLowerUpTop = kNavAreaTop + 164;

static const CoordType kNoradLowerDownLeft = kNavAreaLeft + 388;
static const CoordType kNoradLowerDownTop = kNavAreaTop + 212;

static const CoordType kNoradPlatformLeft = kNavAreaLeft + 36;
static const CoordType kNoradPlatformTop = kNavAreaTop + 87;

static const CoordType kNoradSubControlLeft = kNavAreaLeft + 0;
static const CoordType kNoradSubControlTop = kNavAreaTop + 84;

static const CoordType kNoradSubControlPinchLeft = kNoradSubControlLeft + 106;
static const CoordType kNoradSubControlPinchTop = kNoradSubControlTop + 86;

static const CoordType kNoradSubControlDownLeft = kNoradSubControlLeft + 66;
static const CoordType kNoradSubControlDownTop = kNoradSubControlTop + 106;

static const CoordType kNoradSubControlRightLeft = kNoradSubControlLeft + 83;
static const CoordType kNoradSubControlRightTop = kNoradSubControlTop + 90;

static const CoordType kNoradSubControlLeftLeft = kNoradSubControlLeft + 56;
static const CoordType kNoradSubControlLeftTop = kNoradSubControlTop + 91;

static const CoordType kNoradSubControlUpLeft = kNoradSubControlLeft + 66;
static const CoordType kNoradSubControlUpTop = kNoradSubControlTop + 81;

static const CoordType kNoradSubControlCCWLeft = kNoradSubControlLeft + 29;
static const CoordType kNoradSubControlCCWTop = kNoradSubControlTop + 88;

static const CoordType kNoradSubControlCWLeft = kNoradSubControlLeft + 0;
static const CoordType kNoradSubControlCWTop = kNoradSubControlTop + 89;

static const CoordType kNoradClawMonitorLeft = kNavAreaLeft + 288;
static const CoordType kNoradClawMonitorTop = kNavAreaTop + 97;

static const CoordType kNoradGreenBallAtALeft = kNoradClawMonitorLeft + 179;
static const CoordType kNoradGreenBallAtATop = kNoradClawMonitorTop + 82;

static const CoordType kNoradGreenBallAtBLeft = kNoradClawMonitorLeft + 130;
static const CoordType kNoradGreenBallAtBTop = kNoradClawMonitorTop + 73;

static const CoordType kNoradGreenBallAtCLeft = kNoradClawMonitorLeft + 110;
static const CoordType kNoradGreenBallAtCTop = kNoradClawMonitorTop + 26;

static const CoordType kNoradGreenBallAtDLeft = kNoradClawMonitorLeft + 21;
static const CoordType kNoradGreenBallAtDTop = kNoradClawMonitorTop + 49;

/////////////////////////////////////////////
//
// Norad Delta

static const CoordType kGlobeMonitorLeft = kNavAreaLeft + 360;
static const CoordType kGlobeMonitorTop = kNavAreaTop + 144;

static const CoordType kGlobeLeft = kNavAreaLeft + 172;
static const CoordType kGlobeTop = kNavAreaTop;

static const CoordType kGlobeCircleLeftLeft = kNavAreaLeft + 186;
static const CoordType kGlobeCircleLeftTop = kNavAreaTop + 41;

static const CoordType kGlobeCircleRightLeft = kNavAreaLeft + 321;
static const CoordType kGlobeCircleRightTop = kNavAreaTop + 41;

static const CoordType kGlobeCircleUpLeft = kNavAreaLeft + 220;
static const CoordType kGlobeCircleUpTop = kNavAreaTop + 7;

static const CoordType kGlobeCircleDownLeft = kNavAreaLeft + 220;
static const CoordType kGlobeCircleDownTop = kNavAreaTop + 142;

static const CoordType kGlobeUpperLeftHiliteLeft = kNavAreaLeft + 207;
static const CoordType kGlobeUpperLeftHiliteTop = kNavAreaTop + 28;

static const CoordType kGlobeUpperRightHiliteLeft = kNavAreaLeft + 307;
static const CoordType kGlobeUpperRightHiliteTop = kNavAreaTop + 28;

static const CoordType kGlobeLowerLeftHiliteLeft = kNavAreaLeft + 207;
static const CoordType kGlobeLowerLeftHiliteTop = kNavAreaTop + 128;

static const CoordType kGlobeLowerRightHiliteLeft = kNavAreaLeft + 307;
static const CoordType kGlobeLowerRightHiliteTop = kNavAreaTop + 128;

static const CoordType kGlobeLeftMotionHiliteLeft = kNavAreaLeft + 182;
static const CoordType kGlobeLeftMotionHiliteTop = kNavAreaTop + 60;

static const CoordType kGlobeRightMotionHiliteLeft = kNavAreaLeft + 331;
static const CoordType kGlobeRightMotionHiliteTop = kNavAreaTop + 60;

static const CoordType kGlobeUpMotionHiliteLeft = kNavAreaLeft + 239;
static const CoordType kGlobeUpMotionHiliteTop = kNavAreaTop + 3;

static const CoordType kGlobeDownMotionHiliteLeft = kNavAreaLeft + 239;
static const CoordType kGlobeDownMotionHiliteTop = kNavAreaTop + 152;

static const CoordType kGlobeUpperNamesLeft = kNavAreaLeft + 368;
static const CoordType kGlobeUpperNamesTop = kNavAreaTop + 188;

static const CoordType kGlobeLowerNamesLeft = kNavAreaLeft + 368;
static const CoordType kGlobeLowerNamesTop = kNavAreaTop + 212;

static const CoordType kGlobeCountdownLeft = kNavAreaLeft + 478;
static const CoordType kGlobeCountdownTop = kNavAreaTop + 164;

// Norad Alpha display IDs.

static const DisplayElementID kECRSlideShowMovieID = kNeighborhoodDisplayID;
static const DisplayElementID kECRPanID = kECRSlideShowMovieID + 1;
static const DisplayElementID kNoradAlphaDeathMovieID = kECRPanID + 1;
static const DisplayElementID kNoradElevatorControlsID = kNoradAlphaDeathMovieID + 1;
static const DisplayElementID kN01LeftSideID = kNoradElevatorControlsID + 1;
static const DisplayElementID kN01RightSideID = kN01LeftSideID + 1;
static const DisplayElementID kPressureDoorLevelsID = kN01RightSideID + 1;
static const DisplayElementID kPressureDoorTypeID = kPressureDoorLevelsID + 1;
static const DisplayElementID kPressureDoorUpButtonID = kPressureDoorTypeID + 1;
static const DisplayElementID kPressureDoorDownButtonID = kPressureDoorUpButtonID + 1;
static const DisplayElementID kPlatformMonitorID = kPressureDoorDownButtonID + 1;
static const DisplayElementID kSubControlMonitorID = kPlatformMonitorID + 1;
static const DisplayElementID kClawMonitorID = kSubControlMonitorID + 1;
static const DisplayElementID kSubControlPinchID = kClawMonitorID + 1;
static const DisplayElementID kSubControlDownID = kSubControlPinchID + 1;
static const DisplayElementID kSubControlRightID = kSubControlDownID + 1;
static const DisplayElementID kSubControlLeftID = kSubControlRightID + 1;
static const DisplayElementID kSubControlUpID = kSubControlLeftID + 1;
static const DisplayElementID kSubControlCCWID = kSubControlUpID + 1;
static const DisplayElementID kSubControlCWID = kSubControlCCWID + 1;
static const DisplayElementID kClawMonitorGreenBallID = kSubControlCWID + 1;

// Norad Delta display IDs.

static const DisplayElementID kGlobeMonitorID = kNeighborhoodDisplayID;
static const DisplayElementID kGlobeMovieID = kGlobeMonitorID + 14;
static const DisplayElementID kGlobeCircleLeftID = kGlobeMovieID + 1;
static const DisplayElementID kGlobeCircleRightID = kGlobeCircleLeftID + 1;
static const DisplayElementID kGlobeCircleUpID = kGlobeCircleRightID + 1;
static const DisplayElementID kGlobeCircleDownID = kGlobeCircleUpID + 1;
static const DisplayElementID kMotionHiliteLeftID = kGlobeCircleDownID + 1;
static const DisplayElementID kMotionHiliteRightID = kMotionHiliteLeftID + 1;
static const DisplayElementID kMotionHiliteUpID = kMotionHiliteRightID + 1;
static const DisplayElementID kMotionHiliteDownID = kMotionHiliteUpID + 1;
static const DisplayElementID kTargetHiliteUpperLeftID = kMotionHiliteDownID + 1;
static const DisplayElementID kTargetHiliteUpperRightID = kTargetHiliteUpperLeftID + 1;
static const DisplayElementID kTargetHiliteLowerLeftID = kTargetHiliteUpperRightID + 1;
static const DisplayElementID kTargetHiliteLowerRightID = kTargetHiliteLowerLeftID + 1;
static const DisplayElementID kGlobeUpperNamesID = kTargetHiliteLowerRightID + 1;
static const DisplayElementID kGlobeLowerNamesID = kGlobeUpperNamesID + 1;
static const DisplayElementID kGlobeCountdownID = kGlobeLowerNamesID + 1;

// Norad Alpha:

static const DisplayOrder kECRMonitorOrder = kMonitorLayer;
static const DisplayOrder kECRPanOrder = kECRMonitorOrder + 1;

static const DisplayOrder kN01LeftSideOrder = kMonitorLayer;
static const DisplayOrder kN01RightSideOrder = kN01LeftSideOrder + 1;

static const DisplayOrder kElevatorControlsOrder = kMonitorLayer;

static const DisplayOrder kPressureLevelsOrder = kMonitorLayer;
static const DisplayOrder kPressureTypeOrder = kPressureLevelsOrder + 1;
static const DisplayOrder kPressureUpOrder = kPressureTypeOrder + 1;
static const DisplayOrder kPressureDownOrder = kPressureUpOrder + 1;

static const DisplayOrder kPlatformOrder = kMonitorLayer;

static const DisplayOrder kSubControlOrder = kMonitorLayer;
static const DisplayOrder kClawMonitorOrder = kSubControlOrder + 1;
static const DisplayOrder kSubControlPinchOrder = kClawMonitorOrder + 1;
static const DisplayOrder kSubControlDownOrder = kSubControlPinchOrder + 1;
static const DisplayOrder kSubControlRightOrder = kSubControlDownOrder + 1;
static const DisplayOrder kSubControlLeftOrder = kSubControlRightOrder + 1;
static const DisplayOrder kSubControlUpOrder = kSubControlLeftOrder + 1;
static const DisplayOrder kSubControlCCWOrder = kSubControlUpOrder + 1;
static const DisplayOrder kSubControlCWOrder = kSubControlCCWOrder + 1;
static const DisplayOrder kClawMonitorGreenBallOrder = kSubControlCWOrder + 1;

// Norad Delta:

static const DisplayOrder kGlobeMonitorLayer = kMonitorLayer;
static const DisplayOrder kGlobeMovieLayer = kGlobeMonitorLayer + 1;
static const DisplayOrder kGlobeCircleLayer = kGlobeMovieLayer + 1;
static const DisplayOrder kGlobeHilitesLayer = kGlobeCircleLayer + 1;
static const DisplayOrder kGlobeUpperNamesLayer = kGlobeHilitesLayer + 1;
static const DisplayOrder kGlobeLowerNamesLayer = kGlobeUpperNamesLayer + 1;
static const DisplayOrder kGlobeCountdownLayer = kGlobeLowerNamesLayer + 1;

// Norad Alpha Tables

static const TimeScale kNoradAlphaMovieScale = 600;
static const TimeScale kNoradAlphaFramesPerSecond = 15;
static const TimeScale kNoradAlphaFrameDuration = 40;

// Alternate IDs.

static const AlternateID kAltNoradAlphaNormal = 0;

// Room IDs.

static const RoomID kNorad01 = 0;
static const RoomID kNorad01East = 1;
static const RoomID kNorad01West = 2;
static const RoomID kNorad02 = 3;
static const RoomID kNorad03 = 4;
static const RoomID kNorad04 = 5;
static const RoomID kNorad05 = 6;
static const RoomID kNorad06 = 7;
static const RoomID kNorad07 = 8;
static const RoomID kNorad07North = 9;
static const RoomID kNorad08 = 10;
static const RoomID kNorad09 = 11;
static const RoomID kNorad10 = 12;
static const RoomID kNorad10East = 13;
static const RoomID kNorad11 = 14;
static const RoomID kNorad11South = 15;
static const RoomID kNorad12 = 16;
static const RoomID kNorad12South = 17;
static const RoomID kNorad13 = 18;
static const RoomID kNorad14 = 19;
static const RoomID kNorad15 = 20;
static const RoomID kNorad16 = 21;
static const RoomID kNorad17 = 22;
static const RoomID kNorad18 = 23;
static const RoomID kNorad19 = 24;
static const RoomID kNorad19West = 25;
static const RoomID kNorad21 = 26;
static const RoomID kNorad21West = 27;
static const RoomID kNorad22 = 28;
static const RoomID kNorad22West = 29;

// Hot Spot Activation IDs.


// Hot Spot IDs.

static const HotSpotID kNorad01ECRSpotID = 5000;
static const HotSpotID kNorad01GasSpotID = 5001;
static const HotSpotID kNorad01ECROutSpotID = 5002;
static const HotSpotID kNorad01GasOutSpotID = 5003;
static const HotSpotID kNorad01MonitorSpotID = 5004;
static const HotSpotID kNorad01IntakeSpotID = 5005;
static const HotSpotID kNorad01DispenseSpotID = 5006;
static const HotSpotID kNorad01ArSpotID = 5007;
static const HotSpotID kNorad01CO2SpotID = 5008;
static const HotSpotID kNorad01HeSpotID = 5009;
static const HotSpotID kNorad01OSpotID = 5010;
static const HotSpotID kNorad01NSpotID = 5011;
static const HotSpotID kN01GasCanisterSpotID = 5012;
static const HotSpotID kN01ArgonCanisterSpotID = 5013;
static const HotSpotID kN01AirMaskSpotID = 5014;
static const HotSpotID kN01NitrogenCanisterSpotID = 5015;
static const HotSpotID kN01GasOutletSpotID = 5016;
static const HotSpotID kNorad07DoorSpotID = 5017;
static const HotSpotID kNorad07DoorOutSpotID = 5018;
static const HotSpotID kNorad10DoorSpotID = 5019;
static const HotSpotID kNorad10EastOutSpotID = 5020;
static const HotSpotID kAlphaUpperPressureDoorUpSpotID = 5021;
static const HotSpotID kAlphaUpperPressureDoorDownSpotID = 5022;
static const HotSpotID kNorad11ElevatorSpotID = 5023;
static const HotSpotID kNorad11ElevatorOutSpotID = 5024;
static const HotSpotID kNorad11ElevatorDownSpotID = 5025;
static const HotSpotID kNorad12ElevatorSpotID = 5026;
static const HotSpotID kNorad12ElevatorOutSpotID = 5027;
static const HotSpotID kNorad12ElevatorUpSpotID = 5028;
static const HotSpotID kNorad19MonitorSpotID = 5029;
static const HotSpotID kNorad19MonitorOutSpotID = 5030;
static const HotSpotID kNorad19ActivateMonitorSpotID = 5031;
static const HotSpotID kNorad21WestSpotID = 5032;
static const HotSpotID kNorad21WestOutSpotID = 5033;
static const HotSpotID kAlphaLowerPressureDoorUpSpotID = 5034;
static const HotSpotID kAlphaLowerPressureDoorDownSpotID = 5035;
static const HotSpotID kNorad22MonitorSpotID = 5036;
static const HotSpotID kNorad22MonitorOutSpotID = 5037;
static const HotSpotID kNorad22LaunchPrepSpotID = 5038;
static const HotSpotID kNorad22ClawControlSpotID = 5039;
static const HotSpotID kNorad22ClawPinchSpotID = 5040;
static const HotSpotID kNorad22ClawDownSpotID = 5041;
static const HotSpotID kNorad22ClawRightSpotID = 5042;
static const HotSpotID kNorad22ClawLeftSpotID = 5043;
static const HotSpotID kNorad22ClawUpSpotID = 5044;
static const HotSpotID kNorad22ClawCCWSpotID = 5045;
static const HotSpotID kNorad22ClawCWSpotID = 5046;

// Extra sequence IDs.

static const ExtraID kNoradArriveFromTSA = 0;
static const ExtraID kNorad01RobotTaunt = 1;
static const ExtraID kNorad01ZoomInWithGasCanister = 2;
static const ExtraID kN01WGasCanister = 3;
static const ExtraID kNorad01ZoomOutWithGasCanister = 4;
static const ExtraID kN01WZEmptyLit = 5;
static const ExtraID kN01WZGasCanisterDim = 6;
static const ExtraID kN01WZGasCanisterLit = 7;
static const ExtraID kN01WZArgonCanisterDim = 8;
static const ExtraID kN01WZArgonCanisterLit = 9;
static const ExtraID kN01WZAirMaskDim = 10;
static const ExtraID kN01WZAirMaskLit = 11;
static const ExtraID kN01WZNitrogenCanisterDim = 12;
static const ExtraID kN01WZNitrogenCanisterLit = 13;
static const ExtraID kNorad04EastDeath = 14;
static const ExtraID kNorad19PrepSub = 15;
static const ExtraID kNorad19ExitToSub = 16;
static const ExtraID kNorad22SouthIntro = 17;
static const ExtraID kNorad22SouthReply = 18;
static const ExtraID kNorad22SouthFinish = 19;
static const ExtraID kN22ClawFromAToB = 20;
static const ExtraID kN22ClawALoop = 21;
static const ExtraID kN22ClawAPinch = 22;
static const ExtraID kN22ClawACounterclockwise = 23;
static const ExtraID kN22ClawAClockwise = 24;
static const ExtraID kN22ClawFromBToA = 25;
static const ExtraID kN22ClawFromBToC = 26;
static const ExtraID kN22ClawFromBToD = 27;
static const ExtraID kN22ClawBLoop = 28;
static const ExtraID kN22ClawBPinch = 29;
static const ExtraID kN22ClawBCounterclockwise = 30;
static const ExtraID kN22ClawBClockwise = 31;
static const ExtraID kN22ClawFromCToB = 32;
static const ExtraID kN22ClawCLoop = 33;
static const ExtraID kN22ClawCPinch = 34;
static const ExtraID kN22ClawCCounterclockwise = 35;
static const ExtraID kN22ClawCClockwise = 36;
static const ExtraID kN22ClawFromDToB = 37;
static const ExtraID kN22ClawDLoop = 38;
static const ExtraID kN22ClawDPinch = 39;
static const ExtraID kN22ClawDCounterclockwise = 40;
static const ExtraID kN22ClawDClockwise = 41;


// Norad Delta Extra sequence IDs.

static const ExtraID kArriveFromSubChase = 0;
static const ExtraID kN59ZoomWithRobot = 1;
static const ExtraID kN59RobotApproaches = 2;
static const ExtraID kN59RobotPunchLoop = 3;
static const ExtraID kN59PlayerWins1 = 4;
static const ExtraID kN59PlayerWins2 = 5;
static const ExtraID kN59RobotWins = 6;
static const ExtraID kN59RobotHeadOpens = 7;
static const ExtraID kN59Biochips111 = 8;
static const ExtraID kN59Biochips011 = 9;
static const ExtraID kN59Biochips101 = 10;
static const ExtraID kN59Biochips001 = 11;
static const ExtraID kN59Biochips110 = 12;
static const ExtraID kN59Biochips010 = 13;
static const ExtraID kN59Biochips100 = 14;
static const ExtraID kN59Biochips000 = 15;
static const ExtraID kN59RobotDisappears = 16;
static const ExtraID kN60ClawFromAToB = 17;
static const ExtraID kN60ClawALoop = 18;
static const ExtraID kN60ClawAPinch = 19;
static const ExtraID kN60ClawACounterclockwise = 20;
static const ExtraID kN60ClawAClockwise = 21;
static const ExtraID kN60ClawFromBToA = 22;
static const ExtraID kN60ClawFromBToC = 23;
static const ExtraID kN60ClawFromBToD = 24;
static const ExtraID kN60ClawBLoop = 25;
static const ExtraID kN60ClawBPinch = 26;
static const ExtraID kN60ClawBCounterclockwise = 27;
static const ExtraID kN60ClawBClockwise = 28;
static const ExtraID kN60ClawFromCToB = 29;
static const ExtraID kN60ClawCLoop = 30;
static const ExtraID kN60ClawCPinch = 31;
static const ExtraID kN60ClawCCounterclockwise = 32;
static const ExtraID kN60ClawCClockwise = 33;
static const ExtraID kN60ClawFromDToB = 34;
static const ExtraID kN60ClawDLoop = 35;
static const ExtraID kN60ClawDPinch = 36;
static const ExtraID kN60ClawDCounterclockwise = 37;
static const ExtraID kN60ClawDClockwise = 38;
static const ExtraID kN60RobotApproaches = 39;
static const ExtraID kN60FirstMistake = 40;
static const ExtraID kN60ArmActivated = 41;
static const ExtraID kN60SecondMistake = 42;
static const ExtraID kN60ArmToPositionB = 43;
static const ExtraID kN60ThirdMistake = 44;
static const ExtraID kN60ArmGrabsRobot = 45;
static const ExtraID kN60FourthMistake = 46;
static const ExtraID kN60ArmCarriesRobotToPositionA = 47;
static const ExtraID kN60PlayerFollowsRobotToDoor = 48;
static const ExtraID kN60RobotHeadOpens = 49;
static const ExtraID kN60Biochips111 = 50;
static const ExtraID kN60Biochips011 = 51;
static const ExtraID kN60Biochips101 = 52;
static const ExtraID kN60Biochips001 = 53;
static const ExtraID kN60Biochips110 = 54;
static const ExtraID kN60Biochips010 = 55;
static const ExtraID kN60Biochips100 = 56;
static const ExtraID kN60Biochips000 = 57;
static const ExtraID kN60RobotDisappears = 58;
static const ExtraID kNoradDeltaRetinalScanBad = 59;
static const ExtraID kNoradDeltaRetinalScanGood = 60;
static const ExtraID kN79BrightView = 61;

// Norad Delta Tables

static const TimeScale kNoradDeltaMovieScale = 600;
static const TimeScale kNoradDeltaFramesPerSecond = 15;
static const TimeScale kNoradDeltaFrameDuration = 40;

// Alternate IDs.

static const AlternateID kAltNoradDeltaNormal = 0;

// Room IDs.

static const RoomID kNorad41 = 0;
static const RoomID kNorad42 = 1;
static const RoomID kNorad43 = 2;
static const RoomID kNorad44 = 3;
static const RoomID kNorad45 = 4;
static const RoomID kNorad46 = 5;
static const RoomID kNorad47 = 6;
static const RoomID kNorad48 = 7;
static const RoomID kNorad48South = 8;
static const RoomID kNorad49 = 9;
static const RoomID kNorad49South = 10;
static const RoomID kNorad50 = 11;
static const RoomID kNorad50East = 12;
static const RoomID kNorad51 = 13;
static const RoomID kNorad52 = 14;
static const RoomID kNorad53 = 15;
static const RoomID kNorad54 = 16;
static const RoomID kNorad54North = 17;
static const RoomID kNorad55 = 18;
static const RoomID kNorad56 = 19;
static const RoomID kNorad57 = 20;
static const RoomID kNorad58 = 21;
static const RoomID kNorad59 = 22;
static const RoomID kNorad59West = 23;
static const RoomID kNorad60 = 24;
static const RoomID kNorad60West = 25;
static const RoomID kNorad61 = 26;
static const RoomID kNorad62 = 27;
static const RoomID kNorad63 = 28;
static const RoomID kNorad64 = 29;
static const RoomID kNorad65 = 30;
static const RoomID kNorad66 = 31;
static const RoomID kNorad67 = 32;
static const RoomID kNorad68 = 33;
static const RoomID kNorad68West = 34;
static const RoomID kNorad69 = 35;
static const RoomID kNorad78 = 36;
static const RoomID kNorad79 = 37;
static const RoomID kNorad79West = 38;

// Hot Spot Activation IDs.


// Hot Spot IDs.

static const HotSpotID kNorad48ElevatorSpotID = 5000;
static const HotSpotID kNorad48ElevatorOutSpotID = 5001;
static const HotSpotID kNorad48ElevatorUpSpotID = 5002;
static const HotSpotID kNorad49ElevatorSpotID = 5003;
static const HotSpotID kNorad49ElevatorOutSpotID = 5004;
static const HotSpotID kNorad49ElevatorDownSpotID = 5005;
static const HotSpotID kNorad50DoorSpotID = 5006;
static const HotSpotID kNorad50DoorOutSpotID = 5007;
static const HotSpotID kDeltaUpperPressureDoorUpSpotID = 5008;
static const HotSpotID kDeltaUpperPressureDoorDownSpotID = 5009;
static const HotSpotID kNorad54DoorSpotID = 5010;
static const HotSpotID kNorad54DoorOutSpotID = 5011;
static const HotSpotID kNorad59WestSpotID = 5012;
static const HotSpotID kNorad59WestOutSpotID = 5013;
static const HotSpotID kDeltaLowerPressureDoorUpSpotID = 5014;
static const HotSpotID kDeltaLowerPressureDoorDownSpotID = 5015;
static const HotSpotID kDelta59RobotHeadSpotID = 5016;
static const HotSpotID kDelta59RobotShieldBiochipSpotID = 5017;
static const HotSpotID kDelta59RobotOpMemBiochipSpotID = 5018;
static const HotSpotID kDelta59RobotRetinalBiochipSpotID = 5019;
static const HotSpotID kNorad60MonitorSpotID = 5020;
static const HotSpotID kNorad60MonitorOutSpotID = 5021;
static const HotSpotID kNorad60LaunchPrepSpotID = 5022;
static const HotSpotID kNorad60ClawControlSpotID = 5023;
static const HotSpotID kNorad60ClawPinchSpotID = 5024;
static const HotSpotID kNorad60ClawDownSpotID = 5025;
static const HotSpotID kNorad60ClawRightSpotID = 5026;
static const HotSpotID kNorad60ClawLeftSpotID = 5027;
static const HotSpotID kNorad60ClawUpSpotID = 5028;
static const HotSpotID kNorad60ClawCCWSpotID = 5029;
static const HotSpotID kNorad60ClawCWSpotID = 5030;
static const HotSpotID kDelta60RobotHeadSpotID = 5031;
static const HotSpotID kDelta60RobotShieldBiochipSpotID = 5032;
static const HotSpotID kDelta60RobotOpMemBiochipSpotID = 5033;
static const HotSpotID kDelta60RobotRetinalBiochipSpotID = 5034;
static const HotSpotID kNorad68WestSpotID = 5035;
static const HotSpotID kNorad68WestOutSpotID = 5036;
static const HotSpotID kNorad79WestSpotID = 5037;
static const HotSpotID kNorad79WestOutSpotID = 5038;
static const HotSpotID kNorad79SpinLeftSpotID = 5039;
static const HotSpotID kNorad79SpinRightSpotID = 5040;
static const HotSpotID kNorad79SpinUpSpotID = 5041;
static const HotSpotID kNorad79SpinDownSpotID = 5042;
static const HotSpotID kNorad79SiloAreaSpotID = 5043;

} // End of namespace Pegasus

#endif
