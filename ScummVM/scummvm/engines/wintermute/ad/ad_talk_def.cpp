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

#include "engines/wintermute/ad/ad_sprite_set.h"
#include "engines/wintermute/ad/ad_talk_def.h"
#include "engines/wintermute/ad/ad_talk_node.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_dynamic_buffer.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/utils/utils.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdTalkDef, false)

//////////////////////////////////////////////////////////////////////////
AdTalkDef::AdTalkDef(BaseGame *inGame) : BaseObject(inGame) {
	_defaultSpriteFilename = NULL;
	_defaultSprite = NULL;

	_defaultSpriteSetFilename = NULL;
	_defaultSpriteSet = NULL;
}


//////////////////////////////////////////////////////////////////////////
AdTalkDef::~AdTalkDef() {
	for (uint32 i = 0; i < _nodes.size(); i++) {
		delete _nodes[i];
	}
	_nodes.clear();

	delete[] _defaultSpriteFilename;
	delete _defaultSprite;
	_defaultSpriteFilename = NULL;
	_defaultSprite = NULL;

	delete[] _defaultSpriteSetFilename;
	delete _defaultSpriteSet;
	_defaultSpriteSetFilename = NULL;
	_defaultSpriteSet = NULL;
}


//////////////////////////////////////////////////////////////////////////
bool AdTalkDef::loadFile(const char *filename) {
	byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == NULL) {
		_gameRef->LOG(0, "AdTalkDef::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) {
		_gameRef->LOG(0, "Error parsing TALK file '%s'", filename);
	}

	delete[] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(TALK)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(ACTION)
TOKEN_DEF(DEFAULT_SPRITESET_FILE)
TOKEN_DEF(DEFAULT_SPRITESET)
TOKEN_DEF(DEFAULT_SPRITE)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool AdTalkDef::loadBuffer(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(TALK)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(ACTION)
	TOKEN_TABLE(DEFAULT_SPRITESET_FILE)
	TOKEN_TABLE(DEFAULT_SPRITESET)
	TOKEN_TABLE(DEFAULT_SPRITE)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	BaseParser parser;

	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_TALK) {
			_gameRef->LOG(0, "'TALK' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (DID_FAIL(loadFile((char *)params))) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_ACTION: {
			AdTalkNode *node = new AdTalkNode(_gameRef);
			if (node && DID_SUCCEED(node->loadBuffer(params, false))) {
				_nodes.add(node);
			} else {
				delete node;
				node = NULL;
				cmd = PARSERR_GENERIC;
			}
		}
		break;

		case TOKEN_DEFAULT_SPRITE:
			BaseUtils::setString(&_defaultSpriteFilename, (char *)params);
			break;

		case TOKEN_DEFAULT_SPRITESET_FILE:
			BaseUtils::setString(&_defaultSpriteSetFilename, (char *)params);
			break;

		case TOKEN_DEFAULT_SPRITESET: {
			delete _defaultSpriteSet;
			_defaultSpriteSet = new AdSpriteSet(_gameRef);
			if (!_defaultSpriteSet || DID_FAIL(_defaultSpriteSet->loadBuffer(params, false))) {
				delete _defaultSpriteSet;
				_defaultSpriteSet = NULL;
				cmd = PARSERR_GENERIC;
			}
		}
		break;


		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in TALK definition");
		return STATUS_FAILED;
	}

	if (cmd == PARSERR_GENERIC) {
		_gameRef->LOG(0, "Error loading TALK definition");
		return STATUS_FAILED;
	}

	delete _defaultSprite;
	delete _defaultSpriteSet;
	_defaultSprite = NULL;
	_defaultSpriteSet = NULL;

	if (_defaultSpriteFilename) {
		_defaultSprite = new BaseSprite(_gameRef);
		if (!_defaultSprite || DID_FAIL(_defaultSprite->loadFile(_defaultSpriteFilename))) {
			return STATUS_FAILED;
		}
	}

	if (_defaultSpriteSetFilename) {
		_defaultSpriteSet = new AdSpriteSet(_gameRef);
		if (!_defaultSpriteSet || DID_FAIL(_defaultSpriteSet->loadFile(_defaultSpriteSetFilename))) {
			return STATUS_FAILED;
		}
	}


	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdTalkDef::persist(BasePersistenceManager *persistMgr) {

	BaseObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_defaultSprite));
	persistMgr->transfer(TMEMBER(_defaultSpriteFilename));
	persistMgr->transfer(TMEMBER(_defaultSpriteSet));
	persistMgr->transfer(TMEMBER(_defaultSpriteSetFilename));

	_nodes.persist(persistMgr);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdTalkDef::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "TALK {\n");
	if (_defaultSpriteFilename) {
		buffer->putTextIndent(indent + 2, "DEFAULT_SPRITE=\"%s\"\n", _defaultSpriteFilename);
	}

	if (_defaultSpriteSetFilename) {
		buffer->putTextIndent(indent + 2, "DEFAULT_SPRITESET_FILE=\"%s\"\n", _defaultSpriteSetFilename);
	} else if (_defaultSpriteSet) {
		_defaultSpriteSet->saveAsText(buffer, indent + 2);
	}

	for (uint32 i = 0; i < _nodes.size(); i++) {
		_nodes[i]->saveAsText(buffer, indent + 2);
		buffer->putTextIndent(indent, "\n");
	}
	BaseClass::saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent, "}\n");

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdTalkDef::loadDefaultSprite() {
	if (_defaultSpriteFilename && !_defaultSprite) {
		_defaultSprite = new BaseSprite(_gameRef);
		if (!_defaultSprite || DID_FAIL(_defaultSprite->loadFile(_defaultSpriteFilename))) {
			delete _defaultSprite;
			_defaultSprite = NULL;
			return STATUS_FAILED;
		} else {
			return STATUS_OK;
		}
	} else if (_defaultSpriteSetFilename && !_defaultSpriteSet) {
		_defaultSpriteSet = new AdSpriteSet(_gameRef);
		if (!_defaultSpriteSet || DID_FAIL(_defaultSpriteSet->loadFile(_defaultSpriteSetFilename))) {
			delete _defaultSpriteSet;
			_defaultSpriteSet = NULL;
			return STATUS_FAILED;
		} else {
			return STATUS_OK;
		}
	} else {
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
BaseSprite *AdTalkDef::getDefaultSprite(TDirection dir) {
	loadDefaultSprite();
	if (_defaultSprite) {
		return _defaultSprite;
	} else if (_defaultSpriteSet) {
		return _defaultSpriteSet->getSprite(dir);
	} else {
		return NULL;
	}
}

} // end of namespace Wintermute
