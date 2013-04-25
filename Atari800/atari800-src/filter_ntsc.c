/*
 * filter_ntsc.c - Atari800-specific interface for atari_ntsc module
 *
 * Copyright (C) 2009 Atari800 development team (see DOC/CREDITS)
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
#include <stdlib.h>
#include <math.h>

#include "filter_ntsc.h"

#include "atari.h" /* for TRUE/FALSE */
#include "atari_ntsc/atari_ntsc.h"
#include "cfg.h"
#include "colours_ntsc.h"
#include "log.h"
#include "util.h"

atari_ntsc_setup_t FILTER_NTSC_setup;

static atari_ntsc_setup_t const * const presets[] = {
	&atari_ntsc_composite,
	&atari_ntsc_svideo,
	&atari_ntsc_rgb,
	&atari_ntsc_monochrome
};

static char const * const preset_cfg_strings[FILTER_NTSC_PRESET_SIZE] = {
	"COMPOSITE",
	"SVIDEO",
	"RGB",
	"MONOCHROME"
};

atari_ntsc_t *FILTER_NTSC_emu = NULL;

atari_ntsc_t *FILTER_NTSC_New(void)
{
	atari_ntsc_t *filter = (atari_ntsc_t*) Util_malloc(sizeof(atari_ntsc_t));
	return filter;
}

void FILTER_NTSC_Delete(atari_ntsc_t *filter)
{
	free(filter);
}

void FILTER_NTSC_Update(atari_ntsc_t *filter)
{
	double yiq_table[768];

	COLOURS_NTSC_GetYIQ(yiq_table, FILTER_NTSC_setup.burst_phase * M_PI);
	/* The gamma setting is not used in atari_ntsc (palette generation is
	   placed in another module), so below we do not set the gamma field
	   of the FILTER_NTSC_setup structure. */
	if (COLOURS_NTSC_external.loaded && !COLOURS_NTSC_external.adjust) {
		/* External palette must not be adjusted, so FILTER_NTSC
		   settings are set to defaults so they don't change the source
		   palette in any way. */
		FILTER_NTSC_setup.hue = 0.0;
		FILTER_NTSC_setup.saturation = 0.0;
		FILTER_NTSC_setup.contrast = 0.0;
		FILTER_NTSC_setup.brightness = 0.0;
	} else {
		/* It's either generated palette, or external palette with
		   need for adjustments. Copy values from
		   COLOURS_NTSC_setup to FILTER_NTSC_setup. */
		FILTER_NTSC_setup.hue = COLOURS_NTSC_setup.hue;
		FILTER_NTSC_setup.saturation = COLOURS_NTSC_setup.saturation;
		FILTER_NTSC_setup.contrast = COLOURS_NTSC_setup.contrast;
		FILTER_NTSC_setup.brightness = COLOURS_NTSC_setup.brightness;
	}

	FILTER_NTSC_setup.yiq_palette = yiq_table;
	atari_ntsc_init(filter, &FILTER_NTSC_setup);
}

void FILTER_NTSC_RestoreDefaults(void)
{
	FILTER_NTSC_setup = atari_ntsc_composite;
}

void FILTER_NTSC_SetPreset(int preset)
{
	if (preset < FILTER_NTSC_PRESET_CUSTOM) {
		FILTER_NTSC_setup = *presets[preset];

		/* Copy settings from the preset to NTSC setup. */
		COLOURS_NTSC_setup.hue = FILTER_NTSC_setup.hue;
		COLOURS_NTSC_setup.saturation = FILTER_NTSC_setup.saturation;
		COLOURS_NTSC_setup.contrast = FILTER_NTSC_setup.contrast;
		COLOURS_NTSC_setup.brightness = FILTER_NTSC_setup.brightness;
		COLOURS_NTSC_setup.gamma = FILTER_NTSC_setup.gamma;
	}
}

int FILTER_NTSC_GetPreset(void)
{
	int i;

	for (i = 0; i < FILTER_NTSC_PRESET_SIZE; i ++) {
		if (Util_almostequal(FILTER_NTSC_setup.sharpness, presets[i]->sharpness, 0.001) &&
		    Util_almostequal(FILTER_NTSC_setup.resolution, presets[i]->resolution, 0.001) &&
		    Util_almostequal(FILTER_NTSC_setup.artifacts, presets[i]->artifacts, 0.001) &&
		    Util_almostequal(FILTER_NTSC_setup.fringing, presets[i]->fringing, 0.001) &&
		    Util_almostequal(FILTER_NTSC_setup.bleed, presets[i]->bleed, 0.001) &&
		    Util_almostequal(FILTER_NTSC_setup.burst_phase, presets[i]->burst_phase, 0.001) &&
		    Util_almostequal(COLOURS_NTSC_setup.hue, presets[i]->hue, 0.001) &&
		    Util_almostequal(COLOURS_NTSC_setup.saturation, presets[i]->saturation, 0.001) &&
		    Util_almostequal(COLOURS_NTSC_setup.contrast, presets[i]->contrast, 0.001) &&
		    Util_almostequal(COLOURS_NTSC_setup.brightness, presets[i]->brightness, 0.001) &&
		    Util_almostequal(COLOURS_NTSC_setup.gamma, presets[i]->gamma, 0.001))
			return i; 
	}
	return FILTER_NTSC_PRESET_CUSTOM;
}

void FILTER_NTSC_NextPreset(void)
{
	int preset = FILTER_NTSC_GetPreset();
	
	if (preset == FILTER_NTSC_PRESET_CUSTOM)
		preset = FILTER_NTSC_PRESET_COMPOSITE;
	else
		preset = (preset + 1) % FILTER_NTSC_PRESET_SIZE;
	FILTER_NTSC_SetPreset(preset);
}

void FILTER_NTSC_PreInitialise(void)
{
	/* atari_ntsc_composite acts as the default setup. */
	FILTER_NTSC_setup = atari_ntsc_composite;
}

int FILTER_NTSC_ReadConfig(char *option, char *ptr)
{
	if (strcmp(option, "FILTER_NTSC_SHARPNESS") == 0)
		return Util_sscandouble(ptr, &FILTER_NTSC_setup.sharpness);
	else if (strcmp(option, "FILTER_NTSC_RESOLUTION") == 0)
		return Util_sscandouble(ptr, &FILTER_NTSC_setup.resolution);
	else if (strcmp(option, "FILTER_NTSC_ARTIFACTS") == 0)
		return Util_sscandouble(ptr, &FILTER_NTSC_setup.artifacts);
	else if (strcmp(option, "FILTER_NTSC_FRINGING") == 0)
		return Util_sscandouble(ptr, &FILTER_NTSC_setup.fringing);
	else if (strcmp(option, "FILTER_NTSC_BLEED") == 0)
		return Util_sscandouble(ptr, &FILTER_NTSC_setup.bleed);
	else if (strcmp(option, "FILTER_NTSC_BURST_PHASE") == 0)
		return Util_sscandouble(ptr, &FILTER_NTSC_setup.burst_phase);
	else
		return FALSE;
}

void FILTER_NTSC_WriteConfig(FILE *fp)
{
	fprintf(fp, "FILTER_NTSC_SHARPNESS=%g\n", FILTER_NTSC_setup.sharpness);
	fprintf(fp, "FILTER_NTSC_RESOLUTION=%g\n", FILTER_NTSC_setup.resolution);
	fprintf(fp, "FILTER_NTSC_ARTIFACTS=%g\n", FILTER_NTSC_setup.artifacts);
	fprintf(fp, "FILTER_NTSC_FRINGING=%g\n", FILTER_NTSC_setup.fringing);
	fprintf(fp, "FILTER_NTSC_BLEED=%g\n", FILTER_NTSC_setup.bleed);
	fprintf(fp, "FILTER_NTSC_BURST_PHASE=%g\n", FILTER_NTSC_setup.burst_phase);
}

int FILTER_NTSC_Initialise(int *argc, char *argv[])
{
	int i;
	int j;

	for (i = j = 1; i < *argc; i++) {
		int i_a = (i + 1 < *argc);		/* is argument available? */
		int a_m = FALSE;			/* error, argument missing! */
		
		if (strcmp(argv[i], "-ntsc-sharpness") == 0) {
			if (i_a)
				FILTER_NTSC_setup.sharpness = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-ntsc-resolution") == 0) {
			if (i_a)
				FILTER_NTSC_setup.resolution = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-ntsc-artifacts") == 0) {
			if (i_a)
				FILTER_NTSC_setup.artifacts = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-ntsc-fringing") == 0) {
			if (i_a)
				FILTER_NTSC_setup.fringing = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-ntsc-bleed") == 0) {
			if (i_a)
				FILTER_NTSC_setup.bleed = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-ntsc-burstphase") == 0) {
			if (i_a)
				FILTER_NTSC_setup.burst_phase = atof(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-ntsc-filter-preset") == 0) {
			if (i_a) {
				int idx = CFG_MatchTextParameter(argv[++i], preset_cfg_strings, FILTER_NTSC_PRESET_SIZE);
				if (idx < 0) {
					Log_print("Invalid value for -ntsc-filter-preset");
					return FALSE;
				}
				FILTER_NTSC_SetPreset(idx);
			} else a_m = TRUE;
		}
		else {
			if (strcmp(argv[i], "-help") == 0) {
				Log_print("\t-ntsc-sharpness <n>   Set sharpness for NTSC filter (default %.2g)", FILTER_NTSC_setup.sharpness);
				Log_print("\t-ntsc-resolution <n>  Set resolution for NTSC filter (default %.2g)", FILTER_NTSC_setup.resolution);
				Log_print("\t-ntsc-artifacts <n>   Set luma artifacts ratio for NTSC filter (default %.2g)", FILTER_NTSC_setup.artifacts);
				Log_print("\t-ntsc-fringing <n>    Set chroma fringing ratio for NTSC filter (default %.2g)", FILTER_NTSC_setup.fringing);
				Log_print("\t-ntsc-bleed <n>       Set bleed for NTSC filter (default %.2g)", FILTER_NTSC_setup.bleed);
				Log_print("\t-ntsc-burstphase <n>  Set burst phase (artifact colours) for NTSC filter (default %.2g)", FILTER_NTSC_setup.burst_phase);
				Log_print("\t-ntsc-filter-preset composite|svideo|rgb|monochrome");
				Log_print("\t                      Use one of predefined NTSC filter adjustments");
			}
			argv[j++] = argv[i];
		}

		if (a_m) {
			Log_print("Missing argument for '%s'", argv[i]);
			return FALSE;
		}
	}
	*argc = j;

	return TRUE;
}
