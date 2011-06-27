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

/* Original code provided by LULU */
static uint8 PRGBank16, CHRBank8;

static void Sync(void)
{
 setprg16(0xc000, PRGBank16);
 setchr8(CHRBank8); 
}

static DECLFW(Mapper92_write)
{
 uint8 reg=(A&0xF0)>>4;
 uint8 bank=A&0xF;

 if(A>=0x9000)
 {
  if(reg==0xD) PRGBank16 = bank;
  else if(reg==0xE) CHRBank8 = bank;
 }
 else
 {
  if(reg==0xB) PRGBank16 = bank;
  else if(reg==0x7) CHRBank8 = bank;
 }
 Sync();
}

static void Power(CartInfo *info)
{
 PRGBank16 = 0xFF;
 CHRBank8 = 0;
 Sync();
 setprg16(0x8000, 0);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(PRGBank16), SFVAR(CHRBank8),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();

 return(ret);
}

int Mapper92_Init(CartInfo *info)
{
 info->Power = Power;
 info->StateAction = StateAction;
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 SetWriteHandler(0x8000,0xFFFF,Mapper92_write);
 return(1);
}
