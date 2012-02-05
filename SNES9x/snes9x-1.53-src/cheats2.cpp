/***********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2010  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),

  (c) Copyright 2002 - 2011  zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja

  (c) Copyright 2009 - 2011  BearOso,
                             OV2


  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com),
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti

  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code used in 1.39-1.51
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  SPC7110 and RTC C++ emulator code used in 1.52+
  (c) Copyright 2009         byuu,
                             neviksti

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001 - 2006  byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound emulator code used in 1.5-1.51
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  Sound emulator code used in 1.52+
  (c) Copyright 2004 - 2007  Shay Green (gblargg@gmail.com)

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  NTSC filter
  (c) Copyright 2006 - 2007  Shay Green

  GTK+ GUI code
  (c) Copyright 2004 - 2011  BearOso

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja
  (c) Copyright 2009 - 2011  OV2

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2011  zones


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com/

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
 ***********************************************************************************/


#include "snes9x.h"
#include "memmap.h"
#include "cheats.h"

static uint8 S9xGetByteFree (uint32);
static void S9xSetByteFree (uint8, uint32);


static uint8 S9xGetByteFree (uint32 address)
{
	uint32	Cycles = CPU.Cycles;
	uint8	byte;

	byte = S9xGetByte(address);
	CPU.Cycles = Cycles;

	return (byte);
}

static void S9xSetByteFree (uint8 byte, uint32 address)
{
	uint32	Cycles = CPU.Cycles;

	S9xSetByte(byte, address);
	CPU.Cycles = Cycles;
}

void S9xInitWatchedAddress (void)
{
	for (unsigned int i = 0; i < sizeof(watches) / sizeof(watches[0]); i++)
		watches[i].on = false;

}

void S9xInitCheatData (void)
{
	Cheat.RAM = Memory.RAM;
	Cheat.SRAM = Memory.SRAM;
	Cheat.FillRAM = Memory.FillRAM;
}

void S9xAddCheat (bool8 enable, bool8 save_current_value, uint32 address, uint8 byte)
{
	if (Cheat.num_cheats < sizeof(Cheat.c) / sizeof(Cheat.c[0]))
	{
		Cheat.c[Cheat.num_cheats].address = address;
		Cheat.c[Cheat.num_cheats].byte = byte;
		Cheat.c[Cheat.num_cheats].enabled = enable;

		if (save_current_value)
		{
			Cheat.c[Cheat.num_cheats].saved_byte = S9xGetByteFree(address);
			Cheat.c[Cheat.num_cheats].saved = TRUE;
		}

		Cheat.num_cheats++;
	}
}

void S9xDeleteCheat (uint32 which1)
{
	if (which1 < Cheat.num_cheats)
	{
		if (Cheat.c[which1].enabled)
			S9xRemoveCheat(which1);

		memmove(&Cheat.c[which1], &Cheat.c[which1 + 1], sizeof(Cheat.c[0]) * (Cheat.num_cheats - which1 - 1));

		Cheat.num_cheats--;
	}
}

void S9xDeleteCheats (void)
{
	S9xRemoveCheats();
	Cheat.num_cheats = 0;
}

void S9xRemoveCheat (uint32 which1)
{
	if (Cheat.c[which1].saved)
	{
		uint32	address = Cheat.c[which1].address;

		int		block = (address & 0xffffff) >> MEMMAP_SHIFT;
		uint8	*ptr = Memory.Map[block];

		if (ptr >= (uint8 *) CMemory::MAP_LAST)
			*(ptr + (address & 0xffff)) = Cheat.c[which1].saved_byte;
		else
			S9xSetByteFree(Cheat.c[which1].saved_byte, address);
	}
}

void S9xRemoveCheats (void)
{
	for (uint32 i = 0; i < Cheat.num_cheats; i++)
		if (Cheat.c[i].enabled)
			S9xRemoveCheat(i);
}

void S9xEnableCheat (uint32 which1)
{
	if (which1 < Cheat.num_cheats && !Cheat.c[which1].enabled)
	{
		Cheat.c[which1].enabled = TRUE;
		S9xApplyCheat(which1);
	}
}

void S9xDisableCheat (uint32 which1)
{
	if (which1 < Cheat.num_cheats && Cheat.c[which1].enabled)
	{
		S9xRemoveCheat(which1);
		Cheat.c[which1].enabled = FALSE;
	}
}

void S9xApplyCheat (uint32 which1)
{
	uint32	address = Cheat.c[which1].address;

	if (!Cheat.c[which1].saved)
	{
		Cheat.c[which1].saved_byte = S9xGetByteFree(address);
		Cheat.c[which1].saved = TRUE;
	}

	int		block = (address & 0xffffff) >> MEMMAP_SHIFT;
	uint8	*ptr = Memory.Map[block];

	if (ptr >= (uint8 *) CMemory::MAP_LAST)
		*(ptr + (address & 0xffff)) = Cheat.c[which1].byte;
	else
		S9xSetByteFree(Cheat.c[which1].byte, address);
}

void S9xApplyCheats (void)
{
	if (Settings.ApplyCheats)
	{
		for (uint32 i = 0; i < Cheat.num_cheats; i++)
			if (Cheat.c[i].enabled)
				S9xApplyCheat(i);
	}
}

bool8 S9xLoadCheatFile (const char *filename)
{
	FILE	*fs;
	uint8	data[28];

	Cheat.num_cheats = 0;

	fs = fopen(filename, "rb");
	if (!fs)
		return (FALSE);

	while (fread((void *) data, 1, 28, fs) == 28)
	{
		Cheat.c[Cheat.num_cheats].enabled = (data[0] & 4) == 0;
		Cheat.c[Cheat.num_cheats].byte = data[1];
		Cheat.c[Cheat.num_cheats].address = data[2] | (data[3] << 8) |  (data[4] << 16);
		Cheat.c[Cheat.num_cheats].saved_byte = data[5];
		Cheat.c[Cheat.num_cheats].saved = (data[0] & 8) != 0;
		memmove(Cheat.c[Cheat.num_cheats].name, &data[8], 20);
		Cheat.c[Cheat.num_cheats++].name[20] = 0;
	}

	fclose(fs);

	return (TRUE);
}

bool8 S9xSaveCheatFile (const char *filename)
{
	if (Cheat.num_cheats == 0)
	{
		remove(filename);
		return (TRUE);
	}

	FILE	*fs;
	uint8	data[28];

	fs = fopen(filename, "wb");
	if (!fs)
		return (FALSE);

	for (uint32 i = 0; i < Cheat.num_cheats; i++)
	{
		ZeroMemory(data, 28);

		if (i == 0)
		{
			data[6] = 254;
			data[7] = 252;
		}

		if (!Cheat.c[i].enabled)
			data[0] |= 4;

		if (Cheat.c[i].saved)
			data[0] |= 8;

		data[1] = Cheat.c[i].byte;
		data[2] = (uint8) (Cheat.c[i].address >> 0);
		data[3] = (uint8) (Cheat.c[i].address >> 8);
		data[4] = (uint8) (Cheat.c[i].address >> 16);
		data[5] = Cheat.c[i].saved_byte;

		memmove(&data[8], Cheat.c[i].name, 19);

		if (fwrite(data, 28, 1, fs) != 1)
		{
			fclose(fs);
			return (FALSE);
		}
	}

	return (fclose(fs) == 0);
}
