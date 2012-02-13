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
 * Dance 2000 12-in-1
 *
 */

#include "mapinc.h"

static uint8 prg, mirr, prgmode;
static uint8 *WRAM=NULL;
static uint32 WRAMSIZE;

static SFORMAT StateRegs[]=
{
  {&prg, 1, "REGS"},
  {&mirr, 1, "MIRR"},
  {&prgmode, 1, "MIRR"},
  {0}
};

static void Sync(void)
{
  setmirror(mirr);
  setprg8r(0x10,0x6000,0);
  setchr8(0);
  if(prgmode)
    setprg32(0x8000,prg&7);
  else {
    setprg16(0x8000,prg&0x0f);
    setprg16(0xC000,0);
  }
}

static DECLFW(UNLD2000Write)
{
	/*  FCEU_printf("write %04x:%04x\n",A,V);*/
	switch(A) {
		case 0x5000: prg = V; Sync(); break;
		case 0x5200: mirr = (V & 1)^1; prgmode = V & 4; Sync(); break;
			     /*    default: FCEU_printf("write %04x:%04x\n",A,V);*/
	}
}

static DECLFR(UNLD2000Read)
{
  if(prg & 0x40)
    return X.DB;
  else
    return CartBR(A);
}

static void UNLD2000Power(void)
{
  prg = prgmode = 0;
  Sync();
  SetReadHandler(0x6000,0x7FFF,CartBR);
  SetWriteHandler(0x6000,0x7FFF,CartBW);
  SetReadHandler(0x8000,0xFFFF,UNLD2000Read);
  SetWriteHandler(0x4020,0x5FFF,UNLD2000Write);
}

static void UNLAX5705IRQ(void)
{
  if(scanline > 174) setchr4(0x0000,1);
  else setchr4(0x0000,0);
}

static void UNLD2000Close(void)
{
  if(WRAM)
    free(WRAM);
  WRAM=NULL;
}


static void StateRestore(int version)
{
  Sync();
}

void UNLD2000_Init(CartInfo *info)
{
  info->Power=UNLD2000Power;
  info->Close=UNLD2000Close;
  GameHBIRQHook=UNLAX5705IRQ;
  GameStateRestore=StateRestore;

  WRAMSIZE=8192;
  WRAM=(uint8*)FCEU_gmalloc(WRAMSIZE);
  SetupCartPRGMapping(0x10,WRAM,WRAMSIZE,1);
  AddExState(WRAM, WRAMSIZE, 0, "WRAM");

  AddExState(&StateRegs, ~0, 0, 0);
}
