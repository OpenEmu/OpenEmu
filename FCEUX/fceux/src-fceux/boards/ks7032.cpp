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

static uint8 reg[8], cmd, IRQa;
static int32 IRQCount;

static SFORMAT StateRegs[]=
{
  {&cmd, 1, "CMD"},
  {reg, 8, "REGS"},
  {&IRQa, 1, "IRQA"},
  {&IRQCount, 4, "IRQC"},
  {0}
};

static void Sync(void)
{
  setprg8(0x6000,reg[4]);
  setprg8(0x8000,reg[1]);
  setprg8(0xA000,reg[2]);
  setprg8(0xC000,reg[3]);
  setprg8(0xE000,~0);
  setchr8(0);
}

static DECLFW(UNLKS7032Write)
{
//  FCEU_printf("bs %04x %02x\n",A,V);
  switch(A)
  {
//    case 0x8FFF: reg[4]=V; Sync(); break;
    case 0x8000: X6502_IRQEnd(FCEU_IQEXT); IRQCount=(IRQCount&0x000F)|(V&0x0F); break;
    case 0x9000: X6502_IRQEnd(FCEU_IQEXT); IRQCount=(IRQCount&0x00F0)|((V&0x0F)<<4); break;
    case 0xA000: X6502_IRQEnd(FCEU_IQEXT); IRQCount=(IRQCount&0x0F00)|((V&0x0F)<<8); break;
    case 0xB000: X6502_IRQEnd(FCEU_IQEXT); IRQCount=(IRQCount&0xF000)|(V<<12); break;
    case 0xC000: X6502_IRQEnd(FCEU_IQEXT); IRQa=1; break;
    case 0xE000: cmd=V&7; break;
    case 0xF000: reg[cmd]=V; Sync(); break;
  }
}

static void UNLSMB2JIRQHook(int a)
{
  if(IRQa)
  {
    IRQCount+=a;
    if(IRQCount>=0xFFFF)
    {
      IRQa=0;
      IRQCount=0;
      X6502_IRQBegin(FCEU_IQEXT);
    }
  }
}

static void UNLKS7032Power(void)
{
  Sync();
  SetReadHandler(0x6000,0x7FFF,CartBR);
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x4020,0xFFFF,UNLKS7032Write);
}

static void StateRestore(int version)
{
  Sync();
}

void UNLKS7032_Init(CartInfo *info)
{
  info->Power=UNLKS7032Power;
  MapIRQHook=UNLSMB2JIRQHook;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
