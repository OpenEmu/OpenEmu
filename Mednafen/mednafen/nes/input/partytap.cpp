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

static uint8 QZVal,QZValR;
static uint8 FunkyMode;

static uint8 QZ_Read(int w, uint8 ret)
{
 if(w) 
 {
  //if(X.PC==0xdc7d) return(0xFF);
  //printf("Blah: %04x\n",X.PC);
  //MDFNI_DumpMem("dmp2",0xc000,0xffff);

  ret|=(QZValR&0x7)<<2;
  QZValR=QZValR>>3;

  if(FunkyMode)
  {
   //ret=0x14;
   //puts("Funky");
   QZValR|=0x28;
  }
  else
  {
   QZValR|=0x38;
  }
 }
 return(ret);
}

static void QZ_Strobe(void)
{
 QZValR=QZVal;
 //puts("Strobe");
}

static void QZ_Write(uint8 V)
{
 //printf("Wr: %02x\n",V);
 FunkyMode=V&4;
}

static void QZ_Update(void *data)
{
 QZVal=*(uint8 *)data;
}

static int StateActionFC(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
   SFVAR(QZValR),
   SFVAR(QZVal),
   SFVAR(FunkyMode),
   SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "INPF");
 if(load)
 {

 }
 return(ret);
}

static INPUTCFC PartyTap = { QZ_Read,QZ_Write,QZ_Strobe,QZ_Update,0,0, StateActionFC, 1, sizeof(uint8) };

INPUTCFC *MDFN_InitPartyTap(void)
{
 QZVal=QZValR=0;

 return(&PartyTap);
}
