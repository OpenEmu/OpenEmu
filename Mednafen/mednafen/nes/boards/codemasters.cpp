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

static uint8 PRGBank16, Mirroring;
static int isbf9097;

static void Sync(void)
{
 setprg16(0x8000, PRGBank16 & 0xF);
 if(isbf9097)
  setmirror(Mirroring?MI_1:MI_0);
}

static DECLFW(WriteHi)
{
 PRGBank16 = V;
 Sync();
}

static DECLFW(WriteLo)
{
 Mirroring = (V >> 4) & 1;
 Sync();
}

static void Power(CartInfo *info)
{
 Mirroring = 0;
 PRGBank16 = 0;
 Sync();
 setchr8(0);
 setprg16(0xc000, 0xF);
 setprg8r(0x10, 0x6000, 0);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 { 
  SFVAR(PRGBank16), 
  SFVAR(Mirroring), // Only for firehawk, though
  SFEND
 };
  
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");

 if(load)
  Sync();

 return(ret);
}

int Mapper71_Init(CartInfo *info)
{
 info->Power = Power;
 info->StateAction = StateAction;
 SetReadHandler(0x6000, 0xFFFF, CartBR);
 SetWriteHandler(0x6000, 0x7FFF, CartBW);
 SetWriteHandler(0xC000, 0xFFFF, WriteHi);

 isbf9097 = 0;

 return(1);
}

int BIC62_Init(CartInfo *info)
{
 Mapper71_Init(info);

 SetWriteHandler(0x8000, 0xBFFF, WriteLo);
 isbf9097 = 1;
 PRGmask16[0] &= 0x7;
 return(1);
}

