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

/***************************************************************************
** decomp.c
**
** Routines that deal with AGI version 3 specific features.
** The original LZW code is from DJJ, October 1989, p.86.
** It has been modified to handle AGI compression.
**
** (c) 1997  Lance Ewing
***************************************************************************/

#include "agi/agi.h"
#include "agi/lzw.h"

#include "common/textconsole.h"

namespace Agi {


class LZWDecoder {
private:

	enum {
		MAXBITS		= 12,
		TABLE_SIZE	= 18041,	// strange number
		START_BITS	= 9
	};

	int32 BITS, MAX_VALUE, MAX_CODE;
	uint32 *prefixCode;
	uint8 *appendCharacter;
	uint8 *decodeStack;
	int32 inputBitCount;	// Number of bits in input bit buffer
	uint32 inputBitBuffer;

public:
	LZWDecoder();
	~LZWDecoder();

	void lzwExpand(uint8 *in, uint8 *out, int32 len);

private:
	int setBits(int32 value);
	uint8 *decodeString(uint8 *buffer, uint32 code);
	uint32 inputCode(uint8 **input);
};

LZWDecoder::LZWDecoder() {
	decodeStack = (uint8 *)calloc(1, 8192);
	prefixCode = (uint32 *)malloc(TABLE_SIZE * sizeof(uint32));
	appendCharacter = (uint8 *)malloc(TABLE_SIZE * sizeof(uint8));
	inputBitCount = 0;	// Number of bits in input bit buffer
	inputBitBuffer = 0L;
}

LZWDecoder::~LZWDecoder() {
	free(decodeStack);
	free(prefixCode);
	free(appendCharacter);
}

/**
 * Adjust the number of bits used to store codes to the value passed in.
 */
int LZWDecoder::setBits(int32 value) {
	if (value == MAXBITS)
		return true;

	BITS = value;
	MAX_VALUE = (1 << BITS) - 1;
	MAX_CODE = MAX_VALUE - 1;

	return false;
}

/**
 * Return the string that the code taken from the input buffer
 * represents. The string is returned as a stack, i.e. the characters are
 * in reverse order.
 */
uint8 *LZWDecoder::decodeString(uint8 *buffer, uint32 code) {
	uint32 i;

	for (i = 0; code > 255;) {
		*buffer++ = appendCharacter[code];
		code = prefixCode[code];
		if (i++ >= 4000) {
			error("lzw: error in code expansion");
		}
	}
	*buffer = code;

	return buffer;
}

/**
 * Return the next code from the input buffer.
 */
uint32 LZWDecoder::inputCode(uint8 **input) {
	uint32 r;

	while (inputBitCount <= 24) {
		inputBitBuffer |= (uint32) * (*input)++ << inputBitCount;
		inputBitCount += 8;
	}
	r = (inputBitBuffer & 0x7FFF) % (1 << BITS);
	inputBitBuffer >>= BITS;
	inputBitCount -= BITS;

	return r;
}

/**
 * Uncompress the data contained in the input buffer and store
 * the result in the output buffer. The fileLength parameter says how
 * many bytes to uncompress. The compression itself is a form of LZW that
 * adjusts the number of bits that it represents its codes in as it fills
 * up the available codes. Two codes have special meaning:
 *
 *  code 256 = start over
 *  code 257 = end of data
 */
void LZWDecoder::lzwExpand(uint8 *in, uint8 *out, int32 len) {
	int32 c, lzwnext, lzwnew, lzwold;
	uint8 *s, *end;

	LZWDecoder d;

	setBits(START_BITS);	// Starts at 9-bits
	lzwnext = 257;		// Next available code to define

	end = (uint8 *)(out + (uint32)len);

	lzwold = inputCode(&in);	// Read in the first code
	c = lzwold;
	lzwnew = inputCode(&in);

	while ((out < end) && (lzwnew != 0x101)) {
		if (lzwnew == 0x100) {
			// Code to "start over"
			lzwnext = 258;
			setBits(START_BITS);
			lzwold = inputCode(&in);
			c = lzwold;
			*out++ = (char)c;
			lzwnew = inputCode(&in);
		} else {
			if (lzwnew >= lzwnext) {
				// Handles special LZW scenario
				*decodeStack = c;
				s = decodeString(decodeStack + 1, lzwold);
			} else
				s = decodeString(decodeStack, lzwnew);

			// Reverse order of decoded string and
			// store in out buffer
			c = *s;
			while (s >= decodeStack)
				*out++ = *s--;

			if (lzwnext > MAX_CODE)
				setBits(BITS + 1);

			prefixCode[lzwnext] = lzwold;
			appendCharacter[lzwnext] = c;
			lzwnext++;
			lzwold = lzwnew;

			lzwnew = inputCode(&in);
		}
	}
}

void lzwExpand(uint8 *in, uint8 *out, int32 len) {
	LZWDecoder d;
	d.lzwExpand(in, out, len);
}

} // End of namespace Agi
