/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2005 CaH4e3
 *  Copyright (C) 2009 qeed
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
 *
 * BMC 42-in-1
 * it seems now, mapper not reset-based,
 * tested on real hardware and it does menus switch by pressing just Select, not Reset
 * new registers behaviour proven this too
 *
 */

#include "mapinc.h"

static uint8 latche[2];
static SFORMAT StateRegs[]=
{
  {&latche, sizeof(latche), "LATCHE"},
  {0}
};

static void Sync(void)
{
  uint8 bank = (latche[0]&0x1f)|((latche[0]&0x80)>>2)|((latche[1]&1))<<6;
  if(!(latche[0] & 0x20))
      setprg32(0x8000,bank >> 1);
  else
  {
      setprg16(0x8000,bank);
      setprg16(0xC000,bank);
  }
  setmirror((latche[0]>>6)&1);
  setchr8(0);
}

static DECLFW(M226Write)
{
    latche[A & 1] = V;
    Sync();
}

static void M226Power(void)
{
  latche[0] = latche[1] = 0;
  Sync();
  SetWriteHandler(0x8000,0xFFFF,M226Write);
  SetReadHandler(0x8000,0xFFFF,CartBR);
}

static void StateRestore(int version)
{
  Sync();
}

void Mapper226_Init(CartInfo *info)
{
  info->Power=M226Power;
  AddExState(&StateRegs, ~0, 0, 0);
  GameStateRestore=StateRestore;
}

