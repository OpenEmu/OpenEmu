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


static DECLFW(Mapper61_write)
{
// printf("$%04x:$%02x\n",A,V);
 switch(A&0x30)
 {
  case 0x00:
  case 0x30:
          ROM_BANK32(A&0xF);
          break;
  case 0x20:
  case 0x10:
          ROM_BANK16(0x8000,((A&0xF)<<1)| (((A&0x20)>>4)) );
          ROM_BANK16(0xC000,((A&0xF)<<1)| (((A&0x20)>>4)) );
          break;
 }
 #ifdef moo
 if(!(A&0x10))
  ROM_BANK32(A&0xF);
 else
 {
  ROM_BANK16(0x8000,((A&0xF)<<1)| (((A&0x10)>>4)^1) );
  ROM_BANK16(0xC000,((A&0xF)<<1)| (((A&0x10)>>4)^1) );
 }
 #endif
 MIRROR_SET((A&0x80)>>7);
}

void Mapper61_init(void)
{
 SetWriteHandler(0x8000,0xffff,Mapper61_write);
}
