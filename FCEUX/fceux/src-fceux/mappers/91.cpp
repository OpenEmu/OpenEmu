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

static DECLFW(Mapper91_write)
{
//if(A>=0x7001)
//printf("$%04x:$%02x, %d\n",A,V,scanline);
 A&=0xF007;

 if(A>=0x6000 && A<=0x6003) VROM_BANK2((A&3)*2048,V);
 else switch(A&0xF003)
 {
  case 0x7000:
  case 0x7001:ROM_BANK8(0x8000+(A&1)*8192,V);break;
  case 0x7002:IRQa=IRQCount=0;X6502_IRQEnd(FCEU_IQEXT);break;
  case 0x7003:IRQa=1;X6502_IRQEnd(FCEU_IQEXT);break;
//  default: printf("Iyee: $%04x:$%02x\n",A,V);break;
 }
 //if(A>=0x7000)
 // printf("$%04x:$%02x, %d\n",A,V,scanline);
}

static void Mapper91_hb(void)
{
 if(IRQCount<8 && IRQa)
 {
  IRQCount++;
  if(IRQCount>=8)
  {
    X6502_IRQBegin(FCEU_IQEXT);   
  } 
 }
}

void Mapper91_init(void)
{
  SetWriteHandler(0x4020,0xFFFF,Mapper91_write);
  GameHBIRQHook=Mapper91_hb;
} 

