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

static uint16 IRQCount;
static uint8 IRQa;

static uint8 WRAM[8192];
static uint8 IRAM[128];

static DECLFR(AWRAM)
{
 return(WRAM[A-0x6000]);
}

static DECLFW(BWRAM)
{
 WRAM[A-0x6000]=V;
}

static uint8 NTAPage[4];

static uint8 dopol;
static uint8 gorfus;
static uint8 gorko;

static void (*DoNamcoSound)(void) = NULL;
static void SyncHQ(int32 ts);

static int is210;	/* Lesser mapper. */

static uint8 PRG[3];
static uint8 CHR[8];

static uint32 PlayIndex[8];
static int32 vcount[8];
static int32 CVBC;

static void SyncPRG(void)
{
 setprg8(0x8000,PRG[0]);
 setprg8(0xa000,PRG[1]);
 setprg8(0xc000,PRG[2]);
 setprg8(0xe000,0x3F);
}

static void NamcoIRQHook(int a)
{
  if(IRQa)
  {
   IRQCount+=a;
   if(IRQCount>=0x7FFF) 
   {
    X6502_IRQBegin(MDFN_IQEXT);
    IRQa=0;
    IRQCount=0x7FFF; //7FFF;
   }
  }
}

static DECLFR(Namco_Read4800)
{
	uint8 ret=IRAM[dopol&0x7f];

	/* Maybe I should call DoNamcoSound() here? */
	if(!fceuindbg)
	 if(dopol&0x80)
	  dopol=(dopol&0x80)|((dopol+1)&0x7f);
	return ret;
}

static DECLFR(Namco_Read5000)
{
 return(IRQCount);
}

static DECLFR(Namco_Read5800)
{
 return(IRQCount>>8);
}

static void DoNTARAMROM(int w, uint8 V)
{
        NTAPage[w]=V;

	if(V>=0xE0)
	 setntamem(NTARAM+((V&1)<<10), 1, w);
	else 
	{
	 V&=CHRmask1[0];
	 setntamem(CHRptr[0]+(V<<10), 0, w);
	}
}

static void FixNTAR(void)
{
 int x;

 for(x=0;x<4;x++)
  DoNTARAMROM(x,NTAPage[x]);
}

static void DoCHRRAMROM(int x, uint8 V)
{
         CHR[x]=V;

         if(!is210 && !((gorfus>>((x>>2)+6))&1) && (V>=0xE0))
	 {
	  // printf("BLAHAHA: %d, %02x\n",x,V);
          //setchr1r(0x10,x<<10,V&7);
	 }
         else
	 {
//	  printf("Noha: %d, %02x\n",x,V);
          setchr1(x<<10,V);
	 }
}

static void FixCRR(void)
{
 int x;
 for(x=0;x<8;x++)
  DoCHRRAMROM(x,CHR[x]);
}

static DECLFW(Mapper19C0D8_write)
{
	DoNTARAMROM((A-0xC000)>>11,V);
}

static uint32 FreqCache[8];
static uint32 EnvCache[8];
static uint32 LengthCache[8];

static void FixCache(int a,int V)
{
                     int w=(a>>3)&0x7;
                     switch(a&0x07)
                     {
                      case 0x00:FreqCache[w]&=~0x000000FF;FreqCache[w]|=V;break;
                      case 0x02:FreqCache[w]&=~0x0000FF00;FreqCache[w]|=V<<8;break;
                      case 0x04:FreqCache[w]&=~0x00030000;FreqCache[w]|=(V&3)<<16;
                                LengthCache[w]=(8-((V>>2)&7))<<2;
                                break;
                      case 0x07:EnvCache[w]=(uint32)(V&0xF)*576716;break;
                     }

}

static DECLFW(Mapper19_write)
{
	A&=0xF800;

	if(A>=0x8000 && A<=0xb800)
	 DoCHRRAMROM((A-0x8000)>>11,V);
        else switch(A)
	{
	 case 0x4800:
		   //printf("Yahaoo: %02x, %02x\n",dopol&0x7F,V);
		   //puts("Hmm");
		   if(dopol&0x40)
                   {
		    DoNamcoSound();
		    FixCache(dopol,V);
                   }
		   IRAM[dopol&0x7f]=V;
		   
                   if(dopol&0x80)
                    dopol=(dopol&0x80)|((dopol+1)&0x7f);
                   break;

        case 0xf800: dopol=V;break;
        case 0x5000: IRQCount&=0xFF00;IRQCount|=V;X6502_IRQEnd(MDFN_IQEXT);break;
        case 0x5800: IRQCount&=0x00ff;IRQCount|=(V&0x7F)<<8;
                     IRQa=V&0x80;
                     X6502_IRQEnd(MDFN_IQEXT);
		     //puts("IRQe");
                     break;

        case 0xE000:gorko=V&0xC0;
		    PRG[0]=V&0x3F;
		    SyncPRG();
                    break;
        case 0xE800:gorfus=V&0xC0;
		    FixCRR();
                    PRG[1]=V&0x3F;
                    SyncPRG();
                    break;
        case 0xF000:PRG[2]=V&0x3F;
		    SyncPRG();
                    break;
        }
}

#define TOINDEXBS (16 + 10)

#define TOINDEX	(16+1)

// 16:15
static void SyncHQ(int32 ts)
{
 CVBC=ts;
}


/* Things to do:
	1	Read freq low
	2	Read freq mid
	3	Read freq high
	4	Read envelope
	...?
*/

static INLINE uint32 FetchDuff(uint32 P, uint32 envelope)
{
    uint32 duff;
    duff=IRAM[((IRAM[0x46+(P<<3)]+(PlayIndex[P]>>TOINDEX))&0xFF)>>1];
    if((IRAM[0x46+(P<<3)]+(PlayIndex[P]>>TOINDEX))&1)
     duff>>=4;
    duff&=0xF;
    duff=(duff*envelope)>>16;
    return(duff);
}

static INLINE uint32 FetchDuffBS(uint32 P, uint32 envelope)
{
    uint32 duff;
    duff=IRAM[((IRAM[0x46+(P<<3)]+(PlayIndex[P]>>TOINDEXBS))&0xFF)>>1];
    if((IRAM[0x46+(P<<3)]+(PlayIndex[P]>>TOINDEXBS))&1)
     duff>>=4;
    duff&=0xF;
    duff=(duff*envelope)>>16;
    return(duff);
}

static void DoNamcoSoundAccurate(void)
{
 int32 P;
 uint32 V;
 int32 cyclesuck=(((IRAM[0x7F]>>4)&7)+1) * 15;

 for(P=7;P>=(7-((IRAM[0x7F]>>4)&7));P--)
 {
  if((IRAM[0x44+(P<<3)]&0xE0) && (IRAM[0x47+(P<<3)]&0xF))
  {
   uint32 freq;
   int32 vco;
   uint32 duff2,lengo,envelope;

   vco = vcount[P];
   freq = FreqCache[P];

   envelope = EnvCache[P];
   lengo=LengthCache[P];

   duff2=FetchDuff(P,envelope);
   for(V=CVBC<<1;V<SOUNDTS<<1;V++)
   {
    WaveHiEx[V>>1]-=duff2;
    if(!vco)
    {
     PlayIndex[P]+=freq;
     while((PlayIndex[P]>>TOINDEX) >=lengo)
      PlayIndex[P]-=lengo<<TOINDEX;
     duff2=FetchDuff(P,envelope);
     vco=cyclesuck;
    }
    vco--;
   }
   vcount[P]=vco;
  }
 }
 CVBC=SOUNDTS;
}


static void DoNamcoSoundBS(void)
{
 int32 P;
 uint32 V;
 int32 cyclesuck=(((IRAM[0x7F]>>4)&7)+1) * 15;

 for(P=7;P>=(7-((IRAM[0x7F]>>4)&7));P--)
 {
  if((IRAM[0x44+(P<<3)]&0xE0) && (IRAM[0x47+(P<<3)]&0xF))
  {
   uint32 freq;
   int32 vco;
   uint32 duff2,lengo,envelope;

   vco = vcount[P];
   freq = FreqCache[P] * 1024 / cyclesuck;

   envelope = EnvCache[P];
   lengo = LengthCache[P];

   duff2 = FetchDuffBS(P,envelope) << 1;

   for(V=CVBC;V<SOUNDTS;V++)
   {
    WaveHiEx[V] -= duff2;
    PlayIndex[P]+=freq;
    while((PlayIndex[P]>>TOINDEXBS) >=lengo)
     PlayIndex[P]-=lengo<<TOINDEXBS;
    duff2 = FetchDuffBS(P,envelope) << 1;
   }
  }
 }
 CVBC=SOUNDTS;
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
        SFARRAY(PRG,3),
        SFARRAY(CHR,8),
        SFARRAYN(NTAPage, 4, "NTA"),
	SFVARN(dopol, "DOPL"),
        SFVARN(gorfus, "GFUS"),
        SFVARN(gorko, "GRKO"),
        SFVARN(IRQCount, "IRQC"),
        SFVARN(IRQa, "IRQA"),

        SFARRAY(WRAM, 8192),
        SFARRAY(IRAM, 128),

	SFARRAY32(PlayIndex, 8),
	SFARRAY32(vcount, 8),
        SFEND
 };

 if(!load)
 {
  // Pre-normalize
  if(DoNamcoSound == DoNamcoSoundBS)
   for(int x = 0; x < 8; x++)
     PlayIndex[x] <<= TOINDEXBS - TOINDEX;
 }

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");

 if(load)
 {
  SyncPRG();
  FixNTAR();
  FixCRR();

  // Post-normalize
  if(DoNamcoSound == DoNamcoSoundBS)
   for(int x = 0; x < 8; x++)
     PlayIndex[x] <<= TOINDEXBS - TOINDEX;

  for(int x=0x40;x<0x80;x++)
   FixCache(x,IRAM[x]);
 }
 return(ret);
}

void Mapper19_ESI(EXPSOUND *ep)
{
 DoNamcoSound = MDFN_GetSettingB("nes.n106bs") ? DoNamcoSoundBS : DoNamcoSoundAccurate;

 ep->HiFill=DoNamcoSound;
 ep->HiSync=SyncHQ;

 memset(vcount,0,sizeof(vcount));
 memset(PlayIndex,0,sizeof(PlayIndex));
 CVBC=0;
}

int NSFN106_Init(EXPSOUND *ep, bool MultiChip)
{
 NSFECSetWriteHandler(0xf800,0xffff,Mapper19_write);
 NSFECSetWriteHandler(0x4800,0x4fff,Mapper19_write);
 SetReadHandler(0x4800,0x4fff,Namco_Read4800);
 Mapper19_ESI(ep);
 return(1);
}

static void InstallRW(CartInfo *info)
{
        SetReadHandler(0x8000,0xFFFF,CartBR);
        SetWriteHandler(0x8000,0xffff,Mapper19_write);
        SetWriteHandler(0x4020,0x5fff,Mapper19_write);

        if(!is210)
        {
         SetWriteHandler(0xc000,0xdfff,Mapper19C0D8_write);
         SetReadHandler(0x4800,0x4fff,Namco_Read4800);
         SetReadHandler(0x5000,0x57ff,Namco_Read5000);
         SetReadHandler(0x5800,0x5fff,Namco_Read5800);
        }

        SetReadHandler(0x6000,0x7FFF,AWRAM);
        SetWriteHandler(0x6000,0x7FFF,BWRAM);


}

static void N106_Power(CartInfo *info)
{
	int x;

	if(!is210)
	{
	 NTAPage[0]=NTAPage[1]=NTAPage[2]=NTAPage[3]=0xFF;
	 FixNTAR();
	}

        MDFNMP_AddRAM(8192, 0x6000, WRAM);

        gorfus=0xFF;
	SyncPRG();
	FixCRR();

	if(!info->battery)
	{
	 memset(WRAM,0,8192);
	 memset(IRAM,0,128);
	}
	for(x=0x40;x<0x80;x++)
	 FixCache(x,IRAM[x]);
}

#ifdef WANT_DEBUGGER
static void GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer)
{
 if(!strcmp(name, "wram"))
 {
  while(Length--)
  {
   Address &= 8192 - 1;
   *Buffer = WRAM[Address];
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "iram"))
 {
  while(Length--)
  {
   Address &= 0x7F;
   *Buffer = IRAM[Address];
   Address++;
   Buffer++;
  }
 }
}

static void PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer)
{
 if(!strcmp(name, "wram"))
 {
  while(Length--)
  {
   Address &= 8192 - 1;
   WRAM[Address] = *Buffer;
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "iram"))
 {
  while(Length--)
  {
   Address &= 0x7F;
   IRAM[Address] = *Buffer;
   Address++;
   Buffer++;
  }
 }
}
#endif

int Mapper19_Init(CartInfo *info)
{
	is210=0;

	info->Power=N106_Power;
	info->StateAction = StateAction;

        MapIRQHook=NamcoIRQHook;

        Mapper19_ESI(&info->CartExpSound);

	if(info->battery)
	{
	 info->SaveGame[0] = WRAM;
         info->SaveGameLen[0] = 8192; 

	 info->SaveGame[1] = IRAM;
	 info->SaveGameLen[1] = 128;
	}

	#ifdef WANT_DEBUGGER
	ASpace_Add(GetAddressSpaceBytes, PutAddressSpaceBytes, "wram", "Cart WRAM", 13);
	ASpace_Add(GetAddressSpaceBytes, PutAddressSpaceBytes, "iram", "N106 Internal RAM", 7);
	#endif

	InstallRW(info);

	return(1);
}

static int Mapper210_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
        SFARRAY(PRG,3),
        SFARRAY(CHR,8),
	SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");
 if(load)
 {
  SyncPRG();
  FixCRR();
 }
 return(ret);
}

int Mapper210_Init(CartInfo *info)
{
 is210=1;
 info->StateAction = Mapper210_StateAction;
 info->Power=N106_Power;

 InstallRW(info);

 return(1);
}
