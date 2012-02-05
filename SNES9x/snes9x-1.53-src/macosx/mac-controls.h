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

/***********************************************************************************
  SNES9X for Mac OS (c) Copyright John Stiles

  Snes9x for Mac OS X

  (c) Copyright 2001 - 2011  zones
  (c) Copyright 2002 - 2005  107
  (c) Copyright 2002         PB1400c
  (c) Copyright 2004         Alexander and Sander
  (c) Copyright 2004 - 2005  Steven Seeger
  (c) Copyright 2005         Ryan Vogt
 ***********************************************************************************/


#ifndef _mac_controls_h_
#define _mac_controls_h_

enum
{
	k_HD = 0x80000000,

	k_JP = 0x01000000,
	k_MO = 0x02000000,
	k_SS = 0x04000000,
	k_LG = 0x08000000,

	k_BT = 0x00100000,
	k_PT = 0x00200000,
	k_PS = 0x00400000,

	k_C1 = 0x00000100,
	k_C2 = 0x00000200,
	k_C3 = 0x00000400,
	k_C4 = 0x00000800,
	k_C5 = 0x00001000,
	k_C6 = 0x00002000,
	k_C7 = 0x00004000,
	k_C8 = 0x00008000
};

enum
{
	kMacCMapPad1PX            = k_HD | k_BT | k_JP | k_C1,
	kMacCMapPad1PA,
	kMacCMapPad1PB,
	kMacCMapPad1PY,
	kMacCMapPad1PL,
	kMacCMapPad1PR,
	kMacCMapPad1PSelect,
	kMacCMapPad1PStart,
	kMacCMapPad1PUp,
	kMacCMapPad1PDown,
	kMacCMapPad1PLeft,
	kMacCMapPad1PRight,

	kMacCMapPad2PX            = k_HD | k_BT | k_JP | k_C2,
	kMacCMapPad2PA,
	kMacCMapPad2PB,
	kMacCMapPad2PY,
	kMacCMapPad2PL,
	kMacCMapPad2PR,
	kMacCMapPad2PSelect,
	kMacCMapPad2PStart,
	kMacCMapPad2PUp,
	kMacCMapPad2PDown,
	kMacCMapPad2PLeft,
	kMacCMapPad2PRight,

	kMacCMapPad3PX            = k_HD | k_BT | k_JP | k_C3,
	kMacCMapPad3PA,
	kMacCMapPad3PB,
	kMacCMapPad3PY,
	kMacCMapPad3PL,
	kMacCMapPad3PR,
	kMacCMapPad3PSelect,
	kMacCMapPad3PStart,
	kMacCMapPad3PUp,
	kMacCMapPad3PDown,
	kMacCMapPad3PLeft,
	kMacCMapPad3PRight,

	kMacCMapPad4PX            = k_HD | k_BT | k_JP | k_C4,
	kMacCMapPad4PA,
	kMacCMapPad4PB,
	kMacCMapPad4PY,
	kMacCMapPad4PL,
	kMacCMapPad4PR,
	kMacCMapPad4PSelect,
	kMacCMapPad4PStart,
	kMacCMapPad4PUp,
	kMacCMapPad4PDown,
	kMacCMapPad4PLeft,
	kMacCMapPad4PRight,

	kMacCMapPad5PX            = k_HD | k_BT | k_JP | k_C5,
	kMacCMapPad5PA,
	kMacCMapPad5PB,
	kMacCMapPad5PY,
	kMacCMapPad5PL,
	kMacCMapPad5PR,
	kMacCMapPad5PSelect,
	kMacCMapPad5PStart,
	kMacCMapPad5PUp,
	kMacCMapPad5PDown,
	kMacCMapPad5PLeft,
	kMacCMapPad5PRight,

	kMacCMapPad6PX            = k_HD | k_BT | k_JP | k_C6,
	kMacCMapPad6PA,
	kMacCMapPad6PB,
	kMacCMapPad6PY,
	kMacCMapPad6PL,
	kMacCMapPad6PR,
	kMacCMapPad6PSelect,
	kMacCMapPad6PStart,
	kMacCMapPad6PUp,
	kMacCMapPad6PDown,
	kMacCMapPad6PLeft,
	kMacCMapPad6PRight,

	kMacCMapPad7PX            = k_HD | k_BT | k_JP | k_C7,
	kMacCMapPad7PA,
	kMacCMapPad7PB,
	kMacCMapPad7PY,
	kMacCMapPad7PL,
	kMacCMapPad7PR,
	kMacCMapPad7PSelect,
	kMacCMapPad7PStart,
	kMacCMapPad7PUp,
	kMacCMapPad7PDown,
	kMacCMapPad7PLeft,
	kMacCMapPad7PRight,

	kMacCMapPad8PX            = k_HD | k_BT | k_JP | k_C8,
	kMacCMapPad8PA,
	kMacCMapPad8PB,
	kMacCMapPad8PY,
	kMacCMapPad8PL,
	kMacCMapPad8PR,
	kMacCMapPad8PSelect,
	kMacCMapPad8PStart,
	kMacCMapPad8PUp,
	kMacCMapPad8PDown,
	kMacCMapPad8PLeft,
	kMacCMapPad8PRight,

	kMacCMapMouse1PL          = k_HD | k_BT | k_MO | k_C1,
	kMacCMapMouse1PR,
	kMacCMapMouse2PL          = k_HD | k_BT | k_MO | k_C2,
	kMacCMapMouse2PR,

	kMacCMapScopeOffscreen    = k_HD | k_BT | k_SS | k_C1,
	kMacCMapScopeFire,
	kMacCMapScopeCursor,
	kMacCMapScopeTurbo,
	kMacCMapScopePause,

	kMacCMapLGun1Offscreen    = k_HD | k_BT | k_LG | k_C1,
	kMacCMapLGun1Trigger,
	kMacCMapLGun1Start,
	kMacCMapLGun2Offscreen    = k_HD | k_BT | k_LG | k_C2,
	kMacCMapLGun2Trigger,
	kMacCMapLGun2Start,

	kMacCMapMouse1Pointer     = k_HD | k_PT | k_MO | k_C1,
	kMacCMapMouse2Pointer     = k_HD | k_PT | k_MO | k_C2,
	kMacCMapSuperscopePointer = k_HD | k_PT | k_SS | k_C1,
	kMacCMapJustifier1Pointer = k_HD | k_PT | k_LG | k_C1,

	kMacCMapPseudoPtrBase     = k_HD | k_PS | k_LG | k_C2	// for Justifier 2P
};

void ControlPadFlagsToS9xReportButtons (int, uint32);
void ControlPadFlagsToS9xPseudoPointer (uint32);

#endif
