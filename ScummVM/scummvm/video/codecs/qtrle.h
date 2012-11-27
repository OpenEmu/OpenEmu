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

#ifndef VIDEO_CODECS_QTRLE_H
#define VIDEO_CODECS_QTRLE_H

#include "graphics/pixelformat.h"
#include "video/codecs/codec.h"

namespace Video {

class QTRLEDecoder : public Codec {
public:
	QTRLEDecoder(uint16 width, uint16 height, byte bitsPerPixel);
	~QTRLEDecoder();

	const Graphics::Surface *decodeImage(Common::SeekableReadStream *stream);
	Graphics::PixelFormat getPixelFormat() const;

private:
	byte _bitsPerPixel;

	Graphics::Surface *_surface;

	void decode1(Common::SeekableReadStream *stream, uint32 rowPtr, uint32 linesToChange);
	void decode2_4(Common::SeekableReadStream *stream, uint32 rowPtr, uint32 linesToChange, byte bpp);
	void decode8(Common::SeekableReadStream *stream, uint32 rowPtr, uint32 linesToChange);
	void decode16(Common::SeekableReadStream *stream, uint32 rowPtr, uint32 linesToChange);
	void decode24(Common::SeekableReadStream *stream, uint32 rowPtr, uint32 linesToChange);
	void decode32(Common::SeekableReadStream *stream, uint32 rowPtr, uint32 linesToChange);
};

} // End of namespace Video

#endif
