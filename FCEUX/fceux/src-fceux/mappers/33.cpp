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

static int is48;

static DECLFW(Mapper33_write)
{
        //printf("%04x:%02x, %d\n",A,V,scanline);

        A&=0xF003;
        if(A>=0xA000 && A<=0xA003)
         VROM_BANK1(0x1000+((A&3)<<10),V);
        else switch(A)
        {
          case 0x8000:if(!is48) MIRROR_SET((V>>6)&1);
                     ROM_BANK8(0x8000,V);
                     break;
         case 0x8001:ROM_BANK8(0xA000,V); break;
         case 0x8002:VROM_BANK2(0x0000,V);break;
         case 0x8003:VROM_BANK2(0x0800,V);break;
         case 0xe000:MIRROR_SET((V>>6)&1);break;
        }
}

static DECLFW(Mapper48_HiWrite)
{
        switch(A&0xF003)
        {
         case 0xc000:IRQLatch=V;break;
         case 0xc001:IRQCount=IRQLatch;break;
         case 0xc003:IRQa=0;X6502_IRQEnd(FCEU_IQEXT);break;
         case 0xc002:IRQa=1;break;
         case 0xe000:MIRROR_SET((V>>6)&1);break;
        }
}

static void heho(void)
{
 if(IRQa)
 {
   IRQCount++;
   if(IRQCount==0x100)
   {
    X6502_IRQBegin(FCEU_IQEXT);
    IRQa=0;
   }
 }
}

void Mapper33_init(void)
{
        SetWriteHandler(0x8000,0xffff,Mapper33_write);
        is48=0;
}

void Mapper48_init(void)
{
        SetWriteHandler(0x8000,0xbfff,Mapper33_write);
        SetWriteHandler(0xc000,0xffff,Mapper48_HiWrite);
        GameHBIRQHook=heho;
        is48=1;
}
