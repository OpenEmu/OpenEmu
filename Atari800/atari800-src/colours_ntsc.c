/*
 * colours_ntsc.c - Atari NTSC colour palette generation and adjustment
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

#include "colours_ntsc.h"

#include "config.h"

#include "atari.h" /* for TRUE/FALSE */
#include "colours.h"
#include "log.h"
#include "util.h"

Colours_setup_t COLOURS_NTSC_setup;
COLOURS_NTSC_setup_t COLOURS_NTSC_specific_setup;

/* Default setup - used by COLOURS_NTSC_RestoreDefaults(). */
static const COLOURS_NTSC_setup_t default_setup = {
	0.0, /* hue */
	21.0, /* color delay, chosen to match color names given in GTIA.PDF */
};

COLOURS_EXTERNAL_t COLOURS_NTSC_external = { "", FALSE, FALSE };

/* NTSC colorburst angle in YIQ colorspace. Colorburst is at
 * 180 degrees in YUV - that is, a gold color. In YIQ, gold is at
 * different angle. However, YIQ is actually YUV turned
 * 33 degrees. So by looking at screenshots at Wikipedia we can
 * conclude that the colorburst angle is 180+33 in YIQ.
 * (See http://en.wikipedia.org/wiki/YUV and
 * http://en.wikipedia.org/wiki/YIQ) */
static const double colorburst_angle = (213.0f) * M_PI / 180.0f;

/* In COLOURS_NTSC colours are generated in 2 stages:
   1. Generate Y, I and Q values for all 256 colours and store in an
      intermediate yiq_table;
   2. Convert YIQ to RGB.
   Splitting the process to 2 stages is necessary to allow the NTSC_FILTER
   module to use the same palette generation routines. Without such
   requirement, NTSC palette generation would be performed in one stage,
   similarly to the COLOURS_PAL module. */

/* Creates YIQ_TABLE from external palette. START_ANGLE and START_SATURATIION
   are provided as parameters, because NTSC_FILTER needs to set these values
   according to its internal setup (burst_phase etc.).
   External palette is not adjusted if COLOURS_NTSC_external.adjust is false. */
static void UpdateYIQTableFromExternal(double yiq_table[768], double start_angle, const double start_saturation)
{
	const double gamma = 1 - COLOURS_NTSC_setup.gamma / 2.0;
	unsigned char *ext_ptr = COLOURS_NTSC_external.palette;
	int n;

	start_angle = - colorburst_angle - start_angle;

	for (n = 0; n < 256; n ++) {
		/* Convert RGB values from external palette to YIQ. */
		double r = (double)*ext_ptr++ / 255.0;
		double g = (double)*ext_ptr++ / 255.0;
		double b = (double)*ext_ptr++ / 255.0;
		double y = 0.299 * r + 0.587 * g + 0.114 * b;
		double i = 0.595716 * r - 0.274453 * g - 0.321263 * b;
		double q = 0.211456 * r - 0.522591 * g + 0.311135 * b;
		double s = sin(start_angle);
		double c = cos(start_angle);
		double tmp_i = i;
		i = tmp_i * c - q * s;
		q = tmp_i * s + q * c;
		/* Optionally adjust external palette. */
		if (COLOURS_NTSC_external.adjust) {
			y = pow(y, gamma);
			y *= COLOURS_NTSC_setup.contrast * 0.5 + 1;
			y += COLOURS_NTSC_setup.brightness * 0.5;
			if (y > 1.0)
				y = 1.0;
			else if (y < 0.0)
				y = 0.0;
			i *= start_saturation + 1;
			q *= start_saturation + 1;
		}

		*yiq_table++ = y;
		*yiq_table++ = i;
		*yiq_table++ = q;
	}
}

/* Generates NTSC palette into YIQ_TABLE. START_ANGLE and START_SATURATIION
   are provided as parameters, because NTSC_FILTER needs to set these values
   according to its internal setup (burst_phase etc.) */
static void UpdateYIQTableFromGenerated(double yiq_table[768], const double start_angle, const double start_saturation)
{
	/* NTSC coloburst frequency is (315.0/88.0)MHz
	 * (see http://en.wikipedia.org/wiki/Colorburst). By dividing 1000 by
	 * this fraction, we get NTSC color cycle duration in nanoseconds. */
	static const double color_cycle_length = 1000.0 * 88.0 / 315.0;
	/* This computes difference between two consecutive chrominances, in
	 * radians. */
	double color_diff = COLOURS_NTSC_specific_setup.color_delay / color_cycle_length * 2 * M_PI;

	int cr, lm;

	double scaled_black_level = (double)COLOURS_NTSC_setup.black_level / 255.0;
	double scaled_white_level = (double)COLOURS_NTSC_setup.white_level / 255.0;
	const double gamma = 1 - COLOURS_NTSC_setup.gamma / 2.0;

	/* NTSC luma multipliers from CGIA.PDF */
	double luma_mult[16]={
		0.6941, 0.7091, 0.7241, 0.7401, 
		0.7560, 0.7741, 0.7931, 0.8121,
		0.8260, 0.8470, 0.8700, 0.8930,
		0.9160, 0.9420, 0.9690, 1.0000};

	for (cr = 0; cr < 16; cr ++) {
		double angle = start_angle - (cr - 1) * color_diff;
		double saturation = (cr ? (start_saturation + 1) * 0.175f: 0.0);
		double i = sin(angle) * saturation;
		double q = cos(angle) * saturation;

		for (lm = 0; lm < 16; lm ++) {
			/* calculate yiq for color entry */
			double y = (luma_mult[lm] - luma_mult[0]) / (luma_mult[15] - luma_mult[0]);
			y = pow(y, gamma);
			y *= COLOURS_NTSC_setup.contrast * 0.5 + 1;
			y += COLOURS_NTSC_setup.brightness * 0.5;
			/* Scale the Y signal's range from 0..1 to
			* scaled_black_level..scaled_white_level */
			y = y * (scaled_white_level - scaled_black_level) + scaled_black_level;
			/*
			if (y < scaled_black_level)
				y = scaled_black_level;
			else if (y > scaled_white_level)
				y = scaled_white_level;
			*/
			*yiq_table++ = y;
			*yiq_table++ = i;
			*yiq_table++ = q;
		}
	}
}

/* Fills YIQ_TABLE with palette. Depending on the current setup, it is
   computed from an internally-generated or external palette. */
static void UpdateYIQTable(double yiq_table[768], double start_angle, const double start_saturation)
{
	if (COLOURS_NTSC_external.loaded)
		UpdateYIQTableFromExternal(yiq_table, start_angle, start_saturation);
	else
		UpdateYIQTableFromGenerated(yiq_table, start_angle, start_saturation);
}

void COLOURS_NTSC_GetYIQ(double yiq_table[768], const double start_angle)
{
	/* Set the generated palette's saturation to 0.0, because NTSC_FILTER
	   applies the saturation setting internally. */
	UpdateYIQTable(yiq_table, start_angle, 0.0);
}

/* Converts YIQ values from YIQ_TABLE to RGB values. Stores them in
   COLOURTABLE. */
static void YIQ2RGB(int colourtable[256], const double yiq_table[768])
{
	const double *yiq_ptr = yiq_table;
	int n;
	for (n = 0; n < 256; n ++) {
		double y = *yiq_ptr++;
		double i = *yiq_ptr++;
		double q = *yiq_ptr++;
		double r, g, b;
		r = y + 0.9563 * i + 0.6210 * q;
		g = y - 0.2721 * i - 0.6474 * q;
		b = y - 1.1070 * i + 1.7046 * q;
		Colours_SetRGB(n, (int) (r * 255), (int) (g * 255), (int) (b * 255), colourtable);
	}
}

void COLOURS_NTSC_Update(int colourtable[256])
{
	double yiq_table[768];
	UpdateYIQTable(yiq_table, - colorburst_angle + COLOURS_NTSC_specific_setup.hue * M_PI, COLOURS_NTSC_setup.saturation);
	YIQ2RGB(colourtable, yiq_table);
}

void COLOURS_NTSC_RestoreDefaults(void)
{
	COLOURS_NTSC_specific_setup = default_setup;
}

void COLOURS_NTSC_SetPreset(Colours_preset_t preset)
{	
	if (preset == COLOURS_PRESET_STANDARD)
		COLOURS_NTSC_specific_setup = default_setup;
}

Colours_preset_t COLOURS_NTSC_GetPreset()
{
	if (Util_almostequal(COLOURS_NTSC_specific_setup.hue, default_setup.hue, 0.001) &&
	    Util_almostequal(COLOURS_NTSC_specific_setup.color_delay, default_setup.color_delay, 0.001))
		return COLOURS_PRESET_STANDARD;
	return COLOURS_PRESET_CUSTOM;
}

void COLOURS_NTSC_PreInitialise(void)
{
	COLOURS_NTSC_specific_setup = default_setup;
}

int COLOURS_NTSC_ReadConfig(char *option, char *ptr)
{
	if (strcmp(option, "COLOURS_NTSC_SATURATION") == 0)
		return Util_sscandouble(ptr, &COLOURS_NTSC_setup.saturation);
	else if (strcmp(option, "COLOURS_NTSC_CONTRAST") == 0)
		return Util_sscandouble(ptr, &COLOURS_NTSC_setup.contrast);
	else if (strcmp(option, "COLOURS_NTSC_BRIGHTNESS") == 0)
		return Util_sscandouble(ptr, &COLOURS_NTSC_setup.brightness);
	else if (strcmp(option, "COLOURS_NTSC_GAMMA") == 0)
		return Util_sscandouble(ptr, &COLOURS_NTSC_setup.gamma);
	else if (strcmp(option, "COLOURS_NTSC_HUE") == 0)
		return Util_sscandouble(ptr, &COLOURS_NTSC_specific_setup.hue);
	else if (strcmp(option, "COLOURS_NTSC_DELAY") == 0)
		return Util_sscandouble(ptr, &COLOURS_NTSC_specific_setup.color_delay);
	else if (strcmp(option, "COLOURS_NTSC_EXTERNAL_PALETTE") == 0)
		Util_strlcpy(COLOURS_NTSC_external.filename, ptr, sizeof(COLOURS_NTSC_external.filename));
	else if (strcmp(option, "COLOURS_NTSC_EXTERNAL_PALETTE_LOADED") == 0)
		/* Use the "loaded" flag to indicate that the palette must be loaded later. */
		return (COLOURS_NTSC_external.loaded = Util_sscanbool(ptr)) != -1;
	else if (strcmp(option, "COLOURS_NTSC_ADJUST_EXTERNAL_PALETTE") == 0)
		return (COLOURS_NTSC_external.adjust = Util_sscanbool(ptr)) != -1;
	else
		return FALSE;
	return TRUE;
}

void COLOURS_NTSC_WriteConfig(FILE *fp)
{
	fprintf(fp, "COLOURS_NTSC_SATURATION=%g\n", COLOURS_NTSC_setup.saturation);
	fprintf(fp, "COLOURS_NTSC_CONTRAST=%g\n", COLOURS_NTSC_setup.contrast);
	fprintf(fp, "COLOURS_NTSC_BRIGHTNESS=%g\n", COLOURS_NTSC_setup.brightness);
	fprintf(fp, "COLOURS_NTSC_GAMMA=%g\n", COLOURS_NTSC_setup.gamma);
	fprintf(fp, "COLOURS_NTSC_HUE=%g\n", COLOURS_NTSC_specific_setup.hue);
	fprintf(fp, "COLOURS_NTSC_DELAY=%g\n", COLOURS_NTSC_specific_setup.color_delay);
	fprintf(fp, "COLOURS_NTSC_EXTERNAL_PALETTE=%s\n", COLOURS_NTSC_external.filename);
	fprintf(fp, "COLOURS_NTSC_EXTERNAL_PALETTE_LOADED=%d\n", COLOURS_NTSC_external.loaded);
	fprintf(fp, "COLOURS_NTSC_ADJUST_EXTERNAL_PALETTE=%d\n", COLOURS_NTSC_external.adjust);
}

int COLOURS_NTSC_Initialise(int *argc, char *argv[])
{
	int i;
	int j;

	for (i = j = 1; i < *argc; i++) {
		int i_a = (i + 1 < *argc);		/* is argument available? */
		int a_m = FALSE;			/* error, argument missing! */
		
		if (strcmp(argv[i], "-ntsc-saturation") == 0) {
			if (i_a)
				COLOURS_NTSC_setup.saturation = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-ntsc-contrast") == 0) {
			if (i_a)
				COLOURS_NTSC_setup.contrast = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-ntsc-brightness") == 0) {
			if (i_a)
				COLOURS_NTSC_setup.brightness = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-ntsc-gamma") == 0) {
			if (i_a)
				COLOURS_NTSC_setup.gamma = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-hue") == 0) {
			if (i_a)
				COLOURS_NTSC_specific_setup.hue = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-colordelay") == 0) {
			if (i_a)
				COLOURS_NTSC_specific_setup.color_delay = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-paletten") == 0) {
			if (i_a) {
				Util_strlcpy(COLOURS_NTSC_external.filename, argv[++i], sizeof(COLOURS_NTSC_external.filename));
				/* Use the "loaded" flag to indicate that the palette must be loaded later. */
				COLOURS_NTSC_external.loaded = TRUE;
			} else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-paletten-adjust") == 0)
			COLOURS_NTSC_external.adjust = TRUE;
		else {
			if (strcmp(argv[i], "-help") == 0) {
				Log_print("\t-ntsc-saturation <num>  Set NTSC color saturation");
				Log_print("\t-ntsc-contrast <num>    Set NTSC contrast");
				Log_print("\t-ntsc-brightness <num>  Set NTSC brightness");
				Log_print("\t-ntsc-gamma <num>       Set NTSC color gamma factor");
				Log_print("\t-hue <num>              Set NTSC hue of colors");
				Log_print("\t-colordelay <num>       Set NTSC GTIA color delay");
				Log_print("\t-paletten <filename>    Load NTSC external palette");
				Log_print("\t-paletten-adjust        Apply adjustments to NTSC external palette");
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
	if (COLOURS_NTSC_external.loaded && !COLOURS_EXTERNAL_Read(&COLOURS_NTSC_external))
		Log_print("Cannot read NTSC palette from %s", COLOURS_NTSC_external.filename);
	
	return TRUE;
}
