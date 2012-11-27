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
 */

#ifndef TEENAGENT_FONT_H
#define TEENAGENT_FONT_H

#include "common/str.h"
#include "graphics/surface.h"

namespace TeenAgent {

class Pack;

class Font {
public:
	Font();
	~Font();

	void load(const Pack &pack, int id, byte height, byte widthPack);
	uint render(Graphics::Surface *surface, int x, int y, const Common::String &str, byte color, bool showGrid = false);
	uint render(Graphics::Surface *surface, int x, int y, char c, byte color);
	static void grid(Graphics::Surface *surface, int x, int y, int w, int h, byte color);

	byte getHeight() { return _height; }
	void setShadowColor(byte color) { _shadowColor = color; }
private:
	byte *_data;

	byte _gridColor, _shadowColor;
	byte _height, _widthPack;
};

} // End of namespace TeenAgent

#endif
