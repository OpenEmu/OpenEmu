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

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/base/base_viewport.h"
#include "engines/wintermute/base/gfx/base_renderer.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(BaseViewport, false)

//////////////////////////////////////////////////////////////////////////
BaseViewport::BaseViewport(BaseGame *inGame) : BaseClass(inGame) {
	BasePlatform::setRectEmpty(&_rect);
	_mainObject = NULL;
	_offsetX = _offsetY = 0;
}


//////////////////////////////////////////////////////////////////////////
BaseViewport::~BaseViewport() {

}


//////////////////////////////////////////////////////////////////////////
bool BaseViewport::persist(BasePersistenceManager *persistMgr) {

	persistMgr->transfer(TMEMBER(_gameRef));

	persistMgr->transfer(TMEMBER(_mainObject));
	persistMgr->transfer(TMEMBER(_offsetX));
	persistMgr->transfer(TMEMBER(_offsetY));
	persistMgr->transfer(TMEMBER(_rect));

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseViewport::setRect(int left, int top, int right, int bottom, bool noCheck) {
	if (!noCheck) {
		left = MAX(left, 0);
		top = MAX(top, 0);
		right = MIN(right, _gameRef->_renderer->_width);
		bottom = MIN(bottom, _gameRef->_renderer->_height);
	}

	BasePlatform::setRect(&_rect, left, top, right, bottom);
	_offsetX = left;
	_offsetY = top;
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
Rect32 *BaseViewport::getRect() {
	return &_rect;
}


//////////////////////////////////////////////////////////////////////////
int BaseViewport::getWidth() {
	return _rect.right - _rect.left;
}


//////////////////////////////////////////////////////////////////////////
int BaseViewport::getHeight() {
	return _rect.bottom - _rect.top;
}

} // end of namespace Wintermute
