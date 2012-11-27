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

// Scripting module: Script resource handling functions
#include "saga/saga.h"

#include "saga/gfx.h"
#include "saga/console.h"

#include "saga/animation.h"
#include "saga/script.h"
#include "saga/interface.h"
#include "saga/itedata.h"
#include "saga/scene.h"
#include "saga/events.h"
#include "saga/actor.h"
#include "saga/objectmap.h"
#include "saga/isomap.h"
#include "saga/resource.h"

namespace Saga {

#define RID_SCENE1_VOICE_START 57
#define RID_SCENE1_VOICE_END 186

SAGA1Script::SAGA1Script(SagaEngine *vm) : Script(vm) {
	ResourceContext *resourceContext;
	ByteArray resourceData;
	int prevTell;
	uint ui;
	int j;
	ByteArray stringsData;

	//initialize member variables
	_abortEnabled = true;
	_skipSpeeches = false;
	_conversingThread = NULL;
	_firstObjectSet = false;
	_secondObjectNeeded = false;
	_pendingVerb = getVerbType(kVerbNone);
	_currentVerb = getVerbType(kVerbNone);
	_stickyVerb = getVerbType(kVerbWalkTo);
	_leftButtonVerb = getVerbType(kVerbNone);
	_rightButtonVerb = getVerbType(kVerbNone);
	_pointerObject = ID_NOTHING;

	_staticSize = 0;
	_commonBuffer.resize(COMMON_BUFFER_SIZE);

	debug(8, "Initializing scripting subsystem");
	// Load script resource file context
	_scriptContext = _vm->_resource->getContext(GAME_SCRIPTFILE);
	if (_scriptContext == NULL) {
		error("Script::Script() script context not found");
	}

	resourceContext = _vm->_resource->getContext(GAME_RESOURCEFILE);
	if (resourceContext == NULL) {
		error("Script::Script() resource context not found");
	}

	uint32 scriptResourceId = 0;
	scriptResourceId = _vm->getResourceDescription()->moduleLUTResourceId;
	debug(3, "Loading module LUT from resource %i", scriptResourceId);
	_vm->_resource->loadResource(resourceContext, scriptResourceId, resourceData);

	// Create logical script LUT from resource
	if (resourceData.size() % 22 == 0) {			// ITE CD
		_modulesLUTEntryLen = 22;
	} else if (resourceData.size() % 16 == 0) {	// ITE disk, IHNM
		_modulesLUTEntryLen = 16;
	} else {
		error("Script::Script() Invalid script lookup table length (%i)", (int)resourceData.size());
	}

	// Calculate number of entries
	int modulesCount = resourceData.size() / _modulesLUTEntryLen;

	debug(3, "LUT has %i entries", modulesCount);

	// Allocate space for logical LUT
	_modules.resize(modulesCount);

	// Convert LUT resource to logical LUT
	ByteArrayReadStreamEndian scriptS(resourceData, resourceContext->isBigEndian());
	for (ui = 0; ui < _modules.size(); ui++) {

		prevTell = scriptS.pos();
		_modules[ui].scriptResourceId = scriptS.readUint16();
		_modules[ui].stringsResourceId = scriptS.readUint16();
		_modules[ui].voicesResourceId = scriptS.readUint16();

		// Skip the unused portion of the structure
		for (j = scriptS.pos(); j < prevTell + _modulesLUTEntryLen; j++) {
			if (scriptS.readByte() != 0)
				warning("Unused scriptLUT part isn't really unused for LUT %d (pos: %d)", ui, j);
		}
	}

	// TODO
	//
	// In ITE, the "main strings" resource contains both the verb strings
	// and the object names.
	//
	// In IHNM, the "main strings" contains the verb strings, but not the
	// object names. At least, I think that's the case.

	_vm->_resource->loadResource(resourceContext, _vm->getResourceDescription()->mainStringsResourceId, stringsData);

	_vm->loadStrings(_mainStrings, stringsData);

	setupScriptOpcodeList();

	// Setup script functions
	switch (_vm->getGameId()) {
		case GID_ITE:
			setupITEScriptFuncList();
			break;
#ifdef ENABLE_IHNM
		case GID_IHNM:
			setupIHNMScriptFuncList();
			break;
#endif
	}
}

SAGA1Script::~SAGA1Script() {
	debug(8, "Shutting down scripting subsystem.");
}

SAGA2Script::SAGA2Script(SagaEngine *vm) : Script(vm) {
	ByteArray resourceData;

	debug(8, "Initializing scripting subsystem");
	// Load script resource file context
	_scriptContext = _vm->_resource->getContext(GAME_SCRIPTFILE);
	if (_scriptContext == NULL) {
		error("Script::Script() script context not found");
	}

	// Script export segment (lookup table)
	uint32 saga2ExportSegId = MKTAG('_','E','X','P');
	int32 entryNum = _scriptContext->getEntryNum(saga2ExportSegId);
	if (entryNum < 0)
		error("Unable to locate the script's export segment");
	debug(3, "Loading module LUT from resource %i", entryNum);
	_vm->_resource->loadResource(_scriptContext, (uint32)entryNum, resourceData);

	_modulesLUTEntryLen = sizeof(uint32);

	// Calculate number of entries
	int modulesCount = resourceData.size() / _modulesLUTEntryLen + 1;

	debug(3, "LUT has %i entries", modulesCount);

	// Script data segment
	/*
	uint32 saga2DataSegId = MKTAG('_','_','D','A');
	entryNum = _scriptContext->getEntryNum(saga2DataSegId);
	if (entryNum < 0)
		error("Unable to locate the script's data segment");
	debug(3, "Loading module data from resource %i", entryNum);
	_vm->_resource->loadResource(_scriptContext, (uint32)entryNum, resourcePointer, resourceLength);
	*/

	// TODO
}

SAGA2Script::~SAGA2Script() {
	debug(8, "Shutting down scripting subsystem.");

	// TODO
}

// Initializes the scripting module.
// Loads script resource look-up table, initializes script data system
Script::Script(SagaEngine *vm) : _vm(vm) {

}

// Shut down script module gracefully; free all allocated module resources
Script::~Script() {

}

// Script opcodes
#define OPCODE(x) {&Script::x, #x}

void Script::setupScriptOpcodeList() {
	static const ScriptOpDescription SAGA1ScriptOpcodes[] = {
		OPCODE(opDummy),		// 00: Undefined
		// Internal operations
		OPCODE(opNextBlock),	// 01: Continue execution at next block
		OPCODE(opDup),			// 02: Duplicate 16-bit value on stack
		OPCODE(opDrop),			// 03: Drop 16-bit value on stack
		// Primary values
		OPCODE(opZero),			// 04: Push a zero on the stack
		OPCODE(opOne),			// 05: Push a one on the stack
		OPCODE(opConstInt),		// 06: Constant integer
		OPCODE(opDummy),		// 07: Constant ID reference (unused)
		OPCODE(opStrLit),		// 08: String literal
		OPCODE(opDummy),		// 09: Symbol address (unused)
		OPCODE(opDummy),		// 10: Symbol contents (unused)
		// References within this module
		OPCODE(opGetFlag),		// 11: Read flag bit
		OPCODE(opGetInt),		// 12: Read integer
		OPCODE(opDummy),		// 13: Read string (unused)
		OPCODE(opDummy),		// 14: Read id (unused)
		OPCODE(opPutFlag),		// 15: Write flag bit
		OPCODE(opPutInt),		// 16: Write integer
		OPCODE(opDummy),		// 17: Write string (unused)
		OPCODE(opDummy),		// 18: Write id (unused)
		// Void versions, which consume their arguments
		OPCODE(opPutFlagV),		// 19: Write flag bit
		OPCODE(opPutIntV),		// 20: Write integer
		OPCODE(opDummy),		// 21: Write string (unused)
		OPCODE(opDummy),		// 22: Write id (unused)
		// Function calling
		OPCODE(opCall),			// 23: Call function
		OPCODE(opCcall),		// 24: Call C function
		OPCODE(opCcallV),		// 25: Call C function ()
		OPCODE(opEnter),		// 26: Enter a function
		OPCODE(opReturn),		// 27: Return from a function
		OPCODE(opReturnV),		// 28: Return from a function ()
		// Branching
		OPCODE(opJmp),			// 29
		OPCODE(opJmpTrueV),		// 30: Test argument and consume it
		OPCODE(opJmpFalseV),	// 31: Test argument and consume it
		OPCODE(opJmpTrue),		// 32: Test argument but don't consume it
		OPCODE(opJmpFalse),		// 33: Test argument but don't consume it
		OPCODE(opJmpSwitch),	// 34: Switch (integer)
		OPCODE(opDummy),		// 35: Switch (string) (unused)
		OPCODE(opJmpRandom),	// 36: Random jump
		// Unary operators
		OPCODE(opNegate),		// 37
		OPCODE(opNot),			// 38
		OPCODE(opCompl),		// 39
		OPCODE(opIncV),			// 40: Increment, don't push
		OPCODE(opDecV),			// 41: Increment, don't push
		OPCODE(opPostInc),		// 42
		OPCODE(opPostDec),		// 43
		// Arithmetic
		OPCODE(opAdd),			// 44
		OPCODE(opSub),			// 45
		OPCODE(opMul),			// 46
		OPCODE(opDiv),			// 47
		OPCODE(opMod),			// 48
		// Conditional
		OPCODE(opDummy),		// 49: opConditional (unused)
		OPCODE(opDummy),		// 50: opComma (unused)
		// Comparison
		OPCODE(opEq),			// 51
		OPCODE(opNe),			// 52
		OPCODE(opGt),			// 53
		OPCODE(opLt),			// 54
		OPCODE(opGe),			// 55
		OPCODE(opLe),			// 56
		// String comparison
		OPCODE(opDummy),		// 57: opStrEq (unused)
		OPCODE(opDummy),		// 58: opStrNe (unused)
		OPCODE(opDummy),		// 59: opStrGt (unused)
		OPCODE(opDummy),		// 60: opStrLt (unused)
		OPCODE(opDummy),		// 61: opStrGe (unused)
		OPCODE(opDummy),		// 62: opStrLe (unused)
		// Shift
		OPCODE(opRsh),			// 63
		OPCODE(opLsh),			// 64
		// Bitwise
		OPCODE(opAnd),			// 65
		OPCODE(opOr),			// 66
		OPCODE(opXor),			// 67
		// Logical
		OPCODE(opLAnd),			// 68
		OPCODE(opLOr),			// 69
		OPCODE(opLXor),			// 70
		// String manipulation
		OPCODE(opDummy),		// 71: opStrCat, string concatenation (unused)
		OPCODE(opDummy),		// 72: opStrFormat, string formatting (unused)
		// Assignment
		OPCODE(opDummy),		// 73: assign (unused)
		OPCODE(opDummy),		// 74: += (unused)
		OPCODE(opDummy),		// 75: -= (unused)
		OPCODE(opDummy),		// 76: *= (unused)
		OPCODE(opDummy),		// 77: /= (unused)
		OPCODE(opDummy),		// 78: %= (unused)
		OPCODE(opDummy),		// 79: <<= (unused)
		OPCODE(opDummy),		// 80: >>= (unused)
		OPCODE(opDummy),		// 81: and (unused)
		OPCODE(opDummy),		// 82: or (unused)
		// Special
		OPCODE(opSpeak),		// 83
		OPCODE(opDialogBegin),	// 84
		OPCODE(opDialogEnd),	// 85
		OPCODE(opReply),		// 86
		OPCODE(opAnimate)		// 87
	};

#ifdef ENABLE_SAGA2
	static const ScriptOpDescription SAGA2ScriptOpcodes[] = {
		OPCODE(opDummy),		// 00: Undefined
		// Internal operations
		OPCODE(opNextBlock),	// 01: Continue execution at next block
		OPCODE(opDup),			// 02: Duplicate 16-bit value on stack
		OPCODE(opDrop),			// 03: Drop 16-bit value on stack
		// Primary values
		OPCODE(opZero),			// 04: Push a zero on the stack
		OPCODE(opOne),			// 05: Push a one on the stack
		OPCODE(opConstInt),		// 06: Constant integer
		OPCODE(opDummy),		// 07: Constant ID reference (unused)
		OPCODE(opStrLit),		// 08: String literal
		OPCODE(opDummy),		// 09: Symbol address (unused)
		OPCODE(opDummy),		// 10: Symbol contents (unused)
		OPCODE(opDummy),		// 11: Reference to "this" (unused)
		OPCODE(opDummy),		// 12: Dereference of an ID (unused)
		// References within this module
		OPCODE(opGetFlag),		// 13: Read flag bit
		OPCODE(opGetByte),		// 14: Read byte
		OPCODE(opGetInt),		// 15: Read integer
		OPCODE(opDummy),		// 16: Read string (unused)
		OPCODE(opDummy),		// 17: Read id (unused)
		OPCODE(opPutFlag),		// 18: Write flag bit
		OPCODE(opPutByte),		// 19: Write byte
		OPCODE(opPutInt),		// 20: Write integer
		OPCODE(opDummy),		// 21: Write string (unused)
		OPCODE(opDummy),		// 22: Write id (unused)
		OPCODE(opDummy),		// 23: Push effective address (unused)
		// Void versions, which consume their arguments
		OPCODE(opPutFlagV),		// 24: Write flag bit
		OPCODE(opPutByteV),		// 25: Write byte
		OPCODE(opPutIntV),		// 26: Write integer
		OPCODE(opDummy),		// 27: Write string (unused)
		OPCODE(opDummy),		// 28: Write id (unused)
		// Function calling
		OPCODE(opCallNear),		// 29: Call function in the same segment
		OPCODE(opCallFar),		// 30: Call function in other segment
		OPCODE(opCcall),		// 31: Call C function
		OPCODE(opCcallV),		// 32: Call C function ()
		OPCODE(opCallMember),	// 33: Call member function
		OPCODE(opCallMemberV),	// 34: Call member function ()
		OPCODE(opEnter),		// 35: Enter a function
		OPCODE(opReturn),		// 36: Return from a function
		OPCODE(opReturnV),		// 37: Return from a function ()
		// Branching
		OPCODE(opJmp),			// 38
		OPCODE(opJmpTrueV),		// 39: Test argument and consume it
		OPCODE(opJmpFalseV),	// 40: Test argument and consume it
		OPCODE(opJmpTrue),		// 41: Test argument but don't consume it
		OPCODE(opJmpFalse),		// 42: Test argument but don't consume it
		OPCODE(opJmpSwitch),	// 43: Switch (integer)
		OPCODE(opDummy),		// 44: Switch (string) (unused)
		OPCODE(opJmpRandom),	// 45: Random jump
		// Unary operators
		OPCODE(opNegate),		// 46
		OPCODE(opNot),			// 47
		OPCODE(opCompl),		// 48
		OPCODE(opIncV),			// 49: Increment, don't push
		OPCODE(opDecV),			// 50: Increment, don't push
		OPCODE(opPostInc),		// 51
		OPCODE(opPostDec),		// 52
		// Arithmetic
		OPCODE(opAdd),			// 53
		OPCODE(opSub),			// 54
		OPCODE(opMul),			// 55
		OPCODE(opDiv),			// 56
		OPCODE(opMod),			// 57
		// Conditional
		OPCODE(opDummy),		// 58: opConditional (unused)
		OPCODE(opDummy),		// 59: opComma (unused)
		// Comparison
		OPCODE(opEq),			// 60
		OPCODE(opNe),			// 61
		OPCODE(opGt),			// 62
		OPCODE(opLt),			// 63
		OPCODE(opGe),			// 64
		OPCODE(opLe),			// 65
		// String comparison
		OPCODE(opDummy),		// 66: opStrEq (unused)
		OPCODE(opDummy),		// 67: opStrNe (unused)
		OPCODE(opDummy),		// 68: opStrGt (unused)
		OPCODE(opDummy),		// 69: opStrLt (unused)
		OPCODE(opDummy),		// 70: opStrGe (unused)
		OPCODE(opDummy),		// 71: opStrLe (unused)
		// Shift
		OPCODE(opRsh),			// 72
		OPCODE(opLsh),			// 73
		// Bitwise
		OPCODE(opAnd),			// 74
		OPCODE(opOr),			// 75
		OPCODE(opXor),			// 76
		// Logical
		OPCODE(opLAnd),			// 77
		OPCODE(opLOr),			// 78
		OPCODE(opLXor),			// 79
		// String manipulation
		OPCODE(opDummy),		// 80: opStrCat, string concatenation (unused)
		OPCODE(opDummy),		// 81: opStrFormat, string formatting (unused)
		// Assignment
		OPCODE(opDummy),		// 82: assign (unused)
		OPCODE(opDummy),		// 83: += (unused)
		OPCODE(opDummy),		// 84: -= (unused)
		OPCODE(opDummy),		// 85: *= (unused)
		OPCODE(opDummy),		// 86: /= (unused)
		OPCODE(opDummy),		// 87: %= (unused)
		OPCODE(opDummy),		// 88: <<= (unused)
		OPCODE(opDummy),		// 89: >>= (unused)
		OPCODE(opDummy),		// 90: and (unused)
		OPCODE(opDummy),		// 91: or (unused)
		// Special
		OPCODE(opSpeak),		// 92
		OPCODE(opDialogBegin),	// 93
		OPCODE(opDialogEnd),	// 94
		OPCODE(opReply),		// 95
		OPCODE(opAnimate),		// 96
		OPCODE(opJmpSeedRandom),// 97: Seeded random jump
		OPCODE(opDummy)			// 98: Get seeded export number (unused)
	};
#endif

	if (!_vm->isSaga2()) {
		_scriptOpsList = SAGA1ScriptOpcodes;
#ifdef ENABLE_SAGA2
	} else {
		_scriptOpsList = SAGA2ScriptOpcodes;
#endif
	}
}


void Script::opDup(SCRIPTOP_PARAMS) {
	thread->push(thread->stackTop());
}

void Script::opDrop(SCRIPTOP_PARAMS) {
	thread->pop();
}

void Script::opZero(SCRIPTOP_PARAMS) {
	thread->push(0);
}

void Script::opOne(SCRIPTOP_PARAMS) {
	thread->push(1);
}

void Script::opConstInt(SCRIPTOP_PARAMS) {
	thread->push(scriptS->readSint16LE());
}

void Script::opStrLit(SCRIPTOP_PARAMS) {
	thread->push(scriptS->readSint16LE());
}

void Script::opGetFlag(SCRIPTOP_PARAMS) {
	byte *addr = thread->baseAddress(scriptS->readByte());
	int16 iparam1 = scriptS->readSint16LE();
	addr += (iparam1 >> 3);
	iparam1 = (1 << (iparam1 & 7));
	thread->push((*addr) & iparam1 ? 1 : 0);
}

void Script::opGetByte(SCRIPTOP_PARAMS) {
	// SAGA 2 opcode
	// TODO
	warning("opGetByte");
}

void Script::opGetInt(SCRIPTOP_PARAMS) {
	byte mode = scriptS->readByte();
	byte *addr = thread->baseAddress(mode);
	int16 iparam1 = scriptS->readSint16LE();
	addr += iparam1;
	thread->push(readUint16(addr, mode));
	debug(8, "0x%X", readUint16(addr, mode));
}

void Script::opPutFlag(SCRIPTOP_PARAMS) {
	byte *addr = thread->baseAddress(scriptS->readByte());
	int16 iparam1 = scriptS->readSint16LE();
	addr += (iparam1 >> 3);
	iparam1 = (1 << (iparam1 & 7));
	if (thread->stackTop()) {
		*addr |= iparam1;
	} else {
		*addr &= ~iparam1;
	}
}

void Script::opPutByte(SCRIPTOP_PARAMS) {
	// SAGA 2 opcode
	// TODO
	warning("opPutByte");
}

void Script::opPutInt(SCRIPTOP_PARAMS) {
	byte mode = scriptS->readByte();
	byte *addr = thread->baseAddress(mode);
	int16 iparam1 = scriptS->readSint16LE();
	addr += iparam1;
	writeUint16(addr, thread->stackTop(), mode);
}

void Script::opPutFlagV(SCRIPTOP_PARAMS) {
	byte *addr = thread->baseAddress(scriptS->readByte());
	int16 iparam1 = scriptS->readSint16LE();
	addr += (iparam1 >> 3);
	iparam1 = (1 << (iparam1 & 7));
	if (thread->pop()) {
		*addr |= iparam1;
	} else {
		*addr &= ~iparam1;
	}
}

void Script::opPutByteV(SCRIPTOP_PARAMS) {
	// SAGA 2 opcode
	// TODO
	warning("opPutByteV");
}

void Script::opPutIntV(SCRIPTOP_PARAMS) {
	byte mode = scriptS->readByte();
	byte *addr = thread->baseAddress(mode);
	int16 iparam1 = scriptS->readSint16LE();
	addr += iparam1;
	writeUint16(addr, thread->pop(), mode);
}

void Script::opCall(SCRIPTOP_PARAMS) {
	byte argumentsCount = scriptS->readByte();
	int16 iparam1 = scriptS->readByte();
	if (iparam1 != kAddressModule) {
		error("Script::runThread iparam1 != kAddressModule");
	}
	iparam1 = scriptS->readSint16LE();
	thread->push(argumentsCount);

	// NOTE: The original pushes the program
	// counter as a pointer here. But I don't think
	// we will have to do that.
	thread->push(scriptS->pos());
	// NOTE2: program counter is 32bit - so we should "emulate" it size - because kAddressStack relies on it
	thread->push(0);
	thread->_instructionOffset = iparam1;
}

void Script::opCallNear(SCRIPTOP_PARAMS) {
	// SAGA 2 opcode
	// TODO
	warning("opCallNear");
}

void Script::opCallFar(SCRIPTOP_PARAMS) {
	// SAGA 2 opcode
	// TODO
	warning("opCallFar");
}

void Script::opCcall(SCRIPTOP_PARAMS) {
	byte argumentsCount = scriptS->readByte();
	uint16 functionNumber = scriptS->readUint16LE();
	if (functionNumber >= ((_vm->getGameId() == GID_IHNM) ?
						   IHNM_SCRIPT_FUNCTION_MAX : ITE_SCRIPT_FUNCTION_MAX)) {
		error("Script::opCcall() Invalid script function number (%d)", functionNumber);
	}

	debug(2, "Calling #%d %s argCount=%i", functionNumber, _scriptFunctionsList[functionNumber].scriptFunctionName, argumentsCount);
	ScriptFunctionType scriptFunction = _scriptFunctionsList[functionNumber].scriptFunction;
	uint16 checkStackTopIndex = thread->_stackTopIndex + argumentsCount;
	(this->*scriptFunction)(thread, argumentsCount, stopParsing);
	if (stopParsing)
		return;

	if (scriptFunction == &Saga::Script::sfScriptGotoScene) {
		stopParsing = true; // cause abortAllThreads called and _this_ thread destroyed
		breakOut = true;
		return;
	}

#ifdef ENABLE_IHNM
	if (scriptFunction == &Saga::Script::sfVsetTrack) {
		stopParsing = true;
		breakOut = true;
		return;		// cause abortAllThreads called and _this_ thread destroyed
	}
#endif

	thread->_stackTopIndex = checkStackTopIndex;

	thread->push(thread->_returnValue);		// return value

	if (thread->_flags & kTFlagAsleep)
		breakOut = true;	// break out of loop!
}

void Script::opCallMember(SCRIPTOP_PARAMS) {
	// SAGA 2 opcode
	// TODO
	warning("opCallMember");
}

void Script::opCallMemberV(SCRIPTOP_PARAMS) {
	// SAGA 2 opcode
	// TODO
	warning("opCallMemberV");
}

void Script::opCcallV(SCRIPTOP_PARAMS) {
	byte argumentsCount = scriptS->readByte();
	uint16 functionNumber = scriptS->readUint16LE();
	if (functionNumber >= ((_vm->getGameId() == GID_IHNM) ?
						   IHNM_SCRIPT_FUNCTION_MAX : ITE_SCRIPT_FUNCTION_MAX)) {
		error("Script::opCcallV() Invalid script function number (%d)", functionNumber);
	}

	debug(2, "Calling #%d %s argCount=%i", functionNumber, _scriptFunctionsList[functionNumber].scriptFunctionName, argumentsCount);
	ScriptFunctionType scriptFunction = _scriptFunctionsList[functionNumber].scriptFunction;
	uint16 checkStackTopIndex = thread->_stackTopIndex + argumentsCount;
	(this->*scriptFunction)(thread, argumentsCount, stopParsing);
	if (stopParsing)
		return;

	if (scriptFunction == &Saga::Script::sfScriptGotoScene) {
		stopParsing = true;
		breakOut = true;
		return;		// cause abortAllThreads called and _this_ thread destroyed
	}

#ifdef ENABLE_IHNM
	if (scriptFunction == &Saga::Script::sfVsetTrack) {
		stopParsing = true;
		breakOut = true;
		return;		// cause abortAllThreads called and _this_ thread destroyed
	}
#endif

	thread->_stackTopIndex = checkStackTopIndex;

	if (thread->_flags & kTFlagAsleep)
		breakOut = true;	// break out of loop!
}

void Script::opEnter(SCRIPTOP_PARAMS) {
	thread->push(thread->_frameIndex);
	thread->_frameIndex = thread->_stackTopIndex;
	thread->_stackTopIndex -= (scriptS->readSint16LE() / 2);
}

void Script::opReturn(SCRIPTOP_PARAMS) {
	thread->_returnValue = thread->pop();		// return value

	thread->_stackTopIndex = thread->_frameIndex;
	thread->_frameIndex = thread->pop();
	if (thread->pushedSize() == 0) {
		thread->_flags |= kTFlagFinished;
		stopParsing = true;
		breakOut = true;
		return;
	} else {
		thread->pop(); //cause it 0
		thread->_instructionOffset = thread->pop();

		// Pop all the call parameters off the stack
		int16 iparam1 = thread->pop();
		while (iparam1--) {
			thread->pop();
		}

		thread->push(thread->_returnValue);
	}
}

void Script::opReturnV(SCRIPTOP_PARAMS) {
	thread->_stackTopIndex = thread->_frameIndex;
	thread->_frameIndex = thread->pop();
	if (thread->pushedSize() == 0) {
		thread->_flags |= kTFlagFinished;
		stopParsing = true;
		breakOut = true;
		return;
	} else {
		thread->pop(); //cause it 0
		thread->_instructionOffset = thread->pop();

		// Pop all the call parameters off the stack
		int16 iparam1 = thread->pop();
		while (iparam1--) {
			thread->pop();
		}
	}
}

void Script::opJmp(SCRIPTOP_PARAMS) {
	thread->_instructionOffset = scriptS->readUint16LE();
}

void Script::opJmpTrueV(SCRIPTOP_PARAMS) {
	uint16 jmpOffset1 = scriptS->readUint16LE();
	if (thread->pop())
		thread->_instructionOffset = jmpOffset1;
}

void Script::opJmpFalseV(SCRIPTOP_PARAMS) {
	uint16 jmpOffset1 = scriptS->readUint16LE();
	if (!thread->pop())
		thread->_instructionOffset = jmpOffset1;
}

void Script::opJmpTrue(SCRIPTOP_PARAMS) {
	uint16 jmpOffset1 = scriptS->readUint16LE();
	if (thread->stackTop())
		thread->_instructionOffset = jmpOffset1;
}

void Script::opJmpFalse(SCRIPTOP_PARAMS) {
	uint16 jmpOffset1 = scriptS->readUint16LE();
	if (!thread->stackTop())
		thread->_instructionOffset = jmpOffset1;
}

void Script::opJmpSwitch(SCRIPTOP_PARAMS) {
	int16 iparam1 = scriptS->readSint16LE();
	int16 iparam2 = thread->pop();
	int16 iparam3;

	while (iparam1--) {
		iparam3 = scriptS->readUint16LE();
		thread->_instructionOffset = scriptS->readUint16LE();
		if (iparam3 == iparam2)
			break;
	}

	if (iparam1 < 0)
		thread->_instructionOffset = scriptS->readUint16LE();
}

void Script::opJmpRandom(SCRIPTOP_PARAMS) {
	// Supposedly the number of possible branches.
	// The original interpreter ignores it.
	scriptS->readUint16LE();
	int16 iparam1 = scriptS->readSint16LE();
	iparam1 = _vm->_rnd.getRandomNumber(iparam1 - 1);
	int16 iparam2;

	while (1) {
		iparam2 = scriptS->readSint16LE();
		thread->_instructionOffset = scriptS->readUint16LE();

		iparam1 -= iparam2;
		if (iparam1 < 0)
			break;
	}
}

void Script::opNegate(SCRIPTOP_PARAMS) {
	thread->push(-thread->pop());
}

void Script::opNot(SCRIPTOP_PARAMS) {
	thread->push(!thread->pop());
}

void Script::opCompl(SCRIPTOP_PARAMS) {
	thread->push(~thread->pop());
}

void Script::opIncV(SCRIPTOP_PARAMS) {
	byte mode = scriptS->readByte();
	byte *addr = thread->baseAddress(mode);
	int16 iparam1 = scriptS->readSint16LE();
	addr += iparam1;
	iparam1 = readUint16(addr, mode);
	writeUint16(addr, iparam1 + 1, mode);
}

void Script::opDecV(SCRIPTOP_PARAMS) {
	byte mode = scriptS->readByte();
	byte *addr = thread->baseAddress(mode);
	int16 iparam1 = scriptS->readSint16LE();
	addr += iparam1;
	iparam1 = readUint16(addr, mode);
	writeUint16(addr, iparam1 - 1, mode);
}

void Script::opPostInc(SCRIPTOP_PARAMS) {
	byte mode = scriptS->readByte();
	byte *addr = thread->baseAddress(mode);
	int16 iparam1 = scriptS->readSint16LE();
	addr += iparam1;
	iparam1 = readUint16(addr, mode);
	thread->push(iparam1);
	writeUint16(addr, iparam1 + 1, mode);
}

void Script::opPostDec(SCRIPTOP_PARAMS) {
	byte mode = scriptS->readByte();
	byte *addr = thread->baseAddress(mode);
	int16 iparam1 = scriptS->readSint16LE();
	addr += iparam1;
	iparam1 = readUint16(addr, mode);
	thread->push(iparam1);
	writeUint16(addr, iparam1 - 1, mode);
}

void Script::opAdd(SCRIPTOP_PARAMS) {
	int16 iparam2 = thread->pop();
	int16 iparam1 = thread->pop();
	thread->push(iparam1 + iparam2);
}

void Script::opSub(SCRIPTOP_PARAMS) {
	int16 iparam2 = thread->pop();
	int16 iparam1 = thread->pop();
	thread->push(iparam1 - iparam2);
}

void Script::opMul(SCRIPTOP_PARAMS) {
	int16 iparam2 = thread->pop();
	int16 iparam1 = thread->pop();
	thread->push(iparam1 * iparam2);
}

void Script::opDiv(SCRIPTOP_PARAMS) {
	int16 iparam2 = thread->pop();
	int16 iparam1 = thread->pop();
	thread->push(iparam1 / iparam2);
}

void Script::opMod(SCRIPTOP_PARAMS) {
	int16 iparam2 = thread->pop();
	int16 iparam1 = thread->pop();
	thread->push(iparam1 % iparam2);
}

void Script::opEq(SCRIPTOP_PARAMS) {
	int16 iparam2 = thread->pop();
	int16 iparam1 = thread->pop();
	thread->push((iparam1 == iparam2) ? 1 : 0);
}

void Script::opNe(SCRIPTOP_PARAMS) {
	int16 iparam2 = thread->pop();
	int16 iparam1 = thread->pop();
	thread->push((iparam1 != iparam2) ? 1 : 0);
}

void Script::opGt(SCRIPTOP_PARAMS) {
	int16 iparam2 = thread->pop();
	int16 iparam1 = thread->pop();
	thread->push((iparam1 > iparam2) ? 1 : 0);
}

void Script::opLt(SCRIPTOP_PARAMS) {
	int16 iparam2 = thread->pop();
	int16 iparam1 = thread->pop();
	thread->push((iparam1 < iparam2) ? 1 : 0);
}

void Script::opGe(SCRIPTOP_PARAMS) {
	int16 iparam2 = thread->pop();
	int16 iparam1 = thread->pop();
	thread->push((iparam1 >= iparam2) ? 1 : 0);
}

void Script::opLe(SCRIPTOP_PARAMS) {
	int16 iparam2 = thread->pop();
	int16 iparam1 = thread->pop();
	thread->push((iparam1 <= iparam2) ? 1 : 0);
}

void Script::opRsh(SCRIPTOP_PARAMS) {
	int16 iparam2 = thread->pop();
	int16 iparam1 = thread->pop();
	thread->push(iparam1 >> iparam2);
}

void Script::opLsh(SCRIPTOP_PARAMS) {
	int16 iparam2 = thread->pop();
	int16 iparam1 = thread->pop();
	thread->push(iparam1 << iparam2);
}

void Script::opAnd(SCRIPTOP_PARAMS) {
	int16 iparam2 = thread->pop();
	int16 iparam1 = thread->pop();
	thread->push(iparam1 & iparam2);
}

void Script::opOr(SCRIPTOP_PARAMS) {
	int16 iparam2 = thread->pop();
	int16 iparam1 = thread->pop();
	thread->push(iparam1 | iparam2);
}

void Script::opXor(SCRIPTOP_PARAMS) {
	int16 iparam2 = thread->pop();
	int16 iparam1 = thread->pop();
	thread->push(iparam1 ^ iparam2);
}

void Script::opLAnd(SCRIPTOP_PARAMS) {
	int16 iparam2 = thread->pop();
	int16 iparam1 = thread->pop();
	thread->push((iparam1 && iparam2) ? 1 : 0);
}

void Script::opLOr(SCRIPTOP_PARAMS) {
	int16 iparam2 = thread->pop();
	int16 iparam1 = thread->pop();
	thread->push((iparam1 || iparam2) ? 1 : 0);
}

void Script::opLXor(SCRIPTOP_PARAMS) {
	int16 iparam2 = thread->pop();
	int16 iparam1 = thread->pop();
	thread->push(((iparam1 && !iparam2) || (!iparam1 && iparam2)) ? 1 : 0);
}

void Script::opSpeak(SCRIPTOP_PARAMS) {
	if (_vm->_actor->isSpeaking()) {
		thread->wait(kWaitTypeSpeech);
		stopParsing = true;
		breakOut = false;
		return;
	}

#ifdef ENABLE_IHNM
	// WORKAROUND for script bug #3358007 in IHNM. When the zeppelin is landing
	// and the player attempts to exit from the right door in room 13, the game
	// scripts change to scene 5, but do not clear the cutaway that appears
	// before Gorrister's speech starts, resulting in a deadlock. We do this
	// manually here.
	if (_vm->getGameId() == GID_IHNM && _vm->_scene->currentChapterNumber() == 1 &&
		_vm->_scene->currentSceneNumber() == 5 && _vm->_anim->hasCutaway()) {
		_vm->_anim->returnFromCutaway();
	}
#endif

	int stringsCount = scriptS->readByte();
	uint16 actorId = scriptS->readUint16LE();
	uint16 speechFlags = scriptS->readByte();
	int sampleResourceId = -1;
	int16 first;
	const char *strings[ACTOR_SPEECH_STRING_MAX];

	scriptS->readUint16LE(); // x,y skip

	if (stringsCount == 0)
		error("opSpeak stringsCount == 0");

	if (stringsCount > ACTOR_SPEECH_STRING_MAX)
		error("opSpeak stringsCount=0x%X exceed ACTOR_SPEECH_STRING_MAX", stringsCount);

	int16 iparam1 = first = thread->stackTop();
	for (int i = 0; i < stringsCount; i++) {
		iparam1 = thread->pop();
		strings[i] = thread->_strings->getString(iparam1);
	}

	// now data contains last string index

#if 0
	if (_vm->getFeatures() & GF_OLD_ITE_DOS) { // special ITE dos
		if ((_vm->_scene->currentSceneNumber() == ITE_DEFAULT_SCENE) &&
			(iparam1 >= 288) && (iparam1 <= (RID_SCENE1_VOICE_END - RID_SCENE1_VOICE_START + 288))) {
			sampleResourceId = RID_SCENE1_VOICE_START + iparam1 - 288;
		}
	} else {
#endif
		if (thread->_voiceLUT->size() > uint16(first))
			sampleResourceId = (*thread->_voiceLUT)[uint16(first)];
#if 0
	}
#endif

	if (sampleResourceId < 0 || sampleResourceId > 4000)
		sampleResourceId = -1;

	if (_vm->getGameId() == GID_ITE && !sampleResourceId)
		sampleResourceId = -1;

	_vm->_actor->actorSpeech(actorId, strings, stringsCount, sampleResourceId, speechFlags);

	if (!(speechFlags & kSpeakAsync)) {
		thread->wait(kWaitTypeSpeech);
	}
}

void Script::opDialogBegin(SCRIPTOP_PARAMS) {
	if (_conversingThread) {
		thread->wait(kWaitTypeDialogBegin);
		stopParsing = true;
		breakOut = false;
		return;
	}
	_conversingThread = thread;
	_vm->_interface->converseClear();
}

void Script::opDialogEnd(SCRIPTOP_PARAMS) {
	if (thread == _conversingThread) {
		_vm->_interface->activate();
		_vm->_interface->setMode(kPanelConverse);
		thread->wait(kWaitTypeDialogEnd);
		stopParsing = true;
		breakOut = false;
		return;
	}
}

void Script::opReply(SCRIPTOP_PARAMS) {
	const char *str;
	byte replyNum = scriptS->readByte();
	byte flags = scriptS->readByte();
	int16 iparam1 = 0;
	int strID = thread->pop();

	if (flags & kReplyOnce) {
		iparam1 = scriptS->readSint16LE();
		byte *addr = thread->_staticBase + (iparam1 >> 3);
		if (*addr & (1 << (iparam1 & 7))) {
			return;
		}
	}

	str = thread->_strings->getString(strID);
	if (_vm->_interface->converseAddText(str, strID, replyNum, flags, iparam1))
		warning("Error adding ConverseText (%s, %d, %d, %d)", str, replyNum, flags, iparam1);
}

void Script::opAnimate(SCRIPTOP_PARAMS) {
	scriptS->readUint16LE();
	scriptS->readUint16LE();
	thread->_instructionOffset += scriptS->readByte();
}

void Script::opJmpSeedRandom(SCRIPTOP_PARAMS) {
	// SAGA 2 opcode
	// TODO
	warning("opJmpSeedRandom");
}

void Script::loadModule(uint scriptModuleNumber) {
	ByteArray resourceData;

	// Validate script number
	if (scriptModuleNumber >= _modules.size()) {
		error("Script::loadScript() Invalid script module number");
	}

	if (_modules[scriptModuleNumber].loaded) {
		return;
	}

	// Initialize script data structure
	debug(3, "Loading script module #%d", scriptModuleNumber);

	_vm->_resource->loadResource(_scriptContext, _modules[scriptModuleNumber].scriptResourceId, resourceData);

	loadModuleBase(_modules[scriptModuleNumber], resourceData);

	_vm->_resource->loadResource(_scriptContext, _modules[scriptModuleNumber].stringsResourceId, resourceData);

	_vm->loadStrings(_modules[scriptModuleNumber].strings, resourceData);

	if (_modules[scriptModuleNumber].voicesResourceId > 0) {
		_vm->_resource->loadResource(_scriptContext, _modules[scriptModuleNumber].voicesResourceId, resourceData);

		loadVoiceLUT(_modules[scriptModuleNumber].voiceLUT, resourceData);
	}

	_modules[scriptModuleNumber].staticOffset = _staticSize;
	_staticSize += _modules[scriptModuleNumber].staticSize;
	if (_staticSize > _commonBuffer.size()) {
		error("Script::loadModule() _staticSize > _commonBuffer.size()");
	}
	_modules[scriptModuleNumber].loaded = true;
}

void Script::clearModules() {
	uint i;
	for (i = 0; i < _modules.size(); i++) {
		if (_modules[i].loaded) {
			_modules[i].clear();
		}
	}
	_staticSize = 0;
}

void Script::loadModuleBase(ModuleData &module, const ByteArray &resourceData) {
	uint i;

	debug(3, "Loading module base...");

	module.moduleBase.assign(resourceData);

	ByteArrayReadStreamEndian scriptS(module.moduleBase, _scriptContext->isBigEndian());

	uint entryPointsCount = scriptS.readUint16();
	scriptS.readUint16(); //skip
	uint16 entryPointsTableOffset;	// offset of entrypoint table in moduleBase
	entryPointsTableOffset = scriptS.readUint16();
	scriptS.readUint16(); //skip

	if ((module.moduleBase.size() - entryPointsTableOffset) < (entryPointsCount * SCRIPT_TBLENTRY_LEN)) {
		error("Script::loadModuleBase() Invalid table offset");
	}

	if (entryPointsCount > SCRIPT_MAX) {
		error("Script::loadModuleBase()Script limit exceeded");
	}

	module.entryPoints.resize(entryPointsCount);

	// Read in the entrypoint table

	module.staticSize = scriptS.readUint16();
	while (scriptS.pos() < entryPointsTableOffset)
		scriptS.readByte();

	for (i = 0; i < module.entryPoints.size(); i++) {
		// First uint16 is the offset of the entrypoint name from the start
		// of the bytecode resource, second uint16 is the offset of the
		// bytecode itself for said entrypoint
		module.entryPoints[i].nameOffset = scriptS.readUint16();
		module.entryPoints[i].offset = scriptS.readUint16();

		// Perform a simple range check on offset values
		if ((module.entryPoints[i].nameOffset >= module.moduleBase.size()) || (module.entryPoints[i].offset >= module.moduleBase.size())) {
			error("Script::loadModuleBase() Invalid offset encountered in script entrypoint table");
		}
	}
}

void Script::loadVoiceLUT(VoiceLUT &voiceLUT, const ByteArray &resourceData) {
	uint16 i;

	voiceLUT.resize(resourceData.size() / 2);

	ByteArrayReadStreamEndian scriptS(resourceData, _scriptContext->isBigEndian());

	for (i = 0; i < voiceLUT.size(); i++) {
		voiceLUT[i] = scriptS.readUint16();
	}
}

// verb
void Script::showVerb(int statusColor) {
	const char *verbName;
	const char *object1Name;
	const char *object2Name;
	Common::String statusString;

	if (_leftButtonVerb == getVerbType(kVerbNone)) {
		_vm->_interface->setStatusText("");
		return;
	}

	if (_vm->getGameId() == GID_ITE)
		verbName = _mainStrings.getString(_leftButtonVerb - 1);
	else
		verbName = _mainStrings.getString(_leftButtonVerb + 1);

	if (objectTypeId(_currentObject[0]) == kGameObjectNone) {
		_vm->_interface->setStatusText(verbName, statusColor);
		return;
	}

	object1Name = _vm->getObjectName(_currentObject[0]);

	if (!_secondObjectNeeded) {
		statusString = Common::String::format("%s %s", verbName, object1Name);
		_vm->_interface->setStatusText(statusString.c_str(), statusColor);
		return;
	}


	if (objectTypeId(_currentObject[1]) != kGameObjectNone) {
		object2Name = _vm->getObjectName(_currentObject[1]);
	} else {
		object2Name = "";
	}

	if (_leftButtonVerb == getVerbType(kVerbGive)) {
		statusString = Common::String::format(_vm->getTextString(kTextGiveTo), object1Name, object2Name);
		_vm->_interface->setStatusText(statusString.c_str(), statusColor);
	} else {
		if (_leftButtonVerb == getVerbType(kVerbUse)) {
			statusString = Common::String::format(_vm->getTextString(kTextUseWidth), object1Name, object2Name);
			_vm->_interface->setStatusText(statusString.c_str(), statusColor);
		} else {
			statusString = Common::String::format("%s %s", verbName, object1Name);
			_vm->_interface->setStatusText(statusString.c_str(), statusColor);
		}
	}
}

int Script::getVerbType(VerbTypes verbType) {
	if (_vm->getGameId() == GID_ITE) {
		switch (verbType) {
		case kVerbNone:
			return kVerbITENone;
		case kVerbWalkTo:
			return kVerbITEWalkTo;
		case kVerbGive:
			return kVerbITEGive;
		case kVerbUse:
			return kVerbITEUse;
		case kVerbEnter:
			return kVerbITEEnter;
		case kVerbLookAt:
			return kVerbITELookAt;
		case kVerbPickUp:
			return kVerbITEPickUp;
		case kVerbOpen:
			return kVerbITEOpen;
		case kVerbClose:
			return kVerbITEClose;
		case kVerbTalkTo:
			return kVerbITETalkTo;
		case kVerbWalkOnly:
			return kVerbITEWalkOnly;
		case kVerbLookOnly:
			return kVerbITELookOnly;
		case kVerbOptions:
			return kVerbITEOptions;
		}
#ifdef ENABLE_IHNM
	} else if (_vm->getGameId() == GID_IHNM) {
		switch (verbType) {
		case kVerbNone:
			return kVerbIHNMNone;
		case kVerbWalkTo:
			return kVerbIHNMWalk;
		case kVerbLookAt:
			return kVerbIHNMLookAt;
		case kVerbPickUp:
			return kVerbIHNMTake;
		case kVerbUse:
			return kVerbIHNMUse;
		case kVerbTalkTo:
			return kVerbIHNMTalkTo;
		case kVerbOpen:
			return kVerbIHNMSwallow;
		case kVerbGive:
			return kVerbIHNMGive;
		case kVerbClose:
			return kVerbIHNMPush;
		case kVerbEnter:
			return kVerbIHNMEnter;
		case kVerbWalkOnly:
			return kVerbIHNMWalkOnly;
		case kVerbLookOnly:
			return kVerbIHNMLookOnly;
		case kVerbOptions:
			return kVerbIHNMOptions;
		}
#endif
	}
	error("Script::getVerbType() unknown verb type %d", verbType);
}

void Script::setVerb(int verb) {
	_pendingObject[0] = ID_NOTHING;
	_currentObject[0] = ID_NOTHING;
	_pendingObject[1] = ID_NOTHING;
	_currentObject[1] = ID_NOTHING;
	_firstObjectSet = false;
	_secondObjectNeeded = false;

	// The pointer object will be updated again immediately. This way the
	// new verb will be applied to it. It's not exactly how the original
	// engine did it, but it appears to work.
	_pointerObject = ID_NOTHING;

	setLeftButtonVerb(verb);
	showVerb();
}

bool Script::isNonInteractiveDemo() {
	// This detection only works in ITE. The early non-interactive demos had
	// a very small script file
	return _vm->getGameId() == GID_ITE && _scriptContext->fileSize() < 50000;
}

void Script::setLeftButtonVerb(int verb) {
	int		oldVerb = _currentVerb;

	_currentVerb = _leftButtonVerb = verb;

	if ((_currentVerb != oldVerb) && (_vm->_interface->getMode() == kPanelMain)){
			if (oldVerb > getVerbType(kVerbNone))
				_vm->_interface->setVerbState(oldVerb, 2);

			if (_currentVerb > getVerbType(kVerbNone))
				_vm->_interface->setVerbState(_currentVerb, 2);
	}
}

void Script::setRightButtonVerb(int verb) {
	int		oldVerb = _rightButtonVerb;

	_rightButtonVerb = verb;

	if ((_rightButtonVerb != oldVerb) && (_vm->_interface->getMode() == kPanelMain)){
		if (oldVerb > getVerbType(kVerbNone))
			_vm->_interface->setVerbState(oldVerb, 2);

		if (_rightButtonVerb > getVerbType(kVerbNone))
			_vm->_interface->setVerbState(_rightButtonVerb, 2);
	}
}

void Script::doVerb() {
	int scriptEntrypointNumber = 0;
	int scriptModuleNumber = 0;
	int objectType;
	Event event;
	const char *excuseText;
	int excuseSampleResourceId;
	const HitZone *hitZone;

	objectType = objectTypeId(_pendingObject[0]);

	if (_pendingVerb == getVerbType(kVerbGive)) {
		scriptEntrypointNumber = _vm->_actor->getObjectScriptEntrypointNumber(_pendingObject[1]);
		if (_vm->_actor->getObjectFlags(_pendingObject[1]) & (kFollower|kProtagonist|kExtended)) {
			scriptModuleNumber = 0;
		} else {
			scriptModuleNumber = _vm->_scene->getScriptModuleNumber();
		}
		// IHNM never sets scriptModuleNumber to 0
		if (_vm->getGameId() == GID_IHNM)
			scriptModuleNumber = _vm->_scene->getScriptModuleNumber();
	} else {
		if (_pendingVerb == getVerbType(kVerbUse)) {
			if ((objectTypeId(_pendingObject[1]) > kGameObjectNone) && (objectType < objectTypeId(_pendingObject[1]))) {
				SWAP(_pendingObject[0], _pendingObject[1]);
				objectType = objectTypeId(_pendingObject[0]);
			}
		}

		if (objectType == 0)
			return;
		else if (objectType == kGameObjectHitZone) {
			scriptModuleNumber = _vm->_scene->getScriptModuleNumber();
			hitZone = _vm->_scene->_objectMap->getHitZone(objectIdToIndex(_pendingObject[0]));

			if (hitZone == NULL)
				return;

			if ((hitZone->getFlags() & kHitZoneExit) == 0) {
				scriptEntrypointNumber = hitZone->getScriptNumber();
			}
		} else {
			if (objectType & (kGameObjectActor | kGameObjectObject)) {
				scriptEntrypointNumber = _vm->_actor->getObjectScriptEntrypointNumber(_pendingObject[0]);

				if ((objectType == kGameObjectActor) && !(_vm->_actor->getObjectFlags(_pendingObject[0]) & (kFollower|kProtagonist|kExtended))) {
					scriptModuleNumber = _vm->_scene->getScriptModuleNumber();
				} else {
					scriptModuleNumber = 0;
				}
				// IHNM never sets scriptModuleNumber to 0
				if (_vm->getGameId() == GID_IHNM)
					scriptModuleNumber = _vm->_scene->getScriptModuleNumber();
			}
		}
	}

	// WORKAROUND for a bug in the original game scripts of IHNM. Edna's script (actor 8197) is problematic, so
	// when the knife (object 16385) is used with her, the expected result is not correct. The first time that
	// the knife is used, Edna's heart is cut out (which is correct). But on subsequent use, the object's script
	// is buggy, therefore it's possible to talk to a dead Edna by using the knife on her, or to incorrectly get her
	// heart again, which remove's Gorrister's heart from the inventory. The solution is to disable the "use knife with
	// Edna" action altogether, because if the player wants to kill Edna, he can do that by talking to her and
	// choosing "[Cut out Edna's heart]", which works correctly. To disable this action, if the knife is used on Edna, we
	// change the action here to "use knife with the knife", which yields a better reply ("I'd just dull my knife").
	// Fixes bug #1826871 - "IHNM: Edna's got two hearts but loves to be on the hook"
	if (_vm->getGameId() == GID_IHNM && _pendingObject[0] == 16385 && _pendingObject[1] == 8197 && _pendingVerb == 4)
		_pendingObject[1] = 16385;

	// WORKAROUND for a bug in the original game scripts of IHNM. Gorrister's heart is not supposed to have a
	// "use" phrase attached to it (it's not used anywhere, it's only given), but when "used", an incorrect
	// reply is given to the player ("It's too narrow for me to pass", said when Gorrister tries to pick up the
	// heart without a rope). Therefore, for object number 16397 (Gorrister's heart), when the active verb is
	// "Use", set it to "Push", which gives a more appropriate reply ("What good will that do me?")
	if (_vm->getGameId() == GID_IHNM && _pendingObject[0] == 16397 && _pendingVerb == 4)
		_pendingVerb = 8;

	if (scriptEntrypointNumber > 0) {

		event.type = kEvTOneshot;
		event.code = kScriptEvent;
		event.op = kEventExecNonBlocking;
		event.time = 0;
		event.param = scriptModuleNumber;
		event.param2 = scriptEntrypointNumber;
		event.param3 = _pendingVerb;		// Action
		event.param4 = _pendingObject[0];	// Object
		event.param5 = _pendingObject[1];	// With Object
		event.param6 = (objectType == kGameObjectActor) ? _pendingObject[0] : ID_PROTAG;		// Actor
		_vm->_events->queue(event);

	} else {
		// Show excuse text in ITE CD Versions
		if (_vm->getGameId() == GID_ITE) {
			_vm->getExcuseInfo(_pendingVerb, excuseText, excuseSampleResourceId);
			if (excuseText) {
				// In Floppy versions we don't have excuse texts
				if (_vm->getFeatures() & GF_ITE_FLOPPY)
					excuseSampleResourceId = -1;

				_vm->_actor->actorSpeech(ID_PROTAG, &excuseText, 1, excuseSampleResourceId, 0);
			}
		}
	}

	if ((_currentVerb == getVerbType(kVerbWalkTo)) || (_currentVerb == getVerbType(kVerbLookAt))) {
		_stickyVerb = _currentVerb;
	}

	_pendingVerb = getVerbType(kVerbNone);
	_currentObject[0] = _currentObject[1] = ID_NOTHING;
	setLeftButtonVerb(_stickyVerb);

	setPointerVerb();
}

void Script::setPointerVerb() {
	if (_vm->_interface->isActive()) {
		_pointerObject = ID_PROTAG;
		whichObject(_vm->mousePos());
	}
}

void Script::hitObject(bool leftButton) {
	int verb;
	verb = leftButton ? _leftButtonVerb : _rightButtonVerb;

	if (verb > getVerbType(kVerbNone)) {
		if (_firstObjectSet) {
			if (_secondObjectNeeded) {
				_pendingObject[0] = _currentObject[0];
				_pendingObject[1] = _currentObject[1];
				_pendingVerb = verb;

				_leftButtonVerb = verb;
				if (_pendingVerb > getVerbType(kVerbNone))
					showVerb(kITEColorBrightWhite);
				else
					showVerb();

				_secondObjectNeeded = false;
				_firstObjectSet = false;
				return;
			}
		} else {
			if (verb == getVerbType(kVerbGive)) {
				_secondObjectNeeded = true;
			} else {
				if (verb == getVerbType(kVerbUse)) {

					if (_currentObjectFlags[0] & kObjUseWith) {
						_secondObjectNeeded = true;
					}
				}
			}

			if (!_secondObjectNeeded) {
				_pendingObject[0] = _currentObject[0];
				_pendingObject[1] = ID_NOTHING;
				_pendingVerb = verb;

				_secondObjectNeeded = false;
				_firstObjectSet = false;
			} else {
				_firstObjectSet = true;
			}
		}

		_leftButtonVerb = verb;
		if (_pendingVerb > getVerbType(kVerbNone))
			showVerb(kITEColorBrightWhite);
		else
			showVerb();
	}

}

void Script::playfieldClick(const Point& mousePoint, bool leftButton) {
	Location pickLocation;
	const HitZone *hitZone;
	Point specialPoint;

	_vm->incrementMouseClickCount();
	_vm->_actor->abortSpeech();

	if ((_vm->_actor->_protagonist->_currentAction != kActionWait) &&
		(_vm->_actor->_protagonist->_currentAction != kActionFreeze) &&
		(_vm->_actor->_protagonist->_currentAction != kActionWalkToLink) &&
		(_vm->_actor->_protagonist->_currentAction != kActionWalkToPoint)) {
		return;
	}
	if (_pendingVerb > getVerbType(kVerbNone)) {
		setLeftButtonVerb(getVerbType(kVerbWalkTo));
	}

	if (_pointerObject != ID_NOTHING) {
		hitObject(leftButton);
	} else {
		_pendingObject[0] = ID_NOTHING;
		_pendingObject[1] = ID_NOTHING;
		_pendingVerb = getVerbType(kVerbWalkTo);
	}


	// tiled stuff
	if (_vm->_scene->getFlags() & kSceneFlagISO) {
		_vm->_isoMap->screenPointToTileCoords(mousePoint, pickLocation);
	} else {
		pickLocation.fromScreenPoint(mousePoint);
	}


	hitZone = NULL;

	if (objectTypeId(_pendingObject[0]) == kGameObjectHitZone) {
		 hitZone = _vm->_scene->_objectMap->getHitZone(objectIdToIndex(_pendingObject[0]));
	} else {
		if ((_pendingVerb == getVerbType(kVerbUse)) && (objectTypeId(_pendingObject[1]) == kGameObjectHitZone)) {
			hitZone = _vm->_scene->_objectMap->getHitZone(objectIdToIndex(_pendingObject[1]));
		}
	}

	if (hitZone != NULL) {
		if (_vm->getGameId() == GID_ITE) {
			if (hitZone->getFlags() & kHitZoneNoWalk) {
				_vm->_actor->actorFaceTowardsPoint(ID_PROTAG, pickLocation);
				doVerb();
				return;
			}
		} else {
			if (_vm->getGameId() == GID_IHNM) {
				if ((hitZone->getFlags() & kHitZoneNoWalk) && (_pendingVerb != getVerbType(kVerbWalkTo))) {
					doVerb();
					return;
				}
			}
		}

		if (hitZone->getFlags() & kHitZoneProject) {
			if (!hitZone->getSpecialPoint(specialPoint)) {
				// Original behaved this way and this prevents from crash
				// at ruins. See bug #1257459
				specialPoint.x = specialPoint.y = 0;
			}

			// tiled stuff
			if (_vm->_scene->getFlags() & kSceneFlagISO) {
				pickLocation.u() = specialPoint.x;
				pickLocation.v() = specialPoint.y;
				pickLocation.z = _vm->_actor->_protagonist->_location.z;
			} else {
				pickLocation.fromScreenPoint(specialPoint);
			}
		}
	}

	if (_vm->getGameId() == GID_ITE) {
		if ((_pendingVerb == getVerbType(kVerbWalkTo)) ||
			(_pendingVerb == getVerbType(kVerbPickUp)) ||
			(_pendingVerb == getVerbType(kVerbOpen)) ||
			(_pendingVerb == getVerbType(kVerbClose)) ||
			(_pendingVerb == getVerbType(kVerbUse))) {
				_vm->_actor->actorWalkTo(ID_PROTAG, pickLocation);
		} else {
			if (_pendingVerb == getVerbType(kVerbLookAt)) {
				if (objectTypeId(_pendingObject[0]) != kGameObjectActor) {
					_vm->_actor->actorWalkTo(ID_PROTAG, pickLocation);
				} else {
					doVerb();
				}
			} else {
				if ((_pendingVerb == getVerbType(kVerbTalkTo)) ||
					(_pendingVerb == getVerbType(kVerbGive))) {
						doVerb();
				}
			}
		}
	}

#ifdef ENABLE_IHNM
	if (_vm->getGameId() == GID_IHNM) {

		if ((_pendingVerb == getVerbType(kVerbWalkTo)) ||
			(_pendingVerb == getVerbType(kVerbPickUp)) ||
			(_pendingVerb == getVerbType(kVerbOpen)) ||
			(_pendingVerb == getVerbType(kVerbClose)) ||
			(_pendingVerb == getVerbType(kVerbUse))) {
				_vm->_actor->actorWalkTo(ID_PROTAG, pickLocation);

				// Auto-use no-walk hitzones in IHNM, needed for Benny's chapter
				if (_pendingVerb == getVerbType(kVerbWalkTo) &&
					hitZone != NULL && (hitZone->getFlags() & kHitZoneNoWalk)) {
					_pendingVerb = getVerbType(kVerbUse);
					if (objectTypeId(_pendingObject[0]) == kGameObjectActor) {
						_vm->_actor->actorFaceTowardsObject(ID_PROTAG, _pendingObject[0]);
						doVerb();
					}
				}

				// Auto-use hitzone with id 24576 (the exit to the left) in screens 16 - 19
				// (screens with Gorrister's heart) in IHNM. For some reason, this zone does
				// not have a corresponding action zone, so we auto-use it here, like the exits
				// in Benny's chapter
				if (_vm->_scene->currentChapterNumber() == 1 &&
					_vm->_scene->currentSceneNumber() >= 16 &&
					_vm->_scene->currentSceneNumber() <= 19 &&
					_pendingVerb == getVerbType(kVerbWalkTo) &&
					hitZone != NULL && hitZone->getHitZoneId() == 24576) {
					_pendingVerb = getVerbType(kVerbUse);
					if (objectTypeId(_pendingObject[0]) == kGameObjectActor) {
						_vm->_actor->actorFaceTowardsObject(ID_PROTAG, _pendingObject[0]);
						doVerb();
					}
				}

		} else {
			if (_pendingVerb == getVerbType(kVerbLookAt)) {
				if (objectTypeId(_pendingObject[0]) != kGameObjectActor) {
					_vm->_actor->actorWalkTo(ID_PROTAG, pickLocation);
				} else {
					_vm->_actor->actorFaceTowardsObject(ID_PROTAG, _pendingObject[0]);
					doVerb();
				}
			} else {
				if ((_pendingVerb == getVerbType(kVerbTalkTo)) ||
					(_pendingVerb == getVerbType(kVerbGive))) {
						doVerb();
				}
			}
		}
	}
#endif

}

void Script::whichObject(const Point& mousePoint) {
	uint16 objectId;
	int16 objectFlags;
	int newRightButtonVerb;
	uint16 newObjectId;
	ActorData *actor;
	ObjectData *obj;
	Point pickPoint;
	Location pickLocation;
	int hitZoneIndex;
	const HitZone * hitZone;
	PanelButton * panelButton;

	objectId = ID_NOTHING;
	objectFlags = 0;
	_leftButtonVerb = _currentVerb;
	newRightButtonVerb = getVerbType(kVerbNone);

	// _protagonist can be null while loading a game from the command line
	if (_vm->_actor->_protagonist == NULL)
		return;

	if (_vm->_actor->_protagonist->_currentAction != kActionWalkDir) {
		if (_vm->_scene->getHeight() >= mousePoint.y) {
			newObjectId = _vm->_actor->hitTest(mousePoint, true);

			if (newObjectId != ID_NOTHING) {
				if (objectTypeId(newObjectId) == kGameObjectObject) {
					objectId = newObjectId;
					objectFlags = 0;
					newRightButtonVerb = getVerbType(kVerbLookAt);

					if ((_currentVerb == getVerbType(kVerbTalkTo)) || ((_currentVerb == getVerbType(kVerbGive)) && _firstObjectSet)) {
						objectId = ID_NOTHING;
						newObjectId = ID_NOTHING;
					}
				} else {
					actor = _vm->_actor->getActor(newObjectId);
					objectId = newObjectId;
					if (_vm->getGameId() == GID_ITE)
						objectFlags = kObjUseWith;
					// Note: for IHNM, the default right button action is "Look at" for actors,
					// but "Talk to" makes much more sense
					newRightButtonVerb = getVerbType(kVerbTalkTo);
					// Slight hack because of the above change: the jukebox in Gorrister's chapter
					// is an actor, so change the right button action to "Look at"
					if (_vm->getGameId() == GID_IHNM && objectId == 8199)
						newRightButtonVerb = getVerbType(kVerbLookAt);

					if ((_currentVerb == getVerbType(kVerbPickUp)) ||
						(_currentVerb == getVerbType(kVerbOpen)) ||
						(_currentVerb == getVerbType(kVerbClose)) ||
						((_currentVerb == getVerbType(kVerbGive)) && !_firstObjectSet) ||
						((_currentVerb == getVerbType(kVerbUse)) && !(actor->_flags & kFollower))) {
							if (_vm->getGameId() == GID_ITE) {
								objectId = ID_NOTHING;
								newObjectId = ID_NOTHING;
							}
						}
				}
			}

			if (newObjectId == ID_NOTHING) {

				pickPoint = mousePoint;

				if (_vm->_scene->getFlags() & kSceneFlagISO) {
					pickPoint.y -= _vm->_actor->_protagonist->_location.z;
					_vm->_isoMap->screenPointToTileCoords(pickPoint, pickLocation);
					pickLocation.toScreenPointUV(pickPoint);
				}

				hitZoneIndex = _vm->_scene->_objectMap->hitTest(pickPoint);

				// WORKAROUND for an incorrect hitzone which exists in IHNM
				// In Gorrister's chapter, in the toilet screen, the hitzone of the exit is
				// placed over the place where Gorrister sits to examine the graffiti on the wall
				// to the left, which makes him exit the screen when the graffiti is examined.
				// We effectively change the left side of the hitzone here so that it starts from
				// pixel 301 onwards. The same workaround is applied in Actor::handleActions
				if (_vm->getGameId() == GID_IHNM) {
					if (_vm->_scene->currentChapterNumber() == 1 && _vm->_scene->currentSceneNumber() == 22)
						if (hitZoneIndex == 8 && pickPoint.x <= 300)
							hitZoneIndex = -1;
				}

				if ((hitZoneIndex != -1)) {
					hitZone = _vm->_scene->_objectMap->getHitZone(hitZoneIndex);
					objectId = hitZone->getHitZoneId();
					objectFlags = 0;
					newRightButtonVerb = hitZone->getRightButtonVerb() & 0x7f;

					// WORKAROUND for a problematic object in IHNM
					// In the freezer room, the key that drops is made of a hitzone which
					// contains the key object itself. We change the object ID that the
					// hitzone contains (object ID 24578 - "The key") to the ID of the key
					// object itself (object ID 16402 - "Edna's key"), as the user can keep
					// hovering the cursor to both items, but can only pick up one
					if (_vm->getGameId() == GID_IHNM) {
						if (_vm->_scene->currentChapterNumber() == 1 && _vm->_scene->currentSceneNumber() == 24) {
							if (objectId == 24578)
								objectId = 16402;
						}
					}

					if (_vm->getGameId() == GID_ITE) {

						if (newRightButtonVerb == getVerbType(kVerbWalkOnly)) {
							if (_firstObjectSet) {
								objectId = ID_NOTHING;
							} else {
								newRightButtonVerb = _leftButtonVerb = getVerbType(kVerbWalkTo);
							}
						} else {
							if (newRightButtonVerb == getVerbType(kVerbLookOnly)) {
								if (_firstObjectSet) {
									objectId = ID_NOTHING;
								} else {
									newRightButtonVerb = _leftButtonVerb = getVerbType(kVerbLookAt);
								}
							}
						}

						if (newRightButtonVerb >= getVerbType(kVerbOptions)) {
							newRightButtonVerb = getVerbType(kVerbNone);
						}
					} else {
						if (newRightButtonVerb >= getVerbType(kVerbOptions)) {
							newRightButtonVerb = getVerbType(kVerbWalkTo);
						}
					}

					if ((_currentVerb == getVerbType(kVerbTalkTo)) || ((_currentVerb == getVerbType(kVerbGive)) && _firstObjectSet)) {
						objectId = ID_NOTHING;
						newObjectId = ID_NOTHING;
					}

					if ((_leftButtonVerb == getVerbType(kVerbUse)) && (hitZone->getRightButtonVerb() & 0x80)) {
						objectFlags = kObjUseWith;
					}
				}
			}
		} else {
			if ((_currentVerb == getVerbType(kVerbTalkTo)) || ((_currentVerb == getVerbType(kVerbGive)) && _firstObjectSet)) {
				// no way
			} else {
				panelButton = _vm->_interface->inventoryHitTest(mousePoint);
				if (panelButton) {
					objectId = _vm->_interface->getInventoryContentByPanelButton(panelButton);
					if (objectId != 0) {
						obj = _vm->_actor->getObj(objectId);
						newRightButtonVerb = getVerbType(kVerbLookAt);
						if (obj->_interactBits & kObjUseWith) {
							objectFlags = kObjUseWith;
						}
					}
				}
			}

			if ((_currentVerb == getVerbType(kVerbPickUp)) || (_currentVerb == getVerbType(kVerbTalkTo)) || (_currentVerb == getVerbType(kVerbWalkTo))) {
				_leftButtonVerb = getVerbType(kVerbLookAt);
			}
		}
	}

	if (objectId != _pointerObject) {
		_pointerObject = objectId;
		_currentObject[_firstObjectSet ? 1 : 0] = objectId;
		_currentObjectFlags[_firstObjectSet ? 1 : 0] = objectFlags;
		if (_pendingVerb == getVerbType(kVerbNone)) {
			showVerb();
		}
	}

	if (newRightButtonVerb != _rightButtonVerb) {
		setRightButtonVerb(newRightButtonVerb);
	}
}

} // End of namespace Saga
