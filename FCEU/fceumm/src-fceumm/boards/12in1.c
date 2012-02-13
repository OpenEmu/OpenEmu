/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2009 CaH4e3
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
 * 7-in-1 Darkwing Duck, Snake, MagicBlock (PCB marked as "12 in 1")
 */

#include "mapinc.h"

static uint8 reg[4];
static SFORMAT StateRegs[]=
{
  {reg, 4, "REGS"},
  {0}
};

static void Sync(void)
{
    uint8 bank = (reg[3]&3)<<3;
    setchr4(0x0000, (reg[1]>>3)|(bank<<2));
    setchr4(0x1000, (reg[2]>>3)|(bank<<2));
    if(reg[3]&8)
    {
      setprg32(0x8000,((reg[2]&7)>>1)|bank);
    }
    else
    {
      setprg16(0x8000, (reg[1]&7)|bank);
      setprg16(0xc000, 7|bank);
    }
    setmirror(((reg[3]&4)>>2)^1);
}

static DECLFW(BMC12IN1Write)
{
  switch(A)
  {
  case 0xafff: reg[0] = V; break;
  case 0xbfff: reg[1] = V; break;
  case 0xdfff: reg[2] = V; break;
  case 0xefff: reg[3] = V; break;
  }
  Sync();
}

static void BMC12IN1Power(void)
{
  reg[0]=reg[1]=reg[2]=reg[3]=0;
  Sync();
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xFFFF,BMC12IN1Write);
}

static void StateRestore(int version)
{
  Sync();
}

void BMC12IN1_Init(CartInfo *info)
{
  info->Power=BMC12IN1Power;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
