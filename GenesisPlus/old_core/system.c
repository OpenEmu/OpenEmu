/***************************************************************************************
 *  Genesis Plus
 *  Main Emulation
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
#include "Fir_Resampler.h"
#include "eq.h"

#define SND_SIZE (snd.buffer_size * sizeof(int16))
extern void update_input(void);

/* Global variables */
t_bitmap bitmap;
t_snd snd;
uint32 count_m68k;
uint32 line_m68k;
uint32 hint_m68k;
uint32 count_z80;
uint32 line_z80;
int32 current_z80;
uint8 system_hw;

/****************************************************************
 * AUDIO equalizer
 ****************************************************************/
static EQSTATE eq;

void audio_init_equalizer(void)
{
  init_3band_state(&eq,880,5000,snd.sample_rate);
  audio_set_equalizer();
}

void audio_set_equalizer(void)
{
  eq.lg = (double)(config.lg);
  eq.mg = (double)(config.mg);
  eq.hg = (double)(config.hg);
}

/****************************************************************
 * AUDIO stream update
 ****************************************************************/
static int ll, rr;
void audio_update (int size)
{
  int i;
  int l, r;
  int psg_preamp = config.psg_preamp;
  int fm_preamp  = config.fm_preamp;
  int filter = config.filter;

  int16 *fm[2] = {snd.fm.buffer[0],snd.fm.buffer[1]};
  int16 *psg = snd.psg.buffer;

#ifdef NGC
  int16 *sb = (int16 *) soundbuffer[mixbuffer];
#endif

  /* resampling */
  if (config.hq_fm)
  {
    int fm_len = (int) ((double)size * Fir_Resampler_ratio() + 0.5) ;
    sound_update(fm_len,size);
    Fir_Resampler_read(fm,size);
  }
  else
  {
    sound_update(size,size);
  }

  /* mix samples */
  for (i = 0; i < size; i ++)
  {
    /* PSG samples (mono) */
    l = r = (((int)*psg++) * psg_preamp)/100;

    /* FM samples (stereo) */
    l += (*fm[0]++ * fm_preamp)/100;
    r += (*fm[1]++ * fm_preamp)/100;

    /* filtering */
    if (filter & 1)
    {
      /* single-pole low-pass filter (6 dB/octave) */
      l = (ll + l) >> 1;
      r = (rr + r) >> 1;
      ll = l;
      rr = r;
    }
    else if (filter & 2)
    {
      /* 3 Band EQ */
      l = do_3band(&eq,l);
      r = do_3band(&eq,r);
    }

    /* clipping */
    if (l > 32767) l = 32767;
    else if (l < -32768) l = -32768;
    if (r > 32767) r = 32767;
    else if (r < -32768) r = -32768;

    /* update sound buffer */
#ifndef NGC
    snd.buffer[0][i] = l;
    snd.buffer[1][i] = r;
#else
    *sb++ = r;
    *sb++ = l;
#endif
  }
}

/****************************************************************
 * AUDIO System initialization
 ****************************************************************/
int audio_init (int rate)
{
  /* Shutdown first */
  audio_shutdown();

  /* Clear the sound data context */
  memset(&snd, 0, sizeof (snd));

  /* Make sure the requested sample rate is valid */
  if (!rate || ((rate < 8000) | (rate > 48000))) return (-1);
  snd.sample_rate = rate;

  /* Calculate the sound buffer size (for one frame) */
#ifdef NGC
  snd.buffer_size = (rate / vdp_rate) + 8;

#else
  snd.buffer_size = (rate / vdp_rate);

  /* Output buffers */
  snd.buffer[0] = (int16 *) malloc(SND_SIZE);
  snd.buffer[1] = (int16 *) malloc(SND_SIZE);
  if (!snd.buffer[0] || !snd.buffer[1]) return (-1);
#endif

  /* SN76489 stream buffers */
  snd.psg.buffer = (int16 *)malloc (SND_SIZE);
  if (!snd.psg.buffer) return (-1);

  /* YM2612 stream buffers */
  snd.fm.buffer[0] = (int16 *)malloc (SND_SIZE);
  snd.fm.buffer[1] = (int16 *)malloc (SND_SIZE);
  if (!snd.fm.buffer[0] || !snd.fm.buffer[1]) return (-1);

  /* Resampling buffer */
  if (config.hq_fm)
  {
    if (!Fir_Resampler_initialize(4096)) return (-1);
  }

  /* 3 band EQ */
  audio_init_equalizer();

  /* Set audio enable flag */
  snd.enabled = 1;

  /* Initialize Sound Chips emulation */
  sound_init(rate);

  return (0);
}

/****************************************************************
 * AUDIO System shutdown
 ****************************************************************/
void audio_shutdown(void)
{
  /* Sound buffers */
  if (snd.buffer[0])    free(snd.buffer[0]);
  if (snd.buffer[1])    free(snd.buffer[1]);
  if (snd.fm.buffer[0]) free(snd.fm.buffer[0]);
  if (snd.fm.buffer[1]) free(snd.fm.buffer[1]);
  if (snd.psg.buffer)   free(snd.psg.buffer);

  /* Resampling buffer */
  Fir_Resampler_shutdown();

  /* sn76489 chip (Blip Buffer allocated memory) */
  SN76489_Shutdown();
}

/****************************************************************
 * Virtual Genesis initialization
 ****************************************************************/
void system_init (void)
{
  gen_init ();
  vdp_init ();
  render_init ();
  cart_hw_init();
}

/****************************************************************
 * Virtual Genesis Hard Reset
 ****************************************************************/
void system_reset (void)
{
  /* Cartridge Hardware */
  cart_hw_reset();

  /* Genesis Hardware */
  gen_reset (1); 
  vdp_reset ();
  render_reset ();
  io_reset();
  SN76489_Reset();

  /* Clear Sound Buffers */
  if (snd.psg.buffer) memset (snd.psg.buffer, 0, SND_SIZE);
  if (snd.fm.buffer[0]) memset (snd.fm.buffer[0], 0, SND_SIZE);
  if (snd.fm.buffer[1]) memset (snd.fm.buffer[1], 0, SND_SIZE);
  Fir_Resampler_clear();
}

/****************************************************************
 * Virtual Genesis shutdown
 ****************************************************************/
void system_shutdown (void)
{
  gen_shutdown ();
  vdp_shutdown ();
  render_shutdown ();
}

/****************************************************************
 * Virtual Genesis Frame emulation
 ****************************************************************/
int system_frame (int do_skip)
{
  if (!gen_running)
  {
    update_input();
    return 0;
  }

  uint32 aim_m68k = 0;
  uint32 aim_z80  = 0;

  /* reset cycles counts */
  count_m68k      = 0;
  count_z80       = 0;
  fifo_write_cnt  = 0;
  fifo_lastwrite  = 0;

  /* update display settings */
  int line;
  int reset = resetline;
  int vdp_height  = bitmap.viewport.h;
  int end_line    = vdp_height + bitmap.viewport.y;
  int start_line  = lines_per_frame - bitmap.viewport.y;
  int old_interlaced = interlaced;
  interlaced = (reg[12] & 2) >> 1;
  if (old_interlaced != interlaced)
  {
    bitmap.viewport.changed |= 1;
    im2_flag = ((reg[12] & 6) == 6);
    odd_frame = 1;
  }
  odd_frame ^= 1;

#ifdef NGC
  if (bitmap.viewport.changed & 2)
  {
    /* Update the width of the viewport */
    bitmap.viewport.w = (reg[12] & 1) ? 320 : 256;
    bitmap.viewport.changed = 1;

    /* Update clipping */
    window_clip();
  }
#endif

  /* clear VBLANK and DMA flags */
  status &= 0xFFF5;

  /* even/odd field flag (interlaced modes only) */
  if (odd_frame && interlaced) status |= 0x0010;
  else status &= 0xFFEF;

  /* reload HCounter */
  int h_counter = reg[10];

  /* parse sprites for line 0 (done on last line) */
  parse_satb (0x80);

  /* process scanlines */
  for (line = 0; line < lines_per_frame; line ++)
  {
    /* update VCounter */
    v_counter = line;

    /* update 6-Buttons or Menacer */
    input_update();

    /* update CPU cycle counters */
    hint_m68k = count_m68k;
    line_m68k = aim_m68k;
    line_z80  = aim_z80;
    aim_z80  += z80cycles_per_line;
    aim_m68k += m68cycles_per_line;

    /* Soft Reset ? */
    if (line == reset)
    {
#ifdef NGC
      /* wait for RESET button to be released */
      while (SYS_ResetButtonDown());
#endif
      gen_reset(0);
    }

    /* active display */
    if (line <= vdp_height)
    {
      /* H Interrupt */
      if(--h_counter < 0)
      {
        h_counter = reg[10];
        hint_pending = 1;
        if (reg[0] & 0x10) irq_status = (irq_status & ~0x40) | 0x14;

        /* adjust timings to take further decrement in account (see below) */
        if ((line != 0) || (h_counter == 0)) aim_m68k += 36;
      }

      /* HINT will be triggered on next line, approx. 36 cycles before VDP starts line rendering */
      /* during this period, any VRAM/CRAM/VSRAM writes should NOT be taken in account before next line */
      /* as a result, current line is shortened */
      /* fix Lotus 1, Lotus 2 RECS, Striker, Zero the Kamikaze Squirell */
      if ((line < vdp_height) && (h_counter == 0)) aim_m68k -= 36;

      /* update DMA timings */
      if (dma_length) vdp_update_dma();

      /* vertical retrace */
      if (line == vdp_height)
      {
        /* render overscan */
        if ((line < end_line) && (!do_skip)) render_line(line, 1);

        /* update inputs (doing this here fix Warriors of Eternal Sun) */
        update_input();

        /* set VBLANK flag */
        status |= 0x08;

        /* Z80 interrupt is 16ms period (one frame) and 64us length (one scanline) */
        zirq = 1;
        z80_set_irq_line(0, ASSERT_LINE);  

        /* delay between HINT, VBLANK and VINT (Dracula, OutRunners, VR Troopers) */
        m68k_run(line_m68k + 84);
        if (zreset && !zbusreq)
        {
          current_z80 = line_z80 + 39 - count_z80;
          if (current_z80 > 0) count_z80 += z80_execute(current_z80);
        }
        else count_z80 = line_z80 + 39;

        /* V Interrupt */
        status |= 0x80;

        /* 36 cycles latency after VINT occurence flag (Ex-Mutants, Tyrant) */
        m68k_run(line_m68k + 113);
        vint_pending = 1;
        if (reg[1] & 0x20) irq_status = (irq_status & ~0x40) | 0x36;
      }
      else if (!do_skip) 
      {
        /* render scanline and parse sprites for line n+1 */
        render_line(line, 0);
        if (line < (vdp_height-1)) parse_satb(0x81 + line);
      }
    }
    else
    {
      /* update DMA timings */
      if (dma_length) vdp_update_dma();

      /* render overscan */
      if ((!do_skip) && ((line < end_line) || (line >= start_line))) render_line(line, 1);

      /* clear any pending Z80 interrupt */
      if (zirq)
      {
        zirq = 0;
        z80_set_irq_line(0, CLEAR_LINE);
      }
    }

    /* process line */
    m68k_run(aim_m68k);
    if (zreset == 1 && zbusreq == 0)
    {
      current_z80 = aim_z80 - count_z80;
      if (current_z80 > 0) count_z80 += z80_execute(current_z80);
    }
    else count_z80 = aim_z80;
    
    /* SVP chip */
    if (svp) ssp1601_run(SVP_cycles);
  }

  return gen_running;
}
