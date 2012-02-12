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

#define MMC4reg mapbyte1
#define latcha1 mapbyte2[0]
#define latcha2 mapbyte2[1]


static void latchcheck(uint32 VAddr)
{
     uint8 l,h;

     h=VAddr>>8;

     if(h>=0x20 || ((h&0xF)!=0xF))
      return;

     l=VAddr&0xF0;

     if(h<0x10)
     {
      if(l==0xD0)
      {
       VROM_BANK4(0x0000,MMC4reg[0]);
       latcha1=0xFD;
      }
      else if(l==0xE0)
      {
       VROM_BANK4(0x0000,MMC4reg[1]);
       latcha1=0xFE;
      }
     }
     else
     {
      if(l==0xD0)
      {
       VROM_BANK4(0x1000,MMC4reg[2]);
       latcha2=0xFD;
      }
      else if(l==0xE0)
      {
       VROM_BANK4(0x1000,MMC4reg[3]);
       latcha2=0xFE;
      }
     }
}

DECLFW(Mapper9_write)        // $Axxx
{
 ROM_BANK8(0x8000,V);
}

DECLFW(Mapper10_write)
{
 ROM_BANK16(0x8000,V);
}

DECLFW(Mapper9and10_write)
{
       switch(A&0xF000)
       {
        case 0xB000:
                if(latcha1==0xFD) { VROM_BANK4(0x0000,V);}
                MMC4reg[0]=V;
                break;
        case 0xC000:
                if(latcha1==0xFE) {VROM_BANK4(0x0000,V);}
                MMC4reg[1]=V;
                break;
        case 0xD000:
                if(latcha2==0xFD) {VROM_BANK4(0x1000,V);}
                MMC4reg[2]=V;
                break;
        case 0xE000:
                if(latcha2==0xFE) {VROM_BANK4(0x1000,V);}
                MMC4reg[3]=V;
                break;
        case 0xF000:
                MIRROR_SET(V&1);
                break;
        }
}

void Mapper9_init(void)
{
        latcha1=0xFE;
        latcha2=0xFE;
        ROM_BANK8(0xA000,~2);
        ROM_BANK8(0x8000,0);
        SetWriteHandler(0xA000,0xAFFF,Mapper9_write);
        SetWriteHandler(0xB000,0xFFFF,Mapper9and10_write);
        PPU_hook=latchcheck;
}

void Mapper10_init(void)
{
        latcha1=latcha2=0xFE;
        SetWriteHandler(0xA000,0xAFFF,Mapper10_write);
        SetWriteHandler(0xB000,0xFFFF,Mapper9and10_write);
        PPU_hook=latchcheck;
}

