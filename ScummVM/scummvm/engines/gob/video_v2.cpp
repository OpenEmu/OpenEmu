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

Video_v2::Video_v2(GobEngine *vm) : Video_v1(vm) {
}

char Video_v2::spriteUncompressor(byte *sprBuf, int16 srcWidth, int16 srcHeight,
	    int16 x, int16 y, int16 transp, Surface &destDesc) {
	byte *memBuffer;
	byte *srcPtr;
	byte temp;
	uint32 sourceLeft;
	uint16 cmdVar;
	int16 curWidth, curHeight;
	int16 offset;
	int16 counter2;
	int16 bufPos;
	int16 strLen;
	int16 lenCmd;

	//_vm->validateVideoMode(destDesc._vidMode);

	if (sprBuf[0] != 1)
		return 0;

	if (sprBuf[1] != 2)
		return 0;

	if (sprBuf[2] == 2) {
		Surface sourceDesc(srcWidth, srcHeight, 1, sprBuf + 3);
		destDesc.blit(sourceDesc, 0, 0, srcWidth - 1, srcHeight - 1, x, y, (transp == 0) ? -1 : 0);
		return 1;
	} else if (sprBuf[2] == 1) {
		memBuffer = new byte[4370];
		assert(memBuffer);

		memset(memBuffer, 0, 4370);

		srcPtr = sprBuf + 3;

		sourceLeft = READ_LE_UINT32(srcPtr);

		Pixel destPtr = destDesc.get(x, y);

		curWidth  = 0;
		curHeight = 0;

		Pixel linePtr = destPtr;
		srcPtr += 4;

		if ((READ_LE_UINT16(srcPtr) == 0x1234) && (READ_LE_UINT16(srcPtr + 2) == 0x5678)) {
			srcPtr += 4;
			bufPos = 273;
			lenCmd = 18;
		} else {
			lenCmd = 100;
			bufPos = 4078;
		}

		memset(memBuffer, 32, bufPos);

		cmdVar = 0;
		while (1) {
			cmdVar >>= 1;
			if ((cmdVar & 0x100) == 0)
				cmdVar = *srcPtr++ | 0xFF00;

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
					if (++curHeight >= srcHeight)
						break;
				}

				memBuffer[bufPos] = temp;

				bufPos = (bufPos + 1) % 4096;

				if (--sourceLeft == 0)
					break;

			} else {
				offset = *srcPtr++;
				temp   = *srcPtr++;

				offset |= (temp & 0xF0) << 4;
				strLen  = (temp & 0x0F)  + 3;

				if (strLen == lenCmd)
					strLen = *srcPtr++ + 18;

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
						if (++curHeight >= srcHeight) {
							delete[] memBuffer;
							return 1;
						}
					}

					memBuffer[bufPos] = temp;
					bufPos = (bufPos + 1) % 4096;
				}

				if (strLen >= ((int32) sourceLeft)) {
					delete[] memBuffer;
					return 1;
				} else
					sourceLeft--;

			}

		}
	} else
		return 0;

	delete[] memBuffer;
	return 1;
}

} // End of namespace Gob
