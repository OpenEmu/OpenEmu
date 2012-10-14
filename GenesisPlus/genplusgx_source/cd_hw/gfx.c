/***************************************************************************************
 *  Genesis Plus
 *  CD graphics processor
 *
 *  Copyright (C) 2012  Eke-Eke (Genesis Plus GX)
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************************/
#include "shared.h"

/***************************************************************/
/*          WORD-RAM DMA interfaces (1M & 2M modes)            */
/***************************************************************/

void word_ram_0_dma_w(unsigned int words)
{
  uint16 data;

  /* CDC buffer source address */
  uint16 src_index = cdc.dac.w & 0x3ffe;
  
  /* WORD-RAM destination address*/
  uint32 dst_index = (scd.regs[0x0a>>1].w << 3) & 0x1fffe;
  
  /* update DMA destination address */
  scd.regs[0x0a>>1].w += (words >> 2);

  /* update DMA source address */
  cdc.dac.w += (words << 1);

  /* DMA transfer */
  while (words--)
  {
    /* read 16-bit word from CDC buffer */
    data = *(uint16 *)(cdc.ram + src_index);

#ifdef LSB_FIRST
    /* source data is stored in big endian format */
    data = ((data >> 8) | (data << 8)) & 0xffff;
#endif

    /* write 16-bit word to WORD-RAM */
    *(uint16 *)(scd.word_ram[0] + dst_index) = data ;

    /* increment CDC buffer source address */
    src_index = (src_index + 2) & 0x3ffe;

    /* increment WORD-RAM destination address */
    dst_index = (dst_index + 2) & 0x1fffe;
  }
}

void word_ram_1_dma_w(unsigned int words)
{
  uint16 data;

  /* CDC buffer source address */
  uint16 src_index = cdc.dac.w & 0x3ffe;
  
  /* WORD-RAM destination address*/
  uint32 dst_index = ((scd.regs[0x0a>>1].w << 3) & 0x1fffe);
  
  /* update DMA destination address */
  scd.regs[0x0a>>1].w += (words >> 2);

  /* update DMA source address */
  cdc.dac.w += (words << 1);

  /* DMA transfer */
  while (words--)
  {
    /* read 16-bit word from CDC buffer */
    data = *(uint16 *)(cdc.ram + src_index);

#ifdef LSB_FIRST
    /* source data is stored in big endian format */
    data = ((data >> 8) | (data << 8)) & 0xffff;
#endif

    /* write 16-bit word to WORD-RAM */
    *(uint16 *)(scd.word_ram[1] + dst_index) = data ;

    /* increment CDC buffer source address */
    src_index = (src_index + 2) & 0x3ffe;

    /* increment WORD-RAM destination address */
    dst_index = (dst_index + 2) & 0x1fffe;
  }
}

void word_ram_2M_dma_w(unsigned int words)
{
  uint16 data;

  /* CDC buffer source address */
  uint16 src_index = cdc.dac.w & 0x3ffe;
  
  /* WORD-RAM destination address*/
  uint32 dst_index = (scd.regs[0x0a>>1].w << 3) & 0x3fffe;
  
  /* update DMA destination address */
  scd.regs[0x0a>>1].w += (words >> 2);

  /* update DMA source address */
  cdc.dac.w += (words << 1);

  /* DMA transfer */
  while (words--)
  {
    /* read 16-bit word from CDC buffer */
    data = *(uint16 *)(cdc.ram + src_index);

#ifdef LSB_FIRST
    /* source data is stored in big endian format */
    data = ((data >> 8) | (data << 8)) & 0xffff;
#endif

    /* write 16-bit word to WORD-RAM */
    *(uint16 *)(scd.word_ram_2M + dst_index) = data ;

    /* increment CDC buffer source address */
    src_index = (src_index + 2) & 0x3ffe;

    /* increment WORD-RAM destination address */
    dst_index = (dst_index + 2) & 0x3fffe;
  }
}


/***************************************************************/
/*   WORD-RAM 0 & 1 DOT image SUB-CPU interface (1M Mode)      */
/***************************************************************/

unsigned int dot_ram_0_read16(unsigned int address)
{
  uint8 data = READ_BYTE(scd.word_ram[0], (address >> 1) & 0x1ffff);
  return ((data & 0x0f) | ((data << 4) & 0xf00));
}

unsigned int dot_ram_1_read16(unsigned int address)
{
  uint8 data = READ_BYTE(scd.word_ram[1], (address >> 1) & 0x1ffff);
  return ((data & 0x0f) | ((data << 4) & 0xf00));
}

void dot_ram_0_write16(unsigned int address, unsigned int data)
{
  uint8 prev;
  address = (address >> 1) & 0x1ffff;
  prev = READ_BYTE(scd.word_ram[0], address);
  data = (data & 0x0f) | ((data >> 4) & 0xf0);
  data = gfx.lut_prio[(scd.regs[0x02>>1].w >> 3) & 0x03][prev][data];
  WRITE_BYTE(scd.word_ram[0], address, data);
}

void dot_ram_1_write16(unsigned int address, unsigned int data)
{
  uint8 prev;
  address = (address >> 1) & 0x1ffff;
  prev = READ_BYTE(scd.word_ram[1], address);
  data = (data & 0x0f) | ((data >> 4) & 0xf0);
  data = gfx.lut_prio[(scd.regs[0x02>>1].w >> 3) & 0x03][prev][data];
  WRITE_BYTE(scd.word_ram[1], address, data);
}

unsigned int dot_ram_0_read8(unsigned int address)
{
  uint8 data = READ_BYTE(scd.word_ram[0], (address >> 1) & 0x1ffff);

  if (address & 1)
  {
    return (data & 0x0f);
  }

  return (data >> 4);
}

unsigned int dot_ram_1_read8(unsigned int address)
{
  uint8 data = READ_BYTE(scd.word_ram[1], (address >> 1) & 0x1ffff);

  if (address & 1)
  {
    return (data & 0x0f);
  }

  return (data >> 4);
}

void dot_ram_0_write8(unsigned int address, unsigned int data)
{
  uint8 prev = READ_BYTE(scd.word_ram[0], (address >> 1) & 0x1ffff);

  if (address & 1)
  {
    data = (prev & 0xf0) | (data & 0x0f);
  }
  else
  {
    data = (prev & 0x0f) | (data << 4);
  }

  data = gfx.lut_prio[(scd.regs[0x02>>1].w >> 3) & 0x03][prev][data];
  WRITE_BYTE(scd.word_ram[0], (address >> 1) & 0x1ffff, data);
}

void dot_ram_1_write8(unsigned int address, unsigned int data)
{
  uint8 prev = READ_BYTE(scd.word_ram[1], (address >> 1) & 0x1ffff);

  if (address & 1)
  {
    data = (prev & 0xf0) | (data & 0x0f);
  }
  else
  {
    data = (prev & 0x0f) | (data << 4);
  }

  data = gfx.lut_prio[(scd.regs[0x02>>1].w >> 3) & 0x03][prev][data];
  WRITE_BYTE(scd.word_ram[1], (address >> 1) & 0x1ffff, data);
}


/***************************************************************/
/*  WORD-RAM 0 & 1 CELL image MAIN-CPU interface (1M Mode)     */
/***************************************************************/

unsigned int cell_ram_0_read16(unsigned int address)
{
  address = gfx.lut_offset[(address >> 2) & 0x7fff] | (address & 0x10002);
  return *(uint16 *)(scd.word_ram[0] + address);
}

unsigned int cell_ram_1_read16(unsigned int address)
{
  address = gfx.lut_offset[(address >> 2) & 0x7fff] | (address & 0x10002);
  return *(uint16 *)(scd.word_ram[1] + address);
}

void cell_ram_0_write16(unsigned int address, unsigned int data)
{
  address = gfx.lut_offset[(address >> 2) & 0x7fff] | (address & 0x10002);
  *(uint16 *)(scd.word_ram[0] + address) = data;
}

void cell_ram_1_write16(unsigned int address, unsigned int data)
{
  address = gfx.lut_offset[(address >> 2) & 0x7fff] | (address & 0x10002);
  *(uint16 *)(scd.word_ram[1] + address) = data;
}

unsigned int cell_ram_0_read8(unsigned int address)
{
  address = gfx.lut_offset[(address >> 2) & 0x7fff] | (address & 0x10003);
  return READ_BYTE(scd.word_ram[0], address);
}

unsigned int cell_ram_1_read8(unsigned int address)
{
  address = gfx.lut_offset[(address >> 2) & 0x7fff] | (address & 0x10003);
  return READ_BYTE(scd.word_ram[1], address);
}

void cell_ram_0_write8(unsigned int address, unsigned int data)
{
  address = gfx.lut_offset[(address >> 2) & 0x7fff] | (address & 0x10003);
  WRITE_BYTE(scd.word_ram[0], address, data);
}

void cell_ram_1_write8(unsigned int address, unsigned int data)
{
  address = gfx.lut_offset[(address >> 2) & 0x7fff] | (address & 0x10003);
  WRITE_BYTE(scd.word_ram[1], address, data);
}


/***************************************************************/
/*      Rotation / Scaling operation (2M Mode)                 */
/***************************************************************/

void gfx_init(void)
{
  int i, j;
  uint16 offset;
  uint8 mask, row, col, temp;

  memset(&gfx, 0, sizeof(gfx_t));

  /* Initialize cell image lookup table */
  /* $220000-$22FFFF corresponds to $200000-$20FFFF */
  for (i=0; i<0x4000; i++)
  {
    offset = (i & 0x07) << 8;                     /* cell vline (0-7) */ 
    offset = offset | (((i >> 8) & 0x3f) << 2);   /* cell x offset (0-63) */
    offset = offset | (((i >> 3) & 0x1f) << 11);  /* cell y offset (0-31) */
    gfx.lut_offset[i] = offset;
  }
  
  /* $230000-$237FFF corresponds to $210000-$217FFF */
  for (i=0x4000; i<0x6000; i++)
  {
    offset = (i & 0x07) << 8;                     /* cell vline (0-7) */ 
    offset = offset | (((i >> 7) & 0x3f) << 2);   /* cell x offset (0-63) */
    offset = offset | (((i >> 3) & 0x0f) << 11);  /* cell y offset (0-15) */
    gfx.lut_offset[i] = offset;
  }

  /* $238000-$23BFFF corresponds to $218000-$21BFFF */
  for (i=0x6000; i<0x7000; i++)
  {
    offset = (i & 0x07) << 8;                     /* cell vline (0-7) */ 
    offset = offset | (((i >> 6) & 0x3f) << 2);   /* cell x offset (0-63) */
    offset = offset | (((i >> 3) & 0x07) << 11);  /* cell y offset (0-7) */
    gfx.lut_offset[i] = offset | 0x8000;
  }

  /* $23C000-$23DFFF corresponds to $21C000-$21DFFF */
  for (i=0x7000; i<0x7800; i++)
  {
    offset = (i & 0x07) << 8;                     /* cell vline (0-7) */ 
    offset = offset | (((i >> 5) & 0x3f) << 2);   /* cell x offset (0-63) */
    offset = offset | (((i >> 3) & 0x03) << 11);  /* cell y offset (0-3) */
    gfx.lut_offset[i] = offset | 0xc000;
  }

  /* $23E000-$23FFFF corresponds to $21E000-$21FFFF */
  for (i=0x7800; i<0x8000; i++)
  {
    offset = (i & 0x07) << 8;                     /* cell vline (0-7) */ 
    offset = offset | (((i >> 5) & 0x3f) << 2);   /* cell x offset (0-63) */
    offset = offset | (((i >> 3) & 0x03) << 11);  /* cell y offset (0-3) */
    gfx.lut_offset[i] = offset | 0xe000;
  }

  /* Initialize priority modes lookup table */
  for (i=0; i<0x100; i++)
  {
    for (j=0; j<0x100; j++)
    {
      /* normal */
      gfx.lut_prio[0][i][j] = j;
      /* underwrite */
      gfx.lut_prio[1][i][j] = ((i & 0x0f) ? (i & 0x0f) : (j & 0x0f)) | ((i & 0xf0) ? (i & 0xf0) : (j & 0xf0));
      /* overwrite */
      gfx.lut_prio[2][i][j] = ((j & 0x0f) ? (j & 0x0f) : (i & 0x0f)) | ((j & 0xf0) ? (j & 0xf0) : (i & 0xf0));
      /* invalid */
      gfx.lut_prio[3][i][j] = i;
    }
  }

  /* Initialize cell lookup table             */
  /* table entry = yyxxshrr (8 bits)          */
  /* with: yy = cell row (0-3)                */
  /*       xx = cell column (0-3)             */
  /*        s = stamp size (0=16x16, 1=32x32) */
  /*      hrr = HFLIP & ROTATION bits         */
  for (i=0; i<0x100; i++)
  {
    /* one stamp = 2x2 cells (16x16) or 4x4 cells (32x32) */
    mask = (i & 8) ? 3 : 1;
    row = (i >> 6) & mask;
    col = (i >> 4) & mask;

    if (i & 4) { col = col ^ mask; }  /* HFLIP (always first)  */ 
    if (i & 2) { col = col ^ mask; row = row ^ mask; }  /* ROLL1 */
    if (i & 1) { temp = col; col = row ^ mask; row = temp; }  /* ROLL0  */

    /* cell offset (0-3 or 0-15) */
    gfx.lut_cell[i] = row + col * (mask + 1);
  }

  /* Initialize pixel lookup table      */
  /* table entry = yyyxxxhrr (9 bits)   */
  /* with:  yyy = pixel row  (0-7)      */
  /*        xxx = pixel column (0-7)    */
  /*        hrr = HFLIP & ROTATION bits */
  for (i=0; i<0x200; i++)
  {
    /* one cell = 8x8 pixels */
    row = (i >> 6) & 7;
    col = (i >> 3) & 7;

    if (i & 4) { col = col ^ 7; }   /* HFLIP (always first) */ 
    if (i & 2) { col = col ^ 7; row = row ^ 7; }  /* ROLL1 */
    if (i & 1) { temp = col; col = row ^ 7; row = temp; } /* ROLL0 */

    /* pixel offset (0-63) */
    gfx.lut_pixel[i] = col + row * 8;
  }
}

void gfx_reset(void)
{ 
  /* Reset cycle counter */
  gfx.cycles = 0;
}

int gfx_context_save(uint8 *state)
{
  uint32 tmp32;
  int bufferptr = 0;

  save_param(&gfx.cycles, sizeof(gfx.cycles));
  save_param(&gfx.cyclesPerLine, sizeof(gfx.cyclesPerLine));
  save_param(&gfx.dotMask, sizeof(gfx.dotMask));
  save_param(&gfx.stampShift, sizeof(gfx.stampShift));
  save_param(&gfx.mapShift, sizeof(gfx.mapShift));
  save_param(&gfx.bufferOffset, sizeof(gfx.bufferOffset));
  save_param(&gfx.bufferStart, sizeof(gfx.bufferStart));

  tmp32 = (uint8 *)(gfx.tracePtr) - scd.word_ram_2M;
  save_param(&tmp32, 4);

  tmp32 = (uint8 *)(gfx.mapPtr) - scd.word_ram_2M;
  save_param(&tmp32, 4);

  return bufferptr;
}

int gfx_context_load(uint8 *state)
{
  uint32 tmp32;
  int bufferptr = 0;

  load_param(&gfx.cycles, sizeof(gfx.cycles));
  load_param(&gfx.cyclesPerLine, sizeof(gfx.cyclesPerLine));
  load_param(&gfx.dotMask, sizeof(gfx.dotMask));
  load_param(&gfx.stampShift, sizeof(gfx.stampShift));
  load_param(&gfx.mapShift, sizeof(gfx.mapShift));
  load_param(&gfx.bufferOffset, sizeof(gfx.bufferOffset));
  load_param(&gfx.bufferStart, sizeof(gfx.bufferStart));

  load_param(&tmp32, 4);
  gfx.tracePtr = (uint16 *)(scd.word_ram_2M + tmp32);

  load_param(&tmp32, 4);
  gfx.mapPtr = (uint16 *)(scd.word_ram_2M + tmp32);

  return bufferptr;
}

INLINE void gfx_render(uint32 bufferIndex, uint32 width)
{
  uint8 pixel_in, pixel_out;
  uint16 stamp_data;
  uint32 stamp_index;

  /* pixel map start position for current line (13.3 format converted to 13.11) */
  uint32 xpos = *gfx.tracePtr++ << 8;
  uint32 ypos = *gfx.tracePtr++ << 8;

  /* pixel map offset values for current line (5.11 format) */
  uint32 xoffset = (int16) *gfx.tracePtr++;
  uint32 yoffset = (int16) *gfx.tracePtr++;

  /* process all dots */
  while (width--)
  {
    /* check if stamp map is repeated */
    if (scd.regs[0x58>>1].byte.l & 0x01)
    {
      /* stamp map range */
      xpos &= gfx.dotMask;
      ypos &= gfx.dotMask;
    }
    else
    {
      /* 24-bit range */
      xpos &= 0xffffff;
      ypos &= 0xffffff;
    }

    /* check if pixel is outside stamp map */
    if ((xpos | ypos) & ~gfx.dotMask)
    {
      /* force pixel output to 0 */
      pixel_out = 0x00;
    }
    else
    {
      /* read stamp map table data */
      stamp_data = gfx.mapPtr[(xpos >> gfx.stampShift) | ((ypos >> gfx.stampShift) << gfx.mapShift)];

      /* stamp generator base index                                     */
      /* sss ssssssss ccyyyxxx (16x16) or sss sssssscc ccyyyxxx (32x32) */
      /* with:  s = stamp number (1 stamp = 16x16 or 32x32 pixels)      */
      /*        c = cell offset  (0-3 for 16x16, 0-15 for 32x32)        */
      /*      yyy = line offset  (0-7)                                  */
      /*      xxx = pixel offset (0-7)                                  */
      stamp_index = (stamp_data & 0x7ff) << 8;

      if (stamp_index)
      {
        /* extract HFLIP & ROTATION bits */
        stamp_data = (stamp_data >> 13) & 7;

        /* cell offset (0-3 or 0-15)                             */
        /* table entry = yyxxshrr (8 bits)                       */
        /* with: yy = cell row  (0-3) = (ypos >> (11 + 3)) & 3   */
        /*       xx = cell column (0-3) = (xpos >> (11 + 3)) & 3 */
        /*        s = stamp size (0=16x16, 1=32x32)              */
        /*      hrr = HFLIP & ROTATION bits                      */
        stamp_index |= gfx.lut_cell[stamp_data | ((scd.regs[0x58>>1].byte.l & 0x02) << 2 ) | ((ypos >> 8) & 0xc0) | ((xpos >> 10) & 0x30)] << 6;
            
        /* pixel  offset (0-63)                              */
        /* table entry = yyyxxxhrr (9 bits)                  */
        /* with: yyy = pixel row  (0-7) = (ypos >> 11) & 7   */
        /*       xxx = pixel column (0-7) = (xpos >> 11) & 7 */
        /*       hrr = HFLIP & ROTATION bits                 */
        stamp_index |= gfx.lut_pixel[stamp_data | ((xpos >> 8) & 0x38) | ((ypos >> 5) & 0x1c0)];

        /* read pixel pair (2 pixels/byte) */
        pixel_out = READ_BYTE(scd.word_ram_2M, stamp_index >> 1);

        /* extract left or rigth pixel */
        if (stamp_index & 1)
        {
           pixel_out &= 0x0f;
        }
        else
        {
           pixel_out >>= 4;
        }
      }
      else
      {
        /* stamp 0 is not used: force pixel output to 0 */
        pixel_out = 0x00;
      }
    }

    /* read out paired pixel data */
    pixel_in = READ_BYTE(scd.word_ram_2M, bufferIndex >> 1);

    /* update left or rigth pixel */
    if (bufferIndex & 1)
    {
      pixel_out |= (pixel_in & 0xf0);
    }
    else
    {
      pixel_out = (pixel_out << 4) | (pixel_in & 0x0f);
    }

    /* priority mode write */
    pixel_out = gfx.lut_prio[(scd.regs[0x02>>1].w >> 3) & 0x03][pixel_in][pixel_out];

    /* write data to image buffer */
    WRITE_BYTE(scd.word_ram_2M, bufferIndex >> 1, pixel_out);

    /* check current pixel position  */
    if ((bufferIndex & 7) != 7)
    {
      /* next pixel */
      bufferIndex++;
    }
    else
    {
      /* next cell: increment image buffer offset by one column (minus 7 pixels) */
      bufferIndex += gfx.bufferOffset;
    }

    /* increment pixel position */
    xpos += xoffset;
    ypos += yoffset;
  }
}

void gfx_start(unsigned int base, int cycles)
{
  /* make sure 2M mode is enabled */
  if (!(scd.regs[0x02>>1].byte.l & 0x04))
  {
    uint32 mask;
    
    /* trace vector pointer */
    gfx.tracePtr = (uint16 *)(scd.word_ram_2M + ((base << 2) & 0x3fff8));

    /* stamps & stamp map size */
    switch ((scd.regs[0x58>>1].byte.l >> 1) & 0x03)
    {
      case 0:
        gfx.dotMask = 0x07ffff;   /* 256x256 dots/map  */
        gfx.stampShift = 11 + 4;  /* 16x16 dots/stamps */
        gfx.mapShift = 4;         /* 16x16 stamps/map  */
        mask = 0x3fe00;           /* 512 bytes/table   */
        break;

      case 1:
        gfx.dotMask = 0x07ffff;   /* 256x256 dots/map  */
        gfx.stampShift = 11 + 5;  /* 32x32 dots/stamps */
        gfx.mapShift = 3;         /* 8x8 stamps/map    */
        mask = 0x3ff80;           /* 128 bytes/table   */
        break;

      case 2:
        gfx.dotMask = 0x7fffff;   /* 4096*4096 dots/map */
        gfx.stampShift = 11 + 4;  /* 16x16 dots/stamps  */
        gfx.mapShift = 8;         /* 256x256 stamps/map */
        mask = 0x20000;           /* 131072 bytes/table */
        break;

      case 3:
        gfx.dotMask = 0x7fffff;   /* 4096*4096 dots/map */
        gfx.stampShift = 11 + 5;  /* 32x32 dots/stamps  */
        gfx.mapShift = 7;         /* 128x128 stamps/map */
        mask = 0x38000;           /* 32768 bytes/table  */
        break;
    }

    /* stamp map table base address */
    gfx.mapPtr = (uint16 *)(scd.word_ram_2M + ((scd.regs[0x5a>>1].w << 2) & mask));

    /* image buffer column offset (64 pixels/cell, minus 7 pixels to restart at cell beginning) */
    gfx.bufferOffset = (((scd.regs[0x5c>>1].byte.l & 0x1f) + 1) << 6) - 7;

    /* image buffer start index in dot units (2 pixels/byte) */
    gfx.bufferStart = (scd.regs[0x5e>>1].w << 3) & 0x7ffc0;

    /* add image buffer horizontal dot offset */
    gfx.bufferStart += (scd.regs[0x60>>1].byte.l & 0x3f);

    /* reset GFX chip cycle counter */
    gfx.cycles = cycles;

    /* update GFX chip timings (see AC3:Thunderhawk / Thunderstrike) */
    gfx.cyclesPerLine = 4 * 5 * scd.regs[0x62>>1].w; 

    /* start graphics operation */
    scd.regs[0x58>>1].byte.h = 0x80;
  }
}

void gfx_update(int cycles)
{
  /* synchronize GFX chip with SUB-CPU */
  cycles -= gfx.cycles;

  /* make sure SUB-CPU is ahead */
  if (cycles > 0)
  {
    /* number of lines to process */
    unsigned int lines = (cycles + gfx.cyclesPerLine - 1) / gfx.cyclesPerLine;

    /* check against remaining lines */
    if (lines < scd.regs[0x64>>1].byte.l)
    {
      /* update Vdot remaining size */
      scd.regs[0x64>>1].byte.l -= lines;

      /* increment cycle counter */
      gfx.cycles += lines * gfx.cyclesPerLine;
    }
    else
    {
      /* process remaining lines */
      lines = scd.regs[0x64>>1].byte.l;

      /* clear Vdot remaining size */
      scd.regs[0x64>>1].byte.l = 0;

      /* end of graphics operation */
      scd.regs[0x58>>1].byte.h = 0;
 
      /* level 1 interrupt enabled ? */
      if (scd.regs[0x32>>1].byte.l & 0x02)
      {
        /* trigger level 1 interrupt */
        scd.pending |= (1 << 1);

        /* update IRQ level */
        s68k_update_irq((scd.pending & scd.regs[0x32>>1].byte.l) >> 1);
      }
    }

    /* render lines */
    while (lines--)
    {
      /* process dots to image buffer */
      gfx_render(gfx.bufferStart, scd.regs[0x62>>1].w);

      /* increment image buffer start index for next line (8 pixels/line) */
      gfx.bufferStart += 8;
    }
  }
}
