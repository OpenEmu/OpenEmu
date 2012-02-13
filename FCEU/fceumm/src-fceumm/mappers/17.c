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


DECLFW(Mapper17_write)
{
        switch(A){
        default:
                   break;
        case 0x42FE:
                   onemir((V>>4)&1);
                   break;
        case 0x42FF:
                   MIRROR_SET((V>>4)&1);
                   break;
        case 0x4501:IRQa=0;X6502_IRQEnd(FCEU_IQEXT);break;
        case 0x4502:IRQCount&=0xFF00;IRQCount|=V;break;
        case 0x4503:IRQCount&=0x00FF;IRQCount|=V<<8;IRQa=1;break;
        case 0x4504: ROM_BANK8(0x8000,V);break;
        case 0x4505: ROM_BANK8(0xA000,V);break;
        case 0x4506: ROM_BANK8(0xC000,V);break;
        case 0x4507: ROM_BANK8(0xE000,V);break;
        case 0x4510: VROM_BANK1(0x0000,V);break;
        case 0x4511: VROM_BANK1(0x0400,V);break;
        case 0x4512: VROM_BANK1(0x0800,V);break;
        case 0x4513: VROM_BANK1(0x0C00,V);break;
        case 0x4514: VROM_BANK1(0x1000,V);break;
        case 0x4515: VROM_BANK1(0x1400,V);break;
        case 0x4516: VROM_BANK1(0x1800,V);break;
        case 0x4517: VROM_BANK1(0x1C00,V);break;
        }
}

void Mapper17_init(void)
{
MapIRQHook=FFEIRQHook;
SetWriteHandler(0x4020,0x5fff,Mapper17_write);
}
