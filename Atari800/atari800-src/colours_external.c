/*
 * colours_external.c - Loading external colour palettes
 *
 * Copyright (C) 1995-1998 David Firth
 * Copyright (C) 1998-2009 Atari800 development team (see DOC/CREDITS)
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
#include <stdio.h>
#include <string.h>

#include "atari.h" /* for TRUE/FALSE */
#include "colours_external.h"
#include "util.h"

int COLOURS_EXTERNAL_Read(COLOURS_EXTERNAL_t *colours)
{
	FILE *fp;
	int i;
	unsigned char *pal_ptr;

	fp = fopen(colours->filename, "rb");
	if (fp == NULL) {
		colours->loaded = FALSE;
		return FALSE;
	}
	for (i = 0, pal_ptr = colours->palette; i < 768; i++) {
		int c = fgetc(fp);
		if (c == EOF) {
			fclose(fp);
			colours->loaded = FALSE;
			return FALSE;
		}
		*pal_ptr++ = c;
	}
	fclose(fp);
	colours->loaded = TRUE;
	return TRUE;
}

void COLOURS_EXTERNAL_Remove(COLOURS_EXTERNAL_t *colours)
{
	colours->loaded = FALSE;
}

int COLOURS_EXTERNAL_ReadFilename(COLOURS_EXTERNAL_t *colours, char *filename)
{
	Util_strlcpy(colours->filename, filename, FILENAME_MAX);
	return COLOURS_EXTERNAL_Read(colours);
}
