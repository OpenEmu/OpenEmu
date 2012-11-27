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

#ifndef LASTEXPRESS_FONT_H
#define LASTEXPRESS_FONT_H

/*
	Font format (FONT.DAT)

	uint16 {40}   - Palette data
	byte {200}    - Character map
	uint16 {2}    - Number of glyphs

	// For each glyph
	    byte {18*8}   - Glyph data

	byte {x}      - Unknown data (probably just garbage)
*/

#include "common/str.h"
#include "graphics/surface.h"

namespace Common {
class SeekableReadStream;
struct Rect;
}

namespace LastExpress {

class Font {
public:
	Font();
	~Font();

	bool load(Common::SeekableReadStream *stream);
	Common::Rect drawString(Graphics::Surface *surface, int16 x, int16 y, Common::String str);
	Common::Rect drawString(Graphics::Surface *surface, int16 x, int16 y, const uint16 *str, uint16 length);

private:
	static const uint32 _paletteSize = 0x10;
	static const uint32 _charMapSize = 0x200;
	static const uint32 _charHeight = 16;

	void reset();

	uint16 getCharGlyph(uint16 c) const;
	byte *getGlyphImg(uint16 g);
	uint8 getGlyphWidth(uint16 g);
	byte *getCharImg(uint16 c);
	uint8 getCharWidth(uint16 c) const;
	uint16 getStringWidth(Common::String str) const;
	uint16 getStringWidth(const uint16 *str, uint16 length) const;
	void drawChar(Graphics::Surface *surface, int16 x, int16 y, uint16 c);

	// Font data
	uint16 _palette[_paletteSize];
	uint8 _charMap[_charMapSize];
	uint16 _numGlyphs;
	byte *_glyphs;
	uint8 *_glyphWidths;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_FONT_H
