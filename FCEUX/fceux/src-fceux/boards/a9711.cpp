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

//static uint8 m_perm[8] = {0, 1, 0, 3, 0, 5, 6, 7};

static void UNLA9711PW(uint32 A, uint8 V)
{
  if((EXPREGS[0]&0xFF) == 0x37)
  {
    setprg8(0x8000, 0x13);
    setprg8(0xA000, 0x13);
    setprg8(0xC000, 0x13);
    setprg8(0xE000, 0x0);
//    uint8 bank=EXPREGS[0]&0x1F;
//   if(EXPREGS[0]&0x20)
//      setprg32(0x8000,bank>>2);
//    else
//    {
//      setprg16(0x8000,bank);
//      setprg16(0xC000,bank);
//    }
  }
  else
    setprg8(A,V&0x3F);
}

//static DECLFW(UNLA9711Write8000)
//{
//  FCEU_printf("bs %04x %02x\n",A,V);
//  if(V&0x80)
//    MMC3_CMDWrite(A,V);
//  else
//    MMC3_CMDWrite(A,m_perm[V&7]);
//  if(V!=0x86) MMC3_CMDWrite(A,V);
//}

static DECLFW(UNLA9711WriteLo)
{
  FCEU_printf("bs %04x %02x\n",A,V);
  EXPREGS[0]=V;
  FixMMC3PRG(MMC3_cmd);
}

static void UNLA9711Power(void)
{
  EXPREGS[0]=EXPREGS[1]=EXPREGS[2]=0;
  GenMMC3Power();
  SetWriteHandler(0x5000,0x5FFF,UNLA9711WriteLo);
//  SetWriteHandler(0x8000,0xbfff,UNLA9711Write8000);
}

void UNLA9711_Init(CartInfo *info)
{
  GenMMC3_Init(info, 256, 256, 0, 0);
  pwrap=UNLA9711PW;
  info->Power=UNLA9711Power;
  AddExState(EXPREGS, 3, 0, "EXPR");
}
