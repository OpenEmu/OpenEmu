/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "common/rect.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/mouse.h"
#include "sword2/screen.h"

namespace Sword2 {

#define MENUDEEP 40
#define MAXMENUANIMS 8

void Mouse::clearIconArea(int menu, int pocket, Common::Rect *r) {
	byte *buf = _vm->_screen->getScreen();
	int16 screenWide = _vm->_screen->getScreenWide();
	byte menuIconWidth;

	// Initialize menu icon width at correct size
	// depending if we are using pc or psx version.
	if (Sword2Engine::isPsx())
		menuIconWidth = RDMENU_PSXICONWIDE;
	else
		menuIconWidth = RDMENU_ICONWIDE;


	r->top = menu * (RENDERDEEP + MENUDEEP) + (MENUDEEP - RDMENU_ICONDEEP) / 2;
	r->bottom = r->top + RDMENU_ICONDEEP;
	r->left = RDMENU_ICONSTART + pocket * (menuIconWidth + RDMENU_ICONSPACING);
	r->right = r->left + menuIconWidth;

	byte *dst = buf + r->top * screenWide + r->left;

	for (int i = 0; i < RDMENU_ICONDEEP; i++) {
		memset(dst, 0, menuIconWidth);
		dst += screenWide;
	}
}

/**
 * This function should be called regularly to process the menubar system. The
 * rate at which this function is called will dictate how smooth the menu
 * system is.
 */

void Mouse::processMenu() {
	uint8 menu;
	uint8 i, j;
	uint8 frameCount;
	Common::Rect r1, r2;
	static int32 lastTime = 0;

	byte *buf = _vm->_screen->getScreen();
	int16 screenWide = _vm->_screen->getScreenWide();
	byte menuIconWidth;

	if (Sword2Engine::isPsx())
		menuIconWidth = RDMENU_PSXICONWIDE;
	else
		menuIconWidth = RDMENU_ICONWIDE;


	if (lastTime == 0) {
		lastTime = _vm->getMillis();
		frameCount = 1;
	} else {
		int32 delta = _vm->getMillis() - lastTime;

		if (delta > 250) {
			lastTime += delta;
			delta = 250;
			frameCount = 1;
		} else {
			frameCount = (uint8) ((_iconCount + 8) * delta / 750);
			lastTime += frameCount * 750 / (_iconCount + 8);
		}
	}

	// Note: The "almost hidden" menu state exists only so that the menu
	// will be redrawn one last time before it's completely hidden. We do
	// not need a corresponding "almost shown" state because the menu will
	// always be redrawn while it's shown anyway. (We may want to change
	// this later.)

	while (frameCount-- > 0) {
		for (menu = RDMENU_TOP; menu <= RDMENU_BOTTOM; menu++) {
			if (_menuStatus[menu] == RDMENU_HIDDEN || _menuStatus[menu] == RDMENU_ALMOST_HIDDEN || _menuStatus[menu] == RDMENU_SHOWN)
				continue;

			int target, direction, nextState;

			if (_menuStatus[menu] == RDMENU_OPENING) {
				target = MAXMENUANIMS;
				direction = 1;
				nextState = RDMENU_SHOWN;
			} else {
				target = 0;
				direction = -1;
				nextState = RDMENU_ALMOST_HIDDEN;
			}

			bool complete = true;

			// Propagate animation from the first icon...
			for (i = RDMENU_MAXPOCKETS - 1; i > 0; i--) {
				_pocketStatus[menu][i] = _pocketStatus[menu][i - 1];

				if (_pocketStatus[menu][i] != target)
					complete = false;
			}

			if (_pocketStatus[menu][i] != target)
				complete = false;

			// ...and animate the first icon
			if (_pocketStatus[menu][0] != target)
				_pocketStatus[menu][0] += direction;

			if (complete)
				_menuStatus[menu] = nextState;
		}
	}

	for (menu = RDMENU_TOP; menu <= RDMENU_BOTTOM; menu++) {
		if (_menuStatus[menu] == RDMENU_HIDDEN)
			continue;

		if (_menuStatus[menu] == RDMENU_ALMOST_HIDDEN)
			_menuStatus[menu] = RDMENU_HIDDEN;

		// Draw the menu here.
		int32 curx = RDMENU_ICONSTART + menuIconWidth / 2;
		int32 cury = (MENUDEEP / 2) + (RENDERDEEP + MENUDEEP) * menu;

		for (i = 0; i < RDMENU_MAXPOCKETS; i++) {
			if (_icons[menu][i]) {
				int32 xoff, yoff;

				// Since we no longer clear the screen after
				// each frame we need to clear the icon area.

				clearIconArea(menu, i, &r1);

				if (_pocketStatus[menu][i] == MAXMENUANIMS) {
					xoff = (menuIconWidth / 2);
					r2.left = curx - xoff;
					r2.right = r2.left + menuIconWidth;
					yoff = (RDMENU_ICONDEEP / 2);
					r2.top = cury - yoff;
					r2.bottom = r2.top + RDMENU_ICONDEEP;
				} else {
					xoff = (menuIconWidth / 2) * _pocketStatus[menu][i] / MAXMENUANIMS;
					r2.left = curx - xoff;
					r2.right = curx + xoff;
					yoff = (RDMENU_ICONDEEP / 2) * _pocketStatus[menu][i] / MAXMENUANIMS;
					r2.top = cury - yoff;
					r2.bottom = cury + yoff;
				}

				if (xoff != 0 && yoff != 0) {
					byte *dst = buf + r2.top * screenWide + r2.left;
					byte *src = _icons[menu][i];

					if (_pocketStatus[menu][i] != MAXMENUANIMS) {
						_vm->_screen->scaleImageFast(
							dst, screenWide, r2.right - r2.left, r2.bottom - r2.top,
							src, menuIconWidth, menuIconWidth, RDMENU_ICONDEEP);
					} else {
						for (j = 0; j < RDMENU_ICONDEEP; j++) {
							memcpy(dst, src, menuIconWidth);
							src += menuIconWidth;
							dst += screenWide;
						}
					}
				}
				_vm->_screen->updateRect(&r1);
			}
			curx += (RDMENU_ICONSPACING + menuIconWidth);
		}
	}
}

/**
 * This function brings a specified menu into view.
 * @param menu RDMENU_TOP or RDMENU_BOTTOM, depending on which menu to show
 * @return RD_OK, or an error code
 */

int32 Mouse::showMenu(uint8 menu) {

	// Do not show menu in PSX version, as there was really
	// nothing similar in the original game (menu was started
	// using SELECT button in psx pad)
	if (Sword2Engine::isPsx() && menu == RDMENU_TOP)
		return RD_OK;

	// Check for invalid menu parameter
	if (menu > RDMENU_BOTTOM)
		return RDERR_INVALIDMENU;

	// Check that the menu is not currently shown, or in the process of
	// being shown.
	if (_menuStatus[menu] == RDMENU_SHOWN || _menuStatus[menu] == RDMENU_OPENING)
		return RDERR_INVALIDCOMMAND;

	_menuStatus[menu] = RDMENU_OPENING;
	return RD_OK;
}

/**
 * This function hides a specified menu.
 * @param menu RDMENU_TOP or RDMENU_BOTTOM depending on which menu to hide
 * @return RD_OK, or an error code
 */

int32 Mouse::hideMenu(uint8 menu) {

	// In PSX version, do nothing. There is no such menu.
	if (Sword2Engine::isPsx() && menu == RDMENU_TOP)
		return RD_OK;

	// Check for invalid menu parameter
	if (menu > RDMENU_BOTTOM)
		return RDERR_INVALIDMENU;

	// Check that the menu is not currently hidden, or in the process of
	// being hidden.
	if (_menuStatus[menu] == RDMENU_HIDDEN || _menuStatus[menu] == RDMENU_CLOSING)
		return RDERR_INVALIDCOMMAND;

	_menuStatus[menu] = RDMENU_CLOSING;
	return RD_OK;
}

/**
 * This function hides both menus immediately.
 */

void Mouse::closeMenuImmediately() {
	Common::Rect r;
	int i;

	_menuStatus[RDMENU_TOP] = RDMENU_HIDDEN;
	_menuStatus[RDMENU_BOTTOM] = RDMENU_HIDDEN;

	for (i = 0; i < RDMENU_MAXPOCKETS; i++) {
		if (_icons[RDMENU_TOP][i]) {
			clearIconArea(RDMENU_TOP, i, &r);
			_vm->_screen->updateRect(&r);
		}
		if (_icons[RDMENU_BOTTOM][i]) {
			clearIconArea(RDMENU_BOTTOM, i, &r);
			_vm->_screen->updateRect(&r);
		}
	}

	memset(_pocketStatus, 0, sizeof(uint8) * 2 * RDMENU_MAXPOCKETS);
}

/**
 * This function sets a menubar icon.
 * @param menu RDMENU_TOP or RDMENU_BOTTOM, depending on which menu to change
 * @param pocket the menu pocket to change
 * @param icon icon data, or NULL to clear the icon
 * @return RD_OK, or an error code
 */

int32 Mouse::setMenuIcon(uint8 menu, uint8 pocket, byte *icon) {
	Common::Rect r;
	byte menuIconWidth;

	if (Sword2Engine::isPsx())
		menuIconWidth = RDMENU_PSXICONWIDE;
	else
		menuIconWidth = RDMENU_ICONWIDE;

	// Check for invalid menu parameter.
	if (menu > RDMENU_BOTTOM)
		return RDERR_INVALIDMENU;

	// Check for invalid pocket parameter
	if (pocket >= RDMENU_MAXPOCKETS)
		return RDERR_INVALIDPOCKET;

	// If there is an icon in the requested menu/pocket, clear it out.
	if (_icons[menu][pocket]) {
		_iconCount--;
		free(_icons[menu][pocket]);
		_icons[menu][pocket] = NULL;
		clearIconArea(menu, pocket, &r);
		_vm->_screen->updateRect(&r);
	}

	// Only put the icon in the pocket if it is not NULL
	if (icon != NULL) {
		_iconCount++;
		_icons[menu][pocket] = (byte *)malloc(menuIconWidth * RDMENU_ICONDEEP);
		if (_icons[menu][pocket] == NULL)
			return RDERR_OUTOFMEMORY;
		memcpy(_icons[menu][pocket], icon, menuIconWidth * RDMENU_ICONDEEP);
	}

	return RD_OK;
}

} // End of namespace Sword2
