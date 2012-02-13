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


#define calreg mapbyte1[0]
#define calchr mapbyte1[1]

DECLFW(Mapper41_write)
{
 if(A<0x8000)
 {
  ROM_BANK32(A&7);
  MIRROR_SET((A>>5)&1);
  calreg=A;
  calchr&=0x3;
  calchr|=(A>>1)&0xC;
  VROM_BANK8(calchr);
 }
 else if(calreg&0x4)
 {
  calchr&=0xC;
  calchr|=A&3;
  VROM_BANK8(calchr);
 }
}

static void M41Reset(void)
{
 calreg=calchr=0;
}

void Mapper41_init(void)
{
 MapperReset=M41Reset;
 ROM_BANK32(0);
 SetWriteHandler(0x8000,0xffff,Mapper41_write);
 SetWriteHandler(0x6000,0x67ff,Mapper41_write);
}
