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
#include "pegasus/neighborhood/neighborhood.h"

namespace Pegasus {

// indexed by [number of hints][number of solves (0, 1, or 2)][which button to highlight]
static const ItemState s_highlightState[4][3][7] = {
	{
		{kAI000,	-1,			-1,			-1,			-1,			kAI005,		kAI006},
		{kAI010,	-1,			-1,			-1,			-1,			kAI015,		kAI016},
		{kAI020,	-1,			-1,			-1,			kAI024,		-1,			-1}
	},
	{
		{kAI100,	kAI101,		-1,			-1,			-1,			kAI105,		kAI106},
		{kAI110,	kAI111,		-1,			-1,			-1,			kAI115,		kAI116},
		{kAI120,	kAI121,		-1,			-1,			kAI124,		kAI125,		kAI126}
	},
	{
		{kAI200,	kAI201,		kAI202,		-1,			-1,			kAI205,		kAI206},
		{kAI210,	kAI211,		kAI212,		-1,			-1,			kAI215,		kAI216},
		{kAI220,	kAI221,		kAI222,		-1,			kAI224,		kAI225,		kAI226}
	},
	{
		{kAI300,	kAI301,		kAI302,		kAI303,		-1,			kAI305,		kAI306},
		{kAI310,	kAI311,		kAI312,		kAI313,		-1,			kAI315,		kAI316},
		{kAI320,	kAI321,		kAI322,		kAI323,		kAI324,		kAI325,		kAI326}
	}
};

AIChip *g_AIChip = 0;

AIChip::AIChip(const ItemID id, const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant direction) :
		BiochipItem(id, neighborhood, room, direction), _briefingSpot(kAIBriefingSpotID), _scanSpot(kAIScanSpotID),
		_hint1Spot(kAIHint1SpotID), _hint2Spot(kAIHint2SpotID), _hint3Spot(kAIHint3SpotID), _solveSpot(kAISolveSpotID) {
	_briefingSpot.setArea(Common::Rect(kAIMiddleAreaLeft + 10, kAIMiddleAreaTop + 27, kAIMiddleAreaLeft + 10 + 81, kAIMiddleAreaTop + 27 + 31));
	_briefingSpot.setHotspotFlags(kAIBiochipSpotFlag);
	g_allHotspots.push_back(&_briefingSpot);

	_scanSpot.setArea(Common::Rect(kAIMiddleAreaLeft + 100, kAIMiddleAreaTop + 27, kAIMiddleAreaLeft + 100 + 81, kAIMiddleAreaTop + 27 + 31));
	_scanSpot.setHotspotFlags(kAIBiochipSpotFlag);
	g_allHotspots.push_back(&_scanSpot);

	_hint1Spot.setArea(Common::Rect(kAIMiddleAreaLeft + 70, kAIMiddleAreaTop + 67, kAIMiddleAreaLeft + 70 + 21, kAIMiddleAreaTop + 67 + 21));
	_hint1Spot.setHotspotFlags(kAIBiochipSpotFlag);
	g_allHotspots.push_back(&_hint1Spot);

	_hint2Spot.setArea(Common::Rect(kAIMiddleAreaLeft + 91, kAIMiddleAreaTop + 67, kAIMiddleAreaLeft + 91 + 20, kAIMiddleAreaTop + 67 + 21));
	_hint2Spot.setHotspotFlags(kAIBiochipSpotFlag);
	g_allHotspots.push_back(&_hint2Spot);

	_hint3Spot.setArea(Common::Rect(kAIMiddleAreaLeft + 111, kAIMiddleAreaTop + 67, kAIMiddleAreaLeft + 111 + 20, kAIMiddleAreaTop + 67 + 21));
	_hint3Spot.setHotspotFlags(kAIBiochipSpotFlag);
	g_allHotspots.push_back(&_hint3Spot);

	_solveSpot.setArea(Common::Rect(kAIMiddleAreaLeft + 131, kAIMiddleAreaTop + 67, kAIMiddleAreaLeft + 131 + 50, kAIMiddleAreaTop + 67 + 21));
	_solveSpot.setHotspotFlags(kAIBiochipSpotFlag);
	g_allHotspots.push_back(&_solveSpot);

	_playingMovie = false;
	setItemState(kAI000);

	g_AIChip = this;
}

AIChip::~AIChip() {
	g_AIChip = NULL;

	g_allHotspots.removeOneHotspot(kAIBriefingSpotID);
	g_allHotspots.removeOneHotspot(kAIScanSpotID);
	g_allHotspots.removeOneHotspot(kAIHint1SpotID);
	g_allHotspots.removeOneHotspot(kAIHint2SpotID);
	g_allHotspots.removeOneHotspot(kAIHint3SpotID);
	g_allHotspots.removeOneHotspot(kAISolveSpotID);
}

void AIChip::select() {
	BiochipItem::select();
	setUpAIChip();
}

void AIChip::takeSharedArea() {
	setUpAIChip();
}

void AIChip::setUpAIChip() {
	if (!_playingMovie) {
		PegasusEngine *vm = (PegasusEngine *)g_engine;

		uint numSolves;
		if (GameState.getWalkthroughMode()) {
			if (vm->canSolve())
				numSolves = 2;
			else
				numSolves = 1;
		} else {
			numSolves = 0;
		}

		setItemState(s_highlightState[vm->getNumHints()][numSolves][0]);
	}
}

// Only does something when there are hints or solves available.
void AIChip::setUpAIChipRude() {
	if (!_playingMovie) {
		PegasusEngine *vm = (PegasusEngine *)g_engine;

		uint numSolves;
		if (GameState.getWalkthroughMode()) {
			if (vm->canSolve())
				numSolves = 2;
			else
				numSolves = 1;
		} else {
			numSolves = 0;
		}

		uint numHints = vm->getNumHints();
		if (numSolves == 2 || numHints != 0)
			setItemState(s_highlightState[numHints][numSolves][0]);
	}
}

void AIChip::activateAIHotspots() {
	PegasusEngine *vm = (PegasusEngine *)g_engine;
	_briefingSpot.setActive();
	_scanSpot.setActive();

	switch (vm->getNumHints()) {
	case 3:
		_hint3Spot.setActive();
		// fall through
	case 2:
		_hint2Spot.setActive();
		// fall through
	case 1:
		_hint1Spot.setActive();
		break;
	}

	if (GameState.getWalkthroughMode() && vm->canSolve())
		_solveSpot.setActive();
}

void AIChip::showBriefingClicked() {
	PegasusEngine *vm = (PegasusEngine *)g_engine;

	_playingMovie = true;

	uint numSolves;
	if (GameState.getWalkthroughMode()) {
		if (vm->canSolve())
			numSolves = 2;
		else
			numSolves = 1;
	} else {
		numSolves = 0;
	}

	ItemState newState = s_highlightState[vm->getNumHints()][numSolves][kAIBriefingSpotID - kAIHint1SpotID + 1];
	if (newState != -1)
		setItemState(newState);
}

void AIChip::showEnvScanClicked() {
	PegasusEngine *vm = (PegasusEngine *)g_engine;

	_playingMovie = true;

	uint numSolves;
	if (GameState.getWalkthroughMode()) {
		if (vm->canSolve())
			numSolves = 2;
		else
			numSolves = 1;
	} else {
		numSolves = 0;
	}

	ItemState newState = s_highlightState[vm->getNumHints()][numSolves][kAIScanSpotID - kAIHint1SpotID + 1];

	if (newState != -1)
		setItemState(newState);
}

void AIChip::clearClicked() {
	_playingMovie = false;
	setUpAIChip();
}

void AIChip::clickInAIHotspot(HotSpotID id) {
	PegasusEngine *vm = (PegasusEngine *)g_engine;

	Common::String movieName;

	switch (id) {
	case kAIBriefingSpotID:
		movieName = vm->getBriefingMovie();
		break;
	case kAIScanSpotID:
		movieName = vm->getEnvScanMovie();
		break;
	case kAIHint1SpotID:
		movieName = vm->getHintMovie(1);
		break;
	case kAIHint2SpotID:
		movieName = vm->getHintMovie(2);
		break;
	case kAIHint3SpotID:
		movieName = vm->getHintMovie(3);
		break;
	case kAISolveSpotID:
		g_neighborhood->doSolve();
		break;
	}

	ItemState state = getItemState();

	if (!movieName.empty()) {
		_playingMovie = true;

		uint numSolves;
		if (GameState.getWalkthroughMode()) {
			if (vm->canSolve())
				numSolves = 2;
			else
				numSolves = 1;
		} else {
			numSolves = 0;
		}

		ItemState newState = s_highlightState[vm->getNumHints()][numSolves][id - kAIHint1SpotID + 1];

		if (newState != -1)
			setItemState(newState);

		if (g_AIArea) {
			vm->prepareForAIHint(movieName);
			g_AIArea->playAIMovie(kRightAreaSignature, movieName, false, kHintInterruption);
			vm->cleanUpAfterAIHint(movieName);
		}

		if (newState != -1)
			setItemState(state);

		_playingMovie = false;
	}
}

} // End of namespace Pegasus
