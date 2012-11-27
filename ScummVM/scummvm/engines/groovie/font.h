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

#ifndef GROOVIE_FONT_H
#define GROOVIE_FONT_H

#include "common/stream.h"
#include "graphics/font.h"

namespace Groovie {

class T7GFont : public Graphics::Font {
public:
	T7GFont();
	~T7GFont();

	bool load(Common::SeekableReadStream &stream);

	int getFontHeight() const { return _maxHeight; }
	int getMaxCharWidth() const { return _maxWidth; }
	int getCharWidth(byte chr) const { return getGlyph(chr)->width; }
	void drawChar(Graphics::Surface *dst, byte chr, int x, int y, uint32 color) const;

private:
	int _maxHeight, _maxWidth;

	struct Glyph {
		Glyph() : pixels(0) {}
		~Glyph() { delete[] pixels; }

		byte width;
		byte height;
		byte julia;
		byte *pixels;
	};

	byte _mapChar2Glyph[128];
	Glyph *_glyphs;
	const Glyph *getGlyph(byte chr) const;
};

} // End of Groovie namespace

#endif // GROOVIE_FONT_H
