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

static uint8 cmd,mir,rmode,IRQmode;
static uint8 DRegs[11];
static uint8 IRQCount,IRQa,IRQLatch;

static void (*setchr1wrap)(unsigned int A, unsigned int V);
static int nomirror;

static void RAMBO1_IRQHook(int a)
{
 static int smallcount;
 if(!IRQmode) return;

 smallcount+=a;
 while(smallcount>=4)
 {
  smallcount-=4;
  IRQCount--;
  if(IRQCount==0xFF)
  {
   if(IRQa)
   {
    //printf("IRQ: %d\n",scanline);
    //rmode = 1;
    X6502_IRQBegin(MDFN_IQEXT);
   }
  }
 }
}

static void RAMBO1_hb(void)
{
      if(IRQmode) return;
      if(scanline==240) return;	/* hmm.  Maybe that should be an mmc3-only
					   call in fce.c. */
      rmode=0;
      IRQCount--;
      if(IRQCount==0xFF)
      {
       if(IRQa)
       {
        rmode = 1;
        X6502_IRQBegin(MDFN_IQEXT);
       }
      }
}

static void Synco(void)
{
 int x;

 if(cmd&0x20)
 {
  setchr1wrap(0x0000,DRegs[0]);
  setchr1wrap(0x0800,DRegs[1]);
  setchr1wrap(0x0400,DRegs[8]);
  setchr1wrap(0x0c00,DRegs[9]);
 }
 else
 {
  setchr1wrap(0x0000,(DRegs[0]&0xFE));
  setchr1wrap(0x0400,(DRegs[0]&0xFE)|1);
  setchr1wrap(0x0800,(DRegs[1]&0xFE));
  setchr1wrap(0x0C00,(DRegs[1]&0xFE)|1);
 }

 for(x=0;x<4;x++)
  setchr1wrap(0x1000+x*0x400,DRegs[2+x]);

 setprg8(0x8000,DRegs[6]);
 setprg8(0xA000,DRegs[7]);

 setprg8(0xC000,DRegs[10]);
}


static DECLFW(RAMBO1_write)
{
 //if(A>=0xC000 && A<=0xFFFF) printf("$%04x:$%02x, %d, %d\n",A,V,scanline,timestamp);
 switch(A&0xF001)
 {
        case 0xa000:mir=V&1;
		    if(!nomirror)
		     setmirror(mir^1);
		    break;
        case 0x8000:cmd = V;
		    break;
        case 0x8001:
		    if((cmd&0xF)<10)
		     DRegs[cmd&0xF]=V;
		    else if((cmd&0xF)==0xF)
		     DRegs[10]=V;
		    Synco();
		    break;
        case 0xc000:IRQLatch=V;
                    if(rmode==1)
                     {
                      IRQCount=IRQLatch;
                     }
                    break;
        case 0xc001:rmode=1;
                    IRQCount=IRQLatch;
		    IRQmode=V&1;
                    break;
        case 0xE000:IRQa=0;X6502_IRQEnd(MDFN_IQEXT);
                    if(rmode==1)
                     {IRQCount=IRQLatch;}
                    break;
        case 0xE001:IRQa=1;
                    if(rmode==1)
                     {IRQCount=IRQLatch;}
                    break;
  }	
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[]={
        SFVARN(cmd, "CMD"),
        SFVARN(mir, "MIR"),
        SFVARN(rmode, "RMOD"),
        SFVARN(IRQmode, "IRQM"),
        SFVARN(IRQCount, "IRQC"),
        SFVARN(IRQa, "IRQA"),
        SFVARN(IRQLatch, "IRQL"),
        SFARRAYN(DRegs, 11, "DREG"),
        SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");

 if(load)
 {
  Synco();
  if(!nomirror)
   setmirror(mir^1);
 }

 return(ret);
}

static void Power(CartInfo *info)
{
	int x;

        for(x=0;x<11;x++)
         DRegs[x]=~0;
        cmd=0;
        mir=0;
        if(!nomirror)
         setmirror(1);
        Synco();
	setprg8(0xe000, 0xFF);
}

static void RAMBO1_init(CartInfo *info)
{
	info->Power = Power;
	info->StateAction = StateAction;
        GameHBIRQHook=RAMBO1_hb;
        MapIRQHook=RAMBO1_IRQHook;
        SetWriteHandler(0x8000,0xffff,RAMBO1_write);
	SetReadHandler(0x8000, 0xFFFF, CartBR);
}

static void CHRWrap(unsigned int A, unsigned int V)
{
 setchr1(A,V);
}

int Mapper64_Init(CartInfo *info)
{
	setchr1wrap=CHRWrap;
	nomirror=0;
	RAMBO1_init(info);
	return(1);
}

static int MirCache[8];
static unsigned int PPUCHRBus;

static void MirWrap(unsigned int A, unsigned int V)
{
 MirCache[A>>10]=(V>>7)&1;
 if(PPUCHRBus==(A>>10))
  setmirror(MI_0+((V>>7)&1));
 setchr1(A,V);
}

static void MirrorFear(uint32 A)
{
 A&=0x1FFF;
 A>>=10;
 PPUCHRBus=A;
 setmirror(MI_0+MirCache[A]);
}

int Mapper158_Init(CartInfo *info)
{
	setchr1wrap=MirWrap;
	PPU_hook=MirrorFear;
	nomirror=1;
	RAMBO1_init(info);
	return(1);
}


static DECLFW(MIMIC1_Write)
{


}

void Mapper159_init(void)
{
	SetWriteHandler(0x8000,0xFFFF,MIMIC1_Write);
}
