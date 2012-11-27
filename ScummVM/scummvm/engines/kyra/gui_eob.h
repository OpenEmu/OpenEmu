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

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

#ifndef KYRA_GUI_EOB_H
#define KYRA_GUI_EOB_H

#include "kyra/gui.h"

#ifdef ENABLE_EOB

namespace Kyra {

struct EoBRect16 {
	int16 x1;
	int16 y1;
	uint16 x2;
	uint16 y2;
};

class DarkMoonEngine;
class Screen_EoB;

class GUI_EoB : public GUI {
	friend class EoBCoreEngine;
	friend class CharacterGenerator;
public:
	GUI_EoB(EoBCoreEngine *vm);
	virtual ~GUI_EoB();

	void initStaticData() {}

	// button specific
	void processButton(Button *button);
	int processButtonList(Button *buttonList, uint16 inputFlags, int8 mouseWheel);

	// Non button based menu handling (main menu, character generation)
	void simpleMenu_setup(int sd, int maxItem, const char *const *strings, int32 menuItemsMask, int unk, int lineSpacing);
	int simpleMenu_process(int sd, const char *const *strings, void *b, int32 menuItemsMask, int unk);

	// Button based menus (camp menu, load menu)
	void runCampMenu();
	bool runLoadMenu(int x, int y);

	bool confirmDialogue2(int dim, int id, int deflt);
	void messageDialogue(int dim, int id, int buttonTextCol);
	void messageDialogue2(int dim, int id, int buttonTextCol);

	void updateBoxFrameHighLight(int box);

	int getTextInput(char *dest, int x, int y, int destMaxLen, int textColor1, int textColor2, int cursorColor);

	// Transfer party
	void transferWaitBox();
	Common::String transferTargetMenu(Common::Array<Common::String> &targets);
	bool transferFileMenu(Common::String &targetName, Common::String &selection);

	// utilities for thumbnail creation
	void createScreenThumbnail(Graphics::Surface &dst);

private:
	int simpleMenu_getMenuItem(int index, int32 menuItemsMask, int itemOffset);
	void simpleMenu_flashSelection(const char *str, int x, int y, int color1, int color2, int color3);
	void simpleMenu_initMenuItemsMask(int menuId, int maxItem, int32 menuItemsMask, int unk);

	bool runSaveMenu(int x, int y);
	int selectSaveSlotDialogue(int x, int y, int id);
	void runMemorizePrayMenu(int charIndex, int spellType);
	void scribeScrollDialogue();
	bool restParty();

	bool confirmDialogue(int id);
	int selectCharacterDialogue(int id);
	void displayTextBox(int id);

	Button *initMenu(int id);
	void drawMenuButton(Button *b, bool clicked, bool highlight, bool noFill);
	void drawMenuButtonBox(int x, int y, int w, int h, bool clicked, bool noFill);
	void drawTextBox(int dim, int id);
	void drawSaveSlotButton(int slot, int redrawBox, int textCol);
	void memorizePrayMenuPrintString(int spellId, int bookPageIndex, int spellType, bool noFill, bool highLight);
	void updateOptionsStrings();
	const char *getMenuString(int id);

	Button *linkButton(Button *list, Button *newbt);
	void releaseButtons(Button *list);

	void setupSaveMenuSlots();
	int getHighlightSlot();
	void sortSaveSlots();

	void restParty_updateRestTime(int hours, bool init);

	char **_menuStringsPrefsTemp;
	char **_saveSlotStringsTemp;
	int16 *_saveSlotIdTemp;
	int _savegameOffset;
	int16 _saveSlotX;
	int16 _saveSlotY;

	EoBCoreEngine *_vm;
	Screen_EoB *_screen;

	bool _pressFlag;

	Button *_specialProcessButton;
	Button *_backupButtonList;
	uint16 _flagsMouseLeft;
	uint16 _flagsMouseRight;
	uint16 _flagsModifier;
	uint16 _progress;
	uint16 _prcButtonUnk3;
	uint16 _cflag;

	int _menuLineSpacing;
	int _menuLastInFlags;

	uint8 _numPages;
	uint8 _numVisPages;
	int8 *_numAssignedSpellsOfType;
	uint32 _clericSpellAvltyFlags;
	uint32 _paladinSpellAvltyFlags;
	bool _needRest;

	int _menuCur;
	int _menuNumItems;
	bool _charSelectRedraw;

	int _updateBoxIndex;
	int _updateBoxColorIndex;
	const uint8 *_highLightColorTable;
	uint32 _highLightBoxTimer;

	static const EoBRect16 _highlightFrames[];
	static const uint8 _highlightColorTableVGA[];
	static const uint8 _highlightColorTableEGA[];
};

} // End of namespace Kyra

#endif // ENABLE_EOB

#endif

#endif // ENABLE_EOB || ENABLE_LOL
