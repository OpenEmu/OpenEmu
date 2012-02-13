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

static uint8 prg, chr;
static uint8 *WRAM=NULL;
static uint32 WRAMSIZE;

static SFORMAT StateRegs[]=
{
  {&prg, 1, "PRG"},
  {&chr, 1, "CHR"},
  {0}
};

static void Sync(void)
{
  setprg8r(0x10,0x6000,0);
  setprg32(0x8000,prg>>1);
  setchr8(chr);
}

static DECLFW(M176Write1)
{ 
  prg = V;
  Sync();
}

static DECLFW(M176Write2)
{ 
  chr = V;
  Sync();
}

static void M176Power(void)
{
  prg = ~0;
  SetReadHandler(0x6000,0x7fff,CartBR);
  SetWriteHandler(0x6000,0x7fff,CartBW);
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x5ff1,0x5ff1,M176Write1);
  SetWriteHandler(0x5ff2,0x5ff2,M176Write2);
  Sync();
}


static void M176Close(void)
{
  if(WRAM)
    free(WRAM);
  WRAM=NULL;
}

static void StateRestore(int version)
{
  Sync();
}

void Mapper176_Init(CartInfo *info)
{
  info->Power=M176Power;
  info->Close=M176Close;

  GameStateRestore=StateRestore;

  WRAMSIZE=8192;
  WRAM=(uint8*)FCEU_gmalloc(WRAMSIZE);
  SetupCartPRGMapping(0x10,WRAM,WRAMSIZE,1);
  AddExState(WRAM, WRAMSIZE, 0, "WRAM");
  AddExState(&StateRegs, ~0, 0, 0);
}
