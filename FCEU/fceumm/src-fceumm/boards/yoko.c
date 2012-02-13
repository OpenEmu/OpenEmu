/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2006 CaH4e3
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
 *
 * YOKO mapper, almost the same as 83, TODO: merge
 *
 * Mortal Kombat 2 YOKO
 * N-CXX(M), XX - размеры PRG+CHR, 12 - 128+256, 22 - 256+256, 14 - 128+512
 * 
 */

#include "mapinc.h"

static uint8 mode, bank, reg[8], low[4], dip, IRQa;
static int32 IRQCount;

static SFORMAT StateRegs[]=
{
  {&mode, 1, "MODE"},
  {&bank, 1, "BANK"},
  {&IRQCount, 4, "IRQC"},
  {&IRQa, 1, "IRQA"},
  {reg, 8, "REGS"},
  {low, 4, "LOWR"},
  {0}
};

static void Sync(void)
{
  setmirror((mode & 1)^1);
  setchr2(0x0000,reg[3]);
  setchr2(0x0800,reg[4]);
  setchr2(0x1000,reg[5]);
  setchr2(0x1800,reg[6]);
  if(mode & 0x10)
  {
    uint32 base = (bank & 8) << 1;
    setprg8(0x8000,(reg[0]&0x0f)|base);
    setprg8(0xA000,(reg[1]&0x0f)|base);
    setprg8(0xC000,(reg[2]&0x0f)|base);
    setprg8(0xE000,0x0f|base);
  }
  else
  {
    if(mode & 8)
      setprg32(0x8000,bank >> 1);
    else
    {
      setprg16(0x8000,bank);
      setprg16(0xC000,~0);
    }
  }
}

static DECLFW(MYOKOWrite)
{
/*  FCEU_printf("bs %04x %02x\n",A,V);*/
  switch(A & 0x8C17)
  {
    case 0x8000: bank=V; break;
    case 0x8400: mode=V; break;
    case 0x8800: IRQCount&=0xFF00; IRQCount|=V; X6502_IRQEnd(FCEU_IQEXT); break;
    case 0x8801: IRQa=mode&0x80; IRQCount&=0xFF; IRQCount|=V<<8; break;
    case 0x8c00: reg[0]=V; break;
    case 0x8c01: reg[1]=V; break;
    case 0x8c02: reg[2]=V; break;
    case 0x8c10: reg[3]=V; break;
    case 0x8c11: reg[4]=V; break;
    case 0x8c16: reg[5]=V; break;
    case 0x8c17: reg[6]=V; break;
  }
  Sync();
}

static DECLFR(MYOKOReadDip)
{
/*  FCEU_printf("read %04x\n",A);*/
  return (X.DB&0xFC)|dip;
}

static DECLFR(MYOKOReadLow)
{
/*  FCEU_printf("read %04x\n",A);*/
  return low[A & 3];
}

static DECLFW(MYOKOWriteLow)
{
/*  FCEU_printf("bs %04x\n",A);*/
  low[A & 3] = V;
}

static void MYOKOPower(void)
{
  mode = bank = 0;
  dip = 3;
  Sync();  
  SetReadHandler(0x5000,0x53FF,MYOKOReadDip);
  SetReadHandler(0x5400,0x5FFF,MYOKOReadLow);
  SetWriteHandler(0x5400,0x5FFF,MYOKOWriteLow);
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xFFFF,MYOKOWrite);
}

static void MYOKOReset(void)
{
  dip = (dip + 1)&3;
  mode = bank = 0;
  Sync();  
}

static void MYOKOIRQHook(int a)
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

static void StateRestore(int version)
{
  Sync();
}

void UNLYOKO_Init(CartInfo *info)
{
  info->Power=MYOKOPower;
  info->Reset=MYOKOReset;
  MapIRQHook=MYOKOIRQHook;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
