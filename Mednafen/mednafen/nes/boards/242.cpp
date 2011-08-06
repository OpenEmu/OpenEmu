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

static uint8 Mirroring, PRGBank32, WRAM[8192];

static void Sync(void)
{
 const int mirtab[4] = { MI_V, MI_H, MI_0, MI_1 };

 setprg32(0x8000, PRGBank32);
 setmirror(mirtab[Mirroring & 3]);
}

static DECLFW(Mapper242_write)
{
 PRGBank32 = (A >> 3) & 0xF;
 Mirroring = V & 3;
 Sync();
}

static void Power(CartInfo *info)
{
 Mirroring = PRGBank32 = 0;
 Sync();
 setchr8(0);
 setprg8r(0x10, 0x6000, 0);
 if(!info->battery)
  memset(WRAM, 0xFF, 8192);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(Mirroring),
  SFVAR(PRGBank32),
  SFARRAY(WRAM, 8192),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();
 return(ret);
}

int Mapper242_Init(CartInfo *info)
{
 SetWriteHandler(0x8000,0xffff,Mapper242_write);
 SetReadHandler(0x6000, 0xFFFF, CartBR);
 SetWriteHandler(0x6000, 0x7FFF, CartBW);
 info->Power = Power;
 info->StateAction = StateAction;
 if(info->battery)
 {
  memset(WRAM, 0xFF, 8192);
  info->SaveGame[0] = WRAM;
  info->SaveGameLen[0] = 8192;
 }
 SetupCartPRGMapping(0x10, WRAM, 8192, 1);
 return(1);
}

