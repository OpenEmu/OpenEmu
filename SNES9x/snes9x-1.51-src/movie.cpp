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

#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

#if defined(__unix) || defined(__linux) || defined(__sun) || defined(__DJGPP) || defined(__MACOSX__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <time.h>

#ifdef __WIN32__
#include <io.h>
#ifndef W_OK
#define W_OK 2
#endif
#define ftruncate chsize
#endif

#include "movie.h"
#include "snes9x.h"
#include "memmap.h"
#include "cpuexec.h"
#include "snapshot.h"
#include "controls.h"
#include "language.h"
#ifdef NETPLAY_SUPPORT
#include "netplay.h"
#endif

#define SMV_MAGIC	0x1a564d53		// SMV0x1a
#define SMV_VERSION	4
#define SMV_HEADER_SIZE	64
#define SMV_EXTRAROMINFO_SIZE	(3 + sizeof(uint32) + 23)
#define CONTROLLER_DATA_SIZE	2
#define BUFFER_GROWTH_SIZE	4096
#define PERIPHERAL_SUPPORT
#ifdef PERIPHERAL_SUPPORT
	#define MOUSE_DATA_SIZE	5
	#define SCOPE_DATA_SIZE	6
	#define JUSTIFIER_DATA_SIZE	11
#endif

// HACK: reduce movie size by not storing changes that can only affect polled input in the movie for these types, because currently no port sets these types to polling
#define SKIPPED_POLLING_PORT_TYPE(x) ((x==CTL_NONE)||(x==CTL_JOYPAD)||(x==CTL_MP5))

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

enum MovieState
{
	MOVIE_STATE_NONE=0,
	MOVIE_STATE_PLAY,
	MOVIE_STATE_RECORD
};

static struct SMovie
{
	enum MovieState State;
	char   Filename [_MAX_PATH];
	FILE*  File;
	uint32 SaveStateOffset;
	uint32 ControllerDataOffset;
	uint32 MovieId;
	uint32 CurrentFrame;
	uint32 MaxFrame;
	uint32 RerecordCount;
	uint8  ControllersMask;
	uint8  Opts;
	uint8  SyncFlags;
	bool8  ReadOnly;
	uint32 BytesPerSample;
	uint8* InputBuffer;
	uint32 InputBufferSize;
	uint8* InputBufferPtr;

	uint32 ROMCRC32;
	char   ROMName [23];

	uint32 CurrentSample;
	uint32 MaxSample;
	uint8  PortType[2];
	int8   PortIDs[2][4];

	bool8  RecordedThisSession;
	uint32 Version;
} Movie;

/*
// For illustration:
	struct MovieFileHeader
	{
		uint32	magic; // SMV0x1a
		uint32	version; // for Snes9x 1.51 this must be 4. In Snes9x 1.43 it was 1.
		uint32	uid; // used to match savestates to a particular movie... this is also the date/time of first recording
		uint32	rerecord_count;
		uint32	length_frames;
		uint8	flags[4]; // (ControllersMask, Opts, Reserved, SyncFlags)
		uint32	offset_to_savestate; // pointer to embedded savestate or SRAM
		uint32	offset_to_controller_data; // pointer to controller data

		uint32	length_samples; // number of recorded input samples, may be more than length_frames for peripheral-using games
		uint8	PortType[2]; // for each controller port, 0=NONE, 1=JOYPAD, 2=MOUSE, 3=SUPERSCOPE, 4=JUSTIFIER, 5=MULTITAP
		int8	PortIDs[2][4]; // for each port, then for each possible controller in the port, the controller ID number counting up from 0, or -1 if unplugged
		uint8	reserved[18];
	};
	// sizeof(MovieFileHeader) == 64

// after the header comes extra metadata, i.e. author info
	// sizeof(metadata) = (offset_to_savestate - sizeof(MovieFileHeader)) - sizeof(ExtraRomInfo);
	// that should be an even number of bytes because the author info consists of 2-byte characters

// after the metadata comes extra info about the ROM used for recording
	struct ExtraRomInfo
	{
		uint8	reserved[3];
		uint32	romCRC32;
		uint8	romName[23];
	};
	// sizeof(ExtraRomInfo) == 30

// after that comes the savestate or SRAM data (depending on the Opts flag)
	// sizeof(SaveData) <= offset_to_controller_data - offset_to_savestate

// after that comes the controller data
	// sizeof(ControllerData) == length_samples * sizeof(InputSample)
	// sizeof(InputSample) == 2*(sum of bits in ControllersMask) + 5*num_mouse_ports + 6*num_superscope_ports + 11*num_justifier_ports

*/

// some state for peripherals etc. (playback)
#ifdef PERIPHERAL_SUPPORT
uint8 prevPortType[2];
int8 prevPortIDs[2][4];
bool8 prevMouseMaster;
bool8 prevSuperScopeMaster;
bool8 prevJustifierMaster;
bool8 prevMultiPlayer5Master;
#endif
bool8 prevForcePal, prevPal, prevForceNTSC, delayedPrevRestore=false;
bool8 prevUpAndDown, prevSoundEnvelopeHeightReading, prevFakeMuteFix, prevSoundSync, prevCPUShutdown;

static int bytes_per_sample()
{
	int i;
	int num_controllers;

	num_controllers=0;
	for(i=0; i<8; ++i)
	{
		if(Movie.ControllersMask & (1<<i))
		{
			++num_controllers;
		}
	}

	int bytes = CONTROLLER_DATA_SIZE*num_controllers;

#ifdef PERIPHERAL_SUPPORT
	for(int port=0; port<2; port++)
	{
		if(Movie.PortType[port] == CTL_MOUSE) bytes += MOUSE_DATA_SIZE;
		if(Movie.PortType[port] == CTL_SUPERSCOPE) bytes += SCOPE_DATA_SIZE;
		if(Movie.PortType[port] == CTL_JUSTIFIER) bytes += JUSTIFIER_DATA_SIZE;
		// MP5 support is already handled by the regular controllers code
	}
#endif

	return bytes;
}

uint32 Read32(const uint8*& ptr)
{
	uint32 v=(ptr[0] | (ptr[1]<<8) | (ptr[2]<<16) | (ptr[3]<<24));
	ptr += 4;
	return v;
}

uint16 Read16(const uint8*& ptr) /* const version */
{
	uint16 v=(ptr[0] | (ptr[1]<<8));
	ptr += 2;
	return v;
}

uint16 Read16(uint8*& ptr) /* non-const version */
{
	uint16 v=(ptr[0] | (ptr[1]<<8));
	ptr += 2;
	return v;
}

static inline uint8 Read8(const uint8*& ptr) /* const version */
{
	uint8 v=(ptr[0]);
	ptr++;
	return v;
}

static inline uint8 Read8(uint8*& ptr) /* non-const version */
{
	uint8 v=(ptr[0]);
	ptr++;
	return v;
}

void Write32(uint32 v, uint8*& ptr)
{
	ptr[0]=(uint8)(v&0xff);
	ptr[1]=(uint8)((v>>8)&0xff);
	ptr[2]=(uint8)((v>>16)&0xff);
	ptr[3]=(uint8)((v>>24)&0xff);
	ptr += 4;
}

void Write16(uint16 v, uint8*& ptr)
{
	ptr[0]=(uint8)(v&0xff);
	ptr[1]=(uint8)((v>>8)&0xff);
	ptr += 2;
}

static inline void Write8(uint8 v, uint8*& ptr)
{
	ptr[0]=(uint8)(v);
	ptr++;
}

static int read_movie_header(FILE* fd, SMovie* movie)
{
	uint8 header[SMV_HEADER_SIZE];
	if(fread(header, 1, SMV_HEADER_SIZE, fd) != SMV_HEADER_SIZE)
		return WRONG_FORMAT;

	const uint8* ptr=header;
	uint32 magic=Read32(ptr);
	if(magic!=SMV_MAGIC)
		return WRONG_FORMAT;

	uint32 version=Read32(ptr);
	if(version>SMV_VERSION)
		return WRONG_VERSION;

	// we can still get this basic info from v1 movies
	movie->MovieId=Read32(ptr);
	movie->RerecordCount=Read32(ptr);
	movie->MaxFrame=Read32(ptr);
	movie->Version=version;

	int i, p, j;
	for(j = 0; j < 2; j++)
	{
		if((j == 0 && version != 3) || (j == 1 && version == 3))
		{
			movie->ControllersMask=Read8(ptr);
			movie->Opts=Read8(ptr);
			Read8(ptr); // reserved byte
			movie->SyncFlags=Read8(ptr);

			movie->SaveStateOffset=Read32(ptr);
			movie->ControllerDataOffset=Read32(ptr);
		}

		// not part of original v1 SMV format:

		if((j == 1 && version > 3) || (j == 0 && version == 3))
		{
			movie->MaxSample=Read32(ptr);
			movie->PortType[0]=Read8(ptr);
			movie->PortType[1]=Read8(ptr);

			for(p=0;p<2;p++)
				for(i=0;i<4;i++)
					movie->PortIDs[p][i]=Read8(ptr);
		}
	}

	if(version<3)
		return WRONG_VERSION;

	// needs to be at least 1 sample per frame. So, assume that to make hex editing easier, at least for non-peripheral-using movies.
	if(movie->MaxSample < movie->MaxFrame)
		movie->MaxSample = movie->MaxFrame;


	ptr += 18; // reserved bytes

	assert(ptr-header==SMV_HEADER_SIZE);

	return SUCCESS;
}

static int read_movie_extrarominfo(FILE* fd, SMovie* movie)
{
	if((movie->SyncFlags & MOVIE_SYNC_HASROMINFO) != 0)
	{
		fseek(fd, movie->SaveStateOffset - SMV_EXTRAROMINFO_SIZE, SEEK_SET);

		uint8 extraRomInfo[SMV_EXTRAROMINFO_SIZE];
		if(fread(extraRomInfo, 1, SMV_EXTRAROMINFO_SIZE, fd) != SMV_EXTRAROMINFO_SIZE)
			return WRONG_FORMAT;

		const uint8* ptr=extraRomInfo;

		ptr ++; // zero byte
		ptr ++; // zero byte
		ptr ++; // zero byte
		movie->ROMCRC32=Read32(ptr);
		strncpy(movie->ROMName,(const char*)ptr,23); ptr += 23;
	}
	else
	{
		movie->ROMCRC32=Memory.ROMCRC32;
		strncpy(movie->ROMName,(const char*)Memory.ROMName,23);
	}

	return SUCCESS;
}

static void write_movie_header(FILE* fd, const SMovie* movie)
{
	uint8 header[SMV_HEADER_SIZE];
	uint8* ptr=header;

	Write32(SMV_MAGIC, ptr);
	Write32(SMV_VERSION, ptr);
	Write32(movie->MovieId, ptr);
	Write32(movie->RerecordCount, ptr);
	Write32(movie->MaxFrame, ptr);

	Write8(movie->ControllersMask, ptr);
	Write8(movie->Opts, ptr);
	Write8(0, ptr); // reserved byte
	Write8(movie->SyncFlags, ptr);

	Write32(movie->SaveStateOffset, ptr);
	Write32(movie->ControllerDataOffset, ptr);

	// not part of original v1 SMV format:

	Write32(movie->MaxSample, ptr);
	Write8(movie->PortType[0], ptr);
	Write8(movie->PortType[1], ptr);

	int i, p;
	for(p=0;p<2;p++)
		for(i=0;i<4;i++)
			Write8(movie->PortIDs[p][i], ptr);

	// 18 reserved bytes, could be anything, ignored by this version when reading
	for(i=0;i<18-4;i++)
		Write8(0, ptr);
	Write32(movie->RerecordCount/2, ptr); // why not...

	assert(ptr-header==SMV_HEADER_SIZE);

	fwrite(header, 1, SMV_HEADER_SIZE, fd);

	assert(!ferror(fd));
}

static void write_movie_extrarominfo(FILE* fd, const SMovie* movie)
{
	if((movie->SyncFlags & MOVIE_SYNC_HASROMINFO) != 0) // should be true...
	{
		uint8 extraRomInfo [SMV_EXTRAROMINFO_SIZE];
		uint8* ptr = extraRomInfo;

		*ptr++=0; // zero byte
		*ptr++=0; // zero byte
		*ptr++=0; // zero byte
		Write32(movie->ROMCRC32, ptr);
		strncpy((char*)ptr,movie->ROMName,23); ptr += 23;

		fwrite(extraRomInfo, 1, SMV_EXTRAROMINFO_SIZE, fd);
		assert(!ferror(fd));
	}
}

static void flush_movie()
{
	if(!Movie.File)
		return;

	fseek(Movie.File, 0, SEEK_SET);
	write_movie_header(Movie.File, &Movie);
	fseek(Movie.File, Movie.ControllerDataOffset, SEEK_SET);
	fwrite(Movie.InputBuffer, 1, Movie.BytesPerSample*(Movie.MaxSample+1), Movie.File);
	assert(!ferror(Movie.File));
}

static void store_previous_settings()
{
#ifdef PERIPHERAL_SUPPORT
	for(int i=0; i<2; i++){
		enum controllers pt;
		S9xGetController(i, &pt, &prevPortIDs[i][0],&prevPortIDs[i][1],&prevPortIDs[i][2],&prevPortIDs[i][3]);
		prevPortType[i] = (uint8) pt;
	}
	prevMouseMaster = Settings.MouseMaster;
	prevSuperScopeMaster = Settings.SuperScopeMaster;
	prevJustifierMaster = Settings.JustifierMaster;
	prevMultiPlayer5Master = Settings.MultiPlayer5Master;
#endif
	if(!delayedPrevRestore)
	{
		prevPal = Settings.PAL;
		prevCPUShutdown = Settings.ShutdownMaster;
	}
	delayedPrevRestore = false;
	prevForcePal = Settings.ForcePAL;
	prevForceNTSC = Settings.ForceNTSC;
	prevUpAndDown = Settings.UpAndDown;
	prevSoundEnvelopeHeightReading = Settings.SoundEnvelopeHeightReading;
	prevFakeMuteFix = Settings.FakeMuteFix;
	prevSoundSync = Settings.SoundSync;
}

static void restore_previous_settings()
{
#ifdef PERIPHERAL_SUPPORT
	Settings.MouseMaster = prevMouseMaster;
	Settings.SuperScopeMaster = prevSuperScopeMaster;
	Settings.JustifierMaster = prevJustifierMaster;
	Settings.MultiPlayer5Master = prevMultiPlayer5Master;
	S9xSetController(0, (enum controllers)prevPortType[0], prevPortIDs[0][0],prevPortIDs[0][1],prevPortIDs[0][2],prevPortIDs[0][3]);
	S9xSetController(1, (enum controllers)prevPortType[1], prevPortIDs[1][0],prevPortIDs[1][1],prevPortIDs[1][2],prevPortIDs[1][3]);
#endif
	Settings.ForcePAL = prevForcePal;
	Settings.ForceNTSC = prevForceNTSC;
	Settings.SoundEnvelopeHeightReading = prevSoundEnvelopeHeightReading;
	Settings.FakeMuteFix = prevFakeMuteFix;
//	Settings.PAL = prevPal; // changing this after the movie while it's still emulating would be bad
//	Settings.ShutdownMaster = prevCPUShutdown; // changing this after the movie while it's still emulating would be bad
	delayedPrevRestore = true; // wait to change the above 2 settings until later
//	Settings.UpAndDown = prevUpAndDown; // doesn't actually affect synchronization, so leave the setting alone; the port can change it if it wants
//	Settings.SoundSync = prevSoundSync; // doesn't seem to affect synchronization, so leave the setting alone; the port can change it if it wants
}

static void store_movie_settings()
{
#ifdef PERIPHERAL_SUPPORT
	for(int i=0; i<2; i++){
		enum controllers pt;
		S9xGetController(i, &pt, &Movie.PortIDs[i][0],&Movie.PortIDs[i][1],&Movie.PortIDs[i][2],&Movie.PortIDs[i][3]);
		Movie.PortType[i] = (uint8) pt;
	}
	if(!Movie.PortType[0] && !Movie.PortType[1])
		fprintf(stderr, "WARNING: S9xMovieCreate: no controller ports set\n");
#endif
	if(Settings.PAL)
		Movie.Opts |= MOVIE_OPT_PAL;
	else
		Movie.Opts &= ~MOVIE_OPT_PAL;
	Movie.SyncFlags = MOVIE_SYNC_DATA_EXISTS | MOVIE_SYNC_HASROMINFO;
	if(Settings.UpAndDown) Movie.SyncFlags |= MOVIE_SYNC_LEFTRIGHT;
	if(Settings.SoundEnvelopeHeightReading) Movie.SyncFlags |= MOVIE_SYNC_VOLUMEENVX;
	if(Settings.FakeMuteFix || !Settings.APUEnabled) Movie.SyncFlags |= MOVIE_SYNC_FAKEMUTE;
	if(Settings.SoundSync) Movie.SyncFlags |= MOVIE_SYNC_SYNCSOUND;
	if(!Settings.ShutdownMaster) Movie.SyncFlags |= MOVIE_SYNC_NOCPUSHUTDOWN;
}

static void restore_movie_settings()
{
#ifdef PERIPHERAL_SUPPORT
	Settings.MouseMaster = (Movie.PortType[0] == CTL_MOUSE || Movie.PortType[1] == CTL_MOUSE);
	Settings.SuperScopeMaster = (Movie.PortType[0] == CTL_SUPERSCOPE || Movie.PortType[1] == CTL_SUPERSCOPE);
	Settings.JustifierMaster = (Movie.PortType[0] == CTL_JUSTIFIER || Movie.PortType[1] == CTL_JUSTIFIER);
	Settings.MultiPlayer5Master = (Movie.PortType[0] == CTL_MP5 || Movie.PortType[1] == CTL_MP5);
	S9xSetController(0, (enum controllers)Movie.PortType[0], Movie.PortIDs[0][0],Movie.PortIDs[0][1],Movie.PortIDs[0][2],Movie.PortIDs[0][3]);
	S9xSetController(1, (enum controllers)Movie.PortType[1], Movie.PortIDs[1][0],Movie.PortIDs[1][1],Movie.PortIDs[1][2],Movie.PortIDs[1][3]);
#endif
	if(Movie.Opts & MOVIE_OPT_PAL)
	{
		Settings.ForcePAL = Settings.PAL = TRUE; // OK to change while starting playing a movie because either we are re-loading the ROM or we are entering a state that already had this setting set
		Settings.ForceNTSC = FALSE;
	}
	else
	{
		Settings.ForcePAL = Settings.PAL = FALSE; // OK to change while starting playing a movie because either we are re-loading the ROM or we are entering a state that already had this setting set
		Settings.ForceNTSC = TRUE;
	}
	Settings.SoundEnvelopeHeightReading = (Movie.SyncFlags & MOVIE_SYNC_VOLUMEENVX) ? TRUE : FALSE;
	Settings.FakeMuteFix = (Movie.SyncFlags & MOVIE_SYNC_FAKEMUTE) ? TRUE : FALSE;
	Settings.ShutdownMaster = (Movie.SyncFlags & MOVIE_SYNC_NOCPUSHUTDOWN) ? FALSE : TRUE; // OK to change while starting playing a movie because either we are re-loading the ROM or we are entering a state that already had this setting set
//	Settings.UpAndDown = (Movie.SyncFlags & MOVIE_SYNC_LEFTRIGHT) ? TRUE : FALSE; // doesn't actually affect synchronization, so leave the setting alone; the port can change it if it wants
//	Settings.SoundSync = (Movie.SyncFlags & MOVIE_SYNC_SYNCSOUND) ? TRUE : FALSE; // doesn't seem to affect synchronization, so leave the setting alone; the port can change it if it wants
}

// file must still be open for this to work
static void truncate_movie()
{
	if(!Settings.MovieTruncate || !Movie.File)
		return;

	assert(Movie.SaveStateOffset <= Movie.ControllerDataOffset);
	if(Movie.SaveStateOffset > Movie.ControllerDataOffset)
		return;

	const unsigned long length = Movie.ControllerDataOffset + Movie.BytesPerSample * (Movie.MaxSample + 1);
	ftruncate(fileno(Movie.File), length);
}

static void change_state(MovieState new_state)
{
	if(new_state==Movie.State)
		return;

	if(Movie.State==MOVIE_STATE_RECORD)
	{
		flush_movie();
	}

	if(new_state==MOVIE_STATE_NONE)
	{
		// truncate movie to MaxSample length if Settings.MovieTruncate is true
		truncate_movie();

		fclose(Movie.File);
		Movie.File=NULL;

		if(S9xMoviePlaying() || S9xMovieRecording()) // even if recording, it could have been switched to from playback
		{
			restore_previous_settings();
		}
	}

	Movie.State=new_state;
}

static void reserve_buffer_space(uint32 space_needed)
{
	if(space_needed > Movie.InputBufferSize)
	{
		uint32 ptr_offset = Movie.InputBufferPtr - Movie.InputBuffer;
		uint32 alloc_chunks = space_needed / BUFFER_GROWTH_SIZE;
		Movie.InputBufferSize = BUFFER_GROWTH_SIZE * (alloc_chunks+1);
		Movie.InputBuffer = (uint8*)realloc(Movie.InputBuffer, Movie.InputBufferSize);
		Movie.InputBufferPtr = Movie.InputBuffer + ptr_offset;
	}
}


/* accessors into controls.cpp static variables */
uint16 MovieGetJoypad(int i);
void MovieSetJoypad(int i, uint16 buttons);
#ifdef PERIPHERAL_SUPPORT
bool MovieGetMouse(int i, uint8 out [MOUSE_DATA_SIZE]);
void MovieSetMouse(int i, const uint8 in [MOUSE_DATA_SIZE], bool inPolling);
bool MovieGetScope(int i, uint8 out [SCOPE_DATA_SIZE]);
void MovieSetScope(int i, const uint8 in [SCOPE_DATA_SIZE]);
bool MovieGetJustifier(int i, uint8 out [JUSTIFIER_DATA_SIZE]);
void MovieSetJustifier(int i, const uint8 in [JUSTIFIER_DATA_SIZE]);
#endif

static void read_frame_controller_data(bool addFrame)
{
	int i;

	// one sample of all 1 bits = reset code
	// (the SNES controller doesn't have enough buttons to possibly generate this sequence)
	// (a single bit indicator was not used, to avoid having to special-case peripheral recording here)
	if(Movie.InputBufferPtr[0] == 0xFF)
	{
		bool reset = true;
		for(i=1; i<(int)Movie.BytesPerSample; i++)
		{
			if(Movie.InputBufferPtr[i] != 0xFF)
			{
				reset = false;
				break;
			}
		}
		if(reset)
		{
			Movie.InputBufferPtr += Movie.BytesPerSample;
			S9xSoftReset();
			return;
		}
	}

	for(i=0; i<8; ++i)
	{
		if(Movie.ControllersMask & (1<<i))
		{
			MovieSetJoypad(i, Read16(Movie.InputBufferPtr));
		}
		else
		{
			MovieSetJoypad(i, 0);		// pretend the controller is disconnected
		}
	}
#ifdef PERIPHERAL_SUPPORT
	for(int port=0;port<2;port++)
	{
		if(Movie.PortType[port] == CTL_MOUSE)
		{
			uint8 buf [MOUSE_DATA_SIZE];
			memcpy(buf, Movie.InputBufferPtr, MOUSE_DATA_SIZE); Movie.InputBufferPtr += MOUSE_DATA_SIZE;
			MovieSetMouse(port, buf, !addFrame);
		}
		if(Movie.PortType[port] == CTL_SUPERSCOPE)
		{
			uint8 buf [SCOPE_DATA_SIZE];
			memcpy(buf, Movie.InputBufferPtr, SCOPE_DATA_SIZE); Movie.InputBufferPtr += SCOPE_DATA_SIZE;
			MovieSetScope(port, buf);
		}
		if(Movie.PortType[port] == CTL_JUSTIFIER)
		{
			uint8 buf [JUSTIFIER_DATA_SIZE];
			memcpy(buf, Movie.InputBufferPtr, JUSTIFIER_DATA_SIZE); Movie.InputBufferPtr += JUSTIFIER_DATA_SIZE;
			MovieSetJustifier(port, buf);
		}
	}
#endif
}

static void write_frame_controller_data()
{
	reserve_buffer_space((uint32)((Movie.InputBufferPtr+Movie.BytesPerSample)-Movie.InputBuffer));

	int i;
	for(i=0; i<8; ++i)
	{
		if(Movie.ControllersMask & (1<<i))
		{
			Write16((uint16)(MovieGetJoypad(i) & 0xffff), Movie.InputBufferPtr);
		}
		else
		{
			MovieSetJoypad(i, 0);		// pretend the controller is disconnected
		}
	}
#ifdef PERIPHERAL_SUPPORT
	for(int port=0;port<2;port++)
	{
		if(Movie.PortType[port] == CTL_MOUSE)
		{
			uint8 buf [MOUSE_DATA_SIZE];
			MovieGetMouse(port, buf);
			memcpy(Movie.InputBufferPtr, buf, MOUSE_DATA_SIZE); Movie.InputBufferPtr += MOUSE_DATA_SIZE;
		}
		if(Movie.PortType[port] == CTL_SUPERSCOPE)
		{
			uint8 buf [SCOPE_DATA_SIZE];
			MovieGetScope(port, buf);
			memcpy(Movie.InputBufferPtr, buf, SCOPE_DATA_SIZE); Movie.InputBufferPtr += SCOPE_DATA_SIZE;
		}
		if(Movie.PortType[port] == CTL_JUSTIFIER)
		{
			uint8 buf [JUSTIFIER_DATA_SIZE];
			MovieGetJustifier(port, buf);
			memcpy(Movie.InputBufferPtr, buf, JUSTIFIER_DATA_SIZE); Movie.InputBufferPtr += JUSTIFIER_DATA_SIZE;
		}
	}
#endif
}

void S9xMovieInit ()
{
	memset(&Movie, 0, sizeof(Movie));
	Movie.State = MOVIE_STATE_NONE;
}

void S9xMovieShutdown ()
{
	// even if movie is not active, we need to do this in case of ports that output these settings to .cfg file on exit
	if(delayedPrevRestore)
	{
		// ok to restore these now (because emulation is shutting down and snes9x is exiting)
		Settings.PAL = prevPal;
		Settings.ShutdownMaster = prevCPUShutdown;
		delayedPrevRestore = false;
	}

	if(S9xMovieActive())
		S9xMovieStop (TRUE);
}

static void S9xMovieResetControls ()
{
	for(int i=0; i<8; i++)
		MovieSetJoypad(i, 0);

#ifdef PERIPHERAL_SUPPORT
	uint8 clearedMouse [MOUSE_DATA_SIZE];
	memset(clearedMouse, 0, MOUSE_DATA_SIZE);
	clearedMouse[4] = 1; // mouse[n].buttons = 1;

	uint8 clearedScope [SCOPE_DATA_SIZE];
	memset(clearedScope, 0, SCOPE_DATA_SIZE);

	uint8 clearedJustifier [JUSTIFIER_DATA_SIZE];
	memset(clearedJustifier, 0, JUSTIFIER_DATA_SIZE);

	for(int i=0; i<2; i++){
		MovieSetMouse(i,clearedMouse,true);
		MovieSetScope(i,clearedScope);
		MovieSetJustifier(i,clearedJustifier);
	}

#endif
}

void S9xUpdateFrameCounter (int offset)
{
	offset++;
	extern bool8 pad_read;
	if(!GFX.FrameDisplay)
		*GFX.FrameDisplayString = 0;
	else if(Movie.State == MOVIE_STATE_RECORD)
		sprintf(GFX.FrameDisplayString, "Recording frame: %d%s", max(0,(int)(Movie.CurrentFrame+offset)), pad_read||!Settings.MovieNotifyIgnored?"":" (ignored)");
	else if(Movie.State == MOVIE_STATE_PLAY)
		sprintf(GFX.FrameDisplayString, "Playing frame: %d / %d", max(0,(int)(Movie.CurrentFrame+offset)), Movie.MaxFrame);
#ifdef NETPLAY_SUPPORT
	else if(Settings.NetPlay)
		sprintf(GFX.FrameDisplayString, "%s frame: %d", Settings.NetPlayServer?"Server":"Client", max(0,(int)(NetPlay.FrameCount+offset)));
#endif
}

int S9xMovieOpen (const char* filename, bool8 read_only)
{
	FILE* fd;
	STREAM stream;
	int result;
	int fn;

	char movie_filename [_MAX_PATH];
#ifdef __WIN32__
	_fullpath(movie_filename, filename, _MAX_PATH);
#else
	strcpy(movie_filename, filename);
#endif

	if(!(fd=fopen(movie_filename, "rb+")))
		if(!(fd=fopen(movie_filename, "rb")))
			return FILE_NOT_FOUND;
		else
			read_only = TRUE;

	const bool8 wasPaused = Settings.Paused;

	// stop current movie before opening
	change_state(MOVIE_STATE_NONE);

	// read header
	if((result=read_movie_header(fd, &Movie))!=SUCCESS)
	{
		fclose(fd);
		return result;
	}

	read_movie_extrarominfo(fd, &Movie);

	fn=dup(fileno(fd));
	fclose(fd);

	// apparently this lseek is necessary
	lseek(fn, Movie.SaveStateOffset, SEEK_SET);
	if(!(stream=REOPEN_STREAM(fn, "rb")))
		return FILE_NOT_FOUND;

	// store previous, before changing to the movie's settings
	store_previous_settings();

	// set from movie
	restore_movie_settings();

	if(Movie.Opts & MOVIE_OPT_FROM_RESET)
	{
		Movie.State = MOVIE_STATE_PLAY; // prevent NSRT controller switching (in S9xPostRomInit)
		if(!Memory.LoadLastROM())
			S9xReset();
		Memory.ClearSRAM(false); // in case the SRAM read fails
		Movie.State = MOVIE_STATE_NONE;
		S9xMovieResetControls();
		// save only SRAM for a from-reset snapshot
		result=(READ_STREAM(Memory.SRAM, 0x20000, stream) == 0x20000) ? SUCCESS : WRONG_FORMAT;
	}
	else
	{
		result=S9xUnfreezeFromStream(stream);
	}
	CLOSE_STREAM(stream);

	if(result!=SUCCESS)
	{
		return result;
	}

	if(!(fd=fopen(movie_filename, "rb+")))
		if(!(fd=fopen(movie_filename, "rb")))
			return FILE_NOT_FOUND;
		else
			read_only = TRUE;

	if(fseek(fd, Movie.ControllerDataOffset, SEEK_SET))
		return WRONG_FORMAT;

	// read controller data
	Movie.File=fd;
	Movie.BytesPerSample=bytes_per_sample();
	Movie.InputBufferPtr=Movie.InputBuffer;
	uint32 to_read=Movie.BytesPerSample * (Movie.MaxSample+1);
	reserve_buffer_space(to_read);
	fread(Movie.InputBufferPtr, 1, to_read, fd);

	// read "baseline" controller data
	if(Movie.MaxSample && Movie.MaxFrame)
		read_frame_controller_data(true);

	strncpy(Movie.Filename, movie_filename, _MAX_PATH);
	Movie.Filename[_MAX_PATH-1]='\0';
	Movie.CurrentFrame=0;
	Movie.CurrentSample=0;
	Movie.ReadOnly=read_only;
	change_state(MOVIE_STATE_PLAY);

	Settings.Paused = wasPaused;

	Movie.RecordedThisSession = false;
	S9xUpdateFrameCounter(-1);

	S9xMessage(S9X_INFO, S9X_MOVIE_INFO, MOVIE_INFO_REPLAY);
	return SUCCESS;
}

int S9xMovieCreate (const char* filename, uint8 controllers_mask, uint8 opts, const wchar_t* metadata, int metadata_length)
{
	FILE* fd;
	STREAM stream;
	int fn;

	if(controllers_mask==0)
		return WRONG_FORMAT;

	char movie_filename [_MAX_PATH];
#ifdef __WIN32__
	_fullpath(movie_filename, filename, _MAX_PATH);
#else
	strcpy(movie_filename, filename);
#endif

	if(!(fd=fopen(movie_filename, "wb")))
		return FILE_NOT_FOUND;

	const bool8 wasPaused = Settings.Paused;

	// stop current movie before opening
	change_state(MOVIE_STATE_NONE);

	if(metadata_length>MOVIE_MAX_METADATA)
	{
		metadata_length=MOVIE_MAX_METADATA;
	}

	Movie.MovieId=(uint32)time(NULL);
	Movie.RerecordCount=0;
	Movie.MaxFrame=0;
	Movie.MaxSample=0;
	Movie.SaveStateOffset=SMV_HEADER_SIZE+(sizeof(uint16)*metadata_length) + SMV_EXTRAROMINFO_SIZE;
	Movie.ControllerDataOffset=0;
	Movie.ControllersMask=controllers_mask;
	Movie.Opts=opts;
	Movie.SyncFlags=MOVIE_SYNC_DATA_EXISTS|MOVIE_SYNC_HASROMINFO;

	// store previous, in case we switch to playback later
	store_previous_settings();

	// store settings in movie
	store_movie_settings();

	// extra rom info
	Movie.ROMCRC32 = Memory.ROMCRC32;
	strncpy(Movie.ROMName, Memory.ROMName, 23);


	write_movie_header(fd, &Movie);


	// convert wchar_t metadata string/array to a uint16 array
	if(metadata_length>0)
	{
		uint8 meta_buf[MOVIE_MAX_METADATA * sizeof(uint16)];
		for(int i=0; i<metadata_length; ++i)
		{
			uint16 c=(uint16)metadata[i];
			meta_buf[i+i]  =(uint8)(c&0xff);
			meta_buf[i+i+1]=(uint8)((c>>8)&0xff);
		}

		fwrite(meta_buf, sizeof(uint16), metadata_length, fd);
		assert(!ferror(fd));
	}

	write_movie_extrarominfo(fd, &Movie);

	// write snapshot
	fn=dup(fileno(fd));
	fclose(fd);

	// lseek(fn, Movie.SaveStateOffset, SEEK_SET);
	if(!(stream=REOPEN_STREAM(fn, "ab")))
		return FILE_NOT_FOUND;

	if(opts & MOVIE_OPT_FROM_RESET)
	{
		if(!Memory.LoadLastROM())
			S9xReset();
		Memory.ClearSRAM(true); // clear non-saving SRAM
		S9xMovieResetControls();
		// save only SRAM for a from-reset snapshot
		WRITE_STREAM(Memory.SRAM, 0x20000, stream);
	}
	else
	{
		S9xFreezeToStream(stream);
	}
	CLOSE_STREAM(stream);

	if(!(fd=fopen(movie_filename, "rb+")))
		return FILE_NOT_FOUND;

	fseek(fd, 0, SEEK_END);
	Movie.ControllerDataOffset=(uint32)ftell(fd);

	// 16-byte align the controller input, for hex-editing friendliness if nothing else
	while(Movie.ControllerDataOffset % 16)
	{
		fputc(0xCC, fd); // arbitrary
		Movie.ControllerDataOffset++;
	}

	// write "baseline" controller data
	Movie.File=fd;
	Movie.BytesPerSample=bytes_per_sample();
	Movie.InputBufferPtr=Movie.InputBuffer;
	write_frame_controller_data();

	strncpy(Movie.Filename, movie_filename, _MAX_PATH);
	Movie.Filename[_MAX_PATH-1]='\0';
	Movie.CurrentFrame=0;
	Movie.CurrentSample=0;
	Movie.ReadOnly=false;
	change_state(MOVIE_STATE_RECORD);

	Settings.Paused = wasPaused;

	Movie.RecordedThisSession = true;
	S9xUpdateFrameCounter(-1);

	S9xMessage(S9X_INFO, S9X_MOVIE_INFO, MOVIE_INFO_RECORD);
	return SUCCESS;
}

void S9xMovieUpdateOnReset ()
{
	switch(Movie.State)
	{
		case MOVIE_STATE_RECORD:
		{
			reserve_buffer_space((uint32)((Movie.InputBufferPtr+Movie.BytesPerSample)-Movie.InputBuffer));
			memset(Movie.InputBufferPtr, 0xFF, Movie.BytesPerSample);
			Movie.InputBufferPtr += Movie.BytesPerSample;
			Movie.MaxSample = ++Movie.CurrentSample;
			Movie.MaxFrame = ++Movie.CurrentFrame;
			fwrite((Movie.InputBufferPtr - Movie.BytesPerSample), 1, Movie.BytesPerSample, Movie.File);
			assert(!ferror(Movie.File));
		}
		break;
		default: break;
	}
}

void S9xMovieUpdate (bool addFrame)
{
movieUpdateStart:
	switch(Movie.State)
	{
	case MOVIE_STATE_PLAY:
		{
			if(Movie.CurrentFrame>=Movie.MaxFrame || Movie.CurrentSample>=Movie.MaxSample)
			{
				if(!Movie.RecordedThisSession)
				{
					// stop movie; it reached the end
					change_state(MOVIE_STATE_NONE);
					S9xMessage(S9X_INFO, S9X_MOVIE_INFO, MOVIE_INFO_END);
					return;
				}
				else
				{
					// if user has been recording this movie since the last time it started playing,
					// they probably don't want the movie to end now during playback,
					// so switch back to recording when it reaches the end
					change_state(MOVIE_STATE_RECORD);
					S9xMessage(S9X_INFO, S9X_MOVIE_INFO, MOVIE_INFO_RECORD);
					fseek(Movie.File, Movie.ControllerDataOffset+(Movie.BytesPerSample * (Movie.CurrentSample+1)), SEEK_SET);
					Settings.Paused = true; // also pause so it doesn't keep going unless they want it to
					goto movieUpdateStart;
				}
			}
			else
			{
				if(addFrame)
					S9xUpdateFrameCounter();
				else if(SKIPPED_POLLING_PORT_TYPE(Movie.PortType[0]) && SKIPPED_POLLING_PORT_TYPE(Movie.PortType[1]))
					return;

				read_frame_controller_data(addFrame);
				++Movie.CurrentSample;
				if(addFrame)
					++Movie.CurrentFrame;
			}
		}
		break;

		case MOVIE_STATE_RECORD:
		{
			if(addFrame)
				S9xUpdateFrameCounter();
			else if(SKIPPED_POLLING_PORT_TYPE(Movie.PortType[0]) && SKIPPED_POLLING_PORT_TYPE(Movie.PortType[1]))
				return;

			write_frame_controller_data();
			Movie.MaxSample = ++Movie.CurrentSample;
			if(addFrame)
				Movie.MaxFrame = ++Movie.CurrentFrame;
			fwrite((Movie.InputBufferPtr - Movie.BytesPerSample), 1, Movie.BytesPerSample, Movie.File);
			assert(!ferror(Movie.File));

			Movie.RecordedThisSession = true;
		}
		break;

	default:
		if(addFrame)
			S9xUpdateFrameCounter();
		break;
	}
}

void S9xMovieStop (bool8 suppress_message)
{
	if(Movie.State!=MOVIE_STATE_NONE)
	{
		change_state(MOVIE_STATE_NONE);

		if(!suppress_message)
			S9xMessage(S9X_INFO, S9X_MOVIE_INFO, MOVIE_INFO_STOP);
	}
}

int S9xMovieGetInfo (const char* filename, struct MovieInfo* info)
{
	flush_movie();

	FILE* fd;
	int result;
	SMovie local_movie;
	int metadata_length;

	memset(info, 0, sizeof(*info));
	if(!(fd=fopen(filename, "rb")))
		return FILE_NOT_FOUND;

	result = read_movie_header(fd, &local_movie);

	// we can still get this basic info from older, unsupported movies (v1 movies)
	info->TimeCreated=(time_t)local_movie.MovieId;
	info->RerecordCount=local_movie.RerecordCount;
	info->LengthFrames=local_movie.MaxFrame;
	info->Version=local_movie.Version;

	if(result!=SUCCESS)
		return result;

	info->LengthSamples=local_movie.MaxSample;
	info->PortType[0]=local_movie.PortType[0];
	info->PortType[1]=local_movie.PortType[1];
	info->Opts=local_movie.Opts;
	info->SyncFlags=local_movie.SyncFlags;
	info->ControllersMask=local_movie.ControllersMask;

	if(local_movie.SaveStateOffset > SMV_HEADER_SIZE)
	{
		uint8 meta_buf[MOVIE_MAX_METADATA * sizeof(uint16)];
		int i;

		int curRomInfoSize = (local_movie.SyncFlags & MOVIE_SYNC_HASROMINFO) != 0 ? SMV_EXTRAROMINFO_SIZE : 0;

		metadata_length=((int)local_movie.SaveStateOffset-SMV_HEADER_SIZE-curRomInfoSize)/sizeof(uint16);
		metadata_length=(metadata_length>=MOVIE_MAX_METADATA) ? MOVIE_MAX_METADATA-1 : metadata_length;
		metadata_length=(int)fread(meta_buf, sizeof(uint16), metadata_length, fd);

		for(i=0; i<metadata_length; ++i)
		{
			uint16 c=meta_buf[i+i] | (meta_buf[i+i+1] << 8);
			info->Metadata[i]=(wchar_t)c;
		}
		info->Metadata[i]='\0';
	}
	else
	{
		info->Metadata[0]='\0';
	}

	read_movie_extrarominfo(fd, &local_movie);

	info->ROMCRC32=local_movie.ROMCRC32;
	strncpy(info->ROMName,local_movie.ROMName,23);

	fclose(fd);

	if(access(filename, W_OK))
		info->ReadOnly=true;

	return SUCCESS;
}

bool8 S9xMovieActive ()
{
	return (Movie.State!=MOVIE_STATE_NONE);
}
bool8 S9xMoviePlaying ()
{
	return (Movie.State==MOVIE_STATE_PLAY);
}
bool8 S9xMovieRecording ()
{
	return (Movie.State==MOVIE_STATE_RECORD);
}

uint8 S9xMovieControllers ()
{
	return Movie.ControllersMask;
}

bool8 S9xMovieReadOnly ()
{
	if(!S9xMovieActive())
		return false;

	return Movie.ReadOnly;
}

uint32 S9xMovieGetId ()
{
	if(!S9xMovieActive())
		return 0;

	return Movie.MovieId;
}

uint32 S9xMovieGetLength ()
{
	if(!S9xMovieActive())
		return 0;

	return Movie.MaxFrame;
}

uint32 S9xMovieGetFrameCounter ()
{
	if(!S9xMovieActive())
		return 0;

	return Movie.CurrentFrame;
}

void S9xMovieToggleRecState()
{
   Movie.ReadOnly=!Movie.ReadOnly;

   if (Movie.ReadOnly)
       S9xMessage(S9X_INFO, S9X_MOVIE_INFO, "Movie is now read-only.");
   else
       S9xMessage(S9X_INFO, S9X_MOVIE_INFO, "Movie is now read+write.");
}

void S9xMovieToggleFrameDisplay ()
{
	GFX.FrameDisplay = !GFX.FrameDisplay;
	// updating the frame counter string here won't work, because it may or may not be 1 too high now
	extern void S9xReRefresh();
	S9xReRefresh();
}

void S9xMovieFreeze (uint8** buf, uint32* size)
{
	// sanity check
	if(!S9xMovieActive())
	{
		return;
	}

	*buf = NULL;
	*size = 0;

	// compute size needed for the buffer
	uint32 size_needed = sizeof(Movie.MovieId) + sizeof(Movie.CurrentFrame) + sizeof(Movie.MaxFrame) + sizeof(Movie.CurrentSample) + sizeof(Movie.MaxSample);
	size_needed += (uint32)(Movie.BytesPerSample * (Movie.MaxSample+1));
	*buf=new uint8[size_needed];
	*size=size_needed;

	uint8* ptr = *buf;
	if(!ptr)
	{
		return;
	}

	Write32(Movie.MovieId, ptr);
	Write32(Movie.CurrentFrame, ptr);
	Write32(Movie.MaxFrame, ptr);
	Write32(Movie.CurrentSample, ptr);
	Write32(Movie.MaxSample, ptr);

	memcpy(ptr, Movie.InputBuffer, Movie.BytesPerSample * (Movie.MaxSample+1));
}

int S9xMovieUnfreeze (const uint8* buf, uint32 size)
{
	// sanity check
	if(!S9xMovieActive())
	{
		return FILE_NOT_FOUND;
	}

	const uint8* ptr = buf;
	if(size < sizeof(Movie.MovieId) + sizeof(Movie.CurrentFrame) + sizeof(Movie.MaxFrame) + sizeof(Movie.CurrentSample) + sizeof(Movie.MaxSample) )
	{
		return WRONG_FORMAT;
	}

	uint32 movie_id = Read32(ptr);
	uint32 current_frame = Read32(ptr);
	uint32 max_frame = Read32(ptr);
	uint32 current_sample = Read32(ptr);
	uint32 max_sample = Read32(ptr);
	uint32 space_needed = (Movie.BytesPerSample * (max_sample+1));

	if(current_frame > max_frame ||
	   current_sample > max_sample ||
	   space_needed > size)
	{
		return WRONG_MOVIE_SNAPSHOT;
	}

	if(movie_id != Movie.MovieId)
		if(Settings.WrongMovieStateProtection)
			if(max_frame < Movie.MaxFrame || max_sample < Movie.MaxSample ||
			   memcmp(Movie.InputBuffer, ptr, space_needed))
				return WRONG_MOVIE_SNAPSHOT;

	if(!Movie.ReadOnly)
	{
		// here, we are going to take the input data from the savestate
		// and make it the input data for the current movie, then continue
		// writing new input data at the currentframe pointer
		change_state(MOVIE_STATE_RECORD);
//		S9xMessage(S9X_INFO, S9X_MOVIE_INFO, MOVIE_INFO_RERECORD);

		Movie.CurrentFrame = current_frame;
		Movie.MaxFrame = max_frame;
		Movie.CurrentSample = current_sample;
		Movie.MaxSample = max_sample;
		++Movie.RerecordCount;

		// when re-recording, update the sync info in the movie to the new settings as of the last re-record.
		store_movie_settings();

		reserve_buffer_space(space_needed);
		memcpy(Movie.InputBuffer, ptr, space_needed);
		flush_movie();
		fseek(Movie.File, Movie.ControllerDataOffset+(Movie.BytesPerSample * (Movie.CurrentSample+1)), SEEK_SET);
	}
	else
	{
		// here, we are going to keep the input data from the movie file
		// and simply rewind to the currentframe pointer
		// this will cause a desync if the savestate is not in sync // <-- NOT ANYMORE
		// with the on-disk recording data, but it's easily solved
		// by loading another savestate or playing the movie from the beginning

		// don't allow loading a state inconsistent with the current movie
		if(current_frame > Movie.MaxFrame || current_sample > Movie.MaxSample ||
		   memcmp(Movie.InputBuffer, ptr, space_needed))
		{
			return SNAPSHOT_INCONSISTENT;
		}

		change_state(MOVIE_STATE_PLAY);
//		S9xMessage(S9X_INFO, S9X_MOVIE_INFO, MOVIE_INFO_REWIND);

		Movie.CurrentFrame = current_frame;
		Movie.CurrentSample = current_sample;
	}

	Movie.InputBufferPtr = Movie.InputBuffer + (Movie.BytesPerSample * (Movie.CurrentSample));
	read_frame_controller_data(true);

	return SUCCESS;
}
