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

static uint8 cmd;
static uint8 DRegs[8];

static SFORMAT DEI_StateRegs[]=
{
  {&cmd, 1, "CMD"},
  {DRegs, 8, "DREG"},
  {0}
};

static void Sync(void)
{
  int x;
  setchr2(0x0000,DRegs[0]);
  setchr2(0x0800,DRegs[1]);
  for(x=0;x<4;x++)
     setchr1(0x1000+(x<<10),DRegs[2+x]);
  setprg8(0x8000,DRegs[6]);
  setprg8(0xa000,DRegs[7]);
}

static void StateRestore(int version)
{
  Sync();
}

static DECLFW(DEIWrite)
{
  switch(A&0x8001)
  {
    case 0x8000: cmd=V&0x07; break;
    case 0x8001: if(cmd<=0x05)
                   V&=0x3F;
                 else
                   V&=0x0F;
                 if(cmd<=0x01) V>>=1;
                 DRegs[cmd&0x07]=V;
                 Sync();
                 break;
  }
}

static void DEIPower(void)
{
  setprg8(0xc000,0xE);
  setprg8(0xe000,0xF);
  cmd=0;
  memset(DRegs,0,8);
  Sync();
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xFFFF,DEIWrite);
}


void DEIROM_Init(CartInfo *info)
{
  info->Power=DEIPower;
  GameStateRestore=StateRestore;
  AddExState(&DEI_StateRegs, ~0, 0, 0);
}
