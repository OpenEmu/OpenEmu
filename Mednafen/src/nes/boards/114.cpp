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

static uint8 master, incmd, IRQCount, cmd, regs[8], Mirroring;

static void dochr(void)
{
  setchr2(0x0000,(regs[0]>>1));
  setchr2(0x0800,(regs[2]>>1));
  setchr1(0x1000, regs[6]); 
  setchr1(0x1400, regs[1]); 
  setchr1(0x1800, regs[7]);
  setchr1(0x1c00, regs[3]);
}

static void doprg()
{
 if(master&0x80)
 {
  setprg16(0x8000,master&0x1F);
 }
 else
 {
  setprg8(0x8000,regs[4]);
  setprg8(0xa000,regs[5]);
 }
}

static DECLFW(Mapper114_write)
{
 if(A<=0x7FFF)
 {
  master=V;
  doprg();
 }
 else if(A==0xe003) IRQCount=V;
 else if(A==0xe002) X6502_IRQEnd(MDFN_IQEXT);
 else switch(A&0xE000)
 {
  case 0x8000:Mirroring = V & 1; setmirror((V & 1) ^ 1);break;
  case 0xa000:cmd=V;incmd=1;break;
  case 0xc000:
	      if(!incmd) break;
	      regs[cmd & 0x7]=V;
	      switch(cmd & 0x7)
	      {
	       case 0x0: case 1: case 2: case 3: case 6: case 7: 
		dochr();break;
	       case 0x4:
	       case 0x5:doprg();break;
	      }
		incmd=0;
		break;
 }

}

static void Mapper114_hb(void)
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
  SFVAR(master), SFVAR(incmd), SFVAR(IRQCount), SFVAR(Mirroring), SFVAR(cmd), 
  SFARRAY(regs, 8),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "M114");
 if(load)
 {
  Mirroring &= 1;
  dochr();
  doprg();
  setmirror(Mirroring ^ 1);
 }

 return(ret);
}
static void Power(CartInfo *info)
{
 int x;

 IRQCount = 0;
 master = incmd = cmd = ~0;

 for(x = 0; x < 8; x++)
  regs[x] = ~0;

 dochr();
 doprg();
 setprg16(0xc000, ~0);

 Mirroring = 1;
 setmirror(Mirroring ^ 1);
}

int Mapper114_Init(CartInfo *info)
{
 GameHBIRQHook=Mapper114_hb;
 SetWriteHandler(0x6000,0xffff,Mapper114_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 info->Power = Power;
 info->StateAction = StateAction;
 return(1);
}

