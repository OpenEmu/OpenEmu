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


static DECLFW(Mapper42_write)
{
// FCEU_printf("%04x:%04x\n",A,V);
 switch(A&0xe003)
 {
  case 0x8000:VROM_BANK8(V);break;
  case 0xe000:mapbyte1[0]=V;ROM_BANK8(0x6000,V&0xF);break;
  case 0xe001:MIRROR_SET((V>>3)&1);break;
  case 0xe002:IRQa=V&2;if(!IRQa) IRQCount=0;X6502_IRQEnd(FCEU_IQEXT);break;
 }
}

static void Mapper42IRQ(int a)
{
 if(IRQa)
 {
        IRQCount+=a;
        if(IRQCount>=32768) IRQCount-=32768;
        if(IRQCount>=24576)
         X6502_IRQBegin(FCEU_IQEXT);
        else
         X6502_IRQEnd(FCEU_IQEXT);
 }
}

static void Mapper42_StateRestore(int version)
{
    ROM_BANK8(0x6000,mapbyte1[0]&0xF);
}


void Mapper42_init(void)
{
  ROM_BANK8(0x6000,0);
  ROM_BANK32(~0);
  SetWriteHandler(0x6000,0xffff,Mapper42_write);
  SetReadHandler(0x6000,0x7fff,CartBR);
  MapStateRestore=Mapper42_StateRestore;
  MapIRQHook=Mapper42IRQ;
}

