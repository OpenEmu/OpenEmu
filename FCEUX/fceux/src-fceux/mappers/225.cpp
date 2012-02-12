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


#define reg1 mapbyte1[0]
#define reg2 mapbyte1[1]
#define reg3 mapbyte1[2]
#define reg4 mapbyte1[3]

DECLFR(A110in1read)
{
switch(A&0x3)
 {
  case 0:return reg1;break;
  case 1:return reg2;break;
  case 2:return reg3;break;
  case 3:return reg4;break;
 }
return 0xF;
}
DECLFW(A110in1regwr)
{
switch(A&0x3)
 {
  case 0:reg1=V&0xF;break;
  case 1:reg2=V&0xF;break;
  case 2:reg3=V&0xF;break;
  case 3:reg4=V&0xF;break;
 }
}

DECLFW(Mapper225_write)
{
 int banks=0;

 MIRROR_SET((A>>13)&1);
 if(A&0x4000)
  banks=1;
 else
  banks=0;

 VROM_BANK8(((A&0x003f)+(banks<<6)));
 if(A&0x1000)
  {
   if(A&0x40)
    {
     ROM_BANK16(0x8000,((((((A>>7)&0x1F)+(banks<<5)))<<1)+1));
     ROM_BANK16(0xC000,((((((A>>7)&0x1F)+(banks<<5)))<<1)+1));
    }
    else
    {
     ROM_BANK16(0x8000,(((((A>>7)&0x1F)+(banks<<5)))<<1));
     ROM_BANK16(0xC000,(((((A>>7)&0x1F)+(banks<<5)))<<1));
    }
  }
  else
  {
    ROM_BANK32(((((A>>7)&0x1F)+(banks<<5))));
  }
}

void Mapper225_init(void)
{
  SetWriteHandler(0x8000,0xffff,Mapper225_write);
  SetReadHandler(0x5800,0x5fff,A110in1read);
  SetWriteHandler(0x5800,0x5fff,A110in1regwr);
}

