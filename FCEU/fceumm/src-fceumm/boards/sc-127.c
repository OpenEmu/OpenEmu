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
 * Wario Land II (Kirby hack)
 */

#include "mapinc.h"

static uint8 reg[8], chr[8];
static uint8 *WRAM=NULL;
static uint32 WRAMSIZE;
static uint16 IRQCount, IRQa;

static SFORMAT StateRegs[]=
{
  {reg, 8, "REGS"},
  {chr, 8, "CHRS"},
  {&IRQCount, 16, "IRQc"},
  {&IRQa, 16, "IRQa"},
  {0}
};

static void Sync(void)
{
  int i;
  setprg8(0x8000,reg[0]);
  setprg8(0xA000,reg[1]);
  setprg8(0xC000,reg[2]);
  for(i=0; i<8; i++)
    setchr1(i << 10,chr[i]);
  setmirror(reg[3]^1);
}

static DECLFW(UNLSC127Write)
{
  switch(A)
  {
    case 0x8000: reg[0] = V; break; 
    case 0x8001: reg[1] = V; break;
    case 0x8002: reg[2] = V; break;
    case 0x9000: chr[0] = V; break;
    case 0x9001: chr[1] = V; break;
    case 0x9002: chr[2] = V; break;
    case 0x9003: chr[3] = V; break;
    case 0x9004: chr[4] = V; break;
    case 0x9005: chr[5] = V; break;
    case 0x9006: chr[6] = V; break;
    case 0x9007: chr[7] = V; break;
    case 0xC002: IRQa=0; X6502_IRQEnd(FCEU_IQEXT); break;
    case 0xC005: IRQCount=V; break;
    case 0xC003: IRQa=1; break;
    case 0xD001: reg[3] = V; break;
  }
  Sync();
}

static void UNLSC127Power(void)
{
  Sync();
  setprg8r(0x10,0x6000,0);
  setprg8(0xE000,~0);
  SetReadHandler(0x6000,0x7fff,CartBR);
  SetWriteHandler(0x6000,0x7fff,CartBW);
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xFFFF,UNLSC127Write);
}

static void UNLSC127IRQ(void)
{
 if(IRQa)
 {
   IRQCount--;
   if(IRQCount==0)
   {
    X6502_IRQBegin(FCEU_IQEXT);
    IRQa=0;
   }
 }
}

static void UNLSC127Reset(void)
{
}

static void UNLSC127Close(void)
{
  if(WRAM)
    free(WRAM);
  WRAM=NULL;
}

static void StateRestore(int version)
{
  Sync();
}

void UNLSC127_Init(CartInfo *info)
{
  info->Reset=UNLSC127Reset;
  info->Power=UNLSC127Power;
  info->Close=UNLSC127Close;
  GameHBIRQHook=UNLSC127IRQ;
  GameStateRestore=StateRestore;
  WRAMSIZE=8192;
  WRAM=(uint8*)FCEU_gmalloc(WRAMSIZE);
  SetupCartPRGMapping(0x10,WRAM,WRAMSIZE,1);
  AddExState(WRAM, WRAMSIZE, 0, "WRAM");
  AddExState(&StateRegs, ~0, 0, 0);
}
