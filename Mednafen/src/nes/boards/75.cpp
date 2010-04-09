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


static uint8 PRGBanks[3], map75ar[2], map75sel;

static void DoPRG(void)
{
 setprg8(0x8000, PRGBanks[0]);
 setprg8(0xa000, PRGBanks[1]);
 setprg8(0xc000, PRGBanks[2]);
}

static void DoCHR(void)
{
 setchr4(0x0000, ((map75sel & 2) << 3) | map75ar[0]);
 setchr4(0x1000, ((map75sel & 4) << 2) | map75ar[1]);
}

static void DoMirroring(void)
{
 setmirror((map75sel & 1) ? MI_H : MI_V);
}

static DECLFW(Mapper75_write)
{
 switch(A&0xF000)
 {
  case 0x8000:PRGBanks[0] = V; DoPRG(); break;
  case 0x9000:map75sel = V; DoCHR(); DoMirroring(); map75sel=V; break;
  case 0xa000:PRGBanks[1] = V; DoPRG();break;
  case 0xc000:PRGBanks[2] = V; DoPRG();break;
  case 0xe000:V&=0xF;map75ar[0]=V;DoCHR();break;
  case 0xf000:V&=0xF;map75ar[1]=V;DoCHR();break;
 }
}
static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(PRGBanks, 3),
  SFARRAY(map75ar, 2),
  SFVAR(map75sel),
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
 PRGBanks[0] = 0;
 PRGBanks[1] = 1;
 PRGBanks[2] = 0xFE;
 map75ar[0] = 0;
 map75ar[1] = 1;
 map75sel = 0;
 DoPRG();
 DoCHR();
 DoMirroring();

 setprg8(0xe000, 0xFF);
}

int Mapper75_Init(CartInfo *info)
{
 SetWriteHandler(0x8000,0xffff,Mapper75_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 info->Power = Power;
 info->StateAction = StateAction;

 return(1);
}

