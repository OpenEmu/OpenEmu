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

static uint8 reg_prg[4];
static uint8 reg_chr[4];
static uint8 dip_switch;

static SFORMAT StateRegs[]=
{
  {reg_prg, 4, "PREGS"},
  {reg_chr, 4, "CREGS"},
  {0}
};

static void Sync(void)
{
  setprg8(0x8000,reg_prg[0]);
  setprg8(0xa000,reg_prg[1]);
  setprg8(0xc000,reg_prg[2]);
  setprg8(0xe000,reg_prg[3]);
  setchr2(0x0000,reg_chr[0]);
  setchr2(0x0800,reg_chr[1]);
  setchr2(0x1000,reg_chr[2]);
  setchr2(0x1800,reg_chr[3]);
  setmirror(MI_V);
}

static DECLFW(MBS5Write)
{
  int bank_sel = (A&0xC00)>>10;
  switch (A&0xF000)
  {
    case 0x8000:
         reg_chr[bank_sel]=A&0x1F;
         break;
    case 0xA000:
         if(A&(1<<(dip_switch+4)))
          reg_prg[bank_sel]=A&0x0F;
         break;
  }
  Sync();
}

static void MBS5Reset(void)
{
  dip_switch++;
  dip_switch&=3;
  reg_prg[0]=reg_prg[1]=reg_prg[2]=reg_prg[3]=~0;     
  Sync();
}

static void MBS5Power(void)
{
  dip_switch=0;
  reg_prg[0]=reg_prg[1]=reg_prg[2]=reg_prg[3]=~0;     
  Sync();
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xFFFF,MBS5Write);
}

static void StateRestore(int version)
{
  Sync();
}

void BMCBS5_Init(CartInfo *info)
{
  info->Power=MBS5Power;
  info->Reset=MBS5Reset;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
