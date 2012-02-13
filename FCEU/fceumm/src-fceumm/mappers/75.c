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


#define map75sel mapbyte1[0]
#define map75ar  mapbyte2

DECLFW(Mapper75_write)
{
switch(A&0xF000)
 {
 case 0x8000:ROM_BANK8(0x8000,V);break;
 case 0x9000:
             VROM_BANK4(0x0000,map75ar[0]|((V&2)<<3));
             VROM_BANK4(0x1000,map75ar[1]|((V&4)<<2));
             map75sel=V;MIRROR_SET(V&1);break;
 case 0xa000:ROM_BANK8(0xa000,V);break;
 case 0xc000:ROM_BANK8(0xc000,V);break;
 case 0xe000:V&=0xF;map75ar[0]=V;V|=(map75sel&2)<<3;VROM_BANK4(0x0000,V);break;
 case 0xf000:V&=0xF;map75ar[1]=V;V|=(map75sel&4)<<2;VROM_BANK4(0x1000,V);break;
 }
}

void Mapper75_init(void)
{
SetWriteHandler(0x8000,0xffff,Mapper75_write);
}

