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

#include "common/textconsole.h"

#include "graphics/maccursor.h"

namespace Graphics {

MacCursor::MacCursor() {
	_surface = 0;
	memset(_palette, 0, 256 * 3);

	_hotspotX = 0;
	_hotspotY = 0;
}

MacCursor::~MacCursor() {
	clear();
}

void MacCursor::clear() {
	delete[] _surface; _surface = 0;
	memset(_palette, 0, 256 * 3);
}

bool MacCursor::readFromStream(Common::SeekableReadStream &stream, bool forceMonochrome) {
	clear();

	// Older Mac CURS monochrome cursors had a set size
	// All crsr cursors are larger than this
	if (stream.size() == 32 * 2 + 4)
		return readFromCURS(stream);

	return readFromCRSR(stream, forceMonochrome);
}

bool MacCursor::readFromCURS(Common::SeekableReadStream &stream) {
	// Grab B/W icon data
	_surface = new byte[16 * 16];
	for (int i = 0; i < 32; i++) {
		byte imageByte = stream.readByte();
		for (int b = 0; b < 8; b++)
			_surface[i * 8 + b] = (byte)((imageByte & (0x80 >> b)) > 0 ? 0 : 1);
	}

	// Apply mask data
	for (int i = 0; i < 32; i++) {
		byte imageByte = stream.readByte();
		for (int b = 0; b < 8; b++)
			if ((imageByte & (0x80 >> b)) == 0)
				_surface[i * 8 + b] = 0xff;
	}

	_hotspotY = stream.readUint16BE();
	_hotspotX = stream.readUint16BE();

	// Setup a basic palette
	_palette[1 * 3 + 0] = 0xff;
	_palette[1 * 3 + 1] = 0xff;
	_palette[1 * 3 + 2] = 0xff;

	return !stream.eos();
}

bool MacCursor::readFromCRSR(Common::SeekableReadStream &stream, bool forceMonochrome) {
	stream.readUint16BE(); // type
	stream.readUint32BE(); // offset to pixel map
	stream.readUint32BE(); // offset to pixel data
	stream.readUint32BE(); // expanded cursor data
	stream.readUint16BE(); // expanded data depth
	stream.readUint32BE(); // reserved

	// Read the B/W data first
	if (!readFromCURS(stream))
		return false;

	// Use b/w cursor on backends which don't support cursor palettes
	if (forceMonochrome)
		return true;

	stream.readUint32BE(); // reserved
	stream.readUint32BE(); // cursorID

	// Color version of cursor
	stream.readUint32BE(); // baseAddr

	// Keep only lowbyte for now
	stream.readByte();
	int iconRowBytes = stream.readByte();

	if (!iconRowBytes)
		return false;

	int iconBounds[4];
	iconBounds[0] = stream.readUint16BE();
	iconBounds[1] = stream.readUint16BE();
	iconBounds[2] = stream.readUint16BE();
	iconBounds[3] = stream.readUint16BE();

	stream.readUint16BE(); // pmVersion
	stream.readUint16BE(); // packType
	stream.readUint32BE(); // packSize

	stream.readUint32BE(); // hRes
	stream.readUint32BE(); // vRes

	stream.readUint16BE(); // pixelType
	stream.readUint16BE(); // pixelSize
	stream.readUint16BE(); // cmpCount
	stream.readUint16BE(); // cmpSize

	stream.readUint32BE(); // planeByte
	stream.readUint32BE(); // pmTable
	stream.readUint32BE(); // reserved

	// Pixel data for cursor
	int iconDataSize =  iconRowBytes * (iconBounds[3] - iconBounds[1]);
	byte *iconData = new byte[iconDataSize];

	if (!iconData)
		error("Cannot allocate Mac color cursor iconData");

	stream.read(iconData, iconDataSize);

	// Color table
	stream.readUint32BE(); // ctSeed
	stream.readUint16BE(); // ctFlag
	uint16 ctSize = stream.readUint16BE() + 1;

	// Read just high byte of 16-bit color
	for (int c = 0; c < ctSize; c++) {
		stream.readUint16BE();
		_palette[c * 3 + 0] = stream.readUint16BE() >> 8;
		_palette[c * 3 + 1] = stream.readUint16BE() >> 8;
		_palette[c * 3 + 2] = stream.readUint16BE() >> 8;
	}

	int pixelsPerByte = (iconBounds[2] - iconBounds[0]) / iconRowBytes;
	int bpp           = 8 / pixelsPerByte;

	// build a mask to make sure the pixels are properly shifted out
	int bitmask = 0;
	for (int m = 0; m < bpp; m++) {
		bitmask <<= 1;
		bitmask  |= 1;
	}

	// Extract pixels from bytes
	for (int j = 0; j < iconDataSize; j++) {
		for (int b = 0; b < pixelsPerByte; b++) {
			int idx = j * pixelsPerByte + (pixelsPerByte - 1 - b);

			if (_surface[idx] != 0xff) // if mask is not there
				_surface[idx] = (byte)((iconData[j] >> (b * bpp)) & bitmask);
		}
	}

	delete[] iconData;
	return stream.pos() == stream.size();
}

} // End of namespace Common
