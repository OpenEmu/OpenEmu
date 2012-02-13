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
 * FDS Conversion
 *
 */

#include "mapinc.h"

static uint8 reg, mirr;
static int32 IRQa, IRQCount, IRQLatch;
static uint8 *WRAM=NULL;
static uint32 WRAMSIZE;

static SFORMAT StateRegs[]=
{
  {&mirr, 1, "MIRR"},
  {&reg, 1, "REGS"},
  {&IRQa, 4, "IRQA"},
  {&IRQCount, 4, "IRQC"},
  {&IRQLatch, 4, "IRQL"},
  {0}
};

static void Sync(void)
{
  setprg16(0x8000,reg);
  setprg16(0xC000,2);
  setmirror(mirr);
}

static DECLFW(UNLKS7017Write)
{
/*  FCEU_printf("bs %04x %02x\n",A,V);*/
    if((A & 0xFF00) == 0x4A00)
    {
      reg = ((A >> 2) & 3)|((A >> 4) & 4);
    }
    else if ((A & 0xFF00) == 0x5100)
    {
      Sync();
    }
    else if (A == 0x4020)
    {
      X6502_IRQEnd(FCEU_IQEXT);
      IRQCount&=0xFF00;
      IRQCount|=V;
    }
    else if (A == 0x4021)
    {
      X6502_IRQEnd(FCEU_IQEXT);
      IRQCount&=0xFF;
      IRQCount|=V<<8;
      IRQa = 1;
    }
    else if (A == 0x4025)
    {
      mirr = ((V & 8) >> 3) ^ 1;
    }
}

static DECLFR(FDSRead4030)
{
  X6502_IRQEnd(FCEU_IQEXT);
  return X.IRQlow&FCEU_IQEXT?1:0;
}

static void UNL7017IRQ(int a)
{
 if(IRQa)
 {
  IRQCount-=a;
  if(IRQCount<=0)
  {
    IRQa=0;
    X6502_IRQBegin(FCEU_IQEXT);
  }
 }
}

static void UNLKS7017Power(void)
{
  Sync();
  setchr8(0);
  setprg8r(0x10,0x6000,0);
  SetReadHandler(0x6000,0x7FFF,CartBR);
  SetWriteHandler(0x6000,0x7FFF,CartBW);
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetReadHandler(0x4030,0x4030,FDSRead4030);
  SetWriteHandler(0x4020,0x5FFF,UNLKS7017Write);
}

static void UNLKS7017Close(void)
{
  if(WRAM)
    free(WRAM);
  WRAM=NULL;
}

static void StateRestore(int version)
{
  Sync();
}

void UNLKS7017_Init(CartInfo *info)
{
  info->Power=UNLKS7017Power;
  info->Close=UNLKS7017Close;
  MapIRQHook=UNL7017IRQ;

  WRAMSIZE=8192;
  WRAM=(uint8*)FCEU_gmalloc(WRAMSIZE);
  SetupCartPRGMapping(0x10,WRAM,WRAMSIZE,1);
  AddExState(WRAM, WRAMSIZE, 0, "WRAM");

  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
