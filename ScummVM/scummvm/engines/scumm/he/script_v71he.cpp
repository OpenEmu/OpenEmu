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

#ifdef ENABLE_HE

#include "scumm/actor.h"
#include "scumm/he/intern_he.h"
#include "scumm/scumm.h"
#include "scumm/util.h"

namespace Scumm {

#define OPCODE(i, x)	_opcodes[i]._OPCODE(ScummEngine_v71he, x)

void ScummEngine_v71he::setupOpcodes() {
	ScummEngine_v70he::setupOpcodes();

	OPCODE(0xc9, o71_kernelSetFunctions);
	OPCODE(0xec, o71_copyString);
	OPCODE(0xed, o71_getStringWidth);
	OPCODE(0xef, o71_appendString);
	OPCODE(0xf0, o71_concatString);
	OPCODE(0xf1, o71_compareString);
	OPCODE(0xf5, o71_getStringLenForWidth);
	OPCODE(0xf6, o71_getCharIndexInString);
	OPCODE(0xf7, o71_findBox);
	OPCODE(0xfb, o71_polygonOps);
	OPCODE(0xfc, o71_polygonHit);
}

byte *ScummEngine_v71he::heFindResourceData(uint32 tag, byte *ptr) {
	ptr = heFindResource(tag, ptr);

	if (ptr == NULL)
		return NULL;
	return ptr + _resourceHeaderSize;
}

byte *ScummEngine_v71he::heFindResource(uint32 tag, byte *searchin) {
	uint32 curpos, totalsize, size;

	debugC(DEBUG_RESOURCE, "heFindResource(%s, %p)", tag2str(tag), (const void *)searchin);

	assert(searchin);
	searchin += 4;
	_resourceLastSearchSize = totalsize = READ_BE_UINT32(searchin);
	curpos = 8;
	searchin += 4;

	while (curpos < totalsize) {
		if (READ_BE_UINT32(searchin) == tag) {
			return searchin;
		}

		size = READ_BE_UINT32(searchin + 4);
		if ((int32)size <= 0) {
			error("(%s) Not found in %d... illegal block len %d", tag2str(tag), 0, size);
			return NULL;
		}

		curpos += size;
		searchin += size;
	}

	return NULL;
}

byte *ScummEngine_v71he::findWrappedBlock(uint32 tag, byte *ptr, int state, bool errorFlag) {
	if (READ_BE_UINT32(ptr) == MKTAG('M','U','L','T')) {
		byte *offs, *wrap;
		uint32 size;

		wrap = heFindResource(MKTAG('W','R','A','P'), ptr);
		if (wrap == NULL)
			return NULL;

		offs = heFindResourceData(MKTAG('O','F','F','S'), wrap);
		if (offs == NULL)
			return NULL;

		size = getResourceDataSize(offs) / 4;
		assert((uint32)state <= (uint32)size);


		offs += READ_LE_UINT32(offs + state * sizeof(uint32));
		offs = heFindResourceData(tag, offs - 8);
		if (offs)
			return offs;

		offs = heFindResourceData(MKTAG('D','E','F','A'), ptr);
		if (offs == NULL)
			return NULL;

		return heFindResourceData(tag, offs - 8);
	} else {
		return heFindResourceData(tag, ptr);
	}
}

int ScummEngine_v71he::getStringCharWidth(byte chr) {
	int charset = _string[0]._default.charset;

	byte *ptr = getResourceAddress(rtCharset, charset);
	assert(ptr);
	ptr += 29;

	int spacing = 0;

	int offs = READ_LE_UINT32(ptr + chr * 4 + 4);
	if (offs) {
		spacing = ptr[offs] + (signed char)ptr[offs + 2];
	}

	return spacing;
}

int ScummEngine_v71he::setupStringArray(int size) {
	writeVar(0, 0);
	defineArray(0, kStringArray, 0, size + 1);
	writeArray(0, 0, 0, 0);
	return readVar(0);
}

void ScummEngine_v71he::appendSubstring(int dst, int src, int srcOffs, int len) {
	int dstOffs, value;
	int i = 0;

	if (len == -1) {
		len = resStrLen(getStringAddress(src));
		srcOffs = 0;
	}

	dstOffs = resStrLen(getStringAddress(dst));

	len -= srcOffs;
	len++;

	while (i < len) {
		writeVar(0, src);
		value = readArray(0, 0, srcOffs + i);
		writeVar(0, dst);
		writeArray(0, 0, dstOffs + i, value);
		i++;
	}

	writeArray(0, 0, dstOffs + i, 0);
}

void ScummEngine_v71he::adjustRect(Common::Rect &rect) {
	// Scripts can set all rect positions to -1
	if (rect.right != -1)
		rect.right += 1;

	if (rect.bottom != -1)
		rect.bottom += 1;
}

void ScummEngine_v71he::o71_kernelSetFunctions() {
	int args[29];
	int num;
	ActorHE *a;

	num = getStackList(args, ARRAYSIZE(args));

	switch (args[0]) {
	case 1:
		// Used to restore images when decorating cake in
		// Fatty Bear's Birthday Surprise
		virtScreenLoad(args[1], args[2], args[3], args[4], args[5]);
		break;
	case 20: // HE72+
		a = (ActorHE *)derefActor(args[1], "o71_kernelSetFunctions: 20");
		queueAuxBlock(a);
		break;
	case 21:
		_skipDrawObject = 1;
		break;
	case 22:
		_skipDrawObject = 0;
		break;
	case 23:
		clearCharsetMask();
		_fullRedraw = true;
		break;
	case 24:
		_skipProcessActors = 1;
		redrawAllActors();
		break;
	case 25:
		_skipProcessActors = 0;
		redrawAllActors();
		break;
	case 26:
		a = (ActorHE *)derefActor(args[1], "o71_kernelSetFunctions: 26");
		a->_auxBlock.r.left = 0;
		a->_auxBlock.r.right = -1;
		a->_auxBlock.r.top = 0;
		a->_auxBlock.r.bottom = -2;
		break;
	case 30:
		a = (ActorHE *)derefActor(args[1], "o71_kernelSetFunctions: 30");
		a->_clipOverride.bottom = args[2];
		break;
	case 42:
		_wiz->_rectOverrideEnabled = true;
		_wiz->_rectOverride.left = args[1];
		_wiz->_rectOverride.top = args[2];
		_wiz->_rectOverride.right = args[3];
		_wiz->_rectOverride.bottom = args[4];
		adjustRect(_wiz->_rectOverride);
		break;
	case 43:
		_wiz->_rectOverrideEnabled = false;
		break;
	default:
		error("o71_kernelSetFunctions: default case %d (param count %d)", args[0], num);
	}
}

void ScummEngine_v71he::o71_copyString() {
	int dst, size;
	int src = pop();

	size = resStrLen(getStringAddress(src)) + 1;
	dst = setupStringArray(size);

	appendSubstring(dst, src, -1, -1);

	push(dst);
}

void ScummEngine_v71he::o71_getStringWidth() {
	int array, pos, len;
	int chr, width = 0;

	len = pop();
	pos = pop();
	array = pop();

	if (len == -1) {
		pos = 0;
		len = resStrLen(getStringAddress(array));
	}

	writeVar(0, array);
	while (pos <= len) {
		chr = readArray(0, 0, pos);
		if (chr == 0)
			break;
		width += getStringCharWidth(chr);
		pos++;
	}

	push(width);
}

void ScummEngine_v71he::o71_appendString() {
	int dst, size;

	int len = pop();
	int srcOffs = pop();
	int src = pop();

	size = len - srcOffs + 2;
	dst = setupStringArray(size);

	appendSubstring(dst, src, srcOffs, len);

	push(dst);
}

void ScummEngine_v71he::o71_concatString() {
	int dst, size;

	int src2 = pop();
	int src1 = pop();

	size = resStrLen(getStringAddress(src1));
	size += resStrLen(getStringAddress(src2)) + 1;
	dst = setupStringArray(size);

	appendSubstring(dst, src1, 0, -1);
	appendSubstring(dst, src2, 0, -1);

	push(dst);
}

void ScummEngine_v71he::o71_compareString() {
	int result;

	int array1 = pop();
	int array2 = pop();

	byte *string1 = getStringAddress(array1);
	if (!string1)
		error("o71_compareString: Reference to zeroed array pointer (%d)", array1);

	byte *string2 = getStringAddress(array2);
	if (!string2)
		error("o71_compareString: Reference to zeroed array pointer (%d)", array2);

	while (*string1 == *string2) {
		if (*string2 == 0) {
			push(0);
			return;
		}

		string1++;
		string2++;
	}

	result = (*string1 > *string2) ? -1 : 1;
	push(result);
}

void ScummEngine_v71he::o71_getStringLenForWidth() {
	int chr, max;
	int array, len, pos, width = 0;

	max = pop();
	pos = pop();
	array = pop();

	len = resStrLen(getStringAddress(array));

	writeVar(0, array);
	while (pos <= len) {
		chr = readArray(0, 0, pos);
		width += getStringCharWidth(chr);
		if (width >= max) {
			push(pos);
			return;
		}
		pos++;
	}

	push(len);
}

void ScummEngine_v71he::o71_getCharIndexInString() {
	int array, end, len, pos, value;

	value = pop();
	end = pop();
	pos = pop();
	array = pop();

	if (end >= 0) {
		len = resStrLen(getStringAddress(array));
		if (len < end)
			end = len;
	} else {
		end = 0;
	}

	if (pos < 0)
		pos = 0;

	writeVar(0, array);
	if (end > pos) {
		while (end >= pos) {
			if (readArray(0, 0, pos) == value) {
				push(pos);
				return;
			}
			pos++;
		}
	} else {
		while (end <= pos) {
			if (readArray(0, 0, pos) == value) {
				push(pos);
				return;
			}
			pos--;
		}
	}

	push(-1);
}

void ScummEngine_v71he::o71_findBox() {
	int y = pop();
	int x = pop();
	push(getSpecialBox(x, y));
}

void ScummEngine_v71he::o71_polygonOps() {
	int vert1x, vert1y, vert2x, vert2y, vert3x, vert3y, vert4x, vert4y;
	int id, fromId, toId;
	bool flag;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 68: // HE 100
	case 69: // HE 100
	case 246:
	case 248:
		vert4y = pop();
		vert4x = pop();
		vert3y = pop();
		vert3x = pop();
		vert2y = pop();
		vert2x = pop();
		vert1y = pop();
		vert1x = pop();
		flag = (subOp == 69 || subOp == 248);
		id = pop();
		_wiz->polygonStore(id, flag, vert1x, vert1y, vert2x, vert2y, vert3x, vert3y, vert4x, vert4y);
		break;
	case 28: // HE 100
	case 247:
		toId = pop();
		fromId = pop();
		_wiz->polygonErase(fromId, toId);
		break;
	default:
		error("o71_polygonOps: default case %d", subOp);
	}
}

void ScummEngine_v71he::o71_polygonHit() {
	int y = pop();
	int x = pop();
	push(_wiz->polygonHit(0, x, y));
}

} // End of namespace Scumm

#endif // ENABLE_HE
