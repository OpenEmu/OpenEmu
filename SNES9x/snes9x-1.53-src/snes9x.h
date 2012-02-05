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


#ifndef _SNES9X_H_
#define _SNES9X_H_

#ifndef VERSION
#define VERSION	"1.53"
#endif

//#include <stdio.h>
//#include <stdlib.h>

#include "port.h"
#include "65c816.h"
#include "messages.h"

#ifdef ZLIB
#include <zlib.h>
#define STREAM					gzFile
#define READ_STREAM(p, l, s)	gzread(s, p, l)
#define WRITE_STREAM(p, l, s)	gzwrite(s, p, l)
#define GETS_STREAM(p, l, s)	gzgets(s, p, l)
#define GETC_STREAM(s)			gzgetc(s)
#define OPEN_STREAM(f, m)		gzopen(f, m)
#define REOPEN_STREAM(f, m)		gzdopen(f, m)
#define FIND_STREAM(f)			gztell(f)
#define REVERT_STREAM(f, o, s)	gzseek(f, o, s)
#define CLOSE_STREAM(s)			gzclose(s)
#else
#define STREAM					FILE *
#define READ_STREAM(p, l, s)	fread(p, 1, l, s)
#define WRITE_STREAM(p, l, s)	fwrite(p, 1, l, s)
#define GETS_STREAM(p, l, s)	fgets(p, l, s)
#define GETC_STREAM(s)			fgetc(s)
#define OPEN_STREAM(f, m)		fopen(f, m)
#define REOPEN_STREAM(f, m)		fdopen(f, m)
#define FIND_STREAM(f)			ftell(f)
#define REVERT_STREAM(f, o, s)	fseek(f, o, s)
#define CLOSE_STREAM(s)			fclose(s)
#endif

#define SNES_WIDTH					256
#define SNES_HEIGHT					224
#define SNES_HEIGHT_EXTENDED		239
#define MAX_SNES_WIDTH				(SNES_WIDTH * 2)
#define MAX_SNES_HEIGHT				(SNES_HEIGHT_EXTENDED * 2)
#define IMAGE_WIDTH					(Settings.SupportHiRes ? MAX_SNES_WIDTH : SNES_WIDTH)
#define IMAGE_HEIGHT				(Settings.SupportHiRes ? MAX_SNES_HEIGHT : SNES_HEIGHT_EXTENDED)

#define	NTSC_MASTER_CLOCK			21477272.0
#define	PAL_MASTER_CLOCK			21281370.0

#define SNES_MAX_NTSC_VCOUNTER		262
#define SNES_MAX_PAL_VCOUNTER		312
#define SNES_HCOUNTER_MAX			341

#define ONE_CYCLE					6
#define SLOW_ONE_CYCLE				8
#define TWO_CYCLES					12
#define	ONE_DOT_CYCLE				4

#define SNES_CYCLES_PER_SCANLINE	(SNES_HCOUNTER_MAX * ONE_DOT_CYCLE)
#define SNES_SCANLINE_TIME			(SNES_CYCLES_PER_SCANLINE / NTSC_MASTER_CLOCK)

#define SNES_WRAM_REFRESH_HC_v1		530
#define SNES_WRAM_REFRESH_HC_v2		538
#define SNES_WRAM_REFRESH_CYCLES	40

#define SNES_HBLANK_START_HC		1096					// H=274
#define	SNES_HDMA_START_HC			1106					// FIXME: not true
#define	SNES_HBLANK_END_HC			4						// H=1
#define	SNES_HDMA_INIT_HC			20						// FIXME: not true
#define	SNES_RENDER_START_HC		(48 * ONE_DOT_CYCLE)	// FIXME: Snes9x renders a line at a time.

#define SNES_TR_MASK		(1 <<  4)
#define SNES_TL_MASK		(1 <<  5)
#define SNES_X_MASK			(1 <<  6)
#define SNES_A_MASK			(1 <<  7)
#define SNES_RIGHT_MASK		(1 <<  8)
#define SNES_LEFT_MASK		(1 <<  9)
#define SNES_DOWN_MASK		(1 << 10)
#define SNES_UP_MASK		(1 << 11)
#define SNES_START_MASK		(1 << 12)
#define SNES_SELECT_MASK	(1 << 13)
#define SNES_Y_MASK			(1 << 14)
#define SNES_B_MASK			(1 << 15)

#define DEBUG_MODE_FLAG		(1 <<  0)	// debugger
#define TRACE_FLAG			(1 <<  1)	// debugger
#define SINGLE_STEP_FLAG	(1 <<  2)	// debugger
#define BREAK_FLAG			(1 <<  3)	// debugger
#define SCAN_KEYS_FLAG		(1 <<  4)	// CPU
#define HALTED_FLAG			(1 << 12)	// APU
#define FRAME_ADVANCE_FLAG	(1 <<  9)

#define ROM_NAME_LEN	23
#define AUTO_FRAMERATE	200

struct SCPUState
{
	uint32	Flags;
	int32	Cycles;
	int32	PrevCycles;
	int32	V_Counter;
	uint8	*PCBase;
	bool8	NMILine;
	bool8	IRQLine;
	bool8	IRQTransition;
	bool8	IRQLastState;
	bool8	IRQExternal;
	int32	IRQPending;
	int32	MemSpeed;
	int32	MemSpeedx2;
	int32	FastROMSpeed;
	bool8	InDMA;
	bool8	InHDMA;
	bool8	InDMAorHDMA;
	bool8	InWRAMDMAorHDMA;
	uint8	HDMARanInDMA;
	int32	CurrentDMAorHDMAChannel;
	uint8	WhichEvent;
	int32	NextEvent;
	bool8	WaitingForInterrupt;
	uint32	AutoSaveTimer;
	bool8	SRAMModified;
};

enum
{
	HC_HBLANK_START_EVENT = 1,
	HC_HDMA_START_EVENT   = 2,
	HC_HCOUNTER_MAX_EVENT = 3,
	HC_HDMA_INIT_EVENT    = 4,
	HC_RENDER_EVENT       = 5,
	HC_WRAM_REFRESH_EVENT = 6
};

struct STimings
{
	int32	H_Max_Master;
	int32	H_Max;
	int32	V_Max_Master;
	int32	V_Max;
	int32	HBlankStart;
	int32	HBlankEnd;
	int32	HDMAInit;
	int32	HDMAStart;
	int32	NMITriggerPos;
	int32	IRQTriggerCycles;
	int32	WRAMRefreshPos;
	int32	RenderPos;
	bool8	InterlaceField;
	int32	DMACPUSync;		// The cycles to synchronize DMA and CPU. Snes9x cannot emulate correctly.
	int32	NMIDMADelay;	// The delay of NMI trigger after DMA transfers. Snes9x cannot emulate correctly.
	int32	IRQPendCount;	// This value is just a hack.
	int32	APUSpeedup;
	bool8	APUAllowTimeOverflow;
};

struct SSettings
{
	bool8	TraceDMA;
	bool8	TraceHDMA;
	bool8	TraceVRAM;
	bool8	TraceUnknownRegisters;
	bool8	TraceDSP;
	bool8	TraceHCEvent;

	bool8	SuperFX;
	uint8	DSP;
	bool8	SA1;
	bool8	C4;
	bool8	SDD1;
	bool8	SPC7110;
	bool8	SPC7110RTC;
	bool8	OBC1;
	uint8	SETA;
	bool8	SRTC;
	bool8	BS;
	bool8	BSXItself;
	bool8	BSXBootup;
	bool8	MouseMaster;
	bool8	SuperScopeMaster;
	bool8	JustifierMaster;
	bool8	MultiPlayer5Master;

	bool8	ForceLoROM;
	bool8	ForceHiROM;
	bool8	ForceHeader;
	bool8	ForceNoHeader;
	bool8	ForceInterleaved;
	bool8	ForceInterleaved2;
	bool8	ForceInterleaveGD24;
	bool8	ForceNotInterleaved;
	bool8	ForcePAL;
	bool8	ForceNTSC;
	bool8	PAL;
	uint32	FrameTimePAL;
	uint32	FrameTimeNTSC;
	uint32	FrameTime;

	bool8	SoundSync;
	bool8	SixteenBitSound;
	uint32	SoundPlaybackRate;
	uint32	SoundInputRate;
	bool8	Stereo;
	bool8	ReverseStereo;
	bool8	Mute;

	bool8	SupportHiRes;
	bool8	Transparency;
	uint8	BG_Forced;
	bool8	DisableGraphicWindows;

	bool8	DisplayFrameRate;
	bool8	DisplayWatchedAddresses;
	bool8	DisplayPressedKeys;
	bool8	DisplayMovieFrame;
	bool8	AutoDisplayMessages;
	uint32	InitialInfoStringTimeout;
	uint16	DisplayColor;

	bool8	Multi;
	char	CartAName[PATH_MAX + 1];
	char	CartBName[PATH_MAX + 1];

	bool8	DisableGameSpecificHacks;
	bool8	BlockInvalidVRAMAccessMaster;
	bool8	BlockInvalidVRAMAccess;
	int32	HDMATimingHack;

	bool8	ForcedPause;
	bool8	Paused;
	bool8	StopEmulation;

	uint32	SkipFrames;
	uint32	TurboSkipFrames;
	uint32	AutoMaxSkipFrames;
	bool8	TurboMode;
	uint32	HighSpeedSeek;
	bool8	FrameAdvance;

	bool8	NetPlay;
	bool8	NetPlayServer;
	char	ServerName[128];
	int		Port;

	bool8	MovieTruncate;
	bool8	MovieNotifyIgnored;
	bool8	WrongMovieStateProtection;
	bool8	DumpStreams;
	int		DumpStreamsMaxFrames;

	bool8	TakeScreenshot;
	int8	StretchScreenshots;
	bool8	SnapshotScreenshots;

	bool8	ApplyCheats;
	bool8	NoPatch;
	int32	AutoSaveDelay;
	bool8	DontSaveOopsSnapshot;
	bool8	UpAndDown;

	bool8	OpenGLEnable;
};

struct SSNESGameFixes
{
	uint8	SRAMInitialValue;
	uint8	Uniracers;
};

enum
{
	PAUSE_NETPLAY_CONNECT		= (1 << 0),
	PAUSE_TOGGLE_FULL_SCREEN	= (1 << 1),
	PAUSE_EXIT					= (1 << 2),
	PAUSE_MENU					= (1 << 3),
	PAUSE_INACTIVE_WINDOW		= (1 << 4),
	PAUSE_WINDOW_ICONISED		= (1 << 5),
	PAUSE_RESTORE_GUI			= (1 << 6),
	PAUSE_FREEZE_FILE			= (1 << 7)
};

void S9xSetPause(uint32);
void S9xClearPause(uint32);
void S9xExit(void);
void S9xMessage(int, int, const char *);

extern struct SSettings			Settings;
extern struct SCPUState			CPU;
extern struct STimings			Timings;
extern struct SSNESGameFixes	SNESGameFixes;
extern char						String[513];

#endif
