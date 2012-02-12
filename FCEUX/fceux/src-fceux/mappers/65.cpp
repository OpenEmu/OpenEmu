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


void IREMIRQHook(int a)
{
  if(IRQa)
  {
   IRQCount-=a;
   if(IRQCount<-4)
   {
    X6502_IRQBegin(FCEU_IQEXT);
    IRQa=0;
    IRQCount=0xFFFF;
   }
  }
}

static DECLFW(Mapper65_write)
{
 //if(A>=0x9000 && A<=0x9006)
 // printf("$%04x:$%02x, %d\n",A,V,scanline);
 switch(A)
 {
  //default: printf("$%04x:$%02x\n",A,V);
  //        break;
  case 0x8000:ROM_BANK8(0x8000,V);break;
 // case 0x9000:printf("$%04x:$%02x\n",A,V);MIRROR_SET2((V>>6)&1);break;
  case 0x9001:MIRROR_SET(V>>7);break;
  case 0x9003:IRQa=V&0x80;X6502_IRQEnd(FCEU_IQEXT);break;
  case 0x9004:IRQCount=IRQLatch;break;
  case 0x9005:          IRQLatch&=0x00FF;
                        IRQLatch|=V<<8;
                        break;
  case 0x9006:          IRQLatch&=0xFF00;IRQLatch|=V;
                        break;
  case 0xB000:VROM_BANK1(0x0000,V);break;
  case 0xB001:VROM_BANK1(0x0400,V);break;
  case 0xB002:VROM_BANK1(0x0800,V);break;
  case 0xB003:VROM_BANK1(0x0C00,V);break;
  case 0xB004:VROM_BANK1(0x1000,V);break;
  case 0xB005:VROM_BANK1(0x1400,V);break;
  case 0xB006:VROM_BANK1(0x1800,V);break;
  case 0xB007:VROM_BANK1(0x1C00,V);break;
  case 0xa000:ROM_BANK8(0xA000,V);break;
  case 0xC000:ROM_BANK8(0xC000,V);break;
 }
 //MIRROR_SET2(1);
}

void Mapper65_init(void)
{
 MapIRQHook=IREMIRQHook;
 SetWriteHandler(0x8000,0xffff,Mapper65_write);
}
