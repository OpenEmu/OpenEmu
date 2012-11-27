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

#include "engines/wintermute/ad/ad_game.h"
#include "engines/wintermute/base/base_script_holder.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_engine.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_stack.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(BaseScriptHolder, false)

//////////////////////////////////////////////////////////////////////
BaseScriptHolder::BaseScriptHolder(BaseGame *inGame) : BaseScriptable(inGame) {
	setName("<unnamed>");

	_freezable = true;
	_filename = NULL;
}


//////////////////////////////////////////////////////////////////////
BaseScriptHolder::~BaseScriptHolder() {
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
bool BaseScriptHolder::cleanup() {
	delete[] _filename;
	_filename = NULL;

	for (uint32 i = 0; i < _scripts.size(); i++) {
		_scripts[i]->finish(true);
		_scripts[i]->_owner = NULL;
	}
	_scripts.clear();

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
void BaseScriptHolder::setFilename(const char *filename) {
	if (_filename != NULL) {
		delete[] _filename;
		_filename = NULL;
	}
	if (filename == NULL) {
		return;
	}
	_filename = new char [strlen(filename) + 1];
	if (_filename != NULL) {
		strcpy(_filename, filename);
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseScriptHolder::applyEvent(const char *eventName, bool unbreakable) {
	int numHandlers = 0;

	bool ret = STATUS_FAILED;
	for (uint32 i = 0; i < _scripts.size(); i++) {
		if (!_scripts[i]->_thread) {
			ScScript *handler = _scripts[i]->invokeEventHandler(eventName, unbreakable);
			if (handler) {
				//_scripts.add(handler);
				numHandlers++;
				ret = STATUS_OK;
			}
		}
	}
	if (numHandlers > 0 && unbreakable) {
		_gameRef->_scEngine->tickUnbreakable();
	}

	return ret;
}


//////////////////////////////////////////////////////////////////////////
bool BaseScriptHolder::listen(BaseScriptHolder *param1, uint32 param2) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool BaseScriptHolder::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// DEBUG_CrashMe
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "DEBUG_CrashMe") == 0) {
		stack->correctParams(0);
		byte *p = 0;
		*p = 10;
		stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ApplyEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ApplyEvent") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		bool ret;
		ret = applyEvent(val->getString());

		if (DID_SUCCEED(ret)) {
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CanHandleEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CanHandleEvent") == 0) {
		stack->correctParams(1);
		stack->pushBool(canHandleEvent(stack->pop()->getString()));

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CanHandleMethod
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CanHandleMethod") == 0) {
		stack->correctParams(1);
		stack->pushBool(canHandleMethod(stack->pop()->getString()));

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AttachScript
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AttachScript") == 0) {
		stack->correctParams(1);
		stack->pushBool(DID_SUCCEED(addScript(stack->pop()->getString())));

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DetachScript
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DetachScript") == 0) {
		stack->correctParams(2);
		const char *filename = stack->pop()->getString();
		bool killThreads = stack->pop()->getBool(false);
		bool ret = false;
		for (uint32 i = 0; i < _scripts.size(); i++) {
			if (scumm_stricmp(_scripts[i]->_filename, filename) == 0) {
				_scripts[i]->finish(killThreads);
				ret = true;
				break;
			}
		}
		stack->pushBool(ret);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsScriptRunning
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsScriptRunning") == 0) {
		stack->correctParams(1);
		const char *filename = stack->pop()->getString();
		bool ret = false;
		for (uint32 i = 0; i < _scripts.size(); i++) {
			if (scumm_stricmp(_scripts[i]->_filename, filename) == 0 && _scripts[i]->_state != SCRIPT_FINISHED && _scripts[i]->_state != SCRIPT_ERROR) {
				ret = true;
				break;
			}
		}
		stack->pushBool(ret);

		return STATUS_OK;
	} else {
		return BaseScriptable::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *BaseScriptHolder::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("script_holder");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Name") {
		_scValue->setString(getName());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Filename (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Filename") {
		_scValue->setString(_filename);
		return _scValue;
	} else {
		return BaseScriptable::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseScriptHolder::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Name") == 0) {
		setName(value->getString());
		return STATUS_OK;
	} else {
		return BaseScriptable::scSetProperty(name, value);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *BaseScriptHolder::scToString() {
	return "[script_holder]";
}

//////////////////////////////////////////////////////////////////////////
bool BaseScriptHolder::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	return BaseClass::saveAsText(buffer, indent);
}


//////////////////////////////////////////////////////////////////////////
bool BaseScriptHolder::persist(BasePersistenceManager *persistMgr) {
	BaseScriptable::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_filename));
	persistMgr->transfer(TMEMBER(_freezable));
	if (persistMgr->getIsSaving()) {
		const char *name = getName();
		persistMgr->transfer(TMEMBER(name));
	} else {
		char *name;
		persistMgr->transfer(TMEMBER(name));
		setName(name);
		delete[] name;
	}
	_scripts.persist(persistMgr);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseScriptHolder::addScript(const char *filename) {
	for (uint32 i = 0; i < _scripts.size(); i++) {
		if (scumm_stricmp(_scripts[i]->_filename, filename) == 0) {
			if (_scripts[i]->_state != SCRIPT_FINISHED) {
				_gameRef->LOG(0, "BaseScriptHolder::AddScript - trying to add script '%s' mutiple times (obj: '%s')", filename, getName());
				return STATUS_OK;
			}
		}
	}

	ScScript *scr =  _gameRef->_scEngine->runScript(filename, this);
	if (!scr) {
		if (_gameRef->_editorForceScripts) {
			// editor hack
			scr = new ScScript(_gameRef,  _gameRef->_scEngine);
			scr->_filename = new char[strlen(filename) + 1];
			strcpy(scr->_filename, filename);
			scr->_state = SCRIPT_ERROR;
			scr->_owner = this;
			_scripts.add(scr);
			_gameRef->_scEngine->_scripts.add(scr);

			return STATUS_OK;
		}
		return STATUS_FAILED;
	} else {
		scr->_freezable = _freezable;
		_scripts.add(scr);
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseScriptHolder::removeScript(ScScript *script) {
	for (uint32 i = 0; i < _scripts.size(); i++) {
		if (_scripts[i] == script) {
			_scripts.remove_at(i);
			break;
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseScriptHolder::canHandleEvent(const char *EventName) {
	for (uint32 i = 0; i < _scripts.size(); i++) {
		if (!_scripts[i]->_thread && _scripts[i]->canHandleEvent(EventName)) {
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool BaseScriptHolder::canHandleMethod(const char *MethodName) {
	for (uint32 i = 0; i < _scripts.size(); i++) {
		if (!_scripts[i]->_thread && _scripts[i]->canHandleMethod(MethodName)) {
			return true;
		}
	}
	return false;
}


TOKEN_DEF_START
TOKEN_DEF(PROPERTY)
TOKEN_DEF(NAME)
TOKEN_DEF(VALUE)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool BaseScriptHolder::parseProperty(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(PROPERTY)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(VALUE)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	BaseParser parser;

	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_PROPERTY) {
			_gameRef->LOG(0, "'PROPERTY' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	char *propName = NULL;
	char *propValue = NULL;

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_NAME:
			delete[] propName;
			propName = new char[strlen((char *)params) + 1];
			if (propName) {
				strcpy(propName, (char *)params);
			} else {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_VALUE:
			delete[] propValue;
			propValue = new char[strlen((char *)params) + 1];
			if (propValue) {
				strcpy(propValue, (char *)params);
			} else {
				cmd = PARSERR_GENERIC;
			}
			break;
		}

	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		delete[] propName;
		delete[] propValue;
		propName = NULL;
		propValue = NULL;
		_gameRef->LOG(0, "Syntax error in PROPERTY definition");
		return STATUS_FAILED;
	}
	if (cmd == PARSERR_GENERIC || propName == NULL || propValue == NULL) {
		delete[] propName;
		delete[] propValue;
		propName = NULL;
		propValue = NULL;
		_gameRef->LOG(0, "Error loading PROPERTY definition");
		return STATUS_FAILED;
	}


	ScValue *val = new ScValue(_gameRef);
	val->setString(propValue);
	scSetProperty(propName, val);

	delete val;
	delete[] propName;
	delete[] propValue;
	propName = NULL;
	propValue = NULL;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void BaseScriptHolder::makeFreezable(bool freezable) {
	_freezable = freezable;
	for (uint32 i = 0; i < _scripts.size(); i++) {
		_scripts[i]->_freezable = freezable;
	}

}


//////////////////////////////////////////////////////////////////////////
ScScript *BaseScriptHolder::invokeMethodThread(const char *methodName) {
	for (int i = _scripts.size() - 1; i >= 0; i--) {
		if (_scripts[i]->canHandleMethod(methodName)) {

			ScScript *thread = new ScScript(_gameRef,  _scripts[i]->_engine);
			if (thread) {
				bool ret = thread->createMethodThread(_scripts[i], methodName);
				if (DID_SUCCEED(ret)) {
					_scripts[i]->_engine->_scripts.add(thread);
					return thread;
				} else {
					delete thread;
				}
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
void BaseScriptHolder::scDebuggerDesc(char *buf, int bufSize) {
	strcpy(buf, scToString());
	if (getName() && strcmp(getName(), "<unnamed>") != 0) {
		strcat(buf, "  Name: ");
		strcat(buf, getName());
	}
	if (_filename) {
		strcat(buf, "  File: ");
		strcat(buf, _filename);
	}
}


//////////////////////////////////////////////////////////////////////////
// IWmeObject
//////////////////////////////////////////////////////////////////////////
bool BaseScriptHolder::sendEvent(const char *eventName) {
	return DID_SUCCEED(applyEvent(eventName));
}

} // end of namespace Wintermute
