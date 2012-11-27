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

#ifndef PEGASUS_NEIGHBORHOOD_MARS_ROBOTSHIP_H
#define PEGASUS_NEIGHBORHOOD_MARS_ROBOTSHIP_H

#include "pegasus/movie.h"

namespace Pegasus {

static const CoordType kShuttleMovieWidth = 114;
static const CoordType kShuttleMovieHeight = 42;

class RobotShip : IdlerTimeBase {
public:
	RobotShip();
	virtual ~RobotShip();

	void initRobotShip();
	void cleanUpRobotShip();

	void startMoving();

	void killRobotShip();

	bool pointInShuttle(Common::Point&);

	void hitByEnergyBeam(Common::Point impactPoint);
	void hitByGravitonCannon(Common::Point impactPoint);

	void getShuttleBounds(Common::Rect &r) { _spritesMovie.getBounds(r); }

	void setGlowing(const bool glowing) { _spritesMovie.setGlowing(glowing); }

	void snareByTractorBeam();
	bool isSnared() { return _snaring && getTime() == _duration; }

	bool isDead() { return _dead; }

	void setUpNextDropTime();

protected:
	void newDestination();
	void moveRobotTo(CoordType, CoordType);
	void timeToDropJunk();
	virtual void timeChanged(const TimeValue);
	void makeVelocityVector(CoordType, CoordType, CoordType, CoordType, Common::Point &);

	GlowingMovie _spritesMovie;
	Common::Rect _shipRange;
	int _shipWidth, _shipHeight;
	Common::Point _p1, _p4, _r1, _r4, _currentLocation;
	FuseFunction _dropJunkFuse;
	TimeValue _duration;
	bool _snaring, _dead;
};

extern RobotShip *g_robotShip;

} // End of namespace Pegasus

#endif
