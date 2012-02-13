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
 * TXC mapper variation, F-15 City War
 */

#include "mapinc.h"



DECLFW(Mapper79_write)
{
 if(A<0x8000 && ((A^0x4100)==0))
 {
  ROM_BANK32((V>>3)&1);
 }
 VROM_BANK8(V);
}

void Mapper79_init(void)
{
 ROM_BANK32(~0);
 SetWriteHandler(0x8000,0xffff,Mapper79_write);
 SetWriteHandler(0x4020,0x5fff,Mapper79_write);
}

