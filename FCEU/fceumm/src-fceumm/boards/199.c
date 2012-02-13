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
 *
 * Dragon Ball Z 2 - Gekishin Freeza! (C)
 * Dragon Ball Z Gaiden - Saiya Jin Zetsumetsu Keikaku (C)
 * San Guo Zhi 2 (C)
 *
 */

#include "mapinc.h"
#include "mmc3.h"

static uint8 *CHRRAM=NULL;
static uint32 CHRRAMSIZE;

static void M199PW(uint32 A, uint8 V)
{
  setprg8(A,V);
  setprg8(0xC000,EXPREGS[0]);
  setprg8(0xE000,EXPREGS[1]);
}

static void M199CW(uint32 A, uint8 V)
{
  setchr1r((V<8)?0x10:0x00,A,V);
  setchr1r((DRegBuf[0]<8)?0x10:0x00,0x0000,DRegBuf[0]);
  setchr1r((EXPREGS[2]<8)?0x10:0x00,0x0400,EXPREGS[2]);
  setchr1r((DRegBuf[1]<8)?0x10:0x00,0x0800,DRegBuf[1]);
  setchr1r((EXPREGS[3]<8)?0x10:0x00,0x0c00,EXPREGS[3]);
}

static void M199MW(uint8 V)
{
/*    FCEU_printf("%02x\n",V);*/
  switch(V&3)
  {
    case 0: setmirror(MI_V); break;
    case 1: setmirror(MI_H); break;
    case 2: setmirror(MI_0); break;
    case 3: setmirror(MI_1); break;
  }
}

static DECLFW(M199Write)
{
  if((A==0x8001)&&(MMC3_cmd&8))
  {
    EXPREGS[MMC3_cmd&3]=V;
    FixMMC3PRG(MMC3_cmd);
    FixMMC3CHR(MMC3_cmd);
  }
  else    
    if(A<0xC000)
      MMC3_CMDWrite(A,V);
    else
      MMC3_IRQWrite(A,V);
}

static void M199Power(void)
{
  EXPREGS[0]=~1;
  EXPREGS[1]=~0;
  EXPREGS[2]=1;
  EXPREGS[3]=3;
  GenMMC3Power();
  SetWriteHandler(0x8000,0xFFFF,M199Write);
}

static void M199Close(void)
{
  if(CHRRAM)
    free(CHRRAM);
  CHRRAM=NULL;
}

void Mapper199_Init(CartInfo *info)
{
  GenMMC3_Init(info, 512, 256, 8, info->battery);
  cwrap=M199CW;
  pwrap=M199PW;
  mwrap=M199MW;
  info->Power=M199Power;
  info->Close=M199Close;

  CHRRAMSIZE=8192;
  CHRRAM=(uint8*)FCEU_gmalloc(CHRRAMSIZE);
  SetupCartCHRMapping(0x10, CHRRAM, CHRRAMSIZE, 1);
  AddExState(CHRRAM, CHRRAMSIZE, 0, "CHRR");

  AddExState(EXPREGS, 4, 0, "EXPR");
}
