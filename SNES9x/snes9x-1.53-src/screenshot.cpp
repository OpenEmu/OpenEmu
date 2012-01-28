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


#ifdef HAVE_LIBPNG
#include <png.h>
#endif
#include "snes9x.h"
#include "memmap.h"
#include "display.h"
#include "screenshot.h"


bool8 S9xDoScreenshot (int width, int height)
{
	Settings.TakeScreenshot = FALSE;

#ifdef HAVE_LIBPNG
	FILE		*fp;
	png_structp	png_ptr;
	png_infop	info_ptr;
	png_color_8	sig_bit;
	int			imgwidth, imgheight;
	const char	*fname;

	fname = S9xGetFilenameInc(".png", SCREENSHOT_DIR);

	fp = fopen(fname, "wb");
	if (!fp)
	{
		S9xMessage(S9X_ERROR, 0, "Failed to take screenshot.");
		return (FALSE);
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		fclose(fp);
		remove(fname);
		S9xMessage(S9X_ERROR, 0, "Failed to take screenshot.");
		return (FALSE);
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
		fclose(fp);
		remove(fname);
		S9xMessage(S9X_ERROR, 0, "Failed to take screenshot.");
		return (FALSE);
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		remove(fname);
		S9xMessage(S9X_ERROR, 0, "Failed to take screenshot.");
		return (FALSE);
	}

	imgwidth  = width;
	imgheight = height;

	if (Settings.StretchScreenshots == 1)
	{
		if (width > SNES_WIDTH && height <= SNES_HEIGHT_EXTENDED)
			imgheight = height << 1;
	}
	else
	if (Settings.StretchScreenshots == 2)
	{
		if (width  <= SNES_WIDTH)
			imgwidth  = width  << 1;
		if (height <= SNES_HEIGHT_EXTENDED)
			imgheight = height << 1;
	}

	png_init_io(png_ptr, fp);

	png_set_IHDR(png_ptr, info_ptr, imgwidth, imgheight, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	sig_bit.red   = 5;
	sig_bit.green = 5;
	sig_bit.blue  = 5;
	png_set_sBIT(png_ptr, info_ptr, &sig_bit);
	png_set_shift(png_ptr, &sig_bit);

	png_write_info(png_ptr, info_ptr);

	png_set_packing(png_ptr);

	png_byte	*row_pointer = new png_byte[png_get_rowbytes(png_ptr, info_ptr)];
	uint16		*screen = GFX.Screen;

	for (int y = 0; y < height; y++, screen += GFX.RealPPL)
	{
		png_byte	*rowpix = row_pointer;

		for (int x = 0; x < width; x++)
		{
			uint32	r, g, b;

			DECOMPOSE_PIXEL(screen[x], r, g, b);

			*(rowpix++) = r;
			*(rowpix++) = g;
			*(rowpix++) = b;

			if (imgwidth != width)
			{
				*(rowpix++) = r;
				*(rowpix++) = g;
				*(rowpix++) = b;
			}
		}

		png_write_row(png_ptr, row_pointer);
		if (imgheight != height)
			png_write_row(png_ptr, row_pointer);
	}

	delete [] row_pointer;

	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(fp);

	fprintf(stderr, "%s saved.\n", fname);

	const char	*base = S9xBasename(fname);
	sprintf(String, "Saved screenshot %s", base);
	S9xMessage(S9X_INFO, 0, String);

	return (TRUE);
#else
	fprintf(stderr, "Screenshot support not available (libpng was not found at build time).\n");
	return (FALSE);
#endif
}
