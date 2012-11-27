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

#include "pegasus/neighborhood/mars/hermite.h"

namespace Pegasus {

CoordType hermite(CoordType p1, CoordType p4, CoordType r1, CoordType r4, int32 time, int32 duration) {
	float t = (float)time / duration;
	float tsq = t * t;
	float tcu = t * tsq;
	float tcu2 = tcu + tcu;
	float tsq2 = tsq + tsq;
	float tsq3 = tsq2 + tsq;
	return (CoordType)((tcu2 - tsq3 + 1) * p1 + (tsq3 - tcu2) * p4 + (tcu - tsq2 + t) * r1 + (tcu - tsq) * r4);
}

CoordType dHermite(CoordType p1, CoordType p4, CoordType r1, CoordType r4, int32 time, int32 duration) {
	float t = (float)time / duration;
	float t2 = t + t;
	float t4 = t2 + t2;
	float t6 = t4 + t2;
	float tsq = t * t;
	float tsq3 = tsq + tsq + tsq;
	float tsq6 = tsq3 + tsq3;
	return (CoordType)((tsq6 - t6) * p1 + (t6 - tsq6) * p4 + (tsq3 - t4 + 1) * r1 + (tsq3 - t2) * r4);
}

void hermite(Common::Point p1, Common::Point p4, Common::Point r1, Common::Point r4, int32 time, int32 duration, Common::Point &result) {
	float t = (float)time / duration;
	float tsq = t * t;
	float tcu = t * tsq;
	float tcu2 = tcu + tcu;
	float tsq2 = tsq + tsq;
	float tsq3 = tsq2 + tsq;

	result.x = (int16)((tcu2 - tsq3 + 1) * p1.x + (tsq3 - tcu2) * p4.x + (tcu - tsq2 + t) * r1.x + (tcu - tsq) * r4.x);
	result.y = (int16)((tcu2 - tsq3 + 1) * p1.y + (tsq3 - tcu2) * p4.y + (tcu - tsq2 + t) * r1.y + (tcu - tsq) * r4.y);
}

void dHermite(Common::Point p1, Common::Point p4, Common::Point r1, Common::Point r4, int32 time, int32 duration, Common::Point &result) {
	float t = (float)time / duration;
	float t2 = t + t;
	float t4 = t2 + t2;
	float t6 = t4 + t2;
	float tsq = t * t;
	float tsq3 = tsq + tsq + tsq;
	float tsq6 = tsq3 + tsq3;

	result.x = (int16)((tsq6 - t6) * p1.x + (t6 - tsq6) * p4.x + (tsq3 - t4 + 1) * r1.x + (tsq3 - t2) * r4.x);
	result.y = (int16)((tsq6 - t6) * p1.y + (t6 - tsq6) * p4.y + (tsq3 - t4 + 1) * r1.y + (tsq3 - t2) * r4.y);
}

} // End of namespace Pegasus
