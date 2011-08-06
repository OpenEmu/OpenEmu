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

static uint8 latch[2];

static void DoSuper(void)
{
 setprg8r((latch[0]&0xC)>>2,0x6000,((latch[0]&0x3)<<4)|0xF);
 if(latch[0]&0x10)
 {
  setprg16r((latch[0]&0xC)>>2,0x8000,((latch[0]&0x3)<<3)|(latch[1]&7));
  setprg16r((latch[0]&0xC)>>2,0xc000,((latch[0]&0x3)<<3)|7);
 }
 else
  setprg32r(4,0x8000,0);

 setmirror(((latch[0]&0x20)>>5)^1);
}

static DECLFW(SuperWrite)
{
 if(!(latch[0]&0x10))
 {
  latch[0]=V;
  DoSuper();
 }
}

static DECLFW(SuperHi)
{
 latch[1]=V; 
 DoSuper();
}

static void SuperReset(CartInfo *info)
{
 latch[0] = latch[1] = 0;
 setprg32r(4,0x8000,0);
 setchr8(0);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(latch, 2),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  DoSuper();
 return(ret);
}

int Supervision16_Init(CartInfo *info)
{
 info->Power=SuperReset;
 info->StateAction = StateAction;

 SetWriteHandler(0x6000,0x7FFF,SuperWrite);
 SetWriteHandler(0x8000,0xFFFF,SuperHi);
 SetReadHandler(0x6000,0xFFFF,CartBR);

 return(1);
}
