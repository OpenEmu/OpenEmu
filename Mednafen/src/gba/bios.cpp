// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2005 Forgotten and the VBA development team

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

#include <math.h>
#include <memory.h>

#include "GBA.h"
#include "bios.h"
#include "GBAinline.h"
#include "Globals.h"

int16 sineTable[256] = {
  (int16)0x0000, (int16)0x0192, (int16)0x0323, (int16)0x04B5, (int16)0x0645, (int16)0x07D5, (int16)0x0964, (int16)0x0AF1,
  (int16)0x0C7C, (int16)0x0E05, (int16)0x0F8C, (int16)0x1111, (int16)0x1294, (int16)0x1413, (int16)0x158F, (int16)0x1708,
  (int16)0x187D, (int16)0x19EF, (int16)0x1B5D, (int16)0x1CC6, (int16)0x1E2B, (int16)0x1F8B, (int16)0x20E7, (int16)0x223D,
  (int16)0x238E, (int16)0x24DA, (int16)0x261F, (int16)0x275F, (int16)0x2899, (int16)0x29CD, (int16)0x2AFA, (int16)0x2C21,
  (int16)0x2D41, (int16)0x2E5A, (int16)0x2F6B, (int16)0x3076, (int16)0x3179, (int16)0x3274, (int16)0x3367, (int16)0x3453,
  (int16)0x3536, (int16)0x3612, (int16)0x36E5, (int16)0x37AF, (int16)0x3871, (int16)0x392A, (int16)0x39DA, (int16)0x3A82,
  (int16)0x3B20, (int16)0x3BB6, (int16)0x3C42, (int16)0x3CC5, (int16)0x3D3E, (int16)0x3DAE, (int16)0x3E14, (int16)0x3E71,
  (int16)0x3EC5, (int16)0x3F0E, (int16)0x3F4E, (int16)0x3F84, (int16)0x3FB1, (int16)0x3FD3, (int16)0x3FEC, (int16)0x3FFB,
  (int16)0x4000, (int16)0x3FFB, (int16)0x3FEC, (int16)0x3FD3, (int16)0x3FB1, (int16)0x3F84, (int16)0x3F4E, (int16)0x3F0E,
  (int16)0x3EC5, (int16)0x3E71, (int16)0x3E14, (int16)0x3DAE, (int16)0x3D3E, (int16)0x3CC5, (int16)0x3C42, (int16)0x3BB6,
  (int16)0x3B20, (int16)0x3A82, (int16)0x39DA, (int16)0x392A, (int16)0x3871, (int16)0x37AF, (int16)0x36E5, (int16)0x3612,
  (int16)0x3536, (int16)0x3453, (int16)0x3367, (int16)0x3274, (int16)0x3179, (int16)0x3076, (int16)0x2F6B, (int16)0x2E5A,
  (int16)0x2D41, (int16)0x2C21, (int16)0x2AFA, (int16)0x29CD, (int16)0x2899, (int16)0x275F, (int16)0x261F, (int16)0x24DA,
  (int16)0x238E, (int16)0x223D, (int16)0x20E7, (int16)0x1F8B, (int16)0x1E2B, (int16)0x1CC6, (int16)0x1B5D, (int16)0x19EF,
  (int16)0x187D, (int16)0x1708, (int16)0x158F, (int16)0x1413, (int16)0x1294, (int16)0x1111, (int16)0x0F8C, (int16)0x0E05,
  (int16)0x0C7C, (int16)0x0AF1, (int16)0x0964, (int16)0x07D5, (int16)0x0645, (int16)0x04B5, (int16)0x0323, (int16)0x0192,
  (int16)0x0000, (int16)0xFE6E, (int16)0xFCDD, (int16)0xFB4B, (int16)0xF9BB, (int16)0xF82B, (int16)0xF69C, (int16)0xF50F,
  (int16)0xF384, (int16)0xF1FB, (int16)0xF074, (int16)0xEEEF, (int16)0xED6C, (int16)0xEBED, (int16)0xEA71, (int16)0xE8F8,
  (int16)0xE783, (int16)0xE611, (int16)0xE4A3, (int16)0xE33A, (int16)0xE1D5, (int16)0xE075, (int16)0xDF19, (int16)0xDDC3,
  (int16)0xDC72, (int16)0xDB26, (int16)0xD9E1, (int16)0xD8A1, (int16)0xD767, (int16)0xD633, (int16)0xD506, (int16)0xD3DF,
  (int16)0xD2BF, (int16)0xD1A6, (int16)0xD095, (int16)0xCF8A, (int16)0xCE87, (int16)0xCD8C, (int16)0xCC99, (int16)0xCBAD,
  (int16)0xCACA, (int16)0xC9EE, (int16)0xC91B, (int16)0xC851, (int16)0xC78F, (int16)0xC6D6, (int16)0xC626, (int16)0xC57E,
  (int16)0xC4E0, (int16)0xC44A, (int16)0xC3BE, (int16)0xC33B, (int16)0xC2C2, (int16)0xC252, (int16)0xC1EC, (int16)0xC18F,
  (int16)0xC13B, (int16)0xC0F2, (int16)0xC0B2, (int16)0xC07C, (int16)0xC04F, (int16)0xC02D, (int16)0xC014, (int16)0xC005,
  (int16)0xC000, (int16)0xC005, (int16)0xC014, (int16)0xC02D, (int16)0xC04F, (int16)0xC07C, (int16)0xC0B2, (int16)0xC0F2,
  (int16)0xC13B, (int16)0xC18F, (int16)0xC1EC, (int16)0xC252, (int16)0xC2C2, (int16)0xC33B, (int16)0xC3BE, (int16)0xC44A,
  (int16)0xC4E0, (int16)0xC57E, (int16)0xC626, (int16)0xC6D6, (int16)0xC78F, (int16)0xC851, (int16)0xC91B, (int16)0xC9EE,
  (int16)0xCACA, (int16)0xCBAD, (int16)0xCC99, (int16)0xCD8C, (int16)0xCE87, (int16)0xCF8A, (int16)0xD095, (int16)0xD1A6,
  (int16)0xD2BF, (int16)0xD3DF, (int16)0xD506, (int16)0xD633, (int16)0xD767, (int16)0xD8A1, (int16)0xD9E1, (int16)0xDB26,
  (int16)0xDC72, (int16)0xDDC3, (int16)0xDF19, (int16)0xE075, (int16)0xE1D5, (int16)0xE33A, (int16)0xE4A3, (int16)0xE611,
  (int16)0xE783, (int16)0xE8F8, (int16)0xEA71, (int16)0xEBED, (int16)0xED6C, (int16)0xEEEF, (int16)0xF074, (int16)0xF1FB,
  (int16)0xF384, (int16)0xF50F, (int16)0xF69C, (int16)0xF82B, (int16)0xF9BB, (int16)0xFB4B, (int16)0xFCDD, (int16)0xFE6E
};

void BIOS_ArcTan()
{
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("ArcTan: %08x (VCOUNT=%2d)\n",
        reg[0].I,
        VCOUNT);
  }
#endif

  int32 a =  -(((int32)(reg[0].I*reg[0].I)) >> 14);
  int32 b = ((0xA9 * a) >> 14) + 0x390;
  b = ((b * a) >> 14) + 0x91C;
  b = ((b * a) >> 14) + 0xFB6;
  b = ((b * a) >> 14) + 0x16AA;
  b = ((b * a) >> 14) + 0x2081;
  b = ((b * a) >> 14) + 0x3651;
  b = ((b * a) >> 14) + 0xA2F9;
  a = ((int32)reg[0].I * b) >> 16;
  reg[0].I = a;

#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("ArcTan: return=%08x\n",
        reg[0].I);
  }
#endif
}

void BIOS_ArcTan2()
{
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("ArcTan2: %08x,%08x (VCOUNT=%2d)\n",
        reg[0].I,
        reg[1].I,
        VCOUNT);
  }
#endif
  
  int32 x = reg[0].I;
  int32 y = reg[1].I;
  uint32 res = 0;
  if (y == 0) {
    res = ((x>>16) & 0x8000);
  } else {
    if (x == 0) {
      res = ((y>>16) & 0x8000) + 0x4000;
    } else {
		if ((abs(x) > abs(y)) || ((abs(x) == abs(y)) && (!((x<0) && (y<0))))) {
        reg[1].I = x;
        reg[0].I = y << 14;
        BIOS_Div();
        BIOS_ArcTan();
        if (x < 0)
          res = 0x8000 + reg[0].I;
        else
          res = (((y>>16) & 0x8000)<<1) + reg[0].I;
      } else {
        reg[0].I = x << 14;
        BIOS_Div();
        BIOS_ArcTan();
        res = (0x4000 + ((y>>16) & 0x8000)) - reg[0].I;
      }
    }
  }
  reg[0].I = res;
  
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("ArcTan2: return=%08x\n",
        reg[0].I);
  }
#endif
}  

void BIOS_BitUnPack()
{
#ifdef DEV_VERSION  
  if(systemVerbose & VERBOSE_SWI) {
    log("BitUnPack: %08x,%08x,%08x (VCOUNT=%2d)\n",
        reg[0].I,
        reg[1].I,
        reg[2].I,
        VCOUNT);
  }
#endif

  uint32 source = reg[0].I;
  uint32 dest = reg[1].I;
  uint32 header = reg[2].I;
  
  int len = CPUReadHalfWord(header);
  // check address
  if(((source & 0xe000000) == 0) || ((source + len) & 0xe000000) == 0)
  	     return;
  int bits = CPUReadByte(header+2);
  int revbits = 8 - bits; 
  // uint32 value = 0;
  uint32 base = CPUReadMemory(header+4);
  bool8 addBase = (base & 0x80000000) ? true : false;
  base &= 0x7fffffff;
  int dataSize = CPUReadByte(header+3);

  int data = 0; 
  int bitwritecount = 0; 
  while(1) {
    len -= 1;
    if(len < 0)
      break;
    int mask = 0xff >> revbits; 
    uint8 b = CPUReadByte(source); 
    source++;
    int bitcount = 0;
    while(1) {
      if(bitcount >= 8)
        break;
      uint32 d = b & mask;
      uint32 temp = d >> bitcount;
      if(d || addBase) {
        temp += base;
      }
      data |= temp << bitwritecount;
      bitwritecount += dataSize;
      if(bitwritecount >= 32) {
        CPUWriteMemory(dest, data);
        dest += 4;
        data = 0;
        bitwritecount = 0;
      }
      mask <<= bits;
      bitcount += bits;
    }
  }
}

void BIOS_BgAffineSet()
{
#ifdef DEV_VERSION  
  if(systemVerbose & VERBOSE_SWI) {
    log("BgAffineSet: %08x,%08x,%08x (VCOUNT=%2d)\n",
        reg[0].I,
        reg[1].I,
        reg[2].I,
        VCOUNT);
  }
#endif
  
  uint32 src = reg[0].I;
  uint32 dest = reg[1].I;
  int num = reg[2].I;

  for(int i = 0; i < num; i++) {
    int32 cx = CPUReadMemory(src);
    src+=4;
    int32 cy = CPUReadMemory(src);
    src+=4;
    int16 dispx = CPUReadHalfWord(src);
    src+=2;
    int16 dispy = CPUReadHalfWord(src);
    src+=2;
    int16 rx = CPUReadHalfWord(src);
    src+=2;
    int16 ry = CPUReadHalfWord(src);
    src+=2;
    uint16 theta = CPUReadHalfWord(src)>>8;
    src+=4; // keep structure alignment
    int32 a = sineTable[(theta+0x40)&255];
    int32 b = sineTable[theta];

    int16 dx =  (rx * a)>>14;
    int16 dmx = (rx * b)>>14;
    int16 dy =  (ry * b)>>14;
    int16 dmy = (ry * a)>>14;
    
    CPUWriteHalfWord(dest, dx);
    dest += 2;
    CPUWriteHalfWord(dest, -dmx);
    dest += 2;
    CPUWriteHalfWord(dest, dy);
    dest += 2;
    CPUWriteHalfWord(dest, dmy);
    dest += 2;

    int32 startx = cx - dx * dispx + dmx * dispy;
    int32 starty = cy - dy * dispx - dmy * dispy;
    
    CPUWriteMemory(dest, startx);
    dest += 4;
    CPUWriteMemory(dest, starty);
    dest += 4;
  }
}  

void BIOS_CpuSet()
{
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("CpuSet: 0x%08x,0x%08x,0x%08x (VCOUNT=%d)\n", reg[0].I, reg[1].I,
        reg[2].I, VCOUNT);
  }
#endif
  
  uint32 source = reg[0].I;
  uint32 dest = reg[1].I;
  uint32 cnt = reg[2].I;

  if(((source & 0xe000000) == 0) ||
     ((source + (((cnt << 11)>>9) & 0x1fffff)) & 0xe000000) == 0)
    return;

  int count = cnt & 0x1FFFFF;

  // 32-bit ?
  if((cnt >> 26) & 1) {
    // needed for 32-bit mode!
    source &= 0xFFFFFFFC;
    dest &= 0xFFFFFFFC;
    // fill ?
    if((cnt >> 24) & 1) {
        uint32 value = (source>0x0EFFFFFF ? 0x1CAD1CAD : CPUReadMemory(source));
      while(count) {
        CPUWriteMemory(dest, value);
        dest += 4;
        count--;
      }
    } else {
      // copy
      while(count) {
        CPUWriteMemory(dest, (source>0x0EFFFFFF ? 0x1CAD1CAD : CPUReadMemory(source)));
        source += 4;
        dest += 4;
        count--;
      }
    }
  } else {
    // 16-bit fill?
    if((cnt >> 24) & 1) {
      uint16 value = (source>0x0EFFFFFF ? 0x1CAD : CPUReadHalfWord(source));
      while(count) {
        CPUWriteHalfWord(dest, value);
        dest += 2;
        count--;
      }
    } else {
      // copy
      while(count) {
        CPUWriteHalfWord(dest, (source>0x0EFFFFFF ? 0x1CAD : CPUReadHalfWord(source)));
        source += 2;
        dest += 2;
        count--;
      }
    }
  }
}

void BIOS_CpuFastSet()
{
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("CpuFastSet: 0x%08x,0x%08x,0x%08x (VCOUNT=%d)\n", reg[0].I, reg[1].I,
        reg[2].I, VCOUNT);
  }
#endif
  
  uint32 source = reg[0].I;
  uint32 dest = reg[1].I;
  uint32 cnt = reg[2].I;

  if(((source & 0xe000000) == 0) ||
     ((source + (((cnt << 11)>>9) & 0x1fffff)) & 0xe000000) == 0)
    return;

  // needed for 32-bit mode!
  source &= 0xFFFFFFFC;
  dest &= 0xFFFFFFFC;
  
  int count = cnt & 0x1FFFFF;
  
  // fill?
  if((cnt >> 24) & 1) {
    while(count > 0) {
      // BIOS always transfers 32 bytes at a time
      uint32 value = (source>0x0EFFFFFF ? 0xBAFFFFFB : CPUReadMemory(source));
      for(int i = 0; i < 8; i++) {
        CPUWriteMemory(dest, value);
        dest += 4;
      }
      count -= 8;
    }
  } else {
    // copy
    while(count > 0) {
      // BIOS always transfers 32 bytes at a time
      for(int i = 0; i < 8; i++) {
        CPUWriteMemory(dest, (source>0x0EFFFFFF ? 0xBAFFFFFB :CPUReadMemory(source)));
        source += 4;
        dest += 4;
      }
      count -= 8;
    }
  }
}

void BIOS_Diff8bitUnFilterWram()
{
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("Diff8bitUnFilterWram: 0x%08x,0x%08x (VCOUNT=%d)\n", reg[0].I,
        reg[1].I, VCOUNT);
  }
#endif
  
  uint32 source = reg[0].I;
  uint32 dest = reg[1].I;

  uint32 header = CPUReadMemory(source);
  source += 4;

  if(((source & 0xe000000) == 0) ||
     ((source + ((header >> 8) & 0x1fffff) & 0xe000000) == 0))
    return;  

  int len = header >> 8;

  uint8 data = CPUReadByte(source++);
  CPUWriteByte(dest++, data);
  len--;
  
  while(len > 0) {
    uint8 diff = CPUReadByte(source++);
    data += diff;
    CPUWriteByte(dest++, data);
    len--;
  }    
}

void BIOS_Diff8bitUnFilterVram()
{
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("Diff8bitUnFilterVram: 0x%08x,0x%08x (VCOUNT=%d)\n", reg[0].I,
        reg[1].I, VCOUNT);
  }
#endif
  
  uint32 source = reg[0].I;
  uint32 dest = reg[1].I;

  uint32 header = CPUReadMemory(source);
  source += 4;

  if(((source & 0xe000000) == 0) ||
     ((source + ((header >> 8) & 0x1fffff)) & 0xe000000) == 0)
    return;  
  
  int len = header >> 8;

  uint8 data = CPUReadByte(source++);
  uint16 writeData = data;
  int shift = 8;
  int bytes = 1;
  
  while(len >= 2) {
    uint8 diff = CPUReadByte(source++);
    data += diff;
    writeData |= (data << shift);
    bytes++;
    shift += 8;
    if(bytes == 2) {
      CPUWriteHalfWord(dest, writeData);
      dest += 2;
      len -= 2;
      bytes = 0;
      writeData = 0;
      shift = 0;
    }
  }  
}

void BIOS_Diff16bitUnFilter()
{
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("Diff16bitUnFilter: 0x%08x,0x%08x (VCOUNT=%d)\n", reg[0].I,
        reg[1].I, VCOUNT);
  }
#endif
  
  uint32 source = reg[0].I;
  uint32 dest = reg[1].I;

  uint32 header = CPUReadMemory(source);
  source += 4;

  if(((source & 0xe000000) == 0) ||
     ((source + ((header >> 8) & 0x1fffff)) & 0xe000000) == 0)
    return;  
  
  int len = header >> 8;

  uint16 data = CPUReadHalfWord(source);
  source += 2;
  CPUWriteHalfWord(dest, data);
  dest += 2;
  len -= 2;
  
  while(len >= 2) {
    uint16 diff = CPUReadHalfWord(source);
    source += 2;
    data += diff;
    CPUWriteHalfWord(dest, data);
    dest += 2;
    len -= 2;
  }
}

void BIOS_Div()
{
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("Div: 0x%08x,0x%08x (VCOUNT=%d)\n",
        reg[0].I,
        reg[1].I,
        VCOUNT);
  }
#endif
  
  int number = reg[0].I;
  int denom = reg[1].I;

  if(denom != 0) {
    reg[0].I = number / denom;
    reg[1].I = number % denom;
    int32 temp = (int32)reg[0].I;
    reg[3].I = temp < 0 ? (uint32)-temp : (uint32)temp;
  }
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("Div: return=0x%08x,0x%08x,0x%08x\n",
        reg[0].I,
        reg[1].I,
        reg[3].I);
  }
#endif
}

void BIOS_DivARM()
{
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("DivARM: 0x%08x, (VCOUNT=%d)\n",
        reg[0].I,
        VCOUNT);
  }
#endif
  
  uint32 temp = reg[0].I;
  reg[0].I = reg[1].I;
  reg[1].I = temp;
  BIOS_Div();
}

void BIOS_HuffUnComp()
{
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("HuffUnComp: 0x%08x,0x%08x (VCOUNT=%d)\n",
        reg[0].I,
        reg[1].I,
        VCOUNT);
  }
#endif
  
  uint32 source = reg[0].I;
  uint32 dest = reg[1].I;

  uint32 header = CPUReadMemory(source);
  source += 4;

  if(((source & 0xe000000) == 0) ||
     ((source + ((header >> 8) & 0x1fffff)) & 0xe000000) == 0)
    return;  
  
  uint8 treeSize = CPUReadByte(source++);

  uint32 treeStart = source;

  source += ((treeSize+1)<<1)-1; // minus because we already skipped one byte
  
  int len = header >> 8;

  uint32 mask = 0x80000000;
  uint32 data = CPUReadMemory(source);
  source += 4;

  int pos = 0;
  uint8 rootNode = CPUReadByte(treeStart);
  uint8 currentNode = rootNode;
  bool8 writeData = false;
  int byteShift = 0;
  int byteCount = 0;
  uint32 writeValue = 0;

  if((header & 0x0F) == 8) {
    while(len > 0) {
      // take left
      if(pos == 0)
        pos++;
      else
        pos += (((currentNode & 0x3F)+1)<<1);
      
      if(data & mask) {
        // right
        if(currentNode & 0x40)
          writeData = true;
        currentNode = CPUReadByte(treeStart+pos+1);
      } else {
        // left
        if(currentNode & 0x80)
          writeData = true;
        currentNode = CPUReadByte(treeStart+pos);
      }
      
      if(writeData) {
        writeValue |= (currentNode << byteShift);
        byteCount++;
        byteShift += 8;

        pos = 0;
        currentNode = rootNode;
        writeData = false;

        if(byteCount == 4) {
          byteCount = 0;
          byteShift = 0;
          CPUWriteMemory(dest, writeValue);
          writeValue = 0;
          dest += 4;
          len -= 4;
        }
      }
      mask >>= 1;
      if(mask == 0) {
        mask = 0x80000000;
        data = CPUReadMemory(source);
        source += 4;
      }
    }
  } else {
    int halfLen = 0;
    int value = 0;
    while(len > 0) {
      // take left
      if(pos == 0)
        pos++;
      else
        pos += (((currentNode & 0x3F)+1)<<1);

      if((data & mask)) {
        // right
        if(currentNode & 0x40)
          writeData = true;
        currentNode = CPUReadByte(treeStart+pos+1);
      } else {
        // left
        if(currentNode & 0x80)
          writeData = true;
        currentNode = CPUReadByte(treeStart+pos);
      }
      
      if(writeData) {
        if(halfLen == 0)
          value |= currentNode;
        else
          value |= (currentNode<<4);

        halfLen += 4;
        if(halfLen == 8) {
          writeValue |= (value << byteShift);
          byteCount++;
          byteShift += 8;
          
          halfLen = 0;
          value = 0;

          if(byteCount == 4) {
            byteCount = 0;
            byteShift = 0;
            CPUWriteMemory(dest, writeValue);
            dest += 4;
            writeValue = 0;
            len -= 4;
          }
        }
        pos = 0;
        currentNode = rootNode;
        writeData = false;
      }
      mask >>= 1;
      if(mask == 0) {
        mask = 0x80000000;
        data = CPUReadMemory(source);
        source += 4;
      }
    }    
  }
}

void BIOS_LZ77UnCompVram()
{
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("LZ77UnCompVram: 0x%08x,0x%08x (VCOUNT=%d)\n",
        reg[0].I,
        reg[1].I,
        VCOUNT);
  }
#endif
  
  uint32 source = reg[0].I;
  uint32 dest = reg[1].I;

  uint32 header = CPUReadMemory(source);
  source += 4;

  if(((source & 0xe000000) == 0) ||
     ((source + ((header >> 8) & 0x1fffff)) & 0xe000000) == 0)
    return;    
  
  int byteCount = 0;
  int byteShift = 0;
  uint32 writeValue = 0;
  
  int len = header >> 8;

  while(len > 0) {
    uint8 d = CPUReadByte(source++);

    if(d) {
      for(int i = 0; i < 8; i++) {
        if(d & 0x80) {
          uint16 data = CPUReadByte(source++) << 8;
          data |= CPUReadByte(source++);
          int length = (data >> 12) + 3;
          int offset = (data & 0x0FFF);
          uint32 windowOffset = dest + byteCount - offset - 1;
          for(int i = 0; i < length; i++) {
            writeValue |= (CPUReadByte(windowOffset++) << byteShift);
            byteShift += 8;
            byteCount++;

            if(byteCount == 2) {
              CPUWriteHalfWord(dest, writeValue);
              dest += 2;
              byteCount = 0;
              byteShift = 0;
              writeValue = 0;
            }
            len--;
            if(len == 0)
              return;
          }
        } else {
          writeValue |= (CPUReadByte(source++) << byteShift);
          byteShift += 8;
          byteCount++;
          if(byteCount == 2) {
            CPUWriteHalfWord(dest, writeValue);
            dest += 2;
            byteCount = 0;
            byteShift = 0;
            writeValue = 0;
          }
          len--;
          if(len == 0)
            return;
        }
        d <<= 1;
      }
    } else {
      for(int i = 0; i < 8; i++) {
        writeValue |= (CPUReadByte(source++) << byteShift);
        byteShift += 8;
        byteCount++;
        if(byteCount == 2) {
          CPUWriteHalfWord(dest, writeValue);
          dest += 2;      
          byteShift = 0;
          byteCount = 0;
          writeValue = 0;
        }
        len--;
        if(len == 0)
          return;
      }
    }
  }
}

void BIOS_LZ77UnCompWram()
{
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("LZ77UnCompWram: 0x%08x,0x%08x (VCOUNT=%d)\n", reg[0].I, reg[1].I,
        VCOUNT);
  }
#endif
  
  uint32 source = reg[0].I;
  uint32 dest = reg[1].I;

  uint32 header = CPUReadMemory(source);
  source += 4;

  if(((source & 0xe000000) == 0) ||
     ((source + ((header >> 8) & 0x1fffff)) & 0xe000000) == 0)
    return;  
  
  int len = header >> 8;

  while(len > 0) {
    uint8 d = CPUReadByte(source++);

    if(d) {
      for(int i = 0; i < 8; i++) {
        if(d & 0x80) {
          uint16 data = CPUReadByte(source++) << 8;
          data |= CPUReadByte(source++);
          int length = (data >> 12) + 3;
          int offset = (data & 0x0FFF);
          uint32 windowOffset = dest - offset - 1;
          for(int i = 0; i < length; i++) {
            CPUWriteByte(dest++, CPUReadByte(windowOffset++));
            len--;
            if(len == 0)
              return;
          }
        } else {
          CPUWriteByte(dest++, CPUReadByte(source++));
          len--;
          if(len == 0)
            return;
        }
        d <<= 1;
      }
    } else {
      for(int i = 0; i < 8; i++) {
        CPUWriteByte(dest++, CPUReadByte(source++));
        len--;
        if(len == 0)
          return;
      }
    }
  }
}

void BIOS_ObjAffineSet()
{
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("ObjAffineSet: 0x%08x,0x%08x,0x%08x,0x%08x (VCOUNT=%d)\n",
        reg[0].I,
        reg[1].I,
        reg[2].I,
        reg[3].I,
        VCOUNT);
  }
#endif
  
  uint32 src = reg[0].I;
  uint32 dest = reg[1].I;
  int num = reg[2].I;
  int offset = reg[3].I;

  for(int i = 0; i < num; i++) {
    int16 rx = CPUReadHalfWord(src);
    src+=2;
    int16 ry = CPUReadHalfWord(src);
    src+=2;
    uint16 theta = CPUReadHalfWord(src)>>8;
    src+=4; // keep structure alignment

    int32 a = (int32)sineTable[(theta+0x40)&255];
    int32 b = (int32)sineTable[theta];

    int16 dx =  ((int32)rx * a)>>14;
    int16 dmx = ((int32)rx * b)>>14;
    int16 dy =  ((int32)ry * b)>>14;
    int16 dmy = ((int32)ry * a)>>14;
    
    CPUWriteHalfWord(dest, dx);
    dest += offset;
    CPUWriteHalfWord(dest, -dmx);
    dest += offset;
    CPUWriteHalfWord(dest, dy);
    dest += offset;
    CPUWriteHalfWord(dest, dmy);
    dest += offset;
  }
}

void BIOS_RegisterRamReset(uint32 flags)
{
  // no need to trace here. this is only called directly from GBA.cpp
  // to emulate bios initialization
  
  CPUUpdateRegister(0x0, 0x80);

  if(flags) {
    if(flags & 0x01) {
      // clear work RAM
      memset(workRAM, 0, 0x40000);
    }
    if(flags & 0x02) {
      // clear internal RAM
      memset(internalRAM, 0, 0x7e00); // don't clear 0x7e00-0x7fff
    }
    if(flags & 0x04) {
      // clear palette RAM
      memset(paletteRAM, 0, 0x400);
    }
    if(flags & 0x08) {
      // clear VRAM
      memset(vram, 0, 0x18000);
    }
    if(flags & 0x10) {
      // clean OAM
      memset(oam, 0, 0x400);
    }

    if(flags & 0x80) {
      int i;
      for(i = 0; i < 0x10; i++)
        CPUUpdateRegister(0x200+i*2, 0);

      for(i = 0; i < 0xF; i++)
        CPUUpdateRegister(0x4+i*2, 0);

      for(i = 0; i < 0x20; i++)
        CPUUpdateRegister(0x20+i*2, 0);

      for(i = 0; i < 0x18; i++)
        CPUUpdateRegister(0xb0+i*2, 0);

      CPUUpdateRegister(0x130, 0);
      CPUUpdateRegister(0x20, 0x100);
      CPUUpdateRegister(0x30, 0x100);
      CPUUpdateRegister(0x26, 0x100);
      CPUUpdateRegister(0x36, 0x100);
    }
    
    if(flags & 0x20) {
      int i;
      for(i = 0; i < 8; i++)
        CPUUpdateRegister(0x110+i*2, 0);
      CPUUpdateRegister(0x134, 0x8000);
      for(i = 0; i < 7; i++)
        CPUUpdateRegister(0x140+i*2, 0);
    }

    if(flags & 0x40) {
      int i;
      CPUWriteByte(0x4000084, 0);
      CPUWriteByte(0x4000084, 0x80);
      CPUWriteMemory(0x4000080, 0x880e0000);
      CPUUpdateRegister(0x88, CPUReadHalfWord(0x4000088)&0x3ff);
      CPUWriteByte(0x4000070, 0x70);
      for(i = 0; i < 8; i++)
        CPUUpdateRegister(0x90+i*2, 0);
      CPUWriteByte(0x4000070, 0);
      for(i = 0; i < 8; i++)
        CPUUpdateRegister(0x90+i*2, 0);
      CPUWriteByte(0x4000084, 0);
    }
  }
}

void BIOS_RegisterRamReset()
{
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("RegisterRamReset: 0x%08x (VCOUNT=%d)\n",
        reg[0].I,
        VCOUNT);
  }
#endif

  BIOS_RegisterRamReset(reg[0].I);
}

void BIOS_RLUnCompVram()
{
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("RLUnCompVram: 0x%08x,0x%08x (VCOUNT=%d)\n",
        reg[0].I,
        reg[1].I,
        VCOUNT);
  }
#endif
  
  uint32 source = reg[0].I;
  uint32 dest = reg[1].I;

  uint32 header = CPUReadMemory(source & 0xFFFFFFFC);
  source += 4;

  if(((source & 0xe000000) == 0) ||
     ((source + ((header >> 8) & 0x1fffff)) & 0xe000000) == 0)
    return;  
  
  int len = header >> 8;
  int byteCount = 0;
  int byteShift = 0;
  uint32 writeValue = 0;

  while(len > 0) {
    uint8 d = CPUReadByte(source++);
    int l = d & 0x7F;
    if(d & 0x80) {
      uint8 data = CPUReadByte(source++);
      l += 3;
      for(int i = 0;i < l; i++) {
        writeValue |= (data << byteShift);
        byteShift += 8;
        byteCount++;

        if(byteCount == 2) {
          CPUWriteHalfWord(dest, writeValue);
          dest += 2;
          byteCount = 0;
          byteShift = 0;
          writeValue = 0;
        }
        len--;
        if(len == 0)
          return;
      }
    } else {
      l++;
      for(int i = 0; i < l; i++) {
        writeValue |= (CPUReadByte(source++) << byteShift);
        byteShift += 8;
        byteCount++;
        if(byteCount == 2) {
          CPUWriteHalfWord(dest, writeValue);
          dest += 2;
          byteCount = 0;
          byteShift = 0;
          writeValue = 0;
        }
        len--;
        if(len == 0)
          return;
      }
    }
  }
}

void BIOS_RLUnCompWram()
{
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("RLUnCompWram: 0x%08x,0x%08x (VCOUNT=%d)\n",
        reg[0].I,
        reg[1].I,
        VCOUNT);
  }
#endif
  
  uint32 source = reg[0].I;
  uint32 dest = reg[1].I;

  uint32 header = CPUReadMemory(source & 0xFFFFFFFC);
  source += 4;

  if(((source & 0xe000000) == 0) ||
     ((source + ((header >> 8) & 0x1fffff)) & 0xe000000) == 0)
    return;  
  
  int len = header >> 8;

  while(len > 0) {
    uint8 d = CPUReadByte(source++);
    int l = d & 0x7F;
    if(d & 0x80) {
      uint8 data = CPUReadByte(source++);
      l += 3;
      for(int i = 0;i < l; i++) {
        CPUWriteByte(dest++, data);
        len--;
        if(len == 0)
          return;
      }
    } else {
      l++;
      for(int i = 0; i < l; i++) {
        CPUWriteByte(dest++,  CPUReadByte(source++));
        len--;
        if(len == 0)
          return;
      }
    }
  }
}

void BIOS_SoftReset()
{
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("SoftReset: (VCOUNT=%d)\n", VCOUNT);
  }
#endif

  armState = true;
  armMode = 0x1F;
  armIrqEnable = false;
  C_FLAG = V_FLAG = Z_FLAG = false;
  N_FLAG = 0;

  reg[13].I = 0x03007F00;
  reg[14].I = 0x00000000;
  reg[16].I = 0x00000000;
  reg[R13_IRQ].I = 0x03007FA0;
  reg[R14_IRQ].I = 0x00000000;
  reg[SPSR_IRQ].I = 0x00000000;
  reg[R13_SVC].I = 0x03007FE0;  
  reg[R14_SVC].I = 0x00000000;
  reg[SPSR_SVC].I = 0x00000000;
  uint8 b = internalRAM[0x7ffa];

  memset(&internalRAM[0x7e00], 0, 0x200);

  if(b) {
    armNextPC = 0x02000000;
    reg[15].I = 0x02000004;
  } else {
    armNextPC = 0x08000000;
    reg[15].I = 0x08000004;
  }
}

void BIOS_Sqrt()
{
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("Sqrt: %08x (VCOUNT=%2d)\n",
        reg[0].I,
        VCOUNT);
  }
#endif
  reg[0].I = (uint32)sqrt((double)reg[0].I);
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("Sqrt: return=%08x\n",
        reg[0].I);
  }
#endif
}

void BIOS_MidiKey2Freq()
{
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("MidiKey2Freq: WaveData=%08x mk=%08x fp=%08x\n",
        reg[0].I,
        reg[1].I,
        reg[2].I);
  }
#endif
  int freq = CPUReadMemory(reg[0].I+4);
  double tmp;
  tmp = ((double)(180 - reg[1].I)) - ((double)reg[2].I / 256.f);
  tmp = pow((double)2.f, tmp / 12.f);
  reg[0].I = (int)((double)freq / tmp);

#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("MidiKey2Freq: return %08x\n",
        reg[0].I);
  }
#endif
}

void BIOS_SndDriverJmpTableCopy()
{
#ifdef DEV_VERSION
  if(systemVerbose & VERBOSE_SWI) {
    log("SndDriverJmpTableCopy: dest=%08x\n",
        reg[0].I);
  }
#endif
  for(int i = 0; i < 0x24; i++) {
    CPUWriteMemory(reg[0].I, 0x9c);
    reg[0].I += 4;
  }
}
