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

#ifndef KYRA_GUI_V2_H
#define KYRA_GUI_V2_H

#include "kyra/gui_v1.h"

namespace Kyra {

#define GUI_V2_BUTTON(button, a, b, c, d, e, f, h, i, j, k, l, m, n, o, p, q, r, s, t) \
	do { \
		button.nextButton = 0; \
		button.index = a; \
		button.keyCode = b; \
		button.keyCode2 = c; \
		button.data0Val1 = d; \
		button.data1Val1 = e; \
		button.data2Val1 = f; \
		button.flags = h; \
		button.data0ShapePtr = button.data1ShapePtr = button.data2ShapePtr = 0; \
		button.dimTableIndex = i; \
		button.x = j; \
		button.y = k; \
		button.width = l; \
		button.height = m; \
		button.data0Val2 = n; \
		button.data0Val3 = o; \
		button.data1Val2 = p; \
		button.data1Val3 = q; \
		button.data2Val2 = r; \
		button.data2Val3 = s; \
		button.flags2 = t; \
		button.mouseWheel = 0; \
		button.arg = 0; \
	} while (0)

#define GUI_V2_MENU(menu, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q) \
	do { \
		menu.x = a; \
		menu.y = b; \
		menu.width = c; \
		menu.height = d; \
		menu.bkgdColor = e; \
		menu.color1 = f; \
		menu.color2 = g; \
		menu.menuNameId = h; \
		menu.textColor = i; \
		menu.titleX = j; \
		menu.titleY = k; \
		menu.highlightedItem = l; \
		menu.numberOfItems = m; \
		menu.scrollUpButtonX = n; \
		menu.scrollUpButtonY = o; \
		menu.scrollDownButtonX = p; \
		menu.scrollDownButtonY = q; \
	 } while (0)

#define GUI_V2_MENU_ITEM(item, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q) \
	do { \
		item.enabled = a; \
		item.itemId = b; \
		item.x = c; \
		item.y = d; \
		item.width = e; \
		item.height = f; \
		item.textColor = g; \
		item.highlightColor = h; \
		item.titleX = i; \
		item.bkgdColor = j; \
		item.color1 = k; \
		item.color2 = l; \
		item.saveSlot = m; \
		item.labelId = n; \
		item.labelX = o; \
		item.labelY = p; \
		item.keyCode = q; \
	} while (0)

class KyraEngine_v2;
class Screen_v2;

class GUI_v2 : public GUI_v1 {
public:
	GUI_v2(KyraEngine_v2 *vm);

	virtual void initStaticData() = 0;

	Button *addButtonToList(Button *list, Button *newButton);

	void processButton(Button *button);
	int processButtonList(Button *button, uint16 inputFlag, int8 mouseWheel);

protected:
	void updateButton(Button *button);

	KyraEngine_v2 *_vm;
	Screen_v2 *_screen;

	bool _buttonListChanged;
	Button *_backUpButtonList;
	Button *_specialProcessButton;
	uint16 _flagsModifier;

protected:
	virtual void setupPalette() {}
	virtual void restorePalette() {}

	virtual char *getTableString(int id) = 0;

	virtual uint8 textFieldColor1() const = 0;
	virtual uint8 textFieldColor2() const = 0;
	virtual uint8 textFieldColor3() const = 0;
protected:
	virtual void getInput();

	Button _menuButtons[7];
	Button _scrollUpButton;
	Button _scrollDownButton;
	Menu _mainMenu, _gameOptions, _audioOptions, _choiceMenu, _loadMenu, _saveMenu, _savenameMenu, _deathMenu;

	Button *getButtonListData() { return _menuButtons; }

	Button *getScrollUpButton() { return &_scrollUpButton; }
	Button *getScrollDownButton() { return &_scrollDownButton; }

	int scrollUpButton(Button *button);
	int scrollDownButton(Button *button);
	Button::Callback _scrollUpFunctor;
	Button::Callback _scrollDownFunctor;
	Button::Callback getScrollUpButtonHandler() const { return _scrollUpFunctor; }
	Button::Callback getScrollDownButtonHandler() const { return _scrollDownFunctor; }

	Button _sliderButtons[3][4];

	void renewHighlight(Menu &menu);

	void backUpPage1(uint8 *buffer);
	void restorePage1(const uint8 *buffer);

	Menu *_currentMenu;
	bool _isLoadMenu;
	bool _isDeathMenu;
	bool _isSaveMenu;
	bool _isDeleteMenu;
	bool _isChoiceMenu;
	bool _isOptionsMenu;
	bool _madeSave;
	bool _loadedSave;
	bool _restartGame;
	bool _reloadTemporarySave;

	int _savegameOffset;

	void setupSavegameNames(Menu &menu, int num);

	// main menu
	int resumeGame(Button *caller);

	// audio menu
	static const int _sliderBarsPosition[];

	// load menu
	bool _noLoadProcess;
	int clickLoadSlot(Button *caller);
	int cancelLoadMenu(Button *caller);

	// save menu
	bool _noSaveProcess;
	int _saveSlot;
	char _saveDescription[0x51];

	int saveMenu(Button *caller);
	int clickSaveSlot(Button *caller);
	int cancelSaveMenu(Button *caller);

	// delete menu
	int _slotToDelete;
	int deleteMenu(Button *caller);

	// options menu
	int quitOptionsMenu(Button *caller);

	int toggleWalkspeed(Button *caller);
	int toggleText(Button *caller);

	virtual void setupOptionsButtons() = 0;

	// audio options
	Button::Callback _sliderHandlerFunctor;
	virtual int sliderHandler(Button *caller) = 0;

	// savename menu
	bool _finishNameInput, _cancelNameInput;

	const char *nameInputProcess(char *buffer, int x, int y, uint8 c1, uint8 c2, uint8 c3, int bufferSize);
	int finishSavename(Button *caller);
	int cancelSavename(Button *caller);

	bool checkSavegameDescription(const char *buffer, int size);
	int getCharWidth(uint8 c);
	void drawTextfieldBlock(int x, int y, uint8 c);

	// choice menu
	bool _choice;

	bool choiceDialog(int name, bool type);
	int choiceYes(Button *caller);
	int choiceNo(Button *caller);
};

} // End of namespace Kyra

#endif
