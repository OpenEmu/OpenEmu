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

/*
 * PNG decoder used in engines:
 *  - sword25
 *  - wintermute
 * Dependencies:
 *  - libpng
 */

#ifndef GRAPHICS_PNG_H
#define GRAPHICS_PNG_H

#include "common/scummsys.h"
#include "common/textconsole.h"
#include "graphics/decoders/image_decoder.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {

struct Surface;
struct PixelFormat;

class PNGDecoder : public ImageDecoder {
public:
	PNGDecoder();
	~PNGDecoder();

	bool loadStream(Common::SeekableReadStream &stream);
	void destroy();
	const Graphics::Surface *getSurface() const { return _outputSurface; }
	const byte *getPalette() const { return _palette; }
	uint16 getPaletteColorCount() const { return _paletteColorCount; }
private:
	Common::SeekableReadStream *_stream;
	byte *_palette;
	uint16 _paletteColorCount;

	Graphics::Surface *_outputSurface;
};

} // End of namespace Graphics

#endif // GRAPHICS_PNG_H
