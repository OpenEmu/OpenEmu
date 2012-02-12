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

extern uint8 m114_perm[8];

static void H2288PW(uint32 A, uint8 V)
{
  if(EXPREGS[0]&0x40)
  {
    uint8 bank=(EXPREGS[0]&5)|((EXPREGS[0]&8)>>2)|((EXPREGS[0]&0x20)>>2);
    if(EXPREGS[0]&2)
      setprg32(0x8000,bank>>1);
    else
    {
      setprg16(0x8000,bank);
      setprg16(0xC000,bank);
    }
  }
  else
    setprg8(A,V&0x3F);
}

static DECLFW(H2288WriteHi)
{
  switch (A&0x8001)
  {
    case 0x8000: MMC3_CMDWrite(0x8000,(V&0xC0)|(m114_perm[V&7])); break;
    case 0x8001: MMC3_CMDWrite(0x8001,V); break;
  }
}

static DECLFW(H2288WriteLo)
{
  if(A&0x800)
  {
    if(A&1)
      EXPREGS[1]=V;
    else
      EXPREGS[0]=V;
    FixMMC3PRG(MMC3_cmd);
  }
}

static DECLFR(H2288Read)
{
  int bit;
  bit=(A&1)^1;
  bit&=((A>>8)&1);
  bit^=1;
  return((X.DB&0xFE)|bit);
}

static void H2288Power(void)
{
  EXPREGS[0]=EXPREGS[1]=0;
  GenMMC3Power();
  SetReadHandler(0x5000,0x5FFF,H2288Read);
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x5000,0x5FFF,H2288WriteLo);
  SetWriteHandler(0x8000,0x8FFF,H2288WriteHi);
}

void UNLH2288_Init(CartInfo *info)
{
  GenMMC3_Init(info, 256, 256, 0, 0);
  pwrap=H2288PW;
  info->Power=H2288Power;
  AddExState(EXPREGS, 2, 0, "EXPR");
}
