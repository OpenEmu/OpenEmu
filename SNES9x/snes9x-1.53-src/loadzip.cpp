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


#ifdef UNZIP_SUPPORT

#include <assert.h>
#include <ctype.h>
#include "unzip/unzip.h"
#include "snes9x.h"
#include "memmap.h"


bool8 LoadZip (const char *zipname, int32 *TotalFileSize, int32 *headers, uint8 *buffer)
{
	*TotalFileSize = 0;
	*headers = 0;

	unzFile	file = unzOpen(zipname);
	if (file == NULL)
		return (FALSE);

	// find largest file in zip file (under MAX_ROM_SIZE) or a file with extension .1
	char	filename[132];
	int		filesize = 0;
	int		port = unzGoToFirstFile(file);

	unz_file_info	info;

	while (port == UNZ_OK)
	{
		char	name[132];
		unzGetCurrentFileInfo(file, &info, name, 128, NULL, 0, NULL, 0);

		if (info.uncompressed_size > CMemory::MAX_ROM_SIZE + 512)
		{
			port = unzGoToNextFile(file);
			continue;
		}

		if ((int) info.uncompressed_size > filesize)
		{
			strcpy(filename, name);
			filesize = info.uncompressed_size;
		}

		int	len = strlen(name);
		if (len > 2 && name[len - 2] == '.' && name[len - 1] == '1')
		{
			strcpy(filename, name);
			filesize = info.uncompressed_size;
			break;
		}

		port = unzGoToNextFile(file);
	}

	if (!(port == UNZ_END_OF_LIST_OF_FILE || port == UNZ_OK) || filesize == 0)
	{
		assert(unzClose(file) == UNZ_OK);
		return (FALSE);
	}

	// find extension
	char	tmp[2] = { 0, 0 };
	char	*ext = strrchr(filename, '.');
	if (ext)
		ext++;
	else
		ext = tmp;

	uint8	*ptr = buffer;
	bool8	more = FALSE;

	unzLocateFile(file, filename, 1);
	unzGetCurrentFileInfo(file, &info, filename, 128, NULL, 0, NULL, 0);

	if (unzOpenCurrentFile(file) != UNZ_OK)
	{
		unzClose(file);
		return (FALSE);
	}

	do
	{
		assert(info.uncompressed_size <= CMemory::MAX_ROM_SIZE + 512);

		int	FileSize = info.uncompressed_size;
		int	l = unzReadCurrentFile(file, ptr, FileSize);

		if (unzCloseCurrentFile(file) == UNZ_CRCERROR)
		{
			unzClose(file);
			return (FALSE);
		}

		if (l <= 0 || l != FileSize)
		{
			unzClose(file);
			return (FALSE);
		}

		FileSize = (int) Memory.HeaderRemove((uint32) FileSize, *headers, ptr);
		ptr += FileSize;
		*TotalFileSize += FileSize;

		int	len;

		if (ptr - Memory.ROM < CMemory::MAX_ROM_SIZE + 512 && (isdigit(ext[0]) && ext[1] == 0 && ext[0] < '9'))
		{
			more = TRUE;
			ext[0]++;
		}
		else
		if (ptr - Memory.ROM < CMemory::MAX_ROM_SIZE + 512)
		{
			if (ext == tmp)
				len = strlen(filename);
			else
				len = ext - filename - 1;

			if ((len == 7 || len == 8) && strncasecmp(filename, "sf", 2) == 0 &&
				isdigit(filename[2]) && isdigit(filename[3]) && isdigit(filename[4]) &&
				isdigit(filename[5]) && isalpha(filename[len - 1]))
			{
				more = TRUE;
				filename[len - 1]++;
			}
		}
		else
			more = FALSE;

		if (more)
		{
			if (unzLocateFile(file, filename, 1) != UNZ_OK ||
				unzGetCurrentFileInfo(file, &info, filename, 128, NULL, 0, NULL, 0) != UNZ_OK ||
				unzOpenCurrentFile(file) != UNZ_OK)
				break;
		}
	} while (more);

	unzClose(file);

	return (TRUE);
}

#endif
