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

#include "pegasus/neighborhood/mars/constants.h"
#include "pegasus/neighborhood/mars/robotship.h"
#include "pegasus/neighborhood/mars/shuttleweapon.h"
#include "pegasus/neighborhood/mars/spacejunk.h"

namespace Pegasus {

ShuttleWeapon::ShuttleWeapon() : IdlerAnimation(kNoDisplayElement) {
	setScale(kShuttleWeaponScale);
	_weaponDuration = kShuttleWeaponScale * 2;
	setSegment(0, _weaponDuration);
	setBounds(kShuttleWindowLeft, kShuttleWindowTop, kShuttleWindowLeft + kShuttleWindowWidth,
			kShuttleWindowTop + kShuttleWindowHeight);
	setDisplayOrder(kShuttleWeaponFrontOrder);
}

void ShuttleWeapon::initShuttleWeapon() {
	startDisplaying();
}

void ShuttleWeapon::cleanUpShuttleWeapon() {
	stop();
	hide();
	stopDisplaying();
}

bool ShuttleWeapon::canFireWeapon() {
	return !isRunning();
}

void ShuttleWeapon::fireWeapon(const CoordType hStop, const CoordType vStop) {
	if (!isRunning()) {
		stop();
		setTime(0);
		show();

		Common::Point pt2D(hStop, vStop);
		project2DTo3D(pt2D, kShuttleDistance, _weaponTarget);
		_weaponTime = 0;
		setDisplayOrder(kShuttleWeaponFrontOrder);
		start();
	}
}

void ShuttleWeapon::updateWeaponPosition() {
	_weaponTime = (float)_lastTime / _weaponDuration;
	linearInterp(_weaponOrigin, _weaponTarget, _weaponTime, _weaponLocation);

	if (_weaponTime == 1.0) {
		stop();
		hide();
	} else {
		triggerRedraw();
	}
}

void ShuttleWeapon::timeChanged(const TimeValue) {
	updateWeaponPosition();

	bool hit = false;
	Common::Point impactPoint;

	if (g_spaceJunk->isJunkFlying()) {
		hit = collisionWithJunk(impactPoint);
		if (hit) {
			stop();
			hide();
			hitJunk(impactPoint);
		}
	}

	if (!hit && _weaponTime == 1.0 && collisionWithShuttle(impactPoint))
		hitShuttle(impactPoint);
}

bool ShuttleWeapon::collisionWithJunk(Common::Point &impactPoint) {
	if (getDisplayOrder() == kShuttleWeaponFrontOrder) {
		Point3D junkPosition;
		g_spaceJunk->getJunkPosition(junkPosition);

		if (junkPosition.z < _weaponLocation.z) {
			setDisplayOrder(kShuttleWeaponBackOrder);
			project3DTo2D(_weaponLocation, impactPoint);
			return g_spaceJunk->pointInJunk(impactPoint);
		}
	}

	return false;
}

bool ShuttleWeapon::collisionWithShuttle(Common::Point &impactPoint) {
	project3DTo2D(_weaponLocation, impactPoint);
	return g_robotShip->pointInShuttle(impactPoint);
}

void ShuttleWeapon::hitJunk(Common::Point impactPoint) {
	g_spaceJunk->hitByEnergyBeam(impactPoint);
}

void ShuttleWeapon::hitShuttle(Common::Point impactPoint) {
	g_robotShip->hitByEnergyBeam(impactPoint);
}

} // End of namespace Pegasus
