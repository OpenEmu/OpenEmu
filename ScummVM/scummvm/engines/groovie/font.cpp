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

#include "common/array.h"
#include "common/textconsole.h"
#include "graphics/surface.h"

#include "groovie/font.h"

namespace Groovie {

T7GFont::T7GFont() : _maxHeight(0), _maxWidth(0), _glyphs(0) {
}

T7GFont::~T7GFont() {
	delete[] _glyphs;
}

bool T7GFont::load(Common::SeekableReadStream &stream) {
	// Read the mapping of characters to glyphs
	if (stream.read(_mapChar2Glyph, 128) < 128) {
		error("Groovie::T7GFont: Couldn't read the character to glyph map");
		return false;
	}

	// Calculate the number of glyphs
	byte numGlyphs = 0;
	for (int i = 0; i < 128; i++)
		if (_mapChar2Glyph[i] >= numGlyphs)
			numGlyphs = _mapChar2Glyph[i] + 1;

	// Read the glyph offsets
	uint16 *glyphOffsets = new uint16[numGlyphs];
	for (int i = 0; i < numGlyphs; i++)
		glyphOffsets[i] = stream.readUint16LE();

	if (stream.eos()) {
		error("Groovie::T7GFont: Couldn't read the glyph offsets");
		delete[] glyphOffsets;
		return false;
	}

	// Allocate the glyph data
	delete[] _glyphs;
	_glyphs = new Glyph[numGlyphs];

	// Ensure we're ready to read the first glyph. (Most versions don't
	// need it, but the russian one does. This fixes bug #3095031.)
	stream.seek(glyphOffsets[0]);

	// Read the glyphs
	_maxHeight = _maxWidth = 0;
	for (int i = 0; (i < numGlyphs) && !stream.eos(); i++) {
		// Verify we're at the expected stream position
		if (stream.pos() != glyphOffsets[i]) {
			uint16 offset = glyphOffsets[i];
			delete[] glyphOffsets;
			error("Groovie::T7GFont: Glyph %d starts at %d but the current "
				"offset is %d", i, offset, stream.pos());
			return false;
		}

		// Read the glyph information
		Glyph *g = &_glyphs[i];
		g->width = stream.readByte();
		g->julia = stream.readByte();

		// Read the pixels data into a dynamic array (we don't know its length)
		Common::Array<byte> data;
		data.reserve(300);
		byte b = stream.readByte();
		while (!stream.eos() && (b != 0xFF)) {
			data.push_back(b);
			b = stream.readByte();
		}

		// Verify the pixel data size
		assert (data.size() % g->width == 0);
		g->height = data.size() / g->width;

		// Copy the pixel data into the definitive static array
		g->pixels = new byte[data.size()];
		memcpy(g->pixels, data.begin(), data.size());

		// Update the max values
		if (g->width > _maxWidth)
			_maxWidth = g->width;
		if (g->height > _maxHeight)
			_maxHeight = g->height;
	}

	delete[] glyphOffsets;
	return true;
}

void T7GFont::drawChar(Graphics::Surface *dst, byte chr, int x, int y, uint32 color) const {
	// We ignore the color, as the font is already colored
	const Glyph *glyph = getGlyph(chr);
	const byte *src = glyph->pixels;
	byte *target = (byte *)dst->getBasePtr(x, y);

	for (int i = 0; i < glyph->height; i++) {
		memcpy(target, src, glyph->width);
		src += glyph->width;
		target += dst->pitch;
	}
}

const T7GFont::Glyph *T7GFont::getGlyph(byte chr) const {
	assert (chr < 128);

	byte numGlyph = _mapChar2Glyph[chr];
	return &_glyphs[numGlyph];
}

} // End of Groovie namespace
