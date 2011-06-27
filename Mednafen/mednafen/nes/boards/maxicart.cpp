/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 1998 BERO
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

typedef struct 
{
 uint8 PRGSelect;
 uint8 RAMSelect;
} M125Data;

static M125Data Mapper125;
static uint8 *WRAM = NULL;

static DECLFW(M125w)
{
 if(A==0x5000) 
 {
  Mapper125.PRGSelect=V&0xF;
  setprg32(0x8000,V&0xF);
 }
 else if(A==0x5001) 
 {
  Mapper125.RAMSelect=V&0x33;
  setchr8(V&3);
  setprg8r(0x10,0x6000,(V>>4)&3);
 }
}

static void M125_Reset(CartInfo *info)
{

}

static void M125_Power(CartInfo *info)
{
 Mapper125.PRGSelect = ~0;
 Mapper125.RAMSelect = 0;

 setprg32(0x8000,Mapper125.PRGSelect&0xF);
 setchr8(Mapper125.RAMSelect&3);
 setprg8r(0x10,0x6000,(Mapper125.RAMSelect>>4)&3);

 if(!info->battery)
 {
  memset(WRAM, 0xFF, 32768);
 }
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(Mapper125.RAMSelect), SFVAR(Mapper125.PRGSelect),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");

 if(load)
 {
  setprg32(0x8000,Mapper125.PRGSelect&0xF);
  setchr8(Mapper125.RAMSelect&3);
  setprg8r(0x10,0x6000,(Mapper125.RAMSelect>>4)&3);
 }
 return(ret);
}

static void Close(void)
{
 if(WRAM)
 {
  free(WRAM);
  WRAM = NULL;
 }
}

int Mapper125_Init(CartInfo *info)
{
 if(!(WRAM = (uint8*) MDFN_malloc(32768, _("expansion RAM"))))
 {
  return(0);
 }

 SetupCartPRGMapping(0x10, WRAM, 32768, 1);
 info->Reset=M125_Reset;
 info->Power=M125_Power;
 info->StateAction = StateAction;
 info->Close = Close;

 if(info->battery)
 {
  info->SaveGame[0] = WRAM;
  info->SaveGameLen[0] = 32768;
 }

 SetReadHandler(0x6000,0xFFFF,CartBR);
 SetWriteHandler(0x5000,0x5001,M125w);
 SetWriteHandler(0x6000,0x7FFF,CartBW);

 return(1);
}
