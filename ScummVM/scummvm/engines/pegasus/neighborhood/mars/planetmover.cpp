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

#include "pegasus/movie.h"
#include "pegasus/pegasus.h"
#include "pegasus/neighborhood/mars/constants.h"
#include "pegasus/neighborhood/mars/hermite.h"
#include "pegasus/neighborhood/mars/planetmover.h"
#include "pegasus/neighborhood/mars/shuttleenergymeter.h"

namespace Pegasus {

static const TimeScale kRovingScale = kTractorBeamScale;
static const TimeValue kRovingTime = kTenSeconds * kRovingScale;
static const TimeValue kRovingSlop = kTwoSeconds * kRovingScale;

static const CoordType kMaxVelocity = 20;

PlanetMover::PlanetMover() {
	setScale(kRovingScale);
	_dropping = false;
	_planetMovie = 0;
}

void PlanetMover::startMoving(Movie *planetMovie) {
	_planetMovie = planetMovie;
	_p4 = kPlanetStartTop;
	_r4 = ((PegasusEngine *)g_engine)->getRandomNumber(kMaxVelocity - 1);
	if (_r4 + _p4 < kPlanetStopTop)
		_r4 = kPlanetStopTop - _p4;
	newDestination();
}

void PlanetMover::stopMoving() {
	stop();
}

void PlanetMover::dropPlanetOutOfSight() {
	stop();
	CoordType currentLoc = hermite(_p1, _p4, _r1, _r4, _lastTime, _duration);
	CoordType currentV = dHermite(_p1, _p4, _r1, _r4, _lastTime, _duration);
	_p1 = currentLoc;
	_r1 = currentV;
	_p4 = kPlanetStartTop;
	_r4 = 0;
	_duration = kTractorBeamTime - kTractorBeamScale;
	_dropping = true;
	setSegment(0, _duration);
	setTime(0);
	start();
}

void PlanetMover::newDestination() {
	_p1 = _p4;
	_r1 = _r4;

	_p4 = kPlanetStopTop + ((PegasusEngine *)g_engine)->getRandomNumber(kPlanetStartTop - kPlanetStopTop - 1);
	_r4 = ((PegasusEngine *)g_engine)->getRandomNumber(kMaxVelocity - 1);

	if (_r4 + _p4 < kPlanetStopTop)
		_r4 = kPlanetStopTop - _p4;

	stop();
	_duration = kRovingTime + ((PegasusEngine *)g_engine)->getRandomNumber(kRovingSlop - 1);
	setSegment(0, _duration);
	setTime(0);
	start();
}

void PlanetMover::timeChanged(const TimeValue) {
	if (_planetMovie) {
		_planetMovie->moveElementTo(kPlanetStartLeft, hermite(_p1, _p4, _r1, _r4, _lastTime, _duration));
		if (_lastTime == _duration) {
			if (_dropping)
				stop();
			else
				newDestination();
		}
	}
}

} // End of namespace Pegasus
