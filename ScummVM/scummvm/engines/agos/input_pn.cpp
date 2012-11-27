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

#include "agos/agos.h"
#include "agos/intern.h"

namespace AGOS {

void AGOSEngine_PN::clearInputLine() {
	_inputting = false;
	_inputReady = false;
	clearWindow(_windowArray[2]);
}

void AGOSEngine_PN::handleKeyboard() {
	if (!_inputReady)
		return;

	if (_hitCalled != 0) {
		mouseHit();
	}

	int16 chr = -1;
	if (_mouseString) {
		const char *strPtr = _mouseString;
		while (*strPtr != 0 && *strPtr != 13)
			addChar(*strPtr++);
		_mouseString = 0;

		chr = *strPtr;
		if (chr == 13) {
			addChar(13);
		}
	}
	if (_mouseString1 && chr != 13) {
		const char *strPtr = _mouseString1;
		while (*strPtr != 13)
			addChar(*strPtr++);
		_mouseString1 = 0;

		chr = *strPtr;
		if (chr == 13) {
			addChar(13);
		}
	}
	if (chr == -1) {
		if (_keyPressed.keycode == Common::KEYCODE_BACKSPACE || _keyPressed.keycode == Common::KEYCODE_RETURN) {
			chr = _keyPressed.keycode;
			addChar(chr);
		} else if (!(_videoLockOut & 0x10)) {
			chr = _keyPressed.ascii;
			if (chr >= 32)
				addChar(chr);
		}
	}

	if (chr == 13) {
		_mouseString = 0;
		_mouseString1 = 0;
		_mousePrintFG = 0;
		_inputReady = false;
	}

	_keyPressed.reset();
}

void AGOSEngine_PN::interact(char *buffer, uint8 size) {
	if (!_inputting) {
		memset(_keyboardBuffer, 0, sizeof(_keyboardBuffer));
		_intputCounter = 0;
		_inputMax = size;
		_inputWindow = _windowArray[_curWindow];
		windowPutChar(_inputWindow, 128);
		windowPutChar(_inputWindow, 8);
		_inputting = true;
		_inputReady = true;
	}

	while (!shouldQuit() && _inputReady) {
		if (!_noScanFlag && _scanFlag) {
			buffer[0] = 1;
			buffer[1] = 0;
			_scanFlag = 0;
			break;
		}
		delay(1);
	}

	if (!_inputReady) {
		memcpy(buffer, _keyboardBuffer, size);
		_inputting = false;
	}
}

void AGOSEngine_PN::addChar(uint8 chr) {
	if (chr == 13) {
		_keyboardBuffer[_intputCounter++] = chr;
		windowPutChar(_inputWindow, 13);
	} else if (chr == 8 && _intputCounter) {
		clearCursor(_inputWindow);
		windowPutChar(_inputWindow, 8);
		windowPutChar(_inputWindow, 128);
		windowPutChar(_inputWindow, 8);

		_keyboardBuffer[--_intputCounter] = 0;
	} else if (chr >= 32 && _intputCounter < _inputMax) {
		_keyboardBuffer[_intputCounter++] = chr;

		clearCursor(_inputWindow);
		windowPutChar(_inputWindow, chr);
		windowPutChar(_inputWindow, 128);
		windowPutChar(_inputWindow, 8);
	}
}

void AGOSEngine_PN::clearCursor(WindowBlock *window) {
	byte oldTextColor = window->textColor;

	window->textColor = window->fillColor;
	windowPutChar(window, 128);
	window->textColor = oldTextColor;

	windowPutChar(window, 8);
}

bool AGOSEngine_PN::processSpecialKeys() {
	if (shouldQuit())
		_exitCutscene = true;

	switch (_keyPressed.keycode) {
	case Common::KEYCODE_ESCAPE:
		_exitCutscene = true;
		break;
	case Common::KEYCODE_PAUSE:
		pause();
		break;
	default:
		break;
	}

	_keyPressed.reset();
	return false;
}

} // End of namespace AGOS
