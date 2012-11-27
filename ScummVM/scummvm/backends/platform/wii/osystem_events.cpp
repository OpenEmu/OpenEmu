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
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include <unistd.h>
#include <malloc.h>

#include "osystem.h"

#include <ogc/lwp_watchdog.h>
#ifndef GAMECUBE
#include <wiiuse/wpad.h>
#endif
#ifdef USE_WII_KBD
#include <wiikeyboard/keyboard.h>
#endif

#include "common/config-manager.h"
#include "backends/timer/default/default-timer.h"

#define TIMER_THREAD_STACKSIZE (1024 * 32)
#define TIMER_THREAD_PRIO 64

#define PAD_CHECK_TIME 40

#ifndef GAMECUBE
#define PADS_A (PAD_BUTTON_A | (WPAD_BUTTON_A << 16))
#define PADS_B (PAD_BUTTON_B | (WPAD_BUTTON_B << 16))
#define PADS_X (PAD_BUTTON_X | (WPAD_BUTTON_MINUS << 16))
#define PADS_Y (PAD_BUTTON_Y | (WPAD_BUTTON_PLUS << 16))
#define PADS_R (PAD_TRIGGER_R | (WPAD_BUTTON_1 << 16))
#define PADS_Z (PAD_TRIGGER_Z | (WPAD_BUTTON_2 << 16))
#define PADS_START (PAD_BUTTON_START | (WPAD_BUTTON_HOME << 16))
#define PADS_UP (PAD_BUTTON_UP | (WPAD_BUTTON_UP << 16))
#define PADS_DOWN (PAD_BUTTON_DOWN | (WPAD_BUTTON_DOWN << 16))
#define PADS_LEFT (PAD_BUTTON_LEFT | (WPAD_BUTTON_LEFT << 16))
#define PADS_RIGHT (PAD_BUTTON_RIGHT | (WPAD_BUTTON_RIGHT << 16))
#else
#define PADS_A PAD_BUTTON_A
#define PADS_B PAD_BUTTON_B
#define PADS_X PAD_BUTTON_X
#define PADS_Y PAD_BUTTON_Y
#define PADS_R PAD_TRIGGER_R
#define PADS_Z PAD_TRIGGER_Z
#define PADS_START PAD_BUTTON_START
#define PADS_UP PAD_BUTTON_UP
#define PADS_DOWN PAD_BUTTON_DOWN
#define PADS_LEFT PAD_BUTTON_LEFT
#define PADS_RIGHT PAD_BUTTON_RIGHT
#endif

#ifdef USE_WII_KBD
static int keymap[][2] = {
	{ KS_Return, Common::KEYCODE_RETURN },
	{ KS_Up, Common::KEYCODE_UP },
	{ KS_Down, Common::KEYCODE_DOWN },
	{ KS_Left, Common::KEYCODE_LEFT },
	{ KS_Right, Common::KEYCODE_RIGHT },
	{ KS_Shift_L, Common::KEYCODE_LSHIFT },
	{ KS_Shift_R, Common::KEYCODE_RSHIFT },
	{ KS_Control_L, Common::KEYCODE_LCTRL },
	{ KS_Control_R, Common::KEYCODE_RCTRL },
	{ KS_Alt_L, Common::KEYCODE_LALT },
	{ KS_Alt_R, Common::KEYCODE_RALT },
	{ KS_Meta_L, Common::KEYCODE_LMETA },
	{ KS_Meta_R, Common::KEYCODE_RMETA },
	{ KS_KP_0, Common::KEYCODE_KP0 },
	{ KS_KP_1, Common::KEYCODE_KP1 },
	{ KS_KP_2, Common::KEYCODE_KP2 },
	{ KS_KP_3, Common::KEYCODE_KP3 },
	{ KS_KP_4, Common::KEYCODE_KP4 },
	{ KS_KP_5, Common::KEYCODE_KP5 },
	{ KS_KP_6, Common::KEYCODE_KP6 },
	{ KS_KP_7, Common::KEYCODE_KP7 },
	{ KS_KP_8, Common::KEYCODE_KP8 },
	{ KS_KP_9, Common::KEYCODE_KP9 },
	{ KS_Home, Common::KEYCODE_HOME },
	{ KS_Insert, Common::KEYCODE_INSERT },
	{ KS_End, Common::KEYCODE_END },
	{ KS_Prior, Common::KEYCODE_PAGEUP },
	{ KS_Next, Common::KEYCODE_PAGEDOWN },
	{ KS_f1, Common::KEYCODE_F1 },
	{ KS_f2, Common::KEYCODE_F2 },
	{ KS_f3, Common::KEYCODE_F3 },
	{ KS_f4, Common::KEYCODE_F4 },
	{ KS_f5, Common::KEYCODE_F5 },
	{ KS_f6, Common::KEYCODE_F6 },
	{ KS_f7, Common::KEYCODE_F7 },
	{ KS_f8, Common::KEYCODE_F8 },
	{ KS_f9, Common::KEYCODE_F9 },
	{ KS_f10, Common::KEYCODE_F10 },
	{ KS_f11, Common::KEYCODE_F11 },
	{ KS_f12, Common::KEYCODE_F12 },
	{ KS_f13, Common::KEYCODE_F13 },
	{ KS_f14, Common::KEYCODE_F14 },
	{ KS_f15, Common::KEYCODE_F15 },
	{ KS_F1, Common::KEYCODE_F1 },
	{ KS_F2, Common::KEYCODE_F2 },
	{ KS_F3, Common::KEYCODE_F3 },
	{ KS_F4, Common::KEYCODE_F4 },
	{ KS_F5, Common::KEYCODE_F5 },
	{ KS_F6, Common::KEYCODE_F6 },
	{ KS_F7, Common::KEYCODE_F7 },
	{ KS_F8, Common::KEYCODE_F8 },
	{ KS_F9, Common::KEYCODE_F9 },
	{ KS_F10, Common::KEYCODE_F10 },
	{ KS_F11, Common::KEYCODE_F11 },
	{ KS_F12, Common::KEYCODE_F12 },
	{ KS_F13, Common::KEYCODE_F13 },
	{ KS_F14, Common::KEYCODE_F14 },
	{ KS_F15, Common::KEYCODE_F15 },
	{ KS_KP_Separator, Common::KEYCODE_KP_PERIOD },
	{ KS_KP_Subtract, Common::KEYCODE_KP_DIVIDE },
	{ KS_KP_Multiply, Common::KEYCODE_KP_MULTIPLY },
	{ KS_KP_Add, Common::KEYCODE_KP_PLUS },
	{ KS_KP_Subtract, Common::KEYCODE_KP_MINUS },
	{ KS_KP_Equal, Common::KEYCODE_KP_EQUALS },
	{ KS_KP_Enter, Common::KEYCODE_KP_ENTER },
	{ 0, 0 }
};
#endif

static lwpq_t timer_queue;
static lwp_t timer_thread;
static u8 *timer_stack;
static bool timer_thread_running = false;
static bool timer_thread_quit = false;

static void * timer_thread_func(void *arg) {
	while (!timer_thread_quit) {
		DefaultTimerManager *tm =
			(DefaultTimerManager *) g_system->getTimerManager();
		tm->handler();

		usleep(1000 * 10);
	}

	return NULL;
}

void OSystem_Wii::initEvents() {
	timer_thread_quit = false;

	timer_stack = (u8 *) memalign(32, TIMER_THREAD_STACKSIZE);
	if (timer_stack) {
		memset(timer_stack, 0, TIMER_THREAD_STACKSIZE);

		LWP_InitQueue(&timer_queue);

		s32 res = LWP_CreateThread(&timer_thread, timer_thread_func, NULL,
									timer_stack, TIMER_THREAD_STACKSIZE,
									TIMER_THREAD_PRIO);

		if (res) {
			printf("ERROR creating timer thread: %d\n", res);
			LWP_CloseQueue(timer_queue);
		}

		timer_thread_running = res == 0;
	}

#ifndef GAMECUBE
	WPAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetIdleTimeout(120);
#endif

	_padSensitivity = 64 - ConfMan.getInt("wii_pad_sensitivity");
	_padAcceleration = 9 - ConfMan.getInt("wii_pad_acceleration");

#ifdef USE_WII_KBD
	_kbd_active = KEYBOARD_Init() >= 0;
#endif
}

void OSystem_Wii::deinitEvents() {
	if (timer_thread_running) {
		timer_thread_quit = true;
		LWP_ThreadBroadcast(timer_queue);

		LWP_JoinThread(timer_thread, NULL);
		LWP_CloseQueue(timer_queue);

		free(timer_stack);
		timer_thread_running = false;
	}

#ifdef USE_WII_KBD
	if (_kbd_active)
		KEYBOARD_Deinit();
#endif

#ifndef GAMECUBE
	WPAD_Shutdown();
#endif
}

void OSystem_Wii::updateEventScreenResolution() {
#ifndef GAMECUBE
	WPAD_SetVRes(WPAD_CHAN_0, _currentWidth + _currentWidth / 5,
					_currentHeight + _currentHeight / 5);
#endif
}

#ifdef USE_WII_KBD
bool OSystem_Wii::pollKeyboard(Common::Event &event) {
	int i;
	keyboard_event kbdEvent;

	if (!KEYBOARD_GetEvent(&kbdEvent) > 0)
		return false;

	switch (kbdEvent.type) {
	case KEYBOARD_PRESSED:
		event.type = Common::EVENT_KEYDOWN;
		break;

	case KEYBOARD_RELEASED:
		event.type = Common::EVENT_KEYUP;
		break;

	case KEYBOARD_CONNECTED:
		printf("keyboard connected\n");
		return false;

	case KEYBOARD_DISCONNECTED:
		printf("keyboard disconnected\n");
		return false;

	default:
		return false;
	}

	if (MOD_ONESET(kbdEvent.modifiers, MOD_ANYSHIFT))
		event.kbd.flags |= Common::KBD_SHIFT;
	if (MOD_ONESET(kbdEvent.modifiers, MOD_ANYCONTROL))
		event.kbd.flags |= Common::KBD_CTRL;
	if (MOD_ONESET(kbdEvent.modifiers, MOD_ANYMETA))
		event.kbd.flags |= Common::KBD_ALT;

	i = 0;
	while (keymap[i][0] != 0) {
		if (keymap[i][0] == kbdEvent.symbol) {
			event.kbd.keycode = static_cast<Common::KeyCode>(keymap[i][1]);
			event.kbd.ascii = 0;
			return true;
		}

		i++;
	}

	// skip unmapped special keys
	if (kbdEvent.symbol > 0xff)
		return false;

	event.kbd.keycode = static_cast<Common::KeyCode>(kbdEvent.symbol);
	event.kbd.ascii = kbdEvent.symbol;

	return true;
}
#endif

#define PAD_EVENT(pad_button, kbd_keycode, kbd_ascii, modifier) \
	do { \
		if ((bd | bu) & pad_button) { \
			if (bd & pad_button) \
				event.type = Common::EVENT_KEYDOWN; \
			else \
				event.type = Common::EVENT_KEYUP; \
			event.kbd.keycode = kbd_keycode; \
			event.kbd.ascii = kbd_ascii; \
			event.kbd.flags = modifier; \
			return true; \
		} \
	} while (0)

bool OSystem_Wii::pollEvent(Common::Event &event) {
	if ((reset_btn_pressed || power_btn_pressed) && !_event_quit) {
		_event_quit = true;
		event.type = Common::EVENT_QUIT;

		printf("quit event\n");

		return true;
	}

	if (needsScreenUpdate())
		updateScreen();

	u32 bd = 0, bh = 0, bu = 0;

	if (PAD_ScanPads() & 1) {
		bd = PAD_ButtonsDown(0);
		bh = PAD_ButtonsHeld(0);
		bu = PAD_ButtonsUp(0);
	}

#ifndef GAMECUBE
	WPAD_ScanPads();

	s32 res = WPAD_Probe(0, NULL);

	if (res == WPAD_ERR_NONE) {
		bd |= WPAD_ButtonsDown(0) << 16;
		bh |= WPAD_ButtonsHeld(0) << 16;
		bu |= WPAD_ButtonsUp(0) << 16;
	}
#endif

	if (bd || bu) {
		byte flags = 0;

		if (bh & PADS_UP) {
			PAD_EVENT(PADS_START, Common::KEYCODE_F5, Common::ASCII_F5,
						Common::KBD_CTRL);

			if (bd & PADS_R) {
				_consoleVisible = !_consoleVisible;
				return false;
			}

			flags = Common::KBD_SHIFT;
		}

		if (bd & PADS_R) {
			showOptionsDialog();
			return false;
		}

		if (bd & PADS_RIGHT) {
			event.type = Common::EVENT_PREDICTIVE_DIALOG;
			return true;
		}

		PAD_EVENT(PADS_Z, Common::KEYCODE_RETURN, Common::ASCII_RETURN, flags);
		PAD_EVENT(PADS_X, Common::KEYCODE_ESCAPE, Common::ASCII_ESCAPE, flags);
		PAD_EVENT(PADS_Y, Common::KEYCODE_PERIOD, '.', flags);
		PAD_EVENT(PADS_START, Common::KEYCODE_F5, Common::ASCII_F5, flags);
		PAD_EVENT(PADS_UP, Common::KEYCODE_LSHIFT, 0, flags);
		PAD_EVENT(PADS_DOWN, Common::KEYCODE_F7, Common::ASCII_F7, flags);
		//PAD_EVENT(PADS_LEFT, Common::KEYCODE_F8, Common::ASCII_F8, 0);

		if ((bd | bu) & (PADS_A | PADS_B)) {
			if (bd & PADS_A)
				event.type = Common::EVENT_LBUTTONDOWN;
			else if (bu & PADS_A)
				event.type = Common::EVENT_LBUTTONUP;
			else if (bd & PADS_B)
				event.type = Common::EVENT_RBUTTONDOWN;
			else if (bu & PADS_B)
				event.type = Common::EVENT_RBUTTONUP;

			event.mouse.x = _mouseX;
			event.mouse.y = _mouseY;

			return true;
		}
	}

	s32 mx = _mouseX;
	s32 my = _mouseY;

#ifndef GAMECUBE
	if (res == WPAD_ERR_NONE) {
		struct ir_t ir;

		WPAD_IR(0, &ir);

		if (ir.valid) {
			mx = s32(ir.x) - _currentWidth / 10;
			my = s32(ir.y) - _currentHeight / 10;

			if (mx < 0)
				mx = 0;

			if (mx >= _currentWidth)
				mx = _currentWidth - 1;

			if (my < 0)
				my = 0;

			if (my >= _currentHeight)
				my = _currentHeight - 1;

			if ((mx != _mouseX) || (my != _mouseY)) {
				event.type = Common::EVENT_MOUSEMOVE;
				event.mouse.x = _mouseX = mx;
				event.mouse.y = _mouseY = my;

				return true;
			}
		}
	}
#endif

	uint32 time = getMillis();
	if (time - _lastPadCheck > PAD_CHECK_TIME) {
		_lastPadCheck = time;

		if (abs (PAD_StickX(0)) > _padSensitivity)
			mx += PAD_StickX(0) /
					(_padAcceleration * _overlayWidth / _currentWidth);
		if (abs (PAD_StickY(0)) > _padSensitivity)
			my -= PAD_StickY(0) /
					(_padAcceleration * _overlayHeight / _currentHeight);

		if (mx < 0)
			mx = 0;

		if (mx >= _currentWidth)
			mx = _currentWidth - 1;

		if (my < 0)
			my = 0;

		if (my >= _currentHeight)
			my = _currentHeight - 1;

		if ((mx != _mouseX) || (my != _mouseY)) {
			event.type = Common::EVENT_MOUSEMOVE;
			event.mouse.x = _mouseX = mx;
			event.mouse.y = _mouseY = my;

			return true;
		}
	}

#ifdef USE_WII_KBD
	if (_kbd_active && pollKeyboard(event))
		return true;
#endif

	return false;
}
