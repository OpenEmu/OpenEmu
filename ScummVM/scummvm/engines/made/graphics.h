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

#ifndef MADE_GRAPHICS_H
#define MADE_GRAPHICS_H

#include "common/scummsys.h"

namespace Graphics {
struct Surface;
}

namespace Made {

class ValueReader {
public:
	ValueReader(byte *source, bool nibbleMode) : _buffer(source), _nibbleBuf(0), _nibbleMode(nibbleMode), _nibbleSwitch(false) {}
	byte readPixel();
	uint16 readUint16();
	uint32 readUint32();
	void resetNibbleSwitch();
protected:
	byte _nibbleBuf;
	bool _nibbleMode, _nibbleSwitch;
	byte *_buffer;
};

void decompressImage(byte *source, Graphics::Surface &surface, uint16 cmdOffs, uint16 pixelOffs, uint16 maskOffs, uint16 lineSize, byte cmdFlags, byte pixelFlags, byte maskFlags, bool deltaFrame = false);
void decompressMovieImage(byte *source, Graphics::Surface &surface, uint16 cmdOffs, uint16 pixelOffs, uint16 maskOffs, uint16 lineSize);

} // End of namespace Made

#endif /* MADE_H */
