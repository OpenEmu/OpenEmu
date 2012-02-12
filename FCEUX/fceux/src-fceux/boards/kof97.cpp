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

static DECLFW(UNLKOF97CMDWrite)
{
  V=(V&0xD8)|((V&0x20)>>4)|((V&4)<<3)|((V&2)>>1)|((V&1)<<2);  //76143502
  if(A==0x9000) A=0x8001;  
  MMC3_CMDWrite(A,V);
}

static DECLFW(UNLKOF97IRQWrite)
{
  V=(V&0xD8)|((V&0x20)>>4)|((V&4)<<3)|((V&2)>>1)|((V&1)<<2);
  if(A==0xD000) A=0xC001;
  else if(A==0xF000) A=0xE001;
  MMC3_IRQWrite(A,V);
}

static void UNLKOF97Power(void)
{
  GenMMC3Power();
  SetWriteHandler(0x8000,0xA000,UNLKOF97CMDWrite);
  SetWriteHandler(0xC000,0xF000,UNLKOF97IRQWrite);
}

void UNLKOF97_Init(CartInfo *info)
{
  GenMMC3_Init(info, 128, 256, 0, 0);
  info->Power=UNLKOF97Power;
}
