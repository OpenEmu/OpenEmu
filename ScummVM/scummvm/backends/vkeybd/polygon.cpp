/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#ifdef ENABLE_VKEYBD

#include "backends/vkeybd/polygon.h"

namespace Common {

bool Polygon::contains(int16 x, int16 y) const {
	int yflag0;
	int yflag1;
	bool inside_flag = false;
	unsigned int pt;

	const Point *vtx0 = &_points[_points.size() - 1];
	const Point *vtx1 = &_points[0];

	yflag0 = (vtx0->y >= y);
	for (pt = 0; pt < _points.size(); pt++, vtx1++) {
		yflag1 = (vtx1->y >= y);
		if (yflag0 != yflag1) {
			if (((vtx1->y - y) * (vtx0->x - vtx1->x) >=
			        (vtx1->x - x) * (vtx0->y - vtx1->y)) == yflag1) {
				inside_flag = !inside_flag;
			}
		}
		yflag0 = yflag1;
		vtx0 = vtx1;
	}

	return inside_flag;
}

} // End of namespace Common

#endif // #ifdef ENABLE_VKEYBD
