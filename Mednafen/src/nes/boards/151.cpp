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

static uint8 PRGBanks[3], CHRBanks[2];

static void Sync(void)
{
 for(int x = 0; x < 3; x++)
  setprg8(0x8000 + x * 8192, PRGBanks[x]);
 setchr4(0x0000, CHRBanks[0]);
 setchr4(0x1000, CHRBanks[1]);
}

static DECLFW(Mapper151_write)
{
 switch(A&0xF000)
 {
  case 0x8000:PRGBanks[0] = V; break;
  case 0xA000:PRGBanks[1] = V; break;
  case 0xC000:PRGBanks[2] = V; break;
  case 0xe000:CHRBanks[0] = V; break;
  case 0xf000:CHRBanks[1] = V; break;
 }
 Sync();
}

static void Power(CartInfo *info)
{
 int x;
 for(x = 0; x < 3; x++)
  PRGBanks[x] = ~0;

 for(x = 0; x < 2; x++)
  CHRBanks[x] = x;

 Sync();
 setprg8(0xe000, ~0);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(PRGBanks, 3), SFARRAY(CHRBanks, 2), SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();
 return(ret);
}

int Mapper151_Init(CartInfo *info)
{
 info->Power = Power;
 info->StateAction = StateAction;
 SetWriteHandler(0x8000,0xffff,Mapper151_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);

 return(1);
}

