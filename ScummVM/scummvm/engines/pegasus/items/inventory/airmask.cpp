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

#include "pegasus/gamestate.h"
#include "pegasus/pegasus.h"
#include "pegasus/ai/ai_area.h"
#include "pegasus/items/inventory/airmask.h"
#include "pegasus/neighborhood/neighborhood.h"

namespace Pegasus {

AirMask *g_airMask = 0;

//	Based on full == 100, which is scale used by GetAirLeft().
static const TimeValue kOxygenLowThreshold = 25;

void AirMask::airMaskTimerExpired() {
	if (g_neighborhood)
		g_neighborhood->checkAirMask();
}

AirMask::AirMask(const ItemID id, const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant direction) :
		InventoryItem(id, neighborhood, room, direction), _toggleSpot(kAirMaskToggleSpotID) {
	g_airMask = this;
	_toggleSpot.setArea(Common::Rect(kAIMiddleAreaLeft + 10, kAIMiddleAreaTop + 17, kAIMiddleAreaLeft + 110, kAIMiddleAreaTop + 57));
	_toggleSpot.setHotspotFlags(kAirMaskSpotFlag);
	g_allHotspots.push_back(&_toggleSpot);
	setItemState(kAirMaskEmptyOff);
	_oxygenTimer.primeFuse(0);
	_oxygenTimer.setFunctor(new Common::Functor0Mem<void, AirMask>(this, &AirMask::airMaskTimerExpired));
}

AirMask::~AirMask() {
	g_allHotspots.removeOneHotspot(kAirMaskToggleSpotID);
	g_airMask = 0;
}

void AirMask::writeToStream(Common::WriteStream *stream) {
	InventoryItem::writeToStream(stream);
	stream->writeUint32BE(_oxygenTimer.getTimeRemaining());
}

void AirMask::readFromStream(Common::ReadStream *stream) {
	_oxygenTimer.stopFuse();
	InventoryItem::readFromStream(stream);
	_oxygenTimer.primeFuse(stream->readUint32BE());
}

void AirMask::putMaskOn() {
	AirQuality airQuality;

	if (g_neighborhood)
		airQuality = g_neighborhood->getAirQuality(GameState.getCurrentRoom());
	else
		airQuality = kAirQualityGood;

	uint airLevel = getAirLeft();
	ItemState newState = getItemState();
	ItemState oldState = newState;

	if (airLevel == 0) {
		newState = kAirMaskEmptyFilter;
	} else if (airLevel <= kOxygenLowThreshold) {
		if (airQuality == kAirQualityVacuum)
			newState = kAirMaskLowOn;
		else
			newState = kAirMaskLowFilter;
	} else {
		if (airQuality == kAirQualityVacuum)
			newState = kAirMaskFullOn;
		else
			newState = kAirMaskFullFilter;
	}

	if (newState != oldState)
		setItemState(newState);
}

void AirMask::takeMaskOff() {
	uint airLevel = getAirLeft();
	ItemState newState = getItemState();
	ItemState oldState = newState;

	if (airLevel == 0)
		newState = kAirMaskEmptyOff;
	else if (airLevel <= kOxygenLowThreshold)
		newState = kAirMaskLowOff;
	else
		newState = kAirMaskFullOff;

	if (newState != oldState)
		setItemState(newState);
}

void AirMask::toggleItemState() {
	if (isAirMaskInUse())
		takeMaskOff();
	else
		putMaskOn();
}

void AirMask::airQualityChanged() {
	if (isAirMaskInUse())
		putMaskOn();
	else
		takeMaskOff();
}

void AirMask::setItemState(const ItemState newState) {
	if (newState != getItemState()) {
		InventoryItem::setItemState(newState);

		switch (newState) {
		case kAirMaskFullOn:
		case kAirMaskLowOn:
			if (!_oxygenTimer.isFuseLit()) {
				_oxygenTimer.lightFuse();
				startIdling();
			}
			break;
		default:
			if (_oxygenTimer.isFuseLit()) {
				_oxygenTimer.stopFuse();
				stopIdling();
			}
			break;
		}

		if (g_neighborhood)
			g_neighborhood->checkAirMask();

		g_AIArea->checkMiddleArea();
	}
}

void AirMask::useIdleTime() {
	if (getAirLeft() == 0)
		setItemState(kAirMaskEmptyOff);
	else if (getAirLeft() <= kOxygenLowThreshold)
		setItemState(kAirMaskLowOn);
}

void AirMask::refillAirMask() {
	switch (getItemState()) {
		case kAirMaskEmptyOff:
		case kAirMaskLowOff:
			setItemState(kAirMaskFullOff);
			break;
		case kAirMaskEmptyFilter:
		case kAirMaskLowFilter:
			setItemState(kAirMaskFullFilter);
			break;
		case kAirMaskLowOn:
			setItemState(kAirMaskFullOn);
			break;
	}

	if (_oxygenTimer.isFuseLit()) {
		_oxygenTimer.stopFuse();
		_oxygenTimer.primeFuse(kOxyMaskFullTime);
		_oxygenTimer.lightFuse();
	} else {
		_oxygenTimer.primeFuse(kOxyMaskFullTime);
	}
}

//	Doesn't return 0 until the timer is actually at 0.
uint AirMask::getAirLeft() {
	return CLIP<int>(((_oxygenTimer.getTimeRemaining() * 100) + kOxyMaskFullTime - 1) / kOxyMaskFullTime, 0, 100);
}

bool AirMask::isAirMaskInUse() {
	switch (getItemState()) {
	case kAirMaskEmptyOff:
	case kAirMaskLowOff:
	case kAirMaskFullOff:
		return false;
		break;
	default:
		return true;
		break;
	}
}

bool AirMask::isAirMaskOn() {
	switch (getItemState()) {
	case kAirMaskLowOn:
	case kAirMaskFullOn:
		return true;
		break;
	default:
		return false;
		break;
	}
}

bool AirMask::isAirFilterOn() {
	switch (getItemState()) {
	case kAirMaskEmptyFilter:
	case kAirMaskLowFilter:
	case kAirMaskFullFilter:
		return true;
		break;
	default:
		return false;
		break;
	}
}

void AirMask::addedToInventory() {
	GameState.setMarsMaskOnFiller(false);
}

void AirMask::removedFromInventory() {
	if (isAirMaskInUse())
		toggleItemState();
}

void AirMask::activateAirMaskHotspots() {
	_toggleSpot.setActive();
}

void AirMask::clickInAirMaskHotspot() {
	toggleItemState();
}

} // End of namespace Pegasus
