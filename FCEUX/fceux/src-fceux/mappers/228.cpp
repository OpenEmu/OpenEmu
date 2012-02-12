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

//16 bits of ram in total
//only use bottom 4 bits as ram though
static uint8 mapper228_ram[4];

static SFORMAT StateRegs[]=
{
  { mapper228_ram, 4, "MAPPER_RAM" },
  { 0 }
};

static DECLFR(Mapper228_read)
{
    return mapper228_ram[A & 3] & 0xF;
}

static DECLFW(Mapper228_write)
{
    uint32 page, pagel, pageh;

    //write to ram
    if (A < 0x6000)
    {
        mapper228_ram[A & 3] = V;
        return;
    }
    MIRROR_SET((A >> 13) & 1);
    page = (A >> 7) & 0x3F;

    if( (page & 0x30) == 0x30)
        page -= 0x10;

    pagel = pageh = (page << 1) + (((A >> 6) & 1) & ((A >> 5) & 1));
    pageh += ((A >> 5) & 1) ^ 1;
  
    ROM_BANK16(0x8000,pagel);
    ROM_BANK16(0xC000,pageh);
    VROM_BANK8( (V&0x3) | ((A&0xF)<<2) );
}

static void A52Reset(void)
{
    Mapper228_write(0x8000, 0);
}

void Mapper228_init(void)
{
    MapperReset=A52Reset;
    A52Reset();
    SetWriteHandler(0x8000, 0xFFFF, Mapper228_write);
    SetWriteHandler(0x4020, 0x5FFF, Mapper228_write);
    SetReadHandler (0x4020, 0x5FFF, Mapper228_read);
    AddExState(StateRegs, ~0, 0, 0);
}

