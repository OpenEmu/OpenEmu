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


#include "scumm/scumm.h"
#include "scumm/file.h"
#include "scumm/nut_renderer.h"
#include "scumm/util.h"

namespace Scumm {

NutRenderer::NutRenderer(ScummEngine *vm, const char *filename) :
	_vm(vm),
	_numChars(0),
	_maxCharSize(0),
	_charBuffer(0),
	_decodedData(0) {
	memset(_chars, 0, sizeof(_chars));
	loadFont(filename);
}

NutRenderer::~NutRenderer() {
	delete[] _charBuffer;
	delete[] _decodedData;
}

void smush_decode_codec1(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);

void NutRenderer::codec1(byte *dst, const byte *src, int width, int height, int pitch) {
	smush_decode_codec1(dst, src, 0, 0, width, height, pitch);
	for (int i = 0; i < width * height; i++)
		_paletteMap[dst[i]] = 1;
}

void NutRenderer::codec21(byte *dst, const byte *src, int width, int height, int pitch) {
	while (height--) {
		byte *dstPtrNext = dst + pitch;
		const byte *srcPtrNext = src + 2 + READ_LE_UINT16(src);
		src += 2;
		int len = width;
		do {
			int offs = READ_LE_UINT16(src); src += 2;
			dst += offs;
			len -= offs;
			if (len <= 0) {
				break;
			}
			int w = READ_LE_UINT16(src) + 1; src += 2;
			len -= w;
			if (len < 0) {
				w += len;
			}
			// the original codec44 handles this part slightly differently (this is the only difference with codec21) :
			//  src bytes equal to 255 are replaced by 0 in dst
			//  src bytes equal to 1 are replaced by a color passed as an argument in the original function
			//  other src bytes values are copied as-is
			for (int i = 0; i < w; i++) {
				_paletteMap[src[i]] = 1;
			}
			memcpy(dst, src, w);
			dst += w;
			src += w;
		} while (len > 0);
		dst = dstPtrNext;
		src = srcPtrNext;
	}
}

void NutRenderer::loadFont(const char *filename) {
	ScummFile file;
	_vm->openFile(file, filename);
	if (!file.isOpen()) {
		error("NutRenderer::loadFont() Can't open font file: %s", filename);
	}

	uint32 tag = file.readUint32BE();
	if (tag != MKTAG('A','N','I','M')) {
		error("NutRenderer::loadFont() there is no ANIM chunk in font header");
	}

	uint32 length = file.readUint32BE();
	byte *dataSrc = new byte[length];
	file.read(dataSrc, length);
	file.close();

	if (READ_BE_UINT32(dataSrc) != MKTAG('A','H','D','R')) {
		error("NutRenderer::loadFont() there is no AHDR chunk in font header");
	}

	// We pre-decode the font, which may seem wasteful at first. Actually,
	// the memory needed for just the decoded glyphs is smaller than the
	// whole of the undecoded font file.

	_numChars = READ_LE_UINT16(dataSrc + 10);
	assert(_numChars <= ARRAYSIZE(_chars));

	uint32 offset = 0;
	uint32 decodedLength = 0;
	int l;

	_paletteMap = new byte[256];
	for (l = 0; l < 256; l++) {
		_paletteMap[l] = 0;
	}

	for (l = 0; l < _numChars; l++) {
		offset += READ_BE_UINT32(dataSrc + offset + 4) + 16;
		int width = READ_LE_UINT16(dataSrc + offset + 14);
		int height = READ_LE_UINT16(dataSrc + offset + 16);
		int size = width * height;
		decodedLength += size;
		if (size > _maxCharSize)
			_maxCharSize = size;
	}

	debug(1, "NutRenderer::loadFont('%s') - decodedLength = %d", filename, decodedLength);

	_decodedData = new byte[decodedLength];
	byte *decodedPtr = _decodedData;

	offset = 0;
	for (l = 0; l < _numChars; l++) {
		offset += READ_BE_UINT32(dataSrc + offset + 4) + 8;
		if (READ_BE_UINT32(dataSrc + offset) != MKTAG('F','R','M','E')) {
			error("NutRenderer::loadFont(%s) there is no FRME chunk %d (offset %x)", filename, l, offset);
			break;
		}
		offset += 8;
		if (READ_BE_UINT32(dataSrc + offset) != MKTAG('F','O','B','J')) {
			error("NutRenderer::loadFont(%s) there is no FOBJ chunk in FRME chunk %d (offset %x)", filename, l, offset);
			break;
		}
		int codec = READ_LE_UINT16(dataSrc + offset + 8);
		// _chars[l].xoffs = READ_LE_UINT16(dataSrc + offset + 10);
		// _chars[l].yoffs = READ_LE_UINT16(dataSrc + offset + 12);
		_chars[l].width = READ_LE_UINT16(dataSrc + offset + 14);
		_chars[l].height = READ_LE_UINT16(dataSrc + offset + 16);
		_chars[l].src = decodedPtr;

		decodedPtr += (_chars[l].width * _chars[l].height);

		// If characters have transparency, then bytes just get skipped and
		// so there may appear some garbage. That's why we have to fill it
		// with a default color first.
		if (codec == 44) {
			memset(_chars[l].src, kSmush44TransparentColor, _chars[l].width * _chars[l].height);
			_paletteMap[kSmush44TransparentColor] = 1;
			_chars[l].transparency = kSmush44TransparentColor;
		} else {
			memset(_chars[l].src, kDefaultTransparentColor, _chars[l].width * _chars[l].height);
			_paletteMap[kDefaultTransparentColor] = 1;
			_chars[l].transparency = kDefaultTransparentColor;
		}

		const uint8 *fobjptr = dataSrc + offset + 22;
		switch (codec) {
		case 1:
			codec1(_chars[l].src, fobjptr, _chars[l].width, _chars[l].height, _chars[l].width);
			break;
		case 21:
		case 44:
			codec21(_chars[l].src, fobjptr, _chars[l].width, _chars[l].height, _chars[l].width);
			break;
		default:
			error("NutRenderer::loadFont: unknown codec: %d", codec);
		}
	}

	// We have decoded the font. Now let's see if we can re-compress it to
	// a more compact format. Start by counting the number of colors.

	int numColors = 0;
	for (l = 0; l < 256; l++) {
		if (_paletteMap[l]) {
			if (numColors < ARRAYSIZE(_palette)) {
				_paletteMap[l] = numColors;
				_palette[numColors] = l;
			}
			numColors++;
		}
	}

	// Now _palette contains all the used colors, and _paletteMap maps the
	// real color to the palette index.

	if (numColors <= 2)
		_bpp = 1;
	else if (numColors <= 4)
		_bpp = 2;
	else if (numColors <= 16)
		_bpp = 4;
	else
		_bpp = 8;

	if (_bpp < 8) {
		int compressedLength = 0;
		for (l = 0; l < 256; l++) {
			compressedLength += (((_bpp * _chars[l].width + 7) / 8) * _chars[l].height);
		}

		debug(1, "NutRenderer::loadFont('%s') - compressedLength = %d (%d bpp)", filename, compressedLength, _bpp);

		byte *compressedData = new byte[compressedLength];
		memset(compressedData, 0, compressedLength);

		offset = 0;

		for (l = 0; l < 256; l++) {
			byte *src = _chars[l].src;
			byte *dst = compressedData + offset;
			int srcPitch = _chars[l].width;
			int dstPitch = (_bpp * _chars[l].width + 7) / 8;

			for (int h = 0; h < _chars[l].height; h++) {
				byte bit = 0x80;
				byte *nextDst = dst + dstPitch;
				for (int w = 0; w < srcPitch; w++) {
					byte color = _paletteMap[src[w]];
					for (int i = 0; i < _bpp; i++) {
						if (color & (1 << i))
							*dst |= bit;
						bit >>= 1;
					}
					if (!bit) {
						bit = 0x80;
						dst++;
					}
				}
				src += srcPitch;
				dst = nextDst;
			}
			_chars[l].src = compressedData + offset;
			offset += (dstPitch * _chars[l].height);
		}

		delete[] _decodedData;
		_decodedData = compressedData;

		_charBuffer = new byte[_maxCharSize];
	}

	delete[] dataSrc;
	delete[] _paletteMap;
}

int NutRenderer::getCharWidth(byte c) const {
	if (c >= 0x80 && _vm->_useCJKMode)
		return _vm->_2byteWidth / 2;

	if (c >= _numChars)
		error("invalid character in NutRenderer::getCharWidth : %d (%d)", c, _numChars);

	return _chars[c].width;
}

int NutRenderer::getCharHeight(byte c) const {
	if (c >= 0x80 && _vm->_useCJKMode)
		return _vm->_2byteHeight;

	if (c >= _numChars)
		error("invalid character in NutRenderer::getCharHeight : %d (%d)", c, _numChars);

	return _chars[c].height;
}

byte *NutRenderer::unpackChar(byte c) {
	if (_bpp == 8)
		return _chars[c].src;

	byte *src = _chars[c].src;
	int pitch = (_bpp * _chars[c].width + 7) / 8;

	for (int ty = 0; ty < _chars[c].height; ty++) {
		for (int tx = 0; tx < _chars[c].width; tx++) {
			byte val;
			int offset;
			byte bit;

			switch (_bpp) {
			case 1:
				offset = tx / 8;
				bit = 0x80 >> (tx % 8);
				break;
			case 2:
				offset = tx / 4;
				bit = 0x80 >> (2 * (tx % 4));
				break;
			default:
				offset = tx / 2;
				bit = 0x80 >> (4 * (tx % 2));
				break;
			}

			val = 0;

			for (int i = 0; i < _bpp; i++) {
				if (src[offset] & (bit >> i))
					val |= (1 << i);
			}

			_charBuffer[ty * _chars[c].width + tx] = _palette[val];
		}
		src += pitch;
	}

	return _charBuffer;
}

void NutRenderer::drawFrame(byte *dst, int c, int x, int y) {
	const int width = MIN((int)_chars[c].width, _vm->_screenWidth - x);
	const int height = MIN((int)_chars[c].height, _vm->_screenHeight - y);
	const byte *src = unpackChar(c);
	const int srcPitch = _chars[c].width;
	byte bits = 0;

	const int minX = x < 0 ? -x : 0;
	const int minY = y < 0 ? -y : 0;

	if (height <= 0 || width <= 0) {
		return;
	}

	dst += _vm->_screenWidth * y + x;
	if (minY) {
		src += minY * srcPitch;
		dst += minY * _vm->_screenWidth;
	}

	for (int ty = minY; ty < height; ty++) {
		for (int tx = minX; tx < width; tx++) {
			bits = src[tx];
			if (bits != 231 && bits) {
				dst[tx] = bits;
			}
		}
		src += srcPitch;
		dst += _vm->_screenWidth;
	}
}

void NutRenderer::drawChar(const Graphics::Surface &s, byte c, int x, int y, byte color) {
	byte *dst = (byte *)s.pixels + y * s.pitch + x;
	const int width = MIN((int)_chars[c].width, s.w - x);
	const int height = MIN((int)_chars[c].height, s.h - y);
	const byte *src = unpackChar(c);
	int srcPitch = _chars[c].width;

	const int minX = x < 0 ? -x : 0;
	const int minY = y < 0 ? -y : 0;

	if (height <= 0 || width <= 0) {
		return;
	}

	if (minY) {
		src += minY * srcPitch;
		dst += minY * s.pitch;
	}

	for (int ty = minY; ty < height; ty++) {
		for (int tx = minX; tx < width; tx++) {
			if (src[tx] != _chars[c].transparency) {
				if (src[tx] == 1) {
					dst[tx] = color;
				} else {
					dst[tx] = src[tx];
				}
			}
		}
		src += srcPitch;
		dst += s.pitch;
	}
}

void NutRenderer::draw2byte(const Graphics::Surface &s, int c, int x, int y, byte color) {
	byte *dst = (byte *)s.pixels + y * s.pitch + x;
	const int width = _vm->_2byteWidth;
	const int height = MIN(_vm->_2byteHeight, s.h - y);
	const byte *src = _vm->get2byteCharPtr(c);
	byte bits = 0;

	if (height <= 0 || width <= 0) {
		return;
	}

	for (int ty = 0; ty < height; ty++) {
		for (int tx = 0; tx < width; tx++) {
			if ((tx & 7) == 0)
				bits = *src++;
			if (x + tx < 0 || x + tx >= s.w || y + ty < 0)
				continue;
			if (bits & revBitMask(tx % 8)) {
				dst[tx] = color;
			}
		}
		dst += s.pitch;
	}
}

} // End of namespace Scumm
