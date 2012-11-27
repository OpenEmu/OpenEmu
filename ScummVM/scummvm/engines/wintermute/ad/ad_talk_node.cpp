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
#include "engines/wintermute/ad/ad_talk_node.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_dynamic_buffer.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/utils/utils.h"
namespace Wintermute {

IMPLEMENT_PERSISTENT(AdTalkNode, false)

//////////////////////////////////////////////////////////////////////////
AdTalkNode::AdTalkNode(BaseGame *inGame) : BaseClass(inGame) {
	_sprite = NULL;
	_spriteFilename = NULL;
	_spriteSet = NULL;
	_spriteSetFilename = NULL;
	_comment = NULL;

	_startTime = _endTime = 0;
	_playToEnd = false;
	_preCache = false;
}


//////////////////////////////////////////////////////////////////////////
AdTalkNode::~AdTalkNode() {
	delete[] _spriteFilename;
	delete _sprite;
	delete[] _spriteSetFilename;
	delete _spriteSet;
	delete _comment;
	_spriteFilename = NULL;
	_sprite = NULL;
	_spriteSetFilename = NULL;
	_spriteSet = NULL;
	_comment = NULL;
}



TOKEN_DEF_START
TOKEN_DEF(ACTION)
TOKEN_DEF(SPRITESET_FILE)
TOKEN_DEF(SPRITESET)
TOKEN_DEF(SPRITE)
TOKEN_DEF(START_TIME)
TOKEN_DEF(END_TIME)
TOKEN_DEF(COMMENT)
TOKEN_DEF(PRECACHE)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool AdTalkNode::loadBuffer(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(ACTION)
	TOKEN_TABLE(SPRITESET_FILE)
	TOKEN_TABLE(SPRITESET)
	TOKEN_TABLE(SPRITE)
	TOKEN_TABLE(START_TIME)
	TOKEN_TABLE(END_TIME)
	TOKEN_TABLE(COMMENT)
	TOKEN_TABLE(PRECACHE)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	BaseParser parser;

	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_ACTION) {
			_gameRef->LOG(0, "'ACTION' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	_endTime = 0;
	_playToEnd = false;
	_preCache = false;

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_SPRITE:
			BaseUtils::setString(&_spriteFilename, (char *)params);
			break;

		case TOKEN_SPRITESET_FILE:
			BaseUtils::setString(&_spriteSetFilename, (char *)params);
			break;

		case TOKEN_SPRITESET: {
			delete _spriteSet;
			_spriteSet = new AdSpriteSet(_gameRef);
			if (!_spriteSet || DID_FAIL(_spriteSet->loadBuffer(params, false))) {
				delete _spriteSet;
				_spriteSet = NULL;
				cmd = PARSERR_GENERIC;
			}
		}
		break;

		case TOKEN_START_TIME:
			parser.scanStr((char *)params, "%d", &_startTime);
			break;

		case TOKEN_END_TIME:
			parser.scanStr((char *)params, "%d", &_endTime);
			break;

		case TOKEN_PRECACHE:
			parser.scanStr((char *)params, "%b", &_preCache);
			break;

		case TOKEN_COMMENT:
			if (_gameRef->_editorMode) {
				BaseUtils::setString(&_comment, (char *)params);
			}
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in ACTION definition");
		return STATUS_FAILED;
	}

	if (cmd == PARSERR_GENERIC) {
		_gameRef->LOG(0, "Error loading ACTION definition");
		return STATUS_FAILED;
	}

	if (_endTime == 0) {
		_playToEnd = true;
	} else {
		_playToEnd = false;
	}

	if (_preCache && _spriteFilename) {
		delete _sprite;
		_sprite = new BaseSprite(_gameRef);
		if (!_sprite || DID_FAIL(_sprite->loadFile(_spriteFilename))) {
			return STATUS_FAILED;
		}
	}

	if (_preCache && _spriteSetFilename) {
		delete _spriteSet;
		_spriteSet = new AdSpriteSet(_gameRef);
		if (!_spriteSet || DID_FAIL(_spriteSet->loadFile(_spriteSetFilename))) {
			return STATUS_FAILED;
		}
	}

	return STATUS_OK;
}



//////////////////////////////////////////////////////////////////////////
bool AdTalkNode::persist(BasePersistenceManager *persistMgr) {
	persistMgr->transfer(TMEMBER(_comment));
	persistMgr->transfer(TMEMBER(_startTime));
	persistMgr->transfer(TMEMBER(_endTime));
	persistMgr->transfer(TMEMBER(_playToEnd));
	persistMgr->transfer(TMEMBER(_sprite));
	persistMgr->transfer(TMEMBER(_spriteFilename));
	persistMgr->transfer(TMEMBER(_spriteSet));
	persistMgr->transfer(TMEMBER(_spriteSetFilename));

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdTalkNode::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "ACTION {\n");
	if (_comment) {
		buffer->putTextIndent(indent + 2, "COMMENT=\"%s\"\n", _comment);
	}
	buffer->putTextIndent(indent + 2, "START_TIME=%d\n", _startTime);
	if (!_playToEnd) {
		buffer->putTextIndent(indent + 2, "END_TIME=%d\n", _endTime);
	}
	if (_spriteFilename) {
		buffer->putTextIndent(indent + 2, "SPRITE=\"%s\"\n", _spriteFilename);
	}
	if (_spriteSetFilename) {
		buffer->putTextIndent(indent + 2, "SPRITESET_FILE=\"%s\"\n", _spriteSetFilename);
	} else if (_spriteSet) {
		_spriteSet->saveAsText(buffer, indent + 2);
	}
	if (_preCache) {
		buffer->putTextIndent(indent + 2, "PRECACHE=\"%s\"\n", _preCache ? "TRUE" : "FALSE");
	}

	BaseClass::saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent, "}\n");

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdTalkNode::loadSprite() {
	if (_spriteFilename && !_sprite) {
		_sprite = new BaseSprite(_gameRef);
		if (!_sprite || DID_FAIL(_sprite->loadFile(_spriteFilename))) {
			delete _sprite;
			_sprite = NULL;
			return STATUS_FAILED;
		} else {
			return STATUS_OK;
		}
	} else if (_spriteSetFilename && !_spriteSet) {
		_spriteSet = new AdSpriteSet(_gameRef);
		if (!_spriteSet || DID_FAIL(_spriteSet->loadFile(_spriteSetFilename))) {
			delete _spriteSet;
			_spriteSet = NULL;
			return STATUS_FAILED;
		} else {
			return STATUS_OK;
		}
	} else {
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
bool AdTalkNode::isInTimeInterval(uint32 time, TDirection dir) {
	if (time >= _startTime) {
		if (_playToEnd) {
			if ((_spriteFilename && _sprite == NULL) || (_sprite && _sprite->isFinished() == false)) {
				return true;
			} else if ((_spriteSetFilename && _spriteSet == NULL) || (_spriteSet && _spriteSet->getSprite(dir) && _spriteSet->getSprite(dir)->isFinished() == false)) {
				return true;
			} else {
				return false;
			}
		} else {
			return _endTime >= time;
		}
	} else {
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
BaseSprite *AdTalkNode::getSprite(TDirection dir) {
	loadSprite();
	if (_sprite) {
		return _sprite;
	} else if (_spriteSet) {
		return _spriteSet->getSprite(dir);
	} else {
		return NULL;
	}
}

} // end of namespace Wintermute
