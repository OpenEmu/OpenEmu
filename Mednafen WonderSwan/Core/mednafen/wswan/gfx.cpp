/* Cygne
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Dox dox@space.pl
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

#include "wswan.h"
#include "gfx.h"
#include "memory.h"
#include "v30mz.h"
#include "rtc.h"
#include "../video.h"
#include <trio/trio.h>

namespace MDFN_IEN_WSWAN
{

static uint32 wsMonoPal[16][4];
static uint32 wsColors[8];
static uint32 wsCols[16][16];

static uint32 ColorMapG[16];
static uint32 ColorMap[16*16*16];
static uint32 LayerEnabled;

static uint8 wsLine;                 /*current scanline*/

static uint8 SpriteTable[0x80][4];
static uint32 SpriteCountCache;
static uint8 DispControl;
static uint8 BGColor;
static uint8 LineCompare;
static uint8 SPRBase;
static uint8 SpriteStart, SpriteCount;
static uint8 FGBGLoc;
static uint8 FGx0, FGy0, FGx1, FGy1;
static uint8 SPRx0, SPRy0, SPRx1, SPRy1;

static uint8 BGXScroll, BGYScroll;
static uint8 FGXScroll, FGYScroll;
static uint8 LCDControl, LCDIcons;

static uint8 BTimerControl;
static uint16 HBTimerPeriod;
static uint16 VBTimerPeriod;

static uint16 HBCounter, VBCounter;
static uint8 VideoMode;

#ifdef WANT_DEBUGGER

RegType WSwanGfxRegs[] =
{
 { 0, "DispControl", "Display Control", 1 },
 { 0, "VideoMode", "Video Mode", 1 },
 { 0, "LCDControl", "LCD Control", 1 },
 { 0, "LCDIcons", "LCD Icons", 1 },
 { 0, "BTimerControl", "VB/HB Timer Control", 1 },
 { 0, "HBTimerPeriod", "Horizontal blank timer counter period", 2 },
 { 0, "VBTimerPeriod", "Vertical blank timer counter period", 2 },
 { 0, "HBCounter", "Horizontal blank counter", 1 },
 { 0, "VBCounter", "Vertical blank counter", 1 },
 { 0, "BGColor", "Background Color", 1 },
 { 0, "LineCompare", "Line Compare", 1 },
 { 0, "SPRBase", "Sprite Table Base", 1 },
 { 0, "SpriteStart", "SpriteStart", 1 },
 { 0, "SpriteCount", "SpriteCount", 1 },

 { 0, "FGBGLoc", "FG/BG Map Location", 1 },

 { 0, "FGx0", "Foreground Window X0", 1 }, 
 { 0, "FGy0", "Foreground Window Y0", 1 },
 { 0, "FGx1", "Foreground Window X1", 1 },
 { 0, "FGy1", "Foreground Window Y1", 1 },
 { 0, "SPRx0", "Sprite Window X0", 1 },
 { 0, "SPRy0", "Sprite Window Y0", 1 },
 { 0, "SPRx1", "Sprite Window X1", 1 },
 { 0, "SPRy1", "Sprite Window Y1", 1 },

 { 0, "BGXScroll", "Background X Scroll", 1 },
 { 0, "BGYScroll", "Background Y Scroll", 1 },
 { 0, "FGXScroll", "Foreground X Scroll", 1 },
 { 0, "FGYScroll", "Foreground Y Scroll", 1 },
 { 0, "", "", 0 },
};

uint32 WSwan_GfxGetRegister(const std::string &oname, std::string *special)
{
 if(oname == "DispControl")
  return(DispControl);
 if(oname == "BGColor")
  return(BGColor);
 if(oname == "LineCompare")
  return(LineCompare);
 if(oname == "SPRBase")
 {
  if(special)
  {
   char tmpstr[256];
   trio_snprintf(tmpstr, 256, "0x%02x * 0x200 = 0x%04x", SPRBase, SPRBase * 0x200);
   *special = std::string(tmpstr);
  }
  return(SPRBase);
 }
 if(oname == "SpriteStart")
  return(SpriteStart);
 if(oname == "SpriteCount")
  return(SpriteCount);
 if(oname == "FGBGLoc")
  return(FGBGLoc);
 if(oname == "FGx0")
  return(FGx0);
 if(oname == "FGy0")
  return(FGy0);
 if(oname == "FGx1")
  return(FGx1);
 if(oname == "FGy1")
  return(FGy1);
 if(oname == "SPRx0")
  return(SPRx0);
 if(oname == "SPRy0")
  return(SPRy0);
 if(oname == "SPRx1")
  return(SPRx1);
 if(oname == "SPRy1")
  return(SPRy1);
 if(oname == "BGXScroll")
  return(BGXScroll);
 if(oname == "BGYScroll")
  return(BGYScroll);
 if(oname == "FGXScroll")
  return(FGXScroll);
 if(oname == "FGYScroll")
  return(FGYScroll);
 if(oname == "LCDIcons")
  return(LCDIcons);
 if(oname == "LCDControl")
  return(LCDControl);
 if(oname == "BTimerControl")
  return(BTimerControl);
 if(oname == "HBTimerPeriod")
  return(HBTimerPeriod);
 if(oname == "VBTimerPeriod")
  return(VBTimerPeriod);
 if(oname == "HBCounter")
  return(HBCounter);
 if(oname == "VBCounter")
  return(VBCounter);
 if(oname == "VideoMode")
  return(VideoMode);

 return(0);
}

void WSwan_GfxSetRegister(const std::string &oname, uint32 value)
{
 if(oname == "DispControl")
  DispControl = value;
 if(oname == "BGColor")
  BGColor = value;
 if(oname == "LineCompare")
  LineCompare = value;
 if(oname == "SPRBase")
  SPRBase = value;
 if(oname == "SpriteStart")
  SpriteStart = value;
 if(oname == "SpriteCount")
  SpriteCount = value;
 if(oname == "FGBGLoc")
  FGBGLoc = value;
 if(oname == "FGx0")
  FGx0 = value;
 if(oname == "FGy0")
  FGy0 = value;
 if(oname == "FGx1")
  FGx1 = value;
 if(oname == "FGy1")
  FGy1 = value;
 if(oname == "SPRx0")
  SPRx0 = value;
 if(oname == "SPRy0")
  SPRy0 = value;
 if(oname == "SPRx1")
  SPRx1 = value;
 if(oname == "SPRy1")
  SPRy1 = value;
 if(oname == "BGXScroll")
  BGXScroll = value;
 if(oname == "BGYScroll")
  BGYScroll = value;
 if(oname == "FGXScroll")
  FGXScroll = value;
 if(oname == "FGYScroll")
  FGYScroll = value;
 if(oname == "LCDIcons")
  LCDIcons = value;
 if(oname == "LCDControl")
  LCDControl = value;
 if(oname == "BTimerControl")
  BTimerControl = value;
 if(oname == "HBTimerPeriod")
  HBTimerPeriod = value;
 if(oname == "VBTimerPeriod")
  VBTimerPeriod = value;
 if(oname == "HBCounter")
  HBCounter = value;
 if(oname == "VBCounter")
  VBCounter = value;
 if(oname == "VideoMode")
 {
  VideoMode = value;
  wsSetVideo(VideoMode >> 5, false);
 }
}

static RegGroupType WSwanGfxRegsGroup =
{
 "Gfx",
 WSwanGfxRegs,
 NULL,
 NULL,
 WSwan_GfxGetRegister,
 WSwan_GfxSetRegister,
};


static void DoGfxDecode(void);
static MDFN_Surface *GfxDecode_Buf = NULL;
static int GfxDecode_Line = -1;
static int GfxDecode_Layer = 0;
static int GfxDecode_Scroll = 0;
static int GfxDecode_Pbn = 0;
#endif

void WSwan_GfxInit(void)
{
 LayerEnabled = 7; // BG, FG, sprites
 #ifdef WANT_DEBUGGER
 MDFNDBG_AddRegGroup(&WSwanGfxRegsGroup);
 #endif
}

void WSwan_GfxWSCPaletteRAMWrite(uint32 ws_offset, uint8 data)
{
 ws_offset=(ws_offset&0xfffe)-0xfe00;
 wsCols[(ws_offset>>1)>>4][(ws_offset>>1)&15] = wsRAM[ws_offset+0xfe00] | ((wsRAM[ws_offset+0xfe01]&0x0f) << 8);
}

void WSwan_GfxWrite(uint32 A, uint8 V)
{
 if(A >= 0x1C && A <= 0x1F)
 {
  wsColors[(A - 0x1C) * 2 + 0] = 0xF - (V & 0xf);
  wsColors[(A - 0x1C) * 2 + 1] = 0xF - (V >> 4);
 }
 else if(A >= 0x20 && A <= 0x3F)
 {
  wsMonoPal[(A - 0x20) >> 1][((A & 0x1) << 1) + 0] = V&7;
  wsMonoPal[(A - 0x20) >> 1][((A & 0x1) << 1) | 1] = (V>>4)&7;
 }
 else switch(A)
 {
  case 0x00: DispControl = V; break;
  case 0x01: BGColor = V; break;
  case 0x03: LineCompare = V; break;
  case 0x04: SPRBase = V & 0x3F; break;
  case 0x05: SpriteStart = V; break;
  case 0x06: SpriteCount = V; break;
  case 0x07: FGBGLoc = V; break;
  case 0x08: FGx0 = V; break;
  case 0x09: FGy0 = V; break;
  case 0x0A: FGx1 = V; break;
  case 0x0B: FGy1 = V; break;
  case 0x0C: SPRx0 = V; break;
  case 0x0D: SPRy0 = V; break;
  case 0x0E: SPRx1 = V; break;
  case 0x0F: SPRy1 = V; break;
  case 0x10: BGXScroll = V; break;
  case 0x11: BGYScroll = V; break;
  case 0x12: FGXScroll = V; break;
  case 0x13: FGYScroll = V; break;

  case 0x14: LCDControl = V; break; //    if((!(wsIO[0x14]&1))&&(data&1)) { wsLine=0; }break; /* LCD off ??*/
  case 0x15: LCDIcons = V; break;

  case 0x60: VideoMode = V; 
	     wsSetVideo(V>>5, false); 
	     //printf("VideoMode: %02x, %02x\n", V, V >> 5);
	     break;

  case 0xa2: if((V & 0x01) && !(BTimerControl & 0x01))
	      HBCounter = HBTimerPeriod;
	     if((V & 0x04) && !(BTimerControl & 0x04))
	      VBCounter = VBTimerPeriod;
	     BTimerControl = V; 
	     //printf("%04x:%02x\n", A, V);
	     break;
  case 0xa4: HBTimerPeriod &= 0xFF00; HBTimerPeriod |= (V << 0); /*printf("%04x:%02x, %d\n", A, V, wsLine);*/ break;
  case 0xa5: HBTimerPeriod &= 0x00FF; HBTimerPeriod |= (V << 8); HBCounter = HBTimerPeriod; /*printf("%04x:%02x, %d\n", A, V, wsLine);*/ break;
  case 0xa6: VBTimerPeriod &= 0xFF00; VBTimerPeriod |= (V << 0); /*printf("%04x:%02x, %d\n", A, V, wsLine);*/ break;
  case 0xa7: VBTimerPeriod &= 0x00FF; VBTimerPeriod |= (V << 8); VBCounter = VBTimerPeriod; /*printf("%04x:%02x, %d\n", A, V, wsLine);*/ break;
  //default: printf("%04x:%02x\n", A, V); break;
 }
}

uint8 WSwan_GfxRead(uint32 A)
{
 if(A >= 0x1C && A <= 0x1F)
 {
  uint8 ret = 0;

  ret |= 0xF - wsColors[(A - 0x1C) * 2 + 0];
  ret |= (0xF - wsColors[(A - 0x1C) * 2 + 1]) << 4;

  return(ret);
 }
 else if(A >= 0x20 && A <= 0x3F)
 {
  uint8 ret = wsMonoPal[(A - 0x20) >> 1][((A & 0x1) << 1) + 0] | (wsMonoPal[(A - 0x20) >> 1][((A & 0x1) << 1) | 1] << 4);

  return(ret);
 }
 else switch(A)
 {
  case 0x00: return(DispControl);
  case 0x01: return(BGColor);
  case 0x02: return(wsLine);
  case 0x03: return(LineCompare);
  case 0x04: return(SPRBase);
  case 0x05: return(SpriteStart);
  case 0x06: return(SpriteCount);
  case 0x07: return(FGBGLoc);
  case 0x08: return(FGx0); break;
  case 0x09: return(FGy0); break;
  case 0x0A: return(FGx1); break;
  case 0x0B: return(FGy1); break;
  case 0x0C: return(SPRx0); break;
  case 0x0D: return(SPRy0); break;
  case 0x0E: return(SPRx1); break;
  case 0x0F: return(SPRy1); break;
  case 0x10: return(BGXScroll);
  case 0x11: return(BGYScroll);
  case 0x12: return(FGXScroll);
  case 0x13: return(FGYScroll);
  case 0x14: return(LCDControl);
  case 0x15: return(LCDIcons);
  case 0x60: return(VideoMode);
  case 0xa0: return(wsc ? 0x87 : 0x86);
  case 0xa2: return(BTimerControl);
  case 0xa4: return((HBTimerPeriod >> 0) & 0xFF);
  case 0xa5: return((HBTimerPeriod >> 8) & 0xFF);
  case 0xa6: return((VBTimerPeriod >> 0) & 0xFF);
  case 0xa7: return((VBTimerPeriod >> 8) & 0xFF);
  case 0xa8: /*printf("%04x\n", A);*/ return((HBCounter >> 0) & 0xFF);
  case 0xa9: /*printf("%04x\n", A);*/ return((HBCounter >> 8) & 0xFF);
  case 0xaa: /*printf("%04x\n", A);*/ return((VBCounter >> 0) & 0xFF);
  case 0xab: /*printf("%04x\n", A);*/ return((VBCounter >> 8) & 0xFF);
  default: return(0);
  //default: printf("GfxRead:  %04x\n", A); return(0);
 }
}

bool wsExecuteLine(MDFN_Surface *surface, bool skip)
{
        bool ret = FALSE;

         #ifdef WANT_DEBUGGER
         if(GfxDecode_Buf && GfxDecode_Line >=0 && wsLine == GfxDecode_Line)
          DoGfxDecode();
         #endif

	if(wsLine < 144)
	{
	 if(!skip)
          wsScanline(surface->pixels + wsLine * surface->pitch32);
	}

	WSwan_CheckSoundDMA();

        // Update sprite data table
        if(wsLine == 142)
        {
         SpriteCountCache = SpriteCount;

         if(SpriteCountCache > 0x80)
          SpriteCountCache = 0x80;

         memcpy(SpriteTable, &wsRAM[(SPRBase << 9) + (SpriteStart << 2)], SpriteCountCache << 2);
        }

        if(wsLine == 144)
        {
                ret = TRUE;
                WSwan_Interrupt(WSINT_VBLANK);
                //printf("VBlank: %d\n", wsLine);
        }


        if(HBCounter && (BTimerControl & 0x01))
        {
         HBCounter--;
         if(!HBCounter)
         {
          // Loop mode?
          if(BTimerControl & 0x02)
           HBCounter = HBTimerPeriod;
          WSwan_Interrupt(WSINT_HBLANK_TIMER);
         }
        }

        v30mz_execute(224);
	wsLine = (wsLine + 1) % 159;
        if(wsLine == LineCompare)
        {
         WSwan_Interrupt(WSINT_LINE_HIT);
         //printf("Line hit: %d\n", wsLine);
        }
	v30mz_execute(32);
	WSwan_RTCClock(256);

        if(!wsLine)
        {
                if(VBCounter && (BTimerControl & 0x04))
                {
                 VBCounter--;
                 if(!VBCounter)
                 {
                  if(BTimerControl & 0x08) // Loop mode?
                   VBCounter = VBTimerPeriod;

                  WSwan_Interrupt(WSINT_VBLANK_TIMER);
                 }
                }
		wsLine = 0;
        }
	//if(ret) puts("Frame");
        return(ret);
}

void WSwan_SetLayerEnableMask(uint64 mask)
{
 LayerEnabled = mask;
}

void WSwan_SetPixelFormat(const MDFN_PixelFormat &format)
{
 for(int r = 0; r < 16; r++)
  for(int g = 0; g < 16; g++)
   for(int b = 0; b < 16; b++)
   {
    uint32 neo_r, neo_g, neo_b;

    neo_r = r * 17;
    neo_g = g * 17;
    neo_b = b * 17;

    ColorMap[(r << 8) | (g << 4) | (b << 0)] = format.MakeColor(neo_r, neo_g, neo_b); //(neo_r << rs) | (neo_g << gs) | (neo_b << bs);
   }

 for(int i = 0; i < 16; i++)
 {
  uint32 neo_r, neo_g, neo_b;

  neo_r = (i) * 17;
  neo_g = (i) * 17;
  neo_b = (i) * 17;

  ColorMapG[i] = format.MakeColor(neo_r, neo_g, neo_b); //(neo_r << rs) | (neo_g << gs) | (neo_b << bs);
 }
}

void wsScanline(uint32 *target)
{
	uint32		start_tile_n,map_a,startindex,adrbuf,b1,b2,j,t,l;
	char		ys2;
	uint8		b_bg[256];
	uint8		b_bg_pal[256];

	if(!wsVMode)
		memset(b_bg, wsColors[BGColor&0xF]&0xF, 256);
	else
	{
		memset(&b_bg[0], BGColor & 0xF, 256);
		memset(&b_bg_pal[0], (BGColor>>4)  & 0xF, 256);
	}
	start_tile_n=(wsLine+BGYScroll)&0xff;/*First line*/
	map_a=(((uint32)(FGBGLoc&0xF))<<11)+((start_tile_n&0xfff8)<<3);
	startindex = BGXScroll >> 3; /*First tile in row*/
	adrbuf = 7-(BGXScroll&7); /*Pixel in tile*/
	
	if((DispControl & 0x01) && (LayerEnabled & 0x01)) /*BG layer*/
        {
	 for(t=0;t<29;t++)
	 {
	  b1=wsRAM[map_a+(startindex<<1)];
	  b2=wsRAM[map_a+(startindex<<1)+1];
	  uint32 palette=(b2>>1)&15;
	  b2=(b2<<8)|b1;
	  wsGetTile(b2&0x1ff,start_tile_n&7,b2&0x8000,b2&0x4000,b2&0x2000);

          if(wsVMode)
          {
           if(wsVMode & 0x2)
	   {
            for(int x = 0; x < 8; x++)
             if(wsTileRow[x])
             {
              b_bg[adrbuf + x] = wsTileRow[x];
              b_bg_pal[adrbuf + x] = palette;
             }
	   }
	   else
	   {
            for(int x = 0; x < 8; x++)
             if(wsTileRow[x] || !(palette & 0x4))
             {
              b_bg[adrbuf + x] = wsTileRow[x];
              b_bg_pal[adrbuf + x] = palette;
             }
	   }
          }
          else
          {
           for(int x = 0; x < 8; x++)
            if(wsTileRow[x] || !(palette & 4))
            {
             b_bg[adrbuf + x] = wsColors[wsMonoPal[palette][wsTileRow[x]]];
            }
          }
	  adrbuf += 8;
	  startindex=(startindex + 1)&31;
	 } // end for(t = 0 ...
	} // End BG layer drawing

	if((DispControl & 0x02) && (LayerEnabled & 0x02))/*FG layer*/
	{
	 uint8 windowtype = DispControl&0x30;
         bool in_window[256 + 8*2];

	 if(windowtype)
         {
          memset(in_window, 0, sizeof(in_window));

	  if(windowtype == 0x20) // Display FG only inside window
	  {
           if((wsLine >= FGy0) && (wsLine < FGy1))
            for(j = FGx0; j <= FGx1 && j < 224; j++)
              in_window[7 + j] = 1;
	  }
	  else if(windowtype == 0x30) // Display FG only outside window
	  {
	   for(j = 0; j < 224; j++)
	   {
	    if(!(j >= FGx0 && j < FGx1) || !((wsLine >= FGy0) && (wsLine < FGy1)))
	     in_window[7 + j] = 1;
	   }
 	  }
	  else
	  {
	   puts("Who knows!");
	  }
         }
         else
          memset(in_window, 1, sizeof(in_window));

	 start_tile_n=(wsLine+FGYScroll)&0xff;
	 map_a=(((uint32)((FGBGLoc>>4)&0xF))<<11)+((start_tile_n>>3)<<6);
	 startindex = FGXScroll >> 3;
	 adrbuf = 7-(FGXScroll&7);

         for(t=0; t<29; t++)
         {
          b1=wsRAM[map_a+(startindex<<1)];
          b2=wsRAM[map_a+(startindex<<1)+1];
          uint32 palette=(b2>>1)&15;
          b2=(b2<<8)|b1;
          wsGetTile(b2&0x1ff,start_tile_n&7,b2&0x8000,b2&0x4000,b2&0x2000);

          if(wsVMode)
          {
	   if(wsVMode & 0x2)
            for(int x = 0; x < 8; x++)
	    {
             if(wsTileRow[x] && in_window[adrbuf + x])
             {
              b_bg[adrbuf + x] = wsTileRow[x] | 0x10;
              b_bg_pal[adrbuf + x] = palette;
             }
	    }
	   else
            for(int x = 0; x < 8; x++)
	    {
             if((wsTileRow[x] || !(palette & 0x4)) && in_window[adrbuf + x])
             {
              b_bg[adrbuf + x] = wsTileRow[x] | 0x10;
              b_bg_pal[adrbuf + x] = palette;
             }
	    }
          }
          else
          {
           for(int x = 0; x < 8; x++)
            if((wsTileRow[x] || !(palette & 4)) && in_window[adrbuf + x])
            {
             b_bg[adrbuf + x] = wsColors[wsMonoPal[palette][wsTileRow[x]]] | 0x10;
            }
          }
          adrbuf += 8;
          startindex=(startindex + 1)&31;
         } // end for(t = 0 ...

	} // end FG drawing

	if((DispControl & 0x04) && SpriteCountCache && (LayerEnabled & 0x04))/*Sprites*/
	{
	  int xs,ts,as,ys,ysx,h;
	  bool in_window[256 + 8*2];

          if(DispControl & 0x08)
	  {
	   memset(in_window, 0, sizeof(in_window));
	   if((wsLine >= SPRy0) && (wsLine < SPRy1))
            for(j = SPRx0; j < SPRx1 && j < 256; j++)
	      in_window[7 + j] = 1;
	  }
	  else
	   memset(in_window, 1, sizeof(in_window));

		for(h = SpriteCountCache - 1; h >= 0; h--)
		{
			ts = SpriteTable[h][0];
			as = SpriteTable[h][1];
			ysx = SpriteTable[h][2];
			ys2 = (int8)SpriteTable[h][2];
			xs = SpriteTable[h][3];

			if(xs >= 249) xs -= 256;

			if(ysx > 150) 
			 ys = ys2;
			else 
			 ys = ysx;

			ys = wsLine - ys;

			if(ys >= 0 && ys < 8 && xs < 224)
			{
			 uint32 palette = ((as >> 1) & 0x7);
			 
			 ts |= (as&1) << 8;
			 wsGetTile(ts, ys, as & 0x80, as & 0x40, 0);

			 if(wsVMode)
			 {
			  if(wsVMode & 0x2)
			  {
			   for(int x = 0; x < 8; x++)
			    if(wsTileRow[x])
			    {
		             if((as & 0x20) || !(b_bg[xs + x + 7] & 0x10))
		             {
			      bool drawthis = 0;

			      if(!(DispControl & 0x08)) 
			       drawthis = TRUE;
			      else if((as & 0x10) && !in_window[7 + xs + x])
			       drawthis = TRUE;
			      else if(!(as & 0x10) && in_window[7 + xs + x])
			       drawthis = TRUE;

			      if(drawthis)
		              {
		               b_bg[xs + x + 7] = wsTileRow[x] | (b_bg[xs + x + 7] & 0x10);
		               b_bg_pal[xs + x + 7] = 8 + palette;
		              }
		             }
		            }
			  }
			  else
			  {
                           for(int x = 0; x < 8; x++)
                            if(wsTileRow[x] || !(palette & 0x4))
                            {
                             if((as & 0x20) || !(b_bg[xs + x + 7] & 0x10))
                             {
                              bool drawthis = 0;

                              if(!(DispControl & 0x08))
                               drawthis = TRUE;
                              else if((as & 0x10) && !in_window[7 + xs + x])
                               drawthis = TRUE;
                              else if(!(as & 0x10) && in_window[7 + xs + x])
                               drawthis = TRUE;

                              if(drawthis)
                              {
                               b_bg[xs + x + 7] = wsTileRow[x] | (b_bg[xs + x + 7] & 0x10);
                               b_bg_pal[xs + x + 7] = 8 + palette;
                              }
                             }
                            }

			  }

			 }
			 else
			 {
                          for(int x = 0; x < 8; x++)
                           if(wsTileRow[x] || !(palette & 4))
                           {
                            if((as & 0x20) || !(b_bg[xs + x + 7] & 0x10))
                            {
                             bool drawthis = 0;

                             if(!(DispControl & 0x08))
                              drawthis = TRUE;
                             else if((as & 0x10) && !in_window[7 + xs + x])
                              drawthis = TRUE;
                             else if(!(as & 0x10) && in_window[7 + xs + x])
                              drawthis = TRUE;

                             if(drawthis)
                             //if((as & 0x10) || in_window[7 + xs + x])
                             {
		              b_bg[xs + x + 7] = wsColors[wsMonoPal[8 + palette][wsTileRow[x]]] | (b_bg[xs + x + 7] & 0x10);
                             }
                            }
                           }

			 }
			}
		}

	}	// End sprite drawing

	if(wsVMode)
	{
	 for(l=0;l<224;l++)
	  target[l] = ColorMap[wsCols[b_bg_pal[l+7]][b_bg[(l+7)]&0xf]];
	}
	else
	{
	 for(l=0;l<224;l++)
 	  target[l] = ColorMapG[(b_bg[l+7])&15];
	}
}


void WSwan_GfxReset(void)
{
 wsLine=0;
 wsSetVideo(0,TRUE);

 memset(SpriteTable, 0, sizeof(SpriteTable));
 SpriteCountCache = 0;
 DispControl = 0;
 BGColor = 0;
 LineCompare = 0xBB;
 SPRBase = 0;

 SpriteStart = 0;
 SpriteCount = 0;
 FGBGLoc = 0;

 FGx0 = 0;
 FGy0 = 0;
 FGx1 = 0;
 FGy1 = 0;
 SPRx0 = 0;
 SPRy0 = 0;
 SPRx1 = 0;
 SPRy1 = 0;

 BGXScroll = BGYScroll = 0;
 FGXScroll = FGYScroll = 0;
 LCDControl = 0;
 LCDIcons = 0;

 BTimerControl = 0;
 HBTimerPeriod = 0;
 VBTimerPeriod = 0;

 HBCounter = 0;
 VBCounter = 0;


 for(int u0=0;u0<16;u0++)
  for(int u1=0;u1<16;u1++)
   wsCols[u0][u1]=0;

}

int WSwan_GfxStateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY32N(&wsMonoPal[0][0], 16 * 4, "wsMonoPal"),
  SFARRAY32(wsColors, 8),

  SFVAR(wsLine),

  SFARRAYN(&SpriteTable[0][0], 0x80 * 4, "SpriteTable"),
  SFVAR(SpriteCountCache),
  SFVAR(DispControl),
  SFVAR(BGColor),
  SFVAR(LineCompare),
  SFVAR(SPRBase),
  SFVAR(SpriteStart),
  SFVAR(SpriteCount),
  SFVAR(FGBGLoc),
  SFVAR(FGx0),
  SFVAR(FGy0),
  SFVAR(FGx1),
  SFVAR(FGy1),

  SFVAR(SPRx0),
  SFVAR(SPRy0),

  SFVAR(SPRx1),
  SFVAR(SPRy1),

  SFVAR(BGXScroll),
  SFVAR(BGYScroll),
  SFVAR(FGXScroll),
  SFVAR(FGYScroll),
  SFVAR(LCDControl),
  SFVAR(LCDIcons),

  SFVAR(BTimerControl),
  SFVAR(HBTimerPeriod),
  SFVAR(VBTimerPeriod),

  SFVAR(HBCounter),
  SFVAR(VBCounter),

  SFVAR(VideoMode),
  SFEND
 };

 if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, "GFX"))
  return(0);

 if(load)
 {
  wsSetVideo(VideoMode >> 5, TRUE);
 }

 return(1);
}

#ifdef WANT_DEBUGGER
static void DoGfxDecode(void)
{
 // FIXME
 uint32 *palette_ptr;
 uint32 *target = GfxDecode_Buf->pixels;
 int w = GfxDecode_Buf->w;
 int h = GfxDecode_Buf->h;
 int scroll = GfxDecode_Scroll;
 uint32 neo_palette[16];
 uint32 tile_limit;
 uint32 zero_color = GfxDecode_Buf->MakeColor(0, 0, 0, 0);

 if(wsVMode && GfxDecode_Layer != 2) // Sprites can't use the extra tile bank in WSC mode
  tile_limit = 0x400;
 else
  tile_limit = 0x200;

 if(GfxDecode_Pbn == -1)
 {
  if(wsVMode)
  {
   for(int x = 0; x < 16; x++)
    neo_palette[x] = GfxDecode_Buf->MakeColor(x * 17, x * 17, x * 17, 0xFF);
  }
  else
   for(int x = 0; x < 16; x++)
    neo_palette[x] = GfxDecode_Buf->MakeColor(x * 85, x * 85, x * 85, 0xFF);
 }
 else
 {
  if(wsVMode)
   for(int x = 0; x < 16; x++)
   {
    uint32 raw = wsCols[GfxDecode_Pbn & 0xF][x];
    uint32 r, g, b;

    r = (raw >> 8) & 0x0F;
    g = (raw >> 4) & 0x0F;
    b = (raw >> 0) & 0x0F;

    neo_palette[x] = GfxDecode_Buf->MakeColor(r * 17, g * 17, b * 17, 0xFF);
   }
  else
   for(int x = 0; x < 4; x++)
   {
    uint32 raw = wsMonoPal[GfxDecode_Pbn & 0xF][x];

    neo_palette[x] = GfxDecode_Buf->MakeColor(raw * 17 , raw * 17, raw * 17, 0xFF);
   }
 }
 palette_ptr = neo_palette;

 for(int y = 0; y < h; y++)
 {
  for(int x = 0; x < w; x += 8)
  {
   unsigned int which_tile = (x / 8) + (scroll + (y / 8)) * (w / 8);

   if(which_tile >= tile_limit)
   {
    for(int sx = 0; sx < 8; sx++)
    {
     target[x + sx] = zero_color;
     target[x + w * 1 + sx] = 0;
     target[x + w * 2 + sx] = 0;
    }
    continue;
   }

   wsGetTile(which_tile & 0x1FF, y&7, 0, 0, which_tile & 0x200);
   if(wsVMode)
   {
    for(int sx = 0; sx < 8; sx++)
     target[x + sx] = neo_palette[wsTileRow[sx]];
   }
   else
   {
    for(int sx = 0; sx < 8; sx++)
     target[x + sx] = neo_palette[wsTileRow[sx]];
   }

   uint32 address_base;
   uint32 tile_bsize;

   if(wsVMode & 0x4)
   {
    tile_bsize = 4 * 8 * 8 / 8;
    if(which_tile & 0x200)
     address_base = 0x8000;
    else
     address_base = 0x4000;
   }
   else
   {
    tile_bsize = 2 * 8 * 8 / 8;
    if(which_tile & 0x200)
     address_base = 0x4000;
    else
     address_base = 0x2000;
   }

   for(int sx = 0; sx < 8; sx++)
   {
    target[x + w * 1 + sx] = which_tile;
    target[x + w * 2 + sx] = address_base + (which_tile & 0x1FF) * tile_bsize;
   }
  }
  target += w * 3;
 }
}

void WSwan_GfxSetGraphicsDecode(MDFN_Surface *surface, int line, int which, int xscroll, int yscroll, int pbn)
{
 GfxDecode_Buf = surface;
 GfxDecode_Line = line;
 GfxDecode_Layer = which;
 GfxDecode_Scroll = yscroll;
 GfxDecode_Pbn = pbn;

 if(GfxDecode_Line == -1)
  DoGfxDecode();
}

#endif

}
