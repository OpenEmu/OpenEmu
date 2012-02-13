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
 *
 * FDS Conversion
 *
 */

#include "mapinc.h"

static uint8 reg, mirr;

static SFORMAT StateRegs[]=
{
  {&reg, 1, "REG"},
  {&mirr, 1, "MIRR"},
  {0}
};

static void Sync(void)
{
  setprg8(0x6000, reg);
  setprg32r(1, 0x8000, 0);
  setchr8(0);
  setmirror(mirr);
}

static DECLFW(AC08Mirr)
{
  mirr = ((V&8)>>3)^1;
  Sync();
}

static DECLFW(AC08Write)
{
  reg = V & 15;
  Sync();
}

static void AC08Power(void)
{
  reg = 0;
  Sync();
  SetReadHandler(0x6000,0xFFFF,CartBR);
  SetWriteHandler(0x4025,0x4025,AC08Mirr);
  SetWriteHandler(0x8000,0xFFFF,AC08Write);
}

static void StateRestore(int version)
{
  Sync();
}

void AC08_Init(CartInfo *info)
{
  info->Power=AC08Power;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
