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
#include "pegasus/neighborhood/mars/energybeam.h"

namespace Pegasus {

static const TimeValue kEnergyBeamTime = kOneSecond * kShuttleWeaponScale / 2;

static const CoordType kEnergyBeamOriginH = kShuttleWindowMidH;
static const CoordType kEnergyBeamOriginV = kShuttleWindowTop + kShuttleWindowHeight;

static const float kBeamXOrigin = convertScreenHToSpaceX(kEnergyBeamOriginH, kEnergyBeamMinDistance);
static const float kBeamYOrigin = convertScreenVToSpaceY(kEnergyBeamOriginV, kEnergyBeamMinDistance);
static const float kBeamZOrigin = kEnergyBeamMinDistance;

EnergyBeam::EnergyBeam() {
	_weaponDuration = kEnergyBeamTime;
	setSegment(0, kEnergyBeamTime);
	_weaponOrigin = Point3D(kBeamXOrigin, kBeamYOrigin, kBeamZOrigin);
}

void EnergyBeam::draw(const Common::Rect &) {
	static const int kBeamColorRed1 = 224;
	static const int kBeamColorRed2 = 64;

	Graphics::Surface *surface = ((PegasusEngine *)g_engine)->_gfx->getWorkArea();

	byte red = linearInterp(0, kEnergyBeamTime, _lastTime, kBeamColorRed1, kBeamColorRed2);
	uint32 color = surface->format.RGBToColor(red, 0, 0);

	Point3D startPoint;
	if (_weaponTime < 0.1)
		startPoint = _weaponOrigin;
	else
		linearInterp(_weaponOrigin, _weaponTarget, _weaponTime - 0.1, startPoint);

	Common::Point lineStart;
	project3DTo2D(startPoint, lineStart);

	Common::Point lineEnd;
	project3DTo2D(_weaponLocation, lineEnd);

	surface->drawThickLine(lineStart.x, lineStart.y, lineEnd.x, lineEnd.y, 2, 1, color);
}

} // End of namespace Pegasus
