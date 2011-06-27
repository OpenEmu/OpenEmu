/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
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

static uint8 PRGBanks[2], CHRBanks[8], Mirroring;

static void DoPRG(void)
{
 setprg8(0x8000, PRGBanks[0]);
 setprg8(0xa000, PRGBanks[1]);
}

static void DoCHR(void)
{
 int x;
 for(x = 0; x < 8; x++)
  setchr1(x * 1024, CHRBanks[x] >> 1);
}

static void DoMirroring(void)
{
 switch(Mirroring & 3)
 {
  case 0: setmirror(MI_V); break;
  case 1: setmirror(MI_H); break;
  case 2: setmirror(MI_0); break;
  case 3: setmirror(MI_1); break;
 }
}

static DECLFW(Mapper22_write)
{
        if(A<=0xAFFF)
         {
          switch(A&0xF000)
          {
           case 0x8000:PRGBanks[0] = V; DoPRG(); break;
           case 0xa000:PRGBanks[1] = V; DoPRG(); break;
           case 0x9000:Mirroring = V & 3; DoMirroring(); break;
          }
         }
        else
        {
	  A&=0xF003;
	  if(A>=0xb000 && A<=0xe003)
	  {
	   int x=(A&1)|((A-0xB000)>>11);

	   CHRBanks[x]&=(0xF0)>>((A&2)<<1);
	   CHRBanks[x]|=(V&0xF)<<((A&2)<<1);
	   DoCHR();
	  }
        }
}

static void Power(CartInfo *info)
{
 int x;

 Mirroring = 3; 

 for(x = 0; x < 8; x++)
  CHRBanks[x] = ~0;
 DoCHR();
 PRGBanks[0] = PRGBanks[1] = ~0;
 DoPRG();

 setprg16(0xc000, 0x7F);
}

int Mapper22_Init(CartInfo *info)
{
	info->Power = Power;
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	SetWriteHandler(0x8000,0xffff,Mapper22_write);
	return(1);
}
