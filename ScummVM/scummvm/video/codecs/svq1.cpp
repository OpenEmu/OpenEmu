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

// Sorenson Video 1 Codec
// Based off FFmpeg's SVQ1 decoder (written by Arpi and Nick Kurshev)

#include "video/codecs/svq1.h"
#include "video/codecs/svq1_cb.h"
#include "video/codecs/svq1_vlc.h"

#include "common/stream.h"
#include "common/bitstream.h"
#include "common/rect.h"
#include "common/system.h"
#include "common/debug.h"
#include "common/textconsole.h"
#include "common/huffman.h"

#include "graphics/yuv_to_rgb.h"

namespace Video {

#define SVQ1_BLOCK_SKIP     0
#define SVQ1_BLOCK_INTER    1
#define SVQ1_BLOCK_INTER_4V 2
#define SVQ1_BLOCK_INTRA    3

SVQ1Decoder::SVQ1Decoder(uint16 width, uint16 height) {
	debug(1, "SVQ1Decoder::SVQ1Decoder(width:%d, height:%d)", width, height);
	_width = width;
	_height = height;
	_frameWidth = _frameHeight = 0;
	_surface = 0;

	_last[0] = 0;
	_last[1] = 0;
	_last[2] = 0;

	// Setup Variable Length Code Tables
	_blockType = new Common::Huffman(0, 4, s_svq1BlockTypeCodes, s_svq1BlockTypeLengths);

	for (int i = 0; i < 6; i++) {
		_intraMultistage[i] = new Common::Huffman(0, 8, s_svq1IntraMultistageCodes[i], s_svq1IntraMultistageLengths[i]);
		_interMultistage[i] = new Common::Huffman(0, 8, s_svq1InterMultistageCodes[i], s_svq1InterMultistageLengths[i]);
	}

	_intraMean = new Common::Huffman(0, 256, s_svq1IntraMeanCodes, s_svq1IntraMeanLengths);
	_interMean = new Common::Huffman(0, 512, s_svq1InterMeanCodes, s_svq1InterMeanLengths);
	_motionComponent = new Common::Huffman(0, 33, s_svq1MotionComponentCodes, s_svq1MotionComponentLengths);
}

SVQ1Decoder::~SVQ1Decoder() {
	if (_surface) {
		_surface->free();
		delete _surface;
	}

	delete[] _last[0];
	delete[] _last[1];
	delete[] _last[2];

	delete _blockType;
	delete _intraMean;
	delete _interMean;
	delete _motionComponent;

	for (int i = 0; i < 6; i++) {
		delete _intraMultistage[i];
		delete _interMultistage[i];
	}
}

#define ALIGN(x, a) (((x)+(a)-1)&~((a)-1))

const Graphics::Surface *SVQ1Decoder::decodeImage(Common::SeekableReadStream *stream) {
	debug(1, "SVQ1Decoder::decodeImage()");

	Common::BitStream32BEMSB frameData(*stream);

	uint32 frameCode = frameData.getBits(22);
	debug(1, " frameCode: %d", frameCode);

	if ((frameCode & ~0x70) || !(frameCode & 0x60)) { // Invalid
		warning("Invalid Image at frameCode");
		return _surface;
	}

	byte temporalReference = frameData.getBits(8);
	debug(1, " temporalReference: %d", temporalReference);
	static const char *const types[4] = { "I (Key)", "P (Delta from Previous)", "B (Delta from Next)", "Invalid" };
	byte frameType = frameData.getBits(2);
	debug(1, " frameType: %d = %s Frame", frameType, types[frameType]);

	if (frameType == 0) { // I Frame
		// TODO: Validate checksum if present
		if (frameCode == 0x50 || frameCode == 0x60) {
			uint32 checksum = frameData.getBits(16);
			debug(1, " checksum:0x%02x", checksum);
			// We're currently just ignoring the checksum
		}

		if ((frameCode ^ 0x10) >= 0x50) {
			// Skip embedded string
			byte stringLen = frameData.getBits(8);
			for (uint16 i = 0; i < stringLen-1; i++)
				frameData.skip(8);
		}

		frameData.skip(5); // Unknown

		static const struct { uint w, h; } standardFrameSizes[7] = {
			{ 160, 120 }, // 0
			{ 128,  96 }, // 1
			{ 176, 144 }, // 2
			{ 352, 288 }, // 3
			{ 704, 576 }, // 4
			{ 240, 180 }, // 5
			{ 320, 240 }  // 6
		};

		byte frameSizeCode = frameData.getBits(3);
		debug(1, " frameSizeCode: %d", frameSizeCode);

		if (frameSizeCode == 7) {
			_frameWidth = frameData.getBits(12);
			_frameHeight = frameData.getBits(12);
		} else {
			_frameWidth = standardFrameSizes[frameSizeCode].w;
			_frameHeight = standardFrameSizes[frameSizeCode].h;
		}

		debug(1, " frameWidth: %d", _frameWidth);
		debug(1, " frameHeight: %d", _frameHeight);
	} else if (frameType == 2) { // B Frame
		warning("B Frames not supported by SVQ1 decoder (yet)");
		return _surface;
	} else if (frameType == 3) { // Invalid
		warning("Invalid Frame Type");
		return _surface;
	}

	bool checksumPresent = frameData.getBit() != 0;
	debug(1, " checksumPresent: %d", checksumPresent);
	if (checksumPresent) {
		bool usePacketChecksum = frameData.getBit() != 0;
		debug(1, " usePacketChecksum: %d", usePacketChecksum);
		bool componentChecksumsAfterImageData = frameData.getBit() != 0;
		debug(1, " componentChecksumsAfterImageData: %d", componentChecksumsAfterImageData);
		byte unk4 = frameData.getBits(2);
		debug(1, " unk4: %d", unk4);
		if (unk4 != 0)
			warning("Invalid Frame Header in SVQ1 Frame Decode");
	}

	// Some more unknown data
	bool unk5 = frameData.getBit() != 0;
	if (unk5) {
		frameData.skip(8);

		while (frameData.getBit() != 0)
			frameData.skip(8);
	}

	uint yWidth = ALIGN(_frameWidth, 16);
	uint yHeight = ALIGN(_frameHeight, 16);
	uint uvWidth = ALIGN(yWidth / 4, 16);
	uint uvHeight = ALIGN(yHeight / 4, 16);
	uint uvPitch = uvWidth + 4; // we need at least one extra column and pitch must be divisible by 4

	byte *current[3];

	// Decode Y, U and V component planes
	for (int i = 0; i < 3; i++) {
		uint width, height, pitch;
		if (i == 0) {
			width = yWidth;
			height = yHeight;
			pitch = width;
			current[i] = new byte[width * height];
		} else {
			width = uvWidth;
			height = uvHeight;
			pitch = uvPitch;

			// Add an extra row here. See below for more information.
			current[i] = new byte[pitch * (height + 1)];
		}

		if (frameType == 0) { // I Frame
			// Keyframe (I)
			byte *currentP = current[i];
			for (uint16 y = 0; y < height; y += 16) {
				for (uint16 x = 0; x < width; x += 16) {
					if (!svq1DecodeBlockIntra(&frameData, &currentP[x], pitch)) {
						warning("svq1DecodeBlockIntra decode failure");
						return _surface;
					}
				}
				currentP += 16 * pitch;
			}
		} else {
			// Delta frame (P or B)

			// Prediction Motion Vector
			Common::Point *pmv = new Common::Point[(width / 8) + 3];

			byte *previous = 0;
			if (frameType == 2) { // B Frame
				error("SVQ1 Video: B Frames not supported");
				//previous = _next[i];
			} else {
				previous = _last[i];
			}

			byte *currentP = current[i];
			for (uint16 y = 0; y < height; y += 16) {
				for (uint16 x = 0; x < width; x += 16) {
					if (!svq1DecodeDeltaBlock(&frameData, &currentP[x], previous, pitch, pmv, x, y)) {
						warning("svq1DecodeDeltaBlock decode failure");
						return _surface;
					}
				}

				pmv[0].x = pmv[0].y = 0;

				currentP += 16 * pitch;
			}

			delete[] pmv;
		}
	}

	// Now we'll create the surface
	if (!_surface) {
		_surface = new Graphics::Surface();
		_surface->create(yWidth, yHeight, g_system->getScreenFormat());
		_surface->w = _width;
		_surface->h = _height;
	}

	// We need to massage the chrominance data a bit to be able to be used by the converter
	// Since the thing peeks at values one column and one row beyond the data, we need to fill it in

	// First, fill in the column-after-last with the last column's value
	for (uint i = 0; i < uvHeight; i++) {
		current[1][i * uvPitch + uvWidth] = current[1][i * uvPitch + uvWidth - 1];
		current[2][i * uvPitch + uvWidth] = current[2][i * uvPitch + uvWidth - 1];
	}

	// Then, copy the last row to the one after the last row
	memcpy(current[1] + uvHeight * uvPitch, current[1] + (uvHeight - 1) * uvPitch, uvWidth + 1);
	memcpy(current[2] + uvHeight * uvPitch, current[2] + (uvHeight - 1) * uvPitch, uvWidth + 1);

	// Finally, actually do the conversion ;)
	YUVToRGBMan.convert410(_surface, Graphics::YUVToRGBManager::kScaleFull, current[0], current[1], current[2], yWidth, yHeight, yWidth, uvPitch);

	// Store the current surfaces for later and free the old ones
	for (int i = 0; i < 3; i++) {
		delete[] _last[i];
		_last[i] = current[i];
	}

	return _surface;
}

bool SVQ1Decoder::svq1DecodeBlockIntra(Common::BitStream *s, byte *pixels, int pitch) {
	// initialize list for breadth first processing of vectors
	byte *list[63];
	list[0] = pixels;

	// recursively process vector
	for (int i = 0, m = 1, n = 1, level = 5; i < n; i++) {
		for (; level > 0; i++) {
			// process next depth
			if (i == m) {
				m = n;
				if (--level == 0)
					break;
			}

			// divide block if next bit set
			if (s->getBit() == 0)
				break;

			// add child nodes
			list[n++] = list[i];
			list[n++] = list[i] + (((level & 1) ? pitch : 1) << ((level / 2) + 1));
		}

		// destination address and vector size
		uint32 *dst = (uint32 *)list[i];
		uint width = 1 << ((level + 4) / 2);
		uint height = 1 << ((level + 3) / 2);

		// get number of stages (-1 skips vector, 0 for mean only)
		int stages = _intraMultistage[level]->getSymbol(*s) - 1;

		if (stages == -1) {
			for (uint y = 0; y < height; y++)
				memset(&dst[y * (pitch / 4)], 0, width);

			continue; // skip vector
		}

		if (stages > 0 && level >= 4) {
			warning("Error (svq1_decode_block_intra): invalid vector: stages = %d, level = %d", stages, level);
			return false; // error - invalid vector
		}

		int mean = _intraMean->getSymbol(*s);

		if (stages == 0) {
			for (uint y = 0; y < height; y++)
				memset(&dst[y * (pitch / 4)], mean, width);
		} else {
			const uint32 *codebook = (const uint32 *)s_svq1IntraCodebooks[level];
			uint32 bitCache = s->getBits(stages * 4);

			// calculate codebook entries for this vector
			int entries[6];
			for (int j = 0; j < stages; j++)
				entries[j] = (((bitCache >> ((stages - j - 1) * 4)) & 0xF) + j * 16) << (level + 1);

			mean -= stages * 128;
			uint32 n4 = ((mean + (mean >> 31)) << 16) | (mean & 0xFFFF);

			for (uint y = 0; y < height; y++) {
				for (uint x = 0; x < (width / 4); x++, codebook++) {
					uint32 n1 = n4;
					uint32 n2 = n4;
					uint32 n3;

					// add codebook entries to vector
					for (int j = 0; j < stages; j++) {
						n3 = READ_UINT32(&codebook[entries[j]]) ^ 0x80808080;
						n1 += (n3 & 0xFF00FF00) >> 8;
						n2 += n3 & 0x00FF00FF;
					}

					// clip to [0..255]
					if (n1 & 0xFF00FF00) {
						n3 = (((n1 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
						n1 += 0x7F007F00;
						n1 |= (((~n1 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
						n1 &= n3 & 0x00FF00FF;
					}

					if (n2 & 0xFF00FF00) {
						n3 = (((n2 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
						n2 += 0x7F007F00;
						n2 |= (((~n2 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
						n2 &= n3 & 0x00FF00FF;
					}

					// store result
					dst[x] = (n1 << 8) | n2;
				}

				dst += pitch / 4;
			}
		}
	}

	return true;
}

bool SVQ1Decoder::svq1DecodeBlockNonIntra(Common::BitStream *s, byte *pixels, int pitch) {
	// initialize list for breadth first processing of vectors
	byte *list[63];
	list[0] = pixels;

	// recursively process vector
	for (int i = 0, m = 1, n = 1, level = 5; i < n; i++) {
		for (; level > 0; i++) {
			// process next depth
			if (i == m) {
				m = n;
				if (--level == 0)
					break;
			}

			// divide block if next bit set
			if (s->getBit() == 0)
				break;

			// add child nodes
			list[n++] = list[i];
			list[n++] = list[i] + (((level & 1) ? pitch : 1) << ((level / 2) + 1));
		}

		// destination address and vector size
		uint32 *dst = (uint32 *)list[i];
		int width = 1 << ((level + 4) / 2);
		int height = 1 << ((level + 3) /  2);

		// get number of stages (-1 skips vector, 0 for mean only)
		int stages = _interMultistage[level]->getSymbol(*s) - 1;

		if (stages == -1)
			continue; // skip vector

		if (stages > 0 && level >= 4) {
			warning("Error (svq1_decode_block_non_intra): invalid vector: stages = %d, level = %d", stages, level);
			return false; // error - invalid vector
		}

		int mean = _interMean->getSymbol(*s) - 256;
		const uint32 *codebook = (const uint32 *)s_svq1InterCodebooks[level];
		uint32 bitCache = s->getBits(stages * 4);

		// calculate codebook entries for this vector
		int entries[6];
		for (int j = 0; j < stages; j++)
			entries[j] = (((bitCache >> ((stages - j - 1) * 4)) & 0xF) + j * 16) << (level + 1);

		mean -= stages * 128;
		uint32 n4 = ((mean + (mean >> 31)) << 16) | (mean & 0xFFFF);

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < (width / 4); x++, codebook++) {
				uint32 n3 = dst[x];

				// add mean value to vector
				uint32 n1 = ((n3 & 0xFF00FF00) >> 8) + n4;
				uint32 n2 = (n3 & 0x00FF00FF) + n4;

				// add codebook entries to vector
				for (int j = 0; j < stages; j++) {
					n3 = READ_UINT32(&codebook[entries[j]]) ^ 0x80808080;
					n1 += (n3 & 0xFF00FF00) >> 8;
					n2 += n3 & 0x00FF00FF;
				}

				// clip to [0..255]
				if (n1 & 0xFF00FF00) {
					n3 = ((( n1 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
					n1 += 0x7F007F00;
					n1 |= (((~n1 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
					n1 &= n3 & 0x00FF00FF;
				}

				if (n2 & 0xFF00FF00) {
					n3  = ((( n2 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
					n2 += 0x7F007F00;
					n2 |= (((~n2 >> 15) & 0x00010001) | 0x01000100) - 0x00010001;
					n2 &= n3 & 0x00FF00FF;
				}

				// store result
				dst[x] = (n1 << 8) | n2;
			}

			dst += pitch / 4;
		}
	}

	return true;
}

// median of 3
static inline int midPred(int a, int b, int c) {
	if (a > b) {
		if (c > b) {
			if (c > a)
				b = a;
			else
				b = c;
		}
	} else {
		if (b > c) {
			if (c > a)
				b = c;
			else
				b = a;
		}
	}

	return b;
}

bool SVQ1Decoder::svq1DecodeMotionVector(Common::BitStream *s, Common::Point *mv, Common::Point **pmv) {
	for (int i = 0; i < 2; i++) {
		// get motion code
		int diff = _motionComponent->getSymbol(*s);
		if (diff < 0)
			return false; // error - invalid motion code
		else if (diff && s->getBit() != 0)
			diff = -diff;

		// add median of motion vector predictors and clip result
		if (i == 1)
			mv->y = ((diff + midPred(pmv[0]->y, pmv[1]->y, pmv[2]->y)) << 26) >> 26;
		else
			mv->x = ((diff + midPred(pmv[0]->x, pmv[1]->x, pmv[2]->x)) << 26) >> 26;
	}

	return true;
}

void SVQ1Decoder::svq1SkipBlock(byte *current, byte *previous, int pitch, int x, int y) {
	const byte *src = &previous[x + y * pitch];
	byte *dst = current;

	for (int i = 0; i < 16; i++) {
		memcpy(dst, src, 16);
		src += pitch;
		dst += pitch;
	}
}

void SVQ1Decoder::putPixels8C(byte *block, const byte *pixels, int lineSize, int h) {
	for (int i = 0; i < h; i++) {
		*((uint32 *)block) = READ_UINT32(pixels);
		*((uint32 *)(block + 4)) = READ_UINT32(pixels + 4);
		pixels += lineSize;
		block += lineSize;
	}
}

static inline uint32 rndAvg32(uint32 a, uint32 b) {
	return (a | b) - (((a ^ b) & ~0x01010101) >> 1);
}

void SVQ1Decoder::putPixels8L2(byte *dst, const byte *src1, const byte *src2,
		int dstStride, int srcStride1, int srcStride2, int h) {
	for (int i = 0; i < h; i++) {
		uint32 a = READ_UINT32(&src1[srcStride1 * i]);
		uint32 b = READ_UINT32(&src2[srcStride2 * i]);
		*((uint32 *)&dst[dstStride * i]) = rndAvg32(a, b);
		a = READ_UINT32(&src1[srcStride1 * i + 4]);
		b = READ_UINT32(&src2[srcStride2 * i + 4]);
		*((uint32 *)&dst[dstStride * i + 4]) = rndAvg32(a, b);
	}
}

void SVQ1Decoder::putPixels8X2C(byte *block, const byte *pixels, int lineSize, int h) {
	putPixels8L2(block, pixels, pixels + 1, lineSize, lineSize, lineSize, h);
}

void SVQ1Decoder::putPixels8Y2C(byte *block, const byte *pixels, int lineSize, int h) {
	putPixels8L2(block, pixels, pixels + lineSize, lineSize, lineSize, lineSize, h);
}

void SVQ1Decoder::putPixels8XY2C(byte *block, const byte *pixels, int lineSize, int h) {
	for (int j = 0; j < 2; j++) {
		uint32 a = READ_UINT32(pixels);
		uint32 b = READ_UINT32(pixels + 1);
		uint32 l0 = (a & 0x03030303UL) + (b & 0x03030303UL) + 0x02020202UL;
		uint32 h0 = ((a & 0xFCFCFCFCUL) >> 2) + ((b & 0xFCFCFCFCUL) >> 2);

		pixels += lineSize;

		for (int i = 0; i < h; i += 2) {
			a = READ_UINT32(pixels);
			b = READ_UINT32(pixels + 1);
			uint32 l1 = (a & 0x03030303UL) + (b & 0x03030303UL);
			uint32 h1 = ((a & 0xFCFCFCFCUL) >> 2) + ((b & 0xFCFCFCFCUL) >> 2);
			*((uint32 *)block) = h0 + h1 + (((l0 + l1) >> 2) & 0x0F0F0F0FUL);
			pixels += lineSize;
			block += lineSize;
			a = READ_UINT32(pixels);
			b = READ_UINT32(pixels + 1);
			l0 = (a & 0x03030303UL) + (b & 0x03030303UL) + 0x02020202UL;
			h0 = ((a & 0xFCFCFCFCUL) >> 2) + ((b & 0xFCFCFCFCUL) >> 2);
			*((uint32 *)block) = h0 + h1 + (((l0 + l1) >> 2) & 0x0F0F0F0FUL);
			pixels += lineSize;
			block += lineSize;
		}

		pixels += 4 - lineSize * (h + 1);
		block += 4 - lineSize * h;
	}
}

void SVQ1Decoder::putPixels16C(byte *block, const byte *pixels, int lineSize, int h) {
	putPixels8C(block, pixels, lineSize, h);
	putPixels8C(block + 8, pixels + 8, lineSize, h);
}

void SVQ1Decoder::putPixels16X2C(byte *block, const byte *pixels, int lineSize, int h) {
	putPixels8X2C(block, pixels, lineSize, h);
	putPixels8X2C(block + 8, pixels + 8, lineSize, h);
}

void SVQ1Decoder::putPixels16Y2C(byte *block, const byte *pixels, int lineSize, int h) {
	putPixels8Y2C(block, pixels, lineSize, h);
	putPixels8Y2C(block + 8, pixels + 8, lineSize, h);
}

void SVQ1Decoder::putPixels16XY2C(byte *block, const byte *pixels, int lineSize, int h) {
	putPixels8XY2C(block, pixels, lineSize, h);
	putPixels8XY2C(block + 8, pixels + 8, lineSize, h);
}

bool SVQ1Decoder::svq1MotionInterBlock(Common::BitStream *ss, byte *current, byte *previous, int pitch,
		Common::Point *motion, int x, int y) {

	// predict and decode motion vector
	Common::Point *pmv[3];
	pmv[0] = &motion[0];
	if (y == 0) {
		pmv[1] = pmv[2] = pmv[0];
	} else {
		pmv[1] = &motion[(x / 8) + 2];
		pmv[2] = &motion[(x / 8) + 4];
	}

	Common::Point mv;
	bool resultValid = svq1DecodeMotionVector(ss, &mv, pmv);
	if (!resultValid)
		return false;

	motion[0].x = motion[(x / 8) + 2].x = motion[(x / 8) + 3].x = mv.x;
	motion[0].y = motion[(x / 8) + 2].y = motion[(x / 8) + 3].y = mv.y;

	if (y + (mv.y >> 1) < 0)
		mv.y = 0;

	if (x + (mv.x >> 1) < 0)
		mv.x = 0;

	const byte *src = &previous[(x + (mv.x >> 1)) + (y + (mv.y >> 1)) * pitch];
	byte *dst = current;

	// Halfpel motion compensation with rounding (a + b + 1) >> 1.
	// 4 motion compensation functions for the 4 halfpel positions
	// for 16x16 blocks
	switch(((mv.y & 1) << 1) + (mv.x & 1)) {
	case 0:
		putPixels16C(dst, src, pitch, 16);
		break;
	case 1:
		putPixels16X2C(dst, src, pitch, 16);
		break;
	case 2:
		putPixels16Y2C(dst, src, pitch, 16);
		break;
	case 3:
		putPixels16XY2C(dst, src, pitch, 16);
		break;
	}

	return true;
}

bool SVQ1Decoder::svq1MotionInter4vBlock(Common::BitStream *ss, byte *current, byte *previous, int pitch,
		Common::Point *motion, int x, int y) {
	// predict and decode motion vector (0)
	Common::Point *pmv[4];
	pmv[0] = &motion[0];
	if (y == 0) {
		pmv[1] = pmv[2] = pmv[0];
	} else {
		pmv[1] = &motion[(x / 8) + 2];
		pmv[2] = &motion[(x / 8) + 4];
	}

	Common::Point mv;
	bool resultValid = svq1DecodeMotionVector(ss, &mv, pmv);
	if (!resultValid)
		return false;

	// predict and decode motion vector (1)
	pmv[0] = &mv;
	if (y == 0)
		pmv[1] = pmv[2] = pmv[0];
	else
		pmv[1] = &motion[(x / 8) + 3];

	resultValid = svq1DecodeMotionVector(ss, &motion[0], pmv);
	if (!resultValid)
		return false;

	// predict and decode motion vector (2)
	pmv[1] = &motion[0];
	pmv[2] = &motion[(x / 8) + 1];

	resultValid = svq1DecodeMotionVector(ss, &motion[(x / 8) + 2], pmv);
	if (!resultValid)
		return false;

	// predict and decode motion vector (3)
	pmv[2] = &motion[(x / 8) + 2];
	pmv[3] = &motion[(x / 8) + 3];

	resultValid = svq1DecodeMotionVector(ss, pmv[3], pmv);
	if (!resultValid)
		return false;

	// form predictions
	for (int i = 0; i < 4; i++) {
		int mvx = pmv[i]->x + (i & 1) * 16;
		int mvy = pmv[i]->y + (i >> 1) * 16;

		// FIXME: clipping or padding?
		if (y + (mvy >> 1) < 0)
			mvy = 0;

		if (x + (mvx >> 1) < 0)
			mvx = 0;

		const byte *src = &previous[(x + (mvx >> 1)) + (y + (mvy >> 1)) * pitch];
		byte *dst = current;

		// Halfpel motion compensation with rounding (a + b + 1) >> 1.
		// 4 motion compensation functions for the 4 halfpel positions
		// for 8x8 blocks
		switch(((mvy & 1) << 1) + (mvx & 1)) {
		case 0:
			putPixels8C(dst, src, pitch, 8);
			break;
		case 1:
			putPixels8X2C(dst, src, pitch, 8);
			break;
		case 2:
			putPixels8Y2C(dst, src, pitch, 8);
			break;
		case 3:
			putPixels8XY2C(dst, src, pitch, 8);
			break;
		}

		// select next block
		if (i & 1)
			current += (pitch - 1) * 8;
		else
			current += 8;
	}

	return true;
}

bool SVQ1Decoder::svq1DecodeDeltaBlock(Common::BitStream *ss, byte *current, byte *previous, int pitch,
		Common::Point *motion, int x, int y) {
	// get block type
	uint32 blockType = _blockType->getSymbol(*ss);

	// reset motion vectors
	if (blockType == SVQ1_BLOCK_SKIP || blockType == SVQ1_BLOCK_INTRA) {
		motion[0].x =
		motion[0].y =
		motion[(x / 8) + 2].x =
		motion[(x / 8) + 2].y =
		motion[(x / 8) + 3].x =
		motion[(x / 8) + 3].y = 0;
	}

	bool resultValid = true;

	switch (blockType) {
	case SVQ1_BLOCK_SKIP:
		svq1SkipBlock(current, previous, pitch, x, y);
		break;
	case SVQ1_BLOCK_INTER:
		resultValid = svq1MotionInterBlock(ss, current, previous, pitch, motion, x, y);
		if (!resultValid) {
			warning("svq1MotionInterBlock decode failure");
			break;
		}
		resultValid = svq1DecodeBlockNonIntra(ss, current, pitch);
		break;
	case SVQ1_BLOCK_INTER_4V:
		resultValid = svq1MotionInter4vBlock(ss, current, previous, pitch, motion, x, y);
		if (!resultValid) {
			warning("svq1MotionInter4vBlock decode failure");
			break;
		}
		resultValid = svq1DecodeBlockNonIntra(ss, current, pitch);
		break;
	case SVQ1_BLOCK_INTRA:
		resultValid = svq1DecodeBlockIntra(ss, current, pitch);
		break;
	}

	return resultValid;
}

} // End of namespace Video
