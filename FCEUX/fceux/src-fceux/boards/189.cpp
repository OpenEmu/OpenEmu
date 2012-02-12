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

static void M189PW(uint32 A, uint8 V)
{
  setprg32(0x8000,EXPREGS[0]&3);
}

static DECLFW(M189Write)
{
  EXPREGS[0]=V|(V>>4); //actually, there is a two versions of 189 mapper with hi or lo bits bankswitching.
  FixMMC3PRG(MMC3_cmd);
}

static void M189Power(void)
{
  EXPREGS[0]=EXPREGS[1]=0;
  GenMMC3Power();
  SetWriteHandler(0x4120,0x7FFF,M189Write);
}

void Mapper189_Init(CartInfo *info)
{
  GenMMC3_Init(info, 256, 256, 0, 0);
  pwrap=M189PW;
  info->Power=M189Power;
  AddExState(EXPREGS, 2, 0, "EXPR");
}
