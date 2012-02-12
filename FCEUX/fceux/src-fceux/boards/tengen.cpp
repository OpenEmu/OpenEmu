/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
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

static uint8 cmd,mir,rmode,IRQmode;
static uint8 DRegs[11];
static uint8 IRQCount,IRQa,IRQLatch;

static SFORMAT Rambo_StateRegs[]={
  {&cmd, 1, "CMD"},
  {&mir, 1, "MIR"},
  {&rmode, 1, "RMOD"},
  {&IRQmode, 1, "IRQM"},
  {&IRQCount, 1, "IRQC"},
  {&IRQa, 1, "IRQA"},
  {&IRQLatch, 1, "IRQL"},
  {DRegs, 11, "DREG"},
  {0}
};

static void (*setchr1wrap)(unsigned int A, unsigned int V);
//static int nomirror;

static void RAMBO1_IRQHook(int a)
{
  static int smallcount;
  if(!IRQmode) return;

  smallcount+=a;
  while(smallcount>=4)
  {
    smallcount-=4;
    IRQCount--;
    if(IRQCount==0xFF)
      if(IRQa) X6502_IRQBegin(FCEU_IQEXT);
  }
}

static void RAMBO1_hb(void)
{
  if(IRQmode) return;
  if(scanline==240) return;        /* hmm.  Maybe that should be an mmc3-only call in fce.c. */
  rmode=0;
  IRQCount--;
  if(IRQCount==0xFF)
  {
    if(IRQa)
    {
      rmode = 1;
      X6502_IRQBegin(FCEU_IQEXT);
    }
  }
}

static void Synco(void)
{
  int x;

  if(cmd&0x20)
  {
    setchr1wrap(0x0000,DRegs[0]);
    setchr1wrap(0x0800,DRegs[1]);
    setchr1wrap(0x0400,DRegs[8]);
    setchr1wrap(0x0c00,DRegs[9]);
  }
  else
  {
    setchr1wrap(0x0000,(DRegs[0]&0xFE));
    setchr1wrap(0x0400,(DRegs[0]&0xFE)|1);
    setchr1wrap(0x0800,(DRegs[1]&0xFE));
    setchr1wrap(0x0C00,(DRegs[1]&0xFE)|1);
  }

  for(x=0;x<4;x++)
     setchr1wrap(0x1000+x*0x400,DRegs[2+x]);

  setprg8(0x8000,DRegs[6]);
  setprg8(0xA000,DRegs[7]);

  setprg8(0xC000,DRegs[10]);
}


static DECLFW(RAMBO1_write)
{
  switch(A&0xF001)
  {
    case 0xa000: mir=V&1;
//                 if(!nomirror)
                   setmirror(mir^1);
                 break;
    case 0x8000: cmd = V;
                 break;
    case 0x8001: if((cmd&0xF)<10)
                   DRegs[cmd&0xF]=V;
                 else if((cmd&0xF)==0xF)
                   DRegs[10]=V;
                 Synco();
                 break;
    case 0xc000: IRQLatch=V;
                 if(rmode==1)
                   IRQCount=IRQLatch;
                 break;
    case 0xc001: rmode=1;
                 IRQCount=IRQLatch;
                 IRQmode=V&1;
                 break;
    case 0xE000: IRQa=0;
                 X6502_IRQEnd(FCEU_IQEXT);
                 if(rmode==1)
                   IRQCount=IRQLatch;
                 break;
    case 0xE001: IRQa=1;
                 if(rmode==1)
                   IRQCount=IRQLatch;
                 break;
  }
}

static void RAMBO1_Restore(int version)
{
  Synco();
//  if(!nomirror)
    setmirror(mir^1);
}

static void RAMBO1_init(void)
{
  int x;
  for(x=0;x<11;x++)
     DRegs[x]=~0;
  cmd=mir=0;
//  if(!nomirror)
    setmirror(1);
  Synco();
  GameHBIRQHook=RAMBO1_hb;
  MapIRQHook=RAMBO1_IRQHook;
  GameStateRestore=RAMBO1_Restore;
  SetWriteHandler(0x8000,0xffff,RAMBO1_write);
  AddExState(Rambo_StateRegs, ~0, 0, 0);
}

static void CHRWrap(unsigned int A, unsigned int V)
{
  setchr1(A,V);
}

void Mapper64_init(void)
{
  setchr1wrap=CHRWrap;
//  nomirror=0;
  RAMBO1_init();
}
/*
static int MirCache[8];
static unsigned int PPUCHRBus;

static void MirWrap(unsigned int A, unsigned int V)
{
  MirCache[A>>10]=(V>>7)&1;
  if(PPUCHRBus==(A>>10))
    setmirror(MI_0+((V>>7)&1));
  setchr1(A,V);
}

static void MirrorFear(uint32 A)
{
  A&=0x1FFF;
  A>>=10;
  PPUCHRBus=A;
  setmirror(MI_0+MirCache[A]);
}

void Mapper158_init(void)
{
  setchr1wrap=MirWrap;
  PPU_hook=MirrorFear;
  nomirror=1;
  RAMBO1_init();
}
*/

