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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <FAppApplication.h>

#include "common/translation.h"
#include "base/main.h"

#include "backends/platform/bada/form.h"
#include "backends/platform/bada/system.h"

using namespace Osp::Base::Runtime;
using namespace Osp::Ui;
using namespace Osp::Ui::Controls;

// number of volume levels
#define LEVEL_RANGE 5

// round down small Y touch values to 1 to allow the
// cursor to be positioned at the top of the screen
#define MIN_TOUCH_Y 10

// block for up to 2.5 seconds during shutdown to
// allow the game thread to exit gracefully.
#define EXIT_SLEEP_STEP 10
#define EXIT_SLEEP 250

//
// BadaAppForm
//
BadaAppForm::BadaAppForm() :
	_gameThread(0),
	_state(kInitState),
	_buttonState(kLeftButton),
	_shortcut(kSetVolume) {
	_eventQueueLock = new Mutex();
	_eventQueueLock->Create();
}

result BadaAppForm::Construct() {
	result r = Form::Construct(Controls::FORM_STYLE_NORMAL);
	if (IsFailed(r)) {
		return r;
	}

	BadaSystem *badaSystem = NULL;
	_gameThread = NULL;

	badaSystem = new BadaSystem(this);
	r = badaSystem != NULL ? E_SUCCESS : E_OUT_OF_MEMORY;

	if (!IsFailed(r)) {
		r = badaSystem->Construct();
	}

	if (!IsFailed(r)) {
		_gameThread = new Thread();
		r = _gameThread != NULL ? E_SUCCESS : E_OUT_OF_MEMORY;
	}

	if (!IsFailed(r)) {
		r = _gameThread->Construct(*this);
	}

	if (IsFailed(r)) {
		if (badaSystem != NULL) {
			delete badaSystem;
		}
		if (_gameThread != NULL) {
			delete _gameThread;
			_gameThread = NULL;
		}
	} else {
		g_system = badaSystem;
	}

	return r;
}

BadaAppForm::~BadaAppForm() {
	logEntered();

	if (_gameThread && _state != kErrorState) {
		terminate();

		_gameThread->Stop();
		if (_state != kErrorState) {
			_gameThread->Join();
		}

		delete _gameThread;
		_gameThread = NULL;
	}

	if (_eventQueueLock) {
		delete _eventQueueLock;
		_eventQueueLock = NULL;
	}

	logLeaving();
}

//
// abort the game thread
//
void BadaAppForm::terminate() {
	if (_state == kActiveState) {
		((BadaSystem *)g_system)->setMute(true);

		_eventQueueLock->Acquire();

		Common::Event e;
		e.type = Common::EVENT_QUIT;
		_eventQueue.push(e);
		_state = kClosingState;

		_eventQueueLock->Release();

		// block while thread ends
		AppLog("waiting for shutdown");
		for (int i = 0; i < EXIT_SLEEP_STEP && _state == kClosingState; i++) {
			Thread::Sleep(EXIT_SLEEP);
		}

		if (_state == kClosingState) {
			// failed to terminate - Join() will freeze
			_state = kErrorState;
		}
	}
}

void BadaAppForm::exitSystem() {
	_state = kErrorState;

	if (_gameThread) {
		_gameThread->Stop();
		delete _gameThread;
		_gameThread = NULL;
	}
}

result BadaAppForm::OnInitializing(void) {
	logEntered();

	SetOrientation(ORIENTATION_LANDSCAPE);
	AddOrientationEventListener(*this);
	AddTouchEventListener(*this);
	AddKeyEventListener(*this);

	// set focus to enable receiving key events
	SetFocusable(true);
	SetFocus();

	return E_SUCCESS;
}

result BadaAppForm::OnDraw(void) {
	logEntered();

	if (g_system) {
		BadaSystem *system = (BadaSystem *)g_system;
		BadaGraphicsManager *graphics = system->getGraphics();
		if (graphics && graphics->isReady()) {
			g_system->updateScreen();
		}
	}

	return E_SUCCESS;
}

bool BadaAppForm::pollEvent(Common::Event &event) {
	bool result = false;

	_eventQueueLock->Acquire();
	if (!_eventQueue.empty()) {
		event = _eventQueue.pop();
		result = true;
	}
	_eventQueueLock->Release();

	return result;
}

void BadaAppForm::pushEvent(Common::EventType type, const Point &currentPosition) {
	BadaSystem *system = (BadaSystem *)g_system;
	BadaGraphicsManager *graphics = system->getGraphics();
	if (graphics) {
		// graphics could be NULL at startup or when
		// displaying the system error screen
		Common::Event e;
		e.type = type;
		e.mouse.x = currentPosition.x;
		e.mouse.y = currentPosition.y > MIN_TOUCH_Y ? currentPosition.y : 1;

		bool moved = graphics->moveMouse(e.mouse.x, e.mouse.y);

		_eventQueueLock->Acquire();

		if (moved && type != Common::EVENT_MOUSEMOVE) {
			Common::Event moveEvent;
			moveEvent.type = Common::EVENT_MOUSEMOVE;
			moveEvent.mouse = e.mouse;
			_eventQueue.push(moveEvent);
		}

		_eventQueue.push(e);
		_eventQueueLock->Release();
	}
}

void BadaAppForm::pushKey(Common::KeyCode keycode) {
	Common::Event e;
	e.synthetic = false;
	e.kbd.keycode = keycode;
	e.kbd.ascii = keycode;
	e.kbd.flags = 0;

	_eventQueueLock->Acquire();

	e.type = Common::EVENT_KEYDOWN;
	_eventQueue.push(e);
	e.type = Common::EVENT_KEYUP;
	_eventQueue.push(e);

	_eventQueueLock->Release();
}

void BadaAppForm::OnOrientationChanged(const Control &source,
																			 OrientationStatus orientationStatus) {
	logEntered();
	if (_state == kInitState) {
		_state = kActiveState;
		_gameThread->Start();
	}
}

Object *BadaAppForm::Run(void) {
	scummvm_main(0, 0);

	if (_state == kActiveState) {
		Application::GetInstance()->SendUserEvent(USER_MESSAGE_EXIT, NULL);
	}
	_state = kDoneState;
	return NULL;
}

void BadaAppForm::setButtonShortcut() {
	switch (_buttonState) {
	case kLeftButton:
		g_system->displayMessageOnOSD(_("Right Click Once"));
		_buttonState = kRightButtonOnce;
		break;
	case kRightButtonOnce:
		g_system->displayMessageOnOSD(_("Right Click"));
		_buttonState = kRightButton;
		break;
	case kRightButton:
		g_system->displayMessageOnOSD(_("Move Only"));
		_buttonState = kMoveOnly;
		break;
	case kMoveOnly:
		g_system->displayMessageOnOSD(_("Left Click"));
		_buttonState = kLeftButton;
		break;
	}
}

void BadaAppForm::setShortcut() {
	// cycle to the next shortcut
	switch (_shortcut) {
	case kControlMouse:
		g_system->displayMessageOnOSD(_("Escape Key"));
		_shortcut = kEscapeKey;
		break;

	case kEscapeKey:
		g_system->displayMessageOnOSD(_("Game Menu"));
		_shortcut = kGameMenu;
		break;

	case kGameMenu:
		g_system->displayMessageOnOSD(_("Show Keypad"));
		_shortcut = kShowKeypad;
		break;

	case kSetVolume:
		// fallthru

	case kShowKeypad:
		g_system->displayMessageOnOSD(_("Control Mouse"));
		_shortcut = kControlMouse;
		break;
	}
}

void BadaAppForm::setVolume(bool up, bool minMax) {
	int level = ((BadaSystem *)g_system)->setVolume(up, minMax);
	if (level != -1) {
		char message[32];
		char ind[LEVEL_RANGE]; // 1..5 (0=off)
		int j = LEVEL_RANGE - 1; // 0..4
		for (int i = 1; i <= LEVEL_RANGE; i++) {
			ind[j--] = level >= i ? '|' : ' ';
		}
		snprintf(message, sizeof(message), "Volume: [ %c%c%c%c%c ]",
						 ind[0], ind[1], ind[2], ind[3], ind[4]);
		g_system->displayMessageOnOSD(message);
	}
}

void BadaAppForm::showKeypad() {
	// display the soft keyboard
	_buttonState = kLeftButton;
	pushKey(Common::KEYCODE_F7);
}

void BadaAppForm::OnTouchDoublePressed(const Control &source,
																			 const Point &currentPosition,
																			 const TouchEventInfo &touchInfo) {
	if (_buttonState != kMoveOnly) {
		pushEvent(_buttonState == kLeftButton ? Common::EVENT_LBUTTONDOWN : Common::EVENT_RBUTTONDOWN,
							currentPosition);
		pushEvent(_buttonState == kLeftButton ? Common::EVENT_LBUTTONDOWN : Common::EVENT_RBUTTONDOWN,
							currentPosition);
	}
}

void BadaAppForm::OnTouchFocusIn(const Control &source,
																 const Point &currentPosition,
																 const TouchEventInfo &touchInfo) {
}

void BadaAppForm::OnTouchFocusOut(const Control &source,
																	const Point &currentPosition,
																	const TouchEventInfo &touchInfo) {
}

void BadaAppForm::OnTouchLongPressed(const Control &source,
																		 const Point &currentPosition,
																		 const TouchEventInfo &touchInfo) {
	if (_buttonState != kLeftButton) {
		pushKey(Common::KEYCODE_RETURN);
	}
}

void BadaAppForm::OnTouchMoved(const Control &source,
															 const Point &currentPosition,
															 const TouchEventInfo &touchInfo) {
	pushEvent(Common::EVENT_MOUSEMOVE, currentPosition);
}

void BadaAppForm::OnTouchPressed(const Control &source,
																 const Point &currentPosition,
																 const TouchEventInfo &touchInfo) {
	if (_buttonState != kMoveOnly) {
		pushEvent(_buttonState == kLeftButton ? Common::EVENT_LBUTTONDOWN : Common::EVENT_RBUTTONDOWN,
							currentPosition);
	}
}

void BadaAppForm::OnTouchReleased(const Control &source,
																	const Point &currentPosition,
																	const TouchEventInfo &touchInfo) {
	if (_buttonState != kMoveOnly) {
		pushEvent(_buttonState == kLeftButton ? Common::EVENT_LBUTTONUP : Common::EVENT_RBUTTONUP,
							currentPosition);
		if (_buttonState == kRightButtonOnce) {
			_buttonState = kLeftButton;
		}
		// flick to skip dialog
		if (touchInfo.IsFlicked()) {
			pushKey(Common::KEYCODE_PERIOD);
		}
	}
}

void BadaAppForm::OnKeyLongPressed(const Control &source, KeyCode keyCode) {
	logEntered();
	switch (keyCode) {
	case KEY_SIDE_UP:
		_shortcut = kSetVolume;
		setVolume(true, true);
		return;

	case KEY_SIDE_DOWN:
		_shortcut = kSetVolume;
		setVolume(false, true);
		return;

	case KEY_CAMERA:
		_shortcut = kShowKeypad;
		showKeypad();
		return;

	default:
		break;
	}
}

void BadaAppForm::OnKeyPressed(const Control &source, KeyCode keyCode) {
	switch (keyCode) {
	case KEY_SIDE_UP:
		if (_shortcut != kSetVolume) {
			_shortcut = kSetVolume;
		} else {
			setVolume(true, false);
		}
		return;

	case KEY_SIDE_DOWN:
		switch (_shortcut) {
		case kControlMouse:
			setButtonShortcut();
			break;

		case kEscapeKey:
			pushKey(Common::KEYCODE_ESCAPE);
			break;

		case kGameMenu:
			_buttonState = kLeftButton;
			pushKey(Common::KEYCODE_F5);
			break;

		case kShowKeypad:
			showKeypad();
			break;

		default:
			setVolume(false, false);
			break;
		}
		break;

	case KEY_CAMERA:
		setShortcut();
		break;

	default:
		break;
	}
}

void BadaAppForm::OnKeyReleased(const Control &source, KeyCode keyCode) {
}
