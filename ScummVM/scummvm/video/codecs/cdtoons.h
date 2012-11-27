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

#ifndef VIDEO_CODECS_CDTOONS_H
#define VIDEO_CODECS_CDTOONS_H

#include "video/codecs/codec.h"

#include "common/hashmap.h"

namespace Video {

struct CDToonsBlock {
	uint16 flags;
	uint32 size;
	uint16 startFrame;
	uint16 endFrame;
	uint16 unknown12;
	byte *data;
};

class CDToonsDecoder : public Codec {
public:
	CDToonsDecoder(uint16 width, uint16 height);
	~CDToonsDecoder();

	Graphics::Surface *decodeImage(Common::SeekableReadStream *stream);
	Graphics::PixelFormat getPixelFormat() const { return Graphics::PixelFormat::createFormatCLUT8(); }
	bool containsPalette() const { return true; }
	const byte *getPalette() { _dirtyPalette = false; return _palette; }
	bool hasDirtyPalette() const { return _dirtyPalette; }

private:
	Graphics::Surface *_surface;
	byte _palette[256 * 3];
	bool _dirtyPalette;
	uint16 _currentPaletteId;

	uint16 _currentFrame;
	Common::HashMap<uint16, CDToonsBlock> _blocks;

	void renderBlock(byte *data, uint size, int x, int y, uint width, uint height);
	void setPalette(byte *data);
};

} // End of namespace Video

#endif
