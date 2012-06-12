/*
 * memory.c - memory emulation
 *
 * Copyright (C) 1995-1998 David Firth
 * Copyright (C) 1998-2008 Atari800 development team (see DOC/CREDITS)
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
#include "antic.h"
#include "cpu.h"
#include "cartridge.h"
#include "esc.h"
#include "gtia.h"
#include "log.h"
#include "memory.h"
#include "pbi.h"
#include "pia.h"
#include "pokey.h"
#include "util.h"
#ifndef BASIC
#include "statesav.h"
#endif

UBYTE MEMORY_mem[65536 + 2];

int MEMORY_ram_size = 64;

#ifndef PAGED_ATTRIB

UBYTE MEMORY_attrib[65536];

#else /* PAGED_ATTRIB */

MEMORY_rdfunc MEMORY_readmap[256];
MEMORY_wrfunc MEMORY_writemap[256];

typedef struct map_save {
	int     code;
	MEMORY_rdfunc  rdptr;
	MEMORY_wrfunc  wrptr;
} map_save;

void MEMORY_ROM_PutByte(UWORD addr, UBYTE value)
{
}

map_save save_map[2] = {
	{0, NULL, NULL},          /* RAM */
	{1, NULL, MEMORY_ROM_PutByte}    /* ROM */
};

#endif /* PAGED_ATTRIB */

UBYTE MEMORY_basic[8192];
UBYTE MEMORY_os[16384];

int MEMORY_xe_bank = 0;
int MEMORY_selftest_enabled = 0;

static UBYTE under_atarixl_os[16384];
static UBYTE under_atari_basic[8192];
static UBYTE *atarixe_memory = NULL;
static ULONG atarixe_memory_size = 0;

int MEMORY_have_basic = FALSE; /* Atari BASIC image has been successfully read (Atari 800 only) */

/* Axlon and Mosaic RAM expansions for Atari 400/800 only */
static void MosaicPutByte(UWORD addr, UBYTE byte);
static UBYTE MosaicGetByte(UWORD addr);
static void AxlonPutByte(UWORD addr, UBYTE byte);
static UBYTE AxlonGetByte(UWORD addr);
static UBYTE *axlon_ram = NULL;
static int axlon_ram_size = 0;
int axlon_curbank = 0;
int MEMORY_axlon_bankmask = 0x07;
int MEMORY_axlon_enabled = FALSE;
int MEMORY_axlon_0f_mirror = FALSE; /* The real Axlon had a mirror bank register at 0x0fc0-0x0fff, compatibles did not*/
static UBYTE *mosaic_ram = NULL;
static int mosaic_ram_size = 0;
static int mosaic_curbank = 0x3f;
int MEMORY_mosaic_maxbank = 0;
int MEMORY_mosaic_enabled = FALSE;

static void alloc_axlon_memory(void){
	axlon_curbank = 0;
	if (MEMORY_axlon_enabled && (Atari800_machine_type == Atari800_MACHINE_OSA || Atari800_machine_type == Atari800_MACHINE_OSB )) {
		int new_axlon_ram_size = (MEMORY_axlon_bankmask+1)*0x4000;
		if ((axlon_ram == NULL) || (axlon_ram_size != new_axlon_ram_size)) {
			axlon_ram_size = new_axlon_ram_size;
			if (axlon_ram != NULL) free(axlon_ram);
			axlon_ram = (UBYTE *)Util_malloc(axlon_ram_size);
		}
		memset(axlon_ram, 0, axlon_ram_size);
	} else {
		if (axlon_ram != NULL) {
			free(axlon_ram);
			axlon_ram_size = 0;
		}
	}
}

static void alloc_mosaic_memory(void){
	mosaic_curbank = 0x3f;
	if (MEMORY_mosaic_enabled && (Atari800_machine_type == Atari800_MACHINE_OSA || Atari800_machine_type == Atari800_MACHINE_OSB)) {
		int new_mosaic_ram_size = (MEMORY_mosaic_maxbank+1)*0x1000;
		if ((mosaic_ram == NULL) || (mosaic_ram_size != new_mosaic_ram_size)) {
			mosaic_ram_size = new_mosaic_ram_size;
			if (mosaic_ram != NULL) free(mosaic_ram);
			mosaic_ram = (UBYTE *)Util_malloc(mosaic_ram_size);
		}
		memset(mosaic_ram, 0, mosaic_ram_size);
	} else {
		if (mosaic_ram != NULL) {
			free(mosaic_ram);
			mosaic_ram_size = 0;
		}
	}

}

static void AllocXEMemory(void)
{
	if (MEMORY_ram_size > 64) {
		/* don't count 64 KB of base memory */
		/* count number of 16 KB banks, add 1 for saving base memory 0x4000-0x7fff */
		ULONG size = (1 + (MEMORY_ram_size - 64) / 16) * 16384;
		if (size != atarixe_memory_size) {
			if (atarixe_memory != NULL)
				free(atarixe_memory);
			atarixe_memory = (UBYTE *) Util_malloc(size);
			atarixe_memory_size = size;
			memset(atarixe_memory, 0, size);
		}
	}
	/* atarixe_memory not needed, free it */
	else if (atarixe_memory != NULL) {
		free(atarixe_memory);
		atarixe_memory = NULL;
		atarixe_memory_size = 0;
	}
}

void MEMORY_InitialiseMachine(void)
{
	ANTIC_xe_ptr = NULL;
	switch (Atari800_machine_type) {
	case Atari800_MACHINE_OSA:
	case Atari800_MACHINE_OSB:
		memcpy(MEMORY_mem + 0xd800, MEMORY_os, 0x2800);
		ESC_PatchOS();
		MEMORY_dFillMem(0x0000, 0x00, MEMORY_ram_size * 1024 - 1);
		MEMORY_SetRAM(0x0000, MEMORY_ram_size * 1024 - 1);
		if (MEMORY_ram_size < 52) {
			MEMORY_dFillMem(MEMORY_ram_size * 1024, 0xff, 0xd000 - MEMORY_ram_size * 1024);
			MEMORY_SetROM(MEMORY_ram_size * 1024, 0xcfff);
		}
		MEMORY_SetROM(0xd800, 0xffff);
#ifndef PAGED_ATTRIB
		MEMORY_SetHARDWARE(0xd000, 0xd7ff);
		if (MEMORY_mosaic_enabled) MEMORY_SetHARDWARE(0xff00, 0xffff); 
		/* only 0xffc0-0xffff are used, but mark the whole  
		 * page to make state saving easier */
		if (MEMORY_axlon_enabled) { 
		       	MEMORY_SetHARDWARE(0xcf00, 0xcfff);
			if (MEMORY_axlon_0f_mirror) MEMORY_SetHARDWARE(0x0f00, 0x0fff);
			/* only ?fc0-?fff are used, but mark the whole page*/
		}
#else
		MEMORY_readmap[0xd0] = GTIA_GetByte;
		MEMORY_readmap[0xd1] = PBI_D1GetByte;
		MEMORY_readmap[0xd2] = POKEY_GetByte;
		MEMORY_readmap[0xd3] = PIA_GetByte;
		MEMORY_readmap[0xd4] = ANTIC_GetByte;
		MEMORY_readmap[0xd5] = CARTRIDGE_GetByte;
		MEMORY_readmap[0xd6] = PBI_D6GetByte;
		MEMORY_readmap[0xd7] = PBI_D7GetByte;
		MEMORY_writemap[0xd0] = GTIA_PutByte;
		MEMORY_writemap[0xd1] = PBI_D1PutByte;
		MEMORY_writemap[0xd2] = POKEY_PutByte;
		MEMORY_writemap[0xd3] = PIA_PutByte;
		MEMORY_writemap[0xd4] = ANTIC_PutByte;
		MEMORY_writemap[0xd5] = CARTRIDGE_PutByte;
		MEMORY_writemap[0xd6] = PBI_D6PutByte;
		MEMORY_writemap[0xd7] = PBI_D7PutByte;
		if (MEMORY_mosaic_enabled) MEMORY_writemap[0xff] = MosaicPutByte;
		if (MEMORY_axlon_enabled) MEMORY_writemap[0xcf] = AxlonPutByte;
		if (MEMORY_axlon_enabled && MEMORY_axlon_0f_mirror) MEMORY_writemap[0x0f] = AxlonPutByte;
#endif
		break;
	case Atari800_MACHINE_XLXE:
		memcpy(MEMORY_mem + 0xc000, MEMORY_os, 0x4000);
		ESC_PatchOS();
		if (MEMORY_ram_size == 16) {
			MEMORY_dFillMem(0x0000, 0x00, 0x4000);
			MEMORY_SetRAM(0x0000, 0x3fff);
			MEMORY_dFillMem(0x4000, 0xff, 0x8000);
			MEMORY_SetROM(0x4000, 0xcfff);
		} else {
			MEMORY_dFillMem(0x0000, 0x00, 0xc000);
			MEMORY_SetRAM(0x0000, 0xbfff);
			MEMORY_SetROM(0xc000, 0xcfff);
		}
#ifndef PAGED_ATTRIB
		MEMORY_SetHARDWARE(0xd000, 0xd7ff);
#else
		MEMORY_readmap[0xd0] = GTIA_GetByte;
		MEMORY_readmap[0xd1] = PBI_D1GetByte;
		MEMORY_readmap[0xd2] = POKEY_GetByte;
		MEMORY_readmap[0xd3] = PIA_GetByte;
		MEMORY_readmap[0xd4] = ANTIC_GetByte;
		MEMORY_readmap[0xd5] = CARTRIDGE_GetByte;
		MEMORY_readmap[0xd6] = PBI_D6GetByte;
		MEMORY_readmap[0xd7] = PBI_D7GetByte;
		MEMORY_writemap[0xd0] = GTIA_PutByte;
		MEMORY_writemap[0xd1] = PBI_D1PutByte;
		MEMORY_writemap[0xd2] = POKEY_PutByte;
		MEMORY_writemap[0xd3] = PIA_PutByte;
		MEMORY_writemap[0xd4] = ANTIC_PutByte;
		MEMORY_writemap[0xd5] = CARTRIDGE_PutByte;
		MEMORY_writemap[0xd6] = PBI_D6PutByte;
		MEMORY_writemap[0xd7] = PBI_D7PutByte;
#endif
		MEMORY_SetROM(0xd800, 0xffff);
		break;
	case Atari800_MACHINE_5200:
		memcpy(MEMORY_mem + 0xf800, MEMORY_os, 0x800);
		MEMORY_dFillMem(0x0000, 0x00, 0xf800);
		MEMORY_SetRAM(0x0000, 0x3fff);
		MEMORY_SetROM(0x4000, 0xffff);
#ifndef PAGED_ATTRIB
		MEMORY_SetHARDWARE(0xc000, 0xcfff);	/* 5200 GTIA Chip */
		MEMORY_SetHARDWARE(0xd400, 0xd4ff);	/* 5200 ANTIC Chip */
		MEMORY_SetHARDWARE(0xe800, 0xefff);	/* 5200 POKEY Chip */
#else
		MEMORY_readmap[0xc0] = GTIA_GetByte;
		MEMORY_readmap[0xc1] = GTIA_GetByte;
		MEMORY_readmap[0xc2] = GTIA_GetByte;
		MEMORY_readmap[0xc3] = GTIA_GetByte;
		MEMORY_readmap[0xc4] = GTIA_GetByte;
		MEMORY_readmap[0xc5] = GTIA_GetByte;
		MEMORY_readmap[0xc6] = GTIA_GetByte;
		MEMORY_readmap[0xc7] = GTIA_GetByte;
		MEMORY_readmap[0xc8] = GTIA_GetByte;
		MEMORY_readmap[0xc9] = GTIA_GetByte;
		MEMORY_readmap[0xca] = GTIA_GetByte;
		MEMORY_readmap[0xcb] = GTIA_GetByte;
		MEMORY_readmap[0xcc] = GTIA_GetByte;
		MEMORY_readmap[0xcd] = GTIA_GetByte;
		MEMORY_readmap[0xce] = GTIA_GetByte;
		MEMORY_readmap[0xcf] = GTIA_GetByte;
		MEMORY_readmap[0xd4] = ANTIC_GetByte;
		MEMORY_readmap[0xe8] = POKEY_GetByte;
		MEMORY_readmap[0xe9] = POKEY_GetByte;
		MEMORY_readmap[0xea] = POKEY_GetByte;
		MEMORY_readmap[0xeb] = POKEY_GetByte;
		MEMORY_readmap[0xec] = POKEY_GetByte;
		MEMORY_readmap[0xed] = POKEY_GetByte;
		MEMORY_readmap[0xee] = POKEY_GetByte;
		MEMORY_readmap[0xef] = POKEY_GetByte;

		MEMORY_writemap[0xc0] = GTIA_PutByte;
		MEMORY_writemap[0xc1] = GTIA_PutByte;
		MEMORY_writemap[0xc2] = GTIA_PutByte;
		MEMORY_writemap[0xc3] = GTIA_PutByte;
		MEMORY_writemap[0xc4] = GTIA_PutByte;
		MEMORY_writemap[0xc5] = GTIA_PutByte;
		MEMORY_writemap[0xc6] = GTIA_PutByte;
		MEMORY_writemap[0xc7] = GTIA_PutByte;
		MEMORY_writemap[0xc8] = GTIA_PutByte;
		MEMORY_writemap[0xc9] = GTIA_PutByte;
		MEMORY_writemap[0xca] = GTIA_PutByte;
		MEMORY_writemap[0xcb] = GTIA_PutByte;
		MEMORY_writemap[0xcc] = GTIA_PutByte;
		MEMORY_writemap[0xcd] = GTIA_PutByte;
		MEMORY_writemap[0xce] = GTIA_PutByte;
		MEMORY_writemap[0xcf] = GTIA_PutByte;
		MEMORY_writemap[0xd4] = ANTIC_PutByte;
		MEMORY_writemap[0xe8] = POKEY_PutByte;
		MEMORY_writemap[0xe9] = POKEY_PutByte;
		MEMORY_writemap[0xea] = POKEY_PutByte;
		MEMORY_writemap[0xeb] = POKEY_PutByte;
		MEMORY_writemap[0xec] = POKEY_PutByte;
		MEMORY_writemap[0xed] = POKEY_PutByte;
		MEMORY_writemap[0xee] = POKEY_PutByte;
		MEMORY_writemap[0xef] = POKEY_PutByte;
#endif
		break;
	}
	AllocXEMemory();
	alloc_axlon_memory();
	alloc_mosaic_memory();
	Atari800_Coldstart();
}

#ifndef BASIC

void MEMORY_StateSave(UBYTE SaveVerbose)
{
	/* Axlon/Mosaic for 400/800 */
	if (Atari800_machine_type == Atari800_MACHINE_OSA  || Atari800_machine_type == Atari800_MACHINE_OSB) {
		StateSav_SaveINT(&MEMORY_axlon_enabled, 1);
		if (MEMORY_axlon_enabled){
			StateSav_SaveINT(&axlon_curbank, 1);
			StateSav_SaveINT(&MEMORY_axlon_bankmask, 1);
			StateSav_SaveINT(&MEMORY_axlon_0f_mirror, 1);
			StateSav_SaveINT(&axlon_ram_size, 1);
			StateSav_SaveUBYTE(&axlon_ram[0], axlon_ram_size);
		}
		StateSav_SaveINT(&MEMORY_mosaic_enabled, 1);
		if (MEMORY_mosaic_enabled){
			StateSav_SaveINT(&mosaic_curbank, 1);
			StateSav_SaveINT(&MEMORY_mosaic_maxbank, 1);
			StateSav_SaveINT(&mosaic_ram_size, 1);
			StateSav_SaveUBYTE(&mosaic_ram[0], mosaic_ram_size);
		}
	}

	StateSav_SaveUBYTE(&MEMORY_mem[0], 65536);
#ifndef PAGED_ATTRIB
	StateSav_SaveUBYTE(&MEMORY_attrib[0], 65536);
#else
	{
		/* I assume here that consecutive calls to StateSav_SaveUBYTE()
		   are equivalent to a single call with all the values
		   (i.e. StateSav_SaveUBYTE() doesn't write any headers). */
		UBYTE attrib_page[256];
		int i;
		for (i = 0; i < 256; i++) {
			if (MEMORY_writemap[i] == NULL)
				memset(attrib_page, MEMORY_RAM, 256);
			else if (MEMORY_writemap[i] == MEMORY_ROM_PutByte)
				memset(attrib_page, MEMORY_ROM, 256);
			else if (i == 0x4f || i == 0x5f || i == 0x8f || i == 0x9f) {
				/* special case: Bounty Bob bank switching registers */
				memset(attrib_page, MEMORY_ROM, 256);
				attrib_page[0xf6] = MEMORY_HARDWARE;
				attrib_page[0xf7] = MEMORY_HARDWARE;
				attrib_page[0xf8] = MEMORY_HARDWARE;
				attrib_page[0xf9] = MEMORY_HARDWARE;
			}
			else {
				memset(attrib_page, MEMORY_HARDWARE, 256);
			}
			StateSav_SaveUBYTE(&attrib_page[0], 256);
		}
	}
#endif

	if (Atari800_machine_type == Atari800_MACHINE_XLXE) {
		if (SaveVerbose != 0)
			StateSav_SaveUBYTE(&MEMORY_basic[0], 8192);
		StateSav_SaveUBYTE(&under_atari_basic[0], 8192);

		if (SaveVerbose != 0)
			StateSav_SaveUBYTE(&MEMORY_os[0], 16384);
		StateSav_SaveUBYTE(&under_atarixl_os[0], 16384);
	}

	if (MEMORY_ram_size > 64) {
		StateSav_SaveUBYTE(&atarixe_memory[0], atarixe_memory_size);
		/* a hack that makes state files compatible with previous versions:
           for 130 XE there's written 192 KB of unused data */
		if (MEMORY_ram_size == 128) {
			UBYTE buffer[256];
			int i;
			memset(buffer, 0, 256);
			for (i = 0; i < 192 * 4; i++)
				StateSav_SaveUBYTE(&buffer[0], 256);
		}
	}
}

void MEMORY_StateRead(UBYTE SaveVerbose, UBYTE StateVersion)
{
	/* Axlon/Mosaic for 400/800 */
	if ((Atari800_machine_type == Atari800_MACHINE_OSA  || Atari800_machine_type == Atari800_MACHINE_OSB) && StateVersion >= 5) {
		StateSav_ReadINT(&MEMORY_axlon_enabled, 1);
		if (MEMORY_axlon_enabled){
			StateSav_ReadINT(&axlon_curbank, 1);
			StateSav_ReadINT(&MEMORY_axlon_bankmask, 1);
			StateSav_ReadINT(&MEMORY_axlon_0f_mirror, 1);
			StateSav_ReadINT(&axlon_ram_size, 1);
			alloc_axlon_memory();
			StateSav_ReadUBYTE(&axlon_ram[0], axlon_ram_size);
		}
		StateSav_ReadINT(&MEMORY_mosaic_enabled, 1);
		if (MEMORY_mosaic_enabled){
			StateSav_ReadINT(&mosaic_curbank, 1);
			StateSav_ReadINT(&MEMORY_mosaic_maxbank, 1);
			StateSav_ReadINT(&mosaic_ram_size, 1);
			alloc_mosaic_memory();
			StateSav_ReadUBYTE(&mosaic_ram[0], mosaic_ram_size);
		}
	}

	StateSav_ReadUBYTE(&MEMORY_mem[0], 65536);
#ifndef PAGED_ATTRIB
	StateSav_ReadUBYTE(&MEMORY_attrib[0], 65536);
#else
	{
		UBYTE attrib_page[256];
		int i;
		for (i = 0; i < 256; i++) {
			StateSav_ReadUBYTE(&attrib_page[0], 256);
			/* note: 0x40 is intentional here:
			   we want ROM on page 0xd1 if H: patches are enabled */
			switch (attrib_page[0x40]) {
			case MEMORY_RAM:
				MEMORY_readmap[i] = NULL;
				MEMORY_writemap[i] = NULL;
				break;
			case MEMORY_ROM:
				if (i != 0xd1 && attrib_page[0xf6] == MEMORY_HARDWARE) {
					if (i == 0x4f || i == 0x8f) {
						MEMORY_readmap[i] = CARTRIDGE_BountyBob1GetByte;
						MEMORY_writemap[i] = CARTRIDGE_BountyBob1PutByte;
					}
					else if (i == 0x5f || i == 0x9f) {
						MEMORY_readmap[i] = CARTRIDGE_BountyBob2GetByte;
						MEMORY_writemap[i] = CARTRIDGE_BountyBob2PutByte;
					}
					/* else something's wrong, so we keep current values */
				}
				else {
					MEMORY_readmap[i] = NULL;
					MEMORY_writemap[i] = MEMORY_ROM_PutByte;
				}
				break;
			case MEMORY_HARDWARE:
				switch (i) {
				case 0xc0:
				case 0xd0:
					MEMORY_readmap[i] = GTIA_GetByte;
					MEMORY_writemap[i] = GTIA_PutByte;
					break;
				case 0xd1:
					MEMORY_readmap[i] = PBI_D1GetByte;
					MEMORY_writemap[i] = PBI_D1PutByte;
					break;
				case 0xd2:
				case 0xe8:
				case 0xeb:
					MEMORY_readmap[i] = POKEY_GetByte;
					MEMORY_writemap[i] = POKEY_PutByte;
					break;
				case 0xd3:
					MEMORY_readmap[i] = PIA_GetByte;
					MEMORY_writemap[i] = PIA_PutByte;
					break;
				case 0xd4:
					MEMORY_readmap[i] = ANTIC_GetByte;
					MEMORY_writemap[i] = ANTIC_PutByte;
					break;
				case 0xd5:
					MEMORY_readmap[i] = CARTRIDGE_GetByte;
					MEMORY_writemap[i] = CARTRIDGE_PutByte;
					break;
				case 0xd6:
					MEMORY_readmap[i] = PBI_D6GetByte;
					MEMORY_writemap[i] = PBI_D6PutByte;
					break;
				case 0xd7:
					MEMORY_readmap[i] = PBI_D7GetByte;
					MEMORY_writemap[i] = PBI_D7PutByte;
					break;
				case 0xff:
					if (MEMORY_mosaic_enabled) MEMORY_writemap[0xff] = MosaicPutByte;
					break;
				case 0xcf:
					if (MEMORY_axlon_enabled) MEMORY_writemap[0xcf] = AxlonPutByte;
					break;
				case 0x0f:
					if (MEMORY_axlon_enabled && MEMORY_axlon_0f_mirror) MEMORY_writemap[0x0f] = AxlonPutByte;
					break;
				default:
					/* something's wrong, so we keep current values */
					break;
				}
				break;
			default:
				/* something's wrong, so we keep current values */
				break;
			}
		}
	}
#endif

	if (Atari800_machine_type == Atari800_MACHINE_XLXE) {
		if (SaveVerbose != 0)
			StateSav_ReadUBYTE(&MEMORY_basic[0], 8192);
		StateSav_ReadUBYTE(&under_atari_basic[0], 8192);

		if (SaveVerbose != 0)
			StateSav_ReadUBYTE(&MEMORY_os[0], 16384);
		StateSav_ReadUBYTE(&under_atarixl_os[0], 16384);
	}

	ANTIC_xe_ptr = NULL;
	AllocXEMemory();
	if (MEMORY_ram_size > 64) {
		StateSav_ReadUBYTE(&atarixe_memory[0], atarixe_memory_size);
		/* a hack that makes state files compatible with previous versions:
           for 130 XE there's written 192 KB of unused data */
		if (MEMORY_ram_size == 128) {
			UBYTE buffer[256];
			int i;
			for (i = 0; i < 192 * 4; i++)
				StateSav_ReadUBYTE(&buffer[0], 256);
		}
	}
}

#endif /* BASIC */

void MEMORY_CopyFromMem(UWORD from, UBYTE *to, int size)
{
	while (--size >= 0) {
		*to++ = MEMORY_GetByte(from);
		from++;
	}
}

void MEMORY_CopyToMem(const UBYTE *from, UWORD to, int size)
{
	while (--size >= 0) {
		MEMORY_PutByte(to, *from);
		from++;
		to++;
	}
}

/*
 * Returns non-zero, if Atari BASIC is disabled by given PORTB output.
 * Normally BASIC is disabled by setting bit 1, but it's also disabled
 * when using 576K and 1088K memory expansions, where bit 1 is used
 * for selecting extended memory bank number.
 */
static int basic_disabled(UBYTE portb)
{
	return (portb & 0x02) != 0
	 || ((portb & 0x10) == 0 && (MEMORY_ram_size == 576 || MEMORY_ram_size == 1088));
}

/* Note: this function is only for XL/XE! */
void MEMORY_HandlePORTB(UBYTE byte, UBYTE oldval)
{
	/* Switch XE memory bank in 0x4000-0x7fff */
	if (MEMORY_ram_size > 64) {
		int bank = 0;
		/* bank = 0 : base RAM */
		/* bank = 1..64 : extended RAM */
		if ((byte & 0x10) == 0)
			switch (MEMORY_ram_size) {
			case 128:
				bank = ((byte & 0x0c) >> 2) + 1;
				break;
			case 192:
				bank = (((byte & 0x0c) + ((byte & 0x40) >> 2)) >> 2) + 1;
				break;
			case MEMORY_RAM_320_RAMBO:
				bank = (((byte & 0x0c) + ((byte & 0x60) >> 1)) >> 2) + 1;
				break;
			case MEMORY_RAM_320_COMPY_SHOP:
				bank = (((byte & 0x0c) + ((byte & 0xc0) >> 2)) >> 2) + 1;
				break;
			case 576:
				bank = (((byte & 0x0e) + ((byte & 0x60) >> 1)) >> 1) + 1;
				break;
			case 1088:
				bank = (((byte & 0x0e) + ((byte & 0xe0) >> 1)) >> 1) + 1;
				break;
			}
		/* Note: in Compy Shop bit 5 (ANTIC access) disables Self Test */
		if (MEMORY_selftest_enabled && (bank != MEMORY_xe_bank || (MEMORY_ram_size == MEMORY_RAM_320_COMPY_SHOP && (byte & 0x20) == 0))) {
			/* Disable Self Test ROM */
			memcpy(MEMORY_mem + 0x5000, under_atarixl_os + 0x1000, 0x800);
			MEMORY_SetRAM(0x5000, 0x57ff);
			MEMORY_selftest_enabled = FALSE;
		}
		if (bank != MEMORY_xe_bank) {
			memcpy(atarixe_memory + (MEMORY_xe_bank << 14), MEMORY_mem + 0x4000, 16384);
			memcpy(MEMORY_mem + 0x4000, atarixe_memory + (bank << 14), 16384);
			MEMORY_xe_bank = bank;
		}
		if (MEMORY_ram_size == 128 || MEMORY_ram_size == MEMORY_RAM_320_COMPY_SHOP)
			switch (byte & 0x30) {
			case 0x20:	/* ANTIC: base, CPU: extended */
				ANTIC_xe_ptr = atarixe_memory;
				break;
			case 0x10:	/* ANTIC: extended, CPU: base */
				if (MEMORY_ram_size == 128)
					ANTIC_xe_ptr = atarixe_memory + ((((byte & 0x0c) >> 2) + 1) << 14);
				else	/* 320 Compy Shop */
					ANTIC_xe_ptr = atarixe_memory + (((((byte & 0x0c) + ((byte & 0xc0) >> 2)) >> 2) + 1) << 14);
				break;
			default:	/* ANTIC same as CPU */
				ANTIC_xe_ptr = NULL;
				break;
			}
	}

	/* Enable/disable OS ROM in 0xc000-0xcfff and 0xd800-0xffff */
	if ((oldval ^ byte) & 0x01) {
		if (byte & 0x01) {
			/* Enable OS ROM */
			if (MEMORY_ram_size > 48) {
				memcpy(under_atarixl_os, MEMORY_mem + 0xc000, 0x1000);
				memcpy(under_atarixl_os + 0x1800, MEMORY_mem + 0xd800, 0x2800);
				MEMORY_SetROM(0xc000, 0xcfff);
				MEMORY_SetROM(0xd800, 0xffff);
			}
			memcpy(MEMORY_mem + 0xc000, MEMORY_os, 0x1000);
			memcpy(MEMORY_mem + 0xd800, MEMORY_os + 0x1800, 0x2800);
			ESC_PatchOS();
		}
		else {
			/* Disable OS ROM */
			if (MEMORY_ram_size > 48) {
				memcpy(MEMORY_mem + 0xc000, under_atarixl_os, 0x1000);
				memcpy(MEMORY_mem + 0xd800, under_atarixl_os + 0x1800, 0x2800);
				MEMORY_SetRAM(0xc000, 0xcfff);
				MEMORY_SetRAM(0xd800, 0xffff);
			} else {
				MEMORY_dFillMem(0xc000, 0xff, 0x1000);
				MEMORY_dFillMem(0xd800, 0xff, 0x2800);
			}
			/* When OS ROM is disabled we also have to disable Self Test - Jindroush */
			if (MEMORY_selftest_enabled) {
				if (MEMORY_ram_size > 20) {
					memcpy(MEMORY_mem + 0x5000, under_atarixl_os + 0x1000, 0x800);
					MEMORY_SetRAM(0x5000, 0x57ff);
				}
				else
					MEMORY_dFillMem(0x5000, 0xff, 0x800);
				MEMORY_selftest_enabled = FALSE;
			}
		}
	}

	/* Enable/disable BASIC ROM in 0xa000-0xbfff */
	if (!MEMORY_cartA0BF_enabled) {
		/* BASIC is disabled if bit 1 set or accessing extended 576K or 1088K memory */
		int now_disabled = basic_disabled(byte);
		if (basic_disabled(oldval) != now_disabled) {
			if (now_disabled) {
				/* Disable BASIC ROM */
				if (MEMORY_ram_size > 40) {
					memcpy(MEMORY_mem + 0xa000, under_atari_basic, 0x2000);
					MEMORY_SetRAM(0xa000, 0xbfff);
				}
				else
					MEMORY_dFillMem(0xa000, 0xff, 0x2000);
			}
			else {
				/* Enable BASIC ROM */
				if (MEMORY_ram_size > 40) {
					memcpy(under_atari_basic, MEMORY_mem + 0xa000, 0x2000);
					MEMORY_SetROM(0xa000, 0xbfff);
				}
				memcpy(MEMORY_mem + 0xa000, MEMORY_basic, 0x2000);
			}
		}
	}

	/* Enable/disable Self Test ROM in 0x5000-0x57ff */
	if (byte & 0x80) {
		if (MEMORY_selftest_enabled) {
			/* Disable Self Test ROM */
			if (MEMORY_ram_size > 20) {
				memcpy(MEMORY_mem + 0x5000, under_atarixl_os + 0x1000, 0x800);
				MEMORY_SetRAM(0x5000, 0x57ff);
			}
			else
				MEMORY_dFillMem(0x5000, 0xff, 0x800);
			MEMORY_selftest_enabled = FALSE;
		}
	}
	else {
		/* We can enable Self Test only if the OS ROM is enabled */
		/* and we're not accessing extended 320K Compy Shop or 1088K memory */
		/* Note: in Compy Shop bit 5 (ANTIC access) disables Self Test */
		if (!MEMORY_selftest_enabled && (byte & 0x01)
		&& !((byte & 0x30) != 0x30 && MEMORY_ram_size == MEMORY_RAM_320_COMPY_SHOP)
		&& !((byte & 0x10) == 0 && MEMORY_ram_size == 1088)) {
			/* Enable Self Test ROM */
			if (MEMORY_ram_size > 20) {
				memcpy(under_atarixl_os + 0x1000, MEMORY_mem + 0x5000, 0x800);
				MEMORY_SetROM(0x5000, 0x57ff);
			}
			memcpy(MEMORY_mem + 0x5000, MEMORY_os + 0x1000, 0x800);
			MEMORY_selftest_enabled = TRUE;
		}
	}
}

/* Mosaic banking scheme: writing to 0xffc0+<n> selects ram bank <n>, if 
 * that is past the last available bank, selects rom.  Banks are 4k, 
 * located at 0xc000-0xcfff.  Tested: Rambrandt (drawing program), Topdos1.5.
 * Reverse engineered from software that uses it.  May be incorrect in some
 * details.  Unknown:  were there mirrors of the bank addresses?  Was the RAM
 * enabled at coldstart? Did the Mosaic home-bank on reset?
 * The Topdos 1.5 manual has some information.
 */
static void MosaicPutByte(UWORD addr, UBYTE byte)
{
	int newbank;
	if (addr < 0xffc0) return;
#ifdef DEBUG
	Log_print("MosaicPutByte:%4X:%2X",addr,byte);
#endif
	newbank = addr - 0xffc0;
	if (newbank == mosaic_curbank || (newbank > MEMORY_mosaic_maxbank && mosaic_curbank > MEMORY_mosaic_maxbank)) return; /*same bank or rom -> rom*/
	if (newbank > MEMORY_mosaic_maxbank && mosaic_curbank <= MEMORY_mosaic_maxbank) {
		/*ram ->rom*/
		memcpy(mosaic_ram + mosaic_curbank*0x1000, MEMORY_mem + 0xc000,0x1000);
		MEMORY_dFillMem(0xc000, 0xff, 0x1000);
		MEMORY_SetROM(0xc000, 0xcfff);
	}
	else if (newbank <= MEMORY_mosaic_maxbank && mosaic_curbank > MEMORY_mosaic_maxbank) {
		/*rom->ram*/
		memcpy(MEMORY_mem + 0xc000, mosaic_ram+newbank*0x1000,0x1000);
		MEMORY_SetRAM(0xc000, 0xcfff);
	}
	else {
		/*ram -> ram*/
		memcpy(mosaic_ram + mosaic_curbank*0x1000, MEMORY_mem + 0xc000, 0x1000);
		memcpy(MEMORY_mem + 0xc000, mosaic_ram + newbank*0x1000, 0x1000);
		MEMORY_SetRAM(0xc000, 0xcfff);
	}
	mosaic_curbank = newbank;
}

static UBYTE MosaicGetByte(UWORD addr)
{
#ifdef DEBUG
	Log_print("MosaicGetByte%4X",addr);
#endif
	return MEMORY_mem[addr];
}

/* Axlon banking scheme: writing <n> to 0xcfc0-0xcfff selects a bank.  The Axlon
 * used 3 bits, giving 8 banks.  Extended versions were constructed that
 * used additional bits, for up to 256 banks.  Banks were 16k, at 0x4000-0x7fff.
 * The total ram was 32+16*numbanks k.  The Axlon did homebank on reset,
 * compatibles did not.  The Axlon had a shadow address at 0x0fc0-0x0fff.
 * A possible explaination for the shadow address is that it allowed the
 * Axlon to work in any 800 slot due to a hardware limitation.
 * The shadow address could cause compatibility problems.  The compatibles
 * did not implement that shadow address.
 * Source: comp.sys.atari.8bit postings, Andreas Magenheimer's FAQ
 */
static void AxlonPutByte(UWORD addr, UBYTE byte)
{
	int newbank;
	/*Write-through to RAM if it is the page 0x0f shadow*/
	if ((addr&0xff00) == 0x0f00) MEMORY_mem[addr] = byte;
	if ((addr&0xff) < 0xc0) return; /*0xffc0-0xffff and 0x0fc0-0x0fff only*/
#ifdef DEBUG
	Log_print("AxlonPutByte:%4X:%2X", addr, byte);
#endif
	newbank = (byte&MEMORY_axlon_bankmask);
	if (newbank == axlon_curbank) return;
	memcpy(axlon_ram + axlon_curbank*0x4000, MEMORY_mem + 0x4000, 0x4000);
	memcpy(MEMORY_mem + 0x4000, axlon_ram + newbank*0x4000, 0x4000);
	axlon_curbank = newbank;
}

static UBYTE AxlonGetByte(UWORD addr)
{
#ifdef DEBUG
	Log_print("AxlonGetByte%4X",addr);
#endif
	return MEMORY_mem[addr];
}

static int cart809F_enabled = FALSE;
int MEMORY_cartA0BF_enabled = FALSE;
static UBYTE under_cart809F[8192];
static UBYTE under_cartA0BF[8192];

void MEMORY_Cart809fDisable(void)
{
	if (cart809F_enabled) {
		if (MEMORY_ram_size > 32) {
			memcpy(MEMORY_mem + 0x8000, under_cart809F, 0x2000);
			MEMORY_SetRAM(0x8000, 0x9fff);
		}
		else
			MEMORY_dFillMem(0x8000, 0xff, 0x2000);
		cart809F_enabled = FALSE;
	}
}

void MEMORY_Cart809fEnable(void)
{
	if (!cart809F_enabled) {
		if (MEMORY_ram_size > 32) {
			memcpy(under_cart809F, MEMORY_mem + 0x8000, 0x2000);
			MEMORY_SetROM(0x8000, 0x9fff);
		}
		cart809F_enabled = TRUE;
	}
}

void MEMORY_CartA0bfDisable(void)
{
	if (MEMORY_cartA0BF_enabled) {
		/* No BASIC if not XL/XE or bit 1 of PORTB set */
		/* or accessing extended 576K or 1088K memory */
		if ((Atari800_machine_type != Atari800_MACHINE_XLXE) || basic_disabled((UBYTE) (PIA_PORTB | PIA_PORTB_mask))) {
			if (MEMORY_ram_size > 40) {
				memcpy(MEMORY_mem + 0xa000, under_cartA0BF, 0x2000);
				MEMORY_SetRAM(0xa000, 0xbfff);
			}
			else
				MEMORY_dFillMem(0xa000, 0xff, 0x2000);
		}
		else
			memcpy(MEMORY_mem + 0xa000, MEMORY_basic, 0x2000);
		MEMORY_cartA0BF_enabled = FALSE;
		if (Atari800_machine_type == Atari800_MACHINE_XLXE) {
			GTIA_TRIG[3] = 0;
			if (GTIA_GRACTL & 4)
				GTIA_TRIG_latch[3] = 0;
		}
	}
}

void MEMORY_CartA0bfEnable(void)
{
	if (!MEMORY_cartA0BF_enabled) {
		/* No BASIC if not XL/XE or bit 1 of PORTB set */
		/* or accessing extended 576K or 1088K memory */
		if (MEMORY_ram_size > 40 && ((Atari800_machine_type != Atari800_MACHINE_XLXE) || (PIA_PORTB & 0x02)
		|| ((PIA_PORTB & 0x10) == 0 && (MEMORY_ram_size == 576 || MEMORY_ram_size == 1088)))) {
			/* Back-up 0xa000-0xbfff RAM */
			memcpy(under_cartA0BF, MEMORY_mem + 0xa000, 0x2000);
			MEMORY_SetROM(0xa000, 0xbfff);
		}
		MEMORY_cartA0BF_enabled = TRUE;
		if (Atari800_machine_type == Atari800_MACHINE_XLXE)
			GTIA_TRIG[3] = 1;
	}
}

void MEMORY_GetCharset(UBYTE *cs)
{
	const UBYTE *p;
	switch (Atari800_machine_type) {
	case Atari800_MACHINE_OSA:
	case Atari800_MACHINE_OSB:
		p = MEMORY_mem + 0xe000;
		break;
	case Atari800_MACHINE_XLXE:
		p = MEMORY_os + 0x2000;
		break;
	case Atari800_MACHINE_5200:
		p = MEMORY_mem + 0xf800;
		break;
	default:
		/* shouldn't happen */
		return;
	}
	/* copy font, but change screencode order to ATASCII order */
	memcpy(cs, p + 0x200, 0x100); /* control chars */
	memcpy(cs + 0x100, p, 0x200); /* !"#$..., uppercase letters */
	memcpy(cs + 0x300, p + 0x300, 0x100); /* lowercase letters */
}

#ifndef PAGED_MEM
UBYTE MEMORY_HwGetByte(UWORD addr)
{
	UBYTE byte = 0xff;
	switch (addr & 0xff00) {
	case 0x4f00:
	case 0x8f00:
		CARTRIDGE_BountyBob1(addr);
		byte = 0;
		break;
	case 0x5f00:
	case 0x9f00:
		CARTRIDGE_BountyBob2(addr);
		byte = 0;
		break;
	case 0xd000:				/* GTIA */
	case 0xc000:				/* GTIA - 5200 */
	case 0xc100:				/* GTIA - 5200 */
	case 0xc200:				/* GTIA - 5200 */
	case 0xc300:				/* GTIA - 5200 */
	case 0xc400:				/* GTIA - 5200 */
	case 0xc500:				/* GTIA - 5200 */
	case 0xc600:				/* GTIA - 5200 */
	case 0xc700:				/* GTIA - 5200 */
	case 0xc800:				/* GTIA - 5200 */
	case 0xc900:				/* GTIA - 5200 */
	case 0xca00:				/* GTIA - 5200 */
	case 0xcb00:				/* GTIA - 5200 */
	case 0xcc00:				/* GTIA - 5200 */
	case 0xcd00:				/* GTIA - 5200 */
	case 0xce00:				/* GTIA - 5200 */
		byte = GTIA_GetByte(addr);
		break;
	case 0xd200:				/* POKEY */
	case 0xe800:				/* POKEY - 5200 */
	case 0xe900:				/* POKEY - 5200 */
	case 0xea00:				/* POKEY - 5200 */
	case 0xeb00:				/* POKEY - 5200 */
	case 0xec00:				/* POKEY - 5200 */
	case 0xed00:				/* POKEY - 5200 */
	case 0xee00:				/* POKEY - 5200 */
	case 0xef00:				/* POKEY - 5200 */
		byte = POKEY_GetByte(addr);
		break;
	case 0xd300:				/* PIA */
		byte = PIA_GetByte(addr);
		break;
	case 0xd400:				/* ANTIC */
		byte = ANTIC_GetByte(addr);
		break;
	case 0xd500:				/* bank-switching cartridges, RTIME-8 */
		byte = CARTRIDGE_GetByte(addr);
		break;
	case 0xff00:				/* Mosaic memory expansion for 400/800 */
		byte = MosaicGetByte(addr);
		break;
	case 0xcf00:				/* Axlon memory expansion for 800 */
	case 0x0f00:				/* Axlon shadow */
		if (Atari800_machine_type == Atari800_MACHINE_5200) {
			byte = GTIA_GetByte(addr); /* GTIA-5200 cfxx */
		}
		else {
			byte = AxlonGetByte(addr);
		}
		break;
	case 0xd100:				/* PBI page D1 */
		byte = PBI_D1GetByte(addr);
		break;
	case 0xd600:				/* PBI page D6 */
		byte = PBI_D6GetByte(addr);
		break;
	case 0xd700:				/* PBI page D7 */
		byte = PBI_D7GetByte(addr);
		break;
	default:
		break;
	}

	return byte;
}

void MEMORY_HwPutByte(UWORD addr, UBYTE byte)
{
	switch (addr & 0xff00) {
	case 0x4f00:
	case 0x8f00:
		CARTRIDGE_BountyBob1(addr);
		break;
	case 0x5f00:
	case 0x9f00:
		CARTRIDGE_BountyBob2(addr);
		break;
	case 0xd000:				/* GTIA */
	case 0xc000:				/* GTIA - 5200 */
	case 0xc100:				/* GTIA - 5200 */
	case 0xc200:				/* GTIA - 5200 */
	case 0xc300:				/* GTIA - 5200 */
	case 0xc400:				/* GTIA - 5200 */
	case 0xc500:				/* GTIA - 5200 */
	case 0xc600:				/* GTIA - 5200 */
	case 0xc700:				/* GTIA - 5200 */
	case 0xc800:				/* GTIA - 5200 */
	case 0xc900:				/* GTIA - 5200 */
	case 0xca00:				/* GTIA - 5200 */
	case 0xcb00:				/* GTIA - 5200 */
	case 0xcc00:				/* GTIA - 5200 */
	case 0xcd00:				/* GTIA - 5200 */
	case 0xce00:				/* GTIA - 5200 */
		GTIA_PutByte(addr, byte);
		break;
	case 0xd200:				/* POKEY */
	case 0xe800:				/* POKEY - 5200 */
	case 0xe900:				/* POKEY - 5200 */
	case 0xea00:				/* POKEY - 5200 */
	case 0xeb00:				/* POKEY - 5200 */
	case 0xec00:				/* POKEY - 5200 */
	case 0xed00:				/* POKEY - 5200 */
	case 0xee00:				/* POKEY - 5200 */
	case 0xef00:				/* POKEY - 5200 */
		POKEY_PutByte(addr, byte);
		break;
	case 0xd300:				/* PIA */
		PIA_PutByte(addr, byte);
		break;
	case 0xd400:				/* ANTIC */
		ANTIC_PutByte(addr, byte);
		break;
	case 0xd500:				/* bank-switching cartridges, RTIME-8 */
		CARTRIDGE_PutByte(addr, byte);
		break;
	case 0xff00:				/* Mosaic memory expansion for 400/800 */
		MosaicPutByte(addr,byte);
		break;
	case 0xcf00:				/* Axlon memory expansion for 800 */
	case 0x0f00:				/* Axlon shadow */
		if (Atari800_machine_type == Atari800_MACHINE_5200) {
			GTIA_PutByte(addr, byte); /* GTIA-5200 cfxx */
		}
		else {
			AxlonPutByte(addr,byte);
		}
		break;
	case 0xd100:				/* PBI page D1 */
		PBI_D1PutByte(addr, byte);
		break;
	case 0xd600:				/* PBI page D6 */
		PBI_D6PutByte(addr, byte);
		break;
	case 0xd700:				/* PBI page D7 */
		PBI_D7PutByte(addr, byte);
		break;
	default:
		break;
	}
}
#endif /* PAGED_MEM */
