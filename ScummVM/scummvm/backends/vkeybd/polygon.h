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

#ifndef COMMON_POLYGON_H
#define COMMON_POLYGON_H

#include "common/scummsys.h"

#ifdef ENABLE_VKEYBD

#include "common/array.h"
#include "common/rect.h"

namespace Common {

struct Polygon {

	Polygon() {}
	Polygon(Array<Point> p) : _points(p) {
		for (uint i = 0; i < p.size(); i++) {
			_bound.extend(Rect(p[i].x, p[i].y, p[i].x, p[i].y));
		}
	}
	Polygon(Point *p, int n) {
		for (int i = 0; i < n; i++) {
			addPoint(p[i]);
		}
	}

	void addPoint(const Point &p) {
		_points.push_back(p);
		_bound.extend(Rect(p.x, p.y, p.x, p.y));
	}

	void addPoint(int16 x, int16 y) {
		addPoint(Point(x, y));
	}

	uint getPointCount() {
		return _points.size();
	}

	/**
	 * Check if given position is inside this polygon.
	 *
	 * @param x the horizontal position to check
	 * @param y the vertical position to check
	 * @return true if the given position is inside this polygon, false otherwise
	*/
	bool contains(int16 x, int16 y) const;

	/**
	 * Check if given point is inside this polygon.
	 *
	 * @param p the point to check
	 * @return true if the given point is inside this polygon, false otherwise
	*/
	bool contains(const Point &p) const {
		return contains(p.x, p.y);
	}

	void moveTo(int16 x, int16 y) {
		int16 dx = x - ((_bound.right + _bound.left) / 2);
		int16 dy = y - ((_bound.bottom + _bound.top) / 2);
		translate(dx, dy);
	}

	void moveTo(const Point &p) {
		moveTo(p.x, p.y);
	}

	void translate(int16 dx, int16 dy) {
		Array<Point>::iterator it;
		for (it = _points.begin(); it != _points.end(); it++) {
			it->x += dx;
			it->y += dy;
		}
	}

	Rect getBoundingRect() const {
		return _bound;
	}

private:
	Array<Point> _points;
	Rect _bound;
};

} // End of namespace Common

#endif // #ifdef ENABLE_VKEYBD

#endif // #ifndef COMMON_POLYGON_H
