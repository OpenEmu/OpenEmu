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

#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/base_game.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(ScStack, false)

//////////////////////////////////////////////////////////////////////////
ScStack::ScStack(BaseGame *inGame) : BaseClass(inGame) {
	_sP = -1;
}


//////////////////////////////////////////////////////////////////////////
ScStack::~ScStack() {

#if _DEBUG
	//_gameRef->LOG(0, "STAT: Stack size: %d, SP=%d", _values.size(), _sP);
#endif

	for (uint32 i = 0; i < _values.size(); i++) {
		delete _values[i];
	}
	_values.clear();
}


//////////////////////////////////////////////////////////////////////////
ScValue *ScStack::pop() {
	if (_sP < 0) {
		_gameRef->LOG(0, "Fatal: Stack underflow");
		return NULL;
	}

	return _values[_sP--];
}


//////////////////////////////////////////////////////////////////////////
void ScStack::push(ScValue *val) {
	_sP++;

	if (_sP < (int32)_values.size()) {
		_values[_sP]->cleanup();
		_values[_sP]->copy(val);
	} else {
		ScValue *copyVal = new ScValue(_gameRef);
		copyVal->copy(val);
		_values.add(copyVal);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *ScStack::getPushValue() {
	_sP++;

	if (_sP >= (int32)_values.size()) {
		ScValue *val = new ScValue(_gameRef);
		_values.add(val);
	}
	_values[_sP]->cleanup();
	return _values[_sP];
}



//////////////////////////////////////////////////////////////////////////
ScValue *ScStack::getTop() {
	if (_sP < 0 || _sP >= (int32)_values.size()) {
		return NULL;
	} else {
		return _values[_sP];
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *ScStack::getAt(int index) {
	index = _sP - index;
	if (index < 0 || index >= (int32)_values.size()) {
		return NULL;
	} else {
		return _values[index];
	}
}


//////////////////////////////////////////////////////////////////////////
void ScStack::correctParams(uint32 expectedParams) {
	uint32 nuParams = (uint32)pop()->getInt();

	if (expectedParams < nuParams) { // too many params
		while (expectedParams < nuParams) {
			//Pop();
			delete _values[_sP - expectedParams];
			_values.remove_at(_sP - expectedParams);
			nuParams--;
			_sP--;
		}
	} else if (expectedParams > nuParams) { // need more params
		while (expectedParams > nuParams) {
			//Push(null_val);
			ScValue *nullVal = new ScValue(_gameRef);
			nullVal->setNULL();
			_values.insert_at(_sP - nuParams + 1, nullVal);
			nuParams++;
			_sP++;

			if ((int32)_values.size() > _sP + 1) {
				delete _values[_values.size() - 1];
				_values.remove_at(_values.size() - 1);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
void ScStack::pushNULL() {
	/*
	ScValue* val = new ScValue(_gameRef);
	val->setNULL();
	Push(val);
	delete val;
	*/
	getPushValue()->setNULL();
}


//////////////////////////////////////////////////////////////////////////
void ScStack::pushInt(int val) {
	/*
	ScValue* val = new ScValue(_gameRef);
	val->setInt(Val);
	Push(val);
	delete val;
	*/
	getPushValue()->setInt(val);
}


//////////////////////////////////////////////////////////////////////////
void ScStack::pushFloat(double val) {
	/*
	ScValue* val = new ScValue(_gameRef);
	val->setFloat(Val);
	Push(val);
	delete val;
	*/
	getPushValue()->setFloat(val);
}


//////////////////////////////////////////////////////////////////////////
void ScStack::pushBool(bool val) {
	/*
	ScValue* val = new ScValue(_gameRef);
	val->setBool(Val);
	Push(val);
	delete val;
	*/
	getPushValue()->setBool(val);
}


//////////////////////////////////////////////////////////////////////////
void ScStack::pushString(const char *val) {
	/*
	ScValue* val = new ScValue(_gameRef);
	val->setString(Val);
	Push(val);
	delete val;
	*/
	getPushValue()->setString(val);
}


//////////////////////////////////////////////////////////////////////////
void ScStack::pushNative(BaseScriptable *val, bool persistent) {
	/*
	ScValue* val = new ScValue(_gameRef);
	val->setNative(Val, Persistent);
	Push(val);
	delete val;
	*/

	getPushValue()->setNative(val, persistent);
}


//////////////////////////////////////////////////////////////////////////
bool ScStack::persist(BasePersistenceManager *persistMgr) {

	persistMgr->transfer(TMEMBER(_gameRef));

	persistMgr->transfer(TMEMBER(_sP));
	_values.persist(persistMgr);

	return STATUS_OK;
}

} // end of namespace Wintermute
