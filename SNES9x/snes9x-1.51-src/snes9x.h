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



#ifndef _SNES9X_H_
#define _SNES9X_H_

#define VERSION "1.51"

#include <stdio.h>
#include <stdlib.h>

#ifdef __WIN32__
#include <windows.h>
#ifdef ZLIB
#include <zlib.h>
#endif
#endif

//#include "language.h" // files should include this as needed, no need to recompile practically everything when it changes

#include "port.h"
#include "65c816.h"
#include "messages.h"

#if defined(USE_GLIDE) && !defined(GFX_MULTI_FORMAT)
#define GFX_MULTI_FORMAT
#endif

#define ROM_NAME_LEN 23

#ifdef ZLIB
#ifndef __WIN32__
#include <zlib.h>
#endif
#define STREAM gzFile
#define READ_STREAM(p,l,s) gzread (s,p,l)
#define WRITE_STREAM(p,l,s) gzwrite (s,p,l)
#define GETS_STREAM(p,l,s) gzgets(s,p,l)
#define GETC_STREAM(s) gzgetc(s)
#define OPEN_STREAM(f,m) gzopen (f,m)
#define REOPEN_STREAM(f,m) gzdopen (f,m)
#define FIND_STREAM(f)	gztell(f)
#define REVERT_STREAM(f,o,s)  gzseek(f,o,s)
#define CLOSE_STREAM(s) gzclose (s)
#else
#define STREAM FILE *
#define READ_STREAM(p,l,s) fread (p,1,l,s)
#define WRITE_STREAM(p,l,s) fwrite (p,1,l,s)
#define GETS_STREAM(p,l,s) fgets(p,l,s)
#define GETC_STREAM(s) fgetc(s)
#define OPEN_STREAM(f,m) fopen (f,m)
#define REOPEN_STREAM(f,m) fdopen (f,m)
#define FIND_STREAM(f)	ftell(f)
#define REVERT_STREAM(f,o,s)	 fseek(f,o,s)
#define CLOSE_STREAM(s) fclose (s)
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

#define	SNES_APU_CLOCK				1024000.0				// 1026900.0?
#define SNES_APU_ACCURACY			10
#define	SNES_APU_ONE_CYCLE_SCALED	((int32) (NTSC_MASTER_CLOCK / SNES_APU_CLOCK * (1 << SNES_APU_ACCURACY)))
#define SNES_APUTIMER2_CYCLE_SCALED	((int32) (NTSC_MASTER_CLOCK / 64000.0 * (1 << SNES_APU_ACCURACY)))


#define AUTO_FRAMERATE	200

#define SNES_TR_MASK		(1 << 4)
#define SNES_TL_MASK		(1 << 5)
#define SNES_X_MASK			(1 << 6)
#define SNES_A_MASK			(1 << 7)
#define SNES_RIGHT_MASK	    (1 << 8)
#define SNES_LEFT_MASK	    (1 << 9)
#define SNES_DOWN_MASK	    (1 << 10)
#define SNES_UP_MASK	    (1 << 11)
#define SNES_START_MASK	    (1 << 12)
#define SNES_SELECT_MASK    (1 << 13)
#define SNES_Y_MASK			(1 << 14)
#define SNES_B_MASK			(1 << 15)

#define DEBUG_MODE_FLAG	    (1 << 0)
#define TRACE_FLAG			(1 << 1)
#define SINGLE_STEP_FLAG    (1 << 2)
#define BREAK_FLAG			(1 << 3)
#define SCAN_KEYS_FLAG	    (1 << 4)
#define SAVE_SNAPSHOT_FLAG  (1 << 5)
#define DELAYED_NMI_FLAG    (1 << 6)
#define NMI_FLAG			(1 << 7)
#define PROCESS_SOUND_FLAG  (1 << 8)
#define FRAME_ADVANCE_FLAG  (1 << 9)
#define DELAYED_NMI_FLAG2   (1 << 10)
#define IRQ_FLAG            (1 << 11)
#define HALTED_FLAG         (1 << 12)

struct SCPUState{
	uint32	Flags;
	bool8	BranchSkip;
	bool8	NMIActive;
	bool8	IRQActive;
	bool8	WaitingForInterrupt;
	bool8	InDMAorHDMA;
	bool8	InWRAMDMAorHDMA;
	uint8	WhichEvent;
	uint8	*PCBase;
	uint32	PBPCAtOpcodeStart;
	uint32	WaitAddress;
	uint32	WaitCounter;
	int32	Cycles;
	int32	NextEvent;
	int32	V_Counter;
	int32	MemSpeed;
	int32	MemSpeedx2;
	int32	FastROMSpeed;
	uint32	AutoSaveTimer;
	bool8	SRAMModified;
	bool8	BRKTriggered;
	bool8	TriedInterleavedMode2;
	int32	IRQPending;
	bool8	InDMA;
	bool8	InHDMA;
	uint8	HDMARanInDMA;
	int32	PrevCycles;
};

struct STimings {
	int32	H_Max_Master;
	int32	H_Max;
	int32	V_Max_Master;
	int32	V_Max;
	int32	HBlankStart;
	int32	HBlankEnd;
	int32	HDMAInit;
	int32	HDMAStart;
	int32	NMITriggerPos;
	int32	WRAMRefreshPos;
	int32	RenderPos;
	bool8	InterlaceField;
	int32	DMACPUSync;
};

enum {
	HC_HBLANK_START_EVENT = 1,
	HC_IRQ_1_3_EVENT      = 2,
	HC_HDMA_START_EVENT   = 3,
	HC_IRQ_3_5_EVENT      = 4,
	HC_HCOUNTER_MAX_EVENT = 5,
	HC_IRQ_5_7_EVENT      = 6,
	HC_HDMA_INIT_EVENT    = 7,
	HC_IRQ_7_9_EVENT      = 8,
	HC_RENDER_EVENT       = 9,
	HC_IRQ_9_A_EVENT      = 10,
	HC_WRAM_REFRESH_EVENT = 11,
	HC_IRQ_A_1_EVENT      = 12
};

struct SSettings{
    /* CPU options */
    bool8  APUEnabled;
    bool8  Shutdown;
    uint8  SoundSkipMethod;
    int32  HDMATimingHack;
    bool8  DisableIRQ;
    bool8  Paused;
    bool8  ForcedPause;
    bool8  StopEmulation;
    bool8  FrameAdvance;

    /* Tracing options */
    bool8  TraceDMA;
    bool8  TraceHDMA;
    bool8  TraceVRAM;
    bool8  TraceUnknownRegisters;
    bool8  TraceDSP;

    /* Joystick options */
    bool8  JoystickEnabled;

    /* ROM timing options (see also H_Max above) */
    bool8  ForcePAL;
    bool8  ForceNTSC;
    bool8  PAL;
    uint32 FrameTimePAL;
    uint32 FrameTimeNTSC;
    uint32 FrameTime;
    uint32 SkipFrames;

    /* ROM image options */
    bool8  ForceLoROM;
    bool8  ForceHiROM;
    bool8  ForceHeader;
    bool8  ForceNoHeader;
    bool8  ForceInterleaved;
    bool8  ForceInterleaved2;
    bool8  ForceNotInterleaved;

    /* Peripherial options */
    bool8  ForceSuperFX;
    bool8  ForceNoSuperFX;
    bool8  ForceDSP1;
    bool8  ForceNoDSP1;
    bool8  ForceSA1;
    bool8  ForceNoSA1;
    bool8  ForceC4;
    bool8  ForceNoC4;
    bool8  ForceSDD1;
    bool8  ForceNoSDD1;
    bool8  SRTC;

    bool8  ShutdownMaster;
    bool8  MultiPlayer5Master;
    bool8  SuperScopeMaster;
    bool8  MouseMaster;
    bool8  JustifierMaster;
    bool8  SuperFX;
    bool8  DSP1Master;
    bool8  SA1;
    bool8  C4;
    bool8  SDD1;
    bool8  SPC7110;
    bool8  SPC7110RTC;
    bool8  OBC1;
    /* Sound options */
    uint32 SoundPlaybackRate;
    bool8  TraceSoundDSP;
    bool8  Stereo;
    bool8  ReverseStereo;
    bool8  SixteenBitSound;
    int    SoundBufferSize;
    int    SoundMixInterval;
    bool8  SoundEnvelopeHeightReading;
    bool8  DisableSoundEcho;
    bool8  DisableSampleCaching;
    bool8  DisableMasterVolume;
    bool8  SoundSync;
    bool8  FakeMuteFix;
    bool8  InterpolatedSound;
    bool8  ThreadSound;
    bool8  Mute;
    bool8  NextAPUEnabled;
    uint8  AltSampleDecode;
    bool8  FixFrequency;

    /* Graphics options */
    bool8  Transparency;
    bool8  SupportHiRes;
    bool8  Mode7Interpolate; // no longer used?
	bool8  AutoDisplayMessages;
    uint8  BG_Forced;
    bool8  SnapshotScreenshots;
    uint32 InitialInfoStringTimeout; // Messages normally display for this many frames

    /* SNES graphics options */
    bool8  BGLayering;
    bool8  DisableGraphicWindows;
    bool8  ForceTransparency;
    bool8  ForceNoTransparency;
    bool8  DisableHDMA;
    bool8  DisplayFrameRate;
    bool8  DisableRangeTimeOver; /* XXX: unused */
	bool8  DisplayWatchedAddresses;

    /* Multi ROMs */
	bool8  Multi;
	char   CartAName[_MAX_PATH + 1];
	char   CartBName[_MAX_PATH + 1];

    /* Others */
    bool8  NetPlay;
    bool8  NetPlayServer;
    char   ServerName [128];
    int    Port;
    bool8  GlideEnable;
    bool8  OpenGLEnable;
    int32  AutoSaveDelay; /* Time in seconds before S-RAM auto-saved if modified. */
    bool8  ApplyCheats;
    bool8  TurboMode;
    bool8  OldTurbo;
    bool8  UpAndDown;
    uint8  DisplayPressedKeys; // The value indicates how to do it.
    uint32 HighSpeedSeek;
    uint32 TurboSkipFrames;
    uint32 AutoMaxSkipFrames;
	bool8  MovieTruncate;
	bool8  MovieNotifyIgnored;
	bool8  WrongMovieStateProtection;

/* Fixes for individual games */
    bool8  WinterGold;
    bool8  BS;	/* Japanese Satellite System games. */
	bool8  BSXItself;
	bool8  BSXBootup;
    uint8  APURAMInitialValue;
    bool8  SampleCatchup;
    int8   SETA;
    bool8  BlockInvalidVRAMAccess;
    bool8  TakeScreenshot;
    int8   StretchScreenshots;
    uint16 DisplayColor;
    int    SoundDriver;
    int    AIDOShmId;
    bool8  SDD1Pack;
    bool8  NoPatch;
    bool8  ForceInterleaveGD24;
};

struct SSNESGameFixes
{
    uint8 APU_OutPorts_ReturnValueFix;
    uint8 SRAMInitialValue;
	uint8 Uniracers;
};

START_EXTERN_C
extern struct SSettings Settings;
extern struct SCPUState CPU;
extern struct STimings	Timings;
extern struct SSNESGameFixes SNESGameFixes;
extern char String [513];

void S9xExit ();
void S9xMessage (int type, int number, const char *message);
void S9xLoadSDD1Data ();
END_EXTERN_C

enum {
    PAUSE_NETPLAY_CONNECT = (1 << 0),
    PAUSE_TOGGLE_FULL_SCREEN = (1 << 1),
    PAUSE_EXIT = (1 << 2),
    PAUSE_MENU = (1 << 3),
    PAUSE_INACTIVE_WINDOW = (1 << 4),
    PAUSE_WINDOW_ICONISED = (1 << 5),
    PAUSE_RESTORE_GUI = (1 << 6),
    PAUSE_FREEZE_FILE = (1 << 7)
};
void S9xSetPause (uint32 mask);
void S9xClearPause (uint32 mask);

#endif

