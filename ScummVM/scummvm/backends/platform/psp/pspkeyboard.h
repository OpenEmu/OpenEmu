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

#ifndef PSPKEYBOARD_H
#define PSPKEYBOARD_H

#include "common/events.h"
#include "common/stream.h"
#include "backends/platform/psp/display_client.h"
//#include "backends/platform/psp/input.h"
#include <pspctrl.h>

//number of modes
#define MODE_COUNT 4
#define guiStringsSize 8 /* size of guistrings array */

class PspEvent;

class PSPKeyboard : public DisplayClient {

private:
	enum State {
		kInvisible,
		kDefault,
		kCornersSelected,
		kLTriggerDown,
		kRTriggerDown,
		kMove
	};

public:
	PSPKeyboard();
	~PSPKeyboard();

	bool load();												// Load keyboard into memory
	bool isInit() const { return _init; }								// Check for initialization
	bool isDirty() const { return _dirty; }							// Check if needs redrawing
	void setDirty() { _dirty = true; }
	void setClean() { _dirty = false; }
	bool isVisible() const { return _state != kInvisible; }			// Check if visible
	void setVisible(bool val);
	bool processInput(Common::Event &event, PspEvent &pspEvent, SceCtrlData &pad);	// Process input
	void moveTo(const int newX, const int newY);				// Move keyboard
	void render();												// Draw the keyboard onscreen
private:
	enum CursorDirections {
		kUp = 0,
		kRight,
		kDown,
		kLeft,
		kCenter
	};

	Buffer _buffers[guiStringsSize];
	Palette _palettes[guiStringsSize];
	GuRenderer _renderer;

	void increaseKeyboardLocationX(int amount);		// Move keyboard onscreen
	void increaseKeyboardLocationY(int amount);
	void convertCursorToXY(CursorDirections cur, int &x, int &y);

	bool handleMoveState(SceCtrlData &pad);
	bool handleDefaultState(Common::Event &event, SceCtrlData &pad);
	bool handleCornersSelectedState(Common::Event &event, SceCtrlData &pad);
	bool getInputChoice(Common::Event &event, SceCtrlData &pad);
	void getCursorMovement(SceCtrlData &pad);
	void handleRTriggerDownState(SceCtrlData &pad);
	void handleLTriggerDownState(SceCtrlData &pad);

	static short _modeChar[MODE_COUNT][5][6];
	static const char *_guiStrings[];
	bool _init;
	uint32 _prevButtons;	// A bit pattern.
	uint32 _buttonsChanged;

	bool _dirty;        		// keyboard needs redrawing
	int _mode;          		// charset selected. (0 - letters or 1 - numbers)
	int _movedX;				// location we've moved the KB to onscreen
	int _movedY;
	bool _moved;				// whether the keyboard was moved

	State _state;				// State of keyboard Keyboard state machine
	State _lastState;

	CursorDirections _oldCursor;			// Point to place of last cursor

};

#endif /* PSPKEYBOARD_H */
