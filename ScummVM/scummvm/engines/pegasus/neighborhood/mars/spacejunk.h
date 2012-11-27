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

#ifndef PEGASUS_NEIGHBORHOOD_MARS_SPACEJUNK_H
#define PEGASUS_NEIGHBORHOOD_MARS_SPACEJUNK_H

#include "pegasus/movie.h"
#include "pegasus/neighborhood/mars/constants.h"
#include "pegasus/neighborhood/mars/spacechase3d.h"

namespace Pegasus {

static const CoordType kJunkMaxScreenSize = 250;

static const float kJunkSize = convertScreenVToSpaceY(kShuttleWindowMidV - kJunkMaxScreenSize / 2, kJunkMinDistance) -
		convertScreenVToSpaceY(kShuttleWindowMidV + kJunkMaxScreenSize / 2, kJunkMinDistance);

class SpaceJunk : public ScalingMovie, public Idler {
public:
	SpaceJunk(const DisplayElementID);
	virtual ~SpaceJunk();

	void setCenter(const CoordType, const CoordType);
	void setScaleSize(const CoordType);

	void useIdleTime();

	void launchJunk(int16, CoordType, CoordType);

	void getJunkPosition(Point3D &);
	bool isJunkFlying();

	bool pointInJunk(const Common::Point &);

	void hitByEnergyBeam(Common::Point impactPoint);
	void hitByGravitonCannon(Common::Point impactPoint);

	bool junkFlying() { return _timer.isRunning(); }

protected:
	void rebound(const TimeValue);

	TimeBase _timer;
	Point3D _launchPoint, _junkPosition;
	Common::Point _center;
	bool _bouncing;
	Common::Point _bounceStart, _bounceStop;
	CoordType _bounceSizeStart, _bounceSizeStop;
	TimeValue _bounceTime;
};

extern SpaceJunk *g_spaceJunk;

} // End of namespace Pegasus

#endif
