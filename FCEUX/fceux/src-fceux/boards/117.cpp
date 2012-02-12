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

static uint8 prgreg[4], chrreg[8], mirror;
static uint8 IRQa, IRQCount, IRQLatch;

static SFORMAT StateRegs[]=
{
  {&IRQa, 1, "IRQA"},
  {&IRQCount, 1, "IRQC"},
  {&IRQLatch, 1, "IRQL"},
  {prgreg, 4, "PREGS"},
  {chrreg, 8, "CREGS"},
  {&mirror, 1, "MREG"},
  {0}
};

static void Sync(void)
{
  int i;
  setprg8(0x8000,prgreg[0]);     
  setprg8(0xa000,prgreg[1]);     
  setprg8(0xc000,prgreg[2]);     
  setprg8(0xe000,prgreg[3]);
  for(i=0; i<8; i++)
     setchr1(i<<10,chrreg[i]);     
  setmirror(mirror^1);   
}

static DECLFW(M117Write)
{
  if(A<0x8004)
  {
    prgreg[A&3]=V;
    Sync();
  }  
  else if((A>=0xA000)&&(A<=0xA007))
  {
    chrreg[A&7]=V;
    Sync();
  }
  else switch(A)
       {
         case 0xc001: IRQLatch=V; break;
         case 0xc003: IRQCount=IRQLatch; IRQa|=2; break;
         case 0xe000: IRQa&=~1; IRQa|=V&1; X6502_IRQEnd(FCEU_IQEXT); break;
         case 0xc002: X6502_IRQEnd(FCEU_IQEXT); break;
         case 0xd000: mirror=V&1;
       }
}

static void M117Power(void)
{
  prgreg[0]=~3; prgreg[1]=~2; prgreg[2]=~1; prgreg[3]=~0;
  Sync();
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xFFFF,M117Write);
}

static void M117IRQHook(void)
{
  if(IRQa==3&&IRQCount)
  {
    IRQCount--;
    if(!IRQCount)
    {
      IRQa&=1;
      X6502_IRQBegin(FCEU_IQEXT);
    }
  }
}

static void StateRestore(int version)
{
  Sync();
}

void Mapper117_Init(CartInfo *info)
{
  info->Power=M117Power;
  GameHBIRQHook=M117IRQHook;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}

