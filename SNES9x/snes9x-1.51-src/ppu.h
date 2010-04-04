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




#ifndef _PPU_H_
#define _PPU_H_

#define FIRST_VISIBLE_LINE 1

extern uint8 GetBank;
extern uint16 SignExtend [2];

#define TILE_2BIT 0
#define TILE_4BIT 1
#define TILE_8BIT 2
#define TILE_2BIT_EVEN 3
#define TILE_2BIT_ODD  4
#define TILE_4BIT_EVEN 5
#define TILE_4BIT_ODD  6

#define MAX_2BIT_TILES 4096
#define MAX_4BIT_TILES 2048
#define MAX_8BIT_TILES 1024

#define PPU_H_BEAM_IRQ_SOURCE	(1 << 0)
#define PPU_V_BEAM_IRQ_SOURCE	(1 << 1)
#define GSU_IRQ_SOURCE		(1 << 2)
#define SA1_IRQ_SOURCE		(1 << 7)
#define SA1_DMA_IRQ_SOURCE	(1 << 5)

struct ClipData {
    uint8   Count;
    uint8   DrawMode[6];
    uint16  Left[6];
    uint16  Right[6];
};

struct InternalPPU {
    bool8  ColorsChanged;
    uint8  HDMA;
    uint8  HDMAEnded;
    uint8  MaxBrightness;
    bool8  LatchedBlanking;
    bool8  OBJChanged;
    bool8  RenderThisFrame;
    bool8  DirectColourMapsNeedRebuild;
    uint32 FrameCount;
    uint32 RenderedFramesCount;
    uint32 DisplayedRenderedFrameCount;
    uint32 TotalEmulatedFrames;
    uint32 SkippedFrames;
    uint32 FrameSkip;
    uint8  *TileCache [7];
    uint8  *TileCached [7];
#ifdef CORRECT_VRAM_READS
    uint16 VRAMReadBuffer;
#else
    bool8  FirstVRAMRead;
#endif
    bool8  Interlace;
    bool8  InterlaceOBJ;
    bool8  PseudoHires;
    bool8  DoubleWidthPixels;
    bool8  DoubleHeightPixels;
    int    RenderedScreenHeight;
    int    RenderedScreenWidth;
    uint32 Red [256];
    uint32 Green [256];
    uint32 Blue [256];
    uint8  *XB;
    uint16 ScreenColors [256];
    int	   PreviousLine;
    int	   CurrentLine;
    struct ClipData Clip[2][6];
};

struct SOBJ
{
    int16  HPos;
    uint16 VPos;
    uint16 Name;
    uint8  VFlip;
    uint8  HFlip;
    uint8  Priority;
    uint8  Palette;
    uint8  Size;
};

struct SPPU {
    uint8  BGMode;
    uint8  BG3Priority;
    uint8  Brightness;

    struct {
	bool8 High;
	uint8 Increment;
	uint16 Address;
	uint16 Mask1;
	uint16 FullGraphicCount;
	uint16 Shift;
    } VMA;

    struct {
	uint16 SCBase;
	uint16 VOffset;
	uint16 HOffset;
	uint8 BGSize;
	uint16 NameBase;
	uint16 SCSize;
    } BG [4];

    bool8  CGFLIP;
    uint16 CGDATA [256];
    uint8  FirstSprite;
    uint8  LastSprite;
    struct SOBJ OBJ [128];
    uint8  OAMPriorityRotation;
    uint16 OAMAddr;
    uint8  RangeTimeOver;

    uint8  OAMFlip;
    uint16 OAMTileAddress;
    uint16 IRQVBeamPos;
    uint16 IRQHBeamPos;
    uint16 VBeamPosLatched;
    uint16 HBeamPosLatched;

    uint8  HBeamFlip;
    uint8  VBeamFlip;
    uint8  HVBeamCounterLatched;

    short  MatrixA;
    short  MatrixB;
    short  MatrixC;
    short  MatrixD;
    short  CentreX;
    short  CentreY;
    short  M7HOFS;
    short  M7VOFS;

    uint8  CGADD;
    uint8  FixedColourRed;
    uint8  FixedColourGreen;
    uint8  FixedColourBlue;
    uint16 SavedOAMAddr;
    uint16 ScreenHeight;
    uint32 WRAM;
    bool8  ForcedBlanking;
    bool8  OBJThroughMain;
    bool8  OBJThroughSub;
    uint8  OBJSizeSelect;
    uint16 OBJNameBase;
    bool8  OBJAddition;
    uint8  OAMReadFlip;
    uint8  OAMData [512 + 32];
    bool8  VTimerEnabled;
    bool8  HTimerEnabled;
    short  HTimerPosition;
    uint8  Mosaic;
    uint8  MosaicStart;
    bool8  BGMosaic [4];
    bool8  Mode7HFlip;
    bool8  Mode7VFlip;
    uint8  Mode7Repeat;
    uint8  Window1Left;
    uint8  Window1Right;
    uint8  Window2Left;
    uint8  Window2Right;
    uint8  ClipCounts [6];
    uint8  ClipWindowOverlapLogic [6];
    uint8  ClipWindow1Enable [6];
    uint8  ClipWindow2Enable [6];
    bool8  ClipWindow1Inside [6];
    bool8  ClipWindow2Inside [6];
    bool8  RecomputeClipWindows;
    uint8  CGFLIPRead;
    uint16 OBJNameSelect;
    bool8  Need16x8Mulitply;

    uint16 OAMWriteRegister;
    uint8  BGnxOFSbyte;
    uint8  M7byte;
    uint8  OpenBus1;
    uint8  OpenBus2;
    uint16 GunVLatch;
    uint16 GunHLatch;
    short  VTimerPosition;
};

#define CLIP_OR 0
#define CLIP_AND 1
#define CLIP_XOR 2
#define CLIP_XNOR 3

struct SDMA {
    /* $43x0 */
    bool8  ReverseTransfer;
    bool8  HDMAIndirectAddressing;
    bool8  UnusedBit43x0;
    bool8  AAddressFixed;
    bool8  AAddressDecrement;
    uint8  TransferMode;

    /* $43x1 */
    uint8  BAddress;

    /* $43x2-4 */
    uint16 AAddress;
    uint8  ABank;

    /* $43x5-6 */
    uint16 DMACount_Or_HDMAIndirectAddress;

    /* $43x7 */
    uint8  IndirectBank;

    /* $43x8-9 */
    uint16 Address;

    /* $43xA */
    uint8  Repeat;
    uint8  LineCount;

    /* $43xB/F */
    uint8 UnknownByte;

    /* internal */
    uint8  DoTransfer;
};
#define TransferBytes DMACount_Or_HDMAIndirectAddress
#define IndirectAddress DMACount_Or_HDMAIndirectAddress

START_EXTERN_C
void S9xUpdateScreen ();
void S9xResetPPU ();
void S9xSoftResetPPU ();
void S9xFixColourBrightness ();
void S9xDoAutoJoypad ();
void S9xSuperFXExec ();

void S9xSetPPU (uint8 Byte, uint16 Address);
uint8 S9xGetPPU (uint16 Address);
void S9xSetCPU (uint8 Byte, uint16 Address);
uint8 S9xGetCPU (uint16 Address);

void S9xInitC4 ();
void S9xSetC4 (uint8 Byte, uint16 Address);
uint8 S9xGetC4 (uint16 Address);
void S9xSetC4RAM (uint8 Byte, uint16 Address);
uint8 S9xGetC4RAM (uint16 Address);
uint8 *S9xGetBasePointerC4 (uint16 Address);

void S9xUpdateHVTimerPosition (void);
void S9xCheckMissingHTimerPosition (int32);
void S9xCheckMissingHTimerRange (int32, int32);
void S9xCheckMissingHTimerHalt (int32, int32);

extern struct SPPU PPU;
extern struct SDMA DMA [8];
extern struct InternalPPU IPPU;
END_EXTERN_C

#include "gfx.h"
#include "memmap.h"

typedef struct{
	uint8 _5C77;
	uint8 _5C78;
	uint8 _5A22;
} SnesModel;

START_EXTERN_C
extern SnesModel* Model;
extern SnesModel M1SNES;
extern SnesModel M2SNES;
END_EXTERN_C

#define MAX_5C77_VERSION 0x01
#define MAX_5C78_VERSION 0x03
#define MAX_5A22_VERSION 0x02

STATIC inline uint8 REGISTER_4212()
{
    GetBank = 0;
    if (CPU.V_Counter >= PPU.ScreenHeight + FIRST_VISIBLE_LINE &&
	CPU.V_Counter < PPU.ScreenHeight + FIRST_VISIBLE_LINE + 3)
	GetBank = 1;

    GetBank |= ((CPU.Cycles < Timings.HBlankEnd) || (CPU.Cycles >= Timings.HBlankStart)) ? 0x40 : 0;
    if (CPU.V_Counter >= PPU.ScreenHeight + FIRST_VISIBLE_LINE)
	GetBank |= 0x80; /* XXX: 0x80 or 0xc0 ? */

    return (GetBank);
}

STATIC inline void FLUSH_REDRAW ()
{
    if (IPPU.PreviousLine != IPPU.CurrentLine)
	S9xUpdateScreen ();
}

STATIC inline void REGISTER_2104 (uint8 byte)
{
    if (PPU.OAMAddr & 0x100)
    {
        int addr = ((PPU.OAMAddr & 0x10f) << 1) + (PPU.OAMFlip & 1);
        if (byte != PPU.OAMData [addr]){
            FLUSH_REDRAW ();
            PPU.OAMData [addr] = byte;
            IPPU.OBJChanged = TRUE;

            // X position high bit, and sprite size (x4)
            struct SOBJ *pObj = &PPU.OBJ [(addr & 0x1f) * 4];

            pObj->HPos = (pObj->HPos & 0xFF) | SignExtend[(byte >> 0) & 1];
            pObj++->Size = byte & 2;
            pObj->HPos = (pObj->HPos & 0xFF) | SignExtend[(byte >> 2) & 1];
            pObj++->Size = byte & 8;
            pObj->HPos = (pObj->HPos & 0xFF) | SignExtend[(byte >> 4) & 1];
            pObj++->Size = byte & 32;
            pObj->HPos = (pObj->HPos & 0xFF) | SignExtend[(byte >> 6) & 1];
            pObj->Size = byte & 128;
        }
        PPU.OAMFlip ^= 1;
		if(!(PPU.OAMFlip & 1)){
			++PPU.OAMAddr;
			PPU.OAMAddr &= 0x1ff;
			if (PPU.OAMPriorityRotation && PPU.FirstSprite != (PPU.OAMAddr >> 1))
			{
				PPU.FirstSprite = (PPU.OAMAddr&0xFE) >> 1;
				IPPU.OBJChanged = TRUE;
			}
		} else {
			if (PPU.OAMPriorityRotation && (PPU.OAMAddr&1)) IPPU.OBJChanged = TRUE;
		}
    } else if(!(PPU.OAMFlip & 1)){
        PPU.OAMWriteRegister &= 0xff00;
        PPU.OAMWriteRegister |= byte;
        PPU.OAMFlip |= 1;
		if (PPU.OAMPriorityRotation && (PPU.OAMAddr&1)) IPPU.OBJChanged = TRUE;
    } else {
        PPU.OAMWriteRegister &= 0x00ff;
        uint8 lowbyte = (uint8)(PPU.OAMWriteRegister);
        uint8 highbyte = byte;
        PPU.OAMWriteRegister |= byte << 8;

        int addr = (PPU.OAMAddr << 1);

        if (lowbyte != PPU.OAMData [addr] ||
            highbyte != PPU.OAMData [addr+1])
        {
            FLUSH_REDRAW ();
            PPU.OAMData [addr] = lowbyte;
            PPU.OAMData [addr+1] = highbyte;
            IPPU.OBJChanged = TRUE;
            if (addr & 2)
            {
                // Tile
                PPU.OBJ[addr = PPU.OAMAddr >> 1].Name = PPU.OAMWriteRegister & 0x1ff;

                // priority, h and v flip.
                PPU.OBJ[addr].Palette = (highbyte >> 1) & 7;
                PPU.OBJ[addr].Priority = (highbyte >> 4) & 3;
                PPU.OBJ[addr].HFlip = (highbyte >> 6) & 1;
                PPU.OBJ[addr].VFlip = (highbyte >> 7) & 1;
            }
            else
            {
                // X position (low)
                PPU.OBJ[addr = PPU.OAMAddr >> 1].HPos &= 0xFF00;
                PPU.OBJ[addr].HPos |= lowbyte;

                // Sprite Y position
                PPU.OBJ[addr].VPos = highbyte;
            }
        }
        PPU.OAMFlip &= ~1;
        ++PPU.OAMAddr;
		if (PPU.OAMPriorityRotation && PPU.FirstSprite != (PPU.OAMAddr >> 1))
		{
			PPU.FirstSprite = (PPU.OAMAddr&0xFE) >> 1;
			IPPU.OBJChanged = TRUE;
		}
    }

    Memory.FillRAM [0x2104] = byte;
}

// This code is correct, however due to Snes9x's inaccurate timings, some games might be broken by this chage. :(
#define CHECK_INBLANK \
{ \
	if (Settings.BlockInvalidVRAMAccess && !PPU.ForcedBlanking && CPU.V_Counter < PPU.ScreenHeight + FIRST_VISIBLE_LINE) \
	{ \
		return; \
	} \
} \

STATIC inline void REGISTER_2118 (uint8 Byte)
{
	CHECK_INBLANK;

    uint32 address;
    if (PPU.VMA.FullGraphicCount)
    {
	uint32 rem = PPU.VMA.Address & PPU.VMA.Mask1;
	address = (((PPU.VMA.Address & ~PPU.VMA.Mask1) +
			 (rem >> PPU.VMA.Shift) +
			 ((rem & (PPU.VMA.FullGraphicCount - 1)) << 3)) << 1) & 0xffff;
	Memory.VRAM [address] = Byte;
    }
    else
    {
	Memory.VRAM[address = (PPU.VMA.Address << 1) & 0xFFFF] = Byte;
    }
    IPPU.TileCached [TILE_2BIT][address >> 4] = FALSE;
    IPPU.TileCached [TILE_4BIT][address >> 5] = FALSE;
    IPPU.TileCached [TILE_8BIT][address >> 6] = FALSE;
    IPPU.TileCached [TILE_2BIT_EVEN][address >> 4] = FALSE;
    IPPU.TileCached [TILE_2BIT_EVEN][((address >> 4)-1)&(MAX_2BIT_TILES-1)] = FALSE;
    IPPU.TileCached [TILE_2BIT_ODD][address >> 4] = FALSE;
    IPPU.TileCached [TILE_2BIT_ODD][((address >> 4)-1)&(MAX_2BIT_TILES-1)] = FALSE;
    IPPU.TileCached [TILE_4BIT_EVEN][address >> 5] = FALSE;
    IPPU.TileCached [TILE_4BIT_EVEN][((address >> 5)-1)&(MAX_4BIT_TILES-1)] = FALSE;
    IPPU.TileCached [TILE_4BIT_ODD][address >> 5] = FALSE;
    IPPU.TileCached [TILE_4BIT_ODD][((address >> 5)-1)&(MAX_4BIT_TILES-1)] = FALSE;
    if (!PPU.VMA.High)
    {
#ifdef DEBUGGER
	if (Settings.TraceVRAM && !CPU.InDMAorHDMA)
	{
	    printf ("VRAM write byte: $%04X (%d,%d)\n", PPU.VMA.Address,
		    Memory.FillRAM[0x2115] & 3,
		    (Memory.FillRAM [0x2115] & 0x0c) >> 2);
	}
#endif
	PPU.VMA.Address += PPU.VMA.Increment;
    }
//    Memory.FillRAM [0x2118] = Byte;
}

STATIC inline void REGISTER_2118_tile (uint8 Byte)
{
	CHECK_INBLANK;

    uint32 address;
    uint32 rem = PPU.VMA.Address & PPU.VMA.Mask1;
    address = (((PPU.VMA.Address & ~PPU.VMA.Mask1) +
		 (rem >> PPU.VMA.Shift) +
		 ((rem & (PPU.VMA.FullGraphicCount - 1)) << 3)) << 1) & 0xffff;
    Memory.VRAM [address] = Byte;
    IPPU.TileCached [TILE_2BIT][address >> 4] = FALSE;
    IPPU.TileCached [TILE_4BIT][address >> 5] = FALSE;
    IPPU.TileCached [TILE_8BIT][address >> 6] = FALSE;
    IPPU.TileCached [TILE_2BIT_EVEN][address >> 4] = FALSE;
    IPPU.TileCached [TILE_2BIT_EVEN][((address >> 4)-1)&(MAX_2BIT_TILES-1)] = FALSE;
    IPPU.TileCached [TILE_2BIT_ODD][address >> 4] = FALSE;
    IPPU.TileCached [TILE_2BIT_ODD][((address >> 4)-1)&(MAX_2BIT_TILES-1)] = FALSE;
    IPPU.TileCached [TILE_4BIT_EVEN][address >> 5] = FALSE;
    IPPU.TileCached [TILE_4BIT_EVEN][((address >> 5)-1)&(MAX_4BIT_TILES-1)] = FALSE;
    IPPU.TileCached [TILE_4BIT_ODD][address >> 5] = FALSE;
    IPPU.TileCached [TILE_4BIT_ODD][((address >> 5)-1)&(MAX_4BIT_TILES-1)] = FALSE;
    if (!PPU.VMA.High)
	PPU.VMA.Address += PPU.VMA.Increment;
//    Memory.FillRAM [0x2118] = Byte;
}

STATIC inline void REGISTER_2118_linear (uint8 Byte)
{
	CHECK_INBLANK;

    uint32 address;
    Memory.VRAM[address = (PPU.VMA.Address << 1) & 0xFFFF] = Byte;
    IPPU.TileCached [TILE_2BIT][address >> 4] = FALSE;
    IPPU.TileCached [TILE_4BIT][address >> 5] = FALSE;
    IPPU.TileCached [TILE_8BIT][address >> 6] = FALSE;
    IPPU.TileCached [TILE_2BIT_EVEN][address >> 4] = FALSE;
    IPPU.TileCached [TILE_2BIT_EVEN][((address >> 4)-1)&(MAX_2BIT_TILES-1)] = FALSE;
    IPPU.TileCached [TILE_2BIT_ODD][address >> 4] = FALSE;
    IPPU.TileCached [TILE_2BIT_ODD][((address >> 4)-1)&(MAX_2BIT_TILES-1)] = FALSE;
    IPPU.TileCached [TILE_4BIT_EVEN][address >> 5] = FALSE;
    IPPU.TileCached [TILE_4BIT_EVEN][((address >> 5)-1)&(MAX_4BIT_TILES-1)] = FALSE;
    IPPU.TileCached [TILE_4BIT_ODD][address >> 5] = FALSE;
    IPPU.TileCached [TILE_4BIT_ODD][((address >> 5)-1)&(MAX_4BIT_TILES-1)] = FALSE;
    if (!PPU.VMA.High)
	PPU.VMA.Address += PPU.VMA.Increment;
//    Memory.FillRAM [0x2118] = Byte;
}

STATIC inline void REGISTER_2119 (uint8 Byte)
{
	CHECK_INBLANK;

    uint32 address;
    if (PPU.VMA.FullGraphicCount)
    {
	uint32 rem = PPU.VMA.Address & PPU.VMA.Mask1;
	address = ((((PPU.VMA.Address & ~PPU.VMA.Mask1) +
		    (rem >> PPU.VMA.Shift) +
		    ((rem & (PPU.VMA.FullGraphicCount - 1)) << 3)) << 1) + 1) & 0xFFFF;
	Memory.VRAM [address] = Byte;
    }
    else
    {
	Memory.VRAM[address = ((PPU.VMA.Address << 1) + 1) & 0xFFFF] = Byte;
    }
    IPPU.TileCached [TILE_2BIT][address >> 4] = FALSE;
    IPPU.TileCached [TILE_4BIT][address >> 5] = FALSE;
    IPPU.TileCached [TILE_8BIT][address >> 6] = FALSE;
    IPPU.TileCached [TILE_2BIT_EVEN][address >> 4] = FALSE;
    IPPU.TileCached [TILE_2BIT_EVEN][((address >> 4)-1)&(MAX_2BIT_TILES-1)] = FALSE;
    IPPU.TileCached [TILE_2BIT_ODD][address >> 4] = FALSE;
    IPPU.TileCached [TILE_2BIT_ODD][((address >> 4)-1)&(MAX_2BIT_TILES-1)] = FALSE;
    IPPU.TileCached [TILE_4BIT_EVEN][address >> 5] = FALSE;
    IPPU.TileCached [TILE_4BIT_EVEN][((address >> 5)-1)&(MAX_4BIT_TILES-1)] = FALSE;
    IPPU.TileCached [TILE_4BIT_ODD][address >> 5] = FALSE;
    IPPU.TileCached [TILE_4BIT_ODD][((address >> 5)-1)&(MAX_4BIT_TILES-1)] = FALSE;
    if (PPU.VMA.High)
    {
#ifdef DEBUGGER
	if (Settings.TraceVRAM && !CPU.InDMAorHDMA)
	{
	    printf ("VRAM write word: $%04X (%d,%d)\n", PPU.VMA.Address,
		    Memory.FillRAM[0x2115] & 3,
		    (Memory.FillRAM [0x2115] & 0x0c) >> 2);
	}
#endif
	PPU.VMA.Address += PPU.VMA.Increment;
    }
//    Memory.FillRAM [0x2119] = Byte;
}

STATIC inline void REGISTER_2119_tile (uint8 Byte)
{
	CHECK_INBLANK;

    uint32 rem = PPU.VMA.Address & PPU.VMA.Mask1;
    uint32 address = ((((PPU.VMA.Address & ~PPU.VMA.Mask1) +
		    (rem >> PPU.VMA.Shift) +
		    ((rem & (PPU.VMA.FullGraphicCount - 1)) << 3)) << 1) + 1) & 0xFFFF;
    Memory.VRAM [address] = Byte;
    IPPU.TileCached [TILE_2BIT][address >> 4] = FALSE;
    IPPU.TileCached [TILE_4BIT][address >> 5] = FALSE;
    IPPU.TileCached [TILE_8BIT][address >> 6] = FALSE;
    IPPU.TileCached [TILE_2BIT_EVEN][address >> 4] = FALSE;
    IPPU.TileCached [TILE_2BIT_EVEN][((address >> 4)-1)&(MAX_2BIT_TILES-1)] = FALSE;
    IPPU.TileCached [TILE_2BIT_ODD][address >> 4] = FALSE;
    IPPU.TileCached [TILE_2BIT_ODD][((address >> 4)-1)&(MAX_2BIT_TILES-1)] = FALSE;
    IPPU.TileCached [TILE_4BIT_EVEN][address >> 5] = FALSE;
    IPPU.TileCached [TILE_4BIT_EVEN][((address >> 5)-1)&(MAX_4BIT_TILES-1)] = FALSE;
    IPPU.TileCached [TILE_4BIT_ODD][address >> 5] = FALSE;
    IPPU.TileCached [TILE_4BIT_ODD][((address >> 5)-1)&(MAX_4BIT_TILES-1)] = FALSE;
    if (PPU.VMA.High)
	PPU.VMA.Address += PPU.VMA.Increment;
//    Memory.FillRAM [0x2119] = Byte;
}

STATIC inline void REGISTER_2119_linear (uint8 Byte)
{
	CHECK_INBLANK;

    uint32 address;
    Memory.VRAM[address = ((PPU.VMA.Address << 1) + 1) & 0xFFFF] = Byte;
    IPPU.TileCached [TILE_2BIT][address >> 4] = FALSE;
    IPPU.TileCached [TILE_4BIT][address >> 5] = FALSE;
    IPPU.TileCached [TILE_8BIT][address >> 6] = FALSE;
    IPPU.TileCached [TILE_2BIT_EVEN][address >> 4] = FALSE;
    IPPU.TileCached [TILE_2BIT_EVEN][((address >> 4)-1)&(MAX_2BIT_TILES-1)] = FALSE;
    IPPU.TileCached [TILE_2BIT_ODD][address >> 4] = FALSE;
    IPPU.TileCached [TILE_2BIT_ODD][((address >> 4)-1)&(MAX_2BIT_TILES-1)] = FALSE;
    IPPU.TileCached [TILE_4BIT_EVEN][address >> 5] = FALSE;
    IPPU.TileCached [TILE_4BIT_EVEN][((address >> 5)-1)&(MAX_4BIT_TILES-1)] = FALSE;
    IPPU.TileCached [TILE_4BIT_ODD][address >> 5] = FALSE;
    IPPU.TileCached [TILE_4BIT_ODD][((address >> 5)-1)&(MAX_4BIT_TILES-1)] = FALSE;
    if (PPU.VMA.High)
	PPU.VMA.Address += PPU.VMA.Increment;
//    Memory.FillRAM [0x2119] = Byte;
}

STATIC inline void REGISTER_2122(uint8 Byte)
{
    // CG-RAM (palette) write

    if (PPU.CGFLIP)
    {
	if ((Byte & 0x7f) != (PPU.CGDATA[PPU.CGADD] >> 8))
	{
            FLUSH_REDRAW ();
	    PPU.CGDATA[PPU.CGADD] &= 0x00FF;
	    PPU.CGDATA[PPU.CGADD] |= (Byte & 0x7f) << 8;
	    IPPU.ColorsChanged = TRUE;
            IPPU.Blue [PPU.CGADD] = IPPU.XB [(Byte >> 2) & 0x1f];
            IPPU.Green [PPU.CGADD] = IPPU.XB [(PPU.CGDATA[PPU.CGADD] >> 5) & 0x1f];
            IPPU.ScreenColors [PPU.CGADD] = (uint16) BUILD_PIXEL (IPPU.Red [PPU.CGADD],
                                                                  IPPU.Green [PPU.CGADD],
                                                                  IPPU.Blue [PPU.CGADD]);
	}
	PPU.CGADD++;
    }
    else
    {
	if (Byte != (uint8) (PPU.CGDATA[PPU.CGADD] & 0xff))
	{
            FLUSH_REDRAW ();
	    PPU.CGDATA[PPU.CGADD] &= 0x7F00;
	    PPU.CGDATA[PPU.CGADD] |= Byte;
	    IPPU.ColorsChanged = TRUE;
            IPPU.Red [PPU.CGADD] = IPPU.XB [Byte & 0x1f];
            IPPU.Green [PPU.CGADD] = IPPU.XB [(PPU.CGDATA[PPU.CGADD] >> 5) & 0x1f];
            IPPU.ScreenColors [PPU.CGADD] = (uint16) BUILD_PIXEL (IPPU.Red [PPU.CGADD],
                                                                  IPPU.Green [PPU.CGADD],
                                                                  IPPU.Blue [PPU.CGADD]);
        }
    }
    PPU.CGFLIP ^= 1;
//    Memory.FillRAM [0x2122] = Byte;
}

STATIC inline void REGISTER_2180(uint8 Byte)
{
    Memory.RAM[PPU.WRAM++] = Byte;
    PPU.WRAM &= 0x1FFFF;
    Memory.FillRAM [0x2180] = Byte;
}


#endif

