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

#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/cursorman.h"

#include "parallaction/exec.h"
#include "parallaction/input.h"
#include "parallaction/parallaction.h"
#include "parallaction/debug.h"

namespace Parallaction {

#define MOUSEARROW_WIDTH_NS		16
#define MOUSEARROW_HEIGHT_NS		16

#define MOUSECOMBO_WIDTH_NS		32	// sizes for cursor + selected inventory item
#define MOUSECOMBO_HEIGHT_NS		32

struct MouseComboProperties {
	int	_xOffset;
	int	_yOffset;
	int	_width;
	int	_height;
};
/*
// TODO: improve NS's handling of normal cursor before merging cursor code.
MouseComboProperties	_mouseComboProps_NS = {
	7,	// combo x offset (the icon from the inventory will be rendered from here)
	7,	// combo y offset (ditto)
	32,	// combo (arrow + icon) width
	32	// combo (arrow + icon) height
};
*/
MouseComboProperties	_mouseComboProps_BR = {
	8,	// combo x offset (the icon from the inventory will be rendered from here)
	8,	// combo y offset (ditto)
	68,	// combo (arrow + icon) width
	68	// combo (arrow + icon) height
};

Input::Input(Parallaction *vm) : _vm(vm) {
	_gameType = _vm->getGameType();
	_transCurrentHoverItem = 0;
	_hasDelayedAction = false;  // actived when the character needs to move before taking an action
	_mouseState = MOUSE_DISABLED;
	_activeItem._index = 0;
	_activeItem._id = 0;
	_mouseButtons = 0;
	_delayedActionZone.reset();

	_dinoCursor = 0;
	_dougCursor = 0;
	_donnaCursor = 0;
	_comboArrow = 0;
	initCursors();
}

Input::~Input() {
	if (_gameType == GType_Nippon) {
		delete _mouseArrow;
	}

	delete _comboArrow;
	delete _dinoCursor;
	delete _dougCursor;
	delete _donnaCursor;
}

// FIXME: the engine has 3 event loops. The following routine hosts the main one,
// and it's called from 8 different places in the code. There exist 2 more specialised
// loops which could possibly be merged into this one with some effort in changing
// caller code, i.e. adding condition checks.
//
void Input::readInput() {
	bool updateMousePos = false;

	Common::Event e;

	_mouseButtons = kMouseNone;
	_hasKeyPressEvent = false;

	Common::EventManager *eventMan = _vm->_system->getEventManager();
	while (eventMan->pollEvent(e)) {
		updateMousePos = true;

		switch (e.type) {
		case Common::EVENT_KEYDOWN:
			_hasKeyPressEvent = true;
			_keyPressed = e.kbd;

			if (e.kbd.hasFlags(Common::KBD_CTRL) && e.kbd.keycode == Common::KEYCODE_d)
				_vm->_debugger->attach();

			updateMousePos = false;
			break;

		case Common::EVENT_LBUTTONDOWN:
			_mouseButtons = kMouseLeftDown;
			break;

		case Common::EVENT_LBUTTONUP:
			_mouseButtons = kMouseLeftUp;
			break;

		case Common::EVENT_RBUTTONDOWN:
			_mouseButtons = kMouseRightDown;
			break;

		case Common::EVENT_RBUTTONUP:
			_mouseButtons = kMouseRightUp;
			break;

		case Common::EVENT_RTL:
		case Common::EVENT_QUIT:
			return;

		default:
			break;

		}

	}

	if (updateMousePos) {
		setCursorPos(e.mouse);
	}

	_vm->_debugger->onFrame();

	return;

}

bool Input::getLastKeyDown(uint16 &ascii) {
	ascii = _keyPressed.ascii;
	return (_hasKeyPressEvent);
}

// FIXME: see comment for readInput()
void Input::waitForButtonEvent(uint32 buttonEventMask, int32 timeout) {

	if (buttonEventMask == kMouseNone) {
		_mouseButtons = kMouseNone;	// don't wait on nothing
		return;
	}

	const int32 LOOP_RESOLUTION = 30;
	if (timeout <= 0) {
		do {
			readInput();
			_vm->_system->delayMillis(LOOP_RESOLUTION);
		} while ((_mouseButtons & buttonEventMask) == 0);
	} else {
		do {
			readInput();
			_vm->_system->delayMillis(LOOP_RESOLUTION);
			timeout -= LOOP_RESOLUTION;
		} while ((timeout > 0) && (_mouseButtons & buttonEventMask) == 0);
	}

}


int Input::updateGameInput() {

	int event = kEvNone;

	if (!isMouseEnabled() ||
		(g_engineFlags & kEngineBlockInput) ||
		(g_engineFlags & kEngineWalking) ||
		(g_engineFlags & kEngineChangeLocation)) {

		debugC(3, kDebugInput, "updateGameInput: input flags (mouse: %i, block: %i, walking: %i, changeloc: %i)",
			isMouseEnabled(),
			(g_engineFlags & kEngineBlockInput) == 0,
			(g_engineFlags & kEngineWalking) == 0,
			(g_engineFlags & kEngineChangeLocation) == 0
		);

		return event;
	}

	if (_gameType == GType_Nippon) {
		if (_hasKeyPressEvent && (_vm->getFeatures() & GF_DEMO) == 0) {
			if (_keyPressed.keycode == Common::KEYCODE_l) event = kEvLoadGame;
			if (_keyPressed.keycode == Common::KEYCODE_s) event = kEvSaveGame;
		}
	} else
	if (_gameType == GType_BRA) {
		if (_hasKeyPressEvent && (_vm->getFeatures() & GF_DEMO) == 0) {
			if (_keyPressed.keycode == Common::KEYCODE_F5) event = kEvIngameMenu;
		}
	} else {
		error("unsupported gametype in updateGameInput");
	}


	if (event == kEvNone) {
		translateGameInput();
	}

	return event;
}


int Input::updateInput() {

	int oldMode = _inputMode;

	int event = kEvNone;
	readInput();

	switch (_inputMode) {
	case kInputModeGame:
		event = updateGameInput();
		break;

	case kInputModeInventory:
		updateInventoryInput();
		break;
	}

	// when mode changes, then consider any input consumed
	// for the current frame
	if (oldMode != _inputMode) {
		_mouseButtons = kEvNone;
		_hasKeyPressEvent = false;
	}

	return event;
}

void Input::trackMouse(ZonePtr z) {
	if ((z != _hoverZone) && (_hoverZone)) {
		stopHovering();
		return;
	}

	if (!z) {
		return;
	}

	if ((!_hoverZone) && ((z->_flags & kFlagsNoName) == 0)) {
		_hoverZone = z;
		_vm->_gfx->showFloatingLabel(_hoverZone->_label);
		return;
	}
}

void Input::stopHovering() {
	_hoverZone.reset();
	_vm->_gfx->hideFloatingLabel();
}

void Input::takeAction(ZonePtr z) {
	stopHovering();
	_vm->pauseJobs();
	_vm->runZone(z);
	_vm->resumeJobs();
}

void Input::walkTo(const Common::Point &dest) {
	stopHovering();
	setArrowCursor();
	_vm->scheduleWalk(dest.x, dest.y, true);
}

bool Input::translateGameInput() {

	if (g_engineFlags & kEnginePauseJobs) {
		return false;
	}

	if (_hasDelayedAction) {
		// if walking is over, then take programmed action
		takeAction(_delayedActionZone);
		_hasDelayedAction = false;
		_delayedActionZone.reset();
		return true;
	}

	if (_mouseButtons == kMouseRightDown) {
		// right button down shows inventory
		enterInventoryMode();
		return true;
	}

	Common::Point mousePos;
	getAbsoluteCursorPos(mousePos);
	// test if mouse is hovering on an interactive zone for the currently selected inventory item
	ZonePtr z = _vm->hitZone(_activeItem._id, mousePos.x, mousePos.y);

	if (((_mouseButtons == kMouseLeftUp) && (_activeItem._id == 0) && ((g_engineFlags & kEngineWalking) == 0)) && ((!z) || (ACTIONTYPE(z) != kZoneCommand))) {
		walkTo(mousePos);
		return true;
	}

	trackMouse(z);
	if (!z) {
		return true;
	}

	if ((_mouseButtons == kMouseLeftUp) && ((_activeItem._id != 0) || (ACTIONTYPE(z) == kZoneCommand))) {

		bool noWalk = z->_flags & kFlagsNoWalk;	// check the explicit no-walk flag
		if (_gameType == GType_BRA) {
			// action performed on object marked for self-use do not need walk in BRA
			noWalk |= ((z->_flags & kFlagsYourself) != 0);
		}

		if (noWalk) {
			takeAction(z);
		} else {
			// action delayed: if Zone defined a moveto position the character is programmed to move there,
			// else it will move to the mouse position
			_delayedActionZone = z;
			_hasDelayedAction = true;
			if (z->_moveTo.y != 0) {
				mousePos = z->_moveTo;
			}

			walkTo(mousePos);
		}

		_vm->beep();
		setArrowCursor();
		return true;
	}

	return true;
}


void Input::enterInventoryMode() {
	Common::Point mousePos;
	getAbsoluteCursorPos(mousePos);
	bool hitCharacter = _vm->hitZone(kZoneYou, mousePos.x, mousePos.y);

	if (hitCharacter) {
		if (_activeItem._id != 0) {
			_activeItem._index = (_activeItem._id >> 16) & 0xFFFF;
			g_engineFlags |= kEngineDragging;
		} else {
			setArrowCursor();
		}
	}

	stopHovering();
	_vm->pauseJobs();
	_vm->openInventory();

	_transCurrentHoverItem = -1;

	_inputMode = kInputModeInventory;
}

void Input::exitInventoryMode() {
	// right up hides inventory
	Common::Point mousePos;
	getCursorPos(mousePos);

	int pos = _vm->getHoverInventoryItem(mousePos.x, mousePos.y);
	_vm->highlightInventoryItem(-1);			// disable

	if ((g_engineFlags & kEngineDragging)) {

		g_engineFlags &= ~kEngineDragging;
		ZonePtr z = _vm->hitZone(kZoneMerge, _activeItem._index, _vm->getInventoryItemIndex(pos));

		if (z) {
			_vm->dropItem(z->u._mergeObj1);
			_vm->dropItem(z->u._mergeObj2);
			_vm->addInventoryItem(z->u._mergeObj3);
			_vm->_cmdExec->run(z->_commands);	// commands might set a new _inputMode
		}

	}

	_vm->closeInventory();
	if (pos == -1) {
		setArrowCursor();
	} else {
		const InventoryItem *item = _vm->getInventoryItem(pos);
		if (item->_index != 0) {
			_activeItem._id = item->_id;
			setInventoryCursor(item->_index);
		}
	}
	_vm->resumeJobs();

	// in case the input mode was not changed by the code above (especially by the commands
	// executed in case of a merge), then assume we are going back to game mode
	if (_inputMode == kInputModeInventory) {
		_inputMode = kInputModeGame;
	}
}

bool Input::updateInventoryInput() {
	if (_mouseButtons == kMouseRightUp) {
		exitInventoryMode();
		return true;
	}

	Common::Point mousePos;
	getCursorPos(mousePos);

	int16 _si = _vm->getHoverInventoryItem(mousePos.x, mousePos.y);
	if (_si != _transCurrentHoverItem) {
		_transCurrentHoverItem = _si;
		_vm->highlightInventoryItem(_si);						// enable
	}

	return true;

}

void Input::setMouseState(MouseTriState state) {
	assert(state == MOUSE_ENABLED_SHOW || state == MOUSE_ENABLED_HIDE || state == MOUSE_DISABLED);
	_mouseState = state;

	switch (_mouseState) {
	case MOUSE_ENABLED_HIDE:
	case MOUSE_DISABLED:
		CursorMan.showMouse(false);
		break;

	case MOUSE_ENABLED_SHOW:
		CursorMan.showMouse(true);
		break;
	}
}

MouseTriState Input::getMouseState() {
	return _mouseState;
}

bool Input::isMouseEnabled() {
	return (_mouseState == MOUSE_ENABLED_SHOW) || (_mouseState == MOUSE_ENABLED_HIDE);
}

void Input::getAbsoluteCursorPos(Common::Point& p) const {
	_vm->_gfx->getScrollPos(p);
	p.x += _mousePos.x;
	p.y += _mousePos.y;
}


void Input::initCursors() {

	_dinoCursor = _donnaCursor = _dougCursor = 0;

	switch (_gameType) {
	case GType_Nippon:
		_comboArrow = _vm->_disk->loadPointer("pointer");
		_mouseArrow = new Cnv(1, MOUSEARROW_WIDTH_NS, MOUSEARROW_HEIGHT_NS, _resMouseArrow_NS, false);
		break;

	case GType_BRA:
		if (_vm->getPlatform() == Common::kPlatformPC) {
			_dinoCursor = _vm->_disk->loadPointer("pointer1");
			_dougCursor = _vm->_disk->loadPointer("pointer2");
			_donnaCursor = _vm->_disk->loadPointer("pointer3");

			Graphics::Surface *surf = new Graphics::Surface;
			surf->create(_mouseComboProps_BR._width, _mouseComboProps_BR._height, Graphics::PixelFormat::createFormatCLUT8());
			_comboArrow = new SurfaceToFrames(surf);

			// TODO: choose the pointer depending on the active character
			// For now, we pick Donna's
			_mouseArrow = _donnaCursor;
		} else {
			// TODO: Where are the Amiga cursors?
			Graphics::Surface *surf1 = new Graphics::Surface;
			surf1->create(_mouseComboProps_BR._width, _mouseComboProps_BR._height, Graphics::PixelFormat::createFormatCLUT8());
			_comboArrow = new SurfaceToFrames(surf1);

			// TODO: scale mouse cursor (see staticres.cpp)
			Graphics::Surface *surf2 = new Graphics::Surface;
			surf2->create(32, 16, Graphics::PixelFormat::createFormatCLUT8());
			memcpy(surf2->pixels, _resMouseArrow_BR_Amiga, 32*16);
			_mouseArrow = new SurfaceToFrames(surf2);
		}
		break;

	default:
		warning("Input::initCursors: unknown gametype");
	}

}

void Input::setArrowCursor() {

	switch (_gameType) {
	case GType_Nippon:
		debugC(1, kDebugInput, "setting mouse cursor to arrow");
		// this stuff is needed to avoid artifacts with labels and selected items when switching cursors
		stopHovering();
		_activeItem._id = 0;
		CursorMan.replaceCursor(_mouseArrow->getData(0), MOUSEARROW_WIDTH_NS, MOUSEARROW_HEIGHT_NS, 0, 0, 0);
		break;

	case GType_BRA: {
		Common::Rect r;
		_mouseArrow->getRect(0, r);
		CursorMan.replaceCursor(_mouseArrow->getData(0), r.width(), r.height(), 0, 0, 0);
		CursorMan.showMouse(true);
		_activeItem._id = 0;
		break;
	}

	default:
		warning("Input::setArrowCursor: unknown gametype");
	}

}

void Input::setInventoryCursor(ItemName name) {
	assert(name > 0);

	switch (_gameType) {
	case GType_Nippon: {
		byte *v8 = _comboArrow->getData(0);
		// FIXME: destination offseting is not clear
		_vm->_inventoryRenderer->drawItem(name, v8 + 7 * MOUSECOMBO_WIDTH_NS + 7, MOUSECOMBO_WIDTH_NS);
		CursorMan.replaceCursor(v8, MOUSECOMBO_WIDTH_NS, MOUSECOMBO_HEIGHT_NS, 0, 0, 0);
		break;
	}

	case GType_BRA: {
		byte *src = _mouseArrow->getData(0);
		byte *dst = _comboArrow->getData(0);
		memcpy(dst, src, _comboArrow->getSize(0));
		// FIXME: destination offseting is not clear
		_vm->_inventoryRenderer->drawItem(name, dst + _mouseComboProps_BR._yOffset * _mouseComboProps_BR._width + _mouseComboProps_BR._xOffset, _mouseComboProps_BR._width);
		CursorMan.replaceCursor(dst, _mouseComboProps_BR._width, _mouseComboProps_BR._height, 0, 0, 0);
		break;
	}

	default:
		warning("Input::setInventoryCursor: unknown gametype");
	}

}

} // namespace Parallaction
