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

#ifndef PEGASUS_NEIGHBORHOOD_MARS_CONSTANTS_H
#define PEGASUS_NEIGHBORHOOD_MARS_CONSTANTS_H

#include "pegasus/constants.h"

namespace Pegasus {

// Element Coordinates

static const CoordType kUndoHiliteLeft = kNavAreaLeft + 140;
static const CoordType kUndoHiliteTop = kNavAreaTop + 36;

static const CoordType kCurrentGuessLeft = kNavAreaLeft + 146;
static const CoordType kCurrentGuessTop = kNavAreaTop + 90;

static const CoordType kReactorChoiceHiliteLeft = kNavAreaLeft + 116;
static const CoordType kReactorChoiceHiliteTop = kNavAreaTop + 158;

static const CoordType kReactorHistoryLeft = kNavAreaLeft + 302;
static const CoordType kReactorHistoryTop = kNavAreaTop + 39;

static const CoordType kAnswerLeft = kNavAreaLeft + 304;
static const CoordType kAnswerTop = kNavAreaTop + 180;

static const CoordType kShuttle1Left = 0;
static const CoordType kShuttle1Top = 0;

static const CoordType kShuttle2Left = 0;
static const CoordType kShuttle2Top = 96;

static const CoordType kShuttle3Left = 500;
static const CoordType kShuttle3Top = 96;

static const CoordType kShuttle4Left = 0;
static const CoordType kShuttle4Top = 320;

static const CoordType kShuttleWindowLeft = 140;
static const CoordType kShuttleWindowTop = 96;
static const CoordType kShuttleWindowWidth = 360;
static const CoordType kShuttleWindowHeight = 224;

static const CoordType kShuttleWindowMidH = (kShuttleWindowLeft * 2 + kShuttleWindowWidth) / 2;
static const CoordType kShuttleWindowMidV = (kShuttleWindowTop * 2 + kShuttleWindowHeight) / 2;

static const CoordType kShuttleLeftLeft = 0;
static const CoordType kShuttleLeftTop = 128;

static const CoordType kShuttleRightLeft = 506;
static const CoordType kShuttleRightTop = 128;

static const CoordType kShuttleLowerLeftLeft = 74;
static const CoordType kShuttleLowerLeftTop = 358;

static const CoordType kShuttleLowerRightLeft = 486;
static const CoordType kShuttleLowerRightTop = 354;

static const CoordType kShuttleCenterLeft = 260;
static const CoordType kShuttleCenterTop = 336;

static const CoordType kShuttleUpperLeftLeft = 30;
static const CoordType kShuttleUpperLeftTop = 32;

static const CoordType kShuttleUpperRightLeft = 506;
static const CoordType kShuttleUpperRightTop = 52;

static const CoordType kShuttleLeftEnergyLeft = 110;
static const CoordType kShuttleLeftEnergyTop = 186;

static const CoordType kShuttleRightEnergyLeft = 510;
static const CoordType kShuttleRightEnergyTop = 186;

static const CoordType kShuttleEnergyLeft = 186;
static const CoordType kShuttleEnergyTop = 60;
static const CoordType kShuttleEnergyWidth = 252;
static const CoordType kShuttleEnergyHeight = 22;

static const CoordType kPlanetStartLeft = kShuttleWindowLeft;
static const CoordType kPlanetStartTop = kShuttleWindowTop + kShuttleWindowHeight;

static const CoordType kPlanetStopLeft = kShuttleWindowLeft;
static const CoordType kPlanetStopTop = kShuttleWindowTop + kShuttleWindowHeight - 100;

static const CoordType kShuttleTractorLeft = kShuttleWindowLeft + 6;
static const CoordType kShuttleTractorTop = kShuttleWindowTop + 56;
static const CoordType kShuttleTractorWidth = 348;
static const CoordType kShuttleTractorHeight = 112;

static const CoordType kShuttleJunkLeft = kShuttleWindowLeft + 6;
static const CoordType kShuttleJunkTop = kShuttleWindowTop + 6;

static const DisplayOrder kShuttlePlanetOrder = kInterfaceLayer;
static const DisplayOrder kShuttleAlienShipOrder = kShuttlePlanetOrder + 1;
static const DisplayOrder kShuttleRobotShipOrder = kShuttleAlienShipOrder + 1;
static const DisplayOrder kShuttleTractorBeamMovieOrder = kShuttleRobotShipOrder + 1;
static const DisplayOrder kShuttleWeaponBackOrder = kShuttleTractorBeamMovieOrder + 1;
static const DisplayOrder kShuttleJunkOrder = kShuttleWeaponBackOrder + 1;
static const DisplayOrder kShuttleWeaponFrontOrder = kShuttleJunkOrder + 1;
static const DisplayOrder kShuttleTractorBeamOrder = kShuttleWeaponFrontOrder + 1;
static const DisplayOrder kShuttleHUDOrder = kShuttleTractorBeamOrder + 1;
static const DisplayOrder kShuttleBackgroundOrder = kShuttleHUDOrder + 1;
static const DisplayOrder kShuttleMonitorOrder = kShuttleBackgroundOrder + 1;
static const DisplayOrder kShuttleStatusOrder = kShuttleMonitorOrder + 1;

static const TimeValue kShuttleSwingStart = 0;
static const TimeValue kShuttleSwingStop = 5 * 600;

static const TimeValue kCanyonChaseStart = kShuttleSwingStop;
static const TimeValue kCanyonChaseStop = 60 * 600 + 43 * 600 + 14 * 40;

static const TimeValue kLaunchTubeReachedTime = 60 * 600 + 38 * 600 - kCanyonChaseStart;
static const TimeValue kCanyonChaseFinishedTime = kCanyonChaseStop - kCanyonChaseStart -
											kLaunchTubeReachedTime;

// Left shuttle.

static const TimeValue kShuttleLeftIntroStart = 0;
static const TimeValue kShuttleLeftIntroStop = 400;

static const TimeValue kShuttleLeftBlankTime = 400;

static const TimeValue kShuttleLeftNormalTime = 440;

static const TimeValue kShuttleLeftAutoTestTime = 480;

static const TimeValue kShuttleLeftDamagedTime = 520;

static const TimeValue kShuttleLeftDampingTime = 560;

static const TimeValue kShuttleLeftGravitonTime = 600;

static const TimeValue kShuttleLeftTractorTime = 640;

// Right shuttle.

static const TimeValue kShuttleRightIntroStart = 0;
static const TimeValue kShuttleRightIntroStop = 400;

static const TimeValue kShuttleRightDestroyedStart = 400;
static const TimeValue kShuttleRightDestroyedStop = 840;

static const TimeValue kShuttleRightBlankTime = 840;

static const TimeValue kShuttleRightNormalTime = 880;

static const TimeValue kShuttleRightDamagedTime = 920;

static const TimeValue kShuttleRightTargetLockTime = 960;

static const TimeValue kShuttleRightGravitonTime = 1000;

static const TimeValue kShuttleRightOverloadTime = 1040;

// Lower Left shuttle.

static const TimeValue kShuttleLowerLeftCollisionTime = 0;

static const TimeValue kShuttleLowerLeftTubeTime = 40;

static const TimeValue kShuttleLowerLeftAutopilotTime = 80;

// Lower Right shuttle.

static const TimeValue kShuttleLowerRightOffTime = 0;

static const TimeValue kShuttleLowerRightTrackingTime = 40;

static const TimeValue kShuttleLowerRightTransportTime = 80;

static const TimeValue kShuttleLowerRightTransportHiliteTime = 120;

// Center shuttle.

static const TimeValue kShuttleCenterBoardingTime = 0;

static const TimeValue kShuttleCenterCheckTime = 40;

static const TimeValue kShuttleCenterNavCompTime = 80;

static const TimeValue kShuttleCenterCommTime = 120;

static const TimeValue kShuttleCenterWeaponsTime = 160;

static const TimeValue kShuttleCenterAllSystemsTime = 200;

static const TimeValue kShuttleCenterSecureLooseTime = 240;

static const TimeValue kShuttleCenterAutoTestTime = 280;

static const TimeValue kShuttleCenterLaunchTime = 320;

static const TimeValue kShuttleCenterEnterTubeTime = 360;

static const TimeValue kShuttleCenterTargetSightedTime = 400;

static const TimeValue kShuttleCenterVerifyingTime = 440;

static const TimeValue kShuttleCenterScanningTime = 480;

static const TimeValue kShuttleCenterSafeTime = 520;

// Upper Left shuttle.

static const TimeValue kShuttleUpperLeftDimTime = 0;

static const TimeValue kShuttleUpperLeftDampingTime = 40;

static const TimeValue kShuttleUpperLeftGravitonTime = 80;

static const TimeValue kShuttleUpperLeftTractorTime = 120;

// Upper Right shuttle.

static const TimeValue kShuttleUpperRightLockedTime = 0;

static const TimeValue kShuttleUpperRightArmedTime = 40;

static const TimeValue kShuttleUpperRightAlienDestroyedTime = 80;

static const TimeValue kShuttleUpperRightOverloadTime = 120;

static const TimeValue kShuttleUpperRightTargetDestroyedTime = 160;

// Shuttle distance

static const int kShuttleDistance = 500;

static const int kJunkMaxDistance = kShuttleDistance;
static const int kJunkMinDistance = 40;

static const int kEnergyBeamMaxDistance = kShuttleDistance;
static const int kEnergyBeamMinDistance = 40;

static const int kGravitonMaxDistance = kShuttleDistance;
static const int kGravitonMinDistance = 40;

static const TimeValue kMarsOxyMaskOnIn = 0;
static const TimeValue kMarsOxyMaskOnOut = 1560;

static const TimeValue kMarsAirlockButtonBeepIn = 1560;
static const TimeValue kMarsAirlockButtonBeepOut = 1620;

static const TimeValue kMarsColorMatchingButtonBeepIn = 1620;
static const TimeValue kMarsColorMatchingButtonBeepOut = 1680;

static const TimeValue kMarsKioskBeepIn = 1680;
static const TimeValue kMarsKioskBeepOut = 1740;

static const TimeValue kMarsBumpIntoWallIn = 1740;
static const TimeValue kMarsBumpIntoWallOut = 1888;

static const TimeValue kMarsGantryDoorCloseIn = 1888;
static const TimeValue kMarsGantryDoorCloseOut = 2866;

static const TimeValue kMarsTransportDoorCloseIn = 2866;
static const TimeValue kMarsTransportDoorCloseOut = 3593;

static const TimeValue kMarsAirlockPressurizeIn = 3593;
static const TimeValue kMarsAirlockPressurizeOut = 4766;

static const TimeValue kMarsBigAirlockDoorCloseIn = 4766;
static const TimeValue kMarsBigAirlockDoorCloseOut = 7872;

static const TimeValue kMarsSmallAirlockDoorCloseIn = 7872;
static const TimeValue kMarsSmallAirlockDoorCloseOut = 10000;

static const TimeValue kMarsMazeDoorCloseIn = 10000;
static const TimeValue kMarsMazeDoorCloseOut = 10969;

static const TimeValue kMarsRobotTakesTransportIn = 10969;
static const TimeValue kMarsRobotTakesTransportOut = 12802;

static const TimeValue kMarsPodDepartedUpperPlatformIn = 12802;
static const TimeValue kMarsPodDepartedUpperPlatformOut = 15783;

static const TimeValue kMarsPodDepartedLowerPlatformIn = 15783;
static const TimeValue kMarsPodDepartedLowerPlatformOut = 18736;

static const TimeValue kMarsPodArrivedUpperPlatformIn = 18736;
static const TimeValue kMarsPodArrivedUpperPlatformOut = 21605;

static const TimeValue kMarsCheckInRequiredIn = 21605;
static const TimeValue kMarsCheckInRequiredOut = 27463;

static const TimeValue kMarsCantOpenShuttleIn = 27463;
static const TimeValue kMarsCantOpenShuttleOut = 29214;

static const TimeValue kMarsShuttleLockOverrideIn = 29214;
static const TimeValue kMarsShuttleLockOverrideOut = 30330;

static const TimeValue kMarsNoShuttleIn = 30330;
static const TimeValue kMarsNoShuttleOut = 31502;

static const TimeValue kMustBeUnlockedIn = 31502;
static const TimeValue kMustBeUnlockedOut = 33960;

static const TimeValue kColorMatchBlueIn = 33960;
static const TimeValue kColorMatchBlueOut = 34240;

static const TimeValue kColorMatchRedIn = 34240;
static const TimeValue kColorMatchRedOut = 34538;

static const TimeValue kColorMatchGreenIn = 34538;
static const TimeValue kColorMatchGreenOut = 34827;

static const TimeValue kColorMatchYellowIn = 34827;
static const TimeValue kColorMatchYellowOut = 35162;

static const TimeValue kColorMatchPurpleIn = 35162;
static const TimeValue kColorMatchPurpleOut = 35426;

static const TimeValue kColorMatchZeroNodesIn = 35426;
static const TimeValue kColorMatchZeroNodesOut = 36376;

static const TimeValue kColorMatchOneNodeIn = 36376;
static const TimeValue kColorMatchOneNodeOut = 37209;

static const TimeValue kColorMatchTwoNodesIn = 37209;
static const TimeValue kColorMatchTwoNodesOut = 37983;

static const TimeValue kColorMatchThreeNodesIn = 37983;
static const TimeValue kColorMatchThreeNodesOut = 38784;

static const TimeValue kMarsShuttle1DepartedIn = 38784;
static const TimeValue kMarsShuttle1DepartedOut = 40323;

static const TimeValue kMarsShuttle2DepartedIn = 40323;
static const TimeValue kMarsShuttle2DepartedOut = 41824;

static const TimeValue kShuttleCockpitIn = 41824;
static const TimeValue kShuttleCockpitOut = 43126;

static const TimeValue kShuttleOnboardIn = 43126;
static const TimeValue kShuttleOnboardOut = 44284;

static const TimeValue kShuttleNavigationIn = 44284;
static const TimeValue kShuttleNavigationOut = 46049;

static const TimeValue kShuttleCommunicationIn = 46049;
static const TimeValue kShuttleCommunicationOut = 47288;

static const TimeValue kShuttleAutoTestingIn = 47288;
static const TimeValue kShuttleAutoTestingOut = 48179;

static const TimeValue kMarsThrusterAutoTestIn = 48179;
static const TimeValue kMarsThrusterAutoTestOut = 49979;

static const TimeValue kShuttleAllSystemsIn = 49979;
static const TimeValue kShuttleAllSystemsOut = 51065;

static const TimeValue kShuttleSecureLooseIn = 51065;
static const TimeValue kShuttleSecureLooseOut = 52346;

static const TimeValue kShuttlePrepareForDropIn = 52346;
static const TimeValue kShuttlePrepareForDropOut = 53216;

static const TimeValue kShuttleAllClearIn = 53216;
static const TimeValue kShuttleAllClearOut = 54031;

static const TimeValue kShuttleConfiguringIn = 54031;
static const TimeValue kShuttleConfiguringOut = 54994;

static const TimeValue kShuttleGeneratingIn = 54994;
static const TimeValue kShuttleGeneratingOut = 56033;

static const TimeValue kShuttleBreakawayIn = 56033;
static const TimeValue kShuttleBreakawayOut = 57346;

static const TimeValue kMarsAtmosphericBreakawayIn = 57346;
static const TimeValue kMarsAtmosphericBreakawayOut = 59237;

static const TimeValue kMarsCockpitChatterIn = 59237;
static const TimeValue kMarsCockpitChatterOut = 70344;

static const TimeValue kShuttleDamperDescIn = 70344;
static const TimeValue kShuttleDamperDescOut = 73262;

static const TimeValue kShuttleGravitonDescIn = 73262;
static const TimeValue kShuttleGravitonDescOut = 75296;

static const TimeValue kShuttleTractorDescIn = 75296;
static const TimeValue kShuttleTractorDescOut = 78381;

static const TimeValue kShuttleTargetSightedIn = 78381;
static const TimeValue kShuttleTargetSightedOut = 79074;

static const TimeValue kShuttleAutopilotEngagedIn = 79074;
static const TimeValue kShuttleAutopilotEngagedOut = 80414;

static const TimeValue kMarsEDBBlastIn = 80414;
static const TimeValue kMarsEDBBlastOut = 80705;

static const TimeValue kMarsGravitonBlastIn = 80705;
static const TimeValue kMarsGravitonBlastOut = 81199;

static const TimeValue kMarsJunkCollisionIn = 81199;
static const TimeValue kMarsJunkCollisionOut = 81961;

static const TimeValue kShuttleGravitonIn = 81961;
static const TimeValue kShuttleGravitonOut = 82587;

static const TimeValue kShuttleDampingBeamIn = 82587;
static const TimeValue kShuttleDampingBeamOut = 83331;

static const TimeValue kShuttleTractorBeamIn = 83331;
static const TimeValue kShuttleTractorBeamOut = 83802;

static const TimeValue kShuttleHullBreachIn = 83802;
static const TimeValue kShuttleHullBreachOut = 84721;

static const TimeValue kShuttleWingDamageIn = 84721;
static const TimeValue kShuttleWingDamageOut = 85640;

static const TimeValue kShuttleHullDamageIn = 85640;
static const TimeValue kShuttleHullDamageOut = 86513;

static const TimeValue kShuttleEnergyTooLowIn = 86513;
static const TimeValue kShuttleEnergyTooLowOut = 87578;

static const TimeValue kShuttleTractorLimitedIn = 87578;
static const TimeValue kShuttleTractorLimitedOut = 89164;

static const TimeValue kShuttleCantHoldIn = 89164;
static const TimeValue kShuttleCantHoldOut = 90945;

static const TimeValue kShuttleBrokeFreeIn = 90945;
static const TimeValue kShuttleBrokeFreeOut = 92322;

static const TimeValue kShuttleDestroyedIn = 92322;
static const TimeValue kShuttleDestroyedOut = 93189;

static const TimeValue kShuttleCoordinatesIn = 93189;
static const TimeValue kShuttleCoordinatesOut = 94018;

static const TimeValue kShuttleScanningIn = 94018;
static const TimeValue kShuttleScanningOut = 94975;

static const TimeValue kShuttleSafeIn = 94975;
static const TimeValue kShuttleSafeOut = 96176;

static const TimeValue kShuttleOverloadedIn = 96176;
static const TimeValue kShuttleOverloadedOut = 101308;

static const TimeScale kMarsMovieScale = 600;
static const TimeScale kMarsFramesPerSecond = 15;
static const TimeScale kMarsFrameDuration = 40;

// Alternate IDs.

static const AlternateID kAltMarsNormal = 0;
static const AlternateID kAltMarsPodAtMars34 = 1;
static const AlternateID kAltMarsTookCard = 2;
static const AlternateID kAltMars35AirlockEast = 3;
static const AlternateID kAltMars35AirlockWest = 4;
static const AlternateID kAltMarsPodAtMars45 = 5;
static const AlternateID kAltMarsTookMask = 6;
static const AlternateID kAltMarsMaskOnFiller = 7;
static const AlternateID kAltMars60AirlockEast = 8;
static const AlternateID kAltMars60AirlockWest = 9;

// Room IDs.

static const RoomID kMars0A = 0;
static const RoomID kMars00 = 1;
static const RoomID kMars01 = 2;
static const RoomID kMars02 = 3;
static const RoomID kMars03 = 4;
static const RoomID kMars04 = 5;
static const RoomID kMars05 = 6;
static const RoomID kMars06 = 7;
static const RoomID kMars07 = 8;
static const RoomID kMars08 = 9;
static const RoomID kMars09 = 10;
static const RoomID kMars10 = 11;
static const RoomID kMars11 = 12;
static const RoomID kMars12 = 13;
static const RoomID kMars13 = 14;
static const RoomID kMars14 = 15;
static const RoomID kMars15 = 16;
static const RoomID kMars16 = 17;
static const RoomID kMars17 = 18;
static const RoomID kMars18 = 19;
static const RoomID kMars19 = 20;
static const RoomID kMars20 = 21;
static const RoomID kMars21 = 22;
static const RoomID kMars22 = 23;
static const RoomID kMars23 = 24;
static const RoomID kMars24 = 25;
static const RoomID kMars25 = 26;
static const RoomID kMars26 = 27;
static const RoomID kMars27 = 28;
static const RoomID kMars28 = 29;
static const RoomID kMars29 = 30;
static const RoomID kMars30 = 31;
static const RoomID kMars31 = 32;
static const RoomID kMars31South = 33;
static const RoomID kMars32 = 34;
static const RoomID kMars33 = 35;
static const RoomID kMars33North = 36;
static const RoomID kMars34 = 37;
static const RoomID kMars35 = 38;
static const RoomID kMars36 = 39;
static const RoomID kMars37 = 40;
static const RoomID kMars38 = 41;
static const RoomID kMars39 = 42;
static const RoomID kMars41 = 43;
static const RoomID kMars42 = 44;
static const RoomID kMars43 = 45;
static const RoomID kMars44 = 46;
static const RoomID kMars45 = 47;
static const RoomID kMars46 = 48;
static const RoomID kMars47 = 49;
static const RoomID kMars48 = 50;
static const RoomID kMars49 = 51;
static const RoomID kMars50 = 52;
static const RoomID kMars51 = 53;
static const RoomID kMars52 = 54;
static const RoomID kMars54 = 55;
static const RoomID kMars56 = 56;
static const RoomID kMars58 = 57;
static const RoomID kMars60 = 58;
static const RoomID kMarsRobotShuttle = 59;
static const RoomID kMarsMaze004 = 60;
static const RoomID kMarsMaze005 = 61;
static const RoomID kMarsMaze006 = 62;
static const RoomID kMarsMaze007 = 63;
static const RoomID kMarsMaze008 = 64;
static const RoomID kMarsMaze009 = 65;
static const RoomID kMarsMaze010 = 66;
static const RoomID kMarsMaze011 = 67;
static const RoomID kMarsMaze012 = 68;
static const RoomID kMarsMaze015 = 69;
static const RoomID kMarsMaze016 = 70;
static const RoomID kMarsMaze017 = 71;
static const RoomID kMarsMaze018 = 72;
static const RoomID kMarsMaze019 = 73;
static const RoomID kMarsMaze020 = 74;
static const RoomID kMarsMaze021 = 75;
static const RoomID kMarsMaze022 = 76;
static const RoomID kMarsMaze023 = 77;
static const RoomID kMarsMaze024 = 78;
static const RoomID kMarsMaze025 = 79;
static const RoomID kMarsMaze026 = 80;
static const RoomID kMarsMaze027 = 81;
static const RoomID kMarsMaze028 = 82;
static const RoomID kMarsMaze031 = 83;
static const RoomID kMarsMaze032 = 84;
static const RoomID kMarsMaze033 = 85;
static const RoomID kMarsMaze034 = 86;
static const RoomID kMarsMaze035 = 87;
static const RoomID kMarsMaze036 = 88;
static const RoomID kMarsMaze037 = 89;
static const RoomID kMarsMaze038 = 90;
static const RoomID kMarsMaze039 = 91;
static const RoomID kMarsMaze042 = 92;
static const RoomID kMarsMaze043 = 93;
static const RoomID kMarsMaze044 = 94;
static const RoomID kMarsMaze045 = 95;
static const RoomID kMarsMaze046 = 96;
static const RoomID kMarsMaze047 = 97;
static const RoomID kMarsMaze049 = 98;
static const RoomID kMarsMaze050 = 99;
static const RoomID kMarsMaze051 = 100;
static const RoomID kMarsMaze052 = 101;
static const RoomID kMarsMaze053 = 102;
static const RoomID kMarsMaze054 = 103;
static const RoomID kMarsMaze055 = 104;
static const RoomID kMarsMaze056 = 105;
static const RoomID kMarsMaze057 = 106;
static const RoomID kMarsMaze058 = 107;
static const RoomID kMarsMaze059 = 108;
static const RoomID kMarsMaze060 = 109;
static const RoomID kMarsMaze061 = 110;
static const RoomID kMarsMaze063 = 111;
static const RoomID kMarsMaze064 = 112;
static const RoomID kMarsMaze065 = 113;
static const RoomID kMarsMaze066 = 114;
static const RoomID kMarsMaze067 = 115;
static const RoomID kMarsMaze068 = 116;
static const RoomID kMarsMaze069 = 117;
static const RoomID kMarsMaze070 = 118;
static const RoomID kMarsMaze071 = 119;
static const RoomID kMarsMaze072 = 120;
static const RoomID kMarsMaze074 = 121;
static const RoomID kMarsMaze076 = 122;
static const RoomID kMarsMaze078 = 123;
static const RoomID kMarsMaze079 = 124;
static const RoomID kMarsMaze081 = 125;
static const RoomID kMarsMaze083 = 126;
static const RoomID kMarsMaze084 = 127;
static const RoomID kMarsMaze085 = 128;
static const RoomID kMarsMaze086 = 129;
static const RoomID kMarsMaze087 = 130;
static const RoomID kMarsMaze088 = 131;
static const RoomID kMarsMaze089 = 132;
static const RoomID kMarsMaze090 = 133;
static const RoomID kMarsMaze091 = 134;
static const RoomID kMarsMaze092 = 135;
static const RoomID kMarsMaze093 = 136;
static const RoomID kMarsMaze098 = 137;
static const RoomID kMarsMaze099 = 138;
static const RoomID kMarsMaze100 = 139;
static const RoomID kMarsMaze101 = 140;
static const RoomID kMarsMaze104 = 141;
static const RoomID kMarsMaze105 = 142;
static const RoomID kMarsMaze106 = 143;
static const RoomID kMarsMaze107 = 144;
static const RoomID kMarsMaze108 = 145;
static const RoomID kMarsMaze111 = 146;
static const RoomID kMarsMaze113 = 147;
static const RoomID kMarsMaze114 = 148;
static const RoomID kMarsMaze115 = 149;
static const RoomID kMarsMaze116 = 150;
static const RoomID kMarsMaze117 = 151;
static const RoomID kMarsMaze118 = 152;
static const RoomID kMarsMaze119 = 153;
static const RoomID kMarsMaze120 = 154;
static const RoomID kMarsMaze121 = 155;
static const RoomID kMarsMaze122 = 156;
static const RoomID kMarsMaze123 = 157;
static const RoomID kMarsMaze124 = 158;
static const RoomID kMarsMaze125 = 159;
static const RoomID kMarsMaze126 = 160;
static const RoomID kMarsMaze127 = 161;
static const RoomID kMarsMaze128 = 162;
static const RoomID kMarsMaze129 = 163;
static const RoomID kMarsMaze130 = 164;
static const RoomID kMarsMaze131 = 165;
static const RoomID kMarsMaze132 = 166;
static const RoomID kMarsMaze133 = 167;
static const RoomID kMarsMaze136 = 168;
static const RoomID kMarsMaze137 = 169;
static const RoomID kMarsMaze138 = 170;
static const RoomID kMarsMaze139 = 171;
static const RoomID kMarsMaze140 = 172;
static const RoomID kMarsMaze141 = 173;
static const RoomID kMarsMaze142 = 174;
static const RoomID kMarsMaze143 = 175;
static const RoomID kMarsMaze144 = 176;
static const RoomID kMarsMaze145 = 177;
static const RoomID kMarsMaze146 = 178;
static const RoomID kMarsMaze147 = 179;
static const RoomID kMarsMaze148 = 180;
static const RoomID kMarsMaze149 = 181;
static const RoomID kMarsMaze152 = 182;
static const RoomID kMarsMaze153 = 183;
static const RoomID kMarsMaze154 = 184;
static const RoomID kMarsMaze155 = 185;
static const RoomID kMarsMaze156 = 186;
static const RoomID kMarsMaze157 = 187;
static const RoomID kMarsMaze159 = 188;
static const RoomID kMarsMaze160 = 189;
static const RoomID kMarsMaze161 = 190;
static const RoomID kMarsMaze162 = 191;
static const RoomID kMarsMaze163 = 192;
static const RoomID kMarsMaze164 = 193;
static const RoomID kMarsMaze165 = 194;
static const RoomID kMarsMaze166 = 195;
static const RoomID kMarsMaze167 = 196;
static const RoomID kMarsMaze168 = 197;
static const RoomID kMarsMaze169 = 198;
static const RoomID kMarsMaze170 = 199;
static const RoomID kMarsMaze171 = 200;
static const RoomID kMarsMaze172 = 201;
static const RoomID kMarsMaze173 = 202;
static const RoomID kMarsMaze174 = 203;
static const RoomID kMarsMaze175 = 204;
static const RoomID kMarsMaze177 = 205;
static const RoomID kMarsMaze178 = 206;
static const RoomID kMarsMaze179 = 207;
static const RoomID kMarsMaze180 = 208;
static const RoomID kMarsMaze181 = 209;
static const RoomID kMarsMaze182 = 210;
static const RoomID kMarsMaze183 = 211;
static const RoomID kMarsMaze184 = 212;
static const RoomID kMarsMaze187 = 213;
static const RoomID kMarsMaze188 = 214;
static const RoomID kMarsMaze189 = 215;
static const RoomID kMarsMaze190 = 216;
static const RoomID kMarsMaze191 = 217;
static const RoomID kMarsMaze192 = 218;
static const RoomID kMarsMaze193 = 219;
static const RoomID kMarsMaze194 = 220;
static const RoomID kMarsMaze195 = 221;
static const RoomID kMarsMaze198 = 222;
static const RoomID kMarsMaze199 = 223;
static const RoomID kMarsMaze200 = 224;
static const RoomID kMarsDeathRoom = 225;

// Hot Spot Activation IDs.

static const HotSpotActivationID kActivationReadyForKiosk = 1;
static const HotSpotActivationID kActivationKioskChoice = 2;
static const HotSpotActivationID kActivationTunnelMapReady = 3;
static const HotSpotActivationID kActivateMarsPodClosed = 4;
static const HotSpotActivationID kActivateMarsPodOpen = 5;
static const HotSpotActivationID kActivateReadyToPressurizeAirlock = 6;
static const HotSpotActivationID kActivateAirlockPressurized = 7;
static const HotSpotActivationID kActivateMaskOnHolder = 8;
static const HotSpotActivationID kActivateMaskOnFiller = 9;
static const HotSpotActivationID kActivateReactorPlatformOut = 10;
static const HotSpotActivationID kActivateReactorPlatformIn = 11;
static const HotSpotActivationID kActivateReactorAskLowerScreen = 12;
static const HotSpotActivationID kActivateReactorReadyForNitrogen = 13;
static const HotSpotActivationID kActivateReactorReadyForCrowBar = 14;
static const HotSpotActivationID kActivateReactorAskOperation = 15;
static const HotSpotActivationID kActivateReactorRanEvaluation = 16;
static const HotSpotActivationID kActivateReactorRanDiagnostics = 17;
static const HotSpotActivationID kActivateReactorAnalyzed = 18;
static const HotSpotActivationID kActivateReactorInstructions = 19;
static const HotSpotActivationID kActivateReactorInGame = 20;
static const HotSpotActivationID kActivateReactorBombSafe = 21;
static const HotSpotActivationID kActivateReactorBombExposed = 22;
static const HotSpotActivationID kActivationRobotHeadClosed = 23;
static const HotSpotActivationID kActivationRobotHeadOpen = 24;

// Hot Spot IDs.

static const HotSpotID kMars11NorthKioskSpotID = 5000;
static const HotSpotID kMars11NorthKioskSightsSpotID = 5001;
static const HotSpotID kMars11NorthKioskColonySpotID = 5002;
static const HotSpotID kMars12NorthKioskSpotID = 5003;
static const HotSpotID kMars12NorthKioskSightsSpotID = 5004;
static const HotSpotID kMars12NorthKioskColonySpotID = 5005;
static const HotSpotID kMars31SouthSpotID = 5006;
static const HotSpotID kMars31SouthOutSpotID = 5007;
static const HotSpotID kMars31SouthCardSpotID = 5008;
static const HotSpotID kMars33NorthSpotID = 5009;
static const HotSpotID kMars33NorthOutSpotID = 5010;
static const HotSpotID kMars33NorthMonitorSpotID = 5011;
static const HotSpotID kMars34NorthCardDropSpotID = 5012;
static const HotSpotID kMars34SouthOpenStorageSpotID = 5013;
static const HotSpotID kMars34SouthCloseStorageSpotID = 5014;
static const HotSpotID kMars34SouthCrowbarSpotID = 5015;
static const HotSpotID kMars35EastPressurizeSpotID = 5016;
static const HotSpotID kMars35EastSpinSpotID = 5017;
static const HotSpotID kMars35WestPressurizeSpotID = 5018;
static const HotSpotID kMars35WestSpinSpotID = 5019;
static const HotSpotID kMars45NorthOpenStorageSpotID = 5020;
static const HotSpotID kMars45NorthCloseStorageSpotID = 5021;
static const HotSpotID kMars45NorthCrowbarSpotID = 5022;
static const HotSpotID kAttackRobotHotSpotID = 5023;
static const HotSpotID kMars49AirMaskSpotID = 5024;
static const HotSpotID kMars49AirMaskFilledSpotID = 5025;
static const HotSpotID kMars49AirFillingDropSpotID = 5026;
static const HotSpotID kMars52MoveLeftSpotID = 5027;
static const HotSpotID kMars52MoveRightSpotID = 5028;
static const HotSpotID kMars52ExtractSpotID = 5029;
static const HotSpotID kMars53RetractSpotID = 5030;
static const HotSpotID kMars54MoveLeftSpotID = 5031;
static const HotSpotID kMars54MoveRightSpotID = 5032;
static const HotSpotID kMars54ExtractSpotID = 5033;
static const HotSpotID kMars55RetractSpotID = 5034;
static const HotSpotID kMars56MoveLeftSpotID = 5035;
static const HotSpotID kMars56MoveRightSpotID = 5036;
static const HotSpotID kMars56ExtractSpotID = 5037;
static const HotSpotID kMars57RetractSpotID = 5038;
static const HotSpotID kMars57LowerScreenSpotID = 5039;
static const HotSpotID kMars57Retract2SpotID = 5040;
static const HotSpotID kMars57DropNitrogenSpotID = 5041;
static const HotSpotID kMars57DropCrowBarSpotID = 5042;
static const HotSpotID kMars57CantOpenPanelSpotID = 5043;
static const HotSpotID kMars57ShieldEvaluationSpotID = 5044;
static const HotSpotID kMars57MeasureOutputSpotID = 5045;
static const HotSpotID kMars57RunDiagnosticsSpotID = 5046;
static const HotSpotID kMars57BackToOperationMenuSpotID = 5047;
static const HotSpotID kMars57AnalyzeObjectSpotID = 5048;
static const HotSpotID kMars57RemoveObjectMenuSpotID = 5049;
static const HotSpotID kMars57CircuitLinkSpotID = 5050;
static const HotSpotID kMars57CancelCircuitLinkSpotID = 5051;
static const HotSpotID kMars57GameInstructionsSpotID = 5052;
static const HotSpotID kMars57UndoMoveSpotID = 5053;
static const HotSpotID kMars57RedMoveSpotID = 5054;
static const HotSpotID kMars57YellowMoveSpotID = 5055;
static const HotSpotID kMars57GreenMoveSpotID = 5056;
static const HotSpotID kMars57BlueMoveSpotID = 5057;
static const HotSpotID kMars57PurpleMoveSpotID = 5058;
static const HotSpotID kMars57LowerScreenSafelySpotID = 5059;
static const HotSpotID kMars57GrabBombSpotID = 5060;
static const HotSpotID kMars58MoveLeftSpotID = 5061;
static const HotSpotID kMars58MoveRightSpotID = 5062;
static const HotSpotID kMars58ExtractSpotID = 5063;
static const HotSpotID kMars59RetractSpotID = 5064;
static const HotSpotID kMars60EastPressurizeSpotID = 5065;
static const HotSpotID kMars60EastSpinSpotID = 5066;
static const HotSpotID kMars60WestPressurizeSpotID = 5067;
static const HotSpotID kMars60WestSpinSpotID = 5068;
static const HotSpotID kRobotShuttleOpenHeadSpotID = 5069;
static const HotSpotID kRobotShuttleMapChipSpotID = 5070;
static const HotSpotID kRobotShuttleOpticalChipSpotID = 5071;
static const HotSpotID kRobotShuttleShieldChipSpotID = 5072;

// Extra sequence IDs.

static const ExtraID kMarsArrivalFromTSA = 0;
static const ExtraID kMars0AWatchShuttleDepart = 1;
static const ExtraID kRobotThrowsPlayer = 2;
static const ExtraID kMarsInfoKioskIntro = 3;
static const ExtraID kMarsColonyInfo = 4;
static const ExtraID kMarsSightsInfo = 5;
static const ExtraID kRobotOnWayToShuttle = 6;
static const ExtraID kMars31SouthZoomInNoCard = 7;
static const ExtraID kMars31SouthViewNoCard = 8;
static const ExtraID kMars31SouthZoomOutNoCard = 9;
static const ExtraID kMars31SouthZoomViewNoCard = 10;
static const ExtraID kMars33SlideShow1 = 11;
static const ExtraID kMars33SlideShow2 = 12;
static const ExtraID kMars33SlideShow3 = 13;
static const ExtraID kMars33SlideShow4 = 14;
static const ExtraID kMars34SpotOpenWithBar = 15;
static const ExtraID kMars34SpotCloseWithBar = 16;
static const ExtraID kMars34SpotOpenNoBar = 17;
static const ExtraID kMars34SpotCloseNoBar = 18;
static const ExtraID kMars34ViewOpenWithBar = 19;
static const ExtraID kMars34ViewOpenNoBar = 20;
static const ExtraID kMars34NorthPodGreeting = 21;
static const ExtraID kMarsTurnOnPod = 22;
static const ExtraID kMarsTakePodToMars45 = 23;
static const ExtraID kMars35WestSpinAirlockToEast = 24;
static const ExtraID kMars35EastSpinAirlockToWest = 25;
static const ExtraID kMars45SpotOpenWithBar = 26;
static const ExtraID kMars45SpotCloseWithBar = 27;
static const ExtraID kMars45SpotOpenNoBar = 28;
static const ExtraID kMars45SpotCloseNoBar = 29;
static const ExtraID kMars45ViewOpenWithBar = 30;
static const ExtraID kMars45ViewOpenNoBar = 31;
static const ExtraID kMars48RobotApproaches = 32;
static const ExtraID kMars48RobotKillsPlayer = 33;
static const ExtraID kMars48RobotLoops = 34;
static const ExtraID kMars48RobotView = 35;
static const ExtraID kMars48RobotDefends = 36;
static const ExtraID kMars49SouthViewMaskFilling = 37;
static const ExtraID kMars52SpinLeft = 38;
static const ExtraID kMars52SpinRight = 39;
static const ExtraID kMars52Extend = 40;
static const ExtraID kMars53Retract = 41;
static const ExtraID kMars54SpinLeft = 42;
static const ExtraID kMars54SpinRight = 43;
static const ExtraID kMars54Extend = 44;
static const ExtraID kMars55Retract = 45;
static const ExtraID kMars56SpinLeft = 46;
static const ExtraID kMars56SpinRight = 47;
static const ExtraID kMars56ExtendWithBomb = 48;
static const ExtraID kMars56ExtendNoBomb = 49;
static const ExtraID kMars57RetractWithBomb = 50;
static const ExtraID kMars57RetractNoBomb = 51;
static const ExtraID kMars57LowerScreenClosed = 52;
static const ExtraID kMars57CantOpenPanel = 53;
static const ExtraID kMars57FreezeLock = 54;
static const ExtraID kMars57BreakLock = 55;
static const ExtraID kMars57LockFrozenView = 56;
static const ExtraID kMars57ThawLock = 57;
static const ExtraID kMars57OpenPanel = 58;
static const ExtraID kMars57OpenPanelChoices = 59;
static const ExtraID kMars57ShieldEvaluation = 60;
static const ExtraID kMars57MeasureOutput = 61;
static const ExtraID kMars57ShieldOkayLoop = 62;
static const ExtraID kMars57RunDiagnostics = 63;
static const ExtraID kMars57BombExplodes = 64;
static const ExtraID kMars57BombAnalysis = 65;
static const ExtraID kMars57DontLink = 66;
static const ExtraID kMars57CircuitLink = 67;
static const ExtraID kMars57GameLevel1 = 68;
static const ExtraID kMars57GameLevel2 = 69;
static const ExtraID kMars57GameLevel3 = 70;
static const ExtraID kMars57BombExplodesInGame = 71;
static const ExtraID kMars57GameSolved = 72;
static const ExtraID kMars57ExposeBomb = 73;
static const ExtraID kMars57BackToNormal = 74;
static const ExtraID kMars57ViewOpenNoBomb = 75;
static const ExtraID kMars58SpinLeft = 76;
static const ExtraID kMars58SpinRight = 77;
static const ExtraID kMars58Extend = 78;
static const ExtraID kMars59Retract = 79;
static const ExtraID kMars60WestSpinAirlockToEast = 80;
static const ExtraID kMars60EastSpinAirlockToWest = 81;
static const ExtraID kMarsRobotHeadOpen = 82;
static const ExtraID kMarsRobotHeadClose = 83;
static const ExtraID kMarsRobotHead000 = 84;
static const ExtraID kMarsRobotHead001 = 85;
static const ExtraID kMarsRobotHead010 = 86;
static const ExtraID kMarsRobotHead011 = 87;
static const ExtraID kMarsRobotHead100 = 88;
static const ExtraID kMarsRobotHead101 = 89;
static const ExtraID kMarsRobotHead110 = 90;
static const ExtraID kMarsRobotHead111 = 91;
static const ExtraID kMarsMaze007RobotApproach = 92;
static const ExtraID kMarsMaze007RobotLoop = 93;
static const ExtraID kMarsMaze007RobotDeath = 94;
static const ExtraID kMarsMaze015SouthRobotApproach = 95;
static const ExtraID kMarsMaze015SouthRobotLoop = 96;
static const ExtraID kMarsMaze015SouthRobotDeath = 97;
static const ExtraID kMarsMaze101EastRobotApproach = 98;
static const ExtraID kMarsMaze101EastRobotLoop = 99;
static const ExtraID kMarsMaze101EastRobotDeath = 100;
static const ExtraID kMarsMaze104WestLoop = 101;
static const ExtraID kMarsMaze104WestDeath = 102;
static const ExtraID kMarsMaze133SouthApproach = 103;
static const ExtraID kMarsMaze133SouthLoop = 104;
static const ExtraID kMarsMaze133SouthDeath = 105;
static const ExtraID kMarsMaze136NorthApproach = 106;
static const ExtraID kMarsMaze136NorthLoop = 107;
static const ExtraID kMarsMaze136NorthDeath = 108;
static const ExtraID kMarsMaze184WestLoop = 109;
static const ExtraID kMarsMaze184WestDeath = 110;
static const ExtraID kMars200DeathInBucket = 111;

static const ResIDType kReactorUndoHilitePICTID = 900;

static const int16 kMars52Compass = 90;
static const int16 kMars54Compass = 180;
static const int16 kMars56Compass = 270;
static const int16 kMars58Compass = 0;

} // End of namespace Pegasus

#endif
