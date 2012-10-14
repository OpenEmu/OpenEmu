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
#ifndef _CD_GFX_
#define _CD_GFX_

#define gfx scd.gfx_hw

typedef struct
{
  uint32 cycles;                    /* current cycles count for graphics operation */
  uint32 cyclesPerLine;             /* current graphics operation timings */
  uint32 dotMask;                   /* stamp map size mask */
  uint16 *tracePtr;                 /* trace vector pointer */
  uint16 *mapPtr;                   /* stamp map table base address */
  uint8 stampShift;                 /* stamp pixel shift value (related to stamp size) */
  uint8 mapShift;                   /* stamp map table shift value (related to stamp map size) */
  uint16 bufferOffset;              /* image buffer column offset */
  uint32 bufferStart;               /* image buffer start index */
  uint16 lut_offset[0x8000];        /* Cell Image -> WORD-RAM offset lookup table (1M Mode) */
  uint8 lut_prio[4][0x100][0x100];  /* WORD-RAM data writes priority lookup table */
  uint8 lut_pixel[0x200];           /* Graphics operation dot offset lookup table */
  uint8 lut_cell[0x100];            /* Graphics operation stamp offset lookup table */
} gfx_t;


/***************************************************************/
/*          WORD-RAM DMA interfaces (1M & 2M modes)            */
/***************************************************************/
extern void word_ram_0_dma_w(unsigned int words);
extern void word_ram_1_dma_w(unsigned int words);
extern void word_ram_2M_dma_w(unsigned int words);

/***************************************************************/
/*          WORD-RAM 0 & 1 CPU interfaces (1M mode)            */
/***************************************************************/
extern unsigned int word_ram_0_read16(unsigned int address);
extern unsigned int word_ram_1_read16(unsigned int address);
extern void word_ram_0_write16(unsigned int address, unsigned int data);
extern void word_ram_1_write16(unsigned int address, unsigned int data);
extern unsigned int word_ram_0_read8(unsigned int address);
extern unsigned int word_ram_1_read8(unsigned int address);
extern void word_ram_0_write8(unsigned int address, unsigned int data);
extern void word_ram_1_write8(unsigned int address, unsigned int data);

/***************************************************************/
/*     WORD-RAM 0 & 1 DOT image SUB-CPU interface (1M mode)    */
/***************************************************************/
extern unsigned int dot_ram_0_read16(unsigned int address);
extern unsigned int dot_ram_1_read16(unsigned int address);
extern void dot_ram_0_write16(unsigned int address, unsigned int data);
extern void dot_ram_1_write16(unsigned int address, unsigned int data);
extern unsigned int dot_ram_0_read8(unsigned int address);
extern unsigned int dot_ram_1_read8(unsigned int address);
extern void dot_ram_0_write8(unsigned int address, unsigned int data);
extern void dot_ram_1_write8(unsigned int address, unsigned int data);


/***************************************************************/
/*    WORD-RAM 0 & 1 CELL image MAIN-CPU interface (1M mode)   */
/***************************************************************/
extern unsigned int cell_ram_0_read16(unsigned int address);
extern unsigned int cell_ram_1_read16(unsigned int address);
extern void cell_ram_0_write16(unsigned int address, unsigned int data);
extern void cell_ram_1_write16(unsigned int address, unsigned int data);
extern unsigned int cell_ram_0_read8(unsigned int address);
extern unsigned int cell_ram_1_read8(unsigned int address);
extern void cell_ram_0_write8(unsigned int address, unsigned int data);
extern void cell_ram_1_write8(unsigned int address, unsigned int data);


/***************************************************************/
/*            Rotation / Scaling operation (2M mode)           */
/***************************************************************/
extern void gfx_init(void);
extern void gfx_reset(void);
extern int gfx_context_save(uint8 *state);
extern int gfx_context_load(uint8 *state);
extern void gfx_start(unsigned int base, int cycles);
extern void gfx_update(int cycles);

#endif
