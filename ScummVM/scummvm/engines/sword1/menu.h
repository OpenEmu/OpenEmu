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

#ifndef SWORD1_MENU_H
#define SWORD1_MENU_H

#include "sword1/sworddefs.h"
#include "sword1/object.h"

namespace Sword1 {

class Screen;
class Mouse;
class ResMan;

#define MENU_TOP 0
#define MENU_BOT 1

struct Subject {
	uint32 subjectRes;
	uint32 frameNo;
};

struct MenuObject {
	int32 textDesc;
	uint32 bigIconRes;
	uint32 bigIconFrame;
	uint32 luggageIconRes;
	uint32 useScript;
};

class MenuIcon {
public:
	MenuIcon(uint8 menuType, uint8 menuPos, uint32 resId, uint32 frame, Screen *screen);
	bool wasClicked(uint16 mouseX, uint16 mouseY);
	void setSelect(bool pSel);
	void draw(const byte *fadeMask = NULL, int8 fadeStatus = 0);

private:
	uint8 _menuType, _menuPos;
	uint32 _resId, _frame;
	bool _selected;
	Screen *_screen;
};

class Menu {
public:
	Menu(Screen *pScreen, Mouse *pMouse);
	~Menu();
	void fnChooser(Object *compact);
	void fnEndChooser();
	void fnAddSubject(int32 sub);
	void cfnReleaseMenu();
	int logicChooser(Object *compact);
	void engine();
	void refresh(uint8 menuType);
	void fnStartMenu();
	void fnEndMenu();
	void checkTopMenu();
	static const MenuObject _objectDefs[TOTAL_pockets + 1];

private:
	void buildSubjects();
	void buildMenu();
	void showMenu(uint8 menuType);
	byte _subjectBarStatus;
	byte _objectBarStatus;
	int8 _fadeSubject;
	int8 _fadeObject;
	void refreshMenus();
	uint8 checkMenuClick(uint8 menuType);
	//- lower menu, speech subjects:
	MenuIcon *_subjects[16];
	uint32 _subjectBar[16];
	//- top menu, items
	MenuIcon *_objects[TOTAL_pockets];
	uint32 _menuList[TOTAL_pockets];
	uint32 _inMenu;

	Screen *_screen;
	Mouse *_mouse;
	static const Subject _subjectList[TOTAL_subjects];

	static const byte _fadeEffectTop[64];
	static const byte _fadeEffectBottom[64];
};

} // End of namespace Sword1

#endif //BSMENU_H
