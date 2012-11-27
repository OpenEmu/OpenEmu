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

#include "agos/agos.h"
#include "agos/intern.h"

#include "common/textconsole.h"

namespace AGOS {

uint32 AGOSEngine_PN::ftext(uint32 base, int n) {
	uint32 b = base;
	int ct = n;
	while (ct) {
		while (_textBase[b++])
			;
		ct--;
	}
	return b;
}

char *AGOSEngine_PN::unctok(char *c, int n) {
	int x;
	uint8 *tokbase;
	tokbase = _textBase + getlong(30);
	x = n;
	while (x-=(*tokbase++ > 127))
		;
	while (*tokbase < 128)
		*c++=*tokbase++;
	*c++ = *tokbase & 127;
	*c = 0;
	return c;
}

void AGOSEngine_PN::uncomstr(char *c, uint32 x) {
	if (x > _textBaseSize)
		error("UNCOMSTR: TBASE over-run");
	while (_textBase[x]) {
		if (_textBase[x] < 244) {
			c = unctok(c, _textBase[x]);
		} else {
			c = unctok(c, (_textBase[x] - 244) * 254 + _textBase[x + 1] - 1);
			x++;
		}
		x++;
	}
	*c++ = 13;
	*c = 0;
}

static const char *const objectNames[30] = {
	"\0",
	"Take \0",
	"Inventory\r",
	"Open \0",
	"Close \0",
	"Lock \0",
	"Unlock \0",
	"Examine \0",
	"Look in \0",
	"Exits \r",
	"Look\r",
	"Wait\r",
	"Pause\r",
	"\0",
	"Save\r",
	"Restore\r",
	"\0",
	"N\r",
	"NE\r",
	"E\r",
	"SE\r",
	"S\r",
	"SW\r",
	"W\r",
	"NW\r",
	"INVENTORY\r",
	"ROOM DESCRIPTION\r",
	"x\r",
	"MORE\r",
	"CLOSE\r"
};

void AGOSEngine_PN::getObjectName(char *v, uint16 x) {
	if (x & 0x8000) {
		x &= ~0x8000;
		if (x > getptr(51))
			error("getObjectName: Object %d out of range", x);
		uncomstr(v, ftext(getlong(27), x * _dataBase[47]));
	} else {
		assert(x < 30);
		strcpy(v, objectNames[x]);
	}
}

void AGOSEngine_PN::pcl(const char *s) {
	strcat(_sb, s);
	if (strchr(s, '\n') == 0) {
		for (char *str = _sb; *str; str++)
			windowPutChar(_windowArray[_curWindow], *str);
		strcpy(_sb, "");
	}
}

void AGOSEngine_PN::pcf(uint8 ch) {
	int ct = 0;
	if (ch == '[')
		ch = '\n';
	if (ch == 0)
		return;	/* Trap any C EOS chrs */
	if (ch == 255) {
		_bp = 0;
		_xofs = 0;
		return;		/* pcf(255) initializes the routine */
	}			/* pcf(254) flushes its working _buffer */
	if (ch != 254) {
		if ((ch != 32) || (_bp + _xofs != 50))
			_buffer[_bp++] = ch;
	}
	if ((ch != 254) && (!Common::isSpace(ch)) && (_bp < 60))
		return;
	/* We know have a case of needing to print the text */
	if (_bp + _xofs > 50) {
		pcl("\n");
		if (_buffer[0] == ' ')
			ct = 1;	/* Skip initial space */
		/* Note '  ' will give a single start of line space */
		_xofs = 0;
	}
	_buffer[_bp] = 0;
	pcl(_buffer + ct);
	_xofs += _bp;
	_bp = 0;
	if (ch == '\n')
		_xofs = 0;	/* At Newline! */
}

void AGOSEngine_PN::patok(int n) {
	int x;
	uint8 *tokbase;
	tokbase = _textBase + getlong(30);
	x = n;
	while (x -= (*tokbase++ > 127))
		;
	while (*tokbase < 128)
		pcf(*tokbase++);
	pcf((uint8)(*tokbase & 127));
}

void AGOSEngine_PN::pmesd(int n) {
	ptext(ftext(getlong(24), n));
}

void AGOSEngine_PN::plocd(int n, int m) {
	if (n > getptr(53))
		error("Location out of range");
	ptext(ftext(getlong(21), n * _dataBase[48] + m));
}

void AGOSEngine_PN::pobjd(int n, int m) {
	if (n > getptr(51))
		error("Object out of range");
	ptext(ftext(getlong(27), n * _dataBase[47] + m));
}

void AGOSEngine_PN::ptext(uint32 tptr) {
	if (tptr > _textBaseSize)
		error("ptext: attempt to print beyond end of TBASE");

	while (_textBase[tptr]) {
		if (_textBase[tptr] < 244) {
			patok(_textBase[tptr++]);
		} else {
			patok((_textBase[tptr] - 244) * 254 + _textBase[tptr + 1] - 1);
			tptr += 2;
		}
	}
}

const uint8 characters[11][80] = {
// PETERMASON
	{
		118, 225,
		 91, 118,
		 94, 124,
		236, 161,
		241, 166,
		168,   4,
		138,  46,
		139,  46,
		249,  50,
		 38,  56,
		 80,  59,
		149,  69,
		 37,  77,
		 93,  93,
		 86,  95,
		  0,
		  0,
		 58, 130,
		 62, 178,
		 83,  95,
		  0,
		121,  58,
		122,  59,
		126,  60,
		124,  61,
		240,  62,
		123,  63,
		0
	},
// JBLANDFORD
	{
		0,
		0,
		0,
		0
	},
// SBLANDFORD
	{
		120, 223,
		 94, 126,
		112, 134,
		 45, 152,
		241, 166,
		168,   3,
		150,  26,
		220,  29,
		138,  42,
		139,  47,
		249,  50,
		 38,  56,
		230,  64,
		 37,  77,
		 93,  94,
		 86,  96,
		  0,
		  0,
		 58, 129,
		 59, 112,
		 83,  96,
		 81, 106,
		 62, 169,
		  0,
		121,  54,
		122,  55,
		119,  56,
		118,  57,
		  0
	},
// MRJONES
	{
		121, 218,
		 91, 118,
		253, 121,
		154, 138,
		235, 173,
		236, 161,
		241, 165,
		168,   0,
		150,  21,
		 36,  33,
		138,  42,
		249,  50,
		 80,  60,
		  4,  60,
		 37,  78,
		 68,  33,
		 93,  92,
		101, 109,
		  0,
		 36,  35,
		 68,  90,
		  0,
		 58, 128,
		 59, 111,
		 62, 182,
		  0,
		122,  13,
		126,  14,
		124,  15,
		240,  16,
		120,  17,
		119,  18,
		118,  19,
		 52,  20,
		125,  21,
		127,  22,
		123,  23,
		117,  24,
		  0
	},
// MRSJONES
	{
		122, 219,
		 91, 119,
		253, 123,
		112, 136,
		154, 137,
		 95, 142,
		 45, 152,
		109, 155,
		235, 160,
		168,   1,
		151,  13,
		145,  15,
		150,  22,
		220,  28,
		 36,  33,
		138,  43,
		 13,  51,
		 80,  59,
		230,  64,
		149,  69,
		 86, 100,
		  0,
		 36,  36,
		  0,
		 58, 127,
		 62, 179,
		 83, 100,
		 81, 102,
		  0,
		121,  25,
		126,  26,
		124,  27,
		120,  28,
		119,  29,
		118,  30,
		 52,  31,
		125,  32,
		127,  33,
		123,  34,
		117,  35,
		0
	},
// MRROBERTS
	{
		123, 229,
		 91, 117,
		253, 120,
		 94, 125,
		112, 134,
		109, 156,
		235, 172,
		236, 162,
		241, 165,
		168,   3,
		 36,  33,
		249,  50,
		 38,  56,
		 80,  58,
		 37,  75,
		 34,  81,
		 68,  33,
		101, 109,
		  0,
		 36,  40,
		 68,  88,
		  0,
		 59, 111,
		 62, 181,
		  0,
		  0
	},
// POSTMISTRESS
	{
		124, 221,
		 91, 119,
		253, 122,
		112, 136,
		 95, 142,
		130, 149,
		109, 155,
		235, 176,
		220,  29,
		 36,  33,
		138,  43,
		 13,  51,
		 80,  57,
		149,  68,
		 37,  73,
		 34,  33,
		 68,  33,
		 86, 100,
		  0,
		 36,  39,
		 34,  80,
		 68,  86,
		  0,
		 58, 130,
		 62, 181,
		 83, 100,
		 81, 103,
		  0,
		121,  41,
		122,  42,
		126,  43,
		240,  44,
		120,  45,
		119,  46,
		118,  47,
		 52,  48,
		123,  49,
		 83,  50,
		117,  51,
		  0
	},
// MWILLIAMS
	{
		125, 227,
		 94, 124,
		 95, 141,
		241, 166,
		168,   4,
		150,  26,
		 38,  54,
		  4,  60,
		230,  65,
		149,  68,
		 37,  76,
		101, 109,
		  0,
		230,  63,
		  0,
		 59, 112,
		 62, 183,
		  0,
		240,  71,
		120,  72,
		118,  73,
		 52,  74,
		117,  75,
		  0
	},
// TONY
	{
		126, 220,
		 95, 143,
		130, 149,
		 45, 153,
		109, 154,
		235, 158,
		241, 166,
		168,   2,
		145,  15,
		150,  24,
		220,  20,
		 36,  20,
		  4,  60,
		 37,  79,
		 86,  97,
		  0,
		150,  23,
		220,  27,
		 36,  34,
		  0,
		 83,  97,
		  0,
		121,  36,
		122,  37,
		124,  38,
		240,  39,
		 52,  40,
		  0
	},
// PIG
	{
		127, 228,
		112, 133,
		 45, 153,
		235, 157,
		236, 163,
		241, 165,
		 36,  33,
		 80,  58,
		 34,  81,
		 68,  33,
		 86,  98,
		  0,
		 36,  37,
		 68,  90,
		  0,
		 62, 184,
		 83,  98,
		  0,
		121,  76,
		122,  77,
		126,  78,
		124,  79,
		240,  80,
		120,  81,
		118,  82,
		 52,  83,
		125,  84,
		123,  85,
		 83,  86,
		117,  87,
		  0
	},
// JUDY
	{
			  0,
			  0,
			  0,
			240, 52,
			117, 53,
			  0
	}
};

void AGOSEngine_PN::getResponse(uint16 charNum, uint16 objNum, uint16 &msgNum1, uint16 &msgNum2) {
	const uint8 *ptr;
	uint16 num;

	msgNum1 = 0;
	msgNum2 = 0;

	if (charNum == 83)
		charNum += 45;

	if (charNum < 118 || charNum > 128) {
		return;
	}

	ptr = characters[charNum - 118];

	while ((num = *ptr++) != 0) {
		if (num == objNum) {
			msgNum1 = *ptr++;
			msgNum1 += 400;

			while ((num = *ptr++) != 0)
				ptr++;
			break;
		}
		ptr++;
	}

	while ((num = *ptr++) != 0) {
		if (num == objNum) {
			msgNum2 = *ptr++;
			msgNum2 += 400;

			if (msgNum1 == 569)
				msgNum1 += 400;
			if (msgNum2 == 0)
				msgNum2 = msgNum1;
			return;
		}
		ptr++;
	}

	if (objNum >= 200)
		msgNum1 = 0;

	objNum -= 200;
	while ((num = *ptr++) != 0) {
		if (num == objNum) {
			msgNum1 = *ptr++;
			msgNum1 += 400;

			if (msgNum1 == 569)
				msgNum1 += 400;
			if (msgNum2 == 0)
				msgNum2 = msgNum1;
			return;
		}
		ptr++;
	}

	objNum += 200;
	while ((num = *ptr++) != 0) {
		if (num == objNum) {
			msgNum1 = *ptr++;
			msgNum1 += 200;

			if (msgNum1 == 569)
				msgNum1 += 400;
			if (msgNum2 == 0)
				msgNum2 = msgNum1;
			return;
		}
		ptr++;
	}

	if (msgNum1 == 569)
		msgNum1 += 400;
	if (msgNum2 == 0)
		msgNum2 = msgNum1;
}

char *AGOSEngine_PN::getMessage(char *msg, uint16 num) {
	char *origPtr, *strPtr1 = msg;
	uint8 count;

	getObjectName(strPtr1, num);
	if (!(num & 0x8000)) {
		return msg;
	}

	if (strPtr1[0] == 0x41 || strPtr1[0] == 0x61) {
		if (strPtr1[1] != 0x20)
			strPtr1 += 2;
	} else if (strPtr1[0] == 0x54 || strPtr1[0] == 0x74) {
		if (strPtr1[1] == 0x68 &&
		    strPtr1[2] == 0x65 &&
		    strPtr1[3] == 0x20)
			strPtr1 += 4;
	}

	origPtr = strPtr1;
	while (*strPtr1 != 13)
		strPtr1++;

	strPtr1[0] = 32;
	strPtr1[1] = 13;
	strPtr1[2] = 0;

	if (_videoLockOut & 0x10) {
		strPtr1 = origPtr;
		count = 6;
		while (*strPtr1) {
			if (*strPtr1 == 32) {
				count = 6;
			} else {
				count--;
				if (count == 0) {
					char *tmpPtr = strPtr1;
					char *strPtr2 = strPtr1;

					while (*strPtr2 != 0 && *strPtr2 != 32)
						strPtr2++;

					while (*strPtr2) {
						*strPtr1++ = *strPtr2++;
					}
					*strPtr1++ = *strPtr2++;

					strPtr1 = tmpPtr;
					count = 6;
				}
			}
			strPtr1++;
		}
	}

	return origPtr;
}

} // End of namespace AGOS
