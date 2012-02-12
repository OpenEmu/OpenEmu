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
 * (VRCII mapper)
 */

#include "mapinc.h"

#define K4buf mapbyte2
#define K4IRQ mapbyte1[1]
#define K4sel mapbyte1[0]

static int acount=0;
static int weirdo=0;
static DECLFW(Mapper25_write)
{
        if(A==0xC007)
		{
			weirdo=8; // Ganbare Goemon Gaiden does strange things!!! at the end credits
		              // quick dirty hack, seems there is no other games with such PCB, so
		              // we never know if it will not work for something else lol
			VROM_BANK1(0x0000,0xFC);
			VROM_BANK1(0x0400,0xFD);
			VROM_BANK1(0x0800,0xFF);
			VROM_BANK1(0x0c00,0xCF);
		}

		A=(A&0xF003)|((A&0xC)>>2);

        if((A&0xF000)==0xA000)
          ROM_BANK8(0xA000,V);
        else if(A>=0xB000 && A<=0xEFFF)
        {
         int x=(A&1)|((A-0xB000)>>11);

         K4buf[x]&=(0xF0)>>((A&2)<<1);
         K4buf[x]|=(V&0xF)<<((A&2)<<1);
		 if(weirdo)
			weirdo--;
		 else
			VROM_BANK1(x<<10,K4buf[x]);
        }
        else if((A&0xF000)==0x8000)
        {
         if(K4sel&2)
          ROM_BANK8(0xC000,V);
         else
          ROM_BANK8(0x8000,V);
        }
        else switch(A)
        {
         case 0x9000:switch(V&0x3)
                     {
                      case 0:MIRROR_SET(0);break;
                      case 1:MIRROR_SET(1);break;
                      case 2:onemir(0);break;
                      case 3:onemir(1);break;
                      }
                     break;
         case 0x9001:if((K4sel&2)!=(V&2))
                     {
                           uint8 swa;
                      swa=PRGBankList[0];
                       ROM_BANK8(0x8000,PRGBankList[2]);
                      ROM_BANK8(0xc000,swa);
                     }
                     K4sel=V;
                     break;
        case 0xf000:IRQLatch&=0xF0;IRQLatch|=V&0xF;break;
        case 0xf002:IRQLatch&=0x0F;IRQLatch|=V<<4;break;
        case 0xf001:IRQCount=IRQLatch;IRQa=V&2;K4IRQ=V&1;acount=0;X6502_IRQEnd(FCEU_IQEXT);break;
        case 0xf003:IRQa=K4IRQ;X6502_IRQEnd(FCEU_IQEXT);break;
 }
}

static void KonamiIRQHook(int a)
{
//  #define LCYCS ((227*2))
  #define LCYCS 341
  if(IRQa)
  {
   acount+=a*3;
  // acount+=a*4;
   if(acount>=LCYCS)
   {
    doagainbub:acount-=LCYCS;IRQCount++;
    if(IRQCount&0x100)
    {//acount=0;
     X6502_IRQBegin(FCEU_IQEXT);IRQCount=IRQLatch;
        }
    if(acount>=LCYCS) goto doagainbub;
   }
 }
}

void Mapper25_init(void)
{
        SetWriteHandler(0x8000,0xffff,Mapper25_write);
        MapIRQHook=KonamiIRQHook;
}

