/**********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2007  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),
                             zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja


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
                             Andreas Naive (andreasnaive@gmail.com)
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
                             Kris Bleakley,
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001-2006    byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight,

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound DSP emulator code is derived from SNEeSe and OpenSPC:
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2007  zones


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com

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
**********************************************************************************/



#ifndef _apumemory_h_
#define _apumemory_h_

START_EXTERN_C
extern uint8 APUROM[64];
END_EXTERN_C

static INLINE uint8 apu_get_reg (uint8 Address)
{
	switch (Address)
	{
		case 0xf0:	// -w TEST
			return 0;

		case 0xf1:	// -w CONTROL
			return 0;

		case 0xf2:	// rw DSPADDR
			return (IAPU.RAM[Address]);

		case 0xf3:	// rw DSPDATA
			return (S9xGetAPUDSP());

		case 0xf4:	// r- CPUI0
		case 0xf5:	// r- CPUI1
		case 0xf6:	// r- CPUI2
		case 0xf7:	// r- CPUI3
		#ifdef SPC700_SHUTDOWN
			IAPU.WaitAddress2 = IAPU.WaitAddress1;
			IAPU.WaitAddress1 = IAPU.PC;
		#endif
			return (IAPU.RAM[Address]);

		case 0xf8:	// rw - Normal RAM
		case 0xf9:	// rw - Normal RAM
			return (IAPU.RAM[Address]);

		case 0xfa:	// -w T0TARGET
		case 0xfb:	// -w T1TARGET
		case 0xfc:	// -w T2TARGET
			return 0;

		case 0xfd:	// r- T0OUT
		case 0xfe:	// r- T1OUT
		case 0xff:	// r- T2OUT
		#ifdef SPC700_SHUTDOWN
			IAPU.WaitAddress2 = IAPU.WaitAddress1;
			IAPU.WaitAddress1 = IAPU.PC;
		#endif
			uint8 t = IAPU.RAM[Address] & 0xF;
			IAPU.RAM[Address] = 0;
			return (t);
	}

	return 0;
}

static INLINE void apu_set_reg (uint8 byte, uint8 Address)
{
	switch (Address)
	{
		case 0xf0:	// -w TEST
			//printf("Write %02X to APU 0xF0!\n", byte);
			return;

		case 0xf1:	// -w CONTROL
			S9xSetAPUControl(byte);
			return;

		case 0xf2:	// rw DSPADDR
			IAPU.RAM[Address] = byte;
			return;

		case 0xf3:	// rw DSPDATA
			S9xSetAPUDSP(byte);
			return;

		case 0xf4:	// -w CPUO0
		case 0xf5:	// -w CPUO1
		case 0xf6:	// -w CPUO2
		case 0xf7:	// -w CPUO3
			APU.OutPorts[Address - 0xf4] = byte;
			return;

		case 0xf8:	// rw - Normal RAM
		case 0xf9:	// rw - Normal RAM
			IAPU.RAM[Address] = byte;
			return;

		case 0xfa:	// -w T0TARGET
		case 0xfb:	// -w T1TARGET
		case 0xfc:	// -w T2TARGET
			IAPU.RAM[Address] = byte;
			if (byte == 0)
				APU.TimerTarget[Address - 0xfa] = 0x100;
			else
				APU.TimerTarget[Address - 0xfa] = byte;
			return;

		case 0xfd:	// r- T0OUT
		case 0xfe:	// r- T1OUT
		case 0xff:	// r- T2OUT
			return;
	}
}

INLINE uint8 S9xAPUGetByteZ (uint8 Address)
{
	if (Address >= 0xf0 && IAPU.DirectPage == IAPU.RAM)
		return (apu_get_reg(Address));
	else
		return (IAPU.DirectPage[Address]);
}

INLINE void S9xAPUSetByteZ (uint8 byte, uint8 Address)
{
    if (Address >= 0xf0 && IAPU.DirectPage == IAPU.RAM)
		apu_set_reg(byte, Address);
    else
		IAPU.DirectPage[Address] = byte;
}

INLINE uint8 S9xAPUGetByte (uint32 Address)
{
    Address &= 0xffff;
    if (Address <= 0xff && Address >= 0xf0)
		return (apu_get_reg(Address & 0xff));
    else
		return (IAPU.RAM[Address]);
}

INLINE void S9xAPUSetByte (uint8 byte, uint32 Address)
{
    Address &= 0xffff;
    if (Address <= 0xff && Address >= 0xf0)
		apu_set_reg(byte, Address & 0xff);
	else
	if (Address < 0xffc0)
	    IAPU.RAM[Address] = byte;
	else
	{
	    APU.ExtraRAM[Address - 0xffc0] = byte;
	    if (!APU.ShowROM)
		IAPU.RAM[Address] = byte;
	}
}

#endif // _apumemory_h_
