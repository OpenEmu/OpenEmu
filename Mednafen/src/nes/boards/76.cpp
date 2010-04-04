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

static uint8 cmd, CHRBanks[4], PRGBanks[2], Mirroring;

static void DoCHR(void)
{
 for(int x = 0; x < 4; x++)
  setchr2(x * 2048, CHRBanks[x]);
}

static void DoPRG(void)
{
 if(cmd & 0x40)
 {
  setprg8(0xc000, PRGBanks[0]);
  setprg8(0x8000, 0xFE);
 }
 else
 {
  setprg8(0x8000, PRGBanks[0]);
  setprg8(0xc000, 0xFE);
 }
 setprg8(0xa000, PRGBanks[1]);
}

static void DoMirroring(void)
{
 setmirror((Mirroring & 1) ? MI_H : MI_V);
}

static DECLFW(Mapper76_write)
{
        switch(A&0xE001)
	{
         case 0x8000: cmd = V; DoPRG(); break;
         case 0x8001:
                switch(cmd&0x07)
		{
		 case 2: CHRBanks[0] = V; DoCHR(); break;
		 case 3: CHRBanks[1] = V; DoCHR(); break;
		 case 4: CHRBanks[2] = V; DoCHR(); break;
		 case 5: CHRBanks[3] = V; DoCHR(); break;
		 case 6: PRGBanks[0] = V; DoPRG(); break;
		 case 7: PRGBanks[1] = V; DoPRG(); break;
               }
               break;
         case 0xA000: Mirroring = V & 1; DoMirroring(); break;
	}
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(cmd),
  SFVAR(Mirroring),
  SFARRAY(CHRBanks, 4),
  SFARRAY(PRGBanks, 2),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
 {
  DoPRG();
  DoCHR();
  DoMirroring();
 }
 return(ret);
}

static void Power(CartInfo *info)
{
 int x;

 for(x = 0; x < 4; x++)
  CHRBanks[x] = x;

 PRGBanks[0] = 0;
 PRGBanks[1] = 1;
 Mirroring = 0;
 cmd = 0;
 DoPRG();
 DoCHR();
 DoMirroring();
 setprg16(0xc000, 0x7F);
}


int Mapper76_Init(CartInfo *info)
{
 SetWriteHandler(0x8000,0xffff,Mapper76_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 info->Power = Power;
 info->StateAction = StateAction;
 return(1);
}

