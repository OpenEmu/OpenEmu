/*
 * cartridge.c - cartridge emulation
 *
 * Copyright (C) 2001-2010 Piotr Fusik
 * Copyright (C) 2001-2010 Atari800 development team (see DOC/CREDITS)
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
#include <stdlib.h>
#include <string.h>

#include "atari.h"
#include "binload.h" /* BINLOAD_loading_basic */
#include "cartridge.h"
#include "memory.h"
#ifdef IDE
#  include "ide.h"
#endif
#include "pia.h"
#include "rtime.h"
#include "util.h"
#ifndef BASIC
#include "statesav.h"
#endif
#ifdef AF80
#include "af80.h"
#endif
#include "log.h"

int CARTRIDGE_kb[CARTRIDGE_LAST_SUPPORTED + 1] = {
	0,
	8,    /* CARTRIDGE_STD_8 */
	16,   /* CARTRIDGE_STD_16 */
	16,   /* CARTRIDGE_OSS_16 */
	32,   /* CARTRIDGE_5200_32 */
	32,   /* CARTRIDGE_DB_32 */
	16,   /* CARTRIDGE_5200_EE_16 */
	40,   /* CARTRIDGE_5200_40 */
	64,   /* CARTRIDGE_WILL_64 */
	64,   /* CARTRIDGE_EXP_64 */
	64,   /* CARTRIDGE_DIAMOND_64 */
	64,   /* CARTRIDGE_SDX */
	32,   /* CARTRIDGE_XEGS_32 */
	64,   /* CARTRIDGE_XEGS_64 */
	128,  /* CARTRIDGE_XEGS_128 */
	16,   /* CARTRIDGE_OSS2_16 */
	16,   /* CARTRIDGE_5200_NS_16 */
	128,  /* CARTRIDGE_ATRAX_128 */
	40,   /* CARTRIDGE_BBSB_40 */
	8,    /* CARTRIDGE_5200_8 */
	4,    /* CARTRIDGE_5200_4 */
	8,    /* CARTRIDGE_RIGHT_8 */
	32,   /* CARTRIDGE_WILL_32 */
	256,  /* CARTRIDGE_XEGS_256 */
	512,  /* CARTRIDGE_XEGS_512 */
	1024, /* CARTRIDGE_XEGS_1024 */
	16,   /* CARTRIDGE_MEGA_16 */
	32,   /* CARTRIDGE_MEGA_32 */
	64,   /* CARTRIDGE_MEGA_64 */
	128,  /* CARTRIDGE_MEGA_128 */
	256,  /* CARTRIDGE_MEGA_256 */
	512,  /* CARTRIDGE_MEGA_512 */
	1024, /* CARTRIDGE_MEGA_1024 */
	32,   /* CARTRIDGE_SWXEGS_32 */
	64,   /* CARTRIDGE_SWXEGS_64 */
	128,  /* CARTRIDGE_SWXEGS_128 */
	256,  /* CARTRIDGE_SWXEGS_256 */
	512,  /* CARTRIDGE_SWXEGS_512 */
	1024, /* CARTRIDGE_SWXEGS_1024 */
	8,    /* CARTRIDGE_PHOENIX_8 */
	16,   /* CARTRIDGE_BLIZZARD_16 */
	128,  /* CARTRIDGE_ATMAX_128 */
	1024, /* CARTRIDGE_ATMAX_1024 */
	128   /* CARTRIDGE_SDX_128 */
};

int CARTRIDGE_IsFor5200(int type)
{
	switch (type) {
	case CARTRIDGE_5200_32:
	case CARTRIDGE_5200_EE_16:
	case CARTRIDGE_5200_40:
	case CARTRIDGE_5200_NS_16:
	case CARTRIDGE_5200_8:
	case CARTRIDGE_5200_4:
		return TRUE;
	default:
		break;
	}
	return FALSE;
}

static UBYTE *cart_image = NULL;		/* cartridge memory */
static UBYTE *second_cart_image = NULL; /* Pass through cartridge memory for SpartaDOSX */
static UBYTE *first_cart_image = NULL; /* Pointer for orignal cart image will using pass through */
static char cart_filename[FILENAME_MAX];
static char second_cart_filename[FILENAME_MAX];
int CARTRIDGE_type = CARTRIDGE_NONE;
int CARTRIDGE_second_type = CARTRIDGE_NONE;
static int cart_pass_through = FALSE;

static int bank;
static void CARTRIDGE_Start_Local(int curr_cart_type);

/* DB_32, XEGS_32, XEGS_64, XEGS_128, XEGS_256, XEGS_512, XEGS_1024,
   SWXEGS_32, SWXEGS_64, SWXEGS_128, SWXEGS_256, SWXEGS_512, SWXEGS_1024 */
static void set_bank_809F(int b, int main)
{
	if (b != bank) {
		if (b & 0x80) {
			MEMORY_Cart809fDisable();
			MEMORY_CartA0bfDisable();
		}
		else {
			MEMORY_Cart809fEnable();
			MEMORY_CartA0bfEnable();
			MEMORY_CopyROM(0x8000, 0x9fff, cart_image + b * 0x2000);
			if (bank & 0x80)
				MEMORY_CopyROM(0xa000, 0xbfff, cart_image + main);
		}
		bank = b;
	}
}

/* OSS_16, OSS2_16 */
static void set_bank_A0AF(int b, int main)
{
	if (b != bank) {
		if (b < 0)
			MEMORY_CartA0bfDisable();
		else {
			MEMORY_CartA0bfEnable();
			MEMORY_CopyROM(0xa000, 0xafff, cart_image + b * 0x1000);
			if (bank < 0)
				MEMORY_CopyROM(0xb000, 0xbfff, cart_image + main);
		}
		bank = b;
	}
}

/* WILL_64, EXP_64, DIAMOND_64, SDX_64, WILL_32, ATMAX_128, ATMAX_1024,
   ATRAX_128 */
static void set_bank_A0BF(int b, int n)
{
	if (b != bank) {
		if (b & n)
			MEMORY_CartA0bfDisable();
		else {
			MEMORY_CartA0bfEnable();
			MEMORY_CopyROM(0xa000, 0xbfff, cart_image + (b & (n - 1)) * 0x2000);
		}
		bank = b;
	}
}

static void set_bank_80BF(int b)
{
	if (b != bank) {
		if (b & 0x80) {
			MEMORY_Cart809fDisable();
			MEMORY_CartA0bfDisable();
		}
		else {
			MEMORY_Cart809fEnable();
			MEMORY_CartA0bfEnable();
			MEMORY_CopyROM(0x8000, 0xbfff, cart_image + b * 0x4000);
		}
		bank = b;
	}
}

static void set_bank_SDX_128(UWORD addr)
{
	if (addr & 8)
		MEMORY_CartA0bfDisable();
	else {
		MEMORY_CartA0bfEnable();
		MEMORY_CopyROM(0xa000, 0xbfff,
			cart_image + (((addr & 7) + ((addr & 0x10) >> 1)) ^ 0xf) * 0x2000);
	}
	bank = addr;
}

/* an access (read or write) to D500-D5FF area */
static void access_D5(int curr_cart_type,UWORD addr)
{
	int b = bank;
	switch (curr_cart_type) {
	case CARTRIDGE_OSS_16:
		if (addr & 0x08)
			b = -1;
		else
			switch (addr & 0x07) {
			case 0x00:
			case 0x01:
				b = 0;
				break;
			case 0x03:
			case 0x07:
				b = 1;
				break;
			case 0x04:
			case 0x05:
				b = 2;
				break;
			/* case 0x02:
			case 0x06: */
			default:
				break;
			}
		set_bank_A0AF(b, 0x3000);
		break;
	case CARTRIDGE_DB_32:
		set_bank_809F(addr & 0x03, 0x6000);
		break;
	case CARTRIDGE_WILL_64:
		set_bank_A0BF(addr, 8);
		break;
	case CARTRIDGE_WILL_32:
		set_bank_A0BF(addr & 0xb, 8);
		break;
	case CARTRIDGE_EXP_64:
		if ((addr & 0xf0) == 0x70)
			set_bank_A0BF(addr ^ 7, 8);
		break;
	case CARTRIDGE_DIAMOND_64:
		if ((addr & 0xf0) == 0xd0)
			set_bank_A0BF(addr ^ 7, 8);
		break;
	case CARTRIDGE_SDX_64:
		if (cart_pass_through) {
			if ((addr & 0xff) == 8) {
				cart_pass_through = FALSE;
				cart_image = first_cart_image;
				MEMORY_CopyROM(0xa000, 0xbfff, cart_image);
				MEMORY_CartA0bfDisable();
			}
			else if ((addr & 0xf0) == 0xe0) {
				cart_pass_through = FALSE;
				cart_image = first_cart_image;
				set_bank_A0BF(addr ^ 7, 8);
			}
			else {
				access_D5(CARTRIDGE_second_type, addr);
		        }
		}
		else {
			if ((addr & 0xf0) == 0xe0) {
				set_bank_A0BF(addr ^ 7, 8);
			}
			else if ((addr & 0xff) == 0) {
				cart_pass_through = TRUE;
				first_cart_image = cart_image;
				if (second_cart_image) {
				    cart_image = second_cart_image;
				    CARTRIDGE_Start_Local(CARTRIDGE_second_type);
				}
			}
		}
		break;
	case CARTRIDGE_SDX_128:
		if (cart_pass_through) {
			if ((addr & 0xff) == 8) {
				cart_pass_through = FALSE;
				cart_image = first_cart_image;
				set_bank_SDX_128(0);
			}
			else if ((addr & 0xe0) == 0xe0) {
				cart_pass_through = FALSE;
				cart_image = first_cart_image;
				set_bank_SDX_128(addr);
			}
			else {
				access_D5(CARTRIDGE_second_type, addr);
		        }
		}
		else {
			if ((addr & 0xe0) == 0xe0)  {
				if (addr != bank)
					set_bank_SDX_128(addr);
			}
			else if ((addr & 0xff) == 0) {
				cart_pass_through = TRUE;
				first_cart_image = cart_image;
				if (second_cart_image) {
				    cart_image = second_cart_image;
				    CARTRIDGE_Start_Local(CARTRIDGE_second_type);
				}
			}
		}
		break;
	case CARTRIDGE_OSS2_16:
		switch (addr & 0x09) {
		case 0x00:
			b = 1;
			break;
		case 0x01:
			b = 3;
			break;
		case 0x08:
			b = -1;
			break;
		case 0x09:
			b = 2;
			break;
		}
		set_bank_A0AF(b, 0x0000);
		break;
	case CARTRIDGE_PHOENIX_8:
		MEMORY_CartA0bfDisable();
		break;
	case CARTRIDGE_BLIZZARD_16:
		MEMORY_Cart809fDisable();
		MEMORY_CartA0bfDisable();
		break;
	case CARTRIDGE_ATMAX_128:
		if ((addr & 0xe0) == 0)
			set_bank_A0BF(addr, 16);
		break;
	case CARTRIDGE_ATMAX_1024:
		set_bank_A0BF(addr, 128);
		break;
	default:
		break;
	}
}

/* a read from D500-D5FF area */
UBYTE CARTRIDGE_GetByte(UWORD addr)
{
#ifdef AF80
	if (AF80_enabled) {
		return AF80_D5GetByte(addr);
	}
#endif
	if (RTIME_enabled && (addr == 0xd5b8 || addr == 0xd5b9))
		return RTIME_GetByte();
#ifdef IDE
	if (IDE_enabled && (addr <= 0xd50f))
		return IDE_GetByte(addr);
#endif
	access_D5(CARTRIDGE_type, addr);
	return 0xff;
}

/* a write to D500-D5FF area */
void CARTRIDGE_PutByte(UWORD addr, UBYTE byte)
{
	int curr_cart_type;
	
#ifdef AF80
	if (AF80_enabled) {
		AF80_D5PutByte(addr,byte);
		return;
	}
#endif
	if (RTIME_enabled && (addr == 0xd5b8 || addr == 0xd5b9)) {
		RTIME_PutByte(byte);
		return;
	}
#ifdef IDE
	if (IDE_enabled && (addr <= 0xd50f)) {
		IDE_PutByte(addr,byte);
		return;
	}
#endif
	
	if (cart_pass_through)
	    curr_cart_type = CARTRIDGE_second_type;
	else
		curr_cart_type = CARTRIDGE_type;
		
	switch (curr_cart_type) {
	case CARTRIDGE_XEGS_32:
		set_bank_809F(byte & 0x03, 0x6000);
		break;
	case CARTRIDGE_XEGS_64:
		set_bank_809F(byte & 0x07, 0xe000);
		break;
	case CARTRIDGE_XEGS_128:
		set_bank_809F(byte & 0x0f, 0x1e000);
		break;
	case CARTRIDGE_XEGS_256:
		set_bank_809F(byte & 0x1f, 0x3e000);
		break;
	case CARTRIDGE_XEGS_512:
		set_bank_809F(byte & 0x3f, 0x7e000);
		break;
	case CARTRIDGE_XEGS_1024:
		set_bank_809F(byte & 0x7f, 0xfe000);
		break;
	case CARTRIDGE_ATRAX_128:
		set_bank_A0BF(byte & 0x8f, 128);
		break;
	case CARTRIDGE_MEGA_16:
		set_bank_80BF(byte & 0x80);
		break;
	case CARTRIDGE_MEGA_32:
		set_bank_80BF(byte & 0x81);
		break;
	case CARTRIDGE_MEGA_64:
		set_bank_80BF(byte & 0x83);
		break;
	case CARTRIDGE_MEGA_128:
		set_bank_80BF(byte & 0x87);
		break;
	case CARTRIDGE_MEGA_256:
		set_bank_80BF(byte & 0x8f);
		break;
	case CARTRIDGE_MEGA_512:
		set_bank_80BF(byte & 0x9f);
		break;
	case CARTRIDGE_MEGA_1024:
		set_bank_80BF(byte & 0xbf);
		break;
	case CARTRIDGE_SWXEGS_32:
		set_bank_809F(byte & 0x83, 0x6000);
		break;
	case CARTRIDGE_SWXEGS_64:
		set_bank_809F(byte & 0x87, 0xe000);
		break;
	case CARTRIDGE_SWXEGS_128:
		set_bank_809F(byte & 0x8f, 0x1e000);
		break;
	case CARTRIDGE_SWXEGS_256:
		set_bank_809F(byte & 0x9f, 0x3e000);
		break;
	case CARTRIDGE_SWXEGS_512:
		set_bank_809F(byte & 0xbf, 0x7e000);
		break;
	case CARTRIDGE_SWXEGS_1024:
		set_bank_809F(byte, 0xfe000);
		break;
	default:
		access_D5(CARTRIDGE_type, addr);
		break;
	}
}

/* special support of Bounty Bob on Atari5200 */
void CARTRIDGE_BountyBob1(UWORD addr)
{
	if (Atari800_machine_type == Atari800_MACHINE_5200) {
		if (addr >= 0x4ff6 && addr <= 0x4ff9) {
			addr -= 0x4ff6;
			MEMORY_CopyROM(0x4000, 0x4fff, cart_image + addr * 0x1000);
		}
	} else {
		if (addr >= 0x8ff6 && addr <= 0x8ff9) {
			addr -= 0x8ff6;
			MEMORY_CopyROM(0x8000, 0x8fff, cart_image + addr * 0x1000);
		}
	}
}

void CARTRIDGE_BountyBob2(UWORD addr)
{
	if (Atari800_machine_type == Atari800_MACHINE_5200) {
		if (addr >= 0x5ff6 && addr <= 0x5ff9) {
			addr -= 0x5ff6;
			MEMORY_CopyROM(0x5000, 0x5fff, cart_image + 0x4000 + addr * 0x1000);
		}
	}
	else {
		if (addr >= 0x9ff6 && addr <= 0x9ff9) {
			addr -= 0x9ff6;
			MEMORY_CopyROM(0x9000, 0x9fff, cart_image + 0x4000 + addr * 0x1000);
		}
	}
}

#ifdef PAGED_ATTRIB
UBYTE CARTRIDGE_BountyBob1GetByte(UWORD addr)
{
	if (Atari800_machine_type == Atari800_MACHINE_5200) {
		if (addr >= 0x4ff6 && addr <= 0x4ff9) {
			CARTRIDGE_BountyBob1(addr);
			return 0;
		}
	} else {
		if (addr >= 0x8ff6 && addr <= 0x8ff9) {
			CARTRIDGE_BountyBob1(addr);
			return 0;
		}
	}
	return MEMORY_dGetByte(addr);
}

UBYTE CARTRIDGE_BountyBob2GetByte(UWORD addr)
{
	if (Atari800_machine_type == Atari800_MACHINE_5200) {
		if (addr >= 0x5ff6 && addr <= 0x5ff9) {
			CARTRIDGE_BountyBob2(addr);
			return 0;
		}
	} else {
		if (addr >= 0x9ff6 && addr <= 0x9ff9) {
			CARTRIDGE_BountyBob2(addr);
			return 0;
		}
	}
	return MEMORY_dGetByte(addr);
}

void CARTRIDGE_BountyBob1PutByte(UWORD addr, UBYTE value)
{
	if (Atari800_machine_type == Atari800_MACHINE_5200) {
		if (addr >= 0x4ff6 && addr <= 0x4ff9) {
			CARTRIDGE_BountyBob1(addr);
		}
	} else {
		if (addr >= 0x8ff6 && addr <= 0x8ff9) {
			CARTRIDGE_BountyBob1(addr);
		}
	}
}

void CARTRIDGE_BountyBob2PutByte(UWORD addr, UBYTE value)
{
	if (Atari800_machine_type == Atari800_MACHINE_5200) {
		if (addr >= 0x5ff6 && addr <= 0x5ff9) {
			CARTRIDGE_BountyBob2(addr);
		}
	} else {
		if (addr >= 0x9ff6 && addr <= 0x9ff9) {
			CARTRIDGE_BountyBob2(addr);
		}
	}
}
#endif

int CARTRIDGE_Checksum(const UBYTE *image, int nbytes)
{
	int checksum = 0;
	while (nbytes > 0) {
		checksum += *image++;
		nbytes--;
	}
	return checksum;
}

int CARTRIDGE_Insert(const char *filename)
{
	FILE *fp;
	int len;
	int type;
	UBYTE header[16];

	/* remove currently inserted cart */
	CARTRIDGE_Remove();

	/* open file */
	fp = fopen(filename, "rb");
	if (fp == NULL)
		return CARTRIDGE_CANT_OPEN;
	/* check file length */
	len = Util_flen(fp);
	Util_rewind(fp);

	/* Save Filename for state save */
	strcpy(cart_filename, filename);

	/* if full kilobytes, assume it is raw image */
	if ((len & 0x3ff) == 0) {
		/* alloc memory and read data */
		cart_image = (UBYTE *) Util_malloc(len);
		if (fread(cart_image, 1, len, fp) < len) {
			Log_print("Error reading cartridge.\n");
		}
		fclose(fp);
		/* find cart type */
		CARTRIDGE_type = CARTRIDGE_NONE;
		len >>= 10;	/* number of kilobytes */
		for (type = 1; type <= CARTRIDGE_LAST_SUPPORTED; type++)
			if (CARTRIDGE_kb[type] == len) {
				if (CARTRIDGE_type == CARTRIDGE_NONE)
					CARTRIDGE_type = type;
				else
					return len;	/* more than one cartridge type of such length - user must select */
			}
		if (CARTRIDGE_type != CARTRIDGE_NONE) {
			CARTRIDGE_Start();
			return 0;	/* ok */
		}
		free(cart_image);
		cart_image = NULL;
		return CARTRIDGE_BAD_FORMAT;
	}
	/* if not full kilobytes, assume it is CART file */
	if (fread(header, 1, 16, fp) < 16) {
		Log_print("Error reading cartridge.\n");
	}
	if ((header[0] == 'C') &&
		(header[1] == 'A') &&
		(header[2] == 'R') &&
		(header[3] == 'T')) {
		type = (header[4] << 24) |
			(header[5] << 16) |
			(header[6] << 8) |
			header[7];
		if (type >= 1 && type <= CARTRIDGE_LAST_SUPPORTED) {
			int checksum;
			len = CARTRIDGE_kb[type] << 10;
			/* alloc memory and read data */
			cart_image = (UBYTE *) Util_malloc(len);
			if (fread(cart_image, 1, len, fp) < len) {
				Log_print("Error reading cartridge.\n");
			}
			fclose(fp);
			checksum = (header[8] << 24) |
				(header[9] << 16) |
				(header[10] << 8) |
				header[11];
			CARTRIDGE_type = type;
			CARTRIDGE_Start();
			return checksum == CARTRIDGE_Checksum(cart_image, len) ? 0 : CARTRIDGE_BAD_CHECKSUM;
		}
	}
	fclose(fp);
	return CARTRIDGE_BAD_FORMAT;
}

int CARTRIDGE_Insert_Second(const char *filename)
{
	FILE *fp;
	int len;
	int type;
	UBYTE header[16];

	/* remove currently inserted cart */
	CARTRIDGE_Remove_Second();

	/* open file */
	fp = fopen(filename, "rb");
	if (fp == NULL)
		return CARTRIDGE_CANT_OPEN;
	/* check file length */
	len = Util_flen(fp);
	Util_rewind(fp);

	/* Save Filename for state save */
	strcpy(second_cart_filename, filename);

	/* if full kilobytes, assume it is raw image */
	if ((len & 0x3ff) == 0) {
		/* alloc memory and read data */
		second_cart_image = (UBYTE *) Util_malloc(len);
		if (fread(second_cart_image, 1, len, fp) < len) {
			Log_print("Error reading cartridge.\n");
		}
		fclose(fp);
		/* find cart type */
		CARTRIDGE_second_type = CARTRIDGE_NONE;
		len >>= 10;	/* number of kilobytes */
		for (type = 1; type <= CARTRIDGE_LAST_SUPPORTED; type++)
			if (CARTRIDGE_kb[type] == len) {
				if (CARTRIDGE_second_type == CARTRIDGE_NONE)
					CARTRIDGE_second_type = type;
				else
					return len;	/* more than one cartridge type of such length - user must select */
			}
		if (CARTRIDGE_second_type != CARTRIDGE_NONE) {
			return 0;	/* ok */
		}
		free(second_cart_image);
		second_cart_image = NULL;
		return CARTRIDGE_BAD_FORMAT;
	}
	/* if not full kilobytes, assume it is CART file */
	if (fread(header, 1, 16, fp) < 16) {
		Log_print("Error reading cartridge.\n");
	}
	if ((header[0] == 'C') &&
		(header[1] == 'A') &&
		(header[2] == 'R') &&
		(header[3] == 'T')) {
		type = (header[4] << 24) |
			(header[5] << 16) |
			(header[6] << 8) |
			header[7];
		if (type >= 1 && type <= CARTRIDGE_LAST_SUPPORTED) {
			int checksum;
			len = CARTRIDGE_kb[type] << 10;
			/* alloc memory and read data */
			second_cart_image = (UBYTE *) Util_malloc(len);
			if (fread(second_cart_image, 1, len, fp) < len) {
				Log_print("Error reading cartridge.\n");
			}
			fclose(fp);
			checksum = (header[8] << 24) |
				(header[9] << 16) |
				(header[10] << 8) |
				header[11];
			CARTRIDGE_second_type = type;
			return checksum == CARTRIDGE_Checksum(second_cart_image, len) ? 0 : CARTRIDGE_BAD_CHECKSUM;
		}
	}
	fclose(fp);
	return CARTRIDGE_BAD_FORMAT;
}

void CARTRIDGE_Remove(void)
{
	if (CARTRIDGE_second_type != CARTRIDGE_NONE)
		CARTRIDGE_Remove_Second();
	CARTRIDGE_type = CARTRIDGE_NONE;
	if (cart_image != NULL) {
		free(cart_image);
		cart_image = NULL;
	}
	CARTRIDGE_Start();
}

void CARTRIDGE_Remove_Second(void)
{
	CARTRIDGE_second_type = CARTRIDGE_NONE;
	if (second_cart_image != NULL) {
		free(second_cart_image);
		second_cart_image = NULL;
	}
	if (cart_pass_through) {
		cart_pass_through = FALSE;
		cart_image = first_cart_image;
		MEMORY_CopyROM(0xa000, 0xbfff, cart_image);
		Atari800_Warmstart();
		}
}

void CARTRIDGE_Start(void) {
	if (cart_pass_through) {
		cart_image = first_cart_image;
		cart_pass_through = FALSE;
		}
        CARTRIDGE_Start_Local(CARTRIDGE_type);
	}

static void CARTRIDGE_Start_Local(int curr_cart_type) {
	if (Atari800_machine_type == Atari800_MACHINE_5200) {
		MEMORY_SetROM(0x4ff6, 0x4ff9);		/* disable Bounty Bob bank switching */
		MEMORY_SetROM(0x5ff6, 0x5ff9);
		switch (curr_cart_type) {
		case CARTRIDGE_5200_32:
			MEMORY_CopyROM(0x4000, 0xbfff, cart_image);
			break;
		case CARTRIDGE_5200_EE_16:
			MEMORY_CopyROM(0x4000, 0x5fff, cart_image);
			MEMORY_CopyROM(0x6000, 0x9fff, cart_image);
			MEMORY_CopyROM(0xa000, 0xbfff, cart_image + 0x2000);
			break;
		case CARTRIDGE_5200_40:
			MEMORY_CopyROM(0x4000, 0x4fff, cart_image);
			MEMORY_CopyROM(0x5000, 0x5fff, cart_image + 0x4000);
			MEMORY_CopyROM(0x8000, 0x9fff, cart_image + 0x8000);
			MEMORY_CopyROM(0xa000, 0xbfff, cart_image + 0x8000);
#ifndef PAGED_ATTRIB
			MEMORY_SetHARDWARE(0x4ff6, 0x4ff9);
			MEMORY_SetHARDWARE(0x5ff6, 0x5ff9);
#else
			MEMORY_readmap[0x4f] = CARTRIDGE_BountyBob1GetByte;
			MEMORY_readmap[0x5f] = CARTRIDGE_BountyBob2GetByte;
			MEMORY_writemap[0x4f] = CARTRIDGE_BountyBob1PutByte;
			MEMORY_writemap[0x5f] = CARTRIDGE_BountyBob2PutByte;
#endif
			break;
		case CARTRIDGE_5200_NS_16:
			MEMORY_CopyROM(0x8000, 0xbfff, cart_image);
			break;
		case CARTRIDGE_5200_8:
			MEMORY_CopyROM(0x8000, 0x9fff, cart_image);
			MEMORY_CopyROM(0xa000, 0xbfff, cart_image);
			break;
		case CARTRIDGE_5200_4:
			MEMORY_CopyROM(0x8000, 0x8fff, cart_image);
			MEMORY_CopyROM(0x9000, 0x9fff, cart_image);
			MEMORY_CopyROM(0xa000, 0xafff, cart_image);
			MEMORY_CopyROM(0xb000, 0xbfff, cart_image);
			break;
		default:
			/* clear cartridge area so the 5200 will crash */
			MEMORY_dFillMem(0x4000, 0, 0x8000);
			break;
		}
	}
	else {
		switch (curr_cart_type) {
		case CARTRIDGE_STD_8:
		case CARTRIDGE_PHOENIX_8:
			MEMORY_Cart809fDisable();
			MEMORY_CartA0bfEnable();
			MEMORY_CopyROM(0xa000, 0xbfff, cart_image);
			break;
		case CARTRIDGE_STD_16:
		case CARTRIDGE_BLIZZARD_16:
			MEMORY_Cart809fEnable();
			MEMORY_CartA0bfEnable();
			MEMORY_CopyROM(0x8000, 0xbfff, cart_image);
			break;
		case CARTRIDGE_OSS_16:
			MEMORY_Cart809fDisable();
			MEMORY_CartA0bfEnable();
			MEMORY_CopyROM(0xa000, 0xafff, cart_image);
			MEMORY_CopyROM(0xb000, 0xbfff, cart_image + 0x3000);
			bank = 0;
			break;
		case CARTRIDGE_DB_32:
			MEMORY_Cart809fEnable();
			MEMORY_CartA0bfEnable();
			MEMORY_CopyROM(0x8000, 0x9fff, cart_image);
			MEMORY_CopyROM(0xa000, 0xbfff, cart_image + 0x6000);
			bank = 0;
			break;
		case CARTRIDGE_WILL_64:
		case CARTRIDGE_WILL_32:
		case CARTRIDGE_EXP_64:
		case CARTRIDGE_DIAMOND_64:
		case CARTRIDGE_SDX_64:
		case CARTRIDGE_SDX_128:
			MEMORY_Cart809fDisable();
			MEMORY_CartA0bfEnable();
			MEMORY_CopyROM(0xa000, 0xbfff, cart_image);
			bank = 0;
			break;
		case CARTRIDGE_XEGS_32:
		case CARTRIDGE_SWXEGS_32:
			MEMORY_Cart809fEnable();
			MEMORY_CartA0bfEnable();
			MEMORY_CopyROM(0x8000, 0x9fff, cart_image);
			MEMORY_CopyROM(0xa000, 0xbfff, cart_image + 0x6000);
			bank = 0;
			break;
		case CARTRIDGE_XEGS_64:
		case CARTRIDGE_SWXEGS_64:
			MEMORY_Cart809fEnable();
			MEMORY_CartA0bfEnable();
			MEMORY_CopyROM(0x8000, 0x9fff, cart_image);
			MEMORY_CopyROM(0xa000, 0xbfff, cart_image + 0xe000);
			bank = 0;
			break;
		case CARTRIDGE_XEGS_128:
		case CARTRIDGE_SWXEGS_128:
			MEMORY_Cart809fEnable();
			MEMORY_CartA0bfEnable();
			MEMORY_CopyROM(0x8000, 0x9fff, cart_image);
			MEMORY_CopyROM(0xa000, 0xbfff, cart_image + 0x1e000);
			bank = 0;
			break;
		case CARTRIDGE_XEGS_256:
		case CARTRIDGE_SWXEGS_256:
			MEMORY_Cart809fEnable();
			MEMORY_CartA0bfEnable();
			MEMORY_CopyROM(0x8000, 0x9fff, cart_image);
			MEMORY_CopyROM(0xa000, 0xbfff, cart_image + 0x3e000);
			bank = 0;
			break;
		case CARTRIDGE_XEGS_512:
		case CARTRIDGE_SWXEGS_512:
			MEMORY_Cart809fEnable();
			MEMORY_CartA0bfEnable();
			MEMORY_CopyROM(0x8000, 0x9fff, cart_image);
			MEMORY_CopyROM(0xa000, 0xbfff, cart_image + 0x7e000);
			bank = 0;
			break;
		case CARTRIDGE_XEGS_1024:
		case CARTRIDGE_SWXEGS_1024:
			MEMORY_Cart809fEnable();
			MEMORY_CartA0bfEnable();
			MEMORY_CopyROM(0x8000, 0x9fff, cart_image);
			MEMORY_CopyROM(0xa000, 0xbfff, cart_image + 0xfe000);
			bank = 0;
			break;
		case CARTRIDGE_OSS2_16:
			MEMORY_Cart809fDisable();
			MEMORY_CartA0bfEnable();
			MEMORY_CopyROM(0xa000, 0xafff, cart_image + 0x1000);
			MEMORY_CopyROM(0xb000, 0xbfff, cart_image);
			bank = 0;
			break;
		case CARTRIDGE_ATRAX_128:
			MEMORY_Cart809fDisable();
			MEMORY_CartA0bfEnable();
			MEMORY_CopyROM(0xa000, 0xbfff, cart_image);
			bank = 0;
			break;
		case CARTRIDGE_BBSB_40:
			MEMORY_Cart809fEnable();
			MEMORY_CartA0bfEnable();
			MEMORY_CopyROM(0x8000, 0x8fff, cart_image);
			MEMORY_CopyROM(0x9000, 0x9fff, cart_image + 0x4000);
			MEMORY_CopyROM(0xa000, 0xbfff, cart_image + 0x8000);
#ifndef PAGED_ATTRIB
			MEMORY_SetHARDWARE(0x8ff6, 0x8ff9);
			MEMORY_SetHARDWARE(0x9ff6, 0x9ff9);
#else
			MEMORY_readmap[0x8f] = CARTRIDGE_BountyBob1GetByte;
			MEMORY_readmap[0x9f] = CARTRIDGE_BountyBob2GetByte;
			MEMORY_writemap[0x8f] = CARTRIDGE_BountyBob1PutByte;
			MEMORY_writemap[0x9f] = CARTRIDGE_BountyBob2PutByte;
#endif
			break;
		case CARTRIDGE_RIGHT_8:
			if (Atari800_machine_type == Atari800_MACHINE_OSA || Atari800_machine_type == Atari800_MACHINE_OSB) {
				MEMORY_Cart809fEnable();
				MEMORY_CopyROM(0x8000, 0x9fff, cart_image);
				if ((!Atari800_disable_basic || BINLOAD_loading_basic) && MEMORY_have_basic) {
					MEMORY_CartA0bfEnable();
					MEMORY_CopyROM(0xa000, 0xbfff, MEMORY_basic);
					break;
				}
				MEMORY_CartA0bfDisable();
				break;
			}
			/* there's no right slot in XL/XE */
			MEMORY_Cart809fDisable();
			MEMORY_CartA0bfDisable();
			break;
		case CARTRIDGE_MEGA_16:
		case CARTRIDGE_MEGA_32:
		case CARTRIDGE_MEGA_64:
		case CARTRIDGE_MEGA_128:
		case CARTRIDGE_MEGA_256:
		case CARTRIDGE_MEGA_512:
		case CARTRIDGE_MEGA_1024:
			MEMORY_Cart809fEnable();
			MEMORY_CartA0bfEnable();
			MEMORY_CopyROM(0x8000, 0xbfff, cart_image);
			bank = 0;
			break;
		case CARTRIDGE_ATMAX_128:
			MEMORY_Cart809fDisable();
			MEMORY_CartA0bfEnable();
			MEMORY_CopyROM(0xa000, 0xbfff, cart_image);
			bank = 0;
			break;
		case CARTRIDGE_ATMAX_1024:
			MEMORY_Cart809fDisable();
			MEMORY_CartA0bfEnable();
			MEMORY_CopyROM(0xa000, 0xbfff, cart_image + 0xfe000);
			bank = 0x7f;
			break;
		default:
			MEMORY_Cart809fDisable();
			if ((Atari800_machine_type == Atari800_MACHINE_OSA || Atari800_machine_type == Atari800_MACHINE_OSB)
			 && (!Atari800_disable_basic || BINLOAD_loading_basic) && MEMORY_have_basic) {
				MEMORY_CartA0bfEnable();
				MEMORY_CopyROM(0xa000, 0xbfff, MEMORY_basic);
				break;
			}
			MEMORY_CartA0bfDisable();
			break;
		}
	}
}

#ifndef BASIC

void CARTRIDGE_StateRead(void)
{
	int savedCartType = CARTRIDGE_NONE;
	char filename[FILENAME_MAX];

	/* Read the cart type from the file.  If there is no cart type, becaused we have
	   reached the end of the file, this will just default to CART_NONE */
	StateSav_ReadINT(&savedCartType, 1);
	if (savedCartType < 0) {
		savedCartType = -savedCartType;
		StateSav_ReadFNAME(filename);
		if (filename[0]) {
			/* Insert the cartridge... */
			if (CARTRIDGE_Insert(filename) >= 0) {
				/* And set the type to the saved type, in case it was a raw cartridge image */
				CARTRIDGE_type = savedCartType;
				CARTRIDGE_Start();
			}
		}
		StateSav_ReadINT(&savedCartType, 1);
		StateSav_ReadFNAME(filename);
		if (filename[0]) {
			/* Insert the cartridge... */
			if (CARTRIDGE_Insert_Second(filename) >= 0) {
				/* And set the type to the saved type, in case it was a raw cartridge image */
				CARTRIDGE_second_type = savedCartType;
				CARTRIDGE_Start();
			}
		}
		/* Get the state of the first/second cart active flag */
		StateSav_ReadINT(&cart_pass_through, 1);
		if (cart_pass_through) {
			first_cart_image = cart_image;
			cart_image = second_cart_image;
		}
	} else if (savedCartType != CARTRIDGE_NONE) {
		StateSav_ReadFNAME(filename);
		if (filename[0]) {
			/* Insert the cartridge... */
			if (CARTRIDGE_Insert(filename) >= 0) {
				/* And set the type to the saved type, in case it was a raw cartridge image */
				CARTRIDGE_type = savedCartType;
				CARTRIDGE_Start();
			}
		}
	}
}

void CARTRIDGE_StateSave(void)
{
	int cart_save;
	
	if (CARTRIDGE_second_type == CARTRIDGE_NONE) {
		/* Save the cartridge type, or CARTRIDGE_NONE if there isn't one...*/
		StateSav_SaveINT(&CARTRIDGE_type, 1);
		if (CARTRIDGE_type != CARTRIDGE_NONE) {
			StateSav_SaveFNAME(cart_filename);
		}
	} else {
		/* Save the cart type as negative, to indicate to CARTStateRead that there is a 
		   second cartridge */
		cart_save = -CARTRIDGE_type;
		/* Save the cartridge type and name*/
		StateSav_SaveINT(&cart_save, 1);
		if (CARTRIDGE_type != CARTRIDGE_NONE) {
			StateSav_SaveFNAME(cart_filename);
		}
		/* Save the second cartridge type and name*/
		StateSav_SaveINT(&CARTRIDGE_second_type, 1);
		StateSav_SaveFNAME(second_cart_filename);
		/* Save the state of the first/second cart active flag */
		StateSav_SaveINT(&cart_pass_through, 1);
	}

}

#endif

/*
vim:ts=4:sw=4:
*/
