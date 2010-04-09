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

static uint8 CHRBanks[4], PRGBank16, Mirroring, suntoggle, IRQa;
static uint16 IRQCount;

static void DoCHR(void)
{
 int x;
 for(x = 0; x < 4; x++)
  setchr2(x * 2048, CHRBanks[x]);
}

static void DoMirroring(void)
{
 switch(Mirroring & 3)
 {
  case 0: setmirror(MI_V); break;
  case 1: setmirror(MI_H); break;
  case 2: setmirror(MI_0); break;
  case 3: setmirror(MI_1); break;
 }
}

static void DoPRG(void)
{
 setprg16(0x8000, PRGBank16);
}

static DECLFW(Mapper67_write)
{
 A&=0xF800;
 if((A&0x800) && A<=0xb800)
 {
  CHRBanks[(A - 0x8800) >> 12] = V;
  DoCHR();
 }
 else switch(A)
 {
  case 0xc800:
  case 0xc000:if(!suntoggle)
              {
	       IRQCount&=0xFF;
               IRQCount|=V<<8;
              }
              else
	      {
               IRQCount&=0xFF00;
	       IRQCount|=V;
	      }
              suntoggle^=1;
              break;
  case 0xd800:suntoggle=0;IRQa=V&0x10;X6502_IRQEnd(MDFN_IQEXT);break;

  case 0xe800:Mirroring = V & 3; DoMirroring(); break;
  case 0xf800:PRGBank16 = V; DoPRG(); break;
 }
}

static void SunIRQHook(int a)
{
  if(IRQa)
  {
   uint16 last = IRQCount;
   IRQCount-=a;
   if(last < IRQCount)
   {
    X6502_IRQBegin(MDFN_IQEXT);
    IRQa=0;
   }
  }
}

static void Power(CartInfo *info)
{
 Mirroring = 0;
 IRQCount = IRQa = 0;
 for(int x = 0; x < 4; x++)
  CHRBanks[x] = x;
 PRGBank16 = 0;
 DoMirroring();
 DoPRG();
 DoCHR();

 setprg16(0xc000, ~0);
}


static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(CHRBanks, 4),
  SFVAR(PRGBank16), SFVAR(Mirroring), SFVAR(suntoggle), SFVAR(IRQa), SFVAR(IRQCount),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
 {
  Mirroring &= 3;
  DoPRG(); 
  DoCHR();
  DoMirroring();
 }
 return(ret);
}

int Mapper67_Init(CartInfo *info)
{
 SetWriteHandler(0x8000, 0xffff, Mapper67_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 MapIRQHook=SunIRQHook;
 info->Power = Power;
 info->StateAction = StateAction;

 return(1);
}
