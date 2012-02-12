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
 * Gimmick Bootleg (VRC4 mapper)
 */

#include "mapinc.h"

static uint8 prg[4];
static uint8 chr[8];
static uint8 IRQCount;
static uint8 IRQPre;
static uint8 IRQa;

static SFORMAT StateRegs[]=
{
  {prg, 4, "PRG"},
  {chr, 8, "CHR"},
  {&IRQCount, 1, "IRQCOUNT"},
  {&IRQPre, 1, "IRQPRE"},
  {&IRQa, 1, "IRQA"},
  {0}
};

static void SyncPrg(void)
{
  setprg8(0x6000,0);
  setprg8(0x8000,prg[0]);
  setprg8(0xA000,prg[1]);
  setprg8(0xC000,prg[2]);
  setprg8(0xE000,~0);
}

static void SyncChr(void)
{
  int i;
  for(i=0; i<8; i++)
     setchr1(i<<10,chr[i]);
}

static void StateRestore(int version)
{
  SyncPrg();
  SyncChr();
}

static DECLFW(M183Write)
{
  if(((A&0xF80C)>=0xB000)&&((A&0xF80C)<=0xE00C))
  {
    uint8 index=(((A>>11)-6)|(A>>3))&7;
    chr[index]=(chr[index]&(0xF0>>(A&4)))|((V&0x0F)<<(A&4));
    SyncChr();
  }
  else switch (A&0xF80C)
  {
    case 0x8800: prg[0]=V; SyncPrg(); break;
    case 0xA800: prg[1]=V; SyncPrg(); break;
    case 0xA000: prg[2]=V; SyncPrg(); break;
    case 0x9800: switch (V&3)
                 {
                   case 0: setmirror(MI_V); break;
                   case 1: setmirror(MI_H); break;
                   case 2: setmirror(MI_0); break;
                   case 3: setmirror(MI_1); break;
                 }
                 break;
    case 0xF000: IRQCount=((IRQCount&0xF0)|(V&0xF)); break;
    case 0xF004: IRQCount=((IRQCount&0x0F)|((V&0xF)<<4)); break;
    case 0xF008: IRQa=V; if(!V)IRQPre=0; X6502_IRQEnd(FCEU_IQEXT); break;
    case 0xF00C: IRQPre=16; break;
  }
}

static void M183IRQCounter(void)
{
  if(IRQa)
  {
    IRQCount++;
    if((IRQCount-IRQPre)==238)
      X6502_IRQBegin(FCEU_IQEXT);
  }
}

static void M183Power(void)
{
  IRQPre=IRQCount=IRQa=0;
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xFFFF,M183Write);
  SetReadHandler(0x6000,0x7FFF,CartBR);
  SyncPrg();
  SyncChr();
}

void Mapper183_Init(CartInfo *info)
{
  info->Power=M183Power;
  GameHBIRQHook=M183IRQCounter;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
