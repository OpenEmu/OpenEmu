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

#ifndef PEGASUS_SCORING_H
#define PEGASUS_SCORING_H

#include "pegasus/types.h"

namespace Pegasus {

/////////////////////////////////////////////
//
// Scoring.

static const CoordType kDeathScreenScoreLeft = 151;
static const CoordType kDeathScreenScoreTop = 212;
static const CoordType kDeathScreenScoreWidth = 124;
static const CoordType kDeathScreenScoreHeight = 12;
static const CoordType kDeathScreenScoreSkipVert = -16;

// Caldoria & TSA

static const GameScoreType kSawINNScore					= 5;
static const GameScoreType kTookShowerScore				= 2;
static const GameScoreType kFixedHairScore				= 2;
static const GameScoreType kGotKeyCardScore				= 5;
static const GameScoreType kReadPaperScore				= 2;
static const GameScoreType kLookThroughTelescopeScore		= 2;
static const GameScoreType kSawCaldoriaKioskScore			= 2;
static const GameScoreType kGoToTSAScore					= 3;

static const GameScoreType kEnterTSAScore					= 2;
static const GameScoreType kSawBust1Score					= 2;
static const GameScoreType kSawBust2Score					= 2;
static const GameScoreType kSawBust3Score					= 2;
static const GameScoreType kSawBust4Score					= 2;
static const GameScoreType kSawBust5Score					= 2;
static const GameScoreType kSawBust6Score					= 2;
static const GameScoreType kSawTheoryScore				= 4;
static const GameScoreType kSawBackgroundScore			= 4;
static const GameScoreType kSawProcedureScore				= 4;
static const GameScoreType kGotJourneymanKeyScore			= 5;
static const GameScoreType kGotPegasusBiochipScore		= 5;
static const GameScoreType kGotBiosuitScore				= 5;
static const GameScoreType kGoToPrehistoricScore			= 5;

static const GameScoreType kPutLogInReaderScore			= 5;
static const GameScoreType kSawCaldoriaNormalScore		= 2;
static const GameScoreType kSawCaldoriaAlteredScore		= 2;
static const GameScoreType kSawNoradNormalScore			= 2;
static const GameScoreType kSawNoradAlteredScore			= 2;
static const GameScoreType kSawMarsNormalScore			= 2;
static const GameScoreType kSawMarsAlteredScore			= 2;
static const GameScoreType kSawWSCNormalScore				= 2;
static const GameScoreType kSawWSCAlteredScore			= 2;
static const GameScoreType kWentToReadyRoom2Score			= 5;
static const GameScoreType kWentAfterSinclairScore		= 5;
static const GameScoreType kUsedCardBombScore				= 10;
static const GameScoreType kShieldedCardBombScore			= 5;
static const GameScoreType kStunnedSinclairScore			= 10;
static const GameScoreType kDisarmedNukeScore				= 10;

static const GameScoreType kMaxCaldoriaTSAScoreBefore		=	kSawINNScore +
														kTookShowerScore +
														kFixedHairScore +
														kGotKeyCardScore +
														kReadPaperScore +
														kLookThroughTelescopeScore +
														kSawCaldoriaKioskScore +
														kGoToTSAScore +
														kEnterTSAScore +
														kSawBust1Score +
														kSawBust2Score +
														kSawBust3Score +
														kSawBust4Score +
														kSawBust5Score +
														kSawBust6Score +
														kSawTheoryScore +
														kSawBackgroundScore +
														kSawProcedureScore +
														kGotJourneymanKeyScore +
														kGotPegasusBiochipScore +
														kGotBiosuitScore +
														kGoToPrehistoricScore +
														kPutLogInReaderScore +
														kSawCaldoriaNormalScore +
														kSawCaldoriaAlteredScore +
														kSawNoradNormalScore +
														kSawNoradAlteredScore +
														kSawMarsNormalScore +
														kSawMarsAlteredScore +
														kSawWSCNormalScore +
														kSawWSCAlteredScore +
														kWentToReadyRoom2Score;

static const GameScoreType kMaxCaldoriaTSAScoreAfter		=	kWentAfterSinclairScore +
														kUsedCardBombScore +
														kShieldedCardBombScore +
														kStunnedSinclairScore +
														kDisarmedNukeScore;

static const GameScoreType kMaxCaldoriaTSAScore			=	kMaxCaldoriaTSAScoreBefore +
														kMaxCaldoriaTSAScoreAfter;

// Prehistoric

static const GameScoreType kThrewBreakerScore				= 10;
static const GameScoreType kExtendedBridgeScore			= 10;
static const GameScoreType kGotHistoricalLogScore			= 5;
static const GameScoreType kFinishedPrehistoricScore		= 10;

static const GameScoreType kMaxPrehistoricScore			=	kThrewBreakerScore +
														kExtendedBridgeScore +
														kGotHistoricalLogScore +
														kFinishedPrehistoricScore;

// Mars

static const GameScoreType kThrownByRobotScore			= 3;
static const GameScoreType kGotMarsCardScore				= 5;
static const GameScoreType kSawMarsKioskScore				= 2;
static const GameScoreType kSawTransportMapScore			= 2;
static const GameScoreType kGotCrowBarScore				= 5;
static const GameScoreType kTurnedOnTransportScore		= 5;
static const GameScoreType kGotOxygenMaskScore			= 5;
static const GameScoreType kAvoidedRobotScore				= 5;
static const GameScoreType kActivatedPlatformScore		= 2;
static const GameScoreType kUsedLiquidNitrogenScore		= 3;
static const GameScoreType kUsedCrowBarScore				= 3;
static const GameScoreType kFoundCardBombScore			= 4;
static const GameScoreType kDisarmedCardBombScore			= 8;
static const GameScoreType kGotCardBombScore				= 5;
static const GameScoreType kThreadedMazeScore				= 5;
static const GameScoreType kThreadedGearRoomScore			= 2;
static const GameScoreType kEnteredShuttleScore			= 2;
static const GameScoreType kEnteredLaunchTubeScore		= 4;
static const GameScoreType kStoppedRobotsShuttleScore		= 10;
static const GameScoreType kGotMarsOpMemChipScore			= 10;
static const GameScoreType kFinishedMarsScore				= 10;

static const GameScoreType kMaxMarsScore					=	kThrownByRobotScore +
														kGotMarsCardScore +
														kSawMarsKioskScore +
														kSawTransportMapScore +
														kGotCrowBarScore +
														kTurnedOnTransportScore +
														kGotOxygenMaskScore +
														kAvoidedRobotScore +
														kActivatedPlatformScore +
														kUsedLiquidNitrogenScore +
														kUsedCrowBarScore +
														kFoundCardBombScore +
														kDisarmedCardBombScore +
														kGotCardBombScore +
														kThreadedMazeScore +
														kThreadedGearRoomScore +
														kEnteredShuttleScore +
														kEnteredLaunchTubeScore +
														kStoppedRobotsShuttleScore +
														kGotMarsOpMemChipScore +
														kFinishedMarsScore;

// Norad

static const GameScoreType kSawSecurityMonitorScore		= 5;
static const GameScoreType kFilledOxygenCanisterScore		= 5;
static const GameScoreType kFilledArgonCanisterScore		= 5;
static const GameScoreType kSawUnconsciousOperatorScore	= 5;
static const GameScoreType kWentThroughPressureDoorScore	= 5;
static const GameScoreType kPreppedSubScore				= 5;
static const GameScoreType kEnteredSubScore				= 5;
static const GameScoreType kExitedSubScore				= 10;
static const GameScoreType kSawRobotAt54NorthScore		= 5;
static const GameScoreType kPlayedWithClawScore			= 5;
static const GameScoreType kUsedRetinalChipScore			= 5;
static const GameScoreType kFinishedGlobeGameScore		= 10;
static const GameScoreType kStoppedNoradRobotScore		= 10;
static const GameScoreType kGotNoradOpMemChipScore		= 10;
static const GameScoreType kFinishedNoradScore			= 10;

static const GameScoreType kMaxNoradScore					=	kSawSecurityMonitorScore +
														kFilledOxygenCanisterScore +
														kFilledArgonCanisterScore +
														kSawUnconsciousOperatorScore +
														kWentThroughPressureDoorScore +
														kPreppedSubScore +
														kEnteredSubScore +
														kExitedSubScore +
														kSawRobotAt54NorthScore +
														kPlayedWithClawScore +
														kUsedRetinalChipScore +
														kFinishedGlobeGameScore +
														kStoppedNoradRobotScore +
														kGotNoradOpMemChipScore +
														kFinishedNoradScore;

// WSC

static const GameScoreType kRemovedDartScore				= 5;
static const GameScoreType kAnalyzedDartScore				= 5;
static const GameScoreType kBuiltAntidoteScore			= 5;
static const GameScoreType kGotSinclairKeyScore			= 5;
static const GameScoreType kGotArgonCanisterScore 		= 5;
static const GameScoreType kGotNitrogenCanisterScore		= 5;
static const GameScoreType kPlayedWithMessagesScore		= 2;
static const GameScoreType kSawMorphExperimentScore		= 3;
static const GameScoreType kEnteredSinclairOfficeScore	= 2;
static const GameScoreType kSawBrochureScore				= 3;
static const GameScoreType kSawSinclairEntry1Score		= 3;
static const GameScoreType kSawSinclairEntry2Score		= 3;
static const GameScoreType kSawSinclairEntry3Score		= 3;
static const GameScoreType kSawWSCDirectoryScore			= 3;
static const GameScoreType kUsedCrowBarInWSCScore			= 5;
static const GameScoreType kFinishedPlasmaDodgeScore		= 10;
static const GameScoreType kOpenedCatwalkScore			= 3;
static const GameScoreType kStoppedWSCRobotScore			= 10;
static const GameScoreType kGotWSCOpMemChipScore			= 10;
static const GameScoreType kFinishedWSCScore				= 10;

static const GameScoreType kMaxWSCScore					=	kRemovedDartScore +
														kAnalyzedDartScore +
														kBuiltAntidoteScore +
														kGotSinclairKeyScore +
														kGotArgonCanisterScore +
														kGotNitrogenCanisterScore +
														kPlayedWithMessagesScore +
														kSawMorphExperimentScore +
														kEnteredSinclairOfficeScore +
														kSawBrochureScore +
														kSawSinclairEntry1Score +
														kSawSinclairEntry2Score +
														kSawSinclairEntry3Score +
														kSawWSCDirectoryScore +
														kUsedCrowBarInWSCScore +
														kFinishedPlasmaDodgeScore +
														kOpenedCatwalkScore +
														kStoppedWSCRobotScore +
														kGotWSCOpMemChipScore +
														kFinishedWSCScore;

// Gandhi

static const GameScoreType kMarsGandhiScore				= 10;
static const GameScoreType kNoradGandhiScore				= 10;
static const GameScoreType kWSCGandhiScore				= 10;

static const GameScoreType kMaxGandhiScore				=	kMarsGandhiScore +
														kNoradGandhiScore +
														kWSCGandhiScore;

static const GameScoreType kMaxTotalScore					=	kMaxCaldoriaTSAScore +
														kMaxPrehistoricScore +
														kMaxMarsScore +
														kMaxNoradScore +
														kMaxWSCScore +
														kMaxGandhiScore;
} // End of namespace Pegasus

#endif
