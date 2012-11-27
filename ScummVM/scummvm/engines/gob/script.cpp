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

#include "common/util.h"
#include "common/endian.h"

#include "gob/gob.h"
#include "gob/script.h"
#include "gob/dataio.h"
#include "gob/expression.h"
#include "gob/videoplayer.h"

namespace Gob {

Script::Script(GobEngine *vm) : _vm(vm) {
	_expression = new Expression(vm);

	_finished = true;

	_totData = 0;
	_totPtr = 0;
	_totSize = 0;

	_lom = 0;

	memset(&_totProperties, 0, sizeof(TOTFile::Properties));
}

Script::~Script() {
	unload();

	delete _expression;
}

uint32 Script::read(byte *data, int32 size) {
	int32 toRead = MIN<int32>(size, _totSize - (_totPtr - _totData));

	if (toRead < 1)
		return 0;

	memcpy(data, _totPtr, toRead);
	_totPtr += toRead;

	return toRead;
}

uint32 Script::peek(byte *data, int32 size, int32 offset) const {
	int32 totOffset = ((_totPtr + offset) - _totData);

	if (totOffset < 1)
		return 0;
	if (((uint32) totOffset) >= _totSize)
		return 0;

	int32 toPeek = MIN<int32>(size, _totSize - totOffset);
	if (toPeek < 1)
		return 0;

	memcpy(data, _totPtr + offset, toPeek);

	return toPeek;
}

int32 Script::pos() const {
	if (!_totData)
		return -1;

	return _totPtr - _totData;
}

int32 Script::getSize() const {
	if (!_totData)
		return -1;

	return _totSize;
}

bool Script::seek(int32 offset, int whence) {
	if (!_totData)
		return false;

	if (whence == SEEK_CUR)
		offset += pos();
	else if (whence == SEEK_END)
		offset += _totSize;

	if ((offset < 0) || (((uint32) offset) >= _totSize))
		return false;

	// Cannot seek into the header
	if (offset < 128) {
		_finished = true;
		return false;
	}

	// A successful seek means the script file continues to be executed
	_finished = false;

	_totPtr = _totData + offset;

	return true;
}

bool Script::skip(int32 offset) {
	return seek(offset, SEEK_CUR);
}

bool Script::skipBlock() {
	return seek(peekUint16(2) + 2, SEEK_CUR);
}

int32 Script::getOffset(byte *ptr) const {
	if (!_totData)
		return -1;

	if ((ptr < _totData) || (ptr >= (_totData + _totSize)))
		return -1;

	return ptr - _totData;
}

byte *Script::getData(int32 offset) const {
	if (!_totData)
		return 0;
	if ((offset < 0) || (((uint32) offset) >= _totSize))
		return 0;

	return _totData + offset;
}

byte *Script::getData() {
	return _totData;
}

byte Script::readByte() {
	byte v = 0;
	uint32 n;

	n = read(&v, 1);
	assert(n == 1);

	return v;
}

char Script::readChar() {
	return (char) readByte();
}

uint8 Script::readUint8() {
	return (uint8) readByte();
}

uint16 Script::readUint16() {
	byte v[2] = {0, 0};
	uint32 n;

	n = read(v, 2);
	assert(n == 2);

	return READ_LE_UINT16(v);
}

uint32 Script::readUint32() {
	byte v[4] = {0, 0, 0, 0};
	uint32 n;

	n = read(v, 4);
	assert(n == 4);

	return READ_LE_UINT32(v);
}

int8 Script::readInt8() {
	return (int8) readByte();
}

int16 Script::readInt16() {
	return (int16) readUint16();
}

int32 Script::readInt32() {
	return (int32) readUint32();
}

char *Script::readString(int32 length) {
	if (length < 0) {
		length = 0;
		while (_totPtr[length++] != '\0') { }
	}

	char *string = (char *)_totPtr;

	_totPtr += length;

	return string;
}

byte Script::peekByte(int32 offset) {
	byte v;

	peek(&v, 1, offset);

	return v;
}

char Script::peekChar(int32 offset) {
	return (char) peekByte(offset);
}

uint8 Script::peekUint8(int32 offset) {
	return (uint8) peekByte(offset);
}

uint16 Script::peekUint16(int32 offset) {
	byte v[2];

	peek(v, 2, offset);

	return READ_LE_UINT16(v);
}

uint32 Script::peekUint32(int32 offset) {
	byte v[4];

	peek(v, 4, offset);

	return READ_LE_UINT32(v);
}

int8 Script::peekInt8(int32 offset) {
	return (int8) peekByte(offset);
}

int16 Script::peekInt16(int32 offset) {
	return (int16) peekUint16(offset);
}

int32 Script::peekInt32(int32 offset) {
	return (int32) peekUint32(offset);
}

char *Script::peekString(int32 offset) {
	return (char *)(_totPtr + offset);
}

int16 Script::readVarIndex(uint16 *size, uint16 *type) {
	return _expression->parseVarIndex(size, type);
}

int16 Script::readValExpr(byte stopToken) {
	return _expression->parseValExpr(stopToken);
}

int16 Script::readExpr(byte stopToken, byte *type) {
	return _expression->parseExpr(stopToken, type);
}

void Script::skipExpr(char stopToken) {
	_expression->skipExpr(stopToken);
}

char Script::evalExpr(int16 *pRes) {
	byte type;

	_expression->printExpr(99);

	_expression->parseExpr(99, &type);
	if (!pRes)
		return type;

	switch (type) {
	case TYPE_IMM_INT16:
		*pRes = _expression->getResultInt();
		break;

	case TYPE_IMM_STR:
	case GOB_FALSE:
		*pRes = 0;
		break;

	case GOB_TRUE:
		*pRes = 1;
		break;
	}

	return type;
}

bool Script::evalBool() {
	byte type;

	_expression->printExpr(99);

	_expression->parseExpr(99, &type);
	if ( (type == GOB_TRUE) ||
	    ((type == TYPE_IMM_INT16) && _expression->getResultInt()))
		return true;
	else
		return false;
}

int32 Script::evalInt() {
	evalExpr(0);
	return getResultInt();
}

const char *Script::evalString() {
	evalExpr(0);
	return getResultStr();
}

int32 Script::getResultInt() const {
	return _expression->getResultInt();
}

char *Script::getResultStr() const {
	return _expression->getResultStr();
}

bool Script::load(const Common::String &fileName) {
	unload();

	_finished = false;

	bool isLOM;

	_totFile = TOTFile::createFileName(fileName, isLOM);

	if (isLOM) {
		if (!loadLOM(_totFile)) {
			unload();
			return false;
		}
	} else {
		if (!loadTOT(_totFile)) {
			unload();
			return false;
		}
	}

	return true;
}

bool Script::loadTOT(const Common::String &fileName) {
	TOTFile totFile(_vm);

	if (!totFile.load(fileName))
		return false;

	Common::SeekableReadStream *stream = totFile.getStream();
	if (!stream)
		return false;

	if (!totFile.getProperties(_totProperties))
		return false;

	_totSize = _totProperties.scriptEnd;
	if (_totSize <= 0)
		return false;

	_totData = new byte[_totSize];
	if (stream->read(_totData, _totSize) != _totSize)
		return false;

	return true;
}

bool Script::loadLOM(const Common::String &fileName) {
	warning("Stub: Script::loadLOM(%s)", _totFile.c_str());

	_lom = _vm->_dataIO->getFile(_totFile);
	if (!_lom)
		return false;

	_lom->seek(48);
	_totSize = _lom->readUint32LE();
	_lom->seek(0);

	_totData = new byte[_totSize];
	_lom->read(_totData, _totSize);

	return false;
}

void Script::unload() {
	unloadTOT();
}

void Script::unloadTOT() {
	delete _lom;
	_lom = 0;

	// Unwind the call stack
	while (!_callStack.empty())
		pop();

	delete[] _totData;

	_totData = 0;
	_totSize = 0;
	_totPtr = 0;
	_totFile.clear();

	_finished = true;
}

bool Script::isLoaded() const {
	return _totData != 0;
}

void Script::setFinished(bool finished) {
	_finished = finished;
}

bool Script::isFinished() const {
	return _finished;
}

void Script::push() {
	if (!isLoaded())
		// Nothing to do
		return;

	CallEntry currentCall;

	currentCall.totPtr = _totPtr;
	currentCall.finished = _finished;

	_callStack.push(currentCall);
}

void Script::pop(bool ret) {
	if (!isLoaded())
		// Nothing to do
		return;

	// Unmatched pop?
	assert(!_callStack.empty());

	CallEntry lastCall = _callStack.pop();

	if (ret) {
		_totPtr = lastCall.totPtr;
		_finished = lastCall.finished;
	}
}

void Script::call(uint32 offset) {
	if (!isLoaded())
		// Nothing to do
		return;

	push();
	seek(offset);
}

uint8 Script::getVersionMajor() const {
	return _totProperties.versionMajor;
}

uint8 Script::getVersionMinor() const {
	return _totProperties.versionMinor;
}

uint32 Script::getVariablesCount() const {
	return _totProperties.variablesCount;
}

uint32 Script::getTextsOffset() const {
	return _totProperties.textsOffset;
}

uint32 Script::getResourcesOffset() const {
	return _totProperties.resourcesOffset;
}

uint16 Script::getAnimDataSize() const {
	return _totProperties.animDataSize;
}

uint8 Script::getImFileNumber() const {
	return _totProperties.imFileNumber;
}

uint8 Script::getExFileNumber() const {
	return _totProperties.exFileNumber;
}

uint8 Script::getCommunHandling() const {
	return _totProperties.communHandling;
}

uint16 Script::getFunctionOffset(uint8 function) const {
	if (!_totData)
		return 0;

	// Offsets 100-128, 2 bytes per function
	assert(function <= 13);

	return _totProperties.functions[function];
}

uint32 Script::getVariablesCount(const char *fileName, GobEngine *vm) {
	Common::SeekableReadStream *stream = vm->_dataIO->getFile(fileName);
	if (!stream)
		return 0;

	stream->seek(0x2C);
	uint32 variablesCount = stream->readUint32LE();
	delete stream;

	return variablesCount;
}

} // End of namespace Gob
