/*
 * videomode.c - common code for changing of the display resolution
 *
 * Copyright (C) 2010 Tomasz Krasuski
 * Copyright (C) 2011 Atari800 development team (see DOC/CREDITS)
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

#include "videomode.h"

#if NTSC_FILTER
#include "atari_ntsc/atari_ntsc.h"
#endif
#ifdef AF80
#include "af80.h"
#endif
#include "atari.h"
#include "cfg.h"
#include "config.h"
#include "log.h"
#ifdef PBI_PROTO80
#include "pbi_proto80.h"
#endif
#include "platform.h"
#include "screen.h"
#include "util.h"
#ifdef XEP80_EMULATION
#include "xep80.h"
#endif

#if defined(XEP80_EMULATION) || defined(PBI_PROTO80) || defined(AF80)
#define COLUMN_80 1
#else
#define COLUMN_80 0
#endif

#if NTSC_FILTER
#define VIDEOMODE_MODE_LASTWITHHUD VIDEOMODE_MODE_NTSC_FILTER
#else
#define VIDEOMODE_MODE_LASTWITHHUD VIDEOMODE_MODE_NORMAL
#endif

int VIDEOMODE_windowed = 1;

int VIDEOMODE_horizontal_area = VIDEOMODE_HORIZONTAL_TV;
unsigned int VIDEOMODE_custom_horizontal_area = Screen_WIDTH;
int VIDEOMODE_vertical_area = VIDEOMODE_VERTICAL_TV;
unsigned int VIDEOMODE_custom_vertical_area = Screen_HEIGHT;
int VIDEOMODE_horizontal_offset = 0;
int VIDEOMODE_vertical_offset = 0;
int VIDEOMODE_stretch = VIDEOMODE_STRETCH_INTEGER;
double VIDEOMODE_custom_stretch = 1.0f;
int VIDEOMODE_fit = VIDEOMODE_FIT_BOTH;
int VIDEOMODE_keep_aspect = VIDEOMODE_KEEP_ASPECT_SQUARE_PIXELS;
#if SUPPORTS_ROTATE_VIDEOMODE
int VIDEOMODE_rotate90 = FALSE;
#endif

/* Values of 0.0 indicate that aspect ratio should be autodetected
   in VIDEOMODE_Initialise(). */
double VIDEOMODE_host_aspect_ratio_w = 0.0;
double VIDEOMODE_host_aspect_ratio_h = 0.0;

unsigned int VIDEOMODE_src_offset_left;
unsigned int VIDEOMODE_src_offset_top;
unsigned int VIDEOMODE_src_width;
unsigned int VIDEOMODE_src_height;

unsigned int VIDEOMODE_actual_width;

unsigned int VIDEOMODE_dest_offset_left;
unsigned int VIDEOMODE_dest_offset_top;
unsigned int VIDEOMODE_dest_width;
unsigned int VIDEOMODE_dest_height;

/* To have square pixels in NTSC, the screen should have a pixel clock of
   12+3/11 MHz (actually half of that, because Atari displays non-interlaced
   picture with half the vertical resolution). However Atari pixel clock
   equals 14,31818/2 MHz, which causes its pixels to be taller.
   Similarly for PAL, the square pixel clock is 14.75 MHz and the Atari pixel
   clock equals 14,187576/2 MHz, which gives slightly wider pixels.
   The two values below are used to stretch the screen a bit in order to
   exactly emulate the pixel aspect ratio. In order for this to work,
   VIDEOMODE_host_aspect_ratio_w and VIDEOMODE_host_aspect_ratio_h must be
   properly set.
   Source: http://en.wikipedia.org/wiki/Pixel_aspect_ratio */
static double const pixel_aspect_ratio_ntsc = (12.0 + 3.0/11.0) / 14.31818;
static double const pixel_aspect_ratio_pal = 14.75 / 14.187576;

#ifdef XEP80_EMULATION
/* XEP80 dot clock is 12 MHz. */
static double const xep80_aspect_ratio_ntsc = (12.0 + 3.0/11.0) / 2.0 / 12.0;
static double const xep80_aspect_ratio_pal = 14.75 / 2.0 / 12.0;
#endif /* XEP80_EMULATION */

static char const * const horizontal_area_cfg_strings[VIDEOMODE_HORIZONTAL_SIZE] = {
	"NARROW",
	"TV",
	"FULL"
};
static char const * const vertical_area_cfg_strings[VIDEOMODE_VERTICAL_SIZE] = {
	"SHORT",
	"TV",
	"FULL"
};
static char const * const stretch_cfg_strings[VIDEOMODE_STRETCH_SIZE] = {
	"NONE",
	"2",
	"3",
	"INTEGRAL",
	"FULL"
};
static char const * const fit_cfg_strings[VIDEOMODE_FIT_SIZE] = {
	"WIDTH",
	"HEIGHT",
	"BOTH"
};
static char const * const keep_aspect_cfg_strings[VIDEOMODE_KEEP_ASPECT_SIZE] = {
	"NONE",
	"SQUARE-PIXELS",
	"REAL"
};

typedef struct display_mode_t {
	unsigned int min_w; /* Minimum horizontal display resolution to show this mode */
	unsigned int min_h; /* Minimum vertical display resolution to show this mode */
	unsigned int src_width; /* Horizontal resolution of source image in this mode */
	unsigned int src_height; /* Vertical resolution of source image in this mode */
	int param2src_w_mult; /* Use this multiplier to compute needed horizontal amount of source image based on current "horizontal area" */
	/* Height multiplier for modes whose aspect ratio is far from 1. Blit
	   functions that don't stretch, should anyway stretch their output
	   vertically by this value. */
	int h_mult;
	double asp_ratio; /* Pixel aspect ratio of this mode */
	unsigned int (*src2out_w_func)(unsigned int); /* Function that converts source image width to output image width */
	unsigned int (*out2src_w_func)(unsigned int); /* Function that converts output image width to source image width */
} display_mode_t;

static unsigned int ReturnSame(unsigned int value);
static unsigned int UpscaleWidthNtsc(unsigned int w);
static unsigned int DownscaleWidthNtsc(unsigned int w);

/* TODO determine pixel aspect ratio for 80 column cards. */
static display_mode_t display_modes[VIDEOMODE_MODE_SIZE] = {
	{ 320, 200, Screen_WIDTH, Screen_HEIGHT, 1, 1, 1.0, &ReturnSame, &ReturnSame }
#if NTSC_FILTER
	, { 640, 400, Screen_WIDTH, Screen_HEIGHT, 1, 2, 0.5, &UpscaleWidthNtsc, &DownscaleWidthNtsc, }
#endif
#ifdef XEP80_EMULATION
	, { XEP80_SCRN_WIDTH, 400, XEP80_SCRN_WIDTH, XEP80_MAX_SCRN_HEIGHT, 2, 2, 0.5, &ReturnSame, &ReturnSame }
#endif
#ifdef PBI_PROTO80
	, { 640, 400, 640, 192, 2, 2, 0.5, &ReturnSame, &ReturnSame }
#endif
#ifdef AF80
	, { 640, 400, 640, 250, 2, 2, 0.5, &ReturnSame, &ReturnSame }
#endif
};

#if NTSC_FILTER
int VIDEOMODE_ntsc_filter = FALSE;
#endif
#if COLUMN_80
int VIDEOMODE_80_column = TRUE;
#endif

static int force_windowed = FALSE;
static int force_standard_screen = FALSE;

static VIDEOMODE_resolution_t *resolutions = NULL;
static unsigned int resolutions_size;

static VIDEOMODE_resolution_t window_resolution = { 336, 240 };
/* This variable is used only during initialisation. */
static VIDEOMODE_resolution_t init_fs_resolution = { 336, 240 };
/* Currently chosen fullscreen resolution. */
static unsigned int current_resolution;

unsigned int VIDEOMODE_NumAvailableResolutions(void)
{
	return resolutions_size;
}

void VIDEOMODE_CopyResolutionName(unsigned int res_id, char *target, unsigned int size)
{
	snprintf(target, size, "%ix%i", resolutions[res_id].width, resolutions[res_id].height);
}

unsigned int VIDEOMODE_GetFullscreenResolution(void)
{
	return current_resolution;
}

/* Chooses the current display mode based on current configuration. */
static VIDEOMODE_MODE_t CurrentDisplayMode(void)
{
#if COLUMN_80
	if (!force_standard_screen && VIDEOMODE_80_column) {
#ifdef XEP80_EMULATION
		if (XEP80_enabled)
			return VIDEOMODE_MODE_XEP80;
#endif
#ifdef PBI_PROTO80
		if (PBI_PROTO80_enabled)
			return VIDEOMODE_MODE_PROTO80;
#endif
#ifdef AF80
		if (AF80_enabled)
			return VIDEOMODE_MODE_AF80;
#endif
	}
#endif /* COLUMN_80 */
#if NTSC_FILTER
	if (VIDEOMODE_ntsc_filter)
		return VIDEOMODE_MODE_NTSC_FILTER;
#endif
	return VIDEOMODE_MODE_NORMAL;
}

/* Finds a good fullscreen resolution, taking into consideration defined
   minimal resolutions for various display modes. */
static VIDEOMODE_resolution_t *ResolutionForMode(VIDEOMODE_MODE_t mode, int rotate)
{
	unsigned int i;
	unsigned int w, h;
	if (rotate) {
		w = display_modes[mode].min_h;
		h = display_modes[mode].min_w;
	} else {
		w = display_modes[mode].min_w;
		h = display_modes[mode].min_h;
	}
	if (resolutions[current_resolution].width >= w &&
	    resolutions[current_resolution].height >= h)
		/* Current resolution is OK for this display mode. */
		return &resolutions[current_resolution];

	/* Search for resolution higher than minimal. */
	for (i = 0; i < resolutions_size; i ++) {
		if (resolutions[i].width >= w &&
		    resolutions[i].height >= h)
			return &resolutions[i];
	}

	/* No resolution found! */
	return NULL;
}

static unsigned int ReturnSame(unsigned int value)
{
	return value;
}

#if NTSC_FILTER
static unsigned int UpscaleWidthNtsc(unsigned int w)
{
	return ATARI_NTSC_OUT_WIDTH(w);
}

static unsigned int DownscaleWidthNtsc(unsigned int w)
{
	return ATARI_NTSC_IN_WIDTH(w);
}
#endif /* NTSC_FILTER */

/* Switches RES's width and height. */
static void RotateResolution(VIDEOMODE_resolution_t *res)
{
	unsigned int tmp = res->width;
	res->width = res->height;
	res->height = tmp;
}

/* Updates Screen_visible_* variables to allow proper positioning of
   on-screen indicators (disk drive activity etc.) */
static void SetScreenVisible(void)
{
	if (VIDEOMODE_src_offset_left < 24)
		Screen_visible_x1 = 24;
	else
		Screen_visible_x1 = VIDEOMODE_src_offset_left;
	if (VIDEOMODE_src_offset_left + VIDEOMODE_src_width > 360)
		Screen_visible_x2 = 360;
	else
		Screen_visible_x2 = VIDEOMODE_src_offset_left + VIDEOMODE_src_width;
	Screen_visible_y1 = VIDEOMODE_src_offset_top;
	Screen_visible_y2 = VIDEOMODE_src_offset_top + VIDEOMODE_src_height;
}

/* Computes source screen's visible area based on current settings. */
static void GetOutArea(unsigned int *w, unsigned int *h, VIDEOMODE_MODE_t display_mode)
{
	switch (VIDEOMODE_horizontal_area) {
	case VIDEOMODE_HORIZONTAL_NARROW:
		VIDEOMODE_custom_horizontal_area = 320;
		break;
	case VIDEOMODE_HORIZONTAL_TV:
		VIDEOMODE_custom_horizontal_area = 336;
		break;
	case VIDEOMODE_HORIZONTAL_FULL:
		VIDEOMODE_custom_horizontal_area = VIDEOMODE_MAX_HORIZONTAL_AREA;
		break;
	}
	*w = VIDEOMODE_custom_horizontal_area * display_modes[display_mode].param2src_w_mult;
	if (*w > display_modes[display_mode].src_width)
		*w = display_modes[display_mode].src_width;
	*w = (*display_modes[display_mode].src2out_w_func)(*w);

	switch (VIDEOMODE_vertical_area) {
	case VIDEOMODE_VERTICAL_SHORT:
		VIDEOMODE_custom_vertical_area = 200;
		break;
	case VIDEOMODE_VERTICAL_TV:
		if (Atari800_tv_mode == Atari800_TV_NTSC)
			VIDEOMODE_custom_vertical_area = 224;
		else
			VIDEOMODE_custom_vertical_area = 240;
		break;
	case VIDEOMODE_VERTICAL_FULL:
		VIDEOMODE_custom_vertical_area = VIDEOMODE_MAX_VERTICAL_AREA;
		break;
	}
	*h = VIDEOMODE_custom_vertical_area;
	if (*h > display_modes[display_mode].src_height)
		*h = display_modes[display_mode].src_height;
}

static void UpdateCustomStretch(void)
{
	switch (VIDEOMODE_stretch) {
	case VIDEOMODE_STRETCH_NONE:
	case VIDEOMODE_STRETCH_2X:
	case VIDEOMODE_STRETCH_3X:
		VIDEOMODE_custom_stretch = (double)(VIDEOMODE_stretch + 1);
		break;
	case VIDEOMODE_STRETCH_INTEGER:
	case VIDEOMODE_STRETCH_FULL:
		VIDEOMODE_custom_stretch = 1.0;
		break;
	}
}

/* Computes VIDEOMODE_dest_width/height based on various parameters. */
static void ComputeVideoArea(VIDEOMODE_resolution_t const *res, VIDEOMODE_resolution_t const *screen_res,
                             VIDEOMODE_MODE_t display_mode, unsigned int out_w, unsigned int out_h,
                             double *mult_w, double *mult_h, int rotate)
{
	double asp_ratio;
	/* asp_ratio = <Atari pixel aspect ratio>/<host pixel aspect ratio>.
	   If the screen is to be rotated, then <host pixel aspect ratio> gets inverted. */
	if (VIDEOMODE_keep_aspect == VIDEOMODE_KEEP_ASPECT_REAL) {
		if (rotate)
			asp_ratio = display_modes[display_mode].asp_ratio * screen_res->height / VIDEOMODE_host_aspect_ratio_h
			            * VIDEOMODE_host_aspect_ratio_w / screen_res->width;
		else
			asp_ratio = display_modes[display_mode].asp_ratio * screen_res->width / VIDEOMODE_host_aspect_ratio_w
			            * VIDEOMODE_host_aspect_ratio_h / screen_res->height;
	}
	else
		asp_ratio = 1.0 / display_modes[display_mode].h_mult;

	if (!PLATFORM_SupportsVideomode(display_mode, TRUE, rotate)) {
		*mult_w = 1.0;
		*mult_h = display_modes[display_mode].h_mult;
		VIDEOMODE_dest_width = out_w * *mult_w;
		VIDEOMODE_dest_height = out_h * *mult_h;
		return;
	}

	switch (VIDEOMODE_stretch) {
	case VIDEOMODE_STRETCH_NONE:
	case VIDEOMODE_STRETCH_2X:
	case VIDEOMODE_STRETCH_3X:
	case VIDEOMODE_STRETCH_CUSTOM:
		*mult_w = asp_ratio * VIDEOMODE_custom_stretch;
		*mult_h = VIDEOMODE_custom_stretch;
		break;
	case VIDEOMODE_STRETCH_INTEGER:
		{
			unsigned int imult_w = (double) res->width / out_w / asp_ratio;
			unsigned int imult_h = res->height / out_h;
			if (imult_w == 0)
				imult_w = 1;
			if (imult_h == 0)
				imult_h = 1;
			if (VIDEOMODE_keep_aspect == VIDEOMODE_KEEP_ASPECT_NONE) {
				*mult_w = (double)imult_w * asp_ratio;
			} else {
				switch (VIDEOMODE_fit) {
				case VIDEOMODE_FIT_WIDTH:
					imult_h = imult_w;
					break;
				case VIDEOMODE_FIT_HEIGHT:
					break;
				default: /* VIDEOMODE_FIT_BOTH */
					if (imult_w < imult_h)
						imult_h = imult_w;
				}
				*mult_w = (double)imult_h * asp_ratio;
			}
			*mult_h = imult_h;
		}
		break;
	default: /* VIDEOMODE_STRETCH_FULL */
		{
			double imult_w = (double) res->width / out_w / asp_ratio;
			double imult_h = (double) res->height / out_h;
			if (imult_w < 1.0)
				imult_w = 1.0;
			if (imult_h < 1.0)
				imult_h = 1.0;
			if (VIDEOMODE_keep_aspect == VIDEOMODE_KEEP_ASPECT_NONE) {
				*mult_w = imult_w * asp_ratio;
			} else {
				switch (VIDEOMODE_fit) {
				case VIDEOMODE_FIT_WIDTH:
					imult_h = imult_w;
					break;
				case VIDEOMODE_FIT_HEIGHT:
					break;
				default: /* VIDEOMODE_FIT_BOTH */
					if (imult_w < imult_h)
						imult_h = imult_w;
				}
				*mult_w = imult_h * asp_ratio;
			}
			*mult_h = imult_h;
			
		}
	}
	VIDEOMODE_dest_width = *mult_w * out_w;
	VIDEOMODE_dest_height = *mult_h * out_h;
}

static void CropVideoArea(VIDEOMODE_resolution_t const *screen_res, unsigned int *out_w, unsigned int *out_h, double mult_w, double mult_h)
{
	if (VIDEOMODE_dest_width > screen_res->width) {
		VIDEOMODE_dest_width = screen_res->width;
		*out_w = (double)VIDEOMODE_dest_width / mult_w;
	}
	if (VIDEOMODE_dest_height > screen_res->height) {
		VIDEOMODE_dest_height = screen_res->height;
		*out_h = (double)VIDEOMODE_dest_height / mult_h;
	}
}

/* Computes videomode parameters and calls the platform-specific videomode setup. */
static void SetVideoMode(VIDEOMODE_resolution_t *res, VIDEOMODE_MODE_t display_mode,
                         unsigned int out_w, unsigned int out_h,
                         int windowed, int rotate)
{
	VIDEOMODE_actual_width = out_w;
	VIDEOMODE_src_width = (*display_modes[display_mode].out2src_w_func)(out_w);
	VIDEOMODE_src_height = out_h;
	VIDEOMODE_src_offset_left = (display_modes[display_mode].src_width - VIDEOMODE_src_width) / 2;
	VIDEOMODE_src_offset_top = (display_modes[display_mode].src_height - VIDEOMODE_src_height) / 2;

	if (VIDEOMODE_horizontal_offset < -VIDEOMODE_MAX_HORIZONTAL_AREA)
		VIDEOMODE_horizontal_offset = -VIDEOMODE_MAX_HORIZONTAL_AREA;
	else if (VIDEOMODE_horizontal_offset > VIDEOMODE_MAX_HORIZONTAL_AREA)
		VIDEOMODE_horizontal_offset = VIDEOMODE_MAX_HORIZONTAL_AREA;
	if (VIDEOMODE_vertical_offset < -VIDEOMODE_MAX_VERTICAL_AREA)
		VIDEOMODE_vertical_offset = -VIDEOMODE_MAX_VERTICAL_AREA;
	else if (VIDEOMODE_vertical_offset > VIDEOMODE_MAX_VERTICAL_AREA)
		VIDEOMODE_vertical_offset = VIDEOMODE_MAX_VERTICAL_AREA;
	{
		int x = VIDEOMODE_horizontal_offset * display_modes[display_mode].param2src_w_mult;
		int y = VIDEOMODE_vertical_offset;
		if (x < -(int)VIDEOMODE_src_offset_left)
			x = -(int)VIDEOMODE_src_offset_left;
		else if (x > (int)(display_modes[display_mode].src_width - VIDEOMODE_src_offset_left - VIDEOMODE_src_width))
			x = (int)display_modes[display_mode].src_width - VIDEOMODE_src_offset_left - VIDEOMODE_src_width;
		if (y < -(int)VIDEOMODE_src_offset_top)
			y = -(int)VIDEOMODE_src_offset_top;
		else if (y > (int)(display_modes[display_mode].src_height - VIDEOMODE_src_offset_top - VIDEOMODE_src_height))
			y = (int)display_modes[display_mode].src_height - VIDEOMODE_src_offset_top - VIDEOMODE_src_height;
		VIDEOMODE_src_offset_left += x;
		VIDEOMODE_src_offset_top += y;
	}
	if (rotate) {
		unsigned int tmp = VIDEOMODE_dest_width;
		VIDEOMODE_dest_width = VIDEOMODE_dest_height;
		VIDEOMODE_dest_height = tmp;
		RotateResolution(res);
	}
	VIDEOMODE_dest_offset_left = (res->width - VIDEOMODE_dest_width) / 2;
	VIDEOMODE_dest_offset_top = (res->height - VIDEOMODE_dest_height) / 2;
	if (display_mode <= VIDEOMODE_MODE_LASTWITHHUD)
		SetScreenVisible();
	PLATFORM_SetVideoMode(res, windowed, display_mode, rotate);
}

/* Sets up the fullscreen video mode. */
static int UpdateVideoFullscreen(void)
{
	VIDEOMODE_MODE_t display_mode = CurrentDisplayMode();
#if SUPPORTS_ROTATE_VIDEOMODE
	int rotate = VIDEOMODE_rotate90 && (PLATFORM_SupportsVideomode(display_mode, TRUE, TRUE)
	                                    || PLATFORM_SupportsVideomode(display_mode, FALSE, TRUE));
#else
	int rotate = FALSE;
#endif
	unsigned int out_w, out_h;
	double mult_w, mult_h;
	VIDEOMODE_resolution_t *res_for_mode = ResolutionForMode(display_mode, rotate);
	VIDEOMODE_resolution_t res;
	if (res_for_mode == NULL)
		return FALSE;

	res = *res_for_mode;
	if (rotate)
		RotateResolution(&res);

	GetOutArea(&out_w, &out_h, display_mode);
	UpdateCustomStretch();
	ComputeVideoArea(&res, res_for_mode, display_mode, out_w, out_h, &mult_w, &mult_h, rotate);
	CropVideoArea(&res, &out_w, &out_h, mult_w, mult_h);
	SetVideoMode(&res, display_mode, out_w, out_h, FALSE, rotate);
	return TRUE;
}

/* Sets up the windowed video mode. */
static int UpdateVideoWindowed(int window_resized)
{
	VIDEOMODE_MODE_t display_mode = CurrentDisplayMode();
#if SUPPORTS_ROTATE_VIDEOMODE
	int rotate = VIDEOMODE_rotate90 && (PLATFORM_SupportsVideomode(display_mode, TRUE, TRUE)
	                                    || PLATFORM_SupportsVideomode(display_mode, FALSE, TRUE));
#else
	int rotate = FALSE;
#endif
	unsigned int out_w, out_h;
	double mult_w, mult_h;
	VIDEOMODE_resolution_t desk_res = *PLATFORM_DesktopResolution();
	VIDEOMODE_resolution_t res = window_resolution;
	VIDEOMODE_resolution_t *max_res;
	int maximised = PLATFORM_WindowMaximised();

	if (rotate)
		RotateResolution(&res);

	GetOutArea(&out_w, &out_h, display_mode);
	UpdateCustomStretch();
	ComputeVideoArea(&res, &desk_res, display_mode, out_w, out_h, &mult_w, &mult_h, rotate);
	if (window_resized || maximised)
		/* If the window was user-resized we don't allow it to grow, only shrink. */
		max_res = &res;
	else
		/* Don't allow the window to be larger than the desktop. */
		max_res = &desk_res;
	CropVideoArea(max_res, &out_w, &out_h, mult_w, mult_h);
	if (!window_resized && !maximised) {
		res.width = VIDEOMODE_dest_width;
		res.height = VIDEOMODE_dest_height;
	}
	SetVideoMode(&res, display_mode, out_w, out_h, TRUE, rotate);
	return TRUE;
}

int VIDEOMODE_SetFullscreenResolution(unsigned int res_id)
{
	current_resolution = res_id;
	if (!VIDEOMODE_windowed && !force_windowed)
		return UpdateVideoFullscreen();
	return TRUE;
}

int VIDEOMODE_SetWindowSize(unsigned int width, unsigned int height)
{
	window_resolution.width = width;
	window_resolution.height = height;
	if (VIDEOMODE_windowed || force_windowed)
		return UpdateVideoWindowed(TRUE);
	return TRUE;
}

int VIDEOMODE_Update(void)
{
	if (VIDEOMODE_windowed || force_windowed)
		return UpdateVideoWindowed(FALSE);
	else
		return UpdateVideoFullscreen();
}

/* Sets an integer parameter and updates the video mode if needed. */
static int SetIntAndUpdateVideo(int *ptr, int value)
{
	int old_value = *ptr;
	if (old_value != value) {
		*ptr = value;
		if (!VIDEOMODE_Update()) {
			*ptr = old_value;
			return FALSE;
		}
	}
	return TRUE;
}

int VIDEOMODE_SetWindowed(int value)
{
	return SetIntAndUpdateVideo(&VIDEOMODE_windowed, value);
}

int VIDEOMODE_ToggleWindowed(void)
{
	return VIDEOMODE_SetWindowed(!VIDEOMODE_windowed);
}

void VIDEOMODE_ForceWindowed(int value)
{
	int prev_windowed = VIDEOMODE_windowed || force_windowed;
	force_windowed = value;
	if (prev_windowed != VIDEOMODE_windowed || force_windowed)
		VIDEOMODE_Update();
}

int VIDEOMODE_SetHorizontalArea(int value)
{
	return SetIntAndUpdateVideo(&VIDEOMODE_horizontal_area, value);
}

int VIDEOMODE_ToggleHorizontalArea(void)
{
	return VIDEOMODE_SetHorizontalArea((VIDEOMODE_horizontal_area + 1) % VIDEOMODE_HORIZONTAL_SIZE);
}

int VIDEOMODE_SetCustomHorizontalArea(unsigned int value)
{
	unsigned int old_value = VIDEOMODE_custom_horizontal_area;
	unsigned int old_area = VIDEOMODE_horizontal_area;
	if (value < VIDEOMODE_MIN_HORIZONTAL_AREA)
		return FALSE;
	if (value > VIDEOMODE_MAX_HORIZONTAL_AREA)
		value = VIDEOMODE_MAX_HORIZONTAL_AREA;
	if (value != VIDEOMODE_custom_horizontal_area || VIDEOMODE_horizontal_area != VIDEOMODE_HORIZONTAL_CUSTOM) {
		VIDEOMODE_custom_horizontal_area = value;
		VIDEOMODE_horizontal_area = VIDEOMODE_HORIZONTAL_CUSTOM;
		if (!VIDEOMODE_Update()) {
			VIDEOMODE_custom_horizontal_area = old_value;
			VIDEOMODE_horizontal_area = old_area;
			return FALSE;
		}
	}
	return TRUE;
}

int VIDEOMODE_SetVerticalArea(int value)
{
	return SetIntAndUpdateVideo(&VIDEOMODE_vertical_area, value);
}

int VIDEOMODE_ToggleVerticalArea(void)
{
	return VIDEOMODE_SetVerticalArea((VIDEOMODE_vertical_area + 1) % VIDEOMODE_VERTICAL_SIZE);
}

int VIDEOMODE_SetCustomVerticalArea(unsigned int value)
{
	unsigned int old_value = VIDEOMODE_custom_vertical_area;
	unsigned int old_area = VIDEOMODE_vertical_area;
	if (value < VIDEOMODE_MIN_VERTICAL_AREA)
		return FALSE;
	if (value > VIDEOMODE_MAX_VERTICAL_AREA)
		value = VIDEOMODE_MAX_VERTICAL_AREA;
	if (value != VIDEOMODE_custom_vertical_area || VIDEOMODE_vertical_area != VIDEOMODE_VERTICAL_CUSTOM) {
		VIDEOMODE_custom_vertical_area = value;
		VIDEOMODE_vertical_area = VIDEOMODE_VERTICAL_CUSTOM;
		if (!VIDEOMODE_Update()) {
			VIDEOMODE_custom_vertical_area = old_value;
			VIDEOMODE_vertical_area = old_area;
			return FALSE;
		}
	}
	return TRUE;
}

int VIDEOMODE_SetHorizontalOffset(int value)
{
	return SetIntAndUpdateVideo(&VIDEOMODE_horizontal_offset, value);
}

int VIDEOMODE_SetVerticalOffset(int value)
{
	return SetIntAndUpdateVideo(&VIDEOMODE_vertical_offset, value);
}

int VIDEOMODE_SetStretch(int value)
{
	return SetIntAndUpdateVideo(&VIDEOMODE_stretch, value);
}

int VIDEOMODE_ToggleStretch(void)
{
	return VIDEOMODE_SetStretch((VIDEOMODE_stretch + 1) % VIDEOMODE_STRETCH_SIZE);
}

int VIDEOMODE_SetCustomStretch(double value)
{
	double old_value = VIDEOMODE_custom_stretch;
	unsigned int old_stretch = VIDEOMODE_stretch;
	if (value < 1.0)
		return FALSE;
	if (value != VIDEOMODE_custom_stretch || VIDEOMODE_stretch != VIDEOMODE_STRETCH_CUSTOM) {
		VIDEOMODE_custom_stretch = value;
		VIDEOMODE_stretch = VIDEOMODE_STRETCH_CUSTOM;
		if (!VIDEOMODE_Update()) {
			VIDEOMODE_custom_stretch = old_value;
			VIDEOMODE_stretch = old_stretch;
			return FALSE;
		}
	}
	return TRUE;
}

int VIDEOMODE_SetFit(int value)
{
	return SetIntAndUpdateVideo(&VIDEOMODE_fit, value);
}

int VIDEOMODE_ToggleFit(void)
{
	return VIDEOMODE_SetFit((VIDEOMODE_fit + 1) % VIDEOMODE_FIT_SIZE);
}

int VIDEOMODE_SetKeepAspect(int value)
{
	return SetIntAndUpdateVideo(&VIDEOMODE_keep_aspect, value);
}

int VIDEOMODE_ToggleKeepAspect(void)
{
	return VIDEOMODE_SetKeepAspect((VIDEOMODE_keep_aspect + 1) % VIDEOMODE_KEEP_ASPECT_SIZE);
}

#if SUPPORTS_ROTATE_VIDEOMODE
int VIDEOMODE_SetRotate90(int value)
{
	return SetIntAndUpdateVideo(&VIDEOMODE_rotate90, value);
}

int VIDEOMODE_ToggleRotate90(void)
{
	return VIDEOMODE_SetRotate90(!VIDEOMODE_rotate90);
}
#endif /* SUPPORTS_ROTATE_VIDEOMODE */

#if NTSC_FILTER
int VIDEOMODE_SetNtscFilter(int value)
{
	return SetIntAndUpdateVideo(&VIDEOMODE_ntsc_filter, value);
}

int VIDEOMODE_ToggleNtscFilter(void)
{
	return VIDEOMODE_SetNtscFilter(!VIDEOMODE_ntsc_filter);
}
#endif /* NTSC_FILTER */

#if COLUMN_80
int VIDEOMODE_Set80Column(int value)
{
	return SetIntAndUpdateVideo(&VIDEOMODE_80_column, value);
}

int VIDEOMODE_Toggle80Column(void)
{
	return VIDEOMODE_Set80Column(!VIDEOMODE_80_column);
}
#endif /* COLUMN_80 */

void VIDEOMODE_ForceStandardScreen(int value)
{
	VIDEOMODE_MODE_t prev_mode = CurrentDisplayMode();
	force_standard_screen = value;
	if (prev_mode != CurrentDisplayMode())
		VIDEOMODE_Update();
}

/* Parses string S to get aspect ratio W:H. */
static int ParseAspectRatio(char const *s, double *w, double *h)
{
	char *s2;
	char *s3;
	*w = strtod(s, &s2);
	if (s2 == s || *w < 0.0)
		return FALSE;
	if (*s2 != ':')
		return FALSE;
	*h = strtod(++s2, &s3);
	if (s3 == s2 || *h < 0.0)
		return FALSE;
	return TRUE;
}

/* Find greatest common divisor of M and N. */
static unsigned int gcd(unsigned int m, unsigned int n) {
	unsigned int t;
	if (m < n) {
		t = m;
		m = n;
		n = t;
	}

	for (;;) {
		if (n == 0)
			return m;
		t = n;
		n = m % n;
		m = t;
	}
}

/* Autodetect host aspect ratio and write it into W and H. */
static void AutodetectHostAspect(double *w, double *h)
{
	VIDEOMODE_resolution_t *res = PLATFORM_DesktopResolution();
	unsigned int d = gcd(res->width, res->height);
	*w = (double)res->width / d;
	*h = (double)res->height / d;
}

int VIDEOMODE_SetHostAspect(double w, double h)
{
	double old_w = VIDEOMODE_host_aspect_ratio_w;
	double old_h = VIDEOMODE_host_aspect_ratio_h;
	if (w < 0.0 || h < 0.0)
		return FALSE;
	if (w == 0.0 || h == 0.0)
		AutodetectHostAspect(&w, &h);
	VIDEOMODE_host_aspect_ratio_w = w;
	VIDEOMODE_host_aspect_ratio_h = h;
	if (!VIDEOMODE_Update()) {
		VIDEOMODE_host_aspect_ratio_w = old_w;
		VIDEOMODE_host_aspect_ratio_h = old_h;
		return FALSE;
	}
	return TRUE;
}

int VIDEOMODE_SetHostAspectString(char const *s)
{
	double w, h;
	if (!ParseAspectRatio(s, &w, &h))
		return FALSE;
	return VIDEOMODE_SetHostAspect(w, h);
}

static void UpdateTvSystemSettings(void)
{
	display_modes[VIDEOMODE_MODE_NORMAL].asp_ratio = Atari800_tv_mode == Atari800_TV_PAL ? pixel_aspect_ratio_pal : pixel_aspect_ratio_ntsc;
}

void VIDEOMODE_SetVideoSystem(int mode)
{
	UpdateTvSystemSettings();
	VIDEOMODE_Update();
}

#ifdef XEP80_EMULATION
void VIDEOMODE_UpdateXEP80(void)
{
	display_modes[VIDEOMODE_MODE_XEP80].src_height = XEP80_scrn_height;
	if (XEP80_char_height == 12) /* PAL */
		display_modes[VIDEOMODE_MODE_XEP80].asp_ratio = xep80_aspect_ratio_pal;
	else
		display_modes[VIDEOMODE_MODE_XEP80].asp_ratio = xep80_aspect_ratio_ntsc;
	if (resolutions != NULL /* Display already initialised */
	    && CurrentDisplayMode() == VIDEOMODE_MODE_XEP80)
		VIDEOMODE_Update();
}
#endif /* XEP80_EMULATION */


void VIDEOMODE_CopyHostAspect(char *target, unsigned int size)
{
	snprintf(target, size, "%g:%g", VIDEOMODE_host_aspect_ratio_w, VIDEOMODE_host_aspect_ratio_h);
}

int VIDEOMODE_AutodetectHostAspect(void)
{
	double w, h;
	AutodetectHostAspect(&w, &h);
	return VIDEOMODE_SetHostAspect(w, h);
}

int VIDEOMODE_ReadConfig(char *option, char *ptr)
{
	if (strcmp(option, "VIDEOMODE_WINDOW_WIDTH") == 0)
		return (window_resolution.width = Util_sscandec(ptr)) != -1;
	else if (strcmp(option, "VIDEOMODE_WINDOW_HEIGHT") == 0)
		return (window_resolution.height = Util_sscandec(ptr)) != -1;
	else if (strcmp(option, "VIDEOMODE_FULLSCREEN_WIDTH") == 0)
		return (init_fs_resolution.width = Util_sscandec(ptr)) != -1;
	else if (strcmp(option, "VIDEOMODE_FULLSCREEN_HEIGHT") == 0)
		return (init_fs_resolution.height = Util_sscandec(ptr)) != -1;
	else if (strcmp(option, "VIDEOMODE_WINDOWED") == 0)
		return (VIDEOMODE_windowed = Util_sscanbool(ptr)) != -1;
	else if (strcmp(option, "VIDEOMODE_HORIZONTAL_AREA") == 0) {
		
		int i = CFG_MatchTextParameter(ptr, horizontal_area_cfg_strings, VIDEOMODE_HORIZONTAL_SIZE);
		if (i < 0) {
			VIDEOMODE_horizontal_area = VIDEOMODE_HORIZONTAL_CUSTOM;
			return (VIDEOMODE_custom_horizontal_area = Util_sscandec(ptr)) != -1
			       && VIDEOMODE_custom_horizontal_area >= VIDEOMODE_MIN_HORIZONTAL_AREA
			       && VIDEOMODE_custom_horizontal_area <= VIDEOMODE_MAX_HORIZONTAL_AREA;
		}
		VIDEOMODE_horizontal_area = i;
	}
	else if (strcmp(option, "VIDEOMODE_VERTICAL_AREA") == 0) {
		int i = CFG_MatchTextParameter(ptr, vertical_area_cfg_strings, VIDEOMODE_VERTICAL_SIZE);
		if (i < 0) {
			VIDEOMODE_vertical_area = VIDEOMODE_VERTICAL_CUSTOM;
			return (VIDEOMODE_custom_vertical_area = Util_sscandec(ptr)) != -1
			       && VIDEOMODE_custom_vertical_area >= VIDEOMODE_MIN_VERTICAL_AREA
			       && VIDEOMODE_custom_vertical_area <= VIDEOMODE_MAX_VERTICAL_AREA;
		}
		VIDEOMODE_vertical_area = i;
	}
	else if (strcmp(option, "VIDEOMODE_HORIZONTAL_SHIFT") == 0)
		return Util_sscansdec(ptr, &VIDEOMODE_horizontal_offset);
	else if (strcmp(option, "VIDEOMODE_VERTICAL_SHIFT") == 0)
		return Util_sscansdec(ptr, &VIDEOMODE_vertical_offset);
	else if (strcmp(option, "VIDEOMODE_STRETCH") == 0) {
		int i = CFG_MatchTextParameter(ptr, stretch_cfg_strings, VIDEOMODE_STRETCH_SIZE);
		if (i < 0) {
			VIDEOMODE_stretch = VIDEOMODE_STRETCH_CUSTOM;
			return Util_sscandouble(ptr, &VIDEOMODE_custom_stretch)
			        && VIDEOMODE_custom_stretch >= 1.0;
		}
		VIDEOMODE_stretch = i;
	}
	else if (strcmp(option, "VIDEOMODE_FIT") == 0) {
		int i = CFG_MatchTextParameter(ptr, fit_cfg_strings, VIDEOMODE_FIT_SIZE);
		if (i < 0)
			return FALSE;
		VIDEOMODE_fit = i;
	}
	else if (strcmp(option, "VIDEOMODE_IMAGE_ASPECT") == 0) {
		int i = CFG_MatchTextParameter(ptr, keep_aspect_cfg_strings, VIDEOMODE_KEEP_ASPECT_SIZE);
		if (i < 0)
			return FALSE;
		VIDEOMODE_keep_aspect = i;
	}
#if SUPPORTS_ROTATE_VIDEOMODE
	else if (strcmp(option, "VIDEOMODE_ROTATE90") == 0)
		return (VIDEOMODE_rotate90 = Util_sscanbool(ptr)) != -1;
#endif
	else if (strcmp(option, "VIDEOMODE_HOST_ASPECT_RATIO") == 0) {
		if (strcmp(ptr, "AUTO") == 0)
			VIDEOMODE_host_aspect_ratio_w = VIDEOMODE_host_aspect_ratio_h = 0.0;
		else
			return ParseAspectRatio(ptr, &VIDEOMODE_host_aspect_ratio_w, &VIDEOMODE_host_aspect_ratio_h);
	}
#if NTSC_FILTER
	else if (strcmp(option, "VIDEOMODE_NTSC_FILTER") == 0)
		return (VIDEOMODE_ntsc_filter = Util_sscanbool(ptr)) != -1;
#endif
#if COLUMN_80
	else if (strcmp(option, "VIDEOMODE_80_COLUMN") == 0)
		return (VIDEOMODE_80_column = Util_sscanbool(ptr)) != -1;
#endif
	else
		return FALSE;
	return TRUE;
}

void VIDEOMODE_WriteConfig(FILE *fp) {
	fprintf(fp, "VIDEOMODE_WINDOW_WIDTH=%u\n", window_resolution.width);
	fprintf(fp, "VIDEOMODE_WINDOW_HEIGHT=%u\n", window_resolution.height);
	fprintf(fp, "VIDEOMODE_FULLSCREEN_WIDTH=%u\n", (resolutions == NULL ? init_fs_resolution.width : resolutions[current_resolution].width));
	fprintf(fp, "VIDEOMODE_FULLSCREEN_HEIGHT=%u\n", (resolutions == NULL ? init_fs_resolution.height : resolutions[current_resolution].height));
	fprintf(fp, "VIDEOMODE_WINDOWED=%d\n", VIDEOMODE_windowed);
	if (VIDEOMODE_horizontal_area == VIDEOMODE_HORIZONTAL_CUSTOM)
		fprintf(fp, "VIDEOMODE_HORIZONTAL_AREA=%d\n", VIDEOMODE_custom_horizontal_area);
	else
		fprintf(fp, "VIDEOMODE_HORIZONTAL_AREA=%s\n", horizontal_area_cfg_strings[VIDEOMODE_horizontal_area]);
	if (VIDEOMODE_vertical_area == VIDEOMODE_VERTICAL_CUSTOM)
		fprintf(fp, "VIDEOMODE_VERTICAL_AREA=%d\n", VIDEOMODE_custom_vertical_area);
	else
		fprintf(fp, "VIDEOMODE_VERTICAL_AREA=%s\n", vertical_area_cfg_strings[VIDEOMODE_vertical_area]);
	fprintf(fp, "VIDEOMODE_HORIZONTAL_SHIFT=%d\n", VIDEOMODE_horizontal_offset);
	fprintf(fp, "VIDEOMODE_VERTICAL_SHIFT=%d\n", VIDEOMODE_vertical_offset);
	if (VIDEOMODE_stretch == VIDEOMODE_STRETCH_CUSTOM)
		fprintf(fp, "VIDEOMODE_STRETCH=%g\n", VIDEOMODE_custom_stretch);
	else
		fprintf(fp, "VIDEOMODE_STRETCH=%s\n", stretch_cfg_strings[VIDEOMODE_stretch]);
	fprintf(fp, "VIDEOMODE_FIT=%s\n", fit_cfg_strings[VIDEOMODE_fit]);
	fprintf(fp, "VIDEOMODE_IMAGE_ASPECT=%s\n", keep_aspect_cfg_strings[VIDEOMODE_keep_aspect]);
#if SUPPORTS_ROTATE_VIDEOMODE
	fprintf(fp, "VIDEOMODE_ROTATE90=%d\n", VIDEOMODE_rotate90);
#endif
	fprintf(fp, "VIDEOMODE_HOST_ASPECT_RATIO=%g:%g\n", VIDEOMODE_host_aspect_ratio_w, VIDEOMODE_host_aspect_ratio_h);
#if NTSC_FILTER
	fprintf(fp, "VIDEOMODE_NTSC_FILTER=%d\n", VIDEOMODE_ntsc_filter);
#endif
#if COLUMN_80
	fprintf(fp, "VIDEOMODE_80_COLUMN=%d\n", VIDEOMODE_80_column);
#endif
}

int VIDEOMODE_Initialise(int *argc, char *argv[])
{
	int i, j;
	for (i = j = 1; i < *argc; i++) {
		int i_a = (i + 1 < *argc); /* is argument available? */
		int a_m = FALSE; /* error, argument missing! */
		int a_i = FALSE; /* error, argument invalid! */

		if (strcmp(argv[i], "-win-width") == 0) {
			if (i_a)
				a_i = (window_resolution.width = Util_sscandec(argv[++i])) == -1;
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-win-height") == 0) {
			if (i_a)
				a_i = (window_resolution.height = Util_sscandec(argv[++i])) == -1;
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-fs-width") == 0) {
			if (i_a)
				a_i = (init_fs_resolution.width = Util_sscandec(argv[++i])) == -1;
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-fs-height") == 0) {
			if (i_a)
				a_i = (init_fs_resolution.height = Util_sscandec(argv[++i])) == -1;
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-fullscreen") == 0)
			VIDEOMODE_windowed = FALSE;
		else if (strcmp(argv[i], "-windowed") == 0)
			VIDEOMODE_windowed = TRUE;
		else if (strcmp(argv[i], "-horiz-area") == 0) {
			if (i_a) {
				int idx = CFG_MatchTextParameter(argv[++i], horizontal_area_cfg_strings, VIDEOMODE_HORIZONTAL_SIZE);
				if (idx < 0) {
					VIDEOMODE_horizontal_area = VIDEOMODE_HORIZONTAL_CUSTOM;
					a_i = (VIDEOMODE_custom_horizontal_area = Util_sscandec(argv[i])) == -1
					      || VIDEOMODE_custom_horizontal_area < VIDEOMODE_MIN_HORIZONTAL_AREA
					      || VIDEOMODE_custom_horizontal_area > VIDEOMODE_MAX_HORIZONTAL_AREA;
				} else
					VIDEOMODE_horizontal_area = idx;
			}
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-vert-area") == 0) {
			if (i_a) {
				int idx = CFG_MatchTextParameter(argv[++i], vertical_area_cfg_strings, VIDEOMODE_VERTICAL_SIZE);
				if (idx < 0) {
					VIDEOMODE_vertical_area = VIDEOMODE_VERTICAL_CUSTOM;
					a_i = (VIDEOMODE_custom_vertical_area = Util_sscandec(argv[i])) == -1
					      || VIDEOMODE_custom_vertical_area < VIDEOMODE_MIN_VERTICAL_AREA
					      || VIDEOMODE_custom_vertical_area > VIDEOMODE_MAX_VERTICAL_AREA;
				} else
					VIDEOMODE_vertical_area = idx;
			}
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-horiz-shift") == 0) {
			if (i_a)
				a_i = !Util_sscansdec(argv[++i], &VIDEOMODE_horizontal_offset);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-vert-shift") == 0) {
			if (i_a)
				a_i = !Util_sscansdec(argv[++i], &VIDEOMODE_vertical_offset);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-stretch") == 0) {
			if (i_a) {
				int idx = CFG_MatchTextParameter(argv[++i], stretch_cfg_strings, VIDEOMODE_STRETCH_SIZE);
				if (idx < 0) {
					VIDEOMODE_stretch = VIDEOMODE_STRETCH_CUSTOM;
					a_i = !Util_sscandouble(argv[i], &VIDEOMODE_custom_stretch)
					      || VIDEOMODE_custom_stretch < 1.0;
				} else
					VIDEOMODE_stretch = idx;
			}
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-fit-screen") == 0) {
			if (i_a) {
				if ((VIDEOMODE_fit = CFG_MatchTextParameter(argv[++i], fit_cfg_strings, VIDEOMODE_FIT_SIZE)) < 0)
					a_i = TRUE;
			}
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-image-aspect") == 0) {
			if (i_a) {
				if ((VIDEOMODE_keep_aspect = CFG_MatchTextParameter(argv[++i], keep_aspect_cfg_strings, VIDEOMODE_KEEP_ASPECT_SIZE)) < 0)
					a_i = TRUE;
			}
			else a_m = TRUE;
		}
#if SUPPORTS_ROTATE_VIDEOMODE
		else if (strcmp(argv[i], "-rotate90") == 0)
			VIDEOMODE_rotate90 = TRUE;
		else if (strcmp(argv[i], "-no-rotate90") == 0)
			VIDEOMODE_rotate90 = FALSE;
#endif /* SUPPORTS_ROTATE_VIDEOMODE */
		else if (strcmp(argv[i], "-host-aspect-ratio") == 0) {
			if (i_a) {
				if (strcmp(argv[++i], "auto") == 0)
					VIDEOMODE_host_aspect_ratio_w = VIDEOMODE_host_aspect_ratio_h = 0.0;
				else
					a_i = !ParseAspectRatio(argv[i], &VIDEOMODE_host_aspect_ratio_w, &VIDEOMODE_host_aspect_ratio_h);
			}
			else a_m = TRUE;
		}
#if NTSC_FILTER
		else if (strcmp(argv[i], "-ntscemu") == 0)
			VIDEOMODE_ntsc_filter = TRUE;
		else if (strcmp(argv[i], "-no-ntscemu") == 0)
			VIDEOMODE_ntsc_filter = FALSE;
#endif /* NTSC_FILTER */
#if COLUMN_80
		else if (strcmp(argv[i], "-80column") == 0)
			VIDEOMODE_80_column = TRUE;
		else if (strcmp(argv[i], "-no-80column") == 0)
			VIDEOMODE_80_column = FALSE;
#endif /* COLUMN_80 */
		else {
			if (strcmp(argv[i], "-help") == 0) {
				Log_print("\t-win-width <num>            Host window width");
				Log_print("\t-win-height <num>           Host window height");
				Log_print("\t-fs-width <num>             Host fullscreen width");
				Log_print("\t-fs-height <num>            Host fullscreen height");
				Log_print("\t-fullscreen                 Run fullscreen");
				Log_print("\t-windowed                   Run in window");
				Log_print("\t-horiz-area narrow|tv|full|<number>");
				Log_print("\t                            Set horizontal view area");
				Log_print("\t-vert-area short|tv|full|<number>");
				Log_print("\t                            Set vertical view area");
				Log_print("\t-horiz-shift <num>          Set horizontal shift of the visible area (-%i..%i)", VIDEOMODE_MAX_HORIZONTAL_AREA, VIDEOMODE_MAX_HORIZONTAL_AREA);
				Log_print("\t-vert-shift <num>           Set vertical shift of the visible area (-%i..%i)", VIDEOMODE_MAX_VERTICAL_AREA, VIDEOMODE_MAX_VERTICAL_AREA);
				Log_print("\t-stretch none|integral|full|<number>");
				Log_print("\t                            Set method of image stretching");
				Log_print("\t-fit-screen width|height|both");
				Log_print("\t                            Set method of image fitting the screen");
				Log_print("\t-image-aspect none|square-pixels|real");
				Log_print("\t                            Set image aspect ratio");
#if SUPPORTS_ROTATE_VIDEOMODE
				Log_print("\t-rotate90                   Rotate the screen sideways");
				Log_print("\t-no-rotate90                Don't rotate the screen");
#endif /* SUPPORTS_ROTATE_VIDEOMODE */
				Log_print("\t-host-aspect-ratio auto|<w>:<h>");
				Log_print("\t                            Set host display aspect ratio");
#if NTSC_FILTER
				Log_print("\t-ntscemu                    Enable NTSC composite video filter");
				Log_print("\t-no-ntscemu                 Disable NTSC composite video filter");
#endif
#if COLUMN_80
				Log_print("\t-80column                   Show output of an 80 column card, if present");
				Log_print("\t-no-80column                Show standard screen output");
#endif
			}
			argv[j++] = argv[i];
		}

		if (a_m) {
			Log_print("Missing argument for '%s'", argv[i]);
			return FALSE;
		} else if (a_i) {
			Log_print("Invalid argument for '%s'", argv[--i]);
			return FALSE;
		}
	}
	*argc = j;

#ifdef XEP80_EMULATION
	display_modes[VIDEOMODE_MODE_XEP80].asp_ratio = xep80_aspect_ratio_ntsc;
#endif
	return TRUE;
}

static int CompareResolutions(const void *res1, const void *res2)
{
	unsigned int w = ((const VIDEOMODE_resolution_t *)res1)->width - ((const VIDEOMODE_resolution_t *)res2)->width;
	if (w != 0)
		return w;
	return ((const VIDEOMODE_resolution_t *)res1)->height - ((const VIDEOMODE_resolution_t *)res2)->height;
}

/* Removes duplicate resolutions from the list. */
static void RemoveDuplicateResolutions(void)
{
	unsigned int i, j;
	
	for (i = 0, j = 1; j < resolutions_size; ++j) {
		if (CompareResolutions(&resolutions[i], &resolutions[j]) != 0) {
			++i;
			if (i != j)
				resolutions[i] = resolutions[j];
		}
	}

	resolutions_size -= j - i - 1;
	/* Beware: unused end of RESOLUTIONS is still malloced! */
}

int VIDEOMODE_InitialiseDisplay(void)
{
	/* PLATFORM_Initialise must be called earlier! */
	resolutions = PLATFORM_AvailableResolutions(&resolutions_size);
	if (resolutions == NULL) {
		Log_print("Fatal error: System reports no display resolutions available");
		return FALSE;
	}

	qsort(resolutions, resolutions_size, sizeof(VIDEOMODE_resolution_t), &CompareResolutions);
	RemoveDuplicateResolutions();
	if (resolutions_size == 0) {
		Log_print("Fatal error: System reports no resolution higher than minimal %ux%u available",
		          display_modes[0].min_w, display_modes[0].min_h);
		return FALSE;
	}

	/* Find the resolution from config file/command line in RESOLUTIONS. */
	for (current_resolution = 0; current_resolution < resolutions_size; current_resolution ++) {
		if (resolutions[current_resolution].width >= init_fs_resolution.width &&
		    resolutions[current_resolution].height >= init_fs_resolution.height)
			break;
	}
	if (current_resolution >= resolutions_size) {
		/* No resolution found, using the biggest one. */
		current_resolution = resolutions_size - 1;
		Log_print("Requested resolution %ux%u is too big, using %ux%u instead.",
		          init_fs_resolution.width, init_fs_resolution.height,
		          resolutions[current_resolution].width, resolutions[current_resolution].height);
	} else if (resolutions[current_resolution].width != init_fs_resolution.width ||
	           resolutions[current_resolution].height != init_fs_resolution.height)
		Log_print("Requested resolution %ux%u is not available, using %ux%u instead.",
		          init_fs_resolution.width, init_fs_resolution.height,
		          resolutions[current_resolution].width, resolutions[current_resolution].height);
	/* Autodetect host display aspect ratio if requested. */
	if (VIDEOMODE_host_aspect_ratio_w == 0.0 || VIDEOMODE_host_aspect_ratio_h == 0.0)
		AutodetectHostAspect(&VIDEOMODE_host_aspect_ratio_w, &VIDEOMODE_host_aspect_ratio_h);

	UpdateTvSystemSettings();
	if (!VIDEOMODE_Update()) {
		Log_print("Fatal error: Cannot initialise video");
		return FALSE;
	}
#if SUPPORTS_PLATFORM_PALETTEUPDATE
	PLATFORM_PaletteUpdate();
#endif
	return TRUE;
}

void VIDEOMODE_Exit()
{
	if (resolutions != NULL)
		free(resolutions);
}
