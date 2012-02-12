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
 */

#include "mapinc.h"

static uint8 reg[4], IRQa;
static int16 IRQCount, IRQPause;

static int16 Count = 0x0000;

static SFORMAT StateRegs[]=
{
  {reg, 4, "REGS"},
  {&IRQa, 1, "IRQA"},
  {&IRQCount, 2, "IRQC"},
  {0}
};

static void Sync(void)
{
  setprg32(0x8000,0);
  setchr8(0);
}

//#define Count 0x1800
#define Pause 0x010

static DECLFW(UNL3DBlockWrite)
{
  switch(A)
  {
//4800 32
//4900 37
//4a00 01
//4e00 18
    case 0x4800: reg[0]=V; break;
    case 0x4900: reg[1]=V; break;
    case 0x4a00: reg[2]=V; break;
    case 0x4e00: reg[3]=V; IRQCount=Count; IRQPause=Pause; IRQa=1; X6502_IRQEnd(FCEU_IQEXT); break;
  } 
}

static void UNL3DBlockPower(void)
{
  Sync();
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x4800,0x4E00,UNL3DBlockWrite);
}

static void UNL3DBlockReset(void)
{
  Count+=0x10;
  FCEU_printf("Count=%04x\n",Count);
}

static void UNL3DBlockIRQHook(int a)
{
  if(IRQa)
  {
    if(IRQCount>0)
    {
      IRQCount-=a;
    }
    else
    {
      if(IRQPause>0)
      {
        IRQPause-=a;
        X6502_IRQBegin(FCEU_IQEXT);
      }
      else
      {
        IRQCount=Count; 
        IRQPause=Pause;
        X6502_IRQEnd(FCEU_IQEXT);
      }
    }
  }
}

static void StateRestore(int version)
{
  Sync();
}

void UNL3DBlock_Init(CartInfo *info)
{
  info->Power=UNL3DBlockPower;
  info->Reset=UNL3DBlockReset;
  MapIRQHook=UNL3DBlockIRQHook;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
