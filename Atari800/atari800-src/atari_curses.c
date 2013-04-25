/*
 * atari_curses.c - Curses based port code
 *
 * Copyright (c) 1995-1998 David Firth
 * Copyright (c) 1998-2005 Atari800 development team (see DOC/CREDITS)

 * This file is part of the Atari800 emulator project which emulates
 * the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
 *
 * Atari800 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atari800 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atari800; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "config.h"
#include <string.h>
#ifdef USE_NCURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include "antic.h" /* ypos */
#include "atari.h"
#include "gtia.h" /* GTIA_COLPFx */
#include "input.h"
#include "akey.h"
#include "log.h"
#include "monitor.h"
#include "platform.h"
#include "ui.h" /* UI_alt_function */

#ifdef SOUND
#include "sound.h"
#endif

#define CURSES_LEFT 0
#define CURSES_CENTRAL 1
#define CURSES_RIGHT 2
#define CURSES_WIDE_1 3
#define CURSES_WIDE_2 4

static int curses_mode = CURSES_LEFT;

static int curses_screen[24][40];

int PLATFORM_Initialise(int *argc, char *argv[])
{
	int i;
	int j;

	for (i = j = 1; i < *argc; i++) {
		if (strcmp(argv[i], "-left") == 0)
			curses_mode = CURSES_LEFT;
		else if (strcmp(argv[i], "-central") == 0)
			curses_mode = CURSES_CENTRAL;
		else if (strcmp(argv[i], "-right") == 0)
			curses_mode = CURSES_RIGHT;
		else if (strcmp(argv[i], "-wide1") == 0)
			curses_mode = CURSES_WIDE_1;
		else if (strcmp(argv[i], "-wide2") == 0)
			curses_mode = CURSES_WIDE_2;
		else {
			if (strcmp(argv[i], "-help") == 0) {
				Log_print("\t-central         Center emulated screen\n"
				       "\t-left            Align left\n"
				       "\t-right           Align right (on 80 columns)\n"
				       "\t-wide1           Use 80 columns\n"
				       "\t-wide2           Use 80 columns, display twice"
				      );
			}
			argv[j++] = argv[i];
		}
	}

	*argc = j;

	initscr();
	noecho();
	cbreak();					/* Don't wait for carriage return */
	keypad(stdscr, TRUE);
	curs_set(0);				/* Disable Cursor */
	nodelay(stdscr, 1);			/* Don't block for keypress */

#ifdef SOUND
	if (!Sound_Initialise(argc, argv))
		return FALSE;
#endif

	return TRUE;
}

int PLATFORM_Exit(int run_monitor)
{
	curs_set(1);
	endwin();
	Log_flushlog();

	if (run_monitor && MONITOR_Run()) {
		curs_set(0);
		return TRUE;
	}
#ifdef SOUND
	Sound_Exit();
#endif
	return FALSE;
}

void curses_clear_screen(void)
{
	int x;
	int y;
	for (y = 0; y < 24; y++)
		for (x = 0; x < 40; x++)
			curses_screen[y][x] = ' ';
}

void curses_clear_rectangle(int x1, int y1, int x2, int y2)
{
	int x;
	int y;
	for (y = y1; y <= y2; y++)
		for (x = x1; x <= x2; x++)
			curses_screen[y][x] = ' ';
}

void curses_putch(int x, int y, int ascii, UBYTE fg, UBYTE bg)
{
	/* handle line drawing chars */
	switch (ascii) {
	case 18:
		ascii = '-';
		break;
	case 17:
	case 3:
		ascii = '/';
		break;
	case 26:
	case 5:
		ascii = '\\';
		break;
	case 124:
		ascii = '|';
		break;
	default:
		break;
	}
	if ((bg & 0xf) > (fg & 0xf))
		curses_screen[y][x] = ascii + A_REVERSE;
	else
		curses_screen[y][x] = ascii;
}

void curses_display_line(int anticmode, const UBYTE *screendata)
{
	int y;
	int *p;
	int w;
	if (ANTIC_ypos < 32 || ANTIC_ypos >= 224)
		return;
	y = (ANTIC_ypos >> 3) - 4;
	p = &(curses_screen[y][0]);
	switch (anticmode) {
	case 2:
	case 3:
	case 4:
	case 5:
		switch (ANTIC_DMACTL & 3) {
		case 1:
			p += 4;
			w = 32;
			break;
		case 2:
			w = 40;
			break;
		case 3:
			screendata += 4;
			w = 40;
			break;
		default:
			return;
		}
		do {
			static const int offset[8] = {
				0x20,                       /* 0x00-0x1f: Numbers + !"$% etc. */
				0x20,                       /* 0x20-0x3f: Upper Case Characters */
				A_BOLD,                     /* 0x40-0x5f: Control Characters */
				0,                          /* 0x60-0x7f: Lower Case Characters */
				-0x80 + 0x20 + A_REVERSE,   /* 0x80-0x9f: Numbers + !"$% etc. */
				-0x80 + 0x20 + A_REVERSE,   /* 0xa0-0xbf: Upper Case Characters */
				-0x80 + A_BOLD + A_REVERSE, /* 0xc0-0xdf: Control Characters */
				-0x80 + A_REVERSE           /* 0xe0-0xff: Lower Case Characters */
			};
			UBYTE c = *screendata++;
			/* PDCurses prints '\x7f' as "^?".
			   This causes problems if this is the last character in line.
			   Use bold '>' for Atari's Tab symbol (filled right-pointing triangle). */
			if (c == 0x7f)
				*p = '>' + A_BOLD;
			else if (c == 0xff)
				*p = '>' + A_BOLD + A_REVERSE;
			else
				*p = c + offset[c >> 5];
			p++;
		} while (--w);
		break;
	case 6:
	case 7:
		switch (ANTIC_DMACTL & 3) {
		case 1:
			p += 12;
			w = 16;
			break;
		case 2:
			p += 10;
			w = 20;
			break;
		case 3:
			p += 8;
			w = 24;
			break;
		default:
			return;
		}
		{
#define LIGHT_THRESHOLD 0x0c
			int light[4];
			light[0] = (GTIA_COLPF0 & 0x0e) >= LIGHT_THRESHOLD ? 0x20 + A_BOLD : 0x20;
			light[1] = (GTIA_COLPF1 & 0x0e) >= LIGHT_THRESHOLD ? -0x40 + 0x20 + A_BOLD : -0x40 + 0x20;
			light[2] = (GTIA_COLPF2 & 0x0e) >= LIGHT_THRESHOLD ? -0x80 + 0x20 + A_BOLD : -0x80 + 0x20;
			light[3] = (GTIA_COLPF3 & 0x0e) >= LIGHT_THRESHOLD ? -0xc0 + 0x20 + A_BOLD : -0xc0 + 0x20;
			do {
				*p++ = *screendata + light[*screendata >> 6];
				screendata++;
			} while (--w);
		}
		break;
	default:
		break;
	}
}

void PLATFORM_DisplayScreen(void)
{
	int x;
	int y;
	for (y = 0; y < 24; y++) {
		for (x = 0; x < 40; x++) {
			int ch = curses_screen[y][x];
			switch (curses_mode) {
			default:
			case CURSES_LEFT:
				move(y, x);
				break;
			case CURSES_CENTRAL:
				move(y, 20 + x);
				break;
			case CURSES_RIGHT:
				move(y, 40 + x);
				break;
			case CURSES_WIDE_1:
				move(y, x + x);
				break;
			case CURSES_WIDE_2:
				move(y, x + x);
				addch(ch);
				ch = ' ' + (ch & A_REVERSE);
				break;
			}
			addch(ch);
		}
	}
	refresh();
}

int PLATFORM_Keyboard(void)
{
	int keycode = getch();

#if 0
	/* for debugging */
	if (keycode > 0) {
		Atari800_ErrExit();
		printf("keycode == %d (0x%x)\n", keycode, keycode);
		exit(1);
	}
#endif

	INPUT_key_consol = INPUT_CONSOL_NONE;

	switch (keycode) {
	case 0x01:
		keycode = AKEY_CTRL_a;
		break;
	case 0x02:
		keycode = AKEY_CTRL_b;
		break;
	case 0x03 :
		keycode = AKEY_CTRL_c;
		break;
	case 0x04:
		keycode = AKEY_CTRL_d;
		break;
	case 0x05:
		keycode = AKEY_CTRL_e;
		break;
	case 0x06:
		keycode = AKEY_CTRL_f;
		break;
	case 0x07:
		keycode = AKEY_CTRL_g;
		break;
/*
	case 0x08:
		keycode = AKEY_CTRL_h;
		break;
 */
	case 0x09:
		keycode = AKEY_TAB;
		break;
/*
	case 0x0a:
		keycode = AKEY_CTRL_j;
		break;
 */
	case 0x0b:
		keycode = AKEY_CTRL_k;
		break;
	case 0x0c:
		keycode = AKEY_CTRL_l;
		break;
/*
	case 0x0d:
		keycode = AKEY_CTRL_m;
		break;
 */
	case 0x0e:
		keycode = AKEY_CTRL_n;
		break;
	case 0x0f:
		keycode = AKEY_CTRL_o;
		break;
	case 0x10:
		keycode = AKEY_CTRL_p;
		break;
	case 0x11:
		keycode = AKEY_CTRL_q;
		break;
	case 0x12:
		keycode = AKEY_CTRL_r;
		break;
	case 0x13:
		keycode = AKEY_CTRL_s;
		break;
	case 0x14:
		keycode = AKEY_CTRL_t;
		break;
	case 0x15:
		keycode = AKEY_CTRL_u;
		break;
	case 0x16:
		keycode = AKEY_CTRL_v;
		break;
	case 0x17:
		keycode = AKEY_CTRL_w;
		break;
	case 0x18:
		keycode = AKEY_CTRL_x;
		break;
	case 0x19:
		keycode = AKEY_CTRL_y;
		break;
	case 0x1a:
		keycode = AKEY_CTRL_z;
		break;
	case '`':
		keycode = AKEY_CAPSTOGGLE;
		break;
	case '!':
		keycode = AKEY_EXCLAMATION;
		break;
	case '"':
		keycode = AKEY_DBLQUOTE;
		break;
	case '#':
		keycode = AKEY_HASH;
		break;
	case '$':
		keycode = AKEY_DOLLAR;
		break;
	case '%':
		keycode = AKEY_PERCENT;
		break;
	case '&':
		keycode = AKEY_AMPERSAND;
		break;
	case '\'':
		keycode = AKEY_QUOTE;
		break;
	case '@':
		keycode = AKEY_AT;
		break;
	case '(':
		keycode = AKEY_PARENLEFT;
		break;
	case ')':
		keycode = AKEY_PARENRIGHT;
		break;
	case '[':
		keycode = AKEY_BRACKETLEFT;
		break;
	case ']':
		keycode = AKEY_BRACKETRIGHT;
		break;
	case '<':
		keycode = AKEY_LESS;
		break;
	case '>':
		keycode = AKEY_GREATER;
		break;
	case '=':
		keycode = AKEY_EQUAL;
		break;
	case '?':
		keycode = AKEY_QUESTION;
		break;
#ifdef PADMINUS
	case PADMINUS:
#endif
	case '-':
		keycode = AKEY_MINUS;
		break;
#ifdef PADPLUS
	case PADPLUS:
#endif
	case '+':
		keycode = AKEY_PLUS;
		break;
#ifdef PADSTAR
	case PADSTAR:
#endif
	case '*':
		keycode = AKEY_ASTERISK;
		break;
#ifdef PADSLASH
	case PADSLASH:
#endif
	case '/':
		keycode = AKEY_SLASH;
		break;
	case ':':
		keycode = AKEY_COLON;
		break;
	case ';':
		keycode = AKEY_SEMICOLON;
		break;
	case ',':
		keycode = AKEY_COMMA;
		break;
	case '.':
		keycode = AKEY_FULLSTOP;
		break;
	case '_':
		keycode = AKEY_UNDERSCORE;
		break;
	case '^':
		keycode = AKEY_CIRCUMFLEX;
		break;
	case '\\':
		keycode = AKEY_BACKSLASH;
		break;
	case '|':
		keycode = AKEY_BAR;
		break;
	case ' ':
		keycode = AKEY_SPACE;
		break;
	case '0':
		keycode = AKEY_0;
		break;
	case '1':
		keycode = AKEY_1;
		break;
	case '2':
		keycode = AKEY_2;
		break;
	case '3':
		keycode = AKEY_3;
		break;
	case '4':
		keycode = AKEY_4;
		break;
	case '5':
		keycode = AKEY_5;
		break;
	case '6':
		keycode = AKEY_6;
		break;
	case '7':
		keycode = AKEY_7;
		break;
	case '8':
		keycode = AKEY_8;
		break;
	case '9':
		keycode = AKEY_9;
		break;
	case 'a':
		keycode = AKEY_a;
		break;
	case 'b':
		keycode = AKEY_b;
		break;
	case 'c':
		keycode = AKEY_c;
		break;
	case 'd':
		keycode = AKEY_d;
		break;
	case 'e':
		keycode = AKEY_e;
		break;
	case 'f':
		keycode = AKEY_f;
		break;
	case 'g':
		keycode = AKEY_g;
		break;
	case 'h':
		keycode = AKEY_h;
		break;
	case 'i':
		keycode = AKEY_i;
		break;
	case 'j':
		keycode = AKEY_j;
		break;
	case 'k':
		keycode = AKEY_k;
		break;
	case 'l':
		keycode = AKEY_l;
		break;
	case 'm':
		keycode = AKEY_m;
		break;
	case 'n':
		keycode = AKEY_n;
		break;
	case 'o':
		keycode = AKEY_o;
		break;
	case 'p':
		keycode = AKEY_p;
		break;
	case 'q':
		keycode = AKEY_q;
		break;
	case 'r':
		keycode = AKEY_r;
		break;
	case 's':
		keycode = AKEY_s;
		break;
	case 't':
		keycode = AKEY_t;
		break;
	case 'u':
		keycode = AKEY_u;
		break;
	case 'v':
		keycode = AKEY_v;
		break;
	case 'w':
		keycode = AKEY_w;
		break;
	case 'x':
		keycode = AKEY_x;
		break;
	case 'y':
		keycode = AKEY_y;
		break;
	case 'z':
		keycode = AKEY_z;
		break;
	case 'A':
		keycode = AKEY_A;
		break;
	case 'B':
		keycode = AKEY_B;
		break;
	case 'C':
		keycode = AKEY_C;
		break;
	case 'D':
		keycode = AKEY_D;
		break;
	case 'E':
		keycode = AKEY_E;
		break;
	case 'F':
		keycode = AKEY_F;
		break;
	case 'G':
		keycode = AKEY_G;
		break;
	case 'H':
		keycode = AKEY_H;
		break;
	case 'I':
		keycode = AKEY_I;
		break;
	case 'J':
		keycode = AKEY_J;
		break;
	case 'K':
		keycode = AKEY_K;
		break;
	case 'L':
		keycode = AKEY_L;
		break;
	case 'M':
		keycode = AKEY_M;
		break;
	case 'N':
		keycode = AKEY_N;
		break;
	case 'O':
		keycode = AKEY_O;
		break;
	case 'P':
		keycode = AKEY_P;
		break;
	case 'Q':
		keycode = AKEY_Q;
		break;
	case 'R':
		keycode = AKEY_R;
		break;
	case 'S':
		keycode = AKEY_S;
		break;
	case 'T':
		keycode = AKEY_T;
		break;
	case 'U':
		keycode = AKEY_U;
		break;
	case 'V':
		keycode = AKEY_V;
		break;
	case 'W':
		keycode = AKEY_W;
		break;
	case 'X':
		keycode = AKEY_X;
		break;
	case 'Y':
		keycode = AKEY_Y;
		break;
	case 'Z':
		keycode = AKEY_Z;
		break;
	case 0x1b:
		keycode = AKEY_ESCAPE;
		break;
	case KEY_F0 + 1:
		keycode = AKEY_UI;
		break;
	case KEY_F0 + 2:
		INPUT_key_consol &= ~INPUT_CONSOL_OPTION;
		keycode = AKEY_NONE;
		break;
	case KEY_F0 + 3:
		INPUT_key_consol &= ~INPUT_CONSOL_SELECT;
		keycode = AKEY_NONE;
		break;
	case KEY_F0 + 4:
		INPUT_key_consol &= ~INPUT_CONSOL_START;
		keycode = AKEY_NONE;
		break;
	case KEY_F0 + 5:
		keycode = AKEY_WARMSTART;
		break;
#ifdef KEY_HELP
	case KEY_HELP:
#endif
#ifdef KEY_SHELP
	case KEY_SHELP:
#endif
#ifdef KEY_LHELP
	case KEY_LHELP:
#endif
	case KEY_F0 + 6:
		keycode = AKEY_HELP;
		break;
#ifdef KEY_BREAK
	case KEY_BREAK:
#endif
	case KEY_F0 + 7:
		keycode = AKEY_BREAK;
		break;
	case KEY_F0 + 8:
		keycode = PLATFORM_Exit(TRUE) ? AKEY_NONE : AKEY_EXIT;
		break;
	case KEY_F0 + 9:
		keycode = AKEY_EXIT;
		break;
	case KEY_F0 + 10:
		keycode = AKEY_SCREENSHOT;
		break;
	case KEY_DOWN:
		keycode = AKEY_DOWN;
		break;
	case KEY_LEFT:
		keycode = AKEY_LEFT;
		break;
	case KEY_RIGHT:
		keycode = AKEY_RIGHT;
		break;
	case KEY_UP:
		keycode = AKEY_UP;
		break;
	case 8:
	case 127:
#ifdef KEY_BACKSPACE
# if KEY_BACKSPACE != 8 && KEY_BACKSPACE != 127
	case KEY_BACKSPACE:
# endif
#endif
		keycode = AKEY_BACKSPACE;
		break;
#ifdef PADENTER
	case PADENTER:
#endif
	case KEY_ENTER:
	case '\n':
		keycode = AKEY_RETURN;
		break;
#ifdef KEY_HOME
	case KEY_HOME:
		keycode = AKEY_CLEAR;
		break;
#endif
#ifdef KEY_CLEAR
	case KEY_CLEAR:
		keycode = AKEY_CLEAR;
		break;
#endif
#ifdef KEY_IC
	case KEY_IC:
		keycode = AKEY_INSERT_CHAR;
		break;
#endif
#ifdef KEY_IL
	case KEY_IL:
		keycode = AKEY_INSERT_LINE;
		break;
#endif
#ifdef KEY_DC
	case KEY_DC:
		keycode = AKEY_DELETE_CHAR;
		break;
#endif
#ifdef KEY_DL
	case KEY_DL:
		keycode = AKEY_DELETE_LINE;
		break;
#endif
#ifdef KEY_STAB
	case KEY_STAB:
		keycode = AKEY_SETTAB;
		break;
#endif
#ifdef KEY_CTAB
	case KEY_CTAB:
		keycode = AKEY_CLRTAB;
		break;
#endif
#ifdef ALT_A
	/* PDCurses specific */
	case ALT_A:
		UI_alt_function = UI_MENU_ABOUT;
		keycode = AKEY_UI;
		break;
	case ALT_C:
		UI_alt_function = UI_MENU_CARTRIDGE;
		keycode = AKEY_UI;
		break;
	case ALT_D:
		UI_alt_function = UI_MENU_DISK;
		keycode = AKEY_UI;
		break;
	case ALT_L:
		UI_alt_function = UI_MENU_LOADSTATE;
		keycode = AKEY_UI;
		break;
	case ALT_O:
		UI_alt_function = UI_MENU_SOUND;
		keycode = AKEY_UI;
		break;
	case ALT_R:
		UI_alt_function = UI_MENU_RUN;
		keycode = AKEY_UI;
		break;
	case ALT_S:
		UI_alt_function = UI_MENU_SAVESTATE;
		keycode = AKEY_UI;
		break;
	case ALT_T:
		UI_alt_function = UI_MENU_CASSETTE;
		keycode = AKEY_UI;
		break;
	case ALT_W:
		UI_alt_function = UI_MENU_SOUND_RECORDING;
		keycode = AKEY_UI;
		break;
	case ALT_Y:
		UI_alt_function = UI_MENU_SYSTEM;
		keycode = AKEY_UI;
		break;
#endif
	default:
		keycode = AKEY_NONE;
		break;
	}
	return keycode;
}

int PLATFORM_PORT(int num)
{
	return 0xff;
}

int PLATFORM_TRIG(int num)
{
	return 1;
}

int main(int argc, char **argv)
{
	/* initialise Atari800 core */
	if (!Atari800_Initialise(&argc, argv))
		return 3;

	/* main loop */
	for (;;) {
		INPUT_key_code = PLATFORM_Keyboard();
		Atari800_Frame();
		if (Atari800_display_screen)
			PLATFORM_DisplayScreen();
	}
}
