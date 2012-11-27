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

// Based on Deniz Oezmen's code and Xentax Wiki documentation
// http://oezmen.eu/
// http://wiki.xentax.com/index.php/The_Last_Express_BG

#include "lastexpress/data/background.h"

#include "lastexpress/debug.h"

#include "common/rect.h"
#include "common/stream.h"

namespace LastExpress {

Background::Background() : _data(NULL) {
	memset(&_header, 0, sizeof(BackgroundHeader));
}

Background::~Background() {
	delete[] _data;
}

bool Background::load(Common::SeekableReadStream *stream) {
	if (!stream)
		return false;

	// Reset data
	delete[] _data;

	// Load Background header
	_header.posX = stream->readUint32LE();
	_header.posY = stream->readUint32LE();
	_header.width = stream->readUint32LE();
	_header.height = stream->readUint32LE();
	_header.redSize = stream->readUint32LE();
	_header.blueSize = stream->readUint32LE();
	_header.greenSize = stream->readUint32LE();

	debugC(3, kLastExpressDebugGraphics, "Background Info: (%d, %d) - (%d x %d) - (%d, %d, %d)",
	                                    _header.posX, _header.posY, _header.width, _header.height,
	                                    _header.redSize, _header.blueSize, _header.greenSize);

	// Load and decompress Background channel data
	uint32 numPix = _header.width * _header.height;
	byte *dataR = decodeComponent(stream, _header.redSize, numPix);
	byte *dataB = decodeComponent(stream, _header.blueSize, numPix);
	byte *dataG = decodeComponent(stream, _header.greenSize, numPix);

	// Save to pixel buffer
	// FIXME handle big-endian case
	_data = new uint16[_header.width * _header.height];
	for (uint i = 0; i < _header.width * _header.height; i++)
		_data[i] = (uint16)((dataR[i] << 10) + (dataG[i] << 5) + dataB[i]);

	// Cleanup buffers
	delete[] dataR;
	delete[] dataG;
	delete[] dataB;

	delete stream;

	return true;
}

Common::Rect Background::draw(Graphics::Surface *surface) {
	if (!_data) {
		debugC(2, kLastExpressDebugGraphics, "Trying to show a background before loading data");
		return Common::Rect();
	}

	int i = 0;
	for (uint16 y = 0; y < _header.height; y++) {
		for (uint16 x = 0; x < _header.width; x++) {
			surface->fillRect(Common::Rect((int16)(_header.posX + x), (int16)(_header.posY + y), (int16)(_header.posX + x + 1), (int16)(_header.posY + y + 1)), _data[i]);
			i ++;
		}
	}

	return Common::Rect((int16)_header.posX, (int16)_header.posY, (int16)(_header.posX + _header.width), (int16)(_header.posY + _header.height));
}

byte *Background::decodeComponent(Common::SeekableReadStream *in, uint32 inSize, uint32 outSize) const {
	// Create the destination array
	byte *out = new byte[outSize];
	if (!out)
		return NULL;

	// Initialize the decoding
	memset(out, 0, outSize * sizeof(byte));
	uint32 inPos = 0;
	uint32 outPos = 0;

	// Decode
	while (inPos < inSize) {
		byte inByte = in->readByte();
		inPos++;

		if (inByte < 0x80) {
			// Direct decompression (RLE)
			byte len = (inByte >> 5) + 1;
			byte data = inByte & 0x1f;
			for (int i = 0; i < len && outPos < outSize; i++)
				out[outPos++] = data;
		} else {
			// Buffer back reference, 4096 byte window
			// Take inByte and the following value as a big endian
			// OfsLen while zeroing the first bit
			uint16 ofsLen = ((inByte & 0x7F) << 8) | in->readByte();
			inPos++;

			int32 len = (ofsLen >> 12) + 3;
			int32 hisPos = (int32)(outPos + (ofsLen & 0x0FFF) - 4096);
			for (int i = 0; i < len && outPos < outSize; i++)
				out[outPos++] = out[hisPos++];
		}
	}

	return out;
}

} // End of namespace LastExpress
