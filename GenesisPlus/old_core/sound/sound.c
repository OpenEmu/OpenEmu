/***************************************************************************************
 *  Genesis Plus
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
 *  Sound Hardware
 ****************************************************************************************/

#include "shared.h"
#include "Fir_Resampler.h"

/* cycle-accurate samples */
static int m68cycles_per_sample[2];

/* return the number of samples that should be retrieved */
static inline int fm_sample_cnt(uint8 z80)
{
  if (z80) return ((((count_z80 + current_z80 - z80_ICount) * 15) / (m68cycles_per_sample[0] * 7)) - snd.fm.pos);
  else return ((count_m68k / m68cycles_per_sample[0]) - snd.fm.pos);
}

static inline int psg_sample_cnt(uint8 z80)
{
  if (z80) return ((((count_z80 + current_z80 - z80_ICount) * 15) / (m68cycles_per_sample[1] * 7)) - snd.psg.pos);
  else return ((count_m68k / m68cycles_per_sample[1]) - snd.psg.pos);
}

/* update FM samples */
static inline void fm_update(int cnt)
{
  if (cnt > 0)
  {
    YM2612Update(cnt);
    snd.fm.pos += cnt;
  }
}

/* update PSG samples */
static inline void psg_update(int cnt)
{
  if (cnt > 0)
  {
    SN76489_Update(snd.psg.buffer + snd.psg.pos, cnt);
    snd.psg.pos += cnt;
  }
}

void sound_init(int rate)
{
  double vclk = (vdp_pal ? (double)CLOCK_PAL : (double)CLOCK_NTSC) / 7.0;  /* 68000 and YM2612 clock */
  double zclk = (vdp_pal ? (double)CLOCK_PAL : (double)CLOCK_NTSC) / 15.0; /* Z80 and SN76489 clock  */

  /* cycle-accurate FM & PSG samples */
  m68cycles_per_sample[0] = (int)(((double)m68cycles_per_line * (double)lines_per_frame * (double)vdp_rate / (double)rate) + 0.5);
  m68cycles_per_sample[1] = m68cycles_per_sample[0];

  /* YM2612 is emulated at its original frequency (VLCK/144) */
  if (config.hq_fm)
  {
    m68cycles_per_sample[0] = 144;
    Fir_Resampler_time_ratio(vclk/144.0/(double)rate);
  }

  /* initialize sound chips */
  SN76489_Init((int)zclk,rate);
  YM2612Init((int)vclk,rate);
} 

void sound_update(int fm_len, int psg_len)
{
  /* update last samples (if needed) */
  fm_update(fm_len - snd.fm.pos);
  psg_update(psg_len - snd.psg.pos);

  /* reset samples count */
  snd.fm.pos   = 0;
  snd.psg.pos  = 0;
}

/* write FM chip */
void fm_write(unsigned int cpu, unsigned int address, unsigned int data)
{
  if (address & 1) fm_update(fm_sample_cnt(cpu));
  YM2612Write(address, data);
}

/* read FM status */
unsigned int fm_read(unsigned int cpu, unsigned int address)
{
  fm_update(fm_sample_cnt(cpu));
  return YM2612Read();
}


/* PSG write */
void psg_write(unsigned int cpu, unsigned int data)
{
  psg_update(psg_sample_cnt(cpu));
  SN76489_Write(data);
}
