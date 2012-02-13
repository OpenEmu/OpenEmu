/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel 2006 CaH4e3
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
 */

#include "mapinc.h"

static uint8 laststrobe, trigger;
static uint8 reg[8];
static uint8 *WRAM=NULL;
static uint32 WRAMSIZE;

static void(*WSync)(void);

static SFORMAT StateRegs[]=
{
  {&laststrobe, 1, "STB"},
  {&trigger, 1, "TRG"},
  {reg, 8, "REGS"},
  {0}
};

static void Sync(void)
{
  setprg8r(0x10,0x6000,0);
  setprg32(0x8000,(reg[0]<<4)|(reg[1]&0xF));
  setchr8(0);
}

static void StateRestore(int version)
{
  WSync();
}

static DECLFR(ReadLow)
{
  switch (A&0x7700)
  {
    case 0x5100: return reg[2]|reg[0]|reg[1]|reg[3]^0xff; break;
    case 0x5500: if(trigger)
                   return reg[2]|reg[1]; /* Lei Dian Huang Bi Ka Qiu Chuan Shuo (NJ046) may broke other games*/
                 else
                   return 0;
  }
  return 4;
}

static void M163HB(void)
{
	if(reg[1]&0x80)
	{
		if(scanline==239)
		{
			setchr4(0x0000,0);
			setchr4(0x1000,0);
		}
		else if(scanline==127)
		{
			setchr4(0x0000,1);
			setchr4(0x1000,1);
		}
		/* Hu Lu Jin Gang (NJ039) (Ch) [!] don't like it */

		/*
		   if(scanline>=127)
		   {
		   setchr4(0x0000,1);
		   setchr4(0x1000,1);
		   }
		   else
		   {
		   setchr4(0x0000,0);
		   setchr4(0x1000,0);
		   }
		 */      
	}
}

static DECLFW(Write)
{
  switch (A&0x7300)
  {
    case 0x5100: reg[0]=V; WSync(); break;
    case 0x5000: reg[1]=V; WSync(); break;
    case 0x5200: reg[3]=V; WSync(); break;
    case 0x5300: reg[2]=V; break;
  }
}

static void Power(void)
{
  memset(reg,0,8);
  reg[1]=0xFF;
  SetWriteHandler(0x5000,0x5FFF,Write);
  SetReadHandler(0x6000,0xFFFF,CartBR);
  SetWriteHandler(0x6000,0x7FFF,CartBW);
  WSync();
}

static void Close(void)
{
  if(WRAM)
    free(WRAM);
  WRAM=NULL;
}

void Mapper164_Init(CartInfo *info)
{
  info->Power=Power;
  info->Close=Close;
  WSync = Sync;

  WRAMSIZE = 8192;
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

static DECLFW(Write2)
{
  if(A==0x5101)
  {
    if(laststrobe&&!V)
    {
      trigger^=1;
    }
    laststrobe=V;
  }else if(A==0x5100&&V==6) /*damn thoose protected games*/
    setprg32(0x8000,3);
  else
  switch (A&0x7300)
  {
    case 0x5200: reg[0]=V; WSync(); break;
    case 0x5100: reg[3]=V; WSync(); break;
    case 0x5000: reg[1]=V; WSync(); if(!(reg[1]&0x80)&&(scanline<128)) setchr8(0); /* setchr8(0); */ break;
    case 0x5300: reg[2]=V; break;
  }
}

static void Power2(void)
{
  memset(reg,0,8);
  laststrobe=1;
  SetReadHandler(0x5000,0x5FFF,ReadLow);
  SetWriteHandler(0x5000,0x5FFF,Write2);
  SetReadHandler(0x6000,0xFFFF,CartBR);
  SetWriteHandler(0x6000,0x7FFF,CartBW);
  WSync();
}

void Mapper163_Init(CartInfo *info)
{
  info->Power=Power2;
  info->Close=Close;
  WSync = Sync;
  GameHBIRQHook=M163HB;

  WRAMSIZE = 8192;
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

static void Sync3(void)
{
  setchr8(0);
  setprg8r(0x10,0x6000,0);
  setprg32(0x8000,(reg[0]&0xe)|((reg[1]>>1)&1)|((reg[2]&15)<<4));
}

static DECLFW(Write3)
{
/*  FCEU_printf("bs %04x %02x\n",A,V);*/
  reg[(A>>8)&3]=V;
  WSync();
}

static void Power3(void)
{
  memset(reg,0,8);
  SetWriteHandler(0x5000,0x5FFF,Write3);
  SetReadHandler(0x6000,0xFFFF,CartBR);
  SetWriteHandler(0x6000,0x7FFF,CartBW);
  WSync();
}

void UNLFS304_Init(CartInfo *info)
{
  info->Power=Power3;
  info->Close=Close;
  WSync = Sync3;
  
  WRAMSIZE = 8192;
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
