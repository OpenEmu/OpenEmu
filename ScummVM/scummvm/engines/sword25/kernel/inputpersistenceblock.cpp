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

#include "common/textconsole.h"

#include "sword25/kernel/inputpersistenceblock.h"

namespace Sword25 {

InputPersistenceBlock::InputPersistenceBlock(const void *data, uint dataLength, int version) :
	_data(static_cast<const byte *>(data), dataLength),
	_errorState(NONE),
	_version(version) {
	_iter = _data.begin();
}

InputPersistenceBlock::~InputPersistenceBlock() {
	if (_iter != _data.end())
		warning("Persistence block was not read to the end.");
}

void InputPersistenceBlock::read(int16 &value) {
	signed int v;
	read(v);
	value = static_cast<int16>(v);
}

void InputPersistenceBlock::read(signed int &value) {
	if (checkMarker(SINT_MARKER)) {
		value = (int32)READ_LE_UINT32(_iter);
		_iter += 4;
	} else {
		value = 0;
	}
}

void InputPersistenceBlock::read(uint &value) {
	if (checkMarker(UINT_MARKER)) {
		value = READ_LE_UINT32(_iter);
		_iter += 4;
	} else {
		value = 0;
	}
}

void InputPersistenceBlock::read(float &value) {
	if (checkMarker(FLOAT_MARKER)) {
		uint32 tmp[1];
		tmp[0] = READ_LE_UINT32(_iter);
		value = ((float *)tmp)[0];
		_iter += 4;
	} else {
		value = 0.0f;
	}
}

void InputPersistenceBlock::read(bool &value) {
	if (checkMarker(BOOL_MARKER)) {
		uint uintBool = READ_LE_UINT32(_iter);
		_iter += 4;
		value = uintBool != 0;
	} else {
		value = false;
	}
}

void InputPersistenceBlock::readString(Common::String &value) {
	value = "";

	if (checkMarker(STRING_MARKER)) {
		uint size;
		read(size);

		if (checkBlockSize(size)) {
			value = Common::String(reinterpret_cast<const char *>(&*_iter), size);
			_iter += size;
		}
	}
}

void InputPersistenceBlock::readByteArray(Common::Array<byte> &value) {
	if (checkMarker(BLOCK_MARKER)) {
		uint size;
		read(size);

		if (checkBlockSize(size)) {
			value = Common::Array<byte>(_iter, size);
			_iter += size;
		}
	}
}

bool InputPersistenceBlock::checkBlockSize(int size) {
	if (_data.end() - _iter >= size) {
		return true;
	} else {
		_errorState = END_OF_DATA;
		error("Unexpected end of persistence block.");
		return false;
	}
}

bool InputPersistenceBlock::checkMarker(byte marker) {
	if (!isGood() || !checkBlockSize(1))
		return false;

	if (*_iter++ == marker) {
		return true;
	} else {
		_errorState = OUT_OF_SYNC;
		error("Wrong type marker found in persistence block.");
		return false;
	}
}

} // End of namespace Sword25
