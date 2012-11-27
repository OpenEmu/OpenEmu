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

#include "made/graphics.h"

#include "common/endian.h"
#include "common/textconsole.h"
#include "common/debug.h"
#include "common/util.h"

#include "graphics/surface.h"

namespace Made {

byte ValueReader::readPixel() {
	byte value;
	if (_nibbleMode) {
		if (_nibbleSwitch) {
			value = (_buffer[0] >> 4) & 0x0F;
			_buffer++;
		} else {
			value = _buffer[0] & 0x0F;
		}
		_nibbleSwitch = !_nibbleSwitch;
	} else {
		value = _buffer[0];
		_buffer++;
	}
	return value;
}

uint16 ValueReader::readUint16() {
	uint16 value = READ_LE_UINT16(_buffer);
	_buffer += 2;
	return value;
}

uint32 ValueReader::readUint32() {
	uint32 value = READ_LE_UINT32(_buffer);
	_buffer += 4;
	return value;
}

void ValueReader::resetNibbleSwitch() {
	_nibbleSwitch = false;
}

void decompressImage(byte *source, Graphics::Surface &surface, uint16 cmdOffs, uint16 pixelOffs, uint16 maskOffs, uint16 lineSize, byte cmdFlags, byte pixelFlags, byte maskFlags, bool deltaFrame) {

	const int offsets[] = {
		0, 1, 2, 3,
		320, 321, 322, 323,
		640, 641, 642, 643,
		960, 961, 962, 963
	};

	uint16 width = surface.w;
	uint16 height = surface.h;

	byte *cmdBuffer = source + cmdOffs;
	ValueReader maskReader(source + maskOffs, (maskFlags & 2) != 0);
	ValueReader pixelReader(source + pixelOffs, (pixelFlags & 2) != 0);

	if ((maskFlags != 0) && (maskFlags != 2) && (pixelFlags != 0) && (pixelFlags != 2) && (cmdFlags != 0))
		error("decompressImage() Unsupported flags: cmdFlags = %02X; maskFlags = %02X, pixelFlags = %02X", cmdFlags, maskFlags, pixelFlags);

	byte *destPtr = (byte *)surface.getBasePtr(0, 0);

	byte lineBuf[640 * 4];
	byte bitBuf[40];

	int bitBufLastOfs = (((lineSize + 1) >> 1) << 1) - 2;
	int bitBufLastCount = ((width + 3) >> 2) & 7;
	if (bitBufLastCount == 0)
		bitBufLastCount = 8;

	while (height > 0) {

		int drawDestOfs = 0;

		memset(lineBuf, 0, sizeof(lineBuf));

		memcpy(bitBuf, cmdBuffer, lineSize);
		cmdBuffer += lineSize;

		for (uint16 bitBufOfs = 0; bitBufOfs < lineSize; bitBufOfs += 2) {

			uint16 bits = READ_LE_UINT16(&bitBuf[bitBufOfs]);

			int bitCount;
			if (bitBufOfs == bitBufLastOfs)
				bitCount = bitBufLastCount;
			else
				bitCount = 8;

			for (int curCmd = 0; curCmd < bitCount; curCmd++) {
				int cmd = bits & 3;
				bits >>= 2;

				byte pixels[4];
				uint32 mask;

				switch (cmd) {

				case 0:
					pixels[0] = pixelReader.readPixel();
					for (int i = 0; i < 16; i++)
						lineBuf[drawDestOfs + offsets[i]] = pixels[0];
					break;

				case 1:
					pixels[0] = pixelReader.readPixel();
					pixels[1] = pixelReader.readPixel();
					mask = maskReader.readUint16();
					for (int i = 0; i < 16; i++) {
						lineBuf[drawDestOfs + offsets[i]] = pixels[mask & 1];
						mask >>= 1;
					}
					break;

				case 2:
					pixels[0] = pixelReader.readPixel();
					pixels[1] = pixelReader.readPixel();
					pixels[2] = pixelReader.readPixel();
					pixels[3] = pixelReader.readPixel();
					mask = maskReader.readUint32();
					for (int i = 0; i < 16; i++) {
						lineBuf[drawDestOfs + offsets[i]] = pixels[mask & 3];
						mask >>= 2;
					}
					break;

				case 3:
					if (!deltaFrame) {
						// For EGA pictures: Pixels are read starting from a new byte
						maskReader.resetNibbleSwitch();
						// Yes, it reads from maskReader here
						for (int i = 0; i < 16; i++)
							lineBuf[drawDestOfs + offsets[i]] = maskReader.readPixel();
					}
					break;

				}

				drawDestOfs += 4;

			}

		}

		if (deltaFrame) {
			for (int y = 0; y < 4 && height > 0; y++, height--) {
				for (int x = 0; x < width; x++) {
					if (lineBuf[x + y * 320] != 0)
						*destPtr = lineBuf[x + y * 320];
					destPtr++;
				}
			}
		} else {
			for (int y = 0; y < 4 && height > 0; y++, height--) {
				memcpy(destPtr, &lineBuf[y * 320], width);
				destPtr += width;
			}
		}

	}

}

void decompressMovieImage(byte *source, Graphics::Surface &surface, uint16 cmdOffs, uint16 pixelOffs, uint16 maskOffs, uint16 lineSize) {

	uint16 width = surface.w;
	uint16 height = surface.h;
	uint16 bx = 0, by = 0, bw = ((width + 3) / 4) * 4;

	byte *cmdBuffer = source + cmdOffs;
	byte *maskBuffer = source + maskOffs;
	byte *pixelBuffer = source + pixelOffs;

	byte *destPtr = (byte *)surface.getBasePtr(0, 0);

	byte bitBuf[40];

	int bitBufLastOfs = (((lineSize + 1) >> 1) << 1) - 2;
	int bitBufLastCount = ((width + 3) >> 2) & 7;
	if (bitBufLastCount == 0)
		bitBufLastCount = 8;

	debug(1, "width = %d; bw = %d", width, bw);

	while (height > 0) {

		memcpy(bitBuf, cmdBuffer, lineSize);
		cmdBuffer += lineSize;

		for (uint16 bitBufOfs = 0; bitBufOfs < lineSize; bitBufOfs += 2) {

			uint16 bits = READ_LE_UINT16(&bitBuf[bitBufOfs]);

			int bitCount;
			if (bitBufOfs == bitBufLastOfs)
				bitCount = bitBufLastCount;
			else
				bitCount = 8;

			for (int curCmd = 0; curCmd < bitCount; curCmd++) {
				uint cmd = bits & 3;
				bits >>= 2;

				byte pixels[4], block[16];
				uint32 mask;

				switch (cmd) {

				case 0:
					pixels[0] = *pixelBuffer++;
					for (int i = 0; i < 16; i++)
						block[i] = pixels[0];
					break;

				case 1:
					pixels[0] = *pixelBuffer++;
					pixels[1] = *pixelBuffer++;
					mask = READ_LE_UINT16(maskBuffer);
					maskBuffer += 2;
					for (int i = 0; i < 16; i++) {
						block[i] = pixels[mask & 1];
						mask >>= 1;
					}
					break;

				case 2:
					pixels[0] = *pixelBuffer++;
					pixels[1] = *pixelBuffer++;
					pixels[2] = *pixelBuffer++;
					pixels[3] = *pixelBuffer++;
					mask = READ_LE_UINT32(maskBuffer);
					maskBuffer += 4;
					for (int i = 0; i < 16; i++) {
						block[i] = pixels[mask & 3];
						mask >>= 2;
					}
					break;

				case 3:
					break;

				}

				if (cmd != 3) {
					uint16 blockPos = 0;
					uint32 maxW = MIN(4, surface.w - bx);
					uint32 maxH = (MIN(4, surface.h - by) + by) * width;
					for (uint32 yc = by * width; yc < maxH; yc += width) {
						for (uint32 xc = 0; xc < maxW; xc++) {
							destPtr[(bx + xc) + yc] = block[xc + blockPos];
						}
						blockPos += 4;
					}
				}

				bx += 4;
				if (bx >= bw) {
					bx = 0;
					by += 4;
				}

			}

		}

		height -= 4;

	}

}

} // End of namespace Made
