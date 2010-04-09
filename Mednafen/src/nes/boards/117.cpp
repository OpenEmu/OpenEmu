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

static uint8 IRQLatch, IRQCount, CHRBanks[8], PRGBanks[4], IRQa;

static void DoPRG(void)
{
 int x;
 for(x = 0; x < 4; x++)
  setprg8(0x8000 + 8192 * x, PRGBanks[x]);
}

static void DoCHR(void)
{
 int x;
 for(x = 0; x < 8; x++)
  setchr1(0x0000 + 1024 * x, CHRBanks[x]);
}

static DECLFW(Mapper117_write)
{
 A &= 0xF00F;

 if(A >= 0xa000 && A <= 0xa007)
 {
  CHRBanks[A & 7] = V;
  DoCHR();
 }
 else
 {
  //A &= 0xF003;
  if(A >= 0x8000 && A <= 0x8003)
  {
   PRGBanks[A & 3] = V;
   DoPRG();
  }
  else
   switch(A)
   {
    case 0xc001:IRQLatch=V;break;
    case 0xc003:IRQCount=IRQLatch;IRQa|=2;break;
    case 0xe000:IRQa&=~1;IRQa|=V&1;X6502_IRQEnd(MDFN_IQEXT);break;
    case 0xc002:X6502_IRQEnd(MDFN_IQEXT);break;
    //default: printf("Oops: %04x $%02x\n", A, V); break;
   }
 }
}

static void Mapper117_hb(void)
{
 if(IRQa==3 && IRQCount)
 {
  IRQCount--;
  if(!IRQCount)
  {
   IRQa&=1;
   X6502_IRQBegin(MDFN_IQEXT);
  }
 }
}

static void Power(CartInfo *info)
{
 int x;

 IRQCount = IRQLatch = ~0;
 IRQa = 3;

 for(x = 0; x < 8; x++)
  CHRBanks[x] = ~0;

 for(x = 0; x < 4; x++)
  PRGBanks[x] = ~0;

 DoCHR();
 DoPRG();

 //IRQa = 0;
 //setprg16(0x8000, 0);
 //setprg16(0xc000, ~0);
}

int Mapper117_Init(CartInfo *info)
{
 info->Power = Power;
 GameHBIRQHook=Mapper117_hb;
 SetWriteHandler(0x8000,0xffff,Mapper117_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 return(1);
}

