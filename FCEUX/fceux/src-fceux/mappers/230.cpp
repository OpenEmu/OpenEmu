/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2003 CaH4e3
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

#define rom_sw mapbyte1[0]

void Mapper230_Reset(void)
{
        rom_sw ^= 1; //1 - rom_sw;

        if( rom_sw ) {
                ROM_BANK16(0x8000,0);
                ROM_BANK16(0xc000,7);
        } else {
                ROM_BANK16(0x8000,8);
                ROM_BANK16(0xc000,39);
        }
        MIRROR_SET2(1);
}

static DECLFW(Mapper230_write)
{
        if( rom_sw ) {
                ROM_BANK16( 0x8000, V&0x07 );
        } else {
                if( V & 0x20 ) {
                        ROM_BANK16( 0x8000, (V&0x1F)+8 );
                        ROM_BANK16( 0xc000, (V&0x1F)+8 );
                } else {
                        ROM_BANK32( ((V&0x1E) >> 1) + 4 );
                }
                MIRROR_SET2( ((V & 0x40) >> 6) );
        }
}

void Mapper230_init(void)
{
        ROM_BANK16(0x8000,0);
        ROM_BANK16(0xc000,7);
        SetWriteHandler(0x8000, 0xffff, Mapper230_write);
        MapperReset = Mapper230_Reset;
        rom_sw = 1;
}

