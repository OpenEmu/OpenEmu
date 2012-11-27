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

#include "pegasus/neighborhood/mars/spacechase3d.h"

namespace Pegasus {

void project3DTo2D(const Point3D &pt3D, Common::Point &pt2D) {
	pt2D.x = (int)convertSpaceXToScreenH(pt3D.x, pt3D.z);
	pt2D.y = (int)convertSpaceYToScreenV(pt3D.y, pt3D.z);
}

void project2DTo3D(const Common::Point &pt2D, const float screenDistance, Point3D &pt3D) {
	pt3D.x = convertScreenHToSpaceX(pt2D.x, screenDistance);
	pt3D.y = convertScreenVToSpaceY(pt2D.y, screenDistance);
	pt3D.z = screenDistance;
}

void linearInterp(const Point3D &pt1, const Point3D &pt2, const float t, Point3D &pt3) {
	pt3.x = pt1.x + (pt2.x - pt1.x) * t;
	pt3.y = pt1.y + (pt2.y - pt1.y) * t;
	pt3.z = pt1.z + (pt2.z - pt1.z) * t;
}

void linearInterp(const Point3D &pt1, const float x2, const float y2, const float z2, const float t, Point3D &pt3) {
	pt3.x = pt1.x + (x2 - pt1.x) * t;
	pt3.y = pt1.y + (y2 - pt1.y) * t;
	pt3.z = pt1.z + (z2 - pt1.z) * t;
}

void linearInterp(const float x1, const float y1, const float z1, const Point3D &pt2, const float t, Point3D &pt3) {
	pt3.x = x1 + (pt2.x - x1) * t;
	pt3.y = y1 + (pt2.y - y1) * t;
	pt3.z = z1 + (pt2.z - z1) * t;
}

void linearInterp(const float x1, const float y1, const float z1, const float x2, const float y2, const float z2,
		const float t, Point3D &pt3) {
	pt3.x = x1 + (x2 - x1) * t;
	pt3.y = y1 + (y2 - y1) * t;
	pt3.z = z1 + (z2 - z1) * t;
}

void linearInterp(const Common::Point &pt1, const Common::Point &pt2, const float t, Common::Point &pt3) {
	pt3.x = (int)(pt1.x + (pt2.x - pt1.x) * t);
	pt3.y = (int)(pt1.y + (pt2.y - pt1.y) * t);
}

void linearInterp(const Common::Point &pt1, const float h2, const float v2, const float t, Common::Point &pt3) {
	pt3.x = (int)(pt1.x + (h2 - pt1.x) * t);
	pt3.y = (int)(pt1.y + (v2 - pt1.y) * t);
}

void linearInterp(const float h1, const float v1, const Common::Point &pt2, const float t, Common::Point &pt3) {
	pt3.x = (int)(h1 + (pt2.x - h1) * t);
	pt3.y = (int)(v1 + (pt2.y - v1) * t);
}

void linearInterp(const float h1, const float v1, const float h2, const float v2, const float t, Common::Point &pt3) {
	pt3.x = (int)(h1 + (h2 - h1) * t);
	pt3.y = (int)(v1 + (v2 - v1) * t);
}

float linearInterp(const float arg1, const float arg2, const float t) {
	return arg1 + (arg2 - arg1) * t;
}

bool isNegative(int a) {
	return a < 0;
}

bool isPositive(int a) {
	return a > 0;
}

int sign(int a) {
	return isNegative(a) ? -1 : isPositive(a) ? 1 : 0;
}

bool sameSign(int a, int b) {
	return sign(a) == sign(b);
}

} // End of namespace Pegasus
