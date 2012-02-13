/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2006 CaH4e3
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

static uint8 *CHRRAM;
static uint8 tekker;

static void MSHCW(uint32 A, uint8 V)
{
  if(EXPREGS[0]&0x40)
    setchr8r(0x10,0);
  else
  {
    if(A<0x800)
      setchr1(A,V|((EXPREGS[0]&8)<<5));
    else if(A<0x1000)
      setchr1(A,V|((EXPREGS[0]&4)<<6));
    else if(A<0x1800)
      setchr1(A,V|((EXPREGS[0]&1)<<8));
    else
      setchr1(A,V|((EXPREGS[0]&2)<<7));
  }
}

static DECLFW(MSHWrite)
{
  EXPREGS[0]=V;
  FixMMC3CHR(MMC3_cmd);
}

static DECLFR(MSHRead)
{
  return(tekker);
}

static void MSHReset(void)
{
  MMC3RegReset();
  tekker^=0xFF; 
}

static void MSHPower(void)
{
  tekker=0x00;
  GenMMC3Power();
  SetWriteHandler(0x4100,0x4100,MSHWrite);
  SetReadHandler(0x4100,0x4100,MSHRead);
}

static void MSHClose(void)
{
  if(CHRRAM)
    free(CHRRAM);
  CHRRAM=NULL;
}

void UNLSHeroes_Init(CartInfo *info)
{
  GenMMC3_Init(info, 256, 512, 0, 0);
  cwrap=MSHCW;
  info->Power=MSHPower;
  info->Reset=MSHReset;
  info->Close=MSHClose;
  CHRRAM = (uint8*)FCEU_gmalloc(8192);
  SetupCartCHRMapping(0x10, CHRRAM, 8192, 1);
  AddExState(EXPREGS, 4, 0, "EXPR");
  AddExState(&tekker, 1, 0, "DIPSW");
}
