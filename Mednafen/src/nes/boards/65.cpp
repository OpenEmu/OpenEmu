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

static uint8 IRQa, CHRBanks[8], PRGBanks[3], Mirroring;
static uint16 IRQCount, IRQLatch;

static void DoCHR(void)
{
 for(int x = 0; x < 8; x++)
  setchr1(x * 1024, CHRBanks[x]);
}

static void DoPRG(void)
{
 for(int x = 0; x < 3; x++)
  setprg8(0x8000 + x * 8192, PRGBanks[x]);
}

static void DoMirroring(void)
{
 setmirror((Mirroring & 1) ^ 1);
}

static void IREMIRQHook(int a)
{
  if(IRQa)
  {
   uint16 last = IRQCount;
   IRQCount-=a;
   if(IRQCount > last)
   {
    X6502_IRQBegin(MDFN_IQEXT);
    IRQa=0;
    IRQCount=0xFFFF;
   }
  }
}

static DECLFW(Mapper65_write)
{
 A&= 0xF007;

 if(A >= 0xb000 && A <= 0xB007)
 {
  CHRBanks[A & 7] = V;
  DoCHR();
 }
 else switch(A)
 {
  case 0x8000:PRGBanks[0] = V; DoPRG(); break;
  case 0x9001:Mirroring = V >> 7; DoMirroring();break;
  case 0x9003:IRQa=V&0x80;X6502_IRQEnd(MDFN_IQEXT);break;
  case 0x9004:IRQCount=IRQLatch;break;
  case 0x9005:          IRQLatch&=0x00FF;
                        IRQLatch|=V<<8;
                        break;
  case 0x9006:          IRQLatch&=0xFF00;IRQLatch|=V;
                        break;
  case 0xa000:PRGBanks[1] = V; DoPRG();break;
  case 0xC000:PRGBanks[2] = V; DoPRG();break;
 }
}

static void Power(CartInfo *info)
{
 int x;

 for(x = 0; x < 8; x++)
  CHRBanks[x] = x;

 Mirroring = ((info->mirror & 1) ^ 1);
 PRGBanks[0] = 0;
 PRGBanks[1] = 1;
 PRGBanks[2] = 0xFE;
 DoPRG();
 DoCHR();
 DoMirroring();
 setprg8(0xe000, 0xFF);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(CHRBanks, 8), SFARRAY(PRGBanks, 3),
  SFVAR(Mirroring),
  SFVAR(IRQCount), SFVAR(IRQLatch), SFVAR(IRQa),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
 {
  DoCHR();
  DoPRG();
  DoMirroring();
 }

 return(ret);
}

int Mapper65_Init(CartInfo *info)
{
 info->Power = Power;
 info->StateAction = StateAction;
 MapIRQHook=IREMIRQHook;
 SetWriteHandler(0x8000,0xffff,Mapper65_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);

 return(1);
}
