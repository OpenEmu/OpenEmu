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

static uint8 Mirroring, PRGBanks[3], CHRBanks[6], WRAM[8192];
static uint32 lastA;
static bool isfu;
static uint8 CCache[8];
static int32 lastmc;

static void Fudou_PPU(uint32 A)
{
 static uint8 z;
  
 if(A>=0x2000) return;

 A>>=10;
 lastA=A;

 z=CCache[A];
 if(z != lastmc)
 {
  setmirror(z?MI_1:MI_0);
  lastmc = z;
 }
}

static void mira()
{
 if(isfu)
 {
  int x;
  CCache[0]=CCache[1]=CHRBanks[0]>>7;
  CCache[2]=CCache[3]=CHRBanks[1]>>7;

  for(x=0;x<4;x++)
   CCache[4+x]=CHRBanks[2+x]>>7;

  setmirror(CCache[lastA] ? MI_1 : MI_0);
 }
 else
  setmirror(Mirroring & 1);
}
static void DoPRG(void)
{
 int x;
 for(x = 0; x < 3; x++)
  setprg8(0x8000 + x * 8192, PRGBanks[x]);
}
static void DoCHR(void)
{
 setchr2(0x0000, (CHRBanks[0] >> 1) & 0x3F);
 setchr2(0x0800, (CHRBanks[1] >> 1) & 0x3F);

 for(int x = 0; x < 4; x++)
  setchr1(0x1000 + x * 1024, CHRBanks[2 + x]);
}
static DECLFW(Mapper80_write)
{
 switch(A)
 {
  case 0x7ef0: CHRBanks[0]=V;DoCHR();mira();break;
  case 0x7ef1: CHRBanks[1]=V;DoCHR();mira();break;

  case 0x7ef2: CHRBanks[2]=V;DoCHR();mira();break;
  case 0x7ef3: CHRBanks[3]=V;DoCHR();mira();break;
  case 0x7ef4: CHRBanks[4]=V;DoCHR();mira();break;
  case 0x7ef5: CHRBanks[5]=V;DoCHR();mira();break;
  case 0x7ef6: Mirroring = V & 1;mira();break;
  case 0x7efa:
  case 0x7efb: PRGBanks[0] = V; DoPRG();break;
  case 0x7efd:
  case 0x7efc: PRGBanks[1] = V; DoPRG();break;
  case 0x7efe:
  case 0x7eff: PRGBanks[2] = V; DoPRG();break;
 }
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(Mirroring),
  SFARRAY(PRGBanks, 3),
  SFVAR(lastA),
  SFVAR(lastmc),
  SFARRAY(CHRBanks, 6),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");

 if(load)
 {
  DoPRG();
  DoCHR();
  mira();
 }
 return(ret);
}

static void Power(CartInfo *info)
{
 int x;

 for(x = 0; x < 6; x++)
  CHRBanks[x] = 0;

 for(x = 0; x < 3; x++)
  PRGBanks[x] = x;

 Mirroring = 0;
 lastA = 0;
 lastmc = -1;

 setprg8(0xe000, ~0);
 DoPRG();
 DoCHR();
 mira();

 memset(WRAM, 0xFF, 8192);
 setprg8r(0x10, 0x6000, 0);
}

int Mapper80_Init(CartInfo *info)
{
 info->Power = Power;
 info->StateAction = StateAction;

 SetWriteHandler(0x7ef0,0x7eff,Mapper80_write);
 SetWriteHandler(0x6000, 0x7eef, CartBW);
 SetWriteHandler(0x7f00, 0x7fff, CartBW);
 SetReadHandler(0x6000, 0xFFFF, CartBR);
 isfu = false;
 SetupCartPRGMapping(0x10, WRAM, 8192, 1); // For Minelvation Saga, at least.  Total amount of RAM on the actual board is unknown, though...

 if(info->battery)
 {
  info->SaveGame[0] = WRAM;
  info->SaveGameLen[0] = 8192;
 }
 return(1);
}

int Mapper207_Init(CartInfo *info)
{
 Mapper80_Init(info);
 isfu = true;
 PPU_hook=Fudou_PPU;

 return(1);
}
