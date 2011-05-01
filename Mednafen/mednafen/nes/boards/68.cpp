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

static uint8 CHRBanks[4], Mirroring, NTBanks[2], PRGBank16;
static uint8 WRAM[8192];

static void DoCHR(void)
{
 if(!(Mirroring & 0x10))
 {
  const int mir_tab[4] = {MI_V, MI_H, MI_0, MI_1};
  setmirror(mir_tab[Mirroring & 3]);
 }
 else
 {
  uint8 *b[2];
  b[0] = CHRptr[0] + (((NTBanks[0] | 0x80)&CHRmask1[0])<<10);
  b[1] = CHRptr[0] + (((NTBanks[1] | 0x80) & CHRmask1[0]) << 10);

  const int zoom_tab[4][4] = 
  {
   { 0, 1, 0, 1 },
   { 0, 0, 1, 1 },
   { 0, 0, 0, 0 },
   { 1, 1, 1, 1 },
  };
  for(int x = 0; x < 4; x++)
   setntamem(b[zoom_tab[Mirroring & 3][x]], 0, x);
 }
 for(int x = 0; x < 4; x++)
  setchr2(x << 11, CHRBanks[x]);
}

static void DoPRG(void)
{
 setprg16(0x8000, PRGBank16);
}

static DECLFW(Mapper68_write)
{
 A&=0xF000;

 if(A>=0x8000 && A<=0xB000)
 {
  CHRBanks[(A - 0x8000) >> 12] = V;
  DoCHR();
 }
 else switch(A)
 {
  case 0xc000:NTBanks[0] = V;
	      DoCHR();
              break;

  case 0xd000:NTBanks[1] = V;
	      DoCHR();
              break;

  case 0xe000: Mirroring = V;
	       DoCHR();
               break;
  case 0xf000: PRGBank16 = V; DoPRG();break;
 }
}

static int StateAction(StateMem *sm, int load, int data_only)
{
	SFORMAT StateRegs[] =
	{
	 SFVAR(Mirroring), SFARRAY(CHRBanks, 4), SFVAR(PRGBank16), SFARRAY(NTBanks, 2),
	 SFARRAY(WRAM, 8192),
	 SFEND
	};

	int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
	if(load)
	{
	 DoPRG();
	 DoCHR();
	}

	return(ret);
}

static void Power(CartInfo *info)
{
	int x;
	for(x = 0; x < 4; x++)
	 CHRBanks[x] = x;
	PRGBank16 = 0;
	Mirroring = 0;
	for(x = 0; x < 2; x++)
	 NTBanks[x] = 0;
	DoPRG();
	DoCHR();
	setprg16(0xc000, ~0);

	if(!info->battery)
	 memset(WRAM, 0, 8192);

	setprg16r(0x10, 0x6000, 0);
}

int Mapper68_Init(CartInfo *info)
{
 info->Power = Power;
 info->StateAction = StateAction;
 SetWriteHandler(0x8000,0xffff,Mapper68_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);

 // external WRAM is apparently used by Nantettatte Baseball
 SetReadHandler(0x6000, 0x7FFF, CartBR);
 SetWriteHandler(0x6000, 0x7FFF, CartBW);
 SetupCartPRGMapping(0x10, WRAM, 8192, 1);

 if(info->battery)
 {
  info->SaveGame[0] = WRAM;
  info->SaveGameLen[0] = 8192;
 }
 return(1);
}
