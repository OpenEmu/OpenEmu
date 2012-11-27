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
 *
 */

#include "toltecs/microtiles.h"

namespace Toltecs {

MicroTileArray::MicroTileArray(int16 width, int16 height) {
	_tilesW = (width / TileSize) + ((width % TileSize) > 0 ? 1 : 0);
	_tilesH = (height / TileSize) + ((height % TileSize) > 0 ? 1 : 0);
	_tiles = new BoundingBox[_tilesW * _tilesH];
	clear();
}

MicroTileArray::~MicroTileArray() {
	delete[] _tiles;
}

void MicroTileArray::addRect(Common::Rect r) {

	int ux0, uy0, ux1, uy1;
	int tx0, ty0, tx1, ty1;
	int ix0, iy0, ix1, iy1;

	r.clip(Common::Rect(0, 0, 639, 399));

	ux0 = r.left / TileSize;
	uy0 = r.top / TileSize;
	ux1 = r.right / TileSize;
	uy1 = r.bottom / TileSize;

	tx0 = r.left % TileSize;
	ty0 = r.top % TileSize;
	tx1 = r.right % TileSize;
	ty1 = r.bottom % TileSize;

	for (int yc = uy0; yc <= uy1; yc++) {
		for (int xc = ux0; xc <= ux1; xc++) {
			ix0 = (xc == ux0) ? tx0 : 0;
			ix1 = (xc == ux1) ? tx1 : TileSize - 1;
			iy0 = (yc == uy0) ? ty0 : 0;
			iy1 = (yc == uy1) ? ty1 : TileSize - 1;
			updateBoundingBox(_tiles[xc + yc * _tilesW], ix0, iy0, ix1, iy1);
		}
	}

}

void MicroTileArray::clear() {
	memset(_tiles, 0, _tilesW * _tilesH * sizeof(BoundingBox));
}

byte MicroTileArray::TileX0(const BoundingBox &boundingBox) {
	return (boundingBox >> 24) & 0xFF;
}

byte MicroTileArray::TileY0(const BoundingBox &boundingBox) {
	return (boundingBox >> 16) & 0xFF;
}

byte MicroTileArray::TileX1(const BoundingBox &boundingBox) {
	return (boundingBox >> 8) & 0xFF;
}

byte MicroTileArray::TileY1(const BoundingBox &boundingBox) {
	return boundingBox & 0xFF;
}

bool MicroTileArray::isBoundingBoxEmpty(const BoundingBox &boundingBox) {
	return boundingBox == EmptyBoundingBox;
}

bool MicroTileArray::isBoundingBoxFull(const BoundingBox &boundingBox) {
	return boundingBox == FullBoundingBox;
}

void MicroTileArray::setBoundingBox(BoundingBox &boundingBox, byte x0, byte y0, byte x1, byte y1) {
	boundingBox = (x0 << 24) | (y0 << 16) | (x1 << 8) | y1;
}

void MicroTileArray::updateBoundingBox(BoundingBox &boundingBox, byte x0, byte y0, byte x1, byte y1) {
	if (!isBoundingBoxEmpty(boundingBox)) {
		x0 = MIN(TileX0(boundingBox), x0);
		y0 = MIN(TileY0(boundingBox), y0);
		x1 = MAX(TileX1(boundingBox), x1);
		y1 = MAX(TileY1(boundingBox), y1);
	}
	setBoundingBox(boundingBox, x0, y0, x1, y1);
}

Common::Rect * MicroTileArray::getRectangles(int *num_rects, int min_x, int min_y, int max_x, int max_y) {

	Common::Rect *rects = new Common::Rect[_tilesW * _tilesH];

	int n_rects = 0;
	int x, y;
	int x0, y0, x1, y1;
	int i = 0;

	for (y = 0; y < _tilesH; ++y) {
		for (x = 0; x < _tilesW; ++x) {
			int finish = 0;
			BoundingBox boundingBox;

			boundingBox = _tiles[i];

			if (isBoundingBoxEmpty(boundingBox)) {
				++i;
				continue;
			}

			x0 = (x * TileSize) + TileX0(boundingBox);
			y0 = (y * TileSize) + TileY0(boundingBox);
			y1 = (y * TileSize) + TileY1(boundingBox);

			x0 = CLIP (x0, min_x, max_x);
			y0 = CLIP (y0, min_y, max_y);
			y1 = CLIP (y1, min_y, max_y);

			// FIXME: Why is the following code in an #if block?
#if 1
			if (TileX1(boundingBox) == TileSize - 1 && x != _tilesW - 1) {	// check if the tile continues
				while (!finish) {
					++x;
					++i;

					if (x == _tilesW || i >= _tilesW * _tilesH ||
						TileY0(_tiles[i]) != TileY0(boundingBox) ||
						TileY1(_tiles[i]) != TileY1(boundingBox) ||
						TileX0(_tiles[i]) != 0)
					{
						--x;
						--i;
						finish = 1;
					}
				}
			}
#endif
			x1 = (x * TileSize) + TileX1(_tiles[i]);

			x1 = CLIP (x1, min_x, max_x);

			// FIXME: Why is the following code in an #if block?

			#if 1

			rects[n_rects].left = x0;
			rects[n_rects].top = y0;
			rects[n_rects].right = x1 + 1;
			rects[n_rects].bottom = y1 + 1;
			n_rects++;

			#else

			// FIXME: Why is this code disabled?

			if (glom [start] != -1 && /* try to glom */
				rects [glom [start]].left == x0 &&
				rects [glom [start]].right == x1 &&
				rects [glom [start]].bottom == y0 - 1)
			{
				rects [glom [start]].bottom = y1;
				if (y != tilesH - 1) {
					glom [start + tilesW] = glom [start];
				}
			} else {
				rects[n_rects].left = x0;
				rects[n_rects].top = y0;
				rects[n_rects].right = x1;
				rects[n_rects].bottom = y1;
				if (y != tilesH - 1) {
					glom [start + tilesW] = n_rects;
				}
				n_rects ++;
			}

			#endif

			++i;
		}	// for (x = 0; x < _tilesW; ++x)
	}	// for (y = 0; y < _tilesH; ++y)

	*num_rects = n_rects;

	//delete glom;

	return rects;

}

} // End of namespace Toltecs
