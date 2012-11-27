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

// Based off ffmpeg's SMC decoder

#include "video/codecs/smc.h"
#include "common/stream.h"
#include "common/textconsole.h"

namespace Video {

#define GET_BLOCK_COUNT() \
  (opcode & 0x10) ? (1 + stream->readByte()) : 1 + (opcode & 0x0F);

#define ADVANCE_BLOCK() \
{ \
	pixelPtr += 4; \
	if (pixelPtr >= _surface->w) { \
		pixelPtr = 0; \
		rowPtr += _surface->w * 4; \
	} \
	totalBlocks--; \
	if (totalBlocks < 0) { \
		warning("block counter just went negative (this should not happen)"); \
		return _surface; \
	} \
}

SMCDecoder::SMCDecoder(uint16 width, uint16 height) {
	_surface = new Graphics::Surface();
	_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
}

SMCDecoder::~SMCDecoder() {
	_surface->free();
	delete _surface;
}

const Graphics::Surface *SMCDecoder::decodeImage(Common::SeekableReadStream *stream) {
	byte *pixels = (byte *)_surface->pixels;

	uint32 numBlocks = 0;
	uint32 colorFlags = 0;
	uint32 colorFlagsA = 0;
	uint32 colorFlagsB = 0;

	const uint16 rowInc = _surface->w - 4;
	int32 rowPtr = 0;
	int32 pixelPtr = 0;
	uint32 blockPtr = 0;
	uint32 prevBlockPtr = 0;
	uint32 prevBlockPtr1 = 0, prevBlockPtr2 = 0;
	byte prevBlockFlag = false;
	uint32 pixel = 0;

	uint32 colorPairIndex = 0;
	uint32 colorQuadIndex = 0;
	uint32 colorOctetIndex = 0;
	uint32 colorTableIndex = 0;  // indices to color pair, quad, or octet tables

	int32 chunkSize = stream->readUint32BE() & 0x00FFFFFF;
	if (chunkSize != stream->size())
		warning("MOV chunk size != SMC chunk size (%d != %d); ignoring SMC chunk size", chunkSize, stream->size());

	int32 totalBlocks = ((_surface->w + 3) / 4) * ((_surface->h + 3) / 4);

	// traverse through the blocks
	while (totalBlocks != 0) {
		// sanity checks

		// make sure stream ptr hasn't gone out of bounds
		if (stream->pos() > stream->size()) {
			warning("SMC decoder just went out of bounds (stream ptr = %d, chunk size = %d)", stream->pos(), stream->size());
			return _surface;
		}

		// make sure the row pointer hasn't gone wild
		if (rowPtr >= _surface->w * _surface->h) {
			warning("SMC decoder just went out of bounds (row ptr = %d, size = %d)", rowPtr, _surface->w * _surface->h);
			return _surface;
		}

		byte opcode = stream->readByte();

		switch (opcode & 0xF0) {
		// skip n blocks
		case 0x00:
		case 0x10:
			numBlocks = GET_BLOCK_COUNT();
			while (numBlocks--) {
				ADVANCE_BLOCK();
			}
			break;

		// repeat last block n times
		case 0x20:
		case 0x30:
			numBlocks = GET_BLOCK_COUNT();

			// sanity check
			if (rowPtr == 0 && pixelPtr == 0) {
				warning("encountered repeat block opcode (%02X) but no blocks rendered yet", opcode & 0xF0);
				break;
			}

			// figure out where the previous block started
			if (pixelPtr == 0)
				prevBlockPtr1 = (rowPtr - _surface->w * 4) + _surface->w - 4;
			else
				prevBlockPtr1 = rowPtr + pixelPtr - 4;

			while (numBlocks--) {
				blockPtr = rowPtr + pixelPtr;
				prevBlockPtr = prevBlockPtr1;
				for (byte y = 0; y < 4; y++) {
					for (byte x = 0; x < 4; x++)
						pixels[blockPtr++] = pixels[prevBlockPtr++];
					blockPtr += rowInc;
					prevBlockPtr += rowInc;
				}
				ADVANCE_BLOCK();
			}
			break;

		// repeat previous pair of blocks n times
		case 0x40:
		case 0x50:
			numBlocks = GET_BLOCK_COUNT();
			numBlocks *= 2;

			// sanity check
			if (rowPtr == 0 && pixelPtr < 2 * 4) {
				warning("encountered repeat block opcode (%02X) but not enough blocks rendered yet", opcode & 0xF0);
				break;
			}

			// figure out where the previous 2 blocks started
			if (pixelPtr == 0)
				prevBlockPtr1 = (rowPtr - _surface->w * 4) + _surface->w - 4 * 2;
			else if (pixelPtr == 4)
				prevBlockPtr1 = (rowPtr - _surface->w * 4) + rowInc;
			else
				prevBlockPtr1 = rowPtr + pixelPtr - 4 * 2;

			if (pixelPtr == 0)
				prevBlockPtr2 = (rowPtr - _surface->w * 4) + rowInc;
			else
				prevBlockPtr2 = rowPtr + pixelPtr - 4;

			prevBlockFlag = 0;
			while (numBlocks--) {
				blockPtr = rowPtr + pixelPtr;

				if (prevBlockFlag)
					prevBlockPtr = prevBlockPtr2;
				else
					prevBlockPtr = prevBlockPtr1;

				prevBlockFlag = !prevBlockFlag;

				for (byte y = 0; y < 4; y++) {
					for (byte x = 0; x < 4; x++)
						pixels[blockPtr++] = pixels[prevBlockPtr++];

					blockPtr += rowInc;
					prevBlockPtr += rowInc;
				}
				ADVANCE_BLOCK();
			}
			break;

		// 1-color block encoding
		case 0x60:
		case 0x70:
			numBlocks = GET_BLOCK_COUNT();
			pixel = stream->readByte();

			while (numBlocks--) {
				blockPtr = rowPtr + pixelPtr;
				for (byte y = 0; y < 4; y++) {
					for (byte x = 0; x < 4; x++)
						pixels[blockPtr++] = pixel;

					blockPtr += rowInc;
				}
				ADVANCE_BLOCK();
			}
			break;

		// 2-color block encoding
		case 0x80:
		case 0x90:
			numBlocks = (opcode & 0x0F) + 1;

			// figure out which color pair to use to paint the 2-color block
			if ((opcode & 0xF0) == 0x80) {
				// fetch the next 2 colors from bytestream and store in next
				// available entry in the color pair table
				for (byte i = 0; i < CPAIR; i++) {
					pixel = stream->readByte();
					colorTableIndex = CPAIR * colorPairIndex + i;
					_colorPairs[colorTableIndex] = pixel;
				}

				// this is the base index to use for this block
				colorTableIndex = CPAIR * colorPairIndex;
				colorPairIndex++;

				// wraparound
				if (colorPairIndex == COLORS_PER_TABLE)
					colorPairIndex = 0;
			} else
				colorTableIndex = CPAIR * stream->readByte();

			while (numBlocks--) {
				colorFlags = stream->readUint16BE();
				uint16 flagMask = 0x8000;
				blockPtr = rowPtr + pixelPtr;
				for (byte y = 0; y < 4; y++) {
					for (byte x = 0; x < 4; x++) {
						if (colorFlags & flagMask)
							pixel = colorTableIndex + 1;
						else
							pixel = colorTableIndex;

						flagMask >>= 1;
						pixels[blockPtr++] = _colorPairs[pixel];
					}

					blockPtr += rowInc;
				}
				ADVANCE_BLOCK();
			}
			break;

		// 4-color block encoding
		case 0xA0:
		case 0xB0:
			numBlocks = (opcode & 0x0F) + 1;

			// figure out which color quad to use to paint the 4-color block
			if ((opcode & 0xF0) == 0xA0) {
				// fetch the next 4 colors from bytestream and store in next
				// available entry in the color quad table
				for (byte i = 0; i < CQUAD; i++) {
					pixel = stream->readByte();
					colorTableIndex = CQUAD * colorQuadIndex + i;
					_colorQuads[colorTableIndex] = pixel;
				}

				// this is the base index to use for this block
				colorTableIndex = CQUAD * colorQuadIndex;
				colorQuadIndex++;

				// wraparound
				if (colorQuadIndex == COLORS_PER_TABLE)
					colorQuadIndex = 0;
			} else
				colorTableIndex = CQUAD * stream->readByte();

			while (numBlocks--) {
				colorFlags = stream->readUint32BE();

				// flag mask actually acts as a bit shift count here
				byte flagMask = 30;
				blockPtr = rowPtr + pixelPtr;

				for (byte y = 0; y < 4; y++) {
					for (byte x = 0; x < 4; x++) {
						pixel = colorTableIndex + ((colorFlags >> flagMask) & 0x03);
						flagMask -= 2;
						pixels[blockPtr++] = _colorQuads[pixel];
					}
					blockPtr += rowInc;
				}
				ADVANCE_BLOCK();
			}
			break;

		// 8-color block encoding
		case 0xC0:
		case 0xD0:
			numBlocks = (opcode & 0x0F) + 1;

			// figure out which color octet to use to paint the 8-color block
			if ((opcode & 0xF0) == 0xC0) {
				// fetch the next 8 colors from bytestream and store in next
				// available entry in the color octet table
				for (byte i = 0; i < COCTET; i++) {
					pixel = stream->readByte();
					colorTableIndex = COCTET * colorOctetIndex + i;
					_colorOctets[colorTableIndex] = pixel;
				}

				// this is the base index to use for this block
				colorTableIndex = COCTET * colorOctetIndex;
				colorOctetIndex++;

				// wraparound
				if (colorOctetIndex == COLORS_PER_TABLE)
					colorOctetIndex = 0;
			} else
				colorTableIndex = COCTET * stream->readByte();

			while (numBlocks--) {
				/*
				  For this input of 6 hex bytes:
				    01 23 45 67 89 AB
				  Mangle it to this output:
				    flags_a = xx012456, flags_b = xx89A37B
				*/

				// build the color flags
				byte flagData[6];
				stream->read(flagData, 6);

				colorFlagsA = ((READ_BE_UINT16(flagData) & 0xFFF0) << 8) | (READ_BE_UINT16(flagData + 2) >> 4);
				colorFlagsB = ((READ_BE_UINT16(flagData + 4) & 0xFFF0) << 8) | ((flagData[1] & 0xF) << 8) |
								((flagData[3] & 0xF) << 4) | (flagData[5] & 0xf);

				colorFlags = colorFlagsA;

				// flag mask actually acts as a bit shift count here
				byte flagMask = 21;
				blockPtr = rowPtr + pixelPtr;
				for (byte y = 0; y < 4; y++) {
					// reload flags at third row (iteration y == 2)
					if (y == 2) {
						colorFlags = colorFlagsB;
						flagMask = 21;
					}

					for (byte x = 0; x < 4; x++) {
						pixel = colorTableIndex + ((colorFlags >> flagMask) & 0x07);
						flagMask -= 3;
						pixels[blockPtr++] = _colorOctets[pixel];
					}

					blockPtr += rowInc;
				}
				ADVANCE_BLOCK();
			}
			break;

		// 16-color block encoding (every pixel is a different color)
		case 0xE0:
			numBlocks = (opcode & 0x0F) + 1;

			while (numBlocks--) {
				blockPtr = rowPtr + pixelPtr;
				for (byte y = 0; y < 4; y++) {
					for (byte x = 0; x < 4; x++)
						pixels[blockPtr++] = stream->readByte();

					blockPtr += rowInc;
				}
				ADVANCE_BLOCK();
			}
			break;

		case 0xF0:
			warning("0xF0 opcode seen in SMC chunk (contact the developers)");
			break;
		}
	}

	return _surface;
}

} // End of namespace Video
