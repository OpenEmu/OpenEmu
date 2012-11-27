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

#include "kyra/script.h"
#include "kyra/kyra_v1.h"
#include "kyra/resource.h"

#include "common/endian.h"

namespace Kyra {
EMCInterpreter::EMCInterpreter(KyraEngine_v1 *vm) : _vm(vm), _scriptData(0), _filename(0) {
#define OPCODE(x) { &EMCInterpreter::x, #x }
	static const OpcodeEntry opcodes[] = {
		// 0x00
		OPCODE(op_jmp),
		OPCODE(op_setRetValue),
		OPCODE(op_pushRetOrPos),
		OPCODE(op_push),
		// 0x04
		OPCODE(op_push),
		OPCODE(op_pushReg),
		OPCODE(op_pushBPNeg),
		OPCODE(op_pushBPAdd),
		// 0x08
		OPCODE(op_popRetOrPos),
		OPCODE(op_popReg),
		OPCODE(op_popBPNeg),
		OPCODE(op_popBPAdd),
		// 0x0C
		OPCODE(op_addSP),
		OPCODE(op_subSP),
		OPCODE(op_sysCall),
		OPCODE(op_ifNotJmp),
		// 0x10
		OPCODE(op_negate),
		OPCODE(op_eval),
		OPCODE(op_setRetAndJmp)
	};
	_opcodes = opcodes;
#undef OPCODE
}

bool EMCInterpreter::callback(Common::IFFChunk &chunk) {
	switch (chunk._type) {
	case MKTAG('T','E','X','T'):
		_scriptData->text = new byte[chunk._size];
		assert(_scriptData->text);
		if (chunk._stream->read(_scriptData->text, chunk._size) != chunk._size)
			error("Couldn't read TEXT chunk from file '%s'", _filename);
		break;

	case MKTAG('O','R','D','R'):
		_scriptData->ordr = new uint16[chunk._size >> 1];
		assert(_scriptData->ordr);
		if (chunk._stream->read(_scriptData->ordr, chunk._size) != chunk._size)
			error("Couldn't read ORDR chunk from file '%s'", _filename);

		for (int i = (chunk._size >> 1) - 1; i >= 0; --i)
			_scriptData->ordr[i] = READ_BE_UINT16(&_scriptData->ordr[i]);
		break;

	case MKTAG('D','A','T','A'):
		_scriptData->data = new uint16[chunk._size >> 1];
		assert(_scriptData->data);
		if (chunk._stream->read(_scriptData->data, chunk._size) != chunk._size)
			error("Couldn't read DATA chunk from file '%s'", _filename);

		for (int i = (chunk._size >> 1) - 1; i >= 0; --i)
			_scriptData->data[i] = READ_BE_UINT16(&_scriptData->data[i]);
		break;

	default:
		warning("Unexpected chunk '%s' of size %d found in file '%s'", tag2str(chunk._type), chunk._size, _filename);
	}

	return false;
}

bool EMCInterpreter::load(const char *filename, EMCData *scriptData, const Common::Array<const Opcode *> *opcodes) {
	Common::SeekableReadStream *stream = _vm->resource()->createReadStream(filename);
	if (!stream) {
		error("Couldn't open script file '%s'", filename);
		return false;  // for compilers that don't support NORETURN
	}

	memset(scriptData, 0, sizeof(EMCData));

	_scriptData = scriptData;
	_filename = filename;

	IFFParser iff(*stream);
	Common::Functor1Mem< Common::IFFChunk &, bool, EMCInterpreter > c(this, &EMCInterpreter::callback);
	iff.parse(c);

	if (!_scriptData->ordr)
		error("No ORDR chunk found in file: '%s'", filename);

	if (!_scriptData->data)
		error("No DATA chunk found in file: '%s'", filename);

	if (stream->err())
		error("Read error while parsing file '%s'", filename);

	delete stream;

	_scriptData->sysFuncs = opcodes;

	Common::strlcpy(_scriptData->filename, filename, 13);

	_scriptData = 0;
	_filename = 0;

	return true;
}

void EMCInterpreter::unload(EMCData *data) {
	if (!data)
		return;

	delete[] data->text;
	delete[] data->ordr;
	delete[] data->data;

	data->text = 0;
	data->ordr = data->data = 0;
}

void EMCInterpreter::init(EMCState *scriptStat, const EMCData *data) {
	scriptStat->dataPtr = data;
	scriptStat->ip = 0;
	scriptStat->stack[EMCState::kStackLastEntry] = 0;
	scriptStat->bp = EMCState::kStackSize+1;
	scriptStat->sp = EMCState::kStackLastEntry;
}

bool EMCInterpreter::start(EMCState *script, int function) {
	if (!script->dataPtr)
		return false;

	uint16 functionOffset = script->dataPtr->ordr[function];
	if (functionOffset == 0xFFFF)
		return false;

	if (_vm->game() == GI_KYRA1) {
		if (_vm->gameFlags().platform == Common::kPlatformFMTowns || _vm->gameFlags().platform == Common::kPlatformPC98)
			script->ip = &script->dataPtr->data[functionOffset+1];
		else
			script->ip = &script->dataPtr->data[functionOffset];
	} else {
		script->ip = &script->dataPtr->data[functionOffset+1];
	}

	return true;
}

bool EMCInterpreter::isValid(EMCState *script) {
	if (!script->ip || !script->dataPtr || _vm->shouldQuit())
		return false;
	return true;
}

bool EMCInterpreter::run(EMCState *script) {
	_parameter = 0;

	if (!script->ip)
		return false;

	// Should be no Problem at all to cast to uint32 here, since that's the biggest ptrdiff the original
	// would allow, of course that's not realistic to happen to be somewhere near the limit of uint32 anyway.
	const uint32 instOffset = (uint32)((const byte *)script->ip - (const byte *)script->dataPtr->data);
	int16 code = *script->ip++;
	int16 opcode = (code >> 8) & 0x1F;

	if (code & 0x8000) {
		opcode = 0;
		_parameter = code & 0x7FFF;
	} else if (code & 0x4000) {
		_parameter = (int8)(code);
	} else if (code & 0x2000) {
		_parameter = *script->ip++;
	} else {
		_parameter = 0;
	}

	if (opcode > 18) {
		error("Unknown script opcode: %d in file '%s' at offset 0x%.08X", opcode, script->dataPtr->filename, instOffset);
	} else {
		debugC(5, kDebugLevelScript, "[0x%.08X] EMCInterpreter::%s([%d/%u])", instOffset, _opcodes[opcode].desc, _parameter, (uint)_parameter);
		(this->*(_opcodes[opcode].proc))(script);
	}

	return (script->ip != 0);
}

#pragma mark -
#pragma mark - Command implementations
#pragma mark -

void EMCInterpreter::op_jmp(EMCState *script) {
	script->ip = script->dataPtr->data + _parameter;
}

void EMCInterpreter::op_setRetValue(EMCState *script) {
	script->retValue = _parameter;
}

void EMCInterpreter::op_pushRetOrPos(EMCState *script) {
	switch (_parameter) {
	case 0:
		script->stack[--script->sp] = script->retValue;
		break;

	case 1:
		script->stack[--script->sp] = script->ip - script->dataPtr->data + 1;
		script->stack[--script->sp] = script->bp;
		script->bp = script->sp + 2;
		break;

	default:
		script->ip = 0;
	}
}

void EMCInterpreter::op_push(EMCState *script) {
	script->stack[--script->sp] = _parameter;
}

void EMCInterpreter::op_pushReg(EMCState *script) {
	script->stack[--script->sp] = script->regs[_parameter];
}

void EMCInterpreter::op_pushBPNeg(EMCState *script) {
	script->stack[--script->sp] = script->stack[(-(int32)(_parameter + 2)) + script->bp];
}

void EMCInterpreter::op_pushBPAdd(EMCState *script) {
	script->stack[--script->sp] = script->stack[(_parameter - 1) + script->bp];
}

void EMCInterpreter::op_popRetOrPos(EMCState *script) {
	switch (_parameter) {
	case 0:
		script->retValue = script->stack[script->sp++];
		break;

	case 1:
		if (script->sp >= EMCState::kStackLastEntry) {
			script->ip = 0;
		} else {
			script->bp = script->stack[script->sp++];
			script->ip = script->dataPtr->data + script->stack[script->sp++];
		}
		break;

	default:
		script->ip = 0;
	}
}

void EMCInterpreter::op_popReg(EMCState *script) {
	script->regs[_parameter] = script->stack[script->sp++];
}

void EMCInterpreter::op_popBPNeg(EMCState *script) {
	script->stack[(-(int32)(_parameter + 2)) + script->bp] = script->stack[script->sp++];
}

void EMCInterpreter::op_popBPAdd(EMCState *script) {
	script->stack[(_parameter - 1) + script->bp] = script->stack[script->sp++];
}

void EMCInterpreter::op_addSP(EMCState *script) {
	script->sp += _parameter;
}

void EMCInterpreter::op_subSP(EMCState *script) {
	script->sp -= _parameter;
}

void EMCInterpreter::op_sysCall(EMCState *script) {
	const uint8 id = _parameter;

	assert(script->dataPtr->sysFuncs);
	assert(id < script->dataPtr->sysFuncs->size());

	if ((*script->dataPtr->sysFuncs)[id] && ((*script->dataPtr->sysFuncs)[id])->isValid()) {
		script->retValue = (*(*script->dataPtr->sysFuncs)[id])(script);
	} else {
		script->retValue = 0;
		warning("Unimplemented system call 0x%.02X/%d used in file '%s'", id, id, script->dataPtr->filename);
	}
}

void EMCInterpreter::op_ifNotJmp(EMCState *script) {
	if (!script->stack[script->sp++]) {
		_parameter &= 0x7FFF;
		script->ip = script->dataPtr->data + _parameter;
	}
}

void EMCInterpreter::op_negate(EMCState *script) {
	int16 value = script->stack[script->sp];
	switch (_parameter) {
	case 0:
		if (!value)
			script->stack[script->sp] = 1;
		else
			script->stack[script->sp] = 0;
		break;

	case 1:
		script->stack[script->sp] = -value;
		break;

	case 2:
		script->stack[script->sp] = ~value;
		break;

	default:
		warning("Unknown negation func: %d", _parameter);
		script->ip = 0;
	}
}

void EMCInterpreter::op_eval(EMCState *script) {
	int16 ret = 0;
	bool error = false;

	int16 val1 = script->stack[script->sp++];
	int16 val2 = script->stack[script->sp++];

	switch (_parameter) {
	case 0:
		ret = (val2 && val1) ? 1 : 0;
		break;

	case 1:
		ret = (val2 || val1) ? 1 : 0;
		break;

	case 2:
		ret = (val1 == val2) ? 1 : 0;
		break;

	case 3:
		ret = (val1 != val2) ? 1 : 0;
		break;

	case 4:
		ret = (val1 > val2) ? 1 : 0;
		break;

	case 5:
		ret = (val1 >= val2) ? 1 : 0;
		break;

	case 6:
		ret = (val1 < val2) ? 1 : 0;
		break;

	case 7:
		ret = (val1 <= val2) ? 1 : 0;
		break;

	case 8:
		ret = val1 + val2;
		break;

	case 9:
		ret = val2 - val1;
		break;

	case 10:
		ret = val1 * val2;
		break;

	case 11:
		ret = val2 / val1;
		break;

	case 12:
		ret = val2 >> val1;
		break;

	case 13:
		ret = val2 << val1;
		break;

	case 14:
		ret = val1 & val2;
		break;

	case 15:
		ret = val1 | val2;
		break;

	case 16:
		ret = val2 % val1;
		break;

	case 17:
		ret = val1 ^ val2;
		break;

	default:
		warning("Unknown evaluate func: %d", _parameter);
		error = true;
	}

	if (error)
		script->ip = 0;
	else
		script->stack[--script->sp] = ret;
}

void EMCInterpreter::op_setRetAndJmp(EMCState *script) {
	if (script->sp >= EMCState::kStackLastEntry) {
		script->ip = 0;
	} else {
		script->retValue = script->stack[script->sp++];
		uint16 temp = script->stack[script->sp++];
		script->stack[EMCState::kStackLastEntry] = 0;
		script->ip = &script->dataPtr->data[temp];
	}
}
} // End of namespace Kyra
