/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2005 CaH4e3
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
 * CAI Shogakko no Sansu
 */

#include "mapinc.h"

static uint8 QTAINTRAM[2048];
static writefunc old2007wrap;

static uint16 CHRSIZE = 8192;
static uint16 WRAMSIZE = 8192 + 4096;
static uint8 *CHRRAM = NULL;
static uint8 *WRAM = NULL;

static uint8 IRQa, K4IRQ;
static uint32 IRQLatch, IRQCount;

static uint8 regs[16];
/*static uint8 test[8];*/
static SFORMAT StateRegs[]=
{
  {&IRQCount, 1, "IRQC"},
  {&IRQLatch, 1, "IRQL"},
  {&IRQa, 1, "IRQA"},
  {&K4IRQ, 1, "K4IRQ"},
  {regs, 16, "REGS"},
  {0}
};

static void chrSync(void)
{
    setchr4r(0x10,0x0000,regs[5]&1);
    setchr4r(0x10,0x1000,0);
}

static void Sync(void)
{
    chrSync();
/*  if(regs[0xA]&0x10)*/
/*  {*/
/*    setchr1r(0x10,0x0000,(((regs[5]&1))<<2)+0);
    setchr1r(0x10,0x0400,(((regs[5]&1))<<2)+1);
    setchr1r(0x10,0x0800,(((regs[5]&1))<<2)+2);
    setchr1r(0x10,0x0c00,(((regs[5]&1))<<2)+3);
    setchr1r(0x10,0x1000,0);
    setchr1r(0x10,0x1400,1);
    setchr1r(0x10,0x1800,2);
    setchr1r(0x10,0x1c00,3);*/
/*    setchr1r(0x10,0x0000,(((regs[5]&1))<<2)+0);
    setchr1r(0x10,0x0400,(((regs[5]&1))<<2)+1);
    setchr1r(0x10,0x0800,(((regs[5]&1))<<2)+2);
    setchr1r(0x10,0x0c00,(((regs[5]&1))<<2)+3);
    setchr1r(0x10,0x1000,(((regs[5]&1)^1)<<2)+4);
    setchr1r(0x10,0x1400,(((regs[5]&1)^1)<<2)+5);
    setchr1r(0x10,0x1800,(((regs[5]&1)^1)<<2)+6);
    setchr1r(0x10,0x1c00,(((regs[5]&1)^1)<<2)+7);
*/
/*  }*/
/*  else*/
/*  {*/
/*
    setchr1r(0x10,0x0000,(((regs[5]&1)^1)<<2)+0);
    setchr1r(0x10,0x0400,(((regs[5]&1)^1)<<2)+1);
    setchr1r(0x10,0x0800,(((regs[5]&1)^1)<<2)+2);
    setchr1r(0x10,0x0c00,(((regs[5]&1)^1)<<2)+3);
    setchr1r(0x10,0x1000,(((regs[5]&1))<<2)+4);
    setchr1r(0x10,0x1400,(((regs[5]&1))<<2)+5);
    setchr1r(0x10,0x1800,(((regs[5]&1))<<2)+6);
    setchr1r(0x10,0x1c00,(((regs[5]&1))<<2)+7);
}*/
/*    setchr1r(1,0x0000,test[0]);
    setchr1r(1,0x0400,test[1]);
    setchr1r(1,0x0800,test[2]);
    setchr1r(1,0x0c00,test[3]);
    setchr1r(1,0x1000,test[4]);
    setchr1r(1,0x1400,test[5]);
    setchr1r(1,0x1800,test[6]);
    setchr1r(1,0x1c00,test[7]);
*/
  setprg4r(0x10,0x6000,regs[0]&1);
  if(regs[2]>=0x40)
    setprg8r(1,0x8000,(regs[2]-0x40));
  else
    setprg8r(0,0x8000,(regs[2]&0x3F));
  if(regs[3]>=0x40)
    setprg8r(1,0xA000,(regs[3]-0x40));
  else
    setprg8r(0,0xA000,(regs[3]&0x3F));
  if(regs[4]>=0x40)
    setprg8r(1,0xC000,(regs[4]-0x40));
  else
    setprg8r(0,0xC000,(regs[4]&0x3F));

  setprg8r(1,0xE000,~0);
  setmirror(MI_V);
}

/*static DECLFW(TestWrite)
{
  test[A&7] = V;
  Sync();
}*/

static DECLFW(M190Write)
{
/* FCEU_printf("write %04x:%04x %d, %d\n",A,V,scanline,timestamp);*/
  regs[(A&0x0F00)>>8]=V;
  switch(A)
  {  
    case 0xd600:IRQLatch&=0xFF00;IRQLatch|=V;break;
    case 0xd700:IRQLatch&=0x00FF;IRQLatch|=V<<8;break;
    case 0xd900:IRQCount=IRQLatch;IRQa=V&2;K4IRQ=V&1;X6502_IRQEnd(FCEU_IQEXT);break;
    case 0xd800:IRQa=K4IRQ;X6502_IRQEnd(FCEU_IQEXT);break;
  }
  Sync();
}

static DECLFR(M190Read)
{
/*  FCEU_printf("read  %04x:%04x %d, %d\n",A,regs[(A&0x0F00)>>8],scanline,timestamp);*/
  return regs[(A&0x0F00)>>8]+regs[0x0B];
}
static void VRC5IRQ(int a)
{
  if(IRQa)
  {
    IRQCount+=a;
    if(IRQCount&0x10000)
    {
      X6502_IRQBegin(FCEU_IQEXT);
      IRQCount=IRQLatch;
    }
  }
}

/*static void Mapper190_PPU(uint32 A)*/
/*{*/
/*  if(A<0x2000)*/
/*     setchr4r(0x10,0x1000,QTAINTRAM[A&0x1FFF]&1);*/
/*  else*/
/*     chrSync();*/
/*}*/

static DECLFW(M1902007Wrap)
{
  if(A>=0x2000)
  {
    if(regs[0xA]&1)
      QTAINTRAM[A&0x1FFF]=V;
    else
      old2007wrap(A,V);
  }
}


static void M190Power(void)
{
/*  test[0]=0;
  test[1]=1;
  test[2]=2;
  test[3]=3;
  test[4]=4;
  test[5]=5;
  test[6]=6;
  test[7]=7;
*/  
  setprg4r(0x10,0x7000,2);

  old2007wrap=GetWriteHandler(0x2007);
  SetWriteHandler(0x2007,0x2007,M1902007Wrap);

  SetReadHandler(0x6000,0xFFFF,CartBR);
/*  SetWriteHandler(0x5000,0x5007,TestWrite);*/
  SetWriteHandler(0x6000,0x7FFF,CartBW);
  SetWriteHandler(0x8000,0xFFFF,M190Write);
  SetReadHandler(0xDC00,0xDC00,M190Read);
  SetReadHandler(0xDD00,0xDD00,M190Read);
  Sync();
}

static void M190Close(void)
{
  if(CHRRAM)
    free(CHRRAM);
  CHRRAM=NULL;
  if(WRAM)
    free(WRAM);
  WRAM=NULL;
}

static void StateRestore(int version)
{
  Sync();
}

void Mapper190_Init(CartInfo *info)
{
  info->Power=M190Power;
  info->Close=M190Close;
  GameStateRestore=StateRestore;

  MapIRQHook=VRC5IRQ;
/*  PPU_hook=Mapper190_PPU;*/

  CHRRAM=(uint8*)FCEU_gmalloc(CHRSIZE);
  SetupCartCHRMapping(0x10,CHRRAM,CHRSIZE,1);
  AddExState(CHRRAM, CHRSIZE, 0, "CHRRAM");

  WRAM=(uint8*)FCEU_gmalloc(WRAMSIZE);
  SetupCartPRGMapping(0x10,WRAM,WRAMSIZE,1);
  AddExState(WRAM, WRAMSIZE, 0, "WRAM");

  if(info->battery)
  {
    info->SaveGame[0] = WRAM;
    info->SaveGameLen[0] = WRAMSIZE - 4096;
  }

  AddExState(&StateRegs, ~0, 0, 0);
}
