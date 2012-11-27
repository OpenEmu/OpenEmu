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

#include "draci/screen.h"
#include "draci/surface.h"

namespace Draci {

Surface::Surface(int width, int height) {
	this->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	this->markClean();
	_transparentColor = kDefaultTransparent;
}

Surface::~Surface() {
	this->free();
}

/**
 * @brief Marks a dirty rectangle on the surface
 * @param r The rectangle to be marked dirty
 */
void Surface::markDirtyRect(Common::Rect r) {
	Common::List<Common::Rect>::iterator it;

	r.clip(w, h);

	if (r.isEmpty())
		return;

	it = _dirtyRects.begin();
	while (it != _dirtyRects.end()) {
		if (it->contains(r))
			return;

		if (r.contains(*it))
			it = _dirtyRects.erase(it);
		else
			++it;
	}

	_dirtyRects.push_back(r);
}

/**
 * @brief Marks the whole surface dirty
 */
void Surface::markDirty() {
	_fullUpdate = true;
}

/**
 * @brief Marks the whole surface clean
 */
void Surface::markClean() {
	_fullUpdate = false;
	_dirtyRects.clear();
}

/**
 * @brief Fills the surface with the specified color
 */
void Surface::fill(uint color) {
	byte *ptr = (byte *)getBasePtr(0, 0);

	memset(ptr, color, w * h);
}

/**
 * @brief Calculates horizontal center of an object
 *
 * @param x     The x coordinate of the center
 * @param width The width of the object to be centered (in pixels)
 *
 * @return The centered x coordinate
 */
uint Surface::centerOnX(int x, int width) const {
	int newX = x - width / 2;

	if (newX + width > w)
		newX = w - width;

	if (newX < 0)
		newX = 0;

	return newX;
}

/**
 * @brief Calculates vertical center of an object
 *
 * @param y      The y coordinate of the center
 * @param height The height of the object to be centered (in pixels)
 *
 * @return The centered y coordinate
 */
uint Surface::putAboveY(int y, int height) const {
	int newY = y - height;

	if (newY + height > h)
		newY = h - height;

	if (newY < 0)
		newY = 0;

	return newY;
}

} // End of namespace Draci
