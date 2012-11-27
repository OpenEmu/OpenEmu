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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef WINTERMUTE_RECT32_H
#define WINTERMUTE_RECT32_H

#include "common/system.h"

namespace Wintermute {

struct Point32 {
	int32 x;
	int32 y;
};

struct Rect32 {
	int32 top, left;        ///< The point at the top left of the rectangle (part of the rect).
	int32 bottom, right;    ///< The point at the bottom right of the rectangle (not part of the rect).

	Rect32() : top(0), left(0), bottom(0), right(0) {}
	Rect32(int32 w, int32 h) : top(0), left(0), bottom(h), right(w) {}
	Rect32(int32 x1, int32 y1, int32 x2, int32 y2) : top(y1), left(x1), bottom(y2), right(x2) {
		assert(isValidRect());
	}
	bool operator==(const Rect32 &rhs) const {
		return equals(rhs);
	}
	bool operator!=(const Rect32 &rhs) const {
		return !equals(rhs);
	}

	int32 width() const {
		return right - left;
	}
	int32 height() const {
		return bottom - top;
	}

	void setWidth(int32 aWidth) {
		right = left + aWidth;
	}

	void setHeight(int32 aHeight) {
		bottom = top + aHeight;
	}

	void setEmpty() {
		left = right = top = bottom = 0;
	}

	void offsetRect(int dx, int dy) {
		left   += dx;
		top    += dy;
		right  += dx;
		bottom += dy;
	}
	/**
	 * Check if the given rect is equal to this one.
	 *
	 * @param r The rectangle to check
	 *
	 * @return true if the given rect is equal, false otherwise
	 */
	bool equals(const Rect32 &r) const {
		return (left == r.left) && (right == r.right) && (top == r.top) && (bottom == r.bottom);
	}

	bool isValidRect() const {
		return (left <= right && top <= bottom);
	}
};

} // end of namespace Wintermute

#endif
