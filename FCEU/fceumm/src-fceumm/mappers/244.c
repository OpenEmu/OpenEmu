/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2011 CaH4e3
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

static uint8 prg_perm[4][4] = {
       {0, 1, 2, 3,},
       {3, 2, 1, 0,}, 
       {0, 2, 1, 3,},
       {3, 1, 2, 0,},
};

static uint8 chr_perm[8][8] = {
       {0, 1, 2, 3, 4, 5, 6, 7,},
       {0, 2, 1, 3, 4, 6, 5, 7,},
       {0, 1, 4, 5, 2, 3, 6, 7,},
       {0, 4, 1, 5, 2, 6, 3, 7,},
       {0, 4, 2, 6, 1, 5, 3, 7,},
       {0, 2, 4, 6, 1, 3, 5, 7,},
       {7, 6, 5, 4, 3, 2, 1, 0,},
       {7, 6, 5, 4, 3, 2, 1, 0,},
};

static DECLFW(Mapper244_write)
{
  if(V&8)
    VROM_BANK8(chr_perm[(V>>4)&7][V&7]);
  else
    ROM_BANK32(prg_perm[(V>>4)&3][V&3]);
}

void Mapper244_init(void)
{
  ROM_BANK32(0);
  SetWriteHandler(0x8000,0xffff,Mapper244_write);
}
