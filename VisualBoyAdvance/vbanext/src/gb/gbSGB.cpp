#include <stdlib.h>
#include <string.h>
#include "../System.h"
#include "../common/Port.h"
#include "../Util.h"
#include "gb.h"
#include "gbGlobals.h"

extern uint8_t *pix;
extern bool speedup;
extern bool gbSgbResetFlag;

#define GBSGB_NONE            0
#define GBSGB_RESET           1
#define GBSGB_PACKET_TRANSMIT 2

uint8_t *gbSgbBorderChar = NULL;
uint8_t *gbSgbBorder = NULL;

int gbSgbCGBSupport        = 0;
int gbSgbMask              = 0;
int gbSgbMode              = 0;
int gbSgbPacketState       = GBSGB_NONE;
int gbSgbBit               = 0;
int gbSgbPacketTimeout     = 0;
int GBSGB_PACKET_TIMEOUT   = 66666;
uint8_t  gbSgbPacket[16*7];
int gbSgbPacketNBits       = 0;
int gbSgbPacketByte        = 0;
int gbSgbPacketNumber      = 0;
int gbSgbMultiplayer       = 0;
int gbSgbFourPlayers       = 0;
uint8_t  gbSgbNextController    = 0x0f;
uint8_t  gbSgbReadingController = 0;
uint16_t gbSgbSCPPalette[4*512];
uint8_t  gbSgbATF[20 * 18];
uint8_t  gbSgbATFList[45 * 20 * 18];
uint8_t  gbSgbScreenBuffer[4160];

INLINE void gbSgbDraw24Bit(uint8_t *p, uint16_t v)
{
	*((uint32_t*) p) = systemColorMap32[v];
}

INLINE void gbSgbDraw32Bit(uint32_t *p, uint16_t v)
{
	*p = systemColorMap32[v];
}

INLINE void gbSgbDraw16Bit(uint16_t *p, uint16_t v)
{
  *p = systemColorMap16[v];
}

void gbSgbReset()
{
  gbSgbPacketTimeout = 0;
  gbSgbCGBSupport = 0;
  gbSgbMask = 0;
  gbSgbPacketState = GBSGB_NONE;
  gbSgbBit = 0;
  gbSgbPacketNBits = 0;
  gbSgbPacketNumber = 0;
  gbSgbMultiplayer = 0;
  gbSgbFourPlayers = 0;
  gbSgbNextController = 0x0f;
  gbSgbReadingController = 0;

  memset(gbSgbSCPPalette, 0, 512*4);
  memset(gbSgbATF, 0, 20*18);
  memset(gbSgbATFList, 0, 45 * 20 * 18);
  memset(gbSgbPacket, 0, 16 * 7);
  memset(gbSgbBorderChar, 0, 32*256);
  memset(gbSgbBorder, 0, 2048);

  int i;
  for(i = 1; i < 2048; i+=2) {
    gbSgbBorder[i] = 1 << 2;
  }

  for(i = 0; i < 32; i++) {
    gbPalette[i*4] = (0x1f) | (0x1f << 5) | (0x1f << 10);
    gbPalette[i*4+1] = (0x15) | (0x15 << 5) | (0x15 << 10);
    gbPalette[i*4+2] = (0x0c) | (0x0c << 5) | (0x0c << 10);
    gbPalette[i*4+3] = 0;
  }
}

void gbSgbInit()
{
  gbSgbBorderChar = (uint8_t *)malloc(32 * 256);
  gbSgbBorder = (uint8_t *)malloc(2048);

  gbSgbReset();
}

void gbSgbShutdown()
{
  if(gbSgbBorderChar != NULL) {
    free(gbSgbBorderChar);
    gbSgbBorderChar = NULL;
  }

  if(gbSgbBorder != NULL) {
    free(gbSgbBorder);
    gbSgbBorder = NULL;
  }
}

void gbSgbFillScreen(uint16_t color)
{
	switch(systemColorDepth) {
		case 16:
			{
				for(int y = 0; y < 144; y++) {
					int yLine = (y+gbBorderRowSkip+1)*(gbBorderLineSkip+2) +
						gbBorderColumnSkip;
					uint16_t *dest = (uint16_t*)pix + yLine;
					for(register int x = 0; x < 160; x++)
						gbSgbDraw16Bit(dest++, color);
				}
			}
			break;
		case 24:
			{
				for(int y = 0; y < 144; y++) {
					int yLine = (y+gbBorderRowSkip)*gbBorderLineSkip + gbBorderColumnSkip;
					uint8_t *dest = (uint8_t *)pix + yLine*3;
					for(register int x = 0; x < 160; x++) {
						SYSTEMCOLORMAP24_LINE_SGB(dest, color);
						dest += 3;
					}
				}
			}
			break;
		case 32:
			{
				for(int y = 0; y < 144; y++) {
					int yLine = (y+gbBorderRowSkip+1)*(gbBorderLineSkip+1) + gbBorderColumnSkip;
					uint32_t *dest = (uint32_t *)pix + yLine;
					for(register int x = 0; x < 160; x++) {
						SYSTEMCOLORMAP32_LINE_SGB(dest++, color);
					}
				}
			}
			break;
	}
}

#define getmem(x) gbMemoryMap[(x) >> 12][(x) & 0xfff]

void gbSgbRenderScreenToBuffer()
{
  uint16_t mapAddress = 0x9800;

  if(register_LCDC & 0x08)
    mapAddress = 0x9c00;

  uint16_t patternAddress = 0x8800;

  int flag = 1;

  if(register_LCDC & 0x10) {
    patternAddress = 0x8000;
    flag = 0;
  }

  uint8_t *toAddress = gbSgbScreenBuffer;

  for(int i = 0; i < 13; i++) {
    for(int j = 0; j < 20; j++) {
      int tile = getmem(mapAddress);
      mapAddress++;

      if(flag) {
        if(tile > 127)
          tile -= 128;
        else
          tile += 128;
      }
      for(int k = 0; k < 16; k++)
        *toAddress++ = getmem(patternAddress + tile*16 + k);
    }
    mapAddress += 12;
  }
}

void gbSgbDrawBorderTile(int x, int y, int tile, int attr)
{
  uint16_t *dest = (uint16_t*)pix + ((y+1) * (256+2)) + x;
  uint8_t *dest8 = (uint8_t*)pix + ((y*256)+x)*3;
  uint32_t *dest32 = (uint32_t*)pix + ((y+1)*257) + x;

  uint8_t *tileAddress = &gbSgbBorderChar[tile * 32];
  uint8_t *tileAddress2 = &gbSgbBorderChar[tile * 32 + 16];

  uint8_t l = 8;

  uint8_t palette = ((attr >> 2 ) & 7);

  if(palette < 4)
    palette += 4;

  palette *= 16;

  uint8_t xx = 0;
  uint8_t yy = 0;

  int flipX = attr & 0x40;
  int flipY = attr & 0x80;

  while(l > 0) {
    uint8_t mask = 0x80;
    uint8_t a = *tileAddress++;
    uint8_t b = *tileAddress++;
    uint8_t c = *tileAddress2++;
    uint8_t d = *tileAddress2++;


 
    uint8_t yyy;
    if(!flipY)
      yyy = yy;
    else
      yyy = 7 - yy;

    while(mask > 0) {

      uint8_t color = 0;
      if(a & mask)
        color++;
      if(b & mask)
        color+=2;
      if(c & mask)
        color+=4;
      if(d & mask)
        color+=8;

      if (color || (y + yy < 40 || y + yy >= 184) || (x + xx < 48 || x + xx >= 208)) {
        uint8_t xxx;

        if(!flipX)
          xxx = xx;
        else
          xxx = 7 - xx;

        uint16_t cc;
        if (color) {
          cc = gbPalette[palette + color];
        } else {
          cc = gbPalette[0];
        }

        switch(systemColorDepth)
	{
		case 16:
			gbSgbDraw16Bit(dest + yyy*(256+2) + xxx, cc);
			break;
		case 24:
			gbSgbDraw24Bit(dest8 + (yyy*256+xxx)*3, cc);
			break;
		case 32:

			uint32_t * p = dest32 + yyy*(256+1)+xxx;
			SYSTEMCOLORMAP32_LINE_SGB(p, cc);
			break;
	}
      }

      mask >>= 1;

      xx++;
    }
    yy++;
    xx = 0;
    l--;
    mask = 0x80;
  }
}

void gbSgbRenderBorder()
{
  if(gbBorderOn) {
    uint8_t *fromAddress = gbSgbBorder;

    for(uint8_t y = 0; y < 28; y++) {
      for(uint8_t x = 0; x< 32; x++) {
        uint8_t tile = *fromAddress++;
        uint8_t attr = *fromAddress++;

        gbSgbDrawBorderTile(x*8,y*8,tile,attr);
      }
    }
  }
}

void gbSgbPicture()
{
  gbSgbRenderScreenToBuffer();

  memcpy(gbSgbBorder, gbSgbScreenBuffer, 2048);

  uint16_t *paletteAddr = (uint16_t *)&gbSgbScreenBuffer[2048];

  for(int i = 64; i < 128; i++) {
    gbPalette[i] = READ16LE(paletteAddr++);
  }

  gbSgbCGBSupport |= 4;

  if(gbBorderAutomatic && !gbBorderOn && gbSgbCGBSupport > 4) {
    gbBorderOn = 1;
    systemGbBorderOn();
  }

  if(gbBorderOn && !gbSgbMask)
    gbSgbRenderBorder();

  if(gbSgbMode && gbCgbMode && gbSgbCGBSupport > 4) {
    gbSgbCGBSupport = 0;
    gbSgbMode = 0;
    gbSgbMask = 0;
    gbSgbRenderBorder();
    gbReset();
  }

  if(gbSgbCGBSupport > 4)
    gbSgbCGBSupport = 0;
}

void gbSgbSetPalette(int a,int b,uint16_t *p)
{
  uint16_t bit00 = READ16LE(p++);
  int i;

  for(i = 1; i < 4; i++) {
    gbPalette[a*4+i] = READ16LE(p++);
  }

  for(i = 1; i < 4; i++) {
    gbPalette[b*4+i] = READ16LE(p++);
  }

  gbPalette[0] = gbPalette[4] = gbPalette[8] = gbPalette[12] = bit00;
  if(gbBorderOn && !gbSgbMask)
    gbSgbRenderBorder();
}

void gbSgbScpPalette()
{
  gbSgbRenderScreenToBuffer();

  uint16_t *fromAddress = (uint16_t *)gbSgbScreenBuffer;

  for(int i = 0; i < 512*4; i++) {
    gbSgbSCPPalette[i] = READ16LE(fromAddress++);
  }
}

void gbSgbSetATF(int n)
{
  if(n < 0)
    n = 0;
  if(n > 44)
    n = 44;
  memcpy(gbSgbATF,&gbSgbATFList[n * 20 * 18], 20 * 18);

  if(gbSgbPacket[1] & 0x40) {
    gbSgbMask = 0;
    if(gbBorderOn)
      gbSgbRenderBorder();
  }
}

void gbSgbSetPalette()
{
  uint16_t pal = READ16LE((((uint16_t *)&gbSgbPacket[1]))) & 511;
  memcpy(&gbPalette[0], &gbSgbSCPPalette[pal*4], 4 * sizeof(uint16_t));

  pal = READ16LE((((uint16_t *)&gbSgbPacket[3]))) & 511;
  memcpy(&gbPalette[4], &gbSgbSCPPalette[pal*4], 4 * sizeof(uint16_t));

  pal = READ16LE((((uint16_t *)&gbSgbPacket[5]))) & 511;
  memcpy(&gbPalette[8], &gbSgbSCPPalette[pal*4], 4 * sizeof(uint16_t));

  pal = READ16LE((((uint16_t *)&gbSgbPacket[7]))) & 511;
  memcpy(&gbPalette[12], &gbSgbSCPPalette[pal*4], 4 * sizeof(uint16_t));

  uint8_t atf = gbSgbPacket[9];

  if(atf & 0x80) {
    gbSgbSetATF(atf & 0x3f);
  }

  if(atf & 0x40) {
    gbSgbMask = 0;
    if(gbBorderOn)
      gbSgbRenderBorder();
  }
}

void gbSgbAttributeBlock()
{
  uint8_t *fromAddress = &gbSgbPacket[1];

  uint8_t nDataSet = *fromAddress++;
  if(nDataSet > 12)
    nDataSet = 12;
  if(nDataSet == 0)
    nDataSet = 1;

  while(nDataSet) {
    uint8_t controlCode = (*fromAddress++) & 7;
    uint8_t paletteDesignation = (*fromAddress++) & 0x3f;
    uint8_t startH = (*fromAddress++) & 0x1f;
    uint8_t startV = (*fromAddress++) & 0x1f;
    uint8_t endH   = (*fromAddress++) & 0x1f;
    uint8_t endV   = (*fromAddress++) & 0x1f;

    uint8_t * toAddress = gbSgbATF;

    for(uint8_t y = 0; y < 18; y++) {
      for(uint8_t x = 0; x < 20; x++) {
        if(x < startH || y < startV ||
           x > endH || y > endV) {
          // outside
          if(controlCode & 0x04)
            *toAddress = (paletteDesignation >> 4) & 0x03;
        } else if(x > startH && x < endH &&
                  y > startV && y < endV) {
          // inside
          if(controlCode & 0x01)
            *toAddress = paletteDesignation & 0x03;
        } else {
          // surrounding line
          if(controlCode & 0x02)
            *toAddress = (paletteDesignation>>2) & 0x03;
          else if(controlCode == 0x01)
            *toAddress = paletteDesignation & 0x03;
        }
        toAddress++;
      }
    }
    nDataSet--;
  }
}

void gbSgbSetColumnPalette(uint8_t col, uint8_t p)
{
  //  if(col < 0)
  //    col = 0;
  if(col > 19)
    col = 19;

  p &= 3;

  uint8_t *toAddress = &gbSgbATF[col];

  for(uint8_t y = 0; y < 18; y++) {
    *toAddress = p;
    toAddress += 20;
  }
}

void gbSgbSetRowPalette(uint8_t row, uint8_t p)
{
  //  if(row < 0)
  //    row = 0;
  if(row > 17)
    row = 17;

  p &= 3;

  uint8_t *toAddress = &gbSgbATF[row*20];

  for(uint8_t x = 0; x < 20; x++) {
    *toAddress++ = p;
  }
}

void gbSgbAttributeDivide()
{
  uint8_t control = gbSgbPacket[1];
  uint8_t coord   = gbSgbPacket[2];
  uint8_t colorBR = control & 3;
  uint8_t colorAL = (control >> 2) & 3;
  uint8_t colorOL = (control >> 4) & 3;

  if(control & 0x40) {
    if(coord > 17)
      coord = 17;

    for(uint8_t i = 0; i < 18; i++) {
      if(i < coord)
        gbSgbSetRowPalette(i, colorAL);
      else if ( i > coord)
        gbSgbSetRowPalette(i, colorBR);
      else
        gbSgbSetRowPalette(i, colorOL);
    }
  } else {
    if(coord > 19)
      coord = 19;

    for(uint8_t i = 0; i < 20; i++) {
      if(i < coord)
        gbSgbSetColumnPalette(i, colorAL);
      else if ( i > coord)
        gbSgbSetColumnPalette(i, colorBR);
      else
        gbSgbSetColumnPalette(i, colorOL);
    }
  }
}

void gbSgbAttributeLine()
{
  uint8_t *fromAddress = &gbSgbPacket[1];

  uint8_t nDataSet = *fromAddress++;

  if(nDataSet > 0x6e)
    nDataSet = 0x6e;

  while(nDataSet) {
    uint8_t line = *fromAddress++;
    uint8_t num = line & 0x1f;
    uint8_t pal = (line >> 5) & 0x03;
    if(line & 0x80) {
      if(num > 17)
        num = 17;
      gbSgbSetRowPalette(num,pal);
    } else {
      if(num > 19)
        num = 19;
      gbSgbSetColumnPalette(num,pal);
    }
    nDataSet--;
  }
}

void gbSgbAttributeCharacter()
{
  uint8_t startH = gbSgbPacket[1] & 0x1f;
  uint8_t startV = gbSgbPacket[2] & 0x1f;
  int nDataSet = READ16LE(((uint16_t *)&gbSgbPacket[3]));
  int style = gbSgbPacket[5] & 1;
  if(startH > 19)
    startH = 19;
  if(startV > 17)
    startV = 17;

  uint8_t s = 6;
  uint8_t *fromAddress = &gbSgbPacket[6];
  uint8_t v = *fromAddress++;

  if(style) {
    while(nDataSet) {
      uint8_t p = (v >> s) & 3;
      gbSgbATF[startV * 20 + startH] = p;
      startV++;
      if(startV == 18) {
        startV = 0;
        startH++;
        if(startH == 20)
          break;
      }

      if(s)
        s -= 2;
      else {
        s = 6;
        v = *fromAddress++;
        nDataSet--;
      }
    }
  } else {
    while(nDataSet) {
      uint8_t p = (v >> s) & 3;
      gbSgbATF[startV * 20 + startH] = p;
      startH++;
      if(startH == 20) {
        startH = 0;
        startV++;
        if(startV == 18)
          break;
      }

      if(s)
        s -= 2;
      else {
        s = 6;
        v = *fromAddress++;
        nDataSet--;
      }
    }
  }
}

void gbSgbSetATFList()
{
  gbSgbRenderScreenToBuffer();

  uint8_t *fromAddress = gbSgbScreenBuffer;
  uint8_t *toAddress   = gbSgbATFList;

  for(int i = 0; i < 45; i++) {
    for(int j = 0; j < 90; j++) {
      uint8_t v = *fromAddress++;
      uint8_t s = 6;
      if(i == 2)
        s = 6;
      for(int k = 0; k < 4; k++) {
        *toAddress++ = (v >> s) & 0x03;
        s -= 2;
      }
    }
  }
}

void gbSgbMaskEnable()
{
  int gbSgbMaskFlag = gbSgbPacket[1] & 3;

  gbSgbMask = gbSgbMaskFlag;

  switch(gbSgbMaskFlag) {
  case 1:
    break;
  case 2:
    gbSgbFillScreen(0x0000);
    //    memset(&gbPalette[0], 0, 128*sizeof(uint16_t));
    break;
  case 3:
    gbSgbFillScreen(gbPalette[0]);
    break;
  }
  if(!gbSgbMask) {
    if(gbBorderOn)
      gbSgbRenderBorder();
  }
}

void gbSgbChrTransfer()
{
  gbSgbRenderScreenToBuffer();

  int address = (gbSgbPacket[1] & 1) * (128*32);

  if(gbSgbPacket[1] & 1)
    gbSgbCGBSupport |= 2;
  else
    gbSgbCGBSupport |= 1;

  memcpy(&gbSgbBorderChar[address], gbSgbScreenBuffer, 128 * 32);

  if(gbBorderAutomatic && !gbBorderOn && gbSgbCGBSupport > 4) {
    gbBorderOn = 1;
    systemGbBorderOn();
  }

  if(gbBorderOn && !gbSgbMask)
    gbSgbRenderBorder();

  if(gbSgbMode && gbCgbMode && gbSgbCGBSupport == 7) {
    gbSgbCGBSupport = 0;
    gbSgbMode = 0;
    gbSgbMask = 0;
    gbSgbRenderBorder();
    gbReset();
  }

  if(gbSgbCGBSupport > 4)
    gbSgbCGBSupport = 0;
}

void gbSgbMultiRequest()
{
  if(gbSgbPacket[1] & 1) {
    gbSgbMultiplayer    = 1;
    if(gbSgbPacket[1] & 2)
      gbSgbFourPlayers = 1;
    else
      gbSgbFourPlayers = 0;
    gbSgbNextController = 0x0e;
  } else {
    gbSgbFourPlayers    = 0;
    gbSgbMultiplayer    = 0;
    gbSgbNextController = 0x0f;
  }
}

void gbSgbCommand()
{
  int command = gbSgbPacket[0] >> 3;
  //  int nPacket = gbSgbPacket[0] & 7;

  switch(command) {
  case 0x00:
    gbSgbSetPalette(0,1,(uint16_t *)&gbSgbPacket[1]);
    break;
  case 0x01:
    gbSgbSetPalette(2,3,(uint16_t *)&gbSgbPacket[1]);
    break;
  case 0x02:
    gbSgbSetPalette(0,3,(uint16_t *)&gbSgbPacket[1]);
    break;
  case 0x03:
    gbSgbSetPalette(1,2,(uint16_t *)&gbSgbPacket[1]);
    break;
  case 0x04:
    gbSgbAttributeBlock();
    break;
  case 0x05:
    gbSgbAttributeLine();
    break;
  case 0x06:
    gbSgbAttributeDivide();
    break;
  case 0x07:
    gbSgbAttributeCharacter();
    break;
  case 0x0a:
    gbSgbSetPalette();
    break;
  case 0x0b:
    gbSgbScpPalette();
    break;
  case 0x11:
    gbSgbMultiRequest();
    break;
  case 0x13:
    gbSgbChrTransfer();
    break;
  case 0x14:
    gbSgbPicture();
    break;
  case 0x15:
    gbSgbSetATFList();
    break;
  case 0x16:
    gbSgbSetATF(gbSgbPacket[1] & 0x3f);
    break;
  case 0x17:
    gbSgbMaskEnable();
    break;
  }
}

void gbSgbResetPacketState()
{
  gbSgbPacketState = GBSGB_NONE;
  gbSgbPacketTimeout = 0;
}

void gbSgbDoBitTransfer(uint8_t value)
{
  value = value & 0x30;
  switch(gbSgbPacketState) {
  case GBSGB_NONE:
    if(value == 0) {
      gbSgbPacketState = GBSGB_RESET;
      gbSgbPacketTimeout = GBSGB_PACKET_TIMEOUT;
    } else if (value == 0x30) {
      if(gbSgbMultiplayer) {
        if((gbSgbReadingController & 7) == 7) {
          gbSgbReadingController = 0;
          if(gbSgbMultiplayer) {
            gbSgbNextController--;
            if(gbSgbFourPlayers) {
              if(gbSgbNextController == 0x0b)
                gbSgbNextController = 0x0f;
            } else {
              if(gbSgbNextController == 0x0d)
                gbSgbNextController = 0x0f;
            }
          }
        } else {
          gbSgbReadingController &= 3;
        }
      }
      gbSgbPacketTimeout = 0;
    } else {
      if(value == 0x10)
        gbSgbReadingController |= 0x2;
      else if(value == 0x20)
        gbSgbReadingController |= 0x01;
      gbSgbPacketTimeout = 0;
    }
    gbSgbPacketTimeout = 0;
    break;
  case GBSGB_RESET:
    if(value == 0x30) {
      gbSgbPacketState = GBSGB_PACKET_TRANSMIT;
      gbSgbPacketByte  = 0;
      gbSgbPacketNBits = 0;
      gbSgbPacketTimeout = GBSGB_PACKET_TIMEOUT;
    } else if(value == 0x00) {
      gbSgbPacketTimeout = GBSGB_PACKET_TIMEOUT;
      gbSgbPacketState = GBSGB_RESET;
    } else {
      gbSgbPacketState = GBSGB_NONE;
      gbSgbPacketTimeout = 0;
    }
    break;
  case GBSGB_PACKET_TRANSMIT:
    if(value == 0) {
      gbSgbPacketState = GBSGB_RESET;
      gbSgbPacketTimeout = 0;
    } else if (value == 0x30){
      if(gbSgbPacketNBits == 128) {
        gbSgbPacketNBits = 0;
        gbSgbPacketByte  = 0;
        gbSgbPacketNumber++;
        gbSgbPacketTimeout = 0;
        if(gbSgbPacketNumber == (gbSgbPacket[0] & 7)) {
          gbSgbCommand();
          gbSgbPacketNumber = 0;
          gbSgbPacketState = GBSGB_NONE;
          gbSgbPacketTimeout = 0;
        }
      } else {
        if(gbSgbPacketNBits < 128) {
          gbSgbPacket[gbSgbPacketNumber * 16 + gbSgbPacketByte] >>= 1;
          gbSgbPacket[gbSgbPacketNumber * 16 + gbSgbPacketByte] |= gbSgbBit;
          gbSgbPacketNBits++;
          if(!(gbSgbPacketNBits & 7)) {
            gbSgbPacketByte++;
          }
          gbSgbPacketTimeout = GBSGB_PACKET_TIMEOUT;
        }
      }
    } else {
      if(value == 0x20)
        gbSgbBit = 0x00;
      else
        gbSgbBit = 0x80;
      gbSgbPacketTimeout = GBSGB_PACKET_TIMEOUT;
    }
    gbSgbReadingController = 0;
    break;
  default:
    gbSgbPacketState = GBSGB_NONE;
    gbSgbPacketTimeout = 0;
    break;
  }
}

variable_desc gbSgbSaveStruct[] = {
  { &gbSgbMask, sizeof(int) },
  { &gbSgbPacketState, sizeof(int) },
  { &gbSgbBit, sizeof(int) },
  { &gbSgbPacketNBits, sizeof(int) },
  { &gbSgbPacketByte, sizeof(int) },
  { &gbSgbPacketNumber, sizeof(int) },
  { &gbSgbMultiplayer, sizeof(int) },
  { &gbSgbNextController, sizeof(uint8_t) },
  { &gbSgbReadingController, sizeof(uint8_t) },
  { NULL, 0 }
};

variable_desc gbSgbSaveStructV3[] = {
  { &gbSgbMask, sizeof(int) },
  { &gbSgbPacketState, sizeof(int) },
  { &gbSgbBit, sizeof(int) },
  { &gbSgbPacketNBits, sizeof(int) },
  { &gbSgbPacketByte, sizeof(int) },
  { &gbSgbPacketNumber, sizeof(int) },
  { &gbSgbMultiplayer, sizeof(int) },
  { &gbSgbNextController, sizeof(uint8_t) },
  { &gbSgbReadingController, sizeof(uint8_t) },
  { &gbSgbFourPlayers, sizeof(int) },
  { NULL, 0 }
};

void gbSgbSaveGame(gzFile gzFile)
{
  utilWriteData(gzFile, gbSgbSaveStructV3);

  utilGzWrite(gzFile, gbSgbBorder, 2048);
  utilGzWrite(gzFile, gbSgbBorderChar, 32*256);

  utilGzWrite(gzFile, gbSgbPacket, 16*7);

  utilGzWrite(gzFile, gbSgbSCPPalette, 4 * 512 * sizeof(uint16_t));
  utilGzWrite(gzFile, gbSgbATF, 20 * 18);
  utilGzWrite(gzFile, gbSgbATFList, 45 * 20 * 18);
}

void gbSgbReadGame(gzFile gzFile, int version)
{
  if(version >= 3)
    utilReadData(gzFile, gbSgbSaveStructV3);
  else {
    utilReadData(gzFile, gbSgbSaveStruct);
    gbSgbFourPlayers = 0;
  }

  if(version >= 8) {
    utilGzRead(gzFile, gbSgbBorder, 2048);
    utilGzRead(gzFile, gbSgbBorderChar, 32*256);
  }

  utilGzRead(gzFile, gbSgbPacket, 16*7);

  utilGzRead(gzFile, gbSgbSCPPalette, 4 * 512 * sizeof(uint16_t));
  utilGzRead(gzFile, gbSgbATF, 20 * 18);
  utilGzRead(gzFile, gbSgbATFList, 45 * 20 * 18);
}
