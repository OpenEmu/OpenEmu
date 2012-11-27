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

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "sword25/kernel/outputpersistenceblock.h"

namespace {
const uint INITIAL_BUFFER_SIZE = 1024 * 64;
}

namespace Sword25 {

OutputPersistenceBlock::OutputPersistenceBlock() {
	_data.reserve(INITIAL_BUFFER_SIZE);
}

void OutputPersistenceBlock::write(signed int value) {
	writeMarker(SINT_MARKER);
	value = TO_LE_32(value);
	rawWrite(&value, sizeof(value));
}

void OutputPersistenceBlock::write(uint value) {
	writeMarker(UINT_MARKER);
	value = TO_LE_32(value);
	rawWrite(&value, sizeof(value));
}

void OutputPersistenceBlock::write(float value) {
	writeMarker(FLOAT_MARKER);
	uint32 tmp[1];

	((float *)tmp)[0] = value;
	tmp[0] = TO_LE_32(tmp[0]);

	rawWrite(&value, sizeof(value));
}

void OutputPersistenceBlock::write(bool value) {
	writeMarker(BOOL_MARKER);

	uint uintBool = value ? 1 : 0;
	uintBool = TO_LE_32(uintBool);
	rawWrite(&uintBool, sizeof(uintBool));
}

void OutputPersistenceBlock::writeString(const Common::String &string) {
	writeMarker(STRING_MARKER);

	write(string.size());
	rawWrite(string.c_str(), string.size());
}

void OutputPersistenceBlock::writeByteArray(Common::Array<byte> &value) {
	writeMarker(BLOCK_MARKER);

	write((uint)value.size());
	rawWrite(&value[0], value.size());
}

void OutputPersistenceBlock::writeMarker(byte marker) {
	_data.push_back(marker);
}

void OutputPersistenceBlock::rawWrite(const void *dataPtr, size_t size) {
	if (size > 0) {
		uint oldSize = _data.size();
		_data.resize(oldSize + size);
		memcpy(&_data[oldSize], dataPtr, size);
	}
}

} // End of namespace Sword25
