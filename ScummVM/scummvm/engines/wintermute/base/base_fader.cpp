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

#include "engines/wintermute/base/base_fader.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "common/util.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_PERSISTENT(BaseFader, false)

//////////////////////////////////////////////////////////////////////////
BaseFader::BaseFader(BaseGame *inGame) : BaseObject(inGame) {
	_active = false;
	_red = _green = _blue = 0;
	_currentAlpha = 0x00;
	_sourceAlpha = 0;
	_targetAlpha = 0;
	_duration = 1000;
	_startTime = 0;
	_system = false;
}


//////////////////////////////////////////////////////////////////////////
BaseFader::~BaseFader() {

}


//////////////////////////////////////////////////////////////////////////
bool BaseFader::update() {
	if (!_active) {
		return STATUS_OK;
	}

	int alphaDelta = _targetAlpha - _sourceAlpha;

	uint32 time;

	if (_system) {
		time = g_system->getMillis() - _startTime;
	} else {
		time = _gameRef->_timer - _startTime;
	}

	if (time >= _duration) {
		_currentAlpha = _targetAlpha;
	} else {
		_currentAlpha = (byte)(_sourceAlpha + (float)time / (float)_duration * alphaDelta);
	}
	_currentAlpha = MIN((unsigned char)255, MAX(_currentAlpha, (byte)0));  // TODO: clean

	_ready = time >= _duration;
	if (_ready && _currentAlpha == 0x00) {
		_active = false;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseFader::display() {
	if (!_active) {
		return STATUS_OK;
	}

	if (_currentAlpha > 0x00) {
		_gameRef->_renderer->fadeToColor(_red, _green, _blue, _currentAlpha);
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseFader::deactivate() {
	_active = false;
	_ready = true;
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseFader::fadeIn(uint32 sourceColor, uint32 duration, bool system) {
	_ready = false;
	_active = true;

	_red   = RGBCOLGetR(sourceColor);
	_green = RGBCOLGetG(sourceColor);
	_blue  = RGBCOLGetB(sourceColor);

	_sourceAlpha = RGBCOLGetA(sourceColor);
	_targetAlpha = 0;

	_duration = duration;
	_system = system;

	if (_system) {
		_startTime = g_system->getMillis();
	} else {
		_startTime = _gameRef->_timer;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseFader::fadeOut(uint32 targetColor, uint32 duration, bool system) {
	_ready = false;
	_active = true;

	_red   = RGBCOLGetR(targetColor);
	_green = RGBCOLGetG(targetColor);
	_blue  = RGBCOLGetB(targetColor);

	//_sourceAlpha = 0;
	_sourceAlpha = _currentAlpha;
	_targetAlpha = RGBCOLGetA(targetColor);

	_duration = duration;
	_system = system;

	if (_system) {
		_startTime = g_system->getMillis();
	} else {
		_startTime = _gameRef->_timer;
	}


	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
uint32 BaseFader::getCurrentColor() {
	return BYTETORGBA(_red, _green, _blue, _currentAlpha);
}



//////////////////////////////////////////////////////////////////////////
bool BaseFader::persist(BasePersistenceManager *persistMgr) {
	BaseObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_active));
	persistMgr->transfer(TMEMBER(_blue));
	persistMgr->transfer(TMEMBER(_currentAlpha));
	persistMgr->transfer(TMEMBER(_duration));
	persistMgr->transfer(TMEMBER(_green));
	persistMgr->transfer(TMEMBER(_red));
	persistMgr->transfer(TMEMBER(_sourceAlpha));
	persistMgr->transfer(TMEMBER(_startTime));
	persistMgr->transfer(TMEMBER(_targetAlpha));
	persistMgr->transfer(TMEMBER(_system));

	if (_system && !persistMgr->getIsSaving()) {
		_startTime = 0;
	}

	return STATUS_OK;
}

} // end of namespace Wintermute
