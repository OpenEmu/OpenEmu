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


DECLFW(Mapper246_write)
{
 switch(A&0xF007)
 {
 case 0x6000:ROM_BANK8(0x8000,V);break;
 case 0x6001:ROM_BANK8(0xA000,V);break;
 case 0x6002:ROM_BANK8(0xC000,V);break;
 case 0x6003:ROM_BANK8(0xE000,V);break;
 case 0x6004:VROM_BANK2(0x0000,V);break;
 case 0x6005:VROM_BANK2(0x0800,V);break;
 case 0x6006:VROM_BANK2(0x1000,V);break;
 case 0x6007:VROM_BANK2(0x1800,V);break;
 }
}

void Mapper246_init(void)
{
  SetWriteHandler(0x4020,0x67ff,Mapper246_write);
  SetWriteHandler(0x8000,0xffff,Mapper246_write);
}

