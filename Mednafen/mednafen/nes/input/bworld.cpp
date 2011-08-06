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

static int seq,ptr,bit,cnt,have;
static uint8 bdata[20];


static uint8 Read(int w, uint8 ret)
{
 if(w && have) 
 {
  switch(seq)
  {
   case 0: seq++; ptr=0; ret|=0x4; break;
   case 1: seq++; bit=bdata[ptr]; cnt=0; ret|=0x4; break;
   case 2: ret|=((bit&0x01)^0x01)<<2; bit>>=1; if(++cnt > 7) seq++;
	   break;
   case 3: if(++ptr > 19)
	   {
	    seq=-1;
	    have=0;
	   }
	   else
	    seq=1;
   default: break;
  }
 }
 return(ret);
}

static void Write(uint8 V)
{
 //printf("%02x\n",V);
}

static void Update(void *data)
{
 if(*(uint8 *)data)
 {
  *(uint8 *)data=0;
  seq=ptr=0;
  have=1;
  strcpy((char *)bdata,(char *)data+1);
  memcpy((char *)&bdata[13],"SUNSOFT", strlen("SUNSOFT"));
 }
}

static int StateActionFC(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
   SFARRAY(bdata, 0x20),
   SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "INPF");
 if(load)
 {

 }
 return(ret);
}


static INPUTCFC BarcodeWorld={Read,Write,0,Update,0,0, StateActionFC, 13, sizeof(uint8) };

INPUTCFC *MDFN_InitBarcodeWorld(void)
{
 return(&BarcodeWorld);
}

