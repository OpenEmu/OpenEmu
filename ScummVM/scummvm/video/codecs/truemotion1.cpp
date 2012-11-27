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

// Based on the TrueMotion 1 decoder by Alex Beregszaszi & Mike Melanson in FFmpeg

#include "common/scummsys.h"
#include "video/codecs/truemotion1.h"

#ifdef VIDEO_CODECS_TRUEMOTION1_H

#include "video/codecs/truemotion1data.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Video {

enum {
	FLAG_SPRITE = (1 << 5),
	FLAG_KEYFRAME = (1 << 4),
	FLAG_INTERFRAME = (1 << 3),
	FLAG_INTERPOLATED = (1 << 2)
};

enum {
	ALGO_NOP = 0,
	ALGO_RGB16V = 1,
	ALGO_RGB16H = 2,
	ALGO_RGB24H = 3
};

// these are the various block sizes that can occupy a 4x4 block
enum {
	BLOCK_2x2 = 0,
	BLOCK_2x4 = 1,
	BLOCK_4x2 = 2,
	BLOCK_4x4 = 3
};

// { valid for metatype }, algorithm, num of deltas, vert res, horiz res
struct CompressionType {
	int algorithm;
	int blockWidth; // vres
	int blockHeight; // hres
	int blockType;
};

static const CompressionType compressionTypes[17] = {
	{ ALGO_NOP,	0, 0, 0 },

	{ ALGO_RGB16V, 4, 4, BLOCK_4x4 },
	{ ALGO_RGB16H, 4, 4, BLOCK_4x4 },
	{ ALGO_RGB16V, 4, 2, BLOCK_4x2 },
	{ ALGO_RGB16H, 4, 2, BLOCK_4x2 },

	{ ALGO_RGB16V, 2, 4, BLOCK_2x4 },
	{ ALGO_RGB16H, 2, 4, BLOCK_2x4 },
	{ ALGO_RGB16V, 2, 2, BLOCK_2x2 },
	{ ALGO_RGB16H, 2, 2, BLOCK_2x2 },

	{ ALGO_NOP,	4, 4, BLOCK_4x4 },
	{ ALGO_RGB24H, 4, 4, BLOCK_4x4 },
	{ ALGO_NOP,	4, 2, BLOCK_4x2 },
	{ ALGO_RGB24H, 4, 2, BLOCK_4x2 },

	{ ALGO_NOP,	2, 4, BLOCK_2x4 },
	{ ALGO_RGB24H, 2, 4, BLOCK_2x4 },
	{ ALGO_NOP,	2, 2, BLOCK_2x2 },
	{ ALGO_RGB24H, 2, 2, BLOCK_2x2 }
};

TrueMotion1Decoder::TrueMotion1Decoder(uint16 width, uint16 height) {
	_surface = new Graphics::Surface();
	_width = width;
	_height = height;

	_surface->create(width, height, getPixelFormat());

	// there is a vertical predictor for each pixel in a line; each vertical
	// predictor is 0 to start with
	_vertPred = new uint32[_width];

	_buf = _mbChangeBits = _indexStream = 0;
	_lastDeltaset = _lastVectable = -1;
}

TrueMotion1Decoder::~TrueMotion1Decoder() {
	_surface->free();
	delete _surface;
	delete[] _vertPred;
}

void TrueMotion1Decoder::selectDeltaTables(int deltaTableIndex) {
	if (deltaTableIndex > 3)
		return;

	for (byte i = 0; i < 8; i++) {
		_ydt[i] = ydts[deltaTableIndex][i];
		_cdt[i] = cdts[deltaTableIndex][i];

		// Y skinny deltas need to be halved for some reason; maybe the
		// skinny Y deltas should be modified
		// Drop the lsb before dividing by 2-- net effect: round down
		// when dividing a negative number (e.g., -3/2 = -2, not -1)
		_ydt[i] &= 0xFFFE;
		_ydt[i] /= 2;
	}
}

int TrueMotion1Decoder::makeYdt16Entry(int p1, int p2) {
#ifdef SCUMM_BIG_ENDIAN
	// Swap the values on BE systems. FFmpeg does this too.
	SWAP<int>(p1, p2);
#endif

	int lo = _ydt[p1];
	lo += (lo << 6) + (lo << 11);
	int hi = _ydt[p2];
	hi += (hi << 6) + (hi << 11);
	return lo + (hi << 16);
}

int TrueMotion1Decoder::makeCdt16Entry(int p1, int p2) {
	int b = _cdt[p2];
	int r = _cdt[p1] << 11;
	int lo = b + r;
	return lo + (lo << 16);
}

void TrueMotion1Decoder::genVectorTable16(const byte *selVectorTable) {
	memset(&_yPredictorTable, 0, sizeof(PredictorTableEntry) * 1024);
	memset(&_cPredictorTable, 0, sizeof(PredictorTableEntry) * 1024);

	for (int i = 0; i < 1024; i += 4) {
		int len = *selVectorTable++ / 2;
		for (int j = 0; j < len; j++) {
			byte deltaPair = *selVectorTable++;
			_yPredictorTable[i + j].color = makeYdt16Entry(deltaPair >> 4, deltaPair & 0xf);
			_cPredictorTable[i + j].color = makeCdt16Entry(deltaPair >> 4, deltaPair & 0xf);
		}

		_yPredictorTable[i + (len - 1)].getNextIndex = true;
		_cPredictorTable[i + (len - 1)].getNextIndex = true;
	}
}

void TrueMotion1Decoder::decodeHeader(Common::SeekableReadStream *stream) {
	_buf = new byte[stream->size()];
	stream->read(_buf, stream->size());

	byte headerBuffer[128];  // logical maximum size of the header
	const byte *selVectorTable;

	// There is 1 change bit per 4 pixels, so each change byte represents
	// 32 pixels; divide width by 4 to obtain the number of change bits and
	// then round up to the nearest byte.
	_mbChangeBitsRowSize = ((_width >> 2) + 7) >> 3;

	_header.headerSize = ((_buf[0] >> 5) | (_buf[0] << 3)) & 0x7f;

	if (_buf[0] < 0x10)
		error("Invalid TrueMotion1 header size %d", _header.headerSize);

	// unscramble the header bytes with a XOR operation
	memset(headerBuffer, 0, 128);
	for (int i = 1; i < _header.headerSize; i++)
		headerBuffer[i - 1] = _buf[i] ^ _buf[i + 1];

	_header.compression = headerBuffer[0];
	_header.deltaset = headerBuffer[1];
	_header.vectable = headerBuffer[2];
	_header.ysize = READ_LE_UINT16(&headerBuffer[3]);
	_header.xsize = READ_LE_UINT16(&headerBuffer[5]);
	_header.checksum = READ_LE_UINT16(&headerBuffer[7]);
	_header.version = headerBuffer[9];
	_header.headerType = headerBuffer[10];
	_header.flags = headerBuffer[11];
	_header.control = headerBuffer[12];

	// Version 2
	if (_header.version >= 2) {
		if (_header.headerType > 3) {
			error("Invalid header type %d", _header.headerType);
		} else if (_header.headerType == 2 || _header.headerType == 3) {
			_flags = _header.flags;
			if (!(_flags & FLAG_INTERFRAME))
				_flags |= FLAG_KEYFRAME;
		} else
			_flags = FLAG_KEYFRAME;
	} else // Version 1
		_flags = FLAG_KEYFRAME;

	if (_flags & FLAG_SPRITE) {
		error("SPRITE frame found, please report the sample to the developers");
	} else if (_header.headerType < 2 && _header.xsize < 213 && _header.ysize >= 176) {
		_flags |= FLAG_INTERPOLATED;
		error("INTERPOLATION selected, please report the sample to the developers");
	}

	if (_header.compression >= 17)
		error("Invalid TrueMotion1 compression type %d", _header.compression);

	if (_header.deltaset != _lastDeltaset || _header.vectable != _lastVectable)
		selectDeltaTables(_header.deltaset);

	if ((_header.compression & 1) && _header.headerType)
		selVectorTable = pc_tbl2;
	else if (_header.vectable < 4)
		selVectorTable = tables[_header.vectable - 1];
	else
		error("Invalid vector table id %d", _header.vectable);

	if (_header.deltaset != _lastDeltaset || _header.vectable != _lastVectable)
		genVectorTable16(selVectorTable);

	// set up pointers to the other key data chunks
	_mbChangeBits = _buf + _header.headerSize;

	if (_flags & FLAG_KEYFRAME) {
		// no change bits specified for a keyframe; only index bytes
		_indexStream = _mbChangeBits;
	} else {
		// one change bit per 4x4 block
		_indexStream = _mbChangeBits + _mbChangeBitsRowSize * (_height >> 2);
	}

	_indexStreamSize = stream->size() - (_indexStream - _buf);

	_lastDeltaset = _header.deltaset;
	_lastVectable = _header.vectable;
	_blockWidth = compressionTypes[_header.compression].blockWidth;
	_blockHeight = compressionTypes[_header.compression].blockHeight;
	_blockType = compressionTypes[_header.compression].blockType;
}

#define GET_NEXT_INDEX() \
do { \
	if (indexStreamIndex >= _indexStreamSize) \
		error("TrueMotion1 decoder went out of bounds"); \
	index = _indexStream[indexStreamIndex++] * 4; \
} while (0) \

#define APPLY_C_PREDICTOR() \
	predictor_pair = _cPredictorTable[index].color; \
	horizPred += predictor_pair; \
	if (_cPredictorTable[index].getNextIndex) { \
		GET_NEXT_INDEX(); \
		if (!index) { \
			GET_NEXT_INDEX(); \
			predictor_pair = _cPredictorTable[index].color; \
			horizPred += predictor_pair * 5; \
			if (_cPredictorTable[index].getNextIndex) \
				GET_NEXT_INDEX(); \
			else \
				index++; \
		} \
	} else \
		index++

#define APPLY_Y_PREDICTOR() \
	predictor_pair = _yPredictorTable[index].color; \
	horizPred += predictor_pair; \
	if (_yPredictorTable[index].getNextIndex) { \
		GET_NEXT_INDEX(); \
		if (!index) { \
			GET_NEXT_INDEX(); \
			predictor_pair = _yPredictorTable[index].color; \
			horizPred += predictor_pair * 5; \
			if (_yPredictorTable[index].getNextIndex) \
				GET_NEXT_INDEX(); \
			else \
				index++; \
		} \
	} else \
		index++

#define OUTPUT_PIXEL_PAIR() \
	*currentPixelPair = *vertPred + horizPred; \
	*vertPred++ = *currentPixelPair++

void TrueMotion1Decoder::decode16() {
	uint32 predictor_pair;
	bool keyframe = _flags & FLAG_KEYFRAME;
	int indexStreamIndex = 0;

	// these variables are for managing the main index stream
	int index;

	// clean out the line buffer
	memset(_vertPred, 0, _width * 4);

	GET_NEXT_INDEX();

	for (int y = 0; y < _height; y++) {
		// re-init variables for the next line iteration
		uint32 horizPred = 0;
		uint32 *currentPixelPair = (uint32 *)_surface->getBasePtr(0, y);
		uint32 *vertPred = _vertPred;
		int mbChangeIndex = 0;
		byte mbChangeByte = _mbChangeBits[mbChangeIndex++];
		byte mbChangeByteMask = 1;

		for (int pixelsLeft = _width; pixelsLeft > 0; pixelsLeft -= 4) {
			if (keyframe || (mbChangeByte & mbChangeByteMask) == 0) {
				switch (y & 3) {
				case 0:
					// if macroblock width is 2, apply C-Y-C-Y; else
					// apply C-Y-Y
					if (_blockWidth == 2) {
						APPLY_C_PREDICTOR();
						APPLY_Y_PREDICTOR();
						OUTPUT_PIXEL_PAIR();
						APPLY_C_PREDICTOR();
						APPLY_Y_PREDICTOR();
						OUTPUT_PIXEL_PAIR();
					} else {
						APPLY_C_PREDICTOR();
						APPLY_Y_PREDICTOR();
						OUTPUT_PIXEL_PAIR();
						APPLY_Y_PREDICTOR();
						OUTPUT_PIXEL_PAIR();
					}
					break;
				case 1:
				case 3:
					// always apply 2 Y predictors on these iterations
					APPLY_Y_PREDICTOR();
					OUTPUT_PIXEL_PAIR();
					APPLY_Y_PREDICTOR();
					OUTPUT_PIXEL_PAIR();
					break;
				case 2:
					// this iteration might be C-Y-C-Y, Y-Y, or C-Y-Y
					// depending on the macroblock type
					if (_blockType == BLOCK_2x2) {
						APPLY_C_PREDICTOR();
						APPLY_Y_PREDICTOR();
						OUTPUT_PIXEL_PAIR();
						APPLY_C_PREDICTOR();
						APPLY_Y_PREDICTOR();
						OUTPUT_PIXEL_PAIR();
					} else if (_blockType == BLOCK_4x2) {
						APPLY_C_PREDICTOR();
						APPLY_Y_PREDICTOR();
						OUTPUT_PIXEL_PAIR();
						APPLY_Y_PREDICTOR();
						OUTPUT_PIXEL_PAIR();
					} else {
						APPLY_Y_PREDICTOR();
						OUTPUT_PIXEL_PAIR();
						APPLY_Y_PREDICTOR();
						OUTPUT_PIXEL_PAIR();
					}
					break;
				}
			} else {
				// skip (copy) four pixels, but reassign the horizontal
				// predictor
				*vertPred++ = *currentPixelPair++;
				horizPred = *currentPixelPair - *vertPred;
				*vertPred++ = *currentPixelPair++;
			}

			if (!keyframe) {
				mbChangeByteMask <<= 1;

				// next byte
				if (!mbChangeByteMask) {
					mbChangeByte = _mbChangeBits[mbChangeIndex++];
					mbChangeByteMask = 1;
				}
			}
		}

		// next change row
		if (((y + 1) & 3) == 0)
			_mbChangeBits += _mbChangeBitsRowSize;
	}
}

const Graphics::Surface *TrueMotion1Decoder::decodeImage(Common::SeekableReadStream *stream) {
	decodeHeader(stream);

	if (compressionTypes[_header.compression].algorithm == ALGO_NOP)
		return 0;

	if (compressionTypes[_header.compression].algorithm == ALGO_RGB24H) {
		warning("Unhandled TrueMotion1 24bpp frame");
		return 0;
	} else
		decode16();

	delete[] _buf;

	return _surface;
}

} // End of namespace Video

#endif
