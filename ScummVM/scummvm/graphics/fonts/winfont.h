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

#ifndef GRAPHICS_WINFONT_H
#define GRAPHICS_WINFONT_H

#include "common/str.h"
#include "graphics/font.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {

struct WinFontDirEntry {
	WinFontDirEntry() {}
	WinFontDirEntry(const Common::String &name, uint16 p) : faceName(name), points(p) {}

	// This is really just a simple identifier to match a directory entry with
	// If need-be, we can add other things to check such as italics and strikethrough, etc.
	Common::String faceName;
	uint16 points;
};

class WinFont : public Font {
public:
	WinFont();
	~WinFont();

	/**
	 * Open a font with a name in an FON file.
	 *
	 * If dirEntry is not given, the first font in the FONTDIR will be loaded
	 */
	bool loadFromFON(const Common::String &fileName, const WinFontDirEntry &dirEntry = WinFontDirEntry());

	/** Open a font from an FNT file */
	bool loadFromFNT(const Common::String &fileName);

	/** Close this font */
	void close();

	// Font API
	int getFontHeight() const { return _pixHeight; }
	int getMaxCharWidth() const { return _maxWidth; }
	int getCharWidth(byte chr) const;
	void drawChar(Surface *dst, byte chr, int x, int y, uint32 color) const;

private:
	bool loadFromPE(const Common::String &fileName, const WinFontDirEntry &dirEntry);
	bool loadFromNE(const Common::String &fileName, const WinFontDirEntry &dirEntry);

	uint32 getFontIndex(Common::SeekableReadStream &stream, const WinFontDirEntry &dirEntry);
	bool loadFromFNT(Common::SeekableReadStream &stream);
	char indexToCharacter(uint16 index) const;
	uint16 characterToIndex(byte character) const;

	uint16 _pixHeight;
	uint16 _maxWidth;
	byte _firstChar;
	byte _lastChar;
	byte _defaultChar;

	uint16 _glyphCount;
	struct GlyphEntry {
		GlyphEntry() { bitmap = 0; }
		~GlyphEntry() { delete[] bitmap; }

		uint16 charWidth;
		uint32 offset;
		byte *bitmap;
	} *_glyphs;
};

} // End of namespace Graphics

#endif
