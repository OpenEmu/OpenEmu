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

#include "share.h"

static uint32 FTVal,FTValR;
static char side;

static uint8 FT_Read(int w, uint8 ret)
{
 if(w) 
 {
  ret|=FTValR;
 }
 return(ret);
}

static void FT_Write(uint8 V)
{
 FTValR=0;

 //printf("%08x\n",FTVal);
 if(!(V&0x1))
  FTValR=(FTVal>>8);
 else if(!(V&0x2))
  FTValR=(FTVal>>4);
 else if(!(V&0x4))
  FTValR=FTVal;

 FTValR=(~FTValR)&0xF;
 if(side=='B')
  FTValR=((FTValR&0x8)>>3) | ((FTValR&0x4)>>1) | ((FTValR&0x2)<<1) | ((FTValR&0x1)<<3);
 FTValR<<=1;
}

static void FT_Update(void *data)
{
 FTVal=*(uint32 *)data;
}

static int StateActionFC(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
   SFVAR(FTValR),
   SFVAR(FTVal),
   SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "INPF");
 if(load)
 {

 }
 return(ret);
}

static INPUTCFC FamilyTrainer={FT_Read,FT_Write,0,FT_Update,0,0, StateActionFC, 1, sizeof(uint32) };

INPUTCFC *MDFN_InitFamilyTrainerA(void)
{
 side='A';
 FTVal=FTValR=0;
 return(&FamilyTrainer);
}

INPUTCFC *MDFN_InitFamilyTrainerB(void)
{
 side='B';
 FTVal=FTValR=0;
 return(&FamilyTrainer);
}

