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



#ifndef CROSSHAIRS_H
#define CROSSHAIRS_H

/*
 * Read in the specified crosshair file, replacing whatever data might be in
 * that slot. Available slots are 1-31. The input file must be a PNG file,
 * 15x15 pixels, palettized, with 3 colors: white, black, and transparent. Or a
 * text file, 15 lines of 16 characters (counting the \n), consisting of ' ',
 * '#', or '.'.
 */
bool S9xLoadCrosshairFile(int idx, const char *filename);

/*
 * Return the specified crosshair. Woo-hoo. char * to a 225-byte string, with
 * '#' marking foreground, '.' marking background, and anything else
 * transparent.
 */
const char *S9xGetCrosshair(int idx);

/*
 * In controls.cpp. Sets the crosshair for the specified device. Defaults are:
 *              cross   fgcolor   bgcolor
 *   Mouse 1:     1     White     Black
 *   Mouse 2:     1     Purple    White
 *   Superscope:  2     White     Black
 *   Justifier 1: 4     Blue      Black
 *   Justifier 2: 4     MagicPink Black
 *
 * Available colors are: Trans, Black, 25Grey, 50Grey, 75Grey, White, Red,
 * Orange, Yellow, Green, Cyan, Sky, Blue, Violet, MagicPink, and Purple. You
 * may also prefix a 't' (e.g. tBlue) for a 50%-transparent version.
 *
 * Use idx=-1 or fg/bg=NULL to keep the current setting.
 */
enum crosscontrols {
    X_MOUSE1,
    X_MOUSE2,
    X_SUPERSCOPE,
    X_JUSTIFIER1,
    X_JUSTIFIER2
};
void S9xSetControllerCrosshair(enum crosscontrols ctl, int8 idx, const char *fg, const char *bg);
void S9xGetControllerCrosshair(enum crosscontrols ctl, int8 *idx, const char **fg, const char **bg);

/*
 * In gfx.cpp, much like DisplayChar() except it takes the parameters listed
 * and looks up GFX.Screen. The 'crosshair' arg is a 15x15 image, with '#'
 * meaning fgcolor, '.' meaning bgcolor, and anything else meaning transparent.
 * Color values should be (RGB):
 *  0 = transparent   4=23 23 23   8=31 31  0  12= 0 0 31
 *  1 =  0  0  0      5=31 31 31   9= 0 31  0  13=23 0 31
 *  2 =  8  8  8      6=31 00 00  10= 0 31 31  14=31 0 31
 *  3 = 16 16 16      7=31 16 00  11= 0 23 31  15=31 0 16
 *  16-31 are 50% transparent versions of 0-15.
 */
void S9xDrawCrosshair(const char *crosshair, uint8 fgcolor, uint8 bgcolor, int16 x, int16 y);

#endif
