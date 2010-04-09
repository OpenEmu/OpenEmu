/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2003 CaH4e3
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

static uint8 PRGBank32, CHRBank8;

static void Sync(void)
{
 setprg32(0x8000, PRGBank32);
 setchr8(CHRBank8);
}
static DECLFW(Mapper244_write_1)
{
 PRGBank32 = (A - 0x8065) & 0x3;
 Sync();
}

static DECLFW(Mapper244_write_2)
{
 CHRBank8 = (A - 0x80A5) & 0x7;
 Sync();
}

static void Power(CartInfo *info)
{
 CHRBank8 = PRGBank32 = 0;
 Sync();
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(PRGBank32),
  SFVAR(CHRBank8),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();
 return(ret);
}

int Mapper244_Init(CartInfo *info)
{
 SetWriteHandler(0x8065,0x80a4,Mapper244_write_1);
 SetWriteHandler(0x80a5,0x80e4,Mapper244_write_2);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 info->Power = Power;
 info->StateAction = StateAction;
 return(1);
}
