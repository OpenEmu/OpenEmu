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

#include "GBA.h"
#include "Globals.h"
#include "Gfx.h"

void mode3RenderLine()
{
  uint16 *palette = (uint16 *)paletteRAM;
  
  if(DISPCNT & 0x80) {
    for(int x = 0; x < 240; x++) {
      lineMix[x] = 0x7fff;
    }
    gfxLastVCOUNT = VCOUNT;    
    return;
  }

  if(layerEnable & 0x0400) {
    int changed = gfxBG2Changed;
    
    if(gfxLastVCOUNT > VCOUNT)
      changed = 3;
    
    gfxDrawRotScreen16Bit(BG2CNT, BG2X_L, BG2X_H,
                          BG2Y_L, BG2Y_H, BG2PA, BG2PB,
                          BG2PC, BG2PD,
                          gfxBG2X, gfxBG2Y, changed,                      
                          line2);
  }

  gfxDrawSprites(lineOBJ);

  uint32 background = (READ16LE(&palette[0]) | 0x30000000);
  
  for(int x = 0; x < 240; x++) {
    uint32 color = background;
    uint8 top = 0x20;

    if(line2[x] < color) {
      color = line2[x];
      top = 0x04;
    }

    if((uint8)(lineOBJ[x]>>24) < (uint8)(color >>24)) {
      color = lineOBJ[x];
      top = 0x10;
    }

    if((top & 0x10) && (color & 0x00010000)) {
      // semi-transparent OBJ
      uint32 back = background;
      uint8 top2 = 0x20;
      
      if(line2[x] < back) {
        back = line2[x];
        top2 = 0x04;
      }
      
      if(top2 & (BLDMOD>>8))
        color = gfxAlphaBlend(color, back,
                              coeff[COLEV & 0x1F],
                              coeff[(COLEV >> 8) & 0x1F]);
      else {
        switch((BLDMOD >> 6) & 3) {
        case 2:
          if(BLDMOD & top)
            color = gfxIncreaseBrightness(color, coeff[COLY & 0x1F]);
          break;
        case 3:
          if(BLDMOD & top)
            color = gfxDecreaseBrightness(color, coeff[COLY & 0x1F]);
          break;
        }         
      }
    }    
      
    lineMix[x] = color;
  }
  gfxBG2Changed = 0;
  gfxLastVCOUNT = VCOUNT;      
}

void mode3RenderLineNoWindow()
{
  uint16 *palette = (uint16 *)paletteRAM;
  
  if(DISPCNT & 0x80) {
    for(int x = 0; x < 240; x++) {
      lineMix[x] = 0x7fff;
    }
    gfxLastVCOUNT = VCOUNT;    
    return;
  }

  if(layerEnable & 0x0400) {
    int changed = gfxBG2Changed;

    if(gfxLastVCOUNT > VCOUNT)
      changed = 3;
    
    gfxDrawRotScreen16Bit(BG2CNT, BG2X_L, BG2X_H,
                          BG2Y_L, BG2Y_H, BG2PA, BG2PB,
                          BG2PC, BG2PD,
                          gfxBG2X, gfxBG2Y, changed,                      
                          line2);
  }

  gfxDrawSprites(lineOBJ);

  uint32 background = (READ16LE(&palette[0]) | 0x30000000);
  
  for(int x = 0; x < 240; x++) {
    uint32 color = background;
    uint8 top = 0x20;

    if(line2[x] < color) {
      color = line2[x];
      top = 0x04;
    }

    if((uint8)(lineOBJ[x]>>24) < (uint8)(color >>24)) {
      color = lineOBJ[x];
      top = 0x10;
    }

    if(!(color & 0x00010000)) {
      switch((BLDMOD >> 6) & 3) {
      case 0:
        break;
      case 1:
        {
          if(top & BLDMOD) {
            uint32 back = background;
            uint8 top2 = 0x20;
            
            if(line2[x] < back) {
              if(top != 0x04) {
                back = line2[x];
                top2 = 0x04;
              }
            }
            
            if((uint8)(lineOBJ[x]>>24) < (uint8)(back >> 24)) {
              if(top != 0x10) {
                back = lineOBJ[x];
                top2 = 0x10;
              }
            }
            
            if(top2 & (BLDMOD>>8))
              color = gfxAlphaBlend(color, back,
                                    coeff[COLEV & 0x1F],
                                    coeff[(COLEV >> 8) & 0x1F]);
            
          }
        }
        break;
      case 2:
        if(BLDMOD & top)
          color = gfxIncreaseBrightness(color, coeff[COLY & 0x1F]);
        break;
      case 3:
        if(BLDMOD & top)
          color = gfxDecreaseBrightness(color, coeff[COLY & 0x1F]);
        break;
      }
    } else {
      // semi-transparent OBJ
      uint32 back = background;
      uint8 top2 = 0x20;
      
      if(line2[x] < back) {
        back = line2[x];
        top2 = 0x04;
      }
      
      if(top2 & (BLDMOD>>8))
        color = gfxAlphaBlend(color, back,
                              coeff[COLEV & 0x1F],
                              coeff[(COLEV >> 8) & 0x1F]);
      else {
        switch((BLDMOD >> 6) & 3) {
        case 2:
          if(BLDMOD & top)
            color = gfxIncreaseBrightness(color, coeff[COLY & 0x1F]);
          break;
        case 3:
          if(BLDMOD & top)
            color = gfxDecreaseBrightness(color, coeff[COLY & 0x1F]);
          break;
        }         
      }
    }    
      
    lineMix[x] = color;
  }
  gfxBG2Changed = 0;
  gfxLastVCOUNT = VCOUNT;  
}

void mode3RenderLineAll()
{
  uint16 *palette = (uint16 *)paletteRAM;
  
  if(DISPCNT & 0x80) {
    for(int x = 0; x < 240; x++) {
      lineMix[x] = 0x7fff;
    }
    gfxLastVCOUNT = VCOUNT;    
    return;
  }

  bool8 inWindow0 = false;
  bool8 inWindow1 = false;

  if(layerEnable & 0x2000) {
    uint8 v0 = WIN0V >> 8;
    uint8 v1 = WIN0V & 255;
    inWindow0 = ((v0 == v1) && (v0 >= 0xe8));
    if(v1 >= v0)
      inWindow0 |= (VCOUNT >= v0 && VCOUNT < v1);
    else
      inWindow0 |= (VCOUNT >= v0 || VCOUNT < v1);
  }
  if(layerEnable & 0x4000) {
    uint8 v0 = WIN1V >> 8;
    uint8 v1 = WIN1V & 255;
    inWindow1 = ((v0 == v1) && (v0 >= 0xe8));    
    if(v1 >= v0)
      inWindow1 |= (VCOUNT >= v0 && VCOUNT < v1);
    else
      inWindow1 |= (VCOUNT >= v0 || VCOUNT < v1);
  }
  
  if(layerEnable & 0x0400) {
    int changed = gfxBG2Changed;

    if(gfxLastVCOUNT > VCOUNT)
      changed = 3;
    
    gfxDrawRotScreen16Bit(BG2CNT, BG2X_L, BG2X_H,
                          BG2Y_L, BG2Y_H, BG2PA, BG2PB,
                          BG2PC, BG2PD,
                          gfxBG2X, gfxBG2Y, changed,                      
                          line2);
  }

  gfxDrawSprites(lineOBJ);
  gfxDrawOBJWin(lineOBJWin);
  
  uint8 inWin0Mask = WININ & 0xFF;
  uint8 inWin1Mask = WININ >> 8;
  uint8 outMask = WINOUT & 0xFF;

  uint32 background = (READ16LE(&palette[0]) | 0x30000000);
  
  for(int x = 0; x < 240; x++) {
    uint32 color = background;
    uint8 top = 0x20;
    uint8 mask = outMask;

    if(!(lineOBJWin[x] & 0x80000000)) {
      mask = WINOUT >> 8;
    }

    if(inWindow1) {
      if(gfxInWin1[x])
        mask = inWin1Mask;
    }

    if(inWindow0) {
      if(gfxInWin0[x]) {
        mask = inWin0Mask;
      }
    }
    
    if((mask & 4) && (line2[x] < color)) {
      color = line2[x];
      top = 0x04;
    }

    if((mask & 16) && ((uint8)(lineOBJ[x]>>24) < (uint8)(color >>24))) {
      color = lineOBJ[x];
      top = 0x10;
    }

    if(mask & 32) {
      if(!(color & 0x00010000)) {
        switch((BLDMOD >> 6) & 3) {
        case 0:
          break;
        case 1:
          {
            if(top & BLDMOD) {
              uint32 back = background;
              uint8 top2 = 0x20;
              
              if((mask & 4) && line2[x] < back) {
                if(top != 0x04) {
                  back = line2[x];
                  top2 = 0x04;
                }
              }
              
              if((mask & 16) && (uint8)(lineOBJ[x]>>24) < (uint8)(back >> 24)) {
                if(top != 0x10) {
                  back = lineOBJ[x];
                  top2 = 0x10;
                }
              }
              
              if(top2 & (BLDMOD>>8))
                color = gfxAlphaBlend(color, back,
                                      coeff[COLEV & 0x1F],
                                      coeff[(COLEV >> 8) & 0x1F]);
              
            }
          }
          break;
        case 2:
          if(BLDMOD & top)
            color = gfxIncreaseBrightness(color, coeff[COLY & 0x1F]);
          break;
        case 3:
          if(BLDMOD & top)
            color = gfxDecreaseBrightness(color, coeff[COLY & 0x1F]);
          break;
        }
      } else {
        // semi-transparent OBJ
        uint32 back = background;
        uint8 top2 = 0x20;
        
        if((mask & 4) && line2[x] < back) {
          back = line2[x];
          top2 = 0x04;
        }
        
        if(top2 & (BLDMOD>>8))
          color = gfxAlphaBlend(color, back,
                                coeff[COLEV & 0x1F],
                                coeff[(COLEV >> 8) & 0x1F]);
        else {
          switch((BLDMOD >> 6) & 3) {
          case 2:
            if(BLDMOD & top)
              color = gfxIncreaseBrightness(color, coeff[COLY & 0x1F]);
            break;
          case 3:
            if(BLDMOD & top)
              color = gfxDecreaseBrightness(color, coeff[COLY & 0x1F]);
            break;
          }       
        }       
      }
    } else if(color & 0x00010000) {
      // semi-transparent OBJ
      uint32 back = background;
      uint8 top2 = 0x20;
      
      if((mask & 4) && line2[x] < back) {
        back = line2[x];
        top2 = 0x04;
      }
      
      if(top2 & (BLDMOD>>8))
        color = gfxAlphaBlend(color, back,
                              coeff[COLEV & 0x1F],
                              coeff[(COLEV >> 8) & 0x1F]);
      else {
        switch((BLDMOD >> 6) & 3) {
        case 2:
          if(BLDMOD & top)
            color = gfxIncreaseBrightness(color, coeff[COLY & 0x1F]);
          break;
        case 3:
          if(BLDMOD & top)
            color = gfxDecreaseBrightness(color, coeff[COLY & 0x1F]);
          break;
        }         
      }       
    }
      
    lineMix[x] = color;
  }
  gfxBG2Changed = 0;
  gfxLastVCOUNT = VCOUNT;  
}
