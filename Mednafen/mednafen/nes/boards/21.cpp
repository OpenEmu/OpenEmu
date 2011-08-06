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

static uint8 CHRBanks[8], PRGBanks[2], K4IRQ, K4sel, IRQCount, IRQLatch, Mirroring, IRQa, WRAM[8192];
static int32 acount;

static void DoPRG(void)
{
 setprg8(0xa000, PRGBanks[1]);

 if(K4sel & 2)
 {
  setprg8(0xc000, PRGBanks[0]);
  setprg8(0x8000, ~1);
 }
 else
 {
  setprg8(0x8000, PRGBanks[0]);
  setprg8(0xC000, ~1);
 }
}

static void DoCHR(void)
{
 int x;
 for(x = 0; x < 8; x++)
  setchr1(x * 1024, CHRBanks[x]);
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

static DECLFW(Mapper21_write)
{
	A|=((A>>5)&0xF);

	if((A&0xF000)==0xA000)
	{
	 PRGBanks[1] = V;
	 DoPRG();
	}
	else if((A&0xF000)==0x8000)
	{
	 PRGBanks[0] = V;
	 DoPRG();
 	}
	else if(A>=0xb000 && A<=0xefff)
	{
	  A&=0xF006;
          {
           int x=((A>>2)&1)|((A-0xB000)>>11);

           CHRBanks[x]&=(0xF0)>>((A&2)<<1);
           CHRBanks[x]|=(V&0xF)<<((A&2)<<1);
	   DoCHR();
          }
	}
	else switch(A&0xF006)
        {
	 case 0x9000: Mirroring = V; DoMirroring();break;
	 case 0x9006:
	 case 0x9004:
	 case 0x9002:K4sel=V;
		     DoPRG();
        	     break;
	 case 0xf000:IRQLatch&=0xF0;IRQLatch|=V&0xF;break;
         case 0xf002:IRQLatch&=0x0F;IRQLatch|=V<<4;break;
	 case 0xf004:IRQCount=IRQLatch;acount=0;
	             IRQa=V&2;K4IRQ=V&1;
		     X6502_IRQEnd(MDFN_IQEXT);
		     break;
	 case 0xf006:IRQa=K4IRQ;X6502_IRQEnd(MDFN_IQEXT);break;
 }
}
static void KonamiIRQHook(int a)
{
  #define LCYCS ((227*2)+1)
  //#define LCYCS 341
  if(IRQa)
  {
//    acount+=a*3;
    acount+=a*4;
    if(acount>=LCYCS)
    {
     doagainbub:acount-=LCYCS;IRQCount++;
     if(!IRQCount) {X6502_IRQBegin(MDFN_IQEXT);IRQCount=IRQLatch;}
     if(acount>=LCYCS) goto doagainbub;
    }
 }
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(CHRBanks, 8), SFARRAY(PRGBanks, 2), SFARRAY(WRAM, 8192),
  SFVAR(K4IRQ), SFVAR(K4sel), SFVAR(IRQCount), SFVAR(IRQLatch), SFVAR(Mirroring), SFVAR(IRQa),
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

static void Power(CartInfo *info)
{
 int x;

 acount = 0;

 for(x = 0; x < 8; x++)
  CHRBanks[x] = x;
 DoCHR();
 K4sel = 0;
 PRGBanks[0] = 0;
 PRGBanks[1] = 1;
 DoPRG();
 Mirroring = 0;
 DoMirroring();

 IRQCount = IRQLatch = 0;
 setprg8(0xe000, ~0);
 setprg8r(0x10, 0x6000, 0);

 if(!info->battery)
  memset(WRAM, 0xFF, 8192);
}

int Mapper21_Init(CartInfo *info)
{
        SetupCartPRGMapping(0x10, WRAM, 8192, 1);
        SetWriteHandler(0x6000, 0x7FFF, CartBW);
        SetWriteHandler(0x8000,0xffff,Mapper21_write);
	SetReadHandler(0x6000, 0xFFFF, CartBR);
	info->Power = Power;
	info->StateAction = StateAction;
        MapIRQHook=KonamiIRQHook;

        if(info->battery)
        {
         info->SaveGame[0] = WRAM;
         info->SaveGameLen[0] = 8192;
        }
	return(1);
}
