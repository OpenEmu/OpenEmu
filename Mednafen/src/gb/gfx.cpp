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

#include <memory.h>

#include "../mednafen.h"
#include "gbGlobals.h"
#include "gb.h"

static const uint8 gbInvertTab[256] = 
{
  0x00,0x80,0x40,0xc0,0x20,0xa0,0x60,0xe0,
  0x10,0x90,0x50,0xd0,0x30,0xb0,0x70,0xf0,
  0x08,0x88,0x48,0xc8,0x28,0xa8,0x68,0xe8,
  0x18,0x98,0x58,0xd8,0x38,0xb8,0x78,0xf8,
  0x04,0x84,0x44,0xc4,0x24,0xa4,0x64,0xe4,
  0x14,0x94,0x54,0xd4,0x34,0xb4,0x74,0xf4,
  0x0c,0x8c,0x4c,0xcc,0x2c,0xac,0x6c,0xec,
  0x1c,0x9c,0x5c,0xdc,0x3c,0xbc,0x7c,0xfc,
  0x02,0x82,0x42,0xc2,0x22,0xa2,0x62,0xe2,
  0x12,0x92,0x52,0xd2,0x32,0xb2,0x72,0xf2,
  0x0a,0x8a,0x4a,0xca,0x2a,0xaa,0x6a,0xea,
  0x1a,0x9a,0x5a,0xda,0x3a,0xba,0x7a,0xfa,
  0x06,0x86,0x46,0xc6,0x26,0xa6,0x66,0xe6,
  0x16,0x96,0x56,0xd6,0x36,0xb6,0x76,0xf6,
  0x0e,0x8e,0x4e,0xce,0x2e,0xae,0x6e,0xee,
  0x1e,0x9e,0x5e,0xde,0x3e,0xbe,0x7e,0xfe,
  0x01,0x81,0x41,0xc1,0x21,0xa1,0x61,0xe1,
  0x11,0x91,0x51,0xd1,0x31,0xb1,0x71,0xf1,
  0x09,0x89,0x49,0xc9,0x29,0xa9,0x69,0xe9,
  0x19,0x99,0x59,0xd9,0x39,0xb9,0x79,0xf9,
  0x05,0x85,0x45,0xc5,0x25,0xa5,0x65,0xe5,
  0x15,0x95,0x55,0xd5,0x35,0xb5,0x75,0xf5,
  0x0d,0x8d,0x4d,0xcd,0x2d,0xad,0x6d,0xed,
  0x1d,0x9d,0x5d,0xdd,0x3d,0xbd,0x7d,0xfd,
  0x03,0x83,0x43,0xc3,0x23,0xa3,0x63,0xe3,
  0x13,0x93,0x53,0xd3,0x33,0xb3,0x73,0xf3,
  0x0b,0x8b,0x4b,0xcb,0x2b,0xab,0x6b,0xeb,
  0x1b,0x9b,0x5b,0xdb,0x3b,0xbb,0x7b,0xfb,
  0x07,0x87,0x47,0xc7,0x27,0xa7,0x67,0xe7,
  0x17,0x97,0x57,0xd7,0x37,0xb7,0x77,0xf7,
  0x0f,0x8f,0x4f,0xcf,0x2f,0xaf,0x6f,0xef,
  0x1f,0x9f,0x5f,0xdf,0x3f,0xbf,0x7f,0xff
};

uint16 gbLineMix[160];

void gbRenderLine()
{
  uint8 * bank0;
  uint8 * bank1;

  if(gbCgbMode) {
    bank0 = &gbVram[0x0000];
    bank1 = &gbVram[0x2000];
  } else {
    bank0 = &gbVram[0x0000];
    bank1 = NULL;
  }
  
  int tile_map = 0x1800;
  if((register_LCDC & 8) != 0)
    tile_map = 0x1c00;

  int tile_pattern = 0x0800;

  if((register_LCDC & 16) != 0)
    tile_pattern = 0x0000;
    
  int x = 0;
  int y = register_LY;

  if(y >= 144)
    return;

  int sx = register_SCX;
  int sy = register_SCY;

  sy+=y;

  sy &= 255;
    
  int tx = sx >> 3;
  int ty = sy >> 3;

  int bx = 1 << (7 - (sx & 7));
  int by = sy & 7;

  int tile_map_line_y = tile_map + ty * 32;
  
  int tile_map_address = tile_map_line_y + tx;

  uint8 attrs = 0;
  if(bank1 != NULL)
    attrs = bank1[tile_map_address];
  
  uint8 tile = bank0[tile_map_address];
  
  tile_map_address++;
  
  if((register_LCDC & 16) == 0) {
    if(tile < 128) tile += 128;
    else tile -= 128;
  }

  int tile_pattern_address = tile_pattern + tile * 16 + by*2;

  if(register_LCDC & 0x80) 
  {
    if((register_LCDC & 0x01 || gbCgbMode) && (gblayerSettings & 0x01)) 
    {
      while(x < 160) {
        uint8 tile_a = 0;
        uint8 tile_b = 0;
        
        if(attrs & 0x40) {
          tile_pattern_address = tile_pattern + tile * 16 + (7-by)*2;
        }
        
        if(attrs & 0x08) {
          tile_a = bank1[tile_pattern_address++];
          tile_b = bank1[tile_pattern_address];
        } else {
          tile_a = bank0[tile_pattern_address++];
          tile_b = bank0[tile_pattern_address];
        }
        
        if(attrs & 0x20) {
          tile_a = gbInvertTab[tile_a];
          tile_b = gbInvertTab[tile_b];
        }
        
        while(bx > 0) {
          uint8 c = (tile_a & bx) ? 1 : 0;
          c += ((tile_b & bx) ? 2 : 0);
          
          gbLineBuffer[x] = c; // mark the gbLineBuffer color
          
          if(attrs & 0x80)
            gbLineBuffer[x] |= 0x300;
          
          if(gbCgbMode) 
	  {
           c = c + (attrs & 7)*4;
           gbLineMix[x] = gbPalette[c];
          } 
	  else 
	  {
	   gbLineMix[x] = gbBgp[c];
          }

          x++;
          if(x >= 160)
            break;
          bx >>= 1;
        }
        tx++;
        if(tx == 32)
          tx = 0;
        bx = 128;
        
        if(bank1)
          attrs = bank1[tile_map_line_y + tx];
        
        tile = bank0[tile_map_line_y + tx];
        
        if((register_LCDC & 16) == 0) {
          if(tile < 128) tile += 128;
          else tile -= 128;
        }
        tile_pattern_address = tile_pattern + tile * 16 + by * 2;
      }
    } 
    else 
    {
     int fill_color = gbCgbMode ? gbPalette[0] : 0;

     for(int i = 0; i < 160; i++) 
     {
      gbLineMix[i] = fill_color;
      gbLineBuffer[i] = 0;
     }
    }
    
    // do the window display
    if((register_LCDC & 0x20) && (gblayerSettings & 0x02)) {
      int wy = register_WY;
      
      if(y >= wy) {
        int wx = register_WX;
        wx -= 7;
        
        if( wx <= 159 && gbWindowLine <= 143) {
          
          tile_map = 0x1800;
          
          if((register_LCDC & 0x40) != 0)
            tile_map = 0x1c00;
          
          if(gbWindowLine == -1) {
            gbWindowLine = 0;
          }
          
          tx = 0;
          ty = gbWindowLine >> 3;
          
          bx = 128;
          by = gbWindowLine & 7;
          
          if(wx < 0) {
            bx >>= (-wx);
            wx = 0;
          }
          
          tile_map_line_y = tile_map + ty * 32;
          
          tile_map_address = tile_map_line_y + tx;
          
          x = wx;
          
          tile = bank0[tile_map_address];

          attrs = 0;

          if(bank1)
            attrs = bank1[tile_map_address];
          tile_map_address++;
          
          if((register_LCDC & 16) == 0) {    
            if(tile < 128) tile += 128;
            else tile -= 128;
          }
          
          tile_pattern_address = tile_pattern + tile * 16 + by*2;

          while(x < 160) {
            uint8 tile_a = 0;
            uint8 tile_b = 0;
            
            if(attrs & 0x40) {
              tile_pattern_address = tile_pattern + tile * 16 + (7-by)*2;
            }
            
            if(attrs & 0x08) {
              tile_a = bank1[tile_pattern_address++];
              tile_b = bank1[tile_pattern_address];
            } else {
              tile_a = bank0[tile_pattern_address++];
              tile_b = bank0[tile_pattern_address];
            }
            
            if(attrs & 0x20) {
              tile_a = gbInvertTab[tile_a];
              tile_b = gbInvertTab[tile_b];
            }
            
            while(bx > 0) {
              uint8 c = (tile_a & bx) != 0 ? 1 : 0;
              c += ((tile_b & bx) != 0 ? 2 : 0);

              if(attrs & 0x80)
                gbLineBuffer[x] = 0x300 + c;
              else
                gbLineBuffer[x] = 0x100 + c;
              
              if(gbCgbMode) 
	      {
               c = c + (attrs & 7) * 4;
               gbLineMix[x] = gbPalette[c];
              } 
	      else 
	      {
               gbLineMix[x] = gbBgp[c];
              }
  
              x++;
              if(x >= 160)
                break;
              bx >>= 1;
            }
            tx++;
            if(tx == 32)
              tx = 0;
            bx = 128;
            tile = bank0[tile_map_line_y + tx];
            if(bank1)
              attrs = bank1[tile_map_line_y + tx];
            
            if((register_LCDC & 16) == 0) {         
              if(tile < 128) tile += 128;
              else tile -= 128;
            }
            tile_pattern_address = tile_pattern + tile * 16 + by * 2;
          }
          gbWindowLine++;
        }
      }
    }
  }
  else 
  {
   int fill_color = gbCgbMode ? gbPalette[0] : 8;

   for(int i = 0; i < 160; i++) 
   {
    gbLineMix[i] = fill_color;
    gbLineBuffer[i] = 0;
   }
  }
}

void gbDrawSpriteTile(int tile, int x,int y,int t, int flags,
                      int size,int spriteNumber)
{
  uint8 * bank0;
  uint8 * bank1;
  if(gbCgbMode) {
    if(register_VBK & 1) {
      bank0 = &gbVram[0x0000];
      bank1 = &gbVram[0x2000];
    } else {
      bank0 = &gbVram[0x0000];
      bank1 = &gbVram[0x2000];
    }
  } else {
    bank0 = &gbVram[0x0000];
    bank1 = NULL;
  }
  
  int init = 0x0000;

  uint8 *pal = gbObp0;

  int flipx = (flags & 0x20);
  int flipy = (flags & 0x40);
  
  if((flags & 0x10))
    pal = gbObp1;

  if(flipy) {
    t = (size ? 15 : 7) - t;
  }

  int prio =  flags & 0x80;
  
  int address = init + tile * 16 + 2*t;
  int a = 0;
  int b = 0;

  if(gbCgbMode && flags & 0x08) {
    a = bank1[address++];
    b = bank1[address++];
  } else {
    a = bank0[address++];
    b = bank0[address++];
  }
  
  for(int xx = 0; xx < 8; xx++) {
    uint8 mask = 1 << (7-xx);
    uint8 c = 0;
    if( (a & mask))
      c++;
    if( (b & mask))
      c+=2;
    
    if(c==0) continue;

    int xxx = xx+x;
    if(flipx)
      xxx = (7-xx+x);

    if(xxx < 0 || xxx > 159)
      continue;

    uint16 color = gbLineBuffer[xxx];
    
    if(prio) {
      if(color < 0x200 && ((color & 0xFF) != 0))
        continue;
    }
    if(color >= 0x300 && color != 0x300)
      continue;
    else if(color >= 0x200 && color < 0x300) {
      int sprite = color & 0xff;

      int spriteX = gbOAM[4 * sprite + 1] - 8;

      if(spriteX == x) {
        if(sprite < spriteNumber)
          continue;
      } else {
        if(gbCgbMode) {
          if(sprite < spriteNumber)
            continue;
        } else {
          if(spriteX < x+8)
            continue;
        }
      }
    } 
    

    gbLineBuffer[xxx] = 0x200 + spriteNumber;

    // make sure that sprites will work even in CGB mode
    if(gbCgbMode) 
    {
     c = c + (flags & 0x07)*4 + 32;
     gbLineMix[xxx] = gbPalette[c];
    } 
    else 
    {
     c = pal[c];
     c += 4;
     gbLineMix[xxx] = c;
    }
  }
}

void gbDrawSprites()
{
  int x = 0;
  int y = 0;
  int count = 0;
  
  int size = (register_LCDC & 4);

  if(!(register_LCDC & 0x80))
    return;

  if((register_LCDC & 2) && (gblayerSettings & 0x04)) {
    int yc = register_LY;
      
    int address = 0;
    for(int i = 0; i < 40; i++) {
      y = gbOAM[address++];
      x = gbOAM[address++];
      int tile = gbOAM[address++];
      if(size)
        tile &= 254;
      int flags = gbOAM[address++];

      if(x > 0 && y > 0 && x < 168 && y < 160) {
        // check if sprite intersects current line
        int t = yc -y + 16;
        if(size && t >=0 && t < 16) {
          gbDrawSpriteTile(tile,x-8,yc,t,flags,size,i);
          count++;
        } else if(!size && t >= 0 && t < 8) {
          gbDrawSpriteTile(tile, x-8, yc, t, flags,size,i);
          count++;
        }
      }
      // sprite limit reached!
      if(count >= 10)
        break;
    }   
  }
}

