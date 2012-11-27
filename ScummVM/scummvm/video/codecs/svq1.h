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

#ifndef VIDEO_CODECS_SVQ1_H
#define VIDEO_CODECS_SVQ1_H

#include "video/codecs/codec.h"

namespace Common {
class BitStream;
class Huffman;
struct Point;
}

namespace Video {

class SVQ1Decoder : public Codec {
public:
	SVQ1Decoder(uint16 width, uint16 height);
	~SVQ1Decoder();

	const Graphics::Surface *decodeImage(Common::SeekableReadStream *stream);
	Graphics::PixelFormat getPixelFormat() const { return _surface->format; }

private:
	Graphics::Surface *_surface;
	uint16 _width, _height;
	uint16 _frameWidth, _frameHeight;

	byte *_last[3];

	Common::Huffman *_blockType;
	Common::Huffman *_intraMultistage[6];
	Common::Huffman *_interMultistage[6];
	Common::Huffman *_intraMean;
	Common::Huffman *_interMean;
	Common::Huffman *_motionComponent;

	bool svq1DecodeBlockIntra(Common::BitStream *s, byte *pixels, int pitch);
	bool svq1DecodeBlockNonIntra(Common::BitStream *s, byte *pixels, int pitch);
	bool svq1DecodeMotionVector(Common::BitStream *s, Common::Point *mv, Common::Point **pmv);
	void svq1SkipBlock(byte *current, byte *previous, int pitch, int x, int y);
	bool svq1MotionInterBlock(Common::BitStream *ss, byte *current, byte *previous, int pitch,
			Common::Point *motion, int x, int y);
	bool svq1MotionInter4vBlock(Common::BitStream *ss, byte *current, byte *previous, int pitch,
			Common::Point *motion, int x, int y);
	bool svq1DecodeDeltaBlock(Common::BitStream *ss, byte *current, byte *previous, int pitch,
			Common::Point *motion, int x, int y);

	void putPixels8C(byte *block, const byte *pixels, int lineSize, int h);
	void putPixels8L2(byte *dst, const byte *src1, const byte *src2, int dstStride, int srcStride1, int srcStride2, int h);
	void putPixels8X2C(byte *block, const byte *pixels, int lineSize, int h);
	void putPixels8Y2C(byte *block, const byte *pixels, int lineSize, int h);
	void putPixels8XY2C(byte *block, const byte *pixels, int lineSize, int h);
	void putPixels16C(byte *block, const byte *pixels, int lineSize, int h);
	void putPixels16X2C(byte *block, const byte *pixels, int lineSize, int h);
	void putPixels16Y2C(byte *block, const byte *pixels, int lineSize, int h);
	void putPixels16XY2C(byte *block, const byte *pixels, int lineSize, int h);
};

} // End of namespace Video

#endif
