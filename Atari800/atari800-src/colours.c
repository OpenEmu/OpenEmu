/*
 * colours.c - Atari colour palette adjustment - functions common for NTSC and
 *             PAL palettes
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

#include "config.h"
#include <stdio.h>
#include <string.h>	/* for strcmp() */
#include <math.h>
#include <stdlib.h>
#include "atari.h"
#include "cfg.h"
#include "colours.h"
#include "colours_external.h"
#include "colours_ntsc.h"
#include "colours_pal.h"
#include "log.h"
#include "util.h"
#include "platform.h"

#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

Colours_setup_t *Colours_setup;
COLOURS_EXTERNAL_t *Colours_external;

static Colours_setup_t const presets[] = {
	/* Saturation, Contrast, Brightness, Gamm adjustment, Black level, White level */
	{ 0.0, 0.0, 0.0, 0.3, 16, 235 }, /* Standard preset */
	{ 0.0, 0.2, -0.16, 0.5, 16, 235 }, /* Deep blacks preset */
	{ 0.26, 0.72, -0.16, 0.16, 16, 235 } /* Vibrant colours & levels preset */
};
static char const * const preset_cfg_strings[COLOURS_PRESET_SIZE] = {
	"STANDARD",
	"DEEP-BLACK",
	"VIBRANT"
};

int Colours_table[256];

void Colours_SetRGB(int i, int r, int g, int b, int *colortable_ptr)
{
	if (r < 0)
		r = 0;
	else if (r > 255)
		r = 255;
	if (g < 0)
		g = 0;
	else if (g > 255)
		g = 255;
	if (b < 0)
		b = 0;
	else if (b > 255)
		b = 255;
	colortable_ptr[i] = (r << 16) + (g << 8) + b;
}

static void UpdateModeDependentPointers(int tv_mode)
{
	/* Set pointers to the currnt setup and external palette */
	if (tv_mode == Atari800_TV_NTSC) {
		Colours_setup = &COLOURS_NTSC_setup;
		Colours_external = &COLOURS_NTSC_external;
	}
       	else if (tv_mode == Atari800_TV_PAL) {
		Colours_setup = &COLOURS_PAL_setup;
		Colours_external = &COLOURS_PAL_external;
	}
	else {
		Atari800_Exit(FALSE);
		Log_print("Interal error: Invalid Atari800_tv_mode\n");
		exit(1);
	}
}

void Colours_SetVideoSystem(int mode)
{
	UpdateModeDependentPointers(mode);
	/* Apply changes */
	Colours_Update();
}

/* Copies the loaded external palette into current palette - without applying
   adjustments. */
static void CopyExternalWithoutAdjustments(void)
{
	int i;
	unsigned char *ext_ptr;
	for (i = 0, ext_ptr = Colours_external->palette; i < 256; i ++, ext_ptr += 3)
		Colours_SetRGB(i, *ext_ptr, *(ext_ptr + 1), *(ext_ptr + 2), Colours_table);
}

/* Updates contents of Colours_table. */
static void UpdatePalette(void)
{
	if (Colours_external->loaded && !Colours_external->adjust)
		CopyExternalWithoutAdjustments();
	else if (Atari800_tv_mode == Atari800_TV_NTSC)
		COLOURS_NTSC_Update(Colours_table);
	else /* PAL */
		COLOURS_PAL_Update(Colours_table);
}

void Colours_Update(void)
{
	UpdatePalette();
#if SUPPORTS_PLATFORM_PALETTEUPDATE
	PLATFORM_PaletteUpdate();
#endif
}

void Colours_RestoreDefaults(void)
{
	*Colours_setup = presets[COLOURS_PRESET_STANDARD];
	if (Atari800_tv_mode == Atari800_TV_NTSC)
		COLOURS_NTSC_RestoreDefaults();
}

/* Sets the video calibration profile to the user preference */
void Colours_SetPreset(Colours_preset_t preset)
{
	if (preset < COLOURS_PRESET_CUSTOM)
		*Colours_setup = presets[preset];

	if (Atari800_tv_mode == Atari800_TV_NTSC) 
		COLOURS_NTSC_SetPreset(COLOURS_PRESET_STANDARD);
}

/* Compares the current settings to the available calibration profiles
   and returns the matching profile -- or CUSTOM if no match is found */
Colours_preset_t Colours_GetPreset(void)
{
	int i;

	if (Atari800_tv_mode == Atari800_TV_NTSC &&
	    COLOURS_NTSC_GetPreset() != COLOURS_PRESET_STANDARD)
		return COLOURS_PRESET_CUSTOM;

	for (i = 0; i < COLOURS_PRESET_SIZE; i ++) {
		if (Util_almostequal(Colours_setup->saturation, presets[i].saturation, 0.001) &&
		    Util_almostequal(Colours_setup->contrast, presets[i].contrast, 0.001) &&
		    Util_almostequal(Colours_setup->brightness, presets[i].brightness, 0.001) &&
		    Util_almostequal(Colours_setup->gamma, presets[i].gamma, 0.001) &&
		    Colours_setup->black_level == presets[i].black_level &&
		    Colours_setup->white_level == presets[i].white_level)
			return i; 
	}
	return COLOURS_PRESET_CUSTOM;
}

int Colours_Save(const char *filename)
{
	FILE *fp;
	int i;

	fp = fopen(filename, "wb");
	if (fp == NULL) {
		return FALSE;
	}

	/* Create a raw 768-byte file with RGB values. */
	for (i = 0; i < 256; i ++) {
		char rgb[3];
		rgb[0] = Colours_GetR(i);
		rgb[1] = Colours_GetG(i);
		rgb[2] = Colours_GetB(i);
		if (fwrite(rgb, sizeof(rgb), 1, fp) != 1) {
			fclose(fp);
			return FALSE;
		}
	}

	fclose(fp);
	return TRUE;
}

void Colours_PreInitialise(void)
{
	/* Copy the default setup for both NTSC and PAL. */
	COLOURS_NTSC_setup = COLOURS_PAL_setup = presets[COLOURS_PRESET_STANDARD];
	COLOURS_NTSC_PreInitialise();
}

int Colours_ReadConfig(char *option, char *ptr)
{
	if (COLOURS_NTSC_ReadConfig(option, ptr)) {
	}
	else if (COLOURS_PAL_ReadConfig(option, ptr)) {
	}
	else return FALSE; /* no match */
	return TRUE; /* matched something */
}

void Colours_WriteConfig(FILE *fp)
{
	COLOURS_NTSC_WriteConfig(fp);
	COLOURS_PAL_WriteConfig(fp);
}

int Colours_Initialise(int *argc, char *argv[])
{
	int i;
	int j;

	for (i = j = 1; i < *argc; i++) {
		int i_a = (i + 1 < *argc);		/* is argument available? */
		int a_m = FALSE;			/* error, argument missing! */
		
		if (strcmp(argv[i], "-saturation") == 0) {
			if (i_a)
				COLOURS_NTSC_setup.saturation = COLOURS_PAL_setup.saturation = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-contrast") == 0) {
			if (i_a)
				COLOURS_NTSC_setup.contrast = COLOURS_PAL_setup.contrast = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-brightness") == 0) {
			if (i_a)
				COLOURS_NTSC_setup.brightness = COLOURS_PAL_setup.brightness = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-gamma") == 0) {
			if (i_a)
				COLOURS_NTSC_setup.gamma = COLOURS_PAL_setup.gamma = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-colors-preset") == 0) {
			if (i_a) {
				int idx = CFG_MatchTextParameter(argv[++i], preset_cfg_strings, COLOURS_PRESET_SIZE);
				if (idx < 0) {
					Log_print("Invalid value for -colors-preset");
					return FALSE;
				}
				COLOURS_NTSC_setup = COLOURS_PAL_setup = presets[idx];
				COLOURS_NTSC_SetPreset(COLOURS_PRESET_STANDARD);
			} else a_m = TRUE;
		}

		else {
			if (strcmp(argv[i], "-help") == 0) {
				Log_print("\t-colors-preset standard|deep-black|vibrant");
				Log_print("\t                       Use one of predefined color adjustments");
				Log_print("\t-saturation <num>      Set color saturation");
				Log_print("\t-contrast <num>        Set contrast");
				Log_print("\t-brightness <num>      Set brightness");
				Log_print("\t-gamma <num>           Set color gamma factor");
			}
			argv[j++] = argv[i];
		}

		if (a_m) {
			Log_print("Missing argument for '%s'", argv[i]);
			return FALSE;
		}
	}
	*argc = j;

	if (!COLOURS_NTSC_Initialise(argc, argv) ||
	    !COLOURS_PAL_Initialise(argc, argv))
		return FALSE;

	/* Assume that Atari800_tv_mode has been already initialised. */
	UpdateModeDependentPointers(Atari800_tv_mode);
	UpdatePalette();
	return TRUE;
}
