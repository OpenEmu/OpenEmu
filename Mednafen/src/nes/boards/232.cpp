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

static uint8 latches[2];

static void Sync(void)
{
  setprg16(0x8000,(latches[1]&3) | ((latches[0]&0x18)>>1));
  setprg16(0xc000,3|(((latches[0])&0x18)>>1));
}

static DECLFW(Mapper232_write)
{
	if(A<=0x9FFF)
	 latches[0]=V;
	else
	 latches[1]=V;
	Sync();
}

static void Reset(CartInfo *info)
{
 latches[0]=0x18;
 Sync();

 setchr8(0);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(latches, 2),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
 {
  Sync();
 }
 return(ret);
}

int BIC48_Init(CartInfo *info)
{
 SetWriteHandler(0x8000,0xffff,Mapper232_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);

 info->Power = info->Reset = Reset;
 info->StateAction = StateAction;

 return(1);
}

