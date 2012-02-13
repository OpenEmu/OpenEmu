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

#define K4buf mapbyte2
#define K4buf2 mapbyte3

void JalecoIRQHook(int a)
{
  if(IRQa && IRQCount)
  {
   IRQCount-=a;
   if(IRQCount<=0)
   {
    X6502_IRQBegin(FCEU_IQEXT);
    IRQCount=0;
    IRQa=0;
   }
  }
}

DECLFW(Mapper18_write)
{
        A&=0xF003;
        if(A>=0x8000 && A<=0x9001)
        {
         int x=((A>>1)&1)|((A-0x8000)>>11);

         K4buf2[x]&=(0xF0)>>((A&1)<<2);
         K4buf2[x]|=(V&0xF)<<((A&1)<<2);
         ROM_BANK8(0x8000+(x<<13),K4buf2[x]);
        }
        else if(A>=0xa000 && A<=0xd003)
        {
         int x=((A>>1)&1)|((A-0xA000)>>11);

         K4buf[x]&=(0xF0)>>((A&1)<<2);
         K4buf[x]|=(V&0xF)<<((A&1)<<2);
         VROM_BANK1(x<<10,K4buf[x]);
        }
        else switch(A)
        {
           case 0xe000:IRQLatch&=0xFFF0;IRQLatch|=(V&0x0f);break;
         case 0xe001:IRQLatch&=0xFF0F;IRQLatch|=(V&0x0f)<<4;break;
          case 0xe002:IRQLatch&=0xF0FF;IRQLatch|=(V&0x0f)<<8;break;
         case 0xe003:IRQLatch&=0x0FFF;IRQLatch|=(V&0x0f)<<12;break;
         case 0xf000:IRQCount=IRQLatch;
                     break;
         case 0xf001:IRQa=V&1;
                     X6502_IRQEnd(FCEU_IQEXT);
                     break;
         case 0xf002:MIRROR_SET2(V&1);
                     if(V&2) onemir(0);
                      break;
        }
}

void Mapper18_init(void)
{
 SetWriteHandler(0x8000,0xffff,Mapper18_write);
 MapIRQHook=JalecoIRQHook;
}
