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
#include "agi/sprite.h"		// for commit_both()
#include "agi/graphics.h"
#include "agi/keyboard.h"

namespace Agi {

void AgiEngine::printText2(int l, const char *msg, int foff, int xoff, int yoff,
				int len, int fg, int bg, bool checkerboard) {
	int x1, y1;
	int maxx, minx, ofoff;
	int update;
	// Note: Must be unsigned to use AGDS cyrillic characters!
#ifdef __DS__
	// On the DS, a compiler bug causes the text to render incorrectly, because
	// GCC tries to optimisie out writes to this pointer (tested on DevkitARM v19b and v20)
	// Making this pointer volatile fixes this.
	volatile const unsigned char *m;
#else
	const unsigned char *m;
#endif

	// kludge!
	update = 1;
	if (l == 2) {
		update = l = 0;
	}

	// FR: strings with len == 1 were not printed
	if (len == 1) {
		_gfx->putTextCharacter(l, xoff + foff, yoff, *msg, fg, bg, checkerboard);
		maxx = 1;
		minx = 0;
		ofoff = foff;
		y1 = 0;		// Check this
	} else {
		maxx = 0;
		minx = GFX_WIDTH;
		ofoff = foff;

		for (m = (const unsigned char *)msg, x1 = y1 = 0; *m; m++) {

			// Note: there were extra checks for *m being a cursor character
			// here (1, 2 or 3), which have been removed, as the cursor
			// character is no longer printed via this function.
			if (*m >= 0x20) {
				int ypos = (y1 * CHAR_LINES) + yoff;

				if ((x1 != (len - 1) || x1 == 39) && (ypos <= (GFX_HEIGHT - CHAR_LINES))) {
					int xpos = (x1 * CHAR_COLS) + xoff + foff;

					if (xpos >= GFX_WIDTH)
						continue;

					_gfx->putTextCharacter(l, xpos, ypos, *m, fg, bg, checkerboard);

					if (x1 > maxx)
						maxx = x1;
					if (x1 < minx)
						minx = x1;
				}

				x1++;

				// Change line if we've reached the end of this one, unless the next
				// character is a new line itself, or the end of the string
				if (x1 == len && m[1] != '\n' && m[1] != 0) {
					y1++;
					x1 = foff = 0;
				}
			} else {
				if (m[1] != 0) {
					y1++;
					x1 = foff = 0;
				}
			}
		}
	}

	if (l)
		return;

	if (maxx < minx)
		return;

	maxx *= CHAR_COLS;
	minx *= CHAR_COLS;

	if (update) {
		_gfx->scheduleUpdate(foff + xoff + minx, yoff, ofoff + xoff + maxx + CHAR_COLS - 1,
				yoff + y1 * CHAR_LINES + CHAR_LINES + 1);

		// Making synchronous text updates reduces CPU load
		// when updating status line and input area
		_gfx->doUpdate();
	}
}

//
// len is in characters, not pixels!!
//
void AgiEngine::blitTextbox(const char *p, int y, int x, int len) {
	// if x | y = -1, then center the box
	int xoff, yoff, lin, h, w;
	char *msg, *m;

	debugC(3, kDebugLevelText, "blitTextbox(): x=%d, y=%d, len=%d", x, y, len);
	if (_game.window.active)
		closeWindow();

	if (x == 0 && y == 0 && len == 0)
		x = y = -1;

	if (len <= 0)
		len = 30;

	xoff = x * CHAR_COLS;
	yoff = y * CHAR_LINES;

	m = msg = wordWrapString(agiSprintf(p), &len);

	for (lin = 1; *m; m++) {
		// Test \r for MacOS 8
		if (*m == '\n' || *m == '\r')
			lin++;
	}

	if (lin * CHAR_LINES > GFX_HEIGHT)
		lin = (GFX_HEIGHT / CHAR_LINES);

	w = (len + 2) * CHAR_COLS;
	h = (lin + 2) * CHAR_LINES;

	if (xoff < 0)
		xoff = (GFX_WIDTH - w - CHAR_COLS) / 2;
	else
		xoff -= CHAR_COLS;

	if (yoff < 0)
		yoff = (GFX_HEIGHT - 3 * CHAR_LINES - h) / 2;

	drawWindow(xoff, yoff, xoff + w - 1, yoff + h - 1);

	printText2(2, msg, 0, CHAR_COLS + xoff, CHAR_LINES + yoff,
			len + 1, MSG_BOX_TEXT, MSG_BOX_COLOR);

	free(msg);

	_gfx->doUpdate();
}

void AgiEngine::eraseTextbox() {
	if (!_game.window.active) {
		debugC(3, kDebugLevelText, "eraseTextbox(): no window active");
		return;
	}

	debugC(4, kDebugLevelText, "eraseTextbox(): x1=%d, y1=%d, x2=%d, y2=%d", _game.window.x1,
			_game.window.y1, _game.window.x2, _game.window.y2);

	_gfx->restoreBlock(_game.window.x1, _game.window.y1,
			_game.window.x2, _game.window.y2, _game.window.buffer);

	free(_game.window.buffer);
	_game.window.active = false;

	_gfx->doUpdate();
}

/*
 * Public functions
 */

/**
 * Print text in the AGI engine screen.
 */
void AgiEngine::printText(const char *msg, int f, int x, int y, int len, int fg, int bg, bool checkerboard) {
	f *= CHAR_COLS;
	x *= CHAR_COLS;
	y *= CHAR_LINES;

	debugC(4, kDebugLevelText, "printText(): %s, %d, %d, %d, %d, %d, %d", msg, f, x, y, len, fg, bg);
	printText2(0, agiSprintf(msg), f, x, y, len, fg, bg, checkerboard);
}

/**
 * Print text in the AGI engine console.
 */
void AgiEngine::printTextConsole(const char *msg, int x, int y, int len, int fg, int bg) {
	x *= CHAR_COLS;
	y *= 10;

	debugC(4, kDebugLevelText, "printTextConsole(): %s, %d, %d, %d, %d, %d", msg, x, y, len, fg, bg);
	printText2(1, msg, 0, x, y, len, fg, bg);
}

/**
 * Wrap text line to the specified width.
 * @param str  String to wrap.
 * @param len  Length of line.
 *
 * Based on GBAGI implementation with permission from the author
 */
char *AgiEngine::wordWrapString(const char *s, int *len) {
	char *outStr, *msgBuf;
	int maxWidth = *len;
	const char *pWord;
	int lnLen, wLen;

	// Allocate some extra space for the final buffer, as
	// outStr may end up being longer than s
	// 26 = 200 (screen height) / 8 (font height) + 1
	msgBuf = outStr = (char *)malloc(strlen(s) + 26);

	int msgWidth = 0;

	lnLen = 0;

	while (*s) {
		pWord = s;

		while (*s != '\0' && *s != ' ' && *s != '\n' && *s != '\r')
			s++;

		wLen = (int)(s - pWord);

		if (wLen && *s == '\n' && s[-1] == ' ')
			wLen--;

		if (wLen + lnLen >= maxWidth) {
			// Check if outStr isn't msgBuf. If this is the case, outStr hasn't advanced
			// yet, so no output has been written yet
			if (outStr != msgBuf) {
				if (outStr[-1] == ' ')
					outStr[-1] = '\n';
				else
					*outStr++ = '\n';
			}

			lnLen = 0;

			while (wLen >= maxWidth) {
				msgWidth = maxWidth;

				memcpy(outStr, pWord, maxWidth);

				wLen -= maxWidth;
				outStr += maxWidth;
				pWord  += maxWidth;
				*outStr++ = '\n';
			}
		}

		if (wLen) {
			memcpy(outStr, pWord, wLen);
			outStr += wLen;
		}
		lnLen += wLen+1;

		if (lnLen > msgWidth) {
			msgWidth = lnLen;

			if (*s == '\0' || *s == ' ' || *s == '\n' || *s == '\r')
				msgWidth--;
		}

		if (*s == '\n')
			lnLen = 0;

		if (*s)
			*outStr++ = *s++;
	}
	*outStr = '\0';
	*len = msgWidth;

	return msgBuf;
}

/**
 * Remove existing window, if any.
 */
void AgiEngine::closeWindow() {
	debugC(4, kDebugLevelText, "closeWindow()");

	_sprites->eraseBoth();
	eraseTextbox();	// remove window, if any
	_sprites->blitBoth();
	_sprites->commitBoth();		// redraw sprites
	_game.hasWindow = false;
}

/**
 * Display a message box.
 * This function displays the specified message in a text box
 * centered in the screen and waits until a key is pressed.
 * @param p The text to be displayed
 */
int AgiEngine::messageBox(const char *s) {
	int k;

	_sprites->eraseBoth();
	blitTextbox(s, -1, -1, -1);
	_sprites->blitBoth();
	k = waitKey();
	debugC(4, kDebugLevelText, "messageBox(): wait_key returned %02x", k);
	closeWindow();

	return k;
}

/**
 * Display a message box with buttons.
 * This function displays the specified message in a text box
 * centered in the screen and waits until a button is pressed.
 * @param p The text to be displayed
 * @param b NULL-terminated list of button labels
 */
int AgiEngine::selectionBox(const char *m, const char **b) {
	int numButtons = 0;
	int x, y, i, s;
	int bx[5], by[5];

	_noSaveLoadAllowed = true;

	_sprites->eraseBoth();
	blitTextbox(m, -1, -1, -1);

	x = _game.window.x1 + 5 * CHAR_COLS / 2;
	y = _game.window.y2 - 5 * CHAR_LINES / 2;
	s = _game.window.x2 - _game.window.x1 + 1 - 5 * CHAR_COLS;
	debugC(3, kDebugLevelText, "selectionBox(): s = %d", s);

	// Automatically position buttons
	for (i = 0; b[i]; i++) {
		numButtons++;
		s -= CHAR_COLS * strlen(b[i]);
	}

	if (i > 1) {
		debugC(3, kDebugLevelText, "selectionBox(): s / %d = %d", i - 1, s / (i - 1));
		s /= (i - 1);
	} else {
		x += s / 2;
	}

	for (i = 0; b[i]; i++) {
		bx[i] = x;
		by[i] = y;
		x += CHAR_COLS * strlen(b[i]) + s;
	}

	_sprites->blitBoth();

	clearKeyQueue();

	AllowSyntheticEvents on(this);

	debugC(4, kDebugLevelText, "selectionBox(): waiting...");
	int key, active = 0;
	int rc = -1;
	while (rc == -1 && !(shouldQuit() || _restartGame)) {
		for (i = 0; b[i]; i++)
			_gfx->drawCurrentStyleButton(bx[i], by[i], b[i], i == active, false, i == 0);

		pollTimer();
		key = doPollKeyboard();
		switch (key) {
		case KEY_ENTER:
			rc = active;
			debugC(4, kDebugLevelText, "selectionBox(): Button pressed: %d", rc);
			break;
		case KEY_RIGHT:
			active++;
			if (active >= numButtons)
				active = 0;
			break;
		case KEY_LEFT:
			active--;
			if (active < 0)
				active = numButtons - 1;
			break;
		case BUTTON_LEFT:
			for (i = 0; b[i]; i++) {
				if (_gfx->testButton(bx[i], by[i], b[i])) {
					rc = active = i;
					debugC(4, kDebugLevelText, "selectionBox(): Button pressed: %d", rc);
					break;
				}
			}
			break;
		case 0x09:	// Tab
			debugC(3, kDebugLevelText, "selectionBox(): Focus change");
			active++;
			active %= i;
			break;
		}
		_gfx->doUpdate();

		if (key == KEY_ESCAPE)
			break;
	}

	closeWindow();
	debugC(2, kDebugLevelText, "selectionBox(): Result = %d", rc);

	_noSaveLoadAllowed = false;

	return rc;
}

/**
 *
 */
int AgiEngine::print(const char *p, int lin, int col, int len) {
	if (p == NULL)
		return 0;

	debugC(4, kDebugLevelText, "print(): lin = %d, col = %d, len = %d", lin, col, len);

	blitTextbox(p, lin, col, len);

	if (getflag(fOutputMode)) {
		// non-blocking window
		setflag(fOutputMode, false);
		return 1;
	}

	// blocking

	_noSaveLoadAllowed = true;

	if (_game.vars[vWindowReset] == 0) {
		int k;
		setvar(vKey, 0);
		k = waitKey();
		closeWindow();

		_noSaveLoadAllowed = false;

		return k;
	}

	// timed window

	debugC(3, kDebugLevelText, "f15==0, v21==%d => timed", getvar(21));
	_game.msgBoxTicks = getvar(vWindowReset) * 10;
	setvar(vKey, 0);

	_menuSelected = false;

	do {
		if (getflag(fRestoreJustRan))
			break;

		if (_menuSelected)
			break;

		mainCycle();
		if (_game.keypress == KEY_ENTER) {
			debugC(4, kDebugLevelText, "KEY_ENTER");
			setvar(vWindowReset, 0);
			_game.keypress = 0;
			break;
		}
	} while (_game.msgBoxTicks > 0 && !(shouldQuit() || _restartGame));

	setvar(vWindowReset, 0);

	closeWindow();

	_noSaveLoadAllowed = false;

	return 0;
}

/**
 *
 */
void AgiEngine::printStatus(const char *message, ...) {
	va_list args;

	va_start(args, message);

	Common::String x = Common::String::vformat(message, args);

	va_end(args);

	debugC(4, kDebugLevelText, "fg=%d, bg=%d", STATUS_FG, STATUS_BG);
	printText(x.c_str(), 0, 0, _game.lineStatus, 40, STATUS_FG, STATUS_BG);
}

static void safeStrcat(Common::String &p, const char *t) {
	if (t != NULL)
		p += t;
}

/**
 * Formats AGI string.
 * This function turns a AGI string into a real string expanding values
 * according to the AGI format specifiers.
 * @param s  string containing the format specifier
 * @param n  logic number
 */
char *AgiEngine::agiSprintf(const char *s) {
	static char agiSprintf_buf[768];
	Common::String p;
	char z[16];

	debugC(3, kDebugLevelText, "logic %d, '%s'", _game.lognum, s);

	while (*s) {
		switch (*s) {
		case '%':
			s++;
			switch (*s++) {
				int i;
			case 'v':
				i = strtoul(s, NULL, 10);
				while (*s >= '0' && *s <= '9')
					s++;
				sprintf(z, "%015i", getvar(i));

				i = 99;
				if (*s == '|') {
					s++;
					i = strtoul(s, NULL, 10);
					while (*s >= '0' && *s <= '9')
						s++;
				}

				if (i == 99) {
					// remove all leading 0
					// don't remove the 3rd zero if 000
					for (i = 0; z[i] == '0' && i < 14; i++)
						;
				} else {
					i = 15 - i;
				}
				safeStrcat(p, z + i);
				break;
			case '0':
				i = strtoul(s, NULL, 10) - 1;
				safeStrcat(p, objectName(i));
				break;
			case 'g':
				i = strtoul(s, NULL, 10) - 1;
				safeStrcat(p, _game.logics[0].texts[i]);
				break;
			case 'w':
				i = strtoul(s, NULL, 10) - 1;
				safeStrcat(p, _game.egoWords[i].word);
				break;
			case 's':
				i = strtoul(s, NULL, 10);
				safeStrcat(p, agiSprintf(_game.strings[i]));
				break;
			case 'm':
				i = strtoul(s, NULL, 10) - 1;
				if (_game.logics[_game.lognum].numTexts > i)
					safeStrcat(p, agiSprintf(_game.logics[_game.lognum].texts[i]));
				break;
			}

			while (*s >= '0' && *s <= '9')
				s++;
			break;

		case '\\':
			s++;
			// FALL THROUGH

		default:
			p += *s++;
			break;
		}
	}

	assert(p.size() < sizeof(agiSprintf_buf));
	strcpy(agiSprintf_buf, p.c_str());
	return agiSprintf_buf;
}

/**
 * Write the status line.
 */
void AgiEngine::writeStatus() {
	char x[64];

	if (_debug.statusline) {
		printStatus("%3d(%03d) %3d,%3d(%3d,%3d)               ",
				getvar(0), getvar(1), _game.viewTable[0].xPos,
				_game.viewTable[0].yPos, WIN_TO_PIC_X(_mouse.x),
				WIN_TO_PIC_Y(_mouse.y));
		return;
	}

	if (!_game.statusLine) {
		clearLines(_game.lineStatus, _game.lineStatus, 0);
		flushLines(_game.lineStatus, _game.lineStatus);

#if 0
		// FIXME: Breaks wrist watch prompt in SQ2

		// Clear the user input line as well when clearing the status line
		// Fixes bug #1893564 - AGI: Texts messed out in Naturette 1
		clearLines(_game.lineUserInput, _game.lineUserInput, 0);
		flushLines(_game.lineUserInput, _game.lineUserInput);
#endif
		return;
	}

	switch (getLanguage()) {
	case Common::RU_RUS:
		sprintf(x, " \x91\xe7\xa5\xe2: %i \xa8\xa7 %-3i", _game.vars[vScore], _game.vars[vMaxScore]);
		printStatus("%-17s              \x87\xa2\xe3\xaa:%s", x, getflag(fSoundOn) ? "\xa2\xaa\xab " : "\xa2\xeb\xaa\xab");
		break;
	default:
		sprintf(x, " Score:%i of %-3i", _game.vars[vScore], _game.vars[vMaxScore]);
		printStatus("%-17s             Sound:%s ", x, getflag(fSoundOn) ? "on " : "off");
		break;
	}
}

/**
 * Print user input prompt.
 */
void AgiEngine::writePrompt() {
	int l, fg, bg, pos;
	int promptLength = strlen(agiSprintf(_game.strings[0]));

	if (!_game.inputEnabled || _game.inputMode != INPUT_NORMAL)
		return;

	l = _game.lineUserInput;
	fg = _game.colorFg;
	bg = _game.colorBg;
	pos = _game.cursorPos;

	debugC(4, kDebugLevelText, "erase line %d", l);
	clearLines(l, l, _game.colorBg);

	debugC(4, kDebugLevelText, "prompt = '%s'", agiSprintf(_game.strings[0]));
	printText(_game.strings[0], 0, 0, l, promptLength + 1, fg, bg);
	printText((char *)_game.inputBuffer, 0, promptLength, l, pos + 1, fg, bg);
	_gfx->printCharacter(pos + promptLength, l, _game.cursorChar, fg, bg);

	flushLines(l, l);
	_gfx->doUpdate();
}

void AgiEngine::clearPrompt(bool useBlackBg) {
	int l;

	l = _game.lineUserInput;
	clearLines(l, l, useBlackBg ? 0 : _game.colorBg);
	flushLines(l, l);

	_gfx->doUpdate();
}

/**
 * Clear text lines in the screen.
 * @param l1  start line
 * @param l2  end line
 * @param c   color
 */
void AgiEngine::clearLines(int l1, int l2, int c) {
	// do we need to adjust for +8 on topline?
	// inc for endline so it matches the correct num
	// ie, from 22 to 24 is 3 lines, not 2 lines.

	debugC(4, kDebugLevelText, "clearLines(%d, %d, %d)", l1, l2, c);

	l1 *= CHAR_LINES;
	l2 *= CHAR_LINES;
	l2 += CHAR_LINES - 1;

	_gfx->drawRectangle(0, l1, GFX_WIDTH - 1, l2, c);
}

/**
 *
 */
void AgiEngine::flushLines(int l1, int l2) {
	l1 *= CHAR_LINES;
	l2 *= CHAR_LINES;
	l2 += CHAR_LINES - 1;

	_gfx->flushBlock(0, l1, GFX_WIDTH - 1, l2);
}

/**
 *
 */
void AgiEngine::drawWindow(int x1, int y1, int x2, int y2) {
	_game.window.active = true;
	_game.window.x1 = x1;
	_game.window.y1 = y1;
	_game.window.x2 = x2;
	_game.window.y2 = y2;
	_game.window.buffer = (uint8 *)malloc((x2 - x1 + 1) * (y2 - y1 + 1));

	debugC(4, kDebugLevelText, "x1=%d, y1=%d, x2=%d, y2=%d", x1, y1, x2, y2);
	_gfx->saveBlock(x1, y1, x2, y2, _game.window.buffer);
	_gfx->drawBox(x1, y1, x2, y2, MSG_BOX_COLOR, MSG_BOX_LINE, 2);
}

} // End of namespace Agi
