/* Mednafen - Multi-system Emulator
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

#include "../nes.h"

#include "palette.h"
#include "palettes/palettes.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* These are dynamically filled/generated palettes: */
MDFNPalStruct palettei[64];       // Custom palette for an individual game.
MDFNPalStruct palettec[64];       // Custom "global" palette.  

static void ChoosePalette(void);
static void WritePalette(void);
static uint8 pale = 0;

MDFNPalStruct *palo;
int MDFNPaletteChanged = 0;
MDFNPalStruct MDFNPalette[256];

static MDFNPalStruct *palpoint[8]=
{
     palette,
     rp2c04001,
     rp2c04002,
     rp2c04003,
     rp2c05004,
};

static uint8 lastd=0;
void SetNESDeemph(uint8 d, int force)
{
 static const double rtmul[7]={1.239,.794,1.019,.905,1.023,.741,.75};
 static const double gtmul[7]={.915,1.086,.98,1.026,.908,.987,.75};
 static const double btmul[7]={.743,.882,.653,1.277,.979,.101,.75};
 double r,g,b;
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
   double m,n,o;

   m =r * palo[x].r;
   n =g * palo[x].g; 
   o =b * palo[x].b; 

   if(m>0xff) m=0xff;
   if(n>0xff) n=0xff;
   if(o>0xff) o=0xff;

   MDFNPalette[x | 0xc0].r = (uint8)m;
   MDFNPalette[x | 0xc0].g = (uint8)n;
   MDFNPalette[x | 0xc0].b = (uint8)o;
  }
  MDFNPaletteChanged = 1;
 } 
 if(!d) return; /* No deemphasis, so return. */

 r=rtmul[d-1];
 g=gtmul[d-1];
 b=btmul[d-1];

 for(x=0;x<0x40;x++)
 {
  double m,n,o;
  
  m = r * palo[x].r;
  n = g * palo[x].g;
  o = b * palo[x].b;

  if(m>0xff) m=0xff;
  if(n>0xff) n=0xff;
  if(o>0xff) o=0xff;
    
  MDFNPalette[x | 0x40].r = (uint8)m;
  MDFNPalette[x | 0x40].g = (uint8)n;
  MDFNPalette[x | 0x40].b = (uint8)o;
 }
 MDFNPaletteChanged = 1;     
 lastd=d;
}
 
static int ipalette=0; 
          
void MDFN_LoadGamePalette(void)
{
  uint8 ptmp[192];
  FILE *fp;

  std::string cpalette_fn = MDFN_GetSettingS("nes.cpalette");

  if(cpalette_fn != "")
  {
   fp = fopen(cpalette_fn.c_str(), "rb");
   fread(ptmp, 1, 192, fp);
   fclose(fp);
   palpoint[0]=palettec;
   for(int x=0;x<64;x++)
   {
    palpoint[0][x].r=ptmp[x * 3];
    palpoint[0][x].g=ptmp[x * 3 + 1];
    palpoint[0][x].b=ptmp[x * 3 + 2];
   }
  }

  ipalette=0;
  if((fp=fopen(MDFN_MakeFName(MDFNMKF_PALETTE,0,0).c_str(),"rb")))
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
}

void MDFN_ResetPalette(void)
{
 if(MDFNGameInfo)
 {
   ChoosePalette();
   WritePalette(); 
 }
}
 
static void ChoosePalette(void)
{
    if(MDFNGameInfo->GameType == GMT_PLAYER)
     palo=0;
    else if(ipalette)
     palo=palettei;  
    else
     palo=palpoint[pale];
}

void MDFN_SetPPUPalette(int fishie)
{
 pale = fishie;
}


/* Returns Mednafen's default GUI palette. */
MDFNPalStruct *MDFNI_GetDefaultPalette(void)
{
 return(unvpalette);
}

void WritePalette(void)
{
    int x;
    
    /* Entries 0-64 are colorsused by GUI components. */
    for(x=0;x<64;x++)
    {
     MDFNPalette[x].r = unvpalette[x].r;
     MDFNPalette[x].g = unvpalette[x].g;
     MDFNPalette[x].b = unvpalette[x].b;
    }

    if(MDFNGameInfo->GameType == GMT_PLAYER)
    {
     //for(x=0;x<128;x++)
     // MDFND_SetPalette(x,x,0,x);
    }
    else
    {   
     for(x=0;x<64;x++)
     {
      MDFNPalette[0x80 + x].r = palo[x].r;
      MDFNPalette[0x80 + x].g = palo[x].g;
      MDFNPalette[0x80 + x].b = palo[x].b;
     }
     SetNESDeemph(lastd,1);
    }
    MDFNPaletteChanged = 1;
}

