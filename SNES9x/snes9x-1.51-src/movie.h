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



//  Input recording/playback code
//  (c) Copyright 2004 blip

#ifndef _MOVIE_H_
#define _MOVIE_H_

#include <stdio.h>
#include <time.h>
#include "snes9x.h"

#ifndef SUCCESS
#  define SUCCESS 1
#  define WRONG_FORMAT (-1)
#  define WRONG_VERSION (-2)
#  define FILE_NOT_FOUND (-3)
#endif

#define MOVIE_OPT_FROM_SNAPSHOT 0
#define MOVIE_OPT_FROM_RESET	(1<<0)
#define MOVIE_OPT_PAL           (1<<1)
#define MOVIE_OPT_NOSAVEDATA    (1<<2)
#define MOVIE_MAX_METADATA		512

#define MOVIE_SYNC_DATA_EXISTS   0x01
#define MOVIE_SYNC_OBSOLETE      0x02
#define MOVIE_SYNC_LEFTRIGHT     0x04
#define MOVIE_SYNC_VOLUMEENVX    0x08
#define MOVIE_SYNC_FAKEMUTE      0x10
#define MOVIE_SYNC_SYNCSOUND     0x20
#define MOVIE_SYNC_HASROMINFO    0x40
#define MOVIE_SYNC_NOCPUSHUTDOWN 0x80

START_EXTERN_C
struct MovieInfo
{
	time_t	TimeCreated;
	uint32	LengthFrames;
	uint32	RerecordCount;
	wchar_t	Metadata[MOVIE_MAX_METADATA];		// really should be wchar_t
	uint8	Opts;
	uint8	ControllersMask;
	bool8	ReadOnly;
	uint8	SyncFlags;

	uint32	ROMCRC32;
	char	ROMName [23];

	uint32	LengthSamples;
	uint8	PortType[2];
	uint32	Version;
};

// methods used by the user-interface code
int S9xMovieOpen (const char* filename, bool8 read_only);
int S9xMovieCreate (const char* filename, uint8 controllers_mask, uint8 opts, const wchar_t* metadata, int metadata_length);
int S9xMovieGetInfo (const char* filename, struct MovieInfo* info);
void S9xMovieStop (bool8 suppress_message);
void S9xMovieToggleRecState ();
void S9xMovieToggleFrameDisplay ();
const char *S9xChooseMovieFilename(bool8 read_only);

// methods used by the emulation
void S9xMovieInit ();
void S9xMovieShutdown ();
void S9xMovieUpdate (bool addFrame=true);
void S9xMovieUpdateOnReset ();
//bool8 S9xMovieRewind (uint32 at_frame);
void S9xMovieFreeze (uint8** buf, uint32* size);
int S9xMovieUnfreeze (const uint8* buf, uint32 size);
void S9xUpdateFrameCounter (int offset=0);

// accessor functions
bool8 S9xMovieActive ();
bool8 S9xMoviePlaying ();
bool8 S9xMovieRecording ();
// the following accessors return 0/false if !S9xMovieActive()
bool8 S9xMovieReadOnly ();
uint32 S9xMovieGetId ();
uint32 S9xMovieGetLength ();
uint32 S9xMovieGetFrameCounter ();
uint8 S9xMovieControllers ();

END_EXTERN_C

#endif
