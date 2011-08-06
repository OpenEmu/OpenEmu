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
 setprg16(0xC000, latch & 0xF);

 /*
switch(V>>6)
 {
 case 0:break;
 case 1:MIRROR_SET2(0);break;
 case 2:MIRROR_SET2(1);break;
 case 3:break;
 }
 */
}

static DECLFW(Mapper97_write)
{
 latch = V;
switch(V>>6)
 {
 case 0:break;
 case 1:setmirror(MI_H);break;
 case 2:setmirror(MI_V);break;
 case 3:break;
 }

 Sync();
}

static void Power(CartInfo *info)
{
 setprg16(0x8000, 0xF);
 setchr8(0);
 latch = 0xFF;
 Sync();
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 { SFVAR(latch), SFEND };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();
 return(ret);
}

int Mapper97_Init(CartInfo *info)
{
 info->Power = Power;
 info->StateAction = StateAction;
 SetWriteHandler(0x8000,0xffff,Mapper97_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 return(1);
}

