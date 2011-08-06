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

static uint8 cmdin;
static uint8 cmd;
static uint8 regs[8];
static uint8 master,chrm;

static int32 IRQCount,IRQa;

static void DoPRG(void)
{
 if(master&0x80)
 {
  if(master&0x20)
  {
   setprg32(0x8000,(master&0xF)>>1);
  }
  else
  {
   setprg16(0x8000,master&0xF);
   setprg16(0xC000,master&0xF);  
  }
 }
 else
 {
  setprg8(0xA000,regs[4]);
  setprg8(0xE000,~0);
  if(cmd&0x40)
  {
   setprg8(0xC000,regs[2]);
   setprg8(0x8000,~1);
  }
  else
  {
   setprg8(0x8000,regs[2]);
   setprg8(0xC000,~1);
  }
 }
}

static void DoCHR(void)
{
 uint32 base=(cmd&0x80)<<5;
 int orie=(chrm&0x4)<<6;

 setchr2(0x0000^base,(orie|regs[0])>>1); //K
 setchr2(0x0800^base,(orie|regs[3])>>1); //43
 
 setchr1(0x1000,orie|regs[1]);
 setchr1(0x1400,orie|regs[5]);
 setchr1(0x1800,orie|regs[6]);
 setchr1(0x1c00,orie|regs[7]);
}

static DECLFW(UNL8237Write)
{
 switch(A&0xF000)
 {
  case 0xf000:IRQCount=V;break;
  case 0xE000:X6502_IRQEnd(MDFN_IQEXT);break;
 }
// if(A<0x8000)
//  printf("$%04x:$%02x, %d\n",A&0xFFFF,V,scanline);
 if(A==0x5000)
 {
  master=V;
  DoPRG();
  DoCHR();  
 }
 else if(A==0x5001)
 {
  chrm=V;
  DoCHR();
 }
 else
 switch(A&0xE000)
 {
  case 0x8000:setmirror(((V|(V>>7))&1)^1);break;
  case 0xa000:cmd=V;cmdin=1;DoPRG();DoCHR();break;
  case 0xC000:if(!cmdin) break;
	      regs[cmd&7]=V;
	      DoPRG();
	      DoCHR();
	      cmdin=0;
	      break;
 }
}

static void UNL8237Reset(CartInfo *info)
{
  int x;

  for(x=0;x<8;x++) regs[x]=0;  
  master=chrm=cmd=cmdin=IRQCount=IRQa=0;
  DoPRG();
  DoCHR();
}

static void hooko(void)
{
 if(IRQCount)
 {
  IRQCount--;
  if(!IRQCount)
  {
   X6502_IRQBegin(MDFN_IQEXT);
   //printf("IRQ: %d\n",scanline);
  }
 }
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAYN(regs, 8, "REGS"),
  SFVARN(IRQCount, "IRQC"),
  SFVARN(IRQa, "IRQA"),
  SFVARN(master, "MAST"),
  SFVARN(chrm, "CHRM"),
  SFVARN(cmd, "CMD"),
  SFVARN(cmdin, "CMDI"),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");

 if(load)
 {
  DoPRG();
  DoCHR();
 }
 return(ret);
}

int UNL8237_Init(CartInfo *info)
{
  GameHBIRQHook=hooko;
  info->Power=UNL8237Reset;
  info->StateAction = StateAction;

  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x5000,0xFFFF,UNL8237Write);

  return(1);
}
