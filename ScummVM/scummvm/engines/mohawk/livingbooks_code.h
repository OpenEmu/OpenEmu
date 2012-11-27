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

#ifndef MOHAWK_LIVINGBOOKS_CODE_H
#define MOHAWK_LIVINGBOOKS_CODE_H

#include "common/ptr.h"
#include "common/rect.h"
#include "common/stack.h"
#include "common/substream.h"

namespace Mohawk {

class MohawkEngine_LivingBooks;
class LBItem;
class LBXObject;
struct LBList;

enum LBValueType {
	kLBValueString,
	kLBValueInteger,
	kLBValueReal,
	kLBValuePoint,
	kLBValueRect,
	kLBValueItemPtr,
	kLBValueLBX,
	kLBValueList
};

struct LBValue {
	LBValue() {
		type = kLBValueInteger;
		integer = 0;
	}
	LBValue(int val) {
		type = kLBValueInteger;
		integer = val;
	}
	LBValue(const Common::String &str) {
		type = kLBValueString;
		string = str;
	}
	LBValue(const Common::Point &p) {
		type = kLBValuePoint;
		point = p;
	}
	LBValue(const Common::Rect &r) {
		type = kLBValueRect;
		rect = r;
	}
	LBValue(LBItem *itm) {
		type = kLBValueItemPtr;
		item = itm;
	}
	LBValue(Common::SharedPtr<LBXObject> l) {
		type = kLBValueLBX;
		lbx = l;
	}
	LBValue(Common::SharedPtr<LBList> l) {
		type = kLBValueList;
		list = l;
	}
	LBValue(const LBValue &val) {
		type = val.type;
		switch (type) {
		case kLBValueString:
			string = val.string;
			break;
		case kLBValueInteger:
			integer = val.integer;
			break;
		case kLBValueReal:
			real = val.real;
			break;
		case kLBValuePoint:
			point = val.point;
			break;
		case kLBValueRect:
			rect = val.rect;
			break;
		case kLBValueItemPtr:
			item = val.item;
			break;
		case kLBValueLBX:
			lbx = val.lbx;
			break;
		case kLBValueList:
			list = val.list;
			break;
		}
	}

	LBValueType type;
	Common::String string;
	int integer;
	double real;
	Common::Point point;
	Common::Rect rect;
	LBItem *item;
	Common::SharedPtr<LBXObject> lbx;
	Common::SharedPtr<LBList> list;

	bool operator==(const LBValue &x) const;
	bool operator!=(const LBValue &x) const;

	bool isNumeric() const;
	bool isZero() const;

	Common::String toString() const;
	int toInt() const;
	double toDouble() const;
	Common::Point toPoint() const;
	Common::Rect toRect() const;
};

struct LBList {
	Common::Array<LBValue> array;
};

enum {
	kLBCodeLiteralInteger = 0x1
};

enum {
	kTokenIdentifier = 0x1,
	kTokenLiteral = 0x5,
	kTokenString = 0x6,
	kTokenEndOfStatement = 0x7,
	kTokenEndOfFile = 0x8,
	kTokenConcat = 0xb,
	kTokenSingleQuote = 0xc, // ??
	kTokenDoubleQuote = 0xd, // ??
	kTokenMultiply = 0xe,
	kTokenOpenBracket = 0xf,
	kTokenCloseBracket = 0x10,
	kTokenMinus = 0x11,
	kTokenMinusMinus = 0x12,
	kTokenPlusEquals = 0x13,
	kTokenPlus = 0x14,
	kTokenPlusPlus = 0x15,
	kTokenEquals = 0x16,
	kTokenMinusEquals = 0x17,
	kTokenMultiplyEquals = 0x18,
	kTokenDivideEquals = 0x19,
	kTokenListStart = 0x1a,
	kTokenListEnd = 0x1b,
	kTokenColon = 0x1c, // ??
	kTokenLessThan = 0x1d,
	kTokenGreaterThan = 0x1e,
	kTokenAndEquals = 0x1f,
	kTokenDotOperator = 0x20,
	kTokenDivide = 0x21,
	kTokenAssign = 0x22,
	kTokenLessThanEq = 0x23,
	kTokenGreaterThanEq = 0x24,
	kTokenNotEq = 0x25,
	kTokenQuote = 0x27, // ??
	kTokenAnd = 0x2a,
	kTokenComma = 0x2c,
	kTokenConstMode = 0x31,
	kTokenIntDivide = 0x32,
	kTokenModulo = 0x34,
	kTokenNot = 0x35,
	kTokenOr = 0x37,
	kTokenTrue = 0x39,
	kTokenFalse = 0x3a,
	kTokenConstDataType = 0x3b, // ??
	kTokenConstItemType = 0x3c, // ??
	kTokenConstEventId = 0x42,
	kTokenConstScriptOpcode = 0x43, // ??
	kTokenConstScriptParam = 0x44, // ??
	kTokenGeneralCommand = 0x4d,
	kTokenItemCommand = 0x4e,
	kTokenNotifyCommand = 0x4f,
	// 0x5e?!
	kTokenKeycode = 0x5f,

	// v5 only:
	kTokenLocal = 0x61,
	kTokenPropListCommand = 0x70,
	kTokenRectCommand = 0x71
};

class LBCode {
public:
	LBCode(MohawkEngine_LivingBooks *vm, uint16 baseId);
	~LBCode();

	LBValue runCode(LBItem *src, uint32 offset);
	uint parseCode(const Common::String &source);

protected:
	MohawkEngine_LivingBooks *_vm;

	uint32 _size;
	byte *_data;
	Common::HashMap<uint16, Common::String> _strings;

	uint32 _currOffset;
	LBItem *_currSource;

	Common::Stack<LBValue> _stack;
	byte _currToken;
	LBValue _currValue;

	void nextToken();

	LBValue runCode(byte terminator);
	void parseStatement();
	void parseComparisons();
	void parseConcat();
	void parseArithmetic1();
	void parseArithmetic2();
	void parseMain();

	LBValue *getIndexedVar(Common::String varname, const Common::Array<LBValue> &index);
	LBItem *resolveItem(const LBValue &value);
	Common::Array<LBValue> readParams();
	Common::Rect getRectFromParams(const Common::Array<LBValue> &params);

	void runGeneralCommand();
	void runItemCommand();
	void runNotifyCommand();

	uint nextFreeString();
	bool parseCodeSymbol(const Common::String &name, uint &pos, Common::Array<byte> &code);

public:
	void cmdUnimplemented(const Common::Array<LBValue> &params);
	void cmdEval(const Common::Array<LBValue> &params);
	void cmdRandom(const Common::Array<LBValue> &params);
	void cmdStringLen(const Common::Array<LBValue> &params);
	void cmdSubstring(const Common::Array<LBValue> &params);
	void cmdMax(const Common::Array<LBValue> &params);
	void cmdMin(const Common::Array<LBValue> &params);
	void cmdAbs(const Common::Array<LBValue> &params);
	void cmdGetRect(const Common::Array<LBValue> &params);
	void cmdMakePoint(const Common::Array<LBValue> &params);
	void cmdTopLeft(const Common::Array<LBValue> &params);
	void cmdBottomRight(const Common::Array<LBValue> &params);
	void cmdMousePos(const Common::Array<LBValue> &params);
	void cmdTop(const Common::Array<LBValue> &params);
	void cmdLeft(const Common::Array<LBValue> &params);
	void cmdBottom(const Common::Array<LBValue> &params);
	void cmdRight(const Common::Array<LBValue> &params);
	void cmdMove(const Common::Array<LBValue> &params);
	void cmdSetDragParams(const Common::Array<LBValue> &params);
	void cmdNewList(const Common::Array<LBValue> &params);
	void cmdAdd(const Common::Array<LBValue> &params);
	void cmdAddAt(const Common::Array<LBValue> &params);
	void cmdSetAt(const Common::Array<LBValue> &params);
	void cmdListLen(const Common::Array<LBValue> &params);
	void cmdDeleteAt(const Common::Array<LBValue> &params);
	void cmdSetProperty(const Common::Array<LBValue> &params);
	void cmdGetProperty(const Common::Array<LBValue> &params);
	void cmdExec(const Common::Array<LBValue> &params);
	void cmdReturn(const Common::Array<LBValue> &params);
	void cmdSetPlayParams(const Common::Array<LBValue> &params);
	void cmdSetKeyEvent(const Common::Array<LBValue> &params);
	void cmdSetHitTest(const Common::Array<LBValue> &params);
	void cmdLBXCreate(const Common::Array<LBValue> &params);
	void cmdLBXFunc(const Common::Array<LBValue> &params);
	void cmdKey(const Common::Array<LBValue> &params);

	void itemClone(const Common::Array<LBValue> &params);
	void itemIsPlaying(const Common::Array<LBValue> &params);
	void itemIsLoaded(const Common::Array<LBValue> &params);
	void itemMoveTo(const Common::Array<LBValue> &params);
	void itemSeek(const Common::Array<LBValue> &params);
	void itemSeekToFrame(const Common::Array<LBValue> &params);
	void itemSetParent(const Common::Array<LBValue> &params);
};

} // End of namespace Mohawk

#endif
