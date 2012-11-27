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

#include "engines/wintermute/ad/ad_response.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/base/font/base_font_storage.h"
#include "engines/wintermute/utils/utils.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdResponse, false)

//////////////////////////////////////////////////////////////////////////
AdResponse::AdResponse(BaseGame *inGame) : BaseObject(inGame) {
	_text = NULL;
	_textOrig = NULL;
	_icon = _iconHover = _iconPressed = NULL;
	_font = NULL;
	_iD = 0;
	_responseType = RESPONSE_ALWAYS;
}


//////////////////////////////////////////////////////////////////////////
AdResponse::~AdResponse() {
	delete[] _text;
	delete[] _textOrig;
	delete _icon;
	delete _iconHover;
	delete _iconPressed;
	_text = NULL;
	_textOrig = NULL;
	_icon = NULL;
	_iconHover = NULL;
	_iconPressed = NULL;
	if (_font) {
		_gameRef->_fontStorage->removeFont(_font);
	}
}


//////////////////////////////////////////////////////////////////////////
void AdResponse::setText(const char *text) {
	BaseUtils::setString(&_text, text);
	BaseUtils::setString(&_textOrig, text);
}


//////////////////////////////////////////////////////////////////////////
bool AdResponse::setIcon(const char *filename) {
	delete _icon;
	_icon = new BaseSprite(_gameRef);
	if (!_icon || DID_FAIL(_icon->loadFile(filename))) {
		_gameRef->LOG(0, "AdResponse::setIcon failed for file '%s'", filename);
		delete _icon;
		_icon = NULL;
		return STATUS_FAILED;
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdResponse::setFont(const char *filename) {
	if (_font) {
		_gameRef->_fontStorage->removeFont(_font);
	}
	_font = _gameRef->_fontStorage->addFont(filename);
	if (!_font) {
		_gameRef->LOG(0, "AdResponse::setFont failed for file '%s'", filename);
		return STATUS_FAILED;
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdResponse::setIconHover(const char *filename) {
	delete _iconHover;
	_iconHover = new BaseSprite(_gameRef);
	if (!_iconHover || DID_FAIL(_iconHover->loadFile(filename))) {
		_gameRef->LOG(0, "AdResponse::setIconHover failed for file '%s'", filename);
		delete _iconHover;
		_iconHover = NULL;
		return STATUS_FAILED;
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdResponse::setIconPressed(const char *filename) {
	delete _iconPressed;
	_iconPressed = new BaseSprite(_gameRef);
	if (!_iconPressed || DID_FAIL(_iconPressed->loadFile(filename))) {
		_gameRef->LOG(0, "AdResponse::setIconPressed failed for file '%s'", filename);
		delete _iconPressed;
		_iconPressed = NULL;
		return STATUS_FAILED;
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdResponse::persist(BasePersistenceManager *persistMgr) {

	BaseObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_icon));
	persistMgr->transfer(TMEMBER(_iconHover));
	persistMgr->transfer(TMEMBER(_iconPressed));
	persistMgr->transfer(TMEMBER(_iD));
	persistMgr->transfer(TMEMBER(_text));
	persistMgr->transfer(TMEMBER(_textOrig));
	persistMgr->transfer(TMEMBER_INT(_responseType));
	persistMgr->transfer(TMEMBER(_font));

	return STATUS_OK;
}

} // end of namespace Wintermute
