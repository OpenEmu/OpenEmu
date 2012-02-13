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



DECLFW(Mapper71_write)
{
switch(A&0xF000)
 {
 case 0xF000:
 case 0xE000:
 case 0xD000:
 case 0xC000:ROM_BANK16(0x8000,V);break;
 case 0x9000:onemir((V>>3)&2);break;
 }
}

void Mapper71_init(void)
{
SetWriteHandler(0x4020,0xffff,Mapper71_write);
}

