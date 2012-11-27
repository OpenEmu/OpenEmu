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

#include "video/codecs/cinepak.h"

#include "common/debug.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "graphics/surface.h"

// Code here partially based off of ffmpeg ;)

namespace Video {

// Convert a color from YUV to RGB colorspace, Cinepak style.
inline static void CPYUV2RGB(byte y, byte u, byte v, byte &r, byte &g, byte &b) {
	r = CLIP<int>(y + 2 * (v - 128), 0, 255);
	g = CLIP<int>(y - (u - 128) / 2 - (v - 128), 0, 255);
	b = CLIP<int>(y + 2 * (u - 128), 0, 255);
}

#define PUT_PIXEL(offset, lum, u, v) \
	if (_pixelFormat.bytesPerPixel != 1) { \
		CPYUV2RGB(lum, u, v, r, g, b); \
		if (_pixelFormat.bytesPerPixel == 2) \
			*((uint16 *)_curFrame.surface->pixels + offset) = _pixelFormat.RGBToColor(r, g, b); \
		else \
			*((uint32 *)_curFrame.surface->pixels + offset) = _pixelFormat.RGBToColor(r, g, b); \
	} else \
		*((byte *)_curFrame.surface->pixels + offset) = lum

CinepakDecoder::CinepakDecoder(int bitsPerPixel) : Codec() {
	_curFrame.surface = NULL;
	_curFrame.strips = NULL;
	_y = 0;

	if (bitsPerPixel == 8)
		_pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
	else
		_pixelFormat = g_system->getScreenFormat();
}

CinepakDecoder::~CinepakDecoder() {
	if (_curFrame.surface) {
		_curFrame.surface->free();
		delete _curFrame.surface;
	}

	delete[] _curFrame.strips;
}

const Graphics::Surface *CinepakDecoder::decodeImage(Common::SeekableReadStream *stream) {
	_curFrame.flags = stream->readByte();
	_curFrame.length = (stream->readByte() << 16);
	_curFrame.length |= stream->readUint16BE();
	_curFrame.width = stream->readUint16BE();
	_curFrame.height = stream->readUint16BE();
	_curFrame.stripCount = stream->readUint16BE();

	if (_curFrame.strips == NULL)
		_curFrame.strips = new CinepakStrip[_curFrame.stripCount];

	debug (4, "Cinepak Frame: Width = %d, Height = %d, Strip Count = %d", _curFrame.width, _curFrame.height, _curFrame.stripCount);

	// Borrowed from FFMPEG. This should cut out the extra data Cinepak for Sega has (which is useless).
	// The theory behind this is that this is here to confuse standard Cinepak decoders. But, we won't let that happen! ;)
	if (_curFrame.length != (uint32)stream->size()) {
		if (stream->readUint16BE() == 0xFE00)
			stream->readUint32BE();
		else if ((stream->size() % _curFrame.length) == 0)
			stream->seek(-2, SEEK_CUR);
	}

	if (!_curFrame.surface) {
		_curFrame.surface = new Graphics::Surface();
		_curFrame.surface->create(_curFrame.width, _curFrame.height, _pixelFormat);
	}

	// Reset the y variable.
	_y = 0;

	for (uint16 i = 0; i < _curFrame.stripCount; i++) {
		if (i > 0 && !(_curFrame.flags & 1)) { // Use codebooks from last strip
			for (uint16 j = 0; j < 256; j++) {
				_curFrame.strips[i].v1_codebook[j] = _curFrame.strips[i - 1].v1_codebook[j];
				_curFrame.strips[i].v4_codebook[j] = _curFrame.strips[i - 1].v4_codebook[j];
			}
		}

		_curFrame.strips[i].id = stream->readUint16BE();
		_curFrame.strips[i].length = stream->readUint16BE() - 12; // Subtract the 12 byte header
		_curFrame.strips[i].rect.top = _y; stream->readUint16BE(); // Ignore, substitute with our own.
		_curFrame.strips[i].rect.left = 0; stream->readUint16BE(); // Ignore, substitute with our own
		_curFrame.strips[i].rect.bottom = _y + stream->readUint16BE();
		_curFrame.strips[i].rect.right = _curFrame.width; stream->readUint16BE(); // Ignore, substitute with our own

		// Sanity check. Because Cinepak is based on 4x4 blocks, the width and height of each strip needs to be divisible by 4.
		assert(!(_curFrame.strips[i].rect.width() % 4) && !(_curFrame.strips[i].rect.height() % 4));

		uint32 pos = stream->pos();

		while ((uint32)stream->pos() < (pos + _curFrame.strips[i].length) && !stream->eos()) {
			byte chunkID = stream->readByte();

			if (stream->eos())
				break;

			// Chunk Size is 24-bit, ignore the first 4 bytes
			uint32 chunkSize = stream->readByte() << 16;
			chunkSize += stream->readUint16BE() - 4;

			int32 startPos = stream->pos();

			switch (chunkID) {
			case 0x20:
			case 0x21:
			case 0x24:
			case 0x25:
				loadCodebook(stream, i, 4, chunkID, chunkSize);
				break;
			case 0x22:
			case 0x23:
			case 0x26:
			case 0x27:
				loadCodebook(stream, i, 1, chunkID, chunkSize);
				break;
			case 0x30:
			case 0x31:
			case 0x32:
				decodeVectors(stream, i, chunkID, chunkSize);
				break;
			default:
				warning("Unknown Cinepak chunk ID %02x", chunkID);
				return _curFrame.surface;
			}

			if (stream->pos() != startPos + (int32)chunkSize)
				stream->seek(startPos + chunkSize);
		}

		_y = _curFrame.strips[i].rect.bottom;
	}

	return _curFrame.surface;
}

void CinepakDecoder::loadCodebook(Common::SeekableReadStream *stream, uint16 strip, byte codebookType, byte chunkID, uint32 chunkSize) {
	CinepakCodebook *codebook = (codebookType == 1) ? _curFrame.strips[strip].v1_codebook : _curFrame.strips[strip].v4_codebook;

	int32 startPos = stream->pos();
	uint32 flag = 0, mask = 0;

	for (uint16 i = 0; i < 256; i++) {
		if ((chunkID & 0x01) && !(mask >>= 1)) {
			if ((stream->pos() - startPos + 4) > (int32)chunkSize)
				break;

			flag  = stream->readUint32BE();
			mask  = 0x80000000;
		}

		if (!(chunkID & 0x01) || (flag & mask)) {
			byte n = (chunkID & 0x04) ? 4 : 6;
			if ((stream->pos() - startPos + n) > (int32)chunkSize)
				break;

			for (byte j = 0; j < 4; j++)
				codebook[i].y[j] = stream->readByte();

			if (n == 6) {
				codebook[i].u  = stream->readByte() + 128;
				codebook[i].v  = stream->readByte() + 128;
			} else {
				// This codebook type indicates either greyscale or
				// palettized video. For greyscale, default us to
				// 128 for both u and v.
				codebook[i].u  = 128;
				codebook[i].v  = 128;
			}
		}
	}
}

void CinepakDecoder::decodeVectors(Common::SeekableReadStream *stream, uint16 strip, byte chunkID, uint32 chunkSize) {
	uint32 flag = 0, mask = 0;
	uint32 iy[4];
	int32 startPos = stream->pos();
	byte r = 0, g = 0, b = 0;

	for (uint16 y = _curFrame.strips[strip].rect.top; y < _curFrame.strips[strip].rect.bottom; y += 4) {
		iy[0] = _curFrame.strips[strip].rect.left + y * _curFrame.width;
		iy[1] = iy[0] + _curFrame.width;
		iy[2] = iy[1] + _curFrame.width;
		iy[3] = iy[2] + _curFrame.width;

		for (uint16 x = _curFrame.strips[strip].rect.left; x < _curFrame.strips[strip].rect.right; x += 4) {
			if ((chunkID & 0x01) && !(mask >>= 1)) {
				if ((stream->pos() - startPos + 4) > (int32)chunkSize)
					return;

				flag  = stream->readUint32BE();
				mask  = 0x80000000;
			}

			if (!(chunkID & 0x01) || (flag & mask)) {
				if (!(chunkID & 0x02) && !(mask >>= 1)) {
					if ((stream->pos() - startPos + 4) > (int32)chunkSize)
						return;

					flag  = stream->readUint32BE();
					mask  = 0x80000000;
				}

				if ((chunkID & 0x02) || (~flag & mask)) {
					if ((stream->pos() - startPos + 1) > (int32)chunkSize)
						return;

					// Get the codebook
					CinepakCodebook codebook = _curFrame.strips[strip].v1_codebook[stream->readByte()];

					PUT_PIXEL(iy[0] + 0, codebook.y[0], codebook.u, codebook.v);
					PUT_PIXEL(iy[0] + 1, codebook.y[0], codebook.u, codebook.v);
					PUT_PIXEL(iy[1] + 0, codebook.y[0], codebook.u, codebook.v);
					PUT_PIXEL(iy[1] + 1, codebook.y[0], codebook.u, codebook.v);

					PUT_PIXEL(iy[0] + 2, codebook.y[1], codebook.u, codebook.v);
					PUT_PIXEL(iy[0] + 3, codebook.y[1], codebook.u, codebook.v);
					PUT_PIXEL(iy[1] + 2, codebook.y[1], codebook.u, codebook.v);
					PUT_PIXEL(iy[1] + 3, codebook.y[1], codebook.u, codebook.v);

					PUT_PIXEL(iy[2] + 0, codebook.y[2], codebook.u, codebook.v);
					PUT_PIXEL(iy[2] + 1, codebook.y[2], codebook.u, codebook.v);
					PUT_PIXEL(iy[3] + 0, codebook.y[2], codebook.u, codebook.v);
					PUT_PIXEL(iy[3] + 1, codebook.y[2], codebook.u, codebook.v);

					PUT_PIXEL(iy[2] + 2, codebook.y[3], codebook.u, codebook.v);
					PUT_PIXEL(iy[2] + 3, codebook.y[3], codebook.u, codebook.v);
					PUT_PIXEL(iy[3] + 2, codebook.y[3], codebook.u, codebook.v);
					PUT_PIXEL(iy[3] + 3, codebook.y[3], codebook.u, codebook.v);
				} else if (flag & mask) {
					if ((stream->pos() - startPos + 4) > (int32)chunkSize)
						return;

					CinepakCodebook codebook = _curFrame.strips[strip].v4_codebook[stream->readByte()];
					PUT_PIXEL(iy[0] + 0, codebook.y[0], codebook.u, codebook.v);
					PUT_PIXEL(iy[0] + 1, codebook.y[1], codebook.u, codebook.v);
					PUT_PIXEL(iy[1] + 0, codebook.y[2], codebook.u, codebook.v);
					PUT_PIXEL(iy[1] + 1, codebook.y[3], codebook.u, codebook.v);

					codebook = _curFrame.strips[strip].v4_codebook[stream->readByte()];
					PUT_PIXEL(iy[0] + 2, codebook.y[0], codebook.u, codebook.v);
					PUT_PIXEL(iy[0] + 3, codebook.y[1], codebook.u, codebook.v);
					PUT_PIXEL(iy[1] + 2, codebook.y[2], codebook.u, codebook.v);
					PUT_PIXEL(iy[1] + 3, codebook.y[3], codebook.u, codebook.v);

					codebook = _curFrame.strips[strip].v4_codebook[stream->readByte()];
					PUT_PIXEL(iy[2] + 0, codebook.y[0], codebook.u, codebook.v);
					PUT_PIXEL(iy[2] + 1, codebook.y[1], codebook.u, codebook.v);
					PUT_PIXEL(iy[3] + 0, codebook.y[2], codebook.u, codebook.v);
					PUT_PIXEL(iy[3] + 1, codebook.y[3], codebook.u, codebook.v);

					codebook = _curFrame.strips[strip].v4_codebook[stream->readByte()];
					PUT_PIXEL(iy[2] + 2, codebook.y[0], codebook.u, codebook.v);
					PUT_PIXEL(iy[2] + 3, codebook.y[1], codebook.u, codebook.v);
					PUT_PIXEL(iy[3] + 2, codebook.y[2], codebook.u, codebook.v);
					PUT_PIXEL(iy[3] + 3, codebook.y[3], codebook.u, codebook.v);
				}
			}

			for (byte i = 0; i < 4; i++)
				iy[i] += 4;
		}
	}
}

} // End of namespace Video
