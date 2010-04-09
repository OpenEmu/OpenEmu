/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2003 Xodnizel
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

static uint8 latch[2];

static void Sync(void)
{
	int x;

	setmirror(((latch[0]>>6)&1)^1);
	switch(latch[1]&0x3)
	{
         case 0x0:
	  for(x=0;x<4;x++) 
           setprg8(0x8000+x*8192,(((latch[0]&0x7F)<<1)+x)^(latch[0]>>7));
	  break;
         case 0x2:
	  for(x=0;x<4;x++) 
	   setprg8(0x8000+x*8192,((latch[0]&0x7F)<<1)+(latch[0]>>7));
	  break; 
	 case 0x1:
         case 0x3:
          for(x=0;x<4;x++)
	  {
	   unsigned int b;

	   b=latch[0]&0x7F;
	   if(x>=2 && !(latch[1]&0x2))
	    b=0x7F;
           setprg8(0x8000+x*8192,(x&1)+((b<<1)^(latch[0]>>7)));
	  }
	  break;
	}
}


static DECLFW(Mapper15_write)
{
 latch[0]=V;
 latch[1]=A&3;
 Sync();
}

static void Power(CartInfo *info)
{
 latch[0] = latch[1] = 0;
 Sync();
 setchr8(0);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(latch, 2),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  Sync();
 return(ret);
}

int Mapper15_Init(CartInfo *info)
{
	info->Power = Power;
	info->StateAction = StateAction;
	SetWriteHandler(0x8000,0xFFFF,Mapper15_write);
	SetReadHandler(0x8000, 0xFFFF, CartBR);
	return(1);
}

