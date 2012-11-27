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

#include "lastexpress/data/font.h"

#include "common/rect.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"

namespace LastExpress {

Font::Font() : _numGlyphs(0), _glyphs(NULL), _glyphWidths(0) {
	memset(&_palette, 0, sizeof(_palette));
	memset(&_charMap, 0, sizeof(_charMap));
}

Font::~Font() {
	reset();
}

void Font::reset() {
	delete[] _glyphs;
	delete[] _glyphWidths;
}

bool Font::load(Common::SeekableReadStream *stream) {
	if (!stream)
		return false;

	// Reset data
	reset();

	// Read the palette
	for (uint i = 0; i < _paletteSize; i++) {
		_palette[i] = stream->readUint16LE();
	}

	// Read the character map
	stream->read(_charMap, _charMapSize);

	// Read the glyphs
	_numGlyphs = stream->readUint16LE();
	_glyphs = new byte[_numGlyphs * 18 * 8];
	stream->read(_glyphs, _numGlyphs * 18 * 8);

	// TODO: Read something else?
	//uint16 unknown = fontFile->readByte();
	//warning("unknown = %d", unknown);
	//warning("pos = %d", fontFile->pos());
	//warning("left = %d", fontFile->size() - fontFile->pos());

	//while (!fontFile->eos()) {
	//unknown = fontFile->readByte();
	//warning("val = %d", unknown);
	//}

	// Precalculate glyph widths
	_glyphWidths = new byte[_numGlyphs];
	for (uint16 i = 0; i < _numGlyphs; i++) {
		_glyphWidths[i] = getGlyphWidth(i);
	}

	delete stream;

	return true;
}


uint16 Font::getCharGlyph(uint16 c) const {
	//warning("%c", c);
	if (c >= 0x200)
		error("[Font::getCharGlyph] Invalid character %d", c);

	return _charMap[c];
}

byte *Font::getGlyphImg(uint16 g) {
	if (!_glyphs)
		error("[Font::getGlyphImg] Invalid glyphs");

	if (g >= _numGlyphs)
		error("[Font::getGlyphImg] Invalid glyph %d (%d available)", g, _numGlyphs);

	return _glyphs + g * 18 * 8;
}

uint8 Font::getGlyphWidth(uint16 g) {
	byte *p = getGlyphImg(g);

	uint8 maxLineWidth = 0;
	for (int j = 0; j < 18; j++) {
		uint8 currentLineWidth = 0;
		for (uint8 i = 0; i < 16; i++) {
			byte index;
			if (i % 2)
				index = *p & 0xf;
			else
				index = *p >> 4;
			uint16 color = _palette[index];
			if (color != 0x1f)
				currentLineWidth = i;
			if (i % 2)
				p++;
		}
		if (currentLineWidth > maxLineWidth)
			maxLineWidth = currentLineWidth;
	}

	return maxLineWidth;
}

byte *Font::getCharImg(uint16 c) {
	return getGlyphImg(getCharGlyph(c));
}

uint8 Font::getCharWidth(uint16 c) const{
	if (c == 0x20) {
		// Space is a special case
		// TODO: this is an arbitrary value
		return 10;
	} else {
		if (!_glyphWidths)
			error("[Font::getCharWidth] Invalid glyphs widths");

		return _glyphWidths[getCharGlyph(c)];
	}
}

uint16 Font::getStringWidth(Common::String str) const {
	uint16 width = 0;
	for (uint i = 0; i < str.size(); i++)
		width += getCharWidth((unsigned char)str[i]);

	return width;
}

uint16 Font::getStringWidth(const uint16 *str, uint16 length) const {
	uint16 width = 0;
	for (uint i = 0; i < length; i++)
		width += getCharWidth(str[i]);

	return width;
}

void Font::drawChar(Graphics::Surface *surface, int16 x, int16 y, uint16 c) {
	byte *p = getCharImg(c);

	for (int16 j = 0; j < 18; j++) {
		for (int16 i = 0; i < 16; i++) {
			byte index;
			if (i % 2)
				index = *p & 0xf;
			else
				index = *p >> 4;
			uint16 color = _palette[index];
			if (color != 0x1f) {
				surface->fillRect(Common::Rect(x+i, y+j, x+i+1, y+j+1), color);
			}
			if (i % 2)
				p++;
		}
	}
}

Common::Rect Font::drawString(Graphics::Surface *surface, int16 x, int16 y, Common::String str) {
	int16 currentX = x;
	for (uint i = 0; i < str.size(); i++) {
		drawChar(surface, currentX, y, (unsigned char)str[i]);
		currentX += getCharWidth((unsigned char)str[i]);
	}

	return Common::Rect(x, y, x + currentX, y + (int16)_charHeight);
}

Common::Rect Font::drawString(Graphics::Surface *surface, int16 x, int16 y, const uint16 *str, uint16 length) {
	int16 currentX = x;
	for (uint i = 0; i < length; i++) {
		drawChar(surface, currentX, y, str[i]);
		currentX += getCharWidth(str[i]);
	}

	return Common::Rect(x, y, x + currentX, y + (int16)_charHeight);
}

} // End of namespace LastExpress
