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
#include "pegasus/items/biochips/pegasuschip.h"
#include "pegasus/neighborhood/tsa/fulltsa.h"
#include "pegasus/neighborhood/tsa/tinytsa.h"

namespace Pegasus {

PegasusChip::PegasusChip(const ItemID id, const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant direction) :
		BiochipItem(id, neighborhood, room, direction), _recallSpot(kPegasusRecallSpotID) {
	_recallSpot.setArea(Common::Rect(kAIMiddleAreaLeft + 116, kAIMiddleAreaTop + 63, kAIMiddleAreaLeft + 184, kAIMiddleAreaTop + 91));
	_recallSpot.setHotspotFlags(kPegasusBiochipSpotFlag);
	g_allHotspots.push_back(&_recallSpot);
	setItemState(kPegasusTSA00);
}

PegasusChip::~PegasusChip() {
	g_allHotspots.removeOneHotspot(kPegasusRecallSpotID);
}

void PegasusChip::select() {
	BiochipItem::select();
	setUpPegasusChip();
}

void PegasusChip::setUpPegasusChip() {
	switch (GameState.getCurrentNeighborhood()) {
	case kCaldoriaID:
		setItemState(kPegasusCaldoria);
		break;
	case kFullTSAID:
	case kFinalTSAID:
	case kTinyTSAID:
		setItemState(kPegasusTSA10);
		break;
	case kPrehistoricID:
		if (((PegasusEngine *)g_engine)->playerHasItemID(kHistoricalLog))
			setItemState(kPegasusPrehistoric00);
		else
			setItemState(kPegasusPrehistoric10);
		break;
	case kMarsID:
		if (GameState.getMarsFinished())
			setItemState(kPegasusMars00);
		else
			setItemState(kPegasusMars10);
		break;
	case kWSCID:
		if (GameState.getWSCFinished())
			setItemState(kPegasusWSC00);
		else
			setItemState(kPegasusWSC10);
		break;
	case kNoradAlphaID:
	case kNoradDeltaID:
		if (GameState.getNoradFinished())
			setItemState(kPegasusNorad00);
		else
			setItemState(kPegasusNorad10);
		break;
	}
}

// Only does something if the chip should be announcing that the time zone is finished...
void PegasusChip::setUpPegasusChipRude() {
	switch (GameState.getCurrentNeighborhood()) {
	case kPrehistoricID:
		if (((PegasusEngine *)g_engine)->playerHasItemID(kHistoricalLog))
			setItemState(kPegasusPrehistoric00);
		break;
	case kMarsID:
		if (GameState.getMarsFinished())
			setItemState(kPegasusMars00);
		break;
	case kWSCID:
		if (GameState.getWSCFinished())
			setItemState(kPegasusWSC00);
		break;
	case kNoradAlphaID:
	case kNoradDeltaID:
		if (GameState.getNoradFinished())
			setItemState(kPegasusNorad00);
		break;
	}
}

void PegasusChip::activatePegasusHotspots() {
	switch (GameState.getCurrentNeighborhood()) {
	case kPrehistoricID:
		// WORKAROUND: Don't allow the player to recall if they don't have
		// the historical log. Otherwise, gameplay is broken when returning
		// to the TSA.
		if (!((PegasusEngine *)g_engine)->playerHasItemID(kHistoricalLog))
			return;
		// fall through
	case kMarsID:
	case kWSCID:
	case kNoradAlphaID:
	case kNoradDeltaID:
		_recallSpot.setActive();
		break;
	}
}

void PegasusChip::clickInPegasusHotspot() {
	PegasusEngine *vm = (PegasusEngine *)g_engine;

	ItemState thisState = getItemState();
	ItemState hiliteState;

	switch (thisState) {
	case kPegasusPrehistoric00:
		hiliteState = kPegasusPrehistoric01;
		break;
	case kPegasusPrehistoric10:
		hiliteState = kPegasusPrehistoric11;
		break;
	case kPegasusMars00:
		hiliteState = kPegasusMars01;
		break;
	case kPegasusMars10:
		hiliteState = kPegasusMars11;
		break;
	case kPegasusNorad00:
		hiliteState = kPegasusNorad01;
		break;
	case kPegasusNorad10:
		hiliteState = kPegasusNorad11;
		break;
	case kPegasusWSC00:
		hiliteState = kPegasusWSC01;
		break;
	case kPegasusWSC10:
		hiliteState = kPegasusWSC11;
		break;
	default:
		error("Invalid pegasus chip state");
	}

	// WORKAROUND: The original called setItemState() here. However,
	// since we're overriding select() to call setUpPegasusChip(),
	// the highlighted frame is never displayed! So, we're manually
	// setting the state and selecting the item. Also of note is that
	// setItemState() for this class is effectively useless since it
	// always gets overriden in the select() function. The only reason
	// that this doesn't end in infinite recursion is because setItemState()
	// has a check against the current state to make sure you don't call
	// select() again. </rant>
	_itemState = hiliteState;
	BiochipItem::select();

	uint32 time = g_system->getMillis();
	while (g_system->getMillis() < time + 500) {
		vm->refreshDisplay();
		g_system->delayMillis(10);
	}

	setItemState(thisState);

	if (!((Neighborhood *)g_neighborhood)->okayToJump())
		return;

	if (g_energyMonitor)
		g_energyMonitor->stopEnergyDraining();

	if (GameState.getTSAState() == kPlayerWentToPrehistoric || GameState.allTimeZonesFinished())
		vm->jumpToNewEnvironment(kFullTSAID, kTSA37, kNorth);
	else
		vm->jumpToNewEnvironment(kTinyTSAID, kTinyTSA37, kNorth);
}

} // End of namespace Pegasus
