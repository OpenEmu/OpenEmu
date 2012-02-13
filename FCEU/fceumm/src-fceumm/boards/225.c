/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2011 CaH4e3
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
 * PCB-018 board, discrete multigame cart 110-in-1
 *
 */

#include "mapinc.h"

static uint8 prot[4], prg, mode, chr, mirr;

static SFORMAT StateRegs[]=
{
  {prot, 4, "PROT"},
  {&prg, 1, "PRG"},
  {&chr, 1, "CHR"},
  {&mode, 1, "MODE"},
  {&mirr, 1, "MIRR"},
  {0}
};

static void Sync(void)
{
  if(mode)
  {
    setprg16(0x8000,prg);
    setprg16(0xC000,prg);
  }
  else
    setprg32(0x8000,prg>>1);
  setchr8(chr);
  setmirror(mirr);
}

static DECLFW(M225Write)
{
  uint32 bank = (A >> 14) & 1;
  mirr = (A >> 13) & 1;
  mode = (A >> 12) & 1;
  chr = (A & 0x3f) | (bank << 6);
  prg = ((A >> 6) & 0x3f) | (bank << 6);
  Sync();
}

static DECLFW(M225LoWrite)
{
}

static DECLFR(M225LoRead)
{
}

static void M225Power(void)
{ 
  prg = 0;
  mode = 0;
  Sync();
  SetReadHandler(0x5000,0x5fff,M225LoRead);
  SetWriteHandler(0x5000,0x5fff,M225LoWrite);
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xFFFF,M225Write);
}

static void M225Reset(void)
{
  prg = 0;
  mode = 0;
  Sync();
}

static void StateRestore(int version)
{
  Sync();
}

void Mapper225_Init(CartInfo *info)
{
  info->Reset=M225Reset;
  info->Power=M225Power;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
