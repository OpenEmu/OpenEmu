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

 // Based off ffmpeg's msvideo.cpp

#include "video/codecs/msvideo1.h"
#include "common/stream.h"
#include "common/textconsole.h"

namespace Video {

#define CHECK_STREAM_PTR(n) \
  if ((stream->pos() + n) > stream->size() ) { \
	warning ("MS Video-1: Stream out of bounds (%d >= %d)", stream->pos() + n, stream->size()); \
    return; \
  }

MSVideo1Decoder::MSVideo1Decoder(uint16 width, uint16 height, byte bitsPerPixel) : Codec() {
	_surface = new Graphics::Surface();
	// TODO: Specify the correct pixel format for 2Bpp mode.
	_surface->create(width, height, (bitsPerPixel == 8) ? Graphics::PixelFormat::createFormatCLUT8() : Graphics::PixelFormat(2, 0, 0, 0, 0, 0, 0, 0, 0));
	_bitsPerPixel = bitsPerPixel;
}

MSVideo1Decoder::~MSVideo1Decoder() {
	_surface->free();
	delete _surface;
}

void MSVideo1Decoder::decode8(Common::SeekableReadStream *stream) {
    byte colors[8];
    byte *pixels = (byte *)_surface->pixels;
    uint16 stride = _surface->w;

    int skipBlocks = 0;
    uint16 blocks_wide = _surface->w / 4;
    uint16 blocks_high = _surface->h / 4;
    uint32 totalBlocks = blocks_wide * blocks_high;
    uint32 blockInc = 4;
    uint16 rowDec = stride + 4;

    for (uint16 block_y = blocks_high; block_y > 0; block_y--) {
        uint32 blockPtr = (block_y * 4 - 1) * stride;
        for (uint16 block_x = blocks_wide; block_x > 0; block_x--) {
            // check if this block should be skipped
            if (skipBlocks > 0) {
                blockPtr += blockInc;
                skipBlocks--;
                totalBlocks--;
                continue;
            }

            uint32 pixelPtr = blockPtr;

            /* get the next two bytes in the encoded data stream */
            CHECK_STREAM_PTR(2);
            byte byte_a = stream->readByte();
            byte byte_b = stream->readByte();

            /* check if the decode is finished */
            if (byte_a == 0 && byte_b == 0 && totalBlocks == 0) {
                return;
            } else if ((byte_b & 0xFC) == 0x84) {
                // skip code, but don't count the current block
                skipBlocks = ((byte_b - 0x84) << 8) + byte_a - 1;
            } else if (byte_b < 0x80) {
                // 2-color encoding
                uint16 flags = (byte_b << 8) | byte_a;

                CHECK_STREAM_PTR(2);
                colors[0] = stream->readByte();
                colors[1] = stream->readByte();

                for (byte pixel_y = 0; pixel_y < 4; pixel_y++) {
                    for (byte pixel_x = 0; pixel_x < 4; pixel_x++, flags >>= 1)
                        pixels[pixelPtr++] = colors[(flags & 0x1) ^ 1];
                    pixelPtr -= rowDec;
                }
            } else if (byte_b >= 0x90) {
                // 8-color encoding
                uint16 flags = (byte_b << 8) | byte_a;

                CHECK_STREAM_PTR(8);
				for (byte i = 0; i < 8; i++)
					colors[i] = stream->readByte();

                for (byte pixel_y = 0; pixel_y < 4; pixel_y++) {
                    for (byte pixel_x = 0; pixel_x < 4; pixel_x++, flags >>= 1)
                        pixels[pixelPtr++] = colors[((pixel_y & 0x2) << 1) + (pixel_x & 0x2) + ((flags & 0x1) ^ 1)];
                    pixelPtr -= rowDec;
                }
            } else {
                // 1-color encoding
                colors[0] = byte_a;

                for (byte pixel_y = 0; pixel_y < 4; pixel_y++) {
                    for (byte pixel_x = 0; pixel_x < 4; pixel_x++)
                        pixels[pixelPtr++] = colors[0];
                    pixelPtr -= rowDec;
                }
            }

            blockPtr += blockInc;
            totalBlocks--;
        }
    }
}

const Graphics::Surface *MSVideo1Decoder::decodeImage(Common::SeekableReadStream *stream) {
	if (_bitsPerPixel == 8)
		decode8(stream);
	else {
    //	decode16(stream);
		error ("Unhandled MS Video-1 16bpp encoding");
	}

    return _surface;
}

} // End of namespace Video
