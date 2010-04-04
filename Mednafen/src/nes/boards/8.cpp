/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
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

static uint8 WRAM[8192], latch;

static void Sync(void)
{
	setprg16(0x8000, latch >> 3);
	setchr8(latch & 0x7);
}

static DECLFW(Mapper8_write)
{
	latch = V;
	Sync();
}

static void Reset(CartInfo *info)
{
	if(!info->battery)
	 memset(WRAM, 0x00, 8192);
	latch = 0;
	Sync();
	setprg8r(0x10, 0x6000, 0);
	setprg16(0xc000, 0x1F);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(WRAM, 8192),
  SFVAR(latch),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();
 return(ret);
}

int Mapper8_Init(CartInfo *info)
{
	latch = 0;
	info->Power = info->Reset = Reset;
	info->StateAction = StateAction;
	if(info->battery)	// Do FFE units initialize 6000-7fff RAM?
	{
	 memset(WRAM, 0x00, 8192);
	 info->SaveGame[0] = WRAM;
	 info->SaveGameLen[0] = 8192;
	}
	SetupCartPRGMapping(0x10, WRAM, 8192, 1);
	Sync();
        SetWriteHandler(0x8000,0xFFFF,Mapper8_write);
	SetReadHandler(0x6000, 0xFFFF, CartBR);
	SetWriteHandler(0x6000, 0x7FFF, CartBW);
	return(1);
}

