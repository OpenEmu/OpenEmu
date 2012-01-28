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


#include "snes9x.h"
#include "memmap.h"
#include "dma.h"
#include "apu/apu.h"
#include "fxemu.h"
#include "sdd1.h"
#include "srtc.h"
#include "snapshot.h"
#include "cheats.h"
#include "logger.h"
#ifdef DEBUGGER
#include "debug.h"
#endif

static void S9xResetCPU (void);
static void S9xSoftResetCPU (void);


static void S9xResetCPU (void)
{
	S9xSoftResetCPU();
	Registers.SL = 0xff;
	Registers.P.W = 0;
	Registers.A.W = 0;
	Registers.X.W = 0;
	Registers.Y.W = 0;
	SetFlags(MemoryFlag | IndexFlag | IRQ | Emulation);
	ClearFlags(Decimal);
}

static void S9xSoftResetCPU (void)
{
	CPU.Cycles = 182; // Or 188. This is the cycle count just after the jump to the Reset Vector.
	CPU.PrevCycles = CPU.Cycles;
	CPU.V_Counter = 0;
	CPU.Flags = CPU.Flags & (DEBUG_MODE_FLAG | TRACE_FLAG);
	CPU.PCBase = NULL;
	CPU.NMILine = FALSE;
	CPU.IRQLine = FALSE;
	CPU.IRQTransition = FALSE;
	CPU.IRQLastState = FALSE;
	CPU.IRQExternal = FALSE;
	CPU.IRQPending = Timings.IRQPendCount;
	CPU.MemSpeed = SLOW_ONE_CYCLE;
	CPU.MemSpeedx2 = SLOW_ONE_CYCLE * 2;
	CPU.FastROMSpeed = SLOW_ONE_CYCLE;
	CPU.InDMA = FALSE;
	CPU.InHDMA = FALSE;
	CPU.InDMAorHDMA = FALSE;
	CPU.InWRAMDMAorHDMA = FALSE;
	CPU.HDMARanInDMA = 0;
	CPU.CurrentDMAorHDMAChannel = -1;
	CPU.WhichEvent = HC_RENDER_EVENT;
	CPU.NextEvent  = Timings.RenderPos;
	CPU.WaitingForInterrupt = FALSE;
	CPU.AutoSaveTimer = 0;
	CPU.SRAMModified = FALSE;

	Registers.PBPC = 0;
	Registers.PB = 0;
	Registers.PCw = S9xGetWord(0xfffc);
	OpenBus = Registers.PCh;
	Registers.D.W = 0;
	Registers.DB = 0;
	Registers.SH = 1;
	Registers.SL -= 3;
	Registers.XH = 0;
	Registers.YH = 0;

	ICPU.ShiftedPB = 0;
	ICPU.ShiftedDB = 0;
	SetFlags(MemoryFlag | IndexFlag | IRQ | Emulation);
	ClearFlags(Decimal);

	Timings.InterlaceField = FALSE;
	Timings.H_Max = Timings.H_Max_Master;
	Timings.V_Max = Timings.V_Max_Master;
	Timings.NMITriggerPos = 0xffff;
	if (Model->_5A22 == 2)
		Timings.WRAMRefreshPos = SNES_WRAM_REFRESH_HC_v2;
	else
		Timings.WRAMRefreshPos = SNES_WRAM_REFRESH_HC_v1;

	S9xSetPCBase(Registers.PBPC);

	ICPU.S9xOpcodes = S9xOpcodesE1;
	ICPU.S9xOpLengths = S9xOpLengthsM1X1;

	S9xUnpackStatus();
}

void S9xReset (void)
{
	S9xResetSaveTimer(FALSE);
	S9xResetLogger();

	memset(Memory.RAM, 0x55, 0x20000);
	memset(Memory.VRAM, 0x00, 0x10000);
	ZeroMemory(Memory.FillRAM, 0x8000);

	if (Settings.BS)
		S9xResetBSX();

	S9xResetCPU();
	S9xResetPPU();
	S9xResetDMA();
	S9xResetAPU();

	if (Settings.DSP)
		S9xResetDSP();
	if (Settings.SuperFX)
		S9xResetSuperFX();
	if (Settings.SA1)
		S9xSA1Init();
	if (Settings.SDD1)
		S9xResetSDD1();
	if (Settings.SPC7110)
		S9xResetSPC7110();
	if (Settings.C4)
		S9xInitC4();
	if (Settings.OBC1)
		S9xResetOBC1();
	if (Settings.SRTC)
		S9xResetSRTC();

	S9xInitCheatData();
}

void S9xSoftReset (void)
{
	S9xResetSaveTimer(FALSE);

	ZeroMemory(Memory.FillRAM, 0x8000);

	if (Settings.BS)
		S9xResetBSX();

	S9xSoftResetCPU();
	S9xSoftResetPPU();
	S9xResetDMA();
	S9xSoftResetAPU();

	if (Settings.DSP)
		S9xResetDSP();
	if (Settings.SuperFX)
		S9xResetSuperFX();
	if (Settings.SA1)
		S9xSA1Init();
	if (Settings.SDD1)
		S9xResetSDD1();
	if (Settings.SPC7110)
		S9xResetSPC7110();
	if (Settings.C4)
		S9xInitC4();
	if (Settings.OBC1)
		S9xResetOBC1();
	if (Settings.SRTC)
		S9xResetSRTC();

	S9xInitCheatData();
}
