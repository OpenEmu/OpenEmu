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

// TODO: Fix NSF variable initialization

#include "mapinc.h"
#include "../nsf.h"

static uint8 CHRBanks[8], PRGBanks[3], WRAM[8192], Mirroring;
static uint8 IRQLatch, IRQCount, IRQEnabled;
static uint8 vrctemp;
static uint8 indox;
static int32 acount;
static int32 divc;

#include "emu2413.h"

static OPLL *VRC7Sound=NULL;
static uint32 V7BC;
static int32 V7out;

void DoVRC7Sound(void)
{
 uint32 V;

 for(V=V7BC;V<SOUNDTS;V++)
 {
  if(!divc)
  {
   V7out=(OPLL_calc(VRC7Sound)<<1);
  }
  divc=(divc+1)%36;
  WaveHiEx[V]+=V7out;
 }

 V7BC = SOUNDTS;
}

static void HiSync(int32 ts)
{
 V7BC = ts;
}


static INLINE void DaMirror(int V)
{
 int salpo[4]={MI_V,MI_H,MI_0,MI_1};
 setmirror(salpo[V&3]);
}

static DECLFW(Mapper85_write)
{
	A|=(A&8)<<1;

	if(A>=0xa000 && A<=0xDFFF)
	{
	// printf("$%04x, $%04x\n",X.PC,A);
	 A&=0xF010;
	 {
	  int x=((A>>4)&1)|((A-0xA000)>>11);
 	  CHRBanks[x] = V;
	  setchr1(x<<10,V);
	 }
	}
	else if(A==0x9030)
	{
	 DoVRC7Sound();
	 OPLL_writeReg(VRC7Sound, indox, V);
	}
	else switch(A&0xF010)
        {
         case 0x8000:PRGBanks[0]=V;setprg8(0x8000,V);break;
         case 0x8010:PRGBanks[1]=V;setprg8(0xa000,V);break;
         case 0x9000:PRGBanks[2]=V;setprg8(0xc000,V);break;
         case 0x9010:indox=V;break;
         case 0xe000:Mirroring=V;DaMirror(V);break;
         case 0xE010:IRQLatch=V;
		     X6502_IRQEnd(MDFN_IQEXT);
                     break;
         case 0xF000:IRQEnabled=V&2;
                     vrctemp=V&1;
                     if(V&2) {IRQCount=IRQLatch;}
		     acount=0;
		     X6502_IRQEnd(MDFN_IQEXT);
                     break;
         case 0xf010:if(vrctemp) IRQEnabled=1;
                     else IRQEnabled=0;
		     X6502_IRQEnd(MDFN_IQEXT);
                     break;
        }
}

static void KonamiIRQHook(int a)
{
  #define ACBOO 341
//  #define ACBOO ((227*2)+1)
  if(IRQEnabled)
   {
    acount+=a*3;
    
    if(acount>=ACBOO)
    {
     doagainbub:acount-=ACBOO;
     IRQCount++;
     if(IRQCount == 0) {X6502_IRQBegin(MDFN_IQEXT);IRQCount=IRQLatch;}
     if(acount>=ACBOO) goto doagainbub;
    }
 }
}

static void Sync(void)
{
 int x;

 for(x=0;x<8;x++)
  setchr1(x*0x400,CHRBanks[x]);
 for(x=0;x<3;x++)
  setprg8(0x8000+x*8192,PRGBanks[x]);
 DaMirror(Mirroring);
}

static void M85SKill(void)
{
 if(VRC7Sound)
  OPLL_delete(VRC7Sound);
 VRC7Sound=NULL;
}

static void VRC7SI(EXPSOUND *ep)
{
  ep->Kill=M85SKill;
  ep->HiFill=DoVRC7Sound;
  ep->HiSync = HiSync;

  VRC7Sound=OPLL_new(3579545);
  OPLL_reset(VRC7Sound);
}

int NSFVRC7_Init(EXPSOUND *ep, bool MultiChip)
{
	NSFECSetWriteHandler(0x9010,0x901F,Mapper85_write);
	NSFECSetWriteHandler(0x9030,0x903F,Mapper85_write);
	VRC7SI(ep);

	return(1);
}

static void Power(CartInfo *info)
{
 if(!info->battery)
  memset(WRAM, 0xFF, 8192);

 setprg8r(0x10, 0x6000, 0);
 setprg8(0xe000, 0xFF);

 for(int x = 0; x < 8; x++)
  CHRBanks[x] = x;

 for(int x = 0; x < 3; x++)
  PRGBanks[x] = x;
 Mirroring = IRQLatch = IRQCount = IRQEnabled = vrctemp = indox = acount = divc = 0;
 V7out = 0;
 Sync();

 OPLL_reset(VRC7Sound);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(CHRBanks, 8), SFARRAY(PRGBanks, 3),
  SFARRAY(WRAM, 8192), SFVAR(Mirroring),
  SFVAR(IRQLatch), SFVAR(IRQCount), SFVAR(IRQEnabled),
  SFVAR(vrctemp), SFVAR(indox), SFVAR(acount),
  SFVAR(divc),

  SFARRAY(VRC7Sound->LowFreq, 6),
  SFARRAY(VRC7Sound->HiFreq, 6),
  SFARRAY(VRC7Sound->InstVol, 6),
  SFARRAY(VRC7Sound->CustInst, 8),
  SFARRAY32(VRC7Sound->slot_on_flag, 6 * 2),
  SFVAR(VRC7Sound->pm_phase), SFVAR(VRC7Sound->lfo_pm),
  SFVAR(VRC7Sound->am_phase), SFVAR(VRC7Sound->lfo_am),
  SFARRAY32(VRC7Sound->patch_number, 6),
  SFARRAY32(VRC7Sound->key_status, 6),

  // FIXME
  SFARRAYN((uint8 *)VRC7Sound->slot, sizeof(VRC7Sound->slot), "VRC7Sound->slot"),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
 {
  Sync();
  OPLL_forceRefresh(VRC7Sound);
 }
 return(ret);
}

int Mapper85_Init(CartInfo *info)
{
  SetupCartPRGMapping(0x10, WRAM, 8192, 1);
  info->Power = Power;
  info->StateAction = StateAction;
  if(info->battery)
  {
   info->SaveGame[0] = WRAM;
   info->SaveGameLen[0] = 8192;
  }
  MapIRQHook=KonamiIRQHook;
  SetWriteHandler(0x8000,0xffff,Mapper85_write);
  SetReadHandler(0x6000, 0xFFFF, CartBR);
  SetWriteHandler(0x6000, 0x7FFF, CartBW);

  VRC7SI(&info->CartExpSound);

 return(1);
}
