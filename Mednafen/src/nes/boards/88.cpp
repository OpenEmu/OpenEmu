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

static int mn;
static uint8 cmd, regs[8];

static void Sync(void)
{
 if(mn)
 {
  setmirror(((cmd >> 6) & 1) ? MI_1 : MI_0);
 }
 setchr2(0x0000, (regs[0] & 0x3E) >> 1);
 setchr2(0x0800, (regs[1] & 0x3E) >> 1);
 setchr1(0x1000, (regs[2] & 0x3F) | 0x40);
 setchr1(0x1400, (regs[3] & 0x3F) | 0x40);
 setchr1(0x1800, (regs[4] & 0x3F) | 0x40);
 setchr1(0x1c00, (regs[5] & 0x3F) | 0x40);
 setprg8(0x8000, regs[6]);
 setprg8(0xa000, regs[7]);
}

static DECLFW(Mapper88_write)
{
 switch(A&0x8001)
 {
  case 0x8000:cmd = V;
	      Sync();
	      break;
  case 0x8001:regs[cmd & 0x7] = V;Sync();break;
 }
}

static void Power(CartInfo *info)
{
 cmd = 0;
 int x;
 for(x = 0; x < 8; x++)
  regs[x] = ~0;

 setprg16(0xc000, ~0);
 Sync();
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(cmd),
  SFARRAY(regs, 8),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();
 return(ret);
}

int Mapper88_Init(CartInfo *info)
{
 mn=0;
 SetWriteHandler(0x8000,0xffff,Mapper88_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 info->Power = Power;
 info->StateAction = StateAction;
 return(1);
}

int Mapper154_Init(CartInfo *info)
{
 mn=1;
 SetWriteHandler(0x8000,0xffff,Mapper88_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 info->Power = Power;
 info->StateAction = StateAction;

 return(1);
}
