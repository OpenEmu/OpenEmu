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



#include "common/file.h"
#include "common/textconsole.h"

#include "gui/about.h"
#include "gui/message.h"

#include "agos/agos.h"
#include "agos/intern.h"

namespace AGOS {

void AGOSEngine::uncompressText(byte *ptr) {
	byte a;
	while (1) {
		if (_awaitTwoByteToken != 0)
			a = _awaitTwoByteToken;
		else
			a = *ptr++;
		if (a == 0)
			return;
		ptr = uncompressToken(a, ptr);
		if (ptr == 0)
			return;
	}
}

byte *AGOSEngine::uncompressToken(byte a, byte *ptr) {
	byte *ptr1 = 0;
	byte *ptr2 = 0;
	byte b;
	int count1 = 0;

	if (a == 0xFF || a == 0xFE || a == 0xFD) {
		if (a == 0xFF)
			ptr2 = _twoByteTokenStrings;
		if (a == 0xFE)
			ptr2 = _secondTwoByteTokenStrings;
		if (a == 0xFD)
			ptr2 = _thirdTwoByteTokenStrings;
		_awaitTwoByteToken = a;
		b = a;
		a = *ptr++;
		if (a == 0)		/* Need to return such that next byte   */
			return 0;	/* is used as two byte token		*/

		_awaitTwoByteToken = 0;
		ptr1 = _twoByteTokens;
		while (*ptr1 != a) {
			ptr1++;
			count1++;
			if (*ptr1 == 0)	{	/* If was not a two byte token  */
				count1 = 0;	/* then was a byte token.	*/
				ptr1 = _byteTokens;
				while (*ptr1 != a) {
					ptr1++;
					count1++;
				}
				ptr1 = _byteTokenStrings;		/* Find it */
				while (count1--)	{
					while (*ptr1++)
						;
				}
				ptr1 = uncompressToken(b, ptr1);	/* Try this one as a two byte token */
				uncompressText(ptr1);			/* Uncompress rest of this token    */
				return ptr;
			}
		}
		while (count1--) {
			while (*ptr2++)
				;
		}
		uncompressText(ptr2);
	} else {
		ptr1 = _byteTokens;
		while (*ptr1 != a) {
			ptr1++;
			count1++;
			if (*ptr1 == 0) {
				_textBuffer[_textCount++] = a;	/* Not a byte token */
				return ptr;			/* must be real character */
			}
		}
		ptr1 = _byteTokenStrings;
		while (count1--)	{		/* Is a byte token so count */
			while (*ptr1++)		/* to start of token */
				;
		}
		uncompressText(ptr1);			/* and do it */
	}
	return ptr;
}

const byte *AGOSEngine::getStringPtrByID(uint16 stringId, bool upperCase) {
	const byte *stringPtr;
	byte *dst;

	_freeStringSlot ^= 1;
	dst = _stringReturnBuffer[_freeStringSlot];

	if (getGameType() == GType_ELVIRA1 && getPlatform() == Common::kPlatformAtariST) {
		byte *ptr = _stringTabPtr[stringId];
		_textCount = 0;
		_awaitTwoByteToken = 0;
		uncompressText(ptr);
		_textBuffer[_textCount] = 0;
		strcpy((char *)dst, (const char *)_textBuffer);
	} else {
		if (stringId < 0x8000) {
			stringPtr = _stringTabPtr[stringId];
		} else {
			stringPtr = getLocalStringByID(stringId);
		}
		strcpy((char *)dst, (const char *)stringPtr);
	}

	// WORKAROUND bug #1538873: The French version of Simon 1 and the
	// Polish version of Simon 2 used excess spaces, at the end of many
	// messages, so we strip off those excess spaces.
	if ((getGameType() == GType_SIMON1 && _language == Common::FR_FRA) ||
		(getGameType() == GType_SIMON2 && _language == Common::PL_POL)) {
		uint16 len = strlen((const char *)dst) - 1;

		while (len && dst[len] == 32) {
			dst[len] = 0;
			len--;
		}

	}

	if (upperCase && *dst) {
		if (Common::isLower(*dst))
			*dst = toupper(*dst);
	}

	return dst;
}

const byte *AGOSEngine::getLocalStringByID(uint16 stringId) {
	if (stringId < _stringIdLocalMin || stringId >= _stringIdLocalMax) {
		loadTextIntoMem(stringId);
	}
	return _localStringtable[stringId - _stringIdLocalMin];
}

TextLocation *AGOSEngine::getTextLocation(uint a) {
	switch (a) {
	case 1:
		return &_textLocation1;
	case 2:
		return &_textLocation2;
	case 101:
		return &_textLocation3;
	case 102:
		return &_textLocation4;
	default:
		error("getTextLocation: Invalid text location %d", a);
	}
	return NULL;	// for compilers that don't support NORETURN
}

void AGOSEngine::allocateStringTable(int num) {
	_stringTabPtr = (byte **)calloc(num, sizeof(byte *));
	_stringTabPos = 0;
	_stringTabSize = num;
}

void AGOSEngine::setupStringTable(byte *mem, int num) {
	int i = 0;

	if (getGameType() == GType_ELVIRA1 && getPlatform() == Common::kPlatformAtariST) {
		int ct1;

		_twoByteTokens = mem;
		while (*mem++) {
			i++;
		}
		_twoByteTokenStrings = mem;
		ct1 = i;
		while (*mem++) {
			while (*mem++)
				;
			i--;
			if ((i == 0) && (ct1 != 0)) {
				_secondTwoByteTokenStrings = mem;
				i = ct1;
				ct1 = 0;
			}
			if (i == 0)
				_thirdTwoByteTokenStrings = mem;
		}
		_byteTokens = mem;
		while (*mem++)
			;
		_byteTokenStrings = mem;
		while (*mem++) {
			while (*mem++)
				;
		}
		i = 0;
l1:		_stringTabPtr[i++] = mem;
		num--;
		if (!num) {
			_stringTabPos = i;
			return;
		}
		while (*mem++)
			;
		goto l1;
	} else {
		for (;;) {
			_stringTabPtr[i++] = mem;
			if (--num == 0)
				break;
			for (; *mem; mem++)
				;
			mem++;
		}

		_stringTabPos = i;
	}
}

void AGOSEngine::setupLocalStringTable(byte *mem, int num) {
	int i = 0;
	for (;;) {
		_localStringtable[i++] = mem;
		if (--num == 0)
			break;
		for (; *mem; mem++)
			;
		mem++;
	}
}

uint AGOSEngine::loadTextFile(const char *filename, byte *dst) {
	if (getFeatures() & GF_OLD_BUNDLE)
		return loadTextFile_simon1(filename, dst);
	else
		return loadTextFile_gme(filename, dst);
}

uint AGOSEngine::loadTextFile_simon1(const char *filename, byte *dst) {
	Common::File fo;
	fo.open(filename);
	uint32 size;

	if (fo.isOpen() == false)
		error("loadTextFile: Can't open '%s'", filename);

	size = fo.size();

	if (fo.read(dst, size) != size)
		error("loadTextFile: fread failed");
	fo.close();

	return size;
}

uint AGOSEngine::loadTextFile_gme(const char *filename, byte *dst) {
	uint res;
	uint32 offs;
	uint32 size;

	res = atoi(filename + 4) + _textIndexBase - 1;
	offs = _gameOffsetsPtr[res];
	size = _gameOffsetsPtr[res + 1] - offs;

	readGameFile(dst, offs, size);

	return size;
}

void AGOSEngine::loadTextIntoMem(uint16 stringId) {
	byte *p;
	uint16 baseMin = 0x8000, baseMax, size;

	_tablesHeapPtr = _tablesheapPtrNew;
	_tablesHeapCurPos = _tablesHeapCurPosNew;

	p = _strippedTxtMem;

	// get filename
	while (*p) {
		Common::String filename;
		while (*p)
			filename += *p++;
		p++;

		if (getPlatform() == Common::kPlatformAcorn) {
			filename += ".DAT";
		}

		baseMax = (p[0] * 256) | p[1];
		p += 2;

		if (stringId < baseMax) {
			_stringIdLocalMin = baseMin;
			_stringIdLocalMax = baseMax;

			_localStringtable = (byte **)_tablesHeapPtr;

			size = (baseMax - baseMin + 1) * sizeof(byte *);
			_tablesHeapPtr += size;
			_tablesHeapCurPos += size;

			size = loadTextFile(filename.c_str(), _tablesHeapPtr);

			setupLocalStringTable(_tablesHeapPtr, baseMax - baseMin + 1);

			_tablesHeapPtr += size;
			_tablesHeapCurPos += size;

			if (_tablesHeapCurPos > _tablesHeapSize) {
				error("loadTextIntoMem: Out of table memory");
			}
			return;
		}

		baseMin = baseMax;
	}

	error("loadTextIntoMem: didn't find %d", stringId);
}

static const byte polish_charWidth[226] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 6, 2, 8, 7, 6,10, 8, 2,
	4, 4, 7, 6, 3, 4, 2, 3, 6, 4,
	6, 6, 7, 6, 6, 6, 6, 6, 2, 8,
	6, 9, 7, 6, 6, 8, 7, 8, 8, 7,
	6, 9, 8, 2, 6, 7, 6,10, 8, 9,
	7, 9, 7, 7, 8, 8, 8,12, 8, 8,
	7, 6, 7, 6, 4, 7, 7, 7, 7, 6,
	7, 7, 4, 7, 6, 2, 3, 6, 2,10,
	6, 7, 7, 7, 5, 6, 4, 6, 6,10,
	6, 6, 6, 0, 0, 0, 0, 0, 8, 6,
	7, 7, 7, 7, 7, 6, 7, 7, 7, 4,
	4, 3, 8, 8, 7, 0, 0, 7, 7, 7,
	6, 6, 6, 9, 8, 0, 0, 0, 0, 0,
	7, 3, 7, 6, 6, 8, 0, 0, 6, 0,
	0, 0, 0, 2, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 7
};

static const byte charWidth[226] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 6, 2, 4, 8, 6,10, 9, 2,
	4, 4, 6, 6, 3, 4, 2, 3, 6, 4,
	6, 6, 7, 6, 6, 6, 6, 6, 2, 3,
	7, 7, 7, 6,11, 8, 7, 8, 8, 7,
	6, 9, 8, 2, 6, 7, 6,10, 8, 9,
	7, 9, 7, 7, 8, 8, 8,12, 8, 8,
	7, 3, 3, 3, 6, 8, 3, 7, 7, 6,
	7, 7, 4, 7, 6, 2, 3, 6, 2,10,
	6, 7, 7, 7, 5, 6, 4, 7, 7,10,
	6, 6, 6, 0, 0, 0, 0, 0, 8, 6,
	7, 7, 7, 7, 7, 6, 7, 7, 7, 4,
	4, 3, 8, 8, 7, 0, 0, 7, 7, 7,
	6, 6, 6, 9, 8, 0, 0, 0, 0, 0,
	7, 3, 7, 6, 6, 8, 0, 6, 0, 0,
	0, 0, 0, 2, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 7
};

const char *AGOSEngine::getPixelLength(const char *string, uint16 maxWidth, uint16 &pixels) {
	pixels = 0;

	while (*string != 0) {
		byte chr = *string;
		uint8 len = (_language == Common::PL_POL) ? polish_charWidth[chr] : charWidth[chr];
		if ((pixels + len) > maxWidth)
			break;
		pixels += len;
		string++;
	}

	return string;
}

bool AGOSEngine::printTextOf(uint a, uint x, uint y) {
	const byte *stringPtr;
	uint16 pixels, w;

	if (getGameType() == GType_SIMON2) {
		if (getBitFlag(79)) {
			Subroutine *sub;
			_variableArray[84] = a;
			sub = getSubroutineByID(5003);
			if (sub != NULL)
				startSubroutineEx(sub);
			return true;
		}
	}

	if (a >= _numTextBoxes)
		return false;


	stringPtr = getStringPtrByID(_shortText[a]);
	if (getGameType() == GType_FF) {
		getPixelLength((const char *)stringPtr, 400, pixels);
		w = pixels + 1;
		x -= w / 2;
		printScreenText(6, 0, (const char *)stringPtr, x, y, w);
	} else {
		showActionString(stringPtr);
	}

	return true;
}

bool AGOSEngine::printNameOf(Item *item, uint x, uint y) {
	SubObject *subObject;
	const byte *stringPtr;
	uint16 pixels, w;

	if (item == 0 || item == _dummyItem2 || item == _dummyItem3)
		return false;

	subObject = (SubObject *)findChildOfType(item, kObjectType);
	if (subObject == NULL)
		return false;

	stringPtr = getStringPtrByID(subObject->objectName);
	if (getGameType() == GType_FF) {
		getPixelLength((const char *)stringPtr, 400, pixels);
		w = pixels + 1;
		x -= w / 2;
		printScreenText(6, 0, (const char *)stringPtr, x, y, w);
	} else {
		showActionString(stringPtr);
	}

	return true;
}

void AGOSEngine::printScreenText(uint vgaSpriteId, uint color, const char *string, int16 x, int16 y, int16 width) {
	char convertedString[320];
	char *convertedString2 = convertedString;
	int16 height, talkDelay;
	int stringLength = strlen(string);
	int padding, lettersPerRow, lettersPerRowJustified;
	const int textHeight = 10;

	height = textHeight;
	lettersPerRow = width / 6;
	lettersPerRowJustified = stringLength / (stringLength / lettersPerRow + 1) + 1;

	talkDelay = (stringLength + 3) / 3;
	if (getGameType() == GType_SIMON1 && (getFeatures() & GF_TALKIE)) {
		if (_variableArray[141] == 0)
			_variableArray[141] = 9;
		_variableArray[85] = _variableArray[141] * talkDelay;
	} else {
		if (_variableArray[86] == 0)
			talkDelay /= 2;
		if (_variableArray[86] == 2)
			talkDelay *= 2;
		_variableArray[85] = talkDelay * 5;
	}

	assert(stringLength > 0);

	while (stringLength > 0) {
		int pos = 0;
		if (stringLength > lettersPerRow) {
			int removeLastWord = 0;
			if (lettersPerRow > lettersPerRowJustified) {
				pos = lettersPerRowJustified;
				while (string[pos] != ' ')
					pos++;
				if (pos > lettersPerRow)
					removeLastWord = 1;
			}
			if (lettersPerRow <= lettersPerRowJustified || removeLastWord) {
				pos = lettersPerRow;
				while (string[pos] != ' ' && pos > 0)
					pos--;
			}
			height += textHeight;
			y -= textHeight;
		} else
			pos = stringLength;
		padding = ((lettersPerRow - pos) % 2) ? (lettersPerRow - pos) / 2 + 1 : (lettersPerRow - pos) / 2;
		while (padding--)
			*convertedString2++ = ' ';
		stringLength -= pos;
		while (pos--)
			*convertedString2++ = *string++;
		*convertedString2++ = '\n';
		string++; // skip space
		stringLength--; // skip space
	}
	*(convertedString2 - 1) = '\0';

	if (getGameType() == GType_SIMON1)
		stopAnimate(vgaSpriteId + 199);
	else
		stopAnimateSimon2(2, vgaSpriteId);

	if (getPlatform() == Common::kPlatformAmiga) {
		color = color * 3 + 1;
		renderStringAmiga(vgaSpriteId, color, width, height, convertedString);
	} else {
		color = color * 3 + 192;
		renderString(vgaSpriteId, color, width, height, convertedString);
	}

	uint16 windowNum = (!getBitFlag(133)) ? 3 : 4;
	if (getGameType() == GType_SIMON1 && (getFeatures() & GF_DEMO))
		windowNum = 4;

	x /= 8;
	if (y < 2)
		y = 2;

	if (getGameType() == GType_SIMON1) {
		uint16 id = 199 + vgaSpriteId;
		animate(windowNum, id / 100, id, x, y, 12);
	} else {
		animate(windowNum, 2, vgaSpriteId, x, y, 12);
	}
}

#ifdef ENABLE_AGOS2
// Swampy Adventures specific
void AGOSEngine_PuzzlePack::printInfoText(const char *itemText) {
	const char *itemName = NULL;
	int flag = (_mouse.y / 32) * 20 + (_mouse.x / 32) + 1300;

	switch (_variableArray[999]) {
		case 80:
			if (_variableArray[flag]) {
				if (_variableArray[flag] == 201)
					itemName = " Bridge: ";
				if (_variableArray[flag] == 231 || _variableArray[flag] == 241)
					itemName = " Log: ";
				if (_variableArray[flag] == 281)
					itemName = " Rubble: ";
				if (_variableArray[flag] == 291)
					itemName = " Boulder: ";
				if (_variableArray[flag] == 311)
					itemName = " Key: ";
				if (_variableArray[flag] == 312)
					itemName = " Spanner: ";
				if (_variableArray[flag] == 321)
					itemName = " Gate: ";
				if (_variableArray[flag] == 331)
					itemName = " Crate: ";
			} else {
				flag -= 300;
				if (_variableArray[flag] == 2)
					itemName = " Water: ";
				if (_variableArray[flag] == 5)
					itemName = " Exit: ";
				if ((_variableArray[flag] == 5) && (_variableArray[81] == 10))
					itemName = " Gem: ";
				if (_variableArray[flag] == 236 || _variableArray[flag] == 246)
					itemName = " Floating Log: ";
				if (_variableArray[flag] == 400)
					itemName = " Valve: ";
			}
			break;

		case 81:
			if (_variableArray[flag]) {
				if (_variableArray[flag] == 281)
					itemName = " Cracked Block: ";
				if (_variableArray[flag] == 291)
					itemName = " Boulder: ";
				if (_variableArray[flag] == 331)
					itemName = " Block: ";
				if (_variableArray[flag] == 341)
					itemName = " Switch: ";
				if (_variableArray[flag] == 343)
					itemName = " Button: ";
				if ((_variableArray[flag] > 430) && (_variableArray[flag] < 480))
					itemName = " Mosaic Block: ";
			} else {
				flag -= 300;
				if (_variableArray[flag] == 5)
					itemName = " Exit: ";
				if ((_variableArray[flag] == 5) && (_variableArray[82] == 10))
					itemName = " Gem: ";
			}
			break;

		case 82:
			if (_variableArray[flag]) {
				if (_variableArray[flag] == 201 || _variableArray[flag] == 211)
					itemName = " Unstable Track: ";
				if (_variableArray[flag] == 281)
					itemName = " Rubble Pile: ";
				if (_variableArray[flag] == 291)
					itemName = " Boulder: ";
				if (_variableArray[flag] == 331)
					itemName = " Crate: ";
				if (_variableArray[flag] == 401 || _variableArray[flag] == 405)
					itemName = " Cart: ";
			} else {
				flag -= 300;
				if (_variableArray[flag] == 4)
					itemName = " Hole: ";
				if (_variableArray[flag] == 5)
					itemName = " Exit: ";
				if ((_variableArray[flag] == 5) && (_variableArray[83] == 10))
					itemName = " Gem: ";
				if ((_variableArray[flag] > 5) && (_variableArray[flag] < 10))
					itemName = " Buffer Track: ";
				if ((_variableArray[flag] > 9) && (_variableArray[flag] < 40))
					itemName = " Track: ";
				if (_variableArray[flag] == 300)
					itemName = " Boulder: ";
			}
			break;

		case 83:
			if (_variableArray[flag]) {
				if (_variableArray[flag] == 201)
					itemName = " Broken Floor: ";
				if (_variableArray[flag] == 231 || _variableArray[flag] == 241)
					itemName = " Barrel: ";
				if (_variableArray[flag] == 281)
					itemName = " Cracked Rock: ";
				if (_variableArray[flag] == 291)
					itemName = " Spacehopper: ";
				if (_variableArray[flag] == 311)
					itemName = " Key: ";
				if (_variableArray[flag] == 321)
					itemName = " Trapdoor: ";
				if (_variableArray[flag] == 324)
					itemName = " Trapdoor: ";
				if (_variableArray[flag] == 331)
					itemName = " Crate: ";
			} else {
				flag -= 300;
				if (_variableArray[flag] == 4)
					itemName = " Hole: ";
				if (_variableArray[flag] == 239 || _variableArray[flag] == 249)
					itemName = " Barrel: ";
			}
			break;

		case 84:
			if (_variableArray[flag]) {
				if (_variableArray[flag] == 201)
					itemName = " Floating Platform: ";
				if (_variableArray[flag] == 231)
					itemName = " Cauldron: ";
				if (_variableArray[flag] == 281)
					itemName = " Cracked Block: ";
				if (_variableArray[flag] == 311 || _variableArray[flag] == 312)
					itemName = " Key: ";
				if (_variableArray[flag] == 321 || _variableArray[flag] == 361 || _variableArray[flag] == 371)
					itemName = " Gate: ";
				if (_variableArray[flag] == 331)
					itemName = " Chest: ";
				if (_variableArray[flag] == 332)
					itemName = " Jewel: ";
				if (_variableArray[flag] == 351 || _variableArray[flag] == 352)
					itemName = " Babies: ";
			} else {
				flag -= 300;
				if (_variableArray[flag] == 6)
					itemName = " Slime: ";
				if (_variableArray[flag] == 334)
					itemName = " Chest: ";
			}
			break;

		default:
			break;
	}

	if (itemName != NULL) {
		Common::String buf = Common::String::format("%s\n%s", itemName, itemText);
		GUI::TimedMessageDialog dialog(buf, 1500);
		dialog.runModal();
	}
}

// The Feeble Files specific
void AGOSEngine_Feeble::printScreenText(uint vgaSpriteId, uint color, const char *string, int16 x, int16 y, int16 width) {
	char convertedString[320];
	char *convertedString2 = convertedString;
	const char *string2 = string;
	int16 height, talkDelay;
	int stringLength = strlen(string);
	const int textHeight = 15;

	height = textHeight;

	talkDelay = (stringLength + 3) / 3;
		if (_variableArray[86] == 0)
			talkDelay /= 2;
		if (_variableArray[86] == 2)
			talkDelay *= 2;
		_variableArray[85] = talkDelay * 5;

	assert(stringLength > 0);

	uint16 b, pixels, spaces;

	while (1) {
		string2 = getPixelLength(string, width, pixels);
		if (*string2 == 0) {
			spaces = (width - pixels) / 12;
			if (spaces != 0)
				spaces--;
			while (spaces) {
					*convertedString2++ = ' ';
					spaces--;
			}
			strcpy(convertedString2, string);
			break;
		}
		while (*string2 != ' ') {
			byte chr = *string2;
			pixels -= (_language == Common::PL_POL) ? polish_charWidth[chr] : charWidth[chr];
			string2--;
		}
		spaces = (width - pixels) / 12;
		if (spaces != 0)
			spaces--;
		while (spaces) {
				*convertedString2++ = ' ';
				spaces--;
		}
		b = string2 - string;
		strncpy(convertedString2, string, b);
		convertedString2 += b;
		*convertedString2++ = '\n';
		height += textHeight;
		y -= textHeight;
		if (y < 2)
			y = 2;
		string = string2;
	}

	stopAnimateSimon2(2, vgaSpriteId);

	renderString(1, color, width, height, convertedString);

	animate(4, 2, vgaSpriteId, x, y, 12);
}

void AGOSEngine_Feeble::printInteractText(uint16 num, const char *string) {
	char convertedString[320];
	char *convertedString2 = convertedString;
	const char *string2 = string;
	uint16 height = 15;
	uint16 w = 0xFFFF;
	uint16 b, pixels, x;

	// It doesn't really matter what 'w' is to begin with, as long as it's
	// something that cannot be generated by getPixelLength(). The original
	// used 620, which was a potential problem.

	while (1) {
		string2 = getPixelLength(string, 620, pixels);
		if (*string2 == 0x00) {
			if (w == 0xFFFF)
				w = pixels;
			strcpy(convertedString2, string);
			break;
		}
		while (*string2 != ' ') {
			byte chr = *string2;
			pixels -= (_language == Common::PL_POL) ? polish_charWidth[chr] : charWidth[chr];
			string2--;
		}
		if (w == 0xFFFF)
			w = pixels;
		b = string2 - string;
		strncpy(convertedString2, string, b);
		convertedString2 += b;
		*convertedString2++ = '\n';
		height += 15;
		string = string2;
	}

	// ScrollX
	x = _variableArray[251];
	x += 20;

	if (num == 1)
		_interactY = 385;

	// Returned values for box definition
	_variableArray[51] = x;
	_variableArray[52] = _interactY;
	_variableArray[53] = w;
	_variableArray[54] = height;

	stopAnimateSimon2(2, num + 6);
	renderString(num, 0, w, height, convertedString);
	animate(4, 2, num + 6, x, _interactY, 12);

	_interactY += height;
}

void AGOSEngine_Feeble::sendInteractText(uint16 num, const char *fmt, ...) {
	va_list arglist;

	va_start(arglist, fmt);
	Common::String string = Common::String::vformat(fmt, arglist);
	va_end(arglist);

	printInteractText(num, string.c_str());
}
#endif

// Waxworks specific
uint16 AGOSEngine_Waxworks::getBoxSize() {
	int x;
	switch (_boxLineCount) {
	case 1: x = _lineCounts[0];
		if (x <= 26)
			return 1;
		if (x <= 64)
			if (checkFit(_linePtrs[0], 32, 2))
				return 2;
		if (x <= 111)
			if (checkFit(_linePtrs[0], 37, 3))
				return 3;
		if (x <= 168)
			if (checkFit(_linePtrs[0], 42, 4))
				return 4;
		if (x <= 240)
			if (checkFit(_linePtrs[0], 48, 5))
				return 5;
		return 6;
	case 2: if (_lineCounts[0] <= 32) {
			if (_lineCounts[1] <= 32)
				return 2;
			if (_lineCounts[1] <= 74)
				if (checkFit(_linePtrs[1], 37, 2))
					return 3;
			if (_lineCounts[1] <= 126)
				if (checkFit(_linePtrs[1], 42, 3))
					return 4;
			if (_lineCounts[1] <= 172)
				if (checkFit(_linePtrs[1], 48, 4))
					return 5;
			return 6;
		}
		if ((_lineCounts[0] <= 74) && (checkFit(_linePtrs[0], 37, 2))) {
			if (_lineCounts[1] <= 37)
				return 3;
			if (_lineCounts[1] <= 84)
				if (checkFit(_linePtrs[1], 42, 2))
					return 4;
			if (_lineCounts[1] <= 144)
				if (checkFit(_linePtrs[1], 48, 3))
					return 5;
			return 6;
		}
		if ((_lineCounts[0] <= 126) && (checkFit(_linePtrs[0], 42, 3))) {
			if (_lineCounts[1] <= 42)
				return 4;
			if (_lineCounts[1] <= 84)
				if (checkFit(_linePtrs[1], 48, 2))
					return 5;
			return 6;
		}
		if ((_lineCounts[0] <= 192) && (checkFit(_linePtrs[0], 48, 4))) {
			if (_lineCounts[1] <= 48)
				return 5;
			return 6;
		}
		return 6;
	case 3: if (_lineCounts[0] <= 37) {
			if (_lineCounts[1] <= 37) {
				if (_lineCounts[2] <= 37)
						return 3;
				if (_lineCounts[2] <= 84)
					if (checkFit(_linePtrs[2], 42, 2))
						return 4;
				if (_lineCounts[2] <= 144)
					if (checkFit(_linePtrs[2], 48, 3))
						return 5;
				return 6;
			}
			if ((_lineCounts[1] <= 84) && (checkFit(_linePtrs[1], 42, 2))) {
				if (_lineCounts[2] <= 42)
					return 4;
				if (_lineCounts[2] <= 96)
					if (checkFit(_linePtrs[2], 48, 2))
						return 5;
				return 6;
			}
			if ((_lineCounts[1] <= 144) && (checkFit(_linePtrs[1], 48, 3))) {
				if (_lineCounts[2] <= 48)
					return 5;
				return 6;
			}
			return 6;
		}
		if ((_lineCounts[0] <= 84) && (checkFit(_linePtrs[0], 42, 2))) {
			if (_lineCounts[1] <= 42) {
				if (_lineCounts[2] <= 42)
					return 4;
				if (_lineCounts[2] <= 96)
					if (checkFit(_linePtrs[2], 48, 2))
						return 5;
				return 6;
			}
			if ((_lineCounts[1] <= 96) && (checkFit(_linePtrs[1], 48, 2))) {
				if (_lineCounts[2] <= 48)
					return 5;
				return 6;
			}
			return 6;
		}
		if ((_lineCounts[0] <= 96) && (checkFit(_linePtrs[0], 48, 3))) {
			if (_lineCounts[1] <= 48) {
				if (_lineCounts[2] <= 48)
					return 5;
			}
			return 6;
		}
		return 6;
	case 4: if (_lineCounts[0] <= 42) {
			if (_lineCounts[1] <= 42) {
				if (_lineCounts[2] <= 42) {
					if (_lineCounts[3] <= 42)
						return 4;
					if (_lineCounts[3] <= 96)
						if (checkFit(_linePtrs[3], 48, 2))
							return 5;
					return 6;
				}
				if ((_lineCounts[2] <= 96) && (checkFit(_linePtrs[2], 48, 2)))
					if (_lineCounts[3] <= 48)
						return 5;
				return 6;
			}
			if ((_lineCounts[1] <= 96) && (checkFit(_linePtrs[1], 48, 2)))
					if ((_lineCounts[2] <= 48) && (_lineCounts[3] <= 48))
						return 5;
			return 6;
		}
		if ((_lineCounts[0] <= 96) && (checkFit(_linePtrs[0], 48, 2)))
			if ((_lineCounts[1] <= 48) && (_lineCounts[2] <= 48) && (_lineCounts[3] <= 48))
				return 5;
		return 6;
	case 5: if ((_lineCounts[0] > 48) || (_lineCounts[1] > 48) || (_lineCounts[2] > 48)
			|| (_lineCounts[3] > 48) || (_lineCounts[4] > 48))
			return 6;
		else
			return 5;
	default:
		return 6;
	}
}


uint16 AGOSEngine_Waxworks::checkFit(char *ptr, int width, int lines) {
	int countw = 0;
	int countl = 0;
	char *x = NULL;
	while (*ptr) {
		if (*ptr == '\n')
			return 1;
		if (countw == width) {
			countl++;
			countw = 0;
			ptr = x;
		}
		if (*ptr == ' ') {
			x = ptr;
			x++;
		}
		countw++;
		if (countl == lines)
			return 0;
		ptr++;
	}

	return 1;
}

void AGOSEngine_Waxworks::boxTextMessage(const char *x) {
	sprintf(_boxBufferPtr, "%s\n", x);
	_lineCounts[_boxLineCount] += strlen(x);
	_boxBufferPtr += strlen(x) + 1;
	_boxLineCount++;
	_linePtrs[_boxLineCount] = _boxBufferPtr;
	_boxCR = 1;
}

void AGOSEngine_Waxworks::boxTextMsg(const char *x) {
	sprintf(_boxBufferPtr, "%s", x);
	_lineCounts[_boxLineCount] += strlen(x);
	_boxBufferPtr += strlen(x);
	_boxCR = 0;
}

void AGOSEngine_Waxworks::printBox() {
	uint16 BoxSize;

	*_boxBufferPtr = 0;
	_linePtrs[0] = _boxBuffer;
	if (_boxCR == 0)
		_boxLineCount++;
	stopAnimate(105);
	BoxSize = getBoxSize();
	_variableArray[53] = BoxSize;
	animate(3, 1, 100, 0, 0, 0);
	changeWindow(5);

	switch (BoxSize) {
	case 1: _textWindow->x = 10;
		_textWindow->y = 163;
		_textWindow->width = 20;
		_textWindow->height = 1;
		_textWindow->textMaxLength = 26;
		break;
	case 2: _textWindow->x = 8;
		_textWindow->y = 160;
		_textWindow->width = 24;
		_textWindow->height = 2;
		_textWindow->textMaxLength = 32;
		break;
	case 3: _textWindow->x = 6;
		_textWindow->y = 156;
		_textWindow->width = 28;
		_textWindow->height = 3;
		_textWindow->textMaxLength = 37;
		break;
	case 4: _textWindow->x = 4;
		_textWindow->y = 153;
		_textWindow->width = 32;
		_textWindow->height = 4;
		_textWindow->textMaxLength = 42;
		break;
	case 5: _textWindow->x = 2;
		_textWindow->y = 150;
		_textWindow->width = 36;
		_textWindow->height = 5;
		_textWindow->textMaxLength = 48;
		break;
	default:_textWindow->x = 1;
		_textWindow->y = 147;
		_textWindow->width = 38;
		_textWindow->height = 6;
		_textWindow->textMaxLength = 50;
		break;
	}
	_textWindow->textColumn = 0;
	_textWindow->textRow = 0;
	_textWindow->textColumnOffset = 0;
	_textWindow->textLength = 0;
	justifyStart();
	waitForSync(99);
	_boxBufferPtr = _boxBuffer;
	while (*_boxBufferPtr)
		justifyOutPut(*_boxBufferPtr++);
	_boxLineCount = 0;
	_boxBufferPtr = _boxBuffer;
	_lineCounts[0] = 0;
	_lineCounts[1] = 0;
	_lineCounts[2] = 0;
	_lineCounts[3] = 0;
	_lineCounts[4] = 0;
	_lineCounts[5] = 0;
	changeWindow(0);
}

} // End of namespace AGOS
