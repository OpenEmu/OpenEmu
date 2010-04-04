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

static bool is48;
static uint8 IRQCount, IRQa, IRQLatch, PRGBanks[2], Mirroring, CHRBanks[6];

static void DoPRG(void)
{
 if(is48)
 {
  setprg8(0x8000, PRGBanks[0]);
  setprg8(0xa000, PRGBanks[1]);
 }
 else
 {
  setprg8(0x8000, PRGBanks[0] & 0x3F);
  setprg8(0xa000, PRGBanks[1] & 0x3F);
 }
}

static void DoCHR(void)
{
 for(int x = 0; x < 4; x++)
  setchr1(0x1000 + x * 1024, CHRBanks[x]);
 setchr2(0x0000, CHRBanks[4]);
 setchr2(0x0800, CHRBanks[5]);
}

static void DoMirroring(void)
{
 if(is48)
  setmirror(((Mirroring >> 6) & 1) ? MI_H : MI_V);
 else
  setmirror(((PRGBanks[0] >> 6) & 1) ? MI_H : MI_V);
}

static DECLFW(Mapper33_write)
{
        A&=0xF003;
        if(A>=0xA000 && A<=0xA003)
	{
	 CHRBanks[A & 3] = V;
	 DoCHR();
	}
        else switch(A)
	{
 	 case 0x8000:PRGBanks[0] = V; DoPRG(); DoMirroring(); break;
         case 0x8001:PRGBanks[1] = V; DoPRG(); break;
         case 0x8002:CHRBanks[4] = V; DoCHR(); break;
         case 0x8003:CHRBanks[5] = V; DoCHR(); break;
        }
}

static DECLFW(Mapper48_HiWrite)
{
	switch(A&0xF003)
	{
         case 0xc000:IRQLatch=V;break;
         case 0xc001:IRQCount=IRQLatch;break;
         case 0xc003:IRQa=0;X6502_IRQEnd(MDFN_IQEXT);break;
         case 0xc002:IRQa=1;break;
         case 0xe000:Mirroring = V; DoMirroring(); ;break; 
	}
}

static void heho(void)
{
 if(IRQa)
 {
   IRQCount++;
   if(!IRQCount) 
   {
    X6502_IRQBegin(MDFN_IQEXT);
    IRQa=0;
   }
 }
}

static void Power(CartInfo *info)
{
 PRGBanks[0] = 0;
 PRGBanks[1] = 1;
 Mirroring = 0;
 IRQLatch = IRQCount = IRQa = 0;
 for(int x = 0; x < 6; x++)
  CHRBanks[x] = 0;
 DoPRG();
 DoCHR();
 DoMirroring();

 if(is48)
  setprg16(0xc000, 0x7F);
 else
  setprg16(0xc000, 0x1F);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(PRGBanks, 2),
  SFARRAY(CHRBanks, 6),
  SFEND
 };

 SFORMAT StateRegs48[] =
 {
  SFARRAY(PRGBanks, 2),
  SFARRAY(CHRBanks, 6),
  SFVAR(Mirroring),
  SFVAR(IRQCount), SFVAR(IRQa), SFVAR(IRQLatch),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, is48 ? StateRegs48 : StateRegs, "MAPR");
 if(load)
 {
  DoPRG();
  DoCHR();
  DoMirroring();
 }
 return(ret);
}

int Mapper33_Init(CartInfo *info)
{
	SetWriteHandler(0x8000,0xffff,Mapper33_write);
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	info->Power = Power;
	info->StateAction = StateAction;
	is48=0;

	return(1);
}

int Mapper48_Init(CartInfo *info)
{
	SetWriteHandler(0x8000,0xffff,Mapper33_write);
	SetWriteHandler(0xc000,0xffff,Mapper48_HiWrite);
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	info->Power = Power;
	info->StateAction = StateAction;
        GameHBIRQHook=heho;
	is48=1;

	return(1);
}
