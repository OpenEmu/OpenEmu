/* FCE Ultra - NES/Famicom Emulator
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "fceu.h"
#include "ppu.h"

#include "cart.h"
#include "memory.h"
#include "x6502.h"

#include "general.h"

/*
   This file contains all code for coordinating the mapping in of the
   address space external to the NES.
   It's also (ab)used by the NSF code.
*/

uint8 *Page[32],*VPage[8];
uint8 **VPageR=VPage;
uint8 *VPageG[8];
uint8 *MMC5SPRVPage[8];
uint8 *MMC5BGVPage[8];

static uint8 PRGIsRAM[32];  /* This page is/is not PRG RAM. */

/* 16 are (sort of) reserved for UNIF/iNES and 16 to map other stuff. */
static int CHRram[32];
static int PRGram[32];

uint8 *PRGptr[32];
uint8 *CHRptr[32];

uint32 PRGsize[32];
uint32 CHRsize[32];

uint32 PRGmask2[32];
uint32 PRGmask4[32];
uint32 PRGmask8[32];
uint32 PRGmask16[32];
uint32 PRGmask32[32];

uint32 CHRmask1[32];
uint32 CHRmask2[32];
uint32 CHRmask4[32];
uint32 CHRmask8[32];

int geniestage=0;

int modcon;

uint8 genieval[3];
uint8 geniech[3];

uint32 genieaddr[3];

static INLINE void setpageptr(int s, uint32 A, uint8 *p, int ram)
{
 uint32 AB=A>>11;
 int x;

 if(p)
  for(x=(s>>1)-1;x>=0;x--)
  {
   PRGIsRAM[AB+x]=ram;
   Page[AB+x]=p-A;
  }
 else
  for(x=(s>>1)-1;x>=0;x--)
  {
   PRGIsRAM[AB+x]=0;
   Page[AB+x]=0;
  }
}

static uint8 nothing[8192];
void ResetCartMapping(void)
{
 int x;

 for(x=0;x<32;x++)
 {
  Page[x]=nothing-x*2048;
  PRGptr[x]=CHRptr[x]=0;
  PRGsize[x]=CHRsize[x]=0;
 }
 for(x=0;x<8;x++)
 {
  MMC5SPRVPage[x]=MMC5BGVPage[x]=VPageR[x]=nothing-0x400*x;
 }

}

void SetupCartPRGMapping(int chip, uint8 *p, uint32 size, int ram)
{
 PRGptr[chip]=p;
 PRGsize[chip]=size;

 PRGmask2[chip]=(size>>11)-1;
 PRGmask4[chip]=(size>>12)-1;
 PRGmask8[chip]=(size>>13)-1;
 PRGmask16[chip]=(size>>14)-1;
 PRGmask32[chip]=(size>>15)-1;

 PRGram[chip]=ram?1:0;
}

void SetupCartCHRMapping(int chip, uint8 *p, uint32 size, int ram)
{
 CHRptr[chip]=p;
 CHRsize[chip]=size;

 CHRmask1[chip]=(size>>10)-1;
 CHRmask2[chip]=(size>>11)-1;
 CHRmask4[chip]=(size>>12)-1;
 CHRmask8[chip]=(size>>13)-1;

 CHRram[chip]=ram;
}

DECLFR(CartBR)
{
 return Page[A>>11][A];
}

DECLFW(CartBW)
{
	/*printf("Ok: %04x:%02x, %d\n",A,V,PRGIsRAM[A>>11]);*/
	if(PRGIsRAM[A>>11] && Page[A>>11])
		Page[A>>11][A]=V;
}

DECLFR(CartBROB)
{
 if(!Page[A>>11]) return(X.DB);
 return Page[A>>11][A];
}

void setprg2r(int r, unsigned int A, unsigned int V)
{
  V&=PRGmask2[r];
  setpageptr(2,A,PRGptr[r]?(&PRGptr[r][V<<11]):0,PRGram[r]);
}

void setprg2(uint32 A, uint32 V)
{
 setprg2r(0,A,V);
}

void setprg4r(int r, unsigned int A, unsigned int V)
{
  V&=PRGmask4[r];
  setpageptr(4,A,PRGptr[r]?(&PRGptr[r][V<<12]):0,PRGram[r]);
}

void setprg4(uint32 A, uint32 V)
{
 setprg4r(0,A,V);
}

void setprg8r(int r, unsigned int A, unsigned int V)
{
  if(PRGsize[r]>=8192)
  {
   V&=PRGmask8[r];
   setpageptr(8,A,PRGptr[r]?(&PRGptr[r][V<<13]):0,PRGram[r]);
  }
  else
  {
   uint32 VA=V<<2;
   int x;
   for(x=0;x<4;x++)
    setpageptr(2,A+(x<<11),PRGptr[r]?(&PRGptr[r][((VA+x)&PRGmask2[r])<<11]):0,PRGram[r]);
  }
}

void setprg8(uint32 A, uint32 V)
{
 setprg8r(0,A,V);
}

void setprg16r(int r, unsigned int A, unsigned int V)
{
  if(PRGsize[r]>=16384)
  {
   V&=PRGmask16[r];
   setpageptr(16,A,PRGptr[r]?(&PRGptr[r][V<<14]):0,PRGram[r]);
  }
  else
  {
   uint32 VA=V<<3;
   int x;

   for(x=0;x<8;x++)
    setpageptr(2,A+(x<<11),PRGptr[r]?(&PRGptr[r][((VA+x)&PRGmask2[r])<<11]):0,PRGram[r]);
  }
}

void setprg16(uint32 A, uint32 V)
{
 setprg16r(0,A,V);
}

void setprg32r(int r,unsigned int A, unsigned int V)
{
  if(PRGsize[r]>=32768)
  {
   V&=PRGmask32[r];
   setpageptr(32,A,PRGptr[r]?(&PRGptr[r][V<<15]):0,PRGram[r]);
  }
  else
  {
   uint32 VA=V<<4;
   int x;

   for(x=0;x<16;x++)
    setpageptr(2,A+(x<<11),PRGptr[r]?(&PRGptr[r][((VA+x)&PRGmask2[r])<<11]):0,PRGram[r]);
  }
}

void setprg32(uint32 A, uint32 V)
{
 setprg32r(0,A,V);
}

void setchr1r(int r, unsigned int A, unsigned int V)
{
  if(!CHRptr[r]) return;
  FCEUPPU_LineUpdate();
  V&=CHRmask1[r];
  if(CHRram[r])
   PPUCHRRAM|=(1<<(A>>10));
  else
   PPUCHRRAM&=~(1<<(A>>10));
  VPageR[(A)>>10]=&CHRptr[r][(V)<<10]-(A);
}

void setchr2r(int r, unsigned int A, unsigned int V)
{
  if(!CHRptr[r]) return;
  FCEUPPU_LineUpdate();
  V&=CHRmask2[r];
  VPageR[(A)>>10]=VPageR[((A)>>10)+1]=&CHRptr[r][(V)<<11]-(A);
  if(CHRram[r])
   PPUCHRRAM|=(3<<(A>>10));
  else
   PPUCHRRAM&=~(3<<(A>>10));
}

void setchr4r(int r, unsigned int A, unsigned int V)
{
  if(!CHRptr[r]) return;
  FCEUPPU_LineUpdate();
  V&=CHRmask4[r];
  VPageR[(A)>>10]=VPageR[((A)>>10)+1]=
  VPageR[((A)>>10)+2]=VPageR[((A)>>10)+3]=&CHRptr[r][(V)<<12]-(A);
  if(CHRram[r])
   PPUCHRRAM|=(15<<(A>>10));
  else
   PPUCHRRAM&=~(15<<(A>>10));
}

void setchr8r(int r, unsigned int V)
{
  int x;

  if(!CHRptr[r]) return;
  FCEUPPU_LineUpdate();
  V&=CHRmask8[r];
  for(x=7;x>=0;x--)
   VPageR[x]=&CHRptr[r][V<<13];
  if(CHRram[r])
   PPUCHRRAM|=(255);
  else
   PPUCHRRAM&=~(255);
}

void setchr1(unsigned int A, unsigned int V)
{
 setchr1r(0,A,V);
}

void setchr2(unsigned int A, unsigned int V)
{
 setchr2r(0,A,V);
}

void setchr4(unsigned int A, unsigned int V)
{
 setchr4r(0,A,V);
}

void setchr8(unsigned int V)
{
 setchr8r(0,V);
}

void setvram8(uint8 *p)
{
  int x;
  for(x=7;x>=0;x--)
   VPageR[x]=p;
  PPUCHRRAM|=255;
}

void setvram4(uint32 A, uint8 *p)
{
  int x;
  for(x=3;x>=0;x--)
   VPageR[(A>>10)+x]=p-A;
  PPUCHRRAM|=(15<<(A>>10));
}

void setvramb1(uint8 *p, uint32 A, uint32 b)
{
  FCEUPPU_LineUpdate();
  VPageR[A>>10]=p-A+(b<<10);
  PPUCHRRAM|=(1<<(A>>10));
}

void setvramb2(uint8 *p, uint32 A, uint32 b)
{
  FCEUPPU_LineUpdate();
  VPageR[(A>>10)]=VPageR[(A>>10)+1]=p-A+(b<<11);
  PPUCHRRAM|=(3<<(A>>10));
}

void setvramb4(uint8 *p, uint32 A, uint32 b)
{
  int x;

  FCEUPPU_LineUpdate();
  for(x=3;x>=0;x--)
   VPageR[(A>>10)+x]=p-A+(b<<12);
  PPUCHRRAM|=(15<<(A>>10));
}

void setvramb8(uint8 *p, uint32 b)
{
  int x;

  FCEUPPU_LineUpdate();
  for(x=7;x>=0;x--)
   VPageR[x]=p+(b<<13);
  PPUCHRRAM|=255;
}

/* This function can be called without calling SetupCartMirroring(). */

void setntamem(uint8 *p, int ram, uint32 b)
{
 FCEUPPU_LineUpdate();
 vnapage[b]=p;
 PPUNTARAM&=~(1<<b);
 if(ram)
  PPUNTARAM|=1<<b;
}

static int mirrorhard=0;
void setmirrorw(int a, int b, int c, int d)
{
 FCEUPPU_LineUpdate();
 vnapage[0]=NTARAM+a*0x400;
 vnapage[1]=NTARAM+b*0x400;
 vnapage[2]=NTARAM+c*0x400;
 vnapage[3]=NTARAM+d*0x400;
}

void setmirror(int t)
{
  FCEUPPU_LineUpdate();
  if(!mirrorhard)
  {
   switch(t)
   {
    case MI_H:
     vnapage[0]=vnapage[1]=NTARAM;vnapage[2]=vnapage[3]=NTARAM+0x400;
     break;
    case MI_V:
     vnapage[0]=vnapage[2]=NTARAM;vnapage[1]=vnapage[3]=NTARAM+0x400;
     break;
    case MI_0:
     vnapage[0]=vnapage[1]=vnapage[2]=vnapage[3]=NTARAM;
     break;
    case MI_1:
     vnapage[0]=vnapage[1]=vnapage[2]=vnapage[3]=NTARAM+0x400;
     break;
   }
  PPUNTARAM=0xF;
 }
}

void SetupCartMirroring(int m, int hard, uint8 *extra)
{
 if(m<4)
 {
  mirrorhard = 0;
  setmirror(m);
 }
 else
 {
  vnapage[0]=NTARAM;
  vnapage[1]=NTARAM+0x400;
  vnapage[2]=extra;
  vnapage[3]=extra+0x400;
  PPUNTARAM=0xF;
 }
 mirrorhard=hard;
}

static uint8 *GENIEROM=0;

/* Called when a game(file) is opened successfully. */
void OpenGenie(void)
{
	FILE *fp;
	int x;

	if(!GENIEROM)
	{
		char *fn;

		if(!(GENIEROM=(uint8 *)FCEU_malloc(4096+1024))) return;

		fn=FCEU_MakeFName(FCEUMKF_GGROM,0,0);
		fp=fopen(fn,"rb");
		if(!fp)
		{
			FCEU_PrintError("Error opening Game Genie ROM image!");
			free(GENIEROM);
			GENIEROM=0;
			fclose(fp);
			return;
		}
		if(fread(GENIEROM,1,16,fp)!=16)
		{
grerr:
			FCEU_PrintError("Error reading from Game Genie ROM image!");
			free(GENIEROM);
			GENIEROM=0;
			fclose(fp);
			return;
		}
		if(GENIEROM[0]==0x4E)  /* iNES ROM image */
		{
			if(fread(GENIEROM,1,4096,fp)!=4096)
				goto grerr;
			if(fseek(fp,16384-4096,SEEK_CUR))
				goto grerr;
			if(fread(GENIEROM+4096,1,256,fp)!=256)
				goto grerr;
		}
		else
		{
			if(fread(GENIEROM+16,1,4352-16,fp)!=(4352-16))
				goto grerr;
		}
		fclose(fp);

		/* Workaround for the FCE Ultra CHR page size only being 1KB */
		for(x=0;x<4;x++)
			memcpy(GENIEROM+4096+(x<<8),GENIEROM+4096,256);
	}

	geniestage=1;
}

/* Called when a game is closed. */
void CloseGenie(void)
{
 /* No good reason to free() the Game Genie ROM image data. */
 geniestage=0;
 FlushGenieRW();
 VPageR=VPage;
}

void FCEU_KillGenie(void)
{
	if(GENIEROM)
	{
		free(GENIEROM);
		GENIEROM=0;
	}
}

static readfunc GenieBackup[3];

static DECLFR(GenieRead)
{
 return GENIEROM[A&4095];
}

static DECLFR(GenieFix3)
{
	uint8 r=GenieBackup[2](A);

	if((modcon>>3)&1)        /* No check*/
		return genieval[2];
	else if(r==geniech[2])
		return genieval[2];

	return r;
}

static DECLFR(GenieFix2)
{
	uint8 r=GenieBackup[1](A);

	if((modcon>>2)&1)        /* No check*/
		return genieval[1];
	else if(r==geniech[1])
		return genieval[1];

	return r;
}

static DECLFR(GenieFix1)
{
	uint8 r=GenieBackup[0](A);

	if((modcon>>1)&1)    /* No check*/
		return genieval[0];
	else if(r==geniech[0])
		return genieval[0];

	return r;
}

static void FixGenieMap(void)
{
	int x;

	geniestage=2;

	for(x=0;x<8;x++)
		VPage[x]=VPageG[x];

	VPageR=VPage;
	FlushGenieRW();
	for(x=0;x<3;x++)
		if((modcon>>(4+x))&1)
		{
			readfunc tmp[3]={GenieFix1,GenieFix2,GenieFix3};
			GenieBackup[x]=GetReadHandler(genieaddr[x]);
			SetReadHandler(genieaddr[x],genieaddr[x],tmp[x]);
		}
}

static DECLFW(GenieWrite)
{
	switch(A)
	{
		case 0x800c:
		case 0x8008:
		case 0x8004:genieval[((A-4)&0xF)>>2]=V;break;

		case 0x800b:
		case 0x8007:
		case 0x8003:geniech[((A-3)&0xF)>>2]=V;break;

		case 0x800a:
		case 0x8006:
		case 0x8002:genieaddr[((A-2)&0xF)>>2]&=0xFF00;genieaddr[((A-2)&0xF)>>2]|=V;break;

		case 0x8009:
		case 0x8005:
		case 0x8001:genieaddr[((A-1)&0xF)>>2]&=0xFF;genieaddr[((A-1)&0xF)>>2]|=(V|0x80)<<8;break;

		case 0x8000:if(!V)
				    FixGenieMap();
			    else
			    {
				    modcon=V^0xFF;
				    if(V==0x71)
					    modcon=0;
			    }
			    break;
	}
}

void GeniePower(void)
{
	uint32 x;

	if(!geniestage)
		return;

	geniestage=1;
	for(x=0;x<3;x++)
	{
		genieval[x]=0xFF;
		geniech[x]=0xFF;
		genieaddr[x]=0xFFFF;
	}
	modcon=0;

	SetWriteHandler(0x8000,0xFFFF,GenieWrite);
	SetReadHandler(0x8000,0xFFFF,GenieRead);

	for(x=0;x<8;x++)
		VPage[x]=GENIEROM+4096-0x400*x;

	if(AllocGenieRW())
		VPageR=VPageG;
	else
		geniestage=2;
}


void FCEU_SaveGameSave(CartInfo *LocalHWInfo)
{
	if(LocalHWInfo->battery && LocalHWInfo->SaveGame[0])
	{
		FILE *sp;
		const char * soot=FCEU_MakeFName(FCEUMKF_SAV,0,"sav");
		if((sp=fopen(soot,"wb"))==NULL)
		{
			FCEU_PrintError("WRAM file \"%s\" cannot be written to.\n",soot);
		}
		else
		{
			int x;

			for(x=0;x<4;x++)
				if(LocalHWInfo->SaveGame[x])
					fwrite(LocalHWInfo->SaveGame[x],1, LocalHWInfo->SaveGameLen[x],sp);

			fclose(sp);
		}
		free(soot);
	}
}

void FCEU_LoadGameSave(CartInfo *LocalHWInfo)
{
	if(LocalHWInfo->battery && LocalHWInfo->SaveGame[0])
	{
		FILE *sp;
		const char * soot= FCEU_MakeFName(FCEUMKF_SAV,0,"sav");
		sp= fopen(soot,"rb");
		if(sp!=NULL)
		{
			int x;
			for(x=0;x<4;x++)
				if(LocalHWInfo->SaveGame[x])
					fread(LocalHWInfo->SaveGame[x],1,LocalHWInfo->SaveGameLen[x],sp);

			fclose(sp);
		}
		free(soot);
	}
}
