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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "kyra/screen_mr.h"
#include "kyra/kyra_mr.h"

namespace Kyra {

Screen_MR::Screen_MR(KyraEngine_MR *vm, OSystem *system)
    : Screen_v2(vm, system, _screenDimTable, _screenDimTableCount) {
}

Screen_MR::~Screen_MR() {
}

int Screen_MR::getLayer(int x, int y) {
	if (x < 0)
		x = 0;
	else if (x >= 320)
		x = 319;
	if (y < 0)
		y = 0;
	else if (y >= 188)
		y = 187;

	if (y < _maskMinY || y > _maskMaxY)
		return 15;

	uint8 pixel = *(getCPagePtr(5) + y * 320 + x);
	pixel &= 0x7F;
	pixel >>= 3;

	if (pixel < 1)
		pixel = 1;
	else if (pixel > 15)
		pixel = 15;
	return pixel;
}

byte Screen_MR::getShapeFlag1(int x, int y) {
	if (y < _maskMinY || y > _maskMaxY)
		return 0;

	uint8 color = _shapePages[0][y * SCREEN_W + x];
	color &= 0x80;
	color ^= 0x80;

	if (color & 0x80)
		return 1;
	return 0;
}

byte Screen_MR::getShapeFlag2(int x, int y) {
	if (y < _maskMinY || y > _maskMaxY)
		return 0;

	uint8 color = _shapePages[0][y * SCREEN_W + x];
	color &= 0x7F;
	color &= 0x87;
	return color;
}

int Screen_MR::getDrawLayer(int x, int y) {
	int xpos = x - 8;
	int ypos = y;
	int layer = 1;

	for (int curX = xpos; curX < xpos + 24; ++curX) {
		int tempLayer = getShapeFlag2(curX, ypos);

		if (layer < tempLayer)
			layer = tempLayer;

		if (layer >= 7)
			return 7;
	}
	return layer;
}

int Screen_MR::getDrawLayer2(int x, int y, int height) {
	int xpos = x - 8;
	int ypos = y;
	int layer = 1;

	for (int useX = xpos; useX < xpos + 24; ++useX) {
		for (int useY = ypos - height; useY < ypos; ++useY) {
			int tempLayer = getShapeFlag2(useX, useY);

			if (tempLayer > layer)
				layer = tempLayer;

			if (tempLayer >= 7)
				return 7;
		}
	}
	return layer;
}

void Screen_MR::drawFilledBox(int x1, int y1, int x2, int y2, uint8 c1, uint8 c2, uint8 c3) {
	fillRect(x1, y1, x2, y2, c1);

	fillRect(x1, y1, x2, y1+1, c2);
	fillRect(x2-1, y1, x2, y2, c2);

	drawClippedLine(x1, y1, x1, y2, c3);
	drawClippedLine(x1+1, y1+1, x1+1, y2-2, c3);
	drawClippedLine(x1, y2, x2, y2, c3);
	drawClippedLine(x1, y2-1, x2-1, y2-1, c3);
}

} // End of namespace Kyra
