/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2007 CaH4e3
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Bandai mappers
 *
 */

#include "mapinc.h"

static uint8 reg[16], is153;
static uint8 IRQa;
static int16 IRQCount, IRQLatch;

static uint8 *WRAM=NULL;
static uint32 WRAMSIZE;

static SFORMAT StateRegs[]=
{
  {reg, 16, "REGS"},
  {&IRQa, 1, "IRQA"},
  {&IRQCount, 2, "IRQC"},
  {&IRQLatch, 2, "IRQL"}, /* need for Famicom Jump II - Saikyou no 7 Nin (J) [!]*/
  {0}
};

static void BandaiIRQHook(int a)
{
	if(IRQa)
	{
		IRQCount-=a;
		if(IRQCount<0)
		{
			X6502_IRQBegin(FCEU_IQEXT);
			IRQa=0;
			IRQCount=0xFFFF;
		}
	}
}

static void BandaiSync(void)
{
  if(is153)
  {
    int base=(reg[0]&1)<<4;
    if(!UNIFchrrama)
    {
     int i;
     for(i=0; i<8; i++) setchr1(i<<10,reg[i]);
    }
    else
     setchr8(0);
    setprg16(0x8000,(reg[8]&0x0F)|base);
    setprg16(0xC000,0x0F|base);
  }
  else
  {
    int i;
    for(i=0; i<8; i++) setchr1(i<<10,reg[i]);
    setprg16(0x8000,reg[8]);
    setprg16(0xC000,~0);
  }
  switch(reg[9]&3)
  {
    case 0: setmirror(MI_V); break;
    case 1: setmirror(MI_H); break;
    case 2: setmirror(MI_0); break;
    case 3: setmirror(MI_1); break;
  }  
}

static DECLFW(BandaiWrite)
{
	A&=0x0F;  
	if(A<0x0A)
	{
		reg[A&0x0F]=V;
		BandaiSync();
	}
	else
		switch(A)
		{
			case 0x0A: X6502_IRQEnd(FCEU_IQEXT); IRQa=V&1; IRQCount=IRQLatch; break;
			case 0x0B: IRQLatch&=0xFF00; IRQLatch|=V; break;
			case 0x0C: IRQLatch&=0xFF; IRQLatch|=V<<8; break;
			case 0x0D: break;/* Serial EEPROM control port */
		}
}

static void BandaiPower(void)
{
  BandaiSync();
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x6000,0xFFFF,BandaiWrite);
}

static void M153Power(void)
{
  BandaiSync();
  setprg8r(0x10,0x6000,0);
  SetReadHandler(0x6000,0x7FFF,CartBR);
  SetWriteHandler(0x6000,0x7FFF,CartBW);
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xFFFF,BandaiWrite);
}


static void M153Close(void)
{
  if(WRAM)
    free(WRAM);
  WRAM=NULL;
}

static void StateRestore(int version)
{
  BandaiSync();
}

void Mapper16_Init(CartInfo *info)
{
  is153=0;
  info->Power=BandaiPower;
  MapIRQHook=BandaiIRQHook;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}

/* Famicom jump 2:*/
/* 0-7: Lower bit of data selects which 256KB PRG block is in use.*/
/* This seems to be a hack on the developers' part, so I'll make emulation*/
/* of it a hack(I think the current PRG block would depend on whatever the*/
/* lowest bit of the CHR bank switching register that corresponds to the*/
/* last CHR address read).*/

void Mapper153_Init(CartInfo *info)
{
	is153=1;
	info->Power=M153Power;
	info->Close=M153Close;
	MapIRQHook=BandaiIRQHook;

	WRAMSIZE=8192;
	WRAM=(uint8*)FCEU_gmalloc(WRAMSIZE);
	SetupCartPRGMapping(0x10,WRAM,WRAMSIZE,1);
	AddExState(WRAM, WRAMSIZE, 0, "WRAM");

	if(info->battery)
	{
		info->SaveGame[0]=WRAM;
		info->SaveGameLen[0]=WRAMSIZE;
	}

	GameStateRestore=StateRestore;
	AddExState(&StateRegs, ~0, 0, 0);
}
