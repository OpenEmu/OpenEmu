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

#ifndef LASTEXPRESS_BACKGROUND_H
#define LASTEXPRESS_BACKGROUND_H

/*
	Background file format (.BG)

	header:
	    uint32 {4}   - position X on screen
	    uint32 {4}   - position Y on screen
	    uint32 {4}   - image width
	    uint32 {4}   - image height
	    uint32 {4}   - red color channel data size
	    uint32 {4}   - blue color channel data size
	    uint32 {4}   - green color channel data size

	data:
	    byte {x}     - red color channel data
	    byte {x}     - blue color channel data
	    byte {x}     - green color channel data
*/

#include "lastexpress/drawable.h"

namespace Common {
class SeekableReadStream;
}

namespace LastExpress {

class Background : public Drawable {
public:
	Background();
	~Background();

	bool load(Common::SeekableReadStream *stream);

	Common::Rect draw(Graphics::Surface *surface);

private:
	struct BackgroundHeader {
		uint32 posX;       ///< position X on screen
		uint32 posY;       ///< position Y on screen
		uint32 width;      ///< image width
		uint32 height;     ///< image height
		uint32 redSize;    ///< red color channel data size
		uint32 blueSize;   ///< blue color channel data size
		uint32 greenSize;  ///< green color channel data size
	};

	BackgroundHeader _header;
	uint16 *_data;         ///< decoded background data

	byte *decodeComponent(Common::SeekableReadStream *in, uint32 inSize, uint32 outSize) const;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_BACKGROUND_H
