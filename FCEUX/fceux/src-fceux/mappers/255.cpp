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



static DECLFW(Mapper255_write)
{
 uint32 pbank=(A>>7)&0x1F;
 uint32 cbank=A&0x3F;
 uint32 rbank=(A>>14)&1;

// printf("$%04x:$%02x\n",A,V);
// printf("%2x:%2x:%2x,%2x\n",pbank,cbank,rbank,(A&0x40)>>6);

 if(A&0x1000)
 {
  ROM_BANK16(0x8000,((pbank|(rbank<<5))<<1)|((A&0x40)>>6));
  ROM_BANK16(0xc000,((pbank|(rbank<<5))<<1)|((A&0x40)>>6));
 }
 else
 {
  ROM_BANK32(pbank|(rbank<<5));
 }
 MIRROR_SET((A>>13)&1);
 VROM_BANK8((rbank<<6)|cbank);
}

static DECLFW(Mapper255_wl)
{
// printf("Wr: $%04x:$%02x\n",A,V);
 mapbyte1[A&3]=V&0xF;
}

static DECLFR(Mapper255_read)
{
 //printf("Rd: $%04x\n",A);
 return(mapbyte1[A&3]); //|(X.DB&0xF0));
}

void Mapper255_init(void)
{
 mapbyte1[0]=mapbyte1[1]=0xF;
 ROM_BANK32(0);
 VROM_BANK8(0);
 SetWriteHandler(0x5800,0x5FF0,Mapper255_wl);
 SetWriteHandler(0x8000,0xffff,Mapper255_write);
 SetReadHandler(0x5800,0x5FFF,Mapper255_read);
}
