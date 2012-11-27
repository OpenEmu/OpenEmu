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
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

// mouse.cpp : Handle all mouse activity

#include "common/debug.h"
#include "common/system.h"

#include "hugo/hugo.h"
#include "hugo/game.h"
#include "hugo/mouse.h"
#include "hugo/schedule.h"
#include "hugo/display.h"
#include "hugo/inventory.h"
#include "hugo/route.h"
#include "hugo/util.h"
#include "hugo/object.h"
#include "hugo/text.h"

namespace Hugo {

MouseHandler::MouseHandler(HugoEngine *vm) : _vm(vm), _hotspots(0) {
	_leftButtonFl  = false;
	_rightButtonFl = false;
	_jumpExitFl = false;                            // Can't jump to a screen exit
	_mouseX = kXPix / 2;
	_mouseY = kYPix / 2;
}

void MouseHandler::resetLeftButton() {
	_leftButtonFl = false;
}

void MouseHandler::resetRightButton() {
	_rightButtonFl = false;
}

void MouseHandler::setLeftButton() {
	_leftButtonFl = true;
}

void MouseHandler::setRightButton() {
	_rightButtonFl = true;
}

void MouseHandler::setJumpExitFl(bool fl) {
	_jumpExitFl = fl;
}

void MouseHandler::setMouseX(int x) {
	_mouseX = x;
}

void MouseHandler::setMouseY(int y) {
	_mouseY = y;
}

void MouseHandler::freeHotspots() {
	free(_hotspots);
}

bool MouseHandler::getJumpExitFl() const {
	return _jumpExitFl;
}

int MouseHandler::getMouseX() const {
	return _mouseX;
}

int MouseHandler::getMouseY() const {
	return _mouseY;
}

int16 MouseHandler::getDirection(const int16 hotspotId) const {
	return _hotspots[hotspotId]._direction;
}

int16 MouseHandler::getHotspotActIndex(const int16 hotspotId) const {
	return _hotspots[hotspotId]._actIndex;
}

/**
 * Shadow-blit supplied string into dib_a at cx,cy and add to display list
 */
void MouseHandler::cursorText(const char *buffer, const int16 cx, const int16 cy, const Uif fontId, const int16 color) {
	debugC(1, kDebugMouse, "cursorText(%s, %d, %d, %d, %d)", buffer, cx, cy, fontId, color);

	_vm->_screen->loadFont(fontId);

	// Find bounding rect for string
	int16 sdx = _vm->_screen->stringLength(buffer);
	int16 sdy = _vm->_screen->fontHeight() + 1;     // + 1 for shadow
	int16 sx, sy;
	if (cx < kXPix / 2) {
		sx = cx + kCursorNameOffX;
		sy = (_vm->_inventory->getInventoryObjId() == -1) ? cy + kCursorNameOffY : cy + kCursorNameOffY - (_vm->_screen->fontHeight() + 1);
	} else {
		sx = cx - sdx - kCursorNameOffX / 2;
		sy = cy + kCursorNameOffY;
	}

	// Display the string and add rect to display list
	_vm->_screen->shadowStr(sx, sy, buffer, _TBRIGHTWHITE);
	_vm->_screen->displayList(kDisplayAdd, sx, sy, sdx, sdy);
}

/**
 * Find the exit hotspot containing cx, cy.
 * Return hotspot index or -1 if not found.
 */
int16 MouseHandler::findExit(const int16 cx, const int16 cy, byte screenId) {
	debugC(2, kDebugMouse, "findExit(%d, %d, %d)", cx, cy, screenId);

	for (int i = 0; _hotspots[i]._screenIndex >= 0; i++) {
		if (_hotspots[i]._screenIndex == screenId) {
			if (cx >= _hotspots[i]._x1 && cx <= _hotspots[i]._x2 && cy >= _hotspots[i]._y1 && cy <= _hotspots[i]._y2)
				return i;
		}
	}
	return -1;
}

/**
 * Process a mouse right click at coord cx, cy over object objid
 */
void MouseHandler::processRightClick(const int16 objId, const int16 cx, const int16 cy) {
	debugC(1, kDebugMouse, "ProcessRightClick(%d, %d, %d)", objId, cx, cy);

	Status &gameStatus = _vm->getGameStatus();

	if (gameStatus._storyModeFl || _vm->_hero->_pathType == kPathQuiet) // Make sure user has control
		return;

	int16 inventObjId = _vm->_inventory->getInventoryObjId();
	bool foundFl = false;                           // TRUE if route found to object
	// Check if this was over iconbar
	if ((_vm->_inventory->getInventoryState() == kInventoryActive) && (cy < kInvDy + kDibOffY)) { // Clicked over iconbar object
		if (inventObjId == -1)
			_vm->_screen->selectInventoryObjId(objId);
		else if (inventObjId == objId)
			_vm->_screen->resetInventoryObjId();
		else
			_vm->_object->useObject(objId);         // Use status.objid on object
	} else {                                        // Clicked over viewport object
		Object *obj = &_vm->_object->_objects[objId];
		int16 x, y;
		switch (obj->_viewx) {                       // Where to walk to
		case -1:                                    // Walk to object position
			if (_vm->_object->findObjectSpace(obj, &x, &y))
				foundFl = _vm->_route->startRoute(kRouteGet, objId, x, y);
			if (!foundFl)                           // Can't get there, try to use from here
				_vm->_object->useObject(objId);
			break;
		case 0:                                     // Immediate use
			_vm->_object->useObject(objId);         // Pick up or use object
			break;
		default:                                    // Walk to view point if possible
			if (!_vm->_route->startRoute(kRouteGet, objId, obj->_viewx, obj->_viewy)) {
				if (_vm->_hero->_cycling == kCycleInvisible) // If invisible do
					_vm->_object->useObject(objId); // immediate use
				else
					Utils::notifyBox(_vm->_text->getTextMouse(kMsNoWayText)); // Can't get there
			}
			break;
		}
	}
}

/** Process a left mouse click over:
 * 1.  An icon - show description
 * 2.  An object - walk to and show description
 * 3.  An icon scroll arrow - scroll the iconbar
 * 4.  Nothing - attempt to walk there
 * 5.  Exit - walk to exit hotspot
 */
void MouseHandler::processLeftClick(const int16 objId, const int16 cx, const int16 cy) {
	debugC(1, kDebugMouse, "ProcessLeftClick(%d, %d, %d)", objId, cx, cy);

	int16 i, x, y;
	Object *obj;

	Status &gameStatus = _vm->getGameStatus();

	if (gameStatus._storyModeFl || _vm->_hero->_pathType == kPathQuiet) // Make sure user has control
		return;

	switch (objId) {
	case -1:                                        // Empty space - attempt to walk there
		_vm->_route->startRoute(kRouteSpace, 0, cx, cy);
		break;
	case kLeftArrow:                                // A scroll arrow - scroll the iconbar
	case kRightArrow:
		// Scroll the iconbar and display results
		_vm->_inventory->processInventory((objId == kLeftArrow) ? kInventoryActionLeft : kInventoryActionRight);
		_vm->_screen->moveImage(_vm->_screen->getIconBuffer(), 0, 0, kXPix, kInvDy, kXPix, _vm->_screen->getFrontBuffer(), 0, kDibOffY, kXPix);
		_vm->_screen->moveImage(_vm->_screen->getIconBuffer(), 0, 0, kXPix, kInvDy, kXPix, _vm->_screen->getBackBuffer(), 0, kDibOffY, kXPix);
		_vm->_screen->displayList(kDisplayAdd, 0, kDibOffY, kXPix, kInvDy);
		break;
	case kExitHotspot:                              // Walk to exit hotspot
		i = findExit(cx, cy, *_vm->_screenPtr);
		x = _hotspots[i]._viewx;
		y = _hotspots[i]._viewy;
		if (x >= 0) {                               // Hotspot refers to an exit
			// Special case of immediate exit
			if (_jumpExitFl) {
				// Get rid of iconbar if necessary
				if (_vm->_inventory->getInventoryState() != kInventoryOff)
					_vm->_inventory->setInventoryState(kInventoryUp);
				_vm->_scheduler->insertActionList(_hotspots[i]._actIndex);
			} else {    // Set up route to exit spot
				if (_hotspots[i]._direction == Common::KEYCODE_RIGHT)
					x -= kHeroMaxWidth;
				else if (_hotspots[i]._direction == Common::KEYCODE_LEFT)
					x += kHeroMaxWidth;
				if (!_vm->_route->startRoute(kRouteExit, i, x, y))
					Utils::notifyBox(_vm->_text->getTextMouse(kMsNoWayText)); // Can't get there
			}

			// Get rid of any attached icon
			_vm->_screen->resetInventoryObjId();
		}
		break;
	default:                                        // Look at an icon or object
		obj = &_vm->_object->_objects[objId];

		// Over iconbar - immediate description
		if ((_vm->_inventory->getInventoryState() == kInventoryActive) && (cy < kInvDy + kDibOffY)) {
			_vm->_object->lookObject(obj);
		} else {
			bool foundFl = false;                   // TRUE if route found to object
			switch (obj->_viewx) {                   // Clicked over viewport object
			case -1:                                // Walk to object position
				if (_vm->_object->findObjectSpace(obj, &x, &y))
					foundFl = _vm->_route->startRoute(kRouteLook, objId, x, y);
				if (!foundFl)                       // Can't get there, immediate description
					_vm->_object->lookObject(obj);
				break;
			case 0:                                 // Immediate description
				_vm->_object->lookObject(obj);
				break;
			default:                                // Walk to view point if possible
				if (!_vm->_route->startRoute(kRouteLook, objId, obj->_viewx, obj->_viewy)) {
					if (_vm->_hero->_cycling == kCycleInvisible) // If invisible do
						_vm->_object->lookObject(obj);          // immediate decription
					else
						Utils::notifyBox(_vm->_text->getTextMouse(kMsNoWayText));  // Can't get there
				}
				break;
			}
		}
		break;
	}
}

/**
 * Process mouse activity
 */
void MouseHandler::mouseHandler() {
	debugC(2, kDebugMouse, "mouseHandler");

	Status &gameStatus = _vm->getGameStatus();
	Istate inventState = _vm->_inventory->getInventoryState();
	if ((gameStatus._viewState != kViewPlay) && (inventState != kInventoryActive))
		return;

	int16 cx = getMouseX();
	int16 cy = getMouseY();

//	gameStatus._cx = cx;                             // Save cursor coords
//	gameStatus._cy = cy;

	// Don't process if outside client area
	if ((cx < 0) || (cx > kXPix) || (cy < kDibOffY) || (cy > kViewSizeY + kDibOffY))
		return;

	int16 objId = -1;                               // Current source object
	// Process cursor over an object or icon
	if (inventState == kInventoryActive) { // Check inventory icon bar first
		objId = _vm->_inventory->processInventory(kInventoryActionGet, cx, cy);
	} else {
		if (cy < 5 && cy > 0) {
			_vm->_topMenu->runModal();
		}
	}

	if (!gameStatus._gameOverFl) {
		if (objId == -1)                            // No match, check rest of view
			objId = _vm->_object->findObject(cx, cy);

		if (objId >= 0) {                           // Got a match
			// Display object name next to cursor (unless CURSOR_NOCHAR)
			// Note test for swapped hero name
			const char *name = _vm->_text->getNoun(_vm->_object->_objects[(objId == kHeroIndex) ? _vm->_heroImage : objId]._nounIndex, kCursorNameIndex);
			if (name[0] != kCursorNochar)
				cursorText(name, cx, cy, U_FONT8, _TBRIGHTWHITE);

			// Process right click over object in view or iconbar
			if (_rightButtonFl)
				processRightClick(objId, cx, cy);
		}

		// Process cursor over an exit hotspot
		if (objId == -1) {
			int i = findExit(cx, cy, *_vm->_screenPtr);
			if (i != -1 && _hotspots[i]._viewx >= 0) {
				objId = kExitHotspot;
				cursorText(_vm->_text->getTextMouse(kMsExit), cx, cy, U_FONT8, _TBRIGHTWHITE);
			}
		}
	}
	// Left click over icon, object or to move somewhere
	if (_leftButtonFl)
		processLeftClick(objId, cx, cy);

	// Clear mouse click states
	resetLeftButton();
	resetRightButton();
}

void MouseHandler::readHotspot(Common::ReadStream &in, Hotspot &hotspot) {
	hotspot._screenIndex = in.readSint16BE();
	hotspot._x1 = in.readSint16BE();
	hotspot._y1 = in.readSint16BE();
	hotspot._x2 = in.readSint16BE();
	hotspot._y2 = in.readSint16BE();
	hotspot._actIndex = in.readUint16BE();
	hotspot._viewx = in.readSint16BE();
	hotspot._viewy = in.readSint16BE();
	hotspot._direction = in.readSint16BE();
}

/**
 * Load hotspots data from hugo.dat
 */
void MouseHandler::loadHotspots(Common::ReadStream &in) {
	Hotspot *wrkHotspots = 0;
	Hotspot tmp;
	memset(&tmp, 0, sizeof(tmp));
	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		int numRows = in.readUint16BE();
		if (varnt == _vm->_gameVariant)
			_hotspots = wrkHotspots = (Hotspot *)malloc(sizeof(Hotspot) * numRows);

		for (int i = 0; i < numRows; i++)
			readHotspot(in, (varnt == _vm->_gameVariant) ? wrkHotspots[i] : tmp);
	}
}

/**
 * Display hotspot boundaries for the current screen
 */
void MouseHandler::drawHotspots() const {
	for (int i = 0; _hotspots[i]._screenIndex >= 0; i++) {
		Hotspot *hotspot = &_hotspots[i];
		if (hotspot->_screenIndex == _vm->_hero->_screenIndex)
			_vm->_screen->drawRectangle(false, hotspot->_x1, hotspot->_y1, hotspot->_x2, hotspot->_y2, _TLIGHTRED);
	}
}
} // End of namespace Hugo
