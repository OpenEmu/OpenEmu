/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 1998 BERO
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

static uint8 latche;

static DECLFW(Mapper34_write)
{
 switch(A)
 {
  case 0x7FFD:ROM_BANK32(V);break;
  case 0x7FFE:VROM_BANK4(0x0000,V);break;
  case 0x7fff:VROM_BANK4(0x1000,V);break;
 }
 if(A>=0x8000)
  ROM_BANK32(V);
}

void Mapper34_init(void)
{
 ROM_BANK32(0);
  SetWriteHandler(0x7ffd,0xffff,Mapper34_write);
}

/* I might want to add some code to the mapper 96 PPU hook function
   to not change CHR banks if the attribute table is being accessed,
   if I make emulation a little more accurate in the future.
*/

static uint8 M96LA;
static DECLFW(Mapper96_write)
{
 latche=V;
 setprg32(0x8000,V&3);
 setchr4r(0x10,0x0000,(latche&4)|M96LA);
 setchr4r(0x10,0x1000,(latche&4)|3);
}

static void M96Hook(uint32 A)
{
 if((A&0x3000)!=0x2000) return;
 //if((A&0x3ff)>=0x3c0) return;
 M96LA=(A>>8)&3;
 setchr4r(0x10,0x0000,(latche&4)|M96LA);
}

static void M96Sync(int v)
{
 setprg32(0x8000,latche&3);
 setchr4r(0x10,0x0000,(latche&4)|M96LA);
 setchr4r(0x10,0x1000,(latche&4)|3);
}

void Mapper96_init(void)
{
 SetWriteHandler(0x8000,0xffff,Mapper96_write);
 PPU_hook=M96Hook;
 AddExState(&latche, 1, 0, "LATC");
 AddExState(&M96LA, 1, 0, "LAVA");
 SetupCartCHRMapping(0x10, MapperExRAM, 32768, 1);
 latche=M96LA=0;
 M96Sync(0);
 setmirror(MI_0);
 GameStateRestore=M96Sync;
}

// DIS23C01 Open Soft, Korea
// Metal Force (K)
// Buzz and Waldog (K)

static DECLFW(M156Write)
{
 if(A>=0xc000 && A<=0xC003)
  VROM_BANK1((A&3)*1024,V);
 else if(A>=0xc008 &&  A<=0xc00b)
  VROM_BANK1(0x1000+(A&3)*1024,V);
 if(A==0xc010) ROM_BANK16(0x8000,V);
// printf("$%04x:$%02x\n",A,V);
}

void Mapper156_init(void)
{
 onemir(0);
 SetWriteHandler(0xc000,0xc010,M156Write);
}
