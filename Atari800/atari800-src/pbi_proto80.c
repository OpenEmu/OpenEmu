/*
 * pbi_proto.c - Emulation of a prototype 80 column board for the
 * Atari 1090 expansion interface.
 *
 * Copyright (C) 2007-2008 Perry McFarlane
 * Copyright (C) 2002-2008 Atari800 development team (see DOC/CREDITS)
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

#include "atari.h"
#include "pbi.h"
#include "pbi_proto80.h"
#include "util.h"
#include "log.h"
#include "memory.h"
#include <stdlib.h>

#define PROTO80_PBI_NUM 2
#define PROTO80_MASK (1 << PROTO80_PBI_NUM)

static UBYTE *proto80rom;
static char proto80_rom_filename[FILENAME_MAX] = "";

int PBI_PROTO80_enabled = FALSE;

#ifdef PBI_DEBUG
#define D(a) a
#else
#define D(a) do{}while(0)
#endif

int PBI_PROTO80_Initialise(int *argc, char *argv[])
{
	int i, j;
	for (i = j = 1; i < *argc; i++) {
		if (strcmp(argv[i], "-proto80") == 0) {
			Log_print("proto80 enabled");
			PBI_PROTO80_enabled = TRUE;
		}
		else {
		 	if (strcmp(argv[i], "-help") == 0) {
				Log_print("\t-proto80         Emulate a prototype 80 column board for the 1090");
			}
			argv[j++] = argv[i];
		}
	}
	*argc = j;

	if (PBI_PROTO80_enabled) {
		proto80rom = (UBYTE *)Util_malloc(0x800);
		if (!Atari800_LoadImage(proto80_rom_filename, proto80rom, 0x800)) {
			free(proto80rom);
			PBI_PROTO80_enabled = FALSE;
			Log_print("Couldn't load proto80 rom image");
			return FALSE;
		}
		else {
			Log_print("loaded proto80 rom image");
			PBI_D6D7ram = TRUE;
		}
	}

	return TRUE;
}

int PBI_PROTO80_ReadConfig(char *string, char *ptr)
{
	if (strcmp(string, "PROTO80_ROM") == 0)
		Util_strlcpy(proto80_rom_filename, ptr, sizeof(proto80_rom_filename));
	else return FALSE; /* no match */
	return TRUE; /* matched something */
}

void PBI_PROTO80_WriteConfig(FILE *fp)
{
	fprintf(fp, "PROTO80_ROM=%s\n", proto80_rom_filename);
}

int PBI_PROTO80_D1GetByte(UWORD addr)
{
	int result = PBI_NOT_HANDLED;
	if (PBI_PROTO80_enabled) {
	}
	return result;
}

void PBI_PROTO80_D1PutByte(UWORD addr, UBYTE byte)
{

}

int PBI_PROTO80_D1ffPutByte(UBYTE byte)
{
	int result = 0; /* handled */
	if (PBI_PROTO80_enabled && byte == PROTO80_MASK) {
		memcpy(MEMORY_mem + 0xd800, proto80rom, 0x800);
		D(printf("PROTO80 rom activated\n"));
	}
	else result = PBI_NOT_HANDLED;
	return result;
}

UBYTE PBI_PROTO80_GetPixels(int scanline, int column)
{
#define PROTO80_ROWS 24
#define PROTO80_CELL_HEIGHT 8
	UBYTE character;
	UBYTE invert;
	UBYTE font_data;
	int row = scanline / PROTO80_CELL_HEIGHT;
	int line = scanline % PROTO80_CELL_HEIGHT;
	if (row  >= PROTO80_ROWS) {
		return 0;
	}
	character = MEMORY_mem[0x9800 + row*80 + column];
	invert = 0x00;
	if (character & 0x80) {
		invert = 0xff;
		character &= 0x7f;
	}
	font_data = MEMORY_mem[0xe000 + character*8 + line];
	font_data ^= invert;
	return font_data;
}

/*
vim:ts=4:sw=4:
*/
