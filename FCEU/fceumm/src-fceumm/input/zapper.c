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

#include        <string.h>
#include        <stdlib.h>

#include        "share.h"

typedef struct {
        uint32 mzx,mzy,mzb;
        int zap_readbit;
        int bogo;
        int zappo;
        uint64 zaphit;
} ZAPPER;

static ZAPPER ZD[2];

static void ZapperFrapper(int w, uint8 *bg, uint8 *spr, uint32 linets, int final)
{
 int xs,xe;
 int zx,zy;

 if(!bg) /* New line, so reset stuff.*/
 {
  ZD[w].zappo=0;
  return;
 }
 xs=ZD[w].zappo;
 xe=final;

 zx=ZD[w].mzx;
 zy=ZD[w].mzy;

 if(xe>256) xe=256;

 if(scanline>=(zy-4) && scanline<=(zy+4))
 {
  while(xs<xe)
  {
    uint8 a1,a2;
    uint32 sum;
    if(xs<=(zx+4) && xs>=(zx-4))
    {
     a1=bg[xs];
     if(spr)
     {
      a2=spr[xs];

      if(!(a2&0x80))
       if(!(a2&0x40) || (a1&64))
        a1=a2;
     }
     a1&=63;

     sum=palo[a1].r+palo[a1].g+palo[a1].b;
     if(sum>=100*3)
     {
      ZD[w].zaphit=((uint64)linets+(xs+16)*(PAL?15:16))/48+timestampbase;
      goto endo;
     }
    }
   xs++;
  }
 }
 endo:
 ZD[w].zappo=final;
}

static INLINE int CheckColor(int w)
{
 FCEUPPU_LineUpdate();

 if((ZD[w].zaphit+100)>=(timestampbase+timestamp)
  && !(ZD[w].mzb&2)) return(0);

 return(1);
}

static uint8 ReadZapperVS(int w)
{
	uint8 ret=0;

	if(ZD[w].zap_readbit==4) ret=1;

	if(ZD[w].zap_readbit==7)
	{
		if(ZD[w].bogo)
			ret|=0x1;
	}
	if(ZD[w].zap_readbit==6)
	{
		if(!CheckColor(w))
			ret|=0x1;
	}
	ZD[w].zap_readbit++;
	return ret;
}

static void StrobeZapperVS(int w)
{
	ZD[w].zap_readbit=0;
}

static uint8 ReadZapper(int w)
{
	uint8 ret=0;
	if(ZD[w].bogo)
		ret|=0x10;
	if(CheckColor(w))
		ret|=0x8;
	return ret;
}

static void DrawZapper(int w, uint8 *buf, int arg)
{
	if(arg)
		FCEU_DrawGunSight(buf, ZD[w].mzx,ZD[w].mzy);
}

static void UpdateZapper(int w, void *data, int arg)
{
	uint32 *ptr=(uint32 *)data;

	/*FCEU_DispMessage("%3d:%3d",ZD[w].mzx,ZD[w].mzy);*/
	if(ZD[w].bogo)
		ZD[w].bogo--;
	if(ptr[2]&3 && (!(ZD[w].mzb&3)))
		ZD[w].bogo=5;

	ZD[w].mzx=ptr[0];
	ZD[w].mzy=ptr[1];
	ZD[w].mzb=ptr[2];
}

static INPUTC ZAPC={ReadZapper,0,0,UpdateZapper,ZapperFrapper,DrawZapper};
static INPUTC ZAPVSC={ReadZapperVS,0,StrobeZapperVS,UpdateZapper,ZapperFrapper,DrawZapper};

INPUTC *FCEU_InitZapper(int w)
{
	memset(&ZD[w],0,sizeof(ZAPPER));
	if(FCEUGameInfo->type == GIT_VSUNI)
		return(&ZAPVSC);
	else
		return(&ZAPC);
}


