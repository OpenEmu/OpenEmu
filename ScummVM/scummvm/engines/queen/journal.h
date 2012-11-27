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

#ifndef QUEEN_JOURNAL_H
#define QUEEN_JOURNAL_H

#include "common/util.h"

class OSystem;

namespace Queen {

class QueenEngine;

class Journal {
public:

	Journal(QueenEngine *vm);
	void use();

	enum {
		JOURNAL_BANK   = 8,
		JOURNAL_FRAMES = 40
	};

	enum {
		ZN_REVIEW_ENTRY = 1,
		ZN_MAKE_ENTRY   = 2,
		ZN_YES          = ZN_MAKE_ENTRY,
		ZN_CLOSE        = 3,
		ZN_NO           = ZN_CLOSE,
		ZN_GIVEUP       = 4,
		ZN_TEXT_SPEED   = 5,
		ZN_SFX_TOGGLE   = 6,
		ZN_MUSIC_VOLUME = 7,
		ZN_DESC_1       = 8,
		ZN_DESC_2       = 9,
		ZN_DESC_3       = 10,
		ZN_DESC_4       = 11,
		ZN_DESC_5       = 12,
		ZN_DESC_6       = 13,
		ZN_DESC_7       = 14,
		ZN_DESC_8       = 15,
		ZN_DESC_9       = 16,
		ZN_DESC_10      = 17,
		ZN_PAGE_A       = 18,
		ZN_PAGE_B       = 19,
		ZN_PAGE_C       = 20,
		ZN_PAGE_D       = 21,
		ZN_PAGE_E       = 22,
		ZN_PAGE_F       = 23,
		ZN_PAGE_G       = 24,
		ZN_PAGE_H       = 25,
		ZN_PAGE_I       = 26,
		ZN_PAGE_J       = 27,
		ZN_INFO_BOX     = 28,
		ZN_MUSIC_TOGGLE = 29,
		ZN_VOICE_TOGGLE = 30,
		ZN_TEXT_TOGGLE  = 31
	};

	enum {
		BOB_LEFT_RECT_1   =  1,
		BOB_LEFT_RECT_2   =  2,
		BOB_LEFT_RECT_3   =  3,
		BOB_LEFT_RECT_4   =  4,
		BOB_TALK_SPEED    =  5,
		BOB_SFX_TOGGLE    =  6,
		BOB_MUSIC_VOLUME  =  7,
		BOB_SAVE_DESC     =  8,
		BOB_SAVE_PAGE     =  9,
		BOB_SPEECH_TOGGLE = 10,
		BOB_TEXT_TOGGLE   = 11,
		BOB_MUSIC_TOGGLE  = 12,
		BOB_INFO_BOX      = 13
	};

	enum {
		FRAME_BLUE_1    =  1,
		FRAME_BLUE_2    =  2,
		FRAME_ORANGE    =  3,
		FRAME_GREY      =  5,
		FRAME_CHECK_BOX = 16,
		FRAME_BLUE_PIN  = 18,
		FRAME_GREEN_PIN = 19,
		FRAME_INFO_BOX  = 20
	};

	enum {
		TXT_CLOSE        = 30,
		TXT_GIVE_UP      = 31,
		TXT_MAKE_ENTRY   = 32,
		TXT_REVIEW_ENTRY = 33,
		TXT_YES          = 34,
		TXT_NO           = 35
	};

	enum {
		NUM_SAVES_PER_PAGE = 10,
		MAX_PANEL_TEXTS    = 8,
		MAX_ZONES          = 31
	};

	enum PanelMode {
		PM_NORMAL,
		PM_INFO_BOX,
		PM_YES_NO
	};

	enum QuitMode {
		QM_LOOP,
		QM_RESTORE,
		QM_CONTINUE
	};


private:

	void continueGame();

	void setup();
	void redraw();
	void update();

	void showBob(int bobNum, int16 x, int16 y, int frameNum);
	void hideBob(int bobNum);

	void drawSaveDescriptions();
	void drawSaveSlot();

	void enterYesNoPanelMode(int16 prevZoneNum, int titleNum);
	void exitYesNoPanelMode();
	void enterInfoPanelMode();
	void exitInfoPanelMode();

	void handleMouseWheel(int inc);
	void handleMouseDown(int x, int y);
	void handleKeyDown(uint16 ascii, int keycode);

	void drawPanelText(int y, const char *text);
	void drawCheckBox(bool active, int bobNum, int16 x, int16 y, int frameNum);
	void drawSlideBar(int value, int maxValue, int bobNum, int16 y, int frameNum);
	void drawPanel(const int *frames, const int *titles, int n);
	void drawNormalPanel();
	void drawYesNoPanel(int titleNum);
	void drawConfigPanel();
	void drawInfoPanel();

	void initTextField(const char *desc);
	void updateTextField(uint16 ascii, int keycode);
	void closeTextField();

	struct TextField {
		bool enabled;
		int posCursor;
		uint textCharsCount;
		char text[32];
		int x, y;
		int w, h;
	};

	struct Zone {
		int num;
		int16 x1, y1, x2, y2;
	};

	PanelMode _panelMode;
	QuitMode _quitMode;

	int _currentSavePage;
	int _currentSaveSlot;

	int _prevJoeX, _prevJoeY;

	int _panelTextCount;
	int _panelTextY[MAX_PANEL_TEXTS];
	TextField _textField;
	uint16 _prevZoneNum;
	char _saveDescriptions[100][32];

	OSystem *_system;
	QueenEngine *_vm;

	static const Zone _zones[MAX_ZONES];
};

} // End of namespace Queen

#endif
