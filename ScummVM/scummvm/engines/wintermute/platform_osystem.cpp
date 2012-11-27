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
#include "engines/wintermute/base/gfx/osystem/base_render_osystem.h"
#include "engines/wintermute/platform_osystem.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "common/system.h"

namespace Wintermute {

BaseGame *BasePlatform::_gameRef = NULL;

#define CLASS_NAME "GF_FRAME"
int BasePlatform::initialize(BaseGame *inGame, int argc, char *argv[]) {
	_gameRef = inGame;
	return true;
}

//////////////////////////////////////////////////////////////////////////
void BasePlatform::handleEvent(Common::Event *event) {
	switch (event->type) {

	case Common::EVENT_LBUTTONDOWN:
		if (_gameRef) {
			if (_gameRef->isLeftDoubleClick()) {
				_gameRef->onMouseLeftDblClick();
			} else {
				_gameRef->onMouseLeftDown();
			}
		}
		break;
	case Common::EVENT_RBUTTONDOWN:
		if (_gameRef) {
			if (_gameRef->isRightDoubleClick()) {
				_gameRef->onMouseRightDblClick();
			} else {
				_gameRef->onMouseRightDown();
			}
		}
		break;
	case Common::EVENT_MBUTTONDOWN:
		if (_gameRef) {
			_gameRef->onMouseMiddleDown();
		}
		break;
	case Common::EVENT_LBUTTONUP:
		if (_gameRef) {
			_gameRef->onMouseLeftUp();
		}
		break;
	case Common::EVENT_RBUTTONUP:
		if (_gameRef) {
			_gameRef->onMouseRightUp();
		}
		break;
	case Common::EVENT_MBUTTONUP:
		if (_gameRef) {
			_gameRef->onMouseMiddleUp();
		}
		break;
	case Common::EVENT_KEYDOWN:
		if (_gameRef) {
			_gameRef->handleKeypress(event);
		}
		break;
	case Common::EVENT_KEYUP:
		if (_gameRef) {
			_gameRef->handleKeyRelease(event);
		}
		break;
	case Common::EVENT_WHEELUP:
	case Common::EVENT_WHEELDOWN:
		if (_gameRef) {
			_gameRef->handleMouseWheel(event->mouse.y);
		}
		break;
// Focus-events have been removed (_gameRef->onActivate originally)
	case Common::EVENT_RTL:
		_gameRef->_quitting = true;
		break;
	case Common::EVENT_QUIT:
// Block kept in case we want to support autoSaveOnExit.
// Originally this was the behaviour for WME Lite on iOS:
//		if (_gameRef) {
//			_gameRef->AutoSaveOnExit();
//			_gameRef->_quitting = true;
//		}
		if (_gameRef) {
			_gameRef->onWindowClose();
		}
		break;
	default:
		// TODO: Do we care about any other events?
		break;

	}
}


//////////////////////////////////////////////////////////////////////////
// Win32 API bindings
//////////////////////////////////////////////////////////////////////////
bool BasePlatform::getCursorPos(Point32 *lpPoint) {
	BaseRenderOSystem *renderer = static_cast<BaseRenderOSystem *>(_gameRef->_renderer);

	Common::Point p = g_system->getEventManager()->getMousePos();
	lpPoint->x = p.x;
	lpPoint->y = p.y;

	renderer->pointFromScreen(lpPoint);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BasePlatform::setCursorPos(int x, int y) {
	BaseRenderOSystem *renderer = static_cast<BaseRenderOSystem *>(_gameRef->_renderer);

	Point32 p;
	p.x = x;
	p.y = y;
	renderer->pointToScreen(&p);

	g_system->warpMouse(x, y);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BasePlatform::showWindow(int nCmdShow) {
	return false;
}

//////////////////////////////////////////////////////////////////////////
void BasePlatform::setCapture() {
	return;
}

//////////////////////////////////////////////////////////////////////////
bool BasePlatform::releaseCapture() {
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool BasePlatform::setRectEmpty(Rect32 *lprc) {
	lprc->left = lprc->right = lprc->top = lprc->bottom = 0;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BasePlatform::isRectEmpty(const Rect32 *lprc) {
	return (lprc->left >= lprc->right) || (lprc->top >= lprc->bottom);
}

//////////////////////////////////////////////////////////////////////////
bool BasePlatform::ptInRect(Rect32 *lprc, Point32 p) {
	return (p.x >= lprc->left) && (p.x < lprc->right) && (p.y >= lprc->top) && (p.y < lprc->bottom);
}

//////////////////////////////////////////////////////////////////////////
bool BasePlatform::setRect(Rect32 *lprc, int left, int top, int right, int bottom) {
	lprc->left   = left;
	lprc->top    = top;
	lprc->right  = right;
	lprc->bottom = bottom;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BasePlatform::intersectRect(Rect32 *lprcDst, const Rect32 *lprcSrc1, const Rect32 *lprcSrc2) {
	if (isRectEmpty(lprcSrc1) || isRectEmpty(lprcSrc2) ||
	        lprcSrc1->left >= lprcSrc2->right || lprcSrc2->left >= lprcSrc1->right ||
	        lprcSrc1->top >= lprcSrc2->bottom || lprcSrc2->top >= lprcSrc1->bottom) {
		setRectEmpty(lprcDst);
		return false;
	}
	lprcDst->left   = MAX(lprcSrc1->left, lprcSrc2->left);
	lprcDst->right  = MIN(lprcSrc1->right, lprcSrc2->right);
	lprcDst->top    = MAX(lprcSrc1->top, lprcSrc2->top);
	lprcDst->bottom = MIN(lprcSrc1->bottom, lprcSrc2->bottom);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BasePlatform::unionRect(Rect32 *lprcDst, Rect32 *lprcSrc1, Rect32 *lprcSrc2) {
	if (isRectEmpty(lprcSrc1)) {
		if (isRectEmpty(lprcSrc2)) {
			setRectEmpty(lprcDst);
			return false;
		} else {
			*lprcDst = *lprcSrc2;
		}
	} else {
		if (isRectEmpty(lprcSrc2)) {
			*lprcDst = *lprcSrc1;
		} else {
			lprcDst->left   = MIN(lprcSrc1->left, lprcSrc2->left);
			lprcDst->top    = MIN(lprcSrc1->top, lprcSrc2->top);
			lprcDst->right  = MAX(lprcSrc1->right, lprcSrc2->right);
			lprcDst->bottom = MAX(lprcSrc1->bottom, lprcSrc2->bottom);
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BasePlatform::copyRect(Rect32 *lprcDst, Rect32 *lprcSrc) {
	if (lprcDst == NULL || lprcSrc == NULL) {
		return false;
	}

	*lprcDst = *lprcSrc;
	return true;
}

//////////////////////////////////////////////////////////////////////////
AnsiString BasePlatform::getPlatformName() {
	// TODO: Should conform to the WME-spec.
	//return AnsiString(SDL_GetPlatform());
	return AnsiString("ScummVM");
}

//////////////////////////////////////////////////////////////////////////
char *BasePlatform::strlwr(char *string) {
	if (string) {
		for (size_t i = 0; i < strlen(string); ++i) {
			string[i] = tolower(string[i]);
		}
	}
	return string;
}

} // end of namespace Wintermute
