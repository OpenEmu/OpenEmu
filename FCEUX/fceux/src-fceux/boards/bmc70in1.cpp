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

static uint8 is_large_banks, hw_switch;
static uint8 large_bank;
static uint8 prg_bank;
static uint8 chr_bank;
static uint8 bank_mode;
static uint8 mirroring;
static SFORMAT StateRegs[]=
{
  {&large_bank, 1, "LB"},
  {&hw_switch, 1, "DIPSW"},
  {&prg_bank, 1, "PRG"},
  {&chr_bank, 1, "CHR"},
  {&bank_mode, 1, "BM"},
  {&mirroring, 1, "MIRR"},
  {0}
};

static void Sync(void)
{
  switch (bank_mode)
  {
    case 0x00:
    case 0x10: setprg16(0x8000,large_bank|prg_bank);
               setprg16(0xC000,large_bank|7);
               break;
    case 0x20: setprg32(0x8000,(large_bank|prg_bank)>>1);
               break;
    case 0x30: setprg16(0x8000,large_bank|prg_bank);
               setprg16(0xC000,large_bank|prg_bank);
               break;
  }
  setmirror(mirroring);
  if(!is_large_banks)
    setchr8(chr_bank);
}

static DECLFR(BMC70in1Read)
{
  if(bank_mode==0x10)
//    if(is_large_banks)
      return CartBR((A&0xFFF0)|hw_switch);
//    else
//      return CartBR((A&0xFFF0)|hw_switch);
  else
    return CartBR(A);
}

static DECLFW(BMC70in1Write)
{
  if(A&0x4000)
  {
    bank_mode=A&0x30;
    prg_bank=A&7;
  }
  else
  {
    mirroring=((A&0x20)>>5)^1;
    if(is_large_banks)
      large_bank=(A&3)<<3;
    else
      chr_bank=A&7;
  }
  Sync();
}

static void BMC70in1Reset(void)
{
  bank_mode=0;
  large_bank=0;
  Sync();
  hw_switch++;
  hw_switch&=0xf;
}

static void BMC70in1Power(void)
{
  setchr8(0);
  bank_mode=0;
  large_bank=0;
  Sync();
  SetReadHandler(0x8000,0xFFFF,BMC70in1Read);
  SetWriteHandler(0x8000,0xffff,BMC70in1Write);
}

static void StateRestore(int version)
{
  Sync();
}

void BMC70in1_Init(CartInfo *info)
{
  is_large_banks=0;
  hw_switch=0xd;
  info->Power=BMC70in1Power;
  info->Reset=BMC70in1Reset;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}

void BMC70in1B_Init(CartInfo *info)
{
  is_large_banks=1;
  hw_switch=0x6;
  info->Power=BMC70in1Power;
  info->Reset=BMC70in1Reset;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
