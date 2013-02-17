/***************************************************************************************
 *  Genesis Plus
 *  PCM sound chip (315-5476A) (RF5C164 compatible)
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

#define PCM_SCYCLES_RATIO (384 * 4)

#define pcm scd.pcm_hw

static blip_t* blip[2];

void pcm_init(blip_t* left, blip_t* right)
{
  /* number of SCD master clocks run per second */
  double mclk = snd.frame_rate ? (SCYCLES_PER_LINE * (vdp_pal ? 313 : 262) * snd.frame_rate) : SCD_CLOCK;

  /* PCM chips is running at original rate and is synchronized with SUB-CPU  */
  /* Chip output is resampled to desired rate using Blip Buffer. */
  blip[0] = left;
  blip[1] = right;
  blip_set_rates(left, mclk / PCM_SCYCLES_RATIO, snd.sample_rate);
  blip_set_rates(right, mclk / PCM_SCYCLES_RATIO, snd.sample_rate);
}

void pcm_reset(void)
{
  /* reset chip & clear external RAM */
  memset(&pcm, 0, sizeof(pcm_t));

  /* reset default bank */
  pcm.bank = pcm.ram;

  /* reset channels stereo panning */
  pcm.chan[0].pan = 0xff;
  pcm.chan[1].pan = 0xff;
  pcm.chan[2].pan = 0xff;
  pcm.chan[3].pan = 0xff;
  pcm.chan[4].pan = 0xff;
  pcm.chan[5].pan = 0xff;
  pcm.chan[6].pan = 0xff;
  pcm.chan[7].pan = 0xff;

  /* reset master clocks counter */
  pcm.cycles = 0;

  /* clear blip buffers */
  blip_clear(blip[0]);
  blip_clear(blip[1]);
}

int pcm_context_save(uint8 *state)
{
  uint8 tmp8;
  int bufferptr = 0;

  tmp8 = (pcm.bank - pcm.ram) >> 12;

  save_param(pcm.chan, sizeof(pcm.chan));
  save_param(pcm.out, sizeof(pcm.out));
  save_param(&tmp8, 1);
  save_param(&pcm.enabled, sizeof(pcm.enabled));
  save_param(&pcm.status, sizeof(pcm.status));
  save_param(&pcm.index, sizeof(pcm.index));
  save_param(pcm.ram, sizeof(pcm.ram));

  return bufferptr;
}

int pcm_context_load(uint8 *state)
{
  uint8 tmp8;
  int bufferptr = 0;

  load_param(pcm.chan, sizeof(pcm.chan));
  load_param(pcm.out, sizeof(pcm.out));

  load_param(&tmp8, 1);
  pcm.bank = &pcm.ram[(tmp8 & 0x0f) << 12];

  load_param(&pcm.enabled, sizeof(pcm.enabled));
  load_param(&pcm.status, sizeof(pcm.status));
  load_param(&pcm.index, sizeof(pcm.index));
  load_param(pcm.ram, sizeof(pcm.ram));

  return bufferptr;
}

void pcm_run(unsigned int length)
{
#ifdef LOG_PCM
  error("[%d][%d]run %d PCM samples (from %d)\n", v_counter, s68k.cycles, length, pcm.cycles);
#endif
  /* check if PCM chip is running */
  if (pcm.enabled)
  {
    int i, j, l, r;
  
    /* generate PCM samples */
    for (i=0; i<length; i++)
    {
      /* clear output */
      l = r = 0;

      /* run eight PCM channels */
      for (j=0; j<8; j++)
      {
        /* check if channel is enabled */
        if (pcm.status & (1 << j))
        {
          /* read from current WAVE RAM address */
          short data = pcm.ram[(pcm.chan[j].addr >> 11) & 0xffff];

          /* loop data ? */
          if (data == 0xff)
          {
            /* reset WAVE RAM address */
            pcm.chan[j].addr = pcm.chan[j].ls.w << 11;

            /* read again from WAVE RAM address */
            data = pcm.ram[pcm.chan[j].ls.w];
          }
          else
          {
            /* increment WAVE RAM address */
            pcm.chan[j].addr += pcm.chan[j].fd.w;
          }

          /* infinite loop should not output any data */
          if (data != 0xff)
          {
            /* check sign bit (output centered around 0) */
            if (data & 0x80)
            {
              /* PCM data is positive */
              data = data & 0x7f;
            }
            else
            {
              /* PCM data is negative */
              data = -(data & 0x7f);
            }

            /* multiply PCM data with ENV & stereo PAN data then add to L/R outputs (14.5 fixed point) */
            l += ((data * pcm.chan[j].env * (pcm.chan[j].pan & 0x0F)) >> 5);
            r += ((data * pcm.chan[j].env * (pcm.chan[j].pan >> 4)) >> 5);
          }
        }
      }

      /* limiter */
      if (l < -32768) l = -32768;
      else if (l > 32767) l = 32767;
      if (r < -32768) r = -32768;
      else if (r > 32767) r = 32767;

      /* check if PCM left output changed */
      if (pcm.out[0] != l)
      {
        blip_add_delta_fast(blip[0], i, l-pcm.out[0]);
        pcm.out[0] = l;
      }

      /* check if PCM right output changed */
      if (pcm.out[1] != r)
      {
        blip_add_delta_fast(blip[1], i, r-pcm.out[1]);
        pcm.out[1] = r;
      }
    }
  }
  else
  {
    /* check if PCM left output changed */
    if (pcm.out[0])
    {
      blip_add_delta_fast(blip[0], 0, -pcm.out[0]);
      pcm.out[0] = 0;
    }

    /* check if PCM right output changed */
    if (pcm.out[1])
    {
      blip_add_delta_fast(blip[1], 0, -pcm.out[1]);
      pcm.out[1] = 0;
    }
  }

  /* end of blip buffer frame */
  blip_end_frame(blip[0], length);
  blip_end_frame(blip[1], length);

  /* update PCM master clock counter */
  pcm.cycles += length * PCM_SCYCLES_RATIO;
}

void pcm_update(unsigned int samples)
{
  /* get number of internal clocks (samples) needed */
  unsigned int clocks = blip_clocks_needed(blip[0], samples);

  /* run PCM chip */
  if (clocks > 0)
  {
    pcm_run(clocks);
  }

  /* reset PCM master clocks counter */
  pcm.cycles = 0;
}

void pcm_write(unsigned int address, unsigned char data)
{
  /* synchronize PCM chip with SUB-CPU */
  int clocks = s68k.cycles - pcm.cycles;
  if (clocks > 0)
  {
    /* number of internal clocks (samples) to run */
    clocks = (clocks + PCM_SCYCLES_RATIO - 1) / PCM_SCYCLES_RATIO;
    pcm_run(clocks);
  }

#ifdef LOG_PCM
  error("[%d][%d]PCM write %x -> 0x%02x (%X)\n", v_counter, s68k.cycles, address, data, s68k.pc);
#endif

  /* external RAM is mapped to $1000-$1FFF */
  if (address >= 0x1000)
  {
    /* 4K bank access */
    pcm.bank[address & 0xfff] = data;
    return;
  }

  /* internal area si mapped to $0000-$0FFF */
  switch (address)
  {
    case 0x00: /* ENV register */
    {
      /* update channel ENV multiplier */
      pcm.chan[pcm.index].env = data;
      return;
    }

    case 0x01: /* PAN register */
    {
      /* update channel stereo panning value */
      pcm.chan[pcm.index].pan = data;
      return;
    }

    case 0x02: /* FD register (LSB) */
    {
      /* update channel WAVE RAM address increment LSB */
      pcm.chan[pcm.index].fd.byte.l = data;
      return;
    }

    case 0x03: /* FD register (MSB) */
    {
      /* update channel WAVE RAM address increment MSB */
      pcm.chan[pcm.index].fd.byte.h = data;
      return;
    }

    case 0x04: /* LS register (LSB) */
    {
      /* update channel WAVE RAM loop address LSB */
      pcm.chan[pcm.index].ls.byte.l = data;
      return;
    }

    case 0x05: /* LS register (MSB) */
    {
      /* update channel WAVE RAM loop address MSB */
      pcm.chan[pcm.index].ls.byte.h = data;
      return;
    }

    case 0x06: /* ST register */
    {
      /* update channel WAVE RAM start address (16.11 fixed point) */
      pcm.chan[pcm.index].st = data << (8 + 11);

      /* reload WAVE RAM address if channel is OFF */
      if (!(pcm.status & (1 << pcm.index)))
      {
        pcm.chan[pcm.index].addr = pcm.chan[pcm.index].st;
      }
      return;
    }

    case 0x07: /* CTRL register */
    {
      if (data & 0x40)
      {
        /* channel selection (0-7) */
        pcm.index = data & 0x07;
      }
      else
      {
        /* external RAM bank selection (16 x 4K) */
        pcm.bank = &pcm.ram[(data & 0x0f) << 12];
      }

      /* update PCM chip status (bit 7) */
      pcm.enabled = data & 0x80;
      return;
    }

    case 0x08: /* ON/OFF register */
    {
      /* update PCM channels status */
      pcm.status = ~data;

      /* reload WAVE RAM address pointers when channels are OFF */
      if (data & 0x01) pcm.chan[0].addr = pcm.chan[0].st;
      if (data & 0x02) pcm.chan[1].addr = pcm.chan[1].st;
      if (data & 0x04) pcm.chan[2].addr = pcm.chan[2].st;
      if (data & 0x08) pcm.chan[3].addr = pcm.chan[3].st;
      if (data & 0x10) pcm.chan[4].addr = pcm.chan[4].st;
      if (data & 0x20) pcm.chan[5].addr = pcm.chan[5].st;
      if (data & 0x40) pcm.chan[6].addr = pcm.chan[6].st;
      if (data & 0x80) pcm.chan[7].addr = pcm.chan[7].st;
      return;
    }

    default:
    {
      /* illegal access */
      return;
    }
  }
}

unsigned char pcm_read(unsigned int address)
{
  /* synchronize PCM chip with SUB-CPU */
  int clocks = s68k.cycles - pcm.cycles;
  if (clocks > 0)
  {
    /* number of internal clocks (samples) to run */
    clocks = (clocks + PCM_SCYCLES_RATIO - 1) / PCM_SCYCLES_RATIO;
    pcm_run(clocks);
  }

#ifdef LOG_PCM
  error("[%d][%d]PCM read (%X)\n", v_counter, s68k.cycles, address, s68k.pc);
#endif

  /* external RAM (TODO: verify if possible to read, some docs claim it's not !) */
  if (address >= 0x1000)
  {
    /* 4K bank access */
    return pcm.bank[address & 0xfff];
  }

  /* read WAVE RAM address pointers */
  if ((address >= 0x10) && (address < 0x20))
  {
    int index = (address >> 1) & 0x07;

    if (address & 1)
    {
      return (pcm.chan[index].addr >> (11 + 8)) & 0xff;
    }
    else
    {
      return (pcm.chan[index].addr >> 11) & 0xff;
    }
  }

  /* illegal access */
  return 0xff;
}

void pcm_ram_dma_w(unsigned int words)
{
  uint16 data;

  /* CDC buffer source address */
  uint16 src_index = cdc.dac.w & 0x3ffe;
  
  /* PCM-RAM destination address*/
  uint16 dst_index = (scd.regs[0x0a>>1].w << 2) & 0xffe;
  
  /* update DMA destination address */
  scd.regs[0x0a>>1].w += (words >> 1);

  /* update DMA source address */
  cdc.dac.w += (words << 1);

  /* DMA transfer */
  while (words--)
  {
    /* read 16-bit word from CDC buffer */
    data = *(uint16 *)(cdc.ram + src_index);

    /* write 16-bit word to PCM RAM (endianness does not matter since PCM RAM is always accessed as byte)*/
    *(uint16 *)(pcm.bank + dst_index) = data ;

    /* increment CDC buffer source address */
    src_index = (src_index + 2) & 0x3ffe;

    /* increment PCM-RAM destination address */
    dst_index = (dst_index + 2) & 0xffe;
  }
}

