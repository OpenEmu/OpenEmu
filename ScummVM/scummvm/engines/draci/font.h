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

#ifndef DRACI_FONT_H
#define DRACI_FONT_H

#include "graphics/font.h"

namespace Draci {

extern const char * const kFontSmall;
extern const char * const kFontBig;

class Surface;

/**
 *  Default font colors. They all seem to remain constant except for the
 *  first one which varies depending on the character speaking.
 *  kOverFontColor is set to transparent.
 */
enum {
	kFontColor1 = 2,
	kFontColor2 = 0,
	kFontColor3 = 3,
	kFontColor4 = 4,
	kOverFontColor = 255,
	kTitleColor = 255,
	kLineActiveColor = 254,
	kLineInactiveColor = 255
};

/**
 *  Represents the game's fonts. See docs for setFont() for font format details.
 */
class Font {

public:
	Font(const Common::String &filename);
	~Font();

	bool loadFont(const Common::String &filename);
	uint8 getFontHeight() const { return _fontHeight; }
	uint8 getMaxCharWidth() const { return _maxCharWidth; }
	uint8 getCharWidth(byte chr) const;
	void drawChar(Surface *dst, uint8 chr, int tx, int ty, int with_color) const;

	void drawString(Surface *dst, const byte *str, uint len, int x, int y, int with_color,
	                int spacing, bool markDirty) const;
	void drawString(Surface *dst, const Common::String &str,
	                int x, int y, int with_color, int spacing, bool markDirty) const;

	uint getStringWidth(const Common::String &str, int spacing) const;
	uint getStringHeight(const Common::String &str) const;
	uint getLineWidth(const Common::String &str, uint startIndex, int spacing) const;

private:
	uint8 _fontHeight;
	uint8 _maxCharWidth;

	/** Pointer to an array of individual char widths */
	uint8 *_charWidths;

	/** Pointer to a raw byte array representing font pixels stored row-wise */
	byte *_charData;

	/** Number of glyphs in the font */
	static const uint kCharNum = 138;

	/**
	 *  Chars are indexed from the space character so this should be subtracted
	 *  to get the index of a glyph
	 */
	static const uint kCharIndexOffset = 32;

	/** Internal function for freeing fonts when destructing/loading another */
	void freeFont();
};

} // End of namespace Draci

#endif // DRACI_FONT_H
