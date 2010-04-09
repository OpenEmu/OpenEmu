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

static char Mirroring;
static uint8 RegSelect;
static uint8 Regs[8];

static uint8 *WRAM = NULL;

static void SyncPRGCHR(void)
{
 setprg8(0x8000, Regs[0]);
 setprg8(0xa000, Regs[1]);
 setchr2(0x0000, Regs[2] >> 1);
 setchr2(0x0800, Regs[3] >> 1);
 setchr1(0x1000, Regs[4]);
 setchr1(0x1400, Regs[5]);
 setchr1(0x1800, Regs[6]);
 setchr1(0x1C00, Regs[7]);
}

static DECLFW(Mapper112_write)
{
 switch(A)
 {
 case 0xe000:Mirroring = V & 1; setmirror((V&1) ^ 1); break;
 case 0x8000:RegSelect = V & 0x7; break;
 case 0xa000:Regs[RegSelect & 7] = V; 
	     SyncPRGCHR(); 
	     break;
 }
}

static void Power(CartInfo *info)
{
 int x;
 for(x = 0; x < 8; x++)
  Regs[x] = ~0;
 Mirroring = 1;
 setmirror(Mirroring ^ 1);
 SyncPRGCHR();
 setprg16(0xc000, 0x7F);
 setprg8r(0x10, 0x6000, 0);

 if(!info->battery)
  memset(WRAM, 0xFF, 8192);
}

static void Close(void)
{
 if(WRAM)
 {
  free(WRAM);
  WRAM = NULL;
 }
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(RegSelect),
  SFVAR(Mirroring),
  SFARRAY(Regs, 8),
  SFARRAY(WRAM, 8192),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "M112");
 if(load)
 {
  Mirroring &= 1;
  SyncPRGCHR();
  setmirror(Mirroring ^ 1);
 }

 return(ret);
}

int Mapper112_Init(CartInfo *info)
{
 SetWriteHandler(0x8000,0xffff,Mapper112_write);
 SetReadHandler(0x6000, 0xFFFF, CartBR);
 SetWriteHandler(0x6000, 0x7FFF, CartBW);
 info->Power = Power;
 info->Close = Close;
 info->StateAction = StateAction;

 if(!(WRAM = (uint8 *)malloc(8192)))
  return(0);

 SetupCartPRGMapping(0x10, WRAM, 8192, 1);

 if(info->battery)
 {
  info->SaveGame[0] = WRAM;
  info->SaveGameLen[0] = 8192;
 }
 return(1);
}

