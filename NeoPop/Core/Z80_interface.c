//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

/*
//---------------------------------------------------------------------------
//=========================================================================

	Z80_interface.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

25 JUL 2002 - neopop_uk
=======================================
- Added Z80 dma triggering.

22 AUG 2002 - neopop_uk
=======================================
- Added correct DMA chaining behaviour.

//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include "mem.h"
#include "sound.h"
#include "Z80_interface.h"
#include "TLCS900h_registers.h"
#include "interrupt.h"
#include "dma.h"

//=============================================================================

int DAsm(char *S,byte *A);

Z80 Z80_regs;

//=============================================================================

_u8 RdZ80(_u16 address)
{
	if (address <= 0xFFF)
		return ram[0x7000 + address];

	if (address == 0x8000)
	{
#ifdef NEOPOP_DEBUG
		if (filter_sound)
			system_debug_message("z80 <- TLCS900h: Read ... %02X", ram[0xBC]);
#endif
		return ram[0xBC];
	}

	return 0;
}

//=============================================================================

void WrZ80(_u16 address, _u8 value)
{
	if (address <= 0x0FFF)
	{
		ram[0x7000 + address] = value;
		return;
	}

	if (address == 0x8000)
	{
#ifdef NEOPOP_DEBUG
		if (filter_sound)
			system_debug_message("z80 -> TLCS900h: Write ... %02X", value);
#endif
		ram[0xBC] = value;
		return;
	}

	if (address == 0x4001)	{	Write_SoundChipTone(value);	return; }
	if (address == 0x4000)	{	Write_SoundChipNoise(value); return; }

	if (address == 0xC000 && (statusIFF() <= (ram[0x71] & 0x7)))
	{
		interrupt(6); // Z80 Int.

		if (ram[0x007C] == 0x0C)		DMA_update(0);
		else { if (ram[0x007D] == 0x0C)	DMA_update(1);
		else { if (ram[0x007E] == 0x0C)	DMA_update(2);
		else { if (ram[0x007F] == 0x0C)	DMA_update(3);	}}}
	}
}

//=============================================================================

void OutZ80(_u16 port, _u8 value)
{
#ifdef NEOPOP_DEBUG
//	if (filter_sound) system_debug_message("Z80: Port out %04X <= %02X", port, value);
#endif
}

_u8 InZ80(_u16 port)
{
#ifdef NEOPOP_DEBUG
//	if (filter_sound) system_debug_message("Z80: Port in %04X", port);
#endif
	return 0;
}

//=============================================================================

void PatchZ80(register Z80 *R)
{
	// Empty
}

word LoopZ80(register Z80 *R)
{
	return INT_QUIT;
}

//=============================================================================

void Z80_nmi(void)
{
	IntZ80(&Z80_regs, INT_NMI);
}

void Z80_irq(void)
{
	Z80_regs.IFF |= IFF_1;
	IntZ80(&Z80_regs, INT_IRQ);
}

void Z80_reset(void)
{
	ResetZ80(&Z80_regs);
	Z80_regs.SP.W = 0;
}

//=============================================================================

_u16 Z80_getReg(_u8 reg)
{
	_u16* r = (_u16*)&Z80_regs;
	return r[reg];
}

void Z80_setReg(_u8 reg, _u16 value)
{
	_u16* r = (_u16*)&Z80_regs;
	r[reg] = value;
}

//=============================================================================

char* Z80_disassemble(_u16* pc_in)
{
	int bcnt, i;
	_u16 pc = *pc_in;
	char instr[64];	//Print the disassembled instruction to this string
	_u8 str[80];
	memset(str, 0, 80);
	
	//Add the program counter
	sprintf(str, "<z80> %03X: ", pc);

	//Disassemble instruction
    bcnt = DAsm(instr,ram + 0x7000 + pc);

	//Add the instruction
	strcat(str, instr);

	//Add the bytes used
	for (i = strlen(str); i < 32; i++)
		str[i] = ' ';
	str[32] = '\"';
	for (i = 0; i < bcnt; i++)
	{
		_u8 tmp[80];
		sprintf(tmp, "%02X ", *(ram + 0x7000 + pc + i));
		strcat(str, tmp);
	}
	str[strlen(str) - 1] = '\"';

	*pc_in = pc + bcnt;
	return strdup(str);
}

//=============================================================================
