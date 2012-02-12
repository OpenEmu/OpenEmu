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

DECLFW(Mapper86_write)
{
 if(A>=0x6000 && A<=0x6fFF)
 {
  VROM_BANK8((V&3)|((V>>4)&4));
  ROM_BANK32((V>>4)&3);
 }
 //else
 //if(A!=0x6000)
 // printf("$%04x:$%02x\n",A,V);
}
void Mapper86_init(void)
{
 SetWriteHandler(0x6000,0x6fff,Mapper86_write);
 SetWriteHandler(0x4020,0xffff,Mapper86_write);
}
