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

#ifndef SKY_RNC_DECO_H
#define SKY_RNC_DECO_H



#define RNC_SIGNATURE   0x524E4301 // "RNC\001"

namespace Sky {

class RncDecoder {

protected:
	uint16 _rawTable[64];
	uint16 _posTable[64];
	uint16 _lenTable[64];
	uint16 _crcTable[256];

	uint16 _bitBuffl;
	uint16 _bitBuffh;
	uint8 _bitCount;

	const uint8 *_srcPtr;
	uint8 *_dstPtr;

public:
	RncDecoder();
	~RncDecoder();
	int32 unpackM1(const void *input, void *output, uint16 key);

protected:
	void initCrc();
	uint16 crcBlock(const uint8 *block, uint32 size);
	uint16 inputBits(uint8 amount);
	void makeHufftable(uint16 *table);
	uint16 inputValue(uint16 *table);

};

} // End of namespace Sky

#endif
