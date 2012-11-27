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

#ifndef LURE_DECODE_H
#define LURE_DECODE_H


#include "lure/luredefs.h"
#include "lure/memory.h"

namespace Lure {

class PictureDecoder {
private:
	MemoryBlock *dataIn;
	uint32 BP;
	uint32 dataPos, dataPos2;
	uint32 outputOffset;
	byte AL, AH;
	byte CH, CL;

	void writeByte(MemoryBlock *dest, byte v);
	void writeBytes(MemoryBlock *dest, byte v, uint16 numBytes);
	byte DSSI(bool incr = true);
	byte ESBX(bool incr = true);
	void decrCtr();
	bool shlCarry();

public:
	MemoryBlock *decode(MemoryBlock *src, uint32 maxOutputSize  = SCREEN_SIZE + 1);
	MemoryBlock *egaDecode(MemoryBlock *src, uint32 maxOutputSize);
	MemoryBlock *vgaDecode(MemoryBlock *src, uint32 maxOutputSize);
};

class AnimationDecoder {
public:
	static void rcl(uint16 &value, bool &carry);
	static uint32 decode_data(MemoryBlock *src, MemoryBlock *dest, uint32 srcPos);
	static void decode_data_2(MemoryBlock *src, byte *&pSrc, uint16 &currData,
						uint16 &bitCtr, uint16 &dx, bool &carry);
};

} // End of namespace Lure

#endif
