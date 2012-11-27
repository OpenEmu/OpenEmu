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

#ifndef PEGASUS_NEIGHBORHOOD_NORAD_PRESSURETRACKER_H
#define PEGASUS_NEIGHBORHOOD_NORAD_PRESSURETRACKER_H

#include "pegasus/input.h"

namespace Pegasus {

// This class assumes that the globe movie is built at 15 frames per second with a
// time scale of 600, yielding 40 time unit per frame.

enum PressureTrackDirection {
	kTrackPressureUp,
	kTrackPressureDown
};

static const int kPressureDoorTrackInterval = 45;

class PressureDoor;
class Sprite;

class PressureTracker : public Tracker {
public:
	PressureTracker(PressureDoor *);
	virtual ~PressureTracker() {}

	void setTrackParameters(const Hotspot *, Sprite *);
	void continueTracking(const Input &);
	void startTracking(const Input &);
	void stopTracking(const Input &);
	void activateHotspots();
	bool stopTrackingInput(const Input &);

protected:
	void trackPressure();

	PressureDoor *_pressureDoor;
	const Hotspot *_trackSpot;
	Sprite *_trackButton;
	long _trackTime;
};

} // End of namespace Pegasus

#endif
