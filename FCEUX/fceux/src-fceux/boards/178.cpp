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

static uint8 reg[3];
static uint8 *WRAM=NULL;
static uint32 WRAMSIZE;

static SFORMAT StateRegs[]=
{
  {reg, 3, "REGS"},
  {0}
};

static void Sync(void)
{
  setmirror(reg[0]);
  setprg8r(0x10,0x6000,0);
  setchr8(0);
  setprg32(0x8000,(reg[1]+reg[2])&0xf);
}

static DECLFW(M178Write0)
{
  reg[0]=(V&1)^1;
  Sync();
}

static DECLFW(M178Write1)
{
  reg[1]=(V>>1)&0xf;
  Sync();
}

static DECLFW(M178Write2)
{
  reg[2]=(V<<2)&0xf;
  Sync();
}

static void M178Power(void)
{
  reg[0]=1; reg[1]=0; reg[2]=0;
  Sync();
  SetReadHandler(0x6000,0x7FFF,CartBR);
  SetWriteHandler(0x6000,0x7FFF,CartBW);
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x4800,0x4800,M178Write0);
  SetWriteHandler(0x4801,0x4801,M178Write1);
  SetWriteHandler(0x4802,0x4802,M178Write2);
}

static void M178Close(void)
{
  if(WRAM)
    free(WRAM);
  WRAM=NULL;
}


static void StateRestore(int version)
{
  Sync();
}

void Mapper178_Init(CartInfo *info)
{
  info->Power=M178Power;
  info->Close=M178Close;
  GameStateRestore=StateRestore;

  WRAMSIZE=8192;
  WRAM=(uint8*)malloc(WRAMSIZE);
  SetupCartPRGMapping(0x10,WRAM,WRAMSIZE,1);
  if(info->battery)
  {
    info->SaveGame[0]=WRAM;
    info->SaveGameLen[0]=WRAMSIZE;
  }
  AddExState(WRAM, WRAMSIZE, 0, "WRAM");

  AddExState(&StateRegs, ~0, 0, 0);
}
