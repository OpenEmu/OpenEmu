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

static uint16 IRQCount, IRQLatch, Mirroring;
static uint8 IRQa, PRGBanks[4], CHRBanks[8];
static uint8 WRAM[8192];

static void JalecoIRQHook(int a)
{
 if(IRQa && IRQCount)
 {
  uint16 prev = IRQCount;
  IRQCount-=a;

  if(IRQCount == 0 || IRQCount > prev)
  {
   X6502_IRQBegin(MDFN_IQEXT);
   IRQCount=0;
   IRQa=0;
  }
 }
}

static void DoCHR(void)
{
 for(int x = 0; x < 8; x++)
  setchr1(x * 1024, CHRBanks[x]);
}

static void DoPRG(void)
{
 for(int x = 0; x < 4; x++)
  setprg8(0x8000 + x * 8192, PRGBanks[x]);
}

static void DoMirroring(void)
{
 if(Mirroring & 2)
  setmirror(MI_0);
 else
  setmirror(Mirroring & 1);
}

static DECLFW(Mapper18_write)
{
	A&=0xF003;
	if(A>=0x8000 && A<=0x9001)
	{
         int x=((A>>1)&1)|((A-0x8000)>>11);

         PRGBanks[x]&=(0xF0)>>((A&1)<<2);
         PRGBanks[x]|=(V&0xF)<<((A&1)<<2);
	 DoPRG();
	}
	else if(A>=0xa000 && A<=0xd003)
	{
	 int x=((A>>1)&1)|((A-0xA000)>>11);

         CHRBanks[x]&=(0xF0)>>((A&1)<<2);
         CHRBanks[x]|=(V&0xF)<<((A&1)<<2);
	 DoCHR();
	}
        else switch(A)
        {
  	 case 0xe000:IRQLatch&=0xFFF0;IRQLatch|=(V&0x0f);break;
	 case 0xe001:IRQLatch&=0xFF0F;IRQLatch|=(V&0x0f)<<4;break;
 	 case 0xe002:IRQLatch&=0xF0FF;IRQLatch|=(V&0x0f)<<8;break;
	 case 0xe003:IRQLatch&=0x0FFF;IRQLatch|=(V&0x0f)<<12;break;
	 case 0xf000:IRQCount=IRQLatch;break;
	 case 0xf001:IRQa=V&1;
		     X6502_IRQEnd(MDFN_IQEXT);
		     break;
	 case 0xf002:Mirroring = V & 3;
		     DoMirroring();
         	     break;
        }
}

static void Power(CartInfo *info)
{
 int x;

 Mirroring = IRQCount = IRQLatch = IRQa = 0;
 for(x = 0; x < 8; x++)
  CHRBanks[x] = x;

 PRGBanks[0] = 0;
 PRGBanks[1] = 1;
 PRGBanks[2] = ~1;
 PRGBanks[3] = ~0;

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
  SFARRAY(WRAM, 8192),
  SFVAR(IRQCount), SFVAR(IRQLatch), SFVAR(Mirroring),
  SFVAR(IRQa), SFARRAY(PRGBanks, 4), SFARRAY(CHRBanks, 8),
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

int Mapper18_Init(CartInfo *info)
{
 SetupCartPRGMapping(0x10, WRAM, 8192, 1);
 SetReadHandler(0x6000, 0x7FFF, CartBR);
 SetWriteHandler(0x6000, 0x7FFF, CartBW);


 info->StateAction = StateAction;
 info->Power = Power;
 SetWriteHandler(0x8000,0xffff,Mapper18_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 MapIRQHook=JalecoIRQHook;


 if(info->battery)
 {
  info->SaveGame[0] = WRAM;
  info->SaveGameLen[0] = 8192;
 }


 return(1);
}
