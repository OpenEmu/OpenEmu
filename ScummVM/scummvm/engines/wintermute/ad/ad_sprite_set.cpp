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
#include "engines/wintermute/base/base_dynamic_buffer.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_sprite.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdSpriteSet, false)

//////////////////////////////////////////////////////////////////////////
AdSpriteSet::AdSpriteSet(BaseGame *inGame, BaseObject *owner) : BaseObject(inGame) {
	_owner = owner;

	for (int i = 0; i < NUM_DIRECTIONS; i++) {
		_sprites[i] = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
AdSpriteSet::~AdSpriteSet() {
	for (int i = 0; i < NUM_DIRECTIONS; i++) {
		delete _sprites[i];
		_sprites[i] = NULL;
	}

	_owner = NULL;
}


//////////////////////////////////////////////////////////////////////////
bool AdSpriteSet::loadFile(const char *filename, int lifeTime, TSpriteCacheType cacheType) {
	byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == NULL) {
		_gameRef->LOG(0, "AdSpriteSet::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	if (DID_FAIL(ret = loadBuffer(buffer, true))) {
		_gameRef->LOG(0, "Error parsing SPRITESET file '%s'", filename);
	}

	delete[] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(SPRITESET)
TOKEN_DEF(NAME)
TOKEN_DEF(UP_LEFT)
TOKEN_DEF(DOWN_LEFT)
TOKEN_DEF(LEFT)
TOKEN_DEF(UP_RIGHT)
TOKEN_DEF(DOWN_RIGHT)
TOKEN_DEF(RIGHT)
TOKEN_DEF(UP)
TOKEN_DEF(DOWN)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool AdSpriteSet::loadBuffer(byte *buffer, bool complete, int lifeTime, TSpriteCacheType cacheType) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(SPRITESET)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(UP_LEFT)
	TOKEN_TABLE(DOWN_LEFT)
	TOKEN_TABLE(LEFT)
	TOKEN_TABLE(UP_RIGHT)
	TOKEN_TABLE(DOWN_RIGHT)
	TOKEN_TABLE(RIGHT)
	TOKEN_TABLE(UP)
	TOKEN_TABLE(DOWN)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	BaseParser parser;

	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_SPRITESET) {
			_gameRef->LOG(0, "'SPRITESET' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	BaseSprite *spr = NULL;
	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (DID_FAIL(loadFile((char *)params, lifeTime, cacheType))) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_LEFT:
			delete _sprites[DI_LEFT];
			_sprites[DI_LEFT] = NULL;
			spr = new BaseSprite(_gameRef,  _owner);
			if (!spr || DID_FAIL(spr->loadFile((char *)params, lifeTime, cacheType))) {
				cmd = PARSERR_GENERIC;
			} else {
				_sprites[DI_LEFT] = spr;
			}
			break;

		case TOKEN_RIGHT:
			delete _sprites[DI_RIGHT];
			_sprites[DI_RIGHT] = NULL;
			spr = new BaseSprite(_gameRef,  _owner);
			if (!spr || DID_FAIL(spr->loadFile((char *)params, lifeTime, cacheType))) {
				cmd = PARSERR_GENERIC;
			} else {
				_sprites[DI_RIGHT] = spr;
			}
			break;

		case TOKEN_UP:
			delete _sprites[DI_UP];
			_sprites[DI_UP] = NULL;
			spr = new BaseSprite(_gameRef,  _owner);
			if (!spr || DID_FAIL(spr->loadFile((char *)params, lifeTime, cacheType))) {
				cmd = PARSERR_GENERIC;
			} else {
				_sprites[DI_UP] = spr;
			}
			break;

		case TOKEN_DOWN:
			delete _sprites[DI_DOWN];
			_sprites[DI_DOWN] = NULL;
			spr = new BaseSprite(_gameRef,  _owner);
			if (!spr || DID_FAIL(spr->loadFile((char *)params, lifeTime, cacheType))) {
				cmd = PARSERR_GENERIC;
			} else {
				_sprites[DI_DOWN] = spr;
			}
			break;

		case TOKEN_UP_LEFT:
			delete _sprites[DI_UPLEFT];
			_sprites[DI_UPLEFT] = NULL;
			spr = new BaseSprite(_gameRef,  _owner);
			if (!spr || DID_FAIL(spr->loadFile((char *)params, lifeTime, cacheType))) {
				cmd = PARSERR_GENERIC;
			} else {
				_sprites[DI_UPLEFT] = spr;
			}
			break;

		case TOKEN_UP_RIGHT:
			delete _sprites[DI_UPRIGHT];
			_sprites[DI_UPRIGHT] = NULL;
			spr = new BaseSprite(_gameRef,  _owner);
			if (!spr || DID_FAIL(spr->loadFile((char *)params, lifeTime, cacheType))) {
				cmd = PARSERR_GENERIC;
			} else {
				_sprites[DI_UPRIGHT] = spr;
			}
			break;

		case TOKEN_DOWN_LEFT:
			delete _sprites[DI_DOWNLEFT];
			_sprites[DI_DOWNLEFT] = NULL;
			spr = new BaseSprite(_gameRef,  _owner);
			if (!spr || DID_FAIL(spr->loadFile((char *)params, lifeTime, cacheType))) {
				cmd = PARSERR_GENERIC;
			} else {
				_sprites[DI_DOWNLEFT] = spr;
			}
			break;

		case TOKEN_DOWN_RIGHT:
			delete _sprites[DI_DOWNRIGHT];
			_sprites[DI_DOWNRIGHT] = NULL;
			spr = new BaseSprite(_gameRef,  _owner);
			if (!spr || DID_FAIL(spr->loadFile((char *)params, lifeTime, cacheType))) {
				cmd = PARSERR_GENERIC;
			} else {
				_sprites[DI_DOWNRIGHT] = spr;
			}
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in SPRITESET definition");
		return STATUS_FAILED;
	}

	if (cmd == PARSERR_GENERIC) {
		_gameRef->LOG(0, "Error loading SPRITESET definition");
		if (spr) {
			delete spr;
		}
		return STATUS_FAILED;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdSpriteSet::persist(BasePersistenceManager *persistMgr) {

	BaseObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_owner));
	for (int i = 0; i < NUM_DIRECTIONS; i++) {
		persistMgr->transfer("", &_sprites[i]);
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
BaseSprite *AdSpriteSet::getSprite(TDirection direction) {
	int dir = (int)direction;
	if (dir < 0) {
		dir = 0;
	}
	if (dir >= NUM_DIRECTIONS) {
		dir = NUM_DIRECTIONS - 1;
	}

	BaseSprite *ret = NULL;

	// find nearest set sprite
	int numSteps = 0;
	for (int i = dir; i >= 0; i--) {
		if (_sprites[i] != NULL) {
			ret = _sprites[i];
			numSteps = dir - i;
			break;
		}
	}

	for (int i = dir; i < NUM_DIRECTIONS; i++) {
		if (_sprites[i] != NULL) {
			if (ret == NULL || numSteps > i - dir) {
				return _sprites[i];
			} else {
				return ret;
			}
		}
	}

	return ret;
}



//////////////////////////////////////////////////////////////////////////
bool AdSpriteSet::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "SPRITESET {\n");
	if (getName()) {
		buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", getName());
	}
	for (int i = 0; i < NUM_DIRECTIONS; i++) {
		if (_sprites[i]) {
			switch (i) {
			case DI_UP:
				buffer->putTextIndent(indent + 2, "UP=\"%s\"\n",         _sprites[i]->getFilename());
				break;
			case DI_UPRIGHT:
				buffer->putTextIndent(indent + 2, "UP_RIGHT=\"%s\"\n",   _sprites[i]->getFilename());
				break;
			case DI_RIGHT:
				buffer->putTextIndent(indent + 2, "RIGHT=\"%s\"\n",      _sprites[i]->getFilename());
				break;
			case DI_DOWNRIGHT:
				buffer->putTextIndent(indent + 2, "DOWN_RIGHT=\"%s\"\n", _sprites[i]->getFilename());
				break;
			case DI_DOWN:
				buffer->putTextIndent(indent + 2, "DOWN=\"%s\"\n",       _sprites[i]->getFilename());
				break;
			case DI_DOWNLEFT:
				buffer->putTextIndent(indent + 2, "DOWN_LEFT=\"%s\"\n",  _sprites[i]->getFilename());
				break;
			case DI_LEFT:
				buffer->putTextIndent(indent + 2, "LEFT=\"%s\"\n",       _sprites[i]->getFilename());
				break;
			case DI_UPLEFT:
				buffer->putTextIndent(indent + 2, "UP_LEFT=\"%s\"\n",    _sprites[i]->getFilename());
				break;
			}
		}
	}

	BaseClass::saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent, "}\n");

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdSpriteSet::containsSprite(BaseSprite *sprite) {
	if (!sprite) {
		return false;
	}

	for (int i = 0; i < NUM_DIRECTIONS; i++) {
		if (_sprites[i] == sprite) {
			return true;
		}
	}
	return false;
}

} // end of namespace Wintermute
