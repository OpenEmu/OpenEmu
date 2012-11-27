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

// Game interface module private header file

#ifndef SAGA_INTERFACE_H
#define SAGA_INTERFACE_H

#include "common/keyboard.h"
#include "common/savefile.h"

#include "saga/displayinfo.h"
#include "saga/sprite.h"
#include "saga/script.h"

namespace Saga {

enum InterfaceUpdateFlags {
	UPDATE_MOUSEMOVE = 1,
	UPDATE_LEFTBUTTONCLICK = 2,
	UPDATE_RIGHTBUTTONCLICK = 4,
	UPDATE_MOUSECLICK = UPDATE_LEFTBUTTONCLICK | UPDATE_RIGHTBUTTONCLICK,
	UPDATE_WHEELUP = 8,
	UPDATE_WHEELDOWN = 16
};

#define CONVERSE_MAX_TEXTS 64
#define CONVERSE_MAX_WORK_STRING 256

#define ITE_INVENTORY_SIZE 24

#define VERB_STRLIMIT 32

#define STATUS_TEXT_LEN 128
#define STATUS_TEXT_INPUT_MAX 256

#define RID_IHNM_BOSS_SCREEN 19		// not in demo
#define RID_ITE_TYCHO_MAP 1686
#define RID_ITE_SPR_CROSSHAIR (73 + 9)
#define TIMETOSAVE (1000000 * 60 * 30) // 30 minutes
#define TIMETOBLINK_ITE	(1000000 * 1)
#define TIMETOBLINK_IHNM	(1000000 / 10)

// Converse-specific stuff

enum PanelModes {
	kPanelNull,
	kPanelMain,
	kPanelOption,
	kPanelSave, //ex- kPanelTextBox,
	kPanelQuit,
	kPanelError,
	kPanelLoad,
	kPanelConverse,
	kPanelProtect,
	kPanelPlacard,
	kPanelMap,
	kPanelSceneSubstitute,
	kPanelChapterSelection,
	kPanelCutaway,
	kPanelVideo,
	kPanelBoss
//	kPanelInventory
};

enum FadeModes {
	kNoFade = 0,
	kFadeIn,
	kFadeOut
};

struct InterfacePanel {
	int x;
	int y;
	ByteArray image;
	int imageWidth;
	int imageHeight;

	PanelButton *currentButton;
	int buttonsCount;
	PanelButton *buttons;
	SpriteList sprites;

	InterfacePanel() {
		x = y = 0;
		imageWidth = imageHeight = 0;
		currentButton = NULL;
		buttonsCount = 0;
		buttons = NULL;
	}

	PanelButton *getButton(int index) {
		if ((index >= 0) && (index < buttonsCount)) {
			return &buttons[index];
		}
		return NULL;
	}

	void getRect(Rect &rect) {
		rect.left = x;
		rect.top = y;
		rect.setWidth(imageWidth);
		rect.setHeight(imageHeight);
	}

	void calcPanelButtonRect(const PanelButton* panelButton, Rect &rect) {
		rect.left = x + panelButton->xOffset;
		rect.right = rect.left + panelButton->width;
		rect.top = y + panelButton->yOffset;
		rect.bottom = rect.top + panelButton->height;
	}

	PanelButton *hitTest(const Point& mousePoint, int buttonType) {
		PanelButton *panelButton;
		Rect rect;
		int i;
		for (i = 0; i < buttonsCount; i++) {
			panelButton = &buttons[i];
			if (panelButton != NULL) {
				if ((panelButton->type & buttonType) > 0) {
					calcPanelButtonRect(panelButton, rect);
					if (rect.contains(mousePoint)) {
						return panelButton;
					}
				}
			}
		}
		return NULL;
	}

	void zeroAllButtonState() {
		int i;
		for (i = 0; i < buttonsCount; i++) {
			buttons[i].state = 0;
		}
	}


};

struct Converse {
	Common::Array<char> text;
	int strId;
	int stringNum;
	int textNum;
	int replyId;
	int replyFlags;
	int replyBit;
};


enum StatusTextInputState {
	kStatusTextInputFirstRun,
	kStatusTextInputEntered,
	kStatusTextInputAborted
};

class Interface {
public:
	Interface(SagaEngine *vm);
	~Interface();

	int activate();
	int deactivate();
	void setSaveReminderState(int state) {
		_saveReminderState = state;
		draw();
	}
	int getSaveReminderState() {
		return _saveReminderState;
	}
	bool isActive() { return _active; }
	void setMode(int mode);
	int getMode() const { return _panelMode; }
	void setFadeMode(int fadeMode) {
		_fadeMode = fadeMode;
		draw();
	}
	int getFadeMode() const {
		return _fadeMode;
	}
	void rememberMode();
	void restoreMode(bool draw_ = true);
	bool isInMainMode() { return _inMainMode; }
	void setStatusText(const char *text, int statusColor = -1);
	void loadScenePortraits(int resourceId);
	void setLeftPortrait(int portrait) {
		_leftPortrait = portrait;
		draw();
	}
	void setRightPortrait(int portrait) {
		_rightPortrait = portrait;
		draw();
	}
	void setPortraitBgColor(int red, int green, int blue) {
		_portraitBgColor.red = red;
		_portraitBgColor.green = green;
		_portraitBgColor.blue = blue;
	}

	void draw();
	void drawOption();
	void drawQuit();
	void drawLoad();
	void drawSave();
	void drawProtect();
	void update(const Point& mousePoint, int updateFlag);
	void drawStatusBar();
	void setVerbState(int verb, int state);

	bool processAscii(Common::KeyState keystate);

	void keyBoss();
	void keyBossExit();

	void disableAbortSpeeches(bool d) { _disableAbortSpeeches = d; }

	static void saveReminderCallback(void *refCon);
	void updateSaveReminder();

	bool _textInput;

	bool _statusTextInput;
	StatusTextInputState _statusTextInputState;
	char _statusTextInputString[STATUS_TEXT_INPUT_MAX];
	void enterStatusString() {
		_statusTextInput = true;
		_statusTextInputPos = 0;
		_statusTextInputString[0] = 0;
		setStatusText(_statusTextInputString);
	}

private:
	void drawInventory();
	void updateInventory(int pos);
	void inventoryChangePos(int chg);
	void inventorySetPos(int key);

public:
	void refreshInventory() {
		updateInventory(_inventoryCount);
		draw();
	}
	void addToInventory(int objectId);
	void removeFromInventory(int objectId);
	void clearInventory();
	int inventoryItemPosition(int objectId);
	int getInventoryContentByPanelButton(PanelButton * panelButton) {
		int cell = _inventoryStart + panelButton->id;
		if (cell >= _inventoryCount) {
			return 0;
		}
		return _inventory[cell];
	}

	PanelButton *inventoryHitTest(const Point& mousePoint) {
		return _mainPanel.hitTest(mousePoint, kPanelButtonInventory);
	}
	PanelButton *verbHitTest(const Point& mousePoint){
		return _mainPanel.hitTest(mousePoint, kPanelButtonVerb);
	}
	void saveState(Common::OutSaveFile *out);
	void loadState(Common::InSaveFile *in);

	void mapPanelDrawCrossHair();

	int32 getProtectHash() { return _protectHash; }

	void resetSaveReminder();

private:
	void handleMainUpdate(const Point& mousePoint);					// main panel update
	void handleMainClick(const Point& mousePoint);					// main panel click

	PanelButton *converseHitTest(const Point& mousePoint) {
		return _conversePanel.hitTest(mousePoint, kPanelAllButtons);
	}
	void handleConverseUpdate(const Point& mousePoint);				// converse panel update
	void handleConverseClick(const Point& mousePoint);				// converse panel click

	PanelButton *optionHitTest(const Point& mousePoint) {
		return _optionPanel.hitTest(mousePoint, kPanelButtonOptionSaveFiles | kPanelButtonOption | kPanelButtonOptionSlider);
	}
	void handleOptionUpdate(const Point& mousePoint);				// option panel update
	void handleOptionClick(const Point& mousePoint);				// option panel click

	PanelButton *quitHitTest(const Point& mousePoint) {
		return _quitPanel.hitTest(mousePoint, kPanelAllButtons);
	}
	void handleQuitUpdate(const Point& mousePoint);					// quit panel update
	void handleQuitClick(const Point& mousePoint);					// quit panel click

	PanelButton *loadHitTest(const Point& mousePoint) {
		return _loadPanel.hitTest(mousePoint, kPanelAllButtons);
	}
	void handleLoadUpdate(const Point& mousePoint);					// load panel update
	void handleLoadClick(const Point& mousePoint);					// load panel click

	PanelButton *saveHitTest(const Point& mousePoint) {
		return _savePanel.hitTest(mousePoint, kPanelAllButtons);
	}
	void handleSaveUpdate(const Point& mousePoint);					// save panel update
	void handleSaveClick(const Point& mousePoint);					// save panel click

	void handleChapterSelectionUpdate(const Point& mousePoint);
	void handleChapterSelectionClick(const Point& mousePoint);

	void mapPanelShow();
	void mapPanelClean();

	void lockMode() { _lockedMode = _panelMode; }
	void unlockMode() { _panelMode = _lockedMode; }

	void setOption(PanelButton *panelButton);
	void setQuit(PanelButton *panelButton);
	void setLoad(PanelButton *panelButton);
	void setSave(PanelButton *panelButton);

	void drawTextInput(InterfacePanel *panel, PanelButton *panelButton);
	void drawPanelText(InterfacePanel *panel, PanelButton *panelButton);
	void drawPanelButtonText(InterfacePanel *panel, PanelButton *panelButton, int spritenum = 0);
	enum ButtonKind {
		kButton,
		kSlider,
		kEdit
	};
	void drawButtonBox(const Rect &rect, ButtonKind kind, bool down);
	void drawPanelButtonArrow(InterfacePanel *panel, PanelButton *panelButton);
	void drawVerbPanelText(PanelButton *panelButton, KnownColor textKnownColor, KnownColor textShadowKnownColor);
	void drawVerbPanel(PanelButton* panelButton);
	void calcOptionSaveSlider();
	bool processTextInput(Common::KeyState keystate);
	void processStatusTextInput(Common::KeyState keystate);

public:
	void converseClear();
	bool converseAddText(const char *text, int strId, int replyId, byte replyFlags, int replyBit);
	void converseDisplayText();
	void converseSetTextLines(int row);
	void converseChangePos(int chg);
	void converseSetPos(int key);

private:
	void converseDisplayTextLines();
	PanelButton *getPanelButtonByVerbType(int verb) {
		if ((verb < 0) || (verb >= kVerbTypeIdsMax)) {
			error("Interface::getPanelButtonByVerbType wrong verb");
		}
		return _verbTypeToPanelButton[verb];
	}

	void validateOptionButtons() {
		if (!_vm->isSaveListFull() && (_optionSaveFileTitleNumber == 0) && (_optionPanel.currentButton != NULL)) {
			if (_optionPanel.currentButton->id == kTextLoad) {
				_optionPanel.currentButton = NULL;
			}
		}
	}
	void validateSaveButtons() {
		if ((_textInputStringLength == 0) && (_savePanel.currentButton != NULL)) {
			if (_savePanel.currentButton->id == kTextSave) {
				_savePanel.currentButton = NULL;
			}
		}
	}

public:
	SpriteList _defPortraits;
	PalEntry _portraitBgColor;

private:
	SagaEngine *_vm;

	ResourceContext *_interfaceContext;
	InterfacePanel _mainPanel;
	PanelButton *_inventoryUpButton;
	PanelButton *_inventoryDownButton;
	InterfacePanel _conversePanel;
	PanelButton *_converseUpButton;
	PanelButton *_converseDownButton;
	SpriteList _scenePortraits;
	PanelButton *_verbTypeToPanelButton[kVerbTypeIdsMax];
	InterfacePanel _optionPanel;
	PanelButton * _optionSaveFileSlider;
	PanelButton * _optionSaveFilePanel;
	InterfacePanel _quitPanel;
	InterfacePanel _loadPanel;
	InterfacePanel _savePanel;
	PanelButton * _saveEdit;
	InterfacePanel _protectPanel;
	PanelButton * _protectEdit;

	bool _disableAbortSpeeches;

	int _saveReminderState;
	bool _active;
	int _fadeMode;
	int _panelMode;
	int _savedMode;
	int _lockedMode;
	int _bossMode;
	bool _inMainMode;
	char _statusText[STATUS_TEXT_LEN];
	int _statusOnceColor;
	int _leftPortrait;
	int _rightPortrait;

	Point _lastMousePoint;

	Common::Array<uint16> _inventory;
	int _inventoryStart;
	int _inventoryEnd;
	int _inventoryPos;
	int _inventoryBox;
	int _inventoryCount;

	char _converseWorkString[CONVERSE_MAX_WORK_STRING];
	Converse _converseText[CONVERSE_MAX_TEXTS];
	int _converseTextCount;
	int _converseStrCount;
	int _converseStartPos;
	int _converseEndPos;
	int _conversePos;

	uint _optionSaveFileTop;
	uint _optionSaveFileTitleNumber;
	int16 _optionSaveFileMouseOff;
	Rect _optionSaveRectTop;
	Rect _optionSaveRectSlider;
	Rect _optionSaveRectBottom;

	char _textInputString[SAVE_TITLE_SIZE];
	uint _textInputStringLength;
	uint _textInputPos;
	uint _textInputMaxWidth;

	uint _statusTextInputPos;

	PalEntry _mapSavedPal[PAL_ENTRIES];
	bool _mapPanelCrossHairState;

	int32 _protectHash;
};

} // End of namespace Saga

#endif
