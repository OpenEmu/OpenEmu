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
#include "pegasus/items/biochips/opticalchip.h"

namespace Pegasus {

OpticalChip *g_opticalChip = 0;

OpticalChip::OpticalChip(const ItemID id, const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant direction) :
		BiochipItem(id, neighborhood, room, direction), _ariesHotspot(kAriesSpotID), _mercuryHotspot(kMercurySpotID),
		_poseidonHotspot(kPoseidonSpotID) {
	_ariesHotspot.setArea(Common::Rect(kAIMiddleAreaLeft + 60, kAIMiddleAreaTop + 27, kAIMiddleAreaLeft + 60 + 121, kAIMiddleAreaTop + 27 + 20));
	_ariesHotspot.setHotspotFlags(kOpticalBiochipSpotFlag);
	g_allHotspots.push_back(&_ariesHotspot);

	_mercuryHotspot.setArea(Common::Rect(kAIMiddleAreaLeft + 60, kAIMiddleAreaTop + 47, kAIMiddleAreaLeft + 60 + 121, kAIMiddleAreaTop + 47 + 20));
	_mercuryHotspot.setHotspotFlags(kOpticalBiochipSpotFlag);
	g_allHotspots.push_back(&_mercuryHotspot);

	_poseidonHotspot.setArea(Common::Rect(kAIMiddleAreaLeft + 60, kAIMiddleAreaTop + 67, kAIMiddleAreaLeft + 60 + 121, kAIMiddleAreaTop + 67 + 20));
	_poseidonHotspot.setHotspotFlags(kOpticalBiochipSpotFlag);
	g_allHotspots.push_back(&_poseidonHotspot);

	setItemState(kOptical000);

	g_opticalChip = this;
}

OpticalChip::~OpticalChip() {
	g_allHotspots.removeOneHotspot(kAriesSpotID);
	g_allHotspots.removeOneHotspot(kMercurySpotID);
	g_allHotspots.removeOneHotspot(kPoseidonSpotID);
}

void OpticalChip::writeToStream(Common::WriteStream *stream) {
	BiochipItem::writeToStream(stream);
	_opticalFlags.writeToStream(stream);
}

void OpticalChip::readFromStream(Common::ReadStream *stream) {
	BiochipItem::readFromStream(stream);
	_opticalFlags.readFromStream(stream);
}

void OpticalChip::addAries() {
	_opticalFlags.setFlag(kOpticalAriesExposed, true);
	setUpOpticalChip();
}

void OpticalChip::addMercury() {
	_opticalFlags.setFlag(kOpticalMercuryExposed, true);
	setUpOpticalChip();
}

void OpticalChip::addPoseidon() {
	_opticalFlags.setFlag(kOpticalPoseidonExposed, true);
	setUpOpticalChip();
}

void OpticalChip::setUpOpticalChip() {
	if (_opticalFlags.getFlag(kOpticalAriesExposed)) {
		if (_opticalFlags.getFlag(kOpticalMercuryExposed)) {
			if (_opticalFlags.getFlag(kOpticalPoseidonExposed))
				setItemState(kOptical111);
			else
				setItemState(kOptical011);
		} else {
			if (_opticalFlags.getFlag(kOpticalPoseidonExposed))
				setItemState(kOptical101);
			else
				setItemState(kOptical001);
		}
	} else {
		if (_opticalFlags.getFlag(kOpticalMercuryExposed)) {
			if (_opticalFlags.getFlag(kOpticalPoseidonExposed))
				setItemState(kOptical110);
			else
				setItemState(kOptical010);
		} else {
			if (_opticalFlags.getFlag(kOpticalPoseidonExposed))
				setItemState(kOptical100);
			else
				setItemState(kOptical000);
		}
	}
}

void OpticalChip::activateOpticalHotspots() {
	if (_opticalFlags.getFlag(kOpticalAriesExposed))
		_ariesHotspot.setActive();
	if (_opticalFlags.getFlag(kOpticalMercuryExposed))
		_mercuryHotspot.setActive();
	if (_opticalFlags.getFlag(kOpticalPoseidonExposed))
		_poseidonHotspot.setActive();
}

void OpticalChip::clickInOpticalHotspot(HotSpotID id) {
	playOpMemMovie(id);
}

void OpticalChip::playOpMemMovie(HotSpotID id) {
	Common::String movieName;
	switch (id) {
	case kAriesSpotID:
		movieName = "Images/AI/Globals/OMAI";
		break;
	case kMercurySpotID:
		movieName = "Images/AI/Globals/OMMI";
		break;
	case kPoseidonSpotID:
		movieName = "Images/AI/Globals/OMPI";
		break;
	}

	ItemState state = getItemState(), newState;
	switch (state) {
	case kOptical001:
		newState = kOptical002;
		break;
	case kOptical010:
		newState = kOptical020;
		break;
	case kOptical011:
		if (id == kAriesSpotID)
			newState = kOptical012;
		else
			newState = kOptical021;
		break;
	case kOptical100:
		newState = kOptical200;
		break;
	case kOptical101:
		if (id == kAriesSpotID)
			newState = kOptical102;
		else
			newState = kOptical201;
		break;
	case kOptical110:
		if (id == kMercurySpotID)
			newState = kOptical120;
		else
			newState = kOptical210;
		break;
	case kOptical111:
		if (id == kAriesSpotID)
			newState = kOptical112;
		else if (id == kMercurySpotID)
			newState = kOptical121;
		else
			newState = kOptical211;
		break;
	case kOptical000: // Can never happen.
	default:
		error("Invalid optical chip state");
	}

	setItemState(newState);

	if (g_AIArea)
		g_AIArea->playAIMovie(kRightAreaSignature, movieName, false, kOpticalInterruption);

	setItemState(state);
}

} // End of namespace Pegasus
