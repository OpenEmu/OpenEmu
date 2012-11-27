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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/scriptables/script_engine.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "common/memstream.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(ScScript, false)

//////////////////////////////////////////////////////////////////////////
ScScript::ScScript(BaseGame *inGame, ScEngine *engine) : BaseClass(inGame) {
	_buffer = NULL;
	_bufferSize = _iP = 0;
	_scriptStream = NULL;
	_filename = NULL;
	_currentLine = 0;

	_symbols = NULL;
	_numSymbols = 0;

	_engine = engine;

	_globals = NULL;

	_scopeStack = NULL;
	_callStack  = NULL;
	_thisStack  = NULL;
	_stack      = NULL;

	_operand    = NULL;
	_reg1       = NULL;

	_functions = NULL;
	_numFunctions = 0;

	_methods = NULL;
	_numMethods = 0;

	_events = NULL;
	_numEvents = 0;

	_externals = NULL;
	_numExternals = 0;

	_state = SCRIPT_FINISHED;
	_origState = SCRIPT_FINISHED;

	_waitObject = NULL;
	_waitTime = 0;
	_waitFrozen = false;
	_waitScript = NULL;

	_timeSlice = 0;

	_thread = false;
	_methodThread = false;
	_threadEvent = NULL;

	_freezable = true;
	_owner = NULL;

	_unbreakable = false;
	_parentScript = NULL;

	_tracingMode = false;
}


//////////////////////////////////////////////////////////////////////////
ScScript::~ScScript() {
	cleanup();
}

void ScScript::readHeader() {
	uint32 oldPos = _scriptStream->pos();
	_scriptStream->seek(0);
	_header.magic = _scriptStream->readUint32LE();
	_header.version = _scriptStream->readUint32LE();
	_header.codeStart = _scriptStream->readUint32LE();
	_header.funcTable = _scriptStream->readUint32LE();
	_header.symbolTable = _scriptStream->readUint32LE();
	_header.eventTable = _scriptStream->readUint32LE();
	_header.externalsTable = _scriptStream->readUint32LE();
	_header.methodTable = _scriptStream->readUint32LE();
	_scriptStream->seek(oldPos);
}


//////////////////////////////////////////////////////////////////////////
bool ScScript::initScript() {
	if (!_scriptStream) {
		_scriptStream = new Common::MemoryReadStream(_buffer, _bufferSize);
	}
	readHeader();

	if (_header.magic != SCRIPT_MAGIC) {
		_gameRef->LOG(0, "File '%s' is not a valid compiled script", _filename);
		cleanup();
		return STATUS_FAILED;
	}

	if (_header.version > SCRIPT_VERSION) {
		_gameRef->LOG(0, "Script '%s' has a wrong version %d.%d (expected %d.%d)", _filename, _header.version / 256, _header.version % 256, SCRIPT_VERSION / 256, SCRIPT_VERSION % 256);
		cleanup();
		return STATUS_FAILED;
	}

	initTables();

	// init stacks
	_scopeStack = new ScStack(_gameRef);
	_callStack  = new ScStack(_gameRef);
	_thisStack  = new ScStack(_gameRef);
	_stack      = new ScStack(_gameRef);

	_operand    = new ScValue(_gameRef);
	_reg1       = new ScValue(_gameRef);


	// skip to the beginning
	_iP = _header.codeStart;
	_scriptStream->seek(_iP);
	_currentLine = 0;

	// ready to rumble...
	_state = SCRIPT_RUNNING;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool ScScript::initTables() {
	uint32 origIP = _iP;

	readHeader();
	// load symbol table
	_iP = _header.symbolTable;

	_numSymbols = getDWORD();
	_symbols = new char*[_numSymbols];
	for (uint32 i = 0; i < _numSymbols; i++) {
		uint32 index = getDWORD();
		_symbols[index] = getString();
	}

	// load functions table
	_iP = _header.funcTable;

	_numFunctions = getDWORD();
	_functions = new TFunctionPos[_numFunctions];
	for (uint32 i = 0; i < _numFunctions; i++) {
		_functions[i].pos = getDWORD();
		_functions[i].name = getString();
	}


	// load events table
	_iP = _header.eventTable;

	_numEvents = getDWORD();
	_events = new TEventPos[_numEvents];
	for (uint32 i = 0; i < _numEvents; i++) {
		_events[i].pos = getDWORD();
		_events[i].name = getString();
	}


	// load externals
	if (_header.version >= 0x0101) {
		_iP = _header.externalsTable;

		_numExternals = getDWORD();
		_externals = new TExternalFunction[_numExternals];
		for (uint32 i = 0; i < _numExternals; i++) {
			_externals[i].dll_name = getString();
			_externals[i].name = getString();
			_externals[i].call_type = (TCallType)getDWORD();
			_externals[i].returns = (TExternalType)getDWORD();
			_externals[i].nu_params = getDWORD();
			if (_externals[i].nu_params > 0) {
				_externals[i].params = new TExternalType[_externals[i].nu_params];
				for (int j = 0; j < _externals[i].nu_params; j++) {
					_externals[i].params[j] = (TExternalType)getDWORD();
				}
			}
		}
	}

	// load method table
	_iP = _header.methodTable;

	_numMethods = getDWORD();
	_methods = new TMethodPos[_numMethods];
	for (uint32 i = 0; i < _numMethods; i++) {
		_methods[i].pos = getDWORD();
		_methods[i].name = getString();
	}


	_iP = origIP;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool ScScript::create(const char *filename, byte *buffer, uint32 size, BaseScriptHolder *owner) {
	cleanup();

	_thread = false;
	_methodThread = false;

	delete[] _threadEvent;
	_threadEvent = NULL;

	_filename = new char[strlen(filename) + 1];
	if (_filename) {
		strcpy(_filename, filename);
	}

	_buffer = new byte [size];
	if (!_buffer) {
		return STATUS_FAILED;
	}

	memcpy(_buffer, buffer, size);

	_bufferSize = size;

	bool res = initScript();
	if (DID_FAIL(res)) {
		return res;
	}

	// establish global variables table
	_globals = new ScValue(_gameRef);

	_owner = owner;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool ScScript::createThread(ScScript *original, uint32 initIP, const Common::String &eventName) {
	cleanup();

	_thread = true;
	_methodThread = false;
	_threadEvent = new char[eventName.size() + 1];
	if (_threadEvent) {
		strcpy(_threadEvent, eventName.c_str());
	}

	// copy filename
	_filename = new char[strlen(original->_filename) + 1];
	if (_filename) {
		strcpy(_filename, original->_filename);
	}

	// copy buffer
	_buffer = new byte [original->_bufferSize];
	if (!_buffer) {
		return STATUS_FAILED;
	}

	memcpy(_buffer, original->_buffer, original->_bufferSize);
	_bufferSize = original->_bufferSize;

	// initialize
	bool res = initScript();
	if (DID_FAIL(res)) {
		return res;
	}

	// copy globals
	_globals = original->_globals;

	// skip to the beginning of the event
	_iP = initIP;
	_scriptStream->seek(_iP);

	_timeSlice = original->_timeSlice;
	_freezable = original->_freezable;
	_owner = original->_owner;

	_engine = original->_engine;
	_parentScript = original;

	return STATUS_OK;
}




//////////////////////////////////////////////////////////////////////////
bool ScScript::createMethodThread(ScScript *original, const Common::String &methodName) {
	uint32 ip = original->getMethodPos(methodName);
	if (ip == 0) {
		return STATUS_FAILED;
	}

	cleanup();

	_thread = true;
	_methodThread = true;
	_threadEvent = new char[methodName.size() + 1];
	if (_threadEvent) {
		strcpy(_threadEvent, methodName.c_str());
	}

	// copy filename
	_filename = new char[strlen(original->_filename) + 1];
	if (_filename) {
		strcpy(_filename, original->_filename);
	}

	// copy buffer
	_buffer = new byte [original->_bufferSize];
	if (!_buffer) {
		return STATUS_FAILED;
	}

	memcpy(_buffer, original->_buffer, original->_bufferSize);
	_bufferSize = original->_bufferSize;

	// initialize
	bool res = initScript();
	if (DID_FAIL(res)) {
		return res;
	}

	// copy globals
	_globals = original->_globals;

	// skip to the beginning of the event
	_iP = ip;

	_timeSlice = original->_timeSlice;
	_freezable = original->_freezable;
	_owner = original->_owner;

	_engine = original->_engine;
	_parentScript = original;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void ScScript::cleanup() {
	if (_buffer) {
		delete[] _buffer;
	}
	_buffer = NULL;

	if (_filename) {
		delete[] _filename;
	}
	_filename = NULL;

	if (_symbols) {
		delete[] _symbols;
	}
	_symbols = NULL;
	_numSymbols = 0;

	if (_globals && !_thread) {
		delete _globals;
	}
	_globals = NULL;

	delete _scopeStack;
	_scopeStack = NULL;

	delete _callStack;
	_callStack = NULL;

	delete _thisStack;
	_thisStack = NULL;

	delete _stack;
	_stack = NULL;

	if (_functions) {
		delete[] _functions;
	}
	_functions = NULL;
	_numFunctions = 0;

	if (_methods) {
		delete[] _methods;
	}
	_methods = NULL;
	_numMethods = 0;

	if (_events) {
		delete[] _events;
	}
	_events = NULL;
	_numEvents = 0;


	if (_externals) {
		for (uint32 i = 0; i < _numExternals; i++) {
			if (_externals[i].nu_params > 0) {
				delete[] _externals[i].params;
			}
		}
		delete[] _externals;
	}
	_externals = NULL;
	_numExternals = 0;

	delete _operand;
	delete _reg1;
	_operand = NULL;
	_reg1 = NULL;

	delete[] _threadEvent;
	_threadEvent = NULL;

	_state = SCRIPT_FINISHED;

	_waitObject = NULL;
	_waitTime = 0;
	_waitFrozen = false;
	_waitScript = NULL;

	_parentScript = NULL; // ref only

	delete _scriptStream;
}


//////////////////////////////////////////////////////////////////////////
uint32 ScScript::getDWORD() {
	_scriptStream->seek((int32)_iP);
	uint32 ret = _scriptStream->readUint32LE();
	_iP += sizeof(uint32);
//	assert(oldRet == ret);
	return ret;
}

//////////////////////////////////////////////////////////////////////////
double ScScript::getFloat() {
	_scriptStream->seek((int32)_iP);
	byte buffer[8];
	_scriptStream->read(buffer, 8);

#ifdef SCUMM_BIG_ENDIAN
	// TODO: For lack of a READ_LE_UINT64
	SWAP(buffer[0], buffer[7]);
	SWAP(buffer[1], buffer[6]);
	SWAP(buffer[2], buffer[5]);
	SWAP(buffer[3], buffer[4]);
#endif

	double ret = *(double *)(buffer);
	_iP += 8; // Hardcode the double-size used originally.
	return ret;
}


//////////////////////////////////////////////////////////////////////////
char *ScScript::getString() {
	char *ret = (char *)(_buffer + _iP);
	while (*(char *)(_buffer + _iP) != '\0') {
		_iP++;
	}
	_iP++; // string terminator
	_scriptStream->seek(_iP);

	return ret;
}


//////////////////////////////////////////////////////////////////////////
bool ScScript::executeInstruction() {
	bool ret = STATUS_OK;

	uint32 dw;
	const char *str = NULL;

	//ScValue* op = new ScValue(_gameRef);
	_operand->cleanup();

	ScValue *op1;
	ScValue *op2;

	uint32 inst = getDWORD();
	switch (inst) {

	case II_DEF_VAR:
		_operand->setNULL();
		dw = getDWORD();
		if (_scopeStack->_sP < 0) {
			_globals->setProp(_symbols[dw], _operand);
		} else {
			_scopeStack->getTop()->setProp(_symbols[dw], _operand);
		}

		break;

	case II_DEF_GLOB_VAR:
	case II_DEF_CONST_VAR: {
		dw = getDWORD();
		/*      char *temp = _symbols[dw]; // TODO delete */
		// only create global var if it doesn't exist
		if (!_engine->_globals->propExists(_symbols[dw])) {
			_operand->setNULL();
			_engine->_globals->setProp(_symbols[dw], _operand, false, inst == II_DEF_CONST_VAR);
		}
		break;
	}

	case II_RET:
		if (_scopeStack->_sP >= 0 && _callStack->_sP >= 0) {
			_scopeStack->pop();
			_iP = (uint32)_callStack->pop()->getInt();
		} else {
			if (_thread) {
				_state = SCRIPT_THREAD_FINISHED;
			} else {
				if (_numEvents == 0 && _numMethods == 0) {
					_state = SCRIPT_FINISHED;
				} else {
					_state = SCRIPT_PERSISTENT;
				}
			}
		}

		break;

	case II_RET_EVENT:
		_state = SCRIPT_FINISHED;
		break;


	case II_CALL:
		dw = getDWORD();

		_operand->setInt(_iP);
		_callStack->push(_operand);

		_iP = dw;

		break;

	case II_CALL_BY_EXP: {
		// push var
		// push string
		str = _stack->pop()->getString();
		char *methodName = new char[strlen(str) + 1];
		strcpy(methodName, str);

		ScValue *var = _stack->pop();
		if (var->_type == VAL_VARIABLE_REF) {
			var = var->_valRef;
		}

		bool res = STATUS_FAILED;
		bool triedNative = false;

		// we are already calling this method, try native
		if (_thread && _methodThread && strcmp(methodName, _threadEvent) == 0 && var->_type == VAL_NATIVE && _owner == var->getNative()) {
			triedNative = true;
			res = var->_valNative->scCallMethod(this, _stack, _thisStack, methodName);
		}

		if (DID_FAIL(res)) {
			if (var->isNative() && var->getNative()->canHandleMethod(methodName)) {
				if (!_unbreakable) {
					_waitScript = var->getNative()->invokeMethodThread(methodName);
					if (!_waitScript) {
						_stack->correctParams(0);
						runtimeError("Error invoking method '%s'.", methodName);
						_stack->pushNULL();
					} else {
						_state = SCRIPT_WAITING_SCRIPT;
						_waitScript->copyParameters(_stack);
					}
				} else {
					// can call methods in unbreakable mode
					_stack->correctParams(0);
					runtimeError("Cannot call method '%s'. Ignored.", methodName);
					_stack->pushNULL();
				}
				delete[] methodName;
				break;
			}
			/*
			ScValue* val = var->getProp(MethodName);
			if (val){
			    dw = GetFuncPos(val->getString());
			    if (dw==0){
			        TExternalFunction* f = GetExternal(val->getString());
			        if (f){
			            ExternalCall(_stack, _thisStack, f);
			        }
			        else{
			            // not an internal nor external, try for native function
			            _gameRef->ExternalCall(this, _stack, _thisStack, val->getString());
			        }
			    }
			    else{
			        _operand->setInt(_iP);
			        _callStack->Push(_operand);
			        _iP = dw;
			    }
			}
			*/
			else {
				res = STATUS_FAILED;
				if (var->_type == VAL_NATIVE && !triedNative) {
					res = var->_valNative->scCallMethod(this, _stack, _thisStack, methodName);
				}

				if (DID_FAIL(res)) {
					_stack->correctParams(0);
					runtimeError("Call to undefined method '%s'. Ignored.", methodName);
					_stack->pushNULL();
				}
			}
		}
		delete[] methodName;
	}
	break;

	case II_EXTERNAL_CALL: {
		uint32 symbolIndex = getDWORD();

		TExternalFunction *f = getExternal(_symbols[symbolIndex]);
		if (f) {
			externalCall(_stack, _thisStack, f);
		} else {
			_gameRef->externalCall(this, _stack, _thisStack, _symbols[symbolIndex]);
		}

		break;
	}
	case II_SCOPE:
		_operand->setNULL();
		_scopeStack->push(_operand);
		break;

	case II_CORRECT_STACK:
		dw = getDWORD(); // params expected
		_stack->correctParams(dw);
		break;

	case II_CREATE_OBJECT:
		_operand->setObject();
		_stack->push(_operand);
		break;

	case II_POP_EMPTY:
		_stack->pop();
		break;

	case II_PUSH_VAR: {
		ScValue *var = getVar(_symbols[getDWORD()]);
		if (false && /*var->_type==VAL_OBJECT ||*/ var->_type == VAL_NATIVE) {
			_operand->setReference(var);
			_stack->push(_operand);
		} else {
			_stack->push(var);
		}
		break;
	}

	case II_PUSH_VAR_REF: {
		ScValue *var = getVar(_symbols[getDWORD()]);
		_operand->setReference(var);
		_stack->push(_operand);
		break;
	}

	case II_POP_VAR: {
		char *varName = _symbols[getDWORD()];
		ScValue *var = getVar(varName);
		if (var) {
			ScValue *val = _stack->pop();
			if (!val) {
				runtimeError("Script stack corruption detected. Please report this script at WME bug reports forum.");
				var->setNULL();
			} else {
				if (val->getType() == VAL_VARIABLE_REF) {
					val = val->_valRef;
				}
				if (val->_type == VAL_NATIVE) {
					var->setValue(val);
				} else {
					var->copy(val);
				}
			}
		}

		break;
	}

	case II_PUSH_VAR_THIS:
		_stack->push(_thisStack->getTop());
		break;

	case II_PUSH_INT:
		_stack->pushInt((int)getDWORD());
		break;

	case II_PUSH_FLOAT:
		_stack->pushFloat(getFloat());
		break;


	case II_PUSH_BOOL:
		_stack->pushBool(getDWORD() != 0);

		break;

	case II_PUSH_STRING:
		_stack->pushString(getString());
		break;

	case II_PUSH_NULL:
		_stack->pushNULL();
		break;

	case II_PUSH_THIS_FROM_STACK:
		_operand->setReference(_stack->getTop());
		_thisStack->push(_operand);
		break;

	case II_PUSH_THIS:
		_operand->setReference(getVar(_symbols[getDWORD()]));
		_thisStack->push(_operand);
		break;

	case II_POP_THIS:
		_thisStack->pop();
		break;

	case II_PUSH_BY_EXP: {
		str = _stack->pop()->getString();
		ScValue *val = _stack->pop()->getProp(str);
		if (val) {
			_stack->push(val);
		} else {
			_stack->pushNULL();
		}

		break;
	}

	case II_POP_BY_EXP: {
		str = _stack->pop()->getString();
		ScValue *var = _stack->pop();
		ScValue *val = _stack->pop();

		if (val == NULL) {
			runtimeError("Script stack corruption detected. Please report this script at WME bug reports forum.");
			var->setNULL();
		} else {
			var->setProp(str, val);
		}

		break;
	}

	case II_PUSH_REG1:
		_stack->push(_reg1);
		break;

	case II_POP_REG1:
		_reg1->copy(_stack->pop());
		break;

	case II_JMP:
		_iP = getDWORD();
		break;

	case II_JMP_FALSE: {
		dw = getDWORD();
		//if (!_stack->pop()->getBool()) _iP = dw;
		ScValue *val = _stack->pop();
		if (!val) {
			runtimeError("Script corruption detected. Did you use '=' instead of '==' for comparison?");
		} else {
			if (!val->getBool()) {
				_iP = dw;
			}
		}
		break;
	}

	case II_ADD:
		op2 = _stack->pop();
		op1 = _stack->pop();

		if (op1->isNULL() || op2->isNULL()) {
			_operand->setNULL();
		} else if (op1->getType() == VAL_STRING || op2->getType() == VAL_STRING) {
			char *tempStr = new char [strlen(op1->getString()) + strlen(op2->getString()) + 1];
			strcpy(tempStr, op1->getString());
			strcat(tempStr, op2->getString());
			_operand->setString(tempStr);
			delete[] tempStr;
		} else if (op1->getType() == VAL_INT && op2->getType() == VAL_INT) {
			_operand->setInt(op1->getInt() + op2->getInt());
		} else {
			_operand->setFloat(op1->getFloat() + op2->getFloat());
		}

		_stack->push(_operand);

		break;

	case II_SUB:
		op2 = _stack->pop();
		op1 = _stack->pop();

		if (op1->isNULL() || op2->isNULL()) {
			_operand->setNULL();
		} else if (op1->getType() == VAL_INT && op2->getType() == VAL_INT) {
			_operand->setInt(op1->getInt() - op2->getInt());
		} else {
			_operand->setFloat(op1->getFloat() - op2->getFloat());
		}

		_stack->push(_operand);

		break;

	case II_MUL:
		op2 = _stack->pop();
		op1 = _stack->pop();

		if (op1->isNULL() || op2->isNULL()) {
			_operand->setNULL();
		} else if (op1->getType() == VAL_INT && op2->getType() == VAL_INT) {
			_operand->setInt(op1->getInt() * op2->getInt());
		} else {
			_operand->setFloat(op1->getFloat() * op2->getFloat());
		}

		_stack->push(_operand);

		break;

	case II_DIV:
		op2 = _stack->pop();
		op1 = _stack->pop();

		if (op2->getFloat() == 0.0f) {
			runtimeError("Division by zero.");
		}

		if (op1->isNULL() || op2->isNULL() || op2->getFloat() == 0.0f) {
			_operand->setNULL();
		} else {
			_operand->setFloat(op1->getFloat() / op2->getFloat());
		}

		_stack->push(_operand);

		break;

	case II_MODULO:
		op2 = _stack->pop();
		op1 = _stack->pop();

		if (op2->getInt() == 0) {
			runtimeError("Division by zero.");
		}

		if (op1->isNULL() || op2->isNULL() || op2->getInt() == 0) {
			_operand->setNULL();
		} else {
			_operand->setInt(op1->getInt() % op2->getInt());
		}

		_stack->push(_operand);

		break;

	case II_NOT:
		op1 = _stack->pop();
		//if (op1->isNULL()) _operand->setNULL();
		if (op1->isNULL()) {
			_operand->setBool(true);
		} else {
			_operand->setBool(!op1->getBool());
		}
		_stack->push(_operand);

		break;

	case II_AND:
		op2 = _stack->pop();
		op1 = _stack->pop();
		if (op1 == NULL || op2 == NULL) {
			runtimeError("Script corruption detected. Did you use '=' instead of '==' for comparison?");
			_operand->setBool(false);
		} else {
			_operand->setBool(op1->getBool() && op2->getBool());
		}
		_stack->push(_operand);
		break;

	case II_OR:
		op2 = _stack->pop();
		op1 = _stack->pop();
		if (op1 == NULL || op2 == NULL) {
			runtimeError("Script corruption detected. Did you use '=' instead of '==' for comparison?");
			_operand->setBool(false);
		} else {
			_operand->setBool(op1->getBool() || op2->getBool());
		}
		_stack->push(_operand);
		break;

	case II_CMP_EQ:
		op2 = _stack->pop();
		op1 = _stack->pop();

		/*
		if ((op1->isNULL() && !op2->isNULL()) || (!op1->isNULL() && op2->isNULL())) _operand->setBool(false);
		else if (op1->isNative() && op2->isNative()){
		    _operand->setBool(op1->getNative() == op2->getNative());
		}
		else if (op1->getType()==VAL_STRING || op2->getType()==VAL_STRING){
		    _operand->setBool(scumm_stricmp(op1->getString(), op2->getString())==0);
		}
		else if (op1->getType()==VAL_FLOAT && op2->getType()==VAL_FLOAT){
		    _operand->setBool(op1->getFloat() == op2->getFloat());
		}
		else{
		    _operand->setBool(op1->getInt() == op2->getInt());
		}
		*/

		_operand->setBool(ScValue::compare(op1, op2) == 0);
		_stack->push(_operand);
		break;

	case II_CMP_NE:
		op2 = _stack->pop();
		op1 = _stack->pop();

		/*
		if ((op1->isNULL() && !op2->isNULL()) || (!op1->isNULL() && op2->isNULL())) _operand->setBool(true);
		else if (op1->isNative() && op2->isNative()){
		    _operand->setBool(op1->getNative() != op2->getNative());
		}
		else if (op1->getType()==VAL_STRING || op2->getType()==VAL_STRING){
		    _operand->setBool(scumm_stricmp(op1->getString(), op2->getString())!=0);
		}
		else if (op1->getType()==VAL_FLOAT && op2->getType()==VAL_FLOAT){
		    _operand->setBool(op1->getFloat() != op2->getFloat());
		}
		else{
		    _operand->setBool(op1->getInt() != op2->getInt());
		}
		*/

		_operand->setBool(ScValue::compare(op1, op2) != 0);
		_stack->push(_operand);
		break;

	case II_CMP_L:
		op2 = _stack->pop();
		op1 = _stack->pop();

		/*
		if (op1->getType()==VAL_FLOAT && op2->getType()==VAL_FLOAT){
		    _operand->setBool(op1->getFloat() < op2->getFloat());
		}
		else _operand->setBool(op1->getInt() < op2->getInt());
		*/

		_operand->setBool(ScValue::compare(op1, op2) < 0);
		_stack->push(_operand);
		break;

	case II_CMP_G:
		op2 = _stack->pop();
		op1 = _stack->pop();

		/*
		if (op1->getType()==VAL_FLOAT && op2->getType()==VAL_FLOAT){
		    _operand->setBool(op1->getFloat() > op2->getFloat());
		}
		else _operand->setBool(op1->getInt() > op2->getInt());
		*/

		_operand->setBool(ScValue::compare(op1, op2) > 0);
		_stack->push(_operand);
		break;

	case II_CMP_LE:
		op2 = _stack->pop();
		op1 = _stack->pop();

		/*
		if (op1->getType()==VAL_FLOAT && op2->getType()==VAL_FLOAT){
		    _operand->setBool(op1->getFloat() <= op2->getFloat());
		}
		else _operand->setBool(op1->getInt() <= op2->getInt());
		*/

		_operand->setBool(ScValue::compare(op1, op2) <= 0);
		_stack->push(_operand);
		break;

	case II_CMP_GE:
		op2 = _stack->pop();
		op1 = _stack->pop();

		/*
		if (op1->getType()==VAL_FLOAT && op2->getType()==VAL_FLOAT){
		    _operand->setBool(op1->getFloat() >= op2->getFloat());
		}
		else _operand->setBool(op1->getInt() >= op2->getInt());
		*/

		_operand->setBool(ScValue::compare(op1, op2) >= 0);
		_stack->push(_operand);
		break;

	case II_CMP_STRICT_EQ:
		op2 = _stack->pop();
		op1 = _stack->pop();

		//_operand->setBool(op1->getType()==op2->getType() && op1->getFloat()==op2->getFloat());
		_operand->setBool(ScValue::compareStrict(op1, op2) == 0);
		_stack->push(_operand);

		break;

	case II_CMP_STRICT_NE:
		op2 = _stack->pop();
		op1 = _stack->pop();

		//_operand->setBool(op1->getType()!=op2->getType() || op1->getFloat()!=op2->getFloat());
		_operand->setBool(ScValue::compareStrict(op1, op2) != 0);
		_stack->push(_operand);
		break;

	case II_DBG_LINE: {
		int newLine = getDWORD();
		if (newLine != _currentLine) {
			_currentLine = newLine;
		}
		break;

	}
	default:
		_gameRef->LOG(0, "Fatal: Invalid instruction %d ('%s', line %d, IP:0x%x)\n", inst, _filename, _currentLine, _iP - sizeof(uint32));
		_state = SCRIPT_FINISHED;
		ret = STATUS_FAILED;
	} // switch(instruction)

	//delete op;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
uint32 ScScript::getFuncPos(const Common::String &name) {
	for (uint32 i = 0; i < _numFunctions; i++) {
		if (name == _functions[i].name) {
			return _functions[i].pos;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
uint32 ScScript::getMethodPos(const Common::String &name) {
	for (uint32 i = 0; i < _numMethods; i++) {
		if (name == _methods[i].name) {
			return _methods[i].pos;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
ScValue *ScScript::getVar(char *name) {
	ScValue *ret = NULL;

	// scope locals
	if (_scopeStack->_sP >= 0) {
		if (_scopeStack->getTop()->propExists(name)) {
			ret = _scopeStack->getTop()->getProp(name);
		}
	}

	// script globals
	if (ret == NULL) {
		if (_globals->propExists(name)) {
			ret = _globals->getProp(name);
		}
	}

	// engine globals
	if (ret == NULL) {
		if (_engine->_globals->propExists(name)) {
			ret = _engine->_globals->getProp(name);
		}
	}

	if (ret == NULL) {
		//RuntimeError("Variable '%s' is inaccessible in the current block. Consider changing the script.", name);
		_gameRef->LOG(0, "Warning: variable '%s' is inaccessible in the current block. Consider changing the script (script:%s, line:%d)", name, _filename, _currentLine);
		ScValue *val = new ScValue(_gameRef);
		ScValue *scope = _scopeStack->getTop();
		if (scope) {
			scope->setProp(name, val);
			ret = _scopeStack->getTop()->getProp(name);
		} else {
			_globals->setProp(name, val);
			ret = _globals->getProp(name);
		}
		delete val;
	}

	return ret;
}


//////////////////////////////////////////////////////////////////////////
bool ScScript::waitFor(BaseObject *object) {
	if (_unbreakable) {
		runtimeError("Script cannot be interrupted.");
		return STATUS_OK;
	}

	_state = SCRIPT_WAITING;
	_waitObject = object;
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool ScScript::waitForExclusive(BaseObject *object) {
	_engine->resetObject(object);
	return waitFor(object);
}


//////////////////////////////////////////////////////////////////////////
bool ScScript::sleep(uint32 duration) {
	if (_unbreakable) {
		runtimeError("Script cannot be interrupted.");
		return STATUS_OK;
	}

	_state = SCRIPT_SLEEPING;
	if (_gameRef->_state == GAME_FROZEN) {
		_waitTime = g_system->getMillis() + duration;
		_waitFrozen = true;
	} else {
		_waitTime = _gameRef->_timer + duration;
		_waitFrozen = false;
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool ScScript::finish(bool includingThreads) {
	if (_state != SCRIPT_FINISHED && includingThreads) {
		_state = SCRIPT_FINISHED;
		finishThreads();
	} else {
		_state = SCRIPT_FINISHED;
	}


	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool ScScript::run() {
	_state = SCRIPT_RUNNING;
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////
void ScScript::runtimeError(const char *fmt, ...) {
	char buff[256];
	va_list va;

	va_start(va, fmt);
	vsprintf(buff, fmt, va);
	va_end(va);

	_gameRef->LOG(0, "Runtime error. Script '%s', line %d", _filename, _currentLine);
	_gameRef->LOG(0, "  %s", buff);

	if (!_gameRef->_suppressScriptErrors) {
		_gameRef->quickMessage("Script runtime error. View log for details.");
	}
}


//////////////////////////////////////////////////////////////////////////
bool ScScript::persist(BasePersistenceManager *persistMgr) {

	persistMgr->transfer(TMEMBER(_gameRef));

	// buffer
	if (persistMgr->getIsSaving()) {
		if (_state != SCRIPT_PERSISTENT && _state != SCRIPT_FINISHED && _state != SCRIPT_THREAD_FINISHED) {
			persistMgr->transfer(TMEMBER(_bufferSize));
			persistMgr->putBytes(_buffer, _bufferSize);
		} else {
			// don't save idle/finished scripts
			int bufferSize = 0;
			persistMgr->transfer(TMEMBER(bufferSize));
		}
	} else {
		persistMgr->transfer(TMEMBER(_bufferSize));
		if (_bufferSize > 0) {
			_buffer = new byte[_bufferSize];
			persistMgr->getBytes(_buffer, _bufferSize);
			_scriptStream = new Common::MemoryReadStream(_buffer, _bufferSize);
			initTables();
		} else {
			_buffer = NULL;
			_scriptStream = NULL;
		}
	}

	persistMgr->transfer(TMEMBER(_callStack));
	persistMgr->transfer(TMEMBER(_currentLine));
	persistMgr->transfer(TMEMBER(_engine));
	persistMgr->transfer(TMEMBER(_filename));
	persistMgr->transfer(TMEMBER(_freezable));
	persistMgr->transfer(TMEMBER(_globals));
	persistMgr->transfer(TMEMBER(_iP));
	persistMgr->transfer(TMEMBER(_scopeStack));
	persistMgr->transfer(TMEMBER(_stack));
	persistMgr->transfer(TMEMBER_INT(_state));
	persistMgr->transfer(TMEMBER(_operand));
	persistMgr->transfer(TMEMBER_INT(_origState));
	persistMgr->transfer(TMEMBER(_owner));
	persistMgr->transfer(TMEMBER(_reg1));
	persistMgr->transfer(TMEMBER(_thread));
	persistMgr->transfer(TMEMBER(_threadEvent));
	persistMgr->transfer(TMEMBER(_thisStack));
	persistMgr->transfer(TMEMBER(_timeSlice));
	persistMgr->transfer(TMEMBER(_waitObject));
	persistMgr->transfer(TMEMBER(_waitScript));
	persistMgr->transfer(TMEMBER(_waitTime));
	persistMgr->transfer(TMEMBER(_waitFrozen));

	persistMgr->transfer(TMEMBER(_methodThread));
	persistMgr->transfer(TMEMBER(_methodThread));
	persistMgr->transfer(TMEMBER(_unbreakable));
	persistMgr->transfer(TMEMBER(_parentScript));

	if (!persistMgr->getIsSaving()) {
		_tracingMode = false;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ScScript *ScScript::invokeEventHandler(const Common::String &eventName, bool unbreakable) {
	//if (_state!=SCRIPT_PERSISTENT) return NULL;

	uint32 pos = getEventPos(eventName);
	if (!pos) {
		return NULL;
	}

	ScScript *thread = new ScScript(_gameRef,  _engine);
	if (thread) {
		bool ret = thread->createThread(this, pos, eventName);
		if (DID_SUCCEED(ret)) {
			thread->_unbreakable = unbreakable;
			_engine->_scripts.add(thread);
			return thread;
		} else {
			delete thread;
			return NULL;
		}
	} else {
		return NULL;
	}

}


//////////////////////////////////////////////////////////////////////////
uint32 ScScript::getEventPos(const Common::String &name) {
	for (int i = _numEvents - 1; i >= 0; i--) {
		if (scumm_stricmp(name.c_str(), _events[i].name) == 0) {
			return _events[i].pos;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
bool ScScript::canHandleEvent(const Common::String &eventName) {
	return getEventPos(eventName) != 0;
}


//////////////////////////////////////////////////////////////////////////
bool ScScript::canHandleMethod(const Common::String &methodName) {
	return getMethodPos(methodName) != 0;
}


//////////////////////////////////////////////////////////////////////////
bool ScScript::pause() {
	if (_state == SCRIPT_PAUSED) {
		_gameRef->LOG(0, "Attempting to pause a paused script ('%s', line %d)", _filename, _currentLine);
		return STATUS_FAILED;
	}

	if (!_freezable || _state == SCRIPT_PERSISTENT) {
		return STATUS_OK;
	}

	_origState = _state;
	_state = SCRIPT_PAUSED;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool ScScript::resume() {
	if (_state != SCRIPT_PAUSED) {
		return STATUS_OK;
	}

	_state = _origState;
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
ScScript::TExternalFunction *ScScript::getExternal(char *name) {
	for (uint32 i = 0; i < _numExternals; i++) {
		if (strcmp(name, _externals[i].name) == 0) {
			return &_externals[i];
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
bool ScScript::externalCall(ScStack *stack, ScStack *thisStack, ScScript::TExternalFunction *function) {

	_gameRef->LOG(0, "External functions are not supported on this platform.");
	stack->correctParams(0);
	stack->pushNULL();
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool ScScript::copyParameters(ScStack *stack) {
	int i;
	int numParams = stack->pop()->getInt();
	for (i = numParams - 1; i >= 0; i--) {
		_stack->push(stack->getAt(i));
	}
	_stack->pushInt(numParams);

	for (i = 0; i < numParams; i++) {
		stack->pop();
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool ScScript::finishThreads() {
	for (uint32 i = 0; i < _engine->_scripts.size(); i++) {
		ScScript *scr = _engine->_scripts[i];
		if (scr->_thread && scr->_state != SCRIPT_FINISHED && scr->_owner == _owner && scumm_stricmp(scr->_filename, _filename) == 0) {
			scr->finish(true);
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
// IWmeDebugScript interface implementation
int ScScript::dbgGetLine() {
	return _currentLine;
}

//////////////////////////////////////////////////////////////////////////
const char *ScScript::dbgGetFilename() {
	return _filename;
}

//////////////////////////////////////////////////////////////////////////
void ScScript::afterLoad() {
	if (_buffer == NULL) {
		byte *buffer = _engine->getCompiledScript(_filename, &_bufferSize);
		if (!buffer) {
			_gameRef->LOG(0, "Error reinitializing script '%s' after load. Script will be terminated.", _filename);
			_state = SCRIPT_ERROR;
			return;
		}

		_buffer = new byte [_bufferSize];
		memcpy(_buffer, buffer, _bufferSize);

		delete _scriptStream;
		_scriptStream = new Common::MemoryReadStream(_buffer, _bufferSize);

		initTables();
	}
}

} // end of namespace Wintermute
