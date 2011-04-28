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

static uint8 latch;

static void Sync(void)
{
 setchr8((latch >> 2) & 1);
 setprg8(0x8000, latch & 0x4);
}

static DECLFW(Write)
{
 latch = V;
 Sync();
}

static void Power(CartInfo *info)
{
 setprg32(0x8000, 0);

 latch = 0;
 Sync();
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] = 
 {
  SFVAR(latch), 
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();
 return(ret);
}

int Mapper99_Init(CartInfo *info)
{
 info->Power = Power;
 info->StateAction = StateAction;

 SetReadHandler(0x8000, 0xFFFF, CartBR);
 SetWriteHandler(0x4016, 0x4016, Write);	// Warning: relies on kludge in input.cpp

 return(1);
}
