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

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/keyboard.h"
#include "agi/menu.h"

namespace Agi {

// TODO: add constructor/destructor for agi_menu, agi_menu_option

struct AgiMenuOption {
	int enabled;			/**< option is enabled or disabled */
	int event;			/**< menu event */
	int index;			/**< number of option in this menu */
	char *text;			/**< text of menu option */
};

struct AgiMenu {
	MenuOptionList down;		/**< list head for menu options */
	int index;			/**< number of menu in menubar */
	int width;			/**< width of menu in characters */
	int height;			/**< height of menu in characters */
	int col;			/**< column of menubar entry */
	int wincol;			/**< column of menu window */
	char *text;			/**< menu name */
};

AgiMenu *Menu::getMenu(int i) {
	MenuList::iterator iter;
	for (iter = _menubar.begin(); iter != _menubar.end(); ++iter) {
		AgiMenu *m = *iter;
		if (m->index == i)
			return m;
	}
	return NULL;
}

AgiMenuOption *Menu::getMenuOption(int i, int j) {
	AgiMenu *m = getMenu(i);
	MenuOptionList::iterator iter;

	for (iter = m->down.begin(); iter != m->down.end(); ++iter) {
		AgiMenuOption* d = *iter;
		if (d->index == j)
			return d;
	}

	return NULL;
}

void Menu::drawMenuBar() {
	_vm->clearLines(0, 0, MENU_BG);
	_vm->flushLines(0, 0);

	MenuList::iterator iter;
	for (iter = _menubar.begin(); iter != _menubar.end(); ++iter) {
		AgiMenu *m = *iter;

		_vm->printText(m->text, 0, m->col, 0, 40, MENU_FG, MENU_BG);
	}

}

void Menu::drawMenuHilite(int curMenu) {
	AgiMenu *m = getMenu(curMenu);

	debugC(6, kDebugLevelMenu, "[%s]", m->text);

	_vm->printText(m->text, 0, m->col, 0, 40, MENU_BG, MENU_FG);
	_vm->flushLines(0, 0);
}

// draw box and pulldowns.
void Menu::drawMenuOption(int hMenu) {
	// find which vertical menu it is
	AgiMenu *m = getMenu(hMenu);

	_gfx->drawBox(m->wincol * CHAR_COLS, 1 * CHAR_LINES, (m->wincol + m->width + 2) * CHAR_COLS,
			(1 + m->height + 2) * CHAR_LINES, MENU_BG, MENU_LINE, 0);

	MenuOptionList::iterator iter;

	for (iter = m->down.begin(); iter != m->down.end(); ++iter) {
		AgiMenuOption* d = *iter;

		_vm->printText(d->text, 0, m->wincol + 1, d->index + 2, m->width + 2,
				MENU_FG, MENU_BG, !d->enabled);
	}
}

void Menu::drawMenuOptionHilite(int hMenu, int vMenu) {
	AgiMenu *m = getMenu(hMenu);
	AgiMenuOption *d = getMenuOption(hMenu, vMenu);

	// Disabled menu items are "greyed out" with a checkerboard effect,
	// rather than having a different color. -- dsymonds
	_vm->printText(d->text, 0, m->wincol + 1, vMenu + 2, m->width + 2,
			MENU_BG, MENU_FG, !d->enabled);
}

void Menu::newMenuSelected(int i) {
	_picture->showPic();
	drawMenuBar();
	drawMenuHilite(i);
	drawMenuOption(i);
}

bool Menu::mouseOverText(int line, int col, char *s) {
	if (_vm->_mouse.x < col * CHAR_COLS)
		return false;

	if (_vm->_mouse.x > (int)(col + strlen(s)) * CHAR_COLS)
		return false;

	if (_vm->_mouse.y < line * CHAR_LINES)
		return false;

	if (_vm->_mouse.y >= (line + 1) * CHAR_LINES)
		return false;

	return true;
}

#if 0
static void add_about_option() {
	const char *text = "About AGI engine";

	agi_menu_option *d = new agi_menu_option;
	d->text = strdup(text);
	d->enabled = true;
	d->event = 255;
	d->index = (v_max_menu[0] += 1);

	agi_menu *m = *menubar.begin();
	m->down.push_back(d);
	m->height++;
	if (m->width < (int)strlen(text))
		m->width = strlen(text);
}
#endif

/*
 * Public functions
 */

Menu::Menu(AgiEngine *vm, GfxMgr *gfx, PictureMgr *picture) {
	_vm = vm;
	_gfx = gfx;
	_picture = picture;
	_hIndex = 0;
	_hCol = 1;
	_hMaxMenu = 0;
	_hCurMenu = 0;
	_vCurMenu = 0;
}

Menu::~Menu() {
	MenuList::iterator iterh;
	for (iterh = _menubar.reverse_begin(); iterh != _menubar.end(); ) {
		AgiMenu *m = *iterh;

		debugC(3, kDebugLevelMenu, "deiniting hmenu %s", m->text);

		MenuOptionList::iterator iterv;

		for (iterv = m->down.reverse_begin(); iterv != m->down.end(); ) {
			AgiMenuOption *d = *iterv;

			debugC(3, kDebugLevelMenu, "  deiniting vmenu %s", d->text);

			free(d->text);
			delete d;

			iterv = m->down.reverse_erase(iterv);
		}
		free(m->text);
		delete m;

		iterh = _menubar.reverse_erase(iterh);
	}
}

void Menu::add(const char *s) {
	AgiMenu *m = new AgiMenu;
	m->text = strdup(s);

	while (m->text[strlen(m->text) - 1] == ' ')
		m->text[strlen(m->text) - 1] = 0;

	m->width = 0;
	m->height = 0;
	m->index = _hIndex++;
	m->col = _hCol;
	m->wincol = _hCol - 1;
	_vIndex = 0;
	_vMaxMenu[m->index] = 0;
	_hCol += strlen(m->text) + 1;
	_hMaxMenu = m->index;

	debugC(3, kDebugLevelMenu, "add menu: '%s' %02x", s, m->text[strlen(m->text)]);
	_menubar.push_back(m);
}

void Menu::addItem(const char *s, int code) {
	int l;

	AgiMenuOption* d = new AgiMenuOption;

	d->text = strdup(s);
	d->enabled = true;
	d->event = code;
	d->index = _vIndex++;

	// add to last menu in list
	assert(_menubar.reverse_begin() != _menubar.end());
	AgiMenu *m = *_menubar.reverse_begin();
	m->height++;

	_vMaxMenu[m->index] = d->index;

	l = strlen(d->text);
	if (l > 40)
		l = 38;
	if (m->wincol + l > 38)
		m->wincol = 38 - l;
	if (l > m->width)
		m->width = l;

	debugC(3, kDebugLevelMenu, "Adding menu item: %s (size = %d)", s, m->height);

	m->down.push_back(d);
}

void Menu::submit() {
	debugC(3, kDebugLevelMenu, "Submitting menu");

	// add_about_option ();

	// If a menu has no options, delete it
	MenuList::iterator iter;
	for (iter = _menubar.reverse_begin(); iter != _menubar.end(); ) {
		AgiMenu *m = *iter;

		if (m->down.empty()) {
			free(m->text);
			delete m;

			_hMaxMenu--;

			iter = _menubar.reverse_erase(iter);
		} else {
			--iter;
		}
	}
}

bool Menu::keyhandler(int key) {
	static int clockVal;
	static int menuActive = false;
	static int buttonUsed = 0;
	bool exitMenu = false;

	if (!_vm->getflag(fMenusWork) && !(_vm->getFeatures() & GF_MENUS))
		return false;

	if (!menuActive) {
		clockVal = _vm->_game.clockEnabled;
		_vm->_game.clockEnabled = false;
		drawMenuBar();
	}

	// Mouse handling
	if (_vm->_mouse.button) {
		int hmenu, vmenu;

		buttonUsed = 1;	// Button has been used at least once

		if (_vm->_mouse.y <= CHAR_LINES) {
			// on the menubar
			hmenu = 0;

			MenuList::iterator iterh;

			for (iterh = _menubar.begin(); iterh != _menubar.end(); ++iterh) {
				AgiMenu *m = *iterh;

				if (mouseOverText(0, m->col, m->text)) {
					break;
				} else {
					hmenu++;
				}
			}

			if (hmenu <= _hMaxMenu) {
				if (_hCurMenu != hmenu) {
					_vCurMenu = -1;
					newMenuSelected(hmenu);
				}
				_hCurMenu = hmenu;
			}
		} else {
			// not in menubar
			vmenu = 0;

			AgiMenu *m = getMenu(_hCurMenu);

			MenuOptionList::iterator iterv;

			for (iterv = m->down.begin(); iterv != m->down.end(); ++iterv) {
				AgiMenuOption *do1 = *iterv;

				if (mouseOverText(2 + do1->index, m->wincol + 1, do1->text)) {
					break;
				} else {
					vmenu++;
				}
			}

			if (vmenu <= _vMaxMenu[_hCurMenu]) {
				if (_vCurMenu != vmenu) {
					drawMenuOption(_hCurMenu);
					drawMenuOptionHilite(_hCurMenu, vmenu);
				}
				_vCurMenu = vmenu;
			}
		}
	} else if (buttonUsed) {
		// Button released
		buttonUsed = 0;

		debugC(6, kDebugLevelMenu | kDebugLevelInput, "button released!");

		if (_vCurMenu < 0)
			_vCurMenu = 0;

		drawMenuOptionHilite(_hCurMenu, _vCurMenu);

		if (_vm->_mouse.y <= CHAR_LINES) {
			// on the menubar
		} else {
			// see which option we selected
			AgiMenu *m = getMenu(_hCurMenu);
			MenuOptionList::iterator iterv;

			for (iterv = m->down.begin(); iterv != m->down.end(); ++iterv) {
				AgiMenuOption *d = *iterv;

				if (mouseOverText(2 + d->index, m->wincol + 1, d->text)) {
					// activate that option
					if (d->enabled) {
						debugC(6, kDebugLevelMenu | kDebugLevelInput, "event %d registered", d->event);
						_vm->_game.controllerOccured[d->event] = true;
						_vm->_menuSelected = true;
						break;
					}
				}
			}
			exitMenu = true;
		}
	}

	if (!exitMenu) {
		if (!menuActive) {
			if (_hCurMenu >= 0) {
				drawMenuHilite(_hCurMenu);
				drawMenuOption(_hCurMenu);
				if (!buttonUsed && _vCurMenu >= 0)
					drawMenuOptionHilite(_hCurMenu, _vCurMenu);
			}
			menuActive = true;
		}

		switch (key) {
		case KEY_ESCAPE:
			debugC(6, kDebugLevelMenu | kDebugLevelInput, "KEY_ESCAPE");
			exitMenu = true;
			break;
		case KEY_ENTER:
		{
			debugC(6, kDebugLevelMenu | kDebugLevelInput, "KEY_ENTER");
			AgiMenuOption* d = getMenuOption(_hCurMenu, _vCurMenu);

			if (d->enabled) {
				debugC(6, kDebugLevelMenu | kDebugLevelInput, "event %d registered", d->event);
				_vm->_game.controllerOccured[d->event] = true;
				_vm->_menuSelected = true;
				exitMenu = true;
			}
			break;
		}
		case KEY_DOWN:
		case KEY_UP:
			_vCurMenu += key == KEY_DOWN ? 1 : -1;

			if (_vCurMenu < 0)
				_vCurMenu = _vMaxMenu[_hCurMenu];
			if (_vCurMenu > _vMaxMenu[_hCurMenu])
				_vCurMenu = 0;

			drawMenuOption(_hCurMenu);
			drawMenuOptionHilite(_hCurMenu, _vCurMenu);
			break;
		case KEY_RIGHT:
		case KEY_LEFT:
			_hCurMenu += key == KEY_RIGHT ? 1 : -1;

			if (_hCurMenu < 0)
				_hCurMenu = _hMaxMenu;
			if (_hCurMenu > _hMaxMenu)
				_hCurMenu = 0;

			_vCurMenu = 0;
			newMenuSelected(_hCurMenu);
			drawMenuOptionHilite(_hCurMenu, _vCurMenu);
			break;
		}
	}

	if (exitMenu) {
		buttonUsed = 0;
		_picture->showPic();
		_vm->writeStatus();

		_vm->setvar(vKey, 0);
		_vm->_game.keypress = 0;
		_vm->_game.clockEnabled = clockVal;
		_vm->oldInputMode();

		debugC(3, kDebugLevelMenu, "exit_menu: input mode reset to %d", _vm->_game.inputMode);
		menuActive = false;
	}

	return true;
}

void Menu::setItem(int event, int state) {
	// scan all menus for event number #

	debugC(6, kDebugLevelMenu, "event = %d, state = %d", event, state);
	MenuList::iterator iterh;

	for (iterh = _menubar.begin(); iterh != _menubar.end(); ++iterh) {
		AgiMenu *m = *iterh;
		MenuOptionList::iterator iterv;

		for (iterv = m->down.begin(); iterv != m->down.end(); ++iterv) {
			AgiMenuOption *d = *iterv;

			if (d->event == event) {
				d->enabled = state;
				// keep going; we need to set the state of every menu item
				// with this event code. -- dsymonds
			}
		}
	}
}

void Menu::enableAll() {
	MenuList::iterator iterh;
	for (iterh = _menubar.begin(); iterh != _menubar.end(); ++iterh) {
		AgiMenu *m = *iterh;
		MenuOptionList::iterator iterv;

		for (iterv = m->down.begin(); iterv != m->down.end(); ++iterv) {
			AgiMenuOption *d = *iterv;

			d->enabled = true;
		}
	}
}


AgiTextColor AgiButtonStyle::getColor(bool hasFocus, bool pressed, bool positive) const {
	if (_amigaStyle) {
		if (positive) {
			if (pressed) { // Positive pressed Amiga-style button
				if (_olderAgi) {
					return AgiTextColor(amigaBlack, amigaOrange);
				} else {
					return AgiTextColor(amigaBlack, amigaPurple);
				}
			} else { // Positive unpressed Amiga-style button
				return AgiTextColor(amigaWhite, amigaGreen);
			}
		} else { // _amigaStyle && !positive
			if (pressed) { // Negative pressed Amiga-style button
				return AgiTextColor(amigaBlack, amigaCyan);
			} else { // Negative unpressed Amiga-style button
				return AgiTextColor(amigaWhite, amigaRed);
			}
		}
	} else { // PC-style button
		if (hasFocus || pressed) { // A pressed or in focus PC-style button
			return AgiTextColor(pcWhite, pcBlack);
		} else { // An unpressed PC-style button without focus
			return AgiTextColor(pcBlack, pcWhite);
		}
	}
}

AgiTextColor AgiButtonStyle::getColor(bool hasFocus, bool pressed, int baseFgColor, int baseBgColor) const {
	return getColor(hasFocus, pressed, AgiTextColor(baseFgColor, baseBgColor));
}

AgiTextColor AgiButtonStyle::getColor(bool hasFocus, bool pressed, const AgiTextColor &baseColor) const {
	if (hasFocus || pressed)
		return baseColor.swap();
	else
		return baseColor;
}

int AgiButtonStyle::getTextOffset(bool hasFocus, bool pressed) const {
	return (pressed && !_amigaStyle) ? 1 : 0;
}

bool AgiButtonStyle::getBorder(bool hasFocus, bool pressed) const {
	return _amigaStyle && !_authenticAmiga && (hasFocus || pressed);
}

void AgiButtonStyle::setAmigaStyle(bool amigaStyle, bool olderAgi, bool authenticAmiga) {
	_amigaStyle		= amigaStyle;
	_olderAgi		= olderAgi;
	_authenticAmiga	= authenticAmiga;
}

void AgiButtonStyle::setPcStyle(bool pcStyle) {
	setAmigaStyle(!pcStyle);
}

AgiButtonStyle::AgiButtonStyle(Common::RenderMode renderMode) {
	setAmigaStyle(renderMode == Common::kRenderAmiga);
}

} // End of namespace Agi
