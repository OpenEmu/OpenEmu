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

#include <pspctrl.h>
#include "gui/message.h"
#include "backends/platform/psp/input.h"

//#define __PSP_DEBUG_FUNCS__	/* Uncomment for debugging the stack */
//#define __PSP_DEBUG_PRINT__ /* Uncomment for debug prints */
#include "backends/platform/psp/trace.h"

// Defines for working with PSP buttons
#define DOWN(x)		 ((pad.Buttons & (x)) == (x))
#define UP(x)		 (!(pad.Buttons & (x)))
#define PSP_DPAD	 (PSP_CTRL_DOWN|PSP_CTRL_UP|PSP_CTRL_LEFT|PSP_CTRL_RIGHT)
#define PSP_4BUTTONS (PSP_CTRL_CROSS | PSP_CTRL_CIRCLE | PSP_CTRL_TRIANGLE | PSP_CTRL_SQUARE)
#define PSP_TRIGGERS (PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER)
#define PSP_ALL_BUTTONS (PSP_DPAD | PSP_4BUTTONS | PSP_TRIGGERS | PSP_CTRL_START | PSP_CTRL_SELECT)

#define PAD_CHECK_TIME	53

Button::Button() {
	clear();
}

inline void Button::clear() {
	_key = Common::KEYCODE_INVALID;
	_ascii = 0;
	_flag = 0;
	_pspEventDown.clear();
	_pspEventUp.clear();
}

inline bool Button::getEvent(Common::Event &event, PspEvent &pspEvent, bool down) {
	if (down) {
		if (!_pspEventDown.isEmpty())
			pspEvent = _pspEventDown;
	} else { // up
		if (!_pspEventUp.isEmpty())
			pspEvent = _pspEventUp;
	}
	if (_key != Common::KEYCODE_INVALID) {
		event.type = down ? Common::EVENT_KEYDOWN : Common::EVENT_KEYUP;
		event.kbd.keycode = _key;
		event.kbd.ascii = _ascii;
		event.kbd.flags |= _flag;
		return true;
	} else if (_flag) {	// handle flag only events
		event.type = down ? Common::EVENT_KEYDOWN : Common::EVENT_KEYUP;
		event.kbd.flags |= down ? _flag : 0;
		return true;
	}
	return false;
}

void Button::setPspEvent(PspEventType typeDown, uint32 dataDown, PspEventType typeUp, uint32 dataUp) {
	_pspEventDown.type = typeDown;
	_pspEventDown.data = dataDown;
	_pspEventUp.type = typeUp;
	_pspEventUp.data = dataUp;
}

// Translates bitfields to our constants
// We put combined bitfields first to make sure we pick up diagonals
const uint32 ButtonPad::_buttonMap[] = {
	PSP_CTRL_UP | PSP_CTRL_LEFT,
	PSP_CTRL_UP | PSP_CTRL_RIGHT,
	PSP_CTRL_DOWN | PSP_CTRL_RIGHT,
	PSP_CTRL_DOWN | PSP_CTRL_LEFT,
	PSP_CTRL_RIGHT, PSP_CTRL_DOWN, PSP_CTRL_LEFT, PSP_CTRL_UP,
	PSP_CTRL_CROSS, PSP_CTRL_CIRCLE, PSP_CTRL_TRIANGLE, PSP_CTRL_SQUARE,
	PSP_CTRL_LTRIGGER, PSP_CTRL_RTRIGGER, PSP_CTRL_START, PSP_CTRL_SELECT
};

ButtonPad::ButtonPad() : _prevButtonState(0), _shifted(UNSHIFTED), _padMode(PAD_MODE_NORMAL),
						_comboMode(false), _combosEnabled(true) {
	for (int i = UNSHIFTED; i < SHIFTED_MODE_LAST; i++)
		_buttonsChanged[i] = 0;
	clearButtons();
}

void ButtonPad::clearButtons() {
	for (int i = BTN_UP_LEFT; i < BTN_LAST; i++) {
		_button[i][UNSHIFTED].clear();
		_button[i][SHIFTED].clear();
	}
}

void ButtonPad::initButtons() {
	switch (_padMode) {
	case PAD_MODE_NORMAL:
		initButtonsNormalMode();
		break;
	case PAD_MODE_LOL:
		initButtonsLolMode();
		break;
	default:
		break;
	}
}

void ButtonPad::initButtonsNormalMode() {
	DEBUG_ENTER_FUNC();
	PSP_DEBUG_PRINT("initializing buttons for normal mode\n");
	clearButtons();

	// Dpad
	_button[BTN_UP_LEFT][UNSHIFTED].setKey(Common::KEYCODE_KP7, '7');
	_button[BTN_LEFT][SHIFTED].setKey(Common::KEYCODE_KP7, '7');		// same as up_left
	_button[BTN_UP][UNSHIFTED].setKey(Common::KEYCODE_KP8, '8');
	_button[BTN_UP_RIGHT][UNSHIFTED].setKey(Common::KEYCODE_KP9, '9');
	_button[BTN_UP][SHIFTED].setKey(Common::KEYCODE_KP9, '9');			// same as up_right
	_button[BTN_LEFT][UNSHIFTED].setKey(Common::KEYCODE_KP4, '4');
	_button[BTN_RIGHT][UNSHIFTED].setKey(Common::KEYCODE_KP6, '6');
	_button[BTN_DOWN_LEFT][UNSHIFTED].setKey(Common::KEYCODE_KP1, '1');
	_button[BTN_DOWN][SHIFTED].setKey(Common::KEYCODE_KP1, '1');		// same as down_left
	_button[BTN_DOWN][UNSHIFTED].setKey(Common::KEYCODE_KP2, '2');
	_button[BTN_DOWN_RIGHT][UNSHIFTED].setKey(Common::KEYCODE_KP3, '3');
	_button[BTN_RIGHT][SHIFTED].setKey(Common::KEYCODE_KP3, '3');		// same as down_right

	// Other buttons
	_button[BTN_CROSS][UNSHIFTED].setPspEvent(PSP_EVENT_LBUTTON, true, PSP_EVENT_LBUTTON, false);
	_button[BTN_CIRCLE][UNSHIFTED].setPspEvent(PSP_EVENT_RBUTTON, true, PSP_EVENT_RBUTTON, false);
	_button[BTN_TRIANGLE][UNSHIFTED].setKey(Common::KEYCODE_RETURN, '\r');
	_button[BTN_SQUARE][UNSHIFTED].setKey(Common::KEYCODE_PERIOD, '.');
	_button[BTN_SQUARE][SHIFTED].setKey(Common::KEYCODE_SPACE, ' ');
	_button[BTN_LTRIGGER][UNSHIFTED].setKey(Common::KEYCODE_ESCAPE, 27);
	_button[BTN_RTRIGGER][SHIFTED].setPspEvent(PSP_EVENT_SHIFT, true, PSP_EVENT_SHIFT, false);
	_button[BTN_RTRIGGER][UNSHIFTED].setPspEvent(PSP_EVENT_SHIFT, true, PSP_EVENT_SHIFT, false);
	_button[BTN_RTRIGGER][SHIFTED].setKey(Common::KEYCODE_INVALID, 0, Common::KBD_SHIFT);
	_button[BTN_RTRIGGER][UNSHIFTED].setKey(Common::KEYCODE_INVALID, 0, Common::KBD_SHIFT);
	_button[BTN_START][SHIFTED].setKey(Common::KEYCODE_F5, Common::ASCII_F5);
	_button[BTN_START][UNSHIFTED].setKey(Common::KEYCODE_F5, Common::ASCII_F5, Common::KBD_CTRL);
	_button[BTN_SELECT][UNSHIFTED].setPspEvent(PSP_EVENT_SHOW_VIRTUAL_KB, true, PSP_EVENT_NONE, 0);
	_button[BTN_SELECT][SHIFTED].setPspEvent(PSP_EVENT_IMAGE_VIEWER, true, PSP_EVENT_NONE, 0);
}

void ButtonPad::initButtonsLolMode() {
	DEBUG_ENTER_FUNC();
	initButtonsNormalMode();	// set normal button configuration
	PSP_DEBUG_PRINT("initializing buttons for LOL mode\n");

	// Square is our new shift button
	_button[BTN_SQUARE][UNSHIFTED].clear();
	_button[BTN_SQUARE][UNSHIFTED].setPspEvent(PSP_EVENT_SHIFT, true, PSP_EVENT_SHIFT, false);
	_button[BTN_SQUARE][SHIFTED].clear();
	_button[BTN_SQUARE][SHIFTED].setPspEvent(PSP_EVENT_SHIFT, true, PSP_EVENT_SHIFT, false);

	// Dpad
	_button[BTN_LEFT][UNSHIFTED].clear();
	_button[BTN_LEFT][UNSHIFTED].setKey(Common::KEYCODE_KP7, '7');
	_button[BTN_LEFT][SHIFTED].clear();
	_button[BTN_LEFT][SHIFTED].setKey(Common::KEYCODE_F1, Common::ASCII_F1);
	_button[BTN_UP][SHIFTED].clear();
	_button[BTN_UP][SHIFTED].setKey(Common::KEYCODE_F2, Common::ASCII_F2);
	_button[BTN_RIGHT][UNSHIFTED].clear();
	_button[BTN_RIGHT][UNSHIFTED].setKey(Common::KEYCODE_KP9, '9');
	_button[BTN_RIGHT][SHIFTED].clear();
	_button[BTN_RIGHT][SHIFTED].setKey(Common::KEYCODE_F3, Common::ASCII_F3);
	_button[BTN_DOWN][SHIFTED].clear();
	_button[BTN_DOWN][SHIFTED].setKey(Common::KEYCODE_F4, Common::ASCII_F4);

	// Buttons
	_button[BTN_LTRIGGER][UNSHIFTED].clear();
	_button[BTN_LTRIGGER][SHIFTED].clear();
	_button[BTN_LTRIGGER][UNSHIFTED].setKey(Common::KEYCODE_KP4, '4');		// Triggers turn
	_button[BTN_RTRIGGER][UNSHIFTED].clear();
	_button[BTN_RTRIGGER][SHIFTED].clear();
	_button[BTN_RTRIGGER][UNSHIFTED].setKey(Common::KEYCODE_KP6, '6');
	_button[BTN_START][SHIFTED].clear();
	_button[BTN_START][SHIFTED].setKey(Common::KEYCODE_ESCAPE, 27);
}

bool ButtonPad::getEvent(Common::Event &event, PspEvent &pspEvent, SceCtrlData &pad) {
	DEBUG_ENTER_FUNC();

	//PSP_DEBUG_PRINT("buttons[%x]\n", pad.Buttons);

	uint32 curButtonState = PSP_ALL_BUTTONS & pad.Buttons;	// we only care about these

	if (_combosEnabled)
		modifyButtonsForCombos(pad);						// change buttons for combos

	return getEventFromButtonState(event, pspEvent, curButtonState);
}

bool ButtonPad::getEventFromButtonState(Common::Event &event, PspEvent &pspEvent, uint32 buttonState) {
	DEBUG_ENTER_FUNC();
	_buttonsChanged[_shifted] |= buttonState ^ _prevButtonState;	// add any buttons that changed
	_prevButtonState = buttonState;

	for (int shiftState = UNSHIFTED; shiftState < SHIFTED_MODE_LAST; shiftState++) {
		if (_buttonsChanged[shiftState]) {	// any button to address?
			PSP_DEBUG_PRINT("found changed buttons\n");
			ButtonType buttonType = BTN_LAST;
			bool buttonDown = false;		// normally we release a button (as in when we're in a different shiftmode)

			for (int i = BTN_UP_LEFT; i < BTN_LAST; i++) {
				uint32 buttonCode = _buttonMap[i];
				if ((_buttonsChanged[shiftState] & buttonCode) == buttonCode) { // check for this changed button
					buttonType = (ButtonType)i;						// we know which button changed
					_buttonsChanged[shiftState] &= ~buttonCode;	// save the fact that we treated this button
					if (shiftState == _shifted)
						buttonDown = buttonState & buttonCode ? true : false; // pressed or released?

					PSP_DEBUG_PRINT("button[%i] pressed\n", i);
					break;
				}
			}

			assert (buttonType < BTN_LAST);
			bool haveEvent = _button[buttonType][shiftState].getEvent(event, pspEvent, buttonDown);
			if (haveEvent)
				PSP_DEBUG_PRINT("have event. key[%d] flag[%x] %s\n", event.kbd.ascii, event.kbd.flags, buttonDown ? "down" : "up");
			return haveEvent;
		}
	}

	return false;
}

void ButtonPad::modifyButtonsForCombos(SceCtrlData &pad) {
	if (DOWN(PSP_CTRL_RTRIGGER | PSP_CTRL_LTRIGGER)) {
		if (!_comboMode) {  // we're entering combo mode
			PSP_DEBUG_PRINT("entering combo mode\n");
			_button[BTN_SQUARE][UNSHIFTED].clear();
			_button[BTN_SQUARE][SHIFTED].clear();
			_button[BTN_DOWN][SHIFTED].clear();
			_button[BTN_DOWN][UNSHIFTED].clear();
			_button[BTN_UP][SHIFTED].clear();
			_button[BTN_UP][UNSHIFTED].clear();
			_button[BTN_SQUARE][UNSHIFTED].setPspEvent(PSP_EVENT_MODE_SWITCH, true, PSP_EVENT_NONE, true);
			_button[BTN_SQUARE][SHIFTED].setPspEvent(PSP_EVENT_MODE_SWITCH, true, PSP_EVENT_NONE, true);
			_button[BTN_DOWN][UNSHIFTED].setPspEvent(PSP_EVENT_CHANGE_SPEED, false, PSP_EVENT_NONE, true);
			_button[BTN_DOWN][SHIFTED].setPspEvent(PSP_EVENT_CHANGE_SPEED, false, PSP_EVENT_NONE, true);
			_button[BTN_UP][UNSHIFTED].setPspEvent(PSP_EVENT_CHANGE_SPEED, true, PSP_EVENT_NONE, true);
			_button[BTN_UP][SHIFTED].setPspEvent(PSP_EVENT_CHANGE_SPEED, true, PSP_EVENT_NONE, true);
			_comboMode = true;
		}
	} else {					// no combo buttons are pressed	now
		if (_comboMode) {		// we have been running in combo mode
			initButtons();		// reset the button configuration
			_comboMode = false;
		}
	}
}

bool Nub::getEvent(Common::Event &event, PspEvent &pspEvent, SceCtrlData &pad) {
	DEBUG_ENTER_FUNC();

	if (_dpadMode) {	// Convert the nub to a D-Pad
		uint32 buttonState;
		translateToDpadState(pad.Lx, pad.Ly, buttonState);
		return _buttonPad.getEventFromButtonState(event, pspEvent, buttonState);
	}

	int32 analogStepX = pad.Lx;		// Goes up to 255.
	int32 analogStepY = pad.Ly;

	analogStepX = modifyNubAxisMotion(analogStepX);
	analogStepY = modifyNubAxisMotion(analogStepY);

	int32 oldX = _cursor->getX();
	int32 oldY = _cursor->getY();

	if (analogStepX != 0 || analogStepY != 0) {

		PSP_DEBUG_PRINT("raw x[%d], y[%d]\n", analogStepX, analogStepY);

		// If no movement then this has no effect
		if (_shifted) {
			// Fine control mode for analog
			if (analogStepX != 0) {
				if (analogStepX > 0)
					_cursor->increaseXY(2, 0);
				else
					_cursor->increaseXY(-2, 0);
			}

			if (analogStepY != 0) {
				if (analogStepY > 0)
					_cursor->increaseXY(0, 2);
				else
					_cursor->increaseXY(0, -2);
			}
		} else {	// Regular speed movement
			_cursor->increaseXY(analogStepX, 0);
			_cursor->increaseXY(0, analogStepY);
		}

		int32 newX = _cursor->getX();
		int32 newY = _cursor->getY();

		if ((oldX != newX) || (oldY != newY)) {
			event.type = Common::EVENT_MOUSEMOVE;
			event.mouse.x = newX;
			event.mouse.y = newY;
			PSP_DEBUG_PRINT("Nub event. X[%d], Y[%d]\n", newX, newY);
			return true;
		}
	}
	return false;
}

void Nub::translateToDpadState(int dpadX, int dpadY, uint32 &buttonState) {
	#define MIN_NUB_POSITION 70
	buttonState = 0;

	if (dpadX > 127 + MIN_NUB_POSITION)
		buttonState |= PSP_CTRL_RIGHT;
	else if (dpadX < 127 - MIN_NUB_POSITION)
		buttonState |= PSP_CTRL_LEFT;

	if (dpadY > 127 + MIN_NUB_POSITION)
		buttonState |= PSP_CTRL_DOWN;
	else if (dpadY < 127 - MIN_NUB_POSITION)
		buttonState |= PSP_CTRL_UP;
}

inline int32 Nub::modifyNubAxisMotion(int32 input) {
	DEBUG_ENTER_FUNC();
	const int MIN_NUB_MOTION = 30;

	input -= 128;	// Center on 0.

	if (input < -MIN_NUB_MOTION - 1)
		input += MIN_NUB_MOTION + 1;	// reduce the velocity
	else if (input > MIN_NUB_MOTION)
		input -= MIN_NUB_MOTION;	// same
	else 				// between these points, dampen the response to 0
		input = 0;

	return input;
}

inline bool Nub::isButtonDown() {
	if (_dpadMode) 		// only relevant in dpad mode
		return _buttonPad.isButtonDown();
	return false;
}

const char *InputHandler::_padModeText[] = {
	"Normal Button Mode",
	"1st Person RPG Button Mode"
};

void InputHandler::init() {
	sceCtrlSetSamplingCycle(0);	// set sampling to vsync. n = n usecs
	sceCtrlSetSamplingMode(1);  // analog

	_buttonPad.initButtons();
	_nub.init();
}

bool InputHandler::getAllInputs(Common::Event &event) {
	DEBUG_ENTER_FUNC();

	uint32 time = g_system->getMillis();	// may not be necessary with read
	if (time - _lastPadCheckTime < PAD_CHECK_TIME) {
		return false;
	}

	_lastPadCheckTime = time;
	SceCtrlData pad;

	sceCtrlPeekBufferPositive(&pad, 1);	// Peek doesn't sleep. Read sleeps the thread

	bool haveEvent;
	//memset(&event, 0, sizeof(event));

	haveEvent = getEvent(event, pad);

	if (haveEvent) {
		PSP_DEBUG_PRINT("Have event[%s]. Type[%d]\n", haveEvent ? "true" : "false", event.type);
	}

	return haveEvent;
}

bool InputHandler::getEvent(Common::Event &event, SceCtrlData &pad) {
	DEBUG_ENTER_FUNC();

	PspEvent pspEvent;
	bool haveEvent = false;

	if (_keyboard->isVisible()) {
		haveEvent = _keyboard->processInput(event, pspEvent, pad);
	} else {	// only process buttonpad if keyboard invisible
		haveEvent = _buttonPad.getEvent(event, pspEvent, pad);
	}

	if (!haveEvent && pspEvent.isEmpty())
		haveEvent = _nub.getEvent(event, pspEvent, pad);

	// handle any pending PSP events
	if (!haveEvent && pspEvent.isEmpty()) {
		if (!_pendingPspEvent.isEmpty()) {
			pspEvent = _pendingPspEvent;
			_pendingPspEvent.clear();
		}
	}

	// handle any PSP events we might have
	if (!pspEvent.isEmpty())
		haveEvent |= handlePspEvent(event, pspEvent);	// overrides any event we might have

	return haveEvent;
}

bool InputHandler::handlePspEvent(Common::Event &event, PspEvent &pspEvent) {
	bool haveEvent = false;

	PSP_DEBUG_PRINT("have pspEvent[%d] data[%d]\n", pspEvent.type, pspEvent.data);

	switch (pspEvent.type) {
	case PSP_EVENT_SHIFT:
		handleShiftEvent((ShiftMode)pspEvent.data);
		break;
	case PSP_EVENT_SHOW_VIRTUAL_KB:
		_keyboard->setVisible((bool)pspEvent.data);
		if ((pspEvent.data && _keyboard->isVisible()) || !pspEvent.data) 	// don't change mode if keyboard didn't load
			_nub.setDpadMode((bool)pspEvent.data);							// set nub to keypad/regular mode
		break;
	case PSP_EVENT_LBUTTON:
		haveEvent = true;
		if (pspEvent.data) // down
			handleMouseEvent(event, Common::EVENT_LBUTTONDOWN, "LButtonDown");
		else
			handleMouseEvent(event, Common::EVENT_LBUTTONUP, "LButtonUp");
		break;
	case PSP_EVENT_RBUTTON:
		haveEvent = true;
		if (pspEvent.data) // down
			handleMouseEvent(event, Common::EVENT_RBUTTONDOWN, "RButtonDown");
		else
			handleMouseEvent(event, Common::EVENT_RBUTTONUP, "RButtonUp");
		break;
	case PSP_EVENT_MODE_SWITCH:
		handleModeSwitchEvent();
		break;
	/*case PSP_EVENT_CHANGE_SPEED:
		handleSpeedChange(pspEvent.data);
		break;*/
	case PSP_EVENT_IMAGE_VIEWER:
		_imageViewer->handleEvent(pspEvent.data);
		break;
	case PSP_EVENT_IMAGE_VIEWER_SET_BUTTONS:
		setImageViewerMode(pspEvent.data);
		break;
	default:
		PSP_ERROR("Unhandled PSP Event[%d]\n", pspEvent.type);
		break;
	}

	return haveEvent;
}

void InputHandler::handleMouseEvent(Common::Event &event, Common::EventType type, const char *string) {
	event.type = type;
	event.mouse.x = _cursor->getX();
	event.mouse.y = _cursor->getY();
	PSP_DEBUG_PRINT("%s event, x[%d], y[%d]\n", string, event.mouse.x, event.mouse.y);
}

void InputHandler::handleShiftEvent(ShiftMode shifted) {
	_buttonPad.setShifted(shifted);
	_nub.setShifted(shifted);
}

void InputHandler::handleModeSwitchEvent() {
	// check if we can't switch modes right now
	if (_buttonPad.isButtonDown() || _nub.isButtonDown()) {	// can't switch yet
		PSP_DEBUG_PRINT("postponing mode switch event\n");
		_pendingPspEvent.type = PSP_EVENT_MODE_SWITCH;		// queue it to be done later
	} else {	// we can switch
		PSP_DEBUG_PRINT("mode switch event\n");
		_padMode = (PspPadMode)(_padMode + 1);
		if (_padMode >= PAD_MODE_LAST)
			_padMode = PAD_MODE_NORMAL;

		GUI::TimedMessageDialog dialog(_padModeText[_padMode], 1500);
		dialog.runModal();

		_buttonPad.setPadMode(_padMode);
		_buttonPad.initButtons();
	}
}

/*
void InputHandler::handleSpeedChange(bool up) {
	char *dialogMsg;

	if (up) {
		dialogMsg = "

	GUI::TimedMessageDialog dialog(_padModeText[_padMode], 1500);
	dialog.runModal();
}*/

void InputHandler::setImageViewerMode(bool active) {
	if (_buttonPad.isButtonDown() || _nub.isButtonDown()) {	// can't switch yet
		PSP_DEBUG_PRINT("postponing image viewer on event\n");
		_pendingPspEvent.type = PSP_EVENT_IMAGE_VIEWER_SET_BUTTONS;		// queue it to be done later
		_pendingPspEvent.data = active;
	} else if (active) {
		_nub.setDpadMode(true);
		_buttonPad.enableCombos(false);	// disable combos
		setButtonsForImageViewer();
	} else {	// deactivate
		_nub.setDpadMode(false);
		_nub.init();
		_buttonPad.enableCombos(true);	// re-enable combos
		_buttonPad.initButtons();
	}
}

void InputHandler::setButtonsForImageViewer() {
	DEBUG_ENTER_FUNC();

	// Dpad
	_buttonPad.clearButtons();
	_buttonPad.getButton(ButtonPad::BTN_UP, UNSHIFTED).setPspEvent(PSP_EVENT_IMAGE_VIEWER, ImageViewer::EVENT_ZOOM_IN,
		PSP_EVENT_NONE, false);
	_buttonPad.getButton(ButtonPad::BTN_DOWN, UNSHIFTED).setPspEvent(PSP_EVENT_IMAGE_VIEWER, ImageViewer::EVENT_ZOOM_OUT,
		PSP_EVENT_NONE, false);
	_buttonPad.getButton(ButtonPad::BTN_LEFT, UNSHIFTED).setPspEvent(PSP_EVENT_IMAGE_VIEWER, ImageViewer::EVENT_LAST_IMAGE,
		PSP_EVENT_NONE, false);
	_buttonPad.getButton(ButtonPad::BTN_RIGHT, UNSHIFTED).setPspEvent(PSP_EVENT_IMAGE_VIEWER, ImageViewer::EVENT_NEXT_IMAGE,
		PSP_EVENT_NONE, false);
	_buttonPad.getButton(ButtonPad::BTN_LTRIGGER, UNSHIFTED).setPspEvent(PSP_EVENT_IMAGE_VIEWER, ImageViewer::EVENT_HIDE,
		PSP_EVENT_NONE, false);
	_buttonPad.getButton(ButtonPad::BTN_RTRIGGER, UNSHIFTED).setPspEvent(PSP_EVENT_IMAGE_VIEWER, ImageViewer::EVENT_HIDE,
		PSP_EVENT_NONE, false);
	_buttonPad.getButton(ButtonPad::BTN_START, UNSHIFTED).setPspEvent(PSP_EVENT_IMAGE_VIEWER, ImageViewer::EVENT_HIDE,
		PSP_EVENT_NONE, false);
	_buttonPad.getButton(ButtonPad::BTN_SELECT, UNSHIFTED).setPspEvent(PSP_EVENT_IMAGE_VIEWER, ImageViewer::EVENT_HIDE,
		PSP_EVENT_NONE, false);

	//Nub
	_nub.getPad().clearButtons();
	_nub.getPad().getButton(ButtonPad::BTN_UP, UNSHIFTED).setPspEvent(PSP_EVENT_IMAGE_VIEWER, ImageViewer::EVENT_MOVE_UP,
		PSP_EVENT_IMAGE_VIEWER, ImageViewer::EVENT_MOVE_STOP);
	_nub.getPad().getButton(ButtonPad::BTN_DOWN, UNSHIFTED).setPspEvent(PSP_EVENT_IMAGE_VIEWER, ImageViewer::EVENT_MOVE_DOWN,
		PSP_EVENT_IMAGE_VIEWER, ImageViewer::EVENT_MOVE_STOP);
	_nub.getPad().getButton(ButtonPad::BTN_LEFT, UNSHIFTED).setPspEvent(PSP_EVENT_IMAGE_VIEWER, ImageViewer::EVENT_MOVE_LEFT,
		PSP_EVENT_IMAGE_VIEWER, ImageViewer::EVENT_MOVE_STOP);
	_nub.getPad().getButton(ButtonPad::BTN_RIGHT, UNSHIFTED).setPspEvent(PSP_EVENT_IMAGE_VIEWER, ImageViewer::EVENT_MOVE_RIGHT,
		PSP_EVENT_IMAGE_VIEWER, ImageViewer::EVENT_MOVE_STOP);
}
