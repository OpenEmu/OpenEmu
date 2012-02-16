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
 * NTDEC, ASDER games
 */

#include "mapinc.h"

static uint8 reg[8];
static uint8 mirror, cmd, bank;
static uint8 *WRAM=NULL;

static SFORMAT StateRegs[]=
{
  {&cmd, 1, "CMD"},
  {&mirror, 1, "MIRR"},
  {&bank, 1, "BANK"},
  {reg, 8, "REGS"},
  {0}
};

static void Sync(void)
{
  setmirror(mirror^1);
  setprg8(0x8000,reg[0]);
  setprg8(0xA000,reg[1]);
  setchr2(0x0000,(reg[2]>>1));
  setchr2(0x0800,(reg[3]>>1));
  setchr1(0x1000,((bank&0x10)<<4)|reg[4]);
  setchr1(0x1400,((bank&0x20)<<3)|reg[5]);
  setchr1(0x1800,((bank&0x40)<<2)|reg[6]);
  setchr1(0x1C00,((bank&0x80)<<1)|reg[7]);
}

static DECLFW(M112Write)
{
  switch(A)
  {
    case 0xe000: mirror=V&1; Sync(); ;break;
    case 0x8000: cmd=V&7; break;
    case 0xa000: reg[cmd]=V; Sync(); break;
    case 0xc000: bank=V; Sync(); break;
  }
}

static void M112Close(void)
{
  if(WRAM)
    free(WRAM);
  WRAM = NULL;
}

static void M112Power(void)
{
  bank=0;
  setprg16(0xC000,~0);
  setprg8r(0x10,0x6000,0);
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xFFFF,M112Write);
  SetWriteHandler(0x4020,0x5FFF,M112Write);
  SetReadHandler(0x6000,0x7FFF,CartBR);
  SetWriteHandler(0x6000,0x7FFF,CartBW);
}

static void StateRestore(int version)
{
  Sync();
}

void Mapper112_Init(CartInfo *info)
{
  info->Power=M112Power;
  info->Close=M112Close;
  GameStateRestore=StateRestore;
  WRAM=(uint8*)FCEU_gmalloc(8192);
  SetupCartPRGMapping(0x10,WRAM,8192,1);
  AddExState(WRAM, 8192, 0, "WRAM");
  AddExState(&StateRegs, ~0, 0, 0);
}
