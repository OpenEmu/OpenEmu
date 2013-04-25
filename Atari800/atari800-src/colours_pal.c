/*
 * colours_pal.c - Atari PAL colour palette generation and adjustment
 *
 * Copyright (C) 2009-2010 Atari800 development team (see DOC/CREDITS)
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
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "colours_pal.h"

#include "atari.h" /* for TRUE/FALSE */
#include "colours.h"
#include "log.h"
#include "util.h"

#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

Colours_setup_t COLOURS_PAL_setup;

/* PAL-specific default setup. */
static struct {
	double color_delay;
} const default_setup = {
	23.2, /* chosen by eye to give a smooth rainbow */
};

COLOURS_EXTERNAL_t COLOURS_PAL_external = { "", FALSE, FALSE };

/* Applies the colours setup to the external palette. Writes output to
   COLOURTABLE. */
static void AdjustExternal(int colourtable[256])
{
	const double gamma = 1 - COLOURS_PAL_setup.gamma / 2.0;
	unsigned char *ext_ptr = COLOURS_PAL_external.palette;
	int n;

	double const hue = COLOURS_PAL_setup.hue * M_PI;
	double const s = sin(hue);
	double const c = cos(hue);

	for (n = 0; n < 256; n ++) {
		/* Convert RGB values from external palette to YUV. */
		double r = (double)*ext_ptr++ / 255.0;
		double g = (double)*ext_ptr++ / 255.0;
		double b = (double)*ext_ptr++ / 255.0;
		double y = 0.299 * r + 0.587 * g + 0.114 * b;
		double u = -0.14713 * r - 0.28886 * g + 0.436 * b;
		double v = 0.615 * r - 0.51499 * g - 0.10001 * b;
		double tmp_u = u;
		u = tmp_u * c - v * s;
		v = tmp_u * s + v * c;
		y = pow(y, gamma);
		y *= COLOURS_PAL_setup.contrast * 0.5 + 1;
		y += COLOURS_PAL_setup.brightness * 0.5;
		if (y > 1.0)
			y = 1.0;
		else if (y < 0.0)
			y = 0.0;
		u *= COLOURS_PAL_setup.saturation + 1.0;
		v *= COLOURS_PAL_setup.saturation + 1.0;

		r = y + 1.13983 * v;
		g = y - 0.39465 * u - 0.58060 * v;
		b = y + 2.03211 * u;

		Colours_SetRGB(n, (int) (r * 255), (int) (g * 255), (int) (b * 255), colourtable);
	}
}

/* Generates a PAL palette, based on colour setup. Result is written into
   COLOURTABLE.
   PAL GTIA produces slightly different hues on odd and even lines;
   they are later averaged in the TV (a feature of PAL system). The generated
   palette reproduces the real machine somewhat - each hue is an average of
   an "odd" hue and an "even" hue. */
static void GeneratePalette(int colourtable[256])
{
	int cr, lm;

	double scaled_black_level = (double)COLOURS_PAL_setup.black_level / 255.0f;
	double scaled_white_level = (double)COLOURS_PAL_setup.white_level / 255.0f;
	const double gamma = 1 - COLOURS_PAL_setup.gamma / 2.0;

	double const start_angle = COLOURS_PAL_setup.hue * M_PI;
	double color_diff = COLOURS_PAL_setup.color_delay * M_PI / 180.0;

	/* NTSC luma multipliers from CGIA.PDF */
	double luma_mult[16] = {
		0.6941, 0.7091, 0.7241, 0.7401,
		0.7560, 0.7741, 0.7931, 0.8121,
		0.8260, 0.8470, 0.8700, 0.8930,
		0.9160, 0.9420, 0.9690, 1.0000};

	for (cr = 0; cr < 16; cr ++) {
		double u = 0.0;
		double v = 0.0;
		if (cr) {
			struct pal_phase {
				double mult;
				double shift;
			};
			static struct pal_phase const phases[14] = {
					{  1.5, M_PI },
					{  2.5, M_PI },
					{  3.5, M_PI },
					{ -1.5,  0.0 },
					{ -0.5,  0.0 },
					{  0.0,  0.0 },
					{  1.5,  0.0 },
					{  2.5,  0.0 },
					{  3.5,  0.0 },
					{  4.0,  0.0 },
					{ -2.5, M_PI },
					{ -1.5, M_PI },
					{ -0.5, M_PI },
					{  0.5, M_PI }
			};
			struct pal_phase const *odd_phase = &phases[(cr - 1) % 14];
			struct pal_phase const *even_phase = &phases[(25 -(cr - 1)) % 14];
			double odd_angle = start_angle + odd_phase->shift - color_diff * (odd_phase->mult);
			double even_angle = start_angle + even_phase->shift + color_diff * (even_phase->mult - 0.5);
			double odd_u = cos(odd_angle);
			double odd_v = sin(odd_angle);
			double even_u = cos(even_angle);
			double even_v = sin(even_angle);
			double saturation = (COLOURS_PAL_setup.saturation + 1) * 0.175;
			u = (even_u + odd_u) * 0.5 * saturation;
			v = (even_v + odd_v) * 0.5 * saturation;
		}
		for (lm = 0; lm < 16; lm ++) {
			/* calculate yuv for color entry */
			double y = (luma_mult[lm] - luma_mult[0]) / (luma_mult[15] - luma_mult[0]);
			double r, g, b;
			y = pow(y, gamma);
			y *= COLOURS_PAL_setup.contrast * 0.5 + 1;
			y += COLOURS_PAL_setup.brightness * 0.5;
			/* Scale the Y signal's range from 0..1 to
			* scaled_black_level..scaled_white_level */
			y = y * (scaled_white_level - scaled_black_level) + scaled_black_level;
			/*
			if (y < scaled_black_level)
				y = scaled_black_level;
			else if (y > scaled_white_level)
				y = scaled_white_level;
			*/
			r = y + 1.13983 * v;
			g = y - 0.39465 * u - 0.58060 * v;
			b = y + 2.03211 * u;
			Colours_SetRGB(cr * 16 + lm, (int) (r * 255), (int) (g * 255), (int) (b * 255), colourtable);
		}
	}
}

void COLOURS_PAL_Update(int colourtable[256])
{
	if (COLOURS_PAL_external.loaded)
		AdjustExternal(colourtable);
	else
		GeneratePalette(colourtable);
}

void COLOURS_PAL_RestoreDefaults(void)
{
	COLOURS_PAL_setup.color_delay = default_setup.color_delay;
}

Colours_preset_t COLOURS_PAL_GetPreset()
{
	if (Util_almostequal(COLOURS_PAL_setup.color_delay, default_setup.color_delay, 0.001))
		return COLOURS_PRESET_STANDARD;
	return COLOURS_PRESET_CUSTOM;
}

int COLOURS_PAL_ReadConfig(char *option, char *ptr)
{
	if (strcmp(option, "COLOURS_PAL_SATURATION") == 0)
		return Util_sscandouble(ptr, &COLOURS_PAL_setup.saturation);
	else if (strcmp(option, "COLOURS_PAL_CONTRAST") == 0)
		return Util_sscandouble(ptr, &COLOURS_PAL_setup.contrast);
	else if (strcmp(option, "COLOURS_PAL_BRIGHTNESS") == 0)
		return Util_sscandouble(ptr, &COLOURS_PAL_setup.brightness);
	else if (strcmp(option, "COLOURS_PAL_GAMMA") == 0)
		return Util_sscandouble(ptr, &COLOURS_PAL_setup.gamma);
	else if (strcmp(option, "COLOURS_PAL_HUE") == 0)
		return Util_sscandouble(ptr, &COLOURS_PAL_setup.hue);
	else if (strcmp(option, "COLOURS_PAL_GTIA_DELAY") == 0)
		return Util_sscandouble(ptr, &COLOURS_PAL_setup.color_delay);
	else if (strcmp(option, "COLOURS_PAL_EXTERNAL_PALETTE") == 0)
		Util_strlcpy(COLOURS_PAL_external.filename, ptr, sizeof(COLOURS_PAL_external.filename));
	else if (strcmp(option, "COLOURS_PAL_EXTERNAL_PALETTE_LOADED") == 0)
		/* Use the "loaded" flag to indicate that the palette must be loaded later. */
		return (COLOURS_PAL_external.loaded = Util_sscanbool(ptr)) != -1;
	else if (strcmp(option, "COLOURS_PAL_ADJUST_EXTERNAL_PALETTE") == 0)
		return (COLOURS_PAL_external.adjust = Util_sscanbool(ptr)) != -1;
	else
		return FALSE;
	return TRUE;
}

void COLOURS_PAL_WriteConfig(FILE *fp)
{
	fprintf(fp, "COLOURS_PAL_SATURATION=%g\n", COLOURS_PAL_setup.saturation);
	fprintf(fp, "COLOURS_PAL_CONTRAST=%g\n", COLOURS_PAL_setup.contrast);
	fprintf(fp, "COLOURS_PAL_BRIGHTNESS=%g\n", COLOURS_PAL_setup.brightness);
	fprintf(fp, "COLOURS_PAL_GAMMA=%g\n", COLOURS_PAL_setup.gamma);
	fprintf(fp, "COLOURS_PAL_HUE=%g\n", COLOURS_PAL_setup.hue);
	fprintf(fp, "COLOURS_PAL_GTIA_DELAY=%g\n", COLOURS_PAL_setup.color_delay);
	fprintf(fp, "COLOURS_PAL_EXTERNAL_PALETTE=%s\n", COLOURS_PAL_external.filename);
	fprintf(fp, "COLOURS_PAL_EXTERNAL_PALETTE_LOADED=%d\n", COLOURS_PAL_external.loaded);
	fprintf(fp, "COLOURS_PAL_ADJUST_EXTERNAL_PALETTE=%d\n", COLOURS_PAL_external.adjust);
}

int COLOURS_PAL_Initialise(int *argc, char *argv[])
{
	int i;
	int j;

	for (i = j = 1; i < *argc; i++) {
		int i_a = (i + 1 < *argc);		/* is argument available? */
		int a_m = FALSE;			/* error, argument missing! */
		
		if (strcmp(argv[i], "-pal-saturation") == 0) {
			if (i_a)
				COLOURS_PAL_setup.saturation = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-pal-contrast") == 0) {
			if (i_a)
				COLOURS_PAL_setup.contrast = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-pal-brightness") == 0) {
			if (i_a)
				COLOURS_PAL_setup.brightness = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-pal-gamma") == 0) {
			if (i_a)
				COLOURS_PAL_setup.gamma = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-pal-tint") == 0) {
			if (i_a)
				COLOURS_PAL_setup.hue = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-pal-colordelay") == 0) {
			if (i_a)
				COLOURS_PAL_setup.color_delay = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-palettep") == 0) {
			if (i_a) {
				Util_strlcpy(COLOURS_PAL_external.filename, argv[++i], sizeof(COLOURS_PAL_external.filename));
				/* Use the "loaded" flag to indicate that the palette must be loaded later. */
				COLOURS_PAL_external.loaded = TRUE;
			} else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-palettep-adjust") == 0)
			COLOURS_PAL_external.adjust = TRUE;
		else {
			if (strcmp(argv[i], "-help") == 0) {
				Log_print("\t-pal-saturation <num>  Set PAL color saturation");
				Log_print("\t-pal-contrast <num>    Set PAL contrast");
				Log_print("\t-pal-brightness <num>  Set PAL brightness");
				Log_print("\t-pal-gamma <num>       Set PAL color gamma factor");
				Log_print("\t-pal-tint <num>        Set PAL tint");
				Log_print("\t-pal-colordelay <num>  Set PAL GTIA color delay");
				Log_print("\t-palettep <filename>   Load PAL external palette");
				Log_print("\t-palettep-adjust       Apply adjustments to PAL external palette");
			}
			argv[j++] = argv[i];
		}

		if (a_m) {
			Log_print("Missing argument for '%s'", argv[i]);
			return FALSE;
		}
	}
	*argc = j;

	/* Try loading an external palette if needed. */
	if (COLOURS_PAL_external.loaded && !COLOURS_EXTERNAL_Read(&COLOURS_PAL_external))
		Log_print("Cannot read PAL palette from %s", COLOURS_PAL_external.filename);

	return TRUE;
}
