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
#include "controls.h"
#include "movie.h"
#include "display.h"
#ifdef NETPLAY_SUPPORT
#include "netplay.h"
#endif
#ifdef DEBUGGER
#include "debug.h"
#include "missing.h"
#endif

extern uint8	*HDMAMemPointers[8];


static inline void S9xLatchCounters (bool force)
{
	if (force || (Memory.FillRAM[0x4213] & 0x80))
	{
		// Latch h and v counters, like the gun
	#ifdef DEBUGGER
		missing.h_v_latch = 1;
	#endif

		PPU.HVBeamCounterLatched = 1;
		PPU.VBeamPosLatched = (uint16) CPU.V_Counter;

		// From byuu:
		// All dots are 4 cycles long, except dots 322 and 326. dots 322 and 326 are 6 cycles long.
		// This holds true for all scanlines except scanline 240 on non-interlace odd frames.
		// The reason for this is because this scanline is only 1360 cycles long,
		// instead of 1364 like all other scanlines.
		// This makes the effective range of hscan_pos 0-339 at all times.
		int32	hc = CPU.Cycles;

		if (Timings.H_Max == Timings.H_Max_Master) // 1364
		{
			if (hc >= 1292)
				hc -= (ONE_DOT_CYCLE / 2);
			if (hc >= 1308)
				hc -= (ONE_DOT_CYCLE / 2);
		}

		PPU.HBeamPosLatched = (uint16) (hc / ONE_DOT_CYCLE);

		Memory.FillRAM[0x213f] |= 0x40;
	}

	if (CPU.V_Counter >  PPU.GunVLatch || (CPU.V_Counter == PPU.GunVLatch && CPU.Cycles >= PPU.GunHLatch * ONE_DOT_CYCLE))
		PPU.GunVLatch = 1000;
}

static inline void S9xTryGunLatch (bool force)
{
	if (CPU.V_Counter >  PPU.GunVLatch || (CPU.V_Counter == PPU.GunVLatch && CPU.Cycles >= PPU.GunHLatch * ONE_DOT_CYCLE))
	{
		if (force || (Memory.FillRAM[0x4213] & 0x80))
		{
		#ifdef DEBUGGER
			missing.h_v_latch = 1;
		#endif

			PPU.HVBeamCounterLatched = 1;
			PPU.VBeamPosLatched = (uint16) PPU.GunVLatch;
			PPU.HBeamPosLatched = (uint16) PPU.GunHLatch;

			Memory.FillRAM[0x213f] |= 0x40;
		}

		PPU.GunVLatch = 1000;
	}
}

void S9xUpdateHVTimerPosition (void)
{
	PPU.HTimerPosition = PPU.IRQHBeamPos * ONE_DOT_CYCLE + Timings.IRQTriggerCycles;
	if (Timings.H_Max == Timings.H_Max_Master)	// 1364
	{
		if (PPU.IRQHBeamPos > 322)
			PPU.HTimerPosition += (ONE_DOT_CYCLE / 2);
		if (PPU.IRQHBeamPos > 326)
			PPU.HTimerPosition += (ONE_DOT_CYCLE / 2);
	}

	PPU.VTimerPosition = PPU.IRQVBeamPos;

	if ((PPU.HTimerPosition >= Timings.H_Max) && (PPU.IRQHBeamPos < 340))
	{
		PPU.HTimerPosition -= Timings.H_Max;
		PPU.VTimerPosition++;
		// FIXME
		if (PPU.VTimerPosition >= Timings.V_Max)
			PPU.VTimerPosition = 0;
	}

#ifdef DEBUGGER
	S9xTraceFormattedMessage("--- IRQ Timer set  HTimer:%d Pos:%04d  VTimer:%d Pos:%03d",
		PPU.HTimerEnabled, PPU.HTimerPosition, PPU.VTimerEnabled, PPU.VTimerPosition);
#endif
}

void S9xFixColourBrightness (void)
{
	IPPU.XB = mul_brightness[PPU.Brightness];

	for (int i = 0; i < 256; i++)
	{
		IPPU.Red[i]   = IPPU.XB[(PPU.CGDATA[i])       & 0x1f];
		IPPU.Green[i] = IPPU.XB[(PPU.CGDATA[i] >>  5) & 0x1f];
		IPPU.Blue[i]  = IPPU.XB[(PPU.CGDATA[i] >> 10) & 0x1f];
		IPPU.ScreenColors[i] = BUILD_PIXEL(IPPU.Red[i], IPPU.Green[i], IPPU.Blue[i]);
	}
}

void S9xSetPPU (uint8 Byte, uint16 Address)
{
	// MAP_PPU: $2000-$3FFF

	if (CPU.InDMAorHDMA)
	{
		if (CPU.CurrentDMAorHDMAChannel >= 0 && DMA[CPU.CurrentDMAorHDMAChannel].ReverseTransfer)
		{
			// S9xSetPPU() is called to write to DMA[].AAddress
			if ((Address & 0xff00) == 0x2100)
			{
				// Cannot access to Address Bus B ($2100-$21ff) via (H)DMA
				return;
			}
			else
			{
				// 0x2000-0x3FFF is connected to Address Bus A
				// SA1, SuperFX and SRTC are mapped here
				// I don't bother for now...
				return;
			}
		}
		else
		{
			// S9xSetPPU() is called to read from $21xx
			// Take care of DMA wrapping
			if (Address > 0x21ff)
				Address = 0x2100 + (Address & 0xff);
		}
	}

#ifdef DEBUGGER
	if (CPU.InHDMA)
		S9xTraceFormattedMessage("--- HDMA PPU %04X -> %02X", Address, Byte);
#endif

	if ((Address & 0xffc0) == 0x2140) // APUIO0, APUIO1, APUIO2, APUIO3
		// write_port will run the APU until given clock before writing value
		S9xAPUWritePort(Address & 3, Byte);
	else
	if (Address <= 0x2183)
	{
		switch (Address)
		{
			case 0x2100: // INIDISP
				if (Byte != Memory.FillRAM[0x2100])
				{
					FLUSH_REDRAW();

					if (PPU.Brightness != (Byte & 0xf))
					{
						IPPU.ColorsChanged = TRUE;
						IPPU.DirectColourMapsNeedRebuild = TRUE;
						PPU.Brightness = Byte & 0xf;
						S9xFixColourBrightness();
						if (PPU.Brightness > IPPU.MaxBrightness)
							IPPU.MaxBrightness = PPU.Brightness;
					}

					if ((Memory.FillRAM[0x2100] & 0x80) != (Byte & 0x80))
					{
						IPPU.ColorsChanged = TRUE;
						PPU.ForcedBlanking = (Byte >> 7) & 1;
					}
				}

				if ((Memory.FillRAM[0x2100] & 0x80) && CPU.V_Counter == PPU.ScreenHeight + FIRST_VISIBLE_LINE)
				{
					PPU.OAMAddr = PPU.SavedOAMAddr;

					uint8 tmp = 0;
					if (PPU.OAMPriorityRotation)
						tmp = (PPU.OAMAddr & 0xfe) >> 1;
					if ((PPU.OAMFlip & 1) || PPU.FirstSprite != tmp)
					{
						PPU.FirstSprite = tmp;
						IPPU.OBJChanged = TRUE;
					}

					PPU.OAMFlip = 0;
				}

				break;

			case 0x2101: // OBSEL
				if (Byte != Memory.FillRAM[0x2101])
				{
					FLUSH_REDRAW();
					PPU.OBJNameBase = (Byte & 3) << 14;
					PPU.OBJNameSelect = ((Byte >> 3) & 3) << 13;
					PPU.OBJSizeSelect = (Byte >> 5) & 7;
					IPPU.OBJChanged = TRUE;
				}

				break;

			case 0x2102: // OAMADDL
				PPU.OAMAddr = ((Memory.FillRAM[0x2103] & 1) << 8) | Byte;
				PPU.OAMFlip = 2;
				PPU.OAMReadFlip = 0;
				PPU.SavedOAMAddr = PPU.OAMAddr;
				if (PPU.OAMPriorityRotation && PPU.FirstSprite != (PPU.OAMAddr >> 1))
				{
					PPU.FirstSprite = (PPU.OAMAddr & 0xfe) >> 1;
					IPPU.OBJChanged = TRUE;
				#ifdef DEBUGGER
					missing.sprite_priority_rotation = 1;
				#endif
				}

				break;

			case 0x2103: // OAMADDH
				PPU.OAMAddr = ((Byte & 1) << 8) | Memory.FillRAM[0x2102];
				PPU.OAMPriorityRotation = (Byte & 0x80) ? 1 : 0;
				if (PPU.OAMPriorityRotation)
				{
					if (PPU.FirstSprite != (PPU.OAMAddr >> 1))
					{
						PPU.FirstSprite = (PPU.OAMAddr & 0xfe) >> 1;
						IPPU.OBJChanged = TRUE;
					#ifdef DEBUGGER
						missing.sprite_priority_rotation = 1;
					#endif
					}
				}
				else
				{
					if (PPU.FirstSprite != 0)
					{
						PPU.FirstSprite = 0;
						IPPU.OBJChanged = TRUE;
					#ifdef DEBUGGER
						missing.sprite_priority_rotation = 1;
					#endif
					}
				}

				PPU.OAMFlip = 0;
				PPU.OAMReadFlip = 0;
				PPU.SavedOAMAddr = PPU.OAMAddr;

				break;

			case 0x2104: // OAMDATA
				REGISTER_2104(Byte);
				break;

			case 0x2105: // BGMODE
				if (Byte != Memory.FillRAM[0x2105])
				{
					FLUSH_REDRAW();
					PPU.BG[0].BGSize = (Byte >> 4) & 1;
					PPU.BG[1].BGSize = (Byte >> 5) & 1;
					PPU.BG[2].BGSize = (Byte >> 6) & 1;
					PPU.BG[3].BGSize = (Byte >> 7) & 1;
					PPU.BGMode = Byte & 7;
					// BJ: BG3Priority only takes effect if BGMode == 1 and the bit is set
					PPU.BG3Priority = ((Byte & 0x0f) == 0x09);
					IPPU.Interlace = Memory.FillRAM[0x2133] & 1;
				#ifdef DEBUGGER
					missing.modes[PPU.BGMode] = 1;
				#endif
				}

				break;

			case 0x2106: // MOSAIC
				if (Byte != Memory.FillRAM[0x2106])
				{
					FLUSH_REDRAW();
					PPU.MosaicStart = CPU.V_Counter;
					if (PPU.MosaicStart > PPU.ScreenHeight)
						PPU.MosaicStart = 0;
					PPU.Mosaic = (Byte >> 4) + 1;
					PPU.BGMosaic[0] = (Byte & 1);
					PPU.BGMosaic[1] = (Byte & 2);
					PPU.BGMosaic[2] = (Byte & 4);
					PPU.BGMosaic[3] = (Byte & 8);
				#ifdef DEBUGGER
					if ((Byte & 0xf0) && (Byte & 0x0f))
						missing.mosaic = 1;
				#endif
				}

				break;

			case 0x2107: // BG1SC
				if (Byte != Memory.FillRAM[0x2107])
				{
					FLUSH_REDRAW();
					PPU.BG[0].SCSize = Byte & 3;
					PPU.BG[0].SCBase = (Byte & 0x7c) << 8;
				}

				break;

			case 0x2108: // BG2SC
				if (Byte != Memory.FillRAM[0x2108])
				{
					FLUSH_REDRAW();
					PPU.BG[1].SCSize = Byte & 3;
					PPU.BG[1].SCBase = (Byte & 0x7c) << 8;
				}

				break;

			case 0x2109: // BG3SC
				if (Byte != Memory.FillRAM[0x2109])
				{
					FLUSH_REDRAW();
					PPU.BG[2].SCSize = Byte & 3;
					PPU.BG[2].SCBase = (Byte & 0x7c) << 8;
				}

				break;

			case 0x210a: // BG4SC
				if (Byte != Memory.FillRAM[0x210a])
				{
					FLUSH_REDRAW();
					PPU.BG[3].SCSize = Byte & 3;
					PPU.BG[3].SCBase = (Byte & 0x7c) << 8;
				}

				break;

			case 0x210b: // BG12NBA
				if (Byte != Memory.FillRAM[0x210b])
				{
					FLUSH_REDRAW();
					PPU.BG[0].NameBase = (Byte & 7) << 12;
					PPU.BG[1].NameBase = ((Byte >> 4) & 7) << 12;
				}

				break;

			case 0x210c: // BG34NBA
				if (Byte != Memory.FillRAM[0x210c])
				{
					FLUSH_REDRAW();
					PPU.BG[2].NameBase = (Byte & 7) << 12;
					PPU.BG[3].NameBase = ((Byte >> 4) & 7) << 12;
				}

				break;

			case 0x210d: // BG1HOFS, M7HOFS
				PPU.BG[0].HOffset = (Byte << 8) | (PPU.BGnxOFSbyte & ~7) | ((PPU.BG[0].HOffset >> 8) & 7);
				PPU.M7HOFS = (Byte << 8) | PPU.M7byte;
				PPU.BGnxOFSbyte = Byte;
				PPU.M7byte = Byte;
				break;

			case 0x210e: // BG1VOFS, M7VOFS
				PPU.BG[0].VOffset = (Byte << 8) | PPU.BGnxOFSbyte;
				PPU.M7VOFS = (Byte << 8) | PPU.M7byte;
				PPU.BGnxOFSbyte = Byte;
				PPU.M7byte = Byte;
				break;

			case 0x210f: // BG2HOFS
				PPU.BG[1].HOffset = (Byte << 8) | (PPU.BGnxOFSbyte & ~7) | ((PPU.BG[1].HOffset >> 8) & 7);
				PPU.BGnxOFSbyte = Byte;
				break;

			case 0x2110: // BG2VOFS
				PPU.BG[1].VOffset = (Byte << 8) | PPU.BGnxOFSbyte;
				PPU.BGnxOFSbyte = Byte;
				break;

			case 0x2111: // BG3HOFS
				PPU.BG[2].HOffset = (Byte << 8) | (PPU.BGnxOFSbyte & ~7) | ((PPU.BG[2].HOffset >> 8) & 7);
				PPU.BGnxOFSbyte = Byte;
				break;

			case 0x2112: // BG3VOFS
				PPU.BG[2].VOffset = (Byte << 8) | PPU.BGnxOFSbyte;
				PPU.BGnxOFSbyte = Byte;
				break;

			case 0x2113: // BG4HOFS
				PPU.BG[3].HOffset = (Byte << 8) | (PPU.BGnxOFSbyte & ~7) | ((PPU.BG[3].HOffset >> 8) & 7);
				PPU.BGnxOFSbyte = Byte;
				break;

			case 0x2114: // BG4VOFS
				PPU.BG[3].VOffset = (Byte << 8) | PPU.BGnxOFSbyte;
				PPU.BGnxOFSbyte = Byte;
				break;

			case 0x2115: // VMAIN
				PPU.VMA.High = (Byte & 0x80) == 0 ? FALSE : TRUE;
				switch (Byte & 3)
				{
					case 0: PPU.VMA.Increment = 1;   break;
					case 1: PPU.VMA.Increment = 32;  break;
					case 2: PPU.VMA.Increment = 128; break;
					case 3: PPU.VMA.Increment = 128; break;
				}

				if (Byte & 0x0c)
				{
					static uint16 Shift[4]    = { 0, 5, 6, 7 };
					static uint16 IncCount[4] = { 0, 32, 64, 128 };

					uint8 i = (Byte & 0x0c) >> 2;
					PPU.VMA.FullGraphicCount = IncCount[i];
					PPU.VMA.Mask1 = IncCount[i] * 8 - 1;
					PPU.VMA.Shift = Shift[i];
				#ifdef DEBUGGER
					missing.vram_full_graphic_inc = (Byte & 0x0c) >> 2;
				#endif
				}
				else
					PPU.VMA.FullGraphicCount = 0;
			#ifdef DEBUGGER
				if (Byte & 3)
					missing.vram_inc = Byte & 3;
			#endif
				break;

			case 0x2116: // VMADDL
				PPU.VMA.Address &= 0xff00;
				PPU.VMA.Address |= Byte;

				if (PPU.VMA.FullGraphicCount)
				{
					uint32 addr = PPU.VMA.Address;
					uint32 rem = addr & PPU.VMA.Mask1;
					uint32 address = (addr & ~PPU.VMA.Mask1) + (rem >> PPU.VMA.Shift) + ((rem & (PPU.VMA.FullGraphicCount - 1)) << 3);
					IPPU.VRAMReadBuffer = READ_WORD(Memory.VRAM + ((address << 1) & 0xffff));
				}
				else
					IPPU.VRAMReadBuffer = READ_WORD(Memory.VRAM + ((PPU.VMA.Address << 1) & 0xffff));

				break;

			case 0x2117: // VMADDH
				PPU.VMA.Address &= 0x00ff;
				PPU.VMA.Address |= Byte << 8;

				if (PPU.VMA.FullGraphicCount)
				{
					uint32 addr = PPU.VMA.Address;
					uint32 rem = addr & PPU.VMA.Mask1;
					uint32 address = (addr & ~PPU.VMA.Mask1) + (rem >> PPU.VMA.Shift) + ((rem & (PPU.VMA.FullGraphicCount - 1)) << 3);
					IPPU.VRAMReadBuffer = READ_WORD(Memory.VRAM + ((address << 1) & 0xffff));
				}
				else
					IPPU.VRAMReadBuffer = READ_WORD(Memory.VRAM + ((PPU.VMA.Address << 1) & 0xffff));

				break;

			case 0x2118: // VMDATAL
				REGISTER_2118(Byte);
				break;

			case 0x2119: // VMDATAH
				REGISTER_2119(Byte);
				break;

			case 0x211a: // M7SEL
				if (Byte != Memory.FillRAM[0x211a])
				{
					FLUSH_REDRAW();
					PPU.Mode7Repeat = Byte >> 6;
					if (PPU.Mode7Repeat == 1)
						PPU.Mode7Repeat = 0;
					PPU.Mode7VFlip = (Byte & 2) >> 1;
					PPU.Mode7HFlip = Byte & 1;
				}

				break;

			case 0x211b: // M7A
				PPU.MatrixA = PPU.M7byte | (Byte << 8);
				PPU.Need16x8Mulitply = TRUE;
				PPU.M7byte = Byte;
				break;

			case 0x211c: // M7B
				PPU.MatrixB = PPU.M7byte | (Byte << 8);
				PPU.Need16x8Mulitply = TRUE;
				PPU.M7byte = Byte;
				break;

			case 0x211d: // M7C
				PPU.MatrixC = PPU.M7byte | (Byte << 8);
				PPU.M7byte = Byte;
				break;

			case 0x211e: // M7D
				PPU.MatrixD = PPU.M7byte | (Byte << 8);
				PPU.M7byte = Byte;
				break;

			case 0x211f: // M7X
				PPU.CentreX = PPU.M7byte | (Byte << 8);
				PPU.M7byte = Byte;
				break;

			case 0x2120: // M7Y
				PPU.CentreY = PPU.M7byte | (Byte << 8);
				PPU.M7byte = Byte;
				break;

			case 0x2121: // CGADD
				PPU.CGFLIP = 0;
				PPU.CGFLIPRead = 0;
				PPU.CGADD = Byte;
				break;

			case 0x2122: // CGDATA
				REGISTER_2122(Byte);
				break;

			case 0x2123: // W12SEL
				if (Byte != Memory.FillRAM[0x2123])
				{
					FLUSH_REDRAW();
					PPU.ClipWindow1Enable[0] = !!(Byte & 0x02);
					PPU.ClipWindow1Enable[1] = !!(Byte & 0x20);
					PPU.ClipWindow2Enable[0] = !!(Byte & 0x08);
					PPU.ClipWindow2Enable[1] = !!(Byte & 0x80);
					PPU.ClipWindow1Inside[0] = !(Byte & 0x01);
					PPU.ClipWindow1Inside[1] = !(Byte & 0x10);
					PPU.ClipWindow2Inside[0] = !(Byte & 0x04);
					PPU.ClipWindow2Inside[1] = !(Byte & 0x40);
					PPU.RecomputeClipWindows = TRUE;
				#ifdef DEBUGGER
					if (Byte & 0x80)
						missing.window2[1] = 1;
					if (Byte & 0x20)
						missing.window1[1] = 1;
					if (Byte & 0x08)
						missing.window2[0] = 1;
					if (Byte & 0x02)
						missing.window1[0] = 1;
				#endif
				}

				break;

			case 0x2124: // W34SEL
				if (Byte != Memory.FillRAM[0x2124])
				{
					FLUSH_REDRAW();
					PPU.ClipWindow1Enable[2] = !!(Byte & 0x02);
					PPU.ClipWindow1Enable[3] = !!(Byte & 0x20);
					PPU.ClipWindow2Enable[2] = !!(Byte & 0x08);
					PPU.ClipWindow2Enable[3] = !!(Byte & 0x80);
					PPU.ClipWindow1Inside[2] = !(Byte & 0x01);
					PPU.ClipWindow1Inside[3] = !(Byte & 0x10);
					PPU.ClipWindow2Inside[2] = !(Byte & 0x04);
					PPU.ClipWindow2Inside[3] = !(Byte & 0x40);
					PPU.RecomputeClipWindows = TRUE;
				#ifdef DEBUGGER
					if (Byte & 0x80)
						missing.window2[3] = 1;
					if (Byte & 0x20)
						missing.window1[3] = 1;
					if (Byte & 0x08)
						missing.window2[2] = 1;
					if (Byte & 0x02)
						missing.window1[2] = 1;
				#endif
				}

				break;

			case 0x2125: // WOBJSEL
				if (Byte != Memory.FillRAM[0x2125])
				{
					FLUSH_REDRAW();
					PPU.ClipWindow1Enable[4] = !!(Byte & 0x02);
					PPU.ClipWindow1Enable[5] = !!(Byte & 0x20);
					PPU.ClipWindow2Enable[4] = !!(Byte & 0x08);
					PPU.ClipWindow2Enable[5] = !!(Byte & 0x80);
					PPU.ClipWindow1Inside[4] = !(Byte & 0x01);
					PPU.ClipWindow1Inside[5] = !(Byte & 0x10);
					PPU.ClipWindow2Inside[4] = !(Byte & 0x04);
					PPU.ClipWindow2Inside[5] = !(Byte & 0x40);
					PPU.RecomputeClipWindows = TRUE;
				#ifdef DEBUGGER
					if (Byte & 0x80)
						missing.window2[5] = 1;
					if (Byte & 0x20)
						missing.window1[5] = 1;
					if (Byte & 0x08)
						missing.window2[4] = 1;
					if (Byte & 0x02)
						missing.window1[4] = 1;
				#endif
				}

				break;

			case 0x2126: // WH0
				if (Byte != Memory.FillRAM[0x2126])
				{
					FLUSH_REDRAW();
					PPU.Window1Left = Byte;
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x2127: // WH1
				if (Byte != Memory.FillRAM[0x2127])
				{
					FLUSH_REDRAW();
					PPU.Window1Right = Byte;
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x2128: // WH2
				if (Byte != Memory.FillRAM[0x2128])
				{
					FLUSH_REDRAW();
					PPU.Window2Left = Byte;
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x2129: // WH3
				if (Byte != Memory.FillRAM[0x2129])
				{
					FLUSH_REDRAW();
					PPU.Window2Right = Byte;
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x212a: // WBGLOG
				if (Byte != Memory.FillRAM[0x212a])
				{
					FLUSH_REDRAW();
					PPU.ClipWindowOverlapLogic[0] = (Byte & 0x03);
					PPU.ClipWindowOverlapLogic[1] = (Byte & 0x0c) >> 2;
					PPU.ClipWindowOverlapLogic[2] = (Byte & 0x30) >> 4;
					PPU.ClipWindowOverlapLogic[3] = (Byte & 0xc0) >> 6;
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x212b: // WOBJLOG
				if (Byte != Memory.FillRAM[0x212b])
				{
					FLUSH_REDRAW();
					PPU.ClipWindowOverlapLogic[4] = (Byte & 0x03);
					PPU.ClipWindowOverlapLogic[5] = (Byte & 0x0c) >> 2;
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x212c: // TM
				if (Byte != Memory.FillRAM[0x212c])
				{
					FLUSH_REDRAW();
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x212d: // TS
				if (Byte != Memory.FillRAM[0x212d])
				{
					FLUSH_REDRAW();
					PPU.RecomputeClipWindows = TRUE;
				#ifdef DEBUGGER
					if (Byte & 0x1f)
						missing.subscreen = 1;
				#endif
				}

				break;

			case 0x212e: // TMW
				if (Byte != Memory.FillRAM[0x212e])
				{
					FLUSH_REDRAW();
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x212f: // TSW
				if (Byte != Memory.FillRAM[0x212f])
				{
					FLUSH_REDRAW();
					PPU.RecomputeClipWindows = TRUE;
				}

				break;

			case 0x2130: // CGWSEL
				if (Byte != Memory.FillRAM[0x2130])
				{
					FLUSH_REDRAW();
					PPU.RecomputeClipWindows = TRUE;
				#ifdef DEBUGGER
					if ((Byte & 1) && (PPU.BGMode == 3 || PPU.BGMode == 4 || PPU.BGMode == 7))
						missing.direct = 1;
				#endif
				}

				break;

			case 0x2131: // CGADSUB
				if (Byte != Memory.FillRAM[0x2131])
				{
					FLUSH_REDRAW();
				#ifdef DEBUGGER
					if (Byte & 0x80)
					{
						if (Memory.FillRAM[0x2130] & 0x02)
							missing.subscreen_sub = 1;
						else
							missing.fixed_colour_sub = 1;
					}
					else
					{
						if (Memory.FillRAM[0x2130] & 0x02)
							missing.subscreen_add = 1;
						else
							missing.fixed_colour_add = 1;
					}
				#endif
				}

				break;

			case 0x2132: // COLDATA
				if (Byte != Memory.FillRAM[0x2132])
				{
					FLUSH_REDRAW();
					if (Byte & 0x80)
						PPU.FixedColourBlue  = Byte & 0x1f;
					if (Byte & 0x40)
						PPU.FixedColourGreen = Byte & 0x1f;
					if (Byte & 0x20)
						PPU.FixedColourRed   = Byte & 0x1f;
				}

				break;

			case 0x2133: // SETINI
				if (Byte != Memory.FillRAM[0x2133])
				{
					if ((Memory.FillRAM[0x2133] ^ Byte) & 8)
					{
						FLUSH_REDRAW();
						IPPU.PseudoHires = Byte & 8;
					}

					if (Byte & 0x04)
					{
						PPU.ScreenHeight = SNES_HEIGHT_EXTENDED;
						if (IPPU.DoubleHeightPixels)
							IPPU.RenderedScreenHeight = PPU.ScreenHeight << 1;
						else
							IPPU.RenderedScreenHeight = PPU.ScreenHeight;
					#ifdef DEBUGGER
						missing.lines_239 = 1;
					#endif
					}
					else
						PPU.ScreenHeight = SNES_HEIGHT;

					if ((Memory.FillRAM[0x2133] ^ Byte) & 3)
					{
						FLUSH_REDRAW();
						if ((Memory.FillRAM[0x2133] ^ Byte) & 2)
							IPPU.OBJChanged = TRUE;
						IPPU.Interlace = Byte & 1;
						IPPU.InterlaceOBJ = Byte & 2;
					}
				#ifdef DEBUGGER
					if (Byte & 0x40)
						missing.mode7_bgmode = 1;
					if (Byte & 0x08)
						missing.pseudo_512 = 1;
					if (Byte & 0x02)
						missing.sprite_double_height = 1;
					if (Byte & 0x01)
						missing.interlace = 1;
				#endif
				}

				break;

			case 0x2134: // MPYL
			case 0x2135: // MPYM
			case 0x2136: // MPYH
			case 0x2137: // SLHV
			case 0x2138: // OAMDATAREAD
			case 0x2139: // VMDATALREAD
			case 0x213a: // VMDATAHREAD
			case 0x213b: // CGDATAREAD
			case 0x213c: // OPHCT
			case 0x213d: // OPVCT
			case 0x213e: // STAT77
			case 0x213f: // STAT78
				return;

			case 0x2180: // WMDATA
				if (!CPU.InWRAMDMAorHDMA)
					REGISTER_2180(Byte);
				break;

			case 0x2181: // WMADDL
				if (!CPU.InWRAMDMAorHDMA)
				{
					PPU.WRAM &= 0x1ff00;
					PPU.WRAM |= Byte;
				}

				break;

			case 0x2182: // WMADDM
				if (!CPU.InWRAMDMAorHDMA)
				{
					PPU.WRAM &= 0x100ff;
					PPU.WRAM |= Byte << 8;
				}

				break;

			case 0x2183: // WMADDH
				if (!CPU.InWRAMDMAorHDMA)
				{
					PPU.WRAM &= 0x0ffff;
					PPU.WRAM |= Byte << 16;
					PPU.WRAM &= 0x1ffff;
				}

				break;
		}
	}
	else
	{
		if (Settings.SuperFX && Address >= 0x3000 && Address <= 0x32ff)
		{
			S9xSetSuperFX(Byte, Address);
			return;
		}
		else
		if (Settings.SA1     && Address >= 0x2200)
		{
			if (Address <= 0x23ff)
				S9xSetSA1(Byte, Address);
			else
				Memory.FillRAM[Address] = Byte;
			return;
		}
		else
		if (Settings.BS      && Address >= 0x2188 && Address <= 0x219f)
			S9xSetBSXPPU(Byte, Address);
		else
		if (Settings.SRTC    && Address == 0x2801)
			S9xSetSRTC(Byte, Address);
	#ifdef DEBUGGER
		else
		{
			missing.unknownppu_write = Address;
			if (Settings.TraceUnknownRegisters)
			{
				sprintf(String, "Unknown register write: $%02X->$%04X\n", Byte, Address);
				S9xMessage(S9X_TRACE, S9X_PPU_TRACE, String);
			}
		}
	#endif
	}

	Memory.FillRAM[Address] = Byte;
}

uint8 S9xGetPPU (uint16 Address)
{
	// MAP_PPU: $2000-$3FFF

	if (Address < 0x2100)
		return (OpenBus);

	if (CPU.InDMAorHDMA)
	{
		if (CPU.CurrentDMAorHDMAChannel >= 0 && !DMA[CPU.CurrentDMAorHDMAChannel].ReverseTransfer)
		{
			// S9xGetPPU() is called to read from DMA[].AAddress
			if ((Address & 0xff00) == 0x2100)
				// Cannot access to Address Bus B ($2100-$21FF) via (H)DMA
				return (OpenBus);
			else
				// $2200-$3FFF are connected to Address Bus A
				// SA1, SuperFX and SRTC are mapped here
				// I don't bother for now...
				return (OpenBus);
		}
		else
		{
			// S9xGetPPU() is called to write to $21xx
			// Take care of DMA wrapping
			if (Address > 0x21ff)
				Address = 0x2100 + (Address & 0xff);
		}
	}

	if ((Address & 0xffc0) == 0x2140) // APUIO0, APUIO1, APUIO2, APUIO3
		// read_port will run the APU until given APU time before reading value
		return (S9xAPUReadPort(Address & 3));
	else
	if (Address <= 0x2183)
    {
		uint8	byte;

		switch (Address)
		{
			case 0x2104: // OAMDATA
			case 0x2105: // BGMODE
			case 0x2106: // MOSAIC
			case 0x2108: // BG2SC
			case 0x2109: // BG3SC
			case 0x210a: // BG4SC
			case 0x2114: // BG4VOFS
			case 0x2115: // VMAIN
			case 0x2116: // VMADDL
			case 0x2118: // VMDATAL
			case 0x2119: // VMDATAH
			case 0x211a: // M7SEL
			case 0x2124: // W34SEL
			case 0x2125: // WOBJSEL
			case 0x2126: // WH0
			case 0x2128: // WH2
			case 0x2129: // WH3
			case 0x212a: // WBGLOG
				return (PPU.OpenBus1);

			case 0x2134: // MPYL
			case 0x2135: // MPYM
			case 0x2136: // MPYH
				if (PPU.Need16x8Mulitply)
				{
					int32 r = (int32) PPU.MatrixA * (int32) (PPU.MatrixB >> 8);
					Memory.FillRAM[0x2134] = (uint8) r;
					Memory.FillRAM[0x2135] = (uint8) (r >> 8);
					Memory.FillRAM[0x2136] = (uint8) (r >> 16);
					PPU.Need16x8Mulitply = FALSE;
				}
			#ifdef DEBUGGER
				missing.matrix_multiply = 1;
			#endif
				return (PPU.OpenBus1 = Memory.FillRAM[Address]);

			case 0x2137: // SLHV
				S9xLatchCounters(0);
				return (OpenBus);

			case 0x2138: // OAMDATAREAD
				if (PPU.OAMAddr & 0x100)
				{
					if (!(PPU.OAMFlip & 1))
						byte = PPU.OAMData[(PPU.OAMAddr & 0x10f) << 1];
					else
					{
						byte = PPU.OAMData[((PPU.OAMAddr & 0x10f) << 1) + 1];
						PPU.OAMAddr = (PPU.OAMAddr + 1) & 0x1ff;
						if (PPU.OAMPriorityRotation && PPU.FirstSprite != (PPU.OAMAddr >> 1))
						{
							PPU.FirstSprite = (PPU.OAMAddr & 0xfe) >> 1;
							IPPU.OBJChanged = TRUE;
						#ifdef DEBUGGER
							missing.sprite_priority_rotation = 1;
						#endif
						}
					}
				}
				else
				{
					if (!(PPU.OAMFlip & 1))
						byte = PPU.OAMData[PPU.OAMAddr << 1];
					else
					{
						byte = PPU.OAMData[(PPU.OAMAddr << 1) + 1];
						++PPU.OAMAddr;
						if (PPU.OAMPriorityRotation && PPU.FirstSprite != (PPU.OAMAddr >> 1))
						{
							PPU.FirstSprite = (PPU.OAMAddr & 0xfe) >> 1;
							IPPU.OBJChanged = TRUE;
						#ifdef DEBUGGER
							missing.sprite_priority_rotation = 1;
						#endif
						}
					}
				}

				PPU.OAMFlip ^= 1;
			#ifdef DEBUGGER
				missing.oam_read = 1;
			#endif
				return (PPU.OpenBus1 = byte);

			case 0x2139: // VMDATALREAD
				byte = IPPU.VRAMReadBuffer & 0xff;
				if (!PPU.VMA.High)
				{
					if (PPU.VMA.FullGraphicCount)
					{
						uint32 addr = PPU.VMA.Address;
						uint32 rem = addr & PPU.VMA.Mask1;
						uint32 address = (addr & ~PPU.VMA.Mask1) + (rem >> PPU.VMA.Shift) + ((rem & (PPU.VMA.FullGraphicCount - 1)) << 3);
						IPPU.VRAMReadBuffer = READ_WORD(Memory.VRAM + ((address << 1) & 0xffff));
					}
					else
						IPPU.VRAMReadBuffer = READ_WORD(Memory.VRAM + ((PPU.VMA.Address << 1) & 0xffff));

					PPU.VMA.Address += PPU.VMA.Increment;
				}

			#ifdef DEBUGGER
				missing.vram_read = 1;
			#endif
				return (PPU.OpenBus1 = byte);

			case 0x213a: // VMDATAHREAD
				byte = (IPPU.VRAMReadBuffer >> 8) & 0xff;
				if (PPU.VMA.High)
				{
					if (PPU.VMA.FullGraphicCount)
					{
						uint32 addr = PPU.VMA.Address;
						uint32 rem = addr & PPU.VMA.Mask1;
						uint32 address = (addr & ~PPU.VMA.Mask1) + (rem >> PPU.VMA.Shift) + ((rem & (PPU.VMA.FullGraphicCount - 1)) << 3);
						IPPU.VRAMReadBuffer = READ_WORD(Memory.VRAM + ((address << 1) & 0xffff));
					}
					else
						IPPU.VRAMReadBuffer = READ_WORD(Memory.VRAM + ((PPU.VMA.Address << 1) & 0xffff));

					PPU.VMA.Address += PPU.VMA.Increment;
				}
			#ifdef DEBUGGER
				missing.vram_read = 1;
			#endif
				return (PPU.OpenBus1 = byte);

			case 0x213b: // CGDATAREAD
				if (PPU.CGFLIPRead)
					byte = (PPU.OpenBus2 & 0x80) | ((PPU.CGDATA[PPU.CGADD++] >> 8) & 0x7f);
				else
					byte = PPU.CGDATA[PPU.CGADD] & 0xff;
				PPU.CGFLIPRead ^= 1;
			#ifdef DEBUGGER
				missing.cgram_read = 1;
			#endif
				return (PPU.OpenBus2 = byte);

			case 0x213c: // OPHCT
				S9xTryGunLatch(false);
				if (PPU.HBeamFlip)
					byte = (PPU.OpenBus2 & 0xfe) | ((PPU.HBeamPosLatched >> 8) & 0x01);
				else
					byte = (uint8) PPU.HBeamPosLatched;
				PPU.HBeamFlip ^= 1;
			#ifdef DEBUGGER
				missing.h_counter_read = 1;
			#endif
				return (PPU.OpenBus2 = byte);

			case 0x213d: // OPVCT
				S9xTryGunLatch(false);
				if (PPU.VBeamFlip)
					byte = (PPU.OpenBus2 & 0xfe) | ((PPU.VBeamPosLatched >> 8) & 0x01);
				else
					byte = (uint8) PPU.VBeamPosLatched;
				PPU.VBeamFlip ^= 1;
			#ifdef DEBUGGER
				missing.v_counter_read = 1;
			#endif
				return (PPU.OpenBus2 = byte);

			case 0x213e: // STAT77
				FLUSH_REDRAW();
				byte = (PPU.OpenBus1 & 0x10) | PPU.RangeTimeOver | Model->_5C77;
				return (PPU.OpenBus1 = byte);

			case 0x213f: // STAT78
				S9xTryGunLatch(false);
				PPU.VBeamFlip = PPU.HBeamFlip = 0;
				byte = (PPU.OpenBus2 & 0x20) | (Memory.FillRAM[0x213f] & 0xc0) | (Settings.PAL ? 0x10 : 0) | Model->_5C78;
				Memory.FillRAM[0x213f] &= ~0x40;
				return (PPU.OpenBus2 = byte);

			case 0x2180: // WMDATA
				if (!CPU.InWRAMDMAorHDMA)
				{
					byte = Memory.RAM[PPU.WRAM++];
					PPU.WRAM &= 0x1ffff;
				}
				else
					byte = OpenBus;
			#ifdef DEBUGGER
				missing.wram_read = 1;
			#endif
				return (byte);

			default:
				return (OpenBus);
		}
	}
	else
    {
		if (Settings.SuperFX && Address >= 0x3000 && Address <= 0x32ff)
			return (S9xGetSuperFX(Address));
		else
		if (Settings.SA1     && Address >= 0x2200)
			return (S9xGetSA1(Address));
		else
		if (Settings.BS      && Address >= 0x2188 && Address <= 0x219f)
			return (S9xGetBSXPPU(Address));
		else	
		if (Settings.SRTC    && Address == 0x2800)
			return (S9xGetSRTC(Address));
		else
		switch (Address)
		{
			case 0x21c2:
				if (Model->_5C77 == 2)
					return (0x20);
				return (OpenBus);

			case 0x21c3:
				if (Model->_5C77 == 2)
					return (0);
				return (OpenBus);

			default:
				return (OpenBus);
		}
	}
}

void S9xSetCPU (uint8 Byte, uint16 Address)
{
	if (Address < 0x4200)
	{
		switch (Address)
		{
			case 0x4016: // JOYSER0
				S9xSetJoypadLatch(Byte & 1);
				break;

			case 0x4017: // JOYSER1
				return;

			default:
				break;
		}
	}
	else
	if ((Address & 0xff80) == 0x4300)
	{
		if (CPU.InDMAorHDMA)
			return;

		int	d = (Address >> 4) & 0x7;

		switch (Address & 0xf)
		{
			case 0x0: // 0x43x0: DMAPx
				DMA[d].ReverseTransfer        = (Byte & 0x80) ? TRUE : FALSE;
				DMA[d].HDMAIndirectAddressing = (Byte & 0x40) ? TRUE : FALSE;
				DMA[d].UnusedBit43x0          = (Byte & 0x20) ? TRUE : FALSE;
				DMA[d].AAddressDecrement      = (Byte & 0x10) ? TRUE : FALSE;
				DMA[d].AAddressFixed          = (Byte & 0x08) ? TRUE : FALSE;
				DMA[d].TransferMode           = (Byte & 7);
				return;

			case 0x1: // 0x43x1: BBADx
				DMA[d].BAddress = Byte;
				return;

			case 0x2: // 0x43x2: A1TxL
				DMA[d].AAddress &= 0xff00;
				DMA[d].AAddress |= Byte;
				return;

			case 0x3: // 0x43x3: A1TxH
				DMA[d].AAddress &= 0xff;
				DMA[d].AAddress |= Byte << 8;
				return;

			case 0x4: // 0x43x4: A1Bx
				DMA[d].ABank = Byte;
				HDMAMemPointers[d] = NULL;
				return;

			case 0x5: // 0x43x5: DASxL
				DMA[d].DMACount_Or_HDMAIndirectAddress &= 0xff00;
				DMA[d].DMACount_Or_HDMAIndirectAddress |= Byte;
				HDMAMemPointers[d] = NULL;
				return;

			case 0x6: // 0x43x6: DASxH
				DMA[d].DMACount_Or_HDMAIndirectAddress &= 0xff;
				DMA[d].DMACount_Or_HDMAIndirectAddress |= Byte << 8;
				HDMAMemPointers[d] = NULL;
				return;

			case 0x7: // 0x43x7: DASBx
				DMA[d].IndirectBank = Byte;
				HDMAMemPointers[d] = NULL;
				return;

			case 0x8: // 0x43x8: A2AxL
				DMA[d].Address &= 0xff00;
				DMA[d].Address |= Byte;
				HDMAMemPointers[d] = NULL;
				return;

			case 0x9: // 0x43x9: A2AxH
				DMA[d].Address &= 0xff;
				DMA[d].Address |= Byte << 8;
				HDMAMemPointers[d] = NULL;
				return;

			case 0xa: // 0x43xa: NLTRx
				if (Byte & 0x7f)
				{
					DMA[d].LineCount = Byte & 0x7f;
					DMA[d].Repeat = !(Byte & 0x80);
				}
				else
				{
					DMA[d].LineCount = 128;
					DMA[d].Repeat = !!(Byte & 0x80);
				}

				return;

			case 0xb: // 0x43xb: ????x
			case 0xf: // 0x43xf: mirror of 0x43xb
				DMA[d].UnknownByte = Byte;
				return;

			default:
				break;
		}
	}
	else
	{
		uint16	pos;

		switch (Address)
		{
			case 0x4200: // NMITIMEN
				if (Byte & 0x20)
				{
					PPU.VTimerEnabled = TRUE;
				#ifdef DEBUGGER
					missing.virq = 1;
					missing.virq_pos = PPU.IRQVBeamPos;
				#endif
				}
				else
					PPU.VTimerEnabled = FALSE;

				if (Byte & 0x10)
				{
					PPU.HTimerEnabled = TRUE;
				#ifdef DEBUGGER
					missing.hirq = 1;
					missing.hirq_pos = PPU.IRQHBeamPos;
				#endif
				}
				else
					PPU.HTimerEnabled = FALSE;

				if (CPU.IRQLine && !PPU.HTimerEnabled && PPU.VTimerEnabled)
					CPU.IRQTransition = TRUE;

				if (!PPU.HTimerEnabled && !PPU.VTimerEnabled)
				{
					CPU.IRQLine = FALSE;
					CPU.IRQTransition = FALSE;
				}

				// NMI can trigger immediately during VBlank as long as NMI_read ($4210) wasn't cleard.
				if ((Byte & 0x80) && !(Memory.FillRAM[0x4200] & 0x80) &&
					(CPU.V_Counter >= PPU.ScreenHeight + FIRST_VISIBLE_LINE) && (Memory.FillRAM[0x4210] & 0x80))
				{
					// FIXME: triggered at HC+=6, checked just before the final CPU cycle,
					// then, when to call S9xOpcode_NMI()?
					CPU.NMILine = TRUE;
					Timings.NMITriggerPos = CPU.Cycles + 6 + 6;
				}

				break;

			case 0x4201: // WRIO
				if ((Byte & 0x80) == 0 && (Memory.FillRAM[0x4213] & 0x80) == 0x80)
					S9xLatchCounters(1);
				else
					S9xTryGunLatch((Byte & 0x80) ? true : false);
				Memory.FillRAM[0x4201] = Memory.FillRAM[0x4213] = Byte;
				break;

			case 0x4202: // WRMPYA
				break;

			case 0x4203: // WRMPYB
			{
				uint32 res = Memory.FillRAM[0x4202] * Byte;
				// FIXME: The update occurs 8 machine cycles after $4203 is set.
				Memory.FillRAM[0x4216] = (uint8) res;
				Memory.FillRAM[0x4217] = (uint8) (res >> 8);
				break;
			}

			case 0x4204: // WRDIVL
			case 0x4205: // WRDIVH
				break;

			case 0x4206: // WRDIVB
			{
				uint16 a = Memory.FillRAM[0x4204] + (Memory.FillRAM[0x4205] << 8);
				uint16 div = Byte ? a / Byte : 0xffff;
				uint16 rem = Byte ? a % Byte : a;
				// FIXME: The update occurs 16 machine cycles after $4206 is set.
				Memory.FillRAM[0x4214] = (uint8) div;
				Memory.FillRAM[0x4215] = div >> 8;
				Memory.FillRAM[0x4216] = (uint8) rem;
				Memory.FillRAM[0x4217] = rem >> 8;
				break;
			}

			case 0x4207: // HTIMEL
				pos = PPU.IRQHBeamPos;
				PPU.IRQHBeamPos = (PPU.IRQHBeamPos & 0xff00) | Byte;
				if (PPU.IRQHBeamPos != pos)
					S9xUpdateHVTimerPosition();
			#ifdef DEBUGGER
				missing.hirq_pos = PPU.IRQHBeamPos;
			#endif
				break;

			case 0x4208: // HTIMEH
				pos = PPU.IRQHBeamPos;
				PPU.IRQHBeamPos = (PPU.IRQHBeamPos & 0xff) | ((Byte & 1) << 8);
				if (PPU.IRQHBeamPos != pos)
					S9xUpdateHVTimerPosition();
			#ifdef DEBUGGER
				missing.hirq_pos = PPU.IRQHBeamPos;
			#endif
				break;

			case 0x4209: // VTIMEL
				pos = PPU.IRQVBeamPos;
				PPU.IRQVBeamPos = (PPU.IRQVBeamPos & 0xff00) | Byte;
				if (PPU.IRQVBeamPos != pos)
					S9xUpdateHVTimerPosition();
			#ifdef DEBUGGER
				missing.virq_pos = PPU.IRQVBeamPos;
			#endif
				break;

			case 0x420a: // VTIMEH
				pos = PPU.IRQVBeamPos;
				PPU.IRQVBeamPos = (PPU.IRQVBeamPos & 0xff) | ((Byte & 1) << 8);
				if (PPU.IRQVBeamPos != pos)
					S9xUpdateHVTimerPosition();
			#ifdef DEBUGGER
				missing.virq_pos = PPU.IRQVBeamPos;
			#endif
				break;

			case 0x420b: // MDMAEN
				if (CPU.InDMAorHDMA)
					return;
				// XXX: Not quite right...
				if (Byte)
					CPU.Cycles += Timings.DMACPUSync;
				if (Byte & 0x01)
					S9xDoDMA(0);
				if (Byte & 0x02)
					S9xDoDMA(1);
				if (Byte & 0x04)
					S9xDoDMA(2);
				if (Byte & 0x08)
					S9xDoDMA(3);
				if (Byte & 0x10)
					S9xDoDMA(4);
				if (Byte & 0x20)
					S9xDoDMA(5);
				if (Byte & 0x40)
					S9xDoDMA(6);
				if (Byte & 0x80)
					S9xDoDMA(7);
			#ifdef DEBUGGER
				missing.dma_this_frame = Byte;
				missing.dma_channels = Byte;
			#endif
				break;

			case 0x420c: // HDMAEN
				if (CPU.InDMAorHDMA)
					return;
				Memory.FillRAM[0x420c] = Byte;
				// Yoshi's Island, Genjyu Ryodan, Mortal Kombat, Tales of Phantasia
				PPU.HDMA = Byte & ~PPU.HDMAEnded;
			#ifdef DEBUGGER
				missing.hdma_this_frame |= Byte;
				missing.hdma_channels |= Byte;
			#endif
				break;

			case 0x420d: // MEMSEL
				if ((Byte & 1) != (Memory.FillRAM[0x420d] & 1))
				{
					if (Byte & 1)
					{
						CPU.FastROMSpeed = ONE_CYCLE;
					#ifdef DEBUGGER
						missing.fast_rom = 1;
					#endif
					}
					else
						CPU.FastROMSpeed = SLOW_ONE_CYCLE;
				}

				break;

			case 0x4210: // RDNMI
			case 0x4211: // TIMEUP
			case 0x4212: // HVBJOY
			case 0x4213: // RDIO
			case 0x4214: // RDDIVL
			case 0x4215: // RDDIVH
			case 0x4216: // RDMPYL
			case 0x4217: // RDMPYH
			case 0x4218: // JOY1L
			case 0x4219: // JOY1H
			case 0x421a: // JOY2L
			case 0x421b: // JOY2H
			case 0x421c: // JOY3L
			case 0x421d: // JOY3H
			case 0x421e: // JOY4L
			case 0x421f: // JOY4H
				return;

			default:
				if (Settings.SPC7110 && Address >= 0x4800)
					S9xSetSPC7110(Byte, Address);
				else
				if (Settings.SDD1 && Address >= 0x4804 && Address <= 0x4807)
					S9xSetSDD1MemoryMap(Address - 0x4804, Byte & 7);
				break;
		}
	}

	Memory.FillRAM[Address] = Byte;
}

uint8 S9xGetCPU (uint16 Address)
{
	if (Address < 0x4200)
	{
	#ifdef SNES_JOY_READ_CALLBACKS
		extern bool8 pad_read;
		if (Address == 0x4016 || Address == 0x4017)
		{
			S9xOnSNESPadRead();
			pad_read = TRUE;
		}
	#endif

		switch (Address)
		{
			case 0x4016: // JOYSER0
			case 0x4017: // JOYSER1
				return (S9xReadJOYSERn(Address));

			default:
				return (OpenBus);
		}
	}
	else
	if ((Address & 0xff80) == 0x4300)
	{
		if (CPU.InDMAorHDMA)
			return (OpenBus);

		int	d = (Address >> 4) & 0x7;

		switch (Address & 0xf)
		{
			case 0x0: // 0x43x0: DMAPx
				return ((DMA[d].ReverseTransfer        ? 0x80 : 0) |
						(DMA[d].HDMAIndirectAddressing ? 0x40 : 0) |
						(DMA[d].UnusedBit43x0          ? 0x20 : 0) |
						(DMA[d].AAddressDecrement      ? 0x10 : 0) |
						(DMA[d].AAddressFixed          ? 0x08 : 0) |
						(DMA[d].TransferMode & 7));

			case 0x1: // 0x43x1: BBADx
				return (DMA[d].BAddress);

			case 0x2: // 0x43x2: A1TxL
				return (DMA[d].AAddress & 0xff);

			case 0x3: // 0x43x3: A1TxH
				return (DMA[d].AAddress >> 8);

			case 0x4: // 0x43x4: A1Bx
				return (DMA[d].ABank);

			case 0x5: // 0x43x5: DASxL
				return (DMA[d].DMACount_Or_HDMAIndirectAddress & 0xff);

			case 0x6: // 0x43x6: DASxH
				return (DMA[d].DMACount_Or_HDMAIndirectAddress >> 8);

			case 0x7: // 0x43x7: DASBx
				return (DMA[d].IndirectBank);

			case 0x8: // 0x43x8: A2AxL
				return (DMA[d].Address & 0xff);

			case 0x9: // 0x43x9: A2AxH
				return (DMA[d].Address >> 8);

			case 0xa: // 0x43xa: NLTRx
				return (DMA[d].LineCount ^ (DMA[d].Repeat ? 0x00 : 0x80));

			case 0xb: // 0x43xb: ????x
			case 0xf: // 0x43xf: mirror of 0x43xb
				return (DMA[d].UnknownByte);

			default:
				return (OpenBus);
		}
	}
	else
	{
		uint8	byte;

		switch (Address)
		{
			case 0x4210: // RDNMI
				byte = Memory.FillRAM[0x4210];
				Memory.FillRAM[0x4210] = Model->_5A22;
				return ((byte & 0x80) | (OpenBus & 0x70) | Model->_5A22);

			case 0x4211: // TIMEUP
				byte = CPU.IRQLine ? 0x80 : 0;
				CPU.IRQLine = FALSE;
				CPU.IRQTransition = FALSE;
				return (byte | (OpenBus & 0x7f));

			case 0x4212: // HVBJOY
				return (REGISTER_4212() | (OpenBus & 0x3e));

			case 0x4213: // RDIO
				return (Memory.FillRAM[0x4213]);

			case 0x4214: // RDDIVL
			case 0x4215: // RDDIVH
			case 0x4216: // RDMPYL
			case 0x4217: // RDMPYH
				return (Memory.FillRAM[Address]);

			case 0x4218: // JOY1L
			case 0x4219: // JOY1H
			case 0x421a: // JOY2L
			case 0x421b: // JOY2H
			case 0x421c: // JOY3L
			case 0x421d: // JOY3H
			case 0x421e: // JOY4L
			case 0x421f: // JOY4H
			#ifdef SNES_JOY_READ_CALLBACKS
				extern bool8 pad_read;
				if (Memory.FillRAM[0x4200] & 1)
				{
					S9xOnSNESPadRead();
					pad_read = TRUE;
				}
			#endif
				return (Memory.FillRAM[Address]);

			default:
				if (Settings.SPC7110 && Address >= 0x4800)
					return (S9xGetSPC7110(Address));
				if (Settings.SDD1 && Address >= 0x4800 && Address <= 0x4807)
					return (Memory.FillRAM[Address]);
				return (OpenBus);
		}
	}
}

void S9xResetPPU (void)
{
	S9xSoftResetPPU();
	S9xControlsReset();
	PPU.M7HOFS = 0;
	PPU.M7VOFS = 0;
	PPU.M7byte = 0;
}

void S9xSoftResetPPU (void)
{
	S9xControlsSoftReset();

	PPU.VMA.High = 0;
	PPU.VMA.Increment = 1;
	PPU.VMA.Address = 0;
	PPU.VMA.FullGraphicCount = 0;
	PPU.VMA.Shift = 0;

	PPU.WRAM = 0;

	for (int c = 0; c < 4; c++)
	{
		PPU.BG[c].SCBase = 0;
		PPU.BG[c].HOffset = 0;
		PPU.BG[c].VOffset = 0;
		PPU.BG[c].BGSize = 0;
		PPU.BG[c].NameBase = 0;
		PPU.BG[c].SCSize = 0;
	}

	PPU.BGMode = 0;
	PPU.BG3Priority = 0;

	PPU.CGFLIP = 0;
	PPU.CGFLIPRead = 0;
	PPU.CGADD = 0;

	for (int c = 0; c < 256; c++)
	{
		IPPU.Red[c]   = (c & 7) << 2;
		IPPU.Green[c] = ((c >> 3) & 7) << 2;
		IPPU.Blue[c]  = ((c >> 6) & 2) << 3;
		PPU.CGDATA[c] = IPPU.Red[c] | (IPPU.Green[c] << 5) | (IPPU.Blue[c] << 10);
	}

	for (int c = 0; c < 128; c++)
	{
		PPU.OBJ[c].HPos = 0;
		PPU.OBJ[c].VPos = 0;
		PPU.OBJ[c].HFlip = 0;
		PPU.OBJ[c].VFlip = 0;
		PPU.OBJ[c].Name = 0;
		PPU.OBJ[c].Priority = 0;
		PPU.OBJ[c].Palette = 0;
		PPU.OBJ[c].Size = 0;
	}

	PPU.OBJThroughMain = FALSE;
	PPU.OBJThroughSub = FALSE;
	PPU.OBJAddition = FALSE;
	PPU.OBJNameBase = 0;
	PPU.OBJNameSelect = 0;
	PPU.OBJSizeSelect = 0;

	PPU.OAMAddr = 0;
	PPU.SavedOAMAddr = 0;
	PPU.OAMPriorityRotation = 0;
	PPU.OAMFlip = 0;
	PPU.OAMReadFlip = 0;
	PPU.OAMTileAddress = 0;
	PPU.OAMWriteRegister = 0;
	ZeroMemory(PPU.OAMData, 512 + 32);

	PPU.FirstSprite = 0;
	PPU.LastSprite = 127;
	PPU.RangeTimeOver = 0;

	PPU.HTimerEnabled = FALSE;
	PPU.VTimerEnabled = FALSE;
	PPU.HTimerPosition = Timings.H_Max + 1;
	PPU.VTimerPosition = Timings.V_Max + 1;
	PPU.IRQHBeamPos = 0x1ff;
	PPU.IRQVBeamPos = 0x1ff;

	PPU.HBeamFlip = 0;
	PPU.VBeamFlip = 0;
	PPU.HBeamPosLatched = 0;
	PPU.VBeamPosLatched = 0;
	PPU.GunHLatch = 0;
	PPU.GunVLatch = 1000;
	PPU.HVBeamCounterLatched = 0;

	PPU.Mode7HFlip = FALSE;
	PPU.Mode7VFlip = FALSE;
	PPU.Mode7Repeat = 0;
	PPU.MatrixA = 0;
	PPU.MatrixB = 0;
	PPU.MatrixC = 0;
	PPU.MatrixD = 0;
	PPU.CentreX = 0;
	PPU.CentreY = 0;

	PPU.Mosaic = 0;
	PPU.BGMosaic[0] = FALSE;
	PPU.BGMosaic[1] = FALSE;
	PPU.BGMosaic[2] = FALSE;
	PPU.BGMosaic[3] = FALSE;
	
	PPU.Window1Left = 1;
	PPU.Window1Right = 0;
	PPU.Window2Left = 1;
	PPU.Window2Right = 0;
	PPU.RecomputeClipWindows = TRUE;

	for (int c = 0; c < 6; c++)
	{
		PPU.ClipCounts[c] = 0;
		PPU.ClipWindowOverlapLogic[c] = CLIP_OR;
		PPU.ClipWindow1Enable[c] = FALSE;
		PPU.ClipWindow2Enable[c] = FALSE;
		PPU.ClipWindow1Inside[c] = TRUE;
		PPU.ClipWindow2Inside[c] = TRUE;
	}

	PPU.ForcedBlanking = TRUE;

	PPU.FixedColourRed = 0;
	PPU.FixedColourGreen = 0;
	PPU.FixedColourBlue = 0;
	PPU.Brightness = 0;
	PPU.ScreenHeight = SNES_HEIGHT;

	PPU.Need16x8Mulitply = FALSE;
	PPU.BGnxOFSbyte = 0;

	PPU.HDMA = 0;
	PPU.HDMAEnded = 0;

	PPU.OpenBus1 = 0;
	PPU.OpenBus2 = 0;

	for (int c = 0; c < 2; c++)
		memset(&IPPU.Clip[c], 0, sizeof(struct ClipData));
	IPPU.ColorsChanged = TRUE;
	IPPU.OBJChanged = TRUE;
	IPPU.DirectColourMapsNeedRebuild = TRUE;
	ZeroMemory(IPPU.TileCached[TILE_2BIT], MAX_2BIT_TILES);
	ZeroMemory(IPPU.TileCached[TILE_4BIT], MAX_4BIT_TILES);
	ZeroMemory(IPPU.TileCached[TILE_8BIT], MAX_8BIT_TILES);
	ZeroMemory(IPPU.TileCached[TILE_2BIT_EVEN], MAX_2BIT_TILES);
	ZeroMemory(IPPU.TileCached[TILE_2BIT_ODD],  MAX_2BIT_TILES);
	ZeroMemory(IPPU.TileCached[TILE_4BIT_EVEN], MAX_4BIT_TILES);
	ZeroMemory(IPPU.TileCached[TILE_4BIT_ODD],  MAX_4BIT_TILES);
	IPPU.VRAMReadBuffer = 0; // XXX: FIXME: anything better?
	IPPU.Interlace = FALSE;
	IPPU.InterlaceOBJ = FALSE;
	IPPU.DoubleWidthPixels = FALSE;
	IPPU.DoubleHeightPixels = FALSE;
	IPPU.CurrentLine = 0;
	IPPU.PreviousLine = 0;
	IPPU.XB = NULL;
	for (int c = 0; c < 256; c++)
		IPPU.ScreenColors[c] = c;
	IPPU.MaxBrightness = 0;
	IPPU.RenderThisFrame = TRUE;
	IPPU.RenderedScreenWidth = SNES_WIDTH;
	IPPU.RenderedScreenHeight = SNES_HEIGHT;
	IPPU.FrameCount = 0;
	IPPU.RenderedFramesCount = 0;
	IPPU.DisplayedRenderedFrameCount = 0;
	IPPU.SkippedFrames = 0;
	IPPU.FrameSkip = 0;

	S9xFixColourBrightness();

	for (int c = 0; c < 0x8000; c += 0x100)
		memset(&Memory.FillRAM[c], c >> 8, 0x100);
	ZeroMemory(&Memory.FillRAM[0x2100], 0x100);
	ZeroMemory(&Memory.FillRAM[0x4200], 0x100);
	ZeroMemory(&Memory.FillRAM[0x4000], 0x100);
	// For BS Suttehakkun 2...
	ZeroMemory(&Memory.FillRAM[0x1000], 0x1000);

	Memory.FillRAM[0x4201] = Memory.FillRAM[0x4213] = 0xff;
}
