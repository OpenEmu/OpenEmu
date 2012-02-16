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
 *
 * FDS Conversion
 *
 */

#include "mapinc.h"

static uint8 reg;
static uint8 *WRAM=NULL;
static uint32 WRAMSIZE;

static SFORMAT StateRegs[]=
{
  {&reg, 1, "REGS"},
  {0}
};

static void Sync(void)
{
  setprg8r(0x10,0x6000,0);
  setprg32(0x8000,reg&1);
  setchr8(0);
}

static DECLFW(UNLKS7012Write)
{
	/*  FCEU_printf("bs %04x %02x\n",A,V);*/
	switch(A)
	{
		case 0xE0A0: reg=0; Sync(); break;
		case 0xEE36: reg=1; Sync(); break;
	}
}

static void UNLKS7012Power(void)
{
  reg = ~0;
  Sync();
  SetReadHandler(0x6000,0x7FFF,CartBR);
  SetWriteHandler(0x6000,0x7FFF,CartBW);
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xFFFF,UNLKS7012Write);
}

static void UNLKS7012Reset(void)
{
  reg = ~0;
  Sync();
}

static void StateRestore(int version)
{
  Sync();
}

static void UNLKS7012Close(void)
{
  if(WRAM)
    free(WRAM);
  WRAM=NULL;
}

void UNLKS7012_Init(CartInfo *info)
{
  info->Power=UNLKS7012Power;
  info->Reset=UNLKS7012Reset;
  info->Close=UNLKS7012Close;

  WRAMSIZE=8192;
  WRAM=(uint8*)FCEU_gmalloc(WRAMSIZE);
  SetupCartPRGMapping(0x10,WRAM,WRAMSIZE,1);
  AddExState(WRAM, WRAMSIZE, 0, "WRAM");

  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
