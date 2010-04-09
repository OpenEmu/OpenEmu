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

static uint8 A64reg, A64wr;

static void Sync(void)
{
  setprg32(0x8000, (A64wr&1)+((A64reg&0xF)<<1));
  setchr8(((A64wr>>4)&7)+((A64reg&0xF0)>>1));
}

static DECLFW(Mapper46_writel)
{
 A64reg=V;
 Sync();
}

static DECLFW(Mapper46_write)
{
 A64wr=V;
 Sync();
}

static void Reset(CartInfo *info)
{
 A64reg = A64wr = 0;
 Sync();
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(A64reg),
  SFVAR(A64wr),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();
 return(ret);
}

int Mapper46_Init(CartInfo *info)
{
 info->Power = Reset;
 info->StateAction = StateAction;
 SetWriteHandler(0x8000,0xffff,Mapper46_write);
 SetWriteHandler(0x6000,0x7fff,Mapper46_writel);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 return(1);
}
