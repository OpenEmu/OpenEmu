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

#include "engines/wintermute/base/base_scriptable.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/base_persistence_manager.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(BaseScriptable, false)

//////////////////////////////////////////////////////////////////////////
BaseScriptable::BaseScriptable(BaseGame *inGame, bool noValue, bool persistable) : BaseNamedObject(inGame) {
	_refCount = 0;

	if (noValue) {
		_scValue = NULL;
	} else {
		_scValue = new ScValue(_gameRef);
	}

	_persistable = persistable;

	_scProp = NULL;
}


//////////////////////////////////////////////////////////////////////////
BaseScriptable::~BaseScriptable() {
	//if (_refCount>0) _gameRef->LOG(0, "Warning: Destroying object, _refCount=%d", _refCount);
	delete _scValue;
	delete _scProp;
	_scValue = NULL;
	_scProp = NULL;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool BaseScriptable::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	/*
	stack->correctParams(0);
	stack->pushNULL();
	script->runtimeError("Call to undefined method '%s'.", name);

	return STATUS_OK;
	*/
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
ScValue *BaseScriptable::scGetProperty(const Common::String &name) {
	if (!_scProp) {
		_scProp = new ScValue(_gameRef);
	}
	if (_scProp) {
		return _scProp->getProp(name.c_str()); // TODO: Change to Common::String
	} else {
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseScriptable::scSetProperty(const char *name, ScValue *value) {
	if (!_scProp) {
		_scProp = new ScValue(_gameRef);
	}
	if (_scProp) {
		return _scProp->setProp(name, value);
	} else {
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
const char *BaseScriptable::scToString() {
	return "[native object]";
}

//////////////////////////////////////////////////////////////////////////
void *BaseScriptable::scToMemBuffer() {
	return (void *)NULL;
}


//////////////////////////////////////////////////////////////////////////
int BaseScriptable::scToInt() {
	return 0;
}


//////////////////////////////////////////////////////////////////////////
double BaseScriptable::scToFloat() {
	return 0.0f;
}


//////////////////////////////////////////////////////////////////////////
bool BaseScriptable::scToBool() {
	return false;
}


//////////////////////////////////////////////////////////////////////////
void BaseScriptable::scSetString(const char *val) {
}


//////////////////////////////////////////////////////////////////////////
void BaseScriptable::scSetInt(int val) {
}


//////////////////////////////////////////////////////////////////////////
void BaseScriptable::scSetFloat(double val) {
}


//////////////////////////////////////////////////////////////////////////
void BaseScriptable::scSetBool(bool val) {
}


//////////////////////////////////////////////////////////////////////////
bool BaseScriptable::persist(BasePersistenceManager *persistMgr) {
	persistMgr->transfer(TMEMBER(_gameRef));
	persistMgr->transfer(TMEMBER(_refCount));
	persistMgr->transfer(TMEMBER(_scProp));
	persistMgr->transfer(TMEMBER(_scValue));

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
int BaseScriptable::scCompare(BaseScriptable *val) {
	if (this < val) {
		return -1;
	} else if (this > val) {
		return 1;
	} else {
		return 0;
	}
}

//////////////////////////////////////////////////////////////////////////
void BaseScriptable::scDebuggerDesc(char *buf, int bufSize) {
	strcpy(buf, scToString());
}

//////////////////////////////////////////////////////////////////////////
bool BaseScriptable::canHandleMethod(const char *eventMethod) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
ScScript *BaseScriptable::invokeMethodThread(const char *methodName) {
	return NULL;
}

} // end of namespace Wintermute
