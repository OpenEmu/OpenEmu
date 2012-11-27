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

namespace Cruise {

struct UnpackCtx {
	int size, datasize;
	uint32 crc;
	uint32 chk;
	byte *dst;
	const byte *src;
};

static int rcr(UnpackCtx *uc, int CF) {
	int rCF = (uc->chk & 1);
	uc->chk >>= 1;
	if (CF) {
		uc->chk |= 0x80000000;
	}
	return rCF;
}

static int nextChunk(UnpackCtx *uc) {
	int CF = rcr(uc, 0);
	if (uc->chk == 0) {
		uc->chk = READ_BE_UINT32(uc->src);
		uc->src -= 4;
		uc->crc ^= uc->chk;
		CF = rcr(uc, 1);
	}
	return CF;
}

static uint16 getCode(UnpackCtx *uc, byte numChunks) {
	uint16 c = 0;
	while (numChunks--) {
		c <<= 1;
		if (nextChunk(uc)) {
			c |= 1;
		}
	}
	return c;
}

static void unpackHelper1(UnpackCtx *uc, byte numChunks, byte addCount) {
	uint16 count = getCode(uc, numChunks) + addCount + 1;
	uc->datasize -= count;
	while (count--) {
		*uc->dst = (byte)getCode(uc, 8);
		--uc->dst;
	}
}

static void unpackHelper2(UnpackCtx *uc, byte numChunks) {
	uint16 i = getCode(uc, numChunks);
	uint16 count = uc->size + 1;
	uc->datasize -= count;
	while (count--) {
		*uc->dst = *(uc->dst + i);
		--uc->dst;
	}
}

bool delphineUnpack(byte *dst, const byte *src, int len) {
	UnpackCtx uc;
	uc.src = src + len - 4;
	uc.datasize = READ_BE_UINT32(uc.src);
	uc.src -= 4;
	uc.dst = dst + uc.datasize - 1;
	uc.size = 0;
	uc.crc = READ_BE_UINT32(uc.src);
	uc.src -= 4;
	uc.chk = READ_BE_UINT32(uc.src);
	uc.src -= 4;
	uc.crc ^= uc.chk;
	do {
		if (!nextChunk(&uc)) {
			uc.size = 1;
			if (!nextChunk(&uc)) {
				unpackHelper1(&uc, 3, 0);
			} else {
				unpackHelper2(&uc, 8);
			}
		} else {
			uint16 c = getCode(&uc, 2);
			if (c == 3) {
				unpackHelper1(&uc, 8, 8);
			} else if (c < 2) {
				uc.size = c + 2;
				unpackHelper2(&uc, c + 9);
			} else {
				uc.size = getCode(&uc, 8);
				unpackHelper2(&uc, 12);
			}
		}
	} while (uc.datasize > 0);
	return uc.crc == 0;
}

} // End of namespace Cruise
