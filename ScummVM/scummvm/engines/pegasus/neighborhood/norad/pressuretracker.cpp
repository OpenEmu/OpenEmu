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

#include "pegasus/hotspot.h"
#include "pegasus/pegasus.h"
#include "pegasus/neighborhood/norad/pressuredoor.h"
#include "pegasus/neighborhood/norad/pressuretracker.h"

namespace Pegasus {

PressureTracker::PressureTracker(PressureDoor *pressureDoor) {
	_pressureDoor = pressureDoor;
	_trackSpot = 0;
	_trackTime = 0;
}

void PressureTracker::setTrackParameters(const Hotspot *trackSpot, Sprite *trackButton) {
	_trackSpot = trackSpot;
	_trackButton = trackButton;
	_trackTime = 0;
}

void PressureTracker::activateHotspots() {
	Tracker::activateHotspots();

	if (_trackSpot)
		g_allHotspots.activateOneHotspot(_trackSpot->getObjectID());
}

// For click-hold dragging.
bool PressureTracker::stopTrackingInput(const Input &input) {
	return !JMPPPInput::isPressingInput(input);
}

void PressureTracker::continueTracking(const Input &input) {
	Common::Point where;
	input.getInputLocation(where);

	if (g_allHotspots.findHotspot(where) == _trackSpot) {
		trackPressure();
		_trackButton->setCurrentFrameIndex(1);
	} else {
		_trackButton->setCurrentFrameIndex(0);
	}
}

void PressureTracker::startTracking(const Input &input) {
	Tracker::startTracking(input);
	trackPressure();
}

void PressureTracker::stopTracking(const Input &input) {
	_trackButton->setCurrentFrameIndex(0);
	_pressureDoor->stopChangingPressure();
	Tracker::stopTracking(input);
}

void PressureTracker::trackPressure() {
	if (g_system->getMillis() - _trackTime > kPressureDoorTrackInterval * 1000 / 60) {
		_pressureDoor->incrementPressure(_trackSpot->getObjectID());
		_trackTime = g_system->getMillis();
	}
}

} // End of namespace Pegasus
