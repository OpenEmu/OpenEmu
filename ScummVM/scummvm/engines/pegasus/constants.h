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

#ifndef PEGASUS_CONSTANTS_H
#define PEGASUS_CONSTANTS_H

#include "common/endian.h"
#include "common/rect.h"

#include "pegasus/types.h"

namespace Pegasus {

// TODO: Organize these

static const GameID kGameIDNothing = -1;

static const ActorID kNoActorID = kGameIDNothing;
static const ActorID kPlayerID = 0;
static const ItemID kNoItemID = kGameIDNothing;
static const RoomID kNoRoomID = kGameIDNothing;
static const ExtraID kNoExtraID = 0xFFFFFFFF;
static const NeighborhoodID kNoNeighborhoodID = kGameIDNothing;
static const AlternateID kNoAlternateID = 0;
static const GameMenuCommand kMenuCmdNoCommand = 0;

static const HotSpotActivationID kActivateHotSpotAlways = 0;
static const HotSpotActivationID kActivateHotSpotNever = -1;

static const ItemState kNoItemState = -1;

static const DirectionConstant kNoDirection = 0xFF;
static const DirectionConstant kNorth = 0;
static const DirectionConstant kSouth = 1;
static const DirectionConstant kEast = 2;
static const DirectionConstant kWest = 3;

static const TurnDirection kNoTurn = 0xFF;
static const TurnDirection kTurnLeft = 0;
static const TurnDirection kTurnRight = 1;
static const TurnDirection kTurnUp = 2;
static const TurnDirection kTurnDown = 3;
static const TurnDirection kMaxTurns = 4;

static const GameMode kNoMode = -1;
static const GameMode kModeNavigation = 0;
static const GameMode kLastGameShellMode = kModeNavigation;

static const CanMoveForwardReason kCanMoveForward = 0;
static const CanMoveForwardReason kCantMoveBlocked = kCanMoveForward + 1;
static const CanMoveForwardReason kCantMoveDoorClosed = kCantMoveBlocked + 1;
static const CanMoveForwardReason kCantMoveDoorLocked = kCantMoveDoorClosed + 1;
static const CanMoveForwardReason kCantMoveLastReason = kCantMoveDoorLocked;

static const CanTurnReason kCanTurn = 0;
static const CanTurnReason kCantTurnNoTurn = kCanTurn + 1;
static const CanTurnReason kCantTurnLastReason = kCantTurnNoTurn;

static const CanOpenDoorReason kCanOpenDoor = 0;
static const CanOpenDoorReason kCantOpenNoDoor = kCanOpenDoor + 1;
static const CanOpenDoorReason kCantOpenLocked = kCantOpenNoDoor + 1;
static const CanOpenDoorReason kCantOpenAlreadyOpen = kCantOpenLocked + 1;
static const CanOpenDoorReason kCantOpenLastReason = kCantOpenAlreadyOpen;

static const DisplayElementID kNoDisplayElement = -1;
static const DisplayElementID kHighestReservedElementID = -2;

static const DisplayElementID kCursorID = kHighestReservedElementID;
static const DisplayElementID kLoadScreenID = kCursorID - 1;

static const DisplayOrder kMinAvailableOrder = 0;
static const DisplayOrder kMaxAvailableOrder = 999998;
static const DisplayOrder kLoadScreenOrder = 900000;
static const DisplayOrder kCursorOrder = 1000000;

static const HotSpotID kNoHotSpotID = -1;
static const HotSpotFlags kNoHotSpotFlags = 0;
static const HotSpotFlags kAllHotSpotFlags = ~kNoHotSpotFlags;

static const NotificationFlags kNoNotificationFlags = 0;

static const DisplayElementID kCurrentDragSpriteID = 1000;

static const TimeScale kDefaultTimeScale = 600;

// Ticks per second.

static const TimeScale kOneTickPerSecond = 1;
static const TimeScale kTwoTicksPerSecond = 2;
static const TimeScale kFifteenTicksPerSecond = 15;
static const TimeScale kThirtyTicksPerSecond = 30;
static const TimeScale kSixtyTicksPerSecond = 60;
static const TimeScale kMovieTicksPerSecond = 600;

// These times are in seconds.

static const TimeValue kOneSecond = 1;
static const TimeValue kTwoSeconds = 2;
static const TimeValue kThreeSeconds = 3;
static const TimeValue kFourSeconds = 4;
static const TimeValue kFiveSeconds = 5;
static const TimeValue kSixSeconds = 6;
static const TimeValue kSevenSeconds = 7;
static const TimeValue kEightSeconds = 8;
static const TimeValue kNineSeconds = 9;
static const TimeValue kTenSeconds = 10;
static const TimeValue kElevenSeconds = 11;
static const TimeValue kTwelveSeconds = 12;
static const TimeValue kThirteenSeconds = 13;
static const TimeValue kFourteenSeconds = 14;
static const TimeValue kFifteenSeconds = 15;
static const TimeValue kSixteenSeconds = 16;
static const TimeValue kSeventeenSeconds = 17;
static const TimeValue kEighteenSeconds = 18;
static const TimeValue kNineteenSeconds = 19;
static const TimeValue kTwentySeconds = 20;
static const TimeValue kThirtySeconds = 30;
static const TimeValue kFortySeconds = 40;
static const TimeValue kFiftySeconds = 50;
static const TimeValue kSixtySeconds = 60;
static const TimeValue kOneMinute = 60;
static const TimeValue kTwoMinutes = kOneMinute * 2;
static const TimeValue kThreeMinutes = kOneMinute * 3;
static const TimeValue kFourMinutes = kOneMinute * 4;
static const TimeValue kFiveMinutes = kOneMinute * 5;
static const TimeValue kSixMinutes = kOneMinute * 6;
static const TimeValue kSevenMinutes = kOneMinute * 7;
static const TimeValue kEightMinutes = kOneMinute * 8;
static const TimeValue kNineMinutes = kOneMinute * 9;
static const TimeValue kTenMinutes = kOneMinute * 10;
static const TimeValue kElevenMinutes = kOneMinute * 11;
static const TimeValue kTwelveMinutes = kOneMinute * 12;
static const TimeValue kThirteenMinutes = kOneMinute * 13;
static const TimeValue kFourteenMinutes = kOneMinute * 14;
static const TimeValue kFifteenMinutes = kOneMinute * 15;
static const TimeValue kSixteenMinutes = kOneMinute * 16;
static const TimeValue kSeventeenMinutes = kOneMinute * 17;
static const TimeValue kEighteenMinutes = kOneMinute * 18;
static const TimeValue kNineteenMinutes = kOneMinute * 19;
static const TimeValue kTwentyMinutes = kOneMinute * 20;
static const TimeValue kThirtyMinutes = kOneMinute * 30;
static const TimeValue kFortyMinutes = kOneMinute * 40;
static const TimeValue kFiftyMinutes = kOneMinute * 50;
static const TimeValue kOneHour = kOneMinute * 60;
static const TimeValue kTwoHours = kOneHour * 2;

// Common times.

static const TimeValue kHalfSecondPerTwoTicks = kTwoTicksPerSecond / 2;
static const TimeValue kHalfSecondPerThirtyTicks = kThirtyTicksPerSecond / 2;
static const TimeValue kHalfSecondPerSixtyTicks = kSixtyTicksPerSecond / 2;

static const TimeValue kOneSecondPerTwoTicks = kTwoTicksPerSecond;
static const TimeValue kOneSecondPerThirtyTicks = kThirtyTicksPerSecond;
static const TimeValue kOneSecondPerSixtyTicks = kSixtyTicksPerSecond;

static const TimeValue kOneMinutePerFifteenTicks = kOneMinute * kFifteenTicksPerSecond;
static const TimeValue kFiveMinutesPerFifteenTicks = kFiveMinutes * kFifteenTicksPerSecond;
static const TimeValue kTenMinutesPerFifteenTicks = kTenMinutes * kFifteenTicksPerSecond;

static const TimeValue kOneMinutePerThirtyTicks = kOneMinute * kThirtyTicksPerSecond;
static const TimeValue kFiveMinutesPerThirtyTicks = kFiveMinutes * kThirtyTicksPerSecond;
static const TimeValue kTenMinutesPerThirtyTicks = kTenMinutes * kThirtyTicksPerSecond;

static const TimeValue kOneMinutePerSixtyTicks = kOneMinute * kSixtyTicksPerSecond;
static const TimeValue kFiveMinutesPerSixtyTicks = kFiveMinutes * kSixtyTicksPerSecond;
static const TimeValue kTenMinutesPerSixtyTicks = kTenMinutes * kSixtyTicksPerSecond;

// Time in seconds you can hang around Caldoria without going to work...
static const TimeValue kLateWarning2TimeLimit = kFiveMinutes;
static const TimeValue kLateWarning3TimeLimit = kTenMinutes;

static const TimeValue kSinclairShootsTimeLimit = kThreeMinutes;
static const TimeValue kCardBombCountDownTime = kTwelveSeconds;

static const TimeValue kOxyMaskFullTime = kThirtyMinutes;

static const TimeValue kTSAUncreatedTimeLimit = kFiveMinutes;
static const TimeValue kRipTimeLimit = kTenMinutesPerFifteenTicks;
static const TimeScale kRipTimeScale = kFifteenTicksPerSecond;

static const TimeValue kIntroTimeOut = kThirtySeconds;

static const TimeValue kMarsRobotPatienceLimit = kFifteenSeconds;
static const TimeValue kLockFreezeTimeLmit = kFifteenSeconds;
static const TimeValue kSpaceChaseTimeLimit = kTenMinutes;
static const TimeValue kVacuumSurvivalTimeLimit = kThirtySeconds;
static const TimeValue kColorMatchingTimeLimit = kFourMinutes;
static const TimeScale kJunkTimeScale = kFifteenTicksPerSecond;
static const TimeValue kJunkDropBaseTime = kFiveSeconds;
static const TimeValue kJunkDropSlopTime = kThreeSeconds;
static const TimeValue kJunkTravelTime = kTenSeconds * kJunkTimeScale;
static const TimeValue kCollisionReboundTime = kOneSecond * kJunkTimeScale;
static const TimeValue kWeaponReboundTime = kTwoSeconds * kJunkTimeScale;

static const TimeValue kGawkAtRobotTime = kTenSeconds;
static const TimeValue kGawkAtRobotTime2 = kThirteenSeconds;
static const TimeValue kPlasmaImpactTime = kTwoSeconds;

static const TimeValue kNoradAirMaskTimeLimit = kOneMinute + kFifteenSeconds;

static const NotificationID kNeighborhoodNotificationID = 1;
static const NotificationID kLastNeighborhoodNotificationID = kNeighborhoodNotificationID;

static const NotificationFlags kNeighborhoodMovieCompletedFlag = 1;
static const NotificationFlags kMoveForwardCompletedFlag = kNeighborhoodMovieCompletedFlag << 1;
static const NotificationFlags kStrideCompletedFlag = kMoveForwardCompletedFlag << 1;
static const NotificationFlags kTurnCompletedFlag = kStrideCompletedFlag << 1;
static const NotificationFlags kSpotCompletedFlag = kTurnCompletedFlag << 1;
static const NotificationFlags kDoorOpenCompletedFlag = kSpotCompletedFlag << 1;
static const NotificationFlags kExtraCompletedFlag = kDoorOpenCompletedFlag << 1;
static const NotificationFlags kSpotSoundCompletedFlag = kExtraCompletedFlag << 1;
static const NotificationFlags kDelayCompletedFlag = kSpotSoundCompletedFlag << 1;
static const NotificationFlags kActionRequestCompletedFlag = kDelayCompletedFlag << 1;
static const NotificationFlags kDeathExtraCompletedFlag = kActionRequestCompletedFlag << 1;
static const NotificationFlags kLastNeighborhoodNotificationFlag = kDeathExtraCompletedFlag;

static const NotificationFlags kNeighborhoodFlags =	kNeighborhoodMovieCompletedFlag |
												kMoveForwardCompletedFlag |
												kStrideCompletedFlag |
												kTurnCompletedFlag |
												kSpotCompletedFlag |
												kDoorOpenCompletedFlag |
												kExtraCompletedFlag |
												kSpotSoundCompletedFlag |
												kDelayCompletedFlag |
												kActionRequestCompletedFlag |
												kDeathExtraCompletedFlag;

static const uint32 kPegasusPrimeCreator = MKTAG('J', 'P', 'P', 'P');
static const uint32 kPegasusPrimeContinueType = MKTAG('P', 'P', 'C', 'T');

static const uint32 kPegasusPrimeDisk1GameType = MKTAG('P', 'P', 'G', '1');
static const uint32 kPegasusPrimeDisk2GameType = MKTAG('P', 'P', 'G', '2');
static const uint32 kPegasusPrimeDisk3GameType = MKTAG('P', 'P', 'G', '3');
static const uint32 kPegasusPrimeDisk4GameType = MKTAG('P', 'P', 'G', '4');

// We only support one of the save versions; the rest are from betas
// and we are not supporting them.
static const uint32 kPegasusPrimeVersion = 0x00009019;

static const char kNormalSave = 0;
static const char kContinueSave = 1;

// Display IDs.

static const DisplayElementID kNavMovieID = 1;
static const DisplayElementID kTurnPushID = 2;

static const DisplayElementID kMaxGameShellDisplayID = kTurnPushID;

// Display ordering.

static const DisplayOrder kNavLayer = 10000;
static const DisplayOrder kNavMovieOrder = kNavLayer;
static const DisplayOrder kTurnPushOrder = kNavMovieOrder + 1;

/////////////////////////////////////////////
//
// Display IDs.

static const DisplayElementID kScreenDimmerID = kMaxGameShellDisplayID + 1;
static const DisplayElementID kInterface1ID = kScreenDimmerID + 1;
static const DisplayElementID kInterface2ID = kInterface1ID + 1;
static const DisplayElementID kInterface3ID = kInterface2ID + 1;
static const DisplayElementID kInterface4ID = kInterface3ID + 1;
static const DisplayElementID kDateID = kInterface4ID + 1;
static const DisplayElementID kCompassID = kDateID + 1;
static const DisplayElementID kInventoryPushID = kCompassID + 1;
static const DisplayElementID kInventoryLidID = kInventoryPushID + 1;
static const DisplayElementID kBiochipPushID = kInventoryLidID + 1;
static const DisplayElementID kBiochipLidID = kBiochipPushID + 1;
static const DisplayElementID kEnergyBarID = kBiochipLidID + 1;
static const DisplayElementID kWarningLightID = kEnergyBarID + 1;
static const DisplayElementID kAILeftAreaID = kWarningLightID + 1;
static const DisplayElementID kAIMiddleAreaID = kAILeftAreaID + 1;
static const DisplayElementID kAIRightAreaID = kAIMiddleAreaID + 1;
static const DisplayElementID kAIMovieID = kAIRightAreaID + 1;
static const DisplayElementID kInventoryDropHighlightID = kAIMovieID + 1;
static const DisplayElementID kBiochipDropHighlightID = kInventoryDropHighlightID + 1;

static const DisplayElementID kDraggingSpriteID = 1000;

static const DisplayElementID kCroppedMovieID = 2000;

static const DisplayElementID kNeighborhoodDisplayID = 3000;

static const DisplayElementID kItemPictureBaseID = 5000;

static const CoordType kNavAreaLeft = 64;
static const CoordType kNavAreaTop = 64;

static const CoordType kBackground1Left = 0;
static const CoordType kBackground1Top = 64;

static const CoordType kBackground2Left = 0;
static const CoordType kBackground2Top = 0;

static const CoordType kBackground3Left = 576;
static const CoordType kBackground3Top = 64;

static const CoordType kBackground4Left = 0;
static const CoordType kBackground4Top = 320;

static const CoordType kOverviewControllerLeft = 540;
static const CoordType kOverviewControllerTop = 348;

static const CoordType kSwapLeft = 194;
static const CoordType kSwapTop = 116;

static const CoordType kSwapHiliteLeft = 200;
static const CoordType kSwapHiliteTop = 206;

static const CoordType kDateLeft = 136;
static const CoordType kDateTop = 44;

static const CoordType kCompassLeft = 222;
static const CoordType kCompassTop = 42;
static const CoordType kCompassWidth = 92;

static const CoordType kInventoryPushLeft = 74;
static const CoordType kInventoryPushTop = 92;

static const CoordType kInventoryLidLeft = 74;
static const CoordType kInventoryLidTop = 316;

static const CoordType kBiochipPushLeft = 362;
static const CoordType kBiochipPushTop = 192;

static const CoordType kBiochipLidLeft = 362;
static const CoordType kBiochipLidTop = 316;

static const CoordType kInventoryDropLeft = 0;
static const CoordType kInventoryDropTop = 320;
static const CoordType kInventoryDropRight = 232;
static const CoordType kInventoryDropBottom = 480;

static const CoordType kBiochipDropLeft = 302;
static const CoordType kBiochipDropTop = 320;
static const CoordType kBiochipDropRight = 640;
static const CoordType kBiochipDropBottom = 480;

static const CoordType kFinalMessageLeft = kInventoryPushLeft + 1;
static const CoordType kFinalMessageTop = kInventoryPushTop + 24;

/////////////////////////////////////////////
//
// Notifications.

static const NotificationID kJMPDCShellNotificationID = kLastNeighborhoodNotificationID + 1;
static const NotificationID kInterfaceNotificationID = kJMPDCShellNotificationID + 1;
static const NotificationID kAINotificationID = kInterfaceNotificationID + 1;
static const NotificationID kNoradNotificationID = kAINotificationID + 1;
static const NotificationID kNoradECRNotificationID = kNoradNotificationID + 1;
static const NotificationID kNoradFillingStationNotificationID = kNoradECRNotificationID + 1;
static const NotificationID kNoradPressureNotificationID = kNoradFillingStationNotificationID + 1;
static const NotificationID kNoradUtilityNotificationID = kNoradPressureNotificationID + 1;
static const NotificationID kNoradElevatorNotificationID = kNoradUtilityNotificationID + 1;
static const NotificationID kNoradSubPlatformNotificationID = kNoradElevatorNotificationID + 1;
static const NotificationID kSubControlNotificationID = kNoradSubPlatformNotificationID + 1;
static const NotificationID kNoradGreenBallNotificationID = kSubControlNotificationID + 1;
static const NotificationID kNoradGlobeNotificationID = kNoradGreenBallNotificationID + 1;
static const NotificationID kCaldoriaVidPhoneNotificationID = kNoradGlobeNotificationID + 1;
static const NotificationID kCaldoriaMessagesNotificationID = kCaldoriaVidPhoneNotificationID + 1;
static const NotificationID kCaldoriaBombTimerNotificationID = kCaldoriaMessagesNotificationID + 1;

// Sent to the shell by fShellNotification.
static const NotificationFlags kGameStartingFlag = 1;
static const NotificationFlags kNeedNewJumpFlag = kGameStartingFlag << 1;
static const NotificationFlags kPlayerDiedFlag = kNeedNewJumpFlag << 1;

static const NotificationFlags kJMPShellNotificationFlags = kGameStartingFlag |
														kNeedNewJumpFlag |
														kPlayerDiedFlag;

// Sent to the interface.
static const NotificationFlags kInventoryLidOpenFlag = 1;
static const NotificationFlags kInventoryLidClosedFlag = kInventoryLidOpenFlag << 1;
static const NotificationFlags kInventoryDrawerUpFlag = kInventoryLidClosedFlag << 1;
static const NotificationFlags kInventoryDrawerDownFlag = kInventoryDrawerUpFlag << 1;
static const NotificationFlags kBiochipLidOpenFlag = kInventoryDrawerDownFlag << 1;
static const NotificationFlags kBiochipLidClosedFlag = kBiochipLidOpenFlag << 1;
static const NotificationFlags kBiochipDrawerUpFlag = kBiochipLidClosedFlag << 1;
static const NotificationFlags kBiochipDrawerDownFlag = kBiochipDrawerUpFlag << 1;

static const NotificationFlags kInterfaceNotificationFlags =	kInventoryLidOpenFlag |
														kInventoryLidClosedFlag |
														kInventoryDrawerUpFlag |
														kInventoryDrawerDownFlag |
														kBiochipLidOpenFlag |
														kBiochipLidClosedFlag |
														kBiochipDrawerUpFlag |
														kBiochipDrawerDownFlag;

// Hot spots.

// Neighborhood hot spots.

static const HotSpotID kFirstNeighborhoodSpotID = 5000;

// kShellSpotFlag is a flag which marks all hot spots which belong to the shell, like
// the current item and current biochip spots.
static const HotSpotFlags kShellSpotFlag = 1;
// kNeighborhoodSpotFlag is a flag which marks all hot spots which belong to a
// neighborhood, like buttons on walls and so on.
static const HotSpotFlags kNeighborhoodSpotFlag = kShellSpotFlag << 1;
// kZoomInSpotFlag is a flag which marks all hot spots which indicate a zoom.
static const HotSpotFlags kZoomInSpotFlag = kNeighborhoodSpotFlag << 1;
// kZoomOutSpotFlag is a flag which marks all hot spots which indicate a zoom.
static const HotSpotFlags kZoomOutSpotFlag = kZoomInSpotFlag << 1;

static const HotSpotFlags kClickSpotFlag = kZoomOutSpotFlag << 1;
static const HotSpotFlags kPlayExtraSpotFlag = kClickSpotFlag << 1;
static const HotSpotFlags kPickUpItemSpotFlag = kPlayExtraSpotFlag << 1;
static const HotSpotFlags kDropItemSpotFlag = kPickUpItemSpotFlag << 1;
static const HotSpotFlags kOpenDoorSpotFlag = kDropItemSpotFlag << 1;

static const HotSpotFlags kZoomSpotFlags = kZoomInSpotFlag | kZoomOutSpotFlag;

static const HotSpotFlags kHighestGameShellSpotFlag = kOpenDoorSpotFlag;

/////////////////////////////////////////////
//
// Hot spots.

// Shell hot spots.
// The shell reserves all hot spot IDs from 0 to 999

static const HotSpotID kCurrentItemSpotID = 0;
static const HotSpotID kCurrentBiochipSpotID = kCurrentItemSpotID + 1;

static const HotSpotID kInventoryDropSpotID = kCurrentBiochipSpotID + 1;
static const HotSpotID kBiochipDropSpotID = kInventoryDropSpotID + 1;

static const HotSpotID kInfoReturnSpotID = kBiochipDropSpotID + 1;

static const HotSpotID kAIHint1SpotID = kInfoReturnSpotID + 1;
static const HotSpotID kAIHint2SpotID = kAIHint1SpotID + 1;
static const HotSpotID kAIHint3SpotID = kAIHint2SpotID + 1;
static const HotSpotID kAISolveSpotID = kAIHint3SpotID + 1;
static const HotSpotID kAIBriefingSpotID = kAISolveSpotID + 1;
static const HotSpotID kAIScanSpotID = kAIBriefingSpotID + 1;

static const HotSpotID kPegasusRecallSpotID = kAIScanSpotID + 1;

static const HotSpotID kAriesSpotID = kPegasusRecallSpotID + 1;
static const HotSpotID kMercurySpotID = kAriesSpotID + 1;
static const HotSpotID kPoseidonSpotID = kMercurySpotID + 1;

static const HotSpotID kAirMaskToggleSpotID = kPoseidonSpotID + 1;

static const HotSpotID kShuttleEnergySpotID = kAirMaskToggleSpotID + 1;
static const HotSpotID kShuttleGravitonSpotID = kShuttleEnergySpotID + 1;
static const HotSpotID kShuttleTractorSpotID = kShuttleGravitonSpotID + 1;
static const HotSpotID kShuttleViewSpotID = kShuttleTractorSpotID + 1;
static const HotSpotID kShuttleTransportSpotID = kShuttleViewSpotID + 1;

// Most of these are obsolete:

// kInventoryDropSpotFlag is a flag which marks hot spots which are valid drop spots
// for inventory items.
// static const HotSpotFlags kInventoryDropSpotFlag = kHighestGameShellSpotFlag << 1;

// kBiochipDropSpotFlag is a flag which marks hot spots which are valid drop spots
// for biochips.
// static const HotSpotFlags kBiochipDropSpotFlag = kInventoryDropSpotFlag << 1;

// kInventorySpotFlag is a flag which marks hot spots which indicate inventory items
// in the environment.
// static const HotSpotFlags kInventorySpotFlag = kBiochipDropSpotFlag << 1;

// kBiochipSpotFlag is a flag which marks hot spots which indicate biochips
// in the environment.
static const HotSpotFlags kPickUpBiochipSpotFlag = kHighestGameShellSpotFlag << 1;
static const HotSpotFlags kDropBiochipSpotFlag = kPickUpBiochipSpotFlag << 1;

static const HotSpotFlags kInfoReturnSpotFlag = kDropBiochipSpotFlag << 1;

// Biochip and inventory hot spot flags...

static const HotSpotFlags kAIBiochipSpotFlag = kInfoReturnSpotFlag << 1;
static const HotSpotFlags kPegasusBiochipSpotFlag = kAIBiochipSpotFlag << 1;
static const HotSpotFlags kOpticalBiochipSpotFlag = kPegasusBiochipSpotFlag << 1;
static const HotSpotFlags kAirMaskSpotFlag = kOpticalBiochipSpotFlag << 1;

static const HotSpotFlags kJMPClickingSpotFlags = kClickSpotFlag |
											kPlayExtraSpotFlag |
											kOpenDoorSpotFlag |
											kInfoReturnSpotFlag |
											kAIBiochipSpotFlag |
											kPegasusBiochipSpotFlag |
											kOpticalBiochipSpotFlag |
											kAirMaskSpotFlag;

static const int32 kMainMenuID = 1;
static const int32 kPauseMenuID = 2;
static const int32 kCreditsMenuID = 3;
static const int32 kDeathMenuID = 4;

/////////////////////////////////////////////
//
// Menu commands.

static const GameMenuCommand kMenuCmdOverview = kMenuCmdNoCommand + 1;
static const GameMenuCommand kMenuCmdStartAdventure = kMenuCmdOverview + 1;
static const GameMenuCommand kMenuCmdStartWalkthrough = kMenuCmdStartAdventure + 1;
static const GameMenuCommand kMenuCmdRestore = kMenuCmdStartWalkthrough + 1;
static const GameMenuCommand kMenuCmdCredits = kMenuCmdRestore + 1;
static const GameMenuCommand kMenuCmdQuit = kMenuCmdCredits + 1;

static const GameMenuCommand kMenuCmdDeathContinue = kMenuCmdQuit + 1;

static const GameMenuCommand kMenuCmdDeathQuitDemo = kMenuCmdDeathContinue + 1;
static const GameMenuCommand kMenuCmdDeathMainMenuDemo = kMenuCmdDeathQuitDemo + 1;

static const GameMenuCommand kMenuCmdDeathRestore = kMenuCmdDeathMainMenuDemo + 1;
static const GameMenuCommand kMenuCmdDeathMainMenu = kMenuCmdDeathRestore + 1;

static const GameMenuCommand kMenuCmdPauseSave = kMenuCmdDeathMainMenu + 1;
static const GameMenuCommand kMenuCmdPauseContinue = kMenuCmdPauseSave + 1;
static const GameMenuCommand kMenuCmdPauseRestore = kMenuCmdPauseContinue + 1;
static const GameMenuCommand kMenuCmdPauseQuit = kMenuCmdPauseRestore + 1;

static const GameMenuCommand kMenuCmdCreditsMainMenu = kMenuCmdPauseQuit + 1;

static const GameMenuCommand kMenuCmdCancelRestart = kMenuCmdCreditsMainMenu + 1;
static const GameMenuCommand kMenuCmdEjectRestart = kMenuCmdCancelRestart + 1;

static const TimeValue kMenuButtonHiliteTime = 20;
static const TimeScale kMenuButtonHiliteScale = kSixtyTicksPerSecond;

// PICT resources:

// Warning light PICTs:

static const ResIDType kLightOffID = 128;
static const ResIDType kLightYellowID = 129;
static const ResIDType kLightOrangeID = 130;
static const ResIDType kLightRedID = 131;

// Date PICTs:

static const ResIDType kDatePrehistoricID = 138;
static const ResIDType kDate2112ID = 139;
static const ResIDType kDate2185ID = 140;
static const ResIDType kDate2310ID = 141;
static const ResIDType kDate2318ID = 142;

/////////////////////////////////////////////
//
// Display Order

static const DisplayOrder kCroppedMovieLayer = 11000;

static const DisplayOrder kMonitorLayer = 12000;

static const DisplayOrder kDragSpriteLayer = 15000;
static const DisplayOrder kDragSpriteOrder = kDragSpriteLayer;

static const DisplayOrder kInterfaceLayer = 20000;
static const DisplayOrder kBackground1Order = kInterfaceLayer;
static const DisplayOrder kBackground2Order = kBackground1Order + 1;
static const DisplayOrder kBackground3Order = kBackground2Order + 1;
static const DisplayOrder kBackground4Order = kBackground3Order + 1;
static const DisplayOrder kDateOrder = kBackground4Order + 1;
static const DisplayOrder kCompassOrder = kDateOrder + 1;
static const DisplayOrder kEnergyBarOrder = kCompassOrder + 1;
static const DisplayOrder kEnergyLightOrder = kEnergyBarOrder + 1;

static const DisplayOrder kAILayer = 22000;
static const DisplayOrder kAILeftAreaOrder = kAILayer;
static const DisplayOrder kAIMiddleAreaOrder = kAILeftAreaOrder + 1;
static const DisplayOrder kAIRightAreaOrder = kAIMiddleAreaOrder + 1;
static const DisplayOrder kAIMovieOrder = kAIRightAreaOrder + 1;

static const DisplayOrder kHilitesLayer = 23000;
static const DisplayOrder kInventoryHiliteOrder = kHilitesLayer;
static const DisplayOrder kBiochipHiliteOrder = kInventoryHiliteOrder + 1;

static const DisplayOrder kPanelsLayer = 25000;
static const DisplayOrder kInventoryPushOrder = kPanelsLayer;
static const DisplayOrder kInventoryLidOrder = kInventoryPushOrder + 1;
static const DisplayOrder kBiochipPushOrder = kInventoryLidOrder + 1;
static const DisplayOrder kBiochipLidOrder = kBiochipPushOrder + 1;
static const DisplayOrder kFinalMessageOrder = kBiochipLidOrder + 1;

static const DisplayOrder kInfoLayer = 26000;
static const DisplayOrder kInfoBackgroundOrder = kInfoLayer;
static const DisplayOrder kInfoSpinOrder = kInfoBackgroundOrder + 1;

static const DisplayOrder kScreenDimmerOrder = 30000;

static const DisplayOrder kPauseScreenLayer = 31000;
static const DisplayOrder kPauseMenuOrder = kPauseScreenLayer;
static const DisplayOrder kSaveGameOrder = kPauseMenuOrder + 1;
static const DisplayOrder kContinueOrder = kSaveGameOrder + 1;
static const DisplayOrder kRestoreOrder = kContinueOrder + 1;
static const DisplayOrder kSoundFXOrder = kRestoreOrder + 1;
static const DisplayOrder kAmbienceOrder = kSoundFXOrder + 1;
static const DisplayOrder kWalkthruOrder = kAmbienceOrder + 1;
static const DisplayOrder kQuitToMainMenuOrder = kWalkthruOrder + 1;
static const DisplayOrder kPauseLargeHiliteOrder = kQuitToMainMenuOrder + 1;
static const DisplayOrder kPauseSmallHiliteOrder = kPauseLargeHiliteOrder + 1;

/////////////////////////////////////////////
//
// Death reasons.
enum {
	// Caldoria
	kDeathUncreatedInCaldoria = 1,
	kDeathCardBomb,
	kDeathShotBySinclair,
	kDeathSinclairShotDelegate,
	kDeathNuclearExplosion,

	// TSA
	kDeathUncreatedInTSA,
	kDeathShotByTSARobots,

	// Prehistoric
	kDeathFallOffCliff,
	kDeathEatenByDinosaur,
	kDeathStranded,

	// Norad
	kDeathGassedInNorad,
	kDeathArrestedInNorad,
	kDeathWokeUpNorad,
	kDeathSubDestroyed, // Unused
	kDeathRobotThroughNoradDoor,
	kDeathRobotSubControlRoom,

	// Mars
	kDeathWrongShuttleLock,
	kDeathArrestedInMars,
	kDeathRunOverByPod,
	kDeathDidntGetOutOfWay,
	kDeathReactorBurn,
	kDeathDidntFindMarsBomb,
	kDeathDidntDisarmMarsBomb,
	kDeathNoMaskInMaze,
	kDeathNoAirInMaze,
	kDeathGroundByMazebot,
	kDeathMissedOreBucket,
	kDeathDidntLeaveBucket,
	kDeathRanIntoCanyonWall, // Unused
	kDeathRanIntoSpaceJunk,

	// WSC
	kDeathDidntStopPoison,
	kDeathArrestedInWSC,
	kDeathHitByPlasma,
	kDeathShotOnCatwalk,

	// Winning
	kPlayerWonGame
};

static const CoordType kAILeftAreaLeft = 76;
static const CoordType kAILeftAreaTop = 334;

static const CoordType kAILeftAreaWidth = 96;
static const CoordType kAILeftAreaHeight = 96;

static const CoordType kAIMiddleAreaLeft = 172;
static const CoordType kAIMiddleAreaTop = 334;

static const CoordType kAIMiddleAreaWidth = 192;
static const CoordType kAIMiddleAreaHeight = 96;

static const CoordType kAIRightAreaLeft = 364;
static const CoordType kAIRightAreaTop = 334;

static const CoordType kAIRightAreaWidth = 96;
static const CoordType kAIRightAreaHeight = 96;

enum {
	kTSAPlayerNotArrived,				// initial state, must be zero
	kTSAPlayerForcedReview,				// Player must watch TBP before rip occurs.
	kTSAPlayerDetectedRip,				// Player finished TBP, rip alarm just went off.
	kTSAPlayerNeedsHistoricalLog,		// Player is instructed to get historical log
	kTSAPlayerGotHistoricalLog,
	kTSAPlayerInstalledHistoricalLog,
	kTSABossSawHistoricalLog,
	kRobotsAtCommandCenter,
	kRobotsAtFrontDoor,
	kRobotsAtReadyRoom,
	kPlayerLockedInPegasus,
	kPlayerOnWayToPrehistoric,
	kPlayerWentToPrehistoric,
	kPlayerOnWayToNorad,
	kPlayerOnWayToMars,
	kPlayerOnWayToWSC,
	kPlayerFinishedWithTSA
};

/////////////////////////////////////////////
//
// Mode static constants.

static const GameMode kModeInventoryPick = kLastGameShellMode + 1;
static const GameMode kModeBiochipPick = kModeInventoryPick + 1;
static const GameMode kModeInfoScreen = kModeBiochipPick + 1;

} // End of namespace Pegasus

#endif
