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

#ifndef PEGASUS_NEIGHBORHOOD_MARS_SPACECHASE3D_H
#define PEGASUS_NEIGHBORHOOD_MARS_SPACECHASE3D_H

#include "pegasus/neighborhood/mars/constants.h"

namespace Pegasus {

// This is approximately right for a field of view of 72 degrees
// (Should be set to the tangent of FOV).
//static const float kTangentFOV = 0.76254;
static const float kTangentFOV = 1.0;

// Define these as macros and they can be used to define constants...
#define convertSpaceXToScreenH(x, z) \
	((x) / (z) * (kScreenWidth / (2 * kTangentFOV)) + kShuttleWindowMidH)

#define convertSpaceYToScreenV(y, z) \
	(kShuttleWindowMidV - (y) / (z) * (kScreenWidth / (2 * kTangentFOV)))

#define convertScreenHToSpaceX(x, d) \
	(((2.0 * kTangentFOV) / kScreenWidth) * ((float)(x) - kShuttleWindowMidH) * (d))

#define convertScreenVToSpaceY(y, d) \
	(((2.0 * kTangentFOV) / kScreenWidth) *	((float)kShuttleWindowMidV - (y)) * (d))

struct Point3D {
	float x, y, z;

	Point3D() : x(0), y(0), z(0) {}
	Point3D(float x1, float y1, float z1) : x(x1), y(y1), z(z1) {}
	bool operator==(const Point3D &p) const { return x == p.x && y == p.y && z == p.z; }
	bool operator!=(const Point3D &p) const { return x != p.x || y != p.y || z != p.z; }

	void translate(float dx, float dy, float dz) {
		x += dx;
		y += dy;
		z += dz;
	}
};

static const int kScreenWidth = kShuttleWindowWidth;

bool isNegative(int a);
bool isPositive(int a);
int sign(int a);
bool sameSign(int a, int b);

void project3DTo2D(const Point3D &pt3D, Common::Point &pt2D);
void project2DTo3D(const Common::Point &pt2D, const float screenDistance, Point3D &pt3D);

void linearInterp(const Point3D &pt1, const Point3D &pt2, const float t, Point3D &pt3);
void linearInterp(const Point3D &pt1, const float x2, const float y2, const float z2, const float t, Point3D &pt3);
void linearInterp(const float x1, const float y1, const float z1, const Point3D &pt2, const float t, Point3D &pt3);
void linearInterp(const float x1, const float y1, const float z1, const float x2,
		const float y2, const float z2, const float t, Point3D &pt3);

void linearInterp(const Common::Point &pt1, const Common::Point &pt2, const float t, Common::Point &pt3);
void linearInterp(const Common::Point &pt1, const float h2, const float v2, const float t, Common::Point &pt3);
void linearInterp(const float h1, const float v1, const Common::Point &pt2, const float t, Common::Point &pt3);
void linearInterp(const float h1, const float v1, const float h2, const float v2, const float t, Common::Point &pt3);

float linearInterp(const float arg1, const float arg2, const float t);

} // End of namespace Pegasus

#endif
