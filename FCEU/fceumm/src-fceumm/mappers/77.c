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

/* Original code provided by LULU */

static DECLFW(Mapper77_write)
{
 mapbyte1[0]=V;
 ROM_BANK32(V&0x7);
 VROM_BANK2(0x0000, (V&0xf0)>>4);
}

static void Mapper77_StateRestore(int version)
{
 int x;
 
 if(version>=7200)
 {
  ROM_BANK32(mapbyte1[0]&0x7);
  VROM_BANK2(0x0000, (mapbyte1[0]&0xf0)>>4);
 }
 for(x=2;x<8;x++)
  VRAM_BANK1(x*0x400,x);
}

void Mapper77_init(void)
{
 int x;

 ROM_BANK32(0);
 for(x=2;x<8;x++)
  VRAM_BANK1(x*0x400,x);
 SetWriteHandler(0x6000,0xffff,Mapper77_write);
 MapStateRestore=Mapper77_StateRestore;
}
