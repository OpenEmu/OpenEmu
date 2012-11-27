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

#include "common/debug.h"

#include "toon/tools.h"
#include "toon/toon.h"

namespace Toon {

uint32 decompressLZSS(byte *src, byte *dst, int dstsize) {
	debugC(5, kDebugTools, "decompressLZSS(src, dst, %d)", dstsize);

	byte *srcp = src;
	byte *dstp = dst;
	uint16 bitbuf;
	int32 len, ofs;
	len = 0;
	while (dstsize > 0) {
		bitbuf = 0x100 | *(srcp++);
		while (bitbuf != 1 && dstsize > 0) {
			if (bitbuf & 1) {
				ofs = READ_LE_UINT16(srcp);
				srcp += 2;
				len = ((ofs & 0xF000) >> 12) + 3;
				ofs = ofs | 0xF000;
				dstsize -= len;
				if (dstsize < 0)
					break;
				while (len--) {
					*dstp = *(byte *)(dstp + (signed short)ofs);
					dstp++;
				}
			} else {
				len = 0;
				while ((bitbuf & 2) == 0) {
					len++;
					bitbuf >>= 1;
				}
				len++;
				dstsize -= len;
				if (dstsize < 0)
					break;
				while (len--)
					*(dstp++) = *(srcp++);
			}
			bitbuf >>= 1;
		}
	}
	len += dstsize;
	if (len < 0)
		return 0;

	while (len--)
		*(dstp++) = *(srcp++);

	return (dstp - dst);
}

uint32 decompressSPCN(byte *src, byte *dst, uint32 dstsize) {
	debugC(1, kDebugTools, "decompressSPCN(src, dst, %d)", dstsize);

	byte *srcp = src;
	byte *dstp = dst, *dste = dst + dstsize;
	byte val;
	uint16 len, ofs;
	if (!(*srcp & 0x80)) srcp++;
	while (dstp < dste) {
		val = *(srcp++);
		if (val & 0x80) {
			if (val & 0x40) {
				if (val == 0xFE) {
					len = READ_LE_UINT16(srcp);
					while (len--)
						*(dstp++) = srcp[2];
					srcp += 3;
				} else {
					if (val == 0xFF) {
						len = READ_LE_UINT16(srcp);
						srcp += 2;
					} else {
						len = (val & 0x3F) + 3;
					}
					ofs = READ_LE_UINT16(srcp);
					srcp += 2;
					while (len--) {
						*dstp = *(byte *)(dstp - ofs);
						dstp++;
					}
				}
			} else {
				len = val & 0x3F;
				while (len--)
					*(dstp++) = *(srcp++);
			}
		} else {
			len = (val >> 4) + 3;
			ofs = ((val & 0x0F) << 8) | *(srcp++);
			while (len--) {
				*dstp = *(byte *)(dstp - ofs);
				dstp++;
			}
		}
	}
	return (dstp - dst);
}

//return codes
#define NOT_PACKED  0
#define PACKED_CRC  -1
#define UNPACKED_CRC    -2

//other defines
#define TABLE_SIZE  (16 * 8)
#define MIN_LENGTH  2
#define HEADER_LEN  18

RncDecoder::RncDecoder() {
	initCrc();
}

RncDecoder::~RncDecoder() { }

void RncDecoder::initCrc() {
	debugC(1, kDebugTools, "initCrc()");

	uint16 cnt = 0;
	uint16 tmp1 = 0;
	uint16 tmp2 = 0;

	for (tmp2 = 0; tmp2 < 0x100; tmp2++) {
		tmp1 = tmp2;
		for (cnt = 8; cnt > 0; cnt--) {
			if (tmp1 % 2) {
				tmp1 >>= 1;
				tmp1 ^= 0x0a001;
			} else
				tmp1 >>= 1;
		}
		_crcTable[tmp2] = tmp1;
	}
}

//calculate 16 bit crc of a block of memory
uint16 RncDecoder::crcBlock(const uint8 *block, uint32 size) {
	debugC(1, kDebugTools, "crcBlock(block, %d)", size);

	uint16 crc = 0;
	uint8 *crcTable8 = (uint8 *)_crcTable; //make a uint8* to crc_table
	uint8 tmp;
	uint32 i;

	for (i = 0; i < size; i++) {
		tmp = *block++;
		crc ^= tmp;
		tmp = (uint8)((crc >> 8) & 0x00FF);
		crc &= 0x00FF;
		crc = *(uint16 *)&crcTable8[crc << 1];
		crc ^= tmp;
	}

	return crc;
}

uint16 RncDecoder::inputBits(uint8 amount) {
	debugC(5, kDebugTools, "inputBits(%d)", amount);

	uint16 newBitBuffh = _bitBuffh;
	uint16 newBitBuffl = _bitBuffl;
	int16 newBitCount = _bitCount;
	uint16 remBits, returnVal;

	returnVal = ((1 << amount) - 1) & newBitBuffl;
	newBitCount -= amount;

	if (newBitCount < 0) {
		newBitCount += amount;
		remBits = (newBitBuffh << (16 - newBitCount));
		newBitBuffh >>= newBitCount;
		newBitBuffl >>= newBitCount;
		newBitBuffl |= remBits;
		_srcPtr += 2;

		// added some more check here to prevent reading in the buffer
		// if there are no bytes anymore.
		_inputByteLeft -= 2;
		if (_inputByteLeft <= 0)
			newBitBuffh = 0;
		else if (_inputByteLeft == 1)
			newBitBuffh = *_srcPtr;
		else
			newBitBuffh = READ_LE_UINT16(_srcPtr);
		amount -= newBitCount;
		newBitCount = 16 - amount;
	}
	remBits = (newBitBuffh << (16 - amount));
	_bitBuffh = newBitBuffh >> amount;
	_bitBuffl = (newBitBuffl >> amount) | remBits;
	_bitCount = (uint8)newBitCount;

	return returnVal;
}

void RncDecoder::makeHufftable(uint16 *table) {
	debugC(1, kDebugTools, "makeHufftable(table)");

	uint16 bitLength, i, j;
	uint16 numCodes = inputBits(5);

	if (!numCodes)
		return;

	uint8 huffLength[16];
	for (i = 0; i < numCodes; i++)
		huffLength[i] = (uint8)(inputBits(4) & 0x00FF);

	uint16 huffCode = 0;

	for (bitLength = 1; bitLength < 17; bitLength++) {
		for (i = 0; i < numCodes; i++) {
			if (huffLength[i] == bitLength) {
				*table++ = (1 << bitLength) - 1;

				uint16 b = huffCode >> (16 - bitLength);
				uint16 a = 0;

				for (j = 0; j < bitLength; j++)
					a |= ((b >> j) & 1) << (bitLength - j - 1);
				*table++ = a;

				*(table + 0x1e) = (huffLength[i] << 8) | (i & 0x00FF);
				huffCode += 1 << (16 - bitLength);
			}
		}
	}
}

uint16 RncDecoder::inputValue(uint16 *table) {
	debugC(5, kDebugTools, "inputValue(table)");

	uint16 valOne, valTwo, value = _bitBuffl;

	do {
		valTwo = (*table++) & value;
		valOne = *table++;
	} while (valOne != valTwo);

	value = *(table + 0x1e);
	inputBits((uint8)((value >> 8) & 0x00FF));
	value &= 0x00FF;

	if (value >= 2) {
		value--;
		valOne = inputBits((uint8)value & 0x00FF);
		valOne |= (1 << value);
		value = valOne;
	}

	return value;
}

int RncDecoder::getbit() {
	debugC(6, kDebugTools, "getbits()");

	if (_bitCount == 0) {
		_bitBuffl = *_srcPtr++;
		_bitCount = 8;
	}
	byte temp = (_bitBuffl & 0x80) >> 7;
	_bitBuffl <<= 1;
	_bitCount--;
	return temp;
}

int32 RncDecoder::unpackM1(const void *input, uint16 inputSize, void *output) {
	debugC(1, kDebugTools, "unpackM1(input, output)");

	uint8 *outputLow, *outputHigh;
	const uint8 *inputHigh, *inputptr = (const uint8 *)input;

	uint32 unpackLen = 0;
	uint32 packLen = 0;
	uint16 counts = 0;
	uint16 crcUnpacked = 0;
	uint16 crcPacked = 0;


	_inputByteLeft = inputSize;
	_bitBuffl = 0;
	_bitBuffh = 0;
	_bitCount = 0;

	//Check for "RNC "
	if (READ_BE_UINT32(inputptr) != RNC1_SIGNATURE)
		return NOT_PACKED;

	inputptr += 4;

	// read unpacked/packed file length
	unpackLen = READ_BE_UINT32(inputptr);
	inputptr += 4;
	packLen = READ_BE_UINT32(inputptr);
	inputptr += 4;

	uint8 blocks = *(inputptr + 5);

	//read CRC's
	crcUnpacked = READ_BE_UINT16(inputptr);
	inputptr += 2;
	crcPacked = READ_BE_UINT16(inputptr);
	inputptr += 2;
	inputptr = (inputptr + HEADER_LEN - 16);

	if (crcBlock(inputptr, packLen) != crcPacked)
		return PACKED_CRC;

	inputptr = (((const uint8 *)input) + HEADER_LEN);
	_srcPtr = inputptr;

	inputHigh = ((const uint8 *)input) + packLen + HEADER_LEN;
	outputLow = (uint8 *)output;
	outputHigh = *(((const uint8 *)input) + 16) + unpackLen + outputLow;

	if (!((inputHigh <= outputLow) || (outputHigh <= inputHigh))) {
		_srcPtr = inputHigh;
		_dstPtr = outputHigh;
		memcpy((_dstPtr - packLen), (_srcPtr - packLen), packLen);
		_srcPtr = (_dstPtr - packLen);
	}

	_inputByteLeft -= HEADER_LEN;

	_dstPtr = (uint8 *)output;
	_bitCount = 0;


	_bitBuffl = READ_LE_UINT16(_srcPtr);
	inputBits(2);

	do {
		makeHufftable(_rawTable);
		makeHufftable(_posTable);
		makeHufftable(_lenTable);

		counts = inputBits(16);

		do {
			uint32 inputLength = inputValue(_rawTable);
			uint32 inputOffset;

			if (inputLength) {
				memcpy(_dstPtr, _srcPtr, inputLength); //memcpy is allowed here
				_dstPtr += inputLength;
				_srcPtr += inputLength;
				_inputByteLeft -= inputLength;
				uint16 a;
				if (_inputByteLeft <= 0)
					a = 0;
				else if (_inputByteLeft == 1)
					a = *_srcPtr;
				else
					a = READ_LE_UINT16(_srcPtr);

				uint16 b;
				if (_inputByteLeft <= 2)
					b = 0;
				else if (_inputByteLeft == 3)
					b = *(_srcPtr + 2);
				else
					b = READ_LE_UINT16(_srcPtr + 2);

				_bitBuffl &= ((1 << _bitCount) - 1);
				_bitBuffl |= (a << _bitCount);
				_bitBuffh = (a >> (16 - _bitCount)) | (b << _bitCount);
			}

			if (counts > 1) {
				inputOffset = inputValue(_posTable) + 1;
				inputLength = inputValue(_lenTable) + MIN_LENGTH;

				// Don't use memcpy here! because input and output overlap.
				uint8 *tmpPtr = (_dstPtr - inputOffset);
				while (inputLength--)
					*_dstPtr++ = *tmpPtr++;
			}
		} while (--counts);
	} while (--blocks);

	if (crcBlock((uint8 *)output, unpackLen) != crcUnpacked)
		return UNPACKED_CRC;

	// all is done..return the amount of unpacked bytes
	return unpackLen;
}

int32 RncDecoder::unpackM2(const void *input, void *output) {
	debugC(1, kDebugTools, "unpackM2(input, output)");

	const uint8 *inputptr = (const uint8 *)input;

	uint32 unpackLen = 0;
	uint32 packLen = 0;
	uint16 crcUnpacked = 0;
	uint16 crcPacked = 0;

	_bitBuffl = 0;
	_bitCount = 0;

	//Check for "RNC "
	if (READ_BE_UINT32(inputptr) != RNC2_SIGNATURE)
		return NOT_PACKED;

	inputptr += 4;

	// read unpacked/packed file length
	unpackLen = READ_BE_UINT32(inputptr);
	inputptr += 4;
	packLen = READ_BE_UINT32(inputptr);
	inputptr += 4;

	//read CRC's
	crcUnpacked = READ_BE_UINT16(inputptr);
	inputptr += 2;
	crcPacked = READ_BE_UINT16(inputptr);
	inputptr += 2;
	inputptr = (inputptr + HEADER_LEN - 16);

	if (crcBlock(inputptr, packLen) != crcPacked)
		return PACKED_CRC;

	inputptr = (((const uint8 *)input) + HEADER_LEN);
	_srcPtr = inputptr;
	_dstPtr = (uint8 *)output;

	uint16 ofs, len;
	byte ofs_hi, ofs_lo;

	len = 0;
	ofs_hi = 0;
	ofs_lo = 0;

	getbit();
	getbit();

	while (1) {

		bool loadVal = false;

		while (getbit() == 0)
			*_dstPtr++ = *_srcPtr++;

		len = 2;
		ofs_hi = 0;
		if (getbit() == 0) {
			len = (len << 1) | getbit();
			if (getbit() == 1) {
				len--;
				len = (len << 1) | getbit();
				if (len == 9) {
					len = 4;
					while (len--)
						ofs_hi = (ofs_hi << 1) | getbit();
					len = (ofs_hi + 3) * 4;
					while (len--)
						*_dstPtr++ = *_srcPtr++;
					continue;
				}
			}
			loadVal = true;
		} else {
			if (getbit() == 1) {
				len++;
				if (getbit() == 1) {
					len = *_srcPtr++;
					if (len == 0) {
						if (getbit() == 1)
							continue;
						else
							break;
					}
					len += 8;
				}
				loadVal = true;
			} else {
				loadVal = false;
			}
		}

		if (loadVal) {
			if (getbit() == 1) {
				ofs_hi = (ofs_hi << 1) | getbit();
				if (getbit() == 1) {
					ofs_hi = ((ofs_hi << 1) | getbit()) | 4;
					if (getbit() == 0)
						ofs_hi = (ofs_hi << 1) | getbit();
				} else if (ofs_hi == 0) {
					ofs_hi = 2 | getbit();
				}
			}
		}

		ofs_lo = *_srcPtr++;
		ofs = (ofs_hi << 8) | ofs_lo;
		while (len--) {
			*_dstPtr = *(byte *)(_dstPtr - ofs - 1);
			_dstPtr++;
		}

	}

	if (crcBlock((uint8 *)output, unpackLen) != crcUnpacked)
		return UNPACKED_CRC;

	// all is done..return the amount of unpacked bytes
	return unpackLen;
}

} // End of namespace Toon
