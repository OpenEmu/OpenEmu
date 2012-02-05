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

static void (*SetDSP3) (void);

static const uint16	DSP3_DataROM[1024] =
{
	0x8000, 0x4000, 0x2000, 0x1000, 0x0800, 0x0400, 0x0200, 0x0100,
	0x0080, 0x0040, 0x0020, 0x0010, 0x0008, 0x0004, 0x0002, 0x0001,
	0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080, 0x0100,
	0x0000, 0x000f, 0x0400, 0x0200, 0x0140, 0x0400, 0x0200, 0x0040,
	0x007d, 0x007e, 0x007e, 0x007b, 0x007c, 0x007d, 0x007b, 0x007c,
	0x0002, 0x0020, 0x0030, 0x0000, 0x000d, 0x0019, 0x0026, 0x0032,
	0x003e, 0x004a, 0x0056, 0x0062, 0x006d, 0x0079, 0x0084, 0x008e,
	0x0098, 0x00a2, 0x00ac, 0x00b5, 0x00be, 0x00c6, 0x00ce, 0x00d5,
	0x00dc, 0x00e2, 0x00e7, 0x00ec, 0x00f1, 0x00f5, 0x00f8, 0x00fb,
	0x00fd, 0x00ff, 0x0100, 0x0100, 0x0100, 0x00ff, 0x00fd, 0x00fb,
	0x00f8, 0x00f5, 0x00f1, 0x00ed, 0x00e7, 0x00e2, 0x00dc, 0x00d5,
	0x00ce, 0x00c6, 0x00be, 0x00b5, 0x00ac, 0x00a2, 0x0099, 0x008e,
	0x0084, 0x0079, 0x006e, 0x0062, 0x0056, 0x004a, 0x003e, 0x0032,
	0x0026, 0x0019, 0x000d, 0x0000, 0xfff3, 0xffe7, 0xffdb, 0xffce,
	0xffc2, 0xffb6, 0xffaa, 0xff9e, 0xff93, 0xff87, 0xff7d, 0xff72,
	0xff68, 0xff5e, 0xff54, 0xff4b, 0xff42, 0xff3a, 0xff32, 0xff2b,
	0xff25, 0xff1e, 0xff19, 0xff14, 0xff0f, 0xff0b, 0xff08, 0xff05,
	0xff03, 0xff01, 0xff00, 0xff00, 0xff00, 0xff01, 0xff03, 0xff05,
	0xff08, 0xff0b, 0xff0f, 0xff13, 0xff18, 0xff1e, 0xff24, 0xff2b,
	0xff32, 0xff3a, 0xff42, 0xff4b, 0xff54, 0xff5d, 0xff67, 0xff72,
	0xff7c, 0xff87, 0xff92, 0xff9e, 0xffa9, 0xffb5, 0xffc2, 0xffce,
	0xffda, 0xffe7, 0xfff3, 0x002b, 0x007f, 0x0020, 0x00ff, 0xff00,
	0xffbe, 0x0000, 0x0044, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffc1, 0x0001, 0x0002, 0x0045,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffc5, 0x0003, 0x0004, 0x0005, 0x0047, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffca, 0x0006, 0x0007, 0x0008,
	0x0009, 0x004a, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffd0, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x004e, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffd7, 0x000f, 0x0010, 0x0011,
	0x0012, 0x0013, 0x0014, 0x0053, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffdf, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b,
	0x0059, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffe8, 0x001c, 0x001d, 0x001e,
	0x001f, 0x0020, 0x0021, 0x0022, 0x0023, 0x0060, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xfff2, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a,
	0x002b, 0x002c, 0x0068, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xfffd, 0x002d, 0x002e, 0x002f,
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0071,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffc7, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d,
	0x003e, 0x003f, 0x0040, 0x0041, 0x007b, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffd4, 0x0000, 0x0001, 0x0002,
	0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a,
	0x000b, 0x0044, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffe2, 0x000c, 0x000d, 0x000e, 0x000f, 0x0010, 0x0011, 0x0012,
	0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0050, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xfff1, 0x0019, 0x001a, 0x001b,
	0x001c, 0x001d, 0x001e, 0x001f, 0x0020, 0x0021, 0x0022, 0x0023,
	0x0024, 0x0025, 0x0026, 0x005d, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffcb, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d,
	0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035,
	0x006b, 0x0000, 0x0000, 0x0000, 0xffdc, 0x0000, 0x0001, 0x0002,
	0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a,
	0x000b, 0x000c, 0x000d, 0x000e, 0x000f, 0x0044, 0x0000, 0x0000,
	0xffee, 0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016,
	0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e,
	0x001f, 0x0020, 0x0054, 0x0000, 0xffee, 0x0021, 0x0022, 0x0023,
	0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b,
	0x002c, 0x002d, 0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0x0065,
	0xffbe, 0x0000, 0xfeac, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffc1, 0x0001, 0x0002, 0xfead,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffc5, 0x0003, 0x0004, 0x0005, 0xfeaf, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffca, 0x0006, 0x0007, 0x0008,
	0x0009, 0xfeb2, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffd0, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0xfeb6, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffd7, 0x000f, 0x0010, 0x0011,
	0x0012, 0x0013, 0x0014, 0xfebb, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffdf, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b,
	0xfec1, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffe8, 0x001c, 0x001d, 0x001e,
	0x001f, 0x0020, 0x0021, 0x0022, 0x0023, 0xfec8, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xfff2, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a,
	0x002b, 0x002c, 0xfed0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xfffd, 0x002d, 0x002e, 0x002f,
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0xfed9,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffc7, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d,
	0x003e, 0x003f, 0x0040, 0x0041, 0xfee3, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffd4, 0x0000, 0x0001, 0x0002,
	0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a,
	0x000b, 0xfeac, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffe2, 0x000c, 0x000d, 0x000e, 0x000f, 0x0010, 0x0011, 0x0012,
	0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0xfeb8, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xfff1, 0x0019, 0x001a, 0x001b,
	0x001c, 0x001d, 0x001e, 0x001f, 0x0020, 0x0021, 0x0022, 0x0023,
	0x0024, 0x0025, 0x0026, 0xfec5, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffcb, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d,
	0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035,
	0xfed3, 0x0000, 0x0000, 0x0000, 0xffdc, 0x0000, 0x0001, 0x0002,
	0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a,
	0x000b, 0x000c, 0x000d, 0x000e, 0x000f, 0xfeac, 0x0000, 0x0000,
	0xffee, 0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016,
	0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e,
	0x001f, 0x0020, 0xfebc, 0x0000, 0xffee, 0x0021, 0x0022, 0x0023,
	0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b,
	0x002c, 0x002d, 0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0xfecd,
	0x0154, 0x0218, 0x0110, 0x00b0, 0x00cc, 0x00b0, 0x0088, 0x00b0,
	0x0044, 0x00b0, 0x0000, 0x00b0, 0x00fe, 0xff07, 0x0002, 0x00ff,
	0x00f8, 0x0007, 0x00fe, 0x00ee, 0x07ff, 0x0200, 0x00ef, 0xf800,
	0x0700, 0x00ee, 0xffff, 0xffff, 0xffff, 0x0000, 0x0000, 0x0001,
	0x0001, 0x0001, 0x0001, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff,
	0xffff, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000,
	0x0000, 0xffff, 0xffff, 0x0000, 0xffff, 0x0001, 0x0000, 0x0001,
	0x0001, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0x0000,
	0xffff, 0x0001, 0x0000, 0x0001, 0x0001, 0x0000, 0x0000, 0xffff,
	0xffff, 0xffff, 0x0000, 0x0000, 0x0000, 0x0044, 0x0088, 0x00cc,
	0x0110, 0x0154, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
};

static bool8 DSP3_GetBits (uint8);
//static void DSP3_MemorySize (void);
static void DSP3_TestMemory (void);
static void DSP3_DumpDataROM (void);
static void DSP3_MemoryDump (void);
static void DSP3_Coordinate (void);
static void DSP3_Command (void);
static void DSP3_Decode_Data (void);
static void DSP3_Decode_Tree (void);
static void DSP3_Decode_Symbols (void);
static void DSP3_Decode (void);
static void DSP3_Decode_A (void);
static void DSP3_Convert (void);
static void DSP3_Convert_A (void);
static void DSP3_OP03 (void);
static void DSP3_OP06 (void);
static void DSP3_OP07 (void);
static void DSP3_OP07_A (void);
static void DSP3_OP07_B (void);
static void DSP3_OP0C (void);
//static void DSP3_OP0C_A (void);
static void DSP3_OP10 (void);
static void DSP3_OP1C (void);
static void DSP3_OP1C_A (void);
static void DSP3_OP1C_B (void);
static void DSP3_OP1C_C (void);
static void DSP3_OP1E (void);
static void DSP3_OP1E_A (void);
static void DSP3_OP1E_A1 (void);
static void DSP3_OP1E_A2 (void);
static void DSP3_OP1E_A3 (void);
static void DSP3_OP1E_B (void);
static void DSP3_OP1E_B1 (void);
static void DSP3_OP1E_B2 (void);
static void DSP3_OP1E_C (void);
static void DSP3_OP1E_C1 (void);
static void DSP3_OP1E_C2 (void);
static void DSP3_OP1E_D (int16, int16 *, int16 *);
static void DSP3_OP1E_D1 (int16, int16 *, int16 *);
static void DSP3_OP3E (void);


void DSP3_Reset (void)
{
	DSP3.DR = 0x0080;
	DSP3.SR = 0x0084;
	SetDSP3 = &DSP3_Command;
}

/*
static void DSP3_MemorySize (void)
{
	DSP3.DR = 0x0300;
	SetDSP3 = &DSP3_Reset;
}
*/

static void DSP3_TestMemory (void)
{
	DSP3.DR = 0x0000;
	SetDSP3 = &DSP3_Reset;
}

static void DSP3_DumpDataROM (void)
{
	DSP3.DR = DSP3_DataROM[DSP3.MemoryIndex++];
	if (DSP3.MemoryIndex == 1024)
		SetDSP3 = &DSP3_Reset;
}

static void DSP3_MemoryDump (void)
{
	DSP3.MemoryIndex = 0;
	SetDSP3 = &DSP3_DumpDataROM;
	DSP3_DumpDataROM();
}

static void DSP3_OP06 (void)
{
	DSP3.WinLo = (uint8) (DSP3.DR);
	DSP3.WinHi = (uint8) (DSP3.DR >> 8);
	DSP3_Reset();
}

static void DSP3_OP03 (void)
{
	int16	Lo  = (uint8) (DSP3.DR);
	int16	Hi  = (uint8) (DSP3.DR >> 8);
	int16	Ofs = (DSP3.WinLo * Hi << 1) + (Lo << 1);

	DSP3.DR = Ofs >> 1;
	SetDSP3 = &DSP3_Reset;
}

static void DSP3_OP07_B (void)
{
	int16	Ofs = (DSP3.WinLo * DSP3.AddHi << 1) + (DSP3.AddLo << 1);

	DSP3.DR = Ofs >> 1;
	SetDSP3 = &DSP3_Reset;
}

static void DSP3_OP07_A (void)
{
	int16	Lo = (uint8) (DSP3.DR);
	int16	Hi = (uint8) (DSP3.DR >> 8);

	if (Lo & 1)
		Hi += (DSP3.AddLo & 1);

	DSP3.AddLo += Lo;
	DSP3.AddHi += Hi;

	if (DSP3.AddLo < 0)
		DSP3.AddLo += DSP3.WinLo;
	else
	if (DSP3.AddLo >= DSP3.WinLo)
		DSP3.AddLo -= DSP3.WinLo;

	if (DSP3.AddHi < 0)
		DSP3.AddHi += DSP3.WinHi;
	else
	if (DSP3.AddHi >= DSP3.WinHi)
		DSP3.AddHi -= DSP3.WinHi;

	DSP3.DR = DSP3.AddLo | (DSP3.AddHi << 8) | ((DSP3.AddHi >> 8) & 0xff);
	SetDSP3 = &DSP3_OP07_B;
}

static void DSP3_OP07 (void)
{
	uint32	dataOfs = ((DSP3.DR << 1) + 0x03b2) & 0x03ff;

	DSP3.AddHi = DSP3_DataROM[dataOfs];
	DSP3.AddLo = DSP3_DataROM[dataOfs + 1];

	SetDSP3 = &DSP3_OP07_A;
	DSP3.SR = 0x0080;
}

static void DSP3_Coordinate (void)
{
	DSP3.Index++;

	switch (DSP3.Index)
	{
		case 3:
			if (DSP3.DR == 0xffff)
				DSP3_Reset();
			break;

		case 4:
			DSP3.X = DSP3.DR;
			break;

		case 5:
			DSP3.Y = DSP3.DR;
			DSP3.DR = 1;
			break;

		case 6:
			DSP3.DR = DSP3.X;
			break;

		case 7:
			DSP3.DR = DSP3.Y;
			DSP3.Index = 0;
			break;
	}
}

static void DSP3_Convert_A (void)
{
	if (DSP3.BMIndex < 8)
	{
		DSP3.Bitmap[DSP3.BMIndex++] = (uint8) (DSP3.DR);
		DSP3.Bitmap[DSP3.BMIndex++] = (uint8) (DSP3.DR >> 8);

		if (DSP3.BMIndex == 8)
		{
			for (int i = 0; i < 8; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					DSP3.Bitplane[j] <<= 1;
					DSP3.Bitplane[j] |= (DSP3.Bitmap[i] >> j) & 1;
				}
			}

			DSP3.BPIndex = 0;
			DSP3.Count--;
		}
	}

	if (DSP3.BMIndex == 8)
	{
		if (DSP3.BPIndex == 8)
		{
			if (!DSP3.Count)
				DSP3_Reset();

			DSP3.BMIndex = 0;
		}
		else
		{
			DSP3.DR  = DSP3.Bitplane[DSP3.BPIndex++];
			DSP3.DR |= DSP3.Bitplane[DSP3.BPIndex++] << 8;
		}
	}
}

static void DSP3_Convert (void)
{
	DSP3.Count = DSP3.DR;
	DSP3.BMIndex = 0;
	SetDSP3 = &DSP3_Convert_A;
}

static bool8 DSP3_GetBits (uint8 Count)
{
	if (!DSP3.BitsLeft)
	{
		DSP3.BitsLeft = Count;
		DSP3.ReqBits = 0;
	}

	do
	{
		if (!DSP3.BitCount)
		{
			DSP3.SR = 0xC0;
			return (FALSE);
		}

		DSP3.ReqBits <<= 1;
		if (DSP3.ReqData & 0x8000)
			DSP3.ReqBits++;
		DSP3.ReqData <<= 1;

		DSP3.BitCount--;
		DSP3.BitsLeft--;

	}
	while (DSP3.BitsLeft);

	return (TRUE);
}

static void DSP3_Decode_Data (void)
{
	if (!DSP3.BitCount)
	{
		if (DSP3.SR & 0x40)
		{
			DSP3.ReqData = DSP3.DR;
			DSP3.BitCount += 16;
		}
		else
		{
			DSP3.SR = 0xC0;
			return;
		}
	}

	if (DSP3.LZCode == 1)
	{
		if (!DSP3_GetBits(1))
			return;

		if (DSP3.ReqBits)
			DSP3.LZLength = 12;
		else
			DSP3.LZLength = 8;

		DSP3.LZCode++;
	}

	if (DSP3.LZCode == 2)
	{
		if (!DSP3_GetBits(DSP3.LZLength))
			return;

		DSP3.LZCode = 0;
		DSP3.Outwords--;
		if (!DSP3.Outwords)
			SetDSP3 = &DSP3_Reset;

		DSP3.SR = 0x80;
		DSP3.DR = DSP3.ReqBits;
		return;
	}

	if (DSP3.BaseCode == 0xffff)
	{
		if (!DSP3_GetBits(DSP3.BaseLength))
			return;

		DSP3.BaseCode = DSP3.ReqBits;
	}

	if (!DSP3_GetBits(DSP3.CodeLengths[DSP3.BaseCode]))
		return;

	DSP3.Symbol = DSP3.Codes[DSP3.CodeOffsets[DSP3.BaseCode] + DSP3.ReqBits];
	DSP3.BaseCode = 0xffff;

	if (DSP3.Symbol & 0xff00)
	{
		DSP3.Symbol += 0x7f02;
		DSP3.LZCode++;
	}
	else
	{
		DSP3.Outwords--;
		if (!DSP3.Outwords)
			SetDSP3 = &DSP3_Reset;
	}

	DSP3.SR = 0x80;
	DSP3.DR = DSP3.Symbol;
}

static void DSP3_Decode_Tree (void)
{
	if (!DSP3.BitCount)
	{
		DSP3.ReqData = DSP3.DR;
		DSP3.BitCount += 16;
	}

	if (!DSP3.BaseCodes)
	{
		DSP3_GetBits(1);

		if (DSP3.ReqBits)
		{
			DSP3.BaseLength = 3;
			DSP3.BaseCodes = 8;
		}
		else
		{
			DSP3.BaseLength = 2;
			DSP3.BaseCodes = 4;
		}
	}

	while (DSP3.BaseCodes)
	{
		if (!DSP3_GetBits(3))
			return;

		DSP3.ReqBits++;

		DSP3.CodeLengths[DSP3.Index] = (uint8) DSP3.ReqBits;
		DSP3.CodeOffsets[DSP3.Index] = DSP3.Symbol;
		DSP3.Index++;

		DSP3.Symbol += 1 << DSP3.ReqBits;
		DSP3.BaseCodes--;
	}

	DSP3.BaseCode = 0xffff;
	DSP3.LZCode = 0;

	SetDSP3 = &DSP3_Decode_Data;
	if (DSP3.BitCount)
		DSP3_Decode_Data();
}

static void DSP3_Decode_Symbols (void)
{
	DSP3.ReqData = DSP3.DR;
	DSP3.BitCount += 16;

	do
	{
		if (DSP3.BitCommand == 0xffff)
		{
			if (!DSP3_GetBits(2))
				return;

			DSP3.BitCommand = DSP3.ReqBits;
		}

		switch (DSP3.BitCommand)
		{
			case 0:
				if (!DSP3_GetBits(9))
					return;
				DSP3.Symbol = DSP3.ReqBits;
				break;

			case 1:
				DSP3.Symbol++;
				break;

			case 2:
				if (!DSP3_GetBits(1))
					return;
				DSP3.Symbol += 2 + DSP3.ReqBits;
				break;

			case 3:
				if (!DSP3_GetBits(4))
					return;
				DSP3.Symbol += 4 + DSP3.ReqBits;
				break;
		}

		DSP3.BitCommand = 0xffff;

		DSP3.Codes[DSP3.Index++] = DSP3.Symbol;
		DSP3.Codewords--;

	}
	while (DSP3.Codewords);

	DSP3.Index = 0;
	DSP3.Symbol = 0;
	DSP3.BaseCodes = 0;

	SetDSP3 = &DSP3_Decode_Tree;
	if (DSP3.BitCount)
		DSP3_Decode_Tree();
}

static void DSP3_Decode_A (void)
{
	DSP3.Outwords = DSP3.DR;
	SetDSP3 = &DSP3_Decode_Symbols;
	DSP3.BitCount = 0;
	DSP3.BitsLeft = 0;
	DSP3.Symbol = 0;
	DSP3.Index = 0;
	DSP3.BitCommand = 0xffff;
	DSP3.SR = 0xC0;
}

static void DSP3_Decode (void)
{
	DSP3.Codewords = DSP3.DR;
	SetDSP3 = &DSP3_Decode_A;
}

// Opcodes 1E/3E bit-perfect to 'dsp3-intro' log
// src: adapted from SD Gundam X/G-Next

static void DSP3_OP3E (void)
{
	DSP3. op3e_x = (uint8)  (DSP3.DR & 0x00ff);
	DSP3. op3e_y = (uint8) ((DSP3.DR & 0xff00) >> 8);

	DSP3_OP03();

	DSP3.op1e_terrain[DSP3.DR] = 0x00;
	DSP3.op1e_cost[DSP3.DR]    = 0xff;
	DSP3.op1e_weight[DSP3.DR]  = 0;

	DSP3.op1e_max_search_radius = 0;
	DSP3.op1e_max_path_radius   = 0;
}

static void DSP3_OP1E (void)
{
	DSP3.op1e_min_radius = (uint8)  (DSP3.DR & 0x00ff);
	DSP3.op1e_max_radius = (uint8) ((DSP3.DR & 0xff00) >> 8);

	if (DSP3.op1e_min_radius == 0)
		DSP3.op1e_min_radius++;

	if (DSP3.op1e_max_search_radius >= DSP3.op1e_min_radius)
		DSP3.op1e_min_radius = DSP3.op1e_max_search_radius + 1;

	if (DSP3.op1e_max_radius > DSP3.op1e_max_search_radius)
		DSP3.op1e_max_search_radius = DSP3.op1e_max_radius;

	DSP3.op1e_lcv_radius = DSP3.op1e_min_radius;
	DSP3.op1e_lcv_steps = DSP3.op1e_min_radius;

	DSP3.op1e_lcv_turns = 6;
	DSP3.op1e_turn = 0;

	DSP3.op1e_x = DSP3. op3e_x;
	DSP3.op1e_y = DSP3. op3e_y;

	for (int lcv = 0; lcv < DSP3.op1e_min_radius; lcv++)
		DSP3_OP1E_D(DSP3.op1e_turn, &DSP3.op1e_x, &DSP3.op1e_y);

	DSP3_OP1E_A();
}

static void DSP3_OP1E_A (void)
{
	if (DSP3.op1e_lcv_steps == 0)
	{
		DSP3.op1e_lcv_radius++;

		DSP3.op1e_lcv_steps = DSP3.op1e_lcv_radius;

		DSP3.op1e_x = DSP3. op3e_x;
		DSP3.op1e_y = DSP3. op3e_y;

		for (int lcv = 0; lcv < DSP3.op1e_lcv_radius; lcv++)
			DSP3_OP1E_D(DSP3.op1e_turn, &DSP3.op1e_x, &DSP3.op1e_y);
	}

	if (DSP3.op1e_lcv_radius > DSP3.op1e_max_radius)
	{
		DSP3.op1e_turn++;
		DSP3.op1e_lcv_turns--;

		DSP3.op1e_lcv_radius = DSP3.op1e_min_radius;
		DSP3.op1e_lcv_steps = DSP3.op1e_min_radius;

		DSP3.op1e_x = DSP3. op3e_x;
		DSP3.op1e_y = DSP3. op3e_y;

		for (int lcv = 0; lcv < DSP3.op1e_min_radius; lcv++)
			DSP3_OP1E_D(DSP3.op1e_turn, &DSP3.op1e_x, &DSP3.op1e_y);
	}

	if (DSP3.op1e_lcv_turns == 0)
	{
		DSP3.DR = 0xffff;
		DSP3.SR = 0x0080;
		SetDSP3 = &DSP3_OP1E_B;
		return;
	}

	DSP3.DR = (uint8) (DSP3.op1e_x) | ((uint8) (DSP3.op1e_y) << 8);
	DSP3_OP03();

	DSP3.op1e_cell = DSP3.DR;

	DSP3.SR = 0x0080;
	SetDSP3 = &DSP3_OP1E_A1;
}

static void DSP3_OP1E_A1 (void)
{
	DSP3.SR = 0x0084;
	SetDSP3 = &DSP3_OP1E_A2;
}

static void DSP3_OP1E_A2 (void)
{
	DSP3.op1e_terrain[DSP3.op1e_cell] = (uint8) (DSP3.DR & 0x00ff);

	DSP3.SR = 0x0084;
	SetDSP3 = &DSP3_OP1E_A3;
}

static void DSP3_OP1E_A3 (void)
{
	DSP3.op1e_cost[DSP3.op1e_cell] = (uint8) (DSP3.DR & 0x00ff);

	if (DSP3.op1e_lcv_radius == 1)
	{
		if (DSP3.op1e_terrain[DSP3.op1e_cell] & 1)
			DSP3.op1e_weight[DSP3.op1e_cell] = 0xff;
		else
			DSP3.op1e_weight[DSP3.op1e_cell] = DSP3.op1e_cost[DSP3.op1e_cell];
	}
	else
		DSP3.op1e_weight[DSP3.op1e_cell] = 0xff;

	DSP3_OP1E_D((int16) (DSP3.op1e_turn + 2), &DSP3.op1e_x, &DSP3.op1e_y);
	DSP3.op1e_lcv_steps--;

	DSP3.SR = 0x0080;
	DSP3_OP1E_A();
}

static void DSP3_OP1E_B (void)
{
	DSP3.op1e_x = DSP3. op3e_x;
	DSP3.op1e_y = DSP3. op3e_y;
	DSP3.op1e_lcv_radius = 1;

	DSP3.op1e_search = 0;

	DSP3_OP1E_B1();

	SetDSP3 = &DSP3_OP1E_C;
}

static void DSP3_OP1E_B1 (void)
{
	while (DSP3.op1e_lcv_radius < DSP3.op1e_max_radius)
	{
		DSP3.op1e_y--;

		DSP3.op1e_lcv_turns = 6;
		DSP3.op1e_turn = 5;

		while (DSP3.op1e_lcv_turns)
		{
			DSP3.op1e_lcv_steps = DSP3.op1e_lcv_radius;

			while (DSP3.op1e_lcv_steps)
			{
				DSP3_OP1E_D1(DSP3.op1e_turn, &DSP3.op1e_x, &DSP3.op1e_y);

				if (0 <= DSP3.op1e_y && DSP3.op1e_y < DSP3.WinHi && 0 <= DSP3.op1e_x && DSP3.op1e_x < DSP3.WinLo)
				{
					DSP3.DR = (uint8) (DSP3.op1e_x) | ((uint8) (DSP3.op1e_y) << 8);
					DSP3_OP03();

					DSP3.op1e_cell = DSP3.DR;
					if (DSP3.op1e_cost[DSP3.op1e_cell] < 0x80 && DSP3.op1e_terrain[DSP3.op1e_cell] < 0x40)
						DSP3_OP1E_B2(); // end cell perimeter
				}

				DSP3.op1e_lcv_steps--;
			} // end search line

			DSP3.op1e_turn--;
			if (DSP3.op1e_turn == 0)
				DSP3.op1e_turn = 6;

			DSP3.op1e_lcv_turns--;
		} // end circle search

		DSP3.op1e_lcv_radius++;
	} // end radius search
}

static void DSP3_OP1E_B2 (void)
{
	int16	cell;
	int16	path;
	int16	x, y;
	int16	lcv_turns;

	path = 0xff;
	lcv_turns = 6;

	while (lcv_turns)
	{
		x = DSP3.op1e_x;
		y = DSP3.op1e_y;

		DSP3_OP1E_D1(lcv_turns, &x, &y);

		DSP3.DR = (uint8) (x) | ((uint8) (y) << 8);
		DSP3_OP03();

		cell = DSP3.DR;

		if (0 <= y && y < DSP3.WinHi && 0 <= x && x < DSP3.WinLo)
		{
			if (DSP3.op1e_terrain[cell] < 0x80 || DSP3.op1e_weight[cell] == 0)
			{
				if (DSP3.op1e_weight[cell] < path)
					path = DSP3.op1e_weight[cell];
			}
		} // end step travel

		lcv_turns--;
	} // end while turns

	if (path != 0xff)
		DSP3.op1e_weight[DSP3.op1e_cell] = path + DSP3.op1e_cost[DSP3.op1e_cell];
}

static void DSP3_OP1E_C (void)
{
	DSP3.op1e_min_radius = (uint8)  (DSP3.DR & 0x00ff);
	DSP3.op1e_max_radius = (uint8) ((DSP3.DR & 0xff00) >> 8);

	if (DSP3.op1e_min_radius == 0)
		DSP3.op1e_min_radius++;

	if (DSP3.op1e_max_path_radius >= DSP3.op1e_min_radius)
		DSP3.op1e_min_radius = DSP3.op1e_max_path_radius + 1;

	if (DSP3.op1e_max_radius > DSP3.op1e_max_path_radius)
		DSP3.op1e_max_path_radius = DSP3.op1e_max_radius;

	DSP3.op1e_lcv_radius = DSP3.op1e_min_radius;
	DSP3.op1e_lcv_steps = DSP3.op1e_min_radius;

	DSP3.op1e_lcv_turns = 6;
	DSP3.op1e_turn = 0;

	DSP3.op1e_x = DSP3. op3e_x;
	DSP3.op1e_y = DSP3. op3e_y;

	for (int lcv = 0; lcv < DSP3.op1e_min_radius; lcv++)
		DSP3_OP1E_D(DSP3.op1e_turn, &DSP3.op1e_x, &DSP3.op1e_y);

	DSP3_OP1E_C1();
}

static void DSP3_OP1E_C1 (void)
{
	if (DSP3.op1e_lcv_steps == 0)
	{
		DSP3.op1e_lcv_radius++;

		DSP3.op1e_lcv_steps = DSP3.op1e_lcv_radius;

		DSP3.op1e_x = DSP3. op3e_x;
		DSP3.op1e_y = DSP3. op3e_y;

		for (int lcv = 0; lcv < DSP3.op1e_lcv_radius; lcv++)
			DSP3_OP1E_D(DSP3.op1e_turn, &DSP3.op1e_x, &DSP3.op1e_y);
	}

	if (DSP3.op1e_lcv_radius > DSP3.op1e_max_radius)
	{
		DSP3.op1e_turn++;
		DSP3.op1e_lcv_turns--;

		DSP3.op1e_lcv_radius = DSP3.op1e_min_radius;
		DSP3.op1e_lcv_steps = DSP3.op1e_min_radius;

		DSP3.op1e_x = DSP3. op3e_x;
		DSP3.op1e_y = DSP3. op3e_y;

		for (int lcv = 0; lcv < DSP3.op1e_min_radius; lcv++)
			DSP3_OP1E_D(DSP3.op1e_turn, &DSP3.op1e_x, &DSP3.op1e_y);
	}

	if (DSP3.op1e_lcv_turns == 0)
	{
		DSP3.DR = 0xffff;
		DSP3.SR = 0x0080;
		SetDSP3 = &DSP3_Reset;
		return;
	}

	DSP3.DR = (uint8) (DSP3.op1e_x) | ((uint8) (DSP3.op1e_y) << 8);
	DSP3_OP03();

	DSP3.op1e_cell = DSP3.DR;

	DSP3.SR = 0x0080;
	SetDSP3 = &DSP3_OP1E_C2;
}

static void DSP3_OP1E_C2 (void)
{
	DSP3.DR = DSP3.op1e_weight[DSP3.op1e_cell];

	DSP3_OP1E_D((int16) (DSP3.op1e_turn + 2), &DSP3.op1e_x, &DSP3.op1e_y);
	DSP3.op1e_lcv_steps--;

	DSP3.SR = 0x0084;
	SetDSP3 = &DSP3_OP1E_C1;
}

static void DSP3_OP1E_D (int16 move, int16 *lo, int16 *hi)
{
	uint32	dataOfs = ((move << 1) + 0x03b2) & 0x03ff;
	int16	Lo;
	int16	Hi;

	DSP3.AddHi = DSP3_DataROM[dataOfs];
	DSP3.AddLo = DSP3_DataROM[dataOfs + 1];

	Lo = (uint8) (*lo);
	Hi = (uint8) (*hi);

	if (Lo & 1)
		Hi += (DSP3.AddLo & 1);

	DSP3.AddLo += Lo;
	DSP3.AddHi += Hi;

	if (DSP3.AddLo < 0)
		DSP3.AddLo += DSP3.WinLo;
	else
	if (DSP3.AddLo >= DSP3.WinLo)
		DSP3.AddLo -= DSP3.WinLo;

	if (DSP3.AddHi < 0)
		DSP3.AddHi += DSP3.WinHi;
	else
	if (DSP3.AddHi >= DSP3.WinHi)
		DSP3.AddHi -= DSP3.WinHi;

	*lo = DSP3.AddLo;
	*hi = DSP3.AddHi;
}

static void DSP3_OP1E_D1 (int16 move, int16 *lo, int16 *hi)
{
	const uint16	HiAdd[] =
	{
		0x00, 0xFF, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x01, 0x00, 0xFF, 0x00
	};

	const uint16	LoAdd[] =
	{
		0x00, 0x00, 0x01, 0x01, 0x00, 0xFF, 0xFF, 0x00
	};

	int16	Lo;
	int16	Hi;

	if ((*lo) & 1)
		DSP3.AddHi = HiAdd[move + 8];
	else
		DSP3.AddHi = HiAdd[move + 0];

	DSP3.AddLo = LoAdd[move];

	Lo = (uint8) (*lo);
	Hi = (uint8) (*hi);

	if (Lo & 1)
		Hi += (DSP3.AddLo & 1);

	DSP3.AddLo += Lo;
	DSP3.AddHi += Hi;

	*lo = DSP3.AddLo;
	*hi = DSP3.AddHi;
}

static void DSP3_OP10 (void)
{
	if (DSP3.DR == 0xffff)
		DSP3_Reset();
	else
		// absorb 2 bytes
		DSP3.DR = DSP3.DR;
}

/*
static void DSP3_OP0C_A (void)
{
	// absorb 2 bytes
	DSP3.DR = 0;
	SetDSP3 = &DSP3_Reset;
}
*/

static void DSP3_OP0C (void)
{
	// absorb 2 bytes
	DSP3.DR = 0;
	//SetDSP3 = &DSP3_OP0C_A;
	SetDSP3 = &DSP3_Reset;
}

static void DSP3_OP1C_C (void)
{
	// return 2 bytes
	DSP3.DR = 0;
	SetDSP3 = &DSP3_Reset;
}

static void DSP3_OP1C_B (void)
{
	// return 2 bytes
	DSP3.DR = 0;
	SetDSP3 = &DSP3_OP1C_C;
}

static void DSP3_OP1C_A (void)
{
	// absorb 2 bytes
	SetDSP3 = &DSP3_OP1C_B;
}

static void DSP3_OP1C (void)
{
	// absorb 2 bytes
	SetDSP3 = &DSP3_OP1C_A;
}

static void DSP3_Command (void)
{
	if (DSP3.DR < 0x40)
	{
		switch (DSP3.DR)
		{
			case 0x02: SetDSP3 = &DSP3_Coordinate; break;
			case 0x03: SetDSP3 = &DSP3_OP03;       break;
			case 0x06: SetDSP3 = &DSP3_OP06;       break;
			case 0x07: SetDSP3 = &DSP3_OP07;       return;
			case 0x0c: SetDSP3 = &DSP3_OP0C;       break;
			case 0x0f: SetDSP3 = &DSP3_TestMemory; break;
			case 0x10: SetDSP3 = &DSP3_OP10;       break;
			case 0x18: SetDSP3 = &DSP3_Convert;    break;
			case 0x1c: SetDSP3 = &DSP3_OP1C;       break;
			case 0x1e: SetDSP3 = &DSP3_OP1E;       break;
			case 0x1f: SetDSP3 = &DSP3_MemoryDump; break;
			case 0x38: SetDSP3 = &DSP3_Decode;     break;
			case 0x3e: SetDSP3 = &DSP3_OP3E;       break;
			default:
				return;
		}

		DSP3.SR = 0x0080;
		DSP3.Index = 0;
	}
}

void DSP3SetByte (uint8 byte, uint16 address)
{
	if (address < DSP0.boundary)
	{
		if (DSP3.SR & 0x04)
		{
			DSP3.DR = (DSP3.DR & 0xff00) + byte;
			(*SetDSP3)();
		}
		else
		{
			DSP3.SR ^= 0x10;

			if (DSP3.SR & 0x10)
				DSP3.DR = (DSP3.DR & 0xff00) + byte;
			else
			{
				DSP3.DR = (DSP3.DR & 0x00ff) + (byte << 8);
				(*SetDSP3)();
			}
		}
	}
}

uint8 DSP3GetByte (uint16 address)
{
	if (address < DSP0.boundary)
	{
		uint8	byte;

		if (DSP3.SR & 0x04)
		{
			byte = (uint8) DSP3.DR;
			(*SetDSP3)();
		}
		else
		{
			DSP3.SR ^= 0x10;

			if (DSP3.SR & 0x10)
				byte = (uint8) (DSP3.DR);
			else
			{
				byte = (uint8) (DSP3.DR >> 8);
				(*SetDSP3)();
			}
		}

		return (byte);
	}

	return (uint8) DSP3.SR;
}
