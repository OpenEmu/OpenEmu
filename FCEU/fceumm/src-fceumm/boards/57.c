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
 */

#include "mapinc.h"

static uint8 prg_reg;
static uint8 chr_reg;
static uint8 hrd_flag;

static SFORMAT StateRegs[]=
{
  {&hrd_flag, 1, "DIPSW"},
  {&prg_reg, 1, "PRG"},
  {&chr_reg, 1, "CHR"},
  {0}
};

static void Sync(void)
{
  if(prg_reg&0x80)
    setprg32(0x8000,prg_reg>>6);
  else
  {
    setprg16(0x8000,(prg_reg>>5)&3);
    setprg16(0xC000,(prg_reg>>5)&3);
  }
  setmirror((prg_reg&8)>>3);
  setchr8((chr_reg&3)|(prg_reg&7)|((prg_reg&0x10)>>1));
}

static DECLFR(M57Read)
{
  return hrd_flag;
}

static DECLFW(M57Write)
{
  if((A&0x8800)==0x8800)
    prg_reg=V;
  else
    chr_reg=V;
  Sync();
}

static void M57Power(void)
{
  prg_reg=0;
  chr_reg=0;
  hrd_flag=0;
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xFFFF,M57Write);
  SetReadHandler(0x6000,0x6000,M57Read);
  Sync();
}

static void M57Reset()
{
  hrd_flag++;
  hrd_flag&=3;
  FCEU_printf("Select Register = %02x\n",hrd_flag);
}

static void StateRestore(int version)
{
  Sync();
}

void Mapper57_Init(CartInfo *info)
{
  info->Power=M57Power;
  info->Reset=M57Reset;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
