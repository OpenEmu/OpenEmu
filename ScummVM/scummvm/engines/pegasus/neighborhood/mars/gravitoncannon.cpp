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

#include "pegasus/neighborhood/mars/gravitoncannon.h"
#include "pegasus/neighborhood/mars/robotship.h"
#include "pegasus/neighborhood/mars/spacejunk.h"

namespace Pegasus {

static const TimeValue kGravitonTime = kOneSecond * kShuttleWeaponScale;

static const CoordType kGravitonOriginH = kShuttleWindowLeft - 1;
static const CoordType kGravitonOriginV = kShuttleWindowMidV;

static const float kGravitonXOrigin = convertScreenHToSpaceX(kGravitonOriginH, kGravitonMinDistance);
static const float kGravitonYOrigin = convertScreenVToSpaceY(kGravitonOriginV, kGravitonMinDistance);
static const float kGravitonZOrigin = kGravitonMinDistance;

// Width of graviton sprite...
static const CoordType kGravitonMaxScreenWidth = 78;
static const CoordType kGravitonMaxScreenHeight = 46;

static const float kGravitonWidth = convertScreenHToSpaceX(kShuttleWindowMidH + kGravitonMaxScreenWidth / 2, kGravitonMinDistance)
		- convertScreenHToSpaceX(kShuttleWindowMidH - kGravitonMaxScreenWidth / 2, kGravitonMinDistance);
static const float kGravitonHeight = convertScreenVToSpaceY(kShuttleWindowMidV - kGravitonMaxScreenHeight / 2, kGravitonMinDistance)
		- convertScreenVToSpaceY(kShuttleWindowMidV + kGravitonMaxScreenHeight / 2, kGravitonMinDistance);

GravitonCannon::GravitonCannon() {
	_weaponDuration = kGravitonTime;
	setSegment(0, kGravitonTime);
	_weaponOrigin = Point3D(kGravitonXOrigin, kGravitonYOrigin, kGravitonZOrigin);
	_rightOrigin = Point3D(-kGravitonXOrigin, kGravitonYOrigin, kGravitonZOrigin);
}

void GravitonCannon::initShuttleWeapon() {
	ShuttleWeapon::initShuttleWeapon();
	_gravitonImage.getImageFromPICTFile("Images/Mars/Graviton Cannon");
	_gravitonImage.getSurfaceBounds(_gravitonBounds);
}

void GravitonCannon::cleanUpShuttleWeapon() {
	_gravitonImage.deallocateSurface();
	ShuttleWeapon::cleanUpShuttleWeapon();
}

void GravitonCannon::draw(const Common::Rect &) {
	// Left graviton...
	Point3D pt3D = _weaponLocation;
	pt3D.translate(-kGravitonWidth / 2, kGravitonHeight / 2, 0);
	Common::Point pt2D;
	project3DTo2D(pt3D, pt2D);
	Common::Rect gravitonRect;
	gravitonRect.left = pt2D.x;
	gravitonRect.top = pt2D.y;

	pt3D.translate(kGravitonWidth, -kGravitonHeight, 0);
	project3DTo2D(pt3D, pt2D);
	gravitonRect.right = pt2D.x;
	gravitonRect.bottom = pt2D.y;

	_gravitonImage.scaleTransparentCopy(_gravitonBounds, gravitonRect);

	// Right graviton...
	pt3D = _rightLocation;
	pt3D.translate(-kGravitonWidth / 2, kGravitonHeight / 2, 0);
	project3DTo2D(pt3D, pt2D);
	gravitonRect.left = pt2D.x;
	gravitonRect.top = pt2D.y;

	pt3D.translate(kGravitonWidth, -kGravitonHeight, 0);
	project3DTo2D(pt3D, pt2D);
	gravitonRect.right = pt2D.x;
	gravitonRect.bottom = pt2D.y;

	_gravitonImage.scaleTransparentCopy(_gravitonBounds, gravitonRect);
}

void GravitonCannon::updateWeaponPosition() {
	ShuttleWeapon::updateWeaponPosition();
	if (_weaponTime != 1.0)
		linearInterp(_rightOrigin, _weaponTarget, _weaponTime, _rightLocation);
}

bool GravitonCannon::collisionWithJunk(Common::Point &impactPoint) {
	if (getDisplayOrder() == kShuttleWeaponFrontOrder) {
		Point3D junkPosition;
		g_spaceJunk->getJunkPosition(junkPosition);

		if (junkPosition.z < _weaponLocation.z) {
			setDisplayOrder(kShuttleWeaponBackOrder);
			project3DTo2D(_weaponLocation, impactPoint);

			if (g_spaceJunk->pointInJunk(impactPoint))
				return true;

			project3DTo2D(_rightLocation, impactPoint);
			return g_spaceJunk->pointInJunk(impactPoint);
		}
	}

	return false;
}

void GravitonCannon::hitJunk(Common::Point impactPoint) {
	g_spaceJunk->hitByGravitonCannon(impactPoint);
}

void GravitonCannon::hitShuttle(Common::Point impactPoint) {
	g_robotShip->hitByGravitonCannon(impactPoint);
}

} // End of namespace Pegasus
