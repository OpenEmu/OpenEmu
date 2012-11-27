/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "common/util.h"
#include "common/stack.h"
#include "common/textconsole.h"

#include "sword2/sword2.h"
#include "sword2/header.h"
#include "sword2/defs.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/memory.h"
#include "sword2/resman.h"

namespace Sword2 {

#define STACK_SIZE 10

// The machine code table

#ifndef REDUCE_MEMORY_USAGE
#	define OPCODE(x)	{ &Logic::x, #x }
#else
#	define OPCODE(x)	{ &Logic::x, "" }
#endif

void Logic::setupOpcodes() {
	static const OpcodeEntry opcodes[] = {
		/* 00 */
		OPCODE(fnTestFunction),
		OPCODE(fnTestFlags),
		OPCODE(fnRegisterStartPoint),
		OPCODE(fnInitBackground),
		/* 04 */
		OPCODE(fnSetSession),
		OPCODE(fnBackSprite),
		OPCODE(fnSortSprite),
		OPCODE(fnForeSprite),
		/* 08 */
		OPCODE(fnRegisterMouse),
		OPCODE(fnAnim),
		OPCODE(fnRandom),
		OPCODE(fnPreLoad),
		/* 0C */
		OPCODE(fnAddSubject),
		OPCODE(fnInteract),
		OPCODE(fnChoose),
		OPCODE(fnWalk),
		/* 10 */
		OPCODE(fnWalkToAnim),
		OPCODE(fnTurn),
		OPCODE(fnStandAt),
		OPCODE(fnStand),
		/* 14 */
		OPCODE(fnStandAfterAnim),
		OPCODE(fnPause),
		OPCODE(fnMegaTableAnim),
		OPCODE(fnAddMenuObject),
		/* 18 */
		OPCODE(fnStartConversation),
		OPCODE(fnEndConversation),
		OPCODE(fnSetFrame),
		OPCODE(fnRandomPause),
		/* 1C */
		OPCODE(fnRegisterFrame),
		OPCODE(fnNoSprite),
		OPCODE(fnSendSync),
		OPCODE(fnUpdatePlayerStats),
		/* 20 */
		OPCODE(fnPassGraph),
		OPCODE(fnInitFloorMouse),
		OPCODE(fnPassMega),
		OPCODE(fnFaceXY),
		/* 24 */
		OPCODE(fnEndSession),
		OPCODE(fnNoHuman),
		OPCODE(fnAddHuman),
		OPCODE(fnWeWait),
		/* 28 */
		OPCODE(fnTheyDoWeWait),
		OPCODE(fnTheyDo),
		OPCODE(fnWalkToTalkToMega),
		OPCODE(fnFadeDown),
		/* 2C */
		OPCODE(fnISpeak),
		OPCODE(fnTotalRestart),
		OPCODE(fnSetWalkGrid),
		OPCODE(fnSpeechProcess),
		/* 30 */
		OPCODE(fnSetScaling),
		OPCODE(fnStartEvent),
		OPCODE(fnCheckEventWaiting),
		OPCODE(fnRequestSpeech),
		/* 34 */
		OPCODE(fnGosub),
		OPCODE(fnTimedWait),
		OPCODE(fnPlayFx),
		OPCODE(fnStopFx),
		/* 38 */
		OPCODE(fnPlayMusic),
		OPCODE(fnStopMusic),
		OPCODE(fnSetValue),
		OPCODE(fnNewScript),
		/* 3C */
		OPCODE(fnGetSync),
		OPCODE(fnWaitSync),
		OPCODE(fnRegisterWalkGrid),
		OPCODE(fnReverseMegaTableAnim),
		/* 40 */
		OPCODE(fnReverseAnim),
		OPCODE(fnAddToKillList),
		OPCODE(fnSetStandbyCoords),
		OPCODE(fnBackPar0Sprite),
		/* 44 */
		OPCODE(fnBackPar1Sprite),
		OPCODE(fnForePar0Sprite),
		OPCODE(fnForePar1Sprite),
		OPCODE(fnSetPlayerActionEvent),
		/* 48 */
		OPCODE(fnSetScrollCoordinate),
		OPCODE(fnStandAtAnim),
		OPCODE(fnSetScrollLeftMouse),
		OPCODE(fnSetScrollRightMouse),
		/* 4C */
		OPCODE(fnColor),
		OPCODE(fnFlash),
		OPCODE(fnPreFetch),
		OPCODE(fnGetPlayerSaveData),
		/* 50 */
		OPCODE(fnPassPlayerSaveData),
		OPCODE(fnSendEvent),
		OPCODE(fnAddWalkGrid),
		OPCODE(fnRemoveWalkGrid),
		/* 54 */
		OPCODE(fnCheckForEvent),
		OPCODE(fnPauseForEvent),
		OPCODE(fnClearEvent),
		OPCODE(fnFaceMega),
		/* 58 */
		OPCODE(fnPlaySequence),
		OPCODE(fnShadedSprite),
		OPCODE(fnUnshadedSprite),
		OPCODE(fnFadeUp),
		/* 5C */
		OPCODE(fnDisplayMsg),
		OPCODE(fnSetObjectHeld),
		OPCODE(fnAddSequenceText),
		OPCODE(fnResetGlobals),
		/* 60 */
		OPCODE(fnSetPalette),
		OPCODE(fnRegisterPointerText),
		OPCODE(fnFetchWait),
		OPCODE(fnRelease),
		/* 64 */
		OPCODE(fnPrepareMusic),
		OPCODE(fnSoundFetch),
		OPCODE(fnPrepareMusic),	// Again, apparently
		OPCODE(fnSmackerLeadIn),
		/* 68 */
		OPCODE(fnSmackerLeadOut),
		OPCODE(fnStopAllFx),
		OPCODE(fnCheckPlayerActivity),
		OPCODE(fnResetPlayerActivityDelay),
		/* 6C */
		OPCODE(fnCheckMusicPlaying),
		OPCODE(fnPlayCredits),
		OPCODE(fnSetScrollSpeedNormal),
		OPCODE(fnSetScrollSpeedSlow),
		/* 70 */
		OPCODE(fnRemoveChooser),
		OPCODE(fnSetFxVolAndPan),
		OPCODE(fnSetFxVol),
		OPCODE(fnRestoreGame),
		/* 74 */
		OPCODE(fnRefreshInventory),
		OPCODE(fnChangeShadows)
	};

	_numOpcodes = ARRAYSIZE(opcodes);
	_opcodes = opcodes;
}

int Logic::runResScript(uint32 scriptRes, uint32 offset) {
	byte *scriptAddr;
	int result;

	scriptAddr = _vm->_resman->openResource(scriptRes);
	result = runScript(scriptAddr, scriptAddr, offset);
	_vm->_resman->closeResource(scriptRes);

	return result;
}

int Logic::runResObjScript(uint32 scriptRes, uint32 objRes, uint32 offset) {
	byte *scriptAddr, *objAddr;
	int result;

	scriptAddr = _vm->_resman->openResource(scriptRes);
	objAddr = _vm->_resman->openResource(objRes);
	result = runScript(scriptAddr, objAddr, offset);
	_vm->_resman->closeResource(objRes);
	_vm->_resman->closeResource(scriptRes);

	return result;
}

int Logic::runScript(byte *scriptData, byte *objectData, uint32 offset) {
	byte pc[4];

	WRITE_LE_UINT32(pc, offset);
	return runScript2(scriptData, objectData, pc);
}

// This form of the runScript function is only called directly from
// the processSession() function, which uses it to update the script PC in the
// current object hub. For reasons which I do not understand, I couldn't get it
// to work if I called the function first with a dummy offset variable, and
// and then updated the object hub myself.

int Logic::runScript2(byte *scriptData, byte *objectData, byte *offsetPtr) {
	int i;

	// Interestingly, unlike our BASS engine the stack is a local variable.
	// I don't know whether or not this is relevant to the working of the
	// BS2 engine.

	Common::FixedStack<int32, STACK_SIZE> stack;
	int32 opcodeParams[STACK_SIZE];

	uint32 offset = READ_LE_UINT32(offsetPtr);

	ResHeader header;

	header.read(scriptData);

	scriptData += ResHeader::size() + ObjectHub::size();

	// The script data format:
	//	int32_TYPE	1		Size of variable space in bytes
	//	...				The variable space
	//	int32_TYPE	1		numberOfScripts
	//	int32_TYPE	numberOfScripts	The offsets for each script

	// Initialize some stuff

	uint32 ip = 0;			 // Code pointer
	int scriptNumber;

	// Get the start of variables and start of code

	byte *localVars = scriptData + 4;
	byte *code = scriptData + READ_LE_UINT32(scriptData) + 4;
	uint32 noScripts = READ_LE_UINT32(code);

	code += 4;

	byte *offsetTable = code;

	if (offset < noScripts) {
		ip = READ_LE_UINT32(offsetTable + offset * 4);
		scriptNumber = offset;
		debug(8, "Starting script %d from %d", scriptNumber, ip);
	} else {
		ip = offset;

		for (i = 1; i < (int)noScripts; i++) {
			if (READ_LE_UINT32(offsetTable + 4 * i) >= ip)
				break;
		}

		scriptNumber = i - 1;
		debug(8, "Resuming script %d from %d", scriptNumber, ip);
	}

	// There are a couple of known script bugs related to interacting with
	// certain objects. We try to work around a few of them.

	bool checkMopBug = false;
	bool checkPyramidBug = false;
	bool checkElevatorBug = false;
	bool checkPearlBug = false;

	if (scriptNumber == 2) {
		if (strcmp((char *)header.name, "mop_73") == 0)
			checkMopBug = true;
		else if (strcmp((char *)header.name, "titipoco_81") == 0)
			checkPyramidBug = true;
		else if (strcmp((char *)header.name, "lift_82") == 0)
			checkElevatorBug = true;
		else if (strcmp((char *)header.name, "pearl_31") == 0)
			checkPearlBug = true;
	}

	code += noScripts * 4;

	// Code should now be pointing at an identifier and a checksum
	byte *checksumBlock = code;

	code += 4 * 3;

	if (READ_LE_UINT32(checksumBlock) != 12345678) {
		error("Invalid script in object %s", header.name);
		//return 0;
	}

	int32 codeLen = READ_LE_UINT32(checksumBlock + 4);
	int32 checksum = 0;

	for (i = 0; i < codeLen; i++)
		checksum += (unsigned char)code[i];

	if (checksum != (int32)READ_LE_UINT32(checksumBlock + 8)) {
		debug(1, "Checksum error in object %s", header.name);
		// This could be bad, but there has been a report about someone
		// who had problems running the German version because of
		// checksum errors. Could there be a version where checksums
		// weren't properly calculated?
	}

	bool runningScript = true;

	int parameterReturnedFromMcodeFunction = 0;	// Allow scripts to return things
	int savedStartOfMcode = 0;	// For saving start of mcode commands

	while (runningScript) {
		int32 a, b;
		int curCommand, parameter, value; // Command and parameter variables
		int retVal;
		int caseCount;
		bool foundCase;
		byte *ptr;

		curCommand = code[ip++];

		switch (curCommand) {

		// Script-related opcodes

		case CP_END_SCRIPT:
			// End the script
			runningScript = false;

			// WORKAROUND: The dreaded pyramid bug makes the torch
			// untakeable when you speak to Titipoco. This is
			// because one of the conditions for the torch to be
			// takeable is that Titipoco isn't doing anything out
			// of the ordinary. Global variable 913 has to be 0 to
			// signify that he is in his "idle" state.
			//
			// Unfortunately, simply the act of speaking to him
			// sets variable 913 to 1 (probably to stop him from
			// turning around every now and then). The script may
			// then go on to set the variable to different values
			// to trigger various behaviors in him, but if you
			// have run out of these cases the script won't ever
			// set it back to 0 again.
			//
			// So if his click hander finishes, and variable 913 is
			// 1, we set it back to 0 manually.

			if (checkPyramidBug && readVar(913) == 1) {
				warning("Working around pyramid bug: Resetting Titipoco's state");
				writeVar(913, 0);
			}

			// WORKAROUND: The not-so-known-but-should-be-dreaded
			// elevator bug.
			//
			// The click handler for the top of the elevator only
			// handles using the elevator, not examining it. When
			// examining it, the mouse cursor is removed but never
			// restored.

			if (checkElevatorBug && readVar(RIGHT_BUTTON)) {
				warning("Working around elevator bug: Restoring mouse pointer");
				fnAddHuman(NULL);
			}

			debug(9, "CP_END_SCRIPT");
			break;
		case CP_QUIT:
			// Quit out for a cycle
			WRITE_LE_UINT32(offsetPtr, ip);
			debug(9, "CP_QUIT");
			return 0;
		case CP_TERMINATE:
			// Quit out immediately without affecting the offset
			// pointer
			debug(9, "CP_TERMINATE");
			return 3;
		case CP_RESTART_SCRIPT:
			// Start the script again
			ip = FROM_LE_32(offsetTable[scriptNumber]);
			debug(9, "CP_RESTART_SCRIPT");
			break;

		// Stack-related opcodes

		case CP_PUSH_INT32:
			// Push a long word value on to the stack
			Read32ip(parameter);
			stack.push(parameter);
			debug(9, "CP_PUSH_INT32: %d", parameter);
			break;
		case CP_PUSH_LOCAL_VAR32:
			// Push the contents of a local variable
			Read16ip(parameter);
			stack.push(READ_LE_UINT32(localVars + parameter));
			debug(9, "CP_PUSH_LOCAL_VAR32: localVars[%d] => %d", parameter / 4, READ_LE_UINT32(localVars + parameter));
			break;
		case CP_PUSH_GLOBAL_VAR32:
			// Push a global variable
			Read16ip(parameter);
			stack.push(readVar(parameter));
			debug(9, "CP_PUSH_GLOBAL_VAR32: scriptVars[%d] => %d", parameter, readVar(parameter));
			break;
		case CP_PUSH_LOCAL_ADDR:
			// Push the address of a local variable

			// From what I understand, some scripts store data
			// (e.g. mouse pointers) in their local variable space
			// from the very beginning, and use this mechanism to
			// pass that data to the opcode function. I don't yet
			// know the conceptual difference between this and the
			// CP_PUSH_DEREFERENCED_STRUCTURE opcode.

			Read16ip(parameter);
			stack.push(_vm->_memory->encodePtr(localVars + parameter));
			debug(9, "CP_PUSH_LOCAL_ADDR: &localVars[%d] => %p", parameter / 4, localVars + parameter);
			break;
		case CP_PUSH_STRING:
			// Push the address of a string on to the stack
			// Get the string size
			Read8ip(parameter);

			// ip now points to the string
			ptr = code + ip;
			stack.push(_vm->_memory->encodePtr(ptr));
			debug(9, "CP_PUSH_STRING: \"%s\"", ptr);
			ip += (parameter + 1);
			break;
		case CP_PUSH_DEREFERENCED_STRUCTURE:
			// Push the address of a dereferenced structure
			Read32ip(parameter);
			ptr = objectData + 4 + ResHeader::size() + ObjectHub::size() + parameter;
			stack.push(_vm->_memory->encodePtr(ptr));
			debug(9, "CP_PUSH_DEREFERENCED_STRUCTURE: %d => %p", parameter, ptr);
			break;
		case CP_POP_LOCAL_VAR32:
			// Pop a value into a local word variable
			Read16ip(parameter);
			value = stack.pop();
			WRITE_LE_UINT32(localVars + parameter, value);
			debug(9, "CP_POP_LOCAL_VAR32: localVars[%d] = %d", parameter / 4, value);
			break;
		case CP_POP_GLOBAL_VAR32:
			// Pop a global variable
			Read16ip(parameter);
			value = stack.pop();

			// WORKAROUND for bug #1214168: The not-at-all dreaded
			// mop bug.
			//
			// At the London Docks, global variable 1003 keeps
			// track of Nico:
			//
			// 0: Hiding behind the first crate.
			// 1: Hiding behind the second crate.
			// 2: Standing in plain view on the deck.
			// 3: Hiding on the roof.
			//
			// The bug happens when trying to pick up the mop while
			// hiding on the roof. Nico climbs down, the mop is
			// picked up, but the variable remains set to 3.
			// Visually, everything looks ok. But as far as the
			// scripts are concerned, she's still hiding up on the
			// roof. This is not fatal, but leads to a number of
			// glitches until the state is corrected. E.g. trying
			// to climb back up the ladder will cause Nico to climb
			// down again.
			//
			// Global variable 1017 keeps track of the mop. Setting
			// it to 2 means that the mop has been picked up. We
			// use that as the signal that Nico's state needs to be
			// updated as well.

			if (checkMopBug && parameter == 1017 && readVar(1003) != 2) {
				warning("Working around mop bug: Setting Nico's state");
				writeVar(1003, 2);
			}

			writeVar(parameter, value);
			debug(9, "CP_POP_GLOBAL_VAR32: scriptsVars[%d] = %d", parameter, value);
			break;
		case CP_ADDNPOP_LOCAL_VAR32:
			Read16ip(parameter);
			value = READ_LE_UINT32(localVars + parameter) + stack.pop();
			WRITE_LE_UINT32(localVars + parameter, value);
			debug(9, "CP_ADDNPOP_LOCAL_VAR32: localVars[%d] => %d", parameter / 4, value);
			break;
		case CP_SUBNPOP_LOCAL_VAR32:
			Read16ip(parameter);
			value = READ_LE_UINT32(localVars + parameter) - stack.pop();
			WRITE_LE_UINT32(localVars + parameter, value);
			debug(9, "CP_SUBNPOP_LOCAL_VAR32: localVars[%d] => %d", parameter / 4, value);
			break;
		case CP_ADDNPOP_GLOBAL_VAR32:
			// Add and pop a global variable
			Read16ip(parameter);
			value = readVar(parameter) + stack.pop();
			writeVar(parameter, value);
			debug(9, "CP_ADDNPOP_GLOBAL_VAR32: scriptVars[%d] => %d", parameter, value);
			break;
		case CP_SUBNPOP_GLOBAL_VAR32:
			// Sub and pop a global variable
			Read16ip(parameter);
			value = readVar(parameter) - stack.pop();
			writeVar(parameter, value);
			debug(9, "CP_SUBNPOP_GLOBAL_VAR32: scriptVars[%d] => %d", parameter, value);
			break;

		// Jump opcodes

		case CP_SKIPONTRUE:
			// Skip if the value on the stack is true
			Read32ipLeaveip(parameter);
			value = stack.pop();
			if (!value) {
				ip += 4;
				debug(9, "CP_SKIPONTRUE: %d (IS FALSE (NOT SKIPPED))", parameter);
			} else {
				ip += parameter;
				debug(9, "CP_SKIPONTRUE: %d (IS TRUE (SKIPPED))", parameter);
			}
			break;
		case CP_SKIPONFALSE:
			// Skip if the value on the stack is false
			Read32ipLeaveip(parameter);
			value = stack.pop();
			if (value) {
				ip += 4;
				debug(9, "CP_SKIPONFALSE: %d (IS TRUE (NOT SKIPPED))", parameter);
			} else {
				ip += parameter;
				debug(9, "CP_SKIPONFALSE: %d (IS FALSE (SKIPPED))", parameter);
			}
			break;
		case CP_SKIPALWAYS:
			// skip a block
			Read32ipLeaveip(parameter);
			ip += parameter;
			debug(9, "CP_SKIPALWAYS: %d", parameter);
			break;
		case CP_SWITCH:
			// switch
			value = stack.pop();
			Read32ip(caseCount);

			// Search the cases
			foundCase = false;
			for (i = 0; i < caseCount && !foundCase; i++) {
				if (value == (int32)READ_LE_UINT32(code + ip)) {
					// We have found the case, so lets
					// jump to it
					foundCase = true;
					ip += READ_LE_UINT32(code + ip + 4);
				} else
					ip += 4 * 2;
			}

			// If we found no matching case then use the default

			if (!foundCase)
				ip += READ_LE_UINT32(code + ip);

			debug(9, "CP_SWITCH: [SORRY, NO DEBUG INFO]");
			break;
		case CP_SAVE_MCODE_START:
			// Save the start position on an mcode instruction in
			// case we need to restart it again
			savedStartOfMcode = ip - 1;
			debug(9, "CP_SAVE_MCODE_START");
			break;
		case CP_CALL_MCODE:
			// Call an mcode routine
			Read16ip(parameter);
			assert(parameter < _numOpcodes);
			// amount to adjust stack by (no of parameters)
			Read8ip(value);
			debug(9, "CP_CALL_MCODE: '%s', %d", _opcodes[parameter].desc, value);

			// The scripts do not always call the mcode command
			// with as many parameters as it can accept. To keep
			// things predictable, initialize the remaining
			// parameters to 0.

			for (i = STACK_SIZE - 1; i >= value; i--) {
				opcodeParams[i] = 0;
			}

			while (--value >= 0) {
				opcodeParams[value] = stack.pop();
			}

			retVal = (this->*_opcodes[parameter].proc)(opcodeParams);

			switch (retVal & 7) {
			case IR_STOP:
				// Quit out for a cycle
				WRITE_LE_UINT32(offsetPtr, ip);
				return 0;
			case IR_CONT:
				// Continue as normal
				break;
			case IR_TERMINATE:
				if (checkPearlBug && readVar(1290) == 0) {
					// Pearl's interaction script will wait
					// until global(1290) is no longer 0
					// before doing anything. But if the
					// script was terminated prematurely,
					// that never happens.
					warning("Working around Pearl bug: Resetting Pearl's state");
					writeVar(1290, 1);
				}
				// Return without updating the offset
				return 2;
			case IR_REPEAT:
				// Return setting offset to start of this
				// function call
				WRITE_LE_UINT32(offsetPtr, savedStartOfMcode);
				return 0;
			case IR_GOSUB:
				// that's really neat
				WRITE_LE_UINT32(offsetPtr, ip);
				return 2;
			default:
				error("Bad return code (%d) from '%s'", retVal & 7, _opcodes[parameter].desc);
			}
			parameterReturnedFromMcodeFunction = retVal >> 3;
			break;
		case CP_JUMP_ON_RETURNED:
			// Jump to a part of the script depending on
			// the return value from an mcode routine

			// Get the maximum value
			Read8ip(parameter);
			debug(9, "CP_JUMP_ON_RETURNED: %d => %d",
				parameterReturnedFromMcodeFunction,
				READ_LE_UINT32(code + ip + parameterReturnedFromMcodeFunction * 4));
			ip += READ_LE_UINT32(code + ip + parameterReturnedFromMcodeFunction * 4);
			break;

		// Operators

		case OP_ISEQUAL:
			b = stack.pop();
			a = stack.pop();
			stack.push(a == b);
			debug(9, "OP_ISEQUAL: RESULT = %d", a == b);
			break;
		case OP_NOTEQUAL:
			b = stack.pop();
			a = stack.pop();
			stack.push(a != b);
			debug(9, "OP_NOTEQUAL: RESULT = %d", a != b);
			break;
		case OP_GTTHAN:
			b = stack.pop();
			a = stack.pop();
			stack.push(a > b);
			debug(9, "OP_GTTHAN: RESULT = %d", a > b);
			break;
		case OP_LSTHAN:
			b = stack.pop();
			a = stack.pop();
			stack.push(a < b);
			debug(9, "OP_LSTHAN: RESULT = %d", a < b);
			break;
		case OP_GTTHANE:
			b = stack.pop();
			a = stack.pop();
			stack.push(a >= b);
			debug(9, "OP_GTTHANE: RESULT = %d", a >= b);
			break;
		case OP_LSTHANE:
			b = stack.pop();
			a = stack.pop();
			stack.push(a <= b);
			debug(9, "OP_LSTHANE: RESULT = %d", a <= b);
			break;
		case OP_PLUS:
			b = stack.pop();
			a = stack.pop();
			stack.push(a + b);
			debug(9, "OP_PLUS: RESULT = %d", a + b);
			break;
		case OP_MINUS:
			b = stack.pop();
			a = stack.pop();
			stack.push(a - b);
			debug(9, "OP_MINUS: RESULT = %d", a - b);
			break;
		case OP_TIMES:
			b = stack.pop();
			a = stack.pop();
			stack.push(a * b);
			debug(9, "OP_TIMES: RESULT = %d", a * b);
			break;
		case OP_DIVIDE:
			b = stack.pop();
			a = stack.pop();
			stack.push(a / b);
			debug(9, "OP_DIVIDE: RESULT = %d", a / b);
			break;
		case OP_ANDAND:
			b = stack.pop();
			a = stack.pop();
			stack.push(a && b);
			debug(9, "OP_ANDAND: RESULT = %d", a && b);
			break;
		case OP_OROR:
			b = stack.pop();
			a = stack.pop();
			stack.push(a || b);
			debug(9, "OP_OROR: RESULT = %d", a || b);
			break;

		// Debugging opcodes, I think

		case CP_DEBUGON:
			debug(9, "CP_DEBUGON");
			break;
		case CP_DEBUGOFF:
			debug(9, "CP_DEBUGOFF");
			break;
		case CP_TEMP_TEXT_PROCESS:
			Read32ip(parameter);
			debug(9, "CP_TEMP_TEXT_PROCESS: %d", parameter);
			break;
		default:
			error("Invalid script command %d", curCommand);
			return 3;	// for compilers that don't support NORETURN
		}
	}

	return 1;
}

} // End of namespace Sword2
