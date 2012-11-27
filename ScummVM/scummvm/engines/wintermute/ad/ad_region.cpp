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

#include "engines/wintermute/ad/ad_region.h"
#include "engines/wintermute/base/base_dynamic_buffer.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdRegion, false)

//////////////////////////////////////////////////////////////////////////
AdRegion::AdRegion(BaseGame *inGame) : BaseRegion(inGame) {
	_blocked = false;
	_decoration = false;
	_zoom = 0;
	_alpha = 0xFFFFFFFF;
}


//////////////////////////////////////////////////////////////////////////
AdRegion::~AdRegion() {
}


//////////////////////////////////////////////////////////////////////////
bool AdRegion::loadFile(const char *filename) {
	byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == NULL) {
		_gameRef->LOG(0, "AdRegion::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) {
		_gameRef->LOG(0, "Error parsing REGION file '%s'", filename);
	}


	delete[] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(REGION)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(NAME)
TOKEN_DEF(ACTIVE)
TOKEN_DEF(ZOOM)
TOKEN_DEF(SCALE)
TOKEN_DEF(BLOCKED)
TOKEN_DEF(DECORATION)
TOKEN_DEF(POINT)
TOKEN_DEF(ALPHA_COLOR)
TOKEN_DEF(ALPHA)
TOKEN_DEF(EDITOR_SELECTED_POINT)
TOKEN_DEF(EDITOR_SELECTED)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(CAPTION)
TOKEN_DEF(PROPERTY)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool AdRegion::loadBuffer(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(REGION)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(ACTIVE)
	TOKEN_TABLE(ZOOM)
	TOKEN_TABLE(SCALE)
	TOKEN_TABLE(BLOCKED)
	TOKEN_TABLE(DECORATION)
	TOKEN_TABLE(POINT)
	TOKEN_TABLE(ALPHA_COLOR)
	TOKEN_TABLE(ALPHA)
	TOKEN_TABLE(EDITOR_SELECTED_POINT)
	TOKEN_TABLE(EDITOR_SELECTED)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(PROPERTY)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	BaseParser parser;

	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_REGION) {
			_gameRef->LOG(0, "'REGION' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	for (uint32 i = 0; i < _points.size(); i++) {
		delete _points[i];
	}
	_points.clear();

	int ar = 255, ag = 255, ab = 255, alpha = 255;

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (DID_FAIL(loadFile((char *)params))) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_CAPTION:
			setCaption((char *)params);
			break;

		case TOKEN_ACTIVE:
			parser.scanStr((char *)params, "%b", &_active);
			break;

		case TOKEN_BLOCKED:
			parser.scanStr((char *)params, "%b", &_blocked);
			break;

		case TOKEN_DECORATION:
			parser.scanStr((char *)params, "%b", &_decoration);
			break;

		case TOKEN_ZOOM:
		case TOKEN_SCALE: {
			int j;
			parser.scanStr((char *)params, "%d", &j);
			_zoom = (float)j;
		}
		break;

		case TOKEN_POINT: {
			int x, y;
			parser.scanStr((char *)params, "%d,%d", &x, &y);
			_points.add(new BasePoint(x, y));
		}
		break;

		case TOKEN_ALPHA_COLOR:
			parser.scanStr((char *)params, "%d,%d,%d", &ar, &ag, &ab);
			break;

		case TOKEN_ALPHA:
			parser.scanStr((char *)params, "%d", &alpha);
			break;

		case TOKEN_EDITOR_SELECTED:
			parser.scanStr((char *)params, "%b", &_editorSelected);
			break;

		case TOKEN_EDITOR_SELECTED_POINT:
			parser.scanStr((char *)params, "%d", &_editorSelectedPoint);
			break;

		case TOKEN_SCRIPT:
			addScript((char *)params);
			break;

		case TOKEN_PROPERTY:
			parseProperty(params, false);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in REGION definition");
		return STATUS_FAILED;
	}

	createRegion();

	_alpha = BYTETORGBA(ar, ag, ab, alpha);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool AdRegion::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	/*
	    //////////////////////////////////////////////////////////////////////////
	    // SkipTo
	    //////////////////////////////////////////////////////////////////////////
	    if (strcmp(name, "SkipTo")==0) {
	        stack->correctParams(2);
	        _posX = stack->pop()->getInt();
	        _posY = stack->pop()->getInt();
	        stack->pushNULL();

	        return STATUS_OK;
	    }

	    else*/ return BaseRegion::scCallMethod(script, stack, thisStack, name);
}


//////////////////////////////////////////////////////////////////////////
ScValue *AdRegion::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("ad region");
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
	// Blocked
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Blocked") {
		_scValue->setBool(_blocked);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Decoration
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Decoration") {
		_scValue->setBool(_decoration);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Scale
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Scale") {
		_scValue->setFloat(_zoom);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AlphaColor
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AlphaColor") {
		_scValue->setInt((int)_alpha);
		return _scValue;
	} else {
		return BaseRegion::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool AdRegion::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Name") == 0) {
		setName(value->getString());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Blocked
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Blocked") == 0) {
		_blocked = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Decoration
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Decoration") == 0) {
		_decoration = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Scale
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Scale") == 0) {
		_zoom = value->getFloat();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AlphaColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AlphaColor") == 0) {
		_alpha = (uint32)value->getInt();
		return STATUS_OK;
	} else {
		return BaseRegion::scSetProperty(name, value);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *AdRegion::scToString() {
	return "[ad region]";
}


//////////////////////////////////////////////////////////////////////////
bool AdRegion::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "REGION {\n");
	buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", getName());
	buffer->putTextIndent(indent + 2, "CAPTION=\"%s\"\n", getCaption());
	buffer->putTextIndent(indent + 2, "BLOCKED=%s\n", _blocked ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "DECORATION=%s\n", _decoration ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "ACTIVE=%s\n", _active ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "SCALE=%d\n", (int)_zoom);
	buffer->putTextIndent(indent + 2, "ALPHA_COLOR { %d,%d,%d }\n", RGBCOLGetR(_alpha), RGBCOLGetG(_alpha), RGBCOLGetB(_alpha));
	buffer->putTextIndent(indent + 2, "ALPHA = %d\n", RGBCOLGetA(_alpha));
	buffer->putTextIndent(indent + 2, "EDITOR_SELECTED=%s\n", _editorSelected ? "TRUE" : "FALSE");

	for (uint32 i = 0; i < _scripts.size(); i++) {
		buffer->putTextIndent(indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	if (_scProp) {
		_scProp->saveAsText(buffer, indent + 2);
	}

	for (uint32 i = 0; i < _points.size(); i++) {
		buffer->putTextIndent(indent + 2, "POINT {%d,%d}\n", _points[i]->x, _points[i]->y);
	}

	BaseClass::saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent, "}\n\n");

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdRegion::persist(BasePersistenceManager *persistMgr) {
	BaseRegion::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_alpha));
	persistMgr->transfer(TMEMBER(_blocked));
	persistMgr->transfer(TMEMBER(_decoration));
	persistMgr->transfer(TMEMBER(_zoom));

	return STATUS_OK;
}

} // end of namespace Wintermute
