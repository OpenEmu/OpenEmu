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

/*****
 * SPC7110 emulator - version 0.03 (2008-08-10)
 * Copyright (c) 2008, byuu and neviksti
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * The software is provided "as is" and the author disclaims all warranties
 * with regard to this software including all implied warranties of
 * merchantibility and fitness, in no event shall the author be liable for
 * any special, direct, indirect, or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether in an
 * action of contract, negligence or other tortious action, arising out of
 * or in connection with the use or performance of this software.
 *****/


#include <limits>

#include "snes9x.h"
#include "memmap.h"
#include "srtc.h"
#include "display.h"

#define memory_cartrom_size()		Memory.CalculatedSize
#define memory_cartrom_read(a)		Memory.ROM[(a)]
#define memory_cartrtc_read(a)		RTCData.reg[(a)]
#define memory_cartrtc_write(a, b)	{ RTCData.reg[(a)] = (b); }
#define cartridge_info_spc7110rtc	Settings.SPC7110RTC
#define cpu_regs_mdr				OpenBus

#include "spc7110emu.h"
#include "spc7110emu.cpp"

SPC7110	s7emu;

static void SetSPC7110SRAMMap (uint8);


void S9xInitSPC7110 (void)
{
	s7emu.power();
	memset(RTCData.reg, 0, 20);
}

void S9xResetSPC7110 (void)
{
	s7emu.reset();
}

static void SetSPC7110SRAMMap (uint8 newstate)
{
	if (newstate & 0x80)
	{
		Memory.Map[0x006] = (uint8 *) Memory.MAP_HIROM_SRAM;
		Memory.Map[0x007] = (uint8 *) Memory.MAP_HIROM_SRAM;
		Memory.Map[0x306] = (uint8 *) Memory.MAP_HIROM_SRAM;
		Memory.Map[0x307] = (uint8 *) Memory.MAP_HIROM_SRAM;
	}
	else
	{
		Memory.Map[0x006] = (uint8 *) Memory.MAP_RONLY_SRAM;
		Memory.Map[0x007] = (uint8 *) Memory.MAP_RONLY_SRAM;
		Memory.Map[0x306] = (uint8 *) Memory.MAP_RONLY_SRAM;
		Memory.Map[0x307] = (uint8 *) Memory.MAP_RONLY_SRAM;
	}
}

uint8 * S9xGetBasePointerSPC7110 (uint32 address)
{
	uint32	i;
	
	switch (address & 0xf00000)
	{
		case 0xd00000:
			i = s7emu.dx_offset;
			break;
			
		case 0xe00000:
			i = s7emu.ex_offset;
			break;
			
		case 0xf00000:
			i = s7emu.fx_offset;
			break;
			
		default:
			i = 0;
			break;
	}
	
	i += address & 0x0f0000;

	return (&Memory.ROM[i]);
}

uint8 S9xGetSPC7110Byte (uint32 address)
{
	uint32	i;

	switch (address & 0xf00000)
	{
		case 0xd00000:
			i = s7emu.dx_offset;
			break;

		case 0xe00000:
			i = s7emu.ex_offset;
			break;

		case 0xf00000:
			i = s7emu.fx_offset;
			break;

		default:
			i = 0;
			break;
	}

	i += address & 0x0fffff;

	return (Memory.ROM[i]);
}

uint8 S9xGetSPC7110 (uint16 address)
{
	if (!Settings.SPC7110RTC && address > 0x483f)
		return (OpenBus);
	
	return (s7emu.mmio_read(address));
}

void S9xSetSPC7110 (uint8 byte, uint16 address)
{
	if (!Settings.SPC7110RTC && address > 0x483f)
		return;

	if (address == 0x4830)
		SetSPC7110SRAMMap(byte);

	s7emu.mmio_write(address, byte);
}

void S9xSPC7110PreSaveState (void)
{
	s7snap.r4801 = s7emu.r4801;
	s7snap.r4802 = s7emu.r4802;
	s7snap.r4803 = s7emu.r4803;
	s7snap.r4804 = s7emu.r4804;
	s7snap.r4805 = s7emu.r4805;
	s7snap.r4806 = s7emu.r4806;
	s7snap.r4807 = s7emu.r4807;
	s7snap.r4808 = s7emu.r4808;
	s7snap.r4809 = s7emu.r4809;
	s7snap.r480a = s7emu.r480a;
	s7snap.r480b = s7emu.r480b;
	s7snap.r480c = s7emu.r480c;

	s7snap.r4811 = s7emu.r4811;
	s7snap.r4812 = s7emu.r4812;
	s7snap.r4813 = s7emu.r4813;
	s7snap.r4814 = s7emu.r4814;
	s7snap.r4815 = s7emu.r4815;
	s7snap.r4816 = s7emu.r4816;
	s7snap.r4817 = s7emu.r4817;
	s7snap.r4818 = s7emu.r4818;

	s7snap.r481x = s7emu.r481x;

	s7snap.r4814_latch = s7emu.r4814_latch ? TRUE : FALSE;
	s7snap.r4815_latch = s7emu.r4815_latch ? TRUE : FALSE;

	s7snap.r4820 = s7emu.r4820;
	s7snap.r4821 = s7emu.r4821;
	s7snap.r4822 = s7emu.r4822;
	s7snap.r4823 = s7emu.r4823;
	s7snap.r4824 = s7emu.r4824;
	s7snap.r4825 = s7emu.r4825;
	s7snap.r4826 = s7emu.r4826;
	s7snap.r4827 = s7emu.r4827;
	s7snap.r4828 = s7emu.r4828;
	s7snap.r4829 = s7emu.r4829;
	s7snap.r482a = s7emu.r482a;
	s7snap.r482b = s7emu.r482b;
	s7snap.r482c = s7emu.r482c;
	s7snap.r482d = s7emu.r482d;
	s7snap.r482e = s7emu.r482e;
	s7snap.r482f = s7emu.r482f;

	s7snap.r4830 = s7emu.r4830;
	s7snap.r4831 = s7emu.r4831;
	s7snap.r4832 = s7emu.r4832;
	s7snap.r4833 = s7emu.r4833;
	s7snap.r4834 = s7emu.r4834;

	s7snap.dx_offset = (uint32) s7emu.dx_offset;
	s7snap.ex_offset = (uint32) s7emu.ex_offset;
	s7snap.fx_offset = (uint32) s7emu.fx_offset;

	s7snap.r4840 = s7emu.r4840;
	s7snap.r4841 = s7emu.r4841;
	s7snap.r4842 = s7emu.r4842;

	s7snap.rtc_state = (int32)  s7emu.rtc_state;
	s7snap.rtc_mode  = (int32)  s7emu.rtc_mode;
	s7snap.rtc_index = (uint32) s7emu.rtc_index;

	s7snap.decomp_mode   = (uint32) s7emu.decomp.decomp_mode;
	s7snap.decomp_offset = (uint32) s7emu.decomp.decomp_offset;

	for (int i = 0; i < SPC7110_DECOMP_BUFFER_SIZE; i++)
		s7snap.decomp_buffer[i] = s7emu.decomp.decomp_buffer[i];

	s7snap.decomp_buffer_rdoffset = (uint32) s7emu.decomp.decomp_buffer_rdoffset;
	s7snap.decomp_buffer_wroffset = (uint32) s7emu.decomp.decomp_buffer_wroffset;
	s7snap.decomp_buffer_length   = (uint32) s7emu.decomp.decomp_buffer_length;

	for (int i = 0; i < 32; i++)
	{
		s7snap.context[i].index  = s7emu.decomp.context[i].index;
		s7snap.context[i].invert = s7emu.decomp.context[i].invert;
	}
}

void S9xSPC7110PostLoadState (int version)
{
	s7emu.r4801 = s7snap.r4801;
	s7emu.r4802 = s7snap.r4802;
	s7emu.r4803 = s7snap.r4803;
	s7emu.r4804 = s7snap.r4804;
	s7emu.r4805 = s7snap.r4805;
	s7emu.r4806 = s7snap.r4806;
	s7emu.r4807 = s7snap.r4807;
	s7emu.r4808 = s7snap.r4808;
	s7emu.r4809 = s7snap.r4809;
	s7emu.r480a = s7snap.r480a;
	s7emu.r480b = s7snap.r480b;
	s7emu.r480c = s7snap.r480c;

	s7emu.r4811 = s7snap.r4811;
	s7emu.r4812 = s7snap.r4812;
	s7emu.r4813 = s7snap.r4813;
	s7emu.r4814 = s7snap.r4814;
	s7emu.r4815 = s7snap.r4815;
	s7emu.r4816 = s7snap.r4816;
	s7emu.r4817 = s7snap.r4817;
	s7emu.r4818 = s7snap.r4818;

	s7emu.r481x = s7snap.r481x;

	s7emu.r4814_latch = s7snap.r4814_latch ? true : false;
	s7emu.r4815_latch = s7snap.r4815_latch ? true : false;

	s7emu.r4820 = s7snap.r4820;
	s7emu.r4821 = s7snap.r4821;
	s7emu.r4822 = s7snap.r4822;
	s7emu.r4823 = s7snap.r4823;
	s7emu.r4824 = s7snap.r4824;
	s7emu.r4825 = s7snap.r4825;
	s7emu.r4826 = s7snap.r4826;
	s7emu.r4827 = s7snap.r4827;
	s7emu.r4828 = s7snap.r4828;
	s7emu.r4829 = s7snap.r4829;
	s7emu.r482a = s7snap.r482a;
	s7emu.r482b = s7snap.r482b;
	s7emu.r482c = s7snap.r482c;
	s7emu.r482d = s7snap.r482d;
	s7emu.r482e = s7snap.r482e;
	s7emu.r482f = s7snap.r482f;

	s7emu.r4830 = s7snap.r4830;
	s7emu.r4831 = s7snap.r4831;
	s7emu.r4832 = s7snap.r4832;
	s7emu.r4833 = s7snap.r4833;
	s7emu.r4834 = s7snap.r4834;

	s7emu.dx_offset = (unsigned) s7snap.dx_offset;
	s7emu.ex_offset = (unsigned) s7snap.ex_offset;
	s7emu.fx_offset = (unsigned) s7snap.fx_offset;

	s7emu.r4840 = s7snap.r4840;
	s7emu.r4841 = s7snap.r4841;
	s7emu.r4842 = s7snap.r4842;

	s7emu.rtc_state = (SPC7110::RTC_State) s7snap.rtc_state;
	s7emu.rtc_mode  = (SPC7110::RTC_Mode)  s7snap.rtc_mode;
	s7emu.rtc_index = (unsigned)           s7snap.rtc_index;

	s7emu.decomp.decomp_mode   = (unsigned) s7snap.decomp_mode;
	s7emu.decomp.decomp_offset = (unsigned) s7snap.decomp_offset;

	for (int i = 0; i < SPC7110_DECOMP_BUFFER_SIZE; i++)
		s7emu.decomp.decomp_buffer[i] = s7snap.decomp_buffer[i];

	s7emu.decomp.decomp_buffer_rdoffset = (unsigned) s7snap.decomp_buffer_rdoffset;
	s7emu.decomp.decomp_buffer_wroffset = (unsigned) s7snap.decomp_buffer_wroffset;
	s7emu.decomp.decomp_buffer_length   = (unsigned) s7snap.decomp_buffer_length;

	for (int i = 0; i < 32; i++)
	{
		s7emu.decomp.context[i].index  = s7snap.context[i].index;
		s7emu.decomp.context[i].invert = s7snap.context[i].invert;
	}

	s7emu.update_time(0);
}
