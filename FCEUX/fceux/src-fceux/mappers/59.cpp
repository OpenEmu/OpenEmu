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

static int ay;
static DECLFW(Mapper59_write)
{
 //printf("$%04x:$%02x\n",A,V);
 setprg32(0x8000,(A&0x70)>>4);
 setchr8(A&0x7);
 //if(A&0x100)
 // setprg32r(0x10,0x8000,0);
 ay=A;
 MIRROR_SET2((A&0x8)>>3);
}

static DECLFR(m59rd)
{
 if(ay&0x100) return(0);
 else
  return(CartBR(A));
}

void Mapper59_init(void)
{
 setprg32(0x8000,0);
 SetReadHandler(0x8000,0xffff,m59rd);
 SetWriteHandler(0x8000,0xffff,Mapper59_write);
}
