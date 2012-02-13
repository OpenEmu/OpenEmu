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
 */

#include "mapinc.h"

static uint8 reg[8];
static uint8 mirror, cmd, is154;

static SFORMAT StateRegs[]=
{
  {&cmd, 1, "CMD"},
  {&mirror, 1, "MIRR"},
  {reg, 8, "REGS"},
  {0}
};

static void Sync(void)
{
  setchr2(0x0000,reg[0]>>1);
  setchr2(0x0800,reg[1]>>1);
  setchr1(0x1000,reg[2]|0x40);
  setchr1(0x1400,reg[3]|0x40);
  setchr1(0x1800,reg[4]|0x40);
  setchr1(0x1C00,reg[5]|0x40);
  setprg8(0x8000,reg[6]);
  setprg8(0xA000,reg[7]);
}

static void MSync(void)
{
  if(is154)setmirror(MI_0+(mirror&1));
}

static DECLFW(M88Write)
{
  switch(A&0x8001)
  {
    case 0x8000: cmd=V&7; mirror=V>>6; MSync(); break;
    case 0x8001: reg[cmd]=V; Sync(); break;
  }
}

static void M88Power(void)
{
  setprg16(0xC000,~0);
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xFFFF,M88Write);
}

static void StateRestore(int version)
{
  Sync();
  MSync();
}

void Mapper88_Init(CartInfo *info)
{
  is154=0;  
  info->Power=M88Power;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}

void Mapper154_Init(CartInfo *info)
{
  is154=1;
  info->Power=M88Power;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
