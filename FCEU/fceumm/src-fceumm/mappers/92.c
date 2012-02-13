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

/* Original code provided by LULU */

static DECLFW(Mapper92_write)
{
 uint8 reg=(A&0xF0)>>4;
 uint8 bank=A&0xF;

 if(A>=0x9000)
 {
  if(reg==0xD) ROM_BANK16(0xc000,bank);
  else if(reg==0xE) VROM_BANK8(bank);
 }
 else
 {
  if(reg==0xB) ROM_BANK16(0xc000,bank);
  else if(reg==0x7) VROM_BANK8(bank);
 }
}

void Mapper92_init(void)
{
 SetWriteHandler(0x8000,0xFFFF,Mapper92_write);
}
