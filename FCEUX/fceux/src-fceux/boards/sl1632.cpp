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

// brk is a system call in *nix, and is an illegal variable name - soules
static uint8 chrcmd[8], prg0, prg1, bbrk, mirr, swap;
static SFORMAT StateRegs[]=
{
  {chrcmd, 8, "CHRCMD"},
  {&prg0, 1, "PRG0"},
  {&prg1, 1, "PRG1"},
  {&bbrk, 1, "BRK"},
  {&mirr, 1, "MIRR"},
  {&swap, 1, "SWAP"},
  {0}
};

static void Sync(void)
{
  int i;
  setprg8(0x8000,prg0);
  setprg8(0xA000,prg1);
  setprg8(0xC000,~1);
  setprg8(0xE000,~0);
  for(i=0; i<8; i++)
     setchr1(i<<10,chrcmd[i]);
  setmirror(mirr^1);
}

static void UNLSL1632CW(uint32 A, uint8 V)
{
  int cbase=(MMC3_cmd&0x80)<<5;
  int page0=(bbrk&0x08)<<5;
  int page1=(bbrk&0x20)<<3;
  int page2=(bbrk&0x80)<<1;
  setchr1(cbase^0x0000,page0|(DRegBuf[0]&(~1)));
  setchr1(cbase^0x0400,page0|DRegBuf[0]|1);
  setchr1(cbase^0x0800,page0|(DRegBuf[1]&(~1)));
  setchr1(cbase^0x0C00,page0|DRegBuf[1]|1);
  setchr1(cbase^0x1000,page1|DRegBuf[2]);
  setchr1(cbase^0x1400,page1|DRegBuf[3]);
  setchr1(cbase^0x1800,page2|DRegBuf[4]);
  setchr1(cbase^0x1c00,page2|DRegBuf[5]);
}

static DECLFW(UNLSL1632CMDWrite)
{
  if(A==0xA131)
  {
    bbrk=V;
  }
  if(bbrk&2)
  {
    FixMMC3PRG(MMC3_cmd);
    FixMMC3CHR(MMC3_cmd);
    if(A<0xC000)
      MMC3_CMDWrite(A,V);
    else
      MMC3_IRQWrite(A,V);
  }
  else
  {
    if((A>=0xB000)&&(A<=0xE003))
    {
      int ind=((((A&2)|(A>>10))>>1)+2)&7;
      int sar=((A&1)<<2);
      chrcmd[ind]=(chrcmd[ind]&(0xF0>>sar))|((V&0x0F)<<sar);
    }
    else
      switch(A&0xF003)
      {
        case 0x8000: prg0=V; break;
        case 0xA000: prg1=V; break;
        case 0x9000: mirr=V&1; break;
      }
    Sync();
  }
}

static void StateRestore(int version)
{
  if(bbrk&2)
  {
    FixMMC3PRG(MMC3_cmd);
    FixMMC3CHR(MMC3_cmd);
  }
  else
    Sync();
}

static void UNLSL1632Power(void)
{
  GenMMC3Power();
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x4100,0xFFFF,UNLSL1632CMDWrite);
}

void UNLSL1632_Init(CartInfo *info)
{
  GenMMC3_Init(info, 256, 512, 0, 0);
  cwrap=UNLSL1632CW;
  info->Power=UNLSL1632Power;
  GameStateRestore=StateRestore;
  AddExState(&StateRegs, ~0, 0, 0);
}
