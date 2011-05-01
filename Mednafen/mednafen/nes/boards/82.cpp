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

static uint8 ctrl, CHRBanks[6], PRGBanks[3], WRAM[8192];

static void DoPRG(void)
{
 int x;
 for(x = 0; x < 3; x++)
  setprg8(0x8000 + 8192 * x, PRGBanks[x]);
}

static void DoCHR(void)
{
 int x;

 setmirror(ctrl & 1);

 for(x=0;x<2;x++)
  setchr2((x<<11)|((ctrl&2)<<11),CHRBanks[x]>>1);
 for(x=0;x<4;x++)
  setchr1((x<<10) | (((ctrl&2)^2)<<11),CHRBanks[2+x]);
}

static DECLFW(Mapper82_write)
{
 if(A>=0x7EF0 && A<=0x7EF5)
 {
  CHRBanks[A - 0x7EF0]=V;
  DoCHR();
 }
 else
  switch(A)
  {
   case 0x7ef6:ctrl=V&3;
 	       DoCHR();
  	       break;
   case 0x7efa:V>>=2;PRGBanks[0]=V;DoPRG();break;
   case 0x7efb:V>>=2;PRGBanks[1]=V;DoPRG();break;
   case 0x7efc:V>>=2;PRGBanks[2]=V;DoPRG();break;
  }
}

static void Power(CartInfo *info)
{
 int x;

 for(x = 0; x < 3; x++)
  PRGBanks[x] = x;
 ctrl = 0;
 for(x = 0; x < 6; x++)
  CHRBanks[x] = 0;

 setprg8(0xe000, 0xFF >> 2);
 DoPRG();
 DoCHR();
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(ctrl),
  SFARRAY(CHRBanks, 6),
  SFARRAY(PRGBanks, 3),
  SFARRAY(WRAM, 8192),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");

 if(load)
 {
  DoPRG();
  DoCHR();
 }

 return(ret);
}

int Mapper82_Init(CartInfo *info)
{
 info->Power = Power;
 info->StateAction = StateAction;
 SetReadHandler(0x6000, 0xFFFF, CartBR);
 SetWriteHandler(0x6000, 0x7FFF, CartBW);
 /* external WRAM might end at $73FF */
 SetWriteHandler(0x7ef0,0x7efc,Mapper82_write);

 if(info->battery)
 {
  info->SaveGame[0] = WRAM;
  info->SaveGameLen[0] = 8192;
 }
 return(1);
}

