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

#ifndef PEGASUS_NEIGHBORHOOD_MARS_SHUTTLEWEAPON_H
#define PEGASUS_NEIGHBORHOOD_MARS_SHUTTLEWEAPON_H

#include "pegasus/elements.h"
#include "pegasus/neighborhood/mars/spacechase3d.h"

namespace Pegasus {

// Can fire multiple times?
// For now, no...
// clone2727 adds: And now forever

static const TimeScale kShuttleWeaponScale = kFifteenTicksPerSecond;

class ShuttleWeapon : public IdlerAnimation {
public:
	ShuttleWeapon();
	virtual ~ShuttleWeapon() {}

	virtual void initShuttleWeapon();
	virtual void cleanUpShuttleWeapon();

	virtual void fireWeapon(const CoordType, const CoordType);

	bool canFireWeapon();

protected:
	void timeChanged(const TimeValue);
	virtual void updateWeaponPosition();
	virtual bool collisionWithJunk(Common::Point &impactPoint);
	bool collisionWithShuttle(Common::Point &impactPoint);
	virtual void hitJunk(Common::Point impactPoint);
	virtual void hitShuttle(Common::Point impactPoint);

	Point3D _weaponOrigin, _weaponTarget;
	Point3D _weaponLocation;
	float _weaponTime;
	TimeValue _weaponDuration;
};

} // End of namespace Pegasus

#endif
