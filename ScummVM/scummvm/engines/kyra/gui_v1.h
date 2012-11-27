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

#ifndef KYRA_GUI_V1_H
#define KYRA_GUI_V1_H

#include "kyra/gui.h"

namespace Kyra {

struct MenuItem {
	bool enabled;

	const char *itemString;
	uint16 itemId;

	int16 x, y;
	uint16 width, height;

	uint8 textColor, highlightColor;

	int16 titleX;

	uint8 color1, color2;
	uint8 bkgdColor;

	Button::Callback callback;

	int16 saveSlot;

	const char *labelString;
	uint16 labelId;
	int16 labelX, labelY;

	uint16 keyCode;
};

struct Menu {
	int16 x, y;
	uint16 width, height;

	uint8 bkgdColor;
	uint8 color1, color2;

	const char *menuNameString;
	uint16 menuNameId;

	uint8 textColor;
	int16 titleX, titleY;

	uint8 highlightedItem;

	uint8 numberOfItems;

	int16 scrollUpButtonX, scrollUpButtonY;
	int16 scrollDownButtonX, scrollDownButtonY;

	MenuItem item[7];
};

class TextDisplayer;

class GUI_v1 : public GUI {
public:
	GUI_v1(KyraEngine_v1 *vm);
	virtual ~GUI_v1() {}

	// button specific
	virtual Button *addButtonToList(Button *list, Button *newButton);

	virtual void processButton(Button *button) = 0;
	virtual int processButtonList(Button *buttonList, uint16 inputFlags, int8 mouseWheel) = 0;

	virtual int redrawShadedButtonCallback(Button *button);
	virtual int redrawButtonCallback(Button *button);

	// menu specific
	virtual void initMenuLayout(Menu &menu);
	void initMenu(Menu &menu);

	void processHighlights(Menu &menu);

	// utilities for thumbnail creation
	virtual void createScreenThumbnail(Graphics::Surface &dst) = 0;

protected:
	TextDisplayer *_text;

	Button *_menuButtonList;
	bool _displayMenu;
	bool _displaySubMenu;
	bool _cancelSubMenu;

	virtual void printMenuText(const char *str, int x, int y, uint8 c0, uint8 c1, uint8 c2);
	virtual int getMenuCenterStringX(const char *str, int x1, int x2);

	Button::Callback _redrawShadedButtonFunctor;
	Button::Callback _redrawButtonFunctor;

	virtual Button *getButtonListData() = 0;
	virtual Button *getScrollUpButton() = 0;
	virtual Button *getScrollDownButton() = 0;

	virtual Button::Callback getScrollUpButtonHandler() const = 0;
	virtual Button::Callback getScrollDownButtonHandler() const = 0;

	virtual uint8 defaultColor1() const = 0;
	virtual uint8 defaultColor2() const = 0;

	virtual const char *getMenuTitle(const Menu &menu) = 0;
	virtual const char *getMenuItemTitle(const MenuItem &menuItem) = 0;
	virtual const char *getMenuItemLabel(const MenuItem &menuItem) = 0;

	void updateAllMenuButtons();
	void updateMenuButton(Button *button);
	virtual void updateButton(Button *button);

	void redrawText(const Menu &menu);
	void redrawHighlight(const Menu &menu);

	uint32 _lastScreenUpdate;
	void checkTextfieldInput();
};

class Movie;

class MainMenu {
public:
	MainMenu(KyraEngine_v1 *vm);
	virtual ~MainMenu() {}

	struct Animation {
		Animation() : anim(0), startFrame(0), endFrame(0), delay(0) {}

		Movie *anim;
		int startFrame;
		int endFrame;
		int delay;
	};

	struct StaticData {
		const char *strings[5];

		uint8 menuTable[7];
		uint8 colorTable[4];

		Screen::FontId font;
		uint8 altColor;
	};

	void init(StaticData data, Animation anim);
	int handle(int dim);
private:
	KyraEngine_v1 *_vm;
	Screen *_screen;
	OSystem *_system;

	StaticData _static;
	struct AnimIntern {
		int curFrame;
		int direction;
	};
	Animation _anim;
	AnimIntern _animIntern;

	uint32 _nextUpdate;

	void updateAnimation();
	void draw(int select);
	void drawBox(int x, int y, int w, int h, int fill);
	bool getInput();

	void printString(const char *string, int x, int y, int col1, int col2, int flags, ...) GCC_PRINTF(2, 8);
};

} // end of namesapce Kyra

#endif
