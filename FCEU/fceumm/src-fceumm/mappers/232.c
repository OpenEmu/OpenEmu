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

static void DoIt(void)
{
  ROM_BANK16(0x8000,(mapbyte1[1]&3) | ((mapbyte1[0]&0x18)>>1));
  ROM_BANK16(0xc000,3|(((mapbyte1[0])&0x18)>>1));
}

DECLFW(Mapper232_write)
{
        if(A<=0x9FFF)
         mapbyte1[0]=V;
        else
         mapbyte1[1]=V;
        DoIt();
}

static void QuattroReset(void)
{
 mapbyte1[0]=0x18;
 DoIt();
}

void Mapper232_init(void)
{
 SetWriteHandler(0x6000,0xffff,Mapper232_write);
 MapperReset=QuattroReset;
 QuattroReset();
}

