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

#include "toon/text.h"

namespace Toon {

TextResource::TextResource(ToonEngine *vm) : _vm(vm) {
	_numTexts = 0;
	_textData = NULL;
}

TextResource::~TextResource(void) {
	delete[] _textData;
}

bool TextResource::loadTextResource(const Common::String &fileName) {
	debugC(1, kDebugText, "loadTextResource(%s)", fileName.c_str());

	uint32 fileSize = 0;
	uint8 *data = _vm->resources()->getFileData(fileName, &fileSize);
	if (!data)
		return false;

	delete[] _textData;
	_textData = new uint8[fileSize];
	memcpy(_textData, data, fileSize);
	_numTexts = READ_LE_UINT16(data);

	return true;
}

int32 TextResource::getNext(int32 offset) {
	debugC(1, kDebugText, "getNext(%d)", offset);

	uint16 *table = (uint16 *)_textData + 1;
	int a = getId(offset);
	return READ_LE_UINT16(table + a + 1);
}

int32 TextResource::getId(int32 offset) {
	debugC(1, kDebugText, "getId(%d)", offset);

	uint16 *table = (uint16 *)_textData + 1;
	int32 found = -1;
	for (int32 i = 0; i < _numTexts; i++) {
		if (offset == READ_LE_UINT16(table + i)) {
			found = i;
			break;
		}
	}
	return found;
}

char *TextResource::getText(int32 offset) {
	debugC(6, kDebugText, "getText(%d)", offset);

	uint16 *table = (uint16 *)_textData + 1;
	int32 found = -1;
	for (int32 i = 0; i < _numTexts; i++) {
		if (offset == READ_LE_UINT16(table + i)) {
			found = i;
			break;
		}
	}
	if (found < 0)
		return NULL;

	int32 realOffset = READ_LE_UINT16((uint16 *)_textData + 1 + _numTexts + found);
	return (char *)_textData + realOffset;
}

} // End of namespace Toon
