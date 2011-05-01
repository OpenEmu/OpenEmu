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

/* Not finished.  Darn evil game... *Mumble*... */

#include "mapinc.h"

static uint8 cmd;
static uint8 regs[8];

static void DoPRG(void)
{
 if(cmd&0x40)
 {
  setprg8(0xC000,regs[4]);
  setprg8(0xA000,regs[5]);
  setprg8(0x8000,~1);
  setprg8(0xE000,~0);
 }
 else
 {
  setprg8(0x8000,regs[4]);
  setprg8(0xA000,regs[5]);
  setprg8(0xC000,~1);
  setprg8(0xE000,~0);
 }
}

static void DoCHR(void)
{
 uint32 base=(cmd&0x80)<<5;

 setchr2(0x0000^base,regs[0]);
 setchr2(0x0800^base,regs[2]);

 setchr1(0x1000^base,regs[6]);
 setchr1(0x1400^base,regs[1]);
 setchr1(0x1800^base,regs[7]);
 setchr1(0x1c00^base,regs[3]);
}

static DECLFW(PWrite)
{
 //printf("$%04x:$%02x\n",A,V);
}

static DECLFW(H2288Write)
{
 //printf("$%04x:$%02x, $%04x\n",A,V,X.PC);
 //MDFNI_DumpMem("dmp",0xc000,0xffff);

 switch(A&0xE001)
 {
  case 0xa000:setmirror((V&1)^1);break;
  case 0x8000:
	      cmd=V;DoPRG();DoCHR();break;
  case 0x8001:regs[cmd&7]=V;
	      if((cmd&7)==4 || (cmd&7)==5)
	       DoPRG();
	      else
	       DoCHR();
	      break;
 }
}

static DECLFR(H2288Read)
{
 int bit;
 //printf("Read: $%04x, $%04x\n",A,X.PC);
 //DumpMem("out",0x8000,0xFFFF);
 bit=(A&1)^1;
 bit&=((A>>8)&1);
 bit^=1;
 return((X.DB&0xFE)|bit);
}

static void H2288Reset(CartInfo *info)
{
  int x;

  for(x=0;x<8;x++) regs[x]=0;
  regs[4]=0;
  regs[5]=1;
  cmd=0;
  DoPRG();
  DoCHR();
}

int H2288_Init(CartInfo *info)
{
 info->Power=H2288Reset;

 SetReadHandler(0x5000,0x5FFF,H2288Read);
 SetReadHandler(0x8000,0xFFFF,CartBR);
 SetWriteHandler(0x5000,0x5FFF,PWrite);
 SetWriteHandler(0x8000,0xFFFF,H2288Write);

 return(1);
}
