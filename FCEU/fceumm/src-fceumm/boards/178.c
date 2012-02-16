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
 * 
 * San Guo Zhong Lie Zhuan (Ch)
 * win200, etc
 *
 */

#include "mapinc.h"

static uint8 reg[4];
static uint8 *WRAM=NULL;
static uint32 WRAMSIZE;

static SFORMAT StateRegs[]=
{
  {reg, 4, "REGS"},
  {0}
};

static void Sync(void)
{
  uint8 bank = (reg[2]&3)<<3;
  setmirror((reg[0]&1)^1);
  setprg8r(0x10,0x6000,0);
  setchr8(0);
  if(reg[0]&2)
  {
    setprg16(0x8000,(reg[1]&7)|bank);
    setprg16(0xC000,((~0)&7)|bank);
  }
  else
  {
    setprg16(0x8000,(reg[1]&6)|bank);
    setprg16(0xC000,(reg[1]&6)|bank|1);
  }
}

static DECLFW(M178Write)
{
  reg[A&3]=V;
  Sync();
}

static void M178Power(void)
{
  reg[0]=1;
  reg[1]=0;
  reg[2]=0;
  reg[3]=0;
  Sync();
  SetReadHandler(0x6000,0x7FFF,CartBR);
  SetWriteHandler(0x6000,0x7FFF,CartBW);
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x4800,0x4803,M178Write);
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
  WRAM=(uint8*)FCEU_gmalloc(WRAMSIZE);
  SetupCartPRGMapping(0x10,WRAM,WRAMSIZE,1);
  if(info->battery)
  {
    info->SaveGame[0]=WRAM;
    info->SaveGameLen[0]=WRAMSIZE;
  }
  AddExState(WRAM, WRAMSIZE, 0, "WRAM");

  AddExState(&StateRegs, ~0, 0, 0);
}
