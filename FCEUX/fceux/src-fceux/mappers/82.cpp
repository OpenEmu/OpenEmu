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

#define ctrl        mapbyte1[6]

static void DoCHR(void)
{
 int x;

 for(x=0;x<2;x++)
  VROM_BANK2((x<<11)|((ctrl&2)<<11),mapbyte1[x]>>1);
 for(x=0;x<4;x++)
  VROM_BANK1((x<<10) | (((ctrl&2)^2)<<11),mapbyte1[2+x]);
}

static DECLFW(Mapper82_write)
{
 if(A<=0x7EF5)
 {
  mapbyte1[A&7]=V;
  DoCHR();
 }
 else
  switch(A)
  {
   case 0x7ef6:ctrl=V&3;
               MIRROR_SET2(V&1);
                DoCHR();
                 break;
   case 0x7efa:V>>=2;mapbyte2[0]=V;ROM_BANK8(0x8000,V);break;
   case 0x7efb:V>>=2;mapbyte2[1]=V;ROM_BANK8(0xa000,V);break;
   case 0x7efc:V>>=2;mapbyte2[2]=V;ROM_BANK8(0xc000,V);break;
  }
}

void Mapper82_init(void)
{
 ROM_BANK8(0xE000,~0);

 /* external WRAM might end at $73FF */
 SetWriteHandler(0x7ef0,0x7efc,Mapper82_write);
}

