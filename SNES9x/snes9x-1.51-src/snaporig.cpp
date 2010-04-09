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



#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <ctype.h>
#include <stdlib.h>

#if defined(__unix) || defined(__linux) || defined(__sun) || defined(__DJGPP)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "snapshot.h"
#include "snaporig.h"
#include "memmap.h"
#include "snes9x.h"
#include "65c816.h"
#include "ppu.h"
#include "cpuexec.h"
#include "display.h"
#include "apu.h"
#include "soundux.h"

#undef TransferBytes
#undef IndirectAddress

struct SOrigPPU OrigPPU;
struct SOrigDMA OrigDMA [8];
struct SOrigRegisters OrigRegisters;
struct SOrigCPUState OrigCPU;
struct SOrigAPU OrigAPU;
SOrigSoundData OrigSoundData;
struct SOrigAPURegisters OrigAPURegisters;
char ROMFilename [1025];

static int ReadOrigSnapshot (STREAM);

bool8 S9xLoadOrigSnapshot (const char *filename)
{
    STREAM snapshot = NULL;
    if (S9xOpenSnapshotFile (filename, TRUE, &snapshot))
    {
	int result;
	if ((result = ReadOrigSnapshot (snapshot)) != SUCCESS)
	{
	    S9xCloseSnapshotFile (snapshot);
	    return (FALSE);
	}
	S9xCloseSnapshotFile (snapshot);
	return (TRUE);
    }
    return (FALSE);
}

static int ReadBlock (const char *key, void *block, int max_len, STREAM snap)
{
    char buffer [20];
    int len = 0;
    int rem = 0;

    if (READ_STREAM (buffer, 11, snap) != 11 ||
	strncmp (buffer, key, 4) != 0 ||
	(len = atoi (&buffer [4])) == 0)
	return (WRONG_FORMAT);

    if (len > max_len)
    {
	rem = len - max_len;
	len = max_len;
    }
    if (READ_STREAM (block, len, snap) != len)
	return (WRONG_FORMAT);

    if (rem)
    {
	char *junk = new char [rem];
	READ_STREAM (junk, rem, snap);
	delete[] junk;
    }

    return (SUCCESS);
}

static int ReadOrigSnapshot (STREAM snap)
{
    char buffer [_MAX_PATH];
    char rom_filename [_MAX_PATH];
    int result;
    int i;
    int j;

    int version;
    int len = strlen (ORIG_SNAPSHOT_MAGIC) + 1 + 4 + 1;
    if (READ_STREAM (buffer, len, snap) != len)
	return (WRONG_FORMAT);
    if (strncmp (buffer, ORIG_SNAPSHOT_MAGIC, strlen (ORIG_SNAPSHOT_MAGIC)) != 0)
	return (WRONG_FORMAT);
    if ((version = atoi (&buffer [strlen (SNAPSHOT_MAGIC) + 1])) > ORIG_SNAPSHOT_VERSION)
	return (WRONG_VERSION);

    if ((result = ReadBlock ("NAM:", rom_filename, _MAX_PATH, snap)) != SUCCESS)
	return (result);

    if ((result = ReadBlock ("HiR:", buffer, 0x41, snap)) != SUCCESS)
	return (result);

    if (strcasecmp (rom_filename, Memory.ROMFilename) != 0 &&
	strcasecmp (S9xBasename (rom_filename), S9xBasename (Memory.ROMFilename)) != 0)
    {
	S9xMessage (S9X_WARNING, S9X_FREEZE_ROM_NAME,
		    "Current loaded ROM image doesn't match that required by freeze-game file.");
    }

    S9xReset ();
    S9xSetSoundMute (TRUE);
    if ((result = ReadBlock ("CPU:", &OrigCPU, sizeof (OrigCPU), snap)) != SUCCESS)
	return (result);
    OrigCPU.FastROMSpeed = OrigCPU.FastROMSpeed_old;
    Memory.FixROMSpeed ();
    if (version == 3)
    {
	OrigCPU.Cycles = OrigCPU.Cycles_old;
	OrigCPU.NextEvent = OrigCPU.NextEvent_old;
	OrigCPU.V_Counter = OrigCPU.V_Counter_old;
	OrigCPU.MemSpeed = OrigCPU.MemSpeed_old;
	OrigCPU.MemSpeedx2 = OrigCPU.MemSpeedx2_old;
	OrigCPU.FastROMSpeed = OrigCPU.FastROMSpeed_old;
    }
    CPU.Flags = OrigCPU.Flags;
    CPU.BranchSkip = OrigCPU.BranchSkip;
    CPU.NMIActive = OrigCPU.NMIActive;
    CPU.IRQActive = OrigCPU.IRQActive;
    CPU.WaitingForInterrupt = OrigCPU.WaitingForInterrupt;
    CPU.WhichEvent = OrigCPU.WhichEvent;
    CPU.Cycles = OrigCPU.Cycles;
    CPU.NextEvent = OrigCPU.NextEvent;
    CPU.V_Counter = OrigCPU.V_Counter;
    CPU.MemSpeed = OrigCPU.MemSpeed;
    CPU.MemSpeedx2 = OrigCPU.MemSpeedx2;
    CPU.FastROMSpeed = OrigCPU.FastROMSpeed;

    if ((result = ReadBlock ("REG:", &OrigRegisters, sizeof (OrigRegisters), snap)) != SUCCESS)
	return (result);

    Registers = *(struct SRegisters *) &OrigRegisters;

    if ((result = ReadBlock ("PPU:", &OrigPPU, sizeof (OrigPPU), snap)) != SUCCESS)
	return (result);

    if (version == 2)
    {
	OrigPPU.OBJNameSelect = OrigPPU.OBJNameSelect_old << 13;
	OrigPPU.OBJNameBase <<= 1;
	OrigPPU.OBJNameSelect <<= 13;
    }
    PPU.BGMode = OrigPPU.BGMode;
    PPU.BG3Priority = OrigPPU.BG3Priority;
    PPU.Brightness = OrigPPU.Brightness;

    PPU.VMA.High = OrigPPU.VMA.High;
    PPU.VMA.Increment = OrigPPU.VMA.Increment;
    PPU.VMA.Address = OrigPPU.VMA.Address;
    PPU.VMA.Mask1 = OrigPPU.VMA.Mask1;
    PPU.VMA.FullGraphicCount = OrigPPU.VMA.FullGraphicCount;
    PPU.VMA.Shift = OrigPPU.VMA.Shift;

    for (i = 0; i < 4; i++)
    {
	PPU.BG[i].SCBase = OrigPPU.BG[i].SCBase;
	PPU.BG[i].VOffset = OrigPPU.BG[i].VOffset;
	PPU.BG[i].HOffset = OrigPPU.BG[i].HOffset;
	PPU.BG[i].BGSize = OrigPPU.BG[i].BGSize;
	PPU.BG[i].NameBase = OrigPPU.BG[i].NameBase;
	PPU.BG[i].SCSize = OrigPPU.BG[i].SCSize;
    }

    PPU.CGFLIP = OrigPPU.CGFLIP;
    for (i = 0; i < 256; i++)
	PPU.CGDATA [i] = OrigPPU.CGDATA [i];
    PPU.FirstSprite = OrigPPU.FirstSprite;
    for (i = 0; i < 128; i++)
    {
	PPU.OBJ[i].HPos = OrigPPU.OBJ [i].HPos;
	PPU.OBJ[i].VPos = OrigPPU.OBJ [i].VPos;
	PPU.OBJ[i].Name = OrigPPU.OBJ [i].Name;
	PPU.OBJ[i].VFlip = OrigPPU.OBJ [i].VFlip;
	PPU.OBJ[i].HFlip = OrigPPU.OBJ [i].HFlip;
	PPU.OBJ[i].Priority = OrigPPU.OBJ [i].Priority;
	PPU.OBJ[i].Palette = OrigPPU.OBJ [i].Palette;
	PPU.OBJ[i].Size = OrigPPU.OBJ [i].Size;
    }
    PPU.OAMPriorityRotation = OrigPPU.OAMPriorityRotation;
    PPU.OAMAddr = OrigPPU.OAMAddr;

    PPU.OAMFlip = OrigPPU.OAMFlip;
    PPU.OAMTileAddress = OrigPPU.OAMTileAddress;
    PPU.IRQVBeamPos = OrigPPU.IRQVBeamPos;
    PPU.IRQHBeamPos = OrigPPU.IRQHBeamPos;
    PPU.VBeamPosLatched = OrigPPU.VBeamPosLatched;
    PPU.HBeamPosLatched = OrigPPU.HBeamPosLatched;

    PPU.HBeamFlip = OrigPPU.HBeamFlip;
    PPU.VBeamFlip = OrigPPU.VBeamFlip;
    PPU.HVBeamCounterLatched = OrigPPU.HVBeamCounterLatched;

    PPU.MatrixA = OrigPPU.MatrixA;
    PPU.MatrixB = OrigPPU.MatrixB;
    PPU.MatrixC = OrigPPU.MatrixC;
    PPU.MatrixD = OrigPPU.MatrixD;
    PPU.CentreX = OrigPPU.CentreX;
    PPU.CentreY = OrigPPU.CentreY;

    PPU.CGADD = OrigPPU.CGADD;
    PPU.FixedColourRed = OrigPPU.FixedColourRed;
    PPU.FixedColourGreen = OrigPPU.FixedColourGreen;
    PPU.FixedColourBlue = OrigPPU.FixedColourBlue;
    PPU.SavedOAMAddr = OrigPPU.SavedOAMAddr;
    PPU.ScreenHeight = OrigPPU.ScreenHeight;
    PPU.WRAM = OrigPPU.WRAM;
    PPU.ForcedBlanking = OrigPPU.ForcedBlanking;
    PPU.OBJNameSelect = OrigPPU.OBJNameSelect;
    PPU.OBJSizeSelect = OrigPPU.OBJSizeSelect;
    PPU.OBJNameBase = OrigPPU.OBJNameBase;
    PPU.OAMReadFlip = OrigPPU.OAMReadFlip;
    memmove (PPU.OAMData, OrigPPU.OAMData, sizeof (PPU.OAMData));
    PPU.VTimerEnabled = OrigPPU.VTimerEnabled;
    PPU.HTimerEnabled = OrigPPU.HTimerEnabled;
    PPU.HTimerPosition = OrigPPU.HTimerPosition;
    PPU.Mosaic = OrigPPU.Mosaic;
    memmove (PPU.BGMosaic, OrigPPU.BGMosaic, sizeof (PPU.BGMosaic));
    PPU.Mode7HFlip = OrigPPU.Mode7HFlip;
    PPU.Mode7VFlip = OrigPPU.Mode7VFlip;
    PPU.Mode7Repeat = OrigPPU.Mode7Repeat;
    PPU.Window1Left = OrigPPU.Window1Left;
    PPU.Window1Right = OrigPPU.Window1Right;
    PPU.Window2Left = OrigPPU.Window2Left;
    PPU.Window2Right = OrigPPU.Window2Right;
    for (i = 0; i < 6; i++)
    {
	PPU.ClipWindowOverlapLogic [i] = OrigPPU.ClipWindowOverlapLogic [i];
	PPU.ClipWindow1Enable [i] = OrigPPU.ClipWindow1Enable [i];
	PPU.ClipWindow2Enable [i] = OrigPPU.ClipWindow2Enable [i];
	PPU.ClipWindow1Inside [i] = OrigPPU.ClipWindow1Inside [i];
	PPU.ClipWindow2Inside [i] = OrigPPU.ClipWindow2Inside [i];
    }
    PPU.CGFLIPRead = OrigPPU.CGFLIPRead;
    PPU.Need16x8Mulitply = OrigPPU.Need16x8Mulitply;

    IPPU.ColorsChanged = TRUE;
    IPPU.OBJChanged = TRUE;
    S9xFixColourBrightness ();
    IPPU.RenderThisFrame = FALSE;

    if ((result = ReadBlock ("DMA:", OrigDMA, sizeof (OrigDMA), snap)) != SUCCESS)
	return (result);

    for (i = 0; i < 8; i++)
    {
	DMA[i].ReverseTransfer = OrigDMA[i].ReverseTransfer;
	DMA[i].AAddressFixed = OrigDMA[i].AAddressFixed;
	DMA[i].AAddressDecrement = OrigDMA[i].AAddressDecrement;
	DMA[i].TransferMode = OrigDMA[i].TransferMode;
	DMA[i].ABank = OrigDMA[i].ABank;
	DMA[i].AAddress = OrigDMA[i].AAddress;
	DMA[i].Address = OrigDMA[i].Address;
	DMA[i].BAddress = OrigDMA[i].BAddress;
	DMA[i].HDMAIndirectAddressing = OrigDMA[i].HDMAIndirectAddressing;
	DMA[i].DMACount_Or_HDMAIndirectAddress = OrigDMA[i].IndirectAddress;
	DMA[i].IndirectBank = OrigDMA[i].IndirectBank;
	DMA[i].Repeat = OrigDMA[i].Repeat;
	DMA[i].LineCount = OrigDMA[i].LineCount;
	DMA[i].DoTransfer = OrigDMA[i].FirstLine;
    }

    if ((result = ReadBlock ("VRA:", Memory.VRAM, 0x10000, snap)) != SUCCESS)
	return (result);
    if ((result = ReadBlock ("RAM:", Memory.RAM, 0x20000, snap)) != SUCCESS)
	return (result);
    if ((result = ReadBlock ("SRA:", Memory.SRAM, 0x10000, snap)) != SUCCESS)
	return (result);
    if ((result = ReadBlock ("FIL:", Memory.FillRAM, 0x8000, snap)) != SUCCESS)
	return (result);
    if (ReadBlock ("APU:", &OrigAPU, sizeof (OrigAPU), snap) == SUCCESS)
    {
	APU = *(struct SAPU *) &OrigAPU;

	if ((result = ReadBlock ("ARE:", &OrigAPURegisters,
				 sizeof (OrigAPURegisters), snap)) != SUCCESS)
	    return (result);
	APURegisters = *(struct SAPURegisters *) &OrigAPURegisters;
	if ((result = ReadBlock ("ARA:", IAPU.RAM, 0x10000, snap)) != SUCCESS)
	    return (result);
	if ((result = ReadBlock ("SOU:", &OrigSoundData,
				 sizeof (SOrigSoundData), snap)) != SUCCESS)
	    return (result);

	SoundData.master_volume_left = OrigSoundData.master_volume_left;
	SoundData.master_volume_right = OrigSoundData.master_volume_right;
	SoundData.echo_volume_left = OrigSoundData.echo_volume_left;
	SoundData.echo_volume_right = OrigSoundData.echo_volume_right;
	SoundData.echo_enable = OrigSoundData.echo_enable;
	SoundData.echo_feedback = OrigSoundData.echo_feedback;
	SoundData.echo_ptr = OrigSoundData.echo_ptr;
	SoundData.echo_buffer_size = OrigSoundData.echo_buffer_size;
	SoundData.echo_write_enabled = OrigSoundData.echo_write_enabled;
	SoundData.echo_channel_enable = OrigSoundData.echo_channel_enable;
	SoundData.pitch_mod = OrigSoundData.pitch_mod;

	for (i = 0; i < 3; i++)
	    SoundData.dummy [i] = OrigSoundData.dummy [i];
	for (i = 0; i < NUM_CHANNELS; i++)
	{
	    SoundData.channels [i].state = OrigSoundData.channels [i].state;
	    SoundData.channels [i].type = OrigSoundData.channels [i].type;
	    SoundData.channels [i].volume_left = OrigSoundData.channels [i].volume_left;
	    SoundData.channels [i].volume_right = OrigSoundData.channels [i].volume_right;
	    SoundData.channels [i].hertz = OrigSoundData.channels [i].frequency;
	    SoundData.channels [i].count = OrigSoundData.channels [i].count;
	    SoundData.channels [i].loop = OrigSoundData.channels [i].loop;
	    SoundData.channels [i].envx = OrigSoundData.channels [i].envx;
	    SoundData.channels [i].left_vol_level = OrigSoundData.channels [i].left_vol_level;
	    SoundData.channels [i].right_vol_level = OrigSoundData.channels [i].right_vol_level;
	    SoundData.channels [i].envx_target = OrigSoundData.channels [i].envx_target;
	    SoundData.channels [i].env_error = OrigSoundData.channels [i].env_error;
	    SoundData.channels [i].erate = OrigSoundData.channels [i].erate;
	    SoundData.channels [i].direction = OrigSoundData.channels [i].direction;
	    SoundData.channels [i].attack_rate = OrigSoundData.channels [i].attack_rate;
	    SoundData.channels [i].decay_rate = OrigSoundData.channels [i].decay_rate;
	    SoundData.channels [i].sustain_rate = OrigSoundData.channels [i].sustain_rate;
	    SoundData.channels [i].release_rate = OrigSoundData.channels [i].release_rate;
	    SoundData.channels [i].sustain_level = OrigSoundData.channels [i].sustain_level;
	    SoundData.channels [i].sample = OrigSoundData.channels [i].sample;
	    for (j = 0; j < 16; j++)
		SoundData.channels [i].decoded [j] = OrigSoundData.channels [i].decoded [j];

	    for (j = 0; j < 2; j++)
		SoundData.channels [i].previous [j] = OrigSoundData.channels [i].previous [j];

	    SoundData.channels [i].sample_number = OrigSoundData.channels [i].sample_number;
	    SoundData.channels [i].last_block = OrigSoundData.channels [i].last_block;
	    SoundData.channels [i].needs_decode = OrigSoundData.channels [i].needs_decode;
	    SoundData.channels [i].block_pointer = OrigSoundData.channels [i].block_pointer;
	    SoundData.channels [i].sample_pointer = OrigSoundData.channels [i].sample_pointer;
	    SoundData.channels [i].mode = OrigSoundData.channels [i].mode;
	}

	S9xSetSoundMute (FALSE);
	IAPU.PC = IAPU.RAM + APURegisters.PC;
	S9xAPUUnpackStatus ();
	if (APUCheckDirectPage ())
	    IAPU.DirectPage = IAPU.RAM + 0x100;
	else
	    IAPU.DirectPage = IAPU.RAM;
	Settings.APUEnabled = TRUE;
	IAPU.APUExecuting = TRUE;
    }
    else
    {
	Settings.APUEnabled = FALSE;
	IAPU.APUExecuting = FALSE;
	S9xSetSoundMute (TRUE);
    }
    S9xFixSoundAfterSnapshotLoad (1);
    ICPU.ShiftedPB = Registers.PB << 16;
    ICPU.ShiftedDB = Registers.DB << 16;
    S9xSetPCBase (Registers.PBPC);
    S9xUnpackStatus ();
    S9xFixCycles ();
    S9xReschedule ();

    return (SUCCESS);
}

