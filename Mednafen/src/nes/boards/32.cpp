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

static uint8 PRGBanks[2], IREMCon, CHRBanks[8], WRAM[8192];

static void DoCHR(void)
{
 for(int x = 0; x < 8; x++)
  setchr1(x * 1024, CHRBanks[x]);
}
static void DoPRG(void)
{
 if(IREMCon & 2)
 {
  setprg8(0xC000, PRGBanks[0]);
  setprg8(0x8000, 0xFE);
 }
 else
 {
  setprg8(0x8000, PRGBanks[0]);
  setprg8(0xC000, 0xFE);
 }
 setprg8(0xa000, PRGBanks[1]);
}

static void DoMirroring(void)
{
 setmirror((IREMCon & 1) ? MI_H : MI_V);
}

static DECLFW(Mapper32_write)
{
 switch(A>>12)
 {
  case 0x8:
	   PRGBanks[0] = V;
	   DoPRG();
           break;
  case 0x9:IREMCon = V;
	   DoPRG();
	   DoMirroring();
           break;
  case 0xa:PRGBanks[1] = V;DoPRG();
           break;
 }

 if((A&0xF000)==0xb000)
 {
  CHRBanks[A & 7] = V;
  DoCHR();
 }
}

static void Power(CartInfo *info)
{
 PRGBanks[0] = 0;
 PRGBanks[1] = 1;
 for(int x = 0; x < 8; x++)
  CHRBanks[x] = x;
 IREMCon = 0;
 setprg16(0xc000, 0x7F);
 DoPRG();
 DoCHR();
 DoMirroring();
 setprg8r(0x10, 0x6000, 0);
 if(!info->battery)
  memset(WRAM, 0xFF, 8192);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] = 
 {
  SFARRAY(PRGBanks, 2),
  SFARRAY(CHRBanks, 8),
  SFVAR(IREMCon),
  SFARRAY(WRAM, 8192),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");

 if(load)
 {
  DoPRG();
  DoCHR();
  DoMirroring();
 }

 return(ret);
}

int Mapper32_Init(CartInfo *info)
{
 info->Power = Power;
 info->StateAction = StateAction;
 SetupCartPRGMapping(0x10, WRAM, 8192, 1);
 SetWriteHandler(0x8000,0xffff,Mapper32_write);
 SetReadHandler(0x6000, 0xFFFF, CartBR);
 SetWriteHandler(0x6000, 0x7FFF, CartBW);
 if(info->battery)
 {
  memset(WRAM, 0xFF, 8192);
  info->SaveGame[0] = WRAM;
  info->SaveGameLen[0] = 8192;
 }
 return(1);
}
