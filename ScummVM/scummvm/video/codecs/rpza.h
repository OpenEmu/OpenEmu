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

#ifndef VIDEO_CODECS_RPZA_H
#define VIDEO_CODECS_RPZA_H

#include "graphics/pixelformat.h"
#include "video/codecs/codec.h"

namespace Video {

class RPZADecoder : public Codec {
public:
	RPZADecoder(uint16 width, uint16 height);
	~RPZADecoder();

	const Graphics::Surface *decodeImage(Common::SeekableReadStream *stream);
	Graphics::PixelFormat getPixelFormat() const { return Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0); }

private:
	Graphics::Surface *_surface;
};

} // End of namespace Video

#endif
