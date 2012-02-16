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

static uint8 is23;
static uint16 IRQCount;
static uint8 IRQLatch,IRQa;
static uint8 prgreg[2];
static uint8 chrreg[8];
static uint8 regcmd, irqcmd, mirr, big_bank;
static uint16 acount=0;

static uint8 *WRAM=NULL;
static uint32 WRAMSIZE;

static SFORMAT StateRegs[]=
{
  {prgreg, 2, "PRGREGS"},
  {chrreg, 8, "CHRREGS"},
  {&regcmd, 1, "REGCMD"},
  {&irqcmd, 1, "IRQCMD"},
  {&mirr, 1, "MIRR"},
  {&big_bank, 1, "MIRR"},
  {&IRQCount, 2, "IRQC"},
  {&IRQLatch, 1, "IRQL"},
  {&IRQa, 1, "IRQA"},
  {0}
};

static void Sync(void)
{
  if(regcmd&2)
  {
    setprg8(0xC000,prgreg[0]|big_bank);
    setprg8(0x8000,((~1)&0x1F)|big_bank);
  }
  else
  {
    setprg8(0x8000,prgreg[0]|big_bank);
    setprg8(0xC000,((~1)&0x1F)|big_bank);
  }
  setprg8(0xA000,prgreg[1]|big_bank);
  setprg8(0xE000,((~0)&0x1F)|big_bank);
  if(UNIFchrrama)
    setchr8(0);
  else
  {
    uint8 i;
    for(i=0; i<8; i++)
       setchr1(i<<10, chrreg[i]);
  }
  switch(mirr&0x3)
  {
    case 0: setmirror(MI_V); break;
    case 1: setmirror(MI_H); break;
    case 2: setmirror(MI_0); break;
    case 3: setmirror(MI_1); break;
  }
}

static DECLFW(M23Write)
{
/*  FCEU_printf("%04x:%04x\n",A,V);*/
  A|=((A>>2)&0x3)|((A>>4)&0x3)|((A>>6)&0x3); /* actually there is many-in-one mapper source, some pirate or*/
                                             /* licensed games use various address bits for registers*/
  A&=0xF003;
  if((A>=0xB000)&&(A<=0xE003))
  {
    if(UNIFchrrama)
      big_bank=(V&8)<<2;                    /* my personally many-in-one feature ;) just for support pirate cart 2-in-1*/
    else
    {    
      uint16 i=((A>>1)&1)|((A-0xB000)>>11);
      chrreg[i]&=(0xF0)>>((A&1)<<2);
      chrreg[i]|=(V&0xF)<<((A&1)<<2);
    }
    Sync();
  }
  else
    switch(A&0xF003)
    {
      case 0x8000: 
      case 0x8001: 
      case 0x8002: 
      case 0x8003: if(is23) 
                     prgreg[0]=V&0x1F; 
                   Sync(); 
                   break;
      case 0xA000: 
      case 0xA001: 
      case 0xA002: 
      case 0xA003: if(is23) 
                     prgreg[1]=V&0x1F; 
                   else
                   {
                     prgreg[0]=(V<<1)&0x1F;
                     prgreg[1]=((V<<1)&0x1F)|1;
                   }
                   Sync();
                   break;
      case 0x9000:
      case 0x9001: if(V!=0xFF) mirr=V; Sync(); break;
      case 0x9002: 
      case 0x9003: regcmd=V; Sync(); break;
      case 0xF000: X6502_IRQEnd(FCEU_IQEXT); IRQLatch&=0xF0; IRQLatch|=V&0xF; break;
      case 0xF001: X6502_IRQEnd(FCEU_IQEXT); IRQLatch&=0x0F; IRQLatch|=V<<4; break;
      case 0xF002: X6502_IRQEnd(FCEU_IQEXT); acount=0; IRQCount=IRQLatch; IRQa=V&2; irqcmd=V&1; break;
      case 0xF003: X6502_IRQEnd(FCEU_IQEXT); IRQa=irqcmd; break;
    }
}

static void M23Power(void)
{
  big_bank=0x20;  
  Sync();
  setprg8r(0x10,0x6000,0);              /* another many-in-one code, WRAM actually contain only WaiWaiWorld game*/
  SetReadHandler(0x6000,0x7FFF,CartBR);
  SetWriteHandler(0x6000,0x7FFF,CartBW);
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xFFFF,M23Write);
}

void M23IRQHook(int a)
{
  #define LCYCS 341
  if(IRQa)
  {
    acount+=a*3;
    if(acount>=LCYCS)
    {
      while(acount>=LCYCS)
      {
        acount-=LCYCS;
        IRQCount++;
        if(IRQCount&0x100) 
        {
          X6502_IRQBegin(FCEU_IQEXT);
          IRQCount=IRQLatch;
        }
      }
    }
  }
}

static void StateRestore(int version)
{
  Sync();
}

static void M23Close(void)
{
  if(WRAM)
    free(WRAM);
}

void Mapper23_Init(CartInfo *info)
{
  is23=1;
  info->Power=M23Power;
  info->Close=M23Close;
  MapIRQHook=M23IRQHook;
  GameStateRestore=StateRestore;

  WRAMSIZE=8192;
  WRAM=(uint8*)FCEU_gmalloc(WRAMSIZE);
  SetupCartPRGMapping(0x10,WRAM,WRAMSIZE,1);
  AddExState(WRAM, WRAMSIZE, 0, "WRAM");

  AddExState(&StateRegs, ~0, 0, 0);
}

void UNLT230_Init(CartInfo *info)
{
  is23=0;
  info->Power=M23Power;
  info->Close=M23Close;
  MapIRQHook=M23IRQHook;
  GameStateRestore=StateRestore;

  WRAMSIZE=8192;
  WRAM=(uint8*)FCEU_gmalloc(WRAMSIZE);
  SetupCartPRGMapping(0x10,WRAM,WRAMSIZE,1);
  AddExState(WRAM, WRAMSIZE, 0, "WRAM");

  AddExState(&StateRegs, ~0, 0, 0);
}
