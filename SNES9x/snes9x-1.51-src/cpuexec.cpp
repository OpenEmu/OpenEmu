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




#include "snes9x.h"
#include "memmap.h"
#include "cpuops.h"
#include "ppu.h"
#include "cpuexec.h"
#include "debug.h"
#include "snapshot.h"
#include "gfx.h"
#include "missing.h"
#include "apu.h"
#include "dma.h"
#include "sa1.h"
#include "spc7110.h"

#ifndef ZSNES_FX
#include "fxemu.h"
extern struct FxInit_s SuperFX;
#endif

void S9xMainLoop (void)
{
	if(ICPU.SavedAtOp)
	{
		ICPU.SavedAtOp = FALSE;
		Registers.PCw = CPU.PBPCAtOpcodeStart;
		if(CPU.PCBase)
			CPU.Cycles -= CPU.MemSpeed;
		goto doOp;
	}

	for (;;)
	{
		if (CPU.Flags)
		{
			if (CPU.Flags & NMI_FLAG)
			{
				if (Timings.NMITriggerPos <= CPU.Cycles)
				{
					CPU.Flags &= ~NMI_FLAG;
					Timings.NMITriggerPos = 0xffff;
					if (CPU.WaitingForInterrupt)
					{
						CPU.WaitingForInterrupt = FALSE;
						Registers.PCw++;
					}

					S9xOpcode_NMI();
				}
			}

#ifdef DEBUGGER
			if ((CPU.Flags & BREAK_FLAG) && !(CPU.Flags & SINGLE_STEP_FLAG))
			{
				for (int Break = 0; Break != 6; Break++)
				{
					if (S9xBreakpoint[Break].Enabled &&
						S9xBreakpoint[Break].Bank == Registers.PB &&
						S9xBreakpoint[Break].Address == Registers.PCw)
					{
						if (S9xBreakpoint[Break].Enabled == 2)
							S9xBreakpoint[Break].Enabled = TRUE;
						else
							CPU.Flags |= DEBUG_MODE_FLAG;
					}
				}
			}
#endif

			CHECK_SOUND();

			if (CPU.Flags & IRQ_FLAG)
			{
				if (CPU.IRQPending)
					// FIXME: In case of IRQ during WRAM refresh
					CPU.IRQPending = 0;
				else
				{
					if (CPU.WaitingForInterrupt)
					{
						CPU.WaitingForInterrupt = FALSE;
						Registers.PCw++;
					}

					if (CPU.IRQActive && !Settings.DisableIRQ)
					{
						if (!CheckFlag(IRQ))
						// in IRQ handler $4211 is supposed to be read, so IRQ_FLAG should be cleared.
							S9xOpcode_IRQ();
					}
					else
						CPU.Flags &= ~IRQ_FLAG;
				}
			}

			if (CPU.Flags & SCAN_KEYS_FLAG)
				break;

#ifdef DEBUGGER
			if (CPU.Flags & DEBUG_MODE_FLAG)
				break;

			if (CPU.Flags & TRACE_FLAG)
				S9xTrace();

			if (CPU.Flags & SINGLE_STEP_FLAG)
			{
				CPU.Flags &= ~SINGLE_STEP_FLAG;
				CPU.Flags |= DEBUG_MODE_FLAG;
			}
#endif
		}

#ifdef CPU_SHUTDOWN
		CPU.PBPCAtOpcodeStart = Registers.PBPC;
#endif
	doOp:
		register uint8				Op;
		register struct	SOpcodes	*Opcodes;

		CPU.PrevCycles = CPU.Cycles;

		if (CPU.PCBase)
		{
			Op = CPU.PCBase[Registers.PCw];
			CPU.Cycles += CPU.MemSpeed;
			Opcodes = ICPU.S9xOpcodes;
		}
		else
		{
			Op = S9xGetByte(Registers.PBPC);
			OpenBus = Op;
			Opcodes = S9xOpcodesSlow;
		}

		if ((Registers.PCw&MEMMAP_MASK) + ICPU.S9xOpLengths[Op] >= MEMMAP_BLOCK_SIZE)
		{
			uint8	*oldPCBase = CPU.PCBase;

			CPU.PCBase = GetBasePointer(ICPU.ShiftedPB + ((uint16) (Registers.PCw + 4)));
			if (oldPCBase!=CPU.PCBase || (Registers.PCw&~MEMMAP_MASK) == (0xffff & ~MEMMAP_MASK))
				Opcodes = S9xOpcodesSlow;
		}

		Registers.PCw++;
		(*Opcodes[Op].S9xOpcode)();

		if(ICPU.SavedAtOp)
		{
			ICPU.SavedAtOp = false;
			continue;
		}

		S9xAPUExecute();

		if (SA1.Executing)
			S9xSA1MainLoop();

		while (CPU.Cycles >= CPU.NextEvent)
			S9xDoHEventProcessing();
    }

    S9xPackStatus();
    APURegisters.PC = IAPU.PC - IAPU.RAM;
    S9xAPUPackStatus();

    if (CPU.Flags & SCAN_KEYS_FLAG)
    {
#ifdef DEBUGGER
		if (!(CPU.Flags & FRAME_ADVANCE_FLAG))
#endif
		S9xSyncSpeed();
		CPU.Flags &= ~SCAN_KEYS_FLAG;
    }
}

void S9xSetIRQ (uint32 source)
{
	CPU.IRQActive |= source;
	CPU.Flags |= IRQ_FLAG;

	if (CPU.WaitingForInterrupt)
	{
		// Force IRQ to trigger immediately after WAI -
		// Final Fantasy Mystic Quest crashes without this.
		CPU.WaitingForInterrupt = FALSE;
		Registers.PCw++;
	}
}

void S9xClearIRQ (uint32 source)
{
	CLEAR_IRQ_SOURCE(source);
}

void S9xDoHEventProcessing (void)
{
#ifdef DEBUGGER
	char	mes[256];
#endif

#ifdef CPU_SHUTDOWN
	CPU.WaitCounter++;
#endif
	switch (CPU.WhichEvent)
    {
		case HC_HBLANK_START_EVENT:
			S9xCheckMissingHTimerPosition(Timings.HBlankStart);

			break;

		case HC_HDMA_START_EVENT:
			if (IPPU.HDMA && CPU.V_Counter <= PPU.ScreenHeight)
			{
			#ifdef DEBUGGER
				sprintf(mes, "*** HDMA  HC:%04d, Channel:%02x", CPU.Cycles, IPPU.HDMA);
				S9xTraceMessage(mes);
			#endif
				IPPU.HDMA = S9xDoHDMA(IPPU.HDMA);
			}

			S9xCheckMissingHTimerPosition(Timings.HDMAStart);

			break;

		case HC_HCOUNTER_MAX_EVENT:
		#ifndef ZSNES_FX
			if (Settings.SuperFX)
			{
				if (!SuperFX.oneLineDone)
					S9xSuperFXExec();
				SuperFX.oneLineDone = FALSE;
			}
		#else
			S9xSuperFXExec();
		#endif

		#ifndef STORM
			if (Settings.SoundSync)
				S9xGenerateSound();
		#endif

			CPU.Cycles -= Timings.H_Max;
			IAPU.NextAPUTimerPos -= (Timings.H_Max << SNES_APU_ACCURACY);
			APU.Cycles -= (Timings.H_Max << SNES_APU_ACCURACY);

			if ((Timings.NMITriggerPos != 0xffff) && (Timings.NMITriggerPos >= Timings.H_Max))
				Timings.NMITriggerPos -= Timings.H_Max;

			ICPU.Scanline++;

			CPU.V_Counter++;
			if (CPU.V_Counter >= Timings.V_Max)	// V ranges from 0 to Timings.V_Max - 1
			{
				CPU.V_Counter = 0;
				Timings.InterlaceField ^= 1;

				// From byuu:
				// [NTSC]
				// interlace mode has 525 scanlines: 263 on the even frame, and 262 on the odd.
				// non-interlace mode has 524 scanlines: 262 scanlines on both even and odd frames.
				// [PAL] <PAL info is unverified on hardware>
				// interlace mode has 625 scanlines: 313 on the even frame, and 312 on the odd.
				// non-interlace mode has 624 scanlines: 312 scanlines on both even and odd frames.
				if (IPPU.Interlace && !Timings.InterlaceField)
					Timings.V_Max = Timings.V_Max_Master + 1;	// 263 (NTSC), 313?(PAL)
				else
					Timings.V_Max = Timings.V_Max_Master;		// 262 (NTSC), 312?(PAL)

				Memory.FillRAM[0x213F] ^= 0x80;
				PPU.RangeTimeOver = 0;

				// FIXME: reading $4210 will wait 2 cycles, then perform reading, then wait 4 more cycles.
				Memory.FillRAM[0x4210] = Model->_5A22;
				CPU.Flags &= ~NMI_FLAG;
				Timings.NMITriggerPos = 0xffff;

				ICPU.Frame++;
				PPU.HVBeamCounterLatched = 0;
				CPU.Flags |= SCAN_KEYS_FLAG;
			}

			// From byuu:
			// In non-interlace mode, there are 341 dots per scanline, and 262 scanlines per frame.
			// On odd frames, scanline 240 is one dot short.
			// In interlace mode, there are always 341 dots per scanline. Even frames have 263 scanlines,
			// and odd frames have 262 scanlines.
			// Interlace mode scanline 240 on odd frames is not missing a dot.
			if (CPU.V_Counter == 240 && !IPPU.Interlace && Timings.InterlaceField)	// V=240
				Timings.H_Max = Timings.H_Max_Master - ONE_DOT_CYCLE;	// HC=1360
			else
				Timings.H_Max = Timings.H_Max_Master;					// HC=1364

			if (Model->_5A22 == 2)
			{
				if (CPU.V_Counter != 240 || IPPU.Interlace || !Timings.InterlaceField)	// V=240
				{
					if (Timings.WRAMRefreshPos == SNES_WRAM_REFRESH_HC_v2 - ONE_DOT_CYCLE)	// HC=534
						Timings.WRAMRefreshPos = SNES_WRAM_REFRESH_HC_v2;					// HC=538
					else
						Timings.WRAMRefreshPos = SNES_WRAM_REFRESH_HC_v2 - ONE_DOT_CYCLE;	// HC=534
				}
			}
			else
				Timings.WRAMRefreshPos = SNES_WRAM_REFRESH_HC_v1;

			S9xCheckMissingHTimerPosition(0);

			if (CPU.V_Counter == PPU.ScreenHeight + FIRST_VISIBLE_LINE)	// VBlank starts from V=225(240).
			{
				S9xEndScreenRefresh();
				IPPU.HDMA = 0;
				// Bits 7 and 6 of $4212 are computed when read in S9xGetPPU.
			#ifdef DEBUGGER
				missing.dma_this_frame = 0;
			#endif
				IPPU.MaxBrightness = PPU.Brightness;
				PPU.ForcedBlanking = (Memory.FillRAM [0x2100] >> 7) & 1;

				if (!PPU.ForcedBlanking)
				{
					PPU.OAMAddr = PPU.SavedOAMAddr;

					uint8	tmp = 0;

					if (PPU.OAMPriorityRotation)
						tmp = (PPU.OAMAddr & 0xFE) >> 1;
					if ((PPU.OAMFlip & 1) || PPU.FirstSprite!=tmp)
					{
						PPU.FirstSprite = tmp;
						IPPU.OBJChanged = TRUE;
					}

					PPU.OAMFlip = 0;
				}

				// FIXME: writing to $4210 will wait 6 cycles.
				Memory.FillRAM[0x4210] = 0x80 | Model->_5A22;
				if (Memory.FillRAM[0x4200] & 0x80)
				{
					// FIXME: triggered at HC=6, checked just before the final CPU cycle,
					// then, when to call S9xOpcode_NMI()?
					CPU.Flags |= NMI_FLAG;
					Timings.NMITriggerPos = 6 + 6;
				}

			}

			if (CPU.V_Counter == PPU.ScreenHeight + 3)	// FIXME: not true
			{
				if (Memory.FillRAM[0x4200] & 1)
					S9xDoAutoJoypad();
			}

			if (CPU.V_Counter == FIRST_VISIBLE_LINE)	// V=1
				S9xStartScreenRefresh();

			CPU.NextEvent = -1;

			break;

		case HC_HDMA_INIT_EVENT:
			if (CPU.V_Counter == 0)
				S9xStartHDMA();

			S9xCheckMissingHTimerPosition(Timings.HDMAInit);

			break;

		case HC_RENDER_EVENT:
			if (CPU.V_Counter >= FIRST_VISIBLE_LINE && CPU.V_Counter <= PPU.ScreenHeight)
				RenderLine((uint8)(CPU.V_Counter - FIRST_VISIBLE_LINE));

			S9xCheckMissingHTimerPosition(Timings.RenderPos);

			break;

		case HC_WRAM_REFRESH_EVENT:
		#ifdef DEBUGGER
			sprintf(mes, "*** WRAM Refresh  HC:%04d", CPU.Cycles);
			S9xTraceMessage(mes);
		#endif
			S9xCheckMissingHTimerHalt(Timings.WRAMRefreshPos, SNES_WRAM_REFRESH_CYCLES);
			CPU.Cycles += SNES_WRAM_REFRESH_CYCLES;
			S9xAPUExecute();

			S9xCheckMissingHTimerPosition(Timings.WRAMRefreshPos);

			break;

		case HC_IRQ_1_3_EVENT:
		case HC_IRQ_3_5_EVENT:
		case HC_IRQ_5_7_EVENT:
		case HC_IRQ_7_9_EVENT:
		case HC_IRQ_9_A_EVENT:
		case HC_IRQ_A_1_EVENT:
			if (PPU.HTimerEnabled && (!PPU.VTimerEnabled || (CPU.V_Counter == PPU.VTimerPosition)))
				S9xSetIRQ(PPU_H_BEAM_IRQ_SOURCE);
			else
			if (PPU.VTimerEnabled && (CPU.V_Counter == PPU.VTimerPosition))
				S9xSetIRQ(PPU_V_BEAM_IRQ_SOURCE);

			break;
    }

    S9xReschedule();
}
