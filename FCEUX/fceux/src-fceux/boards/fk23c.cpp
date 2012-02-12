/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2006 CaH4e3
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

static uint8 unromchr;
static uint32 dipswitch;

static void BMCFK23CCW(uint32 A, uint8 V)
{
  if(EXPREGS[0]&0x40)
    setchr8(EXPREGS[2]|unromchr);
  else
  {
    uint16 base=(EXPREGS[2]&0x7F)<<3;
    setchr1(A,V|base);
    if(EXPREGS[3]&2)
    {
      setchr1(0x0400,EXPREGS[6]|base);
      setchr1(0x0C00,EXPREGS[7]|base);
    }
  }
}

static void BMCFK23CPW(uint32 A, uint8 V)
{
  if((EXPREGS[0]&7)==4)
    setprg32(0x8000,EXPREGS[1]>>1);
  else if ((EXPREGS[0]&7)==3)
  {
    setprg16(0x8000,EXPREGS[1]);
    setprg16(0xC000,EXPREGS[1]);
  }  
  else
  { 
    if(EXPREGS[0]&3)
      setprg8(A,(V&(0x3F>>(EXPREGS[0]&3)))|(EXPREGS[1]<<1));
    else
      setprg8(A,V);
    if(EXPREGS[3]&2)
    {
      setprg8(0xC000,EXPREGS[4]);
      setprg8(0xE000,EXPREGS[5]);
    }
  }
}

static DECLFW(BMCFK23CHiWrite)
{
  if(EXPREGS[0]&0x40)
  {
    if(EXPREGS[0]&0x30)
      unromchr=0;
    else
    {
      unromchr=V&3;
      FixMMC3CHR(MMC3_cmd);
    }
  }
  else
  {
    if((A==0x8001)&&(EXPREGS[3]&2&&MMC3_cmd&8))
    {
      EXPREGS[4|(MMC3_cmd&3)]=V;
      FixMMC3PRG(MMC3_cmd);
      FixMMC3CHR(MMC3_cmd);
    }
    else    
      if(A<0xC000)
        MMC3_CMDWrite(A,V);
      else
        MMC3_IRQWrite(A,V);
  }
}

static DECLFW(BMCFK23CWrite)
{
  if(A&(1<<(dipswitch+4)))
  {
    EXPREGS[A&3]=V;
    FixMMC3PRG(MMC3_cmd);
    FixMMC3CHR(MMC3_cmd);
  }
}

static void BMCFK23CReset(void)
{
  dipswitch++;
  dipswitch&=7;
  EXPREGS[0]=EXPREGS[1]=EXPREGS[2]=EXPREGS[3]=0;
  EXPREGS[4]=EXPREGS[5]=EXPREGS[6]=EXPREGS[7]=0xFF;
  MMC3RegReset();
  FixMMC3PRG(MMC3_cmd);
  FixMMC3CHR(MMC3_cmd);
}

static void BMCFK23CPower(void)
{
  GenMMC3Power();
  EXPREGS[0]=EXPREGS[1]=EXPREGS[2]=EXPREGS[3]=0;
  EXPREGS[4]=EXPREGS[5]=EXPREGS[6]=EXPREGS[7]=0xFF;
  GenMMC3Power();
  SetWriteHandler(0x5000,0x5fff,BMCFK23CWrite);
  SetWriteHandler(0x8000,0xFFFF,BMCFK23CHiWrite);
  FixMMC3PRG(MMC3_cmd);
  FixMMC3CHR(MMC3_cmd);
}

void BMCFK23C_Init(CartInfo *info)
{
  GenMMC3_Init(info, 512, 256, 8, 0);
  cwrap=BMCFK23CCW;
  pwrap=BMCFK23CPW;
  info->Power=BMCFK23CPower;
  info->Reset=BMCFK23CReset;
  AddExState(EXPREGS, 8, 0, "EXPR");
  AddExState(&unromchr, 1, 0, "UNCHR");
  AddExState(&dipswitch, 1, 0, "DIPSW");
}
