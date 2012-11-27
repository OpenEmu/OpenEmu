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

// Based off ffmpeg's msrledec.c

#include "video/codecs/msrle.h"
#include "common/stream.h"
#include "common/textconsole.h"

namespace Video {

MSRLEDecoder::MSRLEDecoder(uint16 width, uint16 height, byte bitsPerPixel) {
	_surface = new Graphics::Surface();
	_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	_bitsPerPixel = bitsPerPixel;
}

MSRLEDecoder::~MSRLEDecoder() {
	_surface->free();
	delete _surface;
}

const Graphics::Surface *MSRLEDecoder::decodeImage(Common::SeekableReadStream *stream) {
	if (_bitsPerPixel == 8) {
		decode8(stream);
	} else
		error("Unhandled %d bit Microsoft RLE encoding", _bitsPerPixel);

	return _surface;
}

void MSRLEDecoder::decode8(Common::SeekableReadStream *stream) {

	int x = 0;
	int y = _surface->h - 1;

	byte *data = (byte *) _surface->pixels;
	uint16 width  = _surface->w;
	uint16 height = _surface->h;

	byte *output     = data + ((height - 1) * width);
	byte *output_end = data + ((height)     * width);

	while (!stream->eos()) {
		byte count = stream->readByte();
		byte value = stream->readByte();

		if (count == 0) {
			if (value == 0) {
				// End of line

				x = 0;
				y--;
				output = data + (y * width);

				if (y < 0) {
					warning("MS RLE Codec: Next line is beyond picture bounds");
					return;
				}

			} else if (value == 1) {
				// End of image

				return;
			} else if (value == 2) {
				// Skip

				count = stream->readByte();
				value = stream->readByte();

				y -= value;
				x += count;

				if (y < 0) {
					warning("MS RLE Codec: Skip beyond picture bounds");
					return;
				}

				output = data + ((y * width) + x);

			} else {
				// Copy data

				if (output + value > output_end) {
					stream->skip(value);
					continue;
				}

				for (int i = 0; i < value; i++)
					*output++ = stream->readByte();

				if (value & 1)
					stream->skip(1);

				x += value;
			}

		} else {
			// Run data

			if (output + count > output_end)
				continue;

			for (int i = 0; i < count; i++, x++)
				*output++ = value;
		}

	}

	warning("MS RLE Codec: No end-of-picture code");
}

} // End of namespace Video
