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


#define IREMCon mapbyte1[0]

static DECLFW(Mapper32_write)
{
 switch(A>>12)
 {
  case 0x8:
           mapbyte1[1]=V;
           if(IREMCon) {ROM_BANK8(0xc000,V);ROM_BANK8(0x8000,~1);}
           else {ROM_BANK8(0x8000,V);ROM_BANK8(0xc000,~1);}
           break;
  case 0x9:IREMCon=(V>>1)&1;
           if(IREMCon) {ROM_BANK8(0xc000,mapbyte1[1]);ROM_BANK8(0x8000,~1);}
           else {ROM_BANK8(0x8000,mapbyte1[1]); ROM_BANK8(0xc000,~1);}
           MIRROR_SET(V&1);
           break;
  case 0xa:ROM_BANK8(0xA000,V);
           break;
 }

 if((A&0xF000)==0xb000)
  VROM_BANK1((A&0x7)<<10,V);
}

void Mapper32_init(void)
{
  ROM_BANK16(0x8000,0);
  ROM_BANK16(0xc000,~0);
  SetWriteHandler(0x8000,0xffff,Mapper32_write);
}
