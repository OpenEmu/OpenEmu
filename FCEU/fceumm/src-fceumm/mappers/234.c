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

#define r1 mapbyte1[0]
#define r2 mapbyte1[1]

static void DoBS(void)
{
 if(r1&0x40)
 {
  ROM_BANK32((r1&0xE)|(r2&1));
  VROM_BANK8( ((r1&0xE)<<2) | ((r2>>4)&7) );
 }
 else
 {
  ROM_BANK32(r1&0xF);
  VROM_BANK8( ((r1&0xF)<<2) | ((r2>>4)&3) );
 }
}

static void R1Set(uint8 V)
{
 if(r1) return;
 r1=V;
 MIRROR_SET(V>>7);
 DoBS();
}

static void R2Set(uint8 V)
{
 r2=V;
 DoBS();
}

DECLFW(R1W)
{
 R1Set(V);
}

DECLFR(R1R)
{
 uint8 r=CartBR(A);
 R1Set(r);
 return r;
}

DECLFW(R2W)
{
 R2Set(V);
}

DECLFR(R2R)
{
 uint8 r=CartBR(A);
 R2Set(r);
 return r;
}

static void M15Restore(int version)
{
 DoBS();
 MIRROR_SET(r1>>7);
}

static void M15Reset(void)
{
 r1=r2=0;
 DoBS();
 MIRROR_SET(0);
}

void Mapper234_init(void)
{
        SetWriteHandler(0xff80,0xff9f,R1W);
        SetReadHandler(0xff80,0xff9f,R1R);

        SetWriteHandler(0xffe8,0xfff7,R2W);
        SetReadHandler(0xffe8,0xfff7,R2R);

        SetReadHandler(0x6000,0x7FFF,0);
        SetWriteHandler(0x6000,0x7FFF,0);

        M15Reset();

        GameStateRestore=M15Restore;
        MapperReset=M15Reset;
}

