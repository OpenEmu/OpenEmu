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

#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include <kernel.h>
#include <malloc.h>
#include <assert.h>
#include <libmouse.h>
#include "backends/platform/ps2/rpckbd.h"
#include "backends/platform/ps2/ps2input.h"
#include "backends/platform/ps2/ps2pad.h"
#include "backends/platform/ps2/systemps2.h"
#include "common/events.h"
#include "common/system.h"

Ps2Input::Ps2Input(OSystem_PS2 *system, bool mouseLoaded, bool kbdLoaded) {
	_system = system;
	_mouseLoaded = mouseLoaded;
	_kbdLoaded = kbdLoaded;
	_pad = new Ps2Pad(system);
	_lastPadCheck = 0;
	_posX = _posY = _mButtons = _padLastButtons = 0;
	_padAccel = 0;
	_minx = _miny = 0;
	_maxy = 239;
	_maxx = 319;
	_keyFlags = 0;
	if (_mouseLoaded) {
		if (PS2MouseInit() >= 0) {
			PS2MouseSetReadMode(PS2MOUSE_READMODE_ABS);
			printf("PS2Mouse initialized\n");
		} else { // shouldn't happen if the drivers were correctly loaded
			printf("unable to initialize PS2Mouse!\n");
			_mouseLoaded = false;
		}
	}
	if (_kbdLoaded) {
		if (PS2KbdInit() >= 0) {
			PS2KbdSetReadmode(PS2KBD_READMODE_RAW);
			printf("PS2Kbd initialized\n");
		} else {
			printf("unable to initialize PS2Kbd!\n");
			_kbdLoaded = false;
		}
	}
}

Ps2Input::~Ps2Input(void) {
}

void Ps2Input::newRange(uint16 minx, uint16 miny, uint16 maxx, uint16 maxy) {
	_minx = minx;
	_miny = miny;
	_maxx = maxx;
	_maxy = maxy;
	if (_mouseLoaded)
		PS2MouseSetBoundary(minx, maxx, miny, maxy);
	warpTo(_posX, _posY);
}

void Ps2Input::warpTo(uint16 x, uint16 y) {
	if ((x >= _minx) && (x <= _maxx) && (y >= _miny) && (y <= _maxy)) {
		_posX = x;
		_posY = y;
	} else {
		_posX = (x < _minx) ? (_minx) : ((x > _maxx) ? (_maxx) : (x));
		_posY = (y < _miny) ? (_miny) : ((y > _maxy) ? (_maxy) : (y));
	}
	if (_mouseLoaded)
		PS2MouseSetPosition(_posX, _posY);
}

#define JOY_THRESHOLD 30
#define PAD_CHECK_TIME 20

int Ps2Input::mapKey(int key, int mod) { // copied from sdl backend
	if (key >= Common::KEYCODE_F1 && key <= Common::KEYCODE_F9) {
		return key - Common::KEYCODE_F1 + Common::ASCII_F1;
	} else if (key >= Common::KEYCODE_KP0 && key <= Common::KEYCODE_KP9) {
		return key - Common::KEYCODE_KP0 + '0';
	} else if (key >= Common::KEYCODE_UP && key <= Common::KEYCODE_PAGEDOWN) {
		return key;
	} else if (key >= 'a' && key <= 'z' && mod & Common::KBD_SHIFT) {
		return key & ~0x20;
	} else if (key >= Common::KEYCODE_NUMLOCK && key <= Common::KEYCODE_EURO) {
		return 0;
	}
	return key;
}

bool Ps2Input::pollEvent(Common::Event *event) {
	bool checkPadMouse, checkPadKbd;
	checkPadMouse = checkPadKbd = _pad->padAlive();

	if (_mouseLoaded && (PS2MouseEnum() > 0)) { // usb mouse connected
		mouse_data mData;
		PS2MouseRead(&mData);
		if ((_posX != mData.x) || (_posY != mData.y)) {
			event->mouse.x = _posX = mData.x;
			event->mouse.y = _posY = mData.y;
			event->type = Common::EVENT_MOUSEMOVE;
            return true;
		}
		if (mData.buttons != _mButtons) {
			uint16 change = _mButtons ^ mData.buttons;
			_mButtons = mData.buttons;
			if (change & (PS2MOUSE_BTN1 | PS2MOUSE_BTN2)) {
				if (change & PS2MOUSE_BTN1)
					event->type = (_mButtons & PS2MOUSE_BTN1) ? Common::EVENT_LBUTTONDOWN : Common::EVENT_LBUTTONUP;
				else
					event->type = (_mButtons & PS2MOUSE_BTN2) ? Common::EVENT_RBUTTONDOWN : Common::EVENT_RBUTTONUP;
				event->mouse.x = _posX;
				event->mouse.y = _posY;
				return true;
			}
		}
		checkPadMouse = false;
	}
	if (_kbdLoaded) { // there's no way to tell if there's actually a keyboard connected
		PS2KbdRawKey key;
		if (PS2KbdReadRaw(&key) == 1) {
			if (_usbToSdlk[key.key]) {
				if ((_usbToSdlk[key.key] == Common::KEYCODE_LSHIFT) || (_usbToSdlk[key.key] == Common::KEYCODE_RSHIFT)) {
					if (key.state & 1)
						_keyFlags |= Common::KBD_SHIFT;
					else
						_keyFlags &= ~Common::KBD_SHIFT;
				} else if ((_usbToSdlk[key.key] == Common::KEYCODE_LCTRL) || (_usbToSdlk[key.key] == Common::KEYCODE_RCTRL)) {
					if (key.state & 1)
						_keyFlags |= Common::KBD_CTRL;
					else
						_keyFlags &= ~Common::KBD_CTRL;
				} else if ((_usbToSdlk[key.key] == Common::KEYCODE_LALT) || (_usbToSdlk[key.key] == Common::KEYCODE_RALT)) {
					if (key.state & 1)
						_keyFlags |= Common::KBD_ALT;
					else
						_keyFlags &= ~Common::KBD_ALT;
				}
				if (key.state & 1) // down
					event->type = Common::EVENT_KEYDOWN;
				else
					event->type = Common::EVENT_KEYUP;
				event->kbd.flags = 0;
				event->kbd.keycode = _usbToSdlk[key.key];
				event->kbd.ascii = mapKey(_usbToSdlk[key.key], _keyFlags);
				return true;
			} else
				printf("unknown keycode %02X - %02X\n", key.state, key.key);
		}
	}
	if (checkPadMouse || checkPadKbd) {
		// no usb mouse, simulate it using the pad
        uint16 buttons;
		int16 joyh, joyv;
		_pad->readPad(&buttons, &joyh, &joyv);
		uint16 btnChange = buttons ^ _padLastButtons;

		if (checkPadMouse) {
			if (btnChange & (PAD_CROSS | PAD_CIRCLE)) {
				if (btnChange & PAD_CROSS)
					event->type = (buttons & PAD_CROSS) ?  Common::EVENT_LBUTTONDOWN : Common::EVENT_LBUTTONUP;
				else
					event->type = (buttons & PAD_CIRCLE) ? Common::EVENT_RBUTTONDOWN : Common::EVENT_RBUTTONUP;
				event->mouse.x = _posX;
				event->mouse.y = _posY;
				_padLastButtons = buttons;
				return true;
			}
			uint32 time = _system->getMillis();
			if (time - _lastPadCheck > PAD_CHECK_TIME) {
				_lastPadCheck = time;
				int16 newX = _posX;
				int16 newY = _posY;
				if ((ABS(joyh) > JOY_THRESHOLD) || (ABS(joyv) > JOY_THRESHOLD)) {
					newX += joyh / 20;
					newY += joyv / 20;
				} else if (buttons & PAD_DIR_MASK) {
					if (_padLastButtons & PAD_DIR_MASK) {
						if (_padAccel < 16)
							_padAccel++;
					} else
						_padAccel = 0;
					_padLastButtons = buttons;
					if (buttons & PAD_LEFT)
						newX -= _padAccel >> 2;
					if (buttons & PAD_RIGHT)
						newX += _padAccel >> 2;
					if (buttons & PAD_UP)
						newY -= _padAccel >> 2;
					if (buttons & PAD_DOWN)
						newY += _padAccel >> 2;
				}
				newX = ((newX < (int16)_minx) ? (_minx) : ((newX > (int16)_maxx) ? (_maxx) : ((int16)newX)));
				newY = ((newY < (int16)_miny) ? (_miny) : ((newY > (int16)_maxy) ? (_maxy) : ((int16)newY)));
				if ((_posX != newX) || (_posY != newY)) {
					event->type = Common::EVENT_MOUSEMOVE;
					event->mouse.x = _posX = newX;
					event->mouse.y = _posY = newY;
					return true;
				}
			}
		}
		if (checkPadKbd) {
			if (getKeyEvent(event, btnChange, (btnChange & buttons) != 0)) {
				_padLastButtons = buttons;
				return true;
			}
		}
	}
	return false;
}

bool Ps2Input::getKeyEvent(Common::Event *event, uint16 buttonCode, bool down) {
	// for simulating key presses with the pad
	if (buttonCode) {
		uint8 entry = 0;
		while (!(buttonCode & 1)) {
			entry++;
			buttonCode >>= 1;
		}
		if (_padCodes[entry]) {
			event->type = (down) ? Common::EVENT_KEYDOWN : Common::EVENT_KEYUP;
			event->kbd.keycode = _padCodes[entry];
			event->kbd.flags = _padFlags[entry];
			event->kbd.ascii = mapKey(_padCodes[entry], _padFlags[entry]);
			return true;
		}
	}
	return false;
}

const Common::KeyCode Ps2Input::_usbToSdlk[0x100] = {
	/* 00 */	Common::KEYCODE_INVALID,
	/* 01 */	Common::KEYCODE_INVALID,
	/* 02 */	Common::KEYCODE_INVALID,
	/* 03 */	Common::KEYCODE_INVALID,
	/* 04 */	Common::KEYCODE_a,
	/* 05 */	Common::KEYCODE_b,
	/* 06 */	Common::KEYCODE_c,
	/* 07 */	Common::KEYCODE_d,
	/* 08 */	Common::KEYCODE_e,
	/* 09 */	Common::KEYCODE_f,
	/* 0A */	Common::KEYCODE_g,
	/* 0B */	Common::KEYCODE_h,
	/* 0C */	Common::KEYCODE_i,
	/* 0D */	Common::KEYCODE_j,
	/* 0E */	Common::KEYCODE_k,
	/* 0F */	Common::KEYCODE_l,
	/* 10 */	Common::KEYCODE_m,
	/* 11 */	Common::KEYCODE_n,
	/* 12 */	Common::KEYCODE_o,
	/* 13 */	Common::KEYCODE_p,
	/* 14 */	Common::KEYCODE_q,
	/* 15 */	Common::KEYCODE_r,
	/* 16 */	Common::KEYCODE_s,
	/* 17 */	Common::KEYCODE_t,
	/* 18 */	Common::KEYCODE_u,
	/* 19 */	Common::KEYCODE_v,
	/* 1A */	Common::KEYCODE_w,
	/* 1B */	Common::KEYCODE_x,
	/* 1C */	Common::KEYCODE_y,
	/* 1D */	Common::KEYCODE_z,
	/* 1E */	Common::KEYCODE_1,
	/* 1F */	Common::KEYCODE_2,
	/* 20 */	Common::KEYCODE_3,
	/* 21 */	Common::KEYCODE_4,
	/* 22 */	Common::KEYCODE_5,
	/* 23 */	Common::KEYCODE_6,
	/* 24 */	Common::KEYCODE_7,
	/* 25 */	Common::KEYCODE_8,
	/* 26 */	Common::KEYCODE_9,
	/* 27 */	Common::KEYCODE_0,
	/* 28 */	Common::KEYCODE_RETURN,
	/* 29 */	Common::KEYCODE_ESCAPE,
	/* 2A */	Common::KEYCODE_BACKSPACE,
	/* 2B */	Common::KEYCODE_TAB,
	/* 2C */	Common::KEYCODE_SPACE,
	/* 2D */	Common::KEYCODE_MINUS,
	/* 2E */	Common::KEYCODE_EQUALS,
	/* 2F */	Common::KEYCODE_LEFTBRACKET,
	/* 30 */	Common::KEYCODE_RIGHTBRACKET,
	/* 31 */	Common::KEYCODE_BACKSLASH,
	/* 32 */	Common::KEYCODE_HASH,
	/* 33 */	Common::KEYCODE_SEMICOLON,
	/* 34 */	Common::KEYCODE_QUOTE,
	/* 35 */	Common::KEYCODE_BACKQUOTE,
	/* 36 */	Common::KEYCODE_COMMA,
	/* 37 */	Common::KEYCODE_PERIOD,
	/* 38 */	Common::KEYCODE_SLASH,
	/* 39 */	Common::KEYCODE_CAPSLOCK,
	/* 3A */	Common::KEYCODE_F1,
	/* 3B */	Common::KEYCODE_F2,
	/* 3C */	Common::KEYCODE_F3,
	/* 3D */	Common::KEYCODE_F4,
	/* 3E */	Common::KEYCODE_F5,
	/* 3F */	Common::KEYCODE_F6,
	/* 40 */	Common::KEYCODE_F7,
	/* 41 */	Common::KEYCODE_F8,
	/* 42 */	Common::KEYCODE_F9,
	/* 43 */	Common::KEYCODE_F10,
	/* 44 */	Common::KEYCODE_F11,
	/* 45 */	Common::KEYCODE_F12,
	/* 46 */	Common::KEYCODE_PRINT,
	/* 47 */	Common::KEYCODE_SCROLLOCK,
	/* 48 */	Common::KEYCODE_PAUSE,
	/* 49 */	Common::KEYCODE_INSERT,
	/* 4A */	Common::KEYCODE_HOME,
	/* 4B */	Common::KEYCODE_PAGEUP,
	/* 4C */	Common::KEYCODE_DELETE,
	/* 4D */	Common::KEYCODE_END,
	/* 4E */	Common::KEYCODE_PAGEDOWN,
	/* 4F */	Common::KEYCODE_RIGHT,
	/* 50 */	Common::KEYCODE_LEFT,
	/* 51 */	Common::KEYCODE_DOWN,
	/* 52 */	Common::KEYCODE_UP,
	/* 53 */	Common::KEYCODE_NUMLOCK,
	/* 54 */	Common::KEYCODE_KP_DIVIDE,
	/* 55 */	Common::KEYCODE_KP_MULTIPLY,
	/* 56 */	Common::KEYCODE_KP_MINUS,
	/* 57 */	Common::KEYCODE_KP_PLUS,
	/* 58 */	Common::KEYCODE_KP_ENTER,
	/* 59 */	Common::KEYCODE_KP1,
	/* 5A */	Common::KEYCODE_KP2,
	/* 5B */	Common::KEYCODE_KP3,
	/* 5C */	Common::KEYCODE_KP4,
	/* 5D */	Common::KEYCODE_KP5,
	/* 5E */	Common::KEYCODE_KP6,
	/* 5F */	Common::KEYCODE_KP7,
	/* 60 */	Common::KEYCODE_KP8,
	/* 61 */	Common::KEYCODE_KP9,
	/* 62 */	Common::KEYCODE_KP0,
	/* 63 */	Common::KEYCODE_KP_PERIOD,
	/* 64 */	Common::KEYCODE_INVALID,
	/* 65 */	Common::KEYCODE_INVALID,
	/* 66 */	Common::KEYCODE_INVALID,
	/* 67 */	Common::KEYCODE_KP_EQUALS,
	/* 68 */	Common::KEYCODE_INVALID,
	/* 69 */	Common::KEYCODE_INVALID,
	/* 6A */	Common::KEYCODE_INVALID,
	/* 6B */	Common::KEYCODE_INVALID,
	/* 6C */	Common::KEYCODE_INVALID,
	/* 6D */	Common::KEYCODE_INVALID,
	/* 6E */	Common::KEYCODE_INVALID,
	/* 6F */	Common::KEYCODE_INVALID,
	/* 70 */	Common::KEYCODE_INVALID,
	/* 71 */	Common::KEYCODE_INVALID,
	/* 72 */	Common::KEYCODE_INVALID,
	/* 73 */	Common::KEYCODE_INVALID,
	/* 74 */	Common::KEYCODE_INVALID,
	/* 75 */	Common::KEYCODE_INVALID,
	/* 76 */	Common::KEYCODE_INVALID,
	/* 77 */	Common::KEYCODE_INVALID,
	/* 78 */	Common::KEYCODE_INVALID,
	/* 79 */	Common::KEYCODE_INVALID,
	/* 7A */	Common::KEYCODE_INVALID,
	/* 7B */	Common::KEYCODE_INVALID,
	/* 7C */	Common::KEYCODE_INVALID,
	/* 7D */	Common::KEYCODE_INVALID,
	/* 7E */	Common::KEYCODE_INVALID,
	/* 7F */	Common::KEYCODE_INVALID,
	/* 80 */	Common::KEYCODE_INVALID,
	/* 81 */	Common::KEYCODE_INVALID,
	/* 82 */	Common::KEYCODE_INVALID,
	/* 83 */	Common::KEYCODE_INVALID,
	/* 84 */	Common::KEYCODE_INVALID,
	/* 85 */	Common::KEYCODE_INVALID,
	/* 86 */	Common::KEYCODE_INVALID,
	/* 87 */	Common::KEYCODE_INVALID,
	/* 88 */	Common::KEYCODE_INVALID,
	/* 89 */	Common::KEYCODE_INVALID,
	/* 8A */	Common::KEYCODE_INVALID,
	/* 8B */	Common::KEYCODE_INVALID,
	/* 8C */	Common::KEYCODE_INVALID,
	/* 8D */	Common::KEYCODE_INVALID,
	/* 8E */	Common::KEYCODE_INVALID,
	/* 8F */	Common::KEYCODE_INVALID,
	/* 90 */	Common::KEYCODE_INVALID,
	/* 91 */	Common::KEYCODE_INVALID,
	/* 92 */	Common::KEYCODE_INVALID,
	/* 93 */	Common::KEYCODE_INVALID,
	/* 94 */	Common::KEYCODE_INVALID,
	/* 95 */	Common::KEYCODE_INVALID,
	/* 96 */	Common::KEYCODE_INVALID,
	/* 97 */	Common::KEYCODE_INVALID,
	/* 98 */	Common::KEYCODE_INVALID,
	/* 99 */	Common::KEYCODE_INVALID,
	/* 9A */	Common::KEYCODE_INVALID,
	/* 9B */	Common::KEYCODE_INVALID,
	/* 9C */	Common::KEYCODE_INVALID,
	/* 9D */	Common::KEYCODE_INVALID,
	/* 9E */	Common::KEYCODE_INVALID,
	/* 9F */	Common::KEYCODE_INVALID,
	/* A0 */	Common::KEYCODE_INVALID,
	/* A1 */	Common::KEYCODE_INVALID,
	/* A2 */	Common::KEYCODE_INVALID,
	/* A3 */	Common::KEYCODE_INVALID,
	/* A4 */	Common::KEYCODE_INVALID,
	/* A5 */	Common::KEYCODE_INVALID,
	/* A6 */	Common::KEYCODE_INVALID,
	/* A7 */	Common::KEYCODE_INVALID,
	/* A8 */	Common::KEYCODE_INVALID,
	/* A9 */	Common::KEYCODE_INVALID,
	/* AA */	Common::KEYCODE_INVALID,
	/* AB */	Common::KEYCODE_INVALID,
	/* AC */	Common::KEYCODE_INVALID,
	/* AD */	Common::KEYCODE_INVALID,
	/* AE */	Common::KEYCODE_INVALID,
	/* AF */	Common::KEYCODE_INVALID,
	/* B0 */	Common::KEYCODE_INVALID,
	/* B1 */	Common::KEYCODE_INVALID,
	/* B2 */	Common::KEYCODE_INVALID,
	/* B3 */	Common::KEYCODE_INVALID,
	/* B4 */	Common::KEYCODE_INVALID,
	/* B5 */	Common::KEYCODE_INVALID,
	/* B6 */	Common::KEYCODE_INVALID,
	/* B7 */	Common::KEYCODE_INVALID,
	/* B8 */	Common::KEYCODE_INVALID,
	/* B9 */	Common::KEYCODE_INVALID,
	/* BA */	Common::KEYCODE_INVALID,
	/* BB */	Common::KEYCODE_INVALID,
	/* BC */	Common::KEYCODE_INVALID,
	/* BD */	Common::KEYCODE_INVALID,
	/* BE */	Common::KEYCODE_INVALID,
	/* BF */	Common::KEYCODE_INVALID,
	/* C0 */	Common::KEYCODE_INVALID,
	/* C1 */	Common::KEYCODE_INVALID,
	/* C2 */	Common::KEYCODE_INVALID,
	/* C3 */	Common::KEYCODE_INVALID,
	/* C4 */	Common::KEYCODE_INVALID,
	/* C5 */	Common::KEYCODE_INVALID,
	/* C6 */	Common::KEYCODE_INVALID,
	/* C7 */	Common::KEYCODE_INVALID,
	/* C8 */	Common::KEYCODE_INVALID,
	/* C9 */	Common::KEYCODE_INVALID,
	/* CA */	Common::KEYCODE_INVALID,
	/* CB */	Common::KEYCODE_INVALID,
	/* CC */	Common::KEYCODE_INVALID,
	/* CD */	Common::KEYCODE_INVALID,
	/* CE */	Common::KEYCODE_INVALID,
	/* CF */	Common::KEYCODE_INVALID,
	/* D0 */	Common::KEYCODE_INVALID,
	/* D1 */	Common::KEYCODE_INVALID,
	/* D2 */	Common::KEYCODE_INVALID,
	/* D3 */	Common::KEYCODE_INVALID,
	/* D4 */	Common::KEYCODE_INVALID,
	/* D5 */	Common::KEYCODE_INVALID,
	/* D6 */	Common::KEYCODE_INVALID,
	/* D7 */	Common::KEYCODE_INVALID,
	/* D8 */	Common::KEYCODE_INVALID,
	/* D9 */	Common::KEYCODE_INVALID,
	/* DA */	Common::KEYCODE_INVALID,
	/* DB */	Common::KEYCODE_INVALID,
	/* DC */	Common::KEYCODE_INVALID,
	/* DD */	Common::KEYCODE_INVALID,
	/* DE */	Common::KEYCODE_INVALID,
	/* DF */	Common::KEYCODE_INVALID,
	/* E0 */    Common::KEYCODE_LCTRL,
	/* E1 */	Common::KEYCODE_LSHIFT,
	/* E2 */	Common::KEYCODE_LALT,
	/* E3 */	Common::KEYCODE_INVALID,
	/* E4 */    Common::KEYCODE_RCTRL,
	/* E5 */	Common::KEYCODE_RSHIFT,
	/* E6 */	Common::KEYCODE_RALT,
	/* E7 */	Common::KEYCODE_INVALID,
	/* E8 */	Common::KEYCODE_INVALID,
	/* E9 */	Common::KEYCODE_INVALID,
	/* EA */	Common::KEYCODE_INVALID,
	/* EB */	Common::KEYCODE_INVALID,
	/* EC */	Common::KEYCODE_INVALID,
	/* ED */	Common::KEYCODE_INVALID,
	/* EE */	Common::KEYCODE_INVALID,
	/* EF */	Common::KEYCODE_INVALID,
	/* F0 */	Common::KEYCODE_INVALID,
	/* F1 */	Common::KEYCODE_INVALID,
	/* F2 */	Common::KEYCODE_INVALID,
	/* F3 */	Common::KEYCODE_INVALID,
	/* F4 */	Common::KEYCODE_INVALID,
	/* F5 */	Common::KEYCODE_INVALID,
	/* F6 */	Common::KEYCODE_INVALID,
	/* F7 */	Common::KEYCODE_INVALID,
	/* F8 */	Common::KEYCODE_INVALID,
	/* F9 */	Common::KEYCODE_INVALID,
	/* FA */	Common::KEYCODE_INVALID,
	/* FB */	Common::KEYCODE_INVALID,
	/* FC */	Common::KEYCODE_INVALID,
	/* FD */	Common::KEYCODE_INVALID,
	/* FE */	Common::KEYCODE_INVALID,
	/* FF */	Common::KEYCODE_INVALID
};

const Common::KeyCode Ps2Input::_padCodes[16] = {
	Common::KEYCODE_1,					// Select
	Common::KEYCODE_INVALID,			// L3
	Common::KEYCODE_INVALID,			// R3
	Common::KEYCODE_F5,					// Start
	Common::KEYCODE_INVALID,			// Up
	Common::KEYCODE_INVALID,			// Right
	Common::KEYCODE_INVALID,			// Down
	Common::KEYCODE_INVALID,			// Left
	Common::KEYCODE_KP0,				// L2
	Common::KEYCODE_PERIOD,				// R2
	Common::KEYCODE_n,					// L1
	Common::KEYCODE_y,					// R1
	Common::KEYCODE_ESCAPE,				// Triangle
	Common::KEYCODE_INVALID,			// Circle  => Right mouse button
	Common::KEYCODE_INVALID,			// Cross	=> Left mouse button
	Common::KEYCODE_RETURN				// Square
};

const Common::KeyCode Ps2Input::_padFlags[16] = {
	Common::KEYCODE_INVALID,			 // Select
	Common::KEYCODE_INVALID,			 // L3
	Common::KEYCODE_INVALID,			 // R3
	Common::KEYCODE_INVALID,			 // Start
	Common::KEYCODE_INVALID,			 //	Up
	Common::KEYCODE_INVALID,			 //	Right
	Common::KEYCODE_INVALID,			 //	Down
	Common::KEYCODE_INVALID,			 //	Left
	Common::KEYCODE_INVALID,			 //	L2
	Common::KEYCODE_INVALID,			 //	R2
	Common::KEYCODE_INVALID,			 //	L1
	Common::KEYCODE_INVALID,			 //	R1
	Common::KEYCODE_INVALID,			 // Triangle
	Common::KEYCODE_INVALID,			 //	Circle
	Common::KEYCODE_INVALID,			 //	Cross
	Common::KEYCODE_INVALID				 //	Square
};
