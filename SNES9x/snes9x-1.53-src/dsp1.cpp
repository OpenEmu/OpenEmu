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

/*
Copyright (C) 1997-2006 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include "snes9x.h"
#include "memmap.h"

#ifdef DEBUGGER
//#define DebugDSP1
#endif

#ifdef DebugDSP1
#include <stdarg.h>
static FILE	*LogFile = NULL;
#endif

static const uint16	DSP1ROM[1024] =
{
	 0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,
	 0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,
	 0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,
	 0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,
	 0x0000,  0x0000,  0x0001,  0x0002,  0x0004,  0x0008,  0x0010,  0x0020,
	 0x0040,  0x0080,  0x0100,  0x0200,  0x0400,  0x0800,  0x1000,  0x2000,
	 0x4000,  0x7fff,  0x4000,  0x2000,  0x1000,  0x0800,  0x0400,  0x0200,
	 0x0100,  0x0080,  0x0040,  0x0020,  0x0001,  0x0008,  0x0004,  0x0002,
	 0x0001,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,
	 0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,
	 0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,
	 0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,
	 0x0000,  0x0000,  0x8000,  0xffe5,  0x0100,  0x7fff,  0x7f02,  0x7e08,
	 0x7d12,  0x7c1f,  0x7b30,  0x7a45,  0x795d,  0x7878,  0x7797,  0x76ba,
	 0x75df,  0x7507,  0x7433,  0x7361,  0x7293,  0x71c7,  0x70fe,  0x7038,
	 0x6f75,  0x6eb4,  0x6df6,  0x6d3a,  0x6c81,  0x6bca,  0x6b16,  0x6a64,
	 0x69b4,  0x6907,  0x685b,  0x67b2,  0x670b,  0x6666,  0x65c4,  0x6523,
	 0x6484,  0x63e7,  0x634c,  0x62b3,  0x621c,  0x6186,  0x60f2,  0x6060,
	 0x5fd0,  0x5f41,  0x5eb5,  0x5e29,  0x5d9f,  0x5d17,  0x5c91,  0x5c0c,
	 0x5b88,  0x5b06,  0x5a85,  0x5a06,  0x5988,  0x590b,  0x5890,  0x5816,
	 0x579d,  0x5726,  0x56b0,  0x563b,  0x55c8,  0x5555,  0x54e4,  0x5474,
	 0x5405,  0x5398,  0x532b,  0x52bf,  0x5255,  0x51ec,  0x5183,  0x511c,
	 0x50b6,  0x5050,  0x4fec,  0x4f89,  0x4f26,  0x4ec5,  0x4e64,  0x4e05,
	 0x4da6,  0x4d48,  0x4cec,  0x4c90,  0x4c34,  0x4bda,  0x4b81,  0x4b28,
	 0x4ad0,  0x4a79,  0x4a23,  0x49cd,  0x4979,  0x4925,  0x48d1,  0x487f,
	 0x482d,  0x47dc,  0x478c,  0x473c,  0x46ed,  0x469f,  0x4651,  0x4604,
	 0x45b8,  0x456c,  0x4521,  0x44d7,  0x448d,  0x4444,  0x43fc,  0x43b4,
	 0x436d,  0x4326,  0x42e0,  0x429a,  0x4255,  0x4211,  0x41cd,  0x4189,
	 0x4146,  0x4104,  0x40c2,  0x4081,  0x4040,  0x3fff,  0x41f7,  0x43e1,
	 0x45bd,  0x478d,  0x4951,  0x4b0b,  0x4cbb,  0x4e61,  0x4fff,  0x5194,
	 0x5322,  0x54a9,  0x5628,  0x57a2,  0x5914,  0x5a81,  0x5be9,  0x5d4a,
	 0x5ea7,  0x5fff,  0x6152,  0x62a0,  0x63ea,  0x6530,  0x6672,  0x67b0,
	 0x68ea,  0x6a20,  0x6b53,  0x6c83,  0x6daf,  0x6ed9,  0x6fff,  0x7122,
	 0x7242,  0x735f,  0x747a,  0x7592,  0x76a7,  0x77ba,  0x78cb,  0x79d9,
	 0x7ae5,  0x7bee,  0x7cf5,  0x7dfa,  0x7efe,  0x7fff,  0x0000,  0x0324,
	 0x0647,  0x096a,  0x0c8b,  0x0fab,  0x12c8,  0x15e2,  0x18f8,  0x1c0b,
	 0x1f19,  0x2223,  0x2528,  0x2826,  0x2b1f,  0x2e11,  0x30fb,  0x33de,
	 0x36ba,  0x398c,  0x3c56,  0x3f17,  0x41ce,  0x447a,  0x471c,  0x49b4,
	 0x4c3f,  0x4ebf,  0x5133,  0x539b,  0x55f5,  0x5842,  0x5a82,  0x5cb4,
	 0x5ed7,  0x60ec,  0x62f2,  0x64e8,  0x66cf,  0x68a6,  0x6a6d,  0x6c24,
	 0x6dca,  0x6f5f,  0x70e2,  0x7255,  0x73b5,  0x7504,  0x7641,  0x776c,
	 0x7884,  0x798a,  0x7a7d,  0x7b5d,  0x7c29,  0x7ce3,  0x7d8a,  0x7e1d,
	 0x7e9d,  0x7f09,  0x7f62,  0x7fa7,  0x7fd8,  0x7ff6,  0x7fff,  0x7ff6,
	 0x7fd8,  0x7fa7,  0x7f62,  0x7f09,  0x7e9d,  0x7e1d,  0x7d8a,  0x7ce3,
	 0x7c29,  0x7b5d,  0x7a7d,  0x798a,  0x7884,  0x776c,  0x7641,  0x7504,
	 0x73b5,  0x7255,  0x70e2,  0x6f5f,  0x6dca,  0x6c24,  0x6a6d,  0x68a6,
	 0x66cf,  0x64e8,  0x62f2,  0x60ec,  0x5ed7,  0x5cb4,  0x5a82,  0x5842,
	 0x55f5,  0x539b,  0x5133,  0x4ebf,  0x4c3f,  0x49b4,  0x471c,  0x447a,
	 0x41ce,  0x3f17,  0x3c56,  0x398c,  0x36ba,  0x33de,  0x30fb,  0x2e11,
	 0x2b1f,  0x2826,  0x2528,  0x2223,  0x1f19,  0x1c0b,  0x18f8,  0x15e2,
	 0x12c8,  0x0fab,  0x0c8b,  0x096a,  0x0647,  0x0324,  0x7fff,  0x7ff6,
	 0x7fd8,  0x7fa7,  0x7f62,  0x7f09,  0x7e9d,  0x7e1d,  0x7d8a,  0x7ce3,
	 0x7c29,  0x7b5d,  0x7a7d,  0x798a,  0x7884,  0x776c,  0x7641,  0x7504,
	 0x73b5,  0x7255,  0x70e2,  0x6f5f,  0x6dca,  0x6c24,  0x6a6d,  0x68a6,
	 0x66cf,  0x64e8,  0x62f2,  0x60ec,  0x5ed7,  0x5cb4,  0x5a82,  0x5842,
	 0x55f5,  0x539b,  0x5133,  0x4ebf,  0x4c3f,  0x49b4,  0x471c,  0x447a,
	 0x41ce,  0x3f17,  0x3c56,  0x398c,  0x36ba,  0x33de,  0x30fb,  0x2e11,
	 0x2b1f,  0x2826,  0x2528,  0x2223,  0x1f19,  0x1c0b,  0x18f8,  0x15e2,
	 0x12c8,  0x0fab,  0x0c8b,  0x096a,  0x0647,  0x0324,  0x0000,  0xfcdc,
	 0xf9b9,  0xf696,  0xf375,  0xf055,  0xed38,  0xea1e,  0xe708,  0xe3f5,
	 0xe0e7,  0xdddd,  0xdad8,  0xd7da,  0xd4e1,  0xd1ef,  0xcf05,  0xcc22,
	 0xc946,  0xc674,  0xc3aa,  0xc0e9,  0xbe32,  0xbb86,  0xb8e4,  0xb64c,
	 0xb3c1,  0xb141,  0xaecd,  0xac65,  0xaa0b,  0xa7be,  0xa57e,  0xa34c,
	 0xa129,  0x9f14,  0x9d0e,  0x9b18,  0x9931,  0x975a,  0x9593,  0x93dc,
	 0x9236,  0x90a1,  0x8f1e,  0x8dab,  0x8c4b,  0x8afc,  0x89bf,  0x8894,
	 0x877c,  0x8676,  0x8583,  0x84a3,  0x83d7,  0x831d,  0x8276,  0x81e3,
	 0x8163,  0x80f7,  0x809e,  0x8059,  0x8028,  0x800a,  0x6488,  0x0080,
	 0x03ff,  0x0116,  0x0002,  0x0080,  0x4000,  0x3fd7,  0x3faf,  0x3f86,
	 0x3f5d,  0x3f34,  0x3f0c,  0x3ee3,  0x3eba,  0x3e91,  0x3e68,  0x3e40,
	 0x3e17,  0x3dee,  0x3dc5,  0x3d9c,  0x3d74,  0x3d4b,  0x3d22,  0x3cf9,
	 0x3cd0,  0x3ca7,  0x3c7f,  0x3c56,  0x3c2d,  0x3c04,  0x3bdb,  0x3bb2,
	 0x3b89,  0x3b60,  0x3b37,  0x3b0e,  0x3ae5,  0x3abc,  0x3a93,  0x3a69,
	 0x3a40,  0x3a17,  0x39ee,  0x39c5,  0x399c,  0x3972,  0x3949,  0x3920,
	 0x38f6,  0x38cd,  0x38a4,  0x387a,  0x3851,  0x3827,  0x37fe,  0x37d4,
	 0x37aa,  0x3781,  0x3757,  0x372d,  0x3704,  0x36da,  0x36b0,  0x3686,
	 0x365c,  0x3632,  0x3609,  0x35df,  0x35b4,  0x358a,  0x3560,  0x3536,
	 0x350c,  0x34e1,  0x34b7,  0x348d,  0x3462,  0x3438,  0x340d,  0x33e3,
	 0x33b8,  0x338d,  0x3363,  0x3338,  0x330d,  0x32e2,  0x32b7,  0x328c,
	 0x3261,  0x3236,  0x320b,  0x31df,  0x31b4,  0x3188,  0x315d,  0x3131,
	 0x3106,  0x30da,  0x30ae,  0x3083,  0x3057,  0x302b,  0x2fff,  0x2fd2,
	 0x2fa6,  0x2f7a,  0x2f4d,  0x2f21,  0x2ef4,  0x2ec8,  0x2e9b,  0x2e6e,
	 0x2e41,  0x2e14,  0x2de7,  0x2dba,  0x2d8d,  0x2d60,  0x2d32,  0x2d05,
	 0x2cd7,  0x2ca9,  0x2c7b,  0x2c4d,  0x2c1f,  0x2bf1,  0x2bc3,  0x2b94,
	 0x2b66,  0x2b37,  0x2b09,  0x2ada,  0x2aab,  0x2a7c,  0x2a4c,  0x2a1d,
	 0x29ed,  0x29be,  0x298e,  0x295e,  0x292e,  0x28fe,  0x28ce,  0x289d,
	 0x286d,  0x283c,  0x280b,  0x27da,  0x27a9,  0x2777,  0x2746,  0x2714,
	 0x26e2,  0x26b0,  0x267e,  0x264c,  0x2619,  0x25e7,  0x25b4,  0x2581,
	 0x254d,  0x251a,  0x24e6,  0x24b2,  0x247e,  0x244a,  0x2415,  0x23e1,
	 0x23ac,  0x2376,  0x2341,  0x230b,  0x22d6,  0x229f,  0x2269,  0x2232,
	 0x21fc,  0x21c4,  0x218d,  0x2155,  0x211d,  0x20e5,  0x20ad,  0x2074,
	 0x203b,  0x2001,  0x1fc7,  0x1f8d,  0x1f53,  0x1f18,  0x1edd,  0x1ea1,
	 0x1e66,  0x1e29,  0x1ded,  0x1db0,  0x1d72,  0x1d35,  0x1cf6,  0x1cb8,
	 0x1c79,  0x1c39,  0x1bf9,  0x1bb8,  0x1b77,  0x1b36,  0x1af4,  0x1ab1,
	 0x1a6e,  0x1a2a,  0x19e6,  0x19a1,  0x195c,  0x1915,  0x18ce,  0x1887,
	 0x183f,  0x17f5,  0x17ac,  0x1761,  0x1715,  0x16c9,  0x167c,  0x162e,
	 0x15df,  0x158e,  0x153d,  0x14eb,  0x1497,  0x1442,  0x13ec,  0x1395,
	 0x133c,  0x12e2,  0x1286,  0x1228,  0x11c9,  0x1167,  0x1104,  0x109e,
	 0x1036,  0x0fcc,  0x0f5f,  0x0eef,  0x0e7b,  0x0e04,  0x0d89,  0x0d0a,
	 0x0c86,  0x0bfd,  0x0b6d,  0x0ad6,  0x0a36,  0x098d,  0x08d7,  0x0811,
	 0x0736,  0x063e,  0x0519,  0x039a,  0x0000,  0x7fff,  0x0100,  0x0080,
	 0x021d,  0x00c8,  0x00ce,  0x0048,  0x0a26,  0x277a,  0x00ce,  0x6488,
	 0x14ac,  0x0001,  0x00f9,  0x00fc,  0x00ff,  0x00fc,  0x00f9,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,
	 0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff,  0xffff
};

static const int16	DSP1_MulTable[256] =
{
	 0x0000,  0x0003,  0x0006,  0x0009,  0x000c,  0x000f,  0x0012,  0x0015,
	 0x0019,  0x001c,  0x001f,  0x0022,  0x0025,  0x0028,  0x002b,  0x002f,
	 0x0032,  0x0035,  0x0038,  0x003b,  0x003e,  0x0041,  0x0045,  0x0048,
	 0x004b,  0x004e,  0x0051,  0x0054,  0x0057,  0x005b,  0x005e,  0x0061,
	 0x0064,  0x0067,  0x006a,  0x006d,  0x0071,  0x0074,  0x0077,  0x007a,
	 0x007d,  0x0080,  0x0083,  0x0087,  0x008a,  0x008d,  0x0090,  0x0093,
	 0x0096,  0x0099,  0x009d,  0x00a0,  0x00a3,  0x00a6,  0x00a9,  0x00ac,
	 0x00af,  0x00b3,  0x00b6,  0x00b9,  0x00bc,  0x00bf,  0x00c2,  0x00c5,
	 0x00c9,  0x00cc,  0x00cf,  0x00d2,  0x00d5,  0x00d8,  0x00db,  0x00df,
	 0x00e2,  0x00e5,  0x00e8,  0x00eb,  0x00ee,  0x00f1,  0x00f5,  0x00f8,
	 0x00fb,  0x00fe,  0x0101,  0x0104,  0x0107,  0x010b,  0x010e,  0x0111,
	 0x0114,  0x0117,  0x011a,  0x011d,  0x0121,  0x0124,  0x0127,  0x012a,
	 0x012d,  0x0130,  0x0133,  0x0137,  0x013a,  0x013d,  0x0140,  0x0143,
	 0x0146,  0x0149,  0x014d,  0x0150,  0x0153,  0x0156,  0x0159,  0x015c,
	 0x015f,  0x0163,  0x0166,  0x0169,  0x016c,  0x016f,  0x0172,  0x0175,
	 0x0178,  0x017c,  0x017f,  0x0182,  0x0185,  0x0188,  0x018b,  0x018e,
	 0x0192,  0x0195,  0x0198,  0x019b,  0x019e,  0x01a1,  0x01a4,  0x01a8,
	 0x01ab,  0x01ae,  0x01b1,  0x01b4,  0x01b7,  0x01ba,  0x01be,  0x01c1,
	 0x01c4,  0x01c7,  0x01ca,  0x01cd,  0x01d0,  0x01d4,  0x01d7,  0x01da,
	 0x01dd,  0x01e0,  0x01e3,  0x01e6,  0x01ea,  0x01ed,  0x01f0,  0x01f3,
	 0x01f6,  0x01f9,  0x01fc,  0x0200,  0x0203,  0x0206,  0x0209,  0x020c,
	 0x020f,  0x0212,  0x0216,  0x0219,  0x021c,  0x021f,  0x0222,  0x0225,
	 0x0228,  0x022c,  0x022f,  0x0232,  0x0235,  0x0238,  0x023b,  0x023e,
	 0x0242,  0x0245,  0x0248,  0x024b,  0x024e,  0x0251,  0x0254,  0x0258,
	 0x025b,  0x025e,  0x0261,  0x0264,  0x0267,  0x026a,  0x026e,  0x0271,
	 0x0274,  0x0277,  0x027a,  0x027d,  0x0280,  0x0284,  0x0287,  0x028a,
	 0x028d,  0x0290,  0x0293,  0x0296,  0x029a,  0x029d,  0x02a0,  0x02a3,
	 0x02a6,  0x02a9,  0x02ac,  0x02b0,  0x02b3,  0x02b6,  0x02b9,  0x02bc,
	 0x02bf,  0x02c2,  0x02c6,  0x02c9,  0x02cc,  0x02cf,  0x02d2,  0x02d5,
	 0x02d8,  0x02db,  0x02df,  0x02e2,  0x02e5,  0x02e8,  0x02eb,  0x02ee,
	 0x02f1,  0x02f5,  0x02f8,  0x02fb,  0x02fe,  0x0301,  0x0304,  0x0307,
	 0x030b,  0x030e,  0x0311,  0x0314,  0x0317,  0x031a,  0x031d,  0x0321
};

static const int16	DSP1_SinTable[256] =
{
	 0x0000,  0x0324,  0x0647,  0x096a,  0x0c8b,  0x0fab,  0x12c8,  0x15e2,
	 0x18f8,  0x1c0b,  0x1f19,  0x2223,  0x2528,  0x2826,  0x2b1f,  0x2e11,
	 0x30fb,  0x33de,  0x36ba,  0x398c,  0x3c56,  0x3f17,  0x41ce,  0x447a,
	 0x471c,  0x49b4,  0x4c3f,  0x4ebf,  0x5133,  0x539b,  0x55f5,  0x5842,
	 0x5a82,  0x5cb4,  0x5ed7,  0x60ec,  0x62f2,  0x64e8,  0x66cf,  0x68a6,
	 0x6a6d,  0x6c24,  0x6dca,  0x6f5f,  0x70e2,  0x7255,  0x73b5,  0x7504,
	 0x7641,  0x776c,  0x7884,  0x798a,  0x7a7d,  0x7b5d,  0x7c29,  0x7ce3,
	 0x7d8a,  0x7e1d,  0x7e9d,  0x7f09,  0x7f62,  0x7fa7,  0x7fd8,  0x7ff6,
	 0x7fff,  0x7ff6,  0x7fd8,  0x7fa7,  0x7f62,  0x7f09,  0x7e9d,  0x7e1d,
	 0x7d8a,  0x7ce3,  0x7c29,  0x7b5d,  0x7a7d,  0x798a,  0x7884,  0x776c,
	 0x7641,  0x7504,  0x73b5,  0x7255,  0x70e2,  0x6f5f,  0x6dca,  0x6c24,
	 0x6a6d,  0x68a6,  0x66cf,  0x64e8,  0x62f2,  0x60ec,  0x5ed7,  0x5cb4,
	 0x5a82,  0x5842,  0x55f5,  0x539b,  0x5133,  0x4ebf,  0x4c3f,  0x49b4,
	 0x471c,  0x447a,  0x41ce,  0x3f17,  0x3c56,  0x398c,  0x36ba,  0x33de,
	 0x30fb,  0x2e11,  0x2b1f,  0x2826,  0x2528,  0x2223,  0x1f19,  0x1c0b,
	 0x18f8,  0x15e2,  0x12c8,  0x0fab,  0x0c8b,  0x096a,  0x0647,  0x0324,
	-0x0000, -0x0324, -0x0647, -0x096a, -0x0c8b, -0x0fab, -0x12c8, -0x15e2,
	-0x18f8, -0x1c0b, -0x1f19, -0x2223, -0x2528, -0x2826, -0x2b1f, -0x2e11,
	-0x30fb, -0x33de, -0x36ba, -0x398c, -0x3c56, -0x3f17, -0x41ce, -0x447a,
	-0x471c, -0x49b4, -0x4c3f, -0x4ebf, -0x5133, -0x539b, -0x55f5, -0x5842,
	-0x5a82, -0x5cb4, -0x5ed7, -0x60ec, -0x62f2, -0x64e8, -0x66cf, -0x68a6,
	-0x6a6d, -0x6c24, -0x6dca, -0x6f5f, -0x70e2, -0x7255, -0x73b5, -0x7504,
	-0x7641, -0x776c, -0x7884, -0x798a, -0x7a7d, -0x7b5d, -0x7c29, -0x7ce3,
	-0x7d8a, -0x7e1d, -0x7e9d, -0x7f09, -0x7f62, -0x7fa7, -0x7fd8, -0x7ff6,
	-0x7fff, -0x7ff6, -0x7fd8, -0x7fa7, -0x7f62, -0x7f09, -0x7e9d, -0x7e1d,
	-0x7d8a, -0x7ce3, -0x7c29, -0x7b5d, -0x7a7d, -0x798a, -0x7884, -0x776c,
	-0x7641, -0x7504, -0x73b5, -0x7255, -0x70e2, -0x6f5f, -0x6dca, -0x6c24,
	-0x6a6d, -0x68a6, -0x66cf, -0x64e8, -0x62f2, -0x60ec, -0x5ed7, -0x5cb4,
	-0x5a82, -0x5842, -0x55f5, -0x539b, -0x5133, -0x4ebf, -0x4c3f, -0x49b4,
	-0x471c, -0x447a, -0x41ce, -0x3f17, -0x3c56, -0x398c, -0x36ba, -0x33de,
	-0x30fb, -0x2e11, -0x2b1f, -0x2826, -0x2528, -0x2223, -0x1f19, -0x1c0b,
	-0x18f8, -0x15e2, -0x12c8, -0x0fab, -0x0c8b, -0x096a, -0x0647, -0x0324
};


#ifdef DebugDSP1

static void Log_Message (const char *Message, ...)
{
	char	Msg[400];
	va_list	ap;
	size_t	ignore;

	va_start(ap, Message);
	vsprintf(Msg, Message, ap);
	va_end(ap);

	strcat(Msg, "\r\n\0");
	ignore = fwrite(Msg, strlen(Msg), 1, LogFile);
	fflush(LogFile);
}

static void Start_Log (void)
{
	LogFile = fopen("dsp1emu.log", "wb");
}

static void Stop_Log (void)
{
	if (LogFile)
	{
		fclose(LogFile);
		LogFile = NULL;
	}
}

#endif

static void DSP1_Op00 (void)
{
	DSP1.Op00Result = DSP1.Op00Multiplicand * DSP1.Op00Multiplier >> 15;

#ifdef DebugDSP1
	Log_Message("OP00 MULT %d*%d/32768=%d", DSP1.Op00Multiplicand, DSP1.Op00Multiplier, DSP1.Op00Result);
#endif
}

static void DSP1_Op20 (void)
{
	DSP1.Op20Result = DSP1.Op20Multiplicand * DSP1.Op20Multiplier >> 15;
	DSP1.Op20Result++;

#ifdef DebugDSP1
	Log_Message("OP20 MULT %d*%d/32768=%d", DSP1.Op20Multiplicand, DSP1.Op20Multiplier, DSP1.Op20Result);
#endif
}

static void DSP1_Inverse (int16 Coefficient, int16 Exponent, int16 *iCoefficient, int16 *iExponent)
{
	// Step One: Division by Zero
	if (Coefficient == 0x0000)
	{
		*iCoefficient = 0x7fff;
		*iExponent    = 0x002f;
	}
	else
	{
		int16	Sign = 1;

		// Step Two: Remove Sign
		if (Coefficient < 0)
		{
			if (Coefficient < -32767)
				Coefficient = -32767;
			Coefficient = -Coefficient;
			Sign = -1;
		}

		// Step Three: Normalize
		while (Coefficient < 0x4000)
		{
			Coefficient <<= 1;
			Exponent--;
		}

		// Step Four: Special Case
		if (Coefficient == 0x4000)
		{
			if (Sign == 1)
				*iCoefficient =  0x7fff;
			else
			{
				*iCoefficient = -0x4000;
				Exponent--;
			}
		}
		else
		{
			// Step Five: Initial Guess
			int16	i = DSP1ROM[((Coefficient - 0x4000) >> 7) + 0x0065];

			// Step Six: Iterate "estimated" Newton's Method
			i = (i + (-i * (Coefficient * i >> 15) >> 15)) << 1;
			i = (i + (-i * (Coefficient * i >> 15) >> 15)) << 1;

			*iCoefficient = i * Sign;
		}

		*iExponent = 1 - Exponent;
	}
}

static void DSP1_Op10 (void)
{
	DSP1_Inverse(DSP1.Op10Coefficient, DSP1.Op10Exponent, &DSP1.Op10CoefficientR, &DSP1.Op10ExponentR);

#ifdef DebugDSP1
	Log_Message("OP10 INV %d*2^%d = %d*2^%d", DSP1.Op10Coefficient, DSP1.Op10Exponent, DSP1.Op10CoefficientR, DSP1.Op10ExponentR);
#endif
}

static int16 DSP1_Sin (int16 Angle)
{
	int32	S;

	if (Angle < 0)
	{
		if (Angle == -32768)
			return (0);

		return (-DSP1_Sin(-Angle));
	}

	S = DSP1_SinTable[Angle >> 8] + (DSP1_MulTable[Angle & 0xff] * DSP1_SinTable[0x40 + (Angle >> 8)] >> 15);
	if (S > 32767)
		S = 32767;

	return ((int16) S);
}

static int16 DSP1_Cos (int16 Angle)
{
	int32	S;

	if (Angle < 0)
	{
		if (Angle == -32768)
			return (-32768);

		Angle = -Angle;
	}

	S = DSP1_SinTable[0x40 + (Angle >> 8)] - (DSP1_MulTable[Angle & 0xff] * DSP1_SinTable[Angle >> 8] >> 15);
	if (S < -32768)
		S = -32767;

	return ((int16) S);
}

static void DSP1_Normalize (int16 m, int16 *Coefficient, int16 *Exponent)
{
	int16	i = 0x4000;
	int16	e = 0;

	if (m < 0)
	{
		while ((m & i) && i)
		{
			i >>= 1;
			e++;
		}
	}
	else
	{
		while (!(m & i) && i)
		{
			i >>= 1;
			e++;
		}
	}

	if (e > 0)
		*Coefficient = m * DSP1ROM[0x21 + e] << 1;
	else
		*Coefficient = m;

	*Exponent -= e;
}

static void DSP1_NormalizeDouble (int32 Product, int16 *Coefficient, int16 *Exponent)
{
	int16	n = Product & 0x7fff;
	int16	m = Product >> 15;
	int16	i = 0x4000;
	int16	e = 0;

	if (m < 0)
	{
		while ((m & i) && i)
		{
			i >>= 1;
			e++;
		}
	}
	else
	{
		while (!(m & i) && i)
		{
			i >>= 1;
			e++;
		}
	}

	if (e > 0)
	{
		*Coefficient = m * DSP1ROM[0x0021 + e] << 1;

		if (e < 15)
			*Coefficient += n * DSP1ROM[0x0040 - e] >> 15;
		else
		{
			i = 0x4000;

			if (m < 0)
			{
				while ((n & i) && i)
				{
					i >>= 1;
					e++;
				}
			}
			else
			{
				while (!(n & i) && i)
				{
					i >>= 1;
					e++;
				}
			}

			if (e > 15)
				*Coefficient = n * DSP1ROM[0x0012 + e] << 1;
			else
				*Coefficient += n;
		}
	}
	else
		*Coefficient = m;

	*Exponent = e;
}

static int16 DSP1_Truncate (int16 C, int16 E)
{
	if (E > 0)
	{
		if (C > 0)
			return (32767);
		else
		if (C < 0)
			return (-32767);
	}
	else
	{
		if (E < 0)
			return (C * DSP1ROM[0x0031 + E] >> 15);
	}

	return (C);
}

static void DSP1_Op04 (void)
{
	DSP1.Op04Sin = DSP1_Sin(DSP1.Op04Angle) * DSP1.Op04Radius >> 15;
	DSP1.Op04Cos = DSP1_Cos(DSP1.Op04Angle) * DSP1.Op04Radius >> 15;
}

static void DSP1_Op0C (void)
{
	DSP1.Op0CX2 = (DSP1.Op0CY1 * DSP1_Sin(DSP1.Op0CA) >> 15) + (DSP1.Op0CX1 * DSP1_Cos(DSP1.Op0CA) >> 15);
	DSP1.Op0CY2 = (DSP1.Op0CY1 * DSP1_Cos(DSP1.Op0CA) >> 15) - (DSP1.Op0CX1 * DSP1_Sin(DSP1.Op0CA) >> 15);
}

static void DSP1_Parameter (int16 Fx, int16 Fy, int16 Fz, int16 Lfe, int16 Les, int16 Aas, int16 Azs, int16 *Vof, int16 *Vva, int16 *Cx, int16 *Cy)
{
	const int16	MaxAZS_Exp[16] =
	{
		0x38b4, 0x38b7, 0x38ba, 0x38be, 0x38c0, 0x38c4, 0x38c7, 0x38ca,
		0x38ce,	0x38d0, 0x38d4, 0x38d7, 0x38da, 0x38dd, 0x38e0, 0x38e4
	};

	int16	CSec, C, E, MaxAZS, Aux;
	int16	LfeNx, LfeNy, LfeNz;
	int16	LesNx, LesNy, LesNz;
	int16	CentreZ;

	// Copy Zenith angle for clipping
	int16	AZS = Azs;

	// Store Sine and Cosine of Azimuth and Zenith angle
	DSP1.SinAas = DSP1_Sin(Aas);
	DSP1.CosAas = DSP1_Cos(Aas);
	DSP1.SinAzs = DSP1_Sin(Azs);
	DSP1.CosAzs = DSP1_Cos(Azs);

	DSP1.Nx = DSP1.SinAzs * -DSP1.SinAas >> 15;
	DSP1.Ny = DSP1.SinAzs *  DSP1.CosAas >> 15;
	DSP1.Nz = DSP1.CosAzs *  0x7fff >> 15;

	LfeNx = Lfe * DSP1.Nx >> 15;
	LfeNy = Lfe * DSP1.Ny >> 15;
	LfeNz = Lfe * DSP1.Nz >> 15;

	// Center of Projection
	DSP1.CentreX = Fx + LfeNx;
	DSP1.CentreY = Fy + LfeNy;
	CentreZ = Fz + LfeNz;

	LesNx = Les * DSP1.Nx >> 15;
	LesNy = Les * DSP1.Ny >> 15;
	LesNz = Les * DSP1.Nz >> 15;

	DSP1.Gx = DSP1.CentreX - LesNx;
	DSP1.Gy = DSP1.CentreY - LesNy;
	DSP1.Gz = CentreZ - LesNz;

	DSP1.E_Les = 0;
	DSP1_Normalize(Les, &DSP1.C_Les, &DSP1.E_Les);
	DSP1.G_Les = Les;

	E = 0;
	DSP1_Normalize(CentreZ, &C, &E);

	DSP1.VPlane_C = C;
	DSP1.VPlane_E = E;

	// Determine clip boundary and clip Zenith angle if necessary
	MaxAZS = MaxAZS_Exp[-E];

	if (AZS < 0)
	{
		MaxAZS = -MaxAZS;
		if (AZS < MaxAZS + 1)
			AZS = MaxAZS + 1;
	}
	else
	{
		if (AZS > MaxAZS)
			AZS = MaxAZS;
	}

	// Store Sine and Cosine of clipped Zenith angle
	DSP1.SinAZS = DSP1_Sin(AZS);
	DSP1.CosAZS = DSP1_Cos(AZS);

	DSP1_Inverse(DSP1.CosAZS, 0, &DSP1.SecAZS_C1, &DSP1.SecAZS_E1);
	DSP1_Normalize(C * DSP1.SecAZS_C1 >> 15, &C, &E);
	E += DSP1.SecAZS_E1;

	C = DSP1_Truncate(C, E) * DSP1.SinAZS >> 15;

	DSP1.CentreX += C * DSP1.SinAas >> 15;
	DSP1.CentreY -= C * DSP1.CosAas >> 15;

	*Cx = DSP1.CentreX;
	*Cy = DSP1.CentreY;

	// Raster number of imaginary center and horizontal line
	*Vof = 0;

	if ((Azs != AZS) || (Azs == MaxAZS))
	{
		if (Azs == -32768)
			Azs = -32767;

		C = Azs - MaxAZS;
		if (C >= 0)
			C--;
		Aux = ~(C << 2);

		C = Aux * DSP1ROM[0x0328] >> 15;
		C = (C * Aux >> 15) + DSP1ROM[0x0327];
		*Vof -= (C * Aux >> 15) * Les >> 15;

		C = Aux * Aux >> 15;
		Aux = (C * DSP1ROM[0x0324] >> 15) + DSP1ROM[0x0325];
		DSP1.CosAZS += (C * Aux >> 15) * DSP1.CosAZS >> 15;
	}

	DSP1.VOffset = Les * DSP1.CosAZS >> 15;

	DSP1_Inverse(DSP1.SinAZS, 0, &CSec, &E);
	DSP1_Normalize(DSP1.VOffset, &C, &E);
	DSP1_Normalize(C * CSec >> 15, &C, &E);

	if (C == -32768)
	{
		C >>= 1;
		E++;
	}

	*Vva = DSP1_Truncate(-C, E);

	// Store Secant of clipped Zenith angle
	DSP1_Inverse(DSP1.CosAZS, 0, &DSP1.SecAZS_C2, &DSP1.SecAZS_E2);
}

static void DSP1_Raster (int16 Vs, int16 *An, int16 *Bn, int16 *Cn, int16 *Dn)
{
	int16	C, E, C1, E1;

	DSP1_Inverse((Vs * DSP1.SinAzs >> 15) + DSP1.VOffset, 7, &C, &E);
	E += DSP1.VPlane_E;

	C1 = C * DSP1.VPlane_C >> 15;
	E1 = E + DSP1.SecAZS_E2;

	DSP1_Normalize(C1, &C, &E);

	C = DSP1_Truncate(C, E);

	*An = C *  DSP1.CosAas >> 15;
	*Cn = C *  DSP1.SinAas >> 15;

	DSP1_Normalize(C1 * DSP1.SecAZS_C2 >> 15, &C, &E1);

	C = DSP1_Truncate(C, E1);

	*Bn = C * -DSP1.SinAas >> 15;
	*Dn = C *  DSP1.CosAas >> 15;
}

static void DSP1_Op02 (void)
{
	DSP1_Parameter(DSP1.Op02FX, DSP1.Op02FY, DSP1.Op02FZ, DSP1.Op02LFE, DSP1.Op02LES, DSP1.Op02AAS, DSP1.Op02AZS, &DSP1.Op02VOF, &DSP1.Op02VVA, &DSP1.Op02CX, &DSP1.Op02CY);
}

static void DSP1_Op0A (void)
{
	DSP1_Raster(DSP1.Op0AVS, &DSP1.Op0AA, &DSP1.Op0AB, &DSP1.Op0AC, &DSP1.Op0AD);
	DSP1.Op0AVS++;
}

static int16 DSP1_ShiftR (int16 C, int16 E)
{
	return (C * DSP1ROM[0x0031 + E] >> 15);
}

static void DSP1_Project (int16 X, int16 Y, int16 Z, int16 *H, int16 *V, int16 *M)
{
	int32	aux, aux4;
	int16	E, E2, E3, E4, E5, refE, E6, E7;
	int16	C2, C4, C6, C8, C9, C10, C11, C12, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26;
	int16	Px, Py, Pz;

	E4 = E3 = E2 = E = E5 = 0;

	DSP1_NormalizeDouble((int32) X - DSP1.Gx, &Px, &E4);
	DSP1_NormalizeDouble((int32) Y - DSP1.Gy, &Py, &E );
	DSP1_NormalizeDouble((int32) Z - DSP1.Gz, &Pz, &E3);
	Px >>= 1; // to avoid overflows when calculating the scalar products
	E4--;
	Py >>= 1;
	E--;
	Pz >>= 1;
	E3--;

	refE = (E < E3) ? E : E3;
	refE = (refE < E4) ? refE : E4;

	Px = DSP1_ShiftR(Px, E4 - refE); // normalize them to the same exponent
	Py = DSP1_ShiftR(Py, E  - refE);
	Pz = DSP1_ShiftR(Pz, E3 - refE);

	C11 =- (Px * DSP1.Nx >> 15);
	C8  =- (Py * DSP1.Ny >> 15);
	C9  =- (Pz * DSP1.Nz >> 15);
	C12 = C11 + C8 + C9; // this cannot overflow!

	aux4 = C12; // de-normalization with 32-bits arithmetic
	refE = 16 - refE; // refE can be up to 3
	if (refE >= 0)
		aux4 <<=  (refE);
	else
		aux4 >>= -(refE);
	if (aux4 == -1)
		aux4 = 0; // why?
	aux4 >>= 1;

	aux = ((uint16) DSP1.G_Les) + aux4; // Les - the scalar product of P with the normal vector of the screen
	DSP1_NormalizeDouble(aux, &C10, &E2);
	E2 = 15 - E2;

	DSP1_Inverse(C10, 0, &C4, &E4);
	C2 = C4 * DSP1.C_Les >> 15; // scale factor

	// H
	E7 = 0;
	C16 = Px * ( DSP1.CosAas *  0x7fff >> 15) >> 15;
	C20 = Py * ( DSP1.SinAas *  0x7fff >> 15) >> 15;
	C17 = C16 + C20; // scalar product of P with the normalized horizontal vector of the screen...

	C18 = C17 * C2 >> 15; // ... multiplied by the scale factor
	DSP1_Normalize(C18, &C19, &E7);
	*H = DSP1_Truncate(C19, DSP1.E_Les - E2 + refE + E7);

	// V
	E6 = 0;
	C21 = Px * ( DSP1.CosAzs * -DSP1.SinAas >> 15) >> 15;
	C22 = Py * ( DSP1.CosAzs *  DSP1.CosAas >> 15) >> 15;
	C23 = Pz * (-DSP1.SinAzs *  0x7fff >> 15) >> 15;
	C24 = C21 + C22 + C23; // scalar product of P with the normalized vertical vector of the screen...

	C26 = C24 * C2 >> 15; // ... multiplied by the scale factor
	DSP1_Normalize(C26, &C25, &E6);
	*V = DSP1_Truncate(C25, DSP1.E_Les - E2 + refE + E6);

	// M
	DSP1_Normalize(C2, &C6, &E4);
	*M = DSP1_Truncate(C6, E4 + DSP1.E_Les - E2 - 7); // M is the scale factor divided by 2^7
}

static void DSP1_Op06 (void)
{
	DSP1_Project(DSP1.Op06X, DSP1.Op06Y, DSP1.Op06Z, &DSP1.Op06H, &DSP1.Op06V, &DSP1.Op06M);
}

static void DSP1_Op01 (void)
{
	int16	SinAz = DSP1_Sin(DSP1.Op01Zr);
	int16	CosAz = DSP1_Cos(DSP1.Op01Zr);
	int16	SinAy = DSP1_Sin(DSP1.Op01Yr);
	int16	CosAy = DSP1_Cos(DSP1.Op01Yr);
	int16	SinAx = DSP1_Sin(DSP1.Op01Xr);
	int16	CosAx = DSP1_Cos(DSP1.Op01Xr);

	DSP1.Op01m >>= 1;

	DSP1.matrixA[0][0] =   (DSP1.Op01m * CosAz >> 15) * CosAy >> 15;
	DSP1.matrixA[0][1] = -((DSP1.Op01m * SinAz >> 15) * CosAy >> 15);
	DSP1.matrixA[0][2] =    DSP1.Op01m * SinAy >> 15;

	DSP1.matrixA[1][0] =  ((DSP1.Op01m * SinAz >> 15) * CosAx >> 15) + (((DSP1.Op01m * CosAz >> 15) * SinAx >> 15) * SinAy >> 15);
	DSP1.matrixA[1][1] =  ((DSP1.Op01m * CosAz >> 15) * CosAx >> 15) - (((DSP1.Op01m * SinAz >> 15) * SinAx >> 15) * SinAy >> 15);
	DSP1.matrixA[1][2] = -((DSP1.Op01m * SinAx >> 15) * CosAy >> 15);

	DSP1.matrixA[2][0] =  ((DSP1.Op01m * SinAz >> 15) * SinAx >> 15) - (((DSP1.Op01m * CosAz >> 15) * CosAx >> 15) * SinAy >> 15);
	DSP1.matrixA[2][1] =  ((DSP1.Op01m * CosAz >> 15) * SinAx >> 15) + (((DSP1.Op01m * SinAz >> 15) * CosAx >> 15) * SinAy >> 15);
	DSP1.matrixA[2][2] =   (DSP1.Op01m * CosAx >> 15) * CosAy >> 15;
}

static void DSP1_Op11 (void)
{
	int16	SinAz = DSP1_Sin(DSP1.Op11Zr);
	int16	CosAz = DSP1_Cos(DSP1.Op11Zr);
	int16	SinAy = DSP1_Sin(DSP1.Op11Yr);
	int16	CosAy = DSP1_Cos(DSP1.Op11Yr);
	int16	SinAx = DSP1_Sin(DSP1.Op11Xr);
	int16	CosAx = DSP1_Cos(DSP1.Op11Xr);

	DSP1.Op11m >>= 1;

	DSP1.matrixB[0][0] =   (DSP1.Op11m * CosAz >> 15) * CosAy >> 15;
	DSP1.matrixB[0][1] = -((DSP1.Op11m * SinAz >> 15) * CosAy >> 15);
	DSP1.matrixB[0][2] =    DSP1.Op11m * SinAy >> 15;

	DSP1.matrixB[1][0] =  ((DSP1.Op11m * SinAz >> 15) * CosAx >> 15) + (((DSP1.Op11m * CosAz >> 15) * SinAx >> 15) * SinAy >> 15);
	DSP1.matrixB[1][1] =  ((DSP1.Op11m * CosAz >> 15) * CosAx >> 15) - (((DSP1.Op11m * SinAz >> 15) * SinAx >> 15) * SinAy >> 15);
	DSP1.matrixB[1][2] = -((DSP1.Op11m * SinAx >> 15) * CosAy >> 15);

	DSP1.matrixB[2][0] =  ((DSP1.Op11m * SinAz >> 15) * SinAx >> 15) - (((DSP1.Op11m * CosAz >> 15) * CosAx >> 15) * SinAy >> 15);
	DSP1.matrixB[2][1] =  ((DSP1.Op11m * CosAz >> 15) * SinAx >> 15) + (((DSP1.Op11m * SinAz >> 15) * CosAx >> 15) * SinAy >> 15);
	DSP1.matrixB[2][2] =   (DSP1.Op11m * CosAx >> 15) * CosAy >> 15;
}

static void DSP1_Op21 (void)
{
	int16	SinAz = DSP1_Sin(DSP1.Op21Zr);
	int16	CosAz = DSP1_Cos(DSP1.Op21Zr);
	int16	SinAy = DSP1_Sin(DSP1.Op21Yr);
	int16	CosAy = DSP1_Cos(DSP1.Op21Yr);
	int16	SinAx = DSP1_Sin(DSP1.Op21Xr);
	int16	CosAx = DSP1_Cos(DSP1.Op21Xr);

	DSP1.Op21m >>= 1;

	DSP1.matrixC[0][0] =   (DSP1.Op21m * CosAz >> 15) * CosAy >> 15;
	DSP1.matrixC[0][1] = -((DSP1.Op21m * SinAz >> 15) * CosAy >> 15);
	DSP1.matrixC[0][2] =    DSP1.Op21m * SinAy >> 15;

	DSP1.matrixC[1][0] =  ((DSP1.Op21m * SinAz >> 15) * CosAx >> 15) + (((DSP1.Op21m * CosAz >> 15) * SinAx >> 15) * SinAy >> 15);
	DSP1.matrixC[1][1] =  ((DSP1.Op21m * CosAz >> 15) * CosAx >> 15) - (((DSP1.Op21m * SinAz >> 15) * SinAx >> 15) * SinAy >> 15);
	DSP1.matrixC[1][2] = -((DSP1.Op21m * SinAx >> 15) * CosAy >> 15);

	DSP1.matrixC[2][0] =  ((DSP1.Op21m * SinAz >> 15) * SinAx >> 15) - (((DSP1.Op21m * CosAz >> 15) * CosAx >> 15) * SinAy >> 15);
	DSP1.matrixC[2][1] =  ((DSP1.Op21m * CosAz >> 15) * SinAx >> 15) + (((DSP1.Op21m * SinAz >> 15) * CosAx >> 15) * SinAy >> 15);
	DSP1.matrixC[2][2] =   (DSP1.Op21m * CosAx >> 15) * CosAy >> 15;
}

static void DSP1_Op0D (void)
{
	DSP1.Op0DF = (DSP1.Op0DX * DSP1.matrixA[0][0] >> 15) + (DSP1.Op0DY * DSP1.matrixA[0][1] >> 15) + (DSP1.Op0DZ * DSP1.matrixA[0][2] >> 15);
	DSP1.Op0DL = (DSP1.Op0DX * DSP1.matrixA[1][0] >> 15) + (DSP1.Op0DY * DSP1.matrixA[1][1] >> 15) + (DSP1.Op0DZ * DSP1.matrixA[1][2] >> 15);
	DSP1.Op0DU = (DSP1.Op0DX * DSP1.matrixA[2][0] >> 15) + (DSP1.Op0DY * DSP1.matrixA[2][1] >> 15) + (DSP1.Op0DZ * DSP1.matrixA[2][2] >> 15);

#ifdef DebugDSP1
	Log_Message("OP0D X: %d Y: %d Z: %d / F: %d L: %d U: %d", DSP1.Op0DX, DSP1.Op0DY, DSP1.Op0DZ, DSP1.Op0DF, DSP1.Op0DL, DSP1.Op0DU);
#endif
}

static void DSP1_Op1D (void)
{
	DSP1.Op1DF = (DSP1.Op1DX * DSP1.matrixB[0][0] >> 15) + (DSP1.Op1DY * DSP1.matrixB[0][1] >> 15) + (DSP1.Op1DZ * DSP1.matrixB[0][2] >> 15);
	DSP1.Op1DL = (DSP1.Op1DX * DSP1.matrixB[1][0] >> 15) + (DSP1.Op1DY * DSP1.matrixB[1][1] >> 15) + (DSP1.Op1DZ * DSP1.matrixB[1][2] >> 15);
	DSP1.Op1DU = (DSP1.Op1DX * DSP1.matrixB[2][0] >> 15) + (DSP1.Op1DY * DSP1.matrixB[2][1] >> 15) + (DSP1.Op1DZ * DSP1.matrixB[2][2] >> 15);

#ifdef DebugDSP1
	Log_Message("OP1D X: %d Y: %d Z: %d / F: %d L: %d U: %d", DSP1.Op1DX, DSP1.Op1DY, DSP1.Op1DZ, DSP1.Op1DF, DSP1.Op1DL, DSP1.Op1DU);
#endif
}

static void DSP1_Op2D (void)
{
	DSP1.Op2DF = (DSP1.Op2DX * DSP1.matrixC[0][0] >> 15) + (DSP1.Op2DY * DSP1.matrixC[0][1] >> 15) + (DSP1.Op2DZ * DSP1.matrixC[0][2] >> 15);
	DSP1.Op2DL = (DSP1.Op2DX * DSP1.matrixC[1][0] >> 15) + (DSP1.Op2DY * DSP1.matrixC[1][1] >> 15) + (DSP1.Op2DZ * DSP1.matrixC[1][2] >> 15);
	DSP1.Op2DU = (DSP1.Op2DX * DSP1.matrixC[2][0] >> 15) + (DSP1.Op2DY * DSP1.matrixC[2][1] >> 15) + (DSP1.Op2DZ * DSP1.matrixC[2][2] >> 15);

#ifdef DebugDSP1
	Log_Message("OP2D X: %d Y: %d Z: %d / F: %d L: %d U: %d", DSP1.Op2DX, DSP1.Op2DY, DSP1.Op2DZ, DSP1.Op2DF, DSP1.Op2DL, DSP1.Op2DU);
#endif
}

static void DSP1_Op03 (void)
{
	DSP1.Op03X = (DSP1.Op03F * DSP1.matrixA[0][0] >> 15) + (DSP1.Op03L * DSP1.matrixA[1][0] >> 15) + (DSP1.Op03U * DSP1.matrixA[2][0] >> 15);
	DSP1.Op03Y = (DSP1.Op03F * DSP1.matrixA[0][1] >> 15) + (DSP1.Op03L * DSP1.matrixA[1][1] >> 15) + (DSP1.Op03U * DSP1.matrixA[2][1] >> 15);
	DSP1.Op03Z = (DSP1.Op03F * DSP1.matrixA[0][2] >> 15) + (DSP1.Op03L * DSP1.matrixA[1][2] >> 15) + (DSP1.Op03U * DSP1.matrixA[2][2] >> 15);

#ifdef DebugDSP1
	Log_Message("OP03 F: %d L: %d U: %d / X: %d Y: %d Z: %d", DSP1.Op03F, DSP1.Op03L, DSP1.Op03U, DSP1.Op03X, DSP1.Op03Y, DSP1.Op03Z);
#endif
}

static void DSP1_Op13 (void)
{
	DSP1.Op13X = (DSP1.Op13F * DSP1.matrixB[0][0] >> 15) + (DSP1.Op13L * DSP1.matrixB[1][0] >> 15) + (DSP1.Op13U * DSP1.matrixB[2][0] >> 15);
	DSP1.Op13Y = (DSP1.Op13F * DSP1.matrixB[0][1] >> 15) + (DSP1.Op13L * DSP1.matrixB[1][1] >> 15) + (DSP1.Op13U * DSP1.matrixB[2][1] >> 15);
	DSP1.Op13Z = (DSP1.Op13F * DSP1.matrixB[0][2] >> 15) + (DSP1.Op13L * DSP1.matrixB[1][2] >> 15) + (DSP1.Op13U * DSP1.matrixB[2][2] >> 15);

#ifdef DebugDSP1
	Log_Message("OP13 F: %d L: %d U: %d / X: %d Y: %d Z: %d", DSP1.Op13F, DSP1.Op13L, DSP1.Op13U, DSP1.Op13X, DSP1.Op13Y, DSP1.Op13Z);
#endif
}

static void DSP1_Op23 (void)
{
	DSP1.Op23X = (DSP1.Op23F * DSP1.matrixC[0][0] >> 15) + (DSP1.Op23L * DSP1.matrixC[1][0] >> 15) + (DSP1.Op23U * DSP1.matrixC[2][0] >> 15);
	DSP1.Op23Y = (DSP1.Op23F * DSP1.matrixC[0][1] >> 15) + (DSP1.Op23L * DSP1.matrixC[1][1] >> 15) + (DSP1.Op23U * DSP1.matrixC[2][1] >> 15);
	DSP1.Op23Z = (DSP1.Op23F * DSP1.matrixC[0][2] >> 15) + (DSP1.Op23L * DSP1.matrixC[1][2] >> 15) + (DSP1.Op23U * DSP1.matrixC[2][2] >> 15);

#ifdef DebugDSP1
	Log_Message("OP23 F: %d L: %d U: %d / X: %d Y: %d Z: %d", DSP1.Op23F, DSP1.Op23L, DSP1.Op23U, DSP1.Op23X, DSP1.Op23Y, DSP1.Op23Z);
#endif
}

static void DSP1_Op14 (void)
{
	int16	CSec, ESec, CTan, CSin, C, E;

	DSP1_Inverse(DSP1_Cos(DSP1.Op14Xr), 0, &CSec, &ESec);

	// Rotation Around Z
	DSP1_NormalizeDouble(DSP1.Op14U * DSP1_Cos(DSP1.Op14Yr) - DSP1.Op14F * DSP1_Sin(DSP1.Op14Yr), &C, &E);

	E = ESec - E;

	DSP1_Normalize(C * CSec >> 15, &C, &E);

	DSP1.Op14Zrr = DSP1.Op14Zr + DSP1_Truncate(C, E);

	// Rotation Around X
	DSP1.Op14Xrr = DSP1.Op14Xr + (DSP1.Op14U * DSP1_Sin(DSP1.Op14Yr) >> 15) + (DSP1.Op14F * DSP1_Cos(DSP1.Op14Yr) >> 15);

	// Rotation Around Y
	DSP1_NormalizeDouble(DSP1.Op14U * DSP1_Cos(DSP1.Op14Yr) + DSP1.Op14F * DSP1_Sin(DSP1.Op14Yr), &C, &E);

	E = ESec - E;

	DSP1_Normalize(DSP1_Sin(DSP1.Op14Xr), &CSin, &E);

	CTan = CSec * CSin >> 15;

	DSP1_Normalize(-(C * CTan >> 15), &C, &E);

	DSP1.Op14Yrr = DSP1.Op14Yr + DSP1_Truncate(C, E) + DSP1.Op14L;
}

static void DSP1_Target (int16 H, int16 V, int16 *X, int16 *Y)
{
	int16	C, E, C1, E1;

	DSP1_Inverse((V * DSP1.SinAzs >> 15) + DSP1.VOffset, 8, &C, &E);
	E += DSP1.VPlane_E;

	C1 = C * DSP1.VPlane_C >> 15;
	E1 = E + DSP1.SecAZS_E1;

	H <<= 8;

	DSP1_Normalize(C1, &C, &E);

	C = DSP1_Truncate(C, E) * H >> 15;

	*X = DSP1.CentreX + (C * DSP1.CosAas >> 15);
	*Y = DSP1.CentreY - (C * DSP1.SinAas >> 15);

	V <<= 8;

	DSP1_Normalize(C1 * DSP1.SecAZS_C1 >> 15, &C, &E1);

	C = DSP1_Truncate(C, E1) * V >> 15;

	*X += C * -DSP1.SinAas >> 15;
	*Y += C *  DSP1.CosAas >> 15;
}

static void DSP1_Op0E (void)
{
	DSP1_Target(DSP1.Op0EH, DSP1.Op0EV, &DSP1.Op0EX, &DSP1.Op0EY);
}

static void DSP1_Op0B (void)
{
	DSP1.Op0BS = (DSP1.Op0BX * DSP1.matrixA[0][0] + DSP1.Op0BY * DSP1.matrixA[0][1] + DSP1.Op0BZ * DSP1.matrixA[0][2]) >> 15;

#ifdef DebugDSP1
	Log_Message("OP0B");
#endif
}

static void DSP1_Op1B (void)
{
	DSP1.Op1BS = (DSP1.Op1BX * DSP1.matrixB[0][0] + DSP1.Op1BY * DSP1.matrixB[0][1] + DSP1.Op1BZ * DSP1.matrixB[0][2]) >> 15;

#ifdef DebugDSP1
	Log_Message("OP1B X: %d Y: %d Z: %d S: %d", DSP1.Op1BX, DSP1.Op1BY, DSP1.Op1BZ, DSP1.Op1BS);
	Log_Message("     MX: %d MY: %d MZ: %d Scale: %d", (int16) (DSP1.matrixB[0][0] * 100), (int16) (DSP1.matrixB[0][1] * 100), (int16) (DSP1.matrixB[0][2] * 100), (int16) (DSP1.Op1BS * 100));
#endif
}

static void DSP1_Op2B (void)
{
	DSP1.Op2BS = (DSP1.Op2BX * DSP1.matrixC[0][0] + DSP1.Op2BY * DSP1.matrixC[0][1] + DSP1.Op2BZ * DSP1.matrixC[0][2]) >> 15;

#ifdef DebugDSP1
	Log_Message("OP2B");
#endif
}

static void DSP1_Op08 (void)
{
	int32	op08Size = (DSP1.Op08X * DSP1.Op08X + DSP1.Op08Y * DSP1.Op08Y + DSP1.Op08Z * DSP1.Op08Z) << 1;
	DSP1.Op08Ll =  op08Size        & 0xffff;
	DSP1.Op08Lh = (op08Size >> 16) & 0xffff;

#ifdef DebugDSP1
	Log_Message("OP08 %d,%d,%d", DSP1.Op08X, DSP1.Op08Y, DSP1.Op08Z);
	Log_Message("OP08 ((OP08X^2)+(OP08Y^2)+(OP08Z^2))=%x", op08Size);
#endif
}

static void DSP1_Op18 (void)
{
	DSP1.Op18D = (DSP1.Op18X * DSP1.Op18X + DSP1.Op18Y * DSP1.Op18Y + DSP1.Op18Z * DSP1.Op18Z - DSP1.Op18R * DSP1.Op18R) >> 15;

#ifdef DebugDSP1
	Log_Message("OP18 X: %d Y: %d Z: %d R: %D DIFF %d", DSP1.Op18X, DSP1.Op18Y, DSP1.Op38Z, DSP1.Op18D);
#endif
}

static void DSP1_Op38 (void)
{
	DSP1.Op38D = (DSP1.Op38X * DSP1.Op38X + DSP1.Op38Y * DSP1.Op38Y + DSP1.Op38Z * DSP1.Op38Z - DSP1.Op38R * DSP1.Op38R) >> 15;
	DSP1.Op38D++;

#ifdef DebugDSP1
	Log_Message("OP38 X: %d Y: %d Z: %d R: %D DIFF %d", DSP1.Op38X, DSP1.Op38Y, DSP1.Op38Z, DSP1.Op38D);
#endif
}

static void DSP1_Op28 (void)
{
	int32	Radius = DSP1.Op28X * DSP1.Op28X + DSP1.Op28Y * DSP1.Op28Y + DSP1.Op28Z * DSP1.Op28Z;

	if (Radius == 0)
		DSP1.Op28R = 0;
	else
	{
		int16	C, E, Pos, Node1, Node2;

		DSP1_NormalizeDouble(Radius, &C, &E);
		if (E & 1)
			C = C * 0x4000 >> 15;

		Pos = C * 0x0040 >> 15;

		Node1 = DSP1ROM[0x00d5 + Pos];
		Node2 = DSP1ROM[0x00d6 + Pos];

		DSP1.Op28R = ((Node2 - Node1) * (C & 0x1ff) >> 9) + Node1;
		DSP1.Op28R >>= (E >> 1);
	}

#ifdef DebugDSP1
	Log_Message("OP28 X:%d Y:%d Z:%d", DSP1.Op28X, DSP1.Op28Y, DSP1.Op28Z);
	Log_Message("OP28 Vector Length %d", DSP1.Op28R);
#endif
}

static void DSP1_Op1C (void)
{
	// Rotate Around Op1CZ1
	DSP1.Op1CX1 = (DSP1.Op1CYBR * DSP1_Sin(DSP1.Op1CZ) >> 15) + (DSP1.Op1CXBR * DSP1_Cos(DSP1.Op1CZ) >> 15);
	DSP1.Op1CY1 = (DSP1.Op1CYBR * DSP1_Cos(DSP1.Op1CZ) >> 15) - (DSP1.Op1CXBR * DSP1_Sin(DSP1.Op1CZ) >> 15);
	DSP1.Op1CXBR = DSP1.Op1CX1;
	DSP1.Op1CYBR = DSP1.Op1CY1;

	// Rotate Around Op1CY1
	DSP1.Op1CZ1 = (DSP1.Op1CXBR * DSP1_Sin(DSP1.Op1CY) >> 15) + (DSP1.Op1CZBR * DSP1_Cos(DSP1.Op1CY) >> 15);
	DSP1.Op1CX1 = (DSP1.Op1CXBR * DSP1_Cos(DSP1.Op1CY) >> 15) - (DSP1.Op1CZBR * DSP1_Sin(DSP1.Op1CY) >> 15);
	DSP1.Op1CXAR = DSP1.Op1CX1;
	DSP1.Op1CZBR = DSP1.Op1CZ1;

	// Rotate Around Op1CX1
	DSP1.Op1CY1 = (DSP1.Op1CZBR * DSP1_Sin(DSP1.Op1CX) >> 15) + (DSP1.Op1CYBR * DSP1_Cos(DSP1.Op1CX) >> 15);
	DSP1.Op1CZ1 = (DSP1.Op1CZBR * DSP1_Cos(DSP1.Op1CX) >> 15) - (DSP1.Op1CYBR * DSP1_Sin(DSP1.Op1CX) >> 15);
	DSP1.Op1CYAR = DSP1.Op1CY1;
	DSP1.Op1CZAR = DSP1.Op1CZ1;

#ifdef DebugDSP1
	Log_Message("OP1C Apply Matrix CX:%d CY:%d CZ", DSP1.Op1CXAR, DSP1.Op1CYAR, DSP1.Op1CZAR);
#endif
}

static void DSP1_Op0F (void)
{
	DSP1.Op0FPass = 0x0000;

#ifdef DebugDSP1
	Log_Message("OP0F RAM Test Pass:%d", DSP1.Op0FPass);
#endif
}

static void DSP1_Op2F (void)
{
	DSP1.Op2FSize = 0x100;
}

void DSP1SetByte (uint8 byte, uint16 address)
{
	if (address < DSP0.boundary)
	{
		if ((DSP1.command == 0x0A || DSP1.command == 0x1A) && DSP1.out_count != 0)
		{
			DSP1.out_count--;
			DSP1.out_index++;
			return;
		}
		else
		if (DSP1.waiting4command)
		{
			DSP1.command         = byte;
			DSP1.in_index        = 0;
			DSP1.waiting4command = FALSE;
			DSP1.first_parameter = TRUE;
			#ifdef DEBUGGER
				//printf("OP%02X\n",byte);
			#endif

			switch (byte)
			{
				case 0x00: DSP1.in_count = 2; break;
				case 0x30:
				case 0x10: DSP1.in_count = 2; break;
				case 0x20: DSP1.in_count = 2; break;
				case 0x24:
				case 0x04: DSP1.in_count = 2; break;
				case 0x08: DSP1.in_count = 3; break;
				case 0x18: DSP1.in_count = 4; break;
				case 0x28: DSP1.in_count = 3; break;
				case 0x38: DSP1.in_count = 4; break;
				case 0x2c:
				case 0x0c: DSP1.in_count = 3; break;
				case 0x3c:
				case 0x1c: DSP1.in_count = 6; break;
				case 0x32:
				case 0x22:
				case 0x12:
				case 0x02: DSP1.in_count = 7; break;
				case 0x0a: DSP1.in_count = 1; break;
				case 0x3a:
				case 0x2a:
				case 0x1a:
					DSP1.command = 0x1a;
					DSP1.in_count = 1;
					break;
				case 0x16:
				case 0x26:
				case 0x36:
				case 0x06: DSP1.in_count = 3; break;
				case 0x1e:
				case 0x2e:
				case 0x3e:
				case 0x0e: DSP1.in_count = 2; break;
				case 0x05:
				case 0x35:
				case 0x31:
				case 0x01: DSP1.in_count = 4; break;
				case 0x15:
				case 0x11: DSP1.in_count = 4; break;
				case 0x25:
				case 0x21: DSP1.in_count = 4; break;
				case 0x09:
				case 0x39:
				case 0x3d:
				case 0x0d: DSP1.in_count = 3; break;
				case 0x19:
				case 0x1d: DSP1.in_count = 3; break;
				case 0x29:
				case 0x2d: DSP1.in_count = 3; break;
				case 0x33:
				case 0x03: DSP1.in_count = 3; break;
				case 0x13: DSP1.in_count = 3; break;
				case 0x23: DSP1.in_count = 3; break;
				case 0x3b:
				case 0x0b: DSP1.in_count = 3; break;
				case 0x1b: DSP1.in_count = 3; break;
				case 0x2b: DSP1.in_count = 3; break;
				case 0x34:
				case 0x14: DSP1.in_count = 6; break;
				case 0x07:
				case 0x0f: DSP1.in_count = 1; break;
				case 0x27:
				case 0x2F: DSP1.in_count = 1; break;
				case 0x17:
				case 0x37:
				case 0x3F:
					DSP1.command = 0x1f;
				case 0x1f: DSP1.in_count = 1; break;
				default:
				#ifdef DEBUGGER
					//printf("OP%02X\n", byte);
				#endif
				case 0x80:
					DSP1.in_count        = 0;
					DSP1.waiting4command = TRUE;
					DSP1.first_parameter = TRUE;
					break;
			}

			DSP1.in_count <<= 1;
		}
		else
		{
			DSP1.parameters[DSP1.in_index] = byte;
			DSP1.first_parameter = FALSE;
			DSP1.in_index++;
		}

		if (DSP1.waiting4command || (DSP1.first_parameter && byte == 0x80))
		{
			DSP1.waiting4command = TRUE;
			DSP1.first_parameter = FALSE;
		}
		else
		if (DSP1.first_parameter && (DSP1.in_count != 0 || (DSP1.in_count == 0 && DSP1.in_index == 0)))
			;
		else
		{
			if (DSP1.in_count)
			{
				if (--DSP1.in_count == 0)
				{
					// Actually execute the command
					DSP1.waiting4command = TRUE;
					DSP1.out_index       = 0;

					switch (DSP1.command)
					{
						case 0x1f:
							DSP1.out_count = 2048;
							break;

						case 0x00: // Multiple
							DSP1.Op00Multiplicand = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op00Multiplier   = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));

							DSP1_Op00();

							DSP1.out_count = 2;
							DSP1.output[0] =  DSP1.Op00Result       & 0xFF;
							DSP1.output[1] = (DSP1.Op00Result >> 8) & 0xFF;
							break;

						case 0x20: // Multiple
							DSP1.Op20Multiplicand = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op20Multiplier   = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));

							DSP1_Op20();

							DSP1.out_count = 2;
							DSP1.output[0] =  DSP1.Op20Result       & 0xFF;
							DSP1.output[1] = (DSP1.Op20Result >> 8) & 0xFF;
							break;

						case 0x30:
						case 0x10: // Inverse
							DSP1.Op10Coefficient = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op10Exponent    = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));

							DSP1_Op10();

							DSP1.out_count = 4;
							DSP1.output[0] = (uint8) ( ((int16) DSP1.Op10CoefficientR)       & 0xFF);
							DSP1.output[1] = (uint8) ((((int16) DSP1.Op10CoefficientR) >> 8) & 0xFF);
							DSP1.output[2] = (uint8) ( ((int16) DSP1.Op10ExponentR   )       & 0xFF);
							DSP1.output[3] = (uint8) ((((int16) DSP1.Op10ExponentR   ) >> 8) & 0xFF);
							break;

						case 0x24:
						case 0x04: // Sin and Cos of angle
							DSP1.Op04Angle  = (int16)  (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op04Radius = (uint16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));

							DSP1_Op04();

							DSP1.out_count = 4;
							DSP1.output[0] = (uint8)  (DSP1.Op04Sin       & 0xFF);
							DSP1.output[1] = (uint8) ((DSP1.Op04Sin >> 8) & 0xFF);
							DSP1.output[2] = (uint8)  (DSP1.Op04Cos       & 0xFF);
							DSP1.output[3] = (uint8) ((DSP1.Op04Cos >> 8) & 0xFF);
							break;

						case 0x08: // Radius
							DSP1.Op08X = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op08Y = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));
							DSP1.Op08Z = (int16) (DSP1.parameters[4] | (DSP1.parameters[5] << 8));

							DSP1_Op08();

							DSP1.out_count = 4;
							DSP1.output[0] = (uint8) ( ((int16) DSP1.Op08Ll)       & 0xFF);
							DSP1.output[1] = (uint8) ((((int16) DSP1.Op08Ll) >> 8) & 0xFF);
							DSP1.output[2] = (uint8) ( ((int16) DSP1.Op08Lh)       & 0xFF);
							DSP1.output[3] = (uint8) ((((int16) DSP1.Op08Lh) >> 8) & 0xFF);
							break;

						case 0x18: // Range

							DSP1.Op18X = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op18Y = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));
							DSP1.Op18Z = (int16) (DSP1.parameters[4] | (DSP1.parameters[5] << 8));
							DSP1.Op18R = (int16) (DSP1.parameters[6] | (DSP1.parameters[7] << 8));

							DSP1_Op18();

							DSP1.out_count = 2;
							DSP1.output[0] = (uint8)  (DSP1.Op18D       & 0xFF);
							DSP1.output[1] = (uint8) ((DSP1.Op18D >> 8) & 0xFF);
							break;

						case 0x38: // Range

							DSP1.Op38X = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op38Y = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));
							DSP1.Op38Z = (int16) (DSP1.parameters[4] | (DSP1.parameters[5] << 8));
							DSP1.Op38R = (int16) (DSP1.parameters[6] | (DSP1.parameters[7] << 8));

							DSP1_Op38();

							DSP1.out_count = 2;
							DSP1.output[0] = (uint8)  (DSP1.Op38D       & 0xFF);
							DSP1.output[1] = (uint8) ((DSP1.Op38D >> 8) & 0xFF);
							break;

						case 0x28: // Distance (vector length)
							DSP1.Op28X = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op28Y = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));
							DSP1.Op28Z = (int16) (DSP1.parameters[4] | (DSP1.parameters[5] << 8));

							DSP1_Op28();

							DSP1.out_count = 2;
							DSP1.output[0] = (uint8)  (DSP1.Op28R       & 0xFF);
							DSP1.output[1] = (uint8) ((DSP1.Op28R >> 8) & 0xFF);
							break;

						case 0x2c:
						case 0x0c: // Rotate (2D rotate)
							DSP1.Op0CA  = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op0CX1 = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));
							DSP1.Op0CY1 = (int16) (DSP1.parameters[4] | (DSP1.parameters[5] << 8));

							DSP1_Op0C();

							DSP1.out_count = 4;
							DSP1.output[0] = (uint8)  (DSP1.Op0CX2       & 0xFF);
							DSP1.output[1] = (uint8) ((DSP1.Op0CX2 >> 8) & 0xFF);
							DSP1.output[2] = (uint8)  (DSP1.Op0CY2       & 0xFF);
							DSP1.output[3] = (uint8) ((DSP1.Op0CY2 >> 8) & 0xFF);
							break;

						case 0x3c:
						case 0x1c: // Polar (3D rotate)
							DSP1.Op1CZ   = (DSP1.parameters[ 0] | (DSP1.parameters[ 1] << 8));
							//MK: reversed X and Y on neviksti and John's advice.
							DSP1.Op1CY   = (DSP1.parameters[ 2] | (DSP1.parameters[ 3] << 8));
							DSP1.Op1CX   = (DSP1.parameters[ 4] | (DSP1.parameters[ 5] << 8));
							DSP1.Op1CXBR = (DSP1.parameters[ 6] | (DSP1.parameters[ 7] << 8));
							DSP1.Op1CYBR = (DSP1.parameters[ 8] | (DSP1.parameters[ 9] << 8));
							DSP1.Op1CZBR = (DSP1.parameters[10] | (DSP1.parameters[11] << 8));

							DSP1_Op1C();

							DSP1.out_count = 6;
							DSP1.output[0] = (uint8)  (DSP1.Op1CXAR       & 0xFF);
							DSP1.output[1] = (uint8) ((DSP1.Op1CXAR >> 8) & 0xFF);
							DSP1.output[2] = (uint8)  (DSP1.Op1CYAR       & 0xFF);
							DSP1.output[3] = (uint8) ((DSP1.Op1CYAR >> 8) & 0xFF);
							DSP1.output[4] = (uint8)  (DSP1.Op1CZAR       & 0xFF);
							DSP1.output[5] = (uint8) ((DSP1.Op1CZAR >> 8) & 0xFF);
							break;

						case 0x32:
						case 0x22:
						case 0x12:
						case 0x02: // Parameter (Projection)
							DSP1.Op02FX  = (int16)  (DSP1.parameters[ 0] | (DSP1.parameters[ 1] << 8));
							DSP1.Op02FY  = (int16)  (DSP1.parameters[ 2] | (DSP1.parameters[ 3] << 8));
							DSP1.Op02FZ  = (int16)  (DSP1.parameters[ 4] | (DSP1.parameters[ 5] << 8));
							DSP1.Op02LFE = (int16)  (DSP1.parameters[ 6] | (DSP1.parameters[ 7] << 8));
							DSP1.Op02LES = (int16)  (DSP1.parameters[ 8] | (DSP1.parameters[ 9] << 8));
							DSP1.Op02AAS = (uint16) (DSP1.parameters[10] | (DSP1.parameters[11] << 8));
							DSP1.Op02AZS = (uint16) (DSP1.parameters[12] | (DSP1.parameters[13] << 8));

							DSP1_Op02();

							DSP1.out_count = 8;
							DSP1.output[0] = (uint8)  (DSP1.Op02VOF       & 0xFF);
							DSP1.output[1] = (uint8) ((DSP1.Op02VOF >> 8) & 0xFF);
							DSP1.output[2] = (uint8)  (DSP1.Op02VVA       & 0xFF);
							DSP1.output[3] = (uint8) ((DSP1.Op02VVA >> 8) & 0xFF);
							DSP1.output[4] = (uint8)  (DSP1.Op02CX        & 0xFF);
							DSP1.output[5] = (uint8) ((DSP1.Op02CX  >> 8) & 0xFF);
							DSP1.output[6] = (uint8)  (DSP1.Op02CY        & 0xFF);
							DSP1.output[7] = (uint8) ((DSP1.Op02CY  >> 8) & 0xFF);
							break;

						case 0x3a:
						case 0x2a:
						case 0x1a: // Raster mode 7 matrix data
						case 0x0a:
							DSP1.Op0AVS = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));

							DSP1_Op0A();

							DSP1.out_count = 8;
							DSP1.output[0] = (uint8)  (DSP1.Op0AA       & 0xFF);
							DSP1.output[1] = (uint8) ((DSP1.Op0AA >> 8) & 0xFF);
							DSP1.output[2] = (uint8)  (DSP1.Op0AB       & 0xFF);
							DSP1.output[3] = (uint8) ((DSP1.Op0AB >> 8) & 0xFF);
							DSP1.output[4] = (uint8)  (DSP1.Op0AC       & 0xFF);
							DSP1.output[5] = (uint8) ((DSP1.Op0AC >> 8) & 0xFF);
							DSP1.output[6] = (uint8)  (DSP1.Op0AD       & 0xFF);
							DSP1.output[7] = (uint8) ((DSP1.Op0AD >> 8) & 0xFF);
							DSP1.in_index  = 0;
							break;

						case 0x16:
						case 0x26:
						case 0x36:
						case 0x06: // Project object
							DSP1.Op06X = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op06Y = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));
							DSP1.Op06Z = (int16) (DSP1.parameters[4] | (DSP1.parameters[5] << 8));

							DSP1_Op06();

							DSP1.out_count = 6;
							DSP1.output[0] = (uint8)  (DSP1.Op06H       & 0xFF);
							DSP1.output[1] = (uint8) ((DSP1.Op06H >> 8) & 0xFF);
							DSP1.output[2] = (uint8)  (DSP1.Op06V       & 0xFF);
							DSP1.output[3] = (uint8) ((DSP1.Op06V >> 8) & 0xFF);
							DSP1.output[4] = (uint8)  (DSP1.Op06M       & 0xFF);
							DSP1.output[5] = (uint8) ((DSP1.Op06M >> 8) & 0xFF);
							break;

						case 0x1e:
						case 0x2e:
						case 0x3e:
						case 0x0e: // Target
							DSP1.Op0EH = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op0EV = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));

							DSP1_Op0E();

							DSP1.out_count = 4;
							DSP1.output[0] = (uint8)  (DSP1.Op0EX       & 0xFF);
							DSP1.output[1] = (uint8) ((DSP1.Op0EX >> 8) & 0xFF);
							DSP1.output[2] = (uint8)  (DSP1.Op0EY       & 0xFF);
							DSP1.output[3] = (uint8) ((DSP1.Op0EY >> 8) & 0xFF);
							break;

							// Extra commands used by Pilot Wings
						case 0x05:
						case 0x35:
						case 0x31:
						case 0x01: // Set attitude matrix A
							DSP1.Op01m  = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op01Zr = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));
							DSP1.Op01Yr = (int16) (DSP1.parameters[4] | (DSP1.parameters[5] << 8));
							DSP1.Op01Xr = (int16) (DSP1.parameters[6] | (DSP1.parameters[7] << 8));

							DSP1_Op01();
							break;

						case 0x15:
						case 0x11: // Set attitude matrix B
							DSP1.Op11m  = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op11Zr = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));
							DSP1.Op11Yr = (int16) (DSP1.parameters[4] | (DSP1.parameters[5] << 8));
							DSP1.Op11Xr = (int16) (DSP1.parameters[7] | (DSP1.parameters[7] << 8));

							DSP1_Op11();
							break;

						case 0x25:
						case 0x21: // Set attitude matrix C
							DSP1.Op21m  = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op21Zr = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));
							DSP1.Op21Yr = (int16) (DSP1.parameters[4] | (DSP1.parameters[5] << 8));
							DSP1.Op21Xr = (int16) (DSP1.parameters[6] | (DSP1.parameters[7] << 8));

							DSP1_Op21();
							break;

						case 0x09:
						case 0x39:
						case 0x3d:
						case 0x0d: // Objective matrix A
							DSP1.Op0DX = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op0DY = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));
							DSP1.Op0DZ = (int16) (DSP1.parameters[4] | (DSP1.parameters[5] << 8));

							DSP1_Op0D();

							DSP1.out_count = 6;
							DSP1.output [0] = (uint8)  (DSP1.Op0DF       & 0xFF);
							DSP1.output [1] = (uint8) ((DSP1.Op0DF >> 8) & 0xFF);
							DSP1.output [2] = (uint8)  (DSP1.Op0DL       & 0xFF);
							DSP1.output [3] = (uint8) ((DSP1.Op0DL >> 8) & 0xFF);
							DSP1.output [4] = (uint8)  (DSP1.Op0DU       & 0xFF);
							DSP1.output [5] = (uint8) ((DSP1.Op0DU >> 8) & 0xFF);
							break;

						case 0x19:
						case 0x1d: // Objective matrix B
							DSP1.Op1DX = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op1DY = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));
							DSP1.Op1DZ = (int16) (DSP1.parameters[4] | (DSP1.parameters[5] << 8));

							DSP1_Op1D();

							DSP1.out_count = 6;
							DSP1.output[0] = (uint8)  (DSP1.Op1DF       & 0xFF);
							DSP1.output[1] = (uint8) ((DSP1.Op1DF >> 8) & 0xFF);
							DSP1.output[2] = (uint8)  (DSP1.Op1DL       & 0xFF);
							DSP1.output[3] = (uint8) ((DSP1.Op1DL >> 8) & 0xFF);
							DSP1.output[4] = (uint8)  (DSP1.Op1DU       & 0xFF);
							DSP1.output[5] = (uint8) ((DSP1.Op1DU >> 8) & 0xFF);
							break;

						case 0x29:
						case 0x2d: // Objective matrix C
							DSP1.Op2DX = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op2DY = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));
							DSP1.Op2DZ = (int16) (DSP1.parameters[4] | (DSP1.parameters[5] << 8));

							DSP1_Op2D();

							DSP1.out_count = 6;
							DSP1.output[0] = (uint8)  (DSP1.Op2DF       & 0xFF);
							DSP1.output[1] = (uint8) ((DSP1.Op2DF >> 8) & 0xFF);
							DSP1.output[2] = (uint8)  (DSP1.Op2DL       & 0xFF);
							DSP1.output[3] = (uint8) ((DSP1.Op2DL >> 8) & 0xFF);
							DSP1.output[4] = (uint8)  (DSP1.Op2DU       & 0xFF);
							DSP1.output[5] = (uint8) ((DSP1.Op2DU >> 8) & 0xFF);
							break;

						case 0x33:
						case 0x03: // Subjective matrix A
							DSP1.Op03F = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op03L = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));
							DSP1.Op03U = (int16) (DSP1.parameters[4] | (DSP1.parameters[5] << 8));

							DSP1_Op03();

							DSP1.out_count = 6;
							DSP1.output[0] = (uint8)  (DSP1.Op03X       & 0xFF);
							DSP1.output[1] = (uint8) ((DSP1.Op03X >> 8) & 0xFF);
							DSP1.output[2] = (uint8)  (DSP1.Op03Y       & 0xFF);
							DSP1.output[3] = (uint8) ((DSP1.Op03Y >> 8) & 0xFF);
							DSP1.output[4] = (uint8)  (DSP1.Op03Z       & 0xFF);
							DSP1.output[5] = (uint8) ((DSP1.Op03Z >> 8) & 0xFF);
							break;

						case 0x13: // Subjective matrix B
							DSP1.Op13F = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op13L = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));
							DSP1.Op13U = (int16) (DSP1.parameters[4] | (DSP1.parameters[5] << 8));

							DSP1_Op13();

							DSP1.out_count = 6;
							DSP1.output[0] = (uint8)  (DSP1.Op13X       & 0xFF);
							DSP1.output[1] = (uint8) ((DSP1.Op13X >> 8) & 0xFF);
							DSP1.output[2] = (uint8)  (DSP1.Op13Y       & 0xFF);
							DSP1.output[3] = (uint8) ((DSP1.Op13Y >> 8) & 0xFF);
							DSP1.output[4] = (uint8)  (DSP1.Op13Z       & 0xFF);
							DSP1.output[5] = (uint8) ((DSP1.Op13Z >> 8) & 0xFF);
							break;

						case 0x23: // Subjective matrix C
							DSP1.Op23F = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op23L = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));
							DSP1.Op23U = (int16) (DSP1.parameters[4] | (DSP1.parameters[5] << 8));

							DSP1_Op23();

							DSP1.out_count = 6;
							DSP1.output[0] = (uint8)  (DSP1.Op23X       & 0xFF);
							DSP1.output[1] = (uint8) ((DSP1.Op23X >> 8) & 0xFF);
							DSP1.output[2] = (uint8)  (DSP1.Op23Y       & 0xFF);
							DSP1.output[3] = (uint8) ((DSP1.Op23Y >> 8) & 0xFF);
							DSP1.output[4] = (uint8)  (DSP1.Op23Z       & 0xFF);
							DSP1.output[5] = (uint8) ((DSP1.Op23Z >> 8) & 0xFF);
							break;

						case 0x3b:
						case 0x0b:
							DSP1.Op0BX = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op0BY = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));
							DSP1.Op0BZ = (int16) (DSP1.parameters[4] | (DSP1.parameters[5] << 8));

							DSP1_Op0B();

							DSP1.out_count = 2;
							DSP1.output[0] = (uint8)  (DSP1.Op0BS       & 0xFF);
							DSP1.output[1] = (uint8) ((DSP1.Op0BS >> 8) & 0xFF);
							break;

						case 0x1b:
							DSP1.Op1BX = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op1BY = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));
							DSP1.Op1BZ = (int16) (DSP1.parameters[4] | (DSP1.parameters[5] << 8));

							DSP1_Op1B();

							DSP1.out_count = 2;
							DSP1.output[0] = (uint8)  (DSP1.Op1BS       & 0xFF);
							DSP1.output[1] = (uint8) ((DSP1.Op1BS >> 8) & 0xFF);
							break;

						case 0x2b:
							DSP1.Op2BX = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));
							DSP1.Op2BY = (int16) (DSP1.parameters[2] | (DSP1.parameters[3] << 8));
							DSP1.Op2BZ = (int16) (DSP1.parameters[4] | (DSP1.parameters[5] << 8));

							DSP1_Op2B();

							DSP1.out_count = 2;
							DSP1.output[0] = (uint8)  (DSP1.Op2BS       & 0xFF);
							DSP1.output[1] = (uint8) ((DSP1.Op2BS >> 8) & 0xFF);
							break;

						case 0x34:
						case 0x14:
							DSP1.Op14Zr = (int16) (DSP1.parameters[ 0] | (DSP1.parameters[ 1] << 8));
							DSP1.Op14Xr = (int16) (DSP1.parameters[ 2] | (DSP1.parameters[ 3] << 8));
							DSP1.Op14Yr = (int16) (DSP1.parameters[ 4] | (DSP1.parameters[ 5] << 8));
							DSP1.Op14U  = (int16) (DSP1.parameters[ 6] | (DSP1.parameters[ 7] << 8));
							DSP1.Op14F  = (int16) (DSP1.parameters[ 8] | (DSP1.parameters[ 9] << 8));
							DSP1.Op14L  = (int16) (DSP1.parameters[10] | (DSP1.parameters[11] << 8));

							DSP1_Op14();

							DSP1.out_count = 6;
							DSP1.output[0] = (uint8)  (DSP1.Op14Zrr       & 0xFF);
							DSP1.output[1] = (uint8) ((DSP1.Op14Zrr >> 8) & 0xFF);
							DSP1.output[2] = (uint8)  (DSP1.Op14Xrr       & 0xFF);
							DSP1.output[3] = (uint8) ((DSP1.Op14Xrr >> 8) & 0xFF);
							DSP1.output[4] = (uint8)  (DSP1.Op14Yrr       & 0xFF);
							DSP1.output[5] = (uint8) ((DSP1.Op14Yrr >> 8) & 0xFF);
							break;

						case 0x27:
						case 0x2F:
							DSP1.Op2FUnknown = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));

							DSP1_Op2F();

							DSP1.out_count = 2;
							DSP1.output[0] = (uint8)  (DSP1.Op2FSize       & 0xFF);
							DSP1.output[1] = (uint8) ((DSP1.Op2FSize >> 8) & 0xFF);
							break;


						case 0x07:
						case 0x0F:
							DSP1.Op0FRamsize = (int16) (DSP1.parameters[0] | (DSP1.parameters[1] << 8));

							DSP1_Op0F();

							DSP1.out_count = 2;
							DSP1.output[0] = (uint8)  (DSP1.Op0FPass       & 0xFF);
							DSP1.output[1] = (uint8) ((DSP1.Op0FPass >> 8) & 0xFF);
							break;

						default:
							break;
					}
				}
			}
		}
	}
}

uint8 DSP1GetByte (uint16 address)
{
	uint8	t;

	if (address < DSP0.boundary)
	{
		if (DSP1.out_count)
		{
			t = (uint8) DSP1.output[DSP1.out_index];

			DSP1.out_index++;

			if (--DSP1.out_count == 0)
			{
				if (DSP1.command == 0x1a || DSP1.command == 0x0a)
				{
					DSP1_Op0A();
					DSP1.out_count = 8;
					DSP1.out_index = 0;
					DSP1.output[0] =  DSP1.Op0AA       & 0xFF;
					DSP1.output[1] = (DSP1.Op0AA >> 8) & 0xFF;
					DSP1.output[2] =  DSP1.Op0AB       & 0xFF;
					DSP1.output[3] = (DSP1.Op0AB >> 8) & 0xFF;
					DSP1.output[4] =  DSP1.Op0AC       & 0xFF;
					DSP1.output[5] = (DSP1.Op0AC >> 8) & 0xFF;
					DSP1.output[6] =  DSP1.Op0AD       & 0xFF;
					DSP1.output[7] = (DSP1.Op0AD >> 8) & 0xFF;
				}

				if (DSP1.command == 0x1f)
				{
					if ((DSP1.out_index % 2) != 0)
						t = (uint8) DSP1ROM[DSP1.out_index >> 1];
					else
						t = DSP1ROM[DSP1.out_index >> 1] >> 8;
				}
			}

			DSP1.waiting4command = TRUE;
		}
		else
			t = 0xff;
	}
	else
		t = 0x80;

	return (t);
}
