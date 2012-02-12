/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2005 CaH4e3
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
#include "mmc3.h"

static uint8 lut[4] = { 0x00, 0x02, 0x02, 0x03 };

static DECLFW(UNL6035052ProtWrite)
{
  EXPREGS[0]=lut[V&3];  
}

static DECLFR(UNL6035052ProtRead)
{
  return EXPREGS[0];
}

static void UNL6035052Power(void)
{
  GenMMC3Power();
  SetWriteHandler(0x4020,0x7FFF,UNL6035052ProtWrite);
  SetReadHandler(0x4020,0x7FFF,UNL6035052ProtRead);
}

void UNL6035052_Init(CartInfo *info)
{
  GenMMC3_Init(info, 128, 256, 0, 0);
  info->Power=UNL6035052Power;
  AddExState(EXPREGS, 6, 0, "EXPR");
}
