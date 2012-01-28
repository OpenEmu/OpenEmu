/***************************************************************************************
 *  Genesis Plus
 *  Video Display Processor (memory handlers)
 *
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald (original code)
 *  Eke-Eke (2007,2008,2009), additional code & fixes for the GCN/Wii port
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ****************************************************************************************/

#include "shared.h"
#include "hvc.h"

/* Pack and unpack CRAM data */
#define PACK_CRAM(d)    ((((d)&0xE00)>>9)|(((d)&0x0E0)>>2)|(((d)&0x00E)<<5))
#define UNPACK_CRAM(d)  ((((d)&0x1C0)>>5)|((d)&0x038)<<2|(((d)&0x007)<<9))

/* Mark a pattern as dirty */
#define MARK_BG_DIRTY(addr)                                           \
{                                                                     \
  name = (addr >> 5) & 0x7FF;                                         \
  if(bg_name_dirty[name] == 0) bg_name_list[bg_list_index++] = name;  \
  bg_name_dirty[name] |= (1 << ((addr >> 2) & 0x07));                 \
}

/* VDP context */
uint8 sat[0x400];     /* Internal copy of sprite attribute table */
uint8 vram[0x10000];  /* Video RAM (64Kx8) */
uint8 cram[0x80];     /* On-chip color RAM (64x9) */
uint8 vsram[0x80];    /* On-chip vertical scroll RAM (40x11) */
uint8 reg[0x20];      /* Internal VDP registers (23x8) */
uint16 addr;          /* Address register */
uint16 addr_latch;    /* Latched A15, A14 of address */
uint8 code;           /* Code register */
uint8 pending;        /* Pending write flag */
uint16 status;        /* VDP status flags */
uint8 dmafill;        /* next VDP Write is DMA Fill */
uint8 hint_pending;   /* 0= Line interrupt is pending */
uint8 vint_pending;   /* 1= Frame interrupt is pending */
uint8 irq_status;     /* Interrupt lines updated */

/* Global variables */
uint16 ntab;                      /* Name table A base address */
uint16 ntbb;                      /* Name table B base address */
uint16 ntwb;                      /* Name table W base address */
uint16 satb;                      /* Sprite attribute table base address */
uint16 hscb;                      /* Horizontal scroll table base address */
uint8 border;                     /* Border color index */
uint8 bg_name_dirty[0x800];       /* 1= This pattern is dirty */
uint16 bg_name_list[0x800];       /* List of modified pattern indices */
uint16 bg_list_index;             /* # of modified patterns in list */
uint8 bg_pattern_cache[0x80000];  /* Cached and flipped patterns */
uint8 playfield_shift;            /* Width of planes A, B (in bits) */
uint8 playfield_col_mask;         /* Vertical scroll mask */
uint16 playfield_row_mask;        /* Horizontal scroll mask */
uint32 y_mask;                    /* Name table Y-index bits mask */
uint16 hc_latch;                  /* latched HCounter (INT2) */
uint16 v_counter;                 /* VDP scanline counter */
uint32 dma_length;                /* Current DMA remaining bytes */
int32 fifo_write_cnt;             /* VDP writes fifo count */
uint32 fifo_lastwrite;            /* last VDP write cycle */
uint8 fifo_latency;               /* VDP write cycles latency */
uint8 odd_frame;                  /* 1: odd field, 0: even field */
uint8 im2_flag;                   /* 1= Interlace mode 2 is being used */
uint8 interlaced;                 /* 1: Interlaced mode 1 or 2 */
uint8 vdp_pal  = 0;               /* 1: PAL , 0: NTSC (default) */
uint8 vdp_rate;                   /* PAL: 50hz, NTSC: 60hz */
uint16 lines_per_frame;           /* PAL: 313 lines, NTSC: 262 lines */


/* Tables that define the playfield layout */
static const uint8 shift_table[] = { 6, 7, 0, 8 };
static const uint8 col_mask_table[] = { 0x0F, 0x1F, 0x0F, 0x3F };
static const uint16 row_mask_table[] = { 0x0FF, 0x1FF, 0x2FF, 0x3FF };
static const uint32 y_mask_table[] = { 0x1FC0, 0x1F80, 0x1FC0, 0x1F00 };

static uint16 sat_base_mask;      /* Base bits of SAT */
static uint16 sat_addr_mask;      /* Index bits of SAT */
static uint32 dma_endCycles;      /* 68k cycles to DMA end */
static uint8 dma_type;            /* Type of DMA */

/* DMA Timings

 According to the manual, here's a table that describes the transfer
 rates of each of the three DMA types:

    DMA Mode      Width       Display      Transfer Count
    -----------------------------------------------------
    68K > VDP     32-cell     Active       16
                              Blanking     167
                  40-cell     Active       18
                              Blanking     205
    VRAM Fill     32-cell     Active       15
                              Blanking     166
                  40-cell     Active       17
                              Blanking     204
    VRAM Copy     32-cell     Active       8
                              Blanking     83
                  40-cell     Active       9
                              Blanking     102

 'Active' is the active display period, 'Blanking' is either the vertical
 blanking period or when the display is forcibly blanked via register #1.

 The above transfer counts are all in bytes, unless the destination is
 CRAM or VSRAM for a 68K > VDP transfer, in which case it is in words.

*/
static const uint32 dma_rates[16] = {
  8,   83,  9, 102, /* 68K to VRAM (1 word = 2 bytes) */
  16, 167, 18, 205, /* 68K to CRAM or VSRAM */
  15, 166, 17, 204, /* DMA fill */
  8,   83,  9, 102, /* DMA Copy */
};

/*--------------------------------------------------------------------------*/
/* Functions prototype                                                      */
/*--------------------------------------------------------------------------*/
static inline void fifo_update();
static inline void data_w(unsigned int data);
static inline void reg_w(unsigned int r, unsigned int d);
static inline void dma_copy(void);
static inline void dma_vbus (void);
static inline void dma_fill(unsigned int data);

/*--------------------------------------------------------------------------*/
/* Init, reset, shutdown functions                                          */
/*--------------------------------------------------------------------------*/
void vdp_init(void)
{
  /* PAL/NTSC timings */
  vdp_rate        = vdp_pal ? 50 : 60;
  lines_per_frame = vdp_pal ? 313 : 262;
}

void vdp_reset(void)
{
  memset ((char *) sat, 0, sizeof (sat));
  memset ((char *) vram, 0, sizeof (vram));
  memset ((char *) cram, 0, sizeof (cram));
  memset ((char *) vsram, 0, sizeof (vsram));
  memset ((char *) reg, 0, sizeof (reg));

  addr = 0;
  addr_latch = 0;
  code = 0;
  pending = 0;

  status = 0x200; /* fifo empty */
  status |= vdp_pal;

  ntab = 0;
  ntbb = 0;
  ntwb = 0;
  satb = 0;
  hscb = 0;

  sat_base_mask = 0xFE00;
  sat_addr_mask = 0x01FF;

  border = 0x00;

  memset ((char *) bg_name_dirty, 0, sizeof (bg_name_dirty));
  memset ((char *) bg_name_list, 0, sizeof (bg_name_list));
  bg_list_index = 0;
  memset ((char *) bg_pattern_cache, 0, sizeof (bg_pattern_cache));

  playfield_shift = 6;
  playfield_col_mask = 0x0F;
  playfield_row_mask = 0x0FF;
  y_mask = 0x1FC0;

  hint_pending = 0;
  vint_pending = 0;
  irq_status = 0;

  hc_latch  = 0;
  v_counter = 0;

  dmafill = 0;
  dma_length = 0;
  dma_endCycles = 0;

  im2_flag = 0;
  interlaced = 0;
  odd_frame   = 0;

  fifo_write_cnt = 0;

  /* reset HVC tables */
  vctab = (vdp_pal) ? vc_pal_224 : vc_ntsc_224;
  hctab = cycle2hc32;

  /* reset display area */
  bitmap.viewport.w = 256;
  bitmap.viewport.h = 224;
  bitmap.viewport.oh = 256;
  bitmap.viewport.ow = 224;

  /* reset border area */
  bitmap.viewport.x = config.overscan ? 14 : 0;
  bitmap.viewport.y = config.overscan ? (vdp_pal ? 32 : 8) : 0;
  bitmap.viewport.changed = 2;

  /* initialize some registers (normally set by BIOS) */
  if (config.bios_enabled != 3)
  {
    reg_w(1 , 0x04);  /* Mode 5 enabled */
    reg_w(10, 0xff);  /* HINT disabled */
    reg_w(12, 0x81);  /* H40 mode */
    reg_w(15, 0x02);  /* auto increment */
  }

  /* default latency */
  fifo_latency = 27;
}

void vdp_shutdown(void)
{}

void vdp_restore(uint8 *vdp_regs)
{
  int i;
  
  for (i=0;i<0x20;i++) 
  {
    reg_w(i, vdp_regs[i]);
  }

  /* reinitialize HVC tables */
  vctab = (vdp_pal) ? ((reg[1] & 8) ? vc_pal_240 : vc_pal_224) : vc_ntsc_224;
  hctab = (reg[12] & 1) ? cycle2hc40 : cycle2hc32;

  /* reinitialize overscan area */
  bitmap.viewport.x = config.overscan ? 14 : 0;
  bitmap.viewport.y = config.overscan ? (((reg[1] & 8) ? 0 : 8) + (vdp_pal ? 24 : 0)) : 0;
  bitmap.viewport.changed = 2;

  /* restore VDP timings */
  fifo_latency = (reg[12] & 1) ? 27 : 30;
  if ((code & 0x0F) == 0x01) fifo_latency = fifo_latency * 2;

  /* remake cache */
  for (i=0;i<0x800;i++) 
  {
    bg_name_list[i]=i;
    bg_name_dirty[i]=0xFF;
  }
  bg_list_index=0x800;

  /* reinitialize palette */
  color_update(0x00, *(uint16 *)&cram[border << 1]);
  for(i = 1; i < 0x40; i += 1)
  {
    color_update(i, *(uint16 *)&cram[i << 1]);
  }
}


/*--------------------------------------------------------------------------*/
/* DMA Timings update                                                       */
/*--------------------------------------------------------------------------*/

void vdp_update_dma()
{
  int dma_cycles = 0;

  /* DMA timings table index */
  int index = (4 * dma_type) + ((reg[12] & 1)*2);
  if ((status&8) || !(reg[1] & 0x40)) index++;

  /* DMA transfer rate */
  int rate = dma_rates[index];

  /* 68k cycles left */
  int left_cycles = (line_m68k + m68cycles_per_line) - count_m68k;
  if (left_cycles < 0) left_cycles = 0;

  /* DMA bytes left */
  int dma_bytes = (left_cycles * rate) / m68cycles_per_line;

  /* determinate DMA length in CPU cycles */
  if (dma_length < dma_bytes)
  {
    /* DMA will be finished during this line */
    dma_cycles = (dma_length * m68cycles_per_line) / rate;
    dma_length = 0;
  }
  else
  {
    /* DMA can not be finished until next scanline */
    dma_cycles = left_cycles;
    dma_length -= dma_bytes;
  }

  /* update 68k cycles counter */
  if (dma_type < 2)
  {
    /* 68K COPY to V-RAM */
    /* 68K is frozen during DMA operation */
    count_m68k += dma_cycles;
  }
  else
  {
    /* VRAM Fill or VRAM Copy */
    /* set DMA end cyles count */
    dma_endCycles = count_m68k + dma_cycles;

    /* set DMA Busy flag */
    status |= 0x0002;
  }
}

/*--------------------------------------------------------------------------*/
/* VDP Ports handler                                                        */
/*--------------------------------------------------------------------------*/

void vdp_ctrl_w(unsigned int data)
{
  if (pending == 0)
  {
    if ((data & 0xC000) == 0x8000)
    {
      /* VDP register write */
      uint8 r = (data >> 8) & 0x1F;
      uint8 d = data & 0xFF;
      reg_w(r,d);
    }
    else pending = 1;

    addr = ((addr_latch & 0xC000) | (data & 0x3FFF));
    code = ((code & 0x3C) | ((data >> 14) & 0x03));
  }
  else
  {
    /* Clear pending flag */
    pending = 0;

    /* Update address and code registers */
    addr = ((addr & 0x3FFF) | ((data & 3) << 14));
    code = ((code & 0x03) | ((data >> 2) & 0x3C));

    /* Save address bits A15 and A14 */
    addr_latch = (addr & 0xC000);

    /* DMA operation */
    if ((code & 0x20) && (reg[1] & 0x10))
    {
      switch (reg[23] & 0xC0)
      {
        case 0x00:    /* V bus to VDP DMA */
        case 0x40:    /* V bus to VDP DMA */
          dma_vbus();
          break;

        case 0x80:    /* VRAM fill */
          dmafill = 1;
          break;

        case 0xC0:    /* VRAM copy */
          dma_copy();
          break;
      }
    }
  }

  /* FIFO emulation:
     ---------------
    HDISP is 256*10/7 = approx. 366 cycles (same for both modes)
    this gives:
      H32: 16 accesses --> 366/16 = 23 cycles per access
      H40: 20 accesses --> 366/20 = 18 cycles per access

    VRAM access are byte wide --> VRAM writes takes 2x CPU cycles
    Memory access requires some additional cyles, the following values 
    seems to work fine (see Chaos Engine/Soldier of Fortune) 
  */
  fifo_latency = (reg[12] & 1) ? 27 : 30;
  if ((code & 0x0F) == 0x01) fifo_latency = fifo_latency * 2;
}

/*
 * Return VDP status
 *
 * Bits are
 * 0   0:1 ntsc:pal
 * 1  DMA Busy
 * 2  During HBlank
 * 3  During VBlank
 * 4  Frame Interlace 0:even 1:odd
 * 5  Sprite collision
 * 6  Too many sprites per line
 * 7  v interrupt occurred
 * 8  Write FIFO full
 * 9  Write FIFO empty
 * 10 - 15  Next word on bus
 */
unsigned int vdp_ctrl_r(void)
{
  /* update FIFO flags */
  fifo_update();
  if (fifo_write_cnt < 4)
  {
    status &= 0xFEFF;
    if (fifo_write_cnt == 0) status |= 0x200; 
  }
  else status ^= 0x200;

  /* update DMA Busy flag */
  if ((status & 2) && !dma_length && (count_m68k >= dma_endCycles))
  {
    status &= 0xFFFD;
  }

  unsigned int temp = status;

  /* display OFF: VBLANK flag is set */
  if (!(reg[1] & 0x40)) temp |= 0x8; 

  /* HBLANK flag (Sonic 3 and Sonic 2 "VS Modes", Lemmings 2) */
  if ((count_m68k <= (line_m68k + 84)) || (count_m68k > (line_m68k + m68cycles_per_line))) temp |= 0x4;

  /* clear pending flag */
  pending = 0;

  /* clear SPR/SCOL flags */
  status &= 0xFF9F;

  return (temp);
}

void vdp_data_w(unsigned int data)
{
  /* Clear pending flag */
  pending = 0;

  if (dmafill)
  {
    dma_fill(data);
    return;
  }

  /* update VDP FIFO (during HDISPLAY only) */
  if (!(status&8) && (reg[1]&0x40))
  {
    fifo_update();
    if (fifo_write_cnt == 0)
    {
      /* reset cycle counter */
      fifo_lastwrite = count_m68k;

      /* FIFO is not empty anymore */
      status &= 0xFDFF;
    }

    /* increase write counter */
    fifo_write_cnt ++;

    /* is FIFO full ? */
    if (fifo_write_cnt >= 4)
    {
      status |= 0x100; 

      /* VDP latency (Chaos Engine, Soldiers of Fortune, Double Clutch) */
      if (fifo_write_cnt > 4) count_m68k = fifo_lastwrite + fifo_latency;
    }
  }

  /* write data */
  data_w(data);
}

unsigned int vdp_data_r(void)
{
  uint16 temp = 0;

  /* Clear pending flag */
  pending = 0;

  switch (code & 0x0F)
  {
    case 0x00:  /* VRAM */
      temp = *(uint16 *) & vram[(addr & 0xFFFE)];
      break;

    case 0x08:  /* CRAM */
      temp = *(uint16 *) & cram[(addr & 0x7E)];
      temp = UNPACK_CRAM (temp);
      break;

    case 0x04:  /* VSRAM */
      temp = *(uint16 *) & vsram[(addr & 0x7E)];
      break;
  }

  /* Increment address register */
  addr += reg[15];

  /* return data */
  return (temp);
}

unsigned int vdp_hvc_r(void)
{
  uint8 hc = (hc_latch & 0x100) ? (hc_latch & 0xFF) : hctab[count_m68k % m68cycles_per_line]; 
  uint8 vc = vctab[v_counter];

  /* interlace mode 2 */
  if (im2_flag) vc = (vc << 1) | ((vc >> 7) & 1);

  return ((vc << 8) | hc);
}

void vdp_test_w(unsigned int value)
{
#ifdef LOGERROR
  error("Unused VDP Write 0x%x (%08x)\n", value, m68k_get_reg (NULL, M68K_REG_PC));
#endif
}

/*--------------------------------------------------------------------------*/
/* VDP Interrupts callback                                                  */
/*--------------------------------------------------------------------------*/

int vdp_int_ack_callback(int int_level)
{
  /* VINT triggered ? */
  if (irq_status&0x20)
  {
    vint_pending = 0;
    status &= ~0x80;  /* clear VINT flag */
  }
  else
  {
    hint_pending = 0;
  }

  /* update IRQ status */
  irq_status = 0x10;
  if (vint_pending && (reg[1] & 0x20)) irq_status |= 6;
  else if (hint_pending && (reg[0] & 0x10)) irq_status |= 4;

  return M68K_INT_ACK_AUTOVECTOR;
}

/*--------------------------------------------------------------------------*/
/* FIFO emulation                                                  */
/*--------------------------------------------------------------------------*/
static inline void fifo_update()
{
  if (fifo_write_cnt > 0)
  {
    /* update FIFO reads */
    uint32 fifo_read = ((count_m68k - fifo_lastwrite) / fifo_latency);
    if (fifo_read > 0)
    {
      fifo_write_cnt -= fifo_read;
      if (fifo_write_cnt < 0) fifo_write_cnt = 0;

      /* update cycle count */
      fifo_lastwrite += fifo_read*fifo_latency;
    }
  }
}

/*--------------------------------------------------------------------------*/
/* Memory access functions                                                  */
/*--------------------------------------------------------------------------*/
static inline void data_w(unsigned int data)
{
  switch (code & 0x0F)
  {
    case 0x01:  /* VRAM */

      /* Byte-swap data if A0 is set */
      if (addr & 1) data = (data >> 8) | (data << 8);

      /* Copy SAT data to the internal SAT */
      if ((addr & sat_base_mask) == satb)
      {
        *(uint16 *) &sat[addr & sat_addr_mask & 0xFFFE] = data;
      }

      /* Only write unique data to VRAM */
      if (data != *(uint16 *) &vram[addr & 0xFFFE])
      {
        /* Write data to VRAM */
        *(uint16 *) &vram[addr & 0xFFFE] = data;

        /* Update the pattern cache */
        int name;
        MARK_BG_DIRTY (addr);
      }
      break;

    case 0x03:  /* CRAM */
    {
      uint16 *p = (uint16 *) &cram[(addr & 0x7E)];
      data = PACK_CRAM (data & 0x0EEE);
      if (data != *p)
      {
        int index = (addr >> 1) & 0x3F;
        *p = data;
        if (index) color_update (index, *p);
        if (border == index) color_update (0x00, *p);
      }
      break;
    }

    case 0x05:  /* VSRAM */
      *(uint16 *) &vsram[(addr & 0x7E)] = data;
      break;
  }

  /* Increment address register */
  addr += reg[15];
}

/*
    The reg[] array is updated at the *end* of this function, so the new
    register data can be compared with the previous data.
*/
static inline void reg_w(unsigned int r, unsigned int d)
{
  /* See if Mode 4 (SMS mode) is enabled 
     According to official doc, VDP registers #11 to #23 can not be written unless bit2 in register #1 is set
     Fix Captain Planet & Avengers (Alt version), Bass Master Classic Pro Edition (they incidentally activate Mode 4) 
  */
  if (!(reg[1] & 4) && (r > 10)) return;

  switch(r)
  {
    case 0: /* CTRL #1 */

      /* Line Interrupt */
      if (((d&0x10) != (reg[0]&0x10)) && hint_pending)
      {
        /* update IRQ status */
        irq_status = 0x50;
        if (vint_pending && (reg[1] & 0x20)) irq_status |= 0x26;
        else if (d & 0x10) irq_status |= 4;
      }

      /* Palette bit  */
      if ((d&0x04) != (reg[0]&0x04))
      {
        /* Update colors */
        reg[0] = d;
        int i;
        color_update (0x00, *(uint16 *) & cram[border << 1]);
        for (i = 1; i < 0x40; i += 1)
        {
          color_update (i, *(uint16 *) & cram[i << 1]);
        }
      }
      break;

    case 1: /* CTRL #2 */

      /* Frame Interrupt */
      if (((d&0x20) != (reg[1]&0x20)) && vint_pending)
      {
        /* update IRQ status */
        irq_status = 0x50;
        if (d & 0x20) irq_status |= 0x26;
        else if (hint_pending && (reg[0] & 0x10)) irq_status |= 4;
      }

      /* See if the viewport height has actually been changed */
      if ((d & 8) != (reg[1] & 8))
      {
        /* update the height of the viewport */
        bitmap.viewport.changed |= 1;
        bitmap.viewport.h = (d & 8) ? 240 : 224;

        /* update overscan height */
        if (config.overscan) bitmap.viewport.y = ((vdp_pal ? 288 : 240) - bitmap.viewport.h) / 2;

        /* update VC table */
        if (vdp_pal) vctab = (d & 8) ? vc_pal_240 : vc_pal_224;
      }

      /* Display activated/blanked during Horizontal Blanking */
      if (((d&0x40) != (reg[1]&0x40)) && !(status & 8))
      {
        if (count_m68k <= (hint_m68k + 120))
        {
          /* Redraw the current line :
            - Legend of Galahad, Lemmings 2, Nigel Mansell's World Championship Racing (set display OFF)
            - Deadly Moves aka Power Athlete (set display ON)
          */
          reg[1] = d;
          render_line(v_counter, 0);
        }
      }
      break;

    case 2: /* NTAB */
      ntab = (d << 10) & 0xE000;
      break;

    case 3: /* NTWB */
      ntwb = (d << 10) & 0xF800;
      if(reg[12] & 1) ntwb &= 0xF000;
      break;

    case 4: /* NTBB */
      ntbb = (d << 13) & 0xE000;
      break;

    case 5: /* SATB */
      if (reg[12] & 1)
      {
        sat_base_mask = 0xFC00;
        sat_addr_mask = 0x03FF;
      }
      else
      {
        sat_base_mask = 0xFE00;
        sat_addr_mask = 0x01FF;
      }
      satb = (d << 9) & sat_base_mask;
      break;

    case 7:
      /* See if the border color has actually changed */
      d &= 0x3F;
      if (d != border)
      {
        /* Mark the border color as modified */
        border = d;
        color_update(0x00, *(uint16 *)&cram[(border << 1)]);

        /* background color modified during Horizontal Blanking */
        if (!(status & 8) && (count_m68k <= (line_m68k + 84)))
        {
          /* remap current line (see Road Rash I,II,III) */
          reg[7] = d;
          remap_buffer(v_counter,bitmap.viewport.w + 2*bitmap.viewport.x);
        }
      }
      break;

    case 12:
      /* See if the viewport width has actually been changed */
      if ((d & 1) !=  (reg[12] & 1))
      {
        if (d & 1)
        {
          /* Update display-dependant registers */
          ntwb = (reg[3] << 10) & 0xF000;
          sat_base_mask = 0xFC00;
          sat_addr_mask = 0x03FF;
          satb = (reg[5] << 9) & sat_base_mask;

          /* Update HC table */
          hctab = cycle2hc40;

#ifndef NGC
          /* Update viewport width */
          bitmap.viewport.w = 320;
#endif
        }
        else
        {
          /* Update display-dependant registers */
          ntwb = (reg[3] << 10) & 0xF800;
          sat_base_mask = 0xFE00;
          sat_addr_mask = 0x01FF;
          satb = (reg[5] << 9) & sat_base_mask;

          /* Update HC table */
          hctab = cycle2hc32;

#ifndef NGC
          /* Update viewport width */
          bitmap.viewport.w = 256;
#endif
        }

#ifndef NGC
        /* Update viewport */
        bitmap.viewport.changed = 1;

        /* Update clipping */
        window_clip();
#else
        /* Postpound update on next frame */
        bitmap.viewport.changed = 2;
#endif
      }

      /* See if the S/TE mode bit has changed */
      if ((d & 8) != (reg[12] & 8))
      {
        int i;

        /* The following color update check this value */
        reg[12] = d;

        /* Update colors */
        color_update (0x00, *(uint16 *) & cram[border << 1]);
        for (i = 1; i < 0x40; i += 1)
        {
          color_update (i, *(uint16 *) & cram[i << 1]);
        }
      }
      break;

    case 13: /* HSCB */
      hscb = (d << 10) & 0xFC00;
      break;

    case 16: /* Playfield size */
      playfield_shift = shift_table[(d & 3)];
      playfield_col_mask = col_mask_table[(d & 3)];
      playfield_row_mask = row_mask_table[(d >> 4) & 3];
      y_mask = y_mask_table[(d & 3)];
      break;

    case 17: /* Update clipping */
      reg[17] = d;
      window_clip();
      break;
  }

  /* Write new register value */
  reg[r] = d;
}

/*--------------------------------------------------------------------------*/
/* DMA Operations                                                           */
/*--------------------------------------------------------------------------*/

/*  DMA Copy
    Read byte from VRAM (source), write to VRAM (addr),
    bump source and add r15 to addr.

    - see how source addr is affected
      (can it make high source byte inc?)
*/
static inline void dma_copy(void)
{
  int name;
  int length = (reg[20] << 8 | reg[19]) & 0xFFFF;
  int source = (reg[22] << 8 | reg[21]) & 0xFFFF;
  if (!length) length = 0x10000;

  dma_type = 3;
  dma_length = length;
  vdp_update_dma();

  /* proceed DMA */
  do
  {
    vram[addr] = vram[source];
    MARK_BG_DIRTY(addr);
    source = (source + 1) & 0xFFFF;
    addr += reg[15];
  } while (--length);

  /* update length & source address registers */
  reg[19] = length & 0xFF;
  reg[20] = (length >> 8) & 0xFF;
  reg[21] = source & 0xFF; /* not sure */
  reg[22] = (source >> 8) & 0xFF; 
}

/* 68K Copy to VRAM, VSRAM or CRAM */
static inline void dma_vbus (void)
{
  uint32 base, source = ((reg[23] & 0x7F) << 17 | reg[22] << 9 | reg[21] << 1) & 0xFFFFFE;
  uint32 length = (reg[20] << 8 | reg[19]) & 0xFFFF;
  uint32 temp;
  
  if (!length) length = 0x10000;
  base = source;

  /* DMA timings */
  dma_type = (code & 0x06) ? 1 : 0;
  dma_length = length;
  vdp_update_dma();

  /* DMA source */
  if ((source >> 17) == 0x50)
  {
    /* Z80 & I/O area */
    do
    {
      /* Return $FFFF only when the Z80 isn't hogging the Z-bus.
        (e.g. Z80 isn't reset and 68000 has the bus) */
      if (source <= 0xa0ffff) temp = (zbusack ? *(uint16 *)(work_ram + (source & 0xffff)) : 0xffff);

      /* The I/O chip and work RAM try to drive the data bus which results 
          in both values being combined in random ways when read.
          We return the I/O chip values which seem to have precedence, */
      else if (source <= 0xa1001f)
      {
        temp = io_read((source >> 1) & 0x0f);
        temp = (temp << 8 | temp);
      }

      /* All remaining locations access work RAM */
      else temp = *(uint16 *)(work_ram + (source & 0xffff));

      source += 2;
      source = ((base & 0xFE0000) | (source & 0x1FFFF));
      data_w(temp);
    }
    while (--length);
  }
  else
  {
    /* SVP latency */
    if (svp && (source < 0x400000))
    {
      source = (source - 2);
    }

    /* ROM & RAM */
    do
    {
      temp = *(uint16 *)(m68k_memory_map[source>>16].base + (source & 0xffff));
      source += 2;
      source = ((base & 0xFE0000) | (source & 0x1FFFF));
      data_w(temp);
    }
    while (--length);
  }

  /* update length & source address registers */
  reg[19] = length & 0xFF;
  reg[20] = (length >> 8) & 0xFF;
  reg[21] = (source >> 1) & 0xFF;
  reg[22] = (source >> 9) & 0xFF;
  reg[23] = (reg[23] & 0x80) | ((source >> 17) & 0x7F);
}

/* VRAM FILL */
static inline void dma_fill(unsigned int data)
{
  int name;
  int length = (reg[20] << 8 | reg[19]) & 0xFFFF;
  if (!length) length = 0x10000;

  /* DMA timings */
  dma_type = 2;
  dma_length = length;
  vdp_update_dma();

  /* proceed DMA */
  data_w(data);

  /* write MSB */
  data = (data >> 8) & 0xff;

  /* detect internal SAT modification */
  if ((addr & sat_base_mask) == satb)
  {
    do
    {
      /* update internal SAT (fix Battletech) */
      WRITE_BYTE(sat, (addr & sat_addr_mask)^1, data);
      WRITE_BYTE(vram, addr^1, data);
      MARK_BG_DIRTY (addr);
      addr += reg[15];
    }
    while (--length);
  }
  else
  {
    do
    {
      WRITE_BYTE(vram, addr^1, data);
      MARK_BG_DIRTY (addr);
      addr += reg[15];
    }
    while (--length);
  }

  /* update length register */
  reg[19] = length & 0xFF;
  reg[20] = (length >> 8) & 0xFF;
  dmafill = 0;
}
