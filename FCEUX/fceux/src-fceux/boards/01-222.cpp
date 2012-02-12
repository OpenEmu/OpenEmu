/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2006 CaH4e3
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
 * TXC mappers
 */

#include "mapinc.h"

static uint8 reg[4], cmd, is172, is173;
static SFORMAT StateRegs[]=
{
  {reg, 4, "REGS"},
  {&cmd, 1, "CMD"},
  {0}
};

static void Sync(void)
{
  setprg32(0x8000,(reg[2]>>2)&1);
  if(is172)
    setchr8((((cmd^reg[2])>>3)&2)|(((cmd^reg[2])>>5)&1)); // 1991 DU MA Racing probably CHR bank sequence is WRONG, so it is possible to
                                                          // rearrange CHR banks for normal UNIF board and mapper 172 is unneccessary
  else
    setchr8(reg[2]&3);
}

static DECLFW(UNL22211WriteLo)
{
//  FCEU_printf("bs %04x %02x\n",A,V);
  reg[A&3]=V;
}

static DECLFW(UNL22211WriteHi)
{
//  FCEU_printf("bs %04x %02x\n",A,V);
  cmd=V;
  Sync();
}

static DECLFR(UNL22211ReadLo)
{
    return (reg[1]^reg[2])|(is173?0x01:0x40);
//  if(reg[3])
//    return reg[2];
//  else
//    return X.DB;
}

static void UNL22211Power(void)
{
  Sync();
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetReadHandler(0x4100,0x4100,UNL22211ReadLo);
  SetWriteHandler(0x4100,0x4103,UNL22211WriteLo);
  SetWriteHandler(0x8000,0xFFFF,UNL22211WriteHi);
}

static void StateRestore(int version)
{
  Sync();
}

void UNL22211_Init(CartInfo *info)
{
  is172=0;
  is173=0;
  info->Power=UNL22211Power;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}

void Mapper172_Init(CartInfo *info)
{
  is172=1;  
  is173=0;
  info->Power=UNL22211Power;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}

void Mapper173_Init(CartInfo *info)
{
  is172=0;  
  is173=1;
  info->Power=UNL22211Power;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
