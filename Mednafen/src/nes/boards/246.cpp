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
static uint8 PRGBanks[4], CHRBanks[4];
static uint8 WRAM[0x800];

static void Sync(void)
{
 int x;
 for(x = 0; x < 4; x++)
 {
  setprg8(0x8000 + x * 8192, PRGBanks[x]);
  setchr2(0x0000 + x * 2048, CHRBanks[x]);
 }
}
static DECLFW(Mapper246_write)
{
 switch(A&0xF007)
 {
  case 0x6000:PRGBanks[0] = V;break;
  case 0x6001:PRGBanks[1] = V;break;
  case 0x6002:PRGBanks[2] = V;break;
  case 0x6003:PRGBanks[3] = V;break;
  case 0x6004:CHRBanks[0] = V;break;
  case 0x6005:CHRBanks[1] = V;break;
  case 0x6006:CHRBanks[2] = V;break;
  case 0x6007:CHRBanks[3] = V;break;
 }
 Sync();
}

static void Reset(CartInfo *info)
{
 int x;
 for(x = 0; x < 4; x++)
  CHRBanks[x] = x;
 PRGBanks[0] = 0;
 PRGBanks[1] = 1;
 PRGBanks[2] = 0xFE;
 PRGBanks[3] = 0xFF;
 setprg2r(0x10, 0x6800, 0);
 Sync();
 if(!info->battery)
  memset(WRAM, 0xFF, 0x800);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(PRGBanks, 4),
  SFARRAY(CHRBanks, 4),
  SFARRAY(WRAM, 0x800),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");

 if(load)
 {
  Sync();
 }
 return(ret);
}

int Mapper246_Init(CartInfo *info)
{
 SetupCartPRGMapping(0x10, WRAM, 0x800, 1);
 SetWriteHandler(0x4020,0x67ff,Mapper246_write);
 SetWriteHandler(0x8000,0xffff,Mapper246_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 SetReadHandler(0x6800, 0x6FFF, CartBR);
 SetWriteHandler(0x6800, 0x6FFF, CartBW);
 info->Reset = info->Power = Reset;
 info->StateAction = StateAction;
 if(info->battery)
 {
  memset(WRAM, 0xFF, 0x800);
  info->SaveGame[0] = WRAM;
  info->SaveGameLen[0] = 2048;
 }
 return(1);
}

