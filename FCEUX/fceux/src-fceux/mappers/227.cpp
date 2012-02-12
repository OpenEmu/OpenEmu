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

#define rg mapbyte1

static void DoSync(uint32 A)
{
 int32 p=((A>>3)&0xF) | ((A>>4)&0x10);

 rg[0]=A;
 rg[1]=A>>8;

 MIRROR_SET((A>>1)&1);
 if(A&1)        //32 KB
 {
  ROM_BANK32(p);
 }
 else                //16 KB
 {
  ROM_BANK16(0x8000,(p<<1)|((A&4)>>2));
  ROM_BANK16(0xc000,(p<<1)|((A&4)>>2));
 }
 if(A&0x80)
 {
  PPUCHRRAM=0;
 }
 else
 {
  PPUCHRRAM=0xFF;
  if(A&0x200)
   ROM_BANK16(0xC000,(p<<1)|7);
  else
   ROM_BANK16(0xC000,(p<<1)&(~7));
 }
}

static DECLFW(Mapper227_write)
{
 rg[A&1]=V;
 DoSync(A);
}

static void M227Reset(void)
{
 rg[0]=rg[1]=0;
 DoSync(0);
}

static void M227Restore(int version)
{
 DoSync(rg[0]|(rg[1]<<8));
}

void Mapper227_init(void)
{
  SetWriteHandler(0x6000,0xffff,Mapper227_write);
  MapperReset=M227Reset;
  GameStateRestore=M227Restore;
  M227Reset();
}
