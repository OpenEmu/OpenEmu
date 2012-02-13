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

extern uint32 ROM_size;
static uint8 latche;

static void Sync(void)
{
  if(latche)
  {
    if(latche&0x10)
      setprg16(0x8000,(latche&7));
    else
      setprg16(0x8000,(latche&7)|8);
  }
  else
    setprg16(0x8000,7+(ROM_size>>4));
}

static DECLFW(M188Write)
{
  latche=V;
  Sync();
}

static DECLFR(ExtDev)
{
  return(3);
}

static void Power(void)
{
  latche=0;
  Sync();
  setchr8(0);
  setprg16(0xc000,0x7);
  SetReadHandler(0x6000,0x7FFF,ExtDev);
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xFFFF,M188Write);
}

static void StateRestore(int version)
{
  Sync();
}

void Mapper188_Init(CartInfo *info)
{
  info->Power=Power;
  GameStateRestore=StateRestore;
  AddExState(&latche, 1, 0, "LATCH");
}
