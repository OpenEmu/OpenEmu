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


static DECLFW(Mapper40_write)
{
 switch(A&0xe000)
 {
  case 0x8000:IRQa=0;IRQCount=0;X6502_IRQEnd(FCEU_IQEXT);break;
  case 0xa000:IRQa=1;break;
  case 0xe000:ROM_BANK8(0xc000,V&7);break;
 }
}

static void Mapper40IRQ(int a)
{
 if(IRQa)
 {
        if(IRQCount<4096)
         IRQCount+=a;
        else
        {
         IRQa=0;
         X6502_IRQBegin(FCEU_IQEXT);
        }
 }
}

void Mapper40_init(void)
{
  ROM_BANK8(0x6000,(~0)-1);
  ROM_BANK8(0x8000,(~0)-3);
  ROM_BANK8(0xa000,(~0)-2);
  SetWriteHandler(0x8000,0xffff,Mapper40_write);
  SetReadHandler(0x6000,0x7fff,CartBR);
  MapIRQHook=Mapper40IRQ;
}


