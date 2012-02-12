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

static uint8 reg[2];

static uint8 *WRAM=NULL;
static uint32 WRAMSIZE;

static SFORMAT StateRegs[]=
{
  {reg, 2, "REG"},
  {0}
};

static void Sync(void)
{
  setchr8(0);
  setprg8r(0x10,0x6000,0);
  setprg32(0x8000,reg[1]>>1);
  setmirror((reg[0]&1)^1);
}

static DECLFW(M179Write)
{
  if(A==0xa000) reg[0]=V;
  Sync();
}

static DECLFW(M179WriteLo)
{
  if(A==0x5ff1) reg[1]=V;
  Sync();
}

static void M179Power(void)
{
  reg[0]=reg[1]=0;
  Sync();     
  SetWriteHandler(0x4020,0x5fff,M179WriteLo);
  SetReadHandler(0x6000,0x7fff,CartBR);
  SetWriteHandler(0x6000,0x7fff,CartBW);
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xFFFF,M179Write);
}

static void M179Close(void)
{
  if(WRAM)
    free(WRAM);
  WRAM=NULL;
}

static void StateRestore(int version)
{
  Sync();
}

void Mapper179_Init(CartInfo *info)
{
  info->Power=M179Power;
  info->Close=M179Close;
  GameStateRestore=StateRestore;

  WRAMSIZE=8192;
  WRAM=(uint8*)malloc(WRAMSIZE);
  SetupCartPRGMapping(0x10,WRAM,WRAMSIZE,1);
  AddExState(WRAM, WRAMSIZE, 0, "WRAM");
  if(info->battery)
  {
    info->SaveGame[0]=WRAM;
    info->SaveGameLen[0]=WRAMSIZE;
  }

  AddExState(&StateRegs, ~0, 0, 0);
}
