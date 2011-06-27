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

static uint8 latche;

static DECLFW(CPROMWrite)
{
 latche=V&3;
 setchr4(0x1000, V & 3);
}

static void CPROMReset(CartInfo *info)
{
 latche = 0;
 setprg32(0x8000,0);
 setchr4(0x0000, 0);
 setchr4(0x1000, 0);
}

static int CPROM_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(latche),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  setchr4(0x1000, latche & 3);
 return(ret);
}

int CPROM_Init(CartInfo *info)
{
 info->Power=CPROMReset;
 info->StateAction = CPROM_StateAction;

 SetReadHandler(0x8000,0xFFFF,CartBR);
 SetWriteHandler(0x8000,0xffff,CPROMWrite);

 return(1);
}

static int CNROM_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(latche),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  setchr8(latche);
 return(ret);
}

static DECLFW(CNROMWrite)
{
 latche = V & 3 & CartBR(A);
 setchr8(latche);
}

static void CNROMReset(CartInfo *info)
{
 latche = 0;
 setprg16(0x8000,0);
 setprg16(0xC000,1);
 setchr8(0);
}

int CNROM_Init(CartInfo *info)
{
 info->Power=CNROMReset;
 info->StateAction = CNROM_StateAction;

 SetReadHandler(0x8000,0xFFFF,CartBR);
 SetWriteHandler(0x8000,0xffff,CNROMWrite);

 return(1);
}

static void NROM128Reset(CartInfo *info)
{
  setprg16(0x8000,0);
  setprg16(0xC000,0);
  setchr8(0);
}

static void NROM256Reset(CartInfo *info)
{
  setprg16(0x8000,0);
  setprg16(0xC000,1);
  setchr8(0);
}

int NROM128_Init(CartInfo *info)
{
 info->Power=NROM128Reset;
 SetReadHandler(0x8000,0xFFFF,CartBR);

 return(1);
}

int NROM256_Init(CartInfo *info)
{
 info->Power=NROM256Reset;
 SetReadHandler(0x8000,0xFFFF,CartBR);

 return(1);
}

static int MHROM_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(latche),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
 {
  setprg32(0x8000, latche >> 4),
  setchr8(latche & 0xF);
 }
 return(ret);
}

static DECLFW(MHROMWrite)
{
 setprg32(0x8000,V>>4);
 setchr8(V & 0xF);
 latche=V;
}

static void MHROMReset(CartInfo *info)
{
 setprg32(0x8000,0);
 setchr8(0);
 latche=0;
}

int MHROM_Init(CartInfo *info)
{ 
 info->Power=MHROMReset;
 info->StateAction = MHROM_StateAction;
 PRGmask32[0]&=1;
 CHRmask8[0]&=1;
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 SetWriteHandler(0x8000, 0xFFFF, MHROMWrite);
 return(1);
}

#ifdef WANT_DEBUGGER
static RegType DBGUNROMRegs[] =
{
 { 0, "PRGBank", "PRG Bank, 16KiB @8000", 1 },
 { 0, "", "", 0 },
};

static uint32 UNROMDBG_GetRegister(const unsigned int id, char *special, const uint32 special_len)
{
 return(latche);
}

static void UNROMDBG_SetRegister(const unsigned int id, uint32 value)
{
 latche = value;
 setprg16(0x8000, latche);
}

static RegGroupType DBGUNROMRegsGroup =
{
 "UNROM",
 DBGUNROMRegs,
 UNROMDBG_GetRegister,
 UNROMDBG_SetRegister,
};

#endif

static int UNROM_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(latche),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  setprg16(0x8000, latche);
 return(ret);
}

static DECLFW(UNROMWrite)
{
 setprg16(0x8000,V);
 latche=V;
}

static void UNROMReset(CartInfo *info)
{
 setchr8(0);
 setprg16(0x8000,0);
 setprg16(0xc000,~0);
 latche=0;
}

int UNROM_Init(CartInfo *info)
{
 info->Power=UNROMReset;
 info->StateAction = UNROM_StateAction;

 #ifdef WANT_DEBUGGER
 MDFNDBG_AddRegGroup(&DBGUNROMRegsGroup);
 #endif

 SetWriteHandler(0x8000,0xffff,UNROMWrite);
 SetReadHandler(0x8000,0xFFFF,CartBR);

 //PRGmask16[0]&=7;
 return(1);
}

static void GNROMSync()
{
 setchr8(latche&3);
 setprg32(0x8000,(latche>>4)&3);
}

static DECLFW(GNROMWrite)
{
 latche=V&0x33;
 GNROMSync();
}

static int GNROM_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(latche),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
  GNROMSync();
 return(ret);
}

static void GNROMReset(CartInfo *info)
{
 latche=0;
 GNROMSync(); 
}

int GNROM_Init(CartInfo *info)
{
 info->Power=GNROMReset;
 info->StateAction = GNROM_StateAction;

 SetWriteHandler(0x8000,0xffff,GNROMWrite);
 SetReadHandler(0x8000,0xFFFF,CartBR);

 return(1);
}

static DECLFW(AOROM_Write)
{
 latche = V;
 setprg32(0x8000, V & 0xF);
 setmirror(((V >> 4) & 1) ? MI_1 : MI_0);
}

static void AOROM_Reset(CartInfo *info)
{
 setprg32(0x8000, 0xF);
 setchr8(0);
 setmirror(MI_0);
 latche = 0;
}

static int AOROM_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(latche),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
 {
  setprg32(0x8000, latche & 0xF);
  setmirror(((latche >> 4) & 1) ? MI_1 : MI_0);
 }
 return(ret);
}


int AOROM_Init(CartInfo *info)
{
 info->Power = AOROM_Reset;
 info->StateAction = AOROM_StateAction;
 SetWriteHandler(0x8000, 0xFFFF, AOROM_Write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 return(1);
}
