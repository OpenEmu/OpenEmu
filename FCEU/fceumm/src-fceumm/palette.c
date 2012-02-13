/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002,2003 Xodnizel
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "types.h"
#include "fceu.h"
#include "general.h"
#include "driver.h"

#include "palette.h"
#include "palettes/palettes.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int ntsccol=0;
static int ntsctint=46+10;
static int ntschue=72;

/* These are dynamically filled/generated palettes: */
pal palettei[64];       /* Custom palette for an individual game.*/
pal palettec[64];       /* Custom "global" palette.*/
pal paletten[64];       /* Mathematically generated palette.*/

uint8 pale=0;

pal *palo;
static pal *palpoint[8]=
{
	palette,
	rp2c04001,
	rp2c04002,
	rp2c04003,
	rp2c05004,
};


void FCEUI_SetPaletteArray(uint8 *pal)
{
	if(!pal)
		palpoint[0]=palette;
	else
	{
		int x;

		palpoint[0]=palettec;
		for(x=0;x<64;x++)
		{
			palpoint[0][x].r=*((uint8 *)pal+x+x+x);
			palpoint[0][x].g=*((uint8 *)pal+x+x+x+1);
			palpoint[0][x].b=*((uint8 *)pal+x+x+x+2);
		}
	}
	FCEU_ResetPalette();
}


void FCEUI_SetNTSCTH(int n, int tint, int hue)
{
	ntsctint=tint;
	ntschue=hue;
	ntsccol=n;
	FCEU_ResetPalette();
}


static uint8 lastd=0;
void SetNESDeemph(uint8 d, int force)
{
	static uint16 rtmul[7]={32768*1.239,32768*.794,32768*1.019,32768*.905,32768*1.023,32768*.741,32768*.75};
	static uint16 gtmul[7]={32768*.915,32768*1.086,32768*.98,32768*1.026,32768*.908,32768*.987,32768*.75};
	static uint16 btmul[7]={32768*.743,32768*.882,32768*.653,32768*1.277,32768*.979,32768*.101,32768*.75};
	uint32 r,g,b;
	int x;

	/* If it's not forced(only forced when the palette changes),
	   don't waste cpu time if the same deemphasis bits are set as the last call.
	 */
	if(!force)
	{
		if(d==lastd)
			return;
	}
	else   /* Only set this when palette has changed. */
	{
		r=rtmul[6];
		g=rtmul[6];
		b=rtmul[6];

		for(x=0;x<0x40;x++)
		{
			uint32 m,n,o;
			m=palo[x].r;
			n=palo[x].g;
			o=palo[x].b;
			m=(m*r)>>15;
			n=(n*g)>>15;
			o=(o*b)>>15;
			if(m>0xff) m=0xff;
			if(n>0xff) n=0xff;
			if(o>0xff) o=0xff;
			FCEUD_SetPalette(x|0xC0,m,n,o);
		}
	}
	if(!d) return; /* No deemphasis, so return. */

	r=rtmul[d-1];
	g=gtmul[d-1];
	b=btmul[d-1];

	for(x=0;x<0x40;x++)
	{
		uint32 m,n,o;

		m=palo[x].r;
		n=palo[x].g;
		o=palo[x].b;
		m=(m*r)>>15;
		n=(n*g)>>15;
		o=(o*b)>>15;
		if(m>0xff) m=0xff;
		if(n>0xff) n=0xff;
		if(o>0xff) o=0xff;

		FCEUD_SetPalette(x|0x40,m,n,o);
	}

	lastd=d;
}

void WritePalette(void)
{
	int x;

	for(x=0;x<7;x++)
		FCEUD_SetPalette(x,unvpalette[x].r,unvpalette[x].g,unvpalette[x].b);
	for(x=0;x<64;x++)
		FCEUD_SetPalette(128+x,palo[x].r,palo[x].g,palo[x].b);
	SetNESDeemph(lastd,1);
}


/* Converted from Kevin Horton's qbasic palette generator. */
static void CalculatePalette(void)
{
	int x,z;
	int r,g,b;
	double s,luma,theta;
	static uint8 cols[16]={0,24,21,18,15,12,9,6,3,0,33,30,27,0,0,0};
	static uint8 br1[4]={6,9,12,12};
	static double br2[4]={.29,.45,.73,.9};
	static double br3[4]={0,.24,.47,.77};

	for(x=0;x<=3;x++)
		for(z=0;z<16;z++)
		{
			s=(double)ntsctint/128;
			luma=br2[x];
			if(z==0)  {s=0;luma=((double)br1[x])/12;}

			if(z>=13)
			{
				s=luma=0;
				if(z==13)
					luma=br3[x];
			}

			theta=(double)M_PI*(double)(((double)cols[z]*10+ (((double)ntschue/2)+300) )/(double)180);
			r=(int)((luma+s*sin(theta))*256);
			g=(int)((luma-(double)27/53*s*sin(theta)+(double)10/53*s*cos(theta))*256);
			b=(int)((luma-s*cos(theta))*256);


			if(r>255) r=255;
			if(g>255) g=255;
			if(b>255) b=255;
			if(r<0) r=0;
			if(g<0) g=0;
			if(b<0) b=0;

			paletten[(x<<4)+z].r=r;
			paletten[(x<<4)+z].g=g;
			paletten[(x<<4)+z].b=b;
		}
	WritePalette();
}

static int ipalette=0;

void FCEU_LoadGamePalette(void)
{
	uint8 ptmp[192];
	FILE *fp;
	char * fn;

	ipalette=0;

	fn=FCEU_MakeFName(FCEUMKF_PALETTE,0,0);

	if((fp=fopen(fn,"rb")))
	{
		int x;
		fread(ptmp,1,192,fp);
		fclose(fp);
		for(x=0;x<64;x++)
		{
			palettei[x].r=ptmp[x+x+x];
			palettei[x].g=ptmp[x+x+x+1];
			palettei[x].b=ptmp[x+x+x+2];
		}
		ipalette=1;
	}
	free(fn);
}

static void ChoosePalette(void)
{
	if(ipalette)
		palo=palettei;
	else if(ntsccol && !PAL && FCEUGameInfo->type!=GIT_VSUNI)
	{
		palo=paletten;
		CalculatePalette();
	}
	else
		palo=palpoint[pale];
}

void FCEU_ResetPalette(void)
{
	if(FCEUGameInfo)
	{
		ChoosePalette();
		WritePalette();
	}
}

void FCEUI_GetNTSCTH(int *tint, int *hue)
{
	*tint=ntsctint;
	*hue=ntschue;
}

static int controlselect=0;
static int controllength=0;

void FCEUI_NTSCDEC(void)
{
	if(ntsccol && FCEUGameInfo->type!=GIT_VSUNI &&!PAL)
	{
		int which;
		if(controlselect)
		{
			if(controllength)
			{
				which=controlselect==1?ntschue:ntsctint;
				which--;
				if(which<0) which=0;
				if(controlselect==1)
					ntschue=which;
				else ntsctint=which;
				CalculatePalette();
			}
			controllength=360;
		}
	}
}

void FCEUI_NTSCINC(void)
{
	if(ntsccol && FCEUGameInfo->type!=GIT_VSUNI && !PAL)
		if(controlselect)
		{
			if(controllength)
			{
				switch(controlselect)
				{
					case 1:ntschue++;
					       if(ntschue>128) ntschue=128;
					       CalculatePalette();
					       break;
					case 2:ntsctint++;
					       if(ntsctint>128) ntsctint=128;
					       CalculatePalette();
					       break;
				}
			}
			controllength=360;
		}
}

void FCEUI_NTSCSELHUE(void)
{
	if(ntsccol && FCEUGameInfo->type!=GIT_VSUNI && !PAL)
	{
		controlselect = 1;
		controllength = 360;
	}
}

void FCEUI_NTSCSELTINT(void)
{
	if(ntsccol && FCEUGameInfo->type!=GIT_VSUNI && !PAL)
	{
		controlselect=2;
		controllength=360;
	}
}
