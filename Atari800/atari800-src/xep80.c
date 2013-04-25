/*
 * xep80.c - XEP80 emulation
 *
 * Copyright (C) 2007 Mark Grebe
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"
#ifdef XEP80_EMULATION
#include "xep80.h"
#include "xep80_fonts.h"
#include "statesav.h"
#include "atari.h"
#include "antic.h"
#include <string.h>
#include "platform.h"
#include "util.h"
#include "log.h"
#if SUPPORTS_CHANGE_VIDEOMODE
#include <videomode.h>
#endif /* SUPPORTS_CHANGE_VIDEOMODE */

#define IN_QUEUE_SIZE		10

/* Definitions for command protocol between the XEP and the Atari */
#define CMD_XX_MASK      0xC0
#define CMD_00           0x00
#define CMD_X_CUR_LOW    0x00
#define CMD_01           0x40
#define CMD_01_MASK      0x70
#define CMD_X_CUR_UPPER  0x40
#define CMD_X_CUR_HIGH   0x50
#define CMD_LEFT_MAR_L   0x60
#define CMD_LEFT_MAR_H   0x70
#define CMD_10           0x80
#define CMD_10_MASK      0xF0
#define CMD_Y_CUR_LOW    0x80
#define CMD_1001         0x90
#define CMD_1001_MASK    0xF8
#define CMD_Y_CUR_HIGH   0x90
#define CMD_Y_CUR_STATUS 0x98
#define CMD_GRAPH_60HZ   0x99
#define CMD_GRAPH_50HZ   0x9A
#define CMD_RIGHT_MAR_L  0xA0
#define CMD_RIGHT_MAR_H  0xB0
#define CMD_11           0xC0
#define CMD_GET_CHAR     0xC0
#define CMD_REQ_X_CUR    0xC1
#define CMD_MRST         0xC2
#define CMD_PRT_STAT     0xC3
#define CMD_FILL_PREV    0xC4
#define CMD_FILL_SPACE   0xC5
#define CMD_FILL_EOL     0xC6
#define CMD_CLR_LIST     0xD0
#define CMD_SET_LIST     0xD1
#define CMD_SCR_NORMAL   0xD2
#define CMD_SCR_BURST    0xD3
#define CMD_CHAR_SET_A   0xD4
#define CMD_CHAR_SET_B   0xD5
#define CMD_CHAR_SET_INT 0xD6
#define CMD_TEXT_50HZ    0xD7
#define CMD_CUR_OFF      0xD8
#define CMD_CUR_ON       0xD9
#define CMD_CUR_BLINK    0xDA
#define CMD_CUR_ST_LINE  0xDB
#define CMD_SET_SCRL_WIN 0xDC
#define CMD_SET_PRINT    0xDD
#define CMD_WHT_ON_BLK   0xDE
#define CMD_BLK_ON_WHT   0xDF
#define CMD_VIDEO_CTRL   0xED
#define CMD_ATTRIB_A     0xF4
#define CMD_ATTRIB_B     0xF5

#define CHAR_SET_A          0
#define CHAR_SET_B          1
#define CHAR_SET_INTERNAL   2

/* These center the graphics screen inside of the XEP80 screen */
#define GRAPH_X_OFFSET ((XEP80_SCRN_WIDTH - XEP80_GRAPH_WIDTH) / 2)
#define GRAPH_Y_OFFSET ((XEP80_scrn_height - XEP80_GRAPH_HEIGHT) / 2)

/* Used to determine if a character is double width */
#define IS_DOUBLE(x,y) (((char_data(y, x) & 0x80) && font_b_double) || \
                        (((char_data(y, x) & 0x80) == 0) && font_a_double))

/* Global variables */
int XEP80_enabled = FALSE;
int XEP80_port = 0;

int XEP80_char_height = XEP80_CHAR_HEIGHT_NTSC;
int XEP80_scrn_height = XEP80_HEIGHT * XEP80_CHAR_HEIGHT_NTSC;

/* Local state variables */
static int output_word = 0;

static UWORD input_queue[IN_QUEUE_SIZE];
static int input_count = 0;

/* Indicates moment when receiving of a word started,
   or a moment when transmitting of the first word in the output queue
   started. Used to compare with ANTIC_CPU_CLOCK when determining
   a bit currently transmitted. */
static unsigned int start_trans_cpu_clock;
/* Indicates that a byte is currently being received. */
static int receiving = FALSE;


/* Values in internal RAM */
static int ypos = 0; /* location: $01 (R1) */
static int xpos = 0; /* location: $02 (R2) */
static UBYTE last_char = 0; /* location: $04 (R4) */
static int lmargin = 0; /* location: $05 (R5) */
static int rmargin = 0x4f; /* location: $06 (R6) */
static int xscroll = 0; /* location: $1f (RAM bank 1 R7) */
/* 25 pointers to start of data for each line. Originally at locations $20..$38. */
static UBYTE *line_pointers[XEP80_HEIGHT];
static int old_ypos = 0; /* location: $39 */
static int old_xpos = 0; /* location: $3a */
static int list_mode = FALSE; /* location: $3b */
static int escape_mode = FALSE; /* location: $3c */
/* location: $3f */
static int burst_mode = FALSE; /* bit 0 */
static int screen_output = TRUE; /* bit 7; indicates screen/printer */

/* Attribute Latch 0 */
static UBYTE attrib_a = 0xff;
static int font_a_index = 0;
static int font_a_double = FALSE;
static int font_a_blank = FALSE;
static int font_a_blink = FALSE;
/* Attribute Latch 1 */
static UBYTE attrib_b = 0xff;
static int font_b_index = 0;
static int font_b_double = FALSE;
static int font_b_blank = FALSE;
static int font_b_blink = FALSE;

/* TCP */
static int cursor_on = TRUE; /* byte 13 */
static int graphics_mode = FALSE;
static int pal_mode = FALSE;

/* VCR*/
static int blink_reverse = FALSE; /* bit 0 */
static int cursor_blink = FALSE; /* bit 1 */
static int cursor_overwrite = FALSE; /* bit 2 */
static int inverse_mode = FALSE; /* bit 3 */
static int char_set = CHAR_SET_A; /* bits 6-7 */

/* CURS */
static int cursor_x = 0;
static int cursor_y = 0;
static int curs = 0; /* Address of cursor in video RAM, $0000..$1fff */

static UBYTE video_ram[0x2000]; /* 8 KB of RAM */
#define char_data(y, x)	(*(line_pointers[(y)]+(x)))
#define graph_data(y, x) (video_ram[(y)*XEP80_GRAPH_WIDTH/8+(x)])
#define tab_stops(x) (video_ram[0x1900+(x)])

static UBYTE const input_mask[2] = {0x02,0x20};
static UBYTE const output_mask[2] = {0x01,0x10};

UBYTE XEP80_screen_1[XEP80_SCRN_WIDTH*XEP80_MAX_SCRN_HEIGHT];
UBYTE XEP80_screen_2[XEP80_SCRN_WIDTH*XEP80_MAX_SCRN_HEIGHT];

UBYTE (*font)[XEP80_FONTS_CHAR_COUNT][XEP80_MAX_CHAR_HEIGHT][XEP80_CHAR_WIDTH];

/* Path to the XEP80's charset ROM image, marked as U12 on Jerzy Sobola's
   schematic: http://www.dereatari.republika.pl/schematy.htm
   The ROM image is also available there. */
static char charset_filename[FILENAME_MAX];

static void UpdateTVSystem(void)
{
	XEP80_char_height = pal_mode ? XEP80_CHAR_HEIGHT_PAL : XEP80_CHAR_HEIGHT_NTSC;
	XEP80_scrn_height = XEP80_HEIGHT * XEP80_char_height;
#if SUPPORTS_CHANGE_VIDEOMODE
	VIDEOMODE_UpdateXEP80();
#endif
}

/* --------------------------------------
   Functions for blitting display buffer.
   -------------------------------------- */

static void BlitChar(int x, int y, int cur)
{
	int screen_col;
	int font_row, font_col;
	UBYTE *from, *to;
	UBYTE ch;
	UBYTE on, off, blink;
	int font_index, font_double, font_blank, font_blink;
	int blink_rev;
	int last_double_cur = FALSE;

	/* Don't Blit characters that aren't on the screen at the moment. */
	if (x < xscroll || x >= xscroll + XEP80_LINE_LEN)
		return;

	screen_col = x-xscroll;
	ch = char_data(y, x);

	/* Dispaly Atari EOL's as spaces */
	if (ch == XEP80_ATARI_EOL && ((font_a_index & XEP80_FONTS_BLK_FONT_BIT) == 0)
	    && char_set != CHAR_SET_INTERNAL)
		ch = 0x20;

	if (ch & 0x80) {
		font_index = font_b_index;
		font_double = font_b_double;
		font_blank = font_b_blank;
		font_blink = font_b_blink;
	}
	else {
		font_index = font_a_index;
		font_double = font_a_double;
		font_blank = font_a_blank;
		font_blink = font_a_blink;
	}

	if (font_blink && blink_reverse && (font_index & XEP80_FONTS_REV_FONT_BIT))
		blink_rev = TRUE;
	else
		blink_rev = FALSE;

	if (inverse_mode)
		font_index ^= XEP80_FONTS_REV_FONT_BIT;

	if (ch==XEP80_ATARI_EOL) {
		if (inverse_mode)
			font_index |= XEP80_FONTS_REV_FONT_BIT;
		else
			font_index &= ~XEP80_FONTS_REV_FONT_BIT;
	}

	/* Skip the charcter if the last one was a displayed double */
	if (screen_col != 0 && !cur) {
		if (IS_DOUBLE(x-1,y)) {
			int firstd;

			firstd = x-1;
			while (firstd > xscroll) {
				if (!IS_DOUBLE(firstd,y)) {
					firstd++;
					break;
				}
				firstd--;
			}
			if ((x-firstd) % 2)
				return;
		}
	}

	/* Check if we are doing a cursor, and the charcter before is double */
	if (cur) {
		if (screen_col != 0) {
			if (IS_DOUBLE(x-1,y))
				last_double_cur = TRUE;
		}
	}

	if (inverse_mode) {
		on = XEP80_FONTS_offcolor;
		off = XEP80_FONTS_oncolor;
	}
	else {
		on = XEP80_FONTS_oncolor;
		off = XEP80_FONTS_offcolor;
	}

	if (font_index & XEP80_FONTS_REV_FONT_BIT)
		blink = on;
	else
		blink = off;

	if (font_blank) {
		UBYTE color;

		to = &XEP80_screen_1[XEP80_SCRN_WIDTH * XEP80_char_height * y +
		                     screen_col * XEP80_CHAR_WIDTH];
		for (font_row=0;font_row < XEP80_char_height; font_row++) {
			if (cur || (font_index & XEP80_FONTS_REV_FONT_BIT))
				color = on;
			else
				color = off;

			for (font_col=0; font_col < XEP80_CHAR_WIDTH; font_col++) {
				if (font_double)
					*to++ = color;
				*to++ = color;
			}
			if (font_double)
				to += XEP80_SCRN_WIDTH - 2*XEP80_CHAR_WIDTH;
			else
				to += XEP80_SCRN_WIDTH - 1*XEP80_CHAR_WIDTH;
		}

		to = &XEP80_screen_2[XEP80_SCRN_WIDTH * XEP80_char_height * y +
		                     screen_col * XEP80_CHAR_WIDTH];
		for (font_row=0;font_row < XEP80_char_height; font_row++) {
			if ((cur && !cursor_blink) || (font_index & XEP80_FONTS_REV_FONT_BIT))
				color = on;
			else
				color = off;

			for (font_col=0; font_col < XEP80_CHAR_WIDTH; font_col++) {
				if (font_double)
					*to++ = color;
				*to++ = color;
			}
			if (font_double)
				to += XEP80_SCRN_WIDTH - 2*XEP80_CHAR_WIDTH;
			else
				to += XEP80_SCRN_WIDTH - 1*XEP80_CHAR_WIDTH;
		}
	}
	else if (font_double && !cur) {
		int width;

		if (screen_col == 79)
			width = XEP80_CHAR_WIDTH/2;
		else
			width = XEP80_CHAR_WIDTH;

		to = &XEP80_screen_1[XEP80_SCRN_WIDTH * XEP80_char_height * y +
		                     screen_col * XEP80_CHAR_WIDTH];
		for (font_row=0;font_row < XEP80_char_height; font_row++) {
			from = XEP80_FONTS_atari_fonts[char_set][font_index][ch][font_row];

			for (font_col=0; font_col < width; font_col++) {
				*to++ = *from;
				*to++ = *from++;
			}
			to += XEP80_SCRN_WIDTH - 2*XEP80_CHAR_WIDTH;
		}

		to = &XEP80_screen_2[XEP80_SCRN_WIDTH * XEP80_char_height * y +
		                     screen_col * XEP80_CHAR_WIDTH];
		for (font_row=0;font_row < XEP80_char_height; font_row++) {
			if (blink_rev)
				from = XEP80_FONTS_atari_fonts[char_set][font_index ^ XEP80_FONTS_REV_FONT_BIT][ch][font_row];
			else
				from = XEP80_FONTS_atari_fonts[char_set][font_index][ch][font_row];

			for (font_col=0; font_col < width; font_col++) {
				if (font_blink && !cur && !blink_rev) {
					if ((font_index & XEP80_FONTS_UNDER_FONT_BIT) && font_row == XEP80_FONTS_UNDER_ROW) {
						*to++ = *from;
						*to++ = *from++;
					}
					else {
						*to++ = blink;
						*to++ = blink;
						from++;
					}
				}
				else {
					*to++ = *from;
					*to++ = *from++;
				}
			}
			to += XEP80_SCRN_WIDTH - 2*XEP80_CHAR_WIDTH;
		}
	}
	else if ((font_double || last_double_cur) && cur && !cursor_overwrite) {
		int first_half, start_col, end_col;

		/* Determine if this is a double first or second half */
		if (screen_col == 0)
			first_half = TRUE;
		else {
			if (IS_DOUBLE(x-1,y)) {
				int firstd;

				firstd = x-1;
				while (firstd > xscroll) {
					if (!IS_DOUBLE(firstd,y)) {
						firstd++;
						break;
					}
					firstd--;
				}
				first_half = (((x-firstd) % 2) == 0);
			}
			else
				first_half = TRUE;
		}

		if (first_half) {
			start_col = 0;
			end_col = 3;
		}
		else {
			start_col = 3;
			end_col = 6;
			ch = char_data(y, x-1);
		}

		to = &XEP80_screen_1[XEP80_SCRN_WIDTH * XEP80_char_height * y +
		                     screen_col * XEP80_CHAR_WIDTH];
		for (font_row=0;font_row < XEP80_char_height; font_row++) {
			from = XEP80_FONTS_atari_fonts[char_set][font_index ^ XEP80_FONTS_REV_FONT_BIT][ch][font_row] + start_col;
			if (first_half)
				*to++ = *from++;
			for (font_col=start_col; font_col < end_col; font_col++) {
				*to++ = *from;
				*to++ = *from++;
			}
			if (!first_half)
				*to++ = *from;
			to += XEP80_SCRN_WIDTH - XEP80_CHAR_WIDTH;
		}
		to = &XEP80_screen_2[XEP80_SCRN_WIDTH * XEP80_char_height * y +
		                     screen_col * XEP80_CHAR_WIDTH];
		for (font_row=0;font_row < XEP80_char_height; font_row++) {
			if (!cursor_blink)
				from = XEP80_FONTS_atari_fonts[char_set][font_index ^ XEP80_FONTS_REV_FONT_BIT][ch][font_row] + start_col;
			else
				from = XEP80_FONTS_atari_fonts[char_set][font_index][ch][font_row] + start_col;
			if (first_half)
				*to++ = *from++;
			for (font_col=start_col; font_col < end_col; font_col++) {
				*to++ = *from;
				*to++ = *from++;
			}
			if (!first_half)
				*to++ = *from;
			to += XEP80_SCRN_WIDTH - XEP80_CHAR_WIDTH;
		}
	}
	else {
		to = &XEP80_screen_1[XEP80_SCRN_WIDTH * XEP80_char_height * y +
		                     screen_col * XEP80_CHAR_WIDTH];
		if (cur & cursor_overwrite) {
			for (font_row=0;font_row < XEP80_char_height; font_row++) {
				for (font_col=0; font_col < XEP80_CHAR_WIDTH; font_col++)
					*to++ = on;
				to += XEP80_SCRN_WIDTH - XEP80_CHAR_WIDTH;
			}
		}
		else {
			for (font_row=0;font_row < XEP80_char_height; font_row++) {
				if (cur)
					from = XEP80_FONTS_atari_fonts[char_set][font_index ^ XEP80_FONTS_REV_FONT_BIT][ch][font_row];
				else
					from = XEP80_FONTS_atari_fonts[char_set][font_index][ch][font_row];

				for (font_col=0; font_col < XEP80_CHAR_WIDTH; font_col++)
					*to++ = *from++;
				to += XEP80_SCRN_WIDTH - XEP80_CHAR_WIDTH;
			}
		}

		to = &XEP80_screen_2[XEP80_SCRN_WIDTH * XEP80_char_height * y +
		                     screen_col * XEP80_CHAR_WIDTH];
		if (cur & cursor_overwrite) {
			if (cursor_blink) {
				for (font_row=0;font_row < XEP80_char_height; font_row++) {
					for (font_col=0; font_col < XEP80_CHAR_WIDTH; font_col++)
						*to++ = off;
					to += XEP80_SCRN_WIDTH - XEP80_CHAR_WIDTH;
				}
			}
			else {
				for (font_row=0;font_row < XEP80_char_height; font_row++) {
					for (font_col=0; font_col < XEP80_CHAR_WIDTH; font_col++)
						*to++ = on;
					to += XEP80_SCRN_WIDTH - XEP80_CHAR_WIDTH;
				}
			}
		}
		else {
			for (font_row=0;font_row < XEP80_char_height; font_row++) {
				if (cur && !cursor_blink)
					from = XEP80_FONTS_atari_fonts[char_set][font_index ^ XEP80_FONTS_REV_FONT_BIT][ch][font_row];
				else {
					if (blink_rev)
						from = XEP80_FONTS_atari_fonts[char_set][font_index ^ XEP80_FONTS_REV_FONT_BIT][ch][font_row];
					else
						from = XEP80_FONTS_atari_fonts[char_set][font_index][ch][font_row];
				}
				for (font_col=0; font_col < XEP80_CHAR_WIDTH; font_col++) {
					if (font_blink && !cur) {
						if ((font_index & XEP80_FONTS_UNDER_FONT_BIT) && font_row == XEP80_FONTS_UNDER_ROW)
							*to++ = *from++;
						else {
							*to++ = blink;
							from++;
						}
					}
					else
						*to++ = *from++;
				}
				to += XEP80_SCRN_WIDTH - XEP80_CHAR_WIDTH;
			}
		}
	}
}

static void UpdateCursor(void)
{
	if (!graphics_mode && cursor_on) {
		/* Redraw character cursor was at */
		BlitChar(cursor_x, cursor_y, FALSE);
		/* Handle reblitting double wide's which cursor may have overwritten */
		if (cursor_x != 0)
			BlitChar(cursor_x-1, cursor_y, FALSE);
		/* Redraw cursor at new location */
		BlitChar(xpos, ypos, TRUE);
	}
	cursor_x = xpos;
	cursor_y = ypos;
	curs = line_pointers[ypos] + xpos - video_ram;
}

static void BlitCharScreen(void)
{
	int screen_row, screen_col;

	for (screen_row = 0; screen_row < XEP80_HEIGHT; screen_row++) {
		for (screen_col = xscroll; screen_col < xscroll + XEP80_LINE_LEN;
		     screen_col++)
			BlitChar(screen_col, screen_row, FALSE);
	}
	UpdateCursor();
}

static void BlitRows(int y_start, int y_end)
{
	int screen_row, screen_col;

	for (screen_row = y_start; screen_row <= y_end; screen_row++) {
		for (screen_col = xscroll; screen_col < xscroll + XEP80_LINE_LEN;
		     screen_col++)
			BlitChar(screen_col, screen_row, FALSE);
	}
}

static void BlitGraphChar(int x, int y)
{
	int graph_col;
	UBYTE *to1,*to2;
	UBYTE ch;
	UBYTE on, off;

	if (inverse_mode) {
		on = XEP80_FONTS_offcolor;
		off = XEP80_FONTS_oncolor;
	}
	else {
		on = XEP80_FONTS_oncolor;
		off = XEP80_FONTS_offcolor;
	}

	ch = graph_data(y, x);

	to1 = &XEP80_screen_1[XEP80_SCRN_WIDTH * (y + GRAPH_Y_OFFSET)
	                      + x * 8 + GRAPH_X_OFFSET];
	to2 = &XEP80_screen_2[XEP80_SCRN_WIDTH * (y + GRAPH_Y_OFFSET)
	                      + x * 8 + GRAPH_X_OFFSET];

	for (graph_col=0; graph_col < 8; graph_col++) {
		if (ch & (1<<graph_col)) {
			*to1++ = on;
			*to2++ = on;
		}
		else {
			*to1++ = off;
			*to2++ = off;
		}
	}
}

static void BlitGraphScreen(void)
{
	int x, y;

	memset(XEP80_screen_1, XEP80_FONTS_offcolor, XEP80_SCRN_WIDTH*XEP80_MAX_SCRN_HEIGHT);
	memset(XEP80_screen_2, XEP80_FONTS_offcolor, XEP80_SCRN_WIDTH*XEP80_MAX_SCRN_HEIGHT);
	for (x=0; x<XEP80_GRAPH_WIDTH/8; x++)
		for (y=0; y<XEP80_GRAPH_HEIGHT; y++)
			BlitGraphChar(x,y);
}

static void BlitScreen(void)
{
	if (graphics_mode)
		BlitGraphScreen();
	else
		BlitCharScreen();
}

/* --------------------------------------------------
   Functions that simulate procedures in XEP80's ROM.
   -------------------------------------------------- */

/* Set whole 8 KB of video RAM to C.
   ROM location: 0643 */
static void FillMem(UBYTE c)
{
	memset(video_ram,c,0x2000);
}
/* Initialise the XEP80.
   ROM location: 001f, 0056 */
static void ColdStart(void)
{
	static UBYTE const initial_tab_stops[0x100] =
		{0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,
		 0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,
		 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,
		 0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,
		 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,
		 0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,
		 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,
		 0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,
		 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,
		 0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,
		 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,
		 0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,
		 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1};

	/* Set VCR. */
	cursor_blink = FALSE;
	cursor_overwrite = FALSE;
	blink_reverse = FALSE;
	inverse_mode = FALSE;
	char_set = CHAR_SET_A;

	attrib_a = 0xff;
	font_a_index = 0;
	font_a_double = FALSE;
	font_a_blank = FALSE;
	font_a_blink = FALSE;
	attrib_b = 0xff;
	font_b_index = 0;
	font_b_double = FALSE;
	font_b_blank = FALSE;
	font_b_blink = FALSE;
	cursor_x-= cursor_y = 0;
	curs = 0;

	/* Set TCP. */
	graphics_mode = FALSE;
	pal_mode = FALSE;
	cursor_on = TRUE;
	UpdateTVSystem();

	/* Set RAM registers. */
	burst_mode = FALSE;
	screen_output = TRUE;
	escape_mode = FALSE;
	list_mode = FALSE;
	old_ypos = 0xff;
	old_xpos = 0xff;
	input_count = 0;
	xscroll = 0;
	last_char = 0;
	xpos = 0;
	lmargin = 0;
	rmargin = 0x4f;

	{
		int i;
		for (i = 0; i < XEP80_HEIGHT; ++i)
			line_pointers[i] = video_ram + 0x100*i;
	}
	ypos = 0;

	FillMem(XEP80_ATARI_EOL);
	memcpy(&video_ram[0x1900], initial_tab_stops, 0x100);

	BlitCharScreen();
}

/* Put a word to send back to host in the queue. */
static void InputWord(int word)
{
	input_queue[input_count] = word;
/*	Log_print("XEP80 -> %03x [i]", word, input_count);*/
	input_count++;
}

/* Checks if cursor position changed and sends it back to host.
   ROM location: 0200 */
static void	SendCursorStatus(void)
{
	if (xpos != old_xpos || ypos == old_ypos) {
		/* Send X cursor position if it changed, or if both X and Y dind't change. */
		int pos = xpos > 0x4f ? 0x150 : (xpos | 0x100);
		if (ypos != old_ypos) /* Indicate Y position will follow */
			pos |= 0x80;
		InputWord(pos);
		old_xpos = xpos;
	}
	if (ypos != old_ypos) {
		/* Send Y position if it changed. */
		InputWord(ypos | 0x1e0);
		old_ypos = ypos;
	}
}

/* Scrolls the screen down starting at line Y.
   ROM location: 053d */
static void ScrollDown(int y)
{
	UBYTE *ptr = line_pointers[XEP80_HEIGHT-2];

	memmove(line_pointers+y+1, line_pointers+y, sizeof(UBYTE*) * (XEP80_HEIGHT-2-y));
	line_pointers[y] = ptr;
}

/* Fills line Y with EOL. */
static void ClearLine(int y)
{
	memset(line_pointers[y]+xscroll, XEP80_ATARI_EOL, XEP80_LINE_LEN);
}

/* Clears (fills with EOL) the current line and moves cursor to left margin.
   ROM location: 06b6 */
static void ClearLineCursorToLeftMargin(void)
{
	ClearLine(ypos);
	xpos = lmargin;
}

/* Scrolls the whole screen up, and clears the last line. */
static void ScrollScreenUp(void)
{
	UBYTE *ptr = line_pointers[0];

	memmove(line_pointers, line_pointers+1, sizeof(UBYTE*) * (XEP80_HEIGHT-2));
	line_pointers[XEP80_HEIGHT-2] = ptr;
	ClearLine(XEP80_HEIGHT-2);
}

/* Sreolls the whole screen up, clears the last line, and moves cursor to left
   margin of the last line.
   ROM location: 0652 */
static void ScrollScreenUpCursorToLeftMargin(void)
{
	ScrollScreenUp();
	ypos = XEP80_HEIGHT-2;
	xpos = lmargin;

	BlitScreen();
}

/* Process the "Insert Line" ATASCII character.
   ROM location: 0537 */
static void InsertLine(void)
{
	ScrollDown(ypos);
	ClearLineCursorToLeftMargin();
	BlitRows(ypos, XEP80_HEIGHT-2);
	UpdateCursor();
}

/* Advance the cursor right. If necessary, scroll the screen or extend logical line.
   ROM location: 05cb */
static void AdvanceCursor(UBYTE prev_char_under_cursor)
{
	if (xpos != rmargin) {
		++xpos;
		UpdateCursor();
		return;
	}
	if (ypos == 23) { /* last non-status line */
		ScrollScreenUpCursorToLeftMargin();
		UpdateCursor();
		return;
	}
	if (ypos == 24) {
		xpos = 0;
		UpdateCursor();
		return;
	}
	++ypos;
	if (prev_char_under_cursor == XEP80_ATARI_EOL) {
		InsertLine();
		return;
	}
	xpos = 0;
	UpdateCursor();
}

/* Add ATASCII character BYTE at cursor position, and advance the cursor.
   ROM location: 05c3 */
static void AddCharAtCursor(UBYTE byte)
{
	UBYTE prev_char = video_ram[curs];
	video_ram[curs] = byte;
	BlitChar(xpos, ypos, FALSE);
	escape_mode = FALSE;
	AdvanceCursor(prev_char);
}

/* Process the "Cursor Up" ATASCII character.
   ROM location: 0523 */
static void CursorUp(void)
{
	if (--ypos < 0)
		ypos = XEP80_HEIGHT-2;
	UpdateCursor();
}

/* Process the "Cursor Down" ATASCII character.
   ROM location: 052d */
static void CursorDown(void)
{
	if (++ypos > XEP80_HEIGHT-2)
		ypos = 0;
	UpdateCursor();
}

/* Process the "Cursor Left" ATASCII character.
   ROM location: 0552 */
static void CursorLeft(void)
{
	if (xpos == lmargin)
		xpos = rmargin;
	else
		--xpos;
	UpdateCursor();
}

/* Process the "Cursor Right" ATASCII character.
   ROM location: 055c */
static void CursorRight(void)
{
	if (xpos == rmargin)
		xpos = lmargin;
	else {
		if (video_ram[curs] == XEP80_ATARI_EOL)
			video_ram[curs] = 0x20;
		++xpos;
	}
	UpdateCursor();
}

/* Process the "Clear Screen" ATASCII character.
   ROM location: 056f */
static void	ClearScreen(void)
{
	int y;

	for (y=0; y<XEP80_HEIGHT-1; y++)
		memset(line_pointers[y] + xscroll, XEP80_ATARI_EOL, XEP80_LINE_LEN);
	xpos = lmargin;
	ypos = 0;
	BlitCharScreen();
}

/* Process the "Backspace" ATASCII character.
   ROM location: 057a */
static void Backspace(void)
{
	if (xpos != lmargin) {
		--xpos;
	}
	else if (ypos != 0 && char_data(ypos-1, rmargin) != XEP80_ATARI_EOL){
		xpos= rmargin;
		--ypos;
	}
	char_data(ypos, xpos) = 0x20;
	BlitChar(xpos, ypos, FALSE);
	UpdateCursor();
}

/* Shift contents of line Y left, from position X to right margin.
   Put the character PREV_DROPPED at the line's end. Return character that
   was dropped out of the shifted line.
   ROM location: 07a7 */
static UBYTE ShiftLeft(int y, int x, UBYTE prev_dropped)
{
	UBYTE new_dropped = *(line_pointers[y]+x);
	memmove(line_pointers[y]+x, line_pointers[y]+x+1, rmargin-x);
	*(line_pointers[y]+rmargin) = prev_dropped;
	return new_dropped;
}

/* Process the "Delete Character" ATASCII character.
   ROM location: 0760 */
static void DeleteChar(void)
{
	UBYTE prev_dropped = XEP80_ATARI_EOL;
	int y_end;
	int y;

	/* Go down; find a line with EOL at end. */
	for (y_end = ypos; char_data(y_end, rmargin) != XEP80_ATARI_EOL && y_end < XEP80_HEIGHT-2; ++y_end);

	for (y = y_end; y > ypos; --y) {
		if (lmargin == rmargin) {
			video_ram[curs] = prev_dropped;
			BlitRows(ypos, y_end);
			UpdateCursor();
			return;
		}
		else {
			prev_dropped = ShiftLeft(y, lmargin, prev_dropped);
		}
	}

	if (xpos == rmargin) {
		video_ram[curs] = prev_dropped;
	}
	else {
		ShiftLeft(y, xpos, prev_dropped);
	}
	BlitRows(ypos, y_end);
	UpdateCursor();
}

/* Process the "Insert Character" ATASCII character.
   ROM location: 047a */
static void InsertChar(void)
{
	UBYTE prev_dropped = 0x20;
	int y = ypos;
	int x = xpos;
	UBYTE to_drop;
	UBYTE new_last_char;

	for (;;) {
		if (y == rmargin) {
			to_drop = video_ram[curs];
			video_ram[curs] = prev_dropped;
			new_last_char = prev_dropped;
		}
		else {
			to_drop = *(line_pointers[y]+rmargin);
			memmove(line_pointers[y]+x+1, line_pointers[y]+x, rmargin-x);
			*(line_pointers[y]+x) = prev_dropped;
			new_last_char = *(line_pointers[y]+rmargin);
		}
		prev_dropped = to_drop;
		if (to_drop == XEP80_ATARI_EOL) {
			if (new_last_char == XEP80_ATARI_EOL)
				break;

			/* Need to extend logical line. */
			if (y == XEP80_HEIGHT - 2) {
				/* Finished in the last line */
				if (ypos == 0)
					break;
				ScrollScreenUp();
				--ypos;
				BlitCharScreen();
				UpdateCursor();
				return;
			}
			else {
				ScrollDown(y+1);
				ClearLine(y+1);
				BlitRows(ypos, XEP80_HEIGHT-2);
				UpdateCursor();
				return;
			}
		}
		if (y == XEP80_HEIGHT-2)
			break;
		++y;
		x = lmargin;
	}
	BlitRows(ypos, y);
	UpdateCursor();
}

/* Process the "Tab" ATASCII character.
   ROM location: 05a7 */
static void GoToNextTab(void)
{
	for (;;) {
		UBYTE prev = video_ram[curs];
		if (prev == XEP80_ATARI_EOL)
			video_ram[curs] = 0x20;
		AdvanceCursor(prev);
		if (xpos == rmargin)
			return;
		if (tab_stops(xpos))
			return;
	}
}

/* Process the "EOL" ATASCII character.
   ROM location: 0253 */
static void AddEOL(void)
{
	xpos = rmargin;
	escape_mode = FALSE;
	AdvanceCursor(0);
}

/* Process the "Delete Line" ATASCII character.
   ROM location: 07b4 */
static void DeleteLogicalLine(void)
{
	if (ypos == XEP80_HEIGHT - 2) {
		ClearLineCursorToLeftMargin();
	}
	else {
		for (;;) {
			UBYTE prev = char_data(ypos, rmargin);
			UBYTE *ptr = line_pointers[ypos];
			memmove(line_pointers+ypos, line_pointers+ypos+1, sizeof(UBYTE*) * (XEP80_HEIGHT-2-ypos));
			line_pointers[XEP80_HEIGHT - 2] = ptr;
			/* Clear last line */
			memset(ptr+xscroll, XEP80_ATARI_EOL, XEP80_LINE_LEN);
			if (prev == XEP80_ATARI_EOL)
				break;
		}
		xpos = lmargin;
	}
	BlitRows(ypos, XEP80_HEIGHT-2);
	UpdateCursor();
}

/* Reverses bit order of a byte. Bytes have to be reversed in graphics mode,
   because NS405 displays graphics from LSB (left) to MSB (right).
   ROM location: 02d7 */
static UBYTE ReverseByte(unsigned long int b)
{
	return ((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;
}

/* Puts a byte of grsphics in RAM (reversing its bit order) and displays it.
   ROM location: 0296 */
static void AddGraphCharAtCursor(UBYTE byte)
{
	int y = (curs & 0x1fff) / (XEP80_GRAPH_WIDTH/8);
	video_ram[curs & 0x1fff] = ReverseByte(byte);

	if (y < XEP80_GRAPH_HEIGHT) {
		BlitGraphChar((curs & 0x1fff) % (XEP80_GRAPH_WIDTH/8), y);
	}
	curs = (curs+1) & 0xffff;
}

/* Process receiving of a character (ie. not a command).
   ROM location: around 024b */
static void ReceiveChar(UBYTE byte)
{
	if (!screen_output) {
		/* Printer characters are thrown away, handled elsewhere.  The
		 * XEP80 driver needs to be set up to send printer characters
		 * to the existing P: driver. */
	}
	else if (graphics_mode)
		AddGraphCharAtCursor(byte);
	else if (byte == XEP80_ATARI_EOL)
		AddEOL();
	else if (escape_mode || list_mode)
		AddCharAtCursor(byte);
	else if (byte == 0x1b) /* Escape - Print next char even if a control char */
		escape_mode = TRUE;
	else if (ypos == 24) {
		if (byte == 0x9c) { /* Delete Line */
			/* ROM location: 07df */
			ClearLineCursorToLeftMargin();
			BlitRows(ypos, XEP80_HEIGHT-2);
			UpdateCursor();
		}
		else
			AddCharAtCursor(byte);
	}
	else {
		switch(byte) {
		case 0x1c: /* Cursor Up */
			CursorUp();
			break;
		case 0x1d: /* Cursor Down */
			CursorDown();
			break;
		case 0x1e: /* Cursor Left */
			CursorLeft();
			break;
		case 0x1f: /* Cursor Right */
			CursorRight();
			break;
		case 0x7d: /* Clear Screen */
			ClearScreen();
			break;
		case 0x7e: /* Backspace */
			Backspace();
			break;
		case 0x7f: /* Tab */
			GoToNextTab();
			break;
		case 0x9c: /* Delete Line */
			DeleteLogicalLine();
			break;
		case 0x9d: /* Insert Line */
			InsertLine();
			break;
		case 0x9e: /* Clear tab */
			tab_stops(xpos) = FALSE;
			break;
		case 0x9f: /* Set Tab */
			tab_stops(xpos) = TRUE;
			break;
		case 0xfd: /* Sound Bell */
			/* Do nothing here */
			break;
		case 0xfe: /* Delete Char */
			DeleteChar();
			break;
		case 0xff: /* Insert Char */
			InsertChar();
			break;
		default:
			AddCharAtCursor(byte);
			break;
		}
	}
}

/* Process commands:
   100****** - Horizontal Cursor Position ($00-3F)
   10100**** - Horizontal Cursor Position ($40-4F)
   ROM location: 0311 */
static void SetXCur(UBYTE new_xpos)
{
	xpos = old_xpos = new_xpos;
	UpdateCursor();
}

/* Process command 10101**** - Horiz Curs Pos High Nibble - for wide screen
   ROM location: 0318 */
static void SetXCurHigh(UBYTE new_xcur)
{
	xpos = old_xpos = ((UBYTE)xpos & 0x0f) | ((new_xcur & 0x0f) << 4);
	UpdateCursor();
}

/* Process command 10110**** - Left Margin Low Nibble - sets high nibble to 00
   ROM location: 0320 */
static void SetLeftMarginLow(UBYTE margin)
{
	lmargin = margin & 0x0f;
}

/* Process command 10111**** - Left Margin High Nibble
   ROM location: 0325 */
static void SetLeftMarginHigh(UBYTE margin)
{
	lmargin = ((UBYTE)lmargin & 0x0f) | ((margin & 0x0f) << 4);
}

/* Process commands:
   11000**** - Vertical Cursor Position ($00-0F)
   110010*** - Vertical Cursor Position ($10-17)
   110011000 - Set Cursor to Status Row ($18) See caution, Pg. 6
   ROM location: 032d, 03fb */
static void SetYCur(UBYTE new_ypos)
{
	ypos = old_ypos = new_ypos;
	UpdateCursor();
}

/* Process command 110011001 - Set Graphics to 60 Hz
   ROM location: 02b1 */
static void SetGraphics60Hz(void)
{
	graphics_mode = TRUE;

	blink_reverse = FALSE;
	cursor_blink = FALSE;
	cursor_overwrite = FALSE;
	inverse_mode = FALSE;

	cursor_x = 0;
	cursor_y = 0;
	curs = 0;

	pal_mode = FALSE;

	UpdateTVSystem();
	BlitGraphScreen();
}

/* Process command 110011010 - Modify Graphics to 50 Hz
   ROM location: 02cd */
static void SetGraphics50Hz(void)
{
	pal_mode = TRUE;
	UpdateTVSystem();
	BlitGraphScreen();
}

/* Process command 11010**** - Right Margin Low Nibble - sets high nibble to 04
   ROM location: 033a */
static void SetRightMarginLow(UBYTE margin)
{
	rmargin = margin ^ 0xe0;
}

/* Process command 11011**** - Right Margin High Nibble
   ROM location: 033f */
static void SetRightMarginHigh(UBYTE margin)
{
	rmargin = ((UBYTE)rmargin & 0x0f) | ((margin & 0x0f) << 4);
}

/* Process command 111000000 - Get Character from XEP80 at cursor (and advance)
   ROM location: 034c */
static void GetChar(void)
{
	InputWord(video_ram[curs]);
	AdvanceCursor(0x00);
	SendCursorStatus();
}

/* Process command 111000001 - Request Horizontal Cursor
   ROM location: 0613 */
static void GetXCur(void)
{
	InputWord(xpos);
}

/* Process command 111000010 - Master Reset
   ROM location: 0615 */
static void MasterReset(void)
{
	ColdStart();
	InputWord(0x01);
}

/* Process command 111000011 - Printer Port Status
   ROM location: 0631 */
static void GetPrinterStatus(void)
{
	/* Printer port is currently not emulated. */
	InputWord(0x01);
}

/* Process commands
   111010000 - Clear List Flag
   111010001 - Set List Flag
   ROM location: 0376 */
static void SetList(int list)
{
	list_mode = list;
}

/* Process commands
   111010010 - Set Screen Normal Mode - cursor returned each char
   111010011 - Set Screen Burst Mode - no cursor returned
   111011101 - Set Printer Output
   ROM location: 03e1, 03e7 */
static void SetOutputDevice(int screen, int burst)
{
	screen_output = screen;
	burst_mode = burst;
}

/* Process commands
   111010100 - Select Character Set A - Atari graphics (ATASCII)
   111010101 - Select Character Set B - Atari international
   111010110 - Select XEP80 Internal Character Set
   ROM location: 037c, 039b */
static void SetCharSet(int set)
{
	char_set = set;
}

/* Process command 111010111 - Modify Text to 50 Hz Operation
   ROM location: 03a6 */
static void SetText50Hz(void)
{
	pal_mode = TRUE;
	UpdateTVSystem();
	BlitCharScreen();
}

/* Process commands
   111011000 - Cursor Off
   111011001 - Cursor On Continuous
   111011010 - Cursor On Blink
   ROM location: 03af, 03b5 */
static void SetCursorMode(int on, int blink)
{
	cursor_on = on;
	cursor_blink = blink;
	if (!graphics_mode) {
		if (!cursor_on)
			BlitChar(xpos, ypos, FALSE);
		else
			UpdateCursor();
	}
}

/* Process command 111011011 - Move Cursor to Start of Logical Line
   ROM location: 03c5 */
static void SetXCurStart(void)
{
	for (;;) {
		if (ypos == 0)
			break;
		--ypos;
		if (char_data(ypos, rmargin) == XEP80_ATARI_EOL) {
			++ypos;
			break;
		}
	}
	UpdateCursor();
}

/* Process command 111011100 - Set Scroll Window to Cursor X Value
   ROM location: 03dc */
static void SetScrollWindow(void)
{
	xscroll = xpos;
	BlitScreen();
}

/* Process commands
   111011110 - Select White Characters on Black Background
   111011111 - Select Black Characters on White Background
   ROM location: 03ed */
static void SetInverse(int inverse)
{
	inverse_mode = inverse;
	BlitScreen();
}

/* Process command 111101101 - Reserved
   ROM location: 0432 */
static void SetVideoCtrl(UBYTE video_ctrl)
{
	if (video_ctrl & 0x08)
		inverse_mode = TRUE;
	else
		inverse_mode = FALSE;
	if (video_ctrl & 0x02)
		cursor_blink = FALSE;
	else
		cursor_blink = TRUE;
	if (video_ctrl & 0x04)
		cursor_overwrite = FALSE;
	else
		cursor_overwrite = TRUE;
	if (video_ctrl & 0x01)
		blink_reverse = TRUE;
	else
		blink_reverse = FALSE;
	BlitScreen();
}

static void UpdateAttributeBits(UBYTE attrib, int *font_index, int *font_double, int *font_blank, int *font_blink)
{
	*font_index = 0;
	if (!(attrib & 0x01))
		*font_index |= XEP80_FONTS_REV_FONT_BIT;
	if (!(attrib & 0x20))
		*font_index |= XEP80_FONTS_UNDER_FONT_BIT;
	if (!(attrib & 0x80))
		*font_index |= XEP80_FONTS_BLK_FONT_BIT;
	if (!(attrib & 0x10))
		*font_double = TRUE;
	else
		*font_double = FALSE;
	if (!(attrib & 0x40))
		*font_blank = TRUE;
	else
		*font_blank = FALSE;
	if (!(attrib & 0x04))
		*font_blink = TRUE;
	else
		*font_blink = FALSE;
}

/* Process command 111110100 - Reserved
   ROM location: 044d */
static void SetAttributeA(UBYTE attrib)
{
	attrib_a = attrib;
	UpdateAttributeBits(attrib, &font_a_index, &font_a_double, &font_a_blank, &font_a_blink);
	BlitScreen();
}

/* Process command 111110101 - Reserved
   ROM location: 0450 */
static void SetAttributeB(UBYTE attrib)
{
	attrib_b = attrib;
	UpdateAttributeBits(attrib, &font_b_index, &font_b_double, &font_b_blank, &font_b_blink);
	BlitScreen();
}

/* Process 1111xxxxx "Reserved" commands. In reality they set values of various
   internal NS405 registers.
   ROM location: 03ff */
static void SetReserved(UBYTE byte)
{
	byte &= 0x1f;
	if (byte == 0)
		return;
	switch (byte) {
	case CMD_VIDEO_CTRL & 0x1f:
		SetVideoCtrl(last_char);
		break;
	case CMD_ATTRIB_A & 0x1f:
		SetAttributeA(last_char);
		break;
	case CMD_ATTRIB_B & 0x1f:
		SetAttributeB(last_char);
		break;
	default:
		/* Other 1111xxxxx reserved commands are not currently emulated -
		   implementation would require exact emulation of the whole NS405.
		   111100001, 111100010: Set CURS
		   111100011: Put character under CURS
		   111100100, 111100101: Put byte into internal RAM
		   111100110, 111100111: Set HOME
		   111101000: Set MASK
		   111101001: Set PSW
		   111101010: Set PORT
		   111101011: Set TIMER
		   111101100: Set SCR
		   111101110, 111101111: Set BEGD
		   111110000, 111110001: Set ENDD
		   111110010, 111110011: Set SROW
		   111110110: Set TCP
		   111110111: Put byte under TCP
		   111111000: Set VINT
		   111111001, 111111010: Set PSR/BAUD
		   111111011: Set UCR
		   111111100: Set UMX
		   111111101: Set XMTR
		   111111110: Ignore
		   111111111: Strobe the parallel port */
		Log_print("XEP80 received not emulated command %03h", 0x100 & byte);
	}
	last_char = byte - 0x1f;
}

/* Process a word received from host. */
static void OutputWord(int word)
{
	UBYTE byte = word & 0xFF;

	/* Is it a command or data word? */
	if (word & 0x100) {
		switch(byte & CMD_XX_MASK) {
		case CMD_00:
			SetXCur(byte);
			break;
		case CMD_01:
			switch(byte & CMD_01_MASK) {
			case CMD_X_CUR_UPPER:
				SetXCur(byte);
				break;
			case CMD_X_CUR_HIGH:
				SetXCurHigh(byte);
				break;
			case CMD_LEFT_MAR_L:
				SetLeftMarginLow(byte);
				break;
			case CMD_LEFT_MAR_H:
				SetLeftMarginHigh(byte);
				break;
			}
			break;
		case CMD_10:
			switch(byte & CMD_10_MASK) {
			case CMD_Y_CUR_LOW:
				SetYCur(byte & 0x0F);
				break;
			case CMD_1001:
				if ((byte & CMD_1001_MASK) == CMD_Y_CUR_HIGH)
					SetYCur(byte & 0x17);
				else {
					switch(byte) {
					case CMD_Y_CUR_STATUS:
						SetYCur(24);
						break;
					case CMD_GRAPH_50HZ:
						SetGraphics50Hz();
						break;
					case CMD_GRAPH_60HZ:
						SetGraphics60Hz();
						break;
					}
				}
				break;
			case CMD_RIGHT_MAR_L:
				SetRightMarginLow(byte);
				break;
			case CMD_RIGHT_MAR_H:
				SetRightMarginHigh(byte);
				break;
			}
			break;
		case CMD_11:
			if ((byte & 0xe0) == 0xe0) {
				SetReserved(byte);
			}
			else {
				switch (byte) {
				case CMD_GET_CHAR:
					GetChar();
					break;
				case CMD_REQ_X_CUR:
					GetXCur();
					break;
				case CMD_MRST:
					MasterReset();
					break;
				case CMD_PRT_STAT:
					GetPrinterStatus();
					break;
				case CMD_FILL_PREV:
					/* Reverts bits in the last char. For use in graphics mode.
					   ROM location: 0636 */
					FillMem(ReverseByte(last_char));
					BlitScreen();
					InputWord(0x01);
					break;
				case CMD_FILL_SPACE:
					/* ROM location: 063d */
					FillMem(0x20);
					BlitScreen();
					InputWord(0x01);
					break;
				case CMD_FILL_EOL:
					/* ROM location: 0641 */
					FillMem(XEP80_ATARI_EOL);
					BlitScreen();
					InputWord(0x01);
					break;
				case CMD_CLR_LIST:
					SetList(FALSE);
					break;
				case CMD_SET_LIST:
					SetList(TRUE);
					break;
				case CMD_SCR_NORMAL:
					SetOutputDevice(TRUE, FALSE);
					break;
				case CMD_SCR_BURST:
					SetOutputDevice(TRUE, TRUE);
					break;
				case CMD_SET_PRINT:
					SetOutputDevice(FALSE, TRUE);
					break;
				case CMD_CHAR_SET_A:
					SetCharSet(CHAR_SET_A);
					BlitScreen();
					break;
				case CMD_CHAR_SET_B:
					SetCharSet(CHAR_SET_B);
					BlitScreen();
					break;
				case CMD_CHAR_SET_INT:
					SetCharSet(CHAR_SET_INTERNAL);
					BlitScreen();
					break;
				case CMD_TEXT_50HZ:
					SetText50Hz();
					break;
				case CMD_CUR_OFF:
					SetCursorMode(FALSE, FALSE);
					break;
				case CMD_CUR_ON:
					SetCursorMode(TRUE, FALSE);
					break;
				case CMD_CUR_BLINK:
					SetCursorMode(TRUE, TRUE);
					break;
				case CMD_CUR_ST_LINE:
					SetXCurStart();
					break;
				case CMD_SET_SCRL_WIN:
					SetScrollWindow();
					break;
				case CMD_WHT_ON_BLK:
					SetInverse(FALSE);
					break;
				case CMD_BLK_ON_WHT:
					SetInverse(TRUE);
					break;
				default:
					/* All command left are 111000111 and 111001xxx, marked as Reserved.
					   Actually they return values of various internal NS405 registers.
					   Not currently emulated - implementation would require exact
					   emulation of the whole NS405.
					   111000111: Get byte at CURS
					   111001000: Get INTR
					   111001001: Get PSW
					   111001010: Get PORT
					   111001011: Get TIMER
					   111001100: Get HPEN
					   111001101: Get VPEN
					   111001110: Get STAT
					   111001111: Get RCVR */
					Log_print("XEP80 received not emulated command %03h", word);
				}
			}
			break;
		}
	}
	/* If it's data, then handle it as a character */
	else {
		last_char = byte;
		ReceiveChar(byte);
		if (!burst_mode)
			SendCursorStatus();
	}
}

/* ----------------
   Other functions.
   ---------------- */

void XEP80_ChangeColors(void)
{
	BlitScreen();
}

int XEP80_ReadConfig(char *string, char *ptr)
{
	if (strcmp(string, "XEP80_CHARSET") == 0)
		Util_strlcpy(charset_filename, ptr, sizeof(charset_filename));
	else return FALSE; /* no match */
	return TRUE; /* matched something */
}

void XEP80_WriteConfig(FILE *fp)
{
	fprintf(fp, "XEP80_CHARSET=%s\n", charset_filename);
}

int XEP80_SetEnabled(int value)
{
	if (value && !XEP80_FONTS_inited && !XEP80_FONTS_InitFonts(charset_filename))
		return FALSE;
	XEP80_enabled = value;
	return TRUE;
}

int XEP80_Initialise(int *argc, char *argv[])
{
	int i, j;
	int help_only = FALSE;
	for (i = j = 1; i < *argc; i++) {
		int i_a = (i + 1 < *argc);		/* is argument available? */
		int a_m = FALSE;			/* error, argument missing! */

		if (strcmp(argv[i], "-xep80") == 0) {
			XEP80_enabled = TRUE;
		}
		else if (strcmp(argv[i], "-xep80port") == 0) {
			if (i_a) {
				XEP80_port = Util_sscandec(argv[++i]);
				if (XEP80_port != 0 && XEP80_port != 1) {
					Log_print("Invalid XEP80 port - should be 0 or 1");
					return FALSE;
				}
			}
			else a_m = TRUE;
		}
		else {
			if (strcmp(argv[i], "-help") == 0) {
				help_only = TRUE;
				Log_print("\t-xep80           Emulate the XEP80");
				Log_print("\t-xep80port <n>   Use XEP80 on joystick port <n>");
			}
			argv[j++] = argv[i];
		}

		if (a_m) {
			Log_print("Missing argument for '%s'", argv[i]);
			return FALSE;
		}
	}
	*argc = j;

	if (help_only)
		return TRUE;

	if (XEP80_enabled && !XEP80_SetEnabled(XEP80_enabled)) {
		XEP80_enabled = FALSE;
		Log_print("Couldn't load XEP80 charset image: %s", charset_filename);
		return FALSE;
	}

	start_trans_cpu_clock = ANTIC_CPU_CLOCK;
	ColdStart();

	return TRUE;
}

UBYTE XEP80_GetBit(void)
{
	UBYTE ret = 0xFF;
	int word_bit_num;
	int input_word;
	int input_word_num;
	/* Number of CPU ticks since start of word receiving.
	   TODO: Avoid overflows in this value (minimal issue, since transmission
	   rate is 15.7 kHz - way too low to allow for overflows). */
	int num_ticks = (int)(ANTIC_CPU_CLOCK - start_trans_cpu_clock);

	int bit_no = num_ticks / ANTIC_LINE_C;

	/* If there is not input to be sent, just return */
	if (input_count == 0 || num_ticks < 0)
		return ret;

	/* Figure out which word of the queue it is in based on bit */
	input_word_num = (bit_no / 11);

	/* If it is greater than we have, then clear queue and return */
	if (input_word_num >= input_count) {
		input_count = 0;
		return ret;
	}

	/* Get the word from the queue, and calculate which bit of the
	 * word we are sending */
	input_word = input_queue[input_word_num];
	word_bit_num = bit_no % 11;

	/* Send the return value based on the bit */
	switch(word_bit_num) {
	case 0: /* Start Bit - 0 */
		ret = 0xFF & ~input_mask[XEP80_port];
		break;
	case 1: /* 9 Data Bits */
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		if (input_word & (1 << (word_bit_num-1)))
			ret = 0xFF;
		else
			ret = 0xFF & ~input_mask[XEP80_port];
		break;
	case 10: /* Stop Bit - 1 */
		ret = 0xFF;
		break;
	}

	return ret;
}

void XEP80_PutBit(UBYTE byte)
{
	/* Number of CPU ticks since start of word receiving.
	   TODO: Avoid overflows in this value (minimal issue, since transmission
	   rate is 15.7 kHz - way too low to allow for overflows). */
	int num_ticks = (int)(ANTIC_CPU_CLOCK - start_trans_cpu_clock);

	int bit_no = (num_ticks + ANTIC_LINE_C / 2) / ANTIC_LINE_C;

	byte &= output_mask[XEP80_port];

	if (receiving) {
		switch (bit_no) {
		case 0:
			return;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			if (byte)
				output_word |= 1 << (bit_no - 1);
			break;
		case 10:
			/* Stop bit */
			/* Clear any unread input from last command */
			input_count = 0;
			receiving = FALSE;
			if (byte) {
				/* Set the start position of the next possible output byte, to
				   0.5 scanline after end of the current stop bit. This is not
				   based on actual hardware - the delay was chosen to work with
				   the Atari and the SpartaDOS X XEP80 drivers. Starting
				   transmission immediately after end of the stop bit
				   (prev_cpu_clock += 11 * ANTIC_LINE_C) would break the SDX
				   driver. */
				start_trans_cpu_clock += 11 * ANTIC_LINE_C + ANTIC_LINE_C / 2;
				/* Handle the new word */
/*				Log_print("XEP80 <- %03x", output_word);*/
				OutputWord(output_word);
			}
			return;
		default:
			/* Transmission timed out without receiving stop bit. */
			receiving = FALSE;
		}
	}

	if (!receiving) {
		/* Either previous byte ended or no byte was received yet. */
		if (!byte) {
			/* Start bit encountered. */
			receiving = TRUE;
			start_trans_cpu_clock = ANTIC_CPU_CLOCK;
			output_word = 0;
		}
	}
}

void XEP80_StateSave(void)
{
	StateSav_SaveINT(&XEP80_enabled, 1);
	if (XEP80_enabled) {
		int num_ticks = (int)(ANTIC_CPU_CLOCK - start_trans_cpu_clock);
#if SUPPORTS_CHANGE_VIDEOMODE
		int show_xep80 = VIDEOMODE_80_column;
#else
		int show_xep80 = 1;
#endif /* SUPPORTS_CHANGE_VIDEOMODE */
		StateSav_SaveINT(&XEP80_port, 1);
		StateSav_SaveINT(&show_xep80, 1);
		StateSav_SaveINT(&num_ticks, 1);
		StateSav_SaveINT(&output_word, 1);
		StateSav_SaveINT(&input_count, 1);
		StateSav_SaveINT(&receiving, 1);
		StateSav_SaveUWORD(input_queue, IN_QUEUE_SIZE);
		StateSav_SaveINT(&receiving, 1);
		StateSav_SaveUBYTE(&last_char, 1);

		StateSav_SaveINT(&xpos, 1);
		StateSav_SaveINT(&xscroll, 1);
		StateSav_SaveINT(&ypos, 1);
		StateSav_SaveINT(&cursor_x, 1);
		StateSav_SaveINT(&cursor_y, 1);
		StateSav_SaveINT(&curs, 1);
		StateSav_SaveINT(&old_xpos, 1);
		StateSav_SaveINT(&old_ypos, 1);
		StateSav_SaveINT(&lmargin, 1);
		StateSav_SaveINT(&rmargin, 1);
		StateSav_SaveUBYTE(&attrib_a, 1);
		StateSav_SaveUBYTE(&attrib_b, 1);
		StateSav_SaveINT(&list_mode, 1);
		StateSav_SaveINT(&escape_mode, 1);
		StateSav_SaveINT(&char_set, 1);
		StateSav_SaveINT(&cursor_on, 1);
		StateSav_SaveINT(&cursor_blink, 1);
		StateSav_SaveINT(&cursor_overwrite, 1);
		StateSav_SaveINT(&blink_reverse, 1);
		StateSav_SaveINT(&inverse_mode, 1);
		StateSav_SaveINT(&screen_output, 1);
		StateSav_SaveINT(&burst_mode, 1);
		StateSav_SaveINT(&graphics_mode, 1);
		StateSav_SaveINT(&pal_mode, 1);
		{
			int i;
			for (i = 0; i < XEP80_HEIGHT; ++i) {
				UBYTE ptr = ((int)(line_pointers[i] - video_ram)) / 0x100;
				StateSav_SaveUBYTE(&ptr, 1);
			}
		}
		StateSav_SaveUBYTE(video_ram, 8192);
	}
}

void XEP80_StateRead(void)
{
	int local_xep80_enabled = FALSE;
	int local_show_xep80 = FALSE;

	/* test for end of file */
	StateSav_ReadINT(&local_xep80_enabled, 1);
	if (!XEP80_SetEnabled(local_xep80_enabled))
		XEP80_enabled = FALSE;

	if (local_xep80_enabled) {
		int num_ticks;
		StateSav_ReadINT(&XEP80_port, 1);
		StateSav_ReadINT(&local_show_xep80, 1);
		StateSav_ReadINT(&num_ticks, 1);
		start_trans_cpu_clock = ANTIC_CPU_CLOCK - num_ticks;
		StateSav_ReadINT(&output_word, 1);
		StateSav_ReadINT(&input_count, 1);
		StateSav_ReadINT(&receiving, 1);
		StateSav_ReadUWORD(input_queue, IN_QUEUE_SIZE);
		StateSav_ReadINT(&receiving, 1);
		StateSav_ReadUBYTE(&last_char, 1);

		StateSav_ReadINT(&xpos, 1);
		StateSav_ReadINT(&xscroll, 1);
		StateSav_ReadINT(&ypos, 1);
		StateSav_ReadINT(&cursor_x, 1);
		StateSav_ReadINT(&cursor_y, 1);
		StateSav_ReadINT(&curs, 1);
		StateSav_ReadINT(&old_xpos, 1);
		StateSav_ReadINT(&old_ypos, 1);
		StateSav_ReadINT(&lmargin, 1);
		StateSav_ReadINT(&rmargin, 1);
		StateSav_ReadUBYTE(&attrib_a, 1);
		UpdateAttributeBits(attrib_a, &font_a_index, &font_a_double, &font_a_blank, &font_a_blink);
		StateSav_ReadUBYTE(&attrib_b, 1);
		UpdateAttributeBits(attrib_b, &font_b_index, &font_b_double, &font_b_blank, &font_b_blink);
		StateSav_ReadINT(&list_mode, 1);
		StateSav_ReadINT(&escape_mode, 1);
		StateSav_ReadINT(&char_set, 1);
		StateSav_ReadINT(&cursor_on, 1);
		StateSav_ReadINT(&cursor_blink, 1);
		StateSav_ReadINT(&cursor_overwrite, 1);
		StateSav_ReadINT(&blink_reverse, 1);
		StateSav_ReadINT(&inverse_mode, 1);
		StateSav_ReadINT(&screen_output, 1);
		StateSav_ReadINT(&burst_mode, 1);
		StateSav_ReadINT(&graphics_mode, 1);
		StateSav_ReadINT(&pal_mode, 1);
		{
			int i;
			for (i = 0; i < XEP80_HEIGHT; ++i) {
				UBYTE ptr;
				StateSav_ReadUBYTE(&ptr, 1);
				line_pointers[i] = video_ram + 0x100*ptr;
			}
		}

		StateSav_ReadUBYTE(video_ram, 8192);
		UpdateTVSystem();
		BlitScreen(); /* Clear the old text screen */
	}
#if SUPPORTS_CHANGE_VIDEOMODE
	VIDEOMODE_Set80Column(local_show_xep80);
#endif
}

#endif /* XEP80 */

/*
vim:ts=4:sw=4:
*/
