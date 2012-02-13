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

static writefunc oldmorko;

static DECLFW(morko)
{
 VROM_BANK8((V>>2)&1);
 oldmorko(A,V);
 setprg8(0x8000,V&0x4);        /* Special for VS Gumshoe */
}

void Mapper99_init(void)
{
 ROM_BANK32(0);
 oldmorko=GetWriteHandler(0x4016);
 SetWriteHandler(0x4016,0x4016,morko);
}
