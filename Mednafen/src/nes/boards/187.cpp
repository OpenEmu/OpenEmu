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

// TODO:  Remove this mapper or add a StateAction for it!

#include "mapinc.h"

static uint8 LastWr;
static uint8 ExtMode;
static uint8 cmd;
static uint8 DRegs[8];
static uint32 count;
static uint32 last;
static int safe;
static uint8 poofish;

static int32 IRQCount;
static uint8 IRQa;
static uint8 IRQLatch;

static void Sync(void)   
{
 int x;
 uint32 base=0;

 if(cmd&0x80) base=0x1000;  
 setchr2(0x0000^base,(0x100|DRegs[0])>>1);
 setchr2(0x0800^base,(0x100|DRegs[1])>>1);
 for(x=0;x<4;x++)
  setchr1((0x1000^base)+x*0x400,DRegs[2+x]);

 if(ExtMode&0x80)
 {
  if(ExtMode&0x20)
   setprg32(0x8000,(ExtMode&0x1F)>>1);
  else
  {
   setprg16(0x8000,(ExtMode&0x1F));
   setprg16(0xc000,(ExtMode&0x1F));
  }
 }
 else
 {
  setprg8(0x8000,DRegs[6]);
  setprg8(0xc000,~1);
  setprg8(0xa000,DRegs[7]);
  setprg8(0xe000,~0);
 }
}

static DECLFW(M187Write)
{
 LastWr=V;
 if(A==0x5000)
  ExtMode=V;
 Sync();
}

static DECLFW(M187HWrite)
{
 //if(A==0x8003 && X.PC<0x800) MDFNI_DumpMem("dmp",0x0000,0xFFFF);
 //printf("$%04x:$%02x, %04x\n",A,V,X.PC);
 LastWr=V;

 if(A==0x8003) 
 {
  if(V==0x28 || V==0x2A)
   poofish=V;
 }
 else if(A==0x8000) poofish=0;

 switch(A)
 {
  case 0xc000:IRQLatch=IRQCount=V;break;
  case 0xc001:IRQCount=IRQLatch;last=count=0;break;
  case 0xe000:IRQa=0;X6502_IRQEnd(MDFN_IQEXT);break;
  case 0xe001:IRQa=1;break;
  case 0xa000:setmirror((V&1) ^ 1);break;

  case 0x8000:cmd=V;safe=1;break;
  case 0x8001:
	      if(safe)
	      {
	       //printf("Cmd: %d, %02x, %04x\n",cmd&0x7,V,X.PC);
	       DRegs[cmd&7]=V;
	       Sync();
               safe=0;
              }
		if(poofish==0x28) setprg8(0xc000,0x17);
	      break;

 }
}

static DECLFR(ProtRead)
{
 switch(LastWr&0x3)
 {
  case 0x1:
  case 0x0: return(0x80);
  case 0x2: return(0x42);
 }
 return(0);
}

static void M187Power(CartInfo *info)
{
 LastWr = 0;
 ExtMode = 0;
 cmd = 0;

 memset(DRegs, 0, sizeof(DRegs));

 count = 0;
 last = 0;
 safe = 0;
 poofish = 0;

 IRQCount = 0;
 IRQa = 0;
 IRQLatch = 0;

 Sync();
}

static void sl(void)
{
 if(IRQa)
 {
  if(IRQCount>=0) 
  {
   IRQCount--;
   if(IRQCount<0)
   {
    X6502_IRQBegin(MDFN_IQEXT);
   }
  }
 }
}

static void foo(uint32 A)
{
 if((A&0x2000) && !(last&0x2000))
 {
  count++;
  if(count==42)
  { 
   sl();
   count=0;
  }
 }
 last=A;
}

int Mapper187_Init(CartInfo *info)
{
 info->Power=M187Power;
 //GameHBIRQHook=sl;
 PPU_hook=foo;

 SetWriteHandler(0x5000,0x5fff,M187Write);
 SetWriteHandler(0x8000,0xFFFF,M187HWrite);
 SetReadHandler(0x5000,0x5FFF,ProtRead);
 SetWriteHandler(0x6000,0x7FFF,M187Write);
 SetReadHandler(0x8000,0xffff,CartBR);

 return(1);
}

