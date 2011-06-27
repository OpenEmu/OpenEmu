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

static uint16 latch;
static uint8 v_latch;

static void Sync(void)
{
 uint32 page,pagel,pageh;

 setmirror(((latch>>13)&1) ? MI_H : MI_V);

 page=(latch>>7)&0x3F;
 //printf("%04x\n",A);
 if((page&0x30)==0x30)
  page-=0x10;

 pagel=pageh=(page<<1) + (((latch>>6)&1)&((latch>>5)&1));
 pageh+=((latch>>5)&1)^1;

 setprg16(0x8000,pagel);
 setprg16(0xC000,pageh);
 setchr8( (v_latch&0x3) | ((latch&0xF)<<2) );
}

static DECLFW(Mapper228_write)
{
 latch = A;
 v_latch = V;
 Sync();
}

static void Reset(CartInfo *info)
{
 latch = v_latch = 0;
 Sync();
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] = 
 {
  SFVAR(latch), SFVAR(v_latch),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();
 return(ret);
}

int Mapper228_Init(CartInfo *info)
{
 info->Power = info->Reset = Reset;
 info->StateAction = StateAction;
 SetWriteHandler(0x8000, 0xffff, Mapper228_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 return(1);
}

