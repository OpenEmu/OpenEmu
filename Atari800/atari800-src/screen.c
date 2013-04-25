/*
 * screen.c - Atari screen handling
 *
 * Copyright (c) 2001 Robert Golias and Piotr Fusik
 * Copyright (C) 2001-2008 Atari800 development team (see DOC/CREDITS)
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
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_LIBPNG
#include <png.h>
#endif

#include "antic.h"
#include "atari.h"
#include "cassette.h"
#include "colours.h"
#include "log.h"
#include "pia.h"
#include "screen.h"
#include "sio.h"
#include "util.h"

#define ATARI_VISIBLE_WIDTH 336
#define ATARI_LEFT_MARGIN 24

ULONG *Screen_atari = NULL;
#ifdef DIRTYRECT
UBYTE *Screen_dirty = NULL;
#endif
#ifdef BITPL_SCR
ULONG *Screen_atari_b = NULL;
ULONG *Screen_atari1 = NULL;
ULONG *Screen_atari2 = NULL;
#endif

/* The area that can been seen is Screen_visible_x1 <= x < Screen_visible_x2,
   Screen_visible_y1 <= y < Screen_visible_y2.
   Full Atari screen is 336x240. Screen_WIDTH is 384 only because
   the code in antic.c sometimes draws more than 336 bytes in a line.
   Currently Screen_visible variables are used only to place
   disk led and snailmeter in the corners of the screen.
*/
int Screen_visible_x1 = 24;				/* 0 .. Screen_WIDTH */
int Screen_visible_y1 = 0;				/* 0 .. Screen_HEIGHT */
int Screen_visible_x2 = 360;			/* 0 .. Screen_WIDTH */
int Screen_visible_y2 = Screen_HEIGHT;	/* 0 .. Screen_HEIGHT */

int Screen_show_atari_speed = FALSE;
int Screen_show_disk_led = TRUE;
int Screen_show_sector_counter = FALSE;
int Screen_show_1200_leds = TRUE;

#ifdef HAVE_LIBPNG
#define DEFAULT_SCREENSHOT_FILENAME_FORMAT "atari%03d.png"
#else
#define DEFAULT_SCREENSHOT_FILENAME_FORMAT "atari%03d.pcx"
#endif

static char screenshot_filename_format[FILENAME_MAX] = DEFAULT_SCREENSHOT_FILENAME_FORMAT;
static int screenshot_no_max = 1000;

/* converts "foo%bar##.pcx" to "foo%%bar%02d.pcx" */
static void Screen_SetScreenshotFilenamePattern(const char *p)
{
	char *f = screenshot_filename_format;
	char no_width = '0';
	screenshot_no_max = 1;
	/* 9 because sprintf'ed "no" can be 9 digits */
	while (f < screenshot_filename_format + FILENAME_MAX - 9) {
		/* replace a sequence of hashes with e.g. "%05d" */
		if (*p == '#') {
			if (no_width > '0') /* already seen a sequence of hashes */
				break;          /* invalid */
			/* count hashes */
			do {
				screenshot_no_max *= 10;
				p++;
				no_width++;
				/* now no_width is the number of hashes seen so far
				   and p points after the counted hashes */
			} while (no_width < '9' && *p == '#'); /* no more than 9 hashes */
			*f++ = '%';
			*f++ = '0';
			*f++ = no_width;
			*f++ = 'd';
			continue;
		}
		if (*p == '%')
			*f++ = '%'; /* double the percents */
		*f++ = *p;
		if (*p == '\0')
			return; /* ok */
		p++;
	}
	Log_print("Invalid filename pattern for screenshots, using default.");
	strcpy(screenshot_filename_format, DEFAULT_SCREENSHOT_FILENAME_FORMAT);
	screenshot_no_max = 1000;
}

int Screen_Initialise(int *argc, char *argv[])
{
	int i;
	int j;
	int help_only = FALSE;

	for (i = j = 1; i < *argc; i++) {
		int i_a = (i + 1 < *argc);		/* is argument available? */
		int a_m = FALSE;			/* error, argument missing! */
		
		if (strcmp(argv[i], "-screenshots") == 0) {
			if (i_a)
				Screen_SetScreenshotFilenamePattern(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-showspeed") == 0) {
			Screen_show_atari_speed = TRUE;
		}
		else {
			if (strcmp(argv[i], "-help") == 0) {
				help_only = TRUE;
				Log_print("\t-screenshots <p> Set filename pattern for screenshots");
				Log_print("\t-showspeed       Show percentage of actual speed");
			}
			argv[j++] = argv[i];
		}

		if (a_m) {
			Log_print("Missing argument for '%s'", argv[i]);
			return FALSE;
		}
	}
	*argc = j;

	/* don't bother mallocing Screen_atari with just "-help" */
	if (help_only)
		return TRUE;

	if (Screen_atari == NULL) { /* platform-specific code can initialize it in theory */
		Screen_atari = (ULONG *) Util_malloc(Screen_HEIGHT * Screen_WIDTH);
		/* Clear the screen. */
		memset(Screen_atari, 0, Screen_HEIGHT * Screen_WIDTH);
#ifdef DIRTYRECT
		Screen_dirty = (UBYTE *) Util_malloc(Screen_HEIGHT * Screen_WIDTH / 8);
		Screen_EntireDirty();
#endif
#ifdef BITPL_SCR
		Screen_atari_b = (ULONG *) Util_malloc(Screen_HEIGHT * Screen_WIDTH);
		memset(Screen_atari_b, 0, Screen_HEIGHT * Screen_WIDTH);
		Screen_atari1 = Screen_atari;
		Screen_atari2 = Screen_atari_b;
#endif
	}

	return TRUE;
}

int Screen_ReadConfig(char *string, char *ptr)
{
	if (strcmp(string, "SCREEN_SHOW_SPEED") == 0)
		return (Screen_show_atari_speed = Util_sscanbool(ptr)) != -1;
	else if (strcmp(string, "SCREEN_SHOW_IO_ACTIVITY") == 0)
		return (Screen_show_disk_led = Util_sscanbool(ptr)) != -1;
	else if (strcmp(string, "SCREEN_SHOW_IO_COUNTER") == 0)
		return (Screen_show_sector_counter = Util_sscanbool(ptr)) != -1;
	else if (strcmp(string, "SCREEN_SHOW_1200XL_LEDS") == 0)
		return (Screen_show_1200_leds = Util_sscanbool(ptr)) != -1;
	else return FALSE;
	return TRUE;
}

void Screen_WriteConfig(FILE *fp)
{
	fprintf(fp, "SCREEN_SHOW_SPEED=%d\n", Screen_show_atari_speed);
	fprintf(fp, "SCREEN_SHOW_IO_ACTIVITY=%d\n", Screen_show_disk_led);
	fprintf(fp, "SCREEN_SHOW_IO_COUNTER=%d\n", Screen_show_sector_counter);
	fprintf(fp, "SCREEN_SHOW_1200XL_LEDS=%d\n", Screen_show_1200_leds);
}

#define SMALLFONT_WIDTH    5
#define SMALLFONT_HEIGHT   7
#define SMALLFONT_PERCENT  10
#define SMALLFONT_C        11
#define SMALLFONT_D        12
#define SMALLFONT_L        13
#define SMALLFONT_SLASH    14
#define SMALLFONT_____ 0x00
#define SMALLFONT___X_ 0x02
#define SMALLFONT__X__ 0x04
#define SMALLFONT__XX_ 0x06
#define SMALLFONT_X___ 0x08
#define SMALLFONT_X_X_ 0x0A
#define SMALLFONT_XX__ 0x0C
#define SMALLFONT_XXX_ 0x0E

static void SmallFont_DrawChar(UBYTE *screen, int ch, UBYTE color1, UBYTE color2)
{
	static const UBYTE font[15][SMALLFONT_HEIGHT] = {
		{
			SMALLFONT_____,
			SMALLFONT__X__,
			SMALLFONT_X_X_,
			SMALLFONT_X_X_,
			SMALLFONT_X_X_,
			SMALLFONT__X__,
			SMALLFONT_____
		},
		{
			SMALLFONT_____,
			SMALLFONT__X__,
			SMALLFONT_XX__,
			SMALLFONT__X__,
			SMALLFONT__X__,
			SMALLFONT__X__,
			SMALLFONT_____
		},
		{
			SMALLFONT_____,
			SMALLFONT_XX__,
			SMALLFONT___X_,
			SMALLFONT__X__,
			SMALLFONT_X___,
			SMALLFONT_XXX_,
			SMALLFONT_____
		},
		{
			SMALLFONT_____,
			SMALLFONT_XX__,
			SMALLFONT___X_,
			SMALLFONT__X__,
			SMALLFONT___X_,
			SMALLFONT_XX__,
			SMALLFONT_____
		},
		{
			SMALLFONT_____,
			SMALLFONT___X_,
			SMALLFONT__XX_,
			SMALLFONT_X_X_,
			SMALLFONT_XXX_,
			SMALLFONT___X_,
			SMALLFONT_____
		},
		{
			SMALLFONT_____,
			SMALLFONT_XXX_,
			SMALLFONT_X___,
			SMALLFONT_XXX_,
			SMALLFONT___X_,
			SMALLFONT_XX__,
			SMALLFONT_____
		},
		{
			SMALLFONT_____,
			SMALLFONT__X__,
			SMALLFONT_X___,
			SMALLFONT_XX__,
			SMALLFONT_X_X_,
			SMALLFONT__X__,
			SMALLFONT_____
		},
		{
			SMALLFONT_____,
			SMALLFONT_XXX_,
			SMALLFONT___X_,
			SMALLFONT__X__,
			SMALLFONT__X__,
			SMALLFONT__X__,
			SMALLFONT_____
		},
		{
			SMALLFONT_____,
			SMALLFONT__X__,
			SMALLFONT_X_X_,
			SMALLFONT__X__,
			SMALLFONT_X_X_,
			SMALLFONT__X__,
			SMALLFONT_____
		},
		{
			SMALLFONT_____,
			SMALLFONT__X__,
			SMALLFONT_X_X_,
			SMALLFONT__XX_,
			SMALLFONT___X_,
			SMALLFONT__X__,
			SMALLFONT_____
		},
		{
			SMALLFONT_____,
			SMALLFONT_X_X_,
			SMALLFONT___X_,
			SMALLFONT__X__,
			SMALLFONT_X___,
			SMALLFONT_X_X_,
			SMALLFONT_____
		},
		{
			SMALLFONT_____,
			SMALLFONT__X__,
			SMALLFONT_X_X_,
			SMALLFONT_X___,
			SMALLFONT_X_X_,
			SMALLFONT__X__,
			SMALLFONT_____
		},
		{
			SMALLFONT_____,
			SMALLFONT_XX__,
			SMALLFONT_X_X_,
			SMALLFONT_X_X_,
			SMALLFONT_X_X_,
			SMALLFONT_XX__,
			SMALLFONT_____
		},
		{
			SMALLFONT_____,
			SMALLFONT_X___,
			SMALLFONT_X___,
			SMALLFONT_X___,
			SMALLFONT_X___,
			SMALLFONT_XXX_,
			SMALLFONT_____
		},
		{
			SMALLFONT_____,
			SMALLFONT___X_,
			SMALLFONT___X_,
			SMALLFONT__X__,
			SMALLFONT__X__,
			SMALLFONT_X___,
			SMALLFONT_____
		}
	};
	int y;
	for (y = 0; y < SMALLFONT_HEIGHT; y++) {
		int src;
		int mask;
		src = font[ch][y];
		for (mask = 1 << (SMALLFONT_WIDTH - 1); mask != 0; mask >>= 1) {
			ANTIC_VideoPutByte(screen, (UBYTE) ((src & mask) != 0 ? color1 : color2));
			screen++;
		}
		screen += Screen_WIDTH - SMALLFONT_WIDTH;
	}
}

/* Returns screen address for placing the next character on the left of the
   drawn number. */
static UBYTE *SmallFont_DrawInt(UBYTE *screen, int n, UBYTE color1, UBYTE color2)
{
	do {
		SmallFont_DrawChar(screen, n % 10, color1, color2);
		screen -= SMALLFONT_WIDTH;
		n /= 10;
	} while (n > 0);
	return screen;
}

void Screen_DrawAtariSpeed(double cur_time)
{
	if (Screen_show_atari_speed) {
		static int percent_display = 100;
		static int last_updated = 0;
		static double last_time = 0;
		if ((cur_time - last_time) >= 0.5) {
			percent_display = (int) (100 * (Atari800_nframes - last_updated) / (cur_time - last_time) / (Atari800_tv_mode == Atari800_TV_PAL ? 50 : 60));
			last_updated = Atari800_nframes;
			last_time = cur_time;
		}
		/* if (percent_display < 99 || percent_display > 101) */
		{
			/* space for 5 digits - up to 99999% Atari speed */
			UBYTE *screen = (UBYTE *) Screen_atari + Screen_visible_x1 + 5 * SMALLFONT_WIDTH
			          	+ (Screen_visible_y2 - SMALLFONT_HEIGHT) * Screen_WIDTH;
			SmallFont_DrawChar(screen, SMALLFONT_PERCENT, 0x0c, 0x00);
			SmallFont_DrawInt(screen - SMALLFONT_WIDTH, percent_display, 0x0c, 0x00);
		}
	}
}

void Screen_DrawDiskLED(void)
{
	if (SIO_last_op_time > 0) {
		UBYTE *screen;
		if (SIO_last_drive != 0x60)
			SIO_last_op_time--;
		screen = (UBYTE *) Screen_atari + Screen_visible_x2 - SMALLFONT_WIDTH
			+ (Screen_visible_y2 - SMALLFONT_HEIGHT) * Screen_WIDTH;
		if (SIO_last_drive == 0x60 || SIO_last_drive == 0x61) {
			if (CASSETTE_status != CASSETTE_STATUS_NONE) {
				if (Screen_show_disk_led)
					SmallFont_DrawChar(screen, SMALLFONT_C, 0x00, (UBYTE) (CASSETTE_record ? 0x2b : 0xac));

				if (Screen_show_sector_counter) {
					/* Displaying tape length during saving is pointless since it would equal the number
					of the currently-written block, which is already displayed. */
					if (!CASSETTE_record) {
						screen = SmallFont_DrawInt(screen - SMALLFONT_WIDTH, CASSETTE_GetSize(), 0x00, 0x88);
						SmallFont_DrawChar(screen, SMALLFONT_SLASH, 0x00, 0x88);
					}
					SmallFont_DrawInt(screen - SMALLFONT_WIDTH, CASSETTE_GetPosition(), 0x00, 0x88);
				}
			}
		}
		else {
			if (Screen_show_disk_led) {
				SmallFont_DrawChar(screen, SIO_last_drive, 0x00, (UBYTE) (SIO_last_op == SIO_LAST_READ ? 0xac : 0x2b));
				SmallFont_DrawChar(screen -= SMALLFONT_WIDTH, SMALLFONT_D, 0x00, (UBYTE) (SIO_last_op == SIO_LAST_READ ? 0xac : 0x2b));
			}

			if (Screen_show_sector_counter)
				SmallFont_DrawInt(screen - SMALLFONT_WIDTH, SIO_last_sector, 0x00, 0x88);
		}
	}
}

void Screen_Draw1200LED(void)
{
	if (Screen_show_1200_leds && Atari800_keyboard_leds) {
		UBYTE *screen = (UBYTE *) Screen_atari + Screen_visible_x1 + SMALLFONT_WIDTH * 10
			+ (Screen_visible_y2 - SMALLFONT_HEIGHT) * Screen_WIDTH;
		UBYTE portb = PIA_PORTB | PIA_PORTB_mask;
		if ((portb & 0x04) == 0) {
			SmallFont_DrawChar(screen, SMALLFONT_L, 0x00, 0x36);
			SmallFont_DrawChar(screen + SMALLFONT_WIDTH, 1, 0x00, 0x36);
		}
		screen += SMALLFONT_WIDTH * 3;
		if ((portb & 0x08) == 0) {
			SmallFont_DrawChar(screen, SMALLFONT_L, 0x00, 0x36);
			SmallFont_DrawChar(screen + SMALLFONT_WIDTH, 2, 0x00, 0x36);
		}
	}
}

void Screen_FindScreenshotFilename(char *buffer, unsigned bufsize)
{
	static int no = -1;
	static int overwrite = FALSE;

	for (;;) {
		if (++no >= screenshot_no_max) {
			no = 0;
			overwrite = TRUE;
		}
		snprintf(buffer, bufsize, screenshot_filename_format, no);
		if (overwrite)
			break;
		if (!Util_fileexists(buffer))
			break; /* file does not exist - we can create it */
	}
}

static void fputw(int x, FILE *fp)
{
	fputc(x & 0xff, fp);
	fputc(x >> 8, fp);
}

static void Screen_SavePCX(FILE *fp, UBYTE *ptr1, UBYTE *ptr2)
{
	int i;
	int x;
	int y;
	UBYTE plane = 16;	/* 16 = Red, 8 = Green, 0 = Blue */
	UBYTE last;
	UBYTE count;

	fputc(0xa, fp);   /* pcx signature */
	fputc(0x5, fp);   /* version 5 */
	fputc(0x1, fp);   /* RLE encoding */
	fputc(0x8, fp);   /* bits per pixel */
	fputw(0, fp);     /* XMin */
	fputw(0, fp);     /* YMin */
	fputw(ATARI_VISIBLE_WIDTH - 1, fp); /* XMax */
	fputw(Screen_HEIGHT - 1, fp);        /* YMax */
	fputw(0, fp);     /* HRes */
	fputw(0, fp);     /* VRes */
	for (i = 0; i < 48; i++)
		fputc(0, fp); /* EGA color palette */
	fputc(0, fp);     /* reserved */
	fputc(ptr2 != NULL ? 3 : 1, fp); /* number of bit planes */
	fputw(ATARI_VISIBLE_WIDTH, fp);  /* number of bytes per scan line per color plane */
	fputw(1, fp);     /* palette info */
	fputw(ATARI_VISIBLE_WIDTH, fp); /* screen resolution */
	fputw(Screen_HEIGHT, fp);
	for (i = 0; i < 54; i++)
		fputc(0, fp);  /* unused */

	for (y = 0; y < Screen_HEIGHT; ) {
		x = 0;
		do {
			last = ptr2 != NULL ? (((Colours_table[*ptr1] >> plane) & 0xff) + ((Colours_table[*ptr2] >> plane) & 0xff)) >> 1 : *ptr1;
			count = 0xc0;
			do {
				ptr1++;
				if (ptr2 != NULL)
					ptr2++;
				count++;
				x++;
			} while (last == (ptr2 != NULL ? (((Colours_table[*ptr1] >> plane) & 0xff) + ((Colours_table[*ptr2] >> plane) & 0xff)) >> 1 : *ptr1)
						&& count < 0xff && x < ATARI_VISIBLE_WIDTH);
			if (count > 0xc1 || last >= 0xc0)
				fputc(count, fp);
			fputc(last, fp);
		} while (x < ATARI_VISIBLE_WIDTH);

		if (ptr2 != NULL && plane) {
			ptr1 -= ATARI_VISIBLE_WIDTH;
			ptr2 -= ATARI_VISIBLE_WIDTH;
			plane -= 8;
		}
		else {
			ptr1 += Screen_WIDTH - ATARI_VISIBLE_WIDTH;
			if (ptr2 != NULL) {
				ptr2 += Screen_WIDTH - ATARI_VISIBLE_WIDTH;
				plane = 16;
			}
			y++;
		}
	}

	if (ptr2 == NULL) {
		/* write palette */
		fputc(0xc, fp);
		for (i = 0; i < 256; i++) {
			fputc(Colours_GetR(i), fp);
			fputc(Colours_GetG(i), fp);
			fputc(Colours_GetB(i), fp);
		}
	}
}

static int striendswith(const char *s1, const char *s2)
{
	int pos;
	pos = strlen(s1) - strlen(s2);
	if (pos < 0)
		return 0;
	return Util_stricmp(s1 + pos, s2) == 0;
}

#ifdef HAVE_LIBPNG
static void Screen_SavePNG(FILE *fp, UBYTE *ptr1, UBYTE *ptr2)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep rows[Screen_HEIGHT];

	png_ptr = png_create_write_struct(
		PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL
	);
	if (png_ptr == NULL)
		return;
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
		return;
	png_init_io(png_ptr, fp);
	png_set_IHDR(
		png_ptr, info_ptr, ATARI_VISIBLE_WIDTH, Screen_HEIGHT,
		8, ptr2 == NULL ? PNG_COLOR_TYPE_PALETTE : PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
	);
	if (ptr2 == NULL) {
		int i;
		png_color palette[256];
		for (i = 0; i < 256; i++) {
			palette[i].red = Colours_GetR(i);
			palette[i].green = Colours_GetG(i);
			palette[i].blue = Colours_GetB(i);
		}
		png_set_PLTE(png_ptr, info_ptr, palette, 256);
		for (i = 0; i < Screen_HEIGHT; i++) {
			rows[i] = ptr1;
			ptr1 += Screen_WIDTH;
		}
	}
	else {
		png_bytep ptr3;
		int x;
		int y;
		ptr3 = (png_bytep) Util_malloc(3 * ATARI_VISIBLE_WIDTH * Screen_HEIGHT);
		for (y = 0; y < Screen_HEIGHT; y++) {
			rows[y] = ptr3;
			for (x = 0; x < ATARI_VISIBLE_WIDTH; x++) {
				*ptr3++ = (png_byte) ((Colours_GetR(*ptr1) + Colours_GetR(*ptr2)) >> 1);
				*ptr3++ = (png_byte) ((Colours_GetG(*ptr1) + Colours_GetG(*ptr2)) >> 1);
				*ptr3++ = (png_byte) ((Colours_GetB(*ptr1) + Colours_GetB(*ptr2)) >> 1);
				ptr1++;
				ptr2++;
			}
			ptr1 += Screen_WIDTH - ATARI_VISIBLE_WIDTH;
			ptr2 += Screen_WIDTH - ATARI_VISIBLE_WIDTH;
		}
	}
	png_set_rows(png_ptr, info_ptr, rows);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	if (ptr2 != NULL)
		free(rows[0]);
}
#endif /* HAVE_LIBPNG */

int Screen_SaveScreenshot(const char *filename, int interlaced)
{
	int is_png;
	FILE *fp;
	ULONG *main_screen_atari;
	UBYTE *ptr1;
	UBYTE *ptr2;
	if (striendswith(filename, ".pcx"))
		is_png = 0;
#ifdef HAVE_LIBPNG
	else if (striendswith(filename, ".png"))
		is_png = 1;
#endif
	else
		return FALSE;
	fp = fopen(filename, "wb");
	if (fp == NULL)
		return FALSE;
	main_screen_atari = Screen_atari;
	ptr1 = (UBYTE *) Screen_atari + ATARI_LEFT_MARGIN;
	if (interlaced) {
		Screen_atari = (ULONG *) Util_malloc(Screen_WIDTH * Screen_HEIGHT);
		ptr2 = (UBYTE *) Screen_atari + ATARI_LEFT_MARGIN;
		ANTIC_Frame(TRUE); /* draw on Screen_atari */
	}
	else {
		ptr2 = NULL;
	}
#ifdef HAVE_LIBPNG
	if (is_png)
		Screen_SavePNG(fp, ptr1, ptr2);
	else
#endif
		Screen_SavePCX(fp, ptr1, ptr2);
	fclose(fp);
	if (interlaced) {
		free(Screen_atari);
		Screen_atari = main_screen_atari;
	}
	return TRUE;
}

void Screen_SaveNextScreenshot(int interlaced)
{
	char filename[FILENAME_MAX];
	Screen_FindScreenshotFilename(filename, sizeof(filename));
	Screen_SaveScreenshot(filename, interlaced);
}

void Screen_EntireDirty(void)
{
#ifdef DIRTYRECT
	if (Screen_dirty)
		memset(Screen_dirty, 1, Screen_WIDTH * Screen_HEIGHT / 8);
#endif /* DIRTYRECT */
}
