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

#include "common/file.h"

#include "draci/draci.h"
#include "draci/font.h"
#include "draci/surface.h"

namespace Draci {

const char * const kFontSmall = "Small.fon";
const char * const kFontBig = "Big.fon";

Font::Font(const Common::String &filename) {
	_fontHeight = 0;
	_maxCharWidth = 0;
	_charWidths = NULL;
	_charData = NULL;

	loadFont(filename);
}

Font::~Font() {
	 freeFont();
}

/**
 * @brief Loads fonts from a file
 * @param path Path to font file
 * @return true if the font was loaded successfully, false otherwise
 *
 * Loads fonts from a file into a Font instance. The original game uses two
 * fonts (located inside files "Small.fon" and "Big.fon"). The characters in the
 * font are indexed from the space character so an appropriate offset must be
 * added to convert them to equivalent char values, i.e. kDraciIndexOffset.
 * Characters in the higher range are non-ASCII and vary between different
 * language versions of the game.
 *
 * font format: [1 byte] maximum character width
 *              [1 byte] font height
 *              [138 bytes] character widths of all 138 characters in the font
 *              [138 * fontHeight * maxWidth bytes] character data, stored row-wise
 */

bool Font::loadFont(const Common::String &filename) {
	// Free previously loaded font (if any)
	freeFont();

	Common::File f;

	f.open(filename);
	if (f.isOpen()) {
		debugC(6, kDraciGeneralDebugLevel, "Opened font file %s",
			filename.c_str());
	} else {
		debugC(6, kDraciGeneralDebugLevel, "Error opening font file %s",
			filename.c_str());
		return false;
	}

	_maxCharWidth = f.readByte();
	_fontHeight = f.readByte();

	// Read in the widths of the glyphs
	_charWidths = new uint8[kCharNum];
	for (uint i = 0; i < kCharNum; ++i) {
		_charWidths[i] = f.readByte();
	}

	// Calculate size of font data
	uint fontDataSize = kCharNum * _maxCharWidth * _fontHeight;

	// Read in all glyphs
	_charData = new byte[fontDataSize];
	f.read(_charData, fontDataSize);

	debugC(5, kDraciGeneralDebugLevel, "Font %s loaded", filename.c_str());

	return true;
}

void Font::freeFont() {
	delete[] _charWidths;
	delete[] _charData;
}

uint8 Font::getCharWidth(uint8 chr) const {
	// Safe-guard against incorrect strings containing localized characters
	// with inaccessible codes.  These strings do not exist in the original
	// Czech version, but they do in the (never properly reviewed) English
	// version.
	return chr >= kCharIndexOffset && chr < kCharIndexOffset + kCharNum
		? _charWidths[chr - kCharIndexOffset] : 0;
}

/**
 * @brief Draw a char to a Draci::Surface
 *
 * @param dst   Pointer to the destination surface
 * @param chr   Character to draw
 * @param tx    Horizontal offset on the surface
 * @param ty    Vertical offset on the surface
 */

void Font::drawChar(Surface *dst, uint8 chr, int tx, int ty, int with_color) const {
	assert(dst != NULL);
	assert(tx >= 0);
	assert(ty >= 0);

	byte *ptr = (byte *)dst->getBasePtr(tx, ty);
	const uint8 currentWidth = getCharWidth(chr);
	if (currentWidth == 0) {
		return;
	}

	const uint8 charIndex = chr - kCharIndexOffset;
	const int charOffset = charIndex * _fontHeight * _maxCharWidth;

	// Determine how many pixels to draw horizontally (to prevent overflow)
	int xSpaceLeft = dst->w - tx - 1;
	int xPixelsToDraw = (currentWidth < xSpaceLeft) ? currentWidth : xSpaceLeft;

	// Determine how many pixels to draw vertically
	int ySpaceLeft = dst->h - ty - 1;
	int yPixelsToDraw = (_fontHeight < ySpaceLeft) ? _fontHeight : ySpaceLeft;

	int _transparent = dst->getTransparentColor();

	for (int y = 0; y < yPixelsToDraw; ++y) {
		for (int x = 0; x <= xPixelsToDraw; ++x) {

			int curr = y * _maxCharWidth + x;
			int color = _charData[charOffset + curr];

			// If pixel is transparent, skip it
			if (color == _transparent)
				continue;

			// Replace color with font colors
			switch (color) {
			case 254:
				color = with_color;
				break;

			case 253:
				color = kFontColor2;
				break;

			case 252:
				color = kFontColor3;
				break;

			case 251:
				color = kFontColor4;
				break;
			}

			// Paint the pixel
			ptr[x] = color;
		}

		// Advance to next row
		ptr += dst->pitch;
	}
}

/**
 * @brief Draw a string to a Draci::Surface
 *
 * @param dst       Pointer to the destination surface
 * @param str       Buffer containing string data
 * @param len       Length of the data
 * @param x         Horizontal offset on the surface
 * @param y         Vertical offset on the surface
 * @param spacing   Space to leave between individual characters. Defaults to 0.
 */
void Font::drawString(Surface *dst, const byte *str, uint len,
	                  int x, int y, int with_color, int spacing, bool markDirty) const {
	drawString(dst, Common::String((const char *)str, len), x, y, with_color, spacing, markDirty);
}

/**
 * @brief Draw a string to a Draci::Surface
 *
 * @param dst       Pointer to the destination surface
 * @param str       String to draw
 * @param x         Horizontal offset on the surface
 * @param y         Vertical offset on the surface
 * @param spacing   Space to leave between individual characters. Defaults to 0.
 */

void Font::drawString(Surface *dst, const Common::String &str,
	                  int x, int y, int with_color, int spacing, bool markDirty) const {
	assert(dst != NULL);
	assert(x >= 0);
	assert(y >= 0);

	uint widest = getStringWidth(str, spacing);

	int curx = x + (widest - getLineWidth(str, 0, spacing)) / 2;
	int cury = y;

	for (uint i = 0; i < str.size(); ++i) {

		// If we encounter the '|' char (newline and end of string marker),
		// skip it and go to the start of the next line
		if (str[i] == '|') {
			cury += getFontHeight();
			curx = x + (widest - getLineWidth(str, i+1, spacing) - 1) / 2;
			continue;
		}

		// Break early if there's no more space on the screen
		if (curx >= dst->w - 1 || cury >= dst->h - 1) {
			break;
		}

		drawChar(dst, str[i], curx, cury, with_color);
		curx += getCharWidth(str[i]) + spacing;
	}

	if (markDirty) {
		Common::Rect r(x, y, x + widest, y + getStringHeight(str));
		dst->markDirtyRect(r);
	}
}

/**
 * @brief Calculate the width of a string when drawn in the current font
 *
 * @param str       String to draw
 * @param spacing   Space to leave between individual characters. Defaults to 0.
 *
 * @return The calculated width of the string
 */
uint Font::getStringWidth(const Common::String &str, int spacing) const {
	uint width = 0;

	// Real length, including '|' separators
	uint len = str.size();

	for (uint i = 0, tmp = 0; i < len; ++i) {

		if (str[i] != '|') {
			tmp += getCharWidth(str[i]) + spacing;
		}

		// Newline char encountered, skip it and store the new length if it is greater.
		// Also, all strings in the data files should end with '|' but not all do.
		// This is why we check whether we are at the last char too.
		if (str[i] == '|' || i == len - 1) {
			if (tmp > width) {
				width = tmp;
			}

			tmp = 0;
		}
	}

	return width + 1;
}

uint Font::getLineWidth(const Common::String &str, uint startIndex, int spacing) const {
	uint width = 0;

	// If the index is greater or equal to the string size,
	// the width of the line is 0
	if (startIndex >= str.size())
		return 0;

	for (uint i = startIndex; i < str.size(); ++i) {

		// EOL encountered
		if (str[i] == '|')
			break;

		// Add width of the current char
		width += getCharWidth(str[i]) + spacing;
	}

	return width;
}

/**
 * @brief Calculate the height of a string by counting the number of '|' chars (which
 *        are used as newline characters and end-of-string markers)
 *
 * @param str       String to draw
 * @param spacing   Space to leave between individual characters. Defaults to 0.
 *
 * @return The calculated height of the string
 */
uint Font::getStringHeight(const Common::String &str) const {
	uint len = str.size();
	int separators = 0;

	for (uint i = 0; i < len; ++i) {
		// All strings in the data files should end with '|' but not all do.
		// This is why we check whether we are at the last char too.
		if (str[i] == '|' || i == len - 1) {
			++separators;
		}
	}

	return separators * getFontHeight();
}

} // End of namespace Draci
