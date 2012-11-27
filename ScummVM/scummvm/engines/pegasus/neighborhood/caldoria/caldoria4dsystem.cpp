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

#include "pegasus/pegasus.h"
#include "pegasus/ai/ai_area.h"
#include "pegasus/neighborhood/caldoria/caldoria.h"
#include "pegasus/neighborhood/caldoria/caldoria4dsystem.h"

namespace Pegasus {

static const TimeValue kSwitchableSlop = 3 * kCaldoriaFrameDuration;
// Two seconds - some slop
static const TimeValue kSwitchableDuration = kCaldoriaMovieScale * 2 - kSwitchableSlop;
// Twelve frames + some slop
static const TimeValue kNonswitchableDuration = kCaldoriaFrameDuration * 12 + kSwitchableSlop;

static const TimeValue kSwitchable1Start = 0;
static const TimeValue kSwitchable1Stop = kSwitchable1Start + kSwitchableDuration;

static const TimeValue kSwitchable2Start = kSwitchable1Stop + kNonswitchableDuration;
static const TimeValue kSwitchable2Stop = kSwitchable2Start + kSwitchableDuration;

static const TimeValue kSwitchable3Start = kSwitchable2Stop + kNonswitchableDuration;
static const TimeValue kSwitchable3Stop = kSwitchable3Start + kSwitchableDuration;

static const NotificationFlags kVidPhoneDoneFlag = 1;

static const TimeValue kRockMusicLoopIn = 0;
static const TimeValue kRockMusicLoopOut = 2088;

static const TimeValue kOrchestralMusicLoopIn = 2088;
static const TimeValue kOrchestralMusicLoopOut = 4985;

static const TimeValue kRhythmsMusicLoopIn = 4985;
static const TimeValue kRhythmsMusicLoopOut = 6824;

static const TimeValue kAcousticMusicLoopIn = 6824;
static const TimeValue kAcousticMusicLoopOut = 9387;

enum {
	k4DVideoMenu,
	k4DAudioMenu,
	k4DShuttingDown,

	// These constants are the exact frame numbers of the sprite movie.
	k4DRockChoice = 0,
	k4DOrchestralChoice,
	k4DRhythmsChoice,
	k4DAcousticChoice,
	k4DIslandChoice,
	k4DDesertChoice,
	k4DMountainChoice,

	k4DFirstVideoChoice = k4DIslandChoice
};

static const ExtraID s_transitionExtras0[3][3] = {
	{ 0xffffffff, k4DIsland0ToDesert0, k4DIsland0ToMountain0 },
	{ k4DDesert0ToIsland0, 0xffffffff, k4DDesert0ToMountain0 },
	{ k4DMountain0ToIsland0, k4DMountain0ToDesert0, 0xffffffff }
};

static const ExtraID s_transitionExtras1[3][3] = {
	{ 0xffffffff, k4DIsland1ToDesert0, k4DIsland1ToMountain0 },
	{ k4DDesert1ToIsland0, 0xffffffff, k4DDesert1ToMountain0 },
	{ k4DMountain1ToIsland0, k4DMountain1ToDesert0, 0xffffffff }
};

static const ExtraID s_transitionExtras2[3][3] = {
	{ 0xffffffff, k4DIsland2ToDesert0, k4DIsland2ToMountain0 },
	{ k4DDesert2ToIsland0, 0xffffffff, k4DDesert2ToMountain0 },
	{ k4DMountain2ToIsland0, k4DMountain2ToDesert0, 0xffffffff }
};

static const ExtraID s_shutDownExtras[3][3] = {
	{ 0xffffffff, k4DIsland1ToIsland0, k4DIsland2ToIsland0 },
	{ k4DDesert0ToIsland0, k4DDesert1ToIsland0, k4DDesert2ToIsland0 },
	{ k4DMountain0ToIsland0, k4DMountain1ToIsland0, k4DMountain2ToIsland0 }
};

Caldoria4DSystem::Caldoria4DSystem(Neighborhood *owner) : GameInteraction(kCaldoria4DInteractionID, owner),
		_4DSpritesMovie(kCaldoria4DSpritesID) {
	g_AIArea->lockAIOut();
}

Caldoria4DSystem::~Caldoria4DSystem() {
	g_AIArea->unlockAI();
}

void Caldoria4DSystem::openInteraction() {
	_whichMenu = k4DVideoMenu;
	_videoChoice = k4DIslandChoice;
	_audioChoice = k4DRockChoice;
	_clickedHotspotID = kNoHotSpotID;

	_4DSpritesMovie.initFromMovieFile("Images/Caldoria/4D Sprites", true);
	_4DSpritesMovie.moveElementTo(kCaldoria4DSpritesLeft, kCaldoria4DSpritesTop);
	_4DSpritesMovie.setDisplayOrder(k4DSpritesOrder);
	_4DSpritesMovie.startDisplaying();
	_4DSpritesMovie.show();

	_4DSpritesScale = _4DSpritesMovie.getScale();

	_neighborhoodNotification = _owner->getNeighborhoodNotification();
	_neighborhoodNotification->notifyMe(this, kExtraCompletedFlag, kExtraCompletedFlag);

	startIdling();
}

void Caldoria4DSystem::loopExtra(const ExtraID extraID) {
	ExtraTable::Entry extraEntry;

	_owner->getExtraEntry(extraID, extraEntry);
	_loopStart = extraEntry.movieStart;
	_owner->loopExtraSequence(extraID);
}

void Caldoria4DSystem::useIdleTime() {
	if (_whichMenu == k4DShuttingDown) {
		TimeValue movieTime = _owner->getNavMovie()->getTime() - _loopStart;
		ExtraID extraID;

		if (movieTime < kSwitchable1Stop)
			extraID = s_shutDownExtras[_videoChoice - k4DFirstVideoChoice][0];
		else if (movieTime >= kSwitchable2Start && movieTime < kSwitchable2Stop)
			extraID = s_shutDownExtras[_videoChoice - k4DFirstVideoChoice][1];
		else if (movieTime >= kSwitchable3Start && movieTime < kSwitchable3Stop)
			extraID = s_shutDownExtras[_videoChoice - k4DFirstVideoChoice][2];
		else
			extraID = 0xffffffff;

		if (extraID != 0xffffffff) {
			setSpritesMovie();
			_loopStart = 0;
			_owner->startExtraSequence(extraID, kExtraCompletedFlag, kFilterNoInput);
		}
	} else if (_clickedHotspotID != kNoHotSpotID) {
		TimeValue movieTime = _owner->getNavMovie()->getTime() - _loopStart;
		ExtraID extraID;

		if (movieTime < kSwitchable1Stop) {
			extraID = s_transitionExtras0[_videoChoice - k4DFirstVideoChoice][_clickedHotspotID - kCa4DChoice1SpotID];
			_clickedHotspotID = kNoHotSpotID;
		} else if (movieTime >= kSwitchable2Start && movieTime < kSwitchable2Stop) {
			extraID = s_transitionExtras1[_videoChoice - k4DFirstVideoChoice][_clickedHotspotID - kCa4DChoice1SpotID];
			_clickedHotspotID = kNoHotSpotID;
		} else if (movieTime >= kSwitchable3Start && movieTime < kSwitchable3Stop) {
			extraID = s_transitionExtras2[_videoChoice - k4DFirstVideoChoice][_clickedHotspotID - kCa4DChoice1SpotID];
			_clickedHotspotID = kNoHotSpotID;
		} else
			extraID = 0xffffffff;

		if (extraID != 0xffffffff) {
			switch (extraID) {
			case k4DDesert0ToIsland0:
			case k4DMountain0ToIsland0:
			case k4DDesert1ToIsland0:
			case k4DMountain1ToIsland0:
			case k4DDesert2ToIsland0:
			case k4DMountain2ToIsland0:
				_videoChoice = k4DIslandChoice;
				break;
			case k4DIsland0ToDesert0:
			case k4DMountain0ToDesert0:
			case k4DIsland1ToDesert0:
			case k4DMountain1ToDesert0:
			case k4DIsland2ToDesert0:
			case k4DMountain2ToDesert0:
				_videoChoice = k4DDesertChoice;
				break;
			case k4DDesert0ToMountain0:
			case k4DIsland0ToMountain0:
			case k4DIsland1ToMountain0:
			case k4DDesert1ToMountain0:
			case k4DIsland2ToMountain0:
			case k4DDesert2ToMountain0:
				_videoChoice = k4DMountainChoice;
				break;
			}

			setSpritesMovie();
			_loopStart = 0;
			_owner->startExtraSequence(extraID, kExtraCompletedFlag, kFilterNoInput);
		}
	}
}

void Caldoria4DSystem::initInteraction() {
	setSpritesMovie();

	_owner->loadLoopSound1("Sounds/Caldoria/Rock.aiff");
	loopExtra(k4DIslandLoop);
}

void Caldoria4DSystem::closeInteraction() {
	stopIdling();
	_neighborhoodNotification->cancelNotification(this);
	_4DSpritesMovie.releaseMovie();
	_owner->loadAmbientLoops();
}

void Caldoria4DSystem::setSpritesMovie() {
	if (_whichMenu == k4DShuttingDown)
		_4DSpritesMovie.setTime(_4DSpritesScale * k4DIslandChoice);
	else if (_whichMenu == k4DVideoMenu)
		_4DSpritesMovie.setTime(_4DSpritesScale * _videoChoice);
	else if (_whichMenu == k4DAudioMenu)
		_4DSpritesMovie.setTime(_4DSpritesScale * _audioChoice);

	_4DSpritesMovie.redrawMovieWorld();
}

void Caldoria4DSystem::handleInput(const Input &input, const Hotspot *cursorSpot) {
	if (input.downButtonAnyDown())
		return;
	if (input.anyDirectionInput())
		shutDown4DSystem();
	else
		GameInteraction::handleInput(input, cursorSpot);
}

void Caldoria4DSystem::activateHotspots() {
	GameInteraction::activateHotspots();
	if (_whichMenu == k4DAudioMenu)
		g_allHotspots.activateOneHotspot(kCa4DChoice4SpotID);
}

void Caldoria4DSystem::clickInHotspot(const Input &input, const Hotspot *spot) {
	switch (spot->getObjectID()) {
	case kCa4DVisualSpotID:
		if (_whichMenu == k4DAudioMenu) {
			_whichMenu = k4DVideoMenu;
			setSpritesMovie();
		}
		break;
	case kCa4DAudioSpotID:
		if (_whichMenu == k4DVideoMenu) {
			_whichMenu = k4DAudioMenu;
			setSpritesMovie();
		}
		break;
	case kCa4DChoice1SpotID:
		if (_whichMenu == k4DVideoMenu)
			makeIslandChoice();
		else if (_whichMenu == k4DAudioMenu)
			makeRockChoice();
		break;
	case kCa4DChoice2SpotID:
		if (_whichMenu == k4DVideoMenu)
			makeDesertChoice();
		else if (_whichMenu == k4DAudioMenu)
			makeOrchestralChoice();
		break;
	case kCa4DChoice3SpotID:
		if (_whichMenu == k4DVideoMenu)
			makeMountainChoice();
		else if (_whichMenu == k4DAudioMenu)
			makeRhythmsChoice();
		break;
	case kCa4DChoice4SpotID:
		if (_whichMenu == k4DAudioMenu)
			makeAcousticChoice();
		else
			_owner->playSpotSoundSync(kCaldoria4DBlankChoiceIn, kCaldoria4DBlankChoiceOut);
		break;
	default:
		GameInteraction::clickInHotspot(input, spot);
	}
}

void Caldoria4DSystem::receiveNotification(Notification *, const NotificationFlags) {
	if (_whichMenu == k4DShuttingDown) {
		_owner->requestDeleteCurrentInteraction();
	} else {
		uint32 extraID;

		switch (_videoChoice) {
		case k4DIslandChoice:
			extraID = k4DIslandLoop;
			break;
		case k4DDesertChoice:
			extraID = k4DDesertLoop;
			break;
		case k4DMountainChoice:
			extraID = k4DMountainLoop;
			break;
		default:
			extraID = 0xffffffff;
			break;
		}

		if (extraID != 0xffffffff)
			loopExtra(extraID);
	}
}

void Caldoria4DSystem::makeIslandChoice() {
	if (_videoChoice != k4DIslandChoice && _clickedHotspotID == kNoHotSpotID)
		_clickedHotspotID = kCa4DChoice1SpotID;
}

void Caldoria4DSystem::makeDesertChoice() {
	if (_videoChoice != k4DDesertChoice && _clickedHotspotID == kNoHotSpotID)
		_clickedHotspotID = kCa4DChoice2SpotID;
}

void Caldoria4DSystem::makeMountainChoice() {
	if (_videoChoice != k4DMountainChoice && _clickedHotspotID == kNoHotSpotID)
		_clickedHotspotID = kCa4DChoice3SpotID;
}

void Caldoria4DSystem::makeRockChoice() {
	if (_audioChoice != k4DRockChoice) {
		_audioChoice = k4DRockChoice;
		setSpritesMovie();
		_owner->loadLoopSound1("Sounds/Caldoria/Rock.aiff");
	}
}

void Caldoria4DSystem::makeOrchestralChoice() {
	if (_audioChoice != k4DOrchestralChoice) {
		_audioChoice = k4DOrchestralChoice;
		setSpritesMovie();
		_owner->loadLoopSound1("Sounds/Caldoria/Orchestral.aiff");
	}
}

void Caldoria4DSystem::makeRhythmsChoice() {
	if (_audioChoice != k4DRhythmsChoice) {
		_audioChoice = k4DRhythmsChoice;
		setSpritesMovie();
		_owner->loadLoopSound1("Sounds/Caldoria/Rhythms.aiff");
	}
}

void Caldoria4DSystem::makeAcousticChoice() {
	if (_audioChoice != k4DAcousticChoice) {
		_audioChoice = k4DAcousticChoice;
		setSpritesMovie();
		_owner->loadLoopSound1("Sounds/Caldoria/Acoustic.aiff");
	}
}

void Caldoria4DSystem::shutDown4DSystem() {
	_whichMenu = k4DShuttingDown;
}

} // End of namespace Pegasus
