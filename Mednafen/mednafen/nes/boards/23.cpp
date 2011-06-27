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

static uint8 CHRBanks[8], PRGBanks[2], IRQCount, IRQLatch, Mirroring, K4IRQ, IRQa, K4sel;
static int32 acount;
static uint8 *WRAM = NULL;

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

static void DoCHR(void)
{
 int x;
 for(x = 0; x < 8; x++)
  setchr1(x * 1024, CHRBanks[x]);
}

static void DoPRG(void)
{
 if(K4sel & 2)
 {
  setprg8(0x8000, 0xFE);
  setprg8(0xc000, PRGBanks[0]);
 }
 else
 {
  setprg8(0xc000, 0xFE);
  setprg8(0x8000, PRGBanks[0]);
 }
 setprg8(0xa000, PRGBanks[1]);
}

static DECLFW(Mapper23_write)
{
  if((A&0xF000)==0x8000)
  {
   PRGBanks[0] = V;
   DoPRG();
  } 
  else if((A&0xF000)==0xA000) 
  {
   PRGBanks[1] = V;
   DoPRG();
  }
  else
  {
   A|=((A>>2)&0x3)|((A>>4)&0x3)|((A>>6)&0x3);
   A&=0xF003;
   if(A>=0xb000 && A<=0xe003)
   {
    int x=((A>>1)&1)|((A-0xB000)>>11);

    CHRBanks[x]&=(0xF0)>>((A&1)<<2);
    CHRBanks[x]|=(V&0xF)<<((A&1)<<2);
    DoCHR();
   }
   else 
    switch(A)
    {
     case 0xf000:X6502_IRQEnd(MDFN_IQEXT);IRQLatch&=0xF0;IRQLatch|=V&0xF;break;
     case 0xf001:X6502_IRQEnd(MDFN_IQEXT);IRQLatch&=0x0F;IRQLatch|=V<<4;break;
     case 0xf002:X6502_IRQEnd(MDFN_IQEXT);acount=0;IRQCount=IRQLatch;IRQa=V&2;K4IRQ=V&1;break;
     case 0xf003:X6502_IRQEnd(MDFN_IQEXT);IRQa=K4IRQ;break;
     case 0x9001:
     case 0x9002:
     case 0x9003:K4sel=V;
		 DoPRG();
                 break;
     case 0x9000: Mirroring = V & 0x3; DoMirroring(); break;
  }
 }
}

static void KonamiIRQHook2(int a)
{
  #define LCYCS 341
  if(IRQa)
  {
   acount+=a*3;
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
  SFVAR(acount),
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
	for(int x = 0; x < 8; x++)
	 CHRBanks[x] = ~0;
	PRGBanks[0] = PRGBanks[1] = ~0;
	setprg8r(0x10, 0x6000, 0);
	setprg16(0xc000, 0x7F);
	IRQCount = IRQLatch = IRQa = K4sel = K4IRQ = 0;
	Mirroring = 3;
	DoPRG();
	DoCHR();
	DoMirroring();

	if(!info->battery)
	 memset(WRAM, 0xFF, 8192);

	acount = 0;
}

static void Close(void)
{
 if(WRAM)
 {
  free(WRAM);
  WRAM = NULL;
 }
}

int Mapper23_Init(CartInfo *info)
{
	info->StateAction = StateAction;
        SetWriteHandler(0x8000,0xffff,Mapper23_write);

	SetReadHandler(0x6000, 0xFFFF, CartBR);
	SetWriteHandler(0x6000, 0x7FFF, CartBW);

        MapIRQHook=KonamiIRQHook2;
	info->Power = Power;
	info->Close = Close;

	if(!(WRAM = (uint8 *)malloc(8192)))
	 return(0);

	SetupCartPRGMapping(0x10, WRAM, 8192, 1);
	if(info->battery)
	{
	 info->SaveGame[0] = WRAM;
	 info->SaveGameLen[0] = 8192;
	}
	return(1);
}

