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

#ifndef BACKENDS_GRAPHICS_DEFAULT_PALETTE_H
#define BACKENDS_GRAPHICS_DEFAULT_PALETTE_H

#include "graphics/palette.h"

/**
 * This is a default implementation of the PaletteManager interface
 * which ensures that grabPalette works as specified. Of course
 * it is still necessary to provide code that actually updates
 * the (possibly emulated) "hardware" palette of the backend.
 * For this purpose, implement the abstract setPaletteIntern
 * method.
 */
class DefaultPaletteManager : public PaletteManager {
protected:
	byte _palette[3 * 256];

	/**
	 * Subclasses should only implement this method and none of the others.
	 * Its semantics are like that of setPalette, only that it does not need
	 * to worry about making it possible to query the palette values once they
	 * have been set.
	 */
	virtual void setPaletteIntern(const byte *colors, uint start, uint num) = 0;

public:
	void setPalette(const byte *colors, uint start, uint num) {
		assert(start + num <= 256);
		memcpy(_palette + 3 * start, colors, 3 * num);
		setPaletteIntern(colors, start, num);
	}
	void grabPalette(byte *colors, uint start, uint num) {
		assert(start + num <= 256);
		memcpy(colors, _palette + 3 * start, 3 * num);
	}
};

#endif
