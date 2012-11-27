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
#include "pegasus/neighborhood/mars/mars.h"
#include "pegasus/neighborhood/mars/spacejunk.h"

namespace Pegasus {

static const CoordType kMaxBounceSize = 90;
static const CoordType kBounceTargetHRange = 640 - kMaxBounceSize - 2;
static const CoordType kBounceTargetVRange = 480 - kMaxBounceSize - 2;

static const float kJunkXTarget = 0;
static const float kJunkYTarget = 0;
static const float kJunkZTarget = kJunkMinDistance;

SpaceJunk *g_spaceJunk = 0;

SpaceJunk::SpaceJunk(const DisplayElementID id) : ScalingMovie(id) {
	_timer.setScale(kJunkTimeScale);
	_bouncing = false;
	g_spaceJunk = this;
}

SpaceJunk::~SpaceJunk() {
	g_spaceJunk = 0;
}

void SpaceJunk::launchJunk(int16 whichJunk, CoordType xOrigin, CoordType yOrigin) {
	_bouncing = false;
	TimeValue startTime = whichJunk * 16 * 40;
	TimeValue stopTime = startTime + 16 * 40;

	_launchPoint = Point3D(convertScreenHToSpaceX(xOrigin, kJunkMaxDistance),
			convertScreenVToSpaceY(yOrigin, kJunkMaxDistance), kJunkMaxDistance);
	startIdling();
	stop();
	setFlags(0);
	setSegment(startTime, stopTime);
	setFlags(kLoopTimeBase);
	setTime(startTime);
	start();
	show();
	_timer.stop();
	_timer.setSegment(0, kJunkTravelTime);
	_timer.setTime(0);

	// Force it to set up correctly from the get-go
	useIdleTime();

	_timer.start();
}

void SpaceJunk::setCenter(const CoordType centerX, const CoordType centerY) {
	_center.x = centerX;
	_center.y = centerY;

	Common::Rect r;
	getBounds(r);
	r.moveTo(CLIP<int>(centerX - (r.width() >> 1), 0, 640 - r.width()), CLIP<int>(centerY - (r.height() >> 1), 0, 480 - r.height()));
	setBounds(r);
}

void SpaceJunk::setScaleSize(const CoordType size) {
	Common::Rect r;
	r.left = _center.x - (size >> 1);
	r.top = _center.y - (size >> 1);
	r.right = r.left + size;
	r.bottom = r.top + size;
	setBounds(r);
}

void SpaceJunk::useIdleTime() {
	if (_bouncing) {
		TimeValue time = _timer.getTime();
		Common::Point pt;
		pt.x = linearInterp(0, _bounceTime, time, _bounceStart.x, _bounceStop.x);
		pt.y = linearInterp(0, _bounceTime, time, _bounceStart.y, _bounceStop.y);
		CoordType size = linearInterp(0, _bounceTime, time, _bounceSizeStart, _bounceSizeStop);
		setCenter(pt.x, pt.y);
		setScaleSize(size);

		if (time == _bounceTime) {
			stop();
			stopIdling();
			hide();
			((Mars *)g_neighborhood)->setUpNextDropTime();
		}
	} else {
		float t = (float)_timer.getTime() / kJunkTravelTime;
		linearInterp(_launchPoint, kJunkXTarget, kJunkYTarget, kJunkZTarget, t, _junkPosition);

		Common::Point pt2D;
		project3DTo2D(_junkPosition, pt2D);
		setCenter(pt2D.x, pt2D.y);
		setScaleSize((int)(convertSpaceYToScreenV(_junkPosition.y - kJunkSize / 2, _junkPosition.z) -
				convertSpaceYToScreenV(_junkPosition.y + kJunkSize / 2, _junkPosition.z)));

		if (t == 1.0) {
			rebound(kCollisionReboundTime);
			((Mars *)g_neighborhood)->hitByJunk();
		}
	}
}

bool SpaceJunk::pointInJunk(const Common::Point &pt) {
	Common::Rect r;
	getBounds(r);

	int dx = r.width() / 4;
	int dy = r.height() / 4;

	r.left += dx;
	r.right -= dx;
	r.top += dy;
	r.top -= dy;

	return r.contains(pt);
}

void SpaceJunk::rebound(const TimeValue reboundTime) {
	Common::Rect bounds;
	getBounds(bounds);

	_bounceStart.x = (bounds.left + bounds.right) >> 1;
	_bounceStart.y = (bounds.top + bounds.bottom) >> 1;

	PegasusEngine *vm = (PegasusEngine *)g_engine;

	switch (vm->getRandomNumber(3)) {
	case 0:
		_bounceStop.x = kMaxBounceSize / 2 + 1 + vm->getRandomNumber(kBounceTargetHRange - 1);
		_bounceStop.y = kMaxBounceSize / 2 + 1;
		break;
	case 1:
		_bounceStop.x = kMaxBounceSize / 2 + 1 + vm->getRandomNumber(kBounceTargetHRange - 1);
		_bounceStop.y = 480 - kMaxBounceSize / 2 + 1;
		break;
	case 2:
		_bounceStop.x = kMaxBounceSize / 2 + 1;
		_bounceStop.y = kMaxBounceSize / 2 + 1 + vm->getRandomNumber(kBounceTargetVRange - 1);
		break;
	case 3:
		_bounceStop.x = 640 - kMaxBounceSize / 2 + 1;
		_bounceStop.y = kMaxBounceSize / 2 + 1 + vm->getRandomNumber(kBounceTargetVRange - 1);
		break;
	}

	_bounceSizeStart = bounds.width();
	_bounceSizeStop = MIN(_bounceSizeStart, kMaxBounceSize);

	_timer.stop();
	_timer.setSegment(0, reboundTime);
	_bounceTime = reboundTime;
	_timer.setTime(0);
	_timer.start();

	_bouncing = true;
}

void SpaceJunk::hitByEnergyBeam(Common::Point) {
	rebound(kWeaponReboundTime);
	setGlowing(true);
	((PegasusEngine *)g_engine)->delayShell(1, 3);
	setGlowing(false);
}

void SpaceJunk::hitByGravitonCannon(Common::Point impactPoint) {
	stop();
	stopIdling();
	hide();

	Common::Rect r;
	getBounds(r);
	r = Common::Rect::center(impactPoint.x, impactPoint.y, r.width(), r.height());

	((Mars *)g_neighborhood)->showBigExplosion(r, kShuttleJunkOrder);
	((Mars *)g_neighborhood)->setUpNextDropTime();
}

void SpaceJunk::getJunkPosition(Point3D &position) {
	position = _junkPosition;
}

bool SpaceJunk::isJunkFlying() {
	return isIdling();
}

} // End of namespace Pegasus
