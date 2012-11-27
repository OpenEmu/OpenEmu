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


#include "sword1/menu.h"
#include "sword1/resman.h"
#include "common/scummsys.h"
#include "common/util.h"
#include "common/system.h"
#include "sword1/mouse.h"
#include "sword1/screen.h"
#include "sword1/logic.h"

namespace Sword1 {

enum {
	MENU_CLOSED,
	MENU_CLOSING,
	MENU_OPENING,
	MENU_OPEN
};

const byte Menu::_fadeEffectTop[64] = {
	1, 7, 5, 3, 2, 4, 6, 0,
	3, 1, 7, 5, 4, 6, 0, 2,
	5, 3, 1, 7, 6, 0, 2, 4,
	7, 5, 3, 1, 0, 2, 4, 6,
	7, 5, 3, 1, 0, 2, 4, 6,
	5, 3, 1, 7, 6, 0, 2, 4,
	3, 1, 7, 5, 4, 6, 0, 2,
	1, 7, 5, 3, 2, 4, 6, 0
};

const byte Menu::_fadeEffectBottom[64] = {
	7, 6, 5, 4, 3, 2, 1, 0,
	0, 7, 6, 5, 4, 3, 2, 1,
	1, 0, 7, 6, 5, 4, 3, 2,
	2, 1, 0, 7, 6, 5, 4, 3,
	3, 2, 1, 0, 7, 6, 5, 4,
	4, 3, 2, 1, 0, 7, 6, 5,
	5, 4, 3, 2, 1, 0, 7, 6,
	6, 5, 4, 3, 2, 1, 0, 7
};

MenuIcon::MenuIcon(uint8 menuType, uint8 menuPos, uint32 resId, uint32 frame, Screen *screen) {
	_menuType = menuType;
	_menuPos = menuPos;
	_resId = resId;
	_frame = frame;
	_screen = screen;
	_selected = false;
}

bool MenuIcon::wasClicked(uint16 mouseX, uint16 mouseY) {
	if (((_menuType == MENU_TOP) && (mouseY >= 40)) || ((_menuType == MENU_BOT) && (mouseY < 440)))
		return false;
	if ((mouseX >= _menuPos * 40) && (mouseX < (_menuPos + 1) * 40))
		return true;
	else
		return false;
}

void MenuIcon::setSelect(bool pSel) {
	_selected = pSel;
}

void MenuIcon::draw(const byte *fadeMask, int8 fadeStatus) {
	uint16 x = _menuPos * 40;
	uint16 y = (_menuType == MENU_TOP) ? (0) : (440);
	_screen->showFrame(x, y, _resId, _frame + (_selected ? 1 : 0), fadeMask, fadeStatus);
}

Menu::Menu(Screen *pScreen, Mouse *pMouse) {
	uint8 cnt;
	_screen = pScreen;
	_mouse = pMouse;
	_subjectBarStatus = MENU_CLOSED;
	_objectBarStatus = MENU_CLOSED;
	_fadeSubject = 0;
	_fadeObject = 0;
	for (cnt = 0; cnt < 16; cnt++)
		_subjects[cnt] = NULL;
	for (cnt = 0; cnt < TOTAL_pockets; cnt++)
		_objects[cnt] = NULL;
	_inMenu = 0;
}

Menu::~Menu() {
	int i;
	// the menu may be open, so delete the icons
	for (i = 0; i < TOTAL_pockets; i++) {
		delete _objects[i];
		_objects[i] = NULL;
	}
	for (i = 0; i < 16; i++) {
		delete _subjects[i];
		_subjects[i] = NULL;
	}
}

void Menu::refreshMenus() {
	if (_objectBarStatus == MENU_OPEN) {
		buildMenu();
		for (uint8 cnt = 0; cnt < 16; cnt++) {
			if (_objects[cnt])
				_objects[cnt]->draw();
			else
				_screen->showFrame(cnt * 40, 0, 0xffffffff, 0);
		}
	}
	if (_subjectBarStatus == MENU_OPEN) {
		buildSubjects();
		for (uint8 cnt = 0; cnt < 16; cnt++) {
			if (_subjects[cnt])
				_subjects[cnt]->draw();
			else
				_screen->showFrame(cnt * 40, 440, 0xffffffff, 0);
		}
	}
}

uint8 Menu::checkMenuClick(uint8 menuType) {
	uint16 mouseEvent = _mouse->testEvent();
	if (!mouseEvent)
		return 0;
	uint16 x, y;
	_mouse->giveCoords(&x, &y);
	if (_subjectBarStatus == MENU_OPEN) {
		// Conversation mode. Icons are highlighted on mouse-down, but
		// the actual response is made on mouse-up.
		if (menuType == MENU_BOT) {
			if (Logic::_scriptVars[OBJECT_HELD] && (mouseEvent & BS1L_BUTTON_UP)) {
				for (uint8 cnt = 0; cnt < Logic::_scriptVars[IN_SUBJECT]; cnt++) {
					if (_subjectBar[cnt] == Logic::_scriptVars[OBJECT_HELD])
						return cnt + 1;
				}
			} else if (mouseEvent & BS1L_BUTTON_DOWN) {
				for (uint8 cnt = 0; cnt < Logic::_scriptVars[IN_SUBJECT]; cnt++) {
					if (_subjects[cnt]->wasClicked(x, y)) {
						Logic::_scriptVars[OBJECT_HELD] = _subjectBar[cnt];
						refreshMenus();
						break;
					}
				}
			}
		} else {
			if (Logic::_scriptVars[OBJECT_HELD] && (mouseEvent & BS1L_BUTTON_UP)) {
				for (uint8 cnt = 0; cnt < _inMenu; cnt++) {
					if (_menuList[cnt] == Logic::_scriptVars[OBJECT_HELD])
						return cnt + 1;
				}
			} else if (mouseEvent & BS1L_BUTTON_DOWN) {
				for (uint8 cnt = 0; cnt < _inMenu; cnt++) {
					if (_objects[cnt]->wasClicked(x, y)) {
						Logic::_scriptVars[OBJECT_HELD] = _menuList[cnt];
						refreshMenus();
						break;
					}
				}
			}
		}
	} else {
		// Normal use, i.e. inventory. Things happen on mouse-down.
		if (menuType == MENU_TOP) {
			for (uint8 cnt = 0; cnt < _inMenu; cnt++) {
				if (_objects[cnt]->wasClicked(x, y)) {
					if (mouseEvent & BS1R_BUTTON_DOWN) { // looking at item
						Logic::_scriptVars[OBJECT_HELD] = _menuList[cnt];
						Logic::_scriptVars[MENU_LOOKING] = 1;
						Logic::_scriptVars[DEFAULT_ICON_TEXT] = _objectDefs[_menuList[cnt]].textDesc;
					} else if (mouseEvent & BS1L_BUTTON_DOWN) {
						if (Logic::_scriptVars[OBJECT_HELD]) {
							if (Logic::_scriptVars[OBJECT_HELD] == _menuList[cnt]) {
								_mouse->setLuggage(0, 0);
								Logic::_scriptVars[OBJECT_HELD] = 0; // reselected => deselect it
							} else { // the player is clicking another item on this one.
								// run its use-script, if there is one
								Logic::_scriptVars[SECOND_ITEM] = _menuList[cnt];
								_mouse->setLuggage(0, 0);
							}
						} else {
							Logic::_scriptVars[OBJECT_HELD] = _menuList[cnt];
							_mouse->setLuggage(_objectDefs[_menuList[cnt]].luggageIconRes, 0);
						}
					}
					refreshMenus();
					break;
				}
			}
		}
	}
	return 0;
}

void Menu::buildSubjects() {
	uint8 cnt;
	for (cnt = 0; cnt < 16; cnt++)
		if (_subjects[cnt]) {
			delete _subjects[cnt];
			_subjects[cnt] = NULL;
		}
	for (cnt = 0; cnt < Logic::_scriptVars[IN_SUBJECT]; cnt++) {
		uint32 res = _subjectList[(_subjectBar[cnt] & 65535) - BASE_SUBJECT].subjectRes;
		uint32 frame = _subjectList[(_subjectBar[cnt] & 65535) - BASE_SUBJECT].frameNo;
		_subjects[cnt] = new MenuIcon(MENU_BOT, cnt, res, frame, _screen);
		if (Logic::_scriptVars[OBJECT_HELD])
			_subjects[cnt]->setSelect(_subjectBar[cnt] == Logic::_scriptVars[OBJECT_HELD]);
		else
			_subjects[cnt]->setSelect(true);
	}
}

void Menu::refresh(uint8 menuType) {
	uint i;

	if (menuType == MENU_TOP) {
		if (_objectBarStatus == MENU_OPENING || _objectBarStatus == MENU_CLOSING) {
			for (i = 0; i < 16; i++) {
				if (_objects[i])
					_objects[i]->draw(_fadeEffectTop, _fadeObject);
				else
					_screen->showFrame(i * 40, 0, 0xffffffff, 0, _fadeEffectTop, _fadeObject);
			}
		}
		if (_objectBarStatus == MENU_OPENING) {
			if (_fadeObject < 8)
				_fadeObject++;
			else
				_objectBarStatus = MENU_OPEN;
		} else if (_objectBarStatus == MENU_CLOSING) {
			if (_fadeObject > 0)
				_fadeObject--;
			else {
				for (i = 0; i < _inMenu; i++) {
					delete _objects[i];
					_objects[i] = NULL;
				}
				_objectBarStatus = MENU_CLOSED;
			}
		}
	} else {
		if (_subjectBarStatus == MENU_OPENING || _subjectBarStatus == MENU_CLOSING) {
			for (i = 0; i < 16; i++) {
				if (_subjects[i])
					_subjects[i]->draw(_fadeEffectBottom, _fadeSubject);
				else
					_screen->showFrame(i * 40, 440, 0xffffffff, 0, _fadeEffectBottom, _fadeSubject);
			}
		}
		if (_subjectBarStatus == MENU_OPENING) {
			if (_fadeSubject < 8)
				_fadeSubject++;
			else
				_subjectBarStatus = MENU_OPEN;
		} else if (_subjectBarStatus == MENU_CLOSING) {
			if (_fadeSubject > 0)
				_fadeSubject--;
			else {
				for (i = 0; i < Logic::_scriptVars[IN_SUBJECT]; i++) {
					delete _subjects[i];
					_subjects[i] = NULL;
				}
				_subjectBarStatus = MENU_CLOSED;
			}
		}
	}
}

void Menu::buildMenu() {
	uint32 *pockets = Logic::_scriptVars + POCKET_1;
	for (uint8 cnt = 0; cnt < _inMenu; cnt++)
		if (_objects[cnt]) {
			delete _objects[cnt];
			_objects[cnt] = NULL;
		}
	_inMenu = 0;
	for (uint32 pocketNo = 0; pocketNo < TOTAL_pockets; pocketNo++)
		if (pockets[pocketNo]) {
			_menuList[_inMenu] = pocketNo + 1;
			_inMenu++;
		}
	for (uint32 menuSlot = 0; menuSlot < _inMenu; menuSlot++) {
		_objects[menuSlot] = new MenuIcon(MENU_TOP, menuSlot, _objectDefs[_menuList[menuSlot]].bigIconRes, _objectDefs[_menuList[menuSlot]].bigIconFrame, _screen);
		uint32 objHeld = Logic::_scriptVars[OBJECT_HELD];

		// check highlighting
		if (Logic::_scriptVars[MENU_LOOKING] || _subjectBarStatus == MENU_OPEN) { // either we're in the chooser or we're doing a 'LOOK AT'
			if ((!objHeld) || (objHeld == _menuList[menuSlot]))
				_objects[menuSlot]->setSelect(true);
		} else if (Logic::_scriptVars[SECOND_ITEM]) { // clicked luggage onto 2nd icon - we need to color-highlight the 2 relevant icons & grey out the rest
			if ((_menuList[menuSlot] == objHeld) || (_menuList[menuSlot] == Logic::_scriptVars[SECOND_ITEM]))
				_objects[menuSlot]->setSelect(true);
		} else { // this object is selected - ie. GREYED OUT
			if (objHeld != _menuList[menuSlot])
				_objects[menuSlot]->setSelect(true);
		}
	}
}

void Menu::showMenu(uint8 menuType) {
	if (menuType == MENU_TOP) {
		if (_objectBarStatus == MENU_OPEN) {
			for (uint8 cnt = 0; cnt < 16; cnt++) {
				if (_objects[cnt])
					_objects[cnt]->draw();
				else
					_screen->showFrame(cnt * 40, 0, 0xffffffff, 0);
			}
		} else if (_objectBarStatus == MENU_CLOSED) {
			_objectBarStatus = MENU_OPENING;
			_fadeObject = 0;
		} else if (_objectBarStatus == MENU_CLOSING)
			_objectBarStatus = MENU_OPENING;
	}
}

void Menu::fnStartMenu() {
	Logic::_scriptVars[OBJECT_HELD]  = 0; // icon no longer selected
	Logic::_scriptVars[SECOND_ITEM]  = 0; // second icon no longer selected (after using one on another)
	Logic::_scriptVars[MENU_LOOKING] = 0; // no longer 'looking at' an icon
	buildMenu();
	showMenu(MENU_TOP);
}

void Menu::fnEndMenu() {
	if (_objectBarStatus != MENU_CLOSED)
		_objectBarStatus = MENU_CLOSING;
}

void Menu::fnChooser(Object *compact) {
	Logic::_scriptVars[OBJECT_HELD] = 0;
	_mouse->setLuggage(0, 0);
	buildSubjects();
	compact->o_logic = LOGIC_choose;
	_mouse->controlPanel(true); // so the mouse cursor will be shown.
	_subjectBarStatus = MENU_OPENING;
}

void Menu::fnEndChooser() {
	Logic::_scriptVars[OBJECT_HELD] = 0;
	_subjectBarStatus = MENU_CLOSING;
	_objectBarStatus = MENU_CLOSING;
	_mouse->controlPanel(false);
	_mouse->setLuggage(0, 0);
}

void Menu::checkTopMenu() {
	if (_objectBarStatus == MENU_OPEN)
		checkMenuClick(MENU_TOP);
}

int Menu::logicChooser(Object *compact) {
	uint8 objSelected = 0;
	if (_objectBarStatus == MENU_OPEN)
		objSelected = checkMenuClick(MENU_TOP);
	if (!objSelected)
		objSelected = checkMenuClick(MENU_BOT);
	if (objSelected) {
		compact->o_logic = LOGIC_script;
		return 1;
	}
	return 0;
}

void Menu::fnAddSubject(int32 sub) {
	_subjectBar[Logic::_scriptVars[IN_SUBJECT]] = sub;
	Logic::_scriptVars[IN_SUBJECT]++;
}

void Menu::cfnReleaseMenu() {
	_objectBarStatus = MENU_CLOSING;
}

} // End of namespace Sword1
