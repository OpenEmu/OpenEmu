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

#include "graphics/fonts/bdf.h"

#include "common/file.h"
#include "common/endian.h"
#include "common/textconsole.h"

#include "graphics/surface.h"

namespace Graphics {

BdfFont::BdfFont(const BdfFontData &data, DisposeAfterUse::Flag dispose)
	: _data(data), _dispose(dispose) {
}

BdfFont::~BdfFont() {
	if (_dispose == DisposeAfterUse::YES) {
		for (int i = 0; i < _data.numCharacters; ++i)
			delete[] _data.bitmaps[i];
		delete[] _data.bitmaps;
		delete[] _data.advances;
		delete[] _data.boxes;
	}
}

int BdfFont::getFontHeight() const {
	return _data.height;
}

int BdfFont::getMaxCharWidth() const {
	return _data.maxAdvance;
}

int BdfFont::getCharWidth(byte chr) const {
	// In case all font have the same advance value, we use the maximum.
	if (!_data.advances)
		return _data.maxAdvance;

	const int ch = mapToIndex(chr);
	// In case no mapping exists, we use the maximum advance.
	if (ch < 0)
		return _data.maxAdvance;
	else
		return _data.advances[ch];
}


template<typename PixelType>
void drawCharIntern(byte *ptr, uint pitch, const byte *src, int h, int width, int minX, int maxX, const PixelType color) {
	byte data = 0;
	while (h--) {
		PixelType *dst = (PixelType *)ptr;

		for (int x = 0; x < width; ++x) {
			if (!(x % 8))
				data = *src++;

			if (x >= minX && x <= maxX && (data & 0x80))
				dst[x] = color;

			data <<= 1;
		}

		ptr += pitch;
	}
}

int BdfFont::mapToIndex(byte ch) const {
	// Check whether the character is included
	if (_data.firstCharacter <= ch && ch <= _data.firstCharacter + _data.numCharacters) {
		if (_data.bitmaps[ch - _data.firstCharacter])
			return ch - _data.firstCharacter;
	}

	return _data.defaultCharacter - _data.firstCharacter;
}

void BdfFont::drawChar(Surface *dst, byte chr, const int tx, const int ty, const uint32 color) const {
	assert(dst != 0);

	// TODO: Where is the relation between the max advance being smaller or
	// equal to 50 and the decision of the theme designer?
	// asserting _data.maxAdvance <= 50: let the theme designer decide what looks best
	assert(_data.maxAdvance <= 50);
	assert(dst->format.bytesPerPixel == 1 || dst->format.bytesPerPixel == 2);

	const int idx = mapToIndex(chr);
	if (idx < 0)
		return;

	int width, height, xOffset, yOffset;

	// Get the bounding box of the character
	if (!_data.boxes) {
		width = _data.defaultBox.width;
		height = _data.defaultBox.height;
		xOffset = _data.defaultBox.xOffset;
		yOffset = _data.defaultBox.yOffset;
	} else {
		width = _data.boxes[idx].width;
		height = _data.boxes[idx].height;
		xOffset = _data.boxes[idx].xOffset;
		yOffset = _data.boxes[idx].yOffset;
	}

	int y = ty + _data.ascent - yOffset - height;
	int x = tx + xOffset;

	const byte *src = _data.bitmaps[idx];

	const int bytesPerRow = (width + 7) / 8;
	const int originalWidth = width;

	// Make sure we do not draw outside the surface
	if (y < 0) {
		src -= y * bytesPerRow;
		height += y;
		y = 0;
	}

	if (y + height > dst->h)
		height = dst->h - y;

	if (height <= 0)
		return;

	int xStart = 0;
	if (x < 0) {
		xStart = -x;
		width += x;
		x = 0;
	}

	if (x + width > dst->w)
		width = dst->w - x;

	if (width <= 0)
		return;

	const int xEnd = xStart + width - 1;

	byte *ptr = (byte *)dst->getBasePtr(x, y);

	if (dst->format.bytesPerPixel == 1)
		drawCharIntern<byte>(ptr, dst->pitch, src, height, originalWidth, xStart, xEnd, color);
	else if (dst->format.bytesPerPixel == 2)
		drawCharIntern<uint16>(ptr, dst->pitch, src, height, originalWidth, xStart, xEnd, color);
}

namespace {

inline byte hexToInt(char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else
		return 0;
}

byte *loadCharacter(Common::SeekableReadStream &stream, int &encoding, int &advance, BdfBoundingBox &box) {
	Common::String line;
	byte *bitmap = 0;

	while (true) {
		line = stream.readLine();
		if (stream.err() || stream.eos()) {
			warning("BdfFont::loadCharacter: Premature end of file");
			delete[] bitmap;
			return 0;
		}

		if (line.hasPrefix("ENCODING ")) {
			if (sscanf(line.c_str(), "ENCODING %d", &encoding) != 1) {
				warning("BdfFont::loadCharacter: Invalid ENCODING");
				delete[] bitmap;
				return 0;
			}
		} else if (line.hasPrefix("DWIDTH ")) {
			int yAdvance;
			if (sscanf(line.c_str(), "DWIDTH %d %d", &advance, &yAdvance) != 2) {
				warning("BdfFont::loadCharacter: Invalid DWIDTH");
				delete[] bitmap;
				return 0;
			}

			if (yAdvance != 0) {
				warning("BdfFont::loadCharacter: Character %d has an y advance of %d", encoding, yAdvance);
				delete[] bitmap;
				return 0;
			}

			if (advance < 0) {
				warning("BdfFont::loadCharacter: Character %d has an x advance of %d", encoding, advance);
				delete[] bitmap;
				return 0;
			}
		} else if (line.hasPrefix("BBX ")) {
			int width, height, xOffset, yOffset;
			if (sscanf(line.c_str(), "BBX %d %d %d %d",
			           &width, &height, &xOffset, &yOffset) != 4) {
				warning("BdfFont::loadCharacter: Invalid BBX");
				delete[] bitmap;
				return 0;
			}

			box.width = width;
			box.height = height;
			box.xOffset = xOffset;
			box.yOffset = yOffset;
		} else if (line == "BITMAP") {
			const uint bytesPerRow = (box.width + 7) / 8;
			byte *dst = bitmap = new byte[box.height * bytesPerRow];

			for (int y = 0; y < box.height; ++y) {
				line = stream.readLine();
				if (stream.err() || stream.eos()) {
					warning("BdfFont::loadCharacter: Premature end of file");
					delete[] bitmap;
					return 0;
				}

				if (line.size() != 2 * bytesPerRow) {
					warning("BdfFont::loadCharacter: Pixel line has wrong size");
					delete[] bitmap;
					return 0;
				}

				for (uint x = 0; x < bytesPerRow; ++x) {
					char nibble1 = line[x * 2 + 0];
					char nibble2 = line[x * 2 + 1];
					*dst++ = (hexToInt(nibble1) << 4) | hexToInt(nibble2);
				}
			}
		} else if (line == "ENDCHAR") {
			return bitmap;
		}
	}

	delete[] bitmap;
	return 0;
}

void freeBitmaps(byte **bitmaps, int size) {
	for (int i = 0; i < size; ++i)
		delete[] bitmaps[i];
}

} // End of anonymous namespace

BdfFont *BdfFont::loadFont(Common::SeekableReadStream &stream) {
	BdfFontData font;
	memset(&font, 0, sizeof(font));
	font.ascent = -1;
	font.defaultCharacter = -1;

	// We only load the first 256 characters
	font.numCharacters = 256;
	byte **bitmaps = new byte *[font.numCharacters];
	memset(bitmaps, 0, sizeof(byte *) * font.numCharacters);
	byte *advances = new byte[font.numCharacters];
	BdfBoundingBox *boxes = new BdfBoundingBox[font.numCharacters];

	int descent = -1;

	Common::String line;
	while (true) {
		line = stream.readLine();
		if (stream.err() || stream.eos()) {
			warning("BdfFont::loadFont: Premature end of file");
			freeBitmaps(bitmaps, font.numCharacters);
			delete[] bitmaps;
			delete[] advances;
			delete[] boxes;
			return 0;
		}

		// Only parse and handle declarations we actually need
		if (line.hasPrefix("FONTBOUNDINGBOX ")) {
			int width, height, xOffset, yOffset;
			if (sscanf(line.c_str(), "FONTBOUNDINGBOX %d %d %d %d",
			           &width, &height, &xOffset, &yOffset) != 4) {
				warning("BdfFont::loadFont: Invalid FONTBOUNDINGBOX");
				freeBitmaps(bitmaps, font.numCharacters);
				delete[] bitmaps;
				delete[] advances;
				delete[] boxes;
				return 0;
			}

			font.defaultBox.width = width;
			font.defaultBox.height = height;
			font.defaultBox.xOffset = xOffset;
			font.defaultBox.yOffset = yOffset;
		} else if (line.hasPrefix("FONT_ASCENT ")) {
			if (sscanf(line.c_str(), "FONT_ASCENT %d", &font.ascent) != 1) {
				warning("BdfFont::loadFont: Invalid FONT_ASCENT");
				freeBitmaps(bitmaps, font.numCharacters);
				delete[] bitmaps;
				delete[] advances;
				delete[] boxes;
				return 0;
			}
		} else if (line.hasPrefix("FONT_DESCENT ")) {
			if (sscanf(line.c_str(), "FONT_DESCENT %d", &descent) != 1) {
				warning("BdfFont::loadFont: Invalid FONT_DESCENT");
				freeBitmaps(bitmaps, font.numCharacters);
				delete[] bitmaps;
				delete[] advances;
				delete[] boxes;
				return 0;
			}
		} else if (line.hasPrefix("DEFAULT_CHAR ")) {
			if (sscanf(line.c_str(), "DEFAULT_CHAR %d", &font.defaultCharacter) != 1) {
				warning("BdfFont::loadFont: Invalid DEFAULT_CHAR");
				freeBitmaps(bitmaps, font.numCharacters);
				delete[] bitmaps;
				delete[] advances;
				delete[] boxes;
				return 0;
			}
		} else if (line.hasPrefix("STARTCHAR ")) {
			BdfBoundingBox box = font.defaultBox;
			int encoding = -1;
			int advance = -1;
			byte *bitmap = loadCharacter(stream, encoding, advance, box);

			// Ignore all characters above 255.
			if (encoding < -1 || encoding >= font.numCharacters) {
				delete[] bitmap;
				encoding = -1;
			}

			// Calculate the max advance
			if (encoding != -1 && advance > font.maxAdvance)
				font.maxAdvance = advance;

			if (!bitmap && encoding != -1) {
				warning("BdfFont::loadFont: Character %d invalid", encoding);
				freeBitmaps(bitmaps, font.numCharacters);
				delete[] bitmaps;
				delete[] advances;
				delete[] boxes;
				return 0;
			}

			if (encoding != -1) {
				bitmaps[encoding] = bitmap;
				advances[encoding] = advance;
				boxes[encoding] = box;
			}
		} else if (line == "ENDFONT") {
			break;
		}
	}

	if (font.ascent < 0 || descent < 0) {
		warning("BdfFont::loadFont: Invalid ascent or descent");
		freeBitmaps(bitmaps, font.numCharacters);
		delete[] bitmaps;
		delete[] advances;
		delete[] boxes;
		return 0;
	}

	font.height = font.ascent + descent;

	font.bitmaps = bitmaps;
	font.advances = advances;
	font.boxes = boxes;

	int firstCharacter = font.numCharacters;
	int lastCharacter = -1;
	bool hasFixedBBox = true;
	bool hasFixedAdvance = true;

	for (int i = 0; i < font.numCharacters; ++i) {
		if (!font.bitmaps[i])
			continue;

		if (i < firstCharacter)
			firstCharacter = i;

		if (i > lastCharacter)
			lastCharacter = i;

		if (font.advances[i] != font.maxAdvance)
			hasFixedAdvance = false;

		const BdfBoundingBox &bbox = font.boxes[i];
		if (bbox.width != font.defaultBox.width
		    || bbox.height != font.defaultBox.height
		    || bbox.xOffset != font.defaultBox.xOffset
		    || bbox.yOffset != font.defaultBox.yOffset)
			hasFixedBBox = false;
	}

	if (lastCharacter == -1) {
		warning("BdfFont::loadFont: No glyphs found");
		delete[] font.bitmaps;
		delete[] font.advances;
		delete[] font.boxes;
		return 0;
	}

	// Free the advance table, in case all glyphs use the same advance
	if (hasFixedAdvance) {
		delete[] font.advances;
		font.advances = 0;
	}

	// Free the box table, in case all glyphs use the same box
	if (hasFixedBBox) {
		delete[] font.boxes;
		font.boxes = 0;
	}

	// Adapt for the fact that we never use encoding 0.
	if (font.defaultCharacter < firstCharacter
	    || font.defaultCharacter > lastCharacter)
		font.defaultCharacter = -1;

	font.firstCharacter = firstCharacter;

	const int charsAvailable = lastCharacter - firstCharacter + 1;
	// Try to compact the tables
	if (charsAvailable < font.numCharacters) {
		byte **newBitmaps = new byte *[charsAvailable];
		boxes = 0;
		advances = 0;
		if (!hasFixedBBox)
			boxes = new BdfBoundingBox[charsAvailable];
		if (!hasFixedAdvance)
			advances = new byte[charsAvailable];

		for (int i = 0; i < charsAvailable; ++i) {
			const int encoding = i + firstCharacter;
			if (font.bitmaps[encoding]) {
				newBitmaps[i] = bitmaps[encoding];

				if (!hasFixedBBox)
					boxes[i] = font.boxes[encoding];
				if (!hasFixedAdvance)
					advances[i] = font.advances[encoding];
			} else {
				newBitmaps[i] = 0;
			}
		}

		delete[] font.bitmaps;
		font.bitmaps = newBitmaps;
		delete[] font.advances;
		font.advances = advances;
		delete[] font.boxes;
		font.boxes = boxes;

		font.numCharacters = charsAvailable;
	}

	return new BdfFont(font, DisposeAfterUse::YES);
}

#define BDF_FONTCACHE_TAG MKTAG('S', 'V', 'F', 'C')
#define BDF_FONTCACHE_VERSION 1

bool BdfFont::cacheFontData(const BdfFont &font, const Common::String &filename) {
	Common::DumpFile cacheFile;
	if (!cacheFile.open(filename)) {
		warning("BdfFont::cacheFontData: Couldn't open file '%s' for writing", filename.c_str());
		return false;
	}

	const BdfFontData &data = font._data;

	cacheFile.writeUint32BE(BDF_FONTCACHE_TAG);
	cacheFile.writeUint32BE(BDF_FONTCACHE_VERSION);
	cacheFile.writeUint16BE(data.maxAdvance);
	cacheFile.writeByte(data.height);
	cacheFile.writeByte(data.defaultBox.width);
	cacheFile.writeByte(data.defaultBox.height);
	cacheFile.writeSByte(data.defaultBox.xOffset);
	cacheFile.writeSByte(data.defaultBox.yOffset);
	cacheFile.writeByte(data.ascent);
	cacheFile.writeUint16BE(data.firstCharacter);
	cacheFile.writeSint16BE(data.defaultCharacter);
	cacheFile.writeUint16BE(data.numCharacters);

	for (int i = 0; i < data.numCharacters; ++i) {
		const BdfBoundingBox &box = data.boxes ? data.boxes[i] : data.defaultBox;
		if (data.bitmaps[i]) {
			const int bytes = ((box.width + 7) / 8) * box.height;
			cacheFile.writeUint32BE(bytes);
			cacheFile.write(data.bitmaps[i], bytes);
		} else {
			cacheFile.writeUint32BE(0);
		}
	}

	if (data.advances) {
		cacheFile.writeByte(0xFF);
		cacheFile.write(data.advances, data.numCharacters);
	} else {
		cacheFile.writeByte(0x00);
	}

	if (data.boxes) {
		cacheFile.writeByte(0xFF);

		for (int i = 0; i < data.numCharacters; ++i) {
			const BdfBoundingBox &box = data.boxes[i];
			cacheFile.writeByte(box.width);
			cacheFile.writeByte(box.height);
			cacheFile.writeSByte(box.xOffset);
			cacheFile.writeSByte(box.yOffset);
		}
	} else {
		cacheFile.writeByte(0x00);
	}

	return !cacheFile.err();
}

BdfFont *BdfFont::loadFromCache(Common::SeekableReadStream &stream) {
	const uint32 magic = stream.readUint32BE();
	if (magic != BDF_FONTCACHE_TAG)
		return 0;

	const uint32 version = stream.readUint32BE();
	if (version != BDF_FONTCACHE_VERSION)
		return 0;

	BdfFontData data;

	data.maxAdvance = stream.readUint16BE();
	data.height = stream.readByte();
	data.defaultBox.width = stream.readByte();
	data.defaultBox.height = stream.readByte();
	data.defaultBox.xOffset = stream.readSByte();
	data.defaultBox.yOffset = stream.readSByte();
	data.ascent = stream.readByte();
	data.firstCharacter = stream.readUint16BE();
	data.defaultCharacter = stream.readSint16BE();
	data.numCharacters = stream.readUint16BE();

	if (stream.err() || stream.eos())
		return 0;

	byte **bitmaps = new byte *[data.numCharacters];
	byte *advances = 0;
	BdfBoundingBox *boxes = 0;
	for (int i = 0; i < data.numCharacters; ++i) {
		uint32 size = stream.readUint32BE();

		if (stream.err() || stream.eos()) {
			for (int j = 0; j < i; ++j)
				delete[] bitmaps[i];
			delete[] bitmaps;
			return 0;
		}

		if (size) {
			bitmaps[i] = new byte[size];
			stream.read(bitmaps[i], size);
		} else {
			bitmaps[i] = 0;
		}
	}


	if (stream.readByte() == 0xFF) {
		advances = new byte[data.numCharacters];
		stream.read(advances, data.numCharacters);
	}

	if (stream.readByte() == 0xFF) {
		boxes = new BdfBoundingBox[data.numCharacters];
		for (int i = 0; i < data.numCharacters; ++i) {
			boxes[i].width = stream.readByte();
			boxes[i].height = stream.readByte();
			boxes[i].xOffset = stream.readSByte();
			boxes[i].yOffset = stream.readSByte();
		}
	}

	if (stream.eos() || stream.err()) {
		for (int i = 0; i < data.numCharacters; ++i)
			delete[] bitmaps[i];
		delete[] bitmaps;
		delete[] advances;
		delete[] boxes;
		return 0;
	}

	data.bitmaps = bitmaps;
	data.advances = advances;
	data.boxes = boxes;
	return new BdfFont(data, DisposeAfterUse::YES);
}

} // End of namespace Graphics
