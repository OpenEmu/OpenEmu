/*
 * af80.c - Emulation of the Austin Franklin 80 column card.
 *
 * Copyright (C) 2009 Perry McFarlane
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

#include "af80.h"
#include "atari.h"
#include "util.h"
#include "log.h"
#include "memory.h"
#include "cpu.h"
#include <stdlib.h>

static UBYTE *af80_rom;
static char af80_rom_filename[FILENAME_MAX] = "";
static UBYTE *af80_charset;
static char af80_charset_filename[FILENAME_MAX] = "";

static UBYTE *af80_screen;
static UBYTE *af80_attrib;

int AF80_enabled = FALSE;

/* Austin Franklin information from forum posts by warerat at Atariage */
static int rom_bank_select; /* bits 0-3 of d5f7, $0-$f 16 banks */
static int not_rom_output_enable; /* bit 4 of d5f7 0 = Enable ROM 1 = Disable ROM */
static int not_right_cartridge_rd4_control; /* 0=$8000-$9fff cart ROM, 1= $8000-$9fff system RAM */
static int not_enable_2k_character_ram;
static int not_enable_2k_attribute_ram;
static int not_enable_crtc_registers;
static int not_enable_80_column_output;
static int video_bank_select; /* bits 0-3 of d5f6, $0-$f 16 banks */
static int crtreg[0x40];
static int const rgbi_palette[16] = {
	0x000000, /* black */
	0x0000AA, /* blue */
	0x00AA00, /* green */
	0x00AAAA, /* cyan */
	0xAA0000, /* red */
	0xAA00AA, /* magenta */
	0xAA5500, /* brown */
	0xAAAAAA, /* white */
	0x555555, /* grey */
	0x5555FF, /* light blue */
	0x55FF55, /* light green */
	0x55FFFF, /* light cyan */
	0xFF5555, /* light red */
	0xFF55FF, /* light magenta */
	0xFFFF55, /* yellow */
	0xFFFFFF  /* white (high intensity) */
};
int AF80_palette[16];

#ifdef AF80_DEBUG
#define D(a) a
#else
#define D(a) do{}while(0)
#endif

static void update_d6(void)
{
	if (!not_enable_2k_character_ram) {
		memcpy(MEMORY_mem + 0xd600, af80_screen + (video_bank_select<<7), 0x80);
		memcpy(MEMORY_mem + 0xd680, af80_screen + (video_bank_select<<7), 0x80);
	}
	else if (!not_enable_2k_attribute_ram) {
		memcpy(MEMORY_mem + 0xd600, af80_attrib + (video_bank_select<<7), 0x80);
		memcpy(MEMORY_mem + 0xd680, af80_attrib + (video_bank_select<<7), 0x80);
	}
	else if (not_enable_crtc_registers) {
		memset(MEMORY_mem + 0xd600, 0xff, 0x100);
	}
}

static void update_d5(void)
{
	if (not_rom_output_enable) {
		memset(MEMORY_mem + 0xd500, 0xff, 0x100);
	}
	else {
		memcpy(MEMORY_mem + 0xd500, af80_rom + (rom_bank_select<<8), 0x100);
	}
}

static void update_8000_9fff(void)
{
	if (not_right_cartridge_rd4_control) return;
	if (not_rom_output_enable) {
		memset(MEMORY_mem + 0x8000, 0xff, 0x2000);
	}
	else {
		int i;
		for (i=0; i<32; i++) {
		memcpy(MEMORY_mem + 0x8000 + (i<<8), af80_rom + (rom_bank_select<<8), 0x100);
		}
	}
}

int AF80_Initialise(int *argc, char *argv[])
{
	int i, j;
	for (i = j = 1; i < *argc; i++) {
		if (strcmp(argv[i], "-af80") == 0) {
			Log_print("Austin Franklin 80 enabled");
			AF80_enabled = TRUE;
		}
		else {
		 	if (strcmp(argv[i], "-help") == 0) {
				Log_print("\t-af80            Emulate the Austin Franklin 80 column board");
			}
			argv[j++] = argv[i];
		}
	}
	*argc = j;

	if (AF80_enabled) {
		af80_rom = (UBYTE *)Util_malloc(0x1000);
		if (!Atari800_LoadImage(af80_rom_filename, af80_rom, 0x1000)) {
			free(af80_rom);
			AF80_enabled = FALSE;
			Log_print("Couldn't load Austin Franklin ROM image");
			return FALSE;
		}
		else {
			Log_print("loaded Austin Franklin rom image");
		}
		af80_charset = (UBYTE *)Util_malloc(0x1000);
		if (!Atari800_LoadImage(af80_charset_filename, af80_charset, 0x1000)) {
			free(af80_charset);
			AF80_enabled = FALSE;
			Log_print("Couldn't load Austin Franklin charset image");
			return FALSE;
		}
		else {
			Log_print("loaded Austin Franklin charset image");
		}
		af80_screen = (UBYTE *)Util_malloc(0x800);
		af80_attrib = (UBYTE *)Util_malloc(0x800);
		AF80_Reset();

		/* swap palette */
		for (i=0; i<16; i++ ) {
			j=i;
			j = (j&0x0a) + ((j&0x01) << 2) + ((j&0x04) >> 2);
			AF80_palette[i] = rgbi_palette[j];
		}
	}

	return TRUE;
}

void AF80_InsertRightCartridge(void)
{
		MEMORY_Cart809fEnable();
		update_d5();
		update_8000_9fff();
}

int AF80_ReadConfig(char *string, char *ptr)
{
	if (strcmp(string, "AF80_ROM") == 0)
		Util_strlcpy(af80_rom_filename, ptr, sizeof(af80_rom_filename));
	else if (strcmp(string, "AF80_CHARSET") == 0)
		Util_strlcpy(af80_charset_filename, ptr, sizeof(af80_charset_filename));
	else return FALSE; /* no match */
	return TRUE; /* matched something */
}

void AF80_WriteConfig(FILE *fp)
{
	fprintf(fp, "AF80_ROM=%s\n", af80_rom_filename);
	fprintf(fp, "AF80_CHARSET=%s\n", af80_charset_filename);
}

int AF80_D6GetByte(UWORD addr)
{
	int result = 0xff;
	if (!not_enable_2k_character_ram) {
		result = MEMORY_dGetByte(addr);
	}
	else if (!not_enable_2k_attribute_ram) {
		result = MEMORY_dGetByte(addr);
	}
	else if (!not_enable_crtc_registers) {
		if (video_bank_select == 0 ) {
			if ((addr&0xff)<0x40) {
				result = crtreg[addr&0xff];
				if ((addr&0xff) == 0x3a) {
					result = 0x01;
				}
			}
			D(printf("AF80 Read addr:%4x cpu:%4x\n", addr, CPU_remember_PC[(CPU_remember_PC_curpos-1)%CPU_REMEMBER_PC_STEPS]));
		}
	}
	return result;
}

void AF80_D6PutByte(UWORD addr, UBYTE byte)
{
	if (!not_enable_2k_character_ram) {
		MEMORY_dPutByte((addr&0xff7f),byte);
		MEMORY_dPutByte((addr&0xff7f)+0x80,byte);
		af80_screen[(addr&0x7f) + (video_bank_select<<7)] = byte;
	}
	else if (!not_enable_2k_attribute_ram) {
		MEMORY_dPutByte((addr&0xff7f),byte);
		MEMORY_dPutByte((addr&0xff7f)+0x80,byte);
		af80_attrib[(addr&0x7f) + (video_bank_select<<7)] = byte;
		D(printf("AF80 Write, attribute,  addr:%4x byte:%2x, cpu:%4x\n", addr, byte,CPU_remember_PC[(CPU_remember_PC_curpos-1)%CPU_REMEMBER_PC_STEPS]));
	}
	else if (!not_enable_crtc_registers) {
		if (video_bank_select == 0 ) {
			if ((addr&0xff)<0x40) {
				crtreg[addr&0xff] = byte;
			}
			D(if (1 || (addr!=0xd618 && addr!=0xd619)) printf("AF80 Write addr:%4x byte:%2x, cpu:%4x\n", addr, byte,CPU_remember_PC[(CPU_remember_PC_curpos-1)%CPU_REMEMBER_PC_STEPS]));
		}
		else {
			D(printf("AF80 Write, video_bank_select!=0, addr:%4x byte:%2x, cpu:%4x\n", addr, byte,CPU_remember_PC[(CPU_remember_PC_curpos-1)%CPU_REMEMBER_PC_STEPS]));
		}
	}
}

int AF80_D5GetByte(UWORD addr)
{
	int result = MEMORY_dGetByte(addr);
	return result;
}

void AF80_D5PutByte(UWORD addr, UBYTE byte)
{
	if (addr == 0xd5f6) {
		int need_update_d6 = FALSE;
		if ((byte&0x10) != not_enable_2k_character_ram) {
			not_enable_2k_character_ram = (byte & 0x10);
			need_update_d6 = TRUE;
		}
		if ((byte&0x20) != not_enable_2k_attribute_ram) {
			not_enable_2k_attribute_ram = (byte & 0x20);
			need_update_d6 = TRUE;
		}
		if ((byte&0x40) != not_enable_crtc_registers) {
			not_enable_crtc_registers = (byte & 0x40);
			need_update_d6 = TRUE;
		}
		if ((byte&0x80) != not_enable_80_column_output) {
			not_enable_80_column_output = (byte & 0x80);
		}
		if ((byte&0x0f) != video_bank_select) {
			video_bank_select = (byte & 0x0f);
			need_update_d6 = TRUE;
		}
		if (need_update_d6) {
			update_d6();
		}
	}
	else if (addr == 0xd5f7) {
		int need_update_d5 = FALSE;
		int need_update_8000_9fff = FALSE;
		if ((byte&0x10) != not_rom_output_enable) {
			not_rom_output_enable = (byte & 0x10);
			need_update_d5 = TRUE;
			if (byte&0x20) {
				need_update_8000_9fff = TRUE;
			}
		}
		if ((byte&0x20) != not_right_cartridge_rd4_control) {
			not_right_cartridge_rd4_control = (byte & 0x20);
			if (not_right_cartridge_rd4_control) {
				MEMORY_Cart809fDisable();
			}
			else {
				MEMORY_Cart809fEnable();
				need_update_8000_9fff = TRUE;
			}
		}
		if ((byte&0x0f) != rom_bank_select) {
			rom_bank_select = (byte & 0x0f);
			if (!not_rom_output_enable) {
				need_update_d5 = TRUE;
				if (!not_right_cartridge_rd4_control) {
					need_update_8000_9fff = TRUE;
				}
			}
		}
		if (need_update_d5) {
			update_d5();
		}
		if (need_update_8000_9fff) {
			update_8000_9fff();
		}
	}
	D(if (addr!=0xd5f7 && addr!=0xd5f6) printf("AF80 Write addr:%4x byte:%2x, cpu:%4x\n", addr, byte,CPU_remember_PC[(CPU_remember_PC_curpos-1)%CPU_REMEMBER_PC_STEPS]));
}

UBYTE AF80_GetPixels(int scanline, int column, int *colour, int blink)
{
#define AF80_ROWS 25
#define AF80_CELL_HEIGHT 10
	UBYTE character;
	int attrib;
	UBYTE font_data;
	int table_start = crtreg[0x0c] + ((crtreg[0x0d]&0x3f)<<8);
	int row = scanline / AF80_CELL_HEIGHT;
	int line = scanline % AF80_CELL_HEIGHT;
	int screen_pos;
	if (row  >= AF80_ROWS) {
		return 0;
	}

	if (row >= crtreg[0x10]) {
		screen_pos = (row-crtreg[0x10])*80 + column + crtreg[0x0e] + ((crtreg[0x0f]&0x3f)<<8);
	}
	else {
		screen_pos = row*80+column + table_start;
	}
	screen_pos &= 0x7ff;
	character = af80_screen[screen_pos];
	attrib = af80_attrib[screen_pos];
	font_data = af80_charset[character*16 + line];
	if (attrib & 0x01) {
	   	font_data ^= 0xff; /* invert */
	}
	if ((attrib & 0x02) && blink) {
	   	font_data = 0x00; /* blink */
	}
	if (line+1 == AF80_CELL_HEIGHT && (attrib & 0x04)) {
		font_data = 0xff; /* underline */
	}
	if (row == crtreg[0x18] && column == crtreg[0x19] && !blink) {
		font_data = 0xff; /* cursor */
	}
	*colour = attrib>>4; /* set number of palette entry */
	return font_data;
}

void AF80_Reset(void)
{
	memset(af80_screen, 0, 0x800);
	memset(af80_attrib, 0, 0x800);
	rom_bank_select = 0;
	not_rom_output_enable = 0;
	not_right_cartridge_rd4_control = 0;
	not_enable_2k_character_ram = 0;
	not_enable_2k_attribute_ram = 0;
	not_enable_crtc_registers = 0;
	not_enable_80_column_output = 0;
	video_bank_select = 0;
	memset(crtreg, 0, 0x40);
}

/*
vim:ts=4:sw=4:
*/
