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



static DECLFW(Mapper73_write)
{
 //if(A>=0xd000 && A<=0xdfff)
  X6502_IRQEnd(FCEU_IQEXT);        /* How are IRQs acknowledged on this chip? */
 switch(A&0xF000)
 {
  //default: printf("$%04x:$%02x\n",A,V);break;
  case 0x8000:IRQCount&=0xFFF0;IRQCount|=(V&0xF);break;
  case 0x9000:IRQCount&=0xFF0F;IRQCount|=(V&0xF)<<4;break;
  case 0xa000:IRQCount&=0xF0FF;IRQCount|=(V&0xF)<<8;break;
  case 0xb000:IRQCount&=0x0FFF;IRQCount|=(V&0xF)<<12;break;
  case 0xc000:IRQa=V&2;break;
  case 0xf000:ROM_BANK16(0x8000,V);break;
 }
}

static void Mapper73IRQHook(int a)
{
 if(IRQa)
 {
  IRQCount+=a;
  if(IRQCount>=0xFFFF)
  {
   IRQCount&=0xFFFF;
   IRQa=0;
   X6502_IRQBegin(FCEU_IQEXT);
  }
 }
}

void Mapper73_init(void)
{
 SetWriteHandler(0x8000,0xffff,Mapper73_write);
 MapIRQHook=Mapper73IRQHook;
}

