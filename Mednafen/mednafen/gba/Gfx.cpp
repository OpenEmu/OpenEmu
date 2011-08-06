// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2004 Forgotten and the VBA development team

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or(at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include "../mednafen.h"

#include "Gfx.h"
#include "gfx-draw.h"

int all_coeff[32] = 
{
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16
};

uint32 AlphaClampLUT[64] = 
{
 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,
 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F
};  


MDFN_ALIGN(16) uint32 line0[512];
MDFN_ALIGN(16) uint32 line1[512];
MDFN_ALIGN(16) uint32 line2[512];
MDFN_ALIGN(16) uint32 line3[512];
MDFN_ALIGN(16) uint32 lineOBJ[512];
MDFN_ALIGN(16) uint32 lineOBJWin[512];
MDFN_ALIGN(16) uint32 lineMix[512];

bool gfxInWin0[512];
bool gfxInWin1[512];

int gfxBG2Changed = 0;
int gfxBG3Changed = 0;

int gfxBG2X = 0;
int gfxBG2Y = 0;
int gfxBG2LastX = 0;
int gfxBG2LastY = 0;
int gfxBG3X = 0;
int gfxBG3Y = 0;
int gfxBG3LastX = 0;
int gfxBG3LastY = 0;
int gfxLastVCOUNT = 0;

void gfxDrawTextScreen(uint16 control, uint16 hofs, uint16 vofs,
                              uint32 *line)
{
  uint16 *palette = (uint16 *)paletteRAM;
  uint8 *charBase = &vram[((control >> 2) & 0x03) * 0x4000];
  uint16 *screenBase = (uint16 *)&vram[((control >> 8) & 0x1f) * 0x800];
  uint32 prio = ((control & 3)<<25) + 0x1000000;
  int sizeX = 256;
  int sizeY = 256;
  switch((control >> 14) & 3) {
  case 0:
    break;
  case 1:
    sizeX = 512;
    break;
  case 2:
    sizeY = 512;
    break;
  case 3:
    sizeX = 512;
    sizeY = 512;
    break;
  }

  int maskX = sizeX-1;
  int maskY = sizeY-1;
  
  bool mosaicOn = (control & 0x40) ? true : false;

  int xxx = hofs & maskX;
  int yyy = (vofs + VCOUNT) & maskY;
  int mosaicX = (MOSAIC & 0x000F)+1;
  int mosaicY = ((MOSAIC & 0x00F0)>>4)+1;

  if(mosaicOn) {
    if((VCOUNT % mosaicY) != 0) {
      mosaicY = (VCOUNT / mosaicY) * mosaicY;
      yyy = (vofs + mosaicY) & maskY;
    }
  }

  if(yyy > 255 && sizeY > 256) {
    yyy &= 255;
    screenBase += 0x400;
    if(sizeX > 256)
      screenBase += 0x400;
  }
  
  int yshift = ((yyy>>3)<<5);
  if((control) & 0x80) 
  {
    uint16 *screenSource = screenBase + 0x400 * (xxx>>8) + ((xxx & 255)>>3) + yshift;
    for(int x = 0; x < 240; x++) {
      uint16 data = READ16LE(screenSource);
      
      int tile = data & 0x3FF;
      int tileX = (xxx & 7);
      int tileY = yyy & 7;
      
      if(data & 0x0400)
        tileX = 7 - tileX;
      if(data & 0x0800)
        tileY = 7 - tileY;
      
      uint8 color = charBase[tile * 64 + tileY * 8 + tileX];
      
      line[x] = color ? (READ16LE(&palette[color]) | prio): 0x80000000;
      
      if(data & 0x0400) {
        if(tileX == 0)
          screenSource++;
      } else if(tileX == 7)
        screenSource++;
      xxx++;
      if(xxx == 256) {
        if(sizeX > 256)
          screenSource = screenBase + 0x400 + yshift;
        else {
          screenSource = screenBase + yshift;
          xxx = 0;
        }
      } else if(xxx >= sizeX) {
        xxx = 0;
        screenSource = screenBase + yshift;
      }
    }
  } 
  else 
  {
    uint16 *screenSource = screenBase + 0x400*(xxx>>8)+((xxx&255)>>3) + yshift;
    uint16 data = READ16LE(screenSource);
    int tile = data & 0x3FF;
    int pal = (READ16LE(screenSource)>>8) & 0xF0;
    int tileXmatch = (data & 0x0400) ? 0 : 7;

    for(int x = 0; x < 240; x++) 
    {
      int tileX = (xxx & 7);
      int tileY = yyy & 7;

      if(data & 0x0400)
        tileX = 7 - tileX;
      if(data & 0x0800)
        tileY = 7 - tileY;

      uint8 color = charBase[(tile<<5) + (tileY<<2) + (tileX>>1)];

      if(tileX & 1) {
        color = (color >> 4);
      } else {
        color &= 0x0F;
      }
      
      line[x] = color ? (READ16LE(&palette[pal + color])|prio): 0x80000000;

      if(tileX == tileXmatch)
      {
       screenSource++;
       data = READ16LE(screenSource);
       tile = data & 0x3FF;
       pal = (READ16LE(screenSource)>>8) & 0xF0;
       tileXmatch = (data & 0x0400) ? 0 : 7;
      }
      xxx++;
      if(xxx == 256) 
      {
        if(sizeX > 256)
          screenSource = screenBase + 0x400 + yshift;
        else 
	{
          screenSource = screenBase + yshift;
          xxx = 0;
        }
	data = READ16LE(screenSource);
        tile = data & 0x3FF;
	pal = (READ16LE(screenSource)>>8) & 0xF0;
	tileXmatch = (data & 0x0400) ? 0 : 7;
      } 
      else if(xxx >= sizeX)  
      {
        xxx = 0;
        screenSource = screenBase + yshift;
        data = READ16LE(screenSource);
        tile = data & 0x3FF;
	pal = (READ16LE(screenSource)>>8) & 0xF0;
	tileXmatch = (data & 0x0400) ? 0 : 7;
      }
    }
  }
  if(mosaicOn) 
  {
    if(mosaicX > 1) 
    {
      int m = 1;
      for(int i = 0; i < 239; i++) 
      {
        line[i+1] = line[i];
        m++;
        if(m == mosaicX) 
        {
          m = 1;
          i++;
        }
      }
    }
  }
}

void gfxDrawRotScreen(uint16 control, 
                             uint16 x_l, uint16 x_h,
                             uint16 y_l, uint16 y_h,
                             uint16 pa,  uint16 pb,
                             uint16 pc,  uint16 pd,
                             int& currentX, int& currentY,
                             int changed,
                             uint32 *line)
{
  uint16 *palette = (uint16 *)paletteRAM;
  uint8 *charBase = &vram[((control >> 2) & 0x03) * 0x4000];
  uint8 *screenBase = (uint8 *)&vram[((control >> 8) & 0x1f) * 0x800];
  int prio = ((control & 3) << 25) + 0x1000000;

  int sizeX = 128;
  int sizeY = 128;
  switch((control >> 14) & 3) {
  case 0:
    break;
  case 1:
    sizeX = sizeY = 256;
    break;
  case 2:
    sizeX = sizeY = 512;
    break;
  case 3:
    sizeX = sizeY = 1024;
    break;
  }

  int dx = pa & 0x7FFF;
  if(pa & 0x8000)
    dx |= 0xFFFF8000;
  int dmx = pb & 0x7FFF;
  if(pb & 0x8000)
    dmx |= 0xFFFF8000;
  int dy = pc & 0x7FFF;
  if(pc & 0x8000)
    dy |= 0xFFFF8000;
  int dmy = pd & 0x7FFF;
  if(pd & 0x8000)
    dmy |= 0xFFFF8000;

  if(VCOUNT == 0)
    changed = 3;

  if(changed & 1) {
    currentX = (x_l) | ((x_h & 0x07FF)<<16);
    if(x_h & 0x0800)
      currentX |= 0xF8000000;
  } else {
    currentX += dmx;
  }

  if(changed & 2) {
    currentY = (y_l) | ((y_h & 0x07FF)<<16);
    if(y_h & 0x0800)
      currentY |= 0xF8000000;
  } else {
    currentY += dmy;
  }  
  
  int realX = currentX;
  int realY = currentY;

  if(control & 0x40) {
    int mosaicY = ((MOSAIC & 0xF0)>>4) + 1;
    int y = (VCOUNT % mosaicY);
    realX -= y*dmx;
    realY -= y*dmy;
  }
  
  int xxx = (realX >> 8);
  int yyy = (realY >> 8);
  
  if(control & 0x2000) {
    xxx %= sizeX;
    yyy %= sizeY;
    if(xxx < 0)
      xxx += sizeX;
    if(yyy < 0)
      yyy += sizeY;
  }
  
  if(control & 0x80) {
    for(int x = 0; x < 240; x++) {
      if(xxx < 0 ||
         yyy < 0 ||
         xxx >= sizeX ||
         yyy >= sizeY) {
        line[x] = 0x80000000;
      } else {
        int tile = screenBase[(xxx>>3) + (yyy>>3)*(sizeX>>3)];
        
        int tileX = (xxx & 7);
        int tileY = yyy & 7;
        
        uint8 color = charBase[(tile<<6) + (tileY<<3) + tileX];
          
        line[x] = color ? (READ16LE(&palette[color])|prio): 0x80000000;
      }
      realX += dx;
      realY += dy;
      
      xxx = (realX >> 8);
      yyy = (realY >> 8);
      
      if(control & 0x2000) {
        xxx %= sizeX;
        yyy %= sizeY;
        if(xxx < 0)
          xxx += sizeX;
        if(yyy < 0)
          yyy += sizeY;
      }
    }
  } else {
    for(int x = 0; x < 240; x++) {
      if(xxx < 0 ||
         yyy < 0 ||
         xxx >= sizeX ||
         yyy >= sizeY) {
        line[x] = 0x80000000;
      } else {
        int tile = screenBase[(xxx>>3) + (yyy>>3)*(sizeX>>3)];
        
        int tileX = (xxx & 7);
        int tileY = yyy & 7;
        
        uint8 color = charBase[(tile<<6) + (tileY<<3) + tileX];
          
        line[x] = color ? (READ16LE(&palette[color])|prio): 0x80000000;
      }
      realX += dx;
      realY += dy;
      
      xxx = (realX >> 8);
      yyy = (realY >> 8);
      
      if(control & 0x2000) {
        xxx %= sizeX;
        yyy %= sizeY;
        if(xxx < 0)
          xxx += sizeX;
        if(yyy < 0)
          yyy += sizeY;
      }
    }    
  }

  if(control & 0x40) {    
    int mosaicX = (MOSAIC & 0xF) + 1;
    if(mosaicX > 1) {
      int m = 1;
      for(int i = 0; i < 239; i++) {
        line[i+1] = line[i];
        m++;
        if(m == mosaicX) {
          m = 1;
          i++;
        }
      }
    }
  }  
}

void gfxDrawRotScreen16Bit(uint16 control,
                                  uint16 x_l, uint16 x_h,
                                  uint16 y_l, uint16 y_h,
                                  uint16 pa,  uint16 pb,
                                  uint16 pc,  uint16 pd,
                                  int& currentX, int& currentY,
                                  int changed,
                                  uint32 *line)
{
  uint16 *screenBase = (uint16 *)&vram[0];
  int prio = ((control & 3) << 25) + 0x1000000;
  int sizeX = 240;
  int sizeY = 160;
  
  int startX = (x_l) | ((x_h & 0x07FF)<<16);
  if(x_h & 0x0800)
    startX |= 0xF8000000;
  int startY = (y_l) | ((y_h & 0x07FF)<<16);
  if(y_h & 0x0800)
    startY |= 0xF8000000;

  int dx = pa & 0x7FFF;
  if(pa & 0x8000)
    dx |= 0xFFFF8000;
  int dmx = pb & 0x7FFF;
  if(pb & 0x8000)
    dmx |= 0xFFFF8000;
  int dy = pc & 0x7FFF;
  if(pc & 0x8000)
    dy |= 0xFFFF8000;
  int dmy = pd & 0x7FFF;
  if(pd & 0x8000)
    dmy |= 0xFFFF8000;

  if(VCOUNT == 0)
    changed = 3;
  
  if(changed & 1) {
    currentX = (x_l) | ((x_h & 0x07FF)<<16);
    if(x_h & 0x0800)
      currentX |= 0xF8000000;
  } else
    currentX += dmx;

  if(changed & 2) {
    currentY = (y_l) | ((y_h & 0x07FF)<<16);
    if(y_h & 0x0800)
      currentY |= 0xF8000000;
  } else {
    currentY += dmy;
  }  
  
  int realX = currentX;
  int realY = currentY;

  if(control & 0x40) {
    int mosaicY = ((MOSAIC & 0xF0)>>4) + 1;
    int y = (VCOUNT % mosaicY);
    realX -= y*dmx;
    realY -= y*dmy;
  }
  
  int xxx = (realX >> 8);
  int yyy = (realY >> 8);
  
  for(int x = 0; x < 240; x++) {
    if(xxx < 0 ||
       yyy < 0 ||
       xxx >= sizeX ||
       yyy >= sizeY) {
      line[x] = 0x80000000;
    } else {
      line[x] = (READ16LE(&screenBase[yyy * sizeX + xxx]) | prio);
    }
    realX += dx;
    realY += dy;
    
    xxx = (realX >> 8);
    yyy = (realY >> 8);
  }

  if(control & 0x40) {    
    int mosaicX = (MOSAIC & 0xF) + 1;
    if(mosaicX > 1) {
      int m = 1;
      for(int i = 0; i < 239; i++) {
        line[i+1] = line[i];
        m++;
        if(m == mosaicX) {
          m = 1;
          i++;
        }
      }
    }
  }  
}

void gfxDrawRotScreen256(uint16 control, 
                                uint16 x_l, uint16 x_h,
                                uint16 y_l, uint16 y_h,
                                uint16 pa,  uint16 pb,
                                uint16 pc,  uint16 pd,
                                int &currentX, int& currentY,
                                int changed,
                                uint32 *line)
{
  uint16 *palette = (uint16 *)paletteRAM;
  uint8 *screenBase = (DISPCNT & 0x0010) ? &vram[0xA000] : &vram[0x0000];
  int prio = ((control & 3) << 25) + 0x1000000;
  int sizeX = 240;
  int sizeY = 160;
  
  int startX = (x_l) | ((x_h & 0x07FF)<<16);
  if(x_h & 0x0800)
    startX |= 0xF8000000;
  int startY = (y_l) | ((y_h & 0x07FF)<<16);
  if(y_h & 0x0800)
    startY |= 0xF8000000;

  int dx = pa & 0x7FFF;
  if(pa & 0x8000)
    dx |= 0xFFFF8000;
  int dmx = pb & 0x7FFF;
  if(pb & 0x8000)
    dmx |= 0xFFFF8000;
  int dy = pc & 0x7FFF;
  if(pc & 0x8000)
    dy |= 0xFFFF8000;
  int dmy = pd & 0x7FFF;
  if(pd & 0x8000)
    dmy |= 0xFFFF8000;

  if(VCOUNT == 0)
    changed = 3;

  if(changed & 1) {
    currentX = (x_l) | ((x_h & 0x07FF)<<16);
    if(x_h & 0x0800)
      currentX |= 0xF8000000;
  } else {
    currentX += dmx;
  }

  if(changed & 2) {
    currentY = (y_l) | ((y_h & 0x07FF)<<16);
    if(y_h & 0x0800)
      currentY |= 0xF8000000;
  } else {
    currentY += dmy;
  }  
  
  int realX = currentX;
  int realY = currentY;

  if(control & 0x40) {
    int mosaicY = ((MOSAIC & 0xF0)>>4) + 1;
    int y = (VCOUNT / mosaicY) * mosaicY;
    realX = startX + y*dmx;
    realY = startY + y*dmy;
  }
  
  int xxx = (realX >> 8);
  int yyy = (realY >> 8);
  
  for(int x = 0; x < 240; x++) {
    if(xxx < 0 ||
         yyy < 0 ||
       xxx >= sizeX ||
       yyy >= sizeY) {
      line[x] = 0x80000000;
    } else {
      uint8 color = screenBase[yyy * 240 + xxx];
      
      line[x] = color ? (READ16LE(&palette[color])|prio): 0x80000000;
    }
    realX += dx;
    realY += dy;
    
    xxx = (realX >> 8);
    yyy = (realY >> 8);
  }

  if(control & 0x40) {    
    int mosaicX = (MOSAIC & 0xF) + 1;
    if(mosaicX > 1) {
      int m = 1;
      for(int i = 0; i < 239; i++) {
        line[i+1] = line[i];
        m++;
        if(m == mosaicX) {
          m = 1;
          i++;
        }
      }
    }
  }    
}

void gfxDrawRotScreen16Bit160(uint16 control,
                                     uint16 x_l, uint16 x_h,
                                     uint16 y_l, uint16 y_h,
                                     uint16 pa,  uint16 pb,
                                     uint16 pc,  uint16 pd,
                                     int& currentX, int& currentY,
                                     int changed,
                                     uint32 *line)
{
  uint16 *screenBase = (DISPCNT & 0x0010) ? (uint16 *)&vram[0xa000] :
    (uint16 *)&vram[0];
  int prio = ((control & 3) << 25) + 0x1000000;
  int sizeX = 160;
  int sizeY = 128;
  
  int startX = (x_l) | ((x_h & 0x07FF)<<16);
  if(x_h & 0x0800)
    startX |= 0xF8000000;
  int startY = (y_l) | ((y_h & 0x07FF)<<16);
  if(y_h & 0x0800)
    startY |= 0xF8000000;

  int dx = pa & 0x7FFF;
  if(pa & 0x8000)
    dx |= 0xFFFF8000;
  int dmx = pb & 0x7FFF;
  if(pb & 0x8000)
    dmx |= 0xFFFF8000;
  int dy = pc & 0x7FFF;
  if(pc & 0x8000)
    dy |= 0xFFFF8000;
  int dmy = pd & 0x7FFF;
  if(pd & 0x8000)
    dmy |= 0xFFFF8000;

  if(VCOUNT == 0)
    changed = 3;

  if(changed & 1) {
    currentX = (x_l) | ((x_h & 0x07FF)<<16);
    if(x_h & 0x0800)
      currentX |= 0xF8000000;
  } else {
    currentX += dmx;
  }

  if(changed & 2) {
    currentY = (y_l) | ((y_h & 0x07FF)<<16);
    if(y_h & 0x0800)
      currentY |= 0xF8000000;
  } else {
    currentY += dmy;
  }  
  
  int realX = currentX;
  int realY = currentY;

  if(control & 0x40) {
    int mosaicY = ((MOSAIC & 0xF0)>>4) + 1;
    int y = (VCOUNT / mosaicY) * mosaicY;
    realX = startX + y*dmx;
    realY = startY + y*dmy;
  }
  
  int xxx = (realX >> 8);
  int yyy = (realY >> 8);
  
  for(int x = 0; x < 240; x++) {
    if(xxx < 0 ||
       yyy < 0 ||
       xxx >= sizeX ||
       yyy >= sizeY) {
      line[x] = 0x80000000;
    } else {
      line[x] = (READ16LE(&screenBase[yyy * sizeX + xxx]) | prio);
    }
    realX += dx;
    realY += dy;
    
    xxx = (realX >> 8);
    yyy = (realY >> 8);
  }

  if(control & 0x40) {    
    int mosaicX = (MOSAIC & 0xF) + 1;
    if(mosaicX > 1) {
      int m = 1;
      for(int i = 0; i < 239; i++) {
        line[i+1] = line[i];
        m++;
        if(m == mosaicX) {
          m = 1;
          i++;
        }
      }
    }
  }      
}

void gfxDrawSprites(void)
{
  int m=0;
  gfxClearArray(lineOBJ);
  if(layerEnable & 0x1000) {
    uint16 *sprites = (uint16 *)oam;
    uint16 *spritePalette = &((uint16 *)paletteRAM)[256];
    int mosaicY = ((MOSAIC & 0xF000)>>12) + 1;
    int mosaicX = ((MOSAIC & 0xF00)>>8) + 1;    

    for(int i = 0; i < 128 ; i++) {

      uint16 a0 = READ16LE(sprites++);
      uint16 a1 = READ16LE(sprites++);
      uint16 a2 = READ16LE(sprites++);
      sprites++;

      // ignore OBJ-WIN
      if((a0 & 0x0c00) == 0x0800)
         continue;
      
      int sizeY = 8;
      int sizeX = 8;
      
      switch(((a0 >>12) & 0x0c)|(a1>>14)) {
      case 0:
        break;
      case 1:
        sizeX = sizeY = 16;
        break;
      case 2:
        sizeX = sizeY = 32;
        break;
      case 3:
        sizeX = sizeY = 64;
        break;
      case 4:
        sizeX = 16;
        break;
      case 5:
        sizeX = 32;
        break;
      case 6:
        sizeX = 32;
        sizeY = 16;
        break;
      case 7:
        sizeX = 64;
        sizeY = 32;
        break;
      case 8:
        sizeY = 16;
        break;
      case 9:
        sizeY = 32;
        break;
      case 10:
        sizeX = 16;
        sizeY = 32;
        break;
      case 11:
        sizeX = 32;
        sizeY = 64;
        break;
      default:
        continue;
      }

#ifdef SPRITE_DEBUG
      int maskX = sizeX-1;
      int maskY = sizeY-1;
#endif

      int sy = (a0 & 255);

      if(sy > 160)
        sy -= 256;
      
      if(a0 & 0x0100) {
        int fieldX = sizeX;
        int fieldY = sizeY;
        if(a0 & 0x0200) {
          fieldX <<= 1;
          fieldY <<= 1;
        }
        
        int t = VCOUNT - sy;
        if((t >= 0) && (t < fieldY)) {
          int sx = (a1 & 0x1FF);
          if((sx < 240) || (((sx + fieldX) & 511) < 240)) {
            // int t2 = t - (fieldY >> 1);
            int rot = (a1 >> 9) & 0x1F;
            uint16 *OAM = (uint16 *)oam;
            int dx = READ16LE(&OAM[3 + (rot << 4)]);
            if(dx & 0x8000)
              dx |= 0xFFFF8000;
            int dmx = READ16LE(&OAM[7 + (rot << 4)]);
            if(dmx & 0x8000)
              dmx |= 0xFFFF8000;
            int dy = READ16LE(&OAM[11 + (rot << 4)]);
            if(dy & 0x8000)
              dy |= 0xFFFF8000;
            int dmy = READ16LE(&OAM[15 + (rot << 4)]);
            if(dmy & 0x8000)
              dmy |= 0xFFFF8000;
            
            if(a0 & 0x1000) {
              t -= (t % mosaicY);
            }

            int realX = ((sizeX) << 7) - (fieldX >> 1)*dx - (fieldY>>1)*dmx
              + t * dmx;
            int realY = ((sizeY) << 7) - (fieldX >> 1)*dy - (fieldY>>1)*dmy
              + t * dmy;

            uint32 prio = (((a2 >> 10) & 3) << 25) | ((a0 & 0x0c00)<<6);
            
            if(a0 & 0x2000) {
              int c = (a2 & 0x3FF);
              if((DISPCNT & 7) > 2 && (c < 512))
                continue;
              int inc = 32;
              if(DISPCNT & 0x40)
                inc = sizeX >> 2;
              else
                c &= 0x3FE;
              for(int x = 0; x < fieldX; x++) {
                int xxx = realX >> 8;
                int yyy = realY >> 8;
                
                if(xxx < 0 || xxx >= sizeX ||
                   yyy < 0 || yyy >= sizeY ||
                   sx >= 240);
                else {
                  uint32 color = vram[0x10000 + ((((c + (yyy>>3) * inc)<<5)
									+ ((yyy & 7)<<3) + ((xxx >> 3)<<6) +
                                    (xxx & 7))&0x7FFF)];
                  if ((color==0) && (((prio >> 25)&3) < 
                                     ((lineOBJ[sx]>>25)&3))) {
                    lineOBJ[sx] = (lineOBJ[sx] & 0xF9FFFFFF) | prio;
                    if((a0 & 0x1000) && m)
                      lineOBJ[sx]=(lineOBJ[sx-1] & 0xF9FFFFFF) | prio;
                  } else if((color) && (prio < (lineOBJ[sx]&0xFF000000))) {
                    lineOBJ[sx] = READ16LE(&spritePalette[color]) | prio;
                    if((a0 & 0x1000) && m)
                      lineOBJ[sx]=(lineOBJ[sx-1] & 0xF9FFFFFF) | prio;
                  }

                  if (a0 & 0x1000) {
                    m++;
                    if (m==mosaicX)
                      m=0;
                  }
#ifdef SPRITE_DEBUG
                  if(t == 0 || t == maskY || x == 0 || x == maskX)
                    lineOBJ[sx] = 0x001F;
#endif
                }
                sx = (sx+1)&511;;
                realX += dx;
                realY += dy;
              }
            } else {
              int c = (a2 & 0x3FF);
              if((DISPCNT & 7) > 2 && (c < 512))
                continue;
              
              int inc = 32;
              if(DISPCNT & 0x40)
                inc = sizeX >> 3;
              int palette = (a2 >> 8) & 0xF0;                 
              for(int x = 0; x < fieldX; x++) {
                int xxx = realX >> 8;
                int yyy = realY >> 8;
                if(xxx < 0 || xxx >= sizeX ||
                   yyy < 0 || yyy >= sizeY ||
                   sx >= 240);
                else {
                  uint32 color = vram[0x10000 + ((((c + (yyy>>3) * inc)<<5)
                                                + ((yyy & 7)<<2) + ((xxx >> 3)<<5) +
                                               ((xxx & 7)>>1))&0x7FFF)];
                  if(xxx & 1)
                    color >>= 4;
                  else
                    color &= 0x0F;
                  
                  if ((color==0) && (((prio >> 25)&3) < 
                                     ((lineOBJ[sx]>>25)&3))) {
                    lineOBJ[sx] = (lineOBJ[sx] & 0xF9FFFFFF) | prio;
                    if((a0 & 0x1000) && m)
                      lineOBJ[sx]=(lineOBJ[sx-1] & 0xF9FFFFFF) | prio;
                  } else if((color) && (prio < (lineOBJ[sx]&0xFF000000))) {
                    lineOBJ[sx] = READ16LE(&spritePalette[palette+color]) | prio;
                    if((a0 & 0x1000) && m)
                      lineOBJ[sx]=(lineOBJ[sx-1] & 0xF9FFFFFF) | prio;
                  }
                }
                if((a0 & 0x1000) && m) {
                  m++;
                  if (m==mosaicX)
                    m=0;
                }

#ifdef SPRITE_DEBUG
                  if(t == 0 || t == maskY || x == 0 || x == maskX)
                    lineOBJ[sx] = 0x001F;
#endif
                sx = (sx+1)&511;;
                realX += dx;
                realY += dy;
                
              }       
            }
          }
        }
      } else {
        int t = VCOUNT - sy;
        if((t >= 0) && (t < sizeY)) {
          int sx = (a1 & 0x1FF);
          if(((sx < 240)||(((sx+sizeX)&511)<240)) && !(a0 & 0x0200)) {
            if(a0 & 0x2000) {
              if(a1 & 0x2000)
                t = sizeY - t - 1;
              int c = (a2 & 0x3FF);
              if((DISPCNT & 7) > 2 && (c < 512))
                continue;
              
              int inc = 32;
              if(DISPCNT & 0x40) {
                inc = sizeX >> 2;
              } else {
                c &= 0x3FE;
              }
              int xxx = 0;
              if(a1 & 0x1000)
                xxx = sizeX-1;

              if(a0 & 0x1000) {
                t -= (t % mosaicY);
              }

              int address = 0x10000 + ((((c+ (t>>3) * inc) << 5)
                + ((t & 7) << 3) + ((xxx>>3)<<6) + (xxx & 7)) & 0x7FFF);
                
              if(a1 & 0x1000)
                xxx = 7;
              uint32 prio = (((a2 >> 10) & 3) << 25) | ((a0 & 0x0c00)<<6);
                
              for(int xx = 0; xx < sizeX; xx++) {
                if(sx < 240) {
                  uint8 color = vram[address];
                  if ((color==0) && (((prio >> 25)&3) < 
                                     ((lineOBJ[sx]>>25)&3))) {
                    lineOBJ[sx] = (lineOBJ[sx] & 0xF9FFFFFF) | prio;
                    if((a0 & 0x1000) && m)
                      lineOBJ[sx]=(lineOBJ[sx-1] & 0xF9FFFFFF) | prio;
                  } else if((color) && (prio < (lineOBJ[sx]&0xFF000000))) {
                    lineOBJ[sx] = READ16LE(&spritePalette[color]) | prio;
                    if((a0 & 0x1000) && m)
                      lineOBJ[sx]=(lineOBJ[sx-1] & 0xF9FFFFFF) | prio;
                  }

                  if (a0 & 0x1000) {
                    m++;
                    if (m==mosaicX)
                      m=0;
                  }

#ifdef SPRITE_DEBUG
                  if(t == 0 || t == maskY || xx == 0 || xx == maskX)
                    lineOBJ[sx] = 0x001F;
#endif
                }
                  
                sx = (sx+1) & 511;
                if(a1 & 0x1000) {
                  xxx--;
                  address--;
                  if(xxx == -1) {
                    address -= 56;
                    xxx = 7;
                  }
                  if(address < 0x10000)
                    address += 0x8000;
                } else {
                  xxx++;
                  address++;
                  if(xxx == 8) {
                    address += 56;
                    xxx = 0;
                  }
                  if(address > 0x17fff)
                    address -= 0x8000;
                }
              }
            } else {
              if(a1 & 0x2000)
                t = sizeY - t - 1;
              int c = (a2 & 0x3FF);
              if((DISPCNT & 7) > 2 && (c < 512))
                continue;
              
              int inc = 32;
              if(DISPCNT & 0x40) {
                inc = sizeX >> 3;
              }
              int xxx = 0;
              if(a1 & 0x1000)
                xxx = sizeX - 1;
                  
                if(a0 & 0x1000) {
                  t -= (t % mosaicY);
                }

              int address = 0x10000 + ((((c + (t>>3) * inc)<<5)
                + ((t & 7)<<2) + ((xxx>>3)<<5) + ((xxx & 7) >> 1))&0x7FFF);
              uint32 prio = (((a2 >> 10) & 3) << 25) | ((a0 & 0x0c00)<<6);
              int palette = (a2 >> 8) & 0xF0;         
              if(a1 & 0x1000) {
                xxx = 7;
                for(int xx = sizeX - 1; xx >= 0; xx--) {
                  if(sx < 240) {
                    uint8 color = vram[address];
                    if(xx & 1) {
                      color = (color >> 4);
                    } else
                      color &= 0x0F;
                    
                    if ((color==0) && (((prio >> 25)&3) < 
                                       ((lineOBJ[sx]>>25)&3))) {
                      lineOBJ[sx] = (lineOBJ[sx] & 0xF9FFFFFF) | prio;
                      if((a0 & 0x1000) && m)
                        lineOBJ[sx]=(lineOBJ[sx-1] & 0xF9FFFFFF) | prio;
                    } else if((color) && (prio < (lineOBJ[sx]&0xFF000000))) {
                      lineOBJ[sx] = READ16LE(&spritePalette[palette + color]) | prio;
                      if((a0 & 0x1000) && m)
                        lineOBJ[sx]=(lineOBJ[sx-1] & 0xF9FFFFFF) | prio;
                    }
                  }
                  if (a0 & 0x1000) {
                    m++;
                    if (m==mosaicX)
                      m=0;
                  }
#ifdef SPRITE_DEBUG
                  if(t == 0 || t == maskY || xx == 0 || xx == maskX)
                    lineOBJ[sx] = 0x001F;
#endif
                  sx = (sx+1) & 511;
                  xxx--;
                  if(!(xx & 1))
                    address--;
                  if(xxx == -1) {
                    xxx = 7;
                    address -= 28;
                  }
                  if(address < 0x10000)
                    address += 0x8000;
                }           
              } else {        
                for(int xx = 0; xx < sizeX; xx++) {
                  if(sx < 240) {
                    uint8 color = vram[address];
                    if(xx & 1) {
                      color = (color >> 4);
                    } else
                      color &= 0x0F;
                    
                    if ((color==0) && (((prio >> 25)&3) < 
                                       ((lineOBJ[sx]>>25)&3))) {
                      lineOBJ[sx] = (lineOBJ[sx] & 0xF9FFFFFF) | prio;
                      if((a0 & 0x1000) && m)
                        lineOBJ[sx]=(lineOBJ[sx-1] & 0xF9FFFFFF) | prio;
                    } else if((color) && (prio < (lineOBJ[sx]&0xFF000000))) {
                      lineOBJ[sx] = READ16LE(&spritePalette[palette + color]) | prio;
                      if((a0 & 0x1000) && m)
                        lineOBJ[sx]=(lineOBJ[sx-1] & 0xF9FFFFFF) | prio;

                    }
                  }
                  if (a0 & 0x1000) {
                    m++;
                    if (m==mosaicX)
                      m=0;
                  }
#ifdef SPRITE_DEBUG
                  if(t == 0 || t == maskY || xx == 0 || xx == maskX)
                    lineOBJ[sx] = 0x001F;
#endif
                  sx = (sx+1) & 511;
                  xxx++;
                  if(xx & 1)
                    address++;
                  if(xxx == 8) {
                    address += 28;
                    xxx = 0;
                  }
                  if(address > 0x17fff)
                    address -= 0x8000;
                }           
              }
            }
          }
        }
      }
    }
  }
}

void gfxDrawOBJWin(void)
{
  gfxClearArray(lineOBJWin);
  if(layerEnable & 0x8000) {
    uint16 *sprites = (uint16 *)oam;
    // uint16 *spritePalette = &((uint16 *)paletteRAM)[256];
    for(int i = 0; i < 128 ; i++) {
      uint16 a0 = READ16LE(sprites++);
      uint16 a1 = READ16LE(sprites++);
      uint16 a2 = READ16LE(sprites++);
      sprites++;

      // ignore non OBJ-WIN
      if((a0 & 0x0c00) != 0x0800)
        continue;
      
      int sizeY = 8;
      int sizeX = 8;
      
      switch(((a0 >>12) & 0x0c)|(a1>>14)) {
      case 0:
        break;
      case 1:
        sizeX = sizeY = 16;
        break;
      case 2:
        sizeX = sizeY = 32;
        break;
      case 3:
        sizeX = sizeY = 64;
        break;
      case 4:
        sizeX = 16;
        break;
      case 5:
        sizeX = 32;
        break;
      case 6:
        sizeX = 32;
        sizeY = 16;
        break;
      case 7:
        sizeX = 64;
        sizeY = 32;
        break;
      case 8:
        sizeY = 16;
        break;
      case 9:
        sizeY = 32;
        break;
      case 10:
        sizeX = 16;
        sizeY = 32;
        break;
      case 11:
        sizeX = 32;
        sizeY = 64;
        break;
      default:
        continue;
      }

      int sy = (a0 & 255);

      if(sy > 160)
        sy -= 256;
      
      if(a0 & 0x0100) {
        int fieldX = sizeX;
        int fieldY = sizeY;
        if(a0 & 0x0200) {
          fieldX <<= 1;
          fieldY <<= 1;
        }
        
        int t = VCOUNT - sy;
        if((t >= 0) && (t < fieldY)) {
          int sx = (a1 & 0x1FF);
          if((sx < 240) || (((sx + fieldX) & 511) < 240)) {
            // int t2 = t - (fieldY >> 1);
            int rot = (a1 >> 9) & 0x1F;
            uint16 *OAM = (uint16 *)oam;
            int dx = READ16LE(&OAM[3 + (rot << 4)]);
            if(dx & 0x8000)
              dx |= 0xFFFF8000;
            int dmx = READ16LE(&OAM[7 + (rot << 4)]);
            if(dmx & 0x8000)
              dmx |= 0xFFFF8000;
            int dy = READ16LE(&OAM[11 + (rot << 4)]);
            if(dy & 0x8000)
              dy |= 0xFFFF8000;
            int dmy = READ16LE(&OAM[15 + (rot << 4)]);
            if(dmy & 0x8000)
              dmy |= 0xFFFF8000;
            
            int realX = ((sizeX) << 7) - (fieldX >> 1)*dx - (fieldY>>1)*dmx
              + t * dmx;
            int realY = ((sizeY) << 7) - (fieldX >> 1)*dy - (fieldY>>1)*dmy
              + t * dmy;

            // uint32 prio = (((a2 >> 10) & 3) << 25) | ((a0 & 0x0c00)<<6);
            
            if(a0 & 0x2000) {
              int c = (a2 & 0x3FF);
              if((DISPCNT & 7) > 2 && (c < 512))
                continue;
              int inc = 32;
              if(DISPCNT & 0x40)
                inc = sizeX >> 2;
              else
                c &= 0x3FE;
              for(int x = 0; x < fieldX; x++) {
                int xxx = realX >> 8;
                int yyy = realY >> 8;
                
                if(xxx < 0 || xxx >= sizeX ||
                   yyy < 0 || yyy >= sizeY ||
                   sx >= 240) {
                } else {
                  uint32 color = vram[0x10000 + ((((c + (yyy>>3) * inc)<<5)
                                    + ((yyy & 7)<<3) + ((xxx >> 3)<<6) +
                                   (xxx & 7))&0x7fff)];
                  if(color) {
                    lineOBJWin[sx] = 1;
                  }
                }
                sx = (sx+1)&511;;
                realX += dx;
                realY += dy;
              }
            } else {
              int c = (a2 & 0x3FF);
              if((DISPCNT & 7) > 2 && (c < 512))
                continue;
              
              int inc = 32;
              if(DISPCNT & 0x40)
                inc = sizeX >> 3;
              // int palette = (a2 >> 8) & 0xF0;                      
              for(int x = 0; x < fieldX; x++) {
                int xxx = realX >> 8;
                int yyy = realY >> 8;

                //              if(x == 0 || x == (sizeX-1) ||
                //                 t == 0 || t == (sizeY-1)) {
                //                lineOBJ[sx] = 0x001F | prio;
                //              } else {
                  if(xxx < 0 || xxx >= sizeX ||
                     yyy < 0 || yyy >= sizeY ||
                     sx >= 240){
                  } else {
                    uint32 color = vram[0x10000 + ((((c + (yyy>>3) * inc)<<5)
                                     + ((yyy & 7)<<2) + ((xxx >> 3)<<5) +
                                     ((xxx & 7)>>1))&0x7fff)];
                    if(xxx & 1)
                      color >>= 4;
                    else
                      color &= 0x0F;
                    
                    if(color) {
                      lineOBJWin[sx] = 1;
                    }
                  }
                  //            }
                sx = (sx+1)&511;;
                realX += dx;
                realY += dy;
              }       
            }
          }
        }
      } else {
        int t = VCOUNT - sy;
        if((t >= 0) && (t < sizeY)) {
          int sx = (a1 & 0x1FF);
          if(((sx < 240)||(((sx+sizeX)&511)<240)) && !(a0 & 0x0200)) {
            if(a0 & 0x2000) {
              if(a1 & 0x2000)
                t = sizeY - t - 1;
              int c = (a2 & 0x3FF);
              if((DISPCNT & 7) > 2 && (c < 512))
                continue;
              
              int inc = 32;
              if(DISPCNT & 0x40) {
                inc = sizeX >> 2;
              } else {
                c &= 0x3FE;
              }
              int xxx = 0;
              if(a1 & 0x1000)
                xxx = sizeX-1;
              int address = 0x10000 + ((((c+ (t>>3) * inc) << 5)
                + ((t & 7) << 3) + ((xxx>>3)<<6) + (xxx & 7))&0x7fff);
              if(a1 & 0x1000)
                xxx = 7;
              // uint32 prio = (((a2 >> 10) & 3) << 25) | ((a0 & 0x0c00)<<6);
              for(int xx = 0; xx < sizeX; xx++) {
                if(sx < 240) {
                  uint8 color = vram[address];
                  if(color) {
                    lineOBJWin[sx] = 1;
                  }
                }
                  
                sx = (sx+1) & 511;
                if(a1 & 0x1000) {
                  xxx--;
                  address--;
                  if(xxx == -1) {
                    address -= 56;
                    xxx = 7;
                  }
                  if(address < 0x10000)
                    address += 0x8000;
                } else {
                  xxx++;
                  address++;
                  if(xxx == 8) {
                    address += 56;
                    xxx = 0;
                  }
                  if(address > 0x17fff)
                    address -= 0x8000;
                }
              }
            } else {
              if(a1 & 0x2000)
                t = sizeY - t - 1;
              int c = (a2 & 0x3FF);
              if((DISPCNT & 7) > 2 && (c < 512))
                continue;
              
              int inc = 32;
              if(DISPCNT & 0x40) {
                inc = sizeX >> 3;
              }
              int xxx = 0;
              if(a1 & 0x1000)
                xxx = sizeX - 1;
              int address = 0x10000 + ((((c + (t>>3) * inc)<<5)
                + ((t & 7)<<2) + ((xxx>>3)<<5) + ((xxx & 7) >> 1))&0x7fff);
              // uint32 prio = (((a2 >> 10) & 3) << 25) | ((a0 & 0x0c00)<<6);
              // int palette = (a2 >> 8) & 0xF0;              
              if(a1 & 0x1000) {
                xxx = 7;
                for(int xx = sizeX - 1; xx >= 0; xx--) {
                  if(sx < 240) {
                    uint8 color = vram[address];
                    if(xx & 1) {
                      color = (color >> 4);
                    } else
                      color &= 0x0F;
                    
                    if(color) {
                      lineOBJWin[sx] = 1;
                    }
                  }
                  sx = (sx+1) & 511;
                  xxx--;
                  if(!(xx & 1))
                    address--;
                  if(xxx == -1) {
                    xxx = 7;
                    address -= 28;
                  }
                  if(address < 0x10000)
                    address += 0x8000;
                }           
              } else {        
                for(int xx = 0; xx < sizeX; xx++) {
                  if(sx < 240) {
                    uint8 color = vram[address];
                    if(xx & 1) {
                      color = (color >> 4);
                    } else
                      color &= 0x0F;
                    
                    if(color) {
                      lineOBJWin[sx] = 1;
                    }
                  }
                  sx = (sx+1) & 511;
                  xxx++;
                  if(xx & 1)
                    address++;
                  if(xxx == 8) {
                    address += 28;
                    xxx = 0;
                  }
                  if(address > 0x17fff)
                    address -= 0x8000;
                }           
              }
            }
          }
        }
      }
    }
  }
}

