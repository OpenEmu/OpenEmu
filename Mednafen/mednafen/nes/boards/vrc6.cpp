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
#include "../nsf.h"

static void (*sfun[3])(void);

static uint8 PRGBank8, PRGBank16, CHRBanks[8], Mirroring;

static uint8 IRQCount, IRQLatch, IRQEnabled;
static uint8 vrctemp;
static uint8 VPSG[3][3];
static uint8 WRAM[8192];

static int swaparoo;

static uint8 b3;
static int32 phaseacc;

static int32 acount;

static int32 CVBC[3];
static int32 vcount[3];
static uint32 dcount[2];

#define VOLADJ *4/8

static void DoPRG(void)
{
 setprg16(0x8000, PRGBank16);
 setprg8(0xc000, PRGBank8);
}

static void DoCHR(void)
{
 int x;
 for(x = 0; x < 8; x++)
  setchr1(x * 1024, CHRBanks[x]);
}

static void DoMirroring(void)
{
 switch(Mirroring & 3)
 {
  case 0: setmirror(MI_V); break;
  case 1: setmirror(MI_H); break;
  case 2: setmirror(MI_0); break;
  case 3: setmirror(MI_1); break;
 }
}

static void KonamiIRQHook(int a)
{
  #define LCYCS 341
//  #define LCYCS ((227*2)+1)
  if(IRQEnabled)
  {
   acount+=a*3;
   if(acount>=LCYCS)
   {
    doagainbub:acount-=LCYCS;IRQCount++;
    if(IRQCount == 0x00) 
    { 
     X6502_IRQBegin(MDFN_IQEXT);
     IRQCount=IRQLatch;
    }
    if(acount>=LCYCS) goto doagainbub;
   }
 }
}

static DECLFW(VRC6SW)
{
        A&=0xF003;
        if(A>=0x9000 && A<=0x9002)
        {
         VPSG[0][A&3]=V;
         if(sfun[0]) sfun[0]();
        }
        else if(A>=0xa000 && A<=0xa002)
        {
         VPSG[1][A&3]=V;
         if(sfun[1]) sfun[1]();
        }
        else if(A>=0xb000 && A<=0xb002)
        {
         VPSG[2][A&3]=V;
         if(sfun[2]) sfun[2]();
	}

}

static DECLFW(Mapper24_write)
{
	if(swaparoo)
	 A=(A&0xFFFC)|((A>>1)&1)|((A<<1)&2);
        A&=0xF003;

	if(A>=0x9000 && A<=0xb002) 
	{
	 VRC6SW(A,V);
	 return;
	}

	if(A >= 0xD000 && A <= 0xD003)
	{
	 int w = A & 3;
	 CHRBanks[w] = V;
	 setchr1(w * 0x400, V);
	}
	else if(A >= 0xE000 && A <= 0xE003)
	{
	 int w = 4 | (A & 3);
	 CHRBanks[w] = V;
	 setchr1(w * 0x400, V);
	}
        switch(A&0xF003)
	{
         case 0x8000:PRGBank16 = V; DoPRG(); break;
         case 0xB003: Mirroring = (V >> 2) & 0x3; DoMirroring(); break;
         case 0xC000:PRGBank8 = V; DoPRG(); break;
         case 0xF000:IRQLatch=V;
			//acount=0;
			break;
         case 0xF001:IRQEnabled=V&2;
		     vrctemp=V&1;
                     if(V&2) 
		     {
		      IRQCount=IRQLatch;
		      acount=0;
		     }
		     X6502_IRQEnd(MDFN_IQEXT);
                     break;
         case 0xf002:IRQEnabled = vrctemp;
		     X6502_IRQEnd(MDFN_IQEXT);break;
         case 0xF003:break;
  }
}

static INLINE void DoSQVHQ(int x)
{
 uint32 V;
 int32 amp=((VPSG[x][0]&15)<<8)VOLADJ;
     
 if(VPSG[x][0x2]&0x80)  
 {
  if(VPSG[x][0]&0x80)
  {
   for(V=CVBC[x];V<SOUNDTS;V++)
    WaveHiEx[V]+=amp;
  }
  else   
  {
   uint32 thresh=(VPSG[x][0]>>4)&7;
   for(V=CVBC[x];V<SOUNDTS;V++)
   {
    if(dcount[x]>thresh)        /* Greater than, not >=.  Important. */
     WaveHiEx[V]+=amp;
    vcount[x]--;
    if(vcount[x]<=0)            /* Should only be <0 in a few circumstances. */
    {
     vcount[x]=(VPSG[x][0x1]|((VPSG[x][0x2]&15)<<8))+1;
     dcount[x]=(dcount[x]+1)&15;
    }
   }
  }
 }
 CVBC[x]=SOUNDTS;
}
   
static void DoSQV1HQ(void)
{
 DoSQVHQ(0);
}

static void DoSQV2HQ(void)
{ 
 DoSQVHQ(1);
}
   
static void DoSawVHQ(void)
{
 uint32 V;
 
 if(VPSG[2][2]&0x80)
 {
  for(V=CVBC[2];V<SOUNDTS;V++)
  {
   WaveHiEx[V]+=(((phaseacc>>3)&0x1f)<<8) VOLADJ;
   vcount[2]--;
   if(vcount[2]<=0)
   {
    vcount[2]=(VPSG[2][1]+((VPSG[2][2]&15)<<8)+1)<<1;
    phaseacc+=VPSG[2][0]&0x3f;
    b3++;
    if(b3==7)
    {
     b3=0;
     phaseacc=0;
   }
 
   }
  }
 }
 CVBC[2]=SOUNDTS;
}

void VRC6SoundHQ(void)
{
    DoSQV1HQ();
    DoSQV2HQ();
    DoSawVHQ();
}

void VRC6SyncHQ(int32 ts)
{
 int x;
 for(x=0;x<3;x++) CVBC[x]=ts;
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(WRAM, 8192),
  SFARRAY(CHRBanks, 8),
  SFVAR(PRGBank8), SFVAR(PRGBank16),
  SFVAR(IRQCount), SFVAR(IRQLatch), SFVAR(IRQEnabled),
  SFVAR(b3), SFVAR(phaseacc), SFVAR(acount),
  SFVAR(vcount[0]), SFVAR(vcount[1]), SFVAR(vcount[2]),
  SFVAR(dcount[0]), SFVAR(dcount[1]),
  SFARRAYN(&VPSG[0][0], 3 * 3, "VPSG"),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "VRC6");

 if(load)
 {
  DoPRG();
  DoCHR();
  DoMirroring();
 }
 return(ret);
}

static void VRC6_ESI(EXPSOUND *ep)
{
        ep->HiFill=VRC6SoundHQ;
        ep->HiSync=VRC6SyncHQ;

	memset(CVBC,0,sizeof(CVBC));
	memset(vcount,0,sizeof(vcount));
	memset(dcount,0,sizeof(dcount));
        b3 = 0;
        phaseacc = 0;
        acount = 0;

	sfun[0] = DoSQV1HQ;
	sfun[1] = DoSQV2HQ;
	sfun[2] = DoSawVHQ;
}

static void Close(void)
{

}

static void Power(CartInfo *info)
{
	for(int x = 0; x < 8; x++)
	 CHRBanks[x] = x;

	DoCHR();

	if(!info->battery)
	 memset(WRAM, 0xFF, 8192);

	setprg8r(0x10, 0x6000, 0);
	PRGBank8 = ~1;
        PRGBank16 = 0;

	setprg8(0xe000, ~0);
	DoPRG();

	IRQCount = IRQLatch = IRQEnabled = 0;
	Mirroring = 0;
	DoMirroring();

	b3 = 0;
	phaseacc = 0;
	acount = 0;

}

static int VRC6_Init(CartInfo *info)
{
	SetupCartPRGMapping(0x10, WRAM, 8192, 1);
        SetWriteHandler(0x8000, 0xffff, Mapper24_write);
        SetReadHandler(0x6000, 0xFFFF, CartBR);
	SetWriteHandler(0x6000, 0x7FFF, CartBW);

        VRC6_ESI(&info->CartExpSound);

        MapIRQHook = KonamiIRQHook;
	info->StateAction = StateAction;
	info->Power = Power;
	info->Close = Close;

	if(info->battery)
	{
	 info->SaveGame[0] = WRAM;
	 info->SaveGameLen[0] = 8192;
	}
	return(1);
}

int Mapper24_Init(CartInfo *info)
{
	swaparoo = 0;
	return(VRC6_Init(info));
}

int Mapper26_Init(CartInfo *info)
{
	swaparoo = 1;
	return(VRC6_Init(info));
}

int NSFVRC6_Init(EXPSOUND *ep, bool MultiChip)
{
	VRC6_ESI(ep);

	if(MultiChip)
	{
         NSFECSetWriteHandler(0x9000, 0x9002, VRC6SW);
         NSFECSetWriteHandler(0xa000, 0xa002, VRC6SW);
	 NSFECSetWriteHandler(0xb000, 0xb002, VRC6SW);
	}
	else
	{
	 NSFECSetWriteHandler(0x9000,0xbfff,VRC6SW);
	}
	return(1);
}
