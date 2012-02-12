/* FCE Ultra - NES/Famicom Emulator
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

static uint8 FFEmode;

#define FVRAM_BANK8(A,V) {VPage[0]=VPage[1]=VPage[2]=VPage[3]=VPage[4]=VPage[5]=VPage[6]=VPage[7]=V?&MapperExRAM[(V)<<13]-(A):&CHRRAM[(V)<<13]-(A);CHRBankList[0]=((V)<<3);CHRBankList[1]=((V)<<3)+1;CHRBankList[2]=((V)<<3)+2;CHRBankList[3]=((V)<<3)+3;CHRBankList[4]=((V)<<3)+4;CHRBankList[5]=((V)<<3)+5;CHRBankList[6]=((V)<<3)+6;CHRBankList[7]=((V)<<3)+7;PPUCHRRAM=0xFF;}

static void FFEIRQHook(int a)
{
  if(IRQa)
  {
   IRQCount+=a;
   if(IRQCount>=0x10000)
   {
    X6502_IRQBegin(FCEU_IQEXT);
    IRQa=0;
    IRQCount=0;
   }
  }
}

DECLFW(Mapper6_write)
{
        if(A<0x8000)
        {
                switch(A){
                case 0x42FF:MIRROR_SET((V>>4)&1);break;
                case 0x42FE:onemir((V>>3)&2); FFEmode=V&0x80;break;
                case 0x4501:IRQa=0;X6502_IRQEnd(FCEU_IQEXT);break;
                case 0x4502:IRQCount&=0xFF00;IRQCount|=V;break;
                case 0x4503:IRQCount&=0xFF;IRQCount|=V<<8;IRQa=1;break;
                }
        } else {
         switch (FFEmode)
         {
           case 0x80:  setchr8(V); break;
           default: ROM_BANK16(0x8000,V>>2);
                    FVRAM_BANK8(0x0000,V&3);
         }
        }
}
void Mapper6_StateRestore(int version)
{
 int x;
 for(x=0;x<8;x++)
  if(PPUCHRRAM&(1<<x))
  {
   if(CHRBankList[x]>7)
    VPage[x]=&MapperExRAM[(CHRBankList[x]&31)*0x400]-(x*0x400);
   else VPage[x]=&CHRRAM[(CHRBankList[x]&7)*0x400]-(x*0x400);
  }
}
void Mapper6_init(void)
{
MapIRQHook=FFEIRQHook;
ROM_BANK16(0xc000,7);

SetWriteHandler(0x4020,0x5fff,Mapper6_write);
SetWriteHandler(0x8000,0xffff,Mapper6_write);
MapStateRestore=Mapper6_StateRestore;
}
