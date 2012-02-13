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
 * (VRC4 mapper)
 */

#include "mapinc.h"

#define K4buf mapbyte2



DECLFW(Mapper22_write)
{
        if(A<=0xAFFF)
         {
          switch(A&0xF000)
          {
          case 0x8000:ROM_BANK8(0x8000,V);break;
          case 0xa000:ROM_BANK8(0xA000,V);break;
          case 0x9000:switch(V&3)
                      {
                       case 0x00:MIRROR_SET2(1);break;
                       case 0x01:MIRROR_SET2(0);break;
                       case 0x02:onemir(0);break;
                       case 0x03:onemir(1);break;
                      }
                      break;
          }
         }
        else
        {
          A&=0xF003;
          if(A>=0xb000 && A<=0xe003)
          {
           int x=(A&1)|((A-0xB000)>>11);

           K4buf[x]&=(0xF0)>>((A&2)<<1);
           K4buf[x]|=(V&0xF)<<((A&2)<<1);
           VROM_BANK1(x<<10,K4buf[x]>>1);
          }
        }
}


void Mapper22_init(void)
{
        SetWriteHandler(0x8000,0xffff,Mapper22_write);
}
