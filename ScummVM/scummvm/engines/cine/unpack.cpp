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

#include "cine/unpack.h"

namespace Cine {

uint32 CineUnpacker::readSource() {
	if (_src < _srcBegin || _src + 4 > _srcEnd) {
		_error = true;
		return 0; // The source pointer is out of bounds, returning a default value
	}
	uint32 value = READ_BE_UINT32(_src);
	_src -= 4;
	return value;
}

uint CineUnpacker::rcr(bool inputCarry) {
	uint outputCarry = (_chunk32b & 1);
	_chunk32b >>= 1;
	if (inputCarry) {
		_chunk32b |= 0x80000000;
	}
	return outputCarry;
}

uint CineUnpacker::nextBit() {
	uint carry = rcr(false);
	// Normally if the chunk becomes zero then the carry is one as
	// the end of chunk marker is always the last to be shifted out.
	if (_chunk32b == 0) {
		_chunk32b = readSource();
		_crc ^= _chunk32b;
		carry = rcr(true); // Put the end of chunk marker in the most significant bit
	}
	return carry;
}

uint CineUnpacker::getBits(uint numBits) {
	uint c = 0;
	while (numBits--) {
		c <<= 1;
		c |= nextBit();
	}
	return c;
}

void CineUnpacker::unpackRawBytes(uint numBytes) {
	if (_dst >= _dstEnd || _dst - numBytes + 1 < _dstBegin) {
		_error = true;
		return; // Destination pointer is out of bounds for this operation
	}
	while (numBytes--) {
		*_dst = (byte)getBits(8);
		--_dst;
	}
}

void CineUnpacker::copyRelocatedBytes(uint offset, uint numBytes) {
	if (_dst + offset >= _dstEnd || _dst - numBytes + 1 < _dstBegin) {
		_error = true;
		return; // Destination pointer is out of bounds for this operation
	}
	while (numBytes--) {
		*_dst = *(_dst + offset);
		--_dst;
	}
}

bool CineUnpacker::unpack(const byte *src, uint srcLen, byte *dst, uint dstLen) {
	// Initialize variables used for detecting errors during unpacking
	_error    = false;
	_srcBegin = src;
	_srcEnd   = src + srcLen;
	_dstBegin = dst;
	_dstEnd   = dst + dstLen;

	// Handle already unpacked data here
	if (srcLen == dstLen) {
		// Source length is same as destination length so the source
		// data is already unpacked. Let's just copy it then.
		memcpy(dst, src, srcLen);
		return true;
	}

	// Initialize other variables
	_src = _srcBegin + srcLen - 4;
	uint32 unpackedLength = readSource(); // Unpacked length in bytes
	_dst = _dstBegin + unpackedLength - 1;
	_crc = readSource();
	_chunk32b = readSource();
	_crc ^= _chunk32b;

	while (_dst >= _dstBegin && !_error) {
		/*
		Bits  => Action:
		0 0   => unpackRawBytes(3 bits + 1)              i.e. unpackRawBytes(1..8)
		1 1 1 => unpackRawBytes(8 bits + 9)              i.e. unpackRawBytes(9..264)
		0 1   => copyRelocatedBytes(8 bits, 2)           i.e. copyRelocatedBytes(0..255, 2)
		1 0 0 => copyRelocatedBytes(9 bits, 3)           i.e. copyRelocatedBytes(0..511, 3)
		1 0 1 => copyRelocatedBytes(10 bits, 4)          i.e. copyRelocatedBytes(0..1023, 4)
		1 1 0 => copyRelocatedBytes(12 bits, 8 bits + 1) i.e. copyRelocatedBytes(0..4095, 1..256)
		*/
		if (!nextBit()) { // 0...
			if (!nextBit()) { // 0 0
				uint numBytes = getBits(3) + 1;
				unpackRawBytes(numBytes);
			} else { // 0 1
				uint numBytes = 2;
				uint offset   = getBits(8);
				copyRelocatedBytes(offset, numBytes);
			}
		} else { // 1...
			uint c = getBits(2);
			if (c == 3) { // 1 1 1
				uint numBytes = getBits(8) + 9;
				unpackRawBytes(numBytes);
			} else if (c < 2) { // 1 0 x
				uint numBytes = c + 3;
				uint offset   = getBits(c + 9);
				copyRelocatedBytes(offset, numBytes);
			} else { // 1 1 0
				uint numBytes = getBits(8) + 1;
				uint offset   = getBits(12);
				copyRelocatedBytes(offset, numBytes);
			}
		}
	}
	return !_error && (_crc == 0);
}

} // End of namespace Cine
