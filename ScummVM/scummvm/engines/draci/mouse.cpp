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

#include "draci/draci.h"
#include "draci/game.h"
#include "draci/mouse.h"
#include "draci/barchive.h"
#include "draci/screen.h"
#include "draci/sprite.h"

#include "graphics/cursorman.h"

#include "common/system.h"

namespace Draci {

Mouse::Mouse(DraciEngine *vm) {
	_x = 0;
	_y = 0;
	_lButton = false;
	_rButton = false;
	_cursorType = kUninitializedCursor;
	_vm = vm;
}

void Mouse::handleEvent(Common::Event event) {
	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
		debugC(6, kDraciGeneralDebugLevel, "Left button down (x: %u y: %u)", _x, _y);
		_lButton = true;
		break;

	case Common::EVENT_LBUTTONUP:
		debugC(6, kDraciGeneralDebugLevel, "Left button up (x: %u y: %u)", _x, _y);
		// Don't set _lButton to false, because some touchpads generate
		// down and up at such a quick succession, that they will
		// cancel each other in the same call of handleEvents().  Let
		// the game clear this flag by calling lButtonSet() instead.
		break;

	case Common::EVENT_RBUTTONDOWN:
		debugC(6, kDraciGeneralDebugLevel, "Right button down (x: %u y: %u)", _x, _y);
		_rButton = true;
		break;

	case Common::EVENT_RBUTTONUP:
		debugC(6, kDraciGeneralDebugLevel, "Right button up (x: %u y: %u)", _x, _y);
		break;

	case Common::EVENT_MOUSEMOVE:
		debugC(6, kDraciGeneralDebugLevel, "Mouse move (x: %u y: %u)", _x, _y);
		_x = (uint16) event.mouse.x;
		_y = (uint16) event.mouse.y;
		break;

	default:
		break;
	}
}

void Mouse::cursorOn() {
	CursorMan.showMouse(true);
}

void Mouse::cursorOff() {
	CursorMan.showMouse(false);
}

bool Mouse::isCursorOn() const {
	return CursorMan.isVisible();
}

void Mouse::setPosition(uint16 x, uint16 y) {
	_vm->_system->warpMouse(x, y);
}

void Mouse::setCursorType(CursorType cur) {
	if (cur == getCursorType()) {
		return;
	}
	_cursorType = cur;

	const BAFile *f;
	f = _vm->_iconsArchive->getFile(cur);

	Sprite sp(f->_data, f->_length, 0, 0, true);
	CursorMan.replaceCursorPalette(_vm->_screen->getPalette(), 0, kNumColors);
	CursorMan.replaceCursor(sp.getBuffer(), sp.getWidth(), sp.getHeight(),
	        sp.getWidth() / 2, sp.getHeight() / 2, 255);
}

void Mouse::loadItemCursor(const GameItem *item, bool highlighted) {
	const int itemID = item->_absNum;
	const int archiveIndex = 2 * itemID + (highlighted ? 1 : 0);
	CursorType newCursor = static_cast<CursorType> (kItemCursor + archiveIndex);
	if (newCursor == getCursorType()) {
		return;
	}
	_cursorType = newCursor;

	const BAFile *f;
	f = _vm->_itemImagesArchive->getFile(archiveIndex);

	Sprite sp(f->_data, f->_length, 0, 0, true);
	CursorMan.replaceCursorPalette(_vm->_screen->getPalette(), 0, kNumColors);
	CursorMan.replaceCursor(sp.getBuffer(), sp.getWidth(), sp.getHeight(),
	        sp.getWidth() / 2, sp.getHeight() / 2, 255);
}

} // End of namespace Draci
