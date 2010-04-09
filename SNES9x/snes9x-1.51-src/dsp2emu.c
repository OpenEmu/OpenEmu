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





uint16 DSP2Op09Word1=0;
uint16 DSP2Op09Word2=0;
bool DSP2Op05HasLen=false;
int DSP2Op05Len=0;
bool DSP2Op06HasLen=false;
int DSP2Op06Len=0;
uint8 DSP2Op05Transparent=0;

void DSP2_Op05 ()
{
	uint8 color;
	// Overlay bitmap with transparency.
	// Input:
	//
	//   Bitmap 1:  i[0] <=> i[size-1]
	//   Bitmap 2:  i[size] <=> i[2*size-1]
	//
	// Output:
	//
	//   Bitmap 3:  o[0] <=> o[size-1]
	//
	// Processing:
	//
	//   Process all 4-bit pixels (nibbles) in the bitmap
	//
	//   if ( BM2_pixel == transparent_color )
	//      pixelout = BM1_pixel
	//   else
	//      pixelout = BM2_pixel

	// The max size bitmap is limited to 255 because the size parameter is a byte
	// I think size=0 is an error.  The behavior of the chip on size=0 is to
	// return the last value written to DR if you read DR on Op05 with
	// size = 0.  I don't think it's worth implementing this quirk unless it's
	// proven necessary.

	int n;
	unsigned char c1;
	unsigned char c2;
	unsigned char *p1 = DSP1.parameters;
	unsigned char *p2 = &DSP1.parameters[DSP2Op05Len];
	unsigned char *p3 = DSP1.output;

	color = DSP2Op05Transparent&0x0f;

	for( n = 0; n < DSP2Op05Len; n++ )
	{
		c1 = *p1++;
		c2 = *p2++;
		*p3++ = ( ((c2 >> 4) == color ) ? c1 & 0xf0: c2 & 0xf0 ) |
		        ( ((c2 & 0x0f)==color) ? c1 & 0x0f: c2 & 0x0f );
	}
}

void DSP2_Op01 ()
{
	// Op01 size is always 32 bytes input and output.
	// The hardware does strange things if you vary the size.

	int j;
	unsigned char c0, c1, c2, c3;
	unsigned char *p1 = DSP1.parameters;
	unsigned char *p2a = DSP1.output;
	unsigned char *p2b = &DSP1.output[16];	// halfway

	// Process 8 blocks of 4 bytes each

	for ( j = 0; j < 8; j++ )
	{
		c0 = *p1++;
		c1 = *p1++;
		c2 = *p1++;
		c3 = *p1++;

		*p2a++ = (c0 & 0x10) << 3 |
			     (c0 & 0x01) << 6 |
			     (c1 & 0x10) << 1 |
			     (c1 & 0x01) << 4 |
			     (c2 & 0x10) >> 1 |
			     (c2 & 0x01) << 2 |
			     (c3 & 0x10) >> 3 |
			     (c3 & 0x01);

		*p2a++ = (c0 & 0x20) << 2 |
			     (c0 & 0x02) << 5 |
			     (c1 & 0x20)      |
			     (c1 & 0x02) << 3 |
			     (c2 & 0x20) >> 2 |
			     (c2 & 0x02) << 1 |
			     (c3 & 0x20) >> 4 |
			     (c3 & 0x02) >> 1;

		*p2b++ = (c0 & 0x40) << 1 |
			     (c0 & 0x04) << 4 |
			     (c1 & 0x40) >> 1 |
			     (c1 & 0x04) << 2 |
			     (c2 & 0x40) >> 3 |
			     (c2 & 0x04)      |
			     (c3 & 0x40) >> 5 |
			     (c3 & 0x04) >> 2;


		*p2b++ = (c0 & 0x80)      |
			     (c0 & 0x08) << 3 |
			     (c1 & 0x80) >> 2 |
			     (c1 & 0x08) << 1 |
			     (c2 & 0x80) >> 4 |
			     (c2 & 0x08) >> 1 |
			     (c3 & 0x80) >> 6 |
			     (c3 & 0x08) >> 3;
	}
	return;
}

void DSP2_Op06 ()
{
	// Input:
	//    size
	//    bitmap

	int	i, j;

	for ( i = 0, j = DSP2Op06Len - 1; i < DSP2Op06Len; i++, j-- )
	{
		DSP1.output[j] = (DSP1.parameters[i] << 4) | (DSP1.parameters[i] >> 4);
	}
}

bool DSP2Op0DHasLen=false;
int DSP2Op0DOutLen=0;
int DSP2Op0DInLen=0;

#ifndef DSP2_BIT_ACCURRATE_CODE

// Scale bitmap based on input length out output length

void DSP2_Op0D()
{
	// Overload's algorithm - use this unless doing hardware testing

	// One note:  the HW can do odd byte scaling but since we divide
	// by two to get the count of bytes this won't work well for
	// odd byte scaling (in any of the current algorithm implementations).
	// So far I haven't seen Dungeon Master use it.
	// If it does we can adjust the parameters and code to work with it

	int i;
	int pixel_offset;
	uint8 pixelarray[512];

	for(i=0; i<DSP2Op0DOutLen*2; i++)
	{
		pixel_offset = (i * DSP2Op0DInLen) / DSP2Op0DOutLen;
		if ( (pixel_offset&1) == 0 )
			pixelarray[i] = DSP1.parameters[pixel_offset>>1] >> 4;
		else
			pixelarray[i] = DSP1.parameters[pixel_offset>>1] & 0x0f;
	}

	for ( i=0; i < DSP2Op0DOutLen; i++ )
		DSP1.output[i] = ( pixelarray[i<<1] << 4 ) | pixelarray[(i<<1)+1];
}

#else

void DSP2_Op0D()
{
	// Bit accurate hardware algorithm - uses fixed point math
	// This should match the DSP2 Op0D output exactly
	// I wouldn't recommend using this unless you're doing hardware debug.
	// In some situations it has small visual artifacts that
	// are not readily apparent on a TV screen but show up clearly
	// on a monitor.  Use Overload's scaling instead.
	// This is for hardware verification testing.
	//
	// One note:  the HW can do odd byte scaling but since we divide
	// by two to get the count of bytes this won't work well for
	// odd byte scaling (in any of the current algorithm implementations).
	// So far I haven't seen Dungeon Master use it.
	// If it does we can adjust the parameters and code to work with it


	uint32 multiplier;	// Any size int >= 32-bits
	uint32 pixloc;		// match size of multiplier
	int	i, j;
	uint8 pixelarray[512];

	if (DSP2Op0DInLen <= DSP2Op0DOutLen)
		multiplier = 0x10000;	// In our self defined fixed point 0x10000 == 1
	else
		multiplier = (DSP2Op0DInLen << 17) / ((DSP2Op0DOutLen<<1) + 1);

	pixloc = 0;
	for ( i=0; i < DSP2Op0DOutLen * 2; i++ )
	{
		j = pixloc >> 16;

		if ( j & 1 )
			pixelarray[i] = DSP1.parameters[j>>1] & 0x0f;
		else
			pixelarray[i] = (DSP1.parameters[j>>1] & 0xf0) >> 4;

		pixloc += multiplier;
	}

	for ( i=0; i < DSP2Op0DOutLen; i++ )
		DSP1.output[i] = ( pixelarray[i<<1] << 4 ) | pixelarray[(i<<1)+1];
}

#endif

#if 0	// Probably no reason to use this code - it's not quite bit accurate and it doesn't look as good as Overload's algorithm

void DSP2_Op0D()
{
	// Float implementation of Neviksti's algorithm
	// This is the right algorithm to match the DSP2 bits but the precision
	// of the PC float does not match the precision of the fixed point math
	// on the DSP2 causing occasional one off data mismatches (which should
	// be no problem because its just a one pixel difference in a scaled image
	// to be displayed).

	float multiplier;
	float pixloc;
	int	i, j;
	uint8 pixelarray[512];

	if (DSP2Op0DInLen <= DSP2Op0DOutLen)
		multiplier = (float) 1.0;
	else
		multiplier = (float) ((DSP2Op0DInLen * 2.0) / (DSP2Op0DOutLen * 2.0 + 1.0));

	pixloc = 0.0;
	for ( i=0; i < DSP2Op0DOutLen * 2; i++ )
	{
		// j = (int)(i * multiplier);
		j = (int) pixloc;

		if ( j & 1 )
			pixelarray[i] = DSP1.parameters[j>>1] & 0x0f;
		else
			pixelarray[i] = (DSP1.parameters[j>>1] & 0xf0) >> 4;

		pixloc += multiplier;	// use an add in the loop instead of multiply to increase loop speed
	}

	for ( i=0; i < DSP2Op0DOutLen; i++ )
		DSP1.output[i] = ( pixelarray[i<<1] << 4 ) | pixelarray[(i<<1)+1];
}

#endif

