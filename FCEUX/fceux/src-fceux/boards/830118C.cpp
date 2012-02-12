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

// M-022 MMC3 based 830118C T-106 4M + 4M

#include "mapinc.h"
#include "mmc3.h"

static uint8 reset_flag = 0;

static void BMC830118CCW(uint32 A, uint8 V)
{
    setchr1(A,(V&0x7F)|((EXPREGS[0]&0x0c)<<5));
}

static void BMC830118CPW(uint32 A, uint8 V)
{
      if((EXPREGS[0]&0x0C)==0x0C) 
      {  
         if(A==0x8000)
         {
           setprg8(A,(V&0x0F)|((EXPREGS[0]&0x0c)<<2));
           setprg8(0xC000,(V&0x0F)|0x32);
         }
         else if(A==0xA000)
         {
           setprg8(A,(V&0x0F)|((EXPREGS[0]&0x0c)<<2));
           setprg8(0xE000,(V&0x0F)|0x32);
         }
      }
      else
      {
         setprg8(A,(V&0x0F)|((EXPREGS[0]&0x0c)<<2));
      }
}

static DECLFW(BMC830118CLoWrite)
{
  EXPREGS[0] = V;
  FixMMC3PRG(MMC3_cmd);
  FixMMC3CHR(MMC3_cmd);
}

static void BMC830118CReset(void)
{
  EXPREGS[0]=0;
  MMC3RegReset();
}

static void BMC830118CPower(void)
{
  EXPREGS[0] = 0;
  GenMMC3Power();
  SetWriteHandler(0x6800,0x68FF,BMC830118CLoWrite);
}

void BMC830118C_Init(CartInfo *info)
{
  GenMMC3_Init(info, 128, 128, 8, 0);
  pwrap=BMC830118CPW;
  cwrap=BMC830118CCW;
  info->Power=BMC830118CPower;
  info->Reset=BMC830118CReset;
  AddExState(EXPREGS, 1, 0, "EXPR");
}
