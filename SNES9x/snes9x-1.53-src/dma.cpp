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
#include "sdd1emu.h"
#include "spc7110emu.h"
#ifdef DEBUGGER
#include "missing.h"
#endif

#define ADD_CYCLES(n)	{ CPU.PrevCycles = CPU.Cycles; CPU.Cycles += (n); S9xCheckInterrupts(); }

extern uint8	*HDMAMemPointers[8];
extern int		HDMA_ModeByteCounts[8];
extern SPC7110	s7emu;

static uint8	sdd1_decode_buffer[0x10000];

static inline bool8 addCyclesInDMA (uint8);
static inline bool8 HDMAReadLineCount (int);


static inline bool8 addCyclesInDMA (uint8 dma_channel)
{
	// Add 8 cycles per byte, sync APU, and do HC related events.
	// If HDMA was done in S9xDoHEventProcessing(), check if it used the same channel as DMA.
	ADD_CYCLES(SLOW_ONE_CYCLE);
	while (CPU.Cycles >= CPU.NextEvent)
		S9xDoHEventProcessing();

	if (CPU.HDMARanInDMA & (1 << dma_channel))
	{
		CPU.HDMARanInDMA = 0;
	#ifdef DEBUGGER
		printf("HDMA and DMA use the same channel %d!\n", dma_channel);
	#endif
		// If HDMA triggers in the middle of DMA transfer and it uses the same channel,
		// it kills the DMA transfer immediately. $43x2 and $43x5 stop updating.
		return (FALSE);
	}

	CPU.HDMARanInDMA = 0;
	return (TRUE);
}

bool8 S9xDoDMA (uint8 Channel)
{
	CPU.InDMA = TRUE;
    CPU.InDMAorHDMA = TRUE;
	CPU.CurrentDMAorHDMAChannel = Channel;

    SDMA	*d = &DMA[Channel];

	// Check invalid DMA first
	if ((d->ABank == 0x7E || d->ABank == 0x7F) && d->BAddress == 0x80 && !d->ReverseTransfer)
	{
		// Attempting a DMA from WRAM to $2180 will not work, WRAM will not be written.
		// Attempting a DMA from $2180 to WRAM will similarly not work,
		// the value written is (initially) the OpenBus value.
		// In either case, the address in $2181-3 is not incremented.

		// Does an invalid DMA actually take time?
		// I'd say yes, since 'invalid' is probably just the WRAM chip
		// not being able to read and write itself at the same time
		// And no, PPU.WRAM should not be updated.

		int32	c = d->TransferBytes;
		// Writing $0000 to $43x5 actually results in a transfer of $10000 bytes, not 0.
		if (c == 0)
			c = 0x10000;

		// 8 cycles per channel
		ADD_CYCLES(SLOW_ONE_CYCLE);
		// 8 cycles per byte
		while (c)
		{
			d->TransferBytes--;
			d->AAddress++;
			c--;
			if (!addCyclesInDMA(Channel))
			{
				CPU.InDMA = FALSE;
				CPU.InDMAorHDMA = FALSE;
				CPU.CurrentDMAorHDMAChannel = -1;
				return (FALSE);
			}
		}

	#ifdef DEBUGGER
		if (Settings.TraceDMA)
		{
			sprintf(String, "DMA[%d]: WRAM Bank:%02X->$2180", Channel, d->ABank);
			S9xMessage(S9X_TRACE, S9X_DMA_TRACE, String);
		}
	#endif

		CPU.InDMA = FALSE;
		CPU.InDMAorHDMA = FALSE;
		CPU.CurrentDMAorHDMAChannel = -1;
		return (TRUE);
	}

	// Prepare for accessing $2118-2119
	switch (d->BAddress)
	{
		case 0x18:
		case 0x19:
			if (IPPU.RenderThisFrame)
				FLUSH_REDRAW();
			break;
	}

	int32	inc = d->AAddressFixed ? 0 : (!d->AAddressDecrement ? 1 : -1);
	int32	count = d->TransferBytes;
	// Writing $0000 to $43x5 actually results in a transfer of $10000 bytes, not 0.
	if (count == 0)
		count = 0x10000;

	// Prepare for custom chip DMA

	// S-DD1

	uint8	*in_sdd1_dma = NULL;

	if (Settings.SDD1)
	{
		if (d->AAddressFixed && Memory.FillRAM[0x4801] > 0)
		{
			// XXX: Should probably verify that we're DMAing from ROM?
			// And somewhere we should make sure we're not running across a mapping boundary too.
			// Hacky support for pre-decompressed S-DD1 data
			inc = !d->AAddressDecrement ? 1 : -1;

			uint8	*in_ptr = S9xGetBasePointer(((d->ABank << 16) | d->AAddress));
			if (in_ptr)
			{
				in_ptr += d->AAddress;
				SDD1_decompress(sdd1_decode_buffer, in_ptr, d->TransferBytes);
			}
		#ifdef DEBUGGER
			else
			{
				sprintf(String, "S-DD1: DMA from non-block address $%02X:%04X", d->ABank, d->AAddress);
				S9xMessage(S9X_WARNING, S9X_DMA_TRACE, String);
			}
		#endif

			in_sdd1_dma = sdd1_decode_buffer;
		}

		Memory.FillRAM[0x4801] = 0;
	}

	// SPC7110

	uint8	*spc7110_dma = NULL;

	if (Settings.SPC7110)
	{
		if (d->AAddress == 0x4800 || d->ABank == 0x50)
		{
			spc7110_dma = new uint8[d->TransferBytes];
			for (int i = 0; i < d->TransferBytes; i++)
				spc7110_dma[i] = s7emu.decomp.read();

			int32	icount = s7emu.r4809 | (s7emu.r480a << 8);
			icount -= d->TransferBytes;
			s7emu.r4809 =  icount & 0x00ff;
			s7emu.r480a = (icount & 0xff00) >> 8;

			inc = 1;
			d->AAddress -= count;
		}
	}

	// SA-1

	bool8	in_sa1_dma = FALSE;

	if (Settings.SA1)
	{
		if (SA1.in_char_dma && d->BAddress == 0x18 && (d->ABank & 0xf0) == 0x40)
		{
			// Perform packed bitmap to PPU character format conversion on the data
			// before transmitting it to V-RAM via-DMA.
			int32	num_chars = 1 << ((Memory.FillRAM[0x2231] >> 2) & 7);
			int32	depth = (Memory.FillRAM[0x2231] & 3) == 0 ? 8 : (Memory.FillRAM[0x2231] & 3) == 1 ? 4 : 2;
			int32	bytes_per_char = 8 * depth;
			int32	bytes_per_line = depth * num_chars;
			int32	char_line_bytes = bytes_per_char * num_chars;
			uint32	addr = (d->AAddress / char_line_bytes) * char_line_bytes;

			uint8	*base = S9xGetBasePointer((d->ABank << 16) + addr);
			if (!base)
			{
				sprintf(String, "SA-1: DMA from non-block address $%02X:%04X", d->ABank, addr);
				S9xMessage(S9X_WARNING, S9X_DMA_TRACE, String);
				base = Memory.ROM;
			}

			base += addr;

			uint8	*buffer = &Memory.ROM[CMemory::MAX_ROM_SIZE - 0x10000];
			uint8	*p = buffer;
			uint32	inc_sa1 = char_line_bytes - (d->AAddress % char_line_bytes);
			uint32	char_count = inc_sa1 / bytes_per_char;

			in_sa1_dma = TRUE;

		#if 0
			printf("SA-1 DMA: %08x,", base);
			printf("depth = %d, count = %d, bytes_per_char = %d, bytes_per_line = %d, num_chars = %d, char_line_bytes = %d\n",
				depth, count, bytes_per_char, bytes_per_line, num_chars, char_line_bytes);
		#endif

			switch (depth)
			{
				case 2:
					for (int32 i = 0; i < count; i += inc_sa1, base += char_line_bytes, inc_sa1 = char_line_bytes, char_count = num_chars)
					{
						uint8	*line = base + (num_chars - char_count) * 2;
						for (uint32 j = 0; j < char_count && p - buffer < count; j++, line += 2)
						{
							uint8	*q = line;
							for (int32 l = 0; l < 8; l++, q += bytes_per_line)
							{
								for (int32 b = 0; b < 2; b++)
								{
									uint8	r = *(q + b);
									*(p + 0) = (*(p + 0) << 1) | ((r >> 0) & 1);
									*(p + 1) = (*(p + 1) << 1) | ((r >> 1) & 1);
									*(p + 0) = (*(p + 0) << 1) | ((r >> 2) & 1);
									*(p + 1) = (*(p + 1) << 1) | ((r >> 3) & 1);
									*(p + 0) = (*(p + 0) << 1) | ((r >> 4) & 1);
									*(p + 1) = (*(p + 1) << 1) | ((r >> 5) & 1);
									*(p + 0) = (*(p + 0) << 1) | ((r >> 6) & 1);
									*(p + 1) = (*(p + 1) << 1) | ((r >> 7) & 1);
								}

								p += 2;
							}
						}
					}

					break;

				case 4:
					for (int32 i = 0; i < count; i += inc_sa1, base += char_line_bytes, inc_sa1 = char_line_bytes, char_count = num_chars)
					{
						uint8	*line = base + (num_chars - char_count) * 4;
						for (uint32 j = 0; j < char_count && p - buffer < count; j++, line += 4)
						{
							uint8	*q = line;
							for (int32 l = 0; l < 8; l++, q += bytes_per_line)
							{
								for (int32 b = 0; b < 4; b++)
								{
									uint8	r = *(q + b);
									*(p +  0) = (*(p +  0) << 1) | ((r >> 0) & 1);
									*(p +  1) = (*(p +  1) << 1) | ((r >> 1) & 1);
									*(p + 16) = (*(p + 16) << 1) | ((r >> 2) & 1);
									*(p + 17) = (*(p + 17) << 1) | ((r >> 3) & 1);
									*(p +  0) = (*(p +  0) << 1) | ((r >> 4) & 1);
									*(p +  1) = (*(p +  1) << 1) | ((r >> 5) & 1);
									*(p + 16) = (*(p + 16) << 1) | ((r >> 6) & 1);
									*(p + 17) = (*(p + 17) << 1) | ((r >> 7) & 1);
								}

								p += 2;
							}

							p += 32 - 16;
						}
					}

					break;

				case 8:
					for (int32 i = 0; i < count; i += inc_sa1, base += char_line_bytes, inc_sa1 = char_line_bytes, char_count = num_chars)
					{
						uint8	*line = base + (num_chars - char_count) * 8;
						for (uint32 j = 0; j < char_count && p - buffer < count; j++, line += 8)
						{
							uint8	*q = line;
							for (int32 l = 0; l < 8; l++, q += bytes_per_line)
							{
								for (int32 b = 0; b < 8; b++)
								{
									uint8	r = *(q + b);
									*(p +  0) = (*(p +  0) << 1) | ((r >> 0) & 1);
									*(p +  1) = (*(p +  1) << 1) | ((r >> 1) & 1);
									*(p + 16) = (*(p + 16) << 1) | ((r >> 2) & 1);
									*(p + 17) = (*(p + 17) << 1) | ((r >> 3) & 1);
									*(p + 32) = (*(p + 32) << 1) | ((r >> 4) & 1);
									*(p + 33) = (*(p + 33) << 1) | ((r >> 5) & 1);
									*(p + 48) = (*(p + 48) << 1) | ((r >> 6) & 1);
									*(p + 49) = (*(p + 49) << 1) | ((r >> 7) & 1);
								}

								p += 2;
							}

							p += 64 - 16;
						}
					}

					break;
			}
		}
	}

#ifdef DEBUGGER
	if (Settings.TraceDMA)
	{
		sprintf(String, "DMA[%d]: %s Mode:%d 0x%02X%04X->0x21%02X Bytes:%d (%s) V:%03d",
			Channel, d->ReverseTransfer ? "PPU->CPU" : "CPU->PPU", d->TransferMode, d->ABank, d->AAddress, d->BAddress,
			d->TransferBytes, d->AAddressFixed ? "fixed" : (d->AAddressDecrement ? "dec" : "inc"), CPU.V_Counter);

		if (d->BAddress == 0x18 || d->BAddress == 0x19 || d->BAddress == 0x39 || d->BAddress == 0x3a)
			sprintf(String, "%s VRAM: %04X (%d,%d) %s", String,
				PPU.VMA.Address, PPU.VMA.Increment, PPU.VMA.FullGraphicCount, PPU.VMA.High ? "word" : "byte");
		else
		if (d->BAddress == 0x22 || d->BAddress == 0x3b)
			sprintf(String, "%s CGRAM: %02X (%x)", String, PPU.CGADD, PPU.CGFLIP);
		else
		if (d->BAddress == 0x04 || d->BAddress == 0x38)
			sprintf(String, "%s OBJADDR: %04X", String, PPU.OAMAddr);

		S9xMessage(S9X_TRACE, S9X_DMA_TRACE, String);
	}
#endif

	// Do Transfer

	uint8	Work;

	// 8 cycles per channel
	ADD_CYCLES(SLOW_ONE_CYCLE);

	if (!d->ReverseTransfer)
    {
		// CPU -> PPU
		int32	b = 0;
		uint16	p = d->AAddress;
		uint8	*base = S9xGetBasePointer((d->ABank << 16) + d->AAddress);
		bool8	inWRAM_DMA;

		int32	rem = count;
		// Transfer per block if d->AAdressFixed is FALSE
		count = d->AAddressFixed ? rem : (d->AAddressDecrement ? ((p & MEMMAP_MASK) + 1) : (MEMMAP_BLOCK_SIZE - (p & MEMMAP_MASK)));

		// Settings for custom chip DMA
		if (in_sa1_dma)
		{
			base = &Memory.ROM[CMemory::MAX_ROM_SIZE - 0x10000];
			p = 0;
			count = rem;
		}
		else
		if (in_sdd1_dma)
		{
			base = in_sdd1_dma;
			p = 0;
			count = rem;
		}
		else
		if (spc7110_dma)
		{
			base = spc7110_dma;
			p = 0;
			count = rem;
		}

		inWRAM_DMA = ((!in_sa1_dma && !in_sdd1_dma && !spc7110_dma) &&
			(d->ABank == 0x7e || d->ABank == 0x7f || (!(d->ABank & 0x40) && d->AAddress < 0x2000)));

		// 8 cycles per byte
		#define	UPDATE_COUNTERS \
			d->TransferBytes--; \
			d->AAddress += inc; \
			p += inc; \
			if (!addCyclesInDMA(Channel)) \
			{ \
				CPU.InDMA = FALSE; \
				CPU.InDMAorHDMA = FALSE; \
				CPU.InWRAMDMAorHDMA = FALSE; \
				CPU.CurrentDMAorHDMAChannel = -1; \
				return (FALSE); \
			}

		while (1)
		{
			if (count > rem)
				count = rem;
			rem -= count;

			CPU.InWRAMDMAorHDMA = inWRAM_DMA;

			if (!base)
			{
				// DMA SLOW PATH
				if (d->TransferMode == 0 || d->TransferMode == 2 || d->TransferMode == 6)
				{
					do
					{
						Work = S9xGetByte((d->ABank << 16) + p);
						S9xSetPPU(Work, 0x2100 + d->BAddress);
						UPDATE_COUNTERS;
					} while (--count > 0);
				}
				else
				if (d->TransferMode == 1 || d->TransferMode == 5)
				{
					// This is a variation on Duff's Device. It is legal C/C++.
					switch (b)
					{
						default:
						while (count > 1)
						{
							Work = S9xGetByte((d->ABank << 16) + p);
							S9xSetPPU(Work, 0x2100 + d->BAddress);
							UPDATE_COUNTERS;
							count--;

						case 1:
							Work = S9xGetByte((d->ABank << 16) + p);
							S9xSetPPU(Work, 0x2101 + d->BAddress);
							UPDATE_COUNTERS;
							count--;
						}
					}

					if (count == 1)
					{
						Work = S9xGetByte((d->ABank << 16) + p);
						S9xSetPPU(Work, 0x2100 + d->BAddress);
						UPDATE_COUNTERS;
						b = 1;
					}
					else
						b = 0;
				}
				else
				if (d->TransferMode == 3 || d->TransferMode == 7)
				{
					switch (b)
					{
						default:
						do
						{
							Work = S9xGetByte((d->ABank << 16) + p);
							S9xSetPPU(Work, 0x2100 + d->BAddress);
							UPDATE_COUNTERS;
							if (--count <= 0)
							{
								b = 1;
								break;
							}

						case 1:
							Work = S9xGetByte((d->ABank << 16) + p);
							S9xSetPPU(Work, 0x2100 + d->BAddress);
							UPDATE_COUNTERS;
							if (--count <= 0)
							{
								b = 2;
								break;
							}

						case 2:
							Work = S9xGetByte((d->ABank << 16) + p);
							S9xSetPPU(Work, 0x2101 + d->BAddress);
							UPDATE_COUNTERS;
							if (--count <= 0)
							{
								b = 3;
								break;
							}

						case 3:
							Work = S9xGetByte((d->ABank << 16) + p);
							S9xSetPPU(Work, 0x2101 + d->BAddress);
							UPDATE_COUNTERS;
							if (--count <= 0)
							{
								b = 0;
								break;
							}
						} while (1);
					}
				}
				else
				if (d->TransferMode == 4)
				{
					switch (b)
					{
						default:
						do
						{
							Work = S9xGetByte((d->ABank << 16) + p);
							S9xSetPPU(Work, 0x2100 + d->BAddress);
							UPDATE_COUNTERS;
							if (--count <= 0)
							{
								b = 1;
								break;
							}

						case 1:
							Work = S9xGetByte((d->ABank << 16) + p);
							S9xSetPPU(Work, 0x2101 + d->BAddress);
							UPDATE_COUNTERS;
							if (--count <= 0)
							{
								b = 2;
								break;
							}

						case 2:
							Work = S9xGetByte((d->ABank << 16) + p);
							S9xSetPPU(Work, 0x2102 + d->BAddress);
							UPDATE_COUNTERS;
							if (--count <= 0)
							{
								b = 3;
								break;
							}

						case 3:
							Work = S9xGetByte((d->ABank << 16) + p);
							S9xSetPPU(Work, 0x2103 + d->BAddress);
							UPDATE_COUNTERS;
							if (--count <= 0)
							{
								b = 0;
								break;
							}
						} while (1);
					}
				}
			#ifdef DEBUGGER
				else
				{
					sprintf(String, "Unknown DMA transfer mode: %d on channel %d\n", d->TransferMode, Channel);
					S9xMessage(S9X_TRACE, S9X_DMA_TRACE, String);
				}
			#endif
			}
			else
			{
				// DMA FAST PATH
				if (d->TransferMode == 0 || d->TransferMode == 2 || d->TransferMode == 6)
				{
					switch (d->BAddress)
					{
						case 0x04: // OAMDATA
							do
							{
								Work = *(base + p);
								REGISTER_2104(Work);
								UPDATE_COUNTERS;
							} while (--count > 0);

							break;

						case 0x18: // VMDATAL
							if (!PPU.VMA.FullGraphicCount)
							{
								do
								{
									Work = *(base + p);
									REGISTER_2118_linear(Work);
									UPDATE_COUNTERS;
								} while (--count > 0);
							}
							else
							{
								do
								{
									Work = *(base + p);
									REGISTER_2118_tile(Work);
									UPDATE_COUNTERS;
								} while (--count > 0);
							}

							break;

						case 0x19: // VMDATAH
							if (!PPU.VMA.FullGraphicCount)
							{
								do
								{
									Work = *(base + p);
									REGISTER_2119_linear(Work);
									UPDATE_COUNTERS;
								} while (--count > 0);
							}
							else
							{
								do
								{
									Work = *(base + p);
									REGISTER_2119_tile(Work);
									UPDATE_COUNTERS;
								} while (--count > 0);
							}

							break;

						case 0x22: // CGDATA
							do
							{
								Work = *(base + p);
								REGISTER_2122(Work);
								UPDATE_COUNTERS;
							} while (--count > 0);

							break;

						case 0x80: // WMDATA
							if (!CPU.InWRAMDMAorHDMA)
							{
								do
								{
									Work = *(base + p);
									REGISTER_2180(Work);
									UPDATE_COUNTERS;
								} while (--count > 0);
							}
							else
							{
								do
								{
									UPDATE_COUNTERS;
								} while (--count > 0);
							}

							break;

						default:
							do
							{
								Work = *(base + p);
								S9xSetPPU(Work, 0x2100 + d->BAddress);
								UPDATE_COUNTERS;
							} while (--count > 0);

							break;
					}
				}
				else
				if (d->TransferMode == 1 || d->TransferMode == 5)
				{
					if (d->BAddress == 0x18)
					{
						// VMDATAL
						if (!PPU.VMA.FullGraphicCount)
						{
							switch (b)
							{
								default:
								while (count > 1)
								{
									Work = *(base + p);
									REGISTER_2118_linear(Work);
									UPDATE_COUNTERS;
									count--;

								case 1:
									Work = *(base + p);
									REGISTER_2119_linear(Work);
									UPDATE_COUNTERS;
									count--;
								}
							}

							if (count == 1)
							{
								Work = *(base + p);
								REGISTER_2118_linear(Work);
								UPDATE_COUNTERS;
								b = 1;
							}
							else
								b = 0;
						}
						else
						{
							switch (b)
							{
								default:
								while (count > 1)
								{
									Work = *(base + p);
									REGISTER_2118_tile(Work);
									UPDATE_COUNTERS;
									count--;

								case 1:
									Work = *(base + p);
									REGISTER_2119_tile(Work);
									UPDATE_COUNTERS;
									count--;
								}
							}

							if (count == 1)
							{
								Work = *(base + p);
								REGISTER_2118_tile(Work);
								UPDATE_COUNTERS;
								b = 1;
							}
							else
								b = 0;
						}
					}
					else
					{
						// DMA mode 1 general case
						switch (b)
						{
							default:
							while (count > 1)
							{
								Work = *(base + p);
								S9xSetPPU(Work, 0x2100 + d->BAddress);
								UPDATE_COUNTERS;
								count--;

							case 1:
								Work = *(base + p);
								S9xSetPPU(Work, 0x2101 + d->BAddress);
								UPDATE_COUNTERS;
								count--;
							}
						}

						if (count == 1)
						{
							Work = *(base + p);
							S9xSetPPU(Work, 0x2100 + d->BAddress);
							UPDATE_COUNTERS;
							b = 1;
						}
						else
							b = 0;
					}
				}
				else
				if (d->TransferMode == 3 || d->TransferMode == 7)
				{
					switch (b)
					{
						default:
						do
						{
							Work = *(base + p);
							S9xSetPPU(Work, 0x2100 + d->BAddress);
							UPDATE_COUNTERS;
							if (--count <= 0)
							{
								b = 1;
								break;
							}

						case 1:
							Work = *(base + p);
							S9xSetPPU(Work, 0x2100 + d->BAddress);
							UPDATE_COUNTERS;
							if (--count <= 0)
							{
								b = 2;
								break;
							}

						case 2:
							Work = *(base + p);
							S9xSetPPU(Work, 0x2101 + d->BAddress);
							UPDATE_COUNTERS;
							if (--count <= 0)
							{
								b = 3;
								break;
							}

						case 3:
							Work = *(base + p);
							S9xSetPPU(Work, 0x2101 + d->BAddress);
							UPDATE_COUNTERS;
							if (--count <= 0)
							{
								b = 0;
								break;
							}
						} while (1);
					}
				}
				else
				if (d->TransferMode == 4)
				{
					switch (b)
					{
						default:
						do
						{
							Work = *(base + p);
							S9xSetPPU(Work, 0x2100 + d->BAddress);
							UPDATE_COUNTERS;
							if (--count <= 0)
							{
								b = 1;
								break;
							}

						case 1:
							Work = *(base + p);
							S9xSetPPU(Work, 0x2101 + d->BAddress);
							UPDATE_COUNTERS;
							if (--count <= 0)
							{
								b = 2;
								break;
							}

						case 2:
							Work = *(base + p);
							S9xSetPPU(Work, 0x2102 + d->BAddress);
							UPDATE_COUNTERS;
							if (--count <= 0)
							{
								b = 3;
								break;
							}

						case 3:
							Work = *(base + p);
							S9xSetPPU(Work, 0x2103 + d->BAddress);
							UPDATE_COUNTERS;
							if (--count <= 0)
							{
								b = 0;
								break;
							}
						} while (1);
					}
				}
			#ifdef DEBUGGER
				else
				{
					sprintf(String, "Unknown DMA transfer mode: %d on channel %d\n", d->TransferMode, Channel);
					S9xMessage(S9X_TRACE, S9X_DMA_TRACE, String);
				}
			#endif
			}

			if (rem <= 0)
				break;

			base = S9xGetBasePointer((d->ABank << 16) + d->AAddress);
			count = MEMMAP_BLOCK_SIZE;
			inWRAM_DMA = ((!in_sa1_dma && !in_sdd1_dma && !spc7110_dma) &&
				(d->ABank == 0x7e || d->ABank == 0x7f || (!(d->ABank & 0x40) && d->AAddress < 0x2000)));
		}

		#undef UPDATE_COUNTERS
	}
    else
    {
		// PPU -> CPU

		// 8 cycles per byte
		#define	UPDATE_COUNTERS \
			d->TransferBytes--; \
			d->AAddress += inc; \
			if (!addCyclesInDMA(Channel)) \
			{ \
				CPU.InDMA = FALSE; \
				CPU.InDMAorHDMA = FALSE; \
				CPU.InWRAMDMAorHDMA = FALSE; \
				CPU.CurrentDMAorHDMAChannel = -1; \
				return (FALSE); \
			}

		if (d->BAddress > 0x80 - 4 && d->BAddress <= 0x83 && !(d->ABank & 0x40))
		{
			// REVERSE-DMA REALLY-SLOW PATH
			do
			{
				switch (d->TransferMode)
				{
					case 0:
					case 2:
					case 6:
						CPU.InWRAMDMAorHDMA = (d->AAddress < 0x2000);
						Work = S9xGetPPU(0x2100 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						count--;

						break;

					case 1:
					case 5:
						CPU.InWRAMDMAorHDMA = (d->AAddress < 0x2000);
						Work = S9xGetPPU(0x2100 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						if (!--count)
							break;

						CPU.InWRAMDMAorHDMA = (d->AAddress < 0x2000);
						Work = S9xGetPPU(0x2101 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						count--;

						break;

					case 3:
					case 7:
						CPU.InWRAMDMAorHDMA = (d->AAddress < 0x2000);
						Work = S9xGetPPU(0x2100 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						if (!--count)
							break;

						CPU.InWRAMDMAorHDMA = (d->AAddress < 0x2000);
						Work = S9xGetPPU(0x2100 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						if (!--count)
							break;

						CPU.InWRAMDMAorHDMA = (d->AAddress < 0x2000);
						Work = S9xGetPPU(0x2101 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						if (!--count)
							break;

						CPU.InWRAMDMAorHDMA = (d->AAddress < 0x2000);
						Work = S9xGetPPU(0x2101 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						count--;

						break;

					case 4:
						CPU.InWRAMDMAorHDMA = (d->AAddress < 0x2000);
						Work = S9xGetPPU(0x2100 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						if (!--count)
							break;

						CPU.InWRAMDMAorHDMA = (d->AAddress < 0x2000);
						Work = S9xGetPPU(0x2101 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						if (!--count)
							break;

						CPU.InWRAMDMAorHDMA = (d->AAddress < 0x2000);
						Work = S9xGetPPU(0x2102 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						if (!--count)
							break;

						CPU.InWRAMDMAorHDMA = (d->AAddress < 0x2000);
						Work = S9xGetPPU(0x2103 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						count--;

						break;

					default:
					#ifdef DEBUGGER
						sprintf(String, "Unknown DMA transfer mode: %d on channel %d\n", d->TransferMode, Channel);
						S9xMessage(S9X_TRACE, S9X_DMA_TRACE, String);
					#endif
						while (count)
						{
							UPDATE_COUNTERS;
							count--;
						}

						break;
				}
			} while (count);
		}
		else
		{
			// REVERSE-DMA FASTER PATH
			CPU.InWRAMDMAorHDMA = (d->ABank == 0x7e || d->ABank == 0x7f);
			do
			{
				switch (d->TransferMode)
				{
					case 0:
					case 2:
					case 6:
						Work = S9xGetPPU(0x2100 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						count--;

						break;

					case 1:
					case 5:
						Work = S9xGetPPU(0x2100 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						if (!--count)
							break;

						Work = S9xGetPPU(0x2101 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						count--;

						break;

					case 3:
					case 7:
						Work = S9xGetPPU(0x2100 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						if (!--count)
							break;

						Work = S9xGetPPU(0x2100 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						if (!--count)
							break;

						Work = S9xGetPPU(0x2101 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						if (!--count)
							break;

						Work = S9xGetPPU(0x2101 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						count--;

						break;

					case 4:
						Work = S9xGetPPU(0x2100 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						if (!--count)
							break;

						Work = S9xGetPPU(0x2101 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						if (!--count)
							break;

						Work = S9xGetPPU(0x2102 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						if (!--count)
							break;

						Work = S9xGetPPU(0x2103 + d->BAddress);
						S9xSetByte(Work, (d->ABank << 16) + d->AAddress);
						UPDATE_COUNTERS;
						count--;

						break;

					default:
					#ifdef DEBUGGER
						sprintf(String, "Unknown DMA transfer mode: %d on channel %d\n", d->TransferMode, Channel);
						S9xMessage(S9X_TRACE, S9X_DMA_TRACE, String);
					#endif
						while (count)
						{
							UPDATE_COUNTERS;
							count--;
						}

						break;
				}
			} while (count);
		}
	}

	if (CPU.NMILine && (Timings.NMITriggerPos != 0xffff))
	{
		Timings.NMITriggerPos = CPU.Cycles + Timings.NMIDMADelay;
		if (Timings.NMITriggerPos >= Timings.H_Max)
			Timings.NMITriggerPos -= Timings.H_Max;
	}

	// Release the memory used in SPC7110 DMA
    if (Settings.SPC7110)
    {
        if (spc7110_dma)
            delete [] spc7110_dma;
    }

#if 0
	// sanity check
    if (d->TransferBytes != 0)
		fprintf(stderr,"DMA[%d] TransferBytes not 0! $21%02x Reverse:%d %04x\n", Channel, d->BAddress, d->ReverseTransfer, d->TransferBytes);
#endif

	CPU.InDMA = FALSE;
	CPU.InDMAorHDMA = FALSE;
	CPU.InWRAMDMAorHDMA = FALSE;
	CPU.CurrentDMAorHDMAChannel = -1;

	return (TRUE);
}

static inline bool8 HDMAReadLineCount (int d)
{
	// CPU.InDMA is set, so S9xGetXXX() / S9xSetXXX() incur no charges.

	uint8	line;

	line = S9xGetByte((DMA[d].ABank << 16) + DMA[d].Address);
	ADD_CYCLES(SLOW_ONE_CYCLE);

	if (!line)
	{
		DMA[d].Repeat = FALSE;
		DMA[d].LineCount = 128;

		if (DMA[d].HDMAIndirectAddressing)
		{
			if (PPU.HDMA & (0xfe << d))
			{
				DMA[d].Address++;
				ADD_CYCLES(SLOW_ONE_CYCLE << 1);
			}
			else
				ADD_CYCLES(SLOW_ONE_CYCLE);

			DMA[d].IndirectAddress = S9xGetWord((DMA[d].ABank << 16) + DMA[d].Address);
			DMA[d].Address++;
		}

		DMA[d].Address++;
		HDMAMemPointers[d] = NULL;

		return (FALSE);
	}
	else
	if (line == 0x80)
	{
		DMA[d].Repeat = TRUE;
		DMA[d].LineCount = 128;
	}
	else
	{
		DMA[d].Repeat = !(line & 0x80);
		DMA[d].LineCount = line & 0x7f;
	}

	DMA[d].Address++;
	DMA[d].DoTransfer = TRUE;

	if (DMA[d].HDMAIndirectAddressing)
	{
		ADD_CYCLES(SLOW_ONE_CYCLE << 1);
		DMA[d].IndirectAddress = S9xGetWord((DMA[d].ABank << 16) + DMA[d].Address);
		DMA[d].Address += 2;
		HDMAMemPointers[d] = S9xGetMemPointer((DMA[d].IndirectBank << 16) + DMA[d].IndirectAddress);
	}
	else
		HDMAMemPointers[d] = S9xGetMemPointer((DMA[d].ABank << 16) + DMA[d].Address);

	return (TRUE);
}

void S9xStartHDMA (void)
{
	PPU.HDMA = Memory.FillRAM[0x420c];

#ifdef DEBUGGER
	missing.hdma_this_frame = PPU.HDMA;
#endif

	PPU.HDMAEnded = 0;

	int32	tmpch;

	CPU.InHDMA = TRUE;
	CPU.InDMAorHDMA = TRUE;
	tmpch = CPU.CurrentDMAorHDMAChannel;

	// XXX: Not quite right...
	if (PPU.HDMA != 0)
		ADD_CYCLES(Timings.DMACPUSync);

	for (uint8 i = 0; i < 8; i++)
	{
		if (PPU.HDMA & (1 << i))
		{
			CPU.CurrentDMAorHDMAChannel = i;

			DMA[i].Address = DMA[i].AAddress;

			if (!HDMAReadLineCount(i))
			{
				PPU.HDMA &= ~(1 << i);
				PPU.HDMAEnded |= (1 << i);
			}
		}
		else
			DMA[i].DoTransfer = FALSE;
	}

	CPU.InHDMA = FALSE;
	CPU.InDMAorHDMA = CPU.InDMA;
	CPU.HDMARanInDMA = CPU.InDMA ? PPU.HDMA : 0;
	CPU.CurrentDMAorHDMAChannel = tmpch;
}

uint8 S9xDoHDMA (uint8 byte)
{
	struct SDMA	*p = &DMA[0];

	uint32	ShiftedIBank;
	uint16	IAddr;
	bool8	temp;
	int32	tmpch;
	int		d = 0;

	CPU.InHDMA = TRUE;
	CPU.InDMAorHDMA = TRUE;
	CPU.HDMARanInDMA = CPU.InDMA ? byte : 0;
	temp = CPU.InWRAMDMAorHDMA;
	tmpch = CPU.CurrentDMAorHDMAChannel;

	// XXX: Not quite right...
	ADD_CYCLES(Timings.DMACPUSync);

	for (uint8 mask = 1; mask; mask <<= 1, p++, d++)
	{
		if (byte & mask)
		{
			CPU.InWRAMDMAorHDMA = FALSE;
			CPU.CurrentDMAorHDMAChannel = d;

			if (p->HDMAIndirectAddressing)
			{
				ShiftedIBank = (p->IndirectBank << 16);
				IAddr = p->IndirectAddress;
			}
			else
			{
				ShiftedIBank = (p->ABank << 16);
				IAddr = p->Address;
			}

			if (!HDMAMemPointers[d])
				HDMAMemPointers[d] = S9xGetMemPointer(ShiftedIBank + IAddr);

			if (p->DoTransfer)
			{
				// XXX: Hack for Uniracers, because we don't understand
				// OAM Address Invalidation
				if (p->BAddress == 0x04)
				{
					if (SNESGameFixes.Uniracers)
					{
						PPU.OAMAddr = 0x10c;
						PPU.OAMFlip = 0;
					}
				}

			#ifdef DEBUGGER
				if (Settings.TraceHDMA && p->DoTransfer)
				{
					sprintf(String, "H-DMA[%d] %s (%d) 0x%06X->0x21%02X %s, Count: %3d, Rep: %s, V-LINE: %3ld %02X%04X",
							p-DMA, p->ReverseTransfer? "read" : "write",
							p->TransferMode, ShiftedIBank+IAddr, p->BAddress,
							p->HDMAIndirectAddressing ? "ind" : "abs",
							p->LineCount,
							p->Repeat ? "yes" : "no ", (long) CPU.V_Counter,
							p->ABank, p->Address);
					S9xMessage(S9X_TRACE, S9X_HDMA_TRACE, String);
				}
			#endif

				if (!p->ReverseTransfer)
				{
					if ((IAddr & MEMMAP_MASK) + HDMA_ModeByteCounts[p->TransferMode] >= MEMMAP_BLOCK_SIZE)
					{
						// HDMA REALLY-SLOW PATH
						HDMAMemPointers[d] = NULL;

						#define DOBYTE(Addr, RegOff) \
							CPU.InWRAMDMAorHDMA = (ShiftedIBank == 0x7e0000 || ShiftedIBank == 0x7f0000 || \
								(!(ShiftedIBank & 0x400000) && ((uint16) (Addr)) < 0x2000)); \
							S9xSetPPU(S9xGetByte(ShiftedIBank + ((uint16) (Addr))), 0x2100 + p->BAddress + (RegOff));

						switch (p->TransferMode)
						{
							case 0:
								DOBYTE(IAddr, 0);
								ADD_CYCLES(SLOW_ONE_CYCLE);
								break;

							case 5:
								DOBYTE(IAddr + 0, 0);
								ADD_CYCLES(SLOW_ONE_CYCLE);
								DOBYTE(IAddr + 1, 1);
								ADD_CYCLES(SLOW_ONE_CYCLE);
								DOBYTE(IAddr + 2, 0);
								ADD_CYCLES(SLOW_ONE_CYCLE);
								DOBYTE(IAddr + 3, 1);
								ADD_CYCLES(SLOW_ONE_CYCLE);
								break;

							case 1:
								DOBYTE(IAddr + 0, 0);
								ADD_CYCLES(SLOW_ONE_CYCLE);
								DOBYTE(IAddr + 1, 1);
								ADD_CYCLES(SLOW_ONE_CYCLE);
								break;

							case 2:
							case 6:
								DOBYTE(IAddr + 0, 0);
								ADD_CYCLES(SLOW_ONE_CYCLE);
								DOBYTE(IAddr + 1, 0);
								ADD_CYCLES(SLOW_ONE_CYCLE);
								break;

							case 3:
							case 7:
								DOBYTE(IAddr + 0, 0);
								ADD_CYCLES(SLOW_ONE_CYCLE);
								DOBYTE(IAddr + 1, 0);
								ADD_CYCLES(SLOW_ONE_CYCLE);
								DOBYTE(IAddr + 2, 1);
								ADD_CYCLES(SLOW_ONE_CYCLE);
								DOBYTE(IAddr + 3, 1);
								ADD_CYCLES(SLOW_ONE_CYCLE);
								break;

							case 4:
								DOBYTE(IAddr + 0, 0);
								ADD_CYCLES(SLOW_ONE_CYCLE);
								DOBYTE(IAddr + 1, 1);
								ADD_CYCLES(SLOW_ONE_CYCLE);
								DOBYTE(IAddr + 2, 2);
								ADD_CYCLES(SLOW_ONE_CYCLE);
								DOBYTE(IAddr + 3, 3);
								ADD_CYCLES(SLOW_ONE_CYCLE);
								break;
						}

						#undef DOBYTE
					}
					else
					{
						CPU.InWRAMDMAorHDMA = (ShiftedIBank == 0x7e0000 || ShiftedIBank == 0x7f0000 ||
							(!(ShiftedIBank & 0x400000) && IAddr < 0x2000));

						if (!HDMAMemPointers[d])
						{
							// HDMA SLOW PATH
							uint32	Addr = ShiftedIBank + IAddr;

							switch (p->TransferMode)
							{
								case 0:
									S9xSetPPU(S9xGetByte(Addr), 0x2100 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									break;

								case 5:
									S9xSetPPU(S9xGetByte(Addr + 0), 0x2100 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									S9xSetPPU(S9xGetByte(Addr + 1), 0x2101 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									Addr += 2;
									/* fall through */
								case 1:
									S9xSetPPU(S9xGetByte(Addr + 0), 0x2100 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									S9xSetPPU(S9xGetByte(Addr + 1), 0x2101 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									break;

								case 2:
								case 6:
									S9xSetPPU(S9xGetByte(Addr + 0), 0x2100 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									S9xSetPPU(S9xGetByte(Addr + 1), 0x2100 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									break;

								case 3:
								case 7:
									S9xSetPPU(S9xGetByte(Addr + 0), 0x2100 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									S9xSetPPU(S9xGetByte(Addr + 1), 0x2100 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									S9xSetPPU(S9xGetByte(Addr + 2), 0x2101 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									S9xSetPPU(S9xGetByte(Addr + 3), 0x2101 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									break;

								case 4:
									S9xSetPPU(S9xGetByte(Addr + 0), 0x2100 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									S9xSetPPU(S9xGetByte(Addr + 1), 0x2101 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									S9xSetPPU(S9xGetByte(Addr + 2), 0x2102 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									S9xSetPPU(S9xGetByte(Addr + 3), 0x2103 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									break;
							}
						}
						else
						{
							// HDMA FAST PATH
							switch (p->TransferMode)
							{
								case 0:
									S9xSetPPU(*HDMAMemPointers[d]++, 0x2100 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									break;

								case 5:
									S9xSetPPU(*(HDMAMemPointers[d] + 0), 0x2100 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									S9xSetPPU(*(HDMAMemPointers[d] + 1), 0x2101 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									HDMAMemPointers[d] += 2;
									/* fall through */
								case 1:
									S9xSetPPU(*(HDMAMemPointers[d] + 0), 0x2100 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									S9xSetPPU(*(HDMAMemPointers[d] + 1), 0x2101 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									HDMAMemPointers[d] += 2;
									break;

								case 2:
								case 6:
									S9xSetPPU(*(HDMAMemPointers[d] + 0), 0x2100 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									S9xSetPPU(*(HDMAMemPointers[d] + 1), 0x2100 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									HDMAMemPointers[d] += 2;
									break;

								case 3:
								case 7:
									S9xSetPPU(*(HDMAMemPointers[d] + 0), 0x2100 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									S9xSetPPU(*(HDMAMemPointers[d] + 1), 0x2100 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									S9xSetPPU(*(HDMAMemPointers[d] + 2), 0x2101 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									S9xSetPPU(*(HDMAMemPointers[d] + 3), 0x2101 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									HDMAMemPointers[d] += 4;
									break;

								case 4:
									S9xSetPPU(*(HDMAMemPointers[d] + 0), 0x2100 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									S9xSetPPU(*(HDMAMemPointers[d] + 1), 0x2101 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									S9xSetPPU(*(HDMAMemPointers[d] + 2), 0x2102 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									S9xSetPPU(*(HDMAMemPointers[d] + 3), 0x2103 + p->BAddress);
									ADD_CYCLES(SLOW_ONE_CYCLE);
									HDMAMemPointers[d] += 4;
									break;
							}
						}
					}
				}
				else
				{
					// REVERSE HDMA REALLY-SLOW PATH
					// anomie says: Since this is apparently never used
					// (otherwise we would have noticed before now), let's not bother with faster paths.
					HDMAMemPointers[d] = NULL;

					#define DOBYTE(Addr, RegOff) \
						CPU.InWRAMDMAorHDMA = (ShiftedIBank == 0x7e0000 || ShiftedIBank == 0x7f0000 || \
							(!(ShiftedIBank & 0x400000) && ((uint16) (Addr)) < 0x2000)); \
						S9xSetByte(S9xGetPPU(0x2100 + p->BAddress + (RegOff)), ShiftedIBank + ((uint16) (Addr)));

					switch (p->TransferMode)
					{
						case 0:
							DOBYTE(IAddr, 0);
							ADD_CYCLES(SLOW_ONE_CYCLE);
							break;

						case 5:
							DOBYTE(IAddr + 0, 0);
							ADD_CYCLES(SLOW_ONE_CYCLE);
							DOBYTE(IAddr + 1, 1);
							ADD_CYCLES(SLOW_ONE_CYCLE);
							DOBYTE(IAddr + 2, 0);
							ADD_CYCLES(SLOW_ONE_CYCLE);
							DOBYTE(IAddr + 3, 1);
							ADD_CYCLES(SLOW_ONE_CYCLE);
							break;

						case 1:
							DOBYTE(IAddr + 0, 0);
							ADD_CYCLES(SLOW_ONE_CYCLE);
							DOBYTE(IAddr + 1, 1);
							ADD_CYCLES(SLOW_ONE_CYCLE);
							break;

						case 2:
						case 6:
							DOBYTE(IAddr + 0, 0);
							ADD_CYCLES(SLOW_ONE_CYCLE);
							DOBYTE(IAddr + 1, 0);
							ADD_CYCLES(SLOW_ONE_CYCLE);
							break;

						case 3:
						case 7:
							DOBYTE(IAddr + 0, 0);
							ADD_CYCLES(SLOW_ONE_CYCLE);
							DOBYTE(IAddr + 1, 0);
							ADD_CYCLES(SLOW_ONE_CYCLE);
							DOBYTE(IAddr + 2, 1);
							ADD_CYCLES(SLOW_ONE_CYCLE);
							DOBYTE(IAddr + 3, 1);
							ADD_CYCLES(SLOW_ONE_CYCLE);
							break;

						case 4:
							DOBYTE(IAddr + 0, 0);
							ADD_CYCLES(SLOW_ONE_CYCLE);
							DOBYTE(IAddr + 1, 1);
							ADD_CYCLES(SLOW_ONE_CYCLE);
							DOBYTE(IAddr + 2, 2);
							ADD_CYCLES(SLOW_ONE_CYCLE);
							DOBYTE(IAddr + 3, 3);
							ADD_CYCLES(SLOW_ONE_CYCLE);
							break;
					}

					#undef DOBYTE
				}

				if (p->HDMAIndirectAddressing)
					p->IndirectAddress += HDMA_ModeByteCounts[p->TransferMode];
				else
					p->Address += HDMA_ModeByteCounts[p->TransferMode];
			}

			p->DoTransfer = !p->Repeat;

			if (!--p->LineCount)
			{
				if (!HDMAReadLineCount(d))
				{
					byte &= ~mask;
					PPU.HDMAEnded |= mask;
					p->DoTransfer = FALSE;
					continue;
				}
			}
			else
				ADD_CYCLES(SLOW_ONE_CYCLE);
		}
	}

	CPU.InHDMA = FALSE;
	CPU.InDMAorHDMA = CPU.InDMA;
	CPU.InWRAMDMAorHDMA = temp;
	CPU.CurrentDMAorHDMAChannel = tmpch;

	return (byte);
}

void S9xResetDMA (void)
{
	for (int d = 0; d < 8; d++)
	{
		DMA[d].ReverseTransfer = TRUE;
		DMA[d].HDMAIndirectAddressing = TRUE;
		DMA[d].AAddressFixed = TRUE;
		DMA[d].AAddressDecrement = TRUE;
		DMA[d].TransferMode = 7;
		DMA[d].BAddress = 0xff;
		DMA[d].AAddress = 0xffff;
		DMA[d].ABank = 0xff;
		DMA[d].DMACount_Or_HDMAIndirectAddress = 0xffff;
		DMA[d].IndirectBank = 0xff;
		DMA[d].Address = 0xffff;
		DMA[d].Repeat = FALSE;
		DMA[d].LineCount = 0x7f;
		DMA[d].UnknownByte = 0xff;
		DMA[d].DoTransfer = FALSE;
		DMA[d].UnusedBit43x0 = 1;
	}
}
