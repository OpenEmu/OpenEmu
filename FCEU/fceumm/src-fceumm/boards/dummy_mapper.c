/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2009 CaH4e3
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

static uint8 reg[8];
/*
static uint8 *WRAM=NULL;
static uint32 WRAMSIZE;
static uint8 *CHRRAM=NULL;
static uint32 CHRRAMSIZE;
*/

static SFORMAT StateRegs[]=
{
  {reg, 8, "REGS"},
  {0}
};

static void Sync(void)
{
}

static DECLFW(MNNNWrite)
{
}

static void MNNNPower(void)
{
/*  SetReadHandler(0x6000,0x7fff,CartBR);*/
/* SetWriteHandler(0x6000,0x7fff,CartBW);*/
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xFFFF,MNNNWrite);
}

static void MNNNReset(void)
{
}

/*
static void MNNNClose(void)
{
  if(WRAM)
    free(WRAM);
  if(CHRRAM)
    free(CHRRAM);
  WRAM=CHRRAM=NULL;
}
*/

static void MNNNIRQHook(void)
{
  X6502_IRQBegin(FCEU_IQEXT);
}

static void StateRestore(int version)
{
  Sync();
}

void MapperNNN_Init(CartInfo *info)
{
	info->Reset=MNNNReset;
	info->Power=MNNNPower;
	/*  info->Close=MNNNClose;*/
	GameHBIRQHook=MNNNIRQHook;
	GameStateRestore=StateRestore;
	/*
	   CHRRAMSIZE=8192;
	   CHRRAM=(uint8*)FCEU_gmalloc(CHRRAMSIZE);
	   SetupCartCHRMapping(0x10,CHRRAM,CHRRAMSIZE,1);
	   AddExState(CHRRAM, CHRRAMSIZE, 0, "CRAM");
	 */
	/*
	   WRAMSIZE=8192;
	   WRAM=(uint8*)FCEU_gmalloc(WRAMSIZE);
	   SetupCartPRGMapping(0x10,WRAM,WRAMSIZE,1);
	   AddExState(WRAM, WRAMSIZE, 0, "WRAM");
	   if(info->battery)
	   {
	   info->SaveGame[0]=WRAM;
	   info->SaveGameLen[0]=WRAMSIZE;
	   }
	 */
	AddExState(&StateRegs, ~0, 0, 0);
}
