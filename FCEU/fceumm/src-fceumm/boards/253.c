/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2009 CaH4e3
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
 * TODO: cram disable cases! (Shen Hua Jian Yun III cannot boot with CRAM enabled)
 */

#include "mapinc.h"

static uint8 chrlo[8], chrhi[8], prg[2], mirr;
static int32 IRQa, IRQCount, IRQLatch, IRQClock; 
static uint8 *WRAM=NULL;
static uint32 WRAMSIZE;
static uint8 *CHRRAM=NULL;
static uint32 CHRRAMSIZE;

static SFORMAT StateRegs[]=
{
  {chrlo, 8, "CHRLO"},
  {chrhi, 8, "CHRHI"},
  {prg, 2, "PRGR"},
  {&mirr, 1, "MIRR"},
  {&IRQa, 4, "IRQA"},
  {&IRQCount, 4, "IRQC"},
  {&IRQLatch, 4, "IRQL"},
  {&IRQClock, 4, "IRQCL"},
  {0}
};

static void Sync(void)
{
  uint8 i;
  setprg8r(0x10,0x6000,0);
  setprg8(0x8000,prg[0]);
  setprg8(0xa000,prg[1]);
  setprg8(0xc000,~1);
  setprg8(0xe000,~0);
  for(i=0; i<8; i++)
  {
    uint32 chr = chrlo[i]|(chrhi[i]<<8);
    if((chr==4)||(chr==5))
      setchr1r(0x10,i<<10,chr&1);
    else
      setchr1(i<<10,chr);
  }
  switch(mirr)
  {
    case 0: setmirror(MI_V); break;
    case 1: setmirror(MI_H); break;
    case 2: setmirror(MI_0); break;
    case 3: setmirror(MI_1); break;
  }
}

static DECLFW(M253Write)
{
  if((A>=0xB000)&&(A<=0xE00C))
  {
    uint8 ind=((((A&8)|(A>>8))>>3)+2)&7;
    uint8 sar=A&4;
    chrlo[ind]=(chrlo[ind]&(0xF0>>sar))|((V&0x0F)<<sar);
    if(A&4)
      chrhi[ind]=V>>4;
    Sync();
  }
  else
   switch(A)
    {
      case 0x8010: prg[0]=V; Sync(); break;
      case 0xA010: prg[1]=V; Sync(); break;
      case 0x9400: mirr=V&3; Sync(); break;
      case 0xF000: IRQLatch = (IRQLatch & 0xF0) | (V & 0x0F); break;
      case 0xF004: IRQLatch = (IRQLatch & 0x0F) | (V << 4); break;
      case 0xF008:
        IRQa = V&3;
        if(IRQa&2)
        {
          IRQCount = IRQLatch;
          IRQClock = 0;
        }
        X6502_IRQEnd(FCEU_IQEXT);
        break;
    }
}

static void M253Power(void)
{
  Sync();
  SetReadHandler(0x6000,0x7FFF,CartBR);
  SetWriteHandler(0x6000,0x7FFF,CartBW);
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xFFFF,M253Write);
}

static void M253Close(void)
{
  if(WRAM)
    free(WRAM);
  if(CHRRAM)
    free(CHRRAM);
  WRAM=CHRRAM=NULL;
}

static void M253IRQ(int cycles)
{
  if(IRQa&2) 
  {
    if((IRQClock+=cycles)>=0x71) 
    {
      IRQClock -= 0x71;
      if(IRQCount==0xFF)  
      {
        IRQCount = IRQLatch;
        IRQa = IRQa|((IRQa&1)<<1);
        X6502_IRQBegin(FCEU_IQEXT);
      }
      else 
        IRQCount++;
    }
  }
}

static void StateRestore(int version)
{
  Sync();
}

void Mapper253_Init(CartInfo *info)
{
  info->Power=M253Power;
  info->Close=M253Close;
  MapIRQHook=M253IRQ;
  GameStateRestore=StateRestore;

  CHRRAMSIZE=4096;
  CHRRAM=(uint8*)FCEU_gmalloc(CHRRAMSIZE);
  SetupCartCHRMapping(0x10,CHRRAM,CHRRAMSIZE,1);
  AddExState(CHRRAM, CHRRAMSIZE, 0, "CRAM");

  WRAMSIZE=8192;
  WRAM=(uint8*)FCEU_gmalloc(WRAMSIZE);
  SetupCartPRGMapping(0x10,WRAM,WRAMSIZE,1);
  AddExState(WRAM, WRAMSIZE, 0, "WRAM");
  if(info->battery)
  {
    info->SaveGame[0]=WRAM;
    info->SaveGameLen[0]=WRAMSIZE;
  }

  AddExState(&StateRegs, ~0, 0, 0);
}
