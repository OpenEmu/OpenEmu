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

#include "engines/wintermute/ad/ad_entity.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/ui/ui_entity.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_dynamic_buffer.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_stack.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(UIEntity, false)

//////////////////////////////////////////////////////////////////////////
UIEntity::UIEntity(BaseGame *inGame) : UIObject(inGame) {
	_type = UI_CUSTOM;
	_entity = NULL;
}


//////////////////////////////////////////////////////////////////////////
UIEntity::~UIEntity() {
	if (_entity) {
		_gameRef->unregisterObject(_entity);
	}
	_entity = NULL;
}


//////////////////////////////////////////////////////////////////////////
bool UIEntity::loadFile(const char *filename) {
	byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == NULL) {
		_gameRef->LOG(0, "UIEntity::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) {
		_gameRef->LOG(0, "Error parsing ENTITY container file '%s'", filename);
	}


	delete[] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(ENTITY_CONTAINER)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(DISABLED)
TOKEN_DEF(VISIBLE)
TOKEN_DEF(X)
TOKEN_DEF(Y)
TOKEN_DEF(NAME)
TOKEN_DEF(ENTITY)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool UIEntity::loadBuffer(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(ENTITY_CONTAINER)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(DISABLED)
	TOKEN_TABLE(VISIBLE)
	TOKEN_TABLE(X)
	TOKEN_TABLE(Y)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(ENTITY)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd = 2;
	BaseParser parser;

	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_ENTITY_CONTAINER) {
			_gameRef->LOG(0, "'ENTITY_CONTAINER' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	while (cmd > 0 && (cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (DID_FAIL(loadFile((char *)params))) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_X:
			parser.scanStr((char *)params, "%d", &_posX);
			break;

		case TOKEN_Y:
			parser.scanStr((char *)params, "%d", &_posY);
			break;

		case TOKEN_DISABLED:
			parser.scanStr((char *)params, "%b", &_disable);
			break;

		case TOKEN_VISIBLE:
			parser.scanStr((char *)params, "%b", &_visible);
			break;

		case TOKEN_ENTITY:
			if (DID_FAIL(setEntity((char *)params))) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_SCRIPT:
			addScript((char *)params);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in ENTITY_CONTAINER definition");
		return STATUS_FAILED;
	}
	if (cmd == PARSERR_GENERIC) {
		_gameRef->LOG(0, "Error loading ENTITY_CONTAINER definition");
		return STATUS_FAILED;
	}

	correctSize();

	if (_gameRef->_editorMode) {
		_width = 50;
		_height = 50;
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool UIEntity::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "ENTITY_CONTAINER\n");
	buffer->putTextIndent(indent, "{\n");

	buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", getName());

	buffer->putTextIndent(indent + 2, "\n");

	buffer->putTextIndent(indent + 2, "X=%d\n", _posX);
	buffer->putTextIndent(indent + 2, "Y=%d\n", _posY);

	buffer->putTextIndent(indent + 2, "DISABLED=%s\n", _disable ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "VISIBLE=%s\n", _visible ? "TRUE" : "FALSE");

	if (_entity && _entity->getFilename()) {
		buffer->putTextIndent(indent + 2, "ENTITY=\"%s\"\n", _entity->getFilename());
	}

	buffer->putTextIndent(indent + 2, "\n");

	// scripts
	for (uint32 i = 0; i < _scripts.size(); i++) {
		buffer->putTextIndent(indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	buffer->putTextIndent(indent + 2, "\n");

	// editor properties
	BaseClass::saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent, "}\n");
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool UIEntity::setEntity(const char *filename) {
	if (_entity) {
		_gameRef->unregisterObject(_entity);
	}
	_entity = new AdEntity(_gameRef);
	if (!_entity || DID_FAIL(_entity->loadFile(filename))) {
		delete _entity;
		_entity = NULL;
		return STATUS_FAILED;
	} else {
		_entity->_nonIntMouseEvents = true;
		_entity->_sceneIndependent = true;
		_entity->makeFreezable(false);
		_gameRef->registerObject(_entity);
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool UIEntity::display(int offsetX, int offsetY) {
	if (!_visible) {
		return STATUS_OK;
	}

	if (_entity) {
		_entity->_posX = offsetX + _posX;
		_entity->_posY = offsetY + _posY;
		if (_entity->_scale < 0) {
			_entity->_zoomable = false;
		}
		_entity->_shadowable = false;

		_entity->update();

		bool origReg = _entity->_registrable;

		if (_entity->_registrable && _disable) {
			_entity->_registrable = false;
		}

		_entity->display();
		_entity->_registrable = origReg;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool UIEntity::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// GetEntity
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "GetEntity") == 0) {
		stack->correctParams(0);

		if (_entity) {
			stack->pushNative(_entity, true);
		} else {
			stack->pushNULL();
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetEntity") == 0) {
		stack->correctParams(1);

		const char *filename = stack->pop()->getString();

		if (DID_SUCCEED(setEntity(filename))) {
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	} else {
		return UIObject::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *UIEntity::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("entity container");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Freezable
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Freezable") {
		if (_entity) {
			_scValue->setBool(_entity->_freezable);
		} else {
			_scValue->setBool(false);
		}
		return _scValue;
	} else {
		return UIObject::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool UIEntity::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Freezable
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Freezable") == 0) {
		if (_entity) {
			_entity->makeFreezable(value->getBool());
		}
		return STATUS_OK;
	} else {
		return UIObject::scSetProperty(name, value);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *UIEntity::scToString() {
	return "[entity container]";
}


//////////////////////////////////////////////////////////////////////////
bool UIEntity::persist(BasePersistenceManager *persistMgr) {

	UIObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_entity));
	return STATUS_OK;
}

} // end of namespace Wintermute
