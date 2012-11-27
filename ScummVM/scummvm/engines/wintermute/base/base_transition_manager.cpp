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

#include "engines/wintermute/base/base_transition_manager.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/base_renderer.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
BaseTransitionMgr::BaseTransitionMgr(BaseGame *inGame) : BaseClass(inGame) {
	_state = TRANS_MGR_READY;
	_type = TRANSITION_NONE;
	_origInteractive = false;
	_preserveInteractive = false;
	_lastTime = 0;
	_started = false;
}



//////////////////////////////////////////////////////////////////////////
BaseTransitionMgr::~BaseTransitionMgr() {

}


//////////////////////////////////////////////////////////////////////////
bool BaseTransitionMgr::isReady() {
	return (_state == TRANS_MGR_READY);
}


//////////////////////////////////////////////////////////////////////////
bool BaseTransitionMgr::start(TTransitionType type, bool nonInteractive) {
	if (_state != TRANS_MGR_READY) {
		return STATUS_OK;
	}

	if (type == TRANSITION_NONE || type >= NUM_TRANSITION_TYPES) {
		_state = TRANS_MGR_READY;
		return STATUS_OK;
	}

	if (nonInteractive) {
		_preserveInteractive = true;
		_origInteractive = _gameRef->_interactive;
		_gameRef->_interactive = false;
	} /*else _preserveInteractive */;


	_type = type;
	_state = TRANS_MGR_RUNNING;
	_started = false;

	return STATUS_OK;
}

#define FADE_DURATION 200

//////////////////////////////////////////////////////////////////////////
bool BaseTransitionMgr::update() {
	if (isReady()) {
		return STATUS_OK;
	}

	if (!_started) {
		_started = true;
		_lastTime = g_system->getMillis();
	}

	switch (_type) {
	case TRANSITION_NONE:
		_state = TRANS_MGR_READY;
		break;

	case TRANSITION_FADE_OUT: {
		uint32 time = g_system->getMillis() - _lastTime;
		int alpha = (int)(255 - (float)time / (float)FADE_DURATION * 255);
		alpha = MIN(255, MAX(alpha, 0));
		_gameRef->_renderer->fade((uint16)alpha);

		if (time > FADE_DURATION) {
			_state = TRANS_MGR_READY;
		}
	}
	break;

	case TRANSITION_FADE_IN: {
		uint32 time = g_system->getMillis() - _lastTime;
		int alpha = (int)((float)time / (float)FADE_DURATION * 255);
		alpha = MIN(255, MAX(alpha, 0));
		_gameRef->_renderer->fade((uint16)alpha);

		if (time > FADE_DURATION) {
			_state = TRANS_MGR_READY;
		}
	}
	break;
	default:
		error("BaseTransitionMgr::Update - unhandled enum NUM_TRANSITION_TYPES");
	}

	if (isReady()) {
		if (_preserveInteractive) {
			_gameRef->_interactive = _origInteractive;
		}
	}
	return STATUS_OK;
}

} // end of namespace Wintermute
