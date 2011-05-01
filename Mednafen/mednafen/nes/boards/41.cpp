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

static uint8 calreg, calchr;

static void Sync(void)
{
 setprg32(0x8000, calreg & 0x7);
 setmirror((calreg & 0x20) ? MI_H : MI_V);
 setchr8(calchr);
}

static DECLFW(Mapper41_write)
{
 if(A<0x8000)
 {
  calreg=A;
  calchr&=0x3;
  calchr|=(A>>1)&0xC;
 }
 else if(calreg&0x4)
 {
  calchr&=0xC;
  calchr|=A&3;
 }
 Sync();
}

static void Reset(CartInfo *info)
{
 calreg=calchr=0;
 Sync();
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(calreg),
  SFVAR(calchr),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();
 return(ret);
}


int Mapper41_Init(CartInfo *info)
{
 info->Power = info->Reset = Reset;
 info->StateAction = StateAction;
 SetWriteHandler(0x8000,0xffff,Mapper41_write);
 SetWriteHandler(0x6000,0x67ff,Mapper41_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);

 return(1);
}
