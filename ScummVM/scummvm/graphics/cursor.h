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

#ifndef GRAPHICS_CURSOR_H
#define GRAPHICS_CURSOR_H

#include "common/scummsys.h"

namespace Graphics {

/**
 * A simple cursor representation
 * TODO: Switch to using Graphics::Surface instead of a byte*
 */
class Cursor {
public:
	Cursor() {}
	virtual ~Cursor() {}

	/** Return the cursor's width. */
	virtual uint16 getWidth() const = 0;
	/** Return the cursor's height. */
	virtual uint16 getHeight() const = 0;
	/** Return the cursor's hotspot's x coordinate. */
	virtual uint16 getHotspotX() const = 0;
	/** Return the cursor's hotspot's y coordinate. */
	virtual uint16 getHotspotY() const = 0;
	/** Return the cursor's transparent key. */
	virtual byte getKeyColor() const = 0;

	/** Return the cursor's surface. */
	virtual const byte *getSurface() const = 0;

	/** Return the cursor's palette in RGB format. */
	virtual const byte *getPalette() const = 0;
	/** Return the starting index of the palette. */
	virtual byte getPaletteStartIndex() const = 0;
	/** Return the number of colors in the palette. */
	virtual uint16 getPaletteCount() const = 0;
};

} // End of namespace Graphics

#endif
