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

#include "neopop.h"
#include "mem.h"
#include "sound.h"
#include "Z80_interface.h"
#include "TLCS900h_registers.h"
#include "interrupt.h"
#include "dma.h"

static uint8 CommByte;
static bool Z80Enabled;

uint8 Z80_ReadComm(void)
{
 return(CommByte);
}

void Z80_WriteComm(uint8 data)
{
 CommByte = data;
}

static uint8 NGP_z80_readbyte(uint16 address)
{
	if (address <= 0xFFF)
		return loadB(0x7000 + address);

	if (address == 0x8000)
	{
		return CommByte;
	}
	return 0;
}

//=============================================================================

static void NGP_z80_writebyte(uint16 address, uint8 value)
{
	if (address <= 0x0FFF)
	{
		storeB(0x7000 + address, value);
		return;
	}

	if (address == 0x8000)
	{
		CommByte = value;
		return;
	}

	if (address == 0x4001)	{	Write_SoundChipLeft(value); return; }
	if (address == 0x4000)	{	Write_SoundChipRight(value); return; }

	if (address == 0xC000)
	{
		TestIntHDMA(6, 0x0C);
	}
}

//=============================================================================

static void NGP_z80_writeport(uint16 port, uint8 value)
{
	//printf("Portout: %04x %02x\n", port, value);
	z80_set_interrupt(0);
}

static uint8 NGP_z80_readport(uint16 port)
{
	//printf("Portin: %04x\n", port);
	return 0;
}

void Z80_nmi(void)
{
	z80_nmi();
}

void Z80_irq(void)
{
	z80_set_interrupt(1);
}

void Z80_reset(void)
{
	Z80Enabled = 0;

	z80_writebyte = NGP_z80_writebyte;
	z80_readbyte = NGP_z80_readbyte;
	z80_writeport = NGP_z80_writeport;
	z80_readport = NGP_z80_readport;

	z80_init();
	z80_reset();
}

void Z80_SetEnable(bool set)
{
 Z80Enabled = set;
 if(!set)
  z80_reset();
}

bool Z80_IsEnabled(void)
{
 return(Z80Enabled);
}

int Z80_RunOP(void)
{
 if(!Z80Enabled)
  return(-1);

 return(z80_do_opcode());
}

int MDFNNGPCZ80_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(CommByte),
  SFVAR(Z80Enabled),
  SFEND
 };

 if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, "Z80X"))
 {
  return(0);
 }

 if(!z80_state_action(sm, load, data_only, "Z80"))
  return(0);

 return(1);
}
