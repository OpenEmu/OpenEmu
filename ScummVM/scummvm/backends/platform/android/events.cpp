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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#if defined(__ANDROID__)

// Allow use of stuff in <time.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

// Disable printf override in common/forbidden.h to avoid
// clashes with log.h from the Android SDK.
// That header file uses
//   __attribute__ ((format(printf, 3, 4)))
// which gets messed up by our override mechanism; this could
// be avoided by either changing the Android SDK to use the equally
// legal and valid
//   __attribute__ ((format(printf, 3, 4)))
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the Android port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "common/events.h"

#include "backends/platform/android/android.h"
#include "backends/platform/android/jni.h"

// $ANDROID_NDK/platforms/android-9/arch-arm/usr/include/android/keycodes.h
// http://android.git.kernel.org/?p=platform/frameworks/base.git;a=blob;f=libs/ui/Input.cpp
// http://android.git.kernel.org/?p=platform/frameworks/base.git;a=blob;f=core/java/android/view/KeyEvent.java

// event type
enum {
	JE_SYS_KEY = 0,
	JE_KEY = 1,
	JE_DPAD = 2,
	JE_DOWN = 3,
	JE_SCROLL = 4,
	JE_TAP = 5,
	JE_DOUBLE_TAP = 6,
	JE_MULTI = 7,
	JE_BALL = 8,
	JE_LMB_DOWN = 9,
	JE_LMB_UP = 10,
	JE_RMB_DOWN = 11,
	JE_RMB_UP = 12,
	JE_MOUSE_MOVE = 13,
	JE_QUIT = 0x1000
};

// action type
enum {
	JACTION_DOWN = 0,
	JACTION_UP = 1,
	JACTION_MULTIPLE = 2,
	JACTION_POINTER_DOWN = 5,
	JACTION_POINTER_UP = 6
};

// system keys
enum {
	JKEYCODE_SOFT_RIGHT = 2,
	JKEYCODE_HOME = 3,
	JKEYCODE_BACK = 4,
	JKEYCODE_CALL = 5,
	JKEYCODE_ENDCALL = 6,
	JKEYCODE_VOLUME_UP = 24,
	JKEYCODE_VOLUME_DOWN = 25,
	JKEYCODE_POWER = 26,
	JKEYCODE_CAMERA = 27,
	JKEYCODE_HEADSETHOOK = 79,
	JKEYCODE_FOCUS = 80,
	JKEYCODE_MENU = 82,
	JKEYCODE_SEARCH = 84,
	JKEYCODE_MUTE = 91,
	JKEYCODE_MEDIA_PLAY_PAUSE = 85,
	JKEYCODE_MEDIA_STOP = 86,
	JKEYCODE_MEDIA_NEXT = 87,
	JKEYCODE_MEDIA_PREVIOUS = 88,
	JKEYCODE_MEDIA_REWIND = 89,
	JKEYCODE_MEDIA_FAST_FORWARD = 90
};

// five-way navigation control
enum {
	JKEYCODE_DPAD_UP = 19,
	JKEYCODE_DPAD_DOWN = 20,
	JKEYCODE_DPAD_LEFT = 21,
	JKEYCODE_DPAD_RIGHT = 22,
	JKEYCODE_DPAD_CENTER = 23
};

// meta modifier
enum {
	JMETA_SHIFT = 0x01,
	JMETA_ALT = 0x02,
	JMETA_SYM = 0x04,
	JMETA_CTRL = 0x1000
};

// map android key codes to our kbd codes
static const Common::KeyCode jkeymap[] = {
	Common::KEYCODE_INVALID, // KEYCODE_UNKNOWN
	Common::KEYCODE_INVALID, // KEYCODE_SOFT_LEFT
	Common::KEYCODE_INVALID, // KEYCODE_SOFT_RIGHT
	Common::KEYCODE_INVALID, // KEYCODE_HOME
	Common::KEYCODE_INVALID, // KEYCODE_BACK
	Common::KEYCODE_INVALID, // KEYCODE_CALL
	Common::KEYCODE_INVALID, // KEYCODE_ENDCALL
	Common::KEYCODE_0, // KEYCODE_0
	Common::KEYCODE_1, // KEYCODE_1
	Common::KEYCODE_2, // KEYCODE_2
	Common::KEYCODE_3, // KEYCODE_3
	Common::KEYCODE_4, // KEYCODE_4
	Common::KEYCODE_5, // KEYCODE_5
	Common::KEYCODE_6, // KEYCODE_6
	Common::KEYCODE_7, // KEYCODE_7
	Common::KEYCODE_8, // KEYCODE_8
	Common::KEYCODE_9, // KEYCODE_9
	Common::KEYCODE_ASTERISK, // KEYCODE_STAR
	Common::KEYCODE_HASH, // KEYCODE_POUND
	Common::KEYCODE_INVALID, // KEYCODE_DPAD_UP
	Common::KEYCODE_INVALID, // KEYCODE_DPAD_DOWN
	Common::KEYCODE_INVALID, // KEYCODE_DPAD_LEFT
	Common::KEYCODE_INVALID, // KEYCODE_DPAD_RIGHT
	Common::KEYCODE_INVALID, // KEYCODE_DPAD_CENTER
	Common::KEYCODE_INVALID, // KEYCODE_VOLUME_UP
	Common::KEYCODE_INVALID, // KEYCODE_VOLUME_DOWN
	Common::KEYCODE_INVALID, // KEYCODE_POWER
	Common::KEYCODE_INVALID, // KEYCODE_CAMERA
	Common::KEYCODE_INVALID, // KEYCODE_CLEAR
	Common::KEYCODE_a, // KEYCODE_A
	Common::KEYCODE_b, // KEYCODE_B
	Common::KEYCODE_c, // KEYCODE_C
	Common::KEYCODE_d, // KEYCODE_D
	Common::KEYCODE_e, // KEYCODE_E
	Common::KEYCODE_f, // KEYCODE_F
	Common::KEYCODE_g, // KEYCODE_G
	Common::KEYCODE_h, // KEYCODE_H
	Common::KEYCODE_i, // KEYCODE_I
	Common::KEYCODE_j, // KEYCODE_J
	Common::KEYCODE_k, // KEYCODE_K
	Common::KEYCODE_l, // KEYCODE_L
	Common::KEYCODE_m, // KEYCODE_M
	Common::KEYCODE_n, // KEYCODE_N
	Common::KEYCODE_o, // KEYCODE_O
	Common::KEYCODE_p, // KEYCODE_P
	Common::KEYCODE_q, // KEYCODE_Q
	Common::KEYCODE_r, // KEYCODE_R
	Common::KEYCODE_s, // KEYCODE_S
	Common::KEYCODE_t, // KEYCODE_T
	Common::KEYCODE_u, // KEYCODE_U
	Common::KEYCODE_v, // KEYCODE_V
	Common::KEYCODE_w, // KEYCODE_W
	Common::KEYCODE_x, // KEYCODE_X
	Common::KEYCODE_y, // KEYCODE_Y
	Common::KEYCODE_z, // KEYCODE_Z
	Common::KEYCODE_COMMA, // KEYCODE_COMMA
	Common::KEYCODE_PERIOD, // KEYCODE_PERIOD
	Common::KEYCODE_LALT, // KEYCODE_ALT_LEFT
	Common::KEYCODE_RALT, // KEYCODE_ALT_RIGHT
	Common::KEYCODE_LSHIFT, // KEYCODE_SHIFT_LEFT
	Common::KEYCODE_RSHIFT, // KEYCODE_SHIFT_RIGHT
	Common::KEYCODE_TAB, // KEYCODE_TAB
	Common::KEYCODE_SPACE, // KEYCODE_SPACE
	Common::KEYCODE_LCTRL, // KEYCODE_SYM
	Common::KEYCODE_INVALID, // KEYCODE_EXPLORER
	Common::KEYCODE_INVALID, // KEYCODE_ENVELOPE
	Common::KEYCODE_RETURN, // KEYCODE_ENTER
	Common::KEYCODE_BACKSPACE, // KEYCODE_DEL
	Common::KEYCODE_BACKQUOTE, // KEYCODE_GRAVE
	Common::KEYCODE_MINUS, // KEYCODE_MINUS
	Common::KEYCODE_EQUALS, // KEYCODE_EQUALS
	Common::KEYCODE_LEFTPAREN, // KEYCODE_LEFT_BRACKET
	Common::KEYCODE_RIGHTPAREN, // KEYCODE_RIGHT_BRACKET
	Common::KEYCODE_BACKSLASH, // KEYCODE_BACKSLASH
	Common::KEYCODE_SEMICOLON, // KEYCODE_SEMICOLON
	Common::KEYCODE_QUOTE, // KEYCODE_APOSTROPHE
	Common::KEYCODE_SLASH, // KEYCODE_SLASH
	Common::KEYCODE_AT, // KEYCODE_AT
	Common::KEYCODE_INVALID, // KEYCODE_NUM
	Common::KEYCODE_INVALID, // KEYCODE_HEADSETHOOK
	Common::KEYCODE_INVALID, // KEYCODE_FOCUS
	Common::KEYCODE_PLUS, // KEYCODE_PLUS
	Common::KEYCODE_INVALID, // KEYCODE_MENU
	Common::KEYCODE_INVALID, // KEYCODE_NOTIFICATION
	Common::KEYCODE_INVALID, // KEYCODE_SEARCH
	Common::KEYCODE_INVALID, // KEYCODE_MEDIA_PLAY_PAUSE
	Common::KEYCODE_INVALID, // KEYCODE_MEDIA_STOP
	Common::KEYCODE_INVALID, // KEYCODE_MEDIA_NEXT
	Common::KEYCODE_INVALID, // KEYCODE_MEDIA_PREVIOUS
	Common::KEYCODE_INVALID, // KEYCODE_MEDIA_REWIND
	Common::KEYCODE_INVALID, // KEYCODE_MEDIA_FAST_FORWARD
	Common::KEYCODE_INVALID, // KEYCODE_MUTE
	Common::KEYCODE_PAGEUP, // KEYCODE_PAGE_UP
	Common::KEYCODE_PAGEDOWN // KEYCODE_PAGE_DOWN
};

// floating point. use sparingly
template<class T>
static inline T scalef(T in, float numerator, float denominator) {
	return static_cast<float>(in) * numerator / denominator;
}

static const int kQueuedInputEventDelay = 50;

void OSystem_Android::setupKeymapper() {
#ifdef ENABLE_KEYMAPPER
	using namespace Common;

	Keymapper *mapper = getEventManager()->getKeymapper();

	HardwareInputSet *inputSet = new HardwareInputSet();

	keySet->addHardwareInput(
		new HardwareInput("n", KeyState(KEYCODE_n), "n (vk)"));

	mapper->registerHardwareInputSet(inputSet);

	Keymap *globalMap = new Keymap(kGlobalKeymapName);
	Action *act;

	act = new Action(globalMap, "VIRT", "Display keyboard");
	act->addKeyEvent(KeyState(KEYCODE_F7, ASCII_F7, 0));

	mapper->addGlobalKeymap(globalMap);

	mapper->pushKeymap(kGlobalKeymapName);
#endif
}

void OSystem_Android::warpMouse(int x, int y) {
	ENTER("%d, %d", x, y);

	Common::Event e;

	e.type = Common::EVENT_MOUSEMOVE;
	e.mouse.x = x;
	e.mouse.y = y;

	clipMouse(e.mouse);

	lockMutex(_event_queue_lock);
	_event_queue.push(e);
	unlockMutex(_event_queue_lock);
}

void OSystem_Android::clipMouse(Common::Point &p) {
	const GLESBaseTexture *tex;

	if (_show_overlay)
		tex = _overlay_texture;
	else
		tex = _game_texture;

	p.x = CLIP(p.x, int16(0), int16(tex->width() - 1));
	p.y = CLIP(p.y, int16(0), int16(tex->height() - 1));
}

void OSystem_Android::scaleMouse(Common::Point &p, int x, int y,
									bool deductDrawRect, bool touchpadMode) {
	const GLESBaseTexture *tex;

	if (_show_overlay)
		tex = _overlay_texture;
	else
		tex = _game_texture;

	const Common::Rect &r = tex->getDrawRect();

	if (touchpadMode) {
		x = x * 100 / _touchpad_scale;
		y = y * 100 / _touchpad_scale;
	}

	if (deductDrawRect) {
		x -= r.left;
		y -= r.top;
	}

	p.x = scalef(x, tex->width(), r.width());
	p.y = scalef(y, tex->height(), r.height());
}

void OSystem_Android::updateEventScale() {
	const GLESBaseTexture *tex;

	if (_show_overlay)
		tex = _overlay_texture;
	else
		tex = _game_texture;

	_eventScaleY = 100 * 480 / tex->height();
	_eventScaleX = 100 * 640 / tex->width();
}

void OSystem_Android::pushEvent(int type, int arg1, int arg2, int arg3,
								int arg4, int arg5) {
	Common::Event e;

	switch (type) {
	case JE_SYS_KEY:
		switch (arg1) {
		case JACTION_DOWN:
			e.type = Common::EVENT_KEYDOWN;
			break;
		case JACTION_UP:
			e.type = Common::EVENT_KEYUP;
			break;
		default:
			LOGE("unhandled jaction on system key: %d", arg1);
			return;
		}

		switch (arg2) {

		// special case. we'll only get it's up event
		case JKEYCODE_BACK:
			e.kbd.keycode = Common::KEYCODE_ESCAPE;
			e.kbd.ascii = Common::ASCII_ESCAPE;

			lockMutex(_event_queue_lock);
			e.type = Common::EVENT_KEYDOWN;
			_event_queue.push(e);
			e.type = Common::EVENT_KEYUP;
			_event_queue.push(e);
			unlockMutex(_event_queue_lock);

			return;

		// special case. we'll only get it's up event
		case JKEYCODE_MENU:
			e.type = Common::EVENT_MAINMENU;

			lockMutex(_event_queue_lock);
			_event_queue.push(e);
			unlockMutex(_event_queue_lock);

			return;

		case JKEYCODE_CAMERA:
		case JKEYCODE_SEARCH:
			if (arg1 == JACTION_DOWN)
				e.type = Common::EVENT_RBUTTONDOWN;
			else
				e.type = Common::EVENT_RBUTTONUP;

			e.mouse = getEventManager()->getMousePos();

			lockMutex(_event_queue_lock);
			_event_queue.push(e);
			unlockMutex(_event_queue_lock);

			return;

		default:
			LOGW("unmapped system key: %d", arg2);
			return;
		}

		break;

	case JE_KEY:
		switch (arg1) {
		case JACTION_DOWN:
			e.type = Common::EVENT_KEYDOWN;
			break;
		case JACTION_UP:
			e.type = Common::EVENT_KEYUP;
			break;
		default:
			LOGE("unhandled jaction on key: %d", arg1);
			return;
		}

		if (arg2 < 1 || arg2 > ARRAYSIZE(jkeymap)) {
			if (arg3 < 1) {
				LOGE("received invalid keycode: %d (%d)", arg2, arg3);
				return;
			} else {
				// lets bet on the ascii code
				e.kbd.keycode = Common::KEYCODE_INVALID;
			}
		} else {
			e.kbd.keycode = jkeymap[arg2];
		}

		if (arg5 > 0)
			e.synthetic = true;

		// map special keys to 'our' ascii codes
		switch (e.kbd.keycode) {
		case Common::KEYCODE_BACKSPACE:
			e.kbd.ascii = Common::ASCII_BACKSPACE;
			break;
		case Common::KEYCODE_TAB:
			e.kbd.ascii = Common::ASCII_TAB;
			break;
		case Common::KEYCODE_RETURN:
			e.kbd.ascii = Common::ASCII_RETURN;
			break;
		case Common::KEYCODE_ESCAPE:
			e.kbd.ascii = Common::ASCII_ESCAPE;
			break;
		case Common::KEYCODE_SPACE:
			e.kbd.ascii = Common::ASCII_SPACE;
			break;
		case Common::KEYCODE_F1:
			e.kbd.ascii = Common::ASCII_F1;
			break;
		case Common::KEYCODE_F2:
			e.kbd.ascii = Common::ASCII_F2;
			break;
		case Common::KEYCODE_F3:
			e.kbd.ascii = Common::ASCII_F3;
			break;
		case Common::KEYCODE_F4:
			e.kbd.ascii = Common::ASCII_F4;
			break;
		case Common::KEYCODE_F5:
			e.kbd.ascii = Common::ASCII_F5;
			break;
		case Common::KEYCODE_F6:
			e.kbd.ascii = Common::ASCII_F6;
			break;
		case Common::KEYCODE_F7:
			e.kbd.ascii = Common::ASCII_F7;
			break;
		case Common::KEYCODE_F8:
			e.kbd.ascii = Common::ASCII_F8;
			break;
		case Common::KEYCODE_F9:
			e.kbd.ascii = Common::ASCII_F9;
			break;
		case Common::KEYCODE_F10:
			e.kbd.ascii = Common::ASCII_F10;
			break;
		case Common::KEYCODE_F11:
			e.kbd.ascii = Common::ASCII_F11;
			break;
		case Common::KEYCODE_F12:
			e.kbd.ascii = Common::ASCII_F12;
			break;
		default:
			e.kbd.ascii = arg3;
			break;
		}

		if (arg4 & JMETA_SHIFT)
			e.kbd.flags |= Common::KBD_SHIFT;
		// JMETA_ALT is Fn on physical keyboards!
		// when mapping this to ALT - as we know it from PC keyboards - all
		// Fn combos will be broken (like Fn+q, which needs to end as 1 and
		// not ALT+1). Do not want.
		//if (arg4 & JMETA_ALT)
		//	e.kbd.flags |= Common::KBD_ALT;
		if (arg4 & (JMETA_SYM | JMETA_CTRL))
			e.kbd.flags |= Common::KBD_CTRL;

		lockMutex(_event_queue_lock);
		_event_queue.push(e);
		unlockMutex(_event_queue_lock);

		return;

	case JE_DPAD:
		switch (arg2) {
		case JKEYCODE_DPAD_UP:
		case JKEYCODE_DPAD_DOWN:
		case JKEYCODE_DPAD_LEFT:
		case JKEYCODE_DPAD_RIGHT:
			if (arg1 != JACTION_DOWN)
				return;

			e.type = Common::EVENT_MOUSEMOVE;

			e.mouse = getEventManager()->getMousePos();

			{
				int16 *c;
				int s;

				if (arg2 == JKEYCODE_DPAD_UP || arg2 == JKEYCODE_DPAD_DOWN) {
					c = &e.mouse.y;
					s = _eventScaleY;
				} else {
					c = &e.mouse.x;
					s = _eventScaleX;
				}

				// the longer the button held, the faster the pointer is
				// TODO put these values in some option dlg?
				int f = CLIP(arg4, 1, 8) * _dpad_scale * 100 / s;

				if (arg2 == JKEYCODE_DPAD_UP || arg2 == JKEYCODE_DPAD_LEFT)
					*c -= f;
				else
					*c += f;
			}

			clipMouse(e.mouse);

			lockMutex(_event_queue_lock);
			_event_queue.push(e);
			unlockMutex(_event_queue_lock);

			return;

		case JKEYCODE_DPAD_CENTER:
			switch (arg1) {
			case JACTION_DOWN:
				e.type = Common::EVENT_LBUTTONDOWN;
				break;
			case JACTION_UP:
				e.type = Common::EVENT_LBUTTONUP;
				break;
			default:
				LOGE("unhandled jaction on dpad key: %d", arg1);
				return;
			}

			e.mouse = getEventManager()->getMousePos();

			lockMutex(_event_queue_lock);
			_event_queue.push(e);
			unlockMutex(_event_queue_lock);

			return;
		}

	case JE_DOWN:
		_touch_pt_down = getEventManager()->getMousePos();
		_touch_pt_scroll.x = -1;
		_touch_pt_scroll.y = -1;
		break;

	case JE_SCROLL:
		e.type = Common::EVENT_MOUSEMOVE;

		if (_touchpad_mode) {
			if (_touch_pt_scroll.x == -1 && _touch_pt_scroll.y == -1) {
				_touch_pt_scroll.x = arg3;
				_touch_pt_scroll.y = arg4;
				return;
			}

			scaleMouse(e.mouse, arg3 - _touch_pt_scroll.x,
						arg4 - _touch_pt_scroll.y, false, true);
			e.mouse += _touch_pt_down;
			clipMouse(e.mouse);
		} else {
			scaleMouse(e.mouse, arg3, arg4);
			clipMouse(e.mouse);
		}

		lockMutex(_event_queue_lock);
		_event_queue.push(e);
		unlockMutex(_event_queue_lock);

		return;

	case JE_TAP:
		if (_fingersDown > 0) {
			_fingersDown = 0;
			return;
		}

		e.type = Common::EVENT_MOUSEMOVE;

		if (_touchpad_mode) {
			e.mouse = getEventManager()->getMousePos();
		} else {
			scaleMouse(e.mouse, arg1, arg2);
			clipMouse(e.mouse);
		}

		{
			Common::EventType down, up;

			// TODO put these values in some option dlg?
			if (arg3 > 1000) {
				down = Common::EVENT_MBUTTONDOWN;
				up = Common::EVENT_MBUTTONUP;
			} else if (arg3 > 500) {
				down = Common::EVENT_RBUTTONDOWN;
				up = Common::EVENT_RBUTTONUP;
			} else {
				down = Common::EVENT_LBUTTONDOWN;
				up = Common::EVENT_LBUTTONUP;
			}

			lockMutex(_event_queue_lock);

			if (_queuedEventTime)
				_event_queue.push(_queuedEvent);

			if (!_touchpad_mode)
				_event_queue.push(e);

			e.type = down;
			_event_queue.push(e);

			e.type = up;
			_queuedEvent = e;
			_queuedEventTime = getMillis() + kQueuedInputEventDelay;

			unlockMutex(_event_queue_lock);
		}

		return;

	case JE_DOUBLE_TAP:
		e.type = Common::EVENT_MOUSEMOVE;

		if (_touchpad_mode) {
			e.mouse = getEventManager()->getMousePos();
		} else {
			scaleMouse(e.mouse, arg1, arg2);
			clipMouse(e.mouse);
		}

		{
			Common::EventType dptype = Common::EVENT_INVALID;

			switch (arg3) {
			case JACTION_DOWN:
				dptype = Common::EVENT_LBUTTONDOWN;
				_touch_pt_dt.x = -1;
				_touch_pt_dt.y = -1;
				break;
			case JACTION_UP:
				dptype = Common::EVENT_LBUTTONUP;
				break;
			// held and moved
			case JACTION_MULTIPLE:
				if (_touch_pt_dt.x == -1 && _touch_pt_dt.y == -1) {
					_touch_pt_dt.x = arg1;
					_touch_pt_dt.y = arg2;
					return;
				}

				dptype = Common::EVENT_MOUSEMOVE;

				if (_touchpad_mode) {
					scaleMouse(e.mouse, arg1 - _touch_pt_dt.x,
								arg2 - _touch_pt_dt.y, false, true);
					e.mouse += _touch_pt_down;

					clipMouse(e.mouse);
				}

				break;
			default:
				LOGE("unhandled jaction on double tap: %d", arg3);
				return;
			}

			lockMutex(_event_queue_lock);
			_event_queue.push(e);
			e.type = dptype;
			_event_queue.push(e);
			unlockMutex(_event_queue_lock);
		}

		return;

	case JE_MULTI:
		switch (arg2) {
		case JACTION_POINTER_DOWN:
			if (arg1 > _fingersDown)
				_fingersDown = arg1;

			return;

		case JACTION_POINTER_UP:
			if (arg1 != _fingersDown)
				return;

			{
				Common::EventType up;

				switch (_fingersDown) {
				case 1:
					e.type = Common::EVENT_RBUTTONDOWN;
					up = Common::EVENT_RBUTTONUP;
					break;
				case 2:
					e.type = Common::EVENT_MBUTTONDOWN;
					up = Common::EVENT_MBUTTONUP;
					break;
				default:
					LOGD("unmapped multi tap: %d", _fingersDown);
					return;
				}

				e.mouse = getEventManager()->getMousePos();

				lockMutex(_event_queue_lock);

				if (_queuedEventTime)
					_event_queue.push(_queuedEvent);

				_event_queue.push(e);

				e.type = up;
				_queuedEvent = e;
				_queuedEventTime = getMillis() + kQueuedInputEventDelay;

				unlockMutex(_event_queue_lock);
				return;

			default:
				LOGE("unhandled jaction on multi tap: %d", arg2);
				return;
			}
		}

		return;

	case JE_BALL:
		e.mouse = getEventManager()->getMousePos();

		switch (arg1) {
		case JACTION_DOWN:
			e.type = Common::EVENT_LBUTTONDOWN;
			break;
		case JACTION_UP:
			e.type = Common::EVENT_LBUTTONUP;
			break;
		case JACTION_MULTIPLE:
			e.type = Common::EVENT_MOUSEMOVE;

			// already multiplied by 100
			e.mouse.x += arg2 * _trackball_scale / _eventScaleX;
			e.mouse.y += arg3 * _trackball_scale / _eventScaleY;

			clipMouse(e.mouse);

			break;
		default:
			LOGE("unhandled jaction on system key: %d", arg1);
			return;
		}

		lockMutex(_event_queue_lock);
		_event_queue.push(e);
		unlockMutex(_event_queue_lock);

		return;

	case JE_MOUSE_MOVE:
		e.type = Common::EVENT_MOUSEMOVE;

		scaleMouse(e.mouse, arg1, arg2);
		clipMouse(e.mouse);

		lockMutex(_event_queue_lock);
		_event_queue.push(e);
		unlockMutex(_event_queue_lock);

		return;

	case JE_LMB_DOWN:
		e.type = Common::EVENT_LBUTTONDOWN;

		scaleMouse(e.mouse, arg1, arg2);
		clipMouse(e.mouse);

		lockMutex(_event_queue_lock);
		_event_queue.push(e);
		unlockMutex(_event_queue_lock);

		return;

	case JE_LMB_UP:
		e.type = Common::EVENT_LBUTTONUP;

		scaleMouse(e.mouse, arg1, arg2);
		clipMouse(e.mouse);

		lockMutex(_event_queue_lock);
		_event_queue.push(e);
		unlockMutex(_event_queue_lock);

		return;

	case JE_RMB_DOWN:
		e.type = Common::EVENT_RBUTTONDOWN;

		scaleMouse(e.mouse, arg1, arg2);
		clipMouse(e.mouse);

		lockMutex(_event_queue_lock);
		_event_queue.push(e);
		unlockMutex(_event_queue_lock);

		return;

	case JE_RMB_UP:
		e.type = Common::EVENT_RBUTTONUP;

		scaleMouse(e.mouse, arg1, arg2);
		clipMouse(e.mouse);

		lockMutex(_event_queue_lock);
		_event_queue.push(e);
		unlockMutex(_event_queue_lock);

		return;

	case JE_QUIT:
		e.type = Common::EVENT_QUIT;

		lockMutex(_event_queue_lock);
		_event_queue.push(e);
		unlockMutex(_event_queue_lock);

		return;

	default:
		LOGE("unknown jevent type: %d", type);

		break;
	}
}

bool OSystem_Android::pollEvent(Common::Event &event) {
	//ENTER();

	if (pthread_self() == _main_thread) {
		if (_screen_changeid != JNI::surface_changeid) {
			if (JNI::egl_surface_width > 0 && JNI::egl_surface_height > 0) {
				// surface changed
				JNI::deinitSurface();
				initSurface();
				initViewport();
				updateScreenRect();
				updateEventScale();

				// double buffered, flip twice
				clearScreen(kClearUpdate, 2);

				event.type = Common::EVENT_SCREEN_CHANGED;

				return true;
			} else {
				// surface lost
				deinitSurface();
			}
		}

		if (JNI::pause) {
			deinitSurface();

			LOGD("main thread going to sleep");
			sem_wait(&JNI::pause_sem);
			LOGD("main thread woke up");
		}
	}

	lockMutex(_event_queue_lock);

	if (_queuedEventTime && (getMillis() > _queuedEventTime)) {
		event = _queuedEvent;
		_queuedEventTime = 0;
		unlockMutex(_event_queue_lock);
		return true;
	}

	if (_event_queue.empty()) {
		unlockMutex(_event_queue_lock);
		return false;
	}

	event = _event_queue.pop();

	unlockMutex(_event_queue_lock);

	if (event.type == Common::EVENT_MOUSEMOVE) {
		const Common::Point &m = getEventManager()->getMousePos();

		if (m != event.mouse)
			_force_redraw = true;
	}

	return true;
}

#endif
