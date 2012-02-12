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


#define suntoggle mapbyte1[0]

static DECLFW(Mapper67_write)
{
 A&=0xF800;
 if((A&0x800) && A<=0xb800)
 {
  VROM_BANK2((A-0x8800)>>1,V);
 }
 else switch(A)
 {
  case 0xc800:
  case 0xc000:if(!suntoggle)
              {
               IRQCount&=0xFF;
               IRQCount|=V<<8;
              }
              else
              {
               IRQCount&=0xFF00;
               IRQCount|=V;
              }
              suntoggle^=1;
              break;
  case 0xd800:suntoggle=0;IRQa=V&0x10;X6502_IRQEnd(FCEU_IQEXT);break;

  case 0xe800:switch(V&3)
              {
               case 0:MIRROR_SET2(1);break;
               case 1:MIRROR_SET2(0);break;
               case 2:onemir(0);break;
               case 3:onemir(1);break;
              }
              break;
  case 0xf800:ROM_BANK16(0x8000,V);break;
 }
}
static void SunIRQHook(int a)
{
  if(IRQa)
  {
   IRQCount-=a;
   if(IRQCount<=0)
   {
    X6502_IRQBegin(FCEU_IQEXT);
    IRQa=0;
    IRQCount=0xFFFF;
   }
  }
}
void Mapper67_init(void)
{
SetWriteHandler(0x8000,0xffff,Mapper67_write);
MapIRQHook=SunIRQHook;
}
