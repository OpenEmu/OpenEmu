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

#include "made/script.h"
#include "made/scriptfuncs.h"
#include "made/made.h"
#include "made/database.h"
#include "made/screen.h"

#include "common/util.h"

namespace Made {

/* ScriptInterpreter */


ScriptInterpreter::ScriptInterpreter(MadeEngine *vm) : _vm(vm) {
#ifdef DUMP_SCRIPTS
#define COMMAND(x, sig) { &ScriptInterpreter::x, #x, sig }
#else
#define COMMAND(x, sig) { &ScriptInterpreter::x, #x}
#endif
	static CommandEntry commandProcs[] = {
		/* 01 */
		COMMAND(cmd_branchTrue, "W"),
		COMMAND(cmd_branchFalse, "W"),
		COMMAND(cmd_branch, "W"),
		COMMAND(cmd_true, ""),
		/* 05 */
		COMMAND(cmd_false, ""),
		COMMAND(cmd_push, ""),
		COMMAND(cmd_not, ""),
		COMMAND(cmd_add, ""),
		/* 09 */
		COMMAND(cmd_sub, ""),
		COMMAND(cmd_mul, ""),
		COMMAND(cmd_div, ""),
		COMMAND(cmd_mod, ""),
		/* 13 */
		COMMAND(cmd_band, ""),
		COMMAND(cmd_bor, ""),
		COMMAND(cmd_bnot, ""),
		COMMAND(cmd_lt, ""),
		/* 17 */
		COMMAND(cmd_eq, ""),
		COMMAND(cmd_gt, ""),
		COMMAND(cmd_loadConstant, "w"),
		COMMAND(cmd_loadVariable, "w"),
		/* 21 */
		COMMAND(cmd_getObjectProperty, ""),
		COMMAND(cmd_setObjectProperty, ""),
		COMMAND(cmd_set, "w"),
		COMMAND(cmd_print, ""),
		/* 25 */
		COMMAND(cmd_terpri, ""),
		COMMAND(cmd_printNumber, ""),
		COMMAND(cmd_vref, ""),
		COMMAND(cmd_vset, ""),
		/* 29 */
		COMMAND(cmd_vsize, ""),
		COMMAND(cmd_exit, ""),
		COMMAND(cmd_return, ""),
		COMMAND(cmd_call, "b"),
		/* 33 */
		COMMAND(cmd_svar, ""),
		COMMAND(cmd_sset, ""),
		COMMAND(cmd_split, ""),
		COMMAND(cmd_snlit, ""),
		/* 37 */
		COMMAND(cmd_yorn, ""),
		COMMAND(cmd_save, ""),
		COMMAND(cmd_restore, ""),
		COMMAND(cmd_arg, "b"),
		/* 41 */
		COMMAND(cmd_aset, "b"),
		COMMAND(cmd_tmp, "b"),
		COMMAND(cmd_tset, "b"),
		COMMAND(cmd_tspace, "b"),
		/* 45 */
		COMMAND(cmd_class, ""),
		COMMAND(cmd_objectp, ""),
		COMMAND(cmd_vectorp, ""),
		COMMAND(cmd_restart, ""),
		/* 49 */
		COMMAND(cmd_rand, ""),
		COMMAND(cmd_randomize, ""),
		COMMAND(cmd_send, "b"),
		COMMAND(cmd_extend, "Eb"),
		/* 53 */
		COMMAND(cmd_catch, ""),
		COMMAND(cmd_cdone, ""),
		COMMAND(cmd_throw, ""),
		COMMAND(cmd_functionp, ""),
		/* 57 */
		COMMAND(cmd_le, ""),
		COMMAND(cmd_ge, ""),
		COMMAND(cmd_varx, ""),
		COMMAND(cmd_setx, "")
	};
	_commands = commandProcs;
	_commandsMax = ARRAYSIZE(commandProcs) + 1;

	_functions = new ScriptFunctions(_vm);
	_functions->setupExternalsTable();

#undef COMMAND
}

ScriptInterpreter::~ScriptInterpreter() {
	delete _functions;
}

void ScriptInterpreter::runScript(int16 scriptObjectIndex) {

	uint32 opcodeSleepCounter = 0;

	_runningScriptObjectIndex = scriptObjectIndex;

	_localStackPos = _stack.getStackPos();

	_codeBase = _vm->_dat->getObject(_runningScriptObjectIndex)->getData();
	_codeIp = _codeBase;

	while (!_vm->shouldQuit()) {
		byte opcode = readByte();

		if (opcode >= 1 && opcode <= _commandsMax) {
			debug(4, "[%04X:%04X] %s", _runningScriptObjectIndex, (uint) (_codeIp - _codeBase), _commands[opcode - 1].desc);
			(this->*_commands[opcode - 1].proc)();
		} else {
			warning("ScriptInterpreter::runScript(%d) Unknown opcode %02X", _runningScriptObjectIndex, opcode);
		}

		/* We sleep a little after 500 opcodes to reduce the CPU load.
		*/
		if (++opcodeSleepCounter > 500) {
			_vm->_screen->updateScreenAndWait(5);
			opcodeSleepCounter = 0;
		}

	}
}

byte ScriptInterpreter::readByte() {
	return *_codeIp++;
}

int16 ScriptInterpreter::readInt16() {
	int16 temp = (int16)READ_LE_UINT16(_codeIp);
	_codeIp += 2;
	debug(4, "readInt16() value = %04X", temp);
	return temp;
}

void ScriptInterpreter::cmd_branchTrue() {
	int16 ofs = readInt16();
	if (_stack.top() != 0)
		_codeIp = _codeBase + ofs;
}

void ScriptInterpreter::cmd_branchFalse() {
	int16 ofs = readInt16();
	if (_stack.top() == 0)
		_codeIp = _codeBase + ofs;
}

void ScriptInterpreter::cmd_branch() {
	int16 ofs = readInt16();
	_codeIp = _codeBase + ofs;
}

void ScriptInterpreter::cmd_true() {
	_stack.setTop(-1);
}

void ScriptInterpreter::cmd_false() {
	_stack.setTop(0);
}

void ScriptInterpreter::cmd_push() {
	_stack.push();
}

void ScriptInterpreter::cmd_not() {
	if (_stack.top() == 0)
		_stack.setTop(-1);
	else
		_stack.setTop(0);
}

void ScriptInterpreter::cmd_add() {
	int16 value = _stack.pop();
	_stack.setTop(_stack.top() + value);
}

void ScriptInterpreter::cmd_sub() {
	int16 value = _stack.pop();
	_stack.setTop(_stack.top() - value);
}

void ScriptInterpreter::cmd_mul() {
	int16 value = _stack.pop();
	_stack.setTop(_stack.top() * value);
}

void ScriptInterpreter::cmd_div() {
	int16 value = _stack.pop();
	if (value == 0)
		_stack.setTop(0);
	else
		_stack.setTop(_stack.top() / value);
}

void ScriptInterpreter::cmd_mod() {
	int16 value = _stack.pop();
	if (value == 0)
		_stack.setTop(0);
	else
		_stack.setTop(_stack.top() % value);
}

void ScriptInterpreter::cmd_band() {
	int16 value = _stack.pop();
	_stack.setTop(_stack.top() & value);
}

void ScriptInterpreter::cmd_bor() {
	int16 value = _stack.pop();
	_stack.setTop(_stack.top() | value);
}

void ScriptInterpreter::cmd_bnot() {
	_stack.setTop(~_stack.top());
}

void ScriptInterpreter::cmd_lt() {
	int16 value = _stack.pop();
	if (_stack.top() < value)
		_stack.setTop(-1);
	else
		_stack.setTop(0);
}

void ScriptInterpreter::cmd_eq() {
	int16 value = _stack.pop();
	if (_stack.top() == value)
		_stack.setTop(-1);
	else
		_stack.setTop(0);
}

void ScriptInterpreter::cmd_gt() {
	int16 value = _stack.pop();
	if (_stack.top() > value)
		_stack.setTop(-1);
	else
		_stack.setTop(0);
}

void ScriptInterpreter::cmd_loadConstant() {
	int16 value = readInt16();
	debug(4, "value = %04X (%d)", value, value);
	_stack.setTop(value);
}

void ScriptInterpreter::cmd_loadVariable() {
	int16 variable = readInt16();
	int16 value = _vm->_dat->getVar(variable);
	debug(4, "variable = %d; value = %d (%04X)", variable, value, value);
	_stack.setTop(value);
}

void ScriptInterpreter::cmd_getObjectProperty() {
	int16 propertyId = _stack.pop();
	int16 objectIndex = _stack.top();
	int16 value = _vm->_dat->getObjectProperty(objectIndex, propertyId);
	debug(4, "value = %04X(%d)", value, value);
	_stack.setTop(value);
}

void ScriptInterpreter::cmd_setObjectProperty() {
	int16 value = _stack.pop();
	int16 propertyId = _stack.pop();
	int16 objectIndex = _stack.top();
	value = _vm->_dat->setObjectProperty(objectIndex, propertyId, value);
	_stack.setTop(value);
}

void ScriptInterpreter::cmd_set() {
	int16 variable = readInt16();
	debug(4, "var(%d) = %04d (%d)", variable, _stack.top(), _stack.top());
	_vm->_dat->setVar(variable, _stack.top());
}

void ScriptInterpreter::cmd_print() {
	// TODO: This opcode was used for printing debug messages
	const char *text = _vm->_dat->getObjectString(_stack.top());
	debug(4, "%s", text);
	_stack.setTop(0);
}

void ScriptInterpreter::cmd_terpri() {
	// TODO: This opcode was used for printing debug messages
	debug(4, "\n");
	_stack.setTop(0);
}

void ScriptInterpreter::cmd_printNumber() {
	// TODO: This opcode was used for printing debug messages
	debug(4, "%d", _stack.top());
}

void ScriptInterpreter::cmd_vref() {
	int16 value = 0;
	int16 index = _stack.pop();
	int16 objectIndex = _stack.top();
	debug(4, "index = %d; objectIndex = %d", index, objectIndex);
	if (objectIndex > 0) {
		Object *obj = _vm->_dat->getObject(objectIndex);
		value = obj->getVectorItem(index);
	}
	_stack.setTop(value);
	debug(4, "--> value = %d", value);
}

void ScriptInterpreter::cmd_vset() {
	int16 value = _stack.pop();
	int16 index = _stack.pop();
	int16 objectIndex = _stack.top();
	debug(4, "index = %d; objectIndex = %d; value = %d", index, objectIndex, value);
	if (objectIndex > 0) {
		Object *obj = _vm->_dat->getObject(objectIndex);
		obj->setVectorItem(index, value);
	}
	_stack.setTop(value);
}

void ScriptInterpreter::cmd_vsize() {
	int16 objectIndex = _stack.top();
	int16 size = 0;
	if (objectIndex > 0) {
		Object *obj = _vm->_dat->getObject(objectIndex);
		size = obj->getVectorSize();
	}
	_stack.setTop(size);
}

void ScriptInterpreter::cmd_exit() {
	_vm->quitGame();
	// Make sure the "quit" event is handled immediately
	_vm->handleEvents();
}

void ScriptInterpreter::cmd_return() {

	// Check if returning from main function
	if (_localStackPos == kScriptStackSize) {
		_vm->quitGame();
		// Make sure the "quit" event is handled immediately
		_vm->handleEvents();
		return;
	}

	int16 funcResult = _stack.top();
	_stack.setStackPos(_localStackPos);
	_localStackPos = kScriptStackLimit - _stack.pop();
	_runningScriptObjectIndex = _stack.pop();
	_codeBase = _vm->_dat->getObject(_runningScriptObjectIndex)->getData();
	_codeIp = _codeBase + _stack.pop();
	byte argc = _stack.pop();
	_stack.free(argc);
	_stack.setTop(funcResult);
	debug(4, "LEAVE: stackPtr = %d; _localStackPos = %d\n", _stack.getStackPos(), _localStackPos);
}

void ScriptInterpreter::cmd_call() {
	debug(4, "\nENTER: stackPtr = %d; _localStackPos = %d", _stack.getStackPos(), _localStackPos);
	byte argc = readByte();

	_stack.push(argc);
	_stack.push(_codeIp - _codeBase);
	_stack.push(_runningScriptObjectIndex);
	_stack.push(kScriptStackLimit - _localStackPos);
	_localStackPos = _stack.getStackPos();
	_runningScriptObjectIndex = _stack.peek(_localStackPos + argc + 4);
	debug(4, "argc = %d; _runningScriptObjectIndex = %04X", argc, _runningScriptObjectIndex);
	_codeBase = _vm->_dat->getObject(_runningScriptObjectIndex)->getData();
	_codeIp = _codeBase;
}

void ScriptInterpreter::cmd_svar() {
	// Never used in LGOP2, RTZ, Manhole:NE, Rodney
	warning("Unimplemented command: cmd_svar");
}

void ScriptInterpreter::cmd_sset() {
	// Never used in LGOP2, RTZ, Manhole:NE, Rodney
	warning("Unimplemented command: cmd_sset");
}

void ScriptInterpreter::cmd_split() {
	// Never used in LGOP2, RTZ, Manhole:NE, Rodney
	warning("Unimplemented command: cmd_split");
}

void ScriptInterpreter::cmd_snlit() {
	// Never used in LGOP2, RTZ, Manhole:NE, Rodney
	warning("Unimplemented command: cmd_snlit");
}

void ScriptInterpreter::cmd_yorn() {
	// Never used in LGOP2, RTZ, Manhole:NE, Rodney
	warning("Unimplemented command: cmd_yorn");
}

void ScriptInterpreter::cmd_save() {
	int16 result = 0;
	int16 stringOfs = _stack.top();
	const char *filename = _vm->_dat->getString(stringOfs);
	result = _vm->_dat->savegame(filename, "", 0);
	_stack.setTop(result);
}

void ScriptInterpreter::cmd_restore() {
	int16 result = 0;
	int16 stringOfs = _stack.top();
	const char *filename = _vm->_dat->getString(stringOfs);
	result = _vm->_dat->loadgame(filename, 0);
	_stack.setTop(result);
}

void ScriptInterpreter::cmd_arg() {
	int16 argIndex = readByte();
	debug(4, "argIndex = %d; value = %04X (%d)", argIndex, _stack.peek(_localStackPos + 4 + argIndex), _stack.peek(_localStackPos + 4 + argIndex));
	_stack.setTop(_stack.peek(_localStackPos + 4 + argIndex));
}

void ScriptInterpreter::cmd_aset() {
	int16 argIndex = readByte();
	debug(4, "argIndex = %d; value = %d", argIndex, _stack.peek(_localStackPos + 4 + argIndex));
	_stack.poke(_localStackPos + 4 + argIndex, _stack.top());
}

void ScriptInterpreter::cmd_tmp() {
	int16 tempIndex = readByte();
	debug(4, "tempIndex = %d; value = %d", tempIndex, _stack.peek(_localStackPos - tempIndex - 1));
	_stack.setTop(_stack.peek(_localStackPos - tempIndex - 1));
}

void ScriptInterpreter::cmd_tset() {
	int16 tempIndex = readByte();
	debug(4, "tempIndex = %d; value = %d", tempIndex, _stack.top());
	_stack.poke(_localStackPos - tempIndex - 1, _stack.top());
}

void ScriptInterpreter::cmd_tspace() {
	int16 tempCount = readByte();
	debug(4, "tempCount = %d", tempCount);
	_stack.alloc(tempCount);
}

void ScriptInterpreter::cmd_class() {
	// Never used in LGOP2, RTZ, Manhole:NE, Rodney
	warning("Unimplemented command: cmd_class");
}

void ScriptInterpreter::cmd_objectp() {
	Object *obj = _vm->_dat->getObject(_stack.top());
	if (obj->isObject())
		_stack.setTop(-1);
	else
		_stack.setTop(0);
}

void ScriptInterpreter::cmd_vectorp() {
	// Never used in LGOP2, RTZ, Manhole:NE, Rodney
	warning("Unimplemented command: cmd_vectorp");
}

void ScriptInterpreter::cmd_restart() {
	_vm->_dat->reload();
	_vm->_screen->clearChannels();
	_vm->resetAllTimers();
	_stack.setTop(0);
}

void ScriptInterpreter::cmd_rand() {
	_stack.setTop(_vm->_rnd->getRandomNumber(_stack.top() - 1));
}

void ScriptInterpreter::cmd_randomize() {
	_vm->_rnd->setSeed(g_system->getMillis());
	_stack.setTop(0);
}

void ScriptInterpreter::cmd_send() {

	debug(4, "\nENTER: stackPtr = %d; _localStackPos = %d", _stack.getStackPos(), _localStackPos);

	byte argc = readByte();

	debug(4, "argc = %d", argc);

	_stack.push(argc);
	_stack.push(_codeIp - _codeBase);
	_stack.push(_runningScriptObjectIndex);
	_stack.push(kScriptStackLimit - _localStackPos);
	_localStackPos = _stack.getStackPos();

	int16 propertyId = _stack.peek(_localStackPos + argc + 2);
	int16 objectIndex = _stack.peek(_localStackPos + argc + 4);

	debug(4, "objectIndex = %d (%04X); propertyId = %d(%04X)", objectIndex, objectIndex, propertyId, propertyId);

	if (objectIndex != 0) {
		objectIndex = _vm->_dat->getObject(objectIndex)->getClass();
	} else {
		objectIndex = _stack.peek(_localStackPos + argc + 3);
	}

	debug(4, "--> objectIndex = %d(%04X)", objectIndex, objectIndex);

	if (objectIndex != 0) {
		_runningScriptObjectIndex = _vm->_dat->getObjectProperty(objectIndex, propertyId);
		if (_runningScriptObjectIndex != 0) {
			_codeBase = _vm->_dat->getObject(_runningScriptObjectIndex)->getData();
			_codeIp = _codeBase;
		} else {
			_stack.push(0);
			cmd_return();
		}
	} else {
		_stack.push(0);
		cmd_return();
	}

}

void ScriptInterpreter::cmd_extend() {

	byte func = readByte();

	byte argc = readByte();
	int16 *argv = _stack.getStackPtr();

	debug(4, "func = %d (%s); argc = %d", func, _functions->getFuncName(func), argc);
	for (int i = 0; i < argc; i++)
		debug(2, "argv[%02d] = %04X (%d)", i, argv[i], argv[i]);

	int16 result = _functions->callFunction(func, argc, argv);
	debug(2, "result = %04X (%d)", result, result);

	_stack.free(argc);

	_stack.setTop(result);

}

void ScriptInterpreter::cmd_catch() {
	// Never used in LGOP2, RTZ, Manhole:NE, Rodney
	warning("Unimplemented command: cmd_catch");
}

void ScriptInterpreter::cmd_cdone() {
	// Never used in LGOP2, RTZ, Manhole:NE, Rodney
	warning("Unimplemented command: cmd_cdone");
}

void ScriptInterpreter::cmd_throw() {
	// Never used in LGOP2, RTZ, Manhole:NE, Rodney
	warning("Unimplemented command: cmd_throw");
}

void ScriptInterpreter::cmd_functionp() {
	// Never used in LGOP2, RTZ, Manhole:NE, Rodney
	warning("Unimplemented command: cmd_functionp");
}

void ScriptInterpreter::cmd_le() {
	int16 value = _stack.pop();
	if (_stack.top() <= value)
		_stack.setTop(-1);
	else
		_stack.setTop(0);
}

void ScriptInterpreter::cmd_ge() {
	int16 value = _stack.pop();
	if (_stack.top() >= value)
		_stack.setTop(-1);
	else
		_stack.setTop(0);
}

void ScriptInterpreter::cmd_varx() {
	// Never used in LGOP2, RTZ, Manhole:NE, Rodney
	warning("Unimplemented command: cmd_varx");
}

void ScriptInterpreter::cmd_setx() {
	// Never used in LGOP2, RTZ, Manhole:NE, Rodney
	warning("Unimplemented command: cmd_setx");
}

#ifdef DUMP_SCRIPTS
void ScriptInterpreter::dumpScript(int16 objectIndex, int *opcodeStats, int *externStats) {

	debug(1, "Dumping code for object %04X", objectIndex);

	Object *obj = _vm->_dat->getObject(objectIndex);
	byte *code = obj->getData(), *codeStart = code, *codeEnd = code + obj->getSize();

	while (code < codeEnd) {
		byte opcode = *code++;
		if (opcode >= 1 && opcode <= _commandsMax) {
			Common::String codeLine;
			const char *desc = _commands[opcode - 1].desc;
			const char *sig = _commands[opcode - 1].sig;
			int valueType; /* 0: dec; 1: hex; 2: extended function */
			int16 value;
			opcodeStats[opcode - 1]++;

			codeLine += Common::String::format("[%04X] ", (uint16)(code - codeStart - 1));
			codeLine += desc;
			for (; *sig != '\0'; sig++) {
				codeLine += " ";
				switch (*sig) {
				case 'b':
					valueType = 0;
					value = *code++;
					break;
				case 'B':
					valueType = 1;
					value = *code++;
					break;
				case 'w':
					valueType = 0;
					value = READ_LE_UINT16(code);
					code += 2;
					break;
				case 'W':
					valueType = 1;
					value = READ_LE_UINT16(code);
					code += 2;
					break;
				case 'E':
					valueType = 2;
					value = *code++;
					break;
				}

				Common::String tempStr;
				switch (valueType) {
				case 0:
					tempStr = Common::String::format("%d", value);
					break;
				case 1:
					tempStr = Common::String::format("0x%X", value);
					break;
				case 2:
					if (value < _functions->getCount()) {
						tempStr = Common::String::format("%s", _functions->getFuncName(value));
						externStats[value]++;
					} else {
						tempStr = Common::String::format("invalid: %d", value);
					}
					break;
				}
				codeLine += tempStr;
			}
			debug(1, "%s", codeLine.c_str());
		} else {
			error("ScriptInterpreter::dumpScript(%d) Unknown opcode %02X", objectIndex, opcode);
		}
	}
	debug(1, "-------------------------------------------");
}

void ScriptInterpreter::dumpAllScripts() {
	int *opcodeStats = new int[_commandsMax - 1];
	int *externStats = new int[_functions->getCount()];

	for (int i = 0; i < _commandsMax; i++)
		opcodeStats[i] = 0;
	for (int i = 0; i < _functions->getCount(); i++)
		externStats[i] = 0;

	for (uint objectIndex = 1; objectIndex <= _vm->_dat->getObjectCount(); objectIndex++) {
		Object *obj = _vm->_dat->getObject(objectIndex);
		// Check if it's a byte array which might contain code
		if (obj->getClass() != 0x7FFF)
			continue;
		// Code objects aren't excplicitly marked as such, we need to check if
		// the last byte is a cmd_return opcode.
		byte *retByte = obj->getData() + obj->getSize() - 1;
		if (*retByte == 0x1F) {
			dumpScript(objectIndex, opcodeStats, externStats);
		}
	}

	debug(1, "OPCODE statistics:");
	for (int i = 0; i < _commandsMax - 1; i++)
		if (opcodeStats[i] > 0)
			debug(1, "%-30s: %d", _commands[i].desc, opcodeStats[i]);
	debug(1, "UNUSED OPCODE statistics:");
	for (int i = 0; i < _commandsMax - 1; i++)
		if (opcodeStats[i] == 0)
			debug(1, "%-30s: %d", _commands[i].desc, opcodeStats[i]);
	debug(1, ".");

	debug(1, "EXTERN statistics (%d):", _functions->getCount());
	for (int i = 0; i < _functions->getCount(); i++)
		if (externStats[i] > 0)
			debug(1, "%-30s: %d", _functions->getFuncName(i), externStats[i]);
	debug(1, "UNUSED EXTERN statistics (%d):", _functions->getCount());
	for (int i = 0; i < _functions->getCount(); i++)
		if (externStats[i] == 0)
			debug(1, "%-30s: %d", _functions->getFuncName(i), externStats[i]);
	debug(1, ".");

	delete[] opcodeStats;
	delete[] externStats;
}
#endif

} // End of namespace Made
