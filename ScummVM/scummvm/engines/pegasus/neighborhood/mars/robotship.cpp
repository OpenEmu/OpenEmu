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
#include "pegasus/neighborhood/mars/hermite.h"
#include "pegasus/neighborhood/mars/mars.h"
#include "pegasus/neighborhood/mars/robotship.h"
#include "pegasus/neighborhood/mars/spacechase3d.h"
#include "pegasus/neighborhood/mars/spacejunk.h"

namespace Pegasus {

static const TimeScale kRovingScale = kTractorBeamScale;
static const TimeValue kRovingTime = kSixSeconds * kRovingScale;
static const TimeValue kRovingSlop = kThreeSeconds * kRovingScale;

static const int kNumSpriteColumns = 15;
static const int kNumSpriteRows = 16;

static const CoordType kInitialLocationLeft = kShuttleWindowLeft - 50;
static const CoordType kInitialLocationTop = kShuttleWindowTop - 50;
static const CoordType kInitialLocationWidth = kShuttleWindowWidth + 100;
static const CoordType kInitialLocationHeight = kShuttleWindowHeight + 100;

static const CoordType kVelocityVectorLength = 100;
static const CoordType kVelocityVectorSlop = 50;

static const CoordType kRovingLeft = kShuttleWindowLeft + 20;
static const CoordType kRovingTop = kShuttleWindowTop + 20;
static const CoordType kRovingWidth = kShuttleWindowMidH - kRovingLeft;
static const CoordType kRovingHeight = kShuttleWindowMidV - kRovingTop;

RobotShip *g_robotShip = 0;

RobotShip::RobotShip() : _spritesMovie(kNoDisplayElement) {
	g_robotShip = this;
	_shipRange = Common::Rect(kShuttleWindowLeft, kShuttleWindowTop, kShuttleWindowLeft + kShuttleWindowWidth,
			kShuttleWindowTop + kShuttleWindowHeight);
	setScale(kRovingScale);
	_currentLocation.x = 0;
	_currentLocation.y = 0;
	_snaring = false;
	_dropJunkFuse.setFunctor(new Common::Functor0Mem<void, RobotShip>(this, &RobotShip::timeToDropJunk));
	_duration = 0xFFFFFFFF;
}

RobotShip::~RobotShip() {
	g_robotShip = 0;
}

void RobotShip::initRobotShip() {
	_spritesMovie.initFromMovieFile("Images/Mars/Ship.movie", true);
	_spritesMovie.setDisplayOrder(kShuttleRobotShipOrder);
	_spritesMovie.moveElementTo(kPlanetStartLeft, kPlanetStartTop);
	_spritesMovie.startDisplaying();
	_spritesMovie.show();

	Common::Rect r;
	_spritesMovie.getBounds(r);
	_shipWidth = r.width();
	_shipHeight = r.height();
	_dead = false;
}

void RobotShip::cleanUpRobotShip() {
	_dropJunkFuse.stopFuse();
	_spritesMovie.stopDisplaying();
	_spritesMovie.releaseMovie();
}

void RobotShip::startMoving() {
	if (((PegasusEngine *)g_engine)->getRandomBit()) {
		_p4.x = kInitialLocationLeft + ((PegasusEngine *)g_engine)->getRandomNumber(kInitialLocationWidth - 1);
		if (((PegasusEngine *)g_engine)->getRandomBit())
			_p4.y = kInitialLocationTop;
		else
			_p4.y = kInitialLocationTop + kInitialLocationHeight;
	} else {
		_p4.y = kInitialLocationTop + ((PegasusEngine *)g_engine)->getRandomNumber(kInitialLocationHeight - 1);
		if (((PegasusEngine *)g_engine)->getRandomBit())
			_p4.x = kInitialLocationLeft;
		else
			_p4.x = kInitialLocationLeft + kInitialLocationWidth;
	}

	makeVelocityVector(_p4.x, _p4.y, kShuttleWindowLeft + kShuttleWindowWidth / 2,
			kShuttleWindowTop + kShuttleWindowHeight / 2, _r4);
	newDestination();
	setUpNextDropTime();
}

void RobotShip::killRobotShip() {
	cleanUpRobotShip();
	_dead = true;
}

void RobotShip::setUpNextDropTime() {
	if (!isSnared()) {
		_dropJunkFuse.primeFuse(kJunkDropBaseTime + ((PegasusEngine *)g_engine)->getRandomNumber(kJunkDropSlopTime));
		_dropJunkFuse.lightFuse();
	}
}

void RobotShip::timeToDropJunk() {
	if (g_spaceJunk) {
		CoordType x, y;
		_spritesMovie.getCenter(x, y);
		g_spaceJunk->launchJunk(((PegasusEngine *)g_engine)->getRandomNumber(24), x, y);
	}
}

void RobotShip::newDestination() {
	_p1 = _p4;
	_r1 = _r4;

	_p4.x = kRovingLeft + ((PegasusEngine *)g_engine)->getRandomNumber(kRovingWidth - 1);
	_p4.y = kRovingTop + ((PegasusEngine *)g_engine)->getRandomNumber(kRovingHeight - 1);

	if (((PegasusEngine *)g_engine)->getRandomNumber(7) < 6) {
		if (!sameSign(_p4.x - kShuttleWindowMidH, kShuttleWindowMidH - _p1.x)) {
			if (sign(_p4.x - kShuttleWindowMidH) > 0)
				_p4.x -= kRovingWidth;
			else
				_p4.x += kRovingWidth;
		}
	}

	if (((PegasusEngine *)g_engine)->getRandomNumber(7) < 6) {
		if (!sameSign(_p4.y - kShuttleWindowMidV, kShuttleWindowMidV - _p1.y)) {
			if (sign(_p4.y - kShuttleWindowMidV) > 0)
				_p4.y -= kRovingHeight;
			else
				_p4.y += kRovingHeight;
		}
	}

	makeVelocityVector(_p4.x, _p4.y, kShuttleWindowLeft + kShuttleWindowWidth / 2,
			kShuttleWindowTop + kShuttleWindowHeight / 2, _r4);
	stop();
	_duration = kRovingTime + ((PegasusEngine *)g_engine)->getRandomNumber(kRovingSlop - 1);
	setSegment(0, _duration);
	setTime(0);
	start();
}

void RobotShip::moveRobotTo(CoordType x, CoordType y) {
	_currentLocation.x = x;
	_currentLocation.y = y;

	if (_spritesMovie.isMovieValid()) {
		_spritesMovie.moveElementTo(x - (_shipWidth >> 1), y - (_shipHeight >> 1));

		if (x < _shipRange.left)
			x = 0;
		else if (x > _shipRange.right - 1)
			x = _shipRange.width() - 1;
		else
			x -= _shipRange.left;

		if (y < _shipRange.top)
			y = 0;
		else if (y > _shipRange.bottom - 1)
			y = _shipRange.height() - 1;
		else
			y -= _shipRange.top;

		x = kNumSpriteColumns * x / _shipRange.width();
		y = kNumSpriteRows * y / _shipRange.height();

		_spritesMovie.setTime(40 * (x + y * kNumSpriteColumns));
		_spritesMovie.redrawMovieWorld();
	}
}

bool RobotShip::pointInShuttle(Common::Point &pt) {
	Common::Rect r;
	_spritesMovie.getBounds(r);

	int dx = r.width() / 4;
	int dy = r.height() / 6;

	r.left += dx;
	r.right -= dx;
	r.top += dy;
	r.bottom -= dy;

	return r.contains(pt);
}

void RobotShip::hitByEnergyBeam(Common::Point impactPoint) {
	((Mars *)g_neighborhood)->decreaseRobotShuttleEnergy(1, impactPoint);
	setGlowing(true);
	((PegasusEngine *)g_engine)->delayShell(1, 3);
	setGlowing(false);
}

void RobotShip::hitByGravitonCannon(Common::Point impactPoint) {
	GameState.setMarsHitRobotWithCannon(true);
	((Mars *)g_neighborhood)->decreaseRobotShuttleEnergy(6, impactPoint);
}

void RobotShip::snareByTractorBeam() {
	_dropJunkFuse.stopFuse();
	stop();

	Common::Point currentV;
	dHermite(_p1, _p4, _r1, _r4, _lastTime, _duration, currentV);

	_p1 = _currentLocation;
	_r1 = currentV;
	_p4.x = kShuttleWindowMidH;
	_p4.y = kShuttleWindowMidV;
	_r4.x = 0;
	_r4.y = 0;
	_duration = kTractorBeamTime;
	_snaring = true;
	setSegment(0, _duration);
	setTime(0);
	start();
}

void RobotShip::timeChanged(const TimeValue) {
	Common::Point newLocation;
	hermite(_p1, _p4, _r1, _r4, _lastTime, _duration, newLocation);
	moveRobotTo(newLocation.x, newLocation.y);

	if (_lastTime == _duration) {
		if (_snaring)
			stop();
		else
			newDestination();
	}
}

void RobotShip::makeVelocityVector(CoordType x1, CoordType y1, CoordType x2, CoordType y2, Common::Point &vector) {
	CoordType length = ((PegasusEngine *)g_engine)->getRandomNumber(kVelocityVectorSlop - 1) + kVelocityVectorLength;
	vector.x = x2 - x1;
	vector.y = y2 - y1;
	float oldLength = sqrt((float)(vector.x * vector.x + vector.y * vector.y));
	vector.x = (int)(vector.x * length / oldLength);
	vector.y = (int)(vector.y * length / oldLength);
}

} // End of namespace Pegasus
