/*
 * ui_basic.c - Atari look&feel user interface driver
 *
 * Copyright (C) 1995-1998 David Firth
 * Copyright (C) 1998-2010 Atari800 development team (see DOC/CREDITS)
 *
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

#define _POSIX_C_SOURCE 200112L /* for snprintf */

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h> /* free() */
#ifdef HAVE_UNISTD_H
#include <unistd.h> /* getcwd() */
#endif
#ifdef HAVE_DIRECT_H
#include <direct.h> /* getcwd on MSVC*/
#endif
/* XXX: <sys/dir.h>, <ndir.h>, <sys/ndir.h> */
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#include "antic.h"
#include "atari.h"
#include "input.h"
#include "akey.h"
#include "log.h"
#include "memory.h"
#include "platform.h"
#include "screen.h" /* Screen_atari */
#include "ui.h"
#include "util.h"

#ifdef DIRECTX
	#include "win32\main.h"
	#include "ui_basic.h"
#endif

static int initialised = FALSE;
static UBYTE charset[1024];

#ifdef DIRECTX
	POINT UI_mouse_click = {-1, -1};
#endif

const unsigned char UI_BASIC_key_to_ascii[256] =
{
	0x6C, 0x6A, 0x3B, 0x00, 0x00, 0x6B, 0x2B, 0x2A, 0x6F, 0x00, 0x70, 0x75, 0x9B, 0x69, 0x2D, 0x3D,
	0x76, 0x00, 0x63, 0x00, 0x00, 0x62, 0x78, 0x7A, 0x34, 0x00, 0x33, 0x36, 0x1B, 0x35, 0x32, 0x31,
	0x2C, 0x20, 0x2E, 0x6E, 0x00, 0x6D, 0x2F, 0x00, 0x72, 0x00, 0x65, 0x79, 0x7F, 0x74, 0x77, 0x71,
	0x39, 0x00, 0x30, 0x37, 0x7E, 0x38, 0x3C, 0x3E, 0x66, 0x68, 0x64, 0x00, 0x00, 0x67, 0x73, 0x61,

	0x4C, 0x4A, 0x3A, 0x00, 0x00, 0x4B, 0x5C, 0x5E, 0x4F, 0x00, 0x50, 0x55, 0x9B, 0x49, 0x5F, 0x7C,
	0x56, 0x00, 0x43, 0x00, 0x00, 0x42, 0x58, 0x5A, 0x24, 0x00, 0x23, 0x26, 0x1B, 0x25, 0x22, 0x21,
	0x5B, 0x20, 0x5D, 0x4E, 0x00, 0x4D, 0x3F, 0x00, 0x52, 0x00, 0x45, 0x59, 0x9F, 0x54, 0x57, 0x51,
	0x28, 0x00, 0x29, 0x27, 0x9C, 0x40, 0x7D, 0x9D, 0x46, 0x48, 0x44, 0x00, 0x00, 0x47, 0x53, 0x41,

	0x0C, 0x0A, 0x7B, 0x00, 0x00, 0x0B, 0x1E, 0x1F, 0x0F, 0x00, 0x10, 0x15, 0x9B, 0x09, 0x1C, 0x1D,
	0x16, 0x00, 0x03, 0x00, 0x00, 0x02, 0x18, 0x1A, 0x00, 0x00, 0x9B, 0x00, 0x1B, 0x00, 0xFD, 0x00,
	0x00, 0x20, 0x60, 0x0E, 0x00, 0x0D, 0x00, 0x00, 0x12, 0x00, 0x05, 0x19, 0x9E, 0x14, 0x17, 0x11,
	0x00, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x7D, 0xFF, 0x06, 0x08, 0x04, 0x00, 0x00, 0x07, 0x13, 0x01,

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define KB_DELAY       20
#define KB_AUTOREPEAT  3

static int GetKeyPress(void)
{
	int keycode;

	if (UI_alt_function >= 0)
		return 0x1b; /* escape - go to Main Menu */

	PLATFORM_DisplayScreen();

	for (;;) {  
		static int rep = KB_DELAY;
		if (PLATFORM_Keyboard() == AKEY_NONE) {
			rep = KB_DELAY;
			break;
		}
		
		if (rep == 0) {
			rep = KB_AUTOREPEAT;
			break;
		}
		rep--;
		Atari800_Sync();
	}

	do { 
#ifdef DIRECTX
		DoEvents();
#endif	
		Atari800_Sync();
		keycode = PLATFORM_Keyboard();
		switch (keycode) {
		case AKEY_WARMSTART:
			UI_alt_function = UI_MENU_RESETW;
			return 0x1b; /* escape */
		case AKEY_COLDSTART:
			UI_alt_function = UI_MENU_RESETC;
			return 0x1b; /* escape */
		case AKEY_EXIT:
			UI_alt_function = UI_MENU_EXIT;
			return 0x1b; /* escape */
		case AKEY_UI:
#ifdef DIRECTX			
			UI_Run();
#else	
			if (UI_alt_function >= 0)  /* Alt+letter, not F1 */
#endif
			return 0x1b; /* escape */				
			break;
		case AKEY_SCREENSHOT:
			UI_alt_function = UI_MENU_PCX;
			return 0x1b; /* escape */
		case AKEY_SCREENSHOT_INTERLACE:
			UI_alt_function = UI_MENU_PCXI;
			return 0x1b; /* escape */
		default:
			UI_alt_function = -1; /* forget previous Main Menu shortcut */
			break;
		}
	} while (keycode < 0);

	return UI_BASIC_key_to_ascii[keycode];
}

#ifdef DIRECTX
/* Convert atari-pixel based mouse click coordinates to simplified
   UI coordinates consisting of 20 horizontal bands and 2 columns */
void SetMouseIndex(int x, int y)
{
	int yband;
	
	/* set the y-band that the user clicked on */
	yband = y / DX_MENU_ITEM_HEIGHT - 5;
	if (y < 37 || x > 346 || yband < 0 || yband > 20)
		UI_mouse_click.y = -1;
	else
		UI_mouse_click.y = yband;
		
	/* set the x-band that the user clicked on */
	if (x >= 37 && x < 186)
		UI_mouse_click.x = 1;
	else if (x >= 186 && x <= 346)
		UI_mouse_click.x = 2;
	else 
		UI_mouse_click.x = -1;
		
	/* set any click outside of any band to -1,-1 */
	if (UI_mouse_click.x == -1 || UI_mouse_click.y == -1)
		UI_mouse_click.x = UI_mouse_click.y = -1;
}
#endif

static void Plot(int fg, int bg, int ch, int x, int y)
{
#ifdef USE_CURSES
	curses_putch(x, y, ch, (UBYTE) fg, (UBYTE) bg);
#else /* USE_CURSES */
	const UBYTE *font_ptr = charset + (ch & 0x7f) * 8;
	UBYTE *ptr = (UBYTE *) Screen_atari + 24 * Screen_WIDTH + 32 + y * (8 * Screen_WIDTH) + x * 8;
	int i;
	int j;

	for (i = 0; i < 8; i++) {
		UBYTE data = *font_ptr++;
		for (j = 0; j < 8; j++) {
#ifdef USE_COLOUR_TRANSLATION_TABLE
			ANTIC_VideoPutByte(ptr++, (UBYTE) colour_translation_table[data & 0x80 ? fg : bg]);
#else
			ANTIC_VideoPutByte(ptr++, (UBYTE) (data & 0x80 ? fg : bg));
#endif
			data <<= 1;
		}
		ptr += Screen_WIDTH - 8;
	}
#endif /* USE_CURSES */
}

static void Print(int fg, int bg, const char *string, int x, int y, int maxwidth)
{
	char tmpbuf[40];
	if ((int) strlen(string) > maxwidth) {
		int firstlen = (maxwidth - 3) >> 1;
		int laststart = strlen(string) - (maxwidth - 3 - firstlen);
		snprintf(tmpbuf, sizeof(tmpbuf), "%.*s...%s", firstlen, string, string + laststart);
		string = tmpbuf;
	}
	while (*string != '\0')
		Plot(fg, bg, *string++, x++, y);
}

static void CenterPrint(int fg, int bg, const char *string, int y)
{
	int length = strlen(string);
	Print(fg, bg, string, (length < 38) ? (40 - length) >> 1 : 1, y, 38);
}

static void Box(int fg, int bg, int x1, int y1, int x2, int y2)
{
	int x;
	int y;

	for (x = x1 + 1; x < x2; x++) {
		Plot(fg, bg, 18, x, y1);
		Plot(fg, bg, 18, x, y2);
	}

	for (y = y1 + 1; y < y2; y++) {
		Plot(fg, bg, 124, x1, y);
		Plot(fg, bg, 124, x2, y);
	}

	Plot(fg, bg, 17, x1, y1);
	Plot(fg, bg, 5, x2, y1);
	Plot(fg, bg, 3, x2, y2);
	Plot(fg, bg, 26, x1, y2);
}

static void ClearRectangle(int bg, int x1, int y1, int x2, int y2)
{
#ifdef USE_CURSES
	curses_clear_rectangle(x1, y1, x2, y2);
#else
	UBYTE *ptr = (UBYTE *) Screen_atari + Screen_WIDTH * 24 + 32 + x1 * 8 + y1 * (Screen_WIDTH * 8);
	int bytesperline = (x2 - x1 + 1) << 3;
	UBYTE *end_ptr = (UBYTE *) Screen_atari + Screen_WIDTH * 32 + 32 + y2 * (Screen_WIDTH * 8);
	while (ptr < end_ptr) {
#ifdef USE_COLOUR_TRANSLATION_TABLE
		ANTIC_VideoMemset(ptr, (UBYTE) colour_translation_table[bg], bytesperline);
#else
		ANTIC_VideoMemset(ptr, (UBYTE) bg, bytesperline);
#endif
		ptr += Screen_WIDTH;
	}
#endif /* USE_CURSES */
}

static void ClearScreen(void)
{
#ifdef USE_CURSES
	curses_clear_screen();
#else
#ifdef USE_COLOUR_TRANSLATION_TABLE
	ANTIC_VideoMemset((UBYTE *) Screen_atari, colour_translation_table[0x00], Screen_HEIGHT * Screen_WIDTH);
#else
	ANTIC_VideoMemset((UBYTE *) Screen_atari, 0x00, Screen_HEIGHT * Screen_WIDTH);
#endif
	ClearRectangle(0x94, 0, 0, 39, 23);
#endif /* USE_CURSES */
}

static void TitleScreen(const char *title)
{
	CenterPrint(0x9a, 0x94, title, 0);
}

static void BasicUIMessage(const char *msg, int waitforkey)
{
	ClearRectangle(0x94, 1, 22, 38, 22);
	CenterPrint(0x94, 0x9a, msg, 22);
	if (waitforkey)
		GetKeyPress();
	else
		PLATFORM_DisplayScreen();
}

#ifdef GUI_SDL
int GetRawKey(void)
{
	ClearRectangle(0x94, 13, 11, 25, 13);
	Box(0x9a, 0x94, 13, 11, 25, 13);
	CenterPrint(0x94, 0x9a, "Press a key", 12);
	PLATFORM_DisplayScreen();
	return PLATFORM_GetRawKey();
}
#endif /* GUI_SDL */

#ifdef DIRECTX
int GetKeyName(void)
{
	ClearRectangle(0x94, 13, 11, 25, 13);
	Box(0x9a, 0x94, 13, 11, 25, 13);
	CenterPrint(0x94, 0x9a, "Press a key", 12);
	PLATFORM_DisplayScreen();
	return PLATFORM_GetKeyName();
}
#endif

static int Select(int default_item, int nitems, const char *item[],
                  const char *prefix[], const char *suffix[],
                  const char *tip[], const int nonselectable[],
                  int nrows, int ncolumns, int xoffset, int yoffset,
                  int itemwidth, int drag, const char *global_tip,
                  int *seltype)
{
	int offset = 0;
	int index = default_item;
	int localseltype;

	if (seltype == NULL)
		seltype = &localseltype;

	for (;;) {
		int col;
		int row;
		int i;
		const char *message = global_tip;

		while (index < offset)
			offset -= nrows;
		while (index >= offset + nrows * ncolumns)
			offset += nrows;

		ClearRectangle(0x94, xoffset, yoffset, xoffset + ncolumns * (itemwidth + 1) - 2, yoffset + nrows - 1);
		col = 0;
		row = 0;
		for (i = offset; i < nitems; i++) {
			char szbuf[40 + FILENAME_MAX]; /* allow for prefix and suffix */
			char *p = szbuf;
			if (prefix != NULL && prefix[i] != NULL)
				p = Util_stpcpy(szbuf, prefix[i]);
			p = Util_stpcpy(p, item[i]);
			if (suffix != NULL && suffix[i] != NULL) {
				char *q = szbuf + itemwidth - strlen(suffix[i]);
				while (p < q)
					*p++ = ' ';
				strcpy(p, suffix[i]);
			}
			else {
				while (p < szbuf + itemwidth)
					*p++ = ' ';
				*p = '\0';
			}
			if (i == index)
				Print(0x94, 0x9a, szbuf, xoffset + col * (itemwidth + 1), yoffset + row, itemwidth);
			else
				Print(0x9a, 0x94, szbuf, xoffset + col * (itemwidth + 1), yoffset + row, itemwidth);
			if (++row >= nrows) {
				if (++col >= ncolumns)
					break;
				row = 0;
			}
		}
		if (tip != NULL && tip[index] != NULL)
			message = tip[index];
		else if (itemwidth < 38 && (int) strlen(item[index]) > itemwidth)
			/* the selected item was shortened */
			message = item[index];
		if (message != NULL)
			CenterPrint(0x94, 0x9a, message, 22);

		for (;;) {
			int ascii;
			int tmp_index;
			ascii = GetKeyPress();
			switch (ascii) {
			case 0x1c:				/* Up */
				if (drag) {
					*seltype = UI_USER_DRAG_UP;
					return index;
				}
				tmp_index = index;
				do
					tmp_index--;
				while (tmp_index >= 0 && nonselectable != NULL && nonselectable[tmp_index]);
				if (tmp_index >= 0) {
					index = tmp_index;
					break;
				}
				continue;
			case 0x1d:				/* Down */
				if (drag) {
					*seltype = UI_USER_DRAG_DOWN;
					return index;
				}
				tmp_index = index;
				do
					tmp_index++;
				while (tmp_index < nitems && nonselectable != NULL && nonselectable[tmp_index]);
				if (tmp_index < nitems) {
					index = tmp_index;
					break;
				}
				continue;
			case 0x1e:				/* Left */
				if (drag)
					continue;		/* cannot drag left */
				index = (index > nrows) ? index - nrows : 0;
				break;
			case 0x1f:				/* Right */
				if (drag)
					continue;		/* cannot drag right */
				index = (index + nrows < nitems) ? index + nrows : nitems - 1;
				break;
			case 0x7f:				/* Tab (for exchanging disk directories) */
				return -2;			/* GOLDA CHANGED */
			case 0x20:				/* Space */
				*seltype = UI_USER_TOGGLE;
				return index;
			case 0x7e:				/* Backspace */
				*seltype = UI_USER_DELETE;
				return index;
			case 0x9b:				/* Return=Select */
				*seltype = UI_USER_SELECT;
				return index;
#ifdef DIRECTX
			case 0xAA:              /* Mouse click */
			
			/* mouse click location, adjusted by context 
			   this is all we need for one column */
			tmp_index = UI_mouse_click.y - yoffset + 2;
					  
			/* handle two column mode scenarios */
			if (ncolumns == 2) {
				/* special case - do nothing if user clicks empty 
			       bottom cell in column 1 in two column mode.   */	
				if (UI_mouse_click.x == 1 && UI_mouse_click.y == 20) {
					UI_mouse_click.x = UI_mouse_click.y = -1;
					break;
				} 
				/* handle two column, multi-page scenarios */
				else if (UI_mouse_click.x == 1) 
					tmp_index += offset;
				else if (UI_mouse_click.x == 2)
					tmp_index += offset + 20;
			}

			/* if cell is a valid one, update the index */
			if (tmp_index > -1 && tmp_index < nitems)
				index = tmp_index;
			else 
				/* otherwise, invalid item, so do nothing */
				UI_mouse_click.x = UI_mouse_click.y = -1;
				
			break;
#endif 
			case 0x1b:				/* Esc=Cancel */
				return -1;
			default:
				if (drag || ascii <= 0x20 || ascii >= 0x7f)
					continue;
				tmp_index = index; /* old index */
				do {
					if (++index >= nitems)
						index = 0;
				} while (index != tmp_index && !Util_chrieq((char) ascii, item[index][0]));
				break;
			}
			break;
		}
		if (message != NULL)
			ClearRectangle(0x94, 1, 22, 38, 22);
	}
}

static int BasicUISelect(const char *title, int flags, int default_item, const UI_tMenuItem *menu, int *seltype)
{
	int nitems;
	int index;
	const UI_tMenuItem *pmenu;
	static const char *prefix[100];
	static const char *item[100];
	static const char *suffix[100];
	static const char *tip[100];
	static int nonselectable[100];
	int w;
	int x1, y1, x2, y2;

	nitems = 0;
	index = 0;
	for (pmenu = menu; pmenu->flags != UI_ITEM_END; pmenu++) {
		if (pmenu->flags != UI_ITEM_HIDDEN) {
			prefix[nitems] = pmenu->prefix;
			item[nitems] = pmenu->item;
			if (pmenu->flags & UI_ITEM_TIP) {
				suffix[nitems] = NULL;
				tip[nitems] = pmenu->suffix;
			}
			else {
				if ((pmenu->flags & UI_ITEM_TYPE) == UI_ITEM_CHECK) {
					if (pmenu->flags & UI_ITEM_CHECKED)
						suffix[nitems] = "Yes";
					else
						suffix[nitems] = "No ";
				}
				else
					suffix[nitems] = pmenu->suffix;
				tip[nitems] = NULL;
			}
			nonselectable[nitems] = (pmenu->retval < 0);
			if (pmenu->retval == default_item)
				index = nitems;
			nitems++;
		}
	}
	if (nitems == 0)
		return -1; /* cancel immediately */

	if (flags & UI_SELECT_POPUP) {
		int i;
		w = 0;
		for (i = 0; i < nitems; i++) {
			int ws = strlen(item[i]);
			if (prefix[i] != NULL)
				ws += strlen(prefix[i]);
			if (suffix[i] != NULL)
				ws += strlen(suffix[i]);
			if (ws > w)
				w = ws;
		}
		if (w > 38)
			w = 38;

		x1 = (40 - w) / 2 - 1;
		x2 = x1 + w + 1;
		y1 = (24 - nitems) / 2 - 1;
		y2 = y1 + nitems + 1;
	}
	else {
		ClearScreen();
		TitleScreen(title);
		w = 38;
		x1 = 0;
		y1 = 1;
		x2 = 39;
		y2 = 23;
	}

	if (y1 < 0)
		y1 = 0;
	if (y2 > 23)
		y2 = 23;

	Box(0x9a, 0x94, x1, y1, x2, y2);
	index = Select(index, nitems, item, prefix, suffix, tip, nonselectable,
	                y2 - y1 - 1, 1, x1 + 1, y1 + 1, w,
	                (flags & UI_SELECT_DRAG) ? TRUE : FALSE, NULL, seltype);
	if (index < 0)
		return index;
	for (pmenu = menu; pmenu->flags != UI_ITEM_END; pmenu++) {
		if (pmenu->flags != UI_ITEM_HIDDEN) {
			if (index == 0)
				return pmenu->retval;
			index--;
		}
	}
	/* shouldn't happen */
	return -1;
}

static int BasicUISelectInt(int default_value, int min_value, int max_value)
{
	static char item_values[100][4];
	static const char *items[100];
	int value;
	int nitems;
	int nrows;
	int ncolumns;
	int x1, y1, x2, y2;
	if (min_value < 0 || max_value > 99 || min_value > max_value)
		return default_value;
	nitems = 0;
	for (value = min_value; value <= max_value; value++) {
		items[nitems] = item_values[nitems];
		snprintf(item_values[nitems], sizeof(item_values[0]), "%2d", value);
		nitems++;
	}
	if (nitems <= 10) {
		nrows = nitems;
		ncolumns = 1;
	}
	else {
		nrows = 10;
		ncolumns = (nitems + 9) / 10;
	}
	x1 = (39 - 3 * ncolumns) >> 1;
	y1 = (22 - nrows) >> 1;
	x2 = x1 + 3 * ncolumns;
	y2 = y1 + nrows + 1;
	Box(0x9a, 0x94, x1, y1, x2, y2);
	value = Select((default_value >= min_value && default_value <= max_value) ? default_value - min_value : 0,
		nitems, items, NULL, NULL, NULL, NULL, nrows, ncolumns, x1 + 1, y1 + 1, 2, FALSE, NULL, NULL);
	return value >= 0 ? value + min_value : default_value;
}

static int SelectSlider(int fg, int bg, int x, int y, int width,
                        char const *title, int start_value, int max_value,
                        void (*label_fun)(char *label, int value, void *user_data),
                        void *user_data)
{
	enum { larrow = 126,
	       rarrow = 127,
	       bar = 18 };
	int i;
	int value = start_value;
	char label[11];
	int label_length;

	if (value < 0)
		value = 0;
	else if (value > max_value)
		value = max_value;
	Box(fg, bg, x, y, x + 1 + width, y + 2);
	
	Print(bg, fg, title, x + 1, y, width);
	Plot(fg, bg, larrow, x + 1, y + 1);
	Plot(fg, bg, rarrow, x + width, y + 1);

	for (;;) {
		int ascii;
		for (i = x + 2; i < x + width; ++i)
			Plot(fg, bg, bar, i, y + 1);
		(*label_fun)(label, value, user_data);
		label_length = strlen(label);
		Print(bg, fg, label,
		      max_value == 0 ? x + 2 + (width - label_length - 2) / 2
		                     : x + 2 + (width - label_length - 2) * value / max_value,
		      y + 1, label_length);
		ascii = GetKeyPress();
		switch (ascii) {
			case 0x1c:				/* Up */
				value = 0;
				break;
			case 0x1d:				/* Down */
				value = max_value;
				break;
			case 0x1e:				/* Left */
				if (value > 0)
					--value;
				break;
			case 0x1f:				/* Right */
				if (value < max_value)
					++value;
				break;
			case 0x1b:				/* Esc=Cancel */
				/* Restore original state if label_fun causes any side effects. */
				(*label_fun)(label, start_value, user_data);
				return -1;
			case 0x7e:				/* Backspace */
				value = start_value;
				if (value < 0)
					value = 0;
				else if (value > max_value)
					value = max_value;
				break;
			case 0x9b:				/* Return=Select */
				return value;
		}
	}
	return -1;
}

static int BasicUISelectSlider(char const *title, int start_value, int max_value,
                               void (*label_fun)(char *label, int value, void *user_data),
                               void *user_data)
{
	return SelectSlider(0x9a, 0x94, 3, 11, 32, title, start_value, max_value,
			    label_fun, user_data);
}

#ifdef HAVE_WINDOWS_H

static WIN32_FIND_DATA wfd;
static HANDLE dh = INVALID_HANDLE_VALUE;

#ifdef _WIN32_WCE
/* WinCE's FindFirstFile/FindNext file don't return "." or "..". */
/* We check if the parent folder exists and add ".." if necessary. */
static char parentdir[FILENAME_MAX];
#endif

static int BasicUIOpenDir(const char *dirname)
{
#ifdef UNICODE
	WCHAR wfilespec[FILENAME_MAX];
	if (MultiByteToWideChar(CP_ACP, 0, dirname, -1, wfilespec, FILENAME_MAX - 4) <= 0)
		return FALSE;
	wcscat(wfilespec, (dirname[0] != '\0' && dirname[strlen(dirname) - 1] != '\\')
		? L"\\*.*" : L"*.*");
	dh = FindFirstFile(wfilespec, &wfd);
#else /* UNICODE */
	char filespec[FILENAME_MAX];
	Util_strlcpy(filespec, dirname, FILENAME_MAX - 4);
	strcat(filespec, (dirname[0] != '\0' && dirname[strlen(dirname) - 1] != '\\')
		? "\\*.*" : "*.*");
	dh = FindFirstFile(filespec, &wfd);
#endif /* UNICODE */
#ifdef _WIN32_WCE
	Util_splitpath(dirname, parentdir, NULL);
#endif
	if (dh == INVALID_HANDLE_VALUE) {
		/* don't raise error if the path is ok but has no entries:
		   Win98 returns ERROR_FILE_NOT_FOUND,
		   WinCE returns ERROR_NO_MORE_FILES */
		DWORD err = GetLastError();
		if (err != ERROR_FILE_NOT_FOUND && err != ERROR_NO_MORE_FILES)
			return FALSE;
	}
	return TRUE;
}

static int BasicUIReadDir(char *filename, int *isdir)
{
	if (dh == INVALID_HANDLE_VALUE) {
#ifdef _WIN32_WCE
		if (parentdir[0] != '\0' && Util_direxists(parentdir)) {
			strcpy(filename, "..");
			*isdir = TRUE;
			parentdir[0] = '\0';
			return TRUE;
		}
#endif /* _WIN32_WCE */
		return FALSE;
	}
#ifdef UNICODE
	if (WideCharToMultiByte(CP_ACP, 0, wfd.cFileName, -1, filename, FILENAME_MAX, NULL, NULL) <= 0)
		filename[0] = '\0';
#else
	Util_strlcpy(filename, wfd.cFileName, FILENAME_MAX);
#endif /* UNICODE */
#ifdef _WIN32_WCE
	/* just in case they will implement it some day */
	if (strcmp(filename, "..") == 0)
		parentdir[0] = '\0';
#endif
	*isdir = (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;
	if (!FindNextFile(dh, &wfd)) {
		FindClose(dh);
		dh = INVALID_HANDLE_VALUE;
	}
	return TRUE;
}

#define DO_DIR

#elif defined(HAVE_OPENDIR)

static char dir_path[FILENAME_MAX];
static DIR *dp = NULL;

static int BasicUIOpenDir(const char *dirname)
{
	Util_strlcpy(dir_path, dirname, FILENAME_MAX);
	dp = opendir(dir_path);
	return dp != NULL;
}

static int BasicUIReadDir(char *filename, int *isdir)
{
	struct dirent *entry;
	char fullfilename[FILENAME_MAX];
	struct stat st;
	entry = readdir(dp);
	if (entry == NULL) {
		closedir(dp);
		dp = NULL;
		return FALSE;
	}
	strcpy(filename, entry->d_name);
	Util_catpath(fullfilename, dir_path, entry->d_name);
	stat(fullfilename, &st);
	*isdir = S_ISDIR(st.st_mode);
	return TRUE;
}

#define DO_DIR

#elif defined(PS2)

int Atari_OpenDir(const char *filename);

static int BasicUIOpenDir(const char *dirname)
{
	char filename[FILENAME_MAX];
	Util_catpath(filename, dirname, "*");
	return Atari_OpenDir(filename);
}

int Atari_ReadDir(char *fullpath, char *filename, int *isdir,
                  int *readonly, int *size, char *timetext);

#define BasicUIReadDir(filename, isdir)  Atari_ReadDir(NULL, filename, isdir, NULL, NULL, NULL)

#define DO_DIR

#endif /* defined(PS2) */


#ifdef DO_DIR

static const char **filenames;
#define FILENAMES_INITIAL_SIZE 256 /* preallocate 1 KB */
static int n_filenames;

/* filename must be malloc'ed or strdup'ed */
static void FilenamesAdd(const char *filename)
{
	if (n_filenames >= FILENAMES_INITIAL_SIZE && (n_filenames & (n_filenames - 1)) == 0) {
		/* n_filenames is a power of two: allocate twice as much */
		filenames = (const char **) Util_realloc((void *) filenames, 2 * n_filenames * sizeof(const char *));
	}
	filenames[n_filenames++] = filename;
}

static int FilenamesCmp(const char *filename1, const char *filename2)
{
	if (filename1[0] == '[') {
		if (filename2[0] != '[')
			return -1;
		if (filename1[1] == '.') {
			if (filename2[1] != '.')
				return -1;
			/* return Util_stricmp(filename1, filename2); */
		}
		else if (filename2[1] == '.')
			return 1;
		/* return Util_stricmp(filename1, filename2); */
	}
	else if (filename2[0] == '[')
		return 1;
	return Util_stricmp(filename1, filename2);
}

/* quicksort */
static void FilenamesSort(const char **start, const char **end)
{
	while (start + 1 < end) {
		const char **left = start + 1;
		const char **right = end;
		const char *pivot = *start;
		const char *tmp;
		while (left < right) {
			if (FilenamesCmp(*left, pivot) <= 0)
				left++;
			else {
				right--;
				tmp = *left;
				*left = *right;
				*right = tmp;
			}
		}
		left--;
		tmp = *left;
		*left = *start;
		*start = tmp;
		FilenamesSort(start, left);
		start = right;
	}
}

static void FilenamesFree(void)
{
	while (n_filenames > 0)
		free((void *) filenames[--n_filenames]);
	free((void *) filenames);
}

static void GetDirectory(const char *directory)
{
#ifdef __DJGPP__
	unsigned short s_backup = _djstat_flags;
	_djstat_flags = _STAT_INODE | _STAT_EXEC_EXT | _STAT_EXEC_MAGIC | _STAT_DIRSIZE |
		_STAT_ROOT_TIME | _STAT_WRITEBIT;
	/* we do not need any of those 'hard-to-get' informations */
#endif	/* DJGPP */

	filenames = (const char **) Util_malloc(FILENAMES_INITIAL_SIZE * sizeof(const char *));
	n_filenames = 0;

	if (BasicUIOpenDir(directory)) {
		char filename[FILENAME_MAX];
		int isdir;

		while (BasicUIReadDir(filename, &isdir)) {
			char *filename2;

			if (filename[0] == '\0' ||
				(filename[0] == '.' && filename[1] == '\0'))
				continue;

			if (isdir) {
				/* add directories as [dir] */
				size_t len = strlen(filename);
				filename2 = (char *) Util_malloc(len + 3);
				memcpy(filename2 + 1, filename, len);
				filename2[0] = '[';
				filename2[len + 1] = ']';
				filename2[len + 2] = '\0';
			}
			else
				filename2 = Util_strdup(filename);

			FilenamesAdd(filename2);
		}

		FilenamesSort(filenames, filenames + n_filenames);
	}
	else {
		Log_print("Error opening '%s' directory", directory);
	}
#ifdef PS2
	FilenamesAdd(Util_strdup("[mc0:]"));
#endif
#ifdef DOS_DRIVES
	/* in DOS/Windows, add all existing disk letters */
	{
		char letter;
#ifdef HAVE_WINDOWS_H
		DWORD drive_mask = GetLogicalDrives();
		for (letter = 'A'; letter <= 'Z'; letter++) {
			if (drive_mask & 1) {
				static char drive2[5] = "[C:]";
				drive2[1] = letter;
				FilenamesAdd(Util_strdup(drive2));
			}
			drive_mask >>= 1;
		}
#else /* HAVE_WINDOWS_H */
		for (letter = 'A'; letter <= 'Z'; letter++) {
#ifdef __DJGPP__
			static char drive[3] = "C:";
			struct stat st;
			drive[0] = letter;
			/* don't check floppies - it's slow */
			if (letter < 'C' || (stat(drive, &st) == 0 && (st.st_mode & S_IXUSR) != 0))
#endif /* __DJGPP__ */
			{
				static char drive2[5] = "[C:]";
				drive2[1] = letter;
				FilenamesAdd(Util_strdup(drive2));
			}
		}
#endif /* HAVE_WINDOWS_H */
	}
#endif /* DOS_DRIVES */
#ifdef __DJGPP__
	_djstat_flags = s_backup;	/* restore the original state */
#endif
}

static void strcatchr(char *s, char c)
{
	while (*s != '\0')
		s++;
	s[0] = c;
	s[1] = '\0';
}

/* Fills BUF with the path of the current working directory (or, if it fails,
   with "." or "/"). */
static void GetCurrentDir(char buf[FILENAME_MAX])
{
#ifdef HAVE_GETCWD
	if (getcwd(buf, FILENAME_MAX) == NULL) {
		buf[0] = '/';
		buf[1] = '\0';
	}
#else
	buf[0] = '.';
	buf[1] = '\0';
#endif
}

/* Select file or directory.
   The result is returned in path and path is where selection begins (i.e. it must be initialized).
   pDirectories are "favourite" directories (there are nDirectories of them). */
static int FileSelector(char *path, int select_dir, char pDirectories[][FILENAME_MAX], int nDirectories)
{
	char current_dir[FILENAME_MAX];
	char highlighted_file[FILENAME_MAX + 2]; /* +2 for square brackets */
	highlighted_file[0] = '\0';
	if (path[0] == '\0' && nDirectories > 0)
		strcpy(current_dir, pDirectories[0]);
	else if (select_dir)
		strcpy(current_dir, path);
	else
		Util_splitpath(path, current_dir, highlighted_file);
#ifdef __DJGPP__
	{
		char help_dir[FILENAME_MAX];
		_fixpath(current_dir, help_dir);
		strcpy(current_dir, help_dir);
	}
#elif defined(HAVE_GETCWD)
	if (current_dir[0] == '\0' || (current_dir[0] == '.' && current_dir[1] == '\0'))
#else
	if (current_dir[0] == '\0')
#endif
		GetCurrentDir(current_dir);
	for (;;) {
		int index = 0;
		int i;

#define NROWS 20
#define NCOLUMNS 2
#define MAX_FILES (NROWS * NCOLUMNS)

		/* The WinCE version may spend several seconds when there are many
		   files in the directory. */
		/* The extra spaces are needed to clear the previous window title. */
		TitleScreen("            Please wait...            ");
		PLATFORM_DisplayScreen();

		for (;;) {
			GetDirectory(current_dir);

			if (n_filenames > 0)
				break;

			/* Can't read directory - maybe it doesn't exist?
			   Split the last part from the path and try again. */
			FilenamesFree();
			{
				char temp[FILENAME_MAX];
				strcpy(temp, current_dir);
				Util_splitpath(temp, current_dir, NULL);
			}
			if (current_dir[0] == '\0') {
				/* Path couldn't be split further.
				   Try the working directory as a last resort. */
				GetCurrentDir(current_dir);
				GetDirectory(current_dir);
				if (n_filenames >= 0)
					break;

				FilenamesFree();
				BasicUIMessage("No files inside directory", 1);
				return FALSE;
			}
		}

		if (highlighted_file[0] != '\0') {
			for (i = 0; i < n_filenames; i++) {
				if (strcmp(filenames[i], highlighted_file) == 0) {
					index = i;
					break;
				}
			}
		}

		for (;;) {
			int seltype;
			const char *selected_filename;

			ClearScreen();
			TitleScreen(current_dir);
			Box(0x9a, 0x94, 0, 1, 39, 23);

			index = Select(index, n_filenames, filenames, NULL, NULL, NULL, NULL,
			               NROWS, NCOLUMNS, 1, 2, 37 / NCOLUMNS, FALSE,
			               select_dir ? "Space: select current directory" : NULL,
			               &seltype);

			if (index == -2) {
				/* Tab = next favourite directory */
				if (nDirectories > 0) {
					/* default: pDirectories[0] */
					int current_index = nDirectories - 1;
					/* are we in one of pDirectories? */
					for (i = 0; i < nDirectories; i++)
						if (strcmp(pDirectories[i], current_dir) == 0) {
							current_index = i;
							break;
						}
					i = current_index;
					do {
						if (++i >= nDirectories)
							i = 0;
						if (Util_direxists(pDirectories[i])) {
							strcpy(current_dir, pDirectories[i]);
							break;
						}
					} while (i != current_index);
				}
				highlighted_file[0] = '\0';
				break;
			}
			if (index < 0) {
				/* Esc = cancel */
				FilenamesFree();
				return FALSE;
			}
			if (seltype == UI_USER_DELETE) {
				/* Backspace = parent directory */
				char new_dir[FILENAME_MAX];
				Util_splitpath(current_dir, new_dir, highlighted_file + 1);
				if (Util_direxists(new_dir)) {
					strcpy(current_dir, new_dir);
					highlighted_file[0] = '[';
					strcatchr(highlighted_file, ']');
					break;
				}
				BasicUIMessage("Cannot enter parent directory", 1);
				continue;
			}
			if (seltype == UI_USER_TOGGLE && select_dir) {
				/* Space = select current directory */
				strcpy(path, current_dir);
				FilenamesFree();
				return TRUE;
			}
			selected_filename = filenames[index];
			if (selected_filename[0] == '[') {
				/* Change directory */
				char new_dir[FILENAME_MAX];

				highlighted_file[0] = '\0';
				if (strcmp(selected_filename, "[..]") == 0) {
					/* go up */
					Util_splitpath(current_dir, new_dir, highlighted_file + 1);
					highlighted_file[0] = '[';
					strcatchr(highlighted_file, ']');
				}
#ifdef PS2
				else if (strcmp(selected_filename, "[mc0:]") == 0) {
					strcpy(new_dir, "mc0:/");
				}
#endif
#ifdef DOS_DRIVES
				else if (selected_filename[2] == ':' && selected_filename[3] == ']') {
					/* disk selected */
					new_dir[0] = selected_filename[1];
					new_dir[1] = ':';
					new_dir[2] = '\\';
					new_dir[3] = '\0';
				}
#endif
				else {
					/* directory selected */
					char *pbracket = strrchr(selected_filename, ']');
					if (pbracket == NULL)
						continue; /* XXX: regular file? */
					*pbracket = '\0';	/*cut ']' */
					Util_catpath(new_dir, current_dir, selected_filename + 1);
				}
				/* check if new directory is valid */
				if (Util_direxists(new_dir)) {
					strcpy(current_dir, new_dir);
					break;
				}
				BasicUIMessage("Cannot enter selected directory", 1);
				continue;
			}
			if (!select_dir) {
				/* normal filename selected */
				Util_catpath(path, current_dir, selected_filename);
				FilenamesFree();
				return TRUE;
			}
		}

		FilenamesFree();
	}
}

#endif /* DO_DIR */

/* nDirectories >= 0 means we are editing a file name */
static int EditString(int fg, int bg, const char *title,
                      char *string, int size, int x, int y, int width,
                      char pDirectories[][FILENAME_MAX], int nDirectories)
{
	int caret = strlen(string);
	int offset = 0;
	for (;;) {
		int i;
		char *p;
		int ascii;
		Box(fg, bg, x, y, x + 1 + width, y + 2);
		Print(bg, fg, title, x + 1, y, width);
		if (caret - offset >= width)
			offset = caret - width + 1;
		else if (caret < offset)
			offset = caret;
		p = string + offset;
		for (i = 0; i < width; i++)
			if (offset + i == caret)
				Plot(bg, fg, *p != '\0' ? *p++ : ' ', x + 1 + i, y + 1);
			else
				Plot(fg, bg, *p != '\0' ? *p++ : ' ', x + 1 + i, y + 1);
		ascii = GetKeyPress();
		switch (ascii) {
		case 0x1e:				/* Cursor Left */
			if (caret > 0)
				caret--;
			break;
		case 0x1f:				/* Cursor Right */
			if (string[caret] != '\0')
				caret++;
			break;
		case 0x7e:				/* Backspace */
			if (caret > 0) {
				caret--;
				p = string + caret;
				do
					p[0] = p[1];
				while (*p++ != '\0');
			}
			break;
		case 0xfe:				/* Delete */
			if (string[caret] != '\0') {
				p = string + caret;
				do
					p[0] = p[1];
				while (*p++ != '\0');
			}
			break;
		case 0x7d:				/* Clear screen */
		case 0x9c:				/* Delete line */
			caret = 0;
			string[0] = '\0';
			break;
		case 0x9b:				/* Return */
			if (nDirectories >= 0) {
				/* check filename */
				char lastchar;
				if (string[0] == '\0')
					return FALSE;
				lastchar = string[strlen(string) - 1];
				return lastchar != '/' && lastchar != '\\';
			}
			return TRUE;
		case 0x1b:				/* Esc */
			return FALSE;
#ifdef DO_DIR
		case 0x7f:				/* Tab = select directory */
			if (nDirectories >= 0) {
				char temp_filename[FILENAME_MAX + 1];
				char temp_path[FILENAME_MAX];
				char temp_file[FILENAME_MAX];
				char *s;
				/* FIXME: now we append '*' and then discard it
				   just to workaround Util_splitpath() not recognizing
				   Util_DIR_SEP_CHAR when it's the last character */
				strcpy(Util_stpcpy(temp_filename, string), "*");
				Util_splitpath(temp_filename, temp_path, temp_file);
				s = temp_file + strlen(temp_file) - 1;
				if (*s == '*') { /* XXX: should be always... */
					*s = '\0';
					if (FileSelector(temp_path, TRUE, pDirectories, nDirectories)) {
						Util_catpath(string, temp_path, temp_file);
						caret = strlen(string);
						offset = 0;
					}
				}
			}
			break;
#endif
		default:
			/* Insert character */
			i = strlen(string);
			if (i + 1 < size && ascii >= ' ' && ascii < 0x7f) {
				do
					string[i + 1] = string[i];
				while (--i >= caret);
				string[caret++] = (char) ascii;
			}
			break;
		}
	}
}

/* returns TRUE if accepted filename */
static int EditFilename(const char *title, char *filename, char directories[][FILENAME_MAX], int n_directories)
{
	char edited_filename[FILENAME_MAX];
	strcpy(edited_filename, filename);
	if (edited_filename[0] == '\0') {
		if (n_directories > 0)
			strcpy(edited_filename, directories[0]);
#ifdef HAVE_GETCWD
		if (edited_filename[0] == '\0') {
			if (getcwd(edited_filename, FILENAME_MAX) == NULL) {
				edited_filename[0] = '/';
				edited_filename[1] = '\0';
			}
			if (edited_filename[0] != '\0' && strlen(edited_filename) < FILENAME_MAX - 1) {
				char *p = edited_filename + strlen(edited_filename) - 1;
				if (*p != '/' && *p != '\\') {
					p[1] = Util_DIR_SEP_CHAR;
					p[2] = '\0';
				}
			}
		}
#endif
	}
	if (!EditString(0x9a, 0x94, title, edited_filename, FILENAME_MAX, 1, 11, 36, directories, n_directories))
		return FALSE;
	strcpy(filename, edited_filename);
	return TRUE;
}

static int BasicUIEditString(const char *title, char *string, int size)
{
	return EditString(0x9a, 0x94, title, string, size, 3, 11, 32, NULL, -1);
}

static int BasicUIGetSaveFilename(char *filename, char directories[][FILENAME_MAX], int n_directories)
{
#ifdef DO_DIR
	return EditFilename("Save as ([Tab] = directory locator)", filename, directories, n_directories);
#else
	return EditFilename("Save as", filename, directories, n_directories);
#endif
}

static int BasicUIGetLoadFilename(char *filename, char directories[][FILENAME_MAX], int n_directories)
{
#ifdef DO_DIR
	return FileSelector(filename, FALSE, directories, n_directories);
#else
	return EditFilename("Filename", filename, directories, n_directories);
#endif
}

static int BasicUIGetDirectoryPath(char *directory)
{
#ifdef DO_DIR
	return FileSelector(directory, TRUE, NULL, 0);
#else
	return EditFilename("Path", directory, NULL, -1);
#endif
}

static void BasicUIInfoScreen(const char *title, const char *message)
{
	int y = 2;
	ClearScreen();
	TitleScreen(title);
	Box(0x9a, 0x94, 0, 1, 39, 23);
	while (*message != '\n') {
		CenterPrint(0x9a, 0x94, message, y++);
		while (*message++ != '\0');
	}
	BasicUIMessage("Press any key to continue", 1);
}

static void BasicUIInit(void)
{
	if (!initialised) {
		MEMORY_GetCharset(charset);
		initialised = TRUE;
	}
}

UI_tDriver UI_BASIC_driver = {
	&BasicUISelect,
	&BasicUISelectInt,
	&BasicUISelectSlider,
	&BasicUIEditString,
	&BasicUIGetSaveFilename,
	&BasicUIGetLoadFilename,
	&BasicUIGetDirectoryPath,
	&BasicUIMessage,
	&BasicUIInfoScreen,
	&BasicUIInit
};

#ifdef USE_UI_BASIC_ONSCREEN_KEYBOARD

int UI_BASIC_OnScreenKeyboard(const char *title, int layout)
{
#define LAYOUT_LEFT    2
#define LAYOUT_TOP     5
#define LAYOUT_WIDTH   36
#define LAYOUT_HEIGHT  6
	const char *layout_lines[LAYOUT_HEIGHT];
	static int modifiers = 0;
	static int key_x = 0;
	static int key_y = 1;

	BasicUIInit();
	ClearScreen();
	TitleScreen(title != NULL ? title : "Keyboard emulator");
	Box(0x9a, 0x94, 0, 1, 39, 23);
#ifdef DREAMCAST
	CenterPrint(0x9a, 0x94, "Dreamcast controller buttons:", 20);
	if (title != NULL) {
		CenterPrint(0x9a, 0x94, "A  --  leave with key selected", 21);
		CenterPrint(0x9a, 0x94, "L, R, B  --  leave without selection", 22);
	}
	else {
		CenterPrint(0x9a, 0x94, "A  --  leave with key pressed", 21);
		CenterPrint(0x9a, 0x94, "L, R, B  --  leave without keypress", 22);
	}
#endif
	modifiers &= AKEY_SHFT;
	switch (layout) {
	case Atari800_MACHINE_800:
		layout_lines[0] = "     Start Select Option Atari Break";
		break;
	case Atari800_MACHINE_XLXE:
		layout_lines[0] = "  Help Start Select Option Inv Break";
		break;
	case Atari800_MACHINE_5200:
		layout_lines[0] = NULL;
		break;
	default:
		layout_lines[0] = NULL;
		break;
	}
	for (;;) {
		int x;
		int y;
		int code;
		const char *layout_line;
		if (layout == Atari800_MACHINE_5200) {
			layout_lines[1] = "        Start  Pause  Reset         ";
			layout_lines[2] = "        --1--  --2--  --3--         ";
			layout_lines[3] = "        --4--  --5--  --6--         ";
			layout_lines[4] = "        --7--  --8--  --9--         ";
			layout_lines[5] = "        --*--  --0--  --#--         ";
		}
		else {
			if ((modifiers & AKEY_CTRL) == 0)
				Print(0x9a, 0x94, "         ", 10, 17, 40);
			else
				Print(0x94, 0x9a, " CONTROL ", 10, 17, 40);
			if ((modifiers & AKEY_SHFT) == 0) {
				Print(0x9a, 0x94, "       ", 2, 17, 40);
				layout_lines[1] = "-Esc 1 2 3 4 5 6 7 8 9 0 < > BackSpc";
				layout_lines[2] = "-Tab- Q W E R T Y U I O P - = Return";
				layout_lines[3] = "-Ctrl- A S D F G H J K L ; + * -Caps";
				layout_lines[4] = "-Shift- Z X C V B N M , . / --Shift-";
			}
			else {
				Print(0x94, 0x9a, " SHIFT ", 2, 17, 40);
				layout_lines[1] = "-Esc ! \" # $ % & ' @ ( ) Clr Ins Del";
				layout_lines[2] = "-Tab- Q W E R T Y U I O P _ | Return";
				layout_lines[3] = "-Ctrl- A S D F G H J K L : \\ ^ -Caps";
				layout_lines[4] = "-Shift- Z X C V B N M [ ] ? --Shift-";
			}
			layout_lines[5] = "        -------Space-------         ";
		}
		for (y = 0; y < LAYOUT_HEIGHT; y++)
			if (layout_lines[y] != NULL)
				Print(0x9a, 0x94, layout_lines[y], LAYOUT_LEFT, LAYOUT_TOP + 2 * y, LAYOUT_WIDTH);
		if (layout_lines[key_y] == NULL)
			key_y = 1;
		layout_line = layout_lines[key_y];
		x = key_x;
		/* key_x normally points to inside of a key... */
		if (layout_line[x] != ' ')
			/* find the beginning of this key */
			while (x > 0 && layout_line[x - 1] != ' ')
				x--;
		/* ... if it does not, take the first key in this line. */
		else
			for (x = 0; layout_line[x] == ' '; x++);
		/* highlight the key */
		do
			Plot(0x94, 0x9a, layout_line[x], LAYOUT_LEFT + x, LAYOUT_TOP + 2 * key_y);
		while (layout_line[++x] > ' ');
		/* handle user input */
		switch (GetKeyPress()) {
		case 0x1c:
			if (key_y == 0 || layout_lines[key_y - 1] == NULL)
				break;
			key_y--;
			if (key_x > 0 && layout_lines[key_y][key_x] == ' ')
				key_x--;
			break;
		case 0x1d:
			if (key_y >= LAYOUT_HEIGHT - 1)
				break;
			key_y++;
			if (layout_lines[key_y][key_x] == ' ' && layout_lines[key_y][key_x + 1] != ' ')
				key_x++;
			break;
		case 0x1e:
			while (x > 0) {
				if (layout_line[--x] == ' ') {
					while (x > 0) {
						if (layout_line[--x] > ' ') {
							key_x = x;
							break;
						}
					}
					break;
				}
			}
			break;
		case 0x1f:
			while (layout_line[x] == ' ')
				x++;
			if (layout_line[x] > ' ')
				key_x = x;
			break;
		case 0x1b:
			return AKEY_NONE;
		case 0x9b:
#ifdef _WIN32_WCE
		case 0x20:
#endif
			code = 0;
			while (--x > 0) {
				if (layout_line[x] == ' ') {
					while (x > 0) {
						if (layout_line[--x] > ' ') {
							code++;
							break;
						}
					}
				}
			}
			if (layout == Atari800_MACHINE_5200) {
				static const UBYTE keycodes_5200[5][3] = {
					{ AKEY_5200_START, AKEY_5200_PAUSE, AKEY_5200_RESET },
					{ AKEY_5200_1, AKEY_5200_2, AKEY_5200_3 },
					{ AKEY_5200_4, AKEY_5200_5, AKEY_5200_6 },
					{ AKEY_5200_7, AKEY_5200_8, AKEY_5200_9 },
					{ AKEY_5200_ASTERISK, AKEY_5200_0, AKEY_5200_HASH }
				};
				return keycodes_5200[key_y - 1][code];
			}
			else {
				static const UBYTE keycodes_normal[4][14] = {
					{ AKEY_ESCAPE, AKEY_1, AKEY_2, AKEY_3, AKEY_4, AKEY_5, AKEY_6,
					  AKEY_7, AKEY_8, AKEY_9, AKEY_0, AKEY_LESS, AKEY_GREATER, AKEY_BACKSPACE },
					{ AKEY_TAB, AKEY_q, AKEY_w, AKEY_e, AKEY_r, AKEY_t, AKEY_y,
					  AKEY_u, AKEY_i, AKEY_o, AKEY_p, AKEY_MINUS, AKEY_EQUAL, AKEY_RETURN },
					{ AKEY_CTRL, AKEY_a, AKEY_s, AKEY_d, AKEY_f, AKEY_g, AKEY_h,
					  AKEY_j, AKEY_k, AKEY_l, AKEY_SEMICOLON, AKEY_PLUS, AKEY_ASTERISK, AKEY_CAPSTOGGLE },
					{ AKEY_SHFT, AKEY_z, AKEY_x, AKEY_c, AKEY_v, AKEY_b, AKEY_n,
					  AKEY_m, AKEY_COMMA, AKEY_FULLSTOP, AKEY_SLASH, AKEY_SHFT, AKEY_SHFT, AKEY_SHFT }
				};
				switch (key_y) {
				case 0:
					switch (code + (layout != Atari800_MACHINE_XLXE ? 1 : 0)) {
					case 0:
						return AKEY_HELP ^ modifiers;
					case 1:
						return AKEY_START;
					case 2:
						return AKEY_SELECT;
					case 3:
						return AKEY_OPTION;
					case 4:
						return AKEY_ATARI ^ modifiers;
					case 5:
						return AKEY_BREAK;
					}
				case 5:
					return AKEY_SPACE ^ modifiers;
				default:
					code = keycodes_normal[key_y - 1][code];
					if (code == AKEY_SHFT || code == AKEY_CTRL)
						modifiers ^= code;
					else
						return code ^ modifiers;
					break;
				}
			}
			break;
		default:
			break;
		}
	}
}

#endif /* USE_UI_BASIC_ONSCREEN_KEYBOARD */
