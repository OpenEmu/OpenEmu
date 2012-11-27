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

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/keyboard.h"
#ifdef __DS__
#include "wordcompletion.h"
#endif

namespace Agi {

//
// IBM-PC keyboard scancodes
//
const uint8 scancodeTable[26] = {
	30,			// A
	48,			// B
	46,			// C
	32,			// D
	18,			// E
	33,			// F
	34,			// G
	35,			// H
	23,			// I
	36,			// J
	37,			// K
	38,			// L
	50,			// M
	49,			// N
	24,			// O
	25,			// P
	16,			// Q
	19,			// R
	31,			// S
	20,			// T
	22,			// U
	47,			// V
	17,			// W
	45,			// X
	21,			// Y
	44			// Z
};

void AgiEngine::initWords() {
	_game.numEgoWords = 0;
}

void AgiEngine::cleanInput() {
	while (_game.numEgoWords)
		free(_game.egoWords[--_game.numEgoWords].word);
}

void AgiEngine::getString(int x, int y, int len, int str) {
	newInputMode(INPUT_GETSTRING);
	_stringdata.x = x;
	_stringdata.y = y;
	_stringdata.len = len;
	_stringdata.str = str;
}

/**
 * Raw key grabber.
 * poll_keyboard() is the raw key grabber (above the gfx driver, that is).
 * It handles console keys and insulates AGI from the console. In the main
 * loop, handle_keys() handles keyboard input and ego movement.
 */
int AgiEngine::doPollKeyboard() {
	int key = 0;

	// If a key is ready, rip it
	if (isKeypress()) {
		key = getKeypress();

		debugC(3, kDebugLevelInput, "key %02x pressed", key);
	}

	return key;
}

int AgiEngine::handleController(int key) {
	VtEntry *v = &_game.viewTable[0];
	int i;

	// AGI 3.149 games, The Black Cauldron and King's Quest 4 need KEY_ESCAPE to use menus
	// Games with the GF_ESCPAUSE flag need KEY_ESCAPE to pause the game
	if (key == 0 ||
		(key == KEY_ESCAPE && getVersion() != 0x3149 && getGameID() != GID_BC && getGameID() != GID_KQ4 && !(getFeatures() & GF_ESCPAUSE)) )
		return false;

	if ((getGameID() == GID_MH1 || getGameID() == GID_MH2) && (key == KEY_ENTER) &&
			(_game.inputMode == INPUT_NONE)) {
		key = 0x20; // Set Enter key to Space in Manhunter when there's no text input
	}

	debugC(3, kDebugLevelInput, "key = %04x", key);

	for (i = 0; i < MAX_CONTROLLERS; i++) {
		if (_game.controllers[i].keycode == key) {
			debugC(3, kDebugLevelInput, "event %d: key press", _game.controllers[i].controller);
			_game.controllerOccured[_game.controllers[i].controller] = true;
			return true;
		}
	}

	if (key == BUTTON_LEFT) {
		if ((getflag(fMenusWork) || (getFeatures() & GF_MENUS)) && _mouse.y <= CHAR_LINES) {
			newInputMode(INPUT_MENU);
			return true;
		}
	}

	// Show predictive dialog if the user clicks on input area
	if (key == BUTTON_LEFT &&
			(int)_mouse.y >= _game.lineUserInput * CHAR_LINES &&
			(int)_mouse.y <= (_game.lineUserInput + 1) * CHAR_LINES) {
		GUI::PredictiveDialog _predictiveDialog;
		_predictiveDialog.runModal();
		strcpy(_predictiveResult, _predictiveDialog.getResult());
		if (strcmp(_predictiveResult, "")) {
			if (_game.inputMode == INPUT_NONE) {
				for (int n = 0; _predictiveResult[n]; n++)
					keyEnqueue(_predictiveResult[n]);
			} else {
				strcpy((char *)_game.inputBuffer, _predictiveResult);
				handleKeys(KEY_ENTER);
			}
		}
		/*
		if (predictiveDialog()) {
			if (_game.inputMode == INPUT_NONE) {
				for (int n = 0; _predictiveResult[n]; n++)
					keyEnqueue(_predictiveResult[n]);
			} else {
				strcpy((char *)_game.inputBuffer, _predictiveResult);
				handleKeys(KEY_ENTER);
			}
		}
		*/
		return true;
	}

	if (_game.playerControl) {
		int d = 0;

		if (!KEY_ASCII(key)) {
			switch (key) {
			case KEY_UP:
				d = 1;
				break;
			case KEY_DOWN:
				d = 5;
				break;
			case KEY_LEFT:
				d = 7;
				break;
			case KEY_RIGHT:
				d = 3;
				break;
			case KEY_UP_RIGHT:
				d = 2;
				break;
			case KEY_DOWN_RIGHT:
				d = 4;
				break;
			case KEY_UP_LEFT:
				d = 8;
				break;
			case KEY_DOWN_LEFT:
				d = 6;
				break;
			}
		}

		if (!(getFeatures() & GF_AGIMOUSE)) {
			// Handle mouse button events
			if (key == BUTTON_LEFT) {
				if (getGameID() == GID_PQ1 && _game.vars[vCurRoom] == 116) {
					// WORKAROUND: Special handling for mouse clicks in the newspaper
					// screen of PQ1. Fixes bug #3018770.
					d = 3;	// fake a right arrow key (next page)
				} else {
					// Click-to-walk mouse interface
					v->flags |= fAdjEgoXY;
					v->parm1 = WIN_TO_PIC_X(_mouse.x);
					v->parm2 = WIN_TO_PIC_Y(_mouse.y);
					return true;
				}
			}
		}

		if (d || key == KEY_STATIONARY) {
			v->flags &= ~fAdjEgoXY;
			v->direction = v->direction == d ? 0 : d;
			return true;
		}
	}

	return false;
}

void AgiEngine::handleGetstring(int key) {
	static int pos = 0;	// Cursor position
	static char buf[40];

	if (KEY_ASCII(key) == 0)
		return;

	debugC(3, kDebugLevelInput, "handling key: %02x", key);

	switch (key) {
	case BUTTON_LEFT:
		if ((int)_mouse.y >= _stringdata.y * CHAR_LINES &&
				(int)_mouse.y <= (_stringdata.y + 1) * CHAR_LINES) {
			GUI::PredictiveDialog _predictiveDialog;
			_predictiveDialog.runModal();
			strcpy(_predictiveResult, _predictiveDialog.getResult());
			if (strcmp(_predictiveResult, "")) {
				strcpy(_game.strings[_stringdata.str], _predictiveResult);
				newInputMode(INPUT_NORMAL);
				_gfx->printCharacter(_stringdata.x + strlen(_game.strings[_stringdata.str]) + 1,
								_stringdata.y, ' ', _game.colorFg, _game.colorBg);
				return;
			}
			/*
			if (predictiveDialog()) {
				strcpy(_game.strings[_stringdata.str], _predictiveResult);
				newInputMode(INPUT_NORMAL);
				_gfx->printCharacter(_stringdata.x + strlen(_game.strings[_stringdata.str]) + 1,
								_stringdata.y, ' ', _game.colorFg, _game.colorBg);
				return;
			}
			*/
		}
		break;
	case KEY_ENTER:
		debugC(3, kDebugLevelInput, "KEY_ENTER");
		_game.hasPrompt = 0;
		buf[pos] = 0;

		strcpy(_game.strings[_stringdata.str], buf);
		debugC(3, kDebugLevelInput, "buffer=[%s]", buf);
		buf[pos = 0] = 0;

		newInputMode(INPUT_NORMAL);
		_gfx->printCharacter(_stringdata.x + strlen(_game.strings[_stringdata.str]) + 1,
				_stringdata.y, ' ', _game.colorFg, _game.colorBg);
		return;
	case KEY_ESCAPE:
		debugC(3, kDebugLevelInput, "KEY_ESCAPE");
		_game.hasPrompt = 0;
		buf[pos = 0] = 0;

		strcpy(_game.strings[_stringdata.str], buf);
		newInputMode(INPUT_NORMAL);

		// newInputMode(INPUT_MENU);
		break;
	case KEY_BACKSPACE:	// 0x08
		if (!pos)
			break;

		_gfx->printCharacter(_stringdata.x + (pos + 1), _stringdata.y,
				' ', _game.colorFg, _game.colorBg);
		pos--;
		buf[pos] = 0;
		break;
	default:
		if (key < 0x20 || key > 0x7f)
			break;

		if (pos >= _stringdata.len)
			break;

		buf[pos++] = key;
		buf[pos] = 0;

		// Echo
		_gfx->printCharacter(_stringdata.x + pos, _stringdata.y, buf[pos - 1],
				_game.colorFg, _game.colorBg);

		break;
	}

	// print cursor
	_gfx->printCharacter(_stringdata.x + pos + 1, _stringdata.y,
			(char)_game.cursorChar, _game.colorFg, _game.colorBg);
}

void AgiEngine::handleKeys(int key) {
	uint8 *p = NULL;
	int c = 0;
	static uint8 formattedEntry[40];
	int l = _game.lineUserInput;
	int fg = _game.colorFg, bg = _game.colorBg;
	int promptLength = strlen(agiSprintf(_game.strings[0]));

	setvar(vWordNotFound, 0);

	debugC(3, kDebugLevelInput, "handling key: %02x", key);

	switch (key) {
	case KEY_ENTER:
		debugC(3, kDebugLevelInput, "KEY_ENTER");
		_game.keypress = 0;

		// Remove all leading spaces
		for (p = _game.inputBuffer; *p && *p == 0x20; p++)
			;

		// Copy to internal buffer
		for (; *p && c < 40-1; p++) {
			// Squash spaces
			if (*p == 0x20 && *(p + 1) == 0x20) {
				p++;
				continue;
			}
			formattedEntry[c++] = tolower(*p);
		}
		formattedEntry[c++] = 0;

		// Handle string only if it's not empty
		if (formattedEntry[0]) {
			strcpy((char *)_game.echoBuffer, (const char *)_game.inputBuffer);
			strcpy(_lastSentence, (const char *)formattedEntry);
			dictionaryWords(_lastSentence);
		}

		// Clear to start a new line
		_game.hasPrompt = 0;
		_game.inputBuffer[_game.cursorPos = 0] = 0;
		debugC(3, kDebugLevelInput | kDebugLevelText, "clear lines");
		clearLines(l, l + 1, bg);
		flushLines(l, l + 1);
#ifdef __DS__
		DS::findWordCompletions((char *) _game.inputBuffer);
#endif

		break;
	case KEY_ESCAPE:
		debugC(3, kDebugLevelInput, "KEY_ESCAPE");
		newInputMode(INPUT_MENU);
		break;
	case KEY_BACKSPACE:
		// Ignore backspace at start of line
		if (_game.cursorPos == 0)
			break;

		// erase cursor
		_gfx->printCharacter(_game.cursorPos + promptLength, l, ' ', fg, bg);
		_game.inputBuffer[--_game.cursorPos] = 0;

		// Print cursor
		_gfx->printCharacter(_game.cursorPos + promptLength, l, _game.cursorChar, fg, bg);

#ifdef __DS__
		DS::findWordCompletions((char *) _game.inputBuffer);
#endif
		break;
	default:
		// Ignore invalid keystrokes
		if (key < 0x20 || key > 0x7f)
			break;

		// Maximum input size reached
		if (_game.cursorPos >= getvar(vMaxInputChars))
			break;

		_game.inputBuffer[_game.cursorPos++] = key;
		_game.inputBuffer[_game.cursorPos] = 0;

#ifdef __DS__
		DS::findWordCompletions((char *) _game.inputBuffer);
#endif

		// echo
		_gfx->printCharacter(_game.cursorPos + promptLength - 1, l, _game.inputBuffer[_game.cursorPos - 1], fg, bg);

		// Print cursor
		_gfx->printCharacter(_game.cursorPos + promptLength, l, _game.cursorChar, fg, bg);
		break;
	}
}

int AgiEngine::waitKey() {
	int key = 0;

	clearKeyQueue();

	debugC(3, kDebugLevelInput, "waiting...");
	while (!(shouldQuit() || _restartGame || getflag(fRestoreJustRan))) {
		pollTimer();
		key = doPollKeyboard();
		if (key == KEY_ENTER || key == KEY_ESCAPE || key == BUTTON_LEFT)
			break;

		pollTimer();
		updateTimer();

		_gfx->doUpdate();
	}

	// Have to clear it as original did not set this variable, and we do it in doPollKeyboard()
	// Fixes bug #2823759
	_game.keypress = 0;

	return key;
}

int AgiEngine::waitAnyKey() {
	int key = 0;

	clearKeyQueue();

	debugC(3, kDebugLevelInput, "waiting... (any key)");
	while (!(shouldQuit() || _restartGame)) {
		pollTimer();
		key = doPollKeyboard();
		if (key)
			break;
		_gfx->doUpdate();
	}

	// Have to clear it as original did not set this variable, and we do it in doPollKeyboard()
	_game.keypress = 0;

	return key;
}

bool AgiEngine::isKeypress() {
	processEvents();
	return _keyQueueStart != _keyQueueEnd;
}

int AgiEngine::getKeypress() {
	int k;

	while (_keyQueueStart == _keyQueueEnd)	// block
		pollTimer();

	keyDequeue(k);

	return k;
}

void AgiEngine::clearKeyQueue() {
	while (isKeypress()) {
		getKeypress();
	}
}

} // End of namespace Agi
