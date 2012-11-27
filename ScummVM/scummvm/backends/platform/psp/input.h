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

#ifndef PSP_INPUT_H
#define PSP_INPUT_H

#include "common/scummsys.h"
#include "common/events.h"
#include "backends/platform/psp/pspkeyboard.h"
#include "backends/platform/psp/cursor.h"
#include "backends/platform/psp/image_viewer.h"
#include <pspctrl.h>

enum PspEventType {
	PSP_EVENT_NONE = 0,
	PSP_EVENT_SHIFT,
	PSP_EVENT_SHOW_VIRTUAL_KB,
	PSP_EVENT_LBUTTON,
	PSP_EVENT_RBUTTON,
	PSP_EVENT_MODE_SWITCH,
	PSP_EVENT_CHANGE_SPEED,
	PSP_EVENT_IMAGE_VIEWER,
	PSP_EVENT_IMAGE_VIEWER_SET_BUTTONS,
	PSP_EVENT_LAST
};

struct PspEvent {
	PspEventType type;
	uint32 data;
	PspEvent() { clear(); }
	void clear() {
		type = PSP_EVENT_NONE;
		data = 0;
	}
	bool isEmpty() { return type == PSP_EVENT_NONE; }
};

enum PspPadMode {
	PAD_MODE_NORMAL,
	PAD_MODE_LOL,
	PAD_MODE_LAST
};

enum ShiftMode {
	UNSHIFTED = 0,
	SHIFTED = 1,
	SHIFTED_MODE_LAST
};


class Button {
private:
	Common::KeyCode _key;
	uint32 _ascii;
	uint32 _flag;
	PspEvent _pspEventDown;	// event when we press
	PspEvent _pspEventUp;	// event when we release
public:
	Button();
	void clear();
	bool getEvent(Common::Event &event, PspEvent &pspEvent, bool buttonDown);
	void setKey(Common::KeyCode key, uint32 ascii = 0, uint32 flag = 0) { _key = key; _ascii = ascii; _flag = flag; }
	void setPspEvent(PspEventType typeDown, uint32 dataDown, PspEventType typeUp, uint32 dataUp);
};

class ButtonPad {
public:
	enum ButtonType {	// must match the buttonMap
		BTN_UP_LEFT,
		BTN_UP_RIGHT,
		BTN_DOWN_RIGHT,
		BTN_DOWN_LEFT,
		BTN_RIGHT,
		BTN_DOWN,
		BTN_LEFT,
		BTN_UP,
		BTN_CROSS,
		BTN_CIRCLE,
		BTN_TRIANGLE,
		BTN_SQUARE,
		BTN_LTRIGGER,
		BTN_RTRIGGER,
		BTN_START,
		BTN_SELECT,
		BTN_LAST
	};

private:
	Button _button[BTN_LAST][SHIFTED_MODE_LAST];
	uint32 _buttonsChanged[SHIFTED_MODE_LAST];		// normal and shifted
	uint32 _prevButtonState;
	ShiftMode _shifted;
	PspPadMode _padMode;
	bool _comboMode;								// are we in the middle of combos
	bool _combosEnabled;							// can we do combos
	static const uint32 _buttonMap[];				// maps the buttons to their values

	void initButtonsNormalMode();
	void initButtonsLolMode();
	void modifyButtonsForCombos(SceCtrlData &pad);

public:
	ButtonPad();
	void initButtons();		// set the buttons to the mode that's selected
	void clearButtons();	// empty the buttons of all events

	bool getEvent(Common::Event &event, PspEvent &pspEvent, SceCtrlData &pad);
	bool getEventFromButtonState(Common::Event &event, PspEvent &pspEvent, uint32 buttonState);

	void setShifted(ShiftMode shifted) { _shifted = shifted; }
	void setPadMode(PspPadMode mode) { _padMode = mode; }
	bool isButtonDown() { return _prevButtonState; }

	void enableCombos(bool value) { _combosEnabled = value; }
	Button &getButton(ButtonType type, ShiftMode mode) { return _button[type][mode]; }
};

class Nub {
private:
	Cursor *_cursor;		// to enable changing/getting cursor position

	ShiftMode _shifted;
	bool _dpadMode;

	ButtonPad _buttonPad;	// private buttonpad for dpad mode

	int32 modifyNubAxisMotion(int32 input);
	void translateToDpadState(int dpadX, int dpadY, uint32 &buttonState);	// convert nub data to dpad data
public:
	Nub() : _shifted(UNSHIFTED), _dpadMode(false) { }
	void init() { _buttonPad.initButtons(); }

	void setCursor(Cursor *cursor) { _cursor = cursor; }

	// setters
	void setDpadMode(bool active) { _dpadMode = active; }
	void setShifted(ShiftMode shifted) { _shifted = shifted; }

	// getters
	bool isButtonDown();
	bool getEvent(Common::Event &event, PspEvent &pspEvent, SceCtrlData &pad);
	ButtonPad &getPad() { return _buttonPad; }
};

class InputHandler {
public:
	InputHandler() : _keyboard(0), _cursor(0), _imageViewer(0), _padMode(PAD_MODE_NORMAL),
					 _lastPadCheckTime(0) {}
	// pointer setters
	void setKeyboard(PSPKeyboard *keyboard) { _keyboard = keyboard; }
	void setCursor(Cursor *cursor) { _cursor = cursor; _nub.setCursor(cursor); }
	void setImageViewer(ImageViewer *imageViewer) { _imageViewer = imageViewer; }

	void init();
	bool getAllInputs(Common::Event &event);
	void setImageViewerMode(bool active);

private:
	Nub _nub;
	ButtonPad _buttonPad;

	// Pointers to relevant other classes
	PSPKeyboard *_keyboard;
	Cursor *_cursor;
	ImageViewer *_imageViewer;

	PspPadMode _padMode;				// whice mode we're in
	PspEvent _pendingPspEvent;			// an event that can't be handled yet
	uint32	_lastPadCheckTime;
	static const char *_padModeText[];

	bool getEvent(Common::Event &event, SceCtrlData &pad);
	bool handlePspEvent(Common::Event &event, PspEvent &pspEvent);
	void handleMouseEvent(Common::Event &event, Common::EventType type, const char *string);
	void handleShiftEvent(ShiftMode shifted);
	void handleModeSwitchEvent();
	void setButtonsForImageViewer();
};

#endif /* PSP_INPUT_H */
