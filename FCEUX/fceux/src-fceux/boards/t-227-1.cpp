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

// T-227-1, 820632, MMC3 based, multimenu, 60000in1 (0010) dip switches

#include "mapinc.h"
#include "mmc3.h"

static uint8 reset_flag = 0x07;

static void BMCT2271CW(uint32 A, uint8 V)
{
    uint32 va = V; 
    if(EXPREGS[0]&0x20)
    {
      va|=0x200;
      va|=(EXPREGS[0]&0x10)<<4;
    }
    else
    {
      va&=0x7F;
      va|=(EXPREGS[0]&0x18)<<4;
    }
    setchr1(A,va);
}

static void BMCT2271PW(uint32 A, uint8 V)
{
    uint32 va = V & 0x3F; 
    if(EXPREGS[0]&0x20)
    {
       va&=0x1F;
       va|=0x40;
       va|=(EXPREGS[0]&0x10)<<1;
    }
    else
    {
       va&=0x0F;
       va|=(EXPREGS[0]&0x18)<<1;
    }
    switch(EXPREGS[0]&3)
    {
      case 0x00: setprg8(A,va); break;
      case 0x02: 
           {
             va=(va&0xFD)|((EXPREGS[0]&4)>>1);
             if(A<0xC000)
             {
               setprg16(0x8000,va >> 1); 
               setprg16(0xC000,va >> 1); 
             }
             break;
           }
      case 0x01: 
      case 0x03: if(A<0xC000) setprg32(0x8000,va >> 2); break;
    }

}
       
static DECLFW(BMCT2271LoWrite)
{
  if(!(EXPREGS[0]&0x80))
    EXPREGS[0] = A & 0xFF;
  FixMMC3PRG(MMC3_cmd);
  FixMMC3CHR(MMC3_cmd);
}

static DECLFR(BMCT2271HiRead)
{
  uint32 av = A;
  if(EXPREGS[0]&0x40) av = (av & 0xFFF0)|reset_flag;
  return CartBR(av);
}

static void BMCT2271Reset(void)
{
  EXPREGS[0] = 0x00;
  reset_flag++;
  reset_flag&=0x0F;
  MMC3RegReset();
}

static void BMCT2271Power(void)
{
  EXPREGS[0] = 0x00;
  GenMMC3Power();
  SetWriteHandler(0x6000,0x7FFF,BMCT2271LoWrite);
  SetReadHandler(0x8000,0xFFFF,BMCT2271HiRead);
}

void BMCT2271_Init(CartInfo *info)
{
  GenMMC3_Init(info, 128, 128, 8, 0);
  pwrap=BMCT2271PW;
  cwrap=BMCT2271CW;
  info->Power=BMCT2271Power;
  info->Reset=BMCT2271Reset;
  AddExState(EXPREGS, 1, 0, "EXPR");
}
