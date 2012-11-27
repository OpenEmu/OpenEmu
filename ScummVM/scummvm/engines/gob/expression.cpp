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

#include "common/endian.h"

#include "gob/gob.h"
#include "gob/expression.h"
#include "gob/global.h"
#include "gob/game.h"
#include "gob/script.h"
#include "gob/inter.h"

namespace Gob {

Expression::Stack::Stack(size_t size) {
	opers  = new byte[size];
	values = new int32[size];
	memset(opers , 0, size * sizeof(byte ));
	memset(values, 0, size * sizeof(int32));
}

Expression::Stack::~Stack() {
	delete[] opers;
	delete[] values;
}

Expression::StackFrame::StackFrame(const Stack &stack) {
	opers = stack.opers - 1;
	values = stack.values - 1;
	pos = -1;
}

void Expression::StackFrame::push(int count) {
	opers  += count;
	values += count;
	pos    += count;
}

void Expression::StackFrame::pop(int count) {
	opers  -= count;
	values -= count;
	pos    -= count;
}

Expression::Expression(GobEngine *vm) : _vm(vm) {
	_resultStr[0] = 0;
	_resultInt = 0;
}

int32 Expression::encodePtr(byte *ptr, int type) {
	int32 offset = 0;

	switch (type) {
	case kExecPtr:
		offset = _vm->_game->_script->getOffset(ptr);
		break;
	case kInterVar:
		offset = ptr - ((byte *)_vm->_inter->_variables->getAddressOff8(0));
		break;
	case kResStr:
		offset = ptr - ((byte *)_resultStr);
		break;
	default:
		error("Expression::encodePtr(): Unknown pointer type");
	}
	assert((offset & 0xF0000000) == 0);
	return (type << 28) | offset;
}

byte *Expression::decodePtr(int32 n) {
	byte *ptr;

	switch (n >> 28) {
	case kExecPtr:
		return _vm->_game->_script->getData((n & 0x0FFFFFFF));
	case kInterVar:
		ptr = (byte *)_vm->_inter->_variables->getAddressOff8(0);
		break;
	case kResStr:
		ptr = (byte *)_resultStr;
		break;
	default:
		error("Expression::decodePtr(): Unknown pointer type");
	}
	return ptr + (n & 0x0FFFFFFF);
}

void Expression::skipExpr(char stopToken) {
	int16 dimCount;
	byte operation;
	int16 num;
	int16 dim;

	num = 0;
	while (true) {
		operation = _vm->_game->_script->readByte();

		if ((operation >= 14) && (operation <= OP_FUNC)) {
			switch (operation) {
			case 14:
				_vm->_game->_script->skip(4);
				if (_vm->_game->_script->peekByte() == 97)
					_vm->_game->_script->skip(1);
				break;

			case OP_LOAD_VAR_INT16:
			case OP_LOAD_VAR_INT8:
			case OP_LOAD_IMM_INT16:
			case OP_LOAD_VAR_INT32:
			case OP_LOAD_VAR_INT32_AS_INT16:
				_vm->_game->_script->skip(2);
				break;

			case OP_LOAD_IMM_INT32:
				_vm->_game->_script->skip(4);
				break;

			case OP_LOAD_IMM_INT8:
				_vm->_game->_script->skip(1);
				break;

			case OP_LOAD_IMM_STR:
				_vm->_game->_script->skip(strlen(_vm->_game->_script->peekString()) + 1);
				break;

			case OP_LOAD_VAR_STR:
				_vm->_game->_script->skip(2);
				if (_vm->_game->_script->peekByte() == 13) {
					_vm->_game->_script->skip(1);
					skipExpr(OP_END_MARKER);
				}
				break;

			case 15:
				_vm->_game->_script->skip(2);

			case OP_ARRAY_INT8:
			case OP_ARRAY_INT32:
			case OP_ARRAY_INT16:
			case OP_ARRAY_STR:
				dimCount = _vm->_game->_script->peekByte(2);
				// skip header and dimensions
				_vm->_game->_script->skip(3 + dimCount);
				// skip indices
				for (dim = 0; dim < dimCount; dim++)
					skipExpr(OP_END_MARKER);

				if ((operation == OP_ARRAY_STR) && (_vm->_game->_script->peekByte() == 13)) {
					_vm->_game->_script->skip(1);
					skipExpr(OP_END_MARKER);
				}
				break;

			case OP_FUNC:
				_vm->_game->_script->skip(1);
				skipExpr(OP_END_EXPR);
			}
			continue;
		} // if ((operation >= OP_ARRAY_INT8) && (operation <= OP_FUNC))

		if (operation == OP_BEGIN_EXPR) {
			num++;
			continue;
		}

		if ((operation == OP_NOT) || ((operation >= OP_NEG) && (operation <= 8)))
			continue;

		if ((operation >= OP_OR) && (operation <= OP_NEQ))
			continue;

		if (operation == OP_END_EXPR)
			num--;

		if (operation != stopToken)
			continue;

		if ((stopToken != OP_END_EXPR) || (num < 0))
			return;
	}
}

void Expression::printExpr(char stopToken) {
	// Expression printing disabled by default
#if 0

	int32 savedPos = _vm->_game->_script->pos();
	printExpr_internal(stopToken);

	// restore IP to start of expression
	_vm->_game->_script->seek(savedPos);
#endif
}

void Expression::printExpr_internal(char stopToken) {
	int16 dimCount;
	byte operation;
	int16 num;
	int16 dim;
	byte *arrDesc;
	byte func;

	num = 0;
	while (true) {
		operation = _vm->_game->_script->readByte();

		if ((operation >= OP_ARRAY_INT8) && (operation <= OP_FUNC)) {
			// operands

			switch (operation) {
			case OP_LOAD_VAR_INT16: // int16 variable load
				debugN(5, "var16_%d", _vm->_game->_script->readUint16());
				break;

			case OP_LOAD_VAR_INT8: // int8 variable load:
				debugN(5, "var8_%d", _vm->_game->_script->readUint16());
				break;

			case OP_LOAD_IMM_INT32: // int32/uint32 immediate
				debugN(5, "%d", _vm->_game->_script->readInt32());
				break;

			case OP_LOAD_IMM_INT16: // int16 immediate
				debugN(5, "%d", _vm->_game->_script->readInt16());
				break;

			case OP_LOAD_IMM_INT8: // int8 immediate
				debugN(5, "%d",  _vm->_game->_script->readInt8());
				break;

			case OP_LOAD_IMM_STR: // string immediate
				debugN(5, "\42%s\42", _vm->_game->_script->readString());
				break;

			case OP_LOAD_VAR_INT32:
			case OP_LOAD_VAR_INT32_AS_INT16:
				debugN(5, "var_%d", _vm->_game->_script->readUint16());
				break;

			case OP_LOAD_VAR_STR: // string variable load
				debugN(5, "(&var_%d)", _vm->_game->_script->readUint16());
				if (_vm->_game->_script->peekByte() == 13) {
					_vm->_game->_script->skip(1);
					debugN(5, "{");
					printExpr_internal(OP_END_MARKER); // this also prints the closing }
				}
				break;

			case OP_ARRAY_INT8:  // int8 array access
			case OP_ARRAY_INT32: // int32 array access
			case OP_ARRAY_INT16: // int16 array access
			case OP_ARRAY_STR:   // string array access
				debugN(5, "\n");
				if (operation == OP_ARRAY_STR)
					debugN(5, "(&");

				debugN(5, "var_%d[", _vm->_game->_script->readInt16());
				dimCount = _vm->_game->_script->readByte();
				arrDesc = _vm->_game->_script->getData() + _vm->_game->_script->pos();
				_vm->_game->_script->skip(dimCount);
				for (dim = 0; dim < dimCount; dim++) {
					printExpr_internal(OP_END_MARKER);
					debugN(5, " of %d", (int16) arrDesc[dim]);
					if (dim != dimCount - 1)
						debugN(5, ",");
				}
				debugN(5, "]");
				if (operation == OP_ARRAY_STR)
					debugN(5, ")");

				if ((operation == OP_ARRAY_STR) && (_vm->_game->_script->peekByte() == 13)) {
					_vm->_game->_script->skip(1);
					debugN(5, "{");
					printExpr_internal(OP_END_MARKER); // this also prints the closing }
				}
				break;

			case OP_FUNC: // function
				func = _vm->_game->_script->readByte();
				if (func == FUNC_SQR)
					debugN(5, "sqr(");
				else if (func == FUNC_RAND)
					debugN(5, "rand(");
				else if (func == FUNC_ABS)
					debugN(5, "abs(");
				else if ((func == FUNC_SQRT1) || (func == FUNC_SQRT2) || (func == FUNC_SQRT3))
					debugN(5, "sqrt(");
				else
					debugN(5, "id(");
				printExpr_internal(OP_END_EXPR);
				break;
			}
			continue;
		}		// if ((operation >= OP_ARRAY_INT8) && (operation <= OP_FUNC))

		// operators
		switch (operation) {
		case OP_BEGIN_EXPR:
			debugN(5, "(");
			break;

		case OP_NOT:
			debugN(5, "!");
			break;

		case OP_END_EXPR:
			debugN(5, ")");
			break;

		case OP_NEG:
			debugN(5, "-");
			break;

		case OP_ADD:
			debugN(5, "+");
			break;

		case OP_SUB:
			debugN(5, "-");
			break;

		case OP_BITOR:
			debugN(5, "|");
			break;

		case OP_MUL:
			debugN(5, "*");
			break;

		case OP_DIV:
			debugN(5, "/");
			break;

		case OP_MOD:
			debugN(5, "%%");
			break;

		case OP_BITAND:
			debugN(5, "&");
			break;

		case OP_OR:
			debugN(5, "||");
			break;

		case 31:
			debugN(5, "&&");
			break;

		case OP_LESS:
			debugN(5, "<");
			break;

		case OP_LEQ:
			debugN(5, "<=");
			break;

		case OP_GREATER:
			debugN(5, ">");
			break;

		case OP_GEQ:
			debugN(5, ">=");
			break;

		case OP_EQ:
			debugN(5, "==");
			break;

		case OP_NEQ:
			debugN(5, "!=");
			break;

		case 99:
			debugN(5, "\n");
			break;

		case OP_END_MARKER:
			debugN(5, "}");
			if (stopToken != OP_END_MARKER) {
				debugN(5, "Closing paren without opening?");
			}
			break;

		default:
			debugN(5, "<%d>", (int16) operation);
			error("Expression::printExpr(): invalid operator in expression");
			break;
		}

		if (operation == OP_BEGIN_EXPR) {
			num++;
			continue;
		}

		if ((operation == OP_NOT) || ((operation >= OP_NEG) && (operation <= 8)))
			continue;

		if ((operation >= OP_OR) && (operation <= OP_NEQ))
			continue;

		if (operation == OP_END_EXPR)
			num--;

		if (operation == stopToken) {
			if ((stopToken != OP_END_EXPR) || (num < 0)) {
				return;
			}
		}
	}
}


void Expression::printVarIndex() {
	byte *arrDesc;
	int16 dim;
	int16 dimCount;
	int16 operation;
	int16 temp;

	int32 pos = _vm->_game->_script->pos();

	operation = _vm->_game->_script->readByte();
	switch (operation) {
	case OP_LOAD_VAR_INT32:
	case OP_LOAD_VAR_STR:
		temp = _vm->_game->_script->readUint16() * 4;
		debugN(5, "&var_%d", temp);
		if ((operation == OP_LOAD_VAR_STR) && (_vm->_game->_script->peekByte() == 13)) {
			_vm->_game->_script->skip(1);
			debugN(5, "+");
			printExpr(OP_END_MARKER);
		}
		break;

	case OP_ARRAY_INT32:
	case OP_ARRAY_STR:
		debugN(5, "&var_%d[", _vm->_game->_script->readUint16());
		dimCount = _vm->_game->_script->readByte();
		arrDesc = _vm->_game->_script->getData() + _vm->_game->_script->pos();
		_vm->_game->_script->skip(dimCount);
		for (dim = 0; dim < dimCount; dim++) {
			printExpr(OP_END_MARKER);
			debugN(5, " of %d", (int16) arrDesc[dim]);
			if (dim != dimCount - 1)
				debugN(5, ",");
		}
		debugN(5, "]");

		if ((operation == OP_ARRAY_STR) && (_vm->_game->_script->peekByte() == 13)) {
			_vm->_game->_script->skip(1);
			debugN(5, "+");
			printExpr(OP_END_MARKER);
		}
		break;

	default:
		debugN(5, "var_0");
		break;
	}
	debugN(5, "\n");

	_vm->_game->_script->seek(pos);
	return;
}

int Expression::cmpHelper(const StackFrame &stackFrame) {
	byte type = stackFrame.opers[-3];
	int cmpTemp = 0;

	if (type == OP_LOAD_IMM_INT16) {
		cmpTemp = (int)stackFrame.values[-3] - (int)stackFrame.values[-1];
	} else if (type == OP_LOAD_IMM_STR) {
		if ((char *)decodePtr(stackFrame.values[-3]) != _resultStr) {
			strcpy(_resultStr, (char *)decodePtr(stackFrame.values[-3]));
			stackFrame.values[-3] = encodePtr((byte *)_resultStr, kResStr);
		}
		cmpTemp = strcmp(_resultStr, (char *)decodePtr(stackFrame.values[-1]));
	}

	return cmpTemp;
}

bool Expression::getVarBase(uint32 &varBase, bool mindStop,
		uint16 *size, uint16 *type) {

	varBase = 0;

	byte operation = _vm->_game->_script->peekByte();
	while ((operation == 14) || (operation == 15)) {
		_vm->_game->_script->skip(1);

		if (operation == 14) {
			// Add a direct offset

			varBase += _vm->_game->_script->readInt16() * 4;

			if (size)
				*size = _vm->_game->_script->peekUint16();
			if (type)
				*type = 14;

			_vm->_game->_script->skip(2);

			debugC(2, kDebugExpression, "varBase: %d, by %d", varBase, operation);

			if (_vm->_game->_script->peekByte() != 97) {
				if (mindStop)
					return true;
			} else
				_vm->_game->_script->skip(1);

		} else if (operation == 15) {
			// Add an offset from an array

			varBase += _vm->_game->_script->readInt16() * 4;

			uint16 offset1 = _vm->_game->_script->readUint16();

			if (size)
				*size = offset1;
			if (type)
				*type = 15;

			uint8 dimCount = _vm->_game->_script->readByte();
			byte *dimArray = _vm->_game->_script->getData() + _vm->_game->_script->pos();

			_vm->_game->_script->skip(dimCount);

			uint16 offset2 = 0;
			for (int i = 0; i < dimCount; i++) {
				int16 dim = CLIP<int>(parseValExpr(OP_END_MARKER), 0, dimArray[i] - 1);

				offset2 = offset2 * dimArray[i] + dim;
			}

			varBase += offset2 * offset1 * 4;

			debugC(2, kDebugExpression, "varBase: %d, by %d", varBase, operation);

			if (_vm->_game->_script->peekByte() != 97) {
				if (mindStop)
					return true;
			} else
				_vm->_game->_script->skip(1);
		}

		operation = _vm->_game->_script->peekByte();
	}

	return false;
}

int16 Expression::parseVarIndex(uint16 *size, uint16 *type) {
	int16 temp2;
	byte *arrDesc;
	int16 dim;
	int16 dimCount;
	int16 operation;
	int16 temp;
	int16 offset;
	int16 val;
	uint32 varBase;

	if (getVarBase(varBase, true, size, type))
		return varBase;

	operation = _vm->_game->_script->readByte();

	if (size)
		*size = 0;
	if (type)
		*type = operation;

	debugC(5, kDebugExpression, "var parse = %d", operation);
	switch (operation) {
	case OP_ARRAY_INT8:
	case OP_ARRAY_INT32:
	case OP_ARRAY_INT16:
	case OP_ARRAY_STR:
		temp = _vm->_game->_script->readInt16();
		dimCount = _vm->_game->_script->readByte();
		arrDesc = _vm->_game->_script->getData() + _vm->_game->_script->pos();
		_vm->_game->_script->skip(dimCount);
		offset = 0;
		for (dim = 0; dim < dimCount; dim++) {
			temp2 = parseValExpr(OP_END_MARKER);
			offset = arrDesc[dim] * offset + temp2;
		}
		if (operation == OP_ARRAY_INT8)
			return varBase + temp + offset;
		if (operation == OP_ARRAY_INT32)
			return varBase + (temp + offset) * 4;
		if (operation == OP_ARRAY_INT16)
			return varBase + (temp + offset) * 2;
		temp *= 4;
		offset *= 4;
		if (_vm->_game->_script->peekByte() == 13) {
			_vm->_game->_script->skip(1);
			temp += parseValExpr(OP_END_MARKER);
		}
		return varBase + offset * _vm->_global->_inter_animDataSize + temp;

	case OP_LOAD_VAR_INT16:
		return varBase + _vm->_game->_script->readUint16() * 2;

	case OP_LOAD_VAR_INT8:
		return varBase + _vm->_game->_script->readUint16();

	case OP_LOAD_VAR_INT32:
	case OP_LOAD_VAR_INT32_AS_INT16:
	case OP_LOAD_VAR_STR:
		temp = _vm->_game->_script->readUint16() * 4;
		debugC(5, kDebugExpression, "oper = %d", _vm->_game->_script->peekInt16());
		if ((operation == OP_LOAD_VAR_STR) && (_vm->_game->_script->peekByte() == 13)) {
			_vm->_game->_script->skip(1);
			val = parseValExpr(OP_END_MARKER);
			temp += val;
			debugC(5, kDebugExpression, "parse subscript = %d", val);
		}
		return varBase + temp;

	default:
		return 0;
	}
}

int16 Expression::parseValExpr(byte stopToken) {
	parseExpr(stopToken, 0);

	return _resultInt;
}

// Load a value according to the operation
void Expression::loadValue(byte operation, uint32 varBase, const StackFrame &stackFrame) {
	int16 dimCount;
	uint16 temp;
	int16 temp2;
	int16 offset;
	int16 dim;
	byte *arrDescPtr;
	int32 prevPrevVal;
	int32 prevVal;
	int32 curVal;

	switch (operation) {
	case OP_ARRAY_INT8:
	case OP_ARRAY_INT32:
	case OP_ARRAY_INT16:
	case OP_ARRAY_STR:
		*stackFrame.opers = (operation == OP_ARRAY_STR) ? OP_LOAD_IMM_STR : OP_LOAD_IMM_INT16;
		temp = _vm->_game->_script->readInt16();
		dimCount = _vm->_game->_script->readByte();
		arrDescPtr = _vm->_game->_script->getData() + _vm->_game->_script->pos();
		_vm->_game->_script->skip(dimCount);
		offset = 0;
		for (dim = 0; dim < dimCount; dim++) {
			temp2 = parseValExpr(OP_END_MARKER);
			offset = offset * arrDescPtr[dim] + temp2;
		}
		if (operation == OP_ARRAY_INT8)
			*stackFrame.values = (int8) READ_VARO_UINT8(varBase + temp + offset);
		else if (operation == OP_ARRAY_INT32)
			*stackFrame.values = READ_VARO_UINT32(varBase + temp * 4 + offset * 4);
		else if (operation == OP_ARRAY_INT16)
			*stackFrame.values = (int16) READ_VARO_UINT16(varBase + temp * 2 + offset * 2);
		else if (operation == OP_ARRAY_STR) {
			*stackFrame.values = encodePtr(_vm->_inter->_variables->getAddressOff8(
						varBase + temp * 4 + offset * _vm->_global->_inter_animDataSize * 4),
					kInterVar);
			if (_vm->_game->_script->peekByte() == 13) {
				_vm->_game->_script->skip(1);
				temp2 = parseValExpr(OP_END_MARKER);
				*stackFrame.opers = OP_LOAD_IMM_INT16;
				*stackFrame.values = READ_VARO_UINT8(varBase + temp * 4 +
						offset * 4 * _vm->_global->_inter_animDataSize + temp2);
			}
		}
		break;

	case OP_LOAD_VAR_INT16:
		*stackFrame.opers = OP_LOAD_IMM_INT16;
		*stackFrame.values = (int16) READ_VARO_UINT16(varBase + _vm->_game->_script->readUint16() * 2);
		break;

	case OP_LOAD_VAR_INT8:
		*stackFrame.opers = OP_LOAD_IMM_INT16;
		*stackFrame.values = (int8) READ_VARO_UINT8(varBase + _vm->_game->_script->readUint16());
		break;

	case OP_LOAD_IMM_INT32:
		*stackFrame.opers = OP_LOAD_IMM_INT16;
		*stackFrame.values = _vm->_game->_script->readInt32();
		break;

	case OP_LOAD_IMM_INT16:
		*stackFrame.opers = OP_LOAD_IMM_INT16;
		*stackFrame.values = _vm->_game->_script->readInt16();
		break;

	case OP_LOAD_IMM_INT8:
		*stackFrame.opers = OP_LOAD_IMM_INT16;
		*stackFrame.values = _vm->_game->_script->readInt8();
		break;

	case OP_LOAD_IMM_STR:
		*stackFrame.opers = OP_LOAD_IMM_STR;
		*stackFrame.values = encodePtr((byte *)_vm->_game->_script->readString(), kExecPtr);
		break;

	case OP_LOAD_VAR_INT32:
		*stackFrame.opers = OP_LOAD_IMM_INT16;
		*stackFrame.values = READ_VARO_UINT32(varBase + _vm->_game->_script->readUint16() * 4);
		break;

	case OP_LOAD_VAR_INT32_AS_INT16:
		*stackFrame.opers = OP_LOAD_IMM_INT16;
		*stackFrame.values = (int16) READ_VARO_UINT16(varBase + _vm->_game->_script->readUint16() * 4);
		break;

	case OP_LOAD_VAR_STR:
		*stackFrame.opers = OP_LOAD_IMM_STR;
		temp = _vm->_game->_script->readUint16() * 4;
		*stackFrame.values = encodePtr(_vm->_inter->_variables->getAddressOff8(varBase + temp), kInterVar);
		if (_vm->_game->_script->peekByte() == 13) {
			_vm->_game->_script->skip(1);
			temp += parseValExpr(OP_END_MARKER);
			*stackFrame.opers = OP_LOAD_IMM_INT16;
			*stackFrame.values = READ_VARO_UINT8(varBase + temp);
		}
		break;

	case OP_FUNC:
		operation = _vm->_game->_script->readByte();
		parseExpr(OP_END_EXPR, 0);

		switch (operation) {
		case FUNC_SQRT1:
		case FUNC_SQRT2:
		case FUNC_SQRT3:
			curVal = 1;
			prevVal = 1;

			do {
				prevPrevVal = prevVal;
				prevVal = curVal;
				curVal = (curVal + _resultInt / curVal) / 2;
			} while ((curVal != prevVal) && (curVal != prevPrevVal));
			_resultInt = curVal;
			break;

		case FUNC_SQR:
			_resultInt =
				_resultInt * _resultInt;
			break;

		case FUNC_ABS:
			if (_resultInt < 0)
				_resultInt = -_resultInt;
			break;

		case FUNC_RAND:
			_resultInt =
				_vm->_util->getRandom(_resultInt);
			break;
		}

		*stackFrame.opers = OP_LOAD_IMM_INT16;
		*stackFrame.values = _resultInt;
		break;
	}
}

void Expression::simpleArithmetic1(StackFrame &stackFrame) {
	switch (stackFrame.opers[-1]) {
	case OP_ADD:
		if (stackFrame.opers[-2] == OP_LOAD_IMM_STR) {
			if ((char *)decodePtr(stackFrame.values[-2]) != _resultStr) {
				strcpy(_resultStr, (char *)decodePtr(stackFrame.values[-2]));
				stackFrame.values[-2] = encodePtr((byte *)_resultStr, kResStr);
			}
			strcat(_resultStr, (char *)decodePtr(stackFrame.values[0]));
			stackFrame.pop(2);
		}
		break;

	case OP_MUL:
		stackFrame.values[-2] *= stackFrame.values[0];
		stackFrame.pop(2);
		break;

	case OP_DIV:
		stackFrame.values[-2] /= stackFrame.values[0];
		stackFrame.pop(2);
		break;

	case OP_MOD:
		stackFrame.values[-2] %= stackFrame.values[0];
		stackFrame.pop(2);
		break;

	case OP_BITAND:
		stackFrame.values[-2] &= stackFrame.values[0];
		stackFrame.pop(2);
		break;
	}
}

void Expression::simpleArithmetic2(StackFrame &stackFrame) {
	if (stackFrame.pos > 1) {
		if (stackFrame.opers[-2] == OP_NEG) {
			stackFrame.opers[-2] = OP_LOAD_IMM_INT16;
			stackFrame.values[-2] = -stackFrame.values[-1];
			stackFrame.pop();
		} else if (stackFrame.opers[-2] == OP_NOT) {
			stackFrame.opers[-2] = (stackFrame.opers[-1] == GOB_FALSE) ? GOB_TRUE : GOB_FALSE;
			stackFrame.pop();
		}
	}

	if (stackFrame.pos > 2) {
		switch (stackFrame.opers[-2]) {
		case OP_MUL:
			stackFrame.values[-3] *= stackFrame.values[-1];
			stackFrame.pop(2);
			break;

		case OP_DIV:
			stackFrame.values[-3] /= stackFrame.values[-1];
			stackFrame.pop(2);
			break;

		case OP_MOD:
			stackFrame.values[-3] %= stackFrame.values[-1];
			stackFrame.pop(2);
			break;

		case OP_BITAND:
			stackFrame.values[-3] &= stackFrame.values[-1];
			stackFrame.pop(2);
			break;
		}
	}

}

// Complex arithmetics with brackets
bool Expression::complexArithmetic(Stack &stack, StackFrame &stackFrame, int16 brackStart) {
	switch (stackFrame.opers[-2]) {
	case OP_ADD:
		if (stack.opers[brackStart] == OP_LOAD_IMM_INT16) {
			stack.values[brackStart] += stackFrame.values[-1];
		} else if (stack.opers[brackStart] == OP_LOAD_IMM_STR) {
			if ((char *)decodePtr(stack.values[brackStart]) != _resultStr) {
				strcpy(_resultStr, (char *)decodePtr(stack.values[brackStart]));
				stack.values[brackStart] =
					encodePtr((byte *)_resultStr, kResStr);
			}
			strcat(_resultStr, (char *)decodePtr(stackFrame.values[-1]));
		}
		stackFrame.pop(2);
		break;

	case OP_SUB:
		stack.values[brackStart] -= stackFrame.values[-1];
		stackFrame.pop(2);
		break;

	case OP_BITOR:
		stack.values[brackStart] |= stackFrame.values[-1];
		stackFrame.pop(2);
		break;

	case OP_MUL:
		stackFrame.values[-3] *= stackFrame.values[-1];
		stackFrame.pop(2);
		break;

	case OP_DIV:
		stackFrame.values[-3] /= stackFrame.values[-1];
		stackFrame.pop(2);
		break;

	case OP_MOD:
		stackFrame.values[-3] %= stackFrame.values[-1];
		stackFrame.pop(2);
		break;

	case OP_BITAND:
		stackFrame.values[-3] &= stackFrame.values[-1];
		stackFrame.pop(2);
		break;

	case OP_OR:
		// (x OR false) == x
		// (x OR true) == true
		if (stackFrame.opers[-3] == GOB_FALSE)
			stackFrame.opers[-3] = stackFrame.opers[-1];
		stackFrame.pop(2);
		break;

	case OP_AND:
		// (x AND false) == false
		// (x AND true) == x
		if (stackFrame.opers[-3] == GOB_TRUE)
			stackFrame.opers[-3] = stackFrame.opers[-1];
		stackFrame.pop(2);
		break;

	case OP_LESS:
		stackFrame.opers[-3] = (cmpHelper(stackFrame) < 0) ? GOB_TRUE : GOB_FALSE;
		stackFrame.pop(2);
		break;

	case OP_LEQ:
		stackFrame.opers[-3] = (cmpHelper(stackFrame) <= 0) ? GOB_TRUE : GOB_FALSE;
		stackFrame.pop(2);
		break;

	case OP_GREATER:
		stackFrame.opers[-3] = (cmpHelper(stackFrame) > 0) ? GOB_TRUE : GOB_FALSE;
		stackFrame.pop(2);
		break;

	case OP_GEQ:
		stackFrame.opers[-3] = (cmpHelper(stackFrame) >= 0) ? GOB_TRUE : GOB_FALSE;
		stackFrame.pop(2);
		break;

	case OP_EQ:
		stackFrame.opers[-3] = (cmpHelper(stackFrame) == 0) ? GOB_TRUE : GOB_FALSE;
		stackFrame.pop(2);
		break;

	case OP_NEQ:
		stackFrame.opers[-3] = (cmpHelper(stackFrame) != 0) ? GOB_TRUE : GOB_FALSE;
		stackFrame.pop(2);
		break;

	default:
		return true;
	}

	return false;
}

// Assign the result to the appropriate _result variable
void Expression::getResult(byte operation, int32 value, byte *type) {
	if (type != 0)
		*type = operation;

	switch (operation) {
	case OP_NOT:
		if (type != 0)
			*type ^= 1;
		break;

	case OP_LOAD_IMM_INT16:
		_resultInt = value;
		break;

	case OP_LOAD_IMM_STR:
		if ((char *)decodePtr(value) != _resultStr)
			strcpy(_resultStr, (char *)decodePtr(value));
		break;

	case OP_LOAD_VAR_INT32:
	case OP_LOAD_VAR_INT32_AS_INT16:
		break;

	default:
		_resultInt = 0;
		if (type != 0)
			*type = OP_LOAD_IMM_INT16;
		break;
	}
}

int16 Expression::parseExpr(byte stopToken, byte *type) {
	Stack stack;
	StackFrame stackFrame(stack);
	byte operation;
	int16 brackStart;
	uint32 varBase;

	while (true) {
		getVarBase(varBase);

		stackFrame.push();

		operation = _vm->_game->_script->readByte();
		if ((operation >= OP_ARRAY_INT8) && (operation <= OP_FUNC)) {

			loadValue(operation, varBase, stackFrame);

			if ((stackFrame.pos > 0) && ((stackFrame.opers[-1] == OP_NEG) || (stackFrame.opers[-1] == OP_NOT))) {
				stackFrame.pop();

				if (*stackFrame.opers == OP_NEG) {
					*stackFrame.opers = OP_LOAD_IMM_INT16;
					stackFrame.values[0] = -stackFrame.values[1];
				} else
					*stackFrame.opers = (stackFrame.opers[1] == GOB_FALSE) ? GOB_TRUE : GOB_FALSE;
			}

			if (stackFrame.pos <= 0)
				continue;

			simpleArithmetic1(stackFrame);

			continue;
		} // (op >= OP_ARRAY_INT8) && (op <= OP_FUNC)

		if ((operation == stopToken) || (operation == OP_OR) ||
				(operation == OP_AND) || (operation == OP_END_EXPR)) {
			while (stackFrame.pos >= 2) {
				if ((stackFrame.opers[-2] == OP_BEGIN_EXPR) &&
						((operation == OP_END_EXPR) || (operation == stopToken))) {
					stackFrame.opers[-2] = stackFrame.opers[-1];
					if ((stackFrame.opers[-2] == OP_LOAD_IMM_INT16) || (stackFrame.opers[-2] == OP_LOAD_IMM_STR))
						stackFrame.values[-2] = stackFrame.values[-1];

					stackFrame.pop();

					simpleArithmetic2(stackFrame);

					if (operation != stopToken)
						break;
				}	// if ((stackFrame.opers[-2] == OP_BEGIN_EXPR) && ...)

				for (brackStart = (stackFrame.pos - 2); (brackStart > 0) &&
				    (stack.opers[brackStart] < OP_OR) && (stack.opers[brackStart] != OP_BEGIN_EXPR);
						brackStart--)
					;

				if ((stack.opers[brackStart] >= OP_OR) || (stack.opers[brackStart] == OP_BEGIN_EXPR))
					brackStart++;

				if (complexArithmetic(stack, stackFrame, brackStart))
					break;

			}	// while (stackFrame.pos >= 2)

			if ((operation == OP_OR) || (operation == OP_AND)) {
				if (stackFrame.opers[-1] == OP_LOAD_IMM_INT16) {
					if (stackFrame.values[-1] != 0)
						stackFrame.opers[-1] = GOB_TRUE;
					else
						stackFrame.opers[-1] = GOB_FALSE;
				}

				if (((operation == OP_OR) && (stackFrame.opers[-1] == GOB_TRUE)) ||
				    ((operation == OP_AND) && (stackFrame.opers[-1] == GOB_FALSE))) {
					if ((stackFrame.pos > 1) && (stackFrame.opers[-2] == OP_BEGIN_EXPR)) {
						skipExpr(OP_END_EXPR);
						stackFrame.opers[-2] = stackFrame.opers[-1];
						stackFrame.pop(2);
					} else {
						skipExpr(stopToken);
					}
					operation = _vm->_game->_script->peekByte(-1);
					if ((stackFrame.pos > 0) && (stackFrame.opers[-1] == OP_NOT)) {
						if (stackFrame.opers[0] == GOB_FALSE)
							stackFrame.opers[-1] = GOB_TRUE;
						else
							stackFrame.opers[-1] = GOB_FALSE;

						stackFrame.pop();
					}
				} else
					stackFrame.opers[0] = operation;
			} else
				stackFrame.pop();

			if (operation != stopToken)
				continue;

			getResult(stack.opers[0], stack.values[0], type);

			return 0;
		}		// (operation == stopToken) || (operation == OP_OR) || (operation == OP_AND) || (operation == OP_END_EXPR)

		if ((operation < OP_NEG) || (operation > OP_NOT)) {
			if ((operation < OP_LESS) || (operation > OP_NEQ))
				continue;

			if (stackFrame.pos > 2) {
				if (stackFrame.opers[-2] == OP_ADD) {
					if (stackFrame.opers[-3] == OP_LOAD_IMM_INT16) {
						stackFrame.values[-3] += stackFrame.values[-1];
					} else if (stackFrame.opers[-3] == OP_LOAD_IMM_STR) {
						if ((char *)decodePtr(stackFrame.values[-3]) != _resultStr) {
							strcpy(_resultStr, (char *)decodePtr(stackFrame.values[-3]));
							stackFrame.values[-3] = encodePtr((byte *)_resultStr, kResStr);
						}
						strcat(_resultStr, (char *)decodePtr(stackFrame.values[-1]));
					}
					stackFrame.pop(2);

				} else if (stackFrame.opers[-2] == OP_SUB) {
					stackFrame.values[-3] -= stackFrame.values[-1];
					stackFrame.pop(2);
				} else if (stackFrame.opers[-2] == OP_BITOR) {
					stackFrame.values[-3] |= stackFrame.values[-1];
					stackFrame.pop(2);
				}
			}
		}
		*stackFrame.opers = operation;
	}
}

int32 Expression::getResultInt() {
	return _resultInt;
}

char *Expression::getResultStr() {
	return _resultStr;
}

} // End of namespace Gob
