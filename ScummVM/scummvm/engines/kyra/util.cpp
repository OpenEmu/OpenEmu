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

#include "kyra/util.h"

namespace Kyra {

int Util::decodeString1(const char *src, char *dst) {
	static const uint8 decodeTable1[] = {
		0x20, 0x65, 0x74, 0x61, 0x69, 0x6E, 0x6F, 0x73, 0x72, 0x6C, 0x68,
		0x63, 0x64, 0x75, 0x70, 0x6D
	};

	static const uint8 decodeTable2[] = {
		0x74, 0x61, 0x73, 0x69, 0x6F, 0x20, 0x77, 0x62, 0x20, 0x72, 0x6E,
		0x73, 0x64, 0x61, 0x6C, 0x6D, 0x68, 0x20, 0x69, 0x65, 0x6F, 0x72,
		0x61, 0x73, 0x6E, 0x72, 0x74, 0x6C, 0x63, 0x20, 0x73, 0x79, 0x6E,
		0x73, 0x74, 0x63, 0x6C, 0x6F, 0x65, 0x72, 0x20, 0x64, 0x74, 0x67,
		0x65, 0x73, 0x69, 0x6F, 0x6E, 0x72, 0x20, 0x75, 0x66, 0x6D, 0x73,
		0x77, 0x20, 0x74, 0x65, 0x70, 0x2E, 0x69, 0x63, 0x61, 0x65, 0x20,
		0x6F, 0x69, 0x61, 0x64, 0x75, 0x72, 0x20, 0x6C, 0x61, 0x65, 0x69,
		0x79, 0x6F, 0x64, 0x65, 0x69, 0x61, 0x20, 0x6F, 0x74, 0x72, 0x75,
		0x65, 0x74, 0x6F, 0x61, 0x6B, 0x68, 0x6C, 0x72, 0x20, 0x65, 0x69,
		0x75, 0x2C, 0x2E, 0x6F, 0x61, 0x6E, 0x73, 0x72, 0x63, 0x74, 0x6C,
		0x61, 0x69, 0x6C, 0x65, 0x6F, 0x69, 0x72, 0x61, 0x74, 0x70, 0x65,
		0x61, 0x6F, 0x69, 0x70, 0x20, 0x62, 0x6D
	};

	int size = 0;
	uint cChar = 0;
	while ((cChar = *src++) != 0) {
		if (cChar & 0x80) {
			cChar &= 0x7F;
			int index = (cChar & 0x78) >> 3;
			*dst++ = decodeTable1[index];
			++size;
			assert(cChar < sizeof(decodeTable2));
			cChar = decodeTable2[cChar];
		}

		*dst++ = cChar;
		++size;
	}

	*dst++ = 0;
	return size;
}

void Util::decodeString2(const char *src, char *dst) {
	if (!src || !dst)
		return;

	char out = 0;
	while ((out = *src) != 0) {
		if (*src == 0x1B) {
			++src;
			out = *src + 0x7F;
		}
		*dst++ = out;
		++src;
	}

	*dst = 0;
}

void Util::convertDOSToISO(char *str) {
	uint8 *s = (uint8 *)str;

	for (; *s; ++s) {
		if (*s >= 128) {
			uint8 c = _charMapDOSToISO[*s - 128];

			if (!c)
				c = 0x20;

			*s = c;
		}
	}
}

void Util::convertISOToDOS(char *str) {
	while (*str)
		convertISOToDOS(*str++);
}

void Util::convertISOToDOS(char &c) {
	uint8 code = (uint8)c;
	if (code >= 128) {
		code = _charMapISOToDOS[code - 128];
		if (!code)
			code = 0x20;
	}

	c = code;
}

// CP850 to ISO-8859-1 (borrowed from engines/saga/font_map.cpp)
const uint8 Util::_charMapDOSToISO[128] = {
	199, 252, 233, 226, 228, 224, 229, 231, 234, 235, 232,
	239, 238, 236, 196, 197, 201, 230, 198, 244, 246, 242,
	251, 249, 255, 214, 220, 248, 163, 216, 215,   0, 225,
	237, 243, 250, 241, 209, 170, 186, 191, 174, 172, 189,
	188, 161, 171, 187,   0,   0,   0,   0,   0, 193, 194,
	192, 169,   0,   0,   0,   0, 162, 165,   0,   0,   0,
	  0,   0,   0,   0, 227, 195,   0,   0,   0,   0,   0,
	  0,   0, 164, 240, 208, 202, 203, 200,   0, 205, 206,
	207,   0,   0,   0,   0, 166, 204,   0, 211, 223, 212,
	210, 245, 213, 181, 254, 222, 218, 219, 217, 253, 221,
	175, 180, 173, 177,   0, 190, 182, 167, 247, 184, 176,
	168, 183, 185, 179, 178,   0, 160
};

// ISO-8859-1 to CP850
const uint8 Util::_charMapISOToDOS[128] = {
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 255,
	173, 189, 156, 207, 190, 221, 245, 249, 184, 166, 174,
	170, 240, 169, 238, 248, 241, 253, 252, 239, 230, 244,
	250, 247, 251, 167, 175, 172, 171, 243, 168, 183, 181,
	182, 199, 142, 143, 146, 128, 212, 144, 210, 211, 222,
	214, 215, 216, 209, 165, 227, 224, 226, 229, 153, 158,
	157, 235, 233, 234, 154, 237, 232, 225, 133, 160, 131,
	198, 132, 134, 145, 135, 138, 130, 136, 137, 141, 161,
	140, 139, 208, 164, 149, 162, 147, 228, 148, 246, 155,
	151, 163, 150, 129, 236, 231, 152
};

} // End of namespace Kyra
