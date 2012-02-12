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

static uint8 cmdin;
static uint8 UNL8237_perm[8] = {0, 2, 6, 1, 7, 3, 4, 5};

static void UNL8237CW(uint32 A, uint8 V)
{
  setchr1(A,((EXPREGS[1]&4)<<6)|V);
}

static void UNL8237PW(uint32 A, uint8 V)
{
  if(EXPREGS[0]&0x80)
  {
    if(EXPREGS[0]&0x20)
      setprg32(0x8000,(EXPREGS[0]&0xF)>>1);
    else
    {
      setprg16(0x8000,(EXPREGS[0]&0x1F));
      setprg16(0xC000,(EXPREGS[0]&0x1F));
    }
  }
  else
    setprg8(A,V&0x3F);
}

static DECLFW(UNL8237Write)
{
  if((A&0xF000)==0xF000)
    IRQCount=V;
  else if((A&0xF000)==0xE000)
    X6502_IRQEnd(FCEU_IQEXT);
  else switch(A&0xE001)
  {
    case 0x8000: setmirror(((V|(V>>7))&1)^1); break;
    case 0xA000: MMC3_CMDWrite(0x8000,(V&0xC0)|(UNL8237_perm[V&7])); cmdin=1; break;
    case 0xC000: if(cmdin)
                 {
                   MMC3_CMDWrite(0x8001,V);
                   cmdin=0;
                 }
                 break;
  }
}

static DECLFW(UNL8237ExWrite)
{
  switch(A)
  {
    case 0x5000: EXPREGS[0]=V; FixMMC3PRG(MMC3_cmd); break;
    case 0x5001: EXPREGS[1]=V; FixMMC3CHR(MMC3_cmd); break;
  }
}

static void UNL8237Power(void)
{
  IRQa=1;
  EXPREGS[0]=EXPREGS[1]=0;
  GenMMC3Power();
  SetWriteHandler(0x8000,0xFFFF,UNL8237Write);
  SetWriteHandler(0x5000,0x7FFF,UNL8237ExWrite);
}

void UNL8237_Init(CartInfo *info)
{
  GenMMC3_Init(info, 256, 256, 0, 0);
  cwrap=UNL8237CW;
  pwrap=UNL8237PW;
  info->Power=UNL8237Power;
  AddExState(EXPREGS, 3, 0, "EXPR");
  AddExState(&cmdin, 1, 0, "CMDIN");
}
