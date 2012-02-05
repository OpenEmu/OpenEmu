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

//  Input recording/playback code
//  (c) Copyright 2004 blip

#ifndef __WIN32__
#include <unistd.h>
#endif
#include "snes9x.h"
#include "memmap.h"
#include "controls.h"
#include "snapshot.h"
#include "movie.h"
#include "language.h"
#ifdef NETPLAY_SUPPORT
#include "netplay.h"
#endif

#ifdef __WIN32__
#include <io.h>
#ifndef W_OK
#define W_OK 2
#endif
#define ftruncate chsize
#endif

#define SMV_MAGIC				0x1a564d53 // SMV0x1a
#define SMV_VERSION				5
#define SMV_HEADER_SIZE			64
#define SMV_EXTRAROMINFO_SIZE	30
#define BUFFER_GROWTH_SIZE		4096

enum MovieState
{
	MOVIE_STATE_NONE = 0,
	MOVIE_STATE_PLAY,
	MOVIE_STATE_RECORD
};

struct SMovie
{
	enum MovieState	State;

	FILE	*File;
	char	Filename[PATH_MAX + 1];
	char	ROMName[23];
	uint32	ROMCRC32;
	uint32	MovieId;
	uint32	Version;

	uint32	SaveStateOffset;
	uint32	ControllerDataOffset;

	uint8	ControllersMask;
	uint8	Opts;
	uint8	SyncFlags;
	uint32	MaxFrame;
	uint32	MaxSample;
	uint32	CurrentFrame;
	uint32	CurrentSample;
	uint32	BytesPerSample;
	uint32	RerecordCount;
	bool8	ReadOnly;
	uint8	PortType[2];
	int8	PortIDs[2][4];

	uint8	*InputBuffer;
	uint8	*InputBufferPtr;
	uint32	InputBufferSize;
};

static struct SMovie	Movie;

static uint8	prevPortType[2];
static int8		prevPortIDs[2][4];
static bool8	prevMouseMaster, prevSuperScopeMaster, prevJustifierMaster, prevMultiPlayer5Master;

static uint8	Read8 (uint8 *&);
static uint16	Read16 (uint8 *&);
static uint32	Read32 (uint8 *&);
static void		Write8 (uint8, uint8 *&);
static void		Write16 (uint16, uint8 *&);
static void		Write32 (uint32, uint8 *&);
static void		store_previous_settings (void);
static void		restore_previous_settings (void);
static void		store_movie_settings (void);
static void		restore_movie_settings (void);
static int		bytes_per_sample (void);
static void		reserve_buffer_space (uint32);
static void		reset_controllers (void);
static void		read_frame_controller_data (bool);
static void		write_frame_controller_data (void);
static void		flush_movie (void);
static void		truncate_movie (void);
static int		read_movie_header (FILE *, SMovie *);
static int		read_movie_extrarominfo (FILE *, SMovie *);
static void		write_movie_header (FILE *, SMovie *);
static void		write_movie_extrarominfo (FILE *, SMovie *);
static void		change_state (MovieState);

// HACK: reduce movie size by not storing changes that can only affect polled input in the movie for these types,
//       because currently no port sets these types to polling
#define SKIPPED_POLLING_PORT_TYPE(x)	(((x) == CTL_NONE) || ((x) == CTL_JOYPAD) || ((x) == CTL_MP5))

#ifndef max
#define max(a, b)	(((a) > (b)) ? (a) : (b))
#endif


static uint8 Read8 (uint8 *&ptr)
{
	uint8	v = *ptr++;
	return (v);
}

static uint16 Read16 (uint8 *&ptr)
{
	uint16	v = READ_WORD(ptr);
	ptr += 2;
	return (v);
}

static uint32 Read32 (uint8 *&ptr)
{
	uint32	v = READ_DWORD(ptr);
	ptr += 4;
	return (v);
}

static void Write8 (uint8 v, uint8 *&ptr)
{
	*ptr++ = v;
}

static void Write16 (uint16 v, uint8 *&ptr)
{
	WRITE_WORD(ptr, v);
	ptr += 2;
}

static void Write32 (uint32 v, uint8 *&ptr)
{
	WRITE_DWORD(ptr, v);
	ptr += 4;
}

static void store_previous_settings (void)
{
	for (int i = 0; i < 2; i++)
	{
		enum controllers pt;
		S9xGetController(i, &pt, &prevPortIDs[i][0], &prevPortIDs[i][1], &prevPortIDs[i][2], &prevPortIDs[i][3]);
		prevPortType[i] = (uint8) pt;
	}

	prevMouseMaster        = Settings.MouseMaster;
	prevSuperScopeMaster   = Settings.SuperScopeMaster;
	prevJustifierMaster    = Settings.JustifierMaster;
	prevMultiPlayer5Master = Settings.MultiPlayer5Master;
}

static void restore_previous_settings (void)
{
	Settings.MouseMaster        = prevMouseMaster;
	Settings.SuperScopeMaster   = prevSuperScopeMaster;
	Settings.JustifierMaster    = prevJustifierMaster;
	Settings.MultiPlayer5Master = prevMultiPlayer5Master;

	S9xSetController(0, (enum controllers) prevPortType[0], prevPortIDs[0][0], prevPortIDs[0][1], prevPortIDs[0][2], prevPortIDs[0][3]);
	S9xSetController(1, (enum controllers) prevPortType[1], prevPortIDs[1][0], prevPortIDs[1][1], prevPortIDs[1][2], prevPortIDs[1][3]);
}

static void store_movie_settings (void)
{
	for (int i = 0; i < 2; i++)
	{
		enum controllers pt;
		S9xGetController(i, &pt, &Movie.PortIDs[i][0], &Movie.PortIDs[i][1], &Movie.PortIDs[i][2], &Movie.PortIDs[i][3]);
		Movie.PortType[i] = (uint8) pt;
	}
}

static void restore_movie_settings (void)
{
	Settings.MouseMaster        = (Movie.PortType[0] == CTL_MOUSE      || Movie.PortType[1] == CTL_MOUSE);
	Settings.SuperScopeMaster   = (Movie.PortType[0] == CTL_SUPERSCOPE || Movie.PortType[1] == CTL_SUPERSCOPE);
	Settings.JustifierMaster    = (Movie.PortType[0] == CTL_JUSTIFIER  || Movie.PortType[1] == CTL_JUSTIFIER);
	Settings.MultiPlayer5Master = (Movie.PortType[0] == CTL_MP5        || Movie.PortType[1] == CTL_MP5);

	S9xSetController(0, (enum controllers) Movie.PortType[0], Movie.PortIDs[0][0], Movie.PortIDs[0][1], Movie.PortIDs[0][2], Movie.PortIDs[0][3]);
	S9xSetController(1, (enum controllers) Movie.PortType[1], Movie.PortIDs[1][0], Movie.PortIDs[1][1], Movie.PortIDs[1][2], Movie.PortIDs[1][3]);
}

static int bytes_per_sample (void)
{
	int	num_controllers = 0;

	for (int i = 0; i < 8; i++)
	{
		if (Movie.ControllersMask & (1 << i))
			num_controllers++;
	}

	int	bytes = CONTROLLER_DATA_SIZE * num_controllers;

	for (int p = 0; p < 2; p++)
	{
		if (Movie.PortType[p] == CTL_MOUSE)
			bytes += MOUSE_DATA_SIZE;
		else
		if (Movie.PortType[p] == CTL_SUPERSCOPE)
			bytes += SCOPE_DATA_SIZE;
		else
		if (Movie.PortType[p] == CTL_JUSTIFIER)
			bytes += JUSTIFIER_DATA_SIZE;
	}

	return (bytes);
}

static void reserve_buffer_space (uint32 space_needed)
{
	if (space_needed > Movie.InputBufferSize)
	{
		uint32 ptr_offset   = Movie.InputBufferPtr - Movie.InputBuffer;
		uint32 alloc_chunks = space_needed / BUFFER_GROWTH_SIZE;

		Movie.InputBufferSize = BUFFER_GROWTH_SIZE * (alloc_chunks + 1);
		Movie.InputBuffer     = (uint8 *) realloc(Movie.InputBuffer, Movie.InputBufferSize);
		Movie.InputBufferPtr  = Movie.InputBuffer + ptr_offset;
	}
}

static void reset_controllers (void)
{
	for (int i = 0; i < 8; i++)
		MovieSetJoypad(i, 0);

	uint8 clearedMouse[MOUSE_DATA_SIZE];
	ZeroMemory(clearedMouse, MOUSE_DATA_SIZE);
	clearedMouse[4] = 1;

	uint8 clearedScope[SCOPE_DATA_SIZE];
	ZeroMemory(clearedScope, SCOPE_DATA_SIZE);

	uint8 clearedJustifier[JUSTIFIER_DATA_SIZE];
	ZeroMemory(clearedJustifier, JUSTIFIER_DATA_SIZE);

	for (int p = 0; p < 2; p++)
	{
		MovieSetMouse(p, clearedMouse, true);
		MovieSetScope(p, clearedScope);
		MovieSetJustifier(p, clearedJustifier);
	}
}

static void read_frame_controller_data (bool addFrame)
{
	// reset code check
	if (Movie.InputBufferPtr[0] == 0xff)
	{
		bool reset = true;
		for (int i = 1; i < (int) Movie.BytesPerSample; i++)
		{
			if (Movie.InputBufferPtr[i] != 0xff)
			{
				reset = false;
				break;
			}
		}

		if (reset)
		{
			Movie.InputBufferPtr += Movie.BytesPerSample;
			S9xSoftReset();
			return;
		}
	}

	for (int i = 0; i < 8; i++)
	{
		if (Movie.ControllersMask & (1 << i))
			MovieSetJoypad(i, Read16(Movie.InputBufferPtr));
		else
			MovieSetJoypad(i, 0); // pretend the controller is disconnected
	}

	for (int p = 0; p < 2; p++)
	{
		if (Movie.PortType[p] == CTL_MOUSE)
		{
			uint8 buf[MOUSE_DATA_SIZE];
			memcpy(buf, Movie.InputBufferPtr, MOUSE_DATA_SIZE);
			Movie.InputBufferPtr += MOUSE_DATA_SIZE;
			MovieSetMouse(p, buf, !addFrame);
		}
		else
		if (Movie.PortType[p] == CTL_SUPERSCOPE)
		{
			uint8 buf[SCOPE_DATA_SIZE];
			memcpy(buf, Movie.InputBufferPtr, SCOPE_DATA_SIZE);
			Movie.InputBufferPtr += SCOPE_DATA_SIZE;
			MovieSetScope(p, buf);
		}
		else
		if (Movie.PortType[p] == CTL_JUSTIFIER)
		{
			uint8 buf[JUSTIFIER_DATA_SIZE];
			memcpy(buf, Movie.InputBufferPtr, JUSTIFIER_DATA_SIZE);
			Movie.InputBufferPtr += JUSTIFIER_DATA_SIZE;
			MovieSetJustifier(p, buf);
		}
	}
}

static void write_frame_controller_data (void)
{
	reserve_buffer_space((uint32) (Movie.InputBufferPtr + Movie.BytesPerSample - Movie.InputBuffer));

	for (int i = 0; i < 8; i++)
	{
		if (Movie.ControllersMask & (1 << i))
			Write16(MovieGetJoypad(i), Movie.InputBufferPtr);
		else
			MovieSetJoypad(i, 0); // pretend the controller is disconnected
	}

	for (int p = 0; p < 2; p++)
	{
		if (Movie.PortType[p] == CTL_MOUSE)
		{
			uint8 buf[MOUSE_DATA_SIZE];
			MovieGetMouse(p, buf);
			memcpy(Movie.InputBufferPtr, buf, MOUSE_DATA_SIZE);
			Movie.InputBufferPtr += MOUSE_DATA_SIZE;
		}
		else
		if (Movie.PortType[p] == CTL_SUPERSCOPE)
		{
			uint8 buf[SCOPE_DATA_SIZE];
			MovieGetScope(p, buf);
			memcpy(Movie.InputBufferPtr, buf, SCOPE_DATA_SIZE);
			Movie.InputBufferPtr += SCOPE_DATA_SIZE;
		}
		else
		if (Movie.PortType[p] == CTL_JUSTIFIER)
		{
			uint8 buf[JUSTIFIER_DATA_SIZE];
			MovieGetJustifier(p, buf);
			memcpy(Movie.InputBufferPtr, buf, JUSTIFIER_DATA_SIZE);
			Movie.InputBufferPtr += JUSTIFIER_DATA_SIZE;
		}
	}
}

static void flush_movie (void)
{
	if (!Movie.File)
		return;

	fseek(Movie.File, 0, SEEK_SET);
	write_movie_header(Movie.File, &Movie);
	fseek(Movie.File, Movie.ControllerDataOffset, SEEK_SET);

	size_t	ignore;
	ignore = fwrite(Movie.InputBuffer, 1, Movie.BytesPerSample * (Movie.MaxSample + 1), Movie.File);
}

static void truncate_movie (void)
{
	if (!Movie.File || !Settings.MovieTruncate)
		return;

	if (Movie.SaveStateOffset > Movie.ControllerDataOffset)
		return;

	int	ignore;
	ignore = ftruncate(fileno(Movie.File), Movie.ControllerDataOffset + Movie.BytesPerSample * (Movie.MaxSample + 1));
}

static int read_movie_header (FILE *fd, SMovie *movie)
{
	uint32	value;
	uint8	buf[SMV_HEADER_SIZE], *ptr = buf;

	if (fread(buf, 1, SMV_HEADER_SIZE, fd) != SMV_HEADER_SIZE)
		return (WRONG_FORMAT);

	value = Read32(ptr);
	if (value != SMV_MAGIC)
		return (WRONG_FORMAT);

	value = Read32(ptr);
	if(value > SMV_VERSION || value < 4)
		return (WRONG_VERSION);

	movie->Version              = value;
	movie->MovieId              = Read32(ptr);
	movie->RerecordCount        = Read32(ptr);
	movie->MaxFrame             = Read32(ptr);
	movie->ControllersMask      = Read8(ptr);
	movie->Opts                 = Read8(ptr);
	ptr++;
	movie->SyncFlags            = Read8(ptr);
	movie->SaveStateOffset      = Read32(ptr);
	movie->ControllerDataOffset = Read32(ptr);
	movie->MaxSample            = Read32(ptr);
	movie->PortType[0]          = Read8(ptr);
	movie->PortType[1]          = Read8(ptr);
	for (int p = 0; p < 2; p++)
	{
		for (int i = 0; i < 4; i++)
			movie->PortIDs[p][i] = Read8(ptr);
	}

	if (movie->MaxSample < movie->MaxFrame)
		movie->MaxSample = movie->MaxFrame;

	return (SUCCESS);
}

static int read_movie_extrarominfo (FILE *fd, SMovie *movie)
{
	uint8	buf[SMV_EXTRAROMINFO_SIZE], *ptr = buf;

	fseek(fd, movie->SaveStateOffset - SMV_EXTRAROMINFO_SIZE, SEEK_SET);

	if (fread(buf, 1, SMV_EXTRAROMINFO_SIZE, fd) != SMV_EXTRAROMINFO_SIZE)
		return (WRONG_FORMAT);

	ptr += 3; // zero bytes
	movie->ROMCRC32 = Read32(ptr);
	strncpy(movie->ROMName, (char *) ptr, 23);

	return (SUCCESS);
}

static void write_movie_header (FILE *fd, SMovie *movie)
{
	uint8	buf[SMV_HEADER_SIZE], *ptr = buf;

	ZeroMemory(buf, sizeof(buf));

	Write32(SMV_MAGIC, ptr);
	Write32(SMV_VERSION, ptr);
	Write32(movie->MovieId, ptr);
	Write32(movie->RerecordCount, ptr);
	Write32(movie->MaxFrame, ptr);
	Write8(movie->ControllersMask, ptr);
	Write8(movie->Opts, ptr);
	ptr++;
	Write8(movie->SyncFlags, ptr);
	Write32(movie->SaveStateOffset, ptr);
	Write32(movie->ControllerDataOffset, ptr);
	Write32(movie->MaxSample, ptr);
	Write8(movie->PortType[0], ptr);
	Write8(movie->PortType[1], ptr);
	for (int p = 0; p < 2; p++)
	{
		for (int i = 0; i < 4; i++)
			Write8(movie->PortIDs[p][i], ptr);
	}

	size_t	ignore;
	ignore = fwrite(buf, 1, SMV_HEADER_SIZE, fd);
}

static void write_movie_extrarominfo (FILE *fd, SMovie *movie)
{
	uint8	buf[SMV_EXTRAROMINFO_SIZE], *ptr = buf;

	Write8(0, ptr);
	Write8(0, ptr);
	Write8(0, ptr);
	Write32(movie->ROMCRC32, ptr);
	strncpy((char *) ptr, movie->ROMName, 23);

	size_t	ignore;
	ignore = fwrite(buf, 1, SMV_EXTRAROMINFO_SIZE, fd);
}

static void change_state (MovieState new_state)
{
	if (new_state == Movie.State)
		return;

	if (Movie.State == MOVIE_STATE_RECORD)
		flush_movie();

	if (new_state == MOVIE_STATE_NONE)
	{
		truncate_movie();
		fclose(Movie.File);
		Movie.File = NULL;

		if (S9xMoviePlaying() || S9xMovieRecording())
			restore_previous_settings();
	}

	Movie.State = new_state;
}

void S9xMovieFreeze (uint8 **buf, uint32 *size)
{
	if (!S9xMovieActive())
		return;

	uint32	size_needed;
	uint8	*ptr;

	size_needed = sizeof(Movie.MovieId) + sizeof(Movie.CurrentFrame) + sizeof(Movie.MaxFrame) + sizeof(Movie.CurrentSample) + sizeof(Movie.MaxSample);
	size_needed += (uint32) (Movie.BytesPerSample * (Movie.MaxSample + 1));
	*size = size_needed;

	*buf = new uint8[size_needed];
	ptr = *buf;
	if (!ptr)
		return;

	Write32(Movie.MovieId, ptr);
	Write32(Movie.CurrentFrame, ptr);
	Write32(Movie.MaxFrame, ptr);
	Write32(Movie.CurrentSample, ptr);
	Write32(Movie.MaxSample, ptr);

	memcpy(ptr, Movie.InputBuffer, Movie.BytesPerSample * (Movie.MaxSample + 1));
}

int S9xMovieUnfreeze (uint8 *buf, uint32 size)
{
	if (!S9xMovieActive())
		return (FILE_NOT_FOUND);

	if (size < sizeof(Movie.MovieId) + sizeof(Movie.CurrentFrame) + sizeof(Movie.MaxFrame) + sizeof(Movie.CurrentSample) + sizeof(Movie.MaxSample))
		return (WRONG_FORMAT);

	uint8	*ptr = buf;

	uint32	movie_id       = Read32(ptr);
	uint32	current_frame  = Read32(ptr);
	uint32	max_frame      = Read32(ptr);
	uint32	current_sample = Read32(ptr);
	uint32	max_sample     = Read32(ptr);
	uint32	space_needed   = (Movie.BytesPerSample * (max_sample + 1));

	if (current_frame > max_frame || current_sample > max_sample || space_needed > size)
		return (WRONG_MOVIE_SNAPSHOT);

	if (Settings.WrongMovieStateProtection)
		if (movie_id != Movie.MovieId)
			if (max_frame < Movie.MaxFrame || max_sample < Movie.MaxSample || memcmp(Movie.InputBuffer, ptr, space_needed))
				return (WRONG_MOVIE_SNAPSHOT);

	if (!Movie.ReadOnly)
	{
		change_state(MOVIE_STATE_RECORD);

		Movie.CurrentFrame  = current_frame;
		Movie.MaxFrame      = max_frame;
		Movie.CurrentSample = current_sample;
		Movie.MaxSample     = max_sample;
		Movie.RerecordCount++;

		store_movie_settings();

		reserve_buffer_space(space_needed);
		memcpy(Movie.InputBuffer, ptr, space_needed);

		flush_movie();
		fseek(Movie.File, Movie.ControllerDataOffset + (Movie.BytesPerSample * (Movie.CurrentSample + 1)), SEEK_SET);
	}
	else
	{
      uint32   space_processed = (Movie.BytesPerSample * (current_sample + 1));
      if (current_frame > Movie.MaxFrame || current_sample > Movie.MaxSample || memcmp(Movie.InputBuffer, ptr, space_processed))
			return (SNAPSHOT_INCONSISTENT);

		change_state(MOVIE_STATE_PLAY);

		Movie.CurrentFrame  = current_frame;
		Movie.CurrentSample = current_sample;
	}

	Movie.InputBufferPtr = Movie.InputBuffer + (Movie.BytesPerSample * Movie.CurrentSample);
	read_frame_controller_data(true);

	return (SUCCESS);
}

int S9xMovieOpen (const char *filename, bool8 read_only)
{
	FILE	*fd;
	STREAM	stream;
	int		result;
	int		fn;

	if (!(fd = fopen(filename, "rb+")))
	{
		if (!(fd = fopen(filename, "rb")))
			return (FILE_NOT_FOUND);
		else
			read_only = TRUE;
	}

	change_state(MOVIE_STATE_NONE);

	result = read_movie_header(fd, &Movie);
	if (result != SUCCESS)
	{
		fclose(fd);
		return (result);
	}

	read_movie_extrarominfo(fd, &Movie);

	fflush(fd);
	fn = fileno(fd);

	store_previous_settings();
	restore_movie_settings();

	lseek(fn, Movie.SaveStateOffset, SEEK_SET);
	stream = REOPEN_STREAM(fn, "rb");
	if (!stream)
		return (FILE_NOT_FOUND);

	if (Movie.Opts & MOVIE_OPT_FROM_RESET)
	{
		S9xReset();
		reset_controllers();
		result = (READ_STREAM(Memory.SRAM, 0x20000, stream) == 0x20000) ? SUCCESS : WRONG_FORMAT;
	}
	else
		result = S9xUnfreezeFromStream(stream);

	CLOSE_STREAM(stream);

	if (result != SUCCESS)
		return (result);

	if (!(fd = fopen(filename, "rb+")))
	{
		if (!(fd = fopen(filename, "rb")))
			return (FILE_NOT_FOUND);
		else
			read_only = TRUE;
	}

	if (fseek(fd, Movie.ControllerDataOffset, SEEK_SET))
		return (WRONG_FORMAT);

	Movie.File           = fd;
	Movie.BytesPerSample = bytes_per_sample();
	Movie.InputBufferPtr = Movie.InputBuffer;
	reserve_buffer_space(Movie.BytesPerSample * (Movie.MaxSample + 1));

	size_t	ignore;
	ignore = fread(Movie.InputBufferPtr, 1, Movie.BytesPerSample * (Movie.MaxSample + 1), fd);

	// read "baseline" controller data
	if (Movie.MaxSample && Movie.MaxFrame)
		read_frame_controller_data(true);

	Movie.CurrentFrame  = 0;
	Movie.CurrentSample = 0;
	Movie.ReadOnly      = read_only;
	strncpy(Movie.Filename, filename, PATH_MAX + 1);
	Movie.Filename[PATH_MAX] = 0;

	change_state(MOVIE_STATE_PLAY);

	S9xUpdateFrameCounter(-1);

	S9xMessage(S9X_INFO, S9X_MOVIE_INFO, MOVIE_INFO_REPLAY);

	return (SUCCESS);
}

int S9xMovieCreate (const char *filename, uint8 controllers_mask, uint8 opts, const wchar_t *metadata, int metadata_length)
{
	FILE	*fd;
	STREAM	stream;

	if (controllers_mask == 0)
		return (WRONG_FORMAT);

	if (!(fd = fopen(filename, "wb")))
		return (FILE_NOT_FOUND);

	if (metadata_length > MOVIE_MAX_METADATA)
		metadata_length = MOVIE_MAX_METADATA;

	change_state(MOVIE_STATE_NONE);

	store_previous_settings();
	store_movie_settings();

	Movie.MovieId              = (uint32) time(NULL);
	Movie.RerecordCount        = 0;
	Movie.MaxFrame             = 0;
	Movie.MaxSample            = 0;
	Movie.SaveStateOffset      = SMV_HEADER_SIZE + (sizeof(uint16) * metadata_length) + SMV_EXTRAROMINFO_SIZE;
	Movie.ControllerDataOffset = 0;
	Movie.ControllersMask      = controllers_mask;
	Movie.Opts                 = opts;
	Movie.SyncFlags            = MOVIE_SYNC_DATA_EXISTS | MOVIE_SYNC_HASROMINFO;

	write_movie_header(fd, &Movie);

	// convert wchar_t metadata string/array to a uint16 array
	// XXX: UTF-8 is much better...
	if (metadata_length > 0)
	{
		uint8 meta_buf[sizeof(uint16) * MOVIE_MAX_METADATA];
		for (int i = 0; i < metadata_length; i++)
		{
			uint16 c = (uint16) metadata[i];
			meta_buf[i * 2]     = (uint8) (c & 0xff);
			meta_buf[i * 2 + 1] = (uint8) ((c >> 8) & 0xff);
		}

		size_t	ignore;
		ignore = fwrite(meta_buf, sizeof(uint16), metadata_length, fd);
	}

	Movie.ROMCRC32 = Memory.ROMCRC32;
	strncpy(Movie.ROMName, Memory.RawROMName, 23);

	write_movie_extrarominfo(fd, &Movie);

	fclose(fd);

	stream = OPEN_STREAM(filename, "ab");
	if (!stream)
		return (FILE_NOT_FOUND);

	if (opts & MOVIE_OPT_FROM_RESET)
	{
		S9xReset();
		reset_controllers();
		WRITE_STREAM(Memory.SRAM, 0x20000, stream);
	}
	else
		S9xFreezeToStream(stream);

	CLOSE_STREAM(stream);

	if (!(fd = fopen(filename, "rb+")))
		return (FILE_NOT_FOUND);

	fseek(fd, 0, SEEK_END);
	Movie.ControllerDataOffset = (uint32) ftell(fd);

	// 16-byte align the controller input, for hex-editing friendliness if nothing else
	while (Movie.ControllerDataOffset % 16)
	{
		fputc(0xcc, fd); // arbitrary
		Movie.ControllerDataOffset++;
	}

	// write "baseline" controller data
	Movie.File           = fd;
	Movie.BytesPerSample = bytes_per_sample();
	Movie.InputBufferPtr = Movie.InputBuffer;
	write_frame_controller_data();

	Movie.CurrentFrame  = 0;
	Movie.CurrentSample = 0;
	Movie.ReadOnly      = false;
	strncpy(Movie.Filename, filename, PATH_MAX + 1);
	Movie.Filename[PATH_MAX] = 0;

	change_state(MOVIE_STATE_RECORD);

	S9xUpdateFrameCounter(-1);

	S9xMessage(S9X_INFO, S9X_MOVIE_INFO, MOVIE_INFO_RECORD);

	return (SUCCESS);
}

int S9xMovieGetInfo (const char *filename, struct MovieInfo *info)
{
	FILE	*fd;
	SMovie	local_movie;
	int		metadata_length;
	int		result, i;

	flush_movie();

	ZeroMemory(info, sizeof(*info));

	if (!(fd = fopen(filename, "rb")))
		return (FILE_NOT_FOUND);

	result = read_movie_header(fd, &local_movie);
	if (result != SUCCESS)
		return (result);

	info->TimeCreated     = (time_t) local_movie.MovieId;
	info->Version         = local_movie.Version;
	info->Opts            = local_movie.Opts;
	info->SyncFlags       = local_movie.SyncFlags;
	info->ControllersMask = local_movie.ControllersMask;
	info->RerecordCount   = local_movie.RerecordCount;
	info->LengthFrames    = local_movie.MaxFrame;
	info->LengthSamples   = local_movie.MaxSample;
	info->PortType[0]     = local_movie.PortType[0];
	info->PortType[1]     = local_movie.PortType[1];

	if (local_movie.SaveStateOffset > SMV_HEADER_SIZE)
	{
		uint8	meta_buf[sizeof(uint16) * MOVIE_MAX_METADATA];
		int		curRomInfoSize = (local_movie.SyncFlags & MOVIE_SYNC_HASROMINFO) ? SMV_EXTRAROMINFO_SIZE : 0;

		metadata_length = ((int) local_movie.SaveStateOffset - SMV_HEADER_SIZE - curRomInfoSize) / sizeof(uint16);
		metadata_length = (metadata_length >= MOVIE_MAX_METADATA) ? MOVIE_MAX_METADATA - 1 : metadata_length;
		metadata_length = (int) fread(meta_buf, sizeof(uint16), metadata_length, fd);

		for (i = 0; i < metadata_length; i++)
		{
			uint16 c = meta_buf[i * 2] | (meta_buf[i * 2 + 1] << 8);
			info->Metadata[i] = (wchar_t) c;
		}

		info->Metadata[i] = '\0';
	}
	else
		info->Metadata[0] = '\0';

	read_movie_extrarominfo(fd, &local_movie);

	info->ROMCRC32 = local_movie.ROMCRC32;
	strncpy(info->ROMName, local_movie.ROMName, 23);

	fclose(fd);
	if ((fd = fopen(filename, "r+")) == NULL)
		info->ReadOnly = true;
	else
		fclose(fd);

	return (SUCCESS);
}

void S9xMovieUpdate (bool addFrame)
{
	switch (Movie.State)
	{
		case MOVIE_STATE_PLAY:
		{
			if (Movie.CurrentFrame >= Movie.MaxFrame || Movie.CurrentSample >= Movie.MaxSample)
			{
				change_state(MOVIE_STATE_NONE);
				S9xMessage(S9X_INFO, S9X_MOVIE_INFO, MOVIE_INFO_END);
				return;
			}
			else
			{
				if (addFrame)
					S9xUpdateFrameCounter();
				else
				if (SKIPPED_POLLING_PORT_TYPE(Movie.PortType[0]) && SKIPPED_POLLING_PORT_TYPE(Movie.PortType[1]))
					return;

				read_frame_controller_data(addFrame);
				Movie.CurrentSample++;
				if (addFrame)
					Movie.CurrentFrame++;
			}

			break;
		}

		case MOVIE_STATE_RECORD:
		{
			if (addFrame)
				S9xUpdateFrameCounter();
			else
			if (SKIPPED_POLLING_PORT_TYPE(Movie.PortType[0]) && SKIPPED_POLLING_PORT_TYPE(Movie.PortType[1]))
				return;

			write_frame_controller_data();
			Movie.MaxSample = ++Movie.CurrentSample;
			if (addFrame)
				Movie.MaxFrame = ++Movie.CurrentFrame;

			size_t	ignore;
			ignore = fwrite((Movie.InputBufferPtr - Movie.BytesPerSample), 1, Movie.BytesPerSample, Movie.File);

			break;
		}

		default:
		{
			if (addFrame)
				S9xUpdateFrameCounter();

			break;
		}
	}
}

void S9xMovieUpdateOnReset (void)
{
	if (Movie.State == MOVIE_STATE_RECORD)
	{
		reserve_buffer_space((uint32) (Movie.InputBufferPtr + Movie.BytesPerSample - Movie.InputBuffer));
		memset(Movie.InputBufferPtr, 0xFF, Movie.BytesPerSample);
		Movie.InputBufferPtr += Movie.BytesPerSample;
		Movie.MaxSample = ++Movie.CurrentSample;
		Movie.MaxFrame = ++Movie.CurrentFrame;

		size_t	ignore;
		ignore = fwrite((Movie.InputBufferPtr - Movie.BytesPerSample), 1, Movie.BytesPerSample, Movie.File);
	}
}

void S9xMovieInit (void)
{
	ZeroMemory(&Movie, sizeof(Movie));
	Movie.State = MOVIE_STATE_NONE;
}

void S9xMovieStop (bool8 suppress_message)
{
	if (Movie.State != MOVIE_STATE_NONE)
	{
		change_state(MOVIE_STATE_NONE);

		if (!suppress_message)
			S9xMessage(S9X_INFO, S9X_MOVIE_INFO, MOVIE_INFO_STOP);
	}
}

void S9xMovieShutdown (void)
{
	if (S9xMovieActive())
		S9xMovieStop(TRUE);
}

bool8 S9xMovieActive (void)
{
	return (Movie.State != MOVIE_STATE_NONE);
}

bool8 S9xMoviePlaying (void)
{
	return (Movie.State == MOVIE_STATE_PLAY);
}

bool8 S9xMovieRecording (void)
{
	return (Movie.State == MOVIE_STATE_RECORD);
}

uint8 S9xMovieControllers (void)
{
	return (Movie.ControllersMask);
}

bool8 S9xMovieReadOnly (void)
{
	if (!S9xMovieActive())
		return (FALSE);
	return (Movie.ReadOnly);
}

uint32 S9xMovieGetId (void)
{
	if (!S9xMovieActive())
		return (0);
	return (Movie.MovieId);
}

uint32 S9xMovieGetLength (void)
{
	if (!S9xMovieActive())
		return (0);
	return (Movie.MaxFrame);
}

uint32 S9xMovieGetFrameCounter (void)
{
	if (!S9xMovieActive())
		return (0);
	return (Movie.CurrentFrame);
}

void S9xMovieToggleRecState (void)
{
	Movie.ReadOnly = !Movie.ReadOnly;

	if (Movie.ReadOnly)
	   S9xMessage(S9X_INFO, S9X_MOVIE_INFO, "Movie is now read-only.");
	else
	   S9xMessage(S9X_INFO, S9X_MOVIE_INFO, "Movie is now read+write.");
}

void S9xMovieToggleFrameDisplay (void)
{
	Settings.DisplayMovieFrame = !Settings.DisplayMovieFrame;
	S9xReRefresh();
}

void S9xUpdateFrameCounter (int offset)
{
	extern bool8	pad_read;

	offset++;

	if (!Settings.DisplayMovieFrame)
		*GFX.FrameDisplayString = 0;
	else
	if (Movie.State == MOVIE_STATE_RECORD)
		sprintf(GFX.FrameDisplayString, "Recording frame: %d%s",
			max(0, (int) (Movie.CurrentFrame + offset)), pad_read || !Settings.MovieNotifyIgnored ? "" : " (ignored)");
	else
	if (Movie.State == MOVIE_STATE_PLAY)
		sprintf(GFX.FrameDisplayString, "Playing frame: %d / %d",
			max(0, (int) (Movie.CurrentFrame + offset)), Movie.MaxFrame);
#ifdef NETPLAY_SUPPORT
	else
	if (Settings.NetPlay)
		sprintf(GFX.FrameDisplayString, "%s frame: %d", Settings.NetPlayServer ? "Server" : "Client",
			max(0, (int) (NetPlay.FrameCount + offset)));
#endif
}
