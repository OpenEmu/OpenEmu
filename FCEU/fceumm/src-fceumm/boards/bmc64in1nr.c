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
 * BMC 42-in-1 reset switch
 */

#include "mapinc.h"

static uint8 regs[4];

static SFORMAT StateRegs[]=
{
  {regs, 4, "REGS"},
  {0}
};

static void Sync(void)
{
  if(regs[0]&0x80)
  {
    if(regs[1]&0x80)
      setprg32(0x8000,regs[1]&0x1F);
    else
    {
      int bank=((regs[1]&0x1f)<<1)|((regs[1]>>6)&1);
      setprg16(0x8000,bank);
      setprg16(0xC000,bank);
    }
  }
  else
  {
    int bank=((regs[1]&0x1f)<<1)|((regs[1]>>6)&1);
    setprg16(0xC000,bank);
  }
  if(regs[0]&0x20)
    setmirror(MI_H);
  else
    setmirror(MI_V);
  setchr8((regs[2]<<2)|((regs[0]>>1)&3));
}

static DECLFW(BMC64in1nrWriteLo)
{
  regs[A&3]=V;
  Sync();
}

static DECLFW(BMC64in1nrWriteHi)
{
  regs[3]=V;
  Sync();
}

static void BMC64in1nrPower(void)
{
  regs[0]=0x80;
  regs[1]=0x43;
  regs[2]=regs[3]=0;
  Sync();
  SetWriteHandler(0x5000,0x5003,BMC64in1nrWriteLo);
  SetWriteHandler(0x8000,0xFFFF,BMC64in1nrWriteHi);
  SetReadHandler(0x8000,0xFFFF,CartBR);
}

static void StateRestore(int version)
{
  Sync();
}

void BMC64in1nr_Init(CartInfo *info)
{
  info->Power=BMC64in1nrPower;
  AddExState(&StateRegs, ~0, 0, 0);
  GameStateRestore=StateRestore;
}


