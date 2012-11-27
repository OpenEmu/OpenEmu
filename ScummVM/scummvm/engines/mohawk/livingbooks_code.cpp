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

#include "mohawk/livingbooks.h"
#include "mohawk/livingbooks_lbx.h"
#include "mohawk/resource.h"

#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"

namespace Mohawk {

bool LBValue::operator==(const LBValue &x) const {
	if (type != x.type) {
		if (isNumeric() && x.isNumeric())
			return toDouble() == x.toDouble();
		else if (type == kLBValueString && x.type == kLBValueItemPtr)
			return string == x.item->getName();
		else if (type == kLBValueItemPtr && x.type == kLBValueString)
			return item->getName() == x.string;
		else
			return false;
	}

	switch (type) {
	case kLBValueString:
		return string == x.string;
	case kLBValueInteger:
		return integer == x.integer;
	case kLBValueReal:
		return real == x.real;
	case kLBValuePoint:
		return point == x.point;
	case kLBValueRect:
		return rect == x.rect;
	case kLBValueItemPtr:
		return item == x.item;
	default:
		error("Unknown type when testing for equality");
	}
}

bool LBValue::operator!=(const LBValue &x) const {
	return !(*this == x);
}

bool LBValue::isNumeric() const {
	if (type == kLBValueInteger || type == kLBValueReal)
		return true;

	// TODO: string checks

	return false;
}

bool LBValue::isZero() const {
	return toInt() == 0; // FIXME
}

Common::String LBValue::toString() const {
	switch (type) {
	case kLBValueString:
		return string;
	case kLBValueInteger:
		return Common::String::format("%d", integer);
	case kLBValueReal:
		return Common::String::format("%f", real);
	default:
		return string; // FIXME
	}
}

int LBValue::toInt() const {
	return integer; // FIXME
}

double LBValue::toDouble() const {
	return real; // FIXME
}

Common::Point LBValue::toPoint() const {
	switch (type) {
	case kLBValueString:
		{
		Common::Point ret;
		sscanf(string.c_str(), "%hd , %hd", &ret.x, &ret.y);
		return ret;
		}
	case kLBValueInteger:
		return Common::Point(integer, integer);
	case kLBValuePoint:
		return point;
	case kLBValueRect:
		return Common::Point(rect.left, rect.top);
	default:
		error("failed to convert to point");
	}
}

Common::Rect LBValue::toRect() const {
	switch (type) {
	case kLBValueString:
		{
		Common::Rect ret;
		sscanf(string.c_str(), "%hd , %hd , %hd , %hd", &ret.left, &ret.top, &ret.right, &ret.bottom);
		return ret;
		}
	case kLBValueInteger:
		return Common::Rect(integer, integer, integer, integer);
	case kLBValuePoint:
		return Common::Rect(point.x, point.y, point.x, point.y);
	case kLBValueRect:
		return rect;
	case kLBValueItemPtr:
		return item->getRect();
	default:
		error("failed to convert to rect");
	}
}

LBCode::LBCode(MohawkEngine_LivingBooks *vm, uint16 baseId) : _vm(vm) {
	if (!baseId) {
		_data = NULL;
		_size = 0;
		return;
	}

	Common::SeekableSubReadStreamEndian *bcodStream = _vm->wrapStreamEndian(ID_BCOD, baseId);

	uint32 totalSize = bcodStream->readUint32();
	if (totalSize != (uint32)bcodStream->size())
		error("BCOD had size %d, but claimed to be of size %d", bcodStream->size(), totalSize);
	_size = bcodStream->readUint32();
	if (_size + 8 > totalSize)
		error("BCOD code was of size %d, beyond size %d", _size, totalSize);

	_data = new byte[_size];
	bcodStream->read(_data, _size);

	uint16 pos = 0;
	while (bcodStream->pos() < bcodStream->size()) {
		if (bcodStream->pos() + 1 == bcodStream->size()) {
			warning("ran out of bytes while reading strings");
			break;
		}
		uint16 unknown = bcodStream->readUint16();
		if (unknown != 0) {
			warning("unknown was %04x, not zero, while reading strings", unknown);
			if (bcodStream->pos() != bcodStream->size())
				error(".. and there was more data afterwards");
			break;
		}
		Common::String string = _vm->readString(bcodStream);
		_strings[pos] = string;
		debug(2, "read '%s' from BCOD at 0x%04x", string.c_str(), pos);
		pos += 2 + string.size() + 1;
	}
}

LBCode::~LBCode() {
	delete[] _data;
}

LBValue LBCode::runCode(LBItem *src, uint32 offset) {
	// TODO: re-entrancy issues?
	_currSource = src;
	_currOffset = offset;

	return runCode(kTokenEndOfFile);
}

void LBCode::nextToken() {
	if (_currOffset >= _size) {
		error("went off the end of code");
	}

	_currToken = _data[_currOffset++];

	// We slurp any value associated with the parameter here too, to simplify things.
	switch (_currToken) {
	case kTokenIdentifier:
		{
		if (_currOffset + 2 > _size)
			error("went off the end of code reading identifier");
		uint16 offset = READ_BE_UINT16(_data + _currOffset);
		// TODO: check string exists
		_currValue = _strings[offset];
		_currOffset += 2;
		}
		break;

	case kTokenLiteral:
		{
		if (_currOffset + 1 > _size)
			error("went off the end of code reading literal");
		byte literalType = _data[_currOffset++];
		switch (literalType) {
		case kLBCodeLiteralInteger:
			if (_currOffset + 2 > _size)
				error("went off the end of code reading literal integer");
			_currValue = READ_BE_UINT16(_data + _currOffset);
			_currOffset += 2;
			break;
		default:
			error("unknown kTokenLiteral type %02x", literalType);
		}
		}
		break;

	case kTokenConstMode:
	case kTokenConstEventId:
	case 0x5e: // TODO: ??
	case kTokenKeycode:
		if (_currOffset + 2 > _size)
			error("went off the end of code reading immediate");
		_currValue = READ_BE_UINT16(_data + _currOffset);
		_currOffset += 2;
		break;

	case kTokenGeneralCommand:
	case kTokenItemCommand:
	case kTokenNotifyCommand:
	case kTokenPropListCommand:
	case kTokenRectCommand:
		_currValue = _data[_currOffset++];
		//_currValue = READ_BE_UINT16(_data + _currOffset);
		//_currOffset += 2;
		break;

	case kTokenString:
		{
		if (_currOffset + 2 > _size)
			error("went off the end of code reading string");
		uint16 offset = READ_BE_UINT16(_data + _currOffset);
		// TODO: check string exists
		_currValue = _strings[offset];
		_currOffset += 2;
		}
		break;

	default:
		_currValue = LBValue();
		break;
	}
}

LBValue LBCode::runCode(byte terminator) {
	LBValue result;

	while (true) {
		nextToken();
		if (_currToken == kTokenEndOfFile)
			break;
		parseStatement();
		if (_stack.size())
			result = _stack.pop();
		if (_currToken == terminator || _currToken == kTokenEndOfFile) {
			debugN("\n");
			break;
		}
		if (_currToken != kTokenEndOfStatement && _currToken != kTokenEndOfFile)
			error("missing EOS (got %02x)", _currToken);
		debugN("\n");
	}

	return result;
}

void LBCode::parseStatement() {
	parseComparisons();

	while (_currToken == kTokenAnd || _currToken == kTokenOr) {
		byte op = _currToken;
		if (op == kTokenAnd)
			debugN(" && ");
		else
			debugN(" || ");

		nextToken();
		parseComparisons();

		LBValue val2 = _stack.pop();
		LBValue val1 = _stack.pop();
		bool result;
		if (op == kTokenAnd)
			result = !val1.isZero() && !val2.isZero();
		else
			result = !val1.isZero() || !val2.isZero();

		debugN(" [--> %s]", result ? "true" : "false");
		_stack.push(result);
	}
}

void LBCode::parseComparisons() {
	parseConcat();

	if (_currToken != kTokenEquals && _currToken != kTokenLessThan && _currToken != kTokenGreaterThan &&
		_currToken != kTokenLessThanEq && _currToken != kTokenGreaterThanEq && _currToken != kTokenNotEq)
		return;
	byte comparison = _currToken;
	switch (comparison) {
	case kTokenEquals:
		debugN(" == ");
		break;
	case kTokenLessThan:
		debugN(" < ");
		break;
	case kTokenGreaterThan:
		debugN(" > ");
		break;
	case kTokenLessThanEq:
		debugN(" <= ");
		break;
	case kTokenGreaterThanEq:
		debugN(" >= ");
		break;
	case kTokenNotEq:
		debugN(" != ");
		break;
	}

	nextToken();
	parseConcat();

	if (_stack.size() < 2)
		error("comparison didn't get enough values");
	LBValue val2 = _stack.pop();
	LBValue val1 = _stack.pop();
	bool result = false;
	// FIXME: should work for non-integers!!
	switch (comparison) {
	case kTokenEquals:
		result = (val1 == val2);
		break;
	case kTokenLessThan:
		result = (val1.integer < val2.integer);
		break;
	case kTokenGreaterThan:
		result = (val1.integer > val2.integer);
		break;
	case kTokenLessThanEq:
		result = (val1.integer <= val2.integer);
		break;
	case kTokenGreaterThanEq:
		result = (val1.integer >= val2.integer);
		break;
	case kTokenNotEq:
		result = (val1 != val2);
		break;
	}

	debugN(" [--> %s]", result ? "true" : "false");
	_stack.push(result);
}

void LBCode::parseConcat() {
	parseArithmetic1();

	while (_currToken == kTokenConcat) {
		debugN(" & ");
		nextToken();
		parseArithmetic1();

		LBValue val2 = _stack.pop();
		LBValue val1 = _stack.pop();
		Common::String result = val1.toString() + val2.toString();
		debugN(" [--> \"%s\"]", result.c_str());
		_stack.push(result);
	}
}

void LBCode::parseArithmetic1() {
	parseArithmetic2();

	while (_currToken == kTokenMinus || _currToken == kTokenPlus) {
		byte op = _currToken;
		if (op == kTokenMinus)
			debugN(" - ");
		else if (op == kTokenPlus)
			debugN(" + ");

		nextToken();
		parseArithmetic2();

		LBValue val2 = _stack.pop();
		LBValue val1 = _stack.pop();
		LBValue result;
		// TODO: cope with non-integers
		if (op == kTokenMinus)
			result = val1.toInt() - val2.toInt();
		else
			result = val1.toInt() + val2.toInt();
		debugN(" [--> %d]", result.toInt());
		_stack.push(result);
	}
}

void LBCode::parseArithmetic2() {
	parseMain();

	while (true) {
		byte op = _currToken;
		switch (op) {
		case kTokenMultiply:
			debugN(" * ");
			break;
		case kTokenDivide:
			debugN(" / ");
			break;
		case kTokenIntDivide:
			debugN(" div ");
			break;
		case kTokenModulo:
			debugN(" %% ");
			break;
		default:
			return;
		}

		nextToken();
		parseMain();

		LBValue val2 = _stack.pop();
		LBValue val1 = _stack.pop();
		LBValue result;
		// TODO: cope with non-integers
		if (op == kTokenMultiply) {
			result = val1.toInt() * val2.toInt();
		} else if (val2.toInt() == 0) {
			result = 1;
		} else {
			switch (op) {
			case kTokenDivide:
				// TODO: fp divide
				result = val1.toInt() / val2.toInt();
				break;
			case kTokenIntDivide:
				result = val1.toInt() / val2.toInt();
				break;
			case kTokenModulo:
				result = val1.toInt() % val2.toInt();
				break;
			}
		}

		_stack.push(result);
	}
}

void LBCode::parseMain() {
	byte prefix = 0;
	if (_currToken == kTokenMinus || _currToken == kTokenPlus) {
		debugN("%s", _currToken == kTokenMinus ? "-" : "+");
		prefix = _currToken;
		nextToken();
	}

	switch (_currToken) {
	case kTokenIdentifier:
		assert(_currValue.type == kLBValueString);
		{
		Common::String varname = _currValue.string;
		debugN("%s", varname.c_str());
		nextToken();
		if (varname.equalsIgnoreCase("self")) {
			_stack.push(LBValue(_currSource));
			if (_currToken == kTokenAssign)
				error("attempted assignment to self");
			break;
		}
		bool indexing = false;
		Common::Array<LBValue> index;
		while (_currToken == kTokenListStart) {
			debugN("[");
			nextToken();
			parseStatement();
			if (_currToken != kTokenListEnd)
				error("expected list end");
			debugN("]");
			nextToken();
			if (!_stack.size())
				error("index failed");
			indexing = true;
			index.push_back(_stack.pop());
		}
		if (_currToken == kTokenAssign) {
			debugN(" = ");
			nextToken();
			parseStatement();
			if (!_stack.size())
				error("assignment failed");
			LBValue *val;
			if (indexing)
				val = getIndexedVar(varname, index);
			else
				val = &_vm->_variables[varname];
			if (val) {
				*val = _stack.pop();
				_stack.push(*val);
			} else
				_stack.push(LBValue());
		} else if (_currToken == kTokenAndEquals) {
			debugN(" &= ");
			nextToken();
			parseStatement();
			if (!_stack.size())
				error("assignment failed");
			LBValue *val;
			if (indexing)
				val = getIndexedVar(varname, index);
			else
				val = &_vm->_variables[varname];
			if (val) {
				if (val->type != kLBValueString)
					error("operator &= used on non-string");
				val->string = val->string + _stack.pop().toString();
				_stack.push(*val);
			} else
				_stack.push(LBValue());
		} else {
			if (indexing) {
				LBValue *val = getIndexedVar(varname, index);
				if (val)
					_stack.push(*val);
				else
					_stack.push(LBValue());
			} else
				_stack.push(_vm->_variables[varname]);
		}
		// FIXME: pre/postincrement for non-integers
		if (_currToken == kTokenPlusPlus) {
			debugN("++");
			if (indexing) {
				LBValue *val = getIndexedVar(varname, index);
				if (val)
					val->integer++;
			} else
				_vm->_variables[varname].integer++;
			nextToken();
		} else if (_currToken == kTokenMinusMinus) {
			debugN("--");
			if (indexing) {
				LBValue *val = getIndexedVar(varname, index);
				if (val)
					val->integer--;
			} else
				_vm->_variables[varname].integer--;
			nextToken();
		}
		}
		break;

	case kTokenPlusPlus:
	case kTokenMinusMinus:
		{
		byte token = _currToken;
		if (token == kTokenPlusPlus)
			debugN("++");
		else
			debugN("--");
		nextToken();

		if (_currToken != kTokenIdentifier)
			error("expected identifier");
		assert(_currValue.type == kLBValueString);
		Common::String varname = _currValue.string;
		debugN("%s", varname.c_str());
		LBValue &val = _vm->_variables[varname];

		// FIXME: pre/postincrement for non-integers
		if (token == kTokenPlusPlus)
			val.integer++;
		else
			val.integer--;
		_stack.push(val);
		nextToken();
		}
		break;

	case kTokenLiteral:
	case kTokenConstMode:
	case kTokenConstEventId:
	case 0x5e: // TODO: ??
	case kTokenKeycode:
		assert(_currValue.type == kLBValueInteger);
		debugN("%d", _currValue.integer);
		_stack.push(_currValue);
		nextToken();
		break;

	case kTokenString:
		assert(_currValue.type == kLBValueString);
		debugN("\"%s\"", _currValue.string.c_str());
		_stack.push(_currValue);
		nextToken();
		break;

	case kTokenTrue:
		debugN("TRUE");
		_stack.push(true);
		nextToken();
		break;
	case kTokenFalse:
		debugN("FALSE");
		_stack.push(false);
		nextToken();
		break;

	case kTokenOpenBracket:
		debugN("(");
		nextToken();
		parseStatement();
		if (_currToken != kTokenCloseBracket)
			error("no kTokenCloseBracket (%02x), multiple entries?", _currToken);
		debugN(")");
		nextToken();
		break;

	case kTokenListStart:
		debugN("[");
		nextToken();
		{
		Common::SharedPtr<LBList> list = Common::SharedPtr<LBList>(new LBList);
		while (_currToken != kTokenListEnd) {
			parseStatement();
			if (!_stack.size())
				error("unexpected empty stack during literal list evaluation");
			list->array.push_back(_stack.pop());
			if (_currToken == kTokenComma) {
				debugN(", ");
				nextToken();
			} else if (_currToken != kTokenListEnd)
				error("encountered unexpected token %02x during literal list", _currToken);
		}
		debugN("]");
		nextToken();
		_stack.push(list);
		}
		break;

	case kTokenNot:
		debugN("!");
		nextToken();
		// not parseStatement, ! takes predecence over logical ops
		parseComparisons();
		if (!_stack.size())
			error("not op failed");
		_stack.push(_stack.pop().isZero() ? 1 : 0);
		break;

	case kTokenGeneralCommand:
		runGeneralCommand();
		break;

	case kTokenItemCommand:
		runItemCommand();
		break;

	case kTokenNotifyCommand:
		runNotifyCommand();
		break;

	default:
		error("unknown token %02x in code", _currToken);
	}

	if (prefix) {
		if (!_stack.size())
			error("+/- prefix failed");
		LBValue val = _stack.pop();
		assert(val.isNumeric());
		// FIXME
		if (prefix == kTokenMinus)
			val.integer--;
		else
			val.integer++;
		_stack.push(val);
	}
}

LBValue *LBCode::getIndexedVar(Common::String varname, const Common::Array<LBValue> &index) {
	LBValue *var = &_vm->_variables[varname];
	for (uint i = 0; i < index.size(); i++) {
		if (var->type != kLBValueList)
			error("variable '%s' was indexed, but isn't a list after %d indexes", varname.c_str(), i);
		if (index[i].type != kLBValueInteger)
			error("index %d wasn't an integer", i);
		if (index[i].integer < 1 || index[i].integer > (int)var->list->array.size())
			return NULL;
		var = &var->list->array[index[i].integer - 1];
	}
	return var;
}

LBItem *LBCode::resolveItem(const LBValue &value) {
	if (value.type == kLBValueItemPtr)
		return value.item;
	if (value.type == kLBValueString)
		return _vm->getItemByName(value.string);
	if (value.type == kLBValueInteger)
		return _vm->getItemById(value.integer);
	return NULL;
}

Common::Array<LBValue> LBCode::readParams() {
	Common::Array<LBValue> params;

	if (_currOffset + 1 >= _size)
		error("went off the end of code");

	byte numParams = _data[_currOffset++];

	if (!numParams) {
		debugN("()");
		nextToken();
		return params;
	}

	nextToken();
	if (_currToken != kTokenOpenBracket)
		error("missing ( before code parameter list (got %02x)", _currToken);
	nextToken();
	debugN("(");

	for (uint i = 0; i < numParams; i++) {
		if (i != 0) {
			if (_currToken != ',')
				error("missing , between code parameters (got %02x)", _currToken);
			debugN(", ");
			nextToken();
		}

		parseStatement();
		if (!_stack.size())
			error("stack empty");
		LBValue nextValue = _stack.pop();

		params.push_back(nextValue);
	}

	if (_currToken != kTokenCloseBracket)
		error("missing ) after code parameter list (got %02x)", _currToken);
	nextToken();
	debugN(")");

	return params;
}

Common::Rect LBCode::getRectFromParams(const Common::Array<LBValue> &params) {
	if (params.size() == 0) {
		assert(_currSource);
		return _currSource->getRect();
	} else if (params.size() == 1) {
		const LBValue &val = params[0];
		LBItem *item = _vm->getItemByName(val.toString());
		if (item)
			return item->getRect();
		else
			return val.toRect();
	} else
		error("getRectFromParams got called with weird state");
}

struct CodeCommandInfo {
	const char *name;
	typedef void (LBCode::*CommandFunc)(const Common::Array<LBValue> &params);
	CommandFunc func;
};

#define NUM_GENERAL_COMMANDS 129
CodeCommandInfo generalCommandInfo[NUM_GENERAL_COMMANDS] = {
	{ "eval", &LBCode::cmdEval },
	{ "random", &LBCode::cmdRandom },
	{ "stringLen", &LBCode::cmdStringLen },
	{ "substring", &LBCode::cmdSubstring },
	{ "max", &LBCode::cmdMax },
	{ "min", &LBCode::cmdMin },
	{ "abs", &LBCode::cmdAbs },
	{ "getRect", &LBCode::cmdGetRect }, // also "makeRect"
	{ "makePt", &LBCode::cmdMakePoint }, // also "makePair"
	{ "topLeft", &LBCode::cmdTopLeft },
	{ "bottomRight", &LBCode::cmdBottomRight },
	{ "mousePos", &LBCode::cmdMousePos },
	{ "top", &LBCode::cmdTop },
	{ "left", &LBCode::cmdLeft },
	{ "bottom", &LBCode::cmdBottom },
	// 0x10
	{ "right", &LBCode::cmdRight },
	{ "xpos", 0 },
	{ "ypos", 0 },
	{ "playFrom", 0 },
	{ "move", &LBCode::cmdMove },
	{ 0, 0 },
	{ 0, 0 },
	{ "setDragParams", &LBCode::cmdSetDragParams },
	{ "resetDragParams", 0 },
	{ "enableRollover", &LBCode::cmdUnimplemented /* FIXME */ },
	{ "setCursor", 0 },
	{ "width", 0 },
	{ "height", 0 },
	{ "getFrameBounds", 0 }, // also "getFrameRect"
	{ "traceRect", 0 },
	{ "sqrt", 0 },
	// 0x20
	{ "deleteVar", 0 },
	{ "saveVars", 0 },
	{ "scriptLink", 0 },
	{ "setViewOrigin", &LBCode::cmdUnimplemented },
	{ "rectSect", 0 },
	{ "getViewOrigin", 0 },
	{ "getViewRect", 0 },
	{ "getPage", 0 },
	{ "getWorldRect", 0 },
	{ "isWorldWrap", 0 },
	{ "newList", &LBCode::cmdNewList },
	{ "deleteList", 0 },
	{ "add", &LBCode::cmdAdd },
	{ 0, 0 },
	{ "addAt", &LBCode::cmdAddAt },
	{ "getAt", 0 },
	// 0x30
	{ 0, 0 },
	{ "getIndex", 0 },
	{ "setAt", &LBCode::cmdSetAt },
	{ "listLen", &LBCode::cmdListLen },
	{ "deleteAt", &LBCode::cmdDeleteAt },
	{ "clearList", &LBCode::cmdUnimplemented },
	{ "setWorld", 0 },
	{ "setProperty", &LBCode::cmdSetProperty },
	{ "getProperty", &LBCode::cmdGetProperty },
	{ "copyList", 0 },
	{ "invoke", 0 },
	{ "exec", &LBCode::cmdExec },
	{ "return", &LBCode::cmdReturn },
	{ "sendSync", 0 },
	{ "moveViewOrigin", 0 },
	{ "addToGroup", 0 },
	// 0x40
	{ "removeFromGroup", 0 },
	{ "clearGroup", 0 },
	{ "setPlayParams", &LBCode::cmdSetPlayParams },
	{ "autoEvent", 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ "getID", 0 },
	{ "setCursorPosition", 0 },
	{ "getTime", 0 },
	{ "logWriteLn", 0 },
	{ "logWrite", 0 },
	{ "getLanguage", 0 },
	{ "setLanguage", 0 },
	{ "getSequence", 0 },
	{ "setSequence", 0 },
	{ "getFileSpec", 0 },
	// 0x50
	{ "setKeyEvent", &LBCode::cmdSetKeyEvent },
	{ "setHitTest", &LBCode::cmdSetHitTest },
	{ "key", &LBCode::cmdKey },
	{ "deleteKeyEvent", 0 },
	{ "setDisplay", &LBCode::cmdUnimplemented },
	{ "getDisplay", 0 },
	{ 0, 0 },
	{ "lbxCreate", &LBCode::cmdLBXCreate },
	{ "lbxFunc", &LBCode::cmdLBXFunc },
	{ "waitCursor", 0 },
	{ "debugBreak", 0 },
	{ "menuItemEnable", 0 },
	{ "showChannel", 0 },
	{ "hideChannel", 0 },
	{ "setPageFade", 0 },
	{ "normalize", 0 },
	// 0x60 (v5+)
	{ "addEvent", 0 },
	{ "setCueEvent", 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ "getName", 0 },
	{ "getProperties", 0 },
	{ "createItem", 0 },
	{ "setProperties", 0 },
	{ "alert", 0 },
	{ "getUniqueID", 0 },
	{ "isNumeric", 0 },
	{ "setKeyFocus", 0 },
	{ "getKeyFocus", 0 },
	{ "isItem", 0 },
	{ "itemHit", 0 },
	{ "getItem ", 0 },
	// 0x70
	{ 0, 0 },
	{ "setCascade", 0 },
	{ "getCascade", 0 },
	{ "getRes", 0 },
	{ "setRes", 0 },
	{ "getFilename", 0 },
	{ "resEnumNames", 0 },
	{ "isList", 0 },
	{ "resetRect", 0 },
	{ "setVolume", 0 },
	{ "getVolume", 0 },
	{ "pause", 0 },
	{ "getTextWidth", 0 },
	{ "setItemVolume", 0 },
	{ "setSoundLoop", 0 },
	// 0x80
	{ "setClipboard", 0 },
	{ "getResDuration", 0 }
};

void LBCode::runGeneralCommand() {
	byte commandType = _currValue.integer;

	if (commandType == 0 || commandType > NUM_GENERAL_COMMANDS)
		error("bad command type 0x%02x in runGeneralCommand", commandType);

	CodeCommandInfo &info = generalCommandInfo[commandType - 1];
	debugN("%s", info.name);
	Common::Array<LBValue> params = readParams();

	if (!info.func)
		error("general command '%s' (0x%02x) unimplemented", info.name, commandType);
	(this->*(info.func))(params);
}

void LBCode::cmdUnimplemented(const Common::Array<LBValue> &params) {
	warning("unimplemented command called");
}

void LBCode::cmdEval(const Common::Array<LBValue> &params) {
	// FIXME: v4 eval is different?
	if (params.size() != 1)
		error("incorrect number of parameters (%d) to eval", params.size());

	LBCode tempCode(_vm, 0);

	uint offset = tempCode.parseCode(params[0].toString());
	_stack.push(tempCode.runCode(_currSource, offset));
}

void LBCode::cmdRandom(const Common::Array<LBValue> &params) {
	if (params.size() != 2)
		error("incorrect number of parameters (%d) to random", params.size());

	int min = params[0].toInt();
	int max = params[1].toInt();
	_stack.push(_vm->_rnd->getRandomNumberRng(min, max));
}

void LBCode::cmdStringLen(const Common::Array<LBValue> &params) {
	if (params.size() != 1)
		error("incorrect number of parameters (%d) to stringLen", params.size());

	const Common::String &string = params[0].toString();
	_stack.push(string.size());
}

void LBCode::cmdSubstring(const Common::Array<LBValue> &params) {
	if (params.size() != 3)
		error("incorrect number of parameters (%d) to substring", params.size());

	const Common::String &string = params[0].toString();
	uint begin = params[1].toInt();
	uint end = params[2].toInt();
	if (begin == 0)
		error("invalid substring call (%d to %d)", begin, end);
	if (begin > end || end > string.size()) {
		_stack.push(Common::String());
		return;
	}
	Common::String substring(string.c_str() + (begin - 1), end - begin + 1);
	_stack.push(substring);
}

void LBCode::cmdMax(const Common::Array<LBValue> &params) {
	if (params.size() != 2)
		error("incorrect number of parameters (%d) to max", params.size());

	// FIXME: fp
	int a = params[0].toInt();
	int b = params[1].toInt();
	_stack.push(MAX(a, b));
}

void LBCode::cmdMin(const Common::Array<LBValue> &params) {
	if (params.size() != 2)
		error("incorrect number of parameters (%d) to min", params.size());

	// FIXME: fp
	int a = params[0].toInt();
	int b = params[1].toInt();
	_stack.push(MIN(a, b));
}

void LBCode::cmdAbs(const Common::Array<LBValue> &params) {
	if (params.size() != 1)
		error("incorrect number of parameters (%d) to abs", params.size());

	// FIXME: fp
	int a = params[0].toInt();
	_stack.push(ABS(a));
}

void LBCode::cmdGetRect(const Common::Array<LBValue> &params) {
	if (params.size() < 2) {
		_stack.push(getRectFromParams(params));
	} else if (params.size() == 2) {
		Common::Point p1 = params[0].toPoint();
		Common::Point p2 = params[1].toPoint();
		_stack.push(Common::Rect(p1.x, p1.y, p2.x, p2.y));
	} else if (params.size() == 4) {
		_stack.push(Common::Rect(params[0].toInt(), params[1].toInt(), params[2].toInt(), params[3].toInt()));
	} else
		error("incorrect number of parameters (%d) to getRect", params.size());
}

void LBCode::cmdMakePoint(const Common::Array<LBValue> &params) {
	if (params.size() != 2)
		error("incorrect number of parameters (%d) to makePoint", params.size());
	_stack.push(Common::Point(params[0].toInt(), params[1].toInt()));
}

void LBCode::cmdTopLeft(const Common::Array<LBValue> &params) {
	if (params.size() > 1)
		error("too many parameters (%d) to topLeft", params.size());

	Common::Rect rect = getRectFromParams(params);
	_stack.push(Common::Point(rect.top, rect.left));
}

void LBCode::cmdBottomRight(const Common::Array<LBValue> &params) {
	if (params.size() > 1)
		error("too many parameters (%d) to bottomRight", params.size());

	Common::Rect rect = getRectFromParams(params);
	_stack.push(Common::Point(rect.bottom, rect.right));
}

void LBCode::cmdMousePos(const Common::Array<LBValue> &params) {
	if (params.size() != 0)
		error("too many parameters (%d) to mousePos", params.size());

	Common::Point pt = _vm->_system->getEventManager()->getMousePos();
	_stack.push(pt);
}

void LBCode::cmdTop(const Common::Array<LBValue> &params) {
	if (params.size() > 1)
		error("too many parameters (%d) to top", params.size());

	Common::Rect rect = getRectFromParams(params);
	_stack.push(rect.top);
}

void LBCode::cmdLeft(const Common::Array<LBValue> &params) {
	if (params.size() > 1)
		error("too many parameters (%d) to left", params.size());

	Common::Rect rect = getRectFromParams(params);
	_stack.push(rect.left);
}

void LBCode::cmdBottom(const Common::Array<LBValue> &params) {
	if (params.size() > 1)
		error("too many parameters (%d) to bottom", params.size());

	Common::Rect rect = getRectFromParams(params);
	_stack.push(rect.bottom);
}

void LBCode::cmdRight(const Common::Array<LBValue> &params) {
	if (params.size() > 1)
		error("too many parameters (%d) to right", params.size());

	Common::Rect rect = getRectFromParams(params);
	_stack.push(rect.right);
}

void LBCode::cmdMove(const Common::Array<LBValue> &params) {
	if (params.size() != 1 && params.size() != 2)
		error("incorrect number of parameters (%d) to move", params.size());

	LBItem *target = _currSource;
	Common::Point pt;
	if (params.size() == 1) {
		pt = params[0].toPoint();
	} else {
		target = resolveItem(params[0]);
		if (!target)
			error("attempted move on invalid item (%s)", params[0].toString().c_str());
		pt = params[1].toPoint();
	}

	target->moveBy(pt);
}

void LBCode::cmdSetDragParams(const Common::Array<LBValue> &params) {
	warning("ignoring setDragParams");
}

void LBCode::cmdNewList(const Common::Array<LBValue> &params) {
	if (params.size() != 0)
		error("incorrect number of parameters (%d) to newList", params.size());

	_stack.push(Common::SharedPtr<LBList>(new LBList));
}

void LBCode::cmdAdd(const Common::Array<LBValue> &params) {
	if (params.size() != 2)
		error("incorrect number of parameters (%d) to add", params.size());

	if (params[0].type != kLBValueList || !params[0].list)
		error("invalid lbx object passed to add");

	params[0].list->array.push_back(params[1]);
}

void LBCode::cmdAddAt(const Common::Array<LBValue> &params) {
	if (params.size() != 3)
		error("incorrect number of parameters (%d) to addAt", params.size());

	if (params[0].type != kLBValueList || !params[0].list)
		error("invalid lbx object passed to addAt");

	if (params[1].type != kLBValueInteger || params[1].integer < 1)
		error("invalid index passed to addAt");

	if ((uint)params[1].integer > params[0].list->array.size())
		params[0].list->array.resize(params[1].integer);
	params[0].list->array.insert_at(params[1].integer - 1, params[2]);
}

void LBCode::cmdSetAt(const Common::Array<LBValue> &params) {
	if (params.size() != 3)
		error("incorrect number of parameters (%d) to setAt", params.size());

	if (params[0].type != kLBValueList || !params[0].list)
		error("invalid lbx object passed to setAt");

	if (params[1].type != kLBValueInteger || params[1].integer < 1)
		error("invalid index passed to setAt");

	if ((uint)params[1].integer > params[0].list->array.size())
		params[0].list->array.resize(params[1].integer);
	params[0].list->array[params[1].integer - 1] =  params[2];
}

void LBCode::cmdListLen(const Common::Array<LBValue> &params) {
	if (params.size() != 1)
		error("incorrect number of parameters (%d) to listLen", params.size());

	if (params[0].type != kLBValueList || !params[0].list)
		error("invalid lbx object passed to lbxFunc");

	_stack.push(params[0].list->array.size());
}

void LBCode::cmdDeleteAt(const Common::Array<LBValue> &params) {
	if (params.size() != 2)
		error("incorrect number of parameters (%d) to deleteAt", params.size());

	if (params[0].type != kLBValueList || !params[0].list)
		error("invalid lbx object passed to deleteAt");

	if (params[1].type != kLBValueInteger)
		error("invalid index passed to deleteAt");
	if (params[1].integer < 1 || params[1].integer > (int)params[0].list->array.size())
		return;
	params[0].list->array.remove_at(params[1].integer - 1);
}

void LBCode::cmdSetProperty(const Common::Array<LBValue> &params) {
	if (params.size() < 2 || params.size() > 3)
		error("incorrect number of parameters (%d) to setProperty", params.size());

	Common::String name;
	LBValue val;
	LBItem *target = _currSource;
	if (params.size() == 3) {
		target = resolveItem(params[0]);
		if (!target)
			error("attempted setProperty on invalid item (%s)", params[0].toString().c_str());
		name = params[1].toString();
		val = params[2];
	} else {
		name = params[0].toString();
		val = params[1];
	}

	target->_variables[name] = val;
}

void LBCode::cmdGetProperty(const Common::Array<LBValue> &params) {
	if (params.size() < 1 || params.size() > 2)
		error("incorrect number of parameters (%d) to getProperty", params.size());

	Common::String name;
	LBItem *target = _currSource;
	if (params.size() == 2) {
		target = resolveItem(params[0]);
		if (!target)
			error("attempted getProperty on invalid item (%s)", params[0].toString().c_str());
		name = params[1].toString();
	} else {
		name = params[0].toString();
	}

	_stack.push(target->_variables[name]);
}

void LBCode::cmdExec(const Common::Array<LBValue> &params) {
	if (params.size() != 1)
		error("incorrect number of parameters (%d) to exec", params.size());
	if (params[0].type != kLBValueInteger || params[0].integer < 0)
		error("invalid offset passed to exec");
	uint offset = (uint)params[0].integer;

	uint32 oldOffset = _currOffset;
	byte oldToken = _currToken;
	LBValue val = runCode(_currSource, offset);
	_currOffset = oldOffset;
	_currToken = oldToken;

	_stack.push(val);
	_stack.push(val);
}

void LBCode::cmdReturn(const Common::Array<LBValue> &params) {
	if (params.size() != 2)
		error("incorrect number of parameters (%d) to return", params.size());

	if (!_stack.size())
		error("empty stack on entry to return");

	if (params[0] == _stack.top()) {
		_stack.pop();
		_stack.push(params[1]);
		_currToken = kTokenEndOfFile;
	} else
		_stack.push(_stack.top());
}

void LBCode::cmdSetPlayParams(const Common::Array<LBValue> &params) {
	if (params.size() > 8)
		error("too many parameters (%d) to setPlayParams", params.size());
	if (!params.size())
		error("no target for setPlayParams");

	if (params[0].type != kLBValueItemPtr)
		error("first param to setPlayParams wasn't item");
	LBItem *target = params[0].item;

	// TODO: type-checking
	switch (params.size()) {
	case 8:
		target->_soundMode = params[7].integer;
	case 7:
		target->_controlMode = params[6].integer;
	case 6:
		// TODO: _relocPoint?
	case 5:
		// TODO: _periodMin/Max
	case 4:
		target->_timingMode = params[3].integer;
	case 3:
		// TODO: _delayMin/Max
	case 2:
		target->_loopMode = params[1].integer;
	}
}

void LBCode::cmdSetKeyEvent(const Common::Array<LBValue> &params) {
	if (params.size() != 2)
		error("incorrect number of parameters (%d) to setKeyEvent", params.size());

	// FIXME: params[0] is key, params[1] is opcode id
	warning("ignoring setKeyEvent");
}

void LBCode::cmdSetHitTest(const Common::Array<LBValue> &params) {
	if (params.size() > 2)
		error("incorrect number of parameters (%d) to setHitTest", params.size());
	warning("ignoring setHitTest");
}

void LBCode::cmdLBXCreate(const Common::Array<LBValue> &params) {
	if (params.size() != 1)
		error("incorrect number of parameters (%d) to lbxCreate", params.size());

	_stack.push(createLBXObject(_vm, params[0].toInt()));
}

void LBCode::cmdLBXFunc(const Common::Array<LBValue> &params) {
	if (params.size() < 2)
		error("incorrect number of parameters (%d) to lbxFunc", params.size());

	if (params[0].type != kLBValueLBX || !params[0].lbx)
		error("invalid lbx object passed to lbxFunc");

	Common::SharedPtr<LBXObject> lbx = params[0].lbx;
	uint callId = params[1].toInt();

	Common::Array<LBValue> callParams;
	for (uint i = 0; i < params.size() - 2; i++)
		callParams.push_back(params[i + 2]);

	LBValue result;
	if (lbx->call(callId, callParams, result))
		_stack.push(result);
}

void LBCode::cmdKey(const Common::Array<LBValue> &params) {
	_stack.push(0); // FIXME
	warning("ignoring Key");
}

#define NUM_ITEM_COMMANDS 34
CodeCommandInfo itemCommandInfo[NUM_ITEM_COMMANDS] = {
	{ "clone", &LBCode::itemClone },
	{ "destroy", 0 },
	{ "dragBeginFrom", 0 },
	{ "dragEnd", 0 },
	{ "enableLocal", 0 },
	{ "enable", 0 },
	{ "showLocal", 0 },
	{ "show", 0 },
	{ "getFrame", 0 },
	{ "getParent", 0 },
	{ "getPosition" , 0 },
	{ "getText", 0 },
	{ "getZNext", 0 },
	{ "getZPrev", 0 },
	{ "hitTest", 0 },
	// 0x10
	{ "isAmbient", 0 },
	{ "isEnabled", 0 },
	{ "isMuted", 0 },
	{ "isPlaying", &LBCode::itemIsPlaying },
	{ "isVisible", 0 },
	{ "isLoaded", &LBCode::itemIsLoaded },
	{ "isDragging", 0 },
	{ "load", 0 },
	{ "moveTo", &LBCode::itemMoveTo },
	{ "mute", 0 },
	{ "play", 0 },
	{ "seek", &LBCode::itemSeek },
	{ "seekToFrame", &LBCode::itemSeekToFrame },
	{ "setParent", &LBCode::itemSetParent },
	{ "setZOrder", 0 },
	{ "setText", 0 },
	// 0x20
	{ "stop", 0 },
	{ "unload", 0 },
	{ "unloadSync", 0}
};

void LBCode::runItemCommand() {
	byte commandType = _currValue.integer;

	if (commandType == 0 || commandType > NUM_ITEM_COMMANDS)
		error("bad command type 0x%02x in runItemCommand", commandType);

	CodeCommandInfo &info = itemCommandInfo[commandType - 1];
	debugN("%s", info.name);
	Common::Array<LBValue> params = readParams();

	if (!info.func)
		error("item command '%s' (0x%02x) unimplemented", info.name, commandType);
	(this->*(info.func))(params);
}

void LBCode::itemClone(const Common::Array<LBValue> &params) {
	// TODO: first param can be target?
	if (params.size() != 2)
		error("incorrect number of parameters (%d) to setParent", params.size());

	uint id = params[0].toInt();
	const Common::String &name = params[1].toString();

	_currSource->clone(id, name);
}

void LBCode::itemIsPlaying(const Common::Array<LBValue> &params) {
	// TODO
	warning("ignoring isPlaying");
	_stack.push(0);
}

void LBCode::itemIsLoaded(const Common::Array<LBValue> &params) {
	if (params.size() != 1)
		error("incorrect number of parameters (%d) to isLoaded", params.size());

	LBItem *item = resolveItem(params[0]);
	if (!item || !item->isLoaded())
		_stack.push(0);
	else
		_stack.push(1);
}

void LBCode::itemMoveTo(const Common::Array<LBValue> &params) {
	if (params.size() != 1 && params.size() != 2)
		error("incorrect number of parameters (%d) to moveTo", params.size());

	LBItem *target = _currSource;
	Common::Point pt;
	if (params.size() == 1) {
		pt = params[0].toPoint();
	} else {
		target = resolveItem(params[0]);
		if (!target)
			error("attempted moveTo on invalid item (%s)", params[0].toString().c_str());
		pt = params[1].toPoint();
	}

	target->moveTo(pt);
}

void LBCode::itemSeek(const Common::Array<LBValue> &params) {
	if (params.size() != 2)
		error("incorrect number of parameters (%d) to seek", params.size());

	LBItem *item = resolveItem(params[0]);
	if (!item)
		error("attempted seek on invalid item (%s)", params[0].toString().c_str());
	uint seekTo = params[1].toInt();
	item->seekToTime(seekTo);
}

void LBCode::itemSeekToFrame(const Common::Array<LBValue> &params) {
	if (params.size() != 2)
		error("incorrect number of parameters (%d) to seekToFrame", params.size());

	LBItem *item = resolveItem(params[0]);
	if (!item)
		error("attempted seekToFrame on invalid item (%s)", params[0].toString().c_str());
	uint seekTo = params[1].toInt();
	item->seek(seekTo);
}

void LBCode::itemSetParent(const Common::Array<LBValue> &params) {
	if (params.size() > 2)
		error("incorrect number of parameters (%d) to setParent", params.size());
	// TODO
	warning("ignoring setParent");
}

void LBCode::runNotifyCommand() {
	byte commandType = _currValue.integer;

	switch (commandType) {
	case kLBNotifyChangePage:
		{
		debugN("goto");
		Common::Array<LBValue> params = readParams();
		// TODO: type-checking
		NotifyEvent notifyEvent(kLBNotifyChangePage, 1);
		switch (params.size()) {
		case 4:
			notifyEvent.type = kLBNotifyChangeMode; // FIXME: type 8?
			notifyEvent.newUnknown = params[0].integer; // FIXME: this is newLanguage
			notifyEvent.newMode = params[1].integer;
			notifyEvent.newPage = params[2].integer;
			notifyEvent.newSubpage = params[3].integer;
			break;

		case 2:
			notifyEvent.type = kLBNotifyChangeMode;
			// FIXME: newPage and newSubpage?
			error("can't handle goto with 2 params");
			break;

		case 1:
			notifyEvent.param = params[0].integer;
			break;

		case 0:
			// FIXME: use cur page?
			error("can't handle goto with 0 params");
			break;

		default:
			error("incorrect number of parameters (%d) to goto", params.size());
		}
		_vm->addNotifyEvent(notifyEvent);
		}
		break;

	case kLBNotifyGoToControls:
	case kLBNotifyGotoQuit:
		{
		debugN(commandType == kLBNotifyGoToControls ? "gotocontrol" : "gotoquit");
		Common::Array<LBValue> params = readParams();
		if (params.size() != 0)
			error("incorrect number of parameters (%d) to notify", params.size());
		_vm->addNotifyEvent(NotifyEvent(commandType, 0));
		}
		break;

	case kLBNotifyIntroDone:
		{
		debugN("startphasemain");
		Common::Array<LBValue> params = readParams();
		if (params.size() != 0)
			error("incorrect number of parameters (%d) to startphasemain", params.size());
		_vm->addNotifyEvent(NotifyEvent(kLBNotifyIntroDone, 1));
		}
		break;

	case kLBNotifyQuit:
		{
		debugN("quit");
		Common::Array<LBValue> params = readParams();
		if (params.size() != 0)
			error("incorrect number of parameters (%d) to quit", params.size());
		_vm->addNotifyEvent(NotifyEvent(kLBNotifyQuit, 0));
		}
		break;

	default:
		error("unknown notify command %02x in code", commandType);
	}
}

/*
 * Helper function for parseCode/parseCodeSymbol:
 * Returns an unused string id.
 */
uint LBCode::nextFreeString() {
	for (uint i = 0; i <= 0xffff; i++) {
		if (!_strings.contains(i))
			return i;
	}

	error("nextFreeString couldn't find a space");
}

/*
 * Helper function for parseCode:
 * Given a name, appends the appropriate data to the provided code array and
 * returns true if it's a function, or false otherwise.
 */
bool LBCode::parseCodeSymbol(const Common::String &name, uint &pos, Common::Array<byte> &code) {
	// first, check whether the name matches a known function
	for (uint i = 0; i < 2; i++) {
		byte cmdToken;
		CodeCommandInfo *cmdInfo = NULL;
		uint cmdCount = 0;

		switch (i) {
		case 0:
			cmdInfo = generalCommandInfo;
			cmdToken = kTokenGeneralCommand;
			cmdCount = NUM_GENERAL_COMMANDS;
			break;
		case 1:
			cmdInfo = itemCommandInfo;
			cmdToken = kTokenItemCommand;
			cmdCount = NUM_ITEM_COMMANDS;
			break;
		}

		for (uint n = 0; n < cmdCount; n++) {
			const char *cmdName = cmdInfo[n].name;
			if (!cmdName)
				continue;
			if (!name.equalsIgnoreCase(cmdName))
				continue;

			// found a matching function
			code.push_back(cmdToken);
			code.push_back(n + 1);
			return true;
		}
	}

	// not a function, so must be an identifier
	code.push_back(kTokenIdentifier);

	uint stringId = nextFreeString();
	_strings[stringId] = name;

	char tmp[2];
	WRITE_BE_UINT16(tmp, (int16)stringId);
	code.push_back(tmp[0]);
	code.push_back(tmp[1]);

	return false;
}

/*
 * Parse a string for later execution, and return the offset where it was
 * stored.
 */
uint LBCode::parseCode(const Common::String &source) {
	struct LBCodeOperator {
		byte token;
		byte op;
		byte lookahead1;
		byte lookahead1Op;
		byte lookahead2;
		byte lookahead2Op;
	};

	#define NUM_LB_OPERATORS 11
	static const LBCodeOperator operators[NUM_LB_OPERATORS] = {
		{ '+', kTokenPlus, '+', kTokenPlusPlus, '=', kTokenPlusEquals },
		{ '-', kTokenMinus, '-', kTokenMinusMinus, '=', kTokenMinusEquals },
		{ '/', kTokenDivide, '=', kTokenDivideEquals, 0, 0 },
		{ '*', kTokenMultiply, '=', kTokenMultiplyEquals, 0, 0 },
		{ '=', kTokenAssign, '=', kTokenEquals, 0, 0 },
		{ '>', kTokenGreaterThan, '=', kTokenGreaterThanEq, 0, 0 },
		{ '<', kTokenLessThan, '=', kTokenLessThanEq, 0, 0 },
		{ '!', kTokenNot, '=', kTokenNotEq, 0, 0 },
		{ '&', kTokenConcat, '&', kTokenAnd, '=', kTokenAndEquals },
		{ '|', 0, '|', kTokenOr, 0, 0 },
		{ ';', kTokenEndOfStatement, 0, 0, 0, 0 }
	};

	uint pos = 0;
	Common::Array<byte> code;
	Common::Array<uint> counterPositions;
	bool wasFunction = false;

	while (pos < source.size()) {
		byte token = source[pos];
		byte lookahead = 0;
		if (pos + 1 < source.size())
			lookahead = source[pos + 1];
		pos++;

		if (token != ' ' && token != '(' && wasFunction)
			error("while parsing script '%s', encountered incomplete function call", source.c_str());

		// First, we check for simple operators.
		for (uint i = 0; i < NUM_LB_OPERATORS; i++) {
			if (token != operators[i].token)
				continue;
			if (lookahead) {
				if (lookahead == operators[i].lookahead1) {
					code.push_back(operators[i].lookahead1Op);
					token = 0;
				} else if (lookahead == operators[i].lookahead2) {
					code.push_back(operators[i].lookahead2Op);
					token = 0;
				}
				if (!token) {
					pos++;
					break;
				}
			}
			if (operators[i].op) {
				code.push_back(operators[i].op);
				token = 0;
			}
			break;
		}
		if (!token)
			continue;

		// Then, we check for more complex tokens.
		switch (token) {
		// whitespace
		case ' ':
			// ignore
			break;
		// literal string
		case '"':
		case '\'':
			{
			Common::String tempString;
			while (pos < source.size()) {
				if (source[pos] == token)
					break;
				tempString += source[pos++];
			}
			if (pos++ == source.size())
				error("while parsing script '%s', string had no end", source.c_str());

			code.push_back(kTokenString);

			uint stringId = nextFreeString();
			_strings[stringId] = tempString;

			char tmp[2];
			WRITE_BE_UINT16(tmp, (int16)stringId);
			code.push_back(tmp[0]);
			code.push_back(tmp[1]);
			}
			break;
		// open bracket
		case '(':
			bool parameterless;
			parameterless = false;
			if (wasFunction) {
				// function call parameters
				wasFunction = false;
				// we will need to back-patch the parameter count,
				// if parameters are encountered
				counterPositions.push_back(code.size());
				code.push_back(1);
				// if the next token is a ) then there are no
				// parameters, otherwise start with 1 and increment
				// if/when we encounter commas
				for (uint i = pos; i < source.size(); i++) {
					if (source[i] == ' ')
						continue;
					if (source[i] != ')')
						break;
					parameterless = true;
					code[code.size() - 1] = 0;
					break;
				}
			} else {
				// brackets around expression
				counterPositions.push_back(0);
			}
			if (!parameterless)
				code.push_back(kTokenOpenBracket);
			break;
		// close bracket
		case ')':
			if (counterPositions.empty())
				error("while parsing script '%s', encountered unmatched )", source.c_str());

			// don't push a kTokenCloseBracket for a parameterless function call
			uint counterPos2;
			counterPos2 = counterPositions.back();
			if (!counterPos2 || code[counterPos2])
				code.push_back(kTokenCloseBracket);
			counterPositions.pop_back();
			break;
		// comma (seperating function params)
		case ',':
			{
			if (counterPositions.empty())
				error("while parsing script '%s', encountered unexpected ,", source.c_str());
			code.push_back(kTokenComma);
			uint counterPos = counterPositions.back();
			if (!counterPos)
				error("while parsing script '%s', encountered , outside parameter list", source.c_str());
			code[counterPos]++;
			}
			break;
		// old-style explicit function call
		case '@':
			{
			Common::String tempString;
			while (pos < source.size()) {
				if (!Common::isAlpha(source[pos]) && !Common::isDigit(source[pos]))
					break;
				tempString += source[pos++];
			}
			wasFunction = parseCodeSymbol(tempString, pos, code);
			if (!wasFunction)
				error("while parsing script '%s', encountered explicit function call to unknown function '%s'",
					source.c_str(), tempString.c_str());
			}
			break;
		default:
			if (Common::isDigit(token)) {
				const char *in = source.c_str() + pos - 1;
				// FIXME: handle floats?
				char *endptr;
				long int intValue = strtol(in, &endptr, 0);
				assert(endptr > in);
				pos += (endptr - in) - 1;

				// FIXME: handle storing longs if needed
				code.push_back(kTokenLiteral);
				code.push_back(kLBCodeLiteralInteger);
				char tmp[2];
				WRITE_BE_UINT16(tmp, (int16)intValue);
				code.push_back(tmp[0]);
				code.push_back(tmp[1]);
			} else if (Common::isAlpha(token)) {
				Common::String tempString;
				tempString += token;
				while (pos < source.size()) {
					if (!Common::isAlpha(source[pos]) && !Common::isDigit(source[pos]))
						break;
					tempString += source[pos++];
				}
				if (tempString.equalsIgnoreCase("true")) {
					code.push_back(kTokenTrue);
				} else if (tempString.equalsIgnoreCase("false")) {
					code.push_back(kTokenFalse);
				} else {
					wasFunction = parseCodeSymbol(tempString, pos, code);
				}
			} else {
				error("while parsing script '%s', couldn't parse '%c'", source.c_str(), token);
			}
		}
	}

	if (wasFunction)
		error("while parsing script '%s', encountered incomplete function call", source.c_str());
	if (counterPositions.size())
		error("while parsing script '%s', unmatched (", source.c_str());

	code.push_back(kTokenEndOfFile);

	uint codeOffset = _size;
	byte *newData = new byte[_size + code.size()];
	memcpy(newData, _data, _size);
	memcpy(newData, &code[0], code.size());
	delete[] _data;
	_data = newData;
	_size += code.size();
	return codeOffset;
}

} // End of namespace Mohawk
