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

#include "cruise/cruise_main.h"
#include "common/util.h"

namespace Cruise {

typedef char ColorP;

#define SCREENHEIGHT 200
#define MAXPTS 10
#define putdot(x,y) {if ((y >= 0) && (y < SCREENHEIGHT)) dots[y][counters[y]++] = x;}

void hline(int x1, int x2, int y, char c) {
	for (; x1 <= x2; x1++) {
		pixel(x1, y, c);
	}
}

void vline(int x, int y1, int y2, char c) {
	for (; y1 <= y2; y1++) {
		pixel(x, y1, c);
	}
}

void bsubline_1(int x1, int y1, int x2, int y2, char c) {
	int x, y, ddx, ddy, e;
	ddx = ABS(x2 - x1);
	ddy = ABS(y2 - y1) << 1;
	e = ddx - ddy;
	ddx <<= 1;

	if (x1 > x2) {
		SWAP(x1, x2);
		SWAP(y1, y2);
	}

	for (x = x1, y = y1; x <= x2; x++) {

		pixel(x, y, c);
		if (e < 0) {
			y++;
			e += ddx - ddy;
		} else {
			e -= ddy;
		}
	}

}

void bsubline_2(int x1, int y1, int x2, int y2, char c) {

	int x, y, ddx, ddy, e;
	ddx = ABS(x2 - x1) << 1;
	ddy = ABS(y2 - y1);
	e = ddy - ddx;
	ddy <<= 1;

	if (y1 > y2) {
		SWAP(x1, x2);
		SWAP(y1, y2);
	}

	for (y = y1, x = x1; y <= y2; y++) {

		pixel(x, y, c);
		if (e < 0) {
			x++;
			e += ddy - ddx;
		} else {
			e -= ddx;
		}
	}

}

void bsubline_3(int x1, int y1, int x2, int y2, char c) {

	int x, y, ddx, ddy, e;

	ddx = ABS(x1 - x2) << 1;
	ddy = ABS(y2 - y1);
	e = ddy - ddx;
	ddy <<= 1;

	if (y1 > y2) {
		SWAP(x1, x2);
		SWAP(y1, y2);
	}

	for (y = y1, x = x1; y <= y2; y++) {

		pixel(x, y, c);
		if (e < 0) {
			x--;
			e += ddy - ddx;
		} else {
			e -= ddx;
		}
	}

}

void bsubline_4(int x1, int y1, int x2, int y2, char c) {

	int x, y, ddx, ddy, e;

	ddy = ABS(y2 - y1) << 1;
	ddx = ABS(x1 - x2);
	e = ddx - ddy;
	ddx <<= 1;

	if (x1 > x2) {
		SWAP(x1, x2);
		SWAP(y1, y2);
	}

	for (x = x1, y = y1; x <= x2; x++) {

		pixel(x, y, c);
		if (e < 0) {
			y--;
			e += ddx - ddy;
		} else {
			e -= ddy;
		}
	}
}

void line(int x1, int y1, int x2, int y2, char c) {
	float k;

	if ((x1 == x2) && (y1 == y2)) {
		pixel(x1, y1, c);
		return;
	}

	if (x1 == x2) {
		vline(x1, MIN(y1, y2), MAX(y1, y2), c);
		return;
	}

	if (y1 == y2) {
		hline(MIN(x1, x2), MAX(x1, x2), y1, c);
		return;
	}

	k = (float)(y2 - y1) / (float)(x2 - x1);

	if ((k >= 0) && (k <= 1)) {
		bsubline_1(x1, y1, x2, y2, c);
	} else if (k > 1) {
		bsubline_2(x1, y1, x2, y2, c);
	} else if ((k < 0) && (k >= -1)) {
		bsubline_4(x1, y1, x2, y2, c);
	} else {
		bsubline_3(x1, y1, x2, y2, c);
	}
}

// Filled polygons. This probably isn't pixel-perfect compared to the original,
// but it seems to work a bit better than the previous version.

static void add_intersect(int *intersect, int x, byte &num) {
	if (num < MAXPTS) {
		int i;

		for (i = num; i > 0 && intersect[i - 1] > x; i--) {
			intersect[i] = intersect[i - 1];
		}

		intersect[i] = x;
		num++;
	}
}

void fillpoly(int16 *point_data, int lineCount, ColorP color) {
	static int intersect[SCREENHEIGHT][MAXPTS];
	static byte num_intersect[SCREENHEIGHT];

	switch (lineCount) {
	case 0:		// do nothing
		return;
	case 1:		// draw pixel
		pixel(point_data[0], point_data[1], color);
		return;
	case 2:		// draw line
		line(point_data[0], point_data[1], point_data[2], point_data[3], color);
		return;
	default:		// go on and draw polygon
		break;
	}

	// Reinit array counters

	int x1, y1, x2, y2;
	int y, i;

	for (i = 0; i < SCREENHEIGHT; i++) {
		num_intersect[i] = 0;
	}

	// Find the top/bottom of the polygon.

	int top = point_data[1];
	int bottom = point_data[1];

	for (i = 1; i < lineCount; i++) {
		if (point_data[2 * i + 1] < top)
			top = point_data[2 * i + 1];
		else if (point_data[2 * i + 1] > bottom)
			bottom = point_data[2 * i + 1];
	}

	if (top < 0)
		top = 0;
	if (bottom >= SCREENHEIGHT)
		bottom = SCREENHEIGHT - 1;

	// Calculate intersections for each scan line

	for (y = top; y <= bottom; y++) {
		x2 = point_data[2 * lineCount - 2];
		y2 = point_data[2 * lineCount - 1];

		for (i = 0; i < lineCount; i++) {
			x1 = x2;
			y1 = y2;
			x2 = point_data[2 * i];
			y2 = point_data[2 * i + 1];

			// Test if the line intersects the scan line

			if ((y < y1) != (y < y2)) {
				if (y1 == y2) {
					add_intersect(intersect[y], x1, num_intersect[y]);
					add_intersect(intersect[y], x2, num_intersect[y]);
				} else if (x1 == x2) {
					add_intersect(intersect[y], x1, num_intersect[y]);
				} else {
					add_intersect(intersect[y], x1 + ((y - y1) * (x2 - x1)) / (y2 - y1), num_intersect[y]);
				}
			}
		}
	}

	// Drawing.

	for (y = top; y <= bottom; y++) {
		for (i = 0; i < num_intersect[y]; i += 2) {
			hline(intersect[y][i], intersect[y][i + 1], y, color);
		}
	}
}

} // End of namespace Cruise
