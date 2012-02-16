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
 * 63in1 ghostbusters
 */

#include "mapinc.h"

static uint8 reg[2], bank;
static uint8 banks[4] = {0, 0, 1, 2};
static uint8 *CHRROM=NULL;
static uint32 CHRROMSIZE;

static SFORMAT StateRegs[]=
{
  {reg, 2, "REGS"},
  {&bank, 1, "BANK"},
  {0}
};

static void Sync(void)
{
  if(reg[0]&0x20)
  {
    setprg16r(banks[bank],0x8000,reg[0]&0x1F);
    setprg16r(banks[bank],0xC000,reg[0]&0x1F);
  }
  else
    setprg32r(banks[bank],0x8000,(reg[0]>>1)&0x0F);
  if(reg[1]&2)
    setchr8r(0x10,0);
  else
    setchr8(0);
  setmirror((reg[0]&0x40)>>6);
}

static DECLFW(BMCGhostbusters63in1Write)
{
  reg[A&1]=V;
  bank=((reg[0]&0x80)>>7)|((reg[1]&1)<<1);
/*  FCEU_printf("reg[0]=%02x, reg[1]=%02x, bank=%02x\n",reg[0],reg[1],bank);*/
  Sync();
}

static DECLFR(BMCGhostbusters63in1Read)
{
  if(bank==1)
    return X.DB;
  else
    return CartBR(A);
}

static void BMCGhostbusters63in1Power(void)
{
  reg[0]=reg[1]=0;
  Sync();
  SetReadHandler(0x8000,0xFFFF,BMCGhostbusters63in1Read);
  SetWriteHandler(0x8000,0xFFFF,BMCGhostbusters63in1Write);
}

static void BMCGhostbusters63in1Reset(void)
{
  reg[0]=reg[1]=0;
}

static void StateRestore(int version)
{
  Sync();
}

static void BMCGhostbusters63in1Close(void)
{
  if(CHRROM)
    free(CHRROM);
  CHRROM=NULL;
}

void BMCGhostbusters63in1_Init(CartInfo *info)
{
  info->Reset=BMCGhostbusters63in1Reset;
  info->Power=BMCGhostbusters63in1Power;
  info->Close=BMCGhostbusters63in1Close;

  CHRROMSIZE=8192; /* dummy CHRROM, VRAM disable*/
  CHRROM=(uint8*)FCEU_gmalloc(CHRROMSIZE);
  SetupCartPRGMapping(0x10,CHRROM,CHRROMSIZE,0);
  AddExState(CHRROM, CHRROMSIZE, 0, "CHRROM");

  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
