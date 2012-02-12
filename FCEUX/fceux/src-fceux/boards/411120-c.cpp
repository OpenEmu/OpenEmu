/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2008 CaH4e3
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

// actually cart ID is 811120-C, sorry ;) K-3094 - another ID

#include "mapinc.h"
#include "mmc3.h"

static uint8 reset_flag = 0;

static void BMC411120CCW(uint32 A, uint8 V)
{
    setchr1(A,V|((EXPREGS[0]&3)<<7));
}

static void BMC411120CPW(uint32 A, uint8 V)
{
    if(EXPREGS[0]&(8|reset_flag))
      setprg32(0x8000,((EXPREGS[0]>>4)&3)|(0x0C));
    else
      setprg8(A,(V&0x0F)|((EXPREGS[0]&3)<<4));
}

static DECLFW(BMC411120CLoWrite)
{
  EXPREGS[0] = A;
  FixMMC3PRG(MMC3_cmd);
  FixMMC3CHR(MMC3_cmd);
}

static void BMC411120CReset(void)
{
  EXPREGS[0]=0;
  reset_flag ^=4;
  MMC3RegReset();
}

static void BMC411120CPower(void)
{
  EXPREGS[0] = 0;
  GenMMC3Power();
  SetWriteHandler(0x6000,0x7FFF,BMC411120CLoWrite);
}

void BMC411120C_Init(CartInfo *info)
{
  GenMMC3_Init(info, 128, 128, 8, 0);
  pwrap=BMC411120CPW;
  cwrap=BMC411120CCW;
  info->Power=BMC411120CPower;
  info->Reset=BMC411120CReset;
  AddExState(EXPREGS, 1, 0, "EXPR");
}
