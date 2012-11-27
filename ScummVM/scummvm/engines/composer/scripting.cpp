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
#include "common/scummsys.h"
#include "common/savefile.h"

#include "composer/composer.h"
#include "composer/graphics.h"
#include "composer/resource.h"

namespace Composer {

// new script ops
enum {
	kOpPlusPlus = 0x1,
	kOpMinusMinus = 0x2,
	kOpAssign = 0x3,
	kOpAdd = 0x4,
	kOpSubtract = 0x5,
	kOpMultiply = 0x6,
	kOpDivide = 0x7,
	kOpModulo = 0x8,
	kOpMaybeAlsoAssign = 0x9,
	kOpBooleanAssign = 0xA,
	kOpNegate = 0xB,
	kOpAnd = 0xC,
	kOpOr = 0xD,
	kOpXor = 0xE,
	kOpNot = 0xF,
	kOpSqrt = 0x10,
	kOpRandom = 0x11,
	kOpExecuteScript = 0x12,
	kOpCallFunc = 0x13,
	kOpBoolLessThanEq = 0x17,
	kOpBoolLessThan = 0x16,
	kOpBoolGreaterThanEq = 0x15,
	kOpBoolGreaterThan = 0x14,
	kOpBoolEqual = 0x18,
	kOpBoolNotEqual = 0x19,
	kOpSaveArgs = 0x1A,
	kOpRestoreArgs = 0x1B,
	kOpReturn = 0x20,
	kOpLessThanEq = 0x22,
	kOpLessThan = 0x21,
	kOpGreaterThanEq = 0x24,
	kOpGreaterThan = 0x23,
	kOpEqual = 0x25,
	kOpNotEqual = 0x26,
	kOpJump = 0x80,
	kOpJumpIfNot = 0x81,
	kOpJumpIf = 0x82,
	kOpJumpIfNotValue = 0x83,
	kOpJumpIfValue = 0x84
};

enum {
	kFuncPlayAnim = 35001,
	kFuncStopAnim = 35002,
	// (no 35003)
	kFuncQueueScript = 35004,
	kFuncDequeueScript = 35005,
	kFuncSetCursor = 35006,
	kFuncGetCursor = 35007,
	kFuncShowCursor = 35008,
	kFuncHideCursor = 35009,
	// (no 35010)
	kFuncActivateButton = 35011,
	kFuncDeactivateButton = 35012,
	kFuncNewPage = 35013,
	kFuncLoadPage = 35014,
	kFuncUnloadPage = 35015,
	kFuncSetPalette = 35016,
	kFuncSaveVars = 35017,
	kFuncLoadVars = 35018,
	kFuncQueueScriptOnce = 35019,
	kFuncGetMousePos = 35020,
	kFuncChangeBackground = 35021,
	kFuncSetBackgroundColor = 35022,
	kFuncClearSprites = 35023,
	kFuncAddSprite = 35024,
	kFuncRemoveSprite = 35025,
	kFuncQuit = 35026,
	kFuncSaveData = 35027,
	kFuncLoadData = 35028,
	kFuncGetSpriteSize = 35029
};

void ComposerEngine::runEvent(uint16 id, int16 param1, int16 param2, int16 param3) {
	if (!hasResource(ID_EVNT, id))
		return;

	Common::SeekableReadStream *stream = getResource(ID_EVNT, id);
	if (stream->size() != 2)
		error("bad EVNT size %d", stream->size());
	uint16 scriptId = stream->readUint16LE();
	delete stream;

	if (!scriptId)
		return;

	debug(2, "running event %d via script %d(%d, %d, %d)", id, scriptId, param1, param2, param3);

	runScript(scriptId, param1, param2, param3);
}

int16 ComposerEngine::runScript(uint16 id, int16 param1, int16 param2, int16 param3) {
	if (getGameType() == GType_ComposerV1) {
		runOldScript(id, param1);
		return 0;
	}

	_vars[1] = param1;
	_vars[2] = param2;
	_vars[3] = param3;

	runScript(id);

	return _vars[0];
}

int16 ComposerEngine::getArg(uint16 arg, uint16 type) {
	switch (type) {
	case 0:
		return (int16)arg;
	case 1:
		return (int16)_vars[arg];
	case 2:
		return (int16)_vars[_vars[arg]];
	default:
		error("invalid argument type %d (getting arg %d)", type, arg);
	}
}

void ComposerEngine::setArg(uint16 arg, uint16 type, uint16 val) {
	switch (type) {
	case 1:
		_vars[arg] = val;
		break;
	case 2:
		_vars[_vars[arg]] = val;
		break;
	default:
		error("invalid argument type %d (setting arg %d)", type, arg);
	}
}

void ComposerEngine::runScript(uint16 id) {
	if (getGameType() == GType_ComposerV1) {
		runOldScript(id, 0);
		return;
	}

	if (!hasResource(ID_SCRP, id)) {
		debug(1, "ignoring attempt to run script %d, because it doesn't exist", id);
		return;
	}

	uint stackBase = _stack.size();
	_stack.resize(_stack.size() + 19);

	Common::SeekableReadStream *stream = getResource(ID_SCRP, id);
	if (stream->size() < 2)
		error("SCRP was too small (%d)", stream->size());
	uint16 size = stream->readUint16LE();
	if (stream->size() < 2 + 2*size)
		error("SCRP was too small (%d, but claimed %d entries)", stream->size(), size);
	uint16 *script = new uint16[size];
	for (uint i = 0; i < size; i++)
		script[i] = stream->readUint16LE();
	delete stream;

	uint16 pos = 0;
	bool lastResult = false;
	while (pos < size) {
		int16 val1, val2, val3;

		byte op = (byte)script[pos];
		uint numParams = (script[pos] & 0x300) >> 8; // 2 bits
		if (pos + numParams >= size)
			error("script ran out of content");
		uint arg1 = (script[pos] & 0xc00) >> 10; // 2 bits
		uint arg2 = (script[pos] & 0x3000) >> 12; // 2 bits
		uint arg3 = (script[pos] & 0xC000) >> 14; // 2 bits
		switch (op) {
		case kOpPlusPlus:
			if (numParams != 1)
				error("kOpPlusPlus had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			debug(9, "[%d/%d]++ (now %d)", script[pos + 1], arg1, val1 + 1);
			setArg(script[pos + 1], arg1, val1 + 1);
			break;
		case kOpMinusMinus:
			if (numParams != 1)
				error("kOpMinusMinus had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			debug(9, "[%d/%d]-- (now %d)", script[pos + 1], arg1, val1 - 1);
			setArg(script[pos + 1], arg1, val1 - 1);
			break;
		case kOpAssign:
			if (numParams != 2)
				error("kOpAssign had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] = [%d/%d] (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2);
			setArg(script[pos + 1], arg1, val2);
			break;
		case kOpAdd:
			if (numParams != 3)
				error("kOpAdd had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d]=%d + [%d/%d]=%d (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2, script[pos+3], arg3, val3, val2 + val3);
			setArg(script[pos + 1], arg1, val2 + val3);
			break;
		case kOpSubtract:
			if (numParams != 3)
				error("kOpSubtract had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d]=%d - [%d/%d]=%d (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2, script[pos+3], arg3, val3, val2 - val3);
			setArg(script[pos + 1], arg1, val2 - val3);
			break;
		case kOpMultiply:
			if (numParams != 3)
				error("kOpMultiply had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d]=%d * [%d/%d]=%d (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2, script[pos+3], arg3, val3, val2 * val3);
			setArg(script[pos + 1], arg1, val2 * val3);
			break;
		case kOpDivide:
			if (numParams != 3)
				error("kOpDivide had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			if (val3 == 0)
				error("script tried to divide by zero");
			debug(9, "[%d/%d] = [%d/%d]=%d / [%d/%d]=%d (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2, script[pos+3], arg3, val3, val2 / val3);
			setArg(script[pos + 1], arg1, val2 / val3);
			break;
		case kOpModulo:
			if (numParams != 3)
				error("kOpModulo had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			if (val3 == 0)
				error("script tried to divide by zero (modulo)");
			debug(9, "[%d/%d] = [%d/%d]=%d %% [%d/%d]=%d (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2, script[pos+3], arg3, val3, val2 % val3);
			setArg(script[pos + 1], arg1, val2 % val3);
			break;
		case kOpMaybeAlsoAssign:
			if (numParams != 2)
				error("kOpMaybeAlsoAssign had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] =(?) [%d/%d] (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2);
			setArg(script[pos + 1], arg1, val2);
			break;
		case kOpBooleanAssign:
			if (numParams != 2)
				error("kOpBooleanAssign had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] = [%d/%d] (%d) ? 1 : 0", script[pos + 1], arg1, script[pos + 2], arg2, val2);
			setArg(script[pos + 1], arg1, val2 ? 1 : 0);
			break;
		case kOpNegate:
			if (numParams != 2)
				error("kOpNegate had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] = -[%d/%d] (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2);
			setArg(script[pos + 1], arg1, -val2);
			break;
		case kOpAnd:
			if (numParams != 3)
				error("kOpAnd had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d]=%d & [%d/%d]=%d (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2, script[pos+3], arg3, val3, val2 & val3);
			setArg(script[pos + 1], arg1, val2 & val3);
			break;
		case kOpOr:
			if (numParams != 3)
				error("kOpOr had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d]=%d | [%d/%d]=%d (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2, script[pos+3], arg3, val3, val2 | val3);
			setArg(script[pos + 1], arg1, val2 | val3);
			break;
		case kOpXor:
			if (numParams != 3)
				error("kOpXor had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d]=%d ^ [%d/%d]=%d (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2, script[pos+3], arg3, val3, val2 ^ val3);
			setArg(script[pos + 1], arg1, val2 ^ val3);
			break;
		case kOpNot:
			if (numParams != 2)
				error("kOpNot had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] = ![%d/%d] (!%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2);
			setArg(script[pos + 1], arg1, val2 ? 0 : 1);
			break;
		case kOpSqrt:
			if (numParams != 2)
				error("kOpSqrt had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] = sqrt([%d/%d] (%d))", script[pos + 1], arg1, script[pos + 2], arg2, val2);
			setArg(script[pos + 1], arg1, (int16)sqrt((double)val2));
			break;
		case kOpRandom:
			if (numParams != 3)
				error("kOpRandom had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			val1 = _rnd->getRandomNumberRng(val2, val3);
			debug(9, "[%d/%d] = rnd([%d/%d]=%d, [%d/%d]=%d) (%d)", script[pos + 1], arg1, script[pos + 2], arg2, val2, script[pos+3], arg3, val3, val1);
			setArg(script[pos + 1], arg1, val1);
			break;
		case kOpExecuteScript:
			if (numParams != 1)
				error("kOpExecuteScript had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			debug(8, "run script [%d/%d]=%d", script[pos + 1], arg1, val1);
			runScript(val1);
			debug(8, "done run script");
			break;
		case kOpCallFunc:
			if (numParams != 1)
				error("kOpCallFunc had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			debug(8, "%d(%d, %d, %d)", (uint16)val1, _vars[1], _vars[2], _vars[3]);
			_vars[0] = scriptFuncCall(val1, _vars[1], _vars[2], _vars[3]);
			break;
		case kOpBoolLessThanEq:
			if (numParams != 2)
				error("kOpBoolLessThanEq had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] <= [%d/%d]? (%d <= %d)", script[pos + 1], arg1, script[pos + 2], arg2, val1, val2);
			lastResult = (val1 <= val2);
			break;
		case kOpBoolLessThan:
			if (numParams != 2)
				error("kOpBoolLessThan had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] < [%d/%d]? (%d < %d)", script[pos + 1], arg1, script[pos + 2], arg2, val1, val2);
			lastResult = (val1 < val2);
			break;
		case kOpBoolGreaterThanEq:
			if (numParams != 2)
				error("kOpBoolGreaterThanEq had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] >= [%d/%d]? (%d >= %d)", script[pos + 1], arg1, script[pos + 2], arg2, val1, val2);
			lastResult = (val1 >= val2);
			break;
		case kOpBoolGreaterThan:
			if (numParams != 2)
				error("kOpBoolGreaterThan had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] > [%d/%d]? (%d > %d)", script[pos + 1], arg1, script[pos + 2], arg2, val1, val2);
			lastResult = (val1 > val2);
			break;
		case kOpBoolEqual:
			if (numParams != 2)
				error("kOpBoolEqual had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] == [%d/%d]? (%d == %d)", script[pos + 1], arg1, script[pos + 2], arg2, val1, val2);
			lastResult = (val1 == val2);
			break;
		case kOpBoolNotEqual:
			if (numParams != 2)
				error("kOpBoolNotEqual had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "[%d/%d] != [%d/%d]? (%d != %d)", script[pos + 1], arg1, script[pos + 2], arg2, val1, val2);
			lastResult = (val1 != val2);
			break;
		case kOpSaveArgs:
			if (numParams != 0)
				error("kOpSaveArgs had wrong number of params (%d)", numParams);
			debug(9, "save args");
			for (uint i = 1; i < 19; i++)
				_stack[stackBase + i] = _vars[i];
			break;
		case kOpRestoreArgs:
			if (numParams != 0)
				error("kOpRestoreArgs had wrong number of params (%d)", numParams);
			debug(9, "restore args");
			for (uint i = 1; i < 19; i++)
				_vars[i] = _stack[stackBase + i];
			break;
		case kOpReturn:
			if (numParams != 1)
				error("kOpReturn had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			debug(9, "return [%d/%d]=%d", script[pos + 1], arg1, val1);
			_vars[0] = val1;
			break;
		case kOpLessThanEq:
			if (numParams != 3)
				error("kOpLessThanEq had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d] <= [%d/%d]? (%d <= %d)", script[pos + 1], arg1, script[pos + 2], arg2, script[pos + 3], arg3, val3, val2);
			setArg(script[pos + 1], arg1, (val3 <= val2) ? 1 : 0);
			break;
		case kOpLessThan:
			if (numParams != 3)
				error("kOpLessThan had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d] < [%d/%d]? (%d < %d)", script[pos + 1], arg1, script[pos + 2], arg2, script[pos + 3], arg3, val3, val2);
			setArg(script[pos + 1], arg1, (val3 < val2) ? 1 : 0);
			break;
		case kOpGreaterThanEq:
			if (numParams != 3)
				error("kOpGreaterThanEq had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d] >= [%d/%d]? (%d >= %d)", script[pos + 1], arg1, script[pos + 2], arg2, script[pos + 3], arg3, val3, val2);
			setArg(script[pos + 1], arg1, (val3 >= val2) ? 1 : 0);
			break;
		case kOpGreaterThan:
			if (numParams != 3)
				error("kOpGreaterThan had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d] > [%d/%d]? (%d > %d)", script[pos + 1], arg1, script[pos + 2], arg2, script[pos + 3], arg3, val3, val2);
			setArg(script[pos + 1], arg1, (val3 > val2) ? 1 : 0);
			break;
		case kOpEqual:
			if (numParams != 3)
				error("kOpEqual had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d] == [%d/%d]? (%d == %d)", script[pos + 1], arg1, script[pos + 2], arg2, script[pos + 3], arg3, val2, val3);
			setArg(script[pos + 1], arg1, (val3 == val2) ? 1 : 0);
			break;
		case kOpNotEqual:
			if (numParams != 3)
				error("kOpNotEqual had wrong number of params (%d)", numParams);
			val2 = getArg(script[pos + 2], arg2);
			val3 = getArg(script[pos + 3], arg3);
			debug(9, "[%d/%d] = [%d/%d] != [%d/%d]? (%d != %d)", script[pos + 1], arg1, script[pos + 2], arg2, script[pos + 3], arg3, val2, val3);
			setArg(script[pos + 1], arg1, (val3 != val2) ? 1 : 0);
			break;
		case kOpJump:
			if (numParams != 1)
				error("kOpJump had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			debug(9, "jump by [%d/%d]=%d", script[pos + 1], arg1, val1);
			pos += val1;
			break;
		case kOpJumpIfNot:
			if (numParams != 1)
				error("kOpJumpIfNot had wrong number of params (%d)", numParams);
			if (lastResult)
				break;
			val1 = getArg(script[pos + 1], arg1);
			debug(9, "jump if not, by [%d/%d]=%d", script[pos + 1], arg1, val1);
			pos += val1;
			break;
		case kOpJumpIf:
			if (numParams != 1)
				error("kOpJumpIf had wrong number of params (%d)", numParams);
			if (!lastResult)
				break;
			val1 = getArg(script[pos + 1], arg1);
			debug(9, "jump if, by [%d/%d]=%d", script[pos + 1], arg1, val1);
			pos += val1;
			break;
		case kOpJumpIfNotValue:
			if (numParams != 2)
				error("kOpJumpIfNotValue had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "jump if not [%d/%d]=%d", script[pos + 1], arg1, val1);
			if (val1)
				break;
			debug(9, "--> jump by [%d/%d]=%d", script[pos + 2], arg2, val2);
			pos += val2;
			break;
		case kOpJumpIfValue:
			if (numParams != 2)
				error("kOpJumpIfValue had wrong number of params (%d)", numParams);
			val1 = getArg(script[pos + 1], arg1);
			val2 = getArg(script[pos + 2], arg2);
			debug(9, "jump if [%d/%d]=%d", script[pos + 1], arg1, val1);
			if (!val1)
				break;
			debug(9, "--> jump by [%d/%d]=%d", script[pos + 2], arg2, val2);
			pos += val2;
			break;
		default:
			error("unknown script op 0x%02x", op);
		}
		pos += (1 + numParams);

		if (op == kOpReturn)
			break;
	}

	delete[] script;
	_stack.resize(_stack.size() - 19);
}

int16 ComposerEngine::scriptFuncCall(uint16 id, int16 param1, int16 param2, int16 param3) {
	switch (id) {
	case kFuncPlayAnim:
		debug(3, "kFuncPlayAnim(%d, %d, %d)", param1, param2, param3);
		playAnimation(param1, param2, param3, 0);
		return 1; // TODO: return 0 on failure
	case kFuncStopAnim:
		debug(3, "kFuncStopAnim(%d)", param1);
		for (Common::List<Animation *>::iterator i = _anims.begin(); i != _anims.end(); i++) {
			if ((*i)->_id == param1)
				stopAnimation(*i);
		}
		return 0;
	case kFuncQueueScript:
		debug(3, "kFuncQueueScript(%d, %d, %d)", param1, param2, param3);
		_queuedScripts[param1]._baseTime = _currentTime;
		_queuedScripts[param1]._duration = 10 * param2;
		_queuedScripts[param1]._count = 0xffffffff;
		_queuedScripts[param1]._scriptId = param3;
		return 0;
	case kFuncDequeueScript:
		debug(3, "kFuncDequeueScript(%d)", param1);
		_queuedScripts[param1]._count = 0;
		_queuedScripts[param1]._scriptId = 0;
		return 0;
	case kFuncSetCursor:
		debug(3, "kSetCursor(%d, (%d, %d))", param1, param2, param3);
		{
		uint16 oldCursor = _mouseSpriteId;
		setCursor(param1, Common::Point(param2, param3));
		return oldCursor;
		}
	case kFuncGetCursor:
		debug(3, "kFuncGetCursor()");
		return _mouseSpriteId;
	case kFuncShowCursor:
		debug(3, "kFuncShowCursor()");
		setCursorVisible(true);
		return 0;
	case kFuncHideCursor:
		debug(3, "kFuncHideCursor()");
		setCursorVisible(false);
		return 0;
	case kFuncActivateButton:
		debug(3, "kFuncActivateButton(%d)", param1);
		setButtonActive(param1, true);
		return 1;
	case kFuncDeactivateButton:
		debug(3, "kFuncDeactivateButton(%d)", param1);
		setButtonActive(param1, false);
		return 1;
	case kFuncNewPage:
		debug(3, "kFuncNewPage(%d, %d)", param1, param2);
		_pendingPageChanges.push_back(PendingPageChange(param1, true));
		_pendingPageChanges.push_back(PendingPageChange(param2, false));
		return 1;
	case kFuncLoadPage:
		debug(3, "kFuncLoadPage(%d)", param1);
		_pendingPageChanges.push_back(PendingPageChange(param1, false));
		return 1;
	case kFuncUnloadPage:
		debug(3, "kFuncUnloadPage(%d)", param1);
		_pendingPageChanges.push_back(PendingPageChange(param1, true));
		return 1;
	case kFuncSetPalette:
		// TODO: return 0 if not disabling (0) and doesn't exist
		debug(4, "kFuncSetPalette(%d, %d)", param1, param2);
		// this seems only needed for a script bug in the Gregory credits, sigh
		if ((uint16)param2 > 100)
			param2 = 100;
		loadCTBL(param1, param2);
		return 1;
	case kFuncSaveVars:
		debug(3, "kFuncSaveVars(%d)", param1);
		{
		Common::String filename = _targetName + Common::String::format(".%03d", param1);
		Common::WriteStream *stream = _saveFileMan->openForSaving(filename);
		for (uint i = 0; i < 1000; i++) {
			stream->writeUint16LE(_vars[i]);
		}
		delete stream;
		}
		return 1;
	case kFuncLoadVars:
		debug(3, "kFuncLoadVars(%d, %d, %d)", param1, param2, param3);
		{
		Common::String filename = _targetName + Common::String::format(".%03d", param1);
		Common::SeekableReadStream *stream = _saveFileMan->openForLoading(filename);
		if (!stream) {
			if (!_bookIni.hasKey(Common::String::format("%d", param1), "Data"))
				return 0;
			filename = getFilename("Data", param1);
			Common::File *file = new Common::File();
			if (!file->open(filename))
				error("couldn't open '%s' to get vars id '%d'", filename.c_str(), param1);
			stream = file;
		}
		if (param3 == 0)
			param3 = 1000;
		if (param2 < 0 || param3 < 0 || param2 + param3 > 1000)
			error("can't read %d entries into %d from file '%s' for vars id '%d'", param3, param2, filename.c_str(), param1);
		stream->skip(param2 * 2);
		for (uint i = 0; i < (uint)param3; i++) {
			if (stream->pos() + 1 > stream->size())
				break;
			_vars[param2 + i] = stream->readUint16LE();
		}
		delete stream;
		}
		return 1;
	case kFuncQueueScriptOnce:
		debug(3, "kFuncQueueScriptOnce(%d, %d, %d)", param1, param2, param3);
		_queuedScripts[param1]._baseTime = _currentTime;
		_queuedScripts[param1]._duration = 10 * param2;
		_queuedScripts[param1]._count = 1;
		_queuedScripts[param1]._scriptId = param3;
		return 0;
	case kFuncGetMousePos:
		debug(3, "kFuncGetMousePos(%d, %d)", param1, param2);
		_vars[param1] = _lastMousePos.x;
		_vars[param2] = _lastMousePos.y;
		return 0;
	case kFuncChangeBackground:
		debug(3, "kFuncChangeBackground(%d)", param1);
		// TODO: return 1 if background existed, else 0
		setBackground(param1);
		return 1;
	case kFuncSetBackgroundColor:
		// TODO
		warning("ignoring kFuncSetBackgroundColor(%d)", param1);
		return 0;
	case kFuncClearSprites:
		debug(3, "kFuncClearSprites()");
		removeSprite(0, 0);
		return 0;
	case kFuncAddSprite:
		{
		Common::Point pos(_vars[param3], _vars[param3 + 1]);
		int16 zorder = _vars[param3 + 2];
		debug(3, "kFuncAddSprite(%d, %d, [%d = (%d, %d), %d])", param1, param2, param3, pos.x, pos.y, zorder);
		addSprite(param1, param2, zorder, pos);
		}
		return 0;
	case kFuncRemoveSprite:
		debug(3, "kFuncRemoveSprite(%d, %d)", param1, param2);
		removeSprite(param1, param2);
		return 0;
	case kFuncQuit:
		debug(3, "kFuncQuit()");
		quitGame();
		return 0;
	case kFuncSaveData:
		// TODO
		warning("ignoring kFuncSaveData(%d, %d, %d)", param1, param2, param3);
		return 1;
	case kFuncLoadData:
		debug(3, "kFuncLoadData(%d, %d, %d)", param1, param2, param3);
		{
		Common::String filename = getFilename("Data", param1);
		Common::File *file = new Common::File();
		if (!file->open(filename))
			error("couldn't open '%s' to get data id '%d'", filename.c_str(), param1);
		if (param3 == 0)
			param3 = 1000;
		if (param2 < 0 || param3 < 0 || param2 + param3 > 1000)
			error("can't read %d entries into %d from file '%s' for data id '%d'", param3, param2, filename.c_str(), param1);
		for (uint i = 0; i < (uint)param3; i++) {
			if (file->pos() + 1 > file->size())
				break;
			_vars[param2 + i] = file->readUint16LE();
		}
		delete file;
		}
		return 1;
	case kFuncGetSpriteSize:
		debug(3, "kFuncGetSpriteSize(%d, %d, %d)", param1, param2, param3);
		int16 width, height;
		width = 0;
		height = 0;
		{
		Common::SeekableReadStream *stream = getStreamForSprite(param1);
		if (stream) {
			stream->readUint16LE();
			height = stream->readSint16LE();
			width = stream->readSint16LE();
			delete stream;
		}
		}
		_vars[param2] = width;
		_vars[param3] = height;
		return 0;
	default:
		error("unknown scriptFuncCall %d(%d, %d, %d)", (uint32)id, param1, param2, param3);
	}
}

OldScript::OldScript(uint16 id, Common::SeekableReadStream *stream) : _id(id), _stream(stream) {
	_size = _stream->readUint32LE();
	_stream->skip(2);
	_currDelay = 0;
	_zorder = 10;
}

OldScript::~OldScript() {
	delete _stream;
}

void ComposerEngine::runOldScript(uint16 id, uint16 wait) {
	stopOldScript(id);

	Common::SeekableReadStream *stream = getResource(ID_SCRP, id);
	OldScript *script = new OldScript(id, stream);
	script->_currDelay = wait;
	_oldScripts.push_back(script);
}

void ComposerEngine::stopOldScript(uint16 id) {
	// FIXME: this could potentially (in the case of buggy script) be called on an in-use script

	for (Common::List<OldScript *>::iterator i = _oldScripts.begin(); i != _oldScripts.end(); i++) {
		if ((*i)->_id == id) {
			delete *i;
			i = _oldScripts.reverse_erase(i);
		}
	}
}

void ComposerEngine::tickOldScripts() {
	for (Common::List<OldScript *>::iterator i = _oldScripts.begin(); i != _oldScripts.end(); i++) {
		if (!tickOldScript(*i)) {
			delete *i;
			i = _oldScripts.reverse_erase(i);
		}
	}
}

enum {
	kOldOpNoOp = 0,
	kOldOpReplaceSprite = 1,
	kOldOpPlayWav = 2,
	kOldOpRunScript = 3,
	kOldOpStopScript = 4,
	kOldOpActivateButton = 5,
	kOldOpDeactivateButton = 6,
	kOldOpDrawSprite = 7,
	kOldOpRemoveSprite = 8,
	kOldOpDisableMouseInput = 9,
	kOldOpEnableMouseInput = 10,
	kOldOpWait = 11,
	kOldOpRandWait = 12,
	kOldOpDrawGlobalSprite = 13,
	kOldOpRemoveGlobalSprite = 14,
	kOldOpSetZOrder = 15,
	kOldOpPlayPipe = 16,
	kOldOpStopPipe = 17,
	kOldOpNewScreen = 20,
	kOldOpRunRandom = 22
};

bool ComposerEngine::tickOldScript(OldScript *script) {
	if (script->_currDelay) {
		script->_currDelay--;
		return true;
	}

	bool running = true;
	bool erasedOldSprite = false;
	while (running && script->_stream->pos() < (int)script->_size) {
		uint16 spriteId, scriptId, buttonId, pipeId;
		Common::Point spritePos;

		script->_stream->skip(0);
		byte op = script->_stream->readByte();
		switch (op) {
		case kOldOpNoOp:
			debug(3, "kOldOpNoOp()");
			running = false;
			break;
		case kOldOpReplaceSprite:
			if (!erasedOldSprite) {
				removeSprite(0, script->_id);
				erasedOldSprite = true;
			}

			spriteId = script->_stream->readUint16LE();
			spritePos.x = script->_stream->readSint16LE();
			spritePos.y = script->_stream->readSint16LE();
			debug(3, "kOldOpReplaceSprite(%d, %d, %d)", spriteId, spritePos.x, spritePos.y);
			addSprite(spriteId, script->_id, script->_zorder, spritePos);
			break;
		case kOldOpPlayWav:
			uint16 wavId, prio;
			wavId = script->_stream->readUint16LE();
			prio = script->_stream->readUint16LE();
			debug(3, "kOldOpPlayWav(%d, %d)", wavId, prio);
			playWaveForAnim(wavId, prio, false);
			break;
		case kOldOpRunScript:
			scriptId = script->_stream->readUint16LE();
			debug(3, "kOldOpRunScript(%d)", scriptId);
			if (scriptId == script->_id) {
				// reset ourselves
				removeSprite(0, script->_id);
				script->_stream->seek(6);
			} else {
				runScript(scriptId);
			}
			break;
		case kOldOpStopScript:
			scriptId = script->_stream->readUint16LE();
			debug(3, "kOldOpStopScript(%d)", scriptId);
			removeSprite(0, scriptId);
			stopOldScript(scriptId);
			break;
		case kOldOpActivateButton:
			buttonId = script->_stream->readUint16LE();
			debug(3, "kOldOpActivateButton(%d)", buttonId);
			setButtonActive(buttonId, true);
			break;
		case kOldOpDeactivateButton:
			buttonId = script->_stream->readUint16LE();
			debug(3, "kOldOpDeactivateButton(%d)", buttonId);
			setButtonActive(buttonId, false);
			break;
		case kOldOpDrawSprite:
			spriteId = script->_stream->readUint16LE();
			spritePos.x = script->_stream->readSint16LE();
			spritePos.y = script->_stream->readSint16LE();
			debug(3, "kOldOpDrawSprite(%d, %d, %d)", spriteId, spritePos.x, spritePos.y);
			addSprite(spriteId, script->_id, script->_zorder, spritePos);
			break;
		case kOldOpRemoveSprite:
			spriteId = script->_stream->readUint16LE();
			debug(3, "kOldOpRemoveSprite(%d)", spriteId);
			removeSprite(spriteId, script->_id);
			break;
		case kOldOpDisableMouseInput:
			debug(3, "kOldOpDisableMouseInput()");
			setCursorVisible(false);
			break;
		case kOldOpEnableMouseInput:
			debug(3, "kOldOpEnableMouseInput()");
			setCursorVisible(true);
			break;
		case kOldOpWait:
			script->_currDelay = script->_stream->readUint16LE();
			debug(3, "kOldOpWait(%d)", script->_currDelay);
			break;
		case kOldOpRandWait:
			uint16 min, max;
			min = script->_stream->readUint16LE();
			max = script->_stream->readUint16LE();
			debug(3, "kOldOpRandWait(%d, %d)", min, max);
			script->_currDelay = _rnd->getRandomNumberRng(min, max);
			break;
		case kOldOpDrawGlobalSprite:
			spriteId = script->_stream->readUint16LE();
			spritePos.x = script->_stream->readSint16LE();
			spritePos.y = script->_stream->readSint16LE();
			debug(3, "kOldOpDrawGlobalSprite(%d, %d, %d)", spriteId, spritePos.x, spritePos.y);
			addSprite(spriteId, 0, script->_zorder, spritePos);
			break;
		case kOldOpRemoveGlobalSprite:
			spriteId = script->_stream->readUint16LE();
			debug(3, "kOldOpRemoveGlobalSprite(%d)", spriteId);
			removeSprite(spriteId, 0);
			break;
		case kOldOpSetZOrder:
			script->_zorder = script->_stream->readUint16LE();
			debug(3, "kOldOpSetZOrder(%d)", script->_zorder);
			break;
		case kOldOpPlayPipe:
			pipeId = script->_stream->readUint16LE();
			debug(3, "kOldOpPlayPipe(%d)", pipeId);
			playPipe(pipeId);
			break;
		case kOldOpStopPipe:
			pipeId = script->_stream->readUint16LE();
			debug(3, "kOldOpStopPipe(%d)", pipeId);
			// yes, pipeId is ignored here..
			stopPipes();
			break;
		case kOldOpNewScreen:
			uint16 newScreenId;
			newScreenId = script->_stream->readUint16LE();
			debug(3, "kOldOpNewScreen(%d)", newScreenId);
			if (!newScreenId) {
				quitGame();
			} else {
				_pendingPageChanges.clear();
				_pendingPageChanges.push_back(PendingPageChange(newScreenId, false));
			}
			break;
		case kOldOpRunRandom:
			uint16 randomId;
			randomId = script->_stream->readUint16LE();
			debug(3, "kOldOpRunRandom(%d)", randomId);
			if (!_randomEvents.contains(randomId)) {
				warning("kOldOpRunRandom found no entries for id %d", randomId);
			} else {
				uint32 randValue = _rnd->getRandomNumberRng(0, 32767);
				const Common::Array<RandomEvent> &events = _randomEvents[randomId];
				uint i = 0;
				for (i = 0; i < events.size(); i++) {
					if ((i + 1 == events.size()) || (randValue <= events[i].weight)) {
						runScript(events[i].scriptId);
						break;
					} else {
						randValue -= events[i].weight;
					}
				}
			}
			break;
		default:
			error("unknown oldScript op %d", op);
		}
	}

	if (script->_stream->pos() >= (int)script->_size) {
		// stop running if we ran out of script
		removeSprite(0, script->_id);
		return false;
	}

	return true;
}

} // End of namespace Composer
