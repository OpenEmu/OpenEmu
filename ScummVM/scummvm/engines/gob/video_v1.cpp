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

#include "common/endian.h"

#include "gob/gob.h"
#include "gob/video.h"

namespace Gob {

Video_v1::Video_v1(GobEngine *vm) : Video(vm) {
}

char Video_v1::spriteUncompressor(byte *sprBuf, int16 srcWidth, int16 srcHeight,
	    int16 x, int16 y, int16 transp, Surface &destDesc) {
	byte *memBuffer;
	byte *srcPtr;
	byte temp;
	uint16 sourceLeft;
	uint16 cmdVar;
	int16 curWidth, curHeight;
	int16 offset;
	int16 counter2;
	int16 bufPos;
	int16 strLen;

	//_vm->validateVideoMode(destDesc._vidMode);

	if (sprBuf[0] != 1)
		return 0;

	if (sprBuf[1] != 2)
		return 0;

	if (sprBuf[2] == 2) {
		Surface sourceDesc(srcWidth, srcHeight, 1, sprBuf + 3);
		destDesc.blit(sourceDesc, 0, 0, srcWidth - 1, srcHeight - 1, x, y, (transp == 0) ? -1 : 0);
		return 1;
	} else {
		memBuffer = new byte[4114];
		assert(memBuffer);

		srcPtr = sprBuf + 3;
		sourceLeft = READ_LE_UINT16(srcPtr);

		Pixel destPtr = destDesc.get(x, y);

		curWidth = 0;
		curHeight = 0;

		Pixel linePtr = destPtr;
		srcPtr += 4;

		for (offset = 0; offset < 4078; offset++)
			memBuffer[offset] = 0x20;

		cmdVar = 0;
		bufPos = 4078;
		while (1) {
			cmdVar >>= 1;
			if ((cmdVar & 0x100) == 0) {
				cmdVar = *srcPtr | 0xFF00;
				srcPtr++;
			}
			if ((cmdVar & 1) != 0) {
				temp = *srcPtr++;
				if ((temp != 0) || (transp == 0))
					destPtr.set(temp);
				destPtr++;
				curWidth++;
				if (curWidth >= srcWidth) {
					curWidth = 0;
					linePtr += destDesc.getWidth();
					destPtr = linePtr;
					curHeight++;
					if (curHeight >= srcHeight)
						break;
				}
				sourceLeft--;
				if (sourceLeft == 0)
					break;

				memBuffer[bufPos] = temp;
				bufPos++;
				bufPos %= 4096;
			} else {
				offset = *srcPtr;
				srcPtr++;
				offset |= (*srcPtr & 0xF0) << 4;
				strLen = (*srcPtr & 0x0F) + 3;
				srcPtr++;

				for (counter2 = 0; counter2 < strLen; counter2++) {
					temp = memBuffer[(offset + counter2) % 4096];
					if ((temp != 0) || (transp == 0))
						destPtr.set(temp);
					destPtr++;

					curWidth++;
					if (curWidth >= srcWidth) {
						curWidth = 0;
						linePtr += destDesc.getWidth();
						destPtr = linePtr;
						curHeight++;
						if (curHeight >= srcHeight) {
							delete[] memBuffer;
							return 1;
						}
					}
					sourceLeft--;
					if (sourceLeft == 0) {
						delete[] memBuffer;
						return 1;
					}
					memBuffer[bufPos] = temp;
					bufPos++;
					bufPos %= 4096;
				}
			}
		}
	}
	delete[] memBuffer;
	return 1;
}

} // End of namespace Gob
