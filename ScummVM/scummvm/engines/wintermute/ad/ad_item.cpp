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

#include "engines/wintermute/ad/ad_item.h"
#include "engines/wintermute/ad/ad_game.h"
#include "engines/wintermute/ad/ad_sentence.h"
#include "engines/wintermute/base/font/base_font_storage.h"
#include "engines/wintermute/base/font/base_font.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/sound/base_sound.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/platform_osystem.h"
#include "common/str.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdItem, false)

//////////////////////////////////////////////////////////////////////////
AdItem::AdItem(BaseGame *inGame) : AdTalkHolder(inGame) {
	_spriteHover = NULL;
	_cursorNormal = _cursorHover = NULL;

	_cursorCombined = true;
	_inInventory = false;

	_displayAmount = false;
	_amount = 0;
	_amountOffsetX = 0;
	_amountOffsetY = 0;
	_amountAlign = TAL_RIGHT;
	_amountString = NULL;

	_state = STATE_READY;

	_movable = false;
}


//////////////////////////////////////////////////////////////////////////
AdItem::~AdItem() {
	delete _spriteHover;
	delete _cursorNormal;
	delete _cursorHover;
	_spriteHover = NULL;
	_cursorNormal = NULL;
	_cursorHover = NULL;

	delete[] _amountString;
	_amountString = NULL;
}


//////////////////////////////////////////////////////////////////////////
bool AdItem::loadFile(const char *filename) {
	byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == NULL) {
		_gameRef->LOG(0, "AdItem::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) {
		_gameRef->LOG(0, "Error parsing ITEM file '%s'", filename);
	}


	delete[] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(ITEM)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(CURSOR_HOVER)
TOKEN_DEF(CURSOR_COMBINED)
TOKEN_DEF(CURSOR)
TOKEN_DEF(NAME)
TOKEN_DEF(IMAGE_HOVER)
TOKEN_DEF(IMAGE)
TOKEN_DEF(EVENTS)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(CAPTION)
TOKEN_DEF(PROPERTY)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF(FONT)
TOKEN_DEF(ALPHA_COLOR)
TOKEN_DEF(ALPHA)
TOKEN_DEF(TALK_SPECIAL)
TOKEN_DEF(TALK)
TOKEN_DEF(SPRITE_HOVER)
TOKEN_DEF(SPRITE)
TOKEN_DEF(DISPLAY_AMOUNT)
TOKEN_DEF(AMOUNT_OFFSET_X)
TOKEN_DEF(AMOUNT_OFFSET_Y)
TOKEN_DEF(AMOUNT_ALIGN)
TOKEN_DEF(AMOUNT_STRING)
TOKEN_DEF(AMOUNT)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool AdItem::loadBuffer(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(ITEM)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(CURSOR_HOVER)
	TOKEN_TABLE(CURSOR_COMBINED)
	TOKEN_TABLE(CURSOR)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(IMAGE_HOVER)
	TOKEN_TABLE(IMAGE)
	TOKEN_TABLE(EVENTS)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(PROPERTY)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE(FONT)
	TOKEN_TABLE(ALPHA_COLOR)
	TOKEN_TABLE(ALPHA)
	TOKEN_TABLE(TALK_SPECIAL)
	TOKEN_TABLE(TALK)
	TOKEN_TABLE(SPRITE_HOVER)
	TOKEN_TABLE(SPRITE)
	TOKEN_TABLE(DISPLAY_AMOUNT)
	TOKEN_TABLE(AMOUNT_OFFSET_X)
	TOKEN_TABLE(AMOUNT_OFFSET_Y)
	TOKEN_TABLE(AMOUNT_ALIGN)
	TOKEN_TABLE(AMOUNT_STRING)
	TOKEN_TABLE(AMOUNT)
	TOKEN_TABLE_END

	byte *params;
	int cmd = 2;
	BaseParser parser;

	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_ITEM) {
			_gameRef->LOG(0, "'ITEM' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	int ar = 0, ag = 0, ab = 0, alpha = 255;
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

		case TOKEN_FONT:
			setFont((char *)params);
			break;

		case TOKEN_CAPTION:
			setCaption((char *)params);
			break;

		case TOKEN_IMAGE:
		case TOKEN_SPRITE:
			delete _sprite;
			_sprite = new BaseSprite(_gameRef, this);
			if (!_sprite || DID_FAIL(_sprite->loadFile((char *)params, ((AdGame *)_gameRef)->_texItemLifeTime))) {
				delete _sprite;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE_HOVER:
		case TOKEN_SPRITE_HOVER:
			delete _spriteHover;
			_spriteHover = new BaseSprite(_gameRef, this);
			if (!_spriteHover || DID_FAIL(_spriteHover->loadFile((char *)params, ((AdGame *)_gameRef)->_texItemLifeTime))) {
				delete _spriteHover;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_AMOUNT:
			parser.scanStr((char *)params, "%d", &_amount);
			break;

		case TOKEN_DISPLAY_AMOUNT:
			parser.scanStr((char *)params, "%b", &_displayAmount);
			break;

		case TOKEN_AMOUNT_OFFSET_X:
			parser.scanStr((char *)params, "%d", &_amountOffsetX);
			break;

		case TOKEN_AMOUNT_OFFSET_Y:
			parser.scanStr((char *)params, "%d", &_amountOffsetY);
			break;

		case TOKEN_AMOUNT_ALIGN:
			if (scumm_stricmp((char *)params, "left") == 0) {
				_amountAlign = TAL_LEFT;
			} else if (scumm_stricmp((char *)params, "right") == 0) {
				_amountAlign = TAL_RIGHT;
			} else {
				_amountAlign = TAL_CENTER;
			}
			break;

		case TOKEN_AMOUNT_STRING:
			BaseUtils::setString(&_amountString, (char *)params);
			break;

		case TOKEN_TALK: {
			BaseSprite *spr = new BaseSprite(_gameRef, this);
			if (!spr || DID_FAIL(spr->loadFile((char *)params, ((AdGame *)_gameRef)->_texTalkLifeTime))) {
				cmd = PARSERR_GENERIC;
			} else {
				_talkSprites.add(spr);
			}
		}
		break;

		case TOKEN_TALK_SPECIAL: {
			BaseSprite *spr = new BaseSprite(_gameRef, this);
			if (!spr || DID_FAIL(spr->loadFile((char *)params, ((AdGame *)_gameRef)->_texTalkLifeTime))) {
				cmd = PARSERR_GENERIC;
			} else {
				_talkSpritesEx.add(spr);
			}
		}
		break;

		case TOKEN_CURSOR:
			delete _cursorNormal;
			_cursorNormal = new BaseSprite(_gameRef);
			if (!_cursorNormal || DID_FAIL(_cursorNormal->loadFile((char *)params, ((AdGame *)_gameRef)->_texItemLifeTime))) {
				delete _cursorNormal;
				_cursorNormal = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_CURSOR_HOVER:
			delete _cursorHover;
			_cursorHover = new BaseSprite(_gameRef);
			if (!_cursorHover || DID_FAIL(_cursorHover->loadFile((char *)params, ((AdGame *)_gameRef)->_texItemLifeTime))) {
				delete _cursorHover;
				_cursorHover = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_CURSOR_COMBINED:
			parser.scanStr((char *)params, "%b", &_cursorCombined);
			break;

		case TOKEN_SCRIPT:
			addScript((char *)params);
			break;

		case TOKEN_PROPERTY:
			parseProperty(params, false);
			break;

		case TOKEN_ALPHA_COLOR:
			parser.scanStr((char *)params, "%d,%d,%d", &ar, &ag, &ab);
			break;

		case TOKEN_ALPHA:
			parser.scanStr((char *)params, "%d", &alpha);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in ITEM definition");
		return STATUS_FAILED;
	}
	if (cmd == PARSERR_GENERIC) {
		_gameRef->LOG(0, "Error loading ITEM definition");
		return STATUS_FAILED;
	}

	if (alpha != 0 && ar == 0 && ag == 0 && ab == 0) {
		ar = ag = ab = 255;
	}
	_alphaColor = BYTETORGBA(ar, ag, ab, alpha);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdItem::update() {
	_currentSprite = NULL;

	if (_state == STATE_READY && _animSprite) {
		delete _animSprite;
		_animSprite = NULL;
	}

	// finished playing animation?
	if (_state == STATE_PLAYING_ANIM && _animSprite != NULL && _animSprite->isFinished()) {
		_state = STATE_READY;
		_currentSprite = _animSprite;
	}

	if (_sentence && _state != STATE_TALKING) {
		_sentence->finish();
	}

	// default: stand animation
	if (!_currentSprite) {
		_currentSprite = _sprite;
	}

	switch (_state) {
		//////////////////////////////////////////////////////////////////////////
	case STATE_PLAYING_ANIM:
		_currentSprite = _animSprite;
		break;

		//////////////////////////////////////////////////////////////////////////
	case STATE_READY:
		if (!_animSprite) {
			if (_gameRef->_activeObject == this && _spriteHover) {
				_currentSprite = _spriteHover;
			} else {
				_currentSprite = _sprite;
			}
		}
		break;

		//////////////////////////////////////////////////////////////////////////
	case STATE_TALKING: {
		_sentence->update();
		if (_sentence->_currentSprite) {
			_tempSprite2 = _sentence->_currentSprite;
		}

		bool timeIsUp = (_sentence->_sound && _sentence->_soundStarted && (!_sentence->_sound->isPlaying() && !_sentence->_sound->isPaused())) || (!_sentence->_sound && _sentence->_duration <= _gameRef->_timer - _sentence->_startTime);
		if (_tempSprite2 == NULL || _tempSprite2->isFinished() || (/*_tempSprite2->_looping &&*/ timeIsUp)) {
			if (timeIsUp) {
				_sentence->finish();
				_tempSprite2 = NULL;
				_state = STATE_READY;
			} else {
				_tempSprite2 = getTalkStance(_sentence->getNextStance());
				if (_tempSprite2) {
					_tempSprite2->reset();
					_currentSprite = _tempSprite2;
				}
				((AdGame *)_gameRef)->addSentence(_sentence);
			}
		} else {
			_currentSprite = _tempSprite2;
			((AdGame *)_gameRef)->addSentence(_sentence);
		}
	}
	default:
		break;
	}
	_ready = (_state == STATE_READY);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdItem::display(int x, int y) {
	int width = 0;
	if (_currentSprite) {
		Rect32 rc;
		_currentSprite->getBoundingRect(&rc, 0, 0);
		width = rc.width();
	}

	_posX = x + width / 2;
	_posY = y;

	bool ret;
	if (_currentSprite) {
		ret = _currentSprite->draw(x, y, this, 100, 100, _alphaColor);
	} else {
		ret = STATUS_OK;
	}

	if (_displayAmount) {
		int amountX = x;
		int amountY = y + _amountOffsetY;

		if (_amountAlign == TAL_RIGHT) {
			width -= _amountOffsetX;
			amountX -= _amountOffsetX;
		}
		amountX += _amountOffsetX;

		BaseFont *font = _font ? _font : _gameRef->_systemFont;
		if (font) {
			if (_amountString) {
				font->drawText((byte *)_amountString, amountX, amountY, width, _amountAlign);
			} else {
				char str[256];
				sprintf(str, "%d", _amount);
				font->drawText((byte *)str, amountX, amountY, width, _amountAlign);
			}
		}
	}

	return ret;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool AdItem::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// SetHoverSprite
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetHoverSprite") == 0) {
		stack->correctParams(1);

		bool setCurrent = false;
		if (_currentSprite && _currentSprite == _spriteHover) {
			setCurrent = true;
		}

		const char *filename = stack->pop()->getString();

		delete _spriteHover;
		_spriteHover = NULL;
		BaseSprite *spr = new BaseSprite(_gameRef, this);
		if (!spr || DID_FAIL(spr->loadFile(filename))) {
			stack->pushBool(false);
			script->runtimeError("Item.SetHoverSprite failed for file '%s'", filename);
		} else {
			_spriteHover = spr;
			if (setCurrent) {
				_currentSprite = _spriteHover;
			}
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverSprite
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetHoverSprite") == 0) {
		stack->correctParams(0);

		if (!_spriteHover || !_spriteHover->getFilename()) {
			stack->pushNULL();
		} else {
			stack->pushString(_spriteHover->getFilename());
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverSpriteObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetHoverSpriteObject") == 0) {
		stack->correctParams(0);
		if (!_spriteHover) {
			stack->pushNULL();
		} else {
			stack->pushNative(_spriteHover, true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetNormalCursor
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetNormalCursor") == 0) {
		stack->correctParams(1);

		const char *filename = stack->pop()->getString();

		delete _cursorNormal;
		_cursorNormal = NULL;
		BaseSprite *spr = new BaseSprite(_gameRef);
		if (!spr || DID_FAIL(spr->loadFile(filename))) {
			stack->pushBool(false);
			script->runtimeError("Item.SetNormalCursor failed for file '%s'", filename);
		} else {
			_cursorNormal = spr;
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetNormalCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetNormalCursor") == 0) {
		stack->correctParams(0);

		if (!_cursorNormal || !_cursorNormal->getFilename()) {
			stack->pushNULL();
		} else {
			stack->pushString(_cursorNormal->getFilename());
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetNormalCursorObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetNormalCursorObject") == 0) {
		stack->correctParams(0);

		if (!_cursorNormal) {
			stack->pushNULL();
		} else {
			stack->pushNative(_cursorNormal, true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetHoverCursor
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetHoverCursor") == 0) {
		stack->correctParams(1);

		const char *filename = stack->pop()->getString();

		delete _cursorHover;
		_cursorHover = NULL;
		BaseSprite *spr = new BaseSprite(_gameRef);
		if (!spr || DID_FAIL(spr->loadFile(filename))) {
			stack->pushBool(false);
			script->runtimeError("Item.SetHoverCursor failed for file '%s'", filename);
		} else {
			_cursorHover = spr;
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetHoverCursor") == 0) {
		stack->correctParams(0);

		if (!_cursorHover || !_cursorHover->getFilename()) {
			stack->pushNULL();
		} else {
			stack->pushString(_cursorHover->getFilename());
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverCursorObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetHoverCursorObject") == 0) {
		stack->correctParams(0);

		if (!_cursorHover) {
			stack->pushNULL();
		} else {
			stack->pushNative(_cursorHover, true);
		}
		return STATUS_OK;
	} else {
		return AdTalkHolder::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *AdItem::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("item");
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
	// DisplayAmount
	//////////////////////////////////////////////////////////////////////////
	else if (name == "DisplayAmount") {
		_scValue->setBool(_displayAmount);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Amount
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Amount") {
		_scValue->setInt(_amount);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountOffsetX
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AmountOffsetX") {
		_scValue->setInt(_amountOffsetX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountOffsetY
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AmountOffsetY") {
		_scValue->setInt(_amountOffsetY);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountAlign
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AmountAlign") {
		_scValue->setInt(_amountAlign);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountString
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AmountString") {
		if (!_amountString) {
			_scValue->setNULL();
		} else {
			_scValue->setString(_amountString);
		}
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorCombined
	//////////////////////////////////////////////////////////////////////////
	else if (name == "CursorCombined") {
		_scValue->setBool(_cursorCombined);
		return _scValue;
	} else {
		return AdTalkHolder::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool AdItem::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Name") == 0) {
		setName(value->getString());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DisplayAmount
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DisplayAmount") == 0) {
		_displayAmount = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Amount
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Amount") == 0) {
		_amount = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountOffsetX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AmountOffsetX") == 0) {
		_amountOffsetX = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountOffsetY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AmountOffsetY") == 0) {
		_amountOffsetY = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountAlign
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AmountAlign") == 0) {
		_amountAlign = (TTextAlign)value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AmountString
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AmountString") == 0) {
		if (value->isNULL()) {
			delete[] _amountString;
			_amountString = NULL;
		} else {
			BaseUtils::setString(&_amountString, value->getString());
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorCombined
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CursorCombined") == 0) {
		_cursorCombined = value->getBool();
		return STATUS_OK;
	} else {
		return AdTalkHolder::scSetProperty(name, value);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *AdItem::scToString() {
	return "[item]";
}


//////////////////////////////////////////////////////////////////////////
bool AdItem::persist(BasePersistenceManager *persistMgr) {

	AdTalkHolder::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_cursorCombined));
	persistMgr->transfer(TMEMBER(_cursorHover));
	persistMgr->transfer(TMEMBER(_cursorNormal));
	persistMgr->transfer(TMEMBER(_spriteHover));
	persistMgr->transfer(TMEMBER(_inInventory));
	persistMgr->transfer(TMEMBER(_displayAmount));
	persistMgr->transfer(TMEMBER(_amount));
	persistMgr->transfer(TMEMBER(_amountOffsetX));
	persistMgr->transfer(TMEMBER(_amountOffsetY));
	persistMgr->transfer(TMEMBER_INT(_amountAlign));
	persistMgr->transfer(TMEMBER(_amountString));

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdItem::getExtendedFlag(const char *flagName) {
	if (!flagName) {
		return false;
	} else if (strcmp(flagName, "usable") == 0) {
		return true;
	} else {
		return AdObject::getExtendedFlag(flagName);
	}
}

} // end of namespace Wintermute
