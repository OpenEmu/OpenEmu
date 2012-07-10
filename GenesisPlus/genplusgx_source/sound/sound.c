/***************************************************************************************
 *  Genesis Plus
 *  Sound Hardware
 *
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald (original code)
 *  Copyright (C) 2007-2012  Eke-Eke (Genesis Plus GX)
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
#include "Fir_Resampler.h"

/* Cycle-accurate samples */
static unsigned int psg_cycles_ratio;
static unsigned int psg_cycles_count;
static unsigned int fm_cycles_ratio;
static unsigned int fm_cycles_count;

/* YM chip function pointers */
static void (*YM_Reset)(void);
static void (*YM_Update)(int *buffer, int length);
static void (*YM_Write)(unsigned int a, unsigned int v);

/* Run FM chip for required M-cycles */
INLINE void fm_update(unsigned int cycles)
{
  if (cycles > fm_cycles_count)
  {
    int32 *buffer;

    /* samples to run */
    unsigned int samples = (cycles - fm_cycles_count + fm_cycles_ratio - 1) / fm_cycles_ratio;

    /* update cycle count */
    fm_cycles_count += samples * fm_cycles_ratio;

    /* select input sample buffer */
    buffer = Fir_Resampler_buffer();
    if (buffer)
    {
      Fir_Resampler_write(samples << 1);
    }
    else
    {
      buffer = snd.fm.pos;
      snd.fm.pos += (samples << 1);
    }

    /* run FM chip & get samples */
    YM_Update(buffer, samples);
  }
}

/* Run PSG chip for required M-cycles */
INLINE void psg_update(unsigned int cycles)
{
  if (cycles > psg_cycles_count)
  {
    /* clocks to run */
    unsigned int clocks = (cycles - psg_cycles_count + psg_cycles_ratio - 1) / psg_cycles_ratio;

    /* update cycle count */
    psg_cycles_count += clocks * psg_cycles_ratio;

    /* run PSG chip & get samples */
    snd.psg.pos += SN76489_Update(snd.psg.pos, clocks);
  }
}

/* Initialize sound chips emulation */
void sound_init(void)
{
  /* Number of M-cycles executed per second.                                              */
  /*                                                                                      */
  /* All emulated chips are kept in sync by using a common oscillator (MCLOCK)            */
  /*                                                                                      */
  /* The original console would run exactly 53693175 M-cycles (53203424 for PAL), with    */
  /* 3420 M-cycles per line and 262 (313 for PAL) lines per frame, which gives an exact   */
  /* framerate of 59.92 (49.70 for PAL) fps.                                              */
  /*                                                                                      */
  /* Since audio samples are generated at the end of the frame, to prevent audio skipping */
  /* or lag between emulated frames, number of samples rendered per frame must be set to  */
  /* output samplerate (number of samples played per second) divided by output framerate  */
  /* (number of frames emulated per seconds).                                             */
  /*                                                                                      */
  /* On some systems, we may want to achieve 100% smooth video rendering by synchronizing */
  /* frame emulation with VSYNC, which frequency is generally not exactly those values.   */
  /* In that case, number of frames emulated per seconds is the same as the number of     */
  /* frames rendered per seconds by the host system video hardware.                       */
  /*                                                                                      */
  /* When no framerate is specified, base clock is original master clock value.           */
  /* Otherwise, it is based on the output framerate.                                      */
  /*                                                                                      */
  double mclk = snd.frame_rate ? (MCYCLES_PER_LINE * lines_per_frame * snd.frame_rate) : system_clock;

  /* For maximal accuracy, sound chips run in synchronization with 68k and Z80 cpus       */
  /* These values give the exact number of M-cycles executed per internal sample clock:   */
  /* . PSG chip runs at original rate and audio is resampled internally after each update */
  /* . FM chips run by default (if HQ mode disabled) at the output rate directly          */
  /* We use 21.11 fixed point precision (max. mcycle value is 3420*313 i.e 21 bits max)   */
  psg_cycles_ratio  = 16 * 15 * (1 << 11);
  fm_cycles_ratio   = (unsigned int)(mclk / (double) snd.sample_rate * 2048.0);
  psg_cycles_count  = 0;
  fm_cycles_count   = 0;

  /* Initialize PSG core (input clock should be based on emulated system clock) */
  SN76489_Init(mclk/15.0,snd.sample_rate);

  /* Initialize FM cores (input clock and samplerate are only used when HQ mode is disabled) */
  if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
  {
    /* YM2612 */
    YM2612Init(mclk/7.0,snd.sample_rate);
    YM_Reset = YM2612ResetChip;
    YM_Update = YM2612Update;
    YM_Write = YM2612Write;

    /* In HQ mode, YM2612 is running at original rate (one sample each 144*7 M-cycles) */
    /* Audio is resampled externally at the end of a frame. */
    if (config.hq_fm)
    {
      fm_cycles_ratio = 144 * 7 * (1 << 11);
      Fir_Resampler_time_ratio(mclk / (double)snd.sample_rate / (144.0 * 7.0), config.rolloff);
    }
  }
  else
  {
    /* YM2413 */
    YM2413Init(mclk/15.0,snd.sample_rate);
    YM_Reset = YM2413ResetChip;
    YM_Update = YM2413Update;
    YM_Write = YM2413Write;

    /* In HQ mode, YM2413 is running at original rate (one sample each 72*15 M-cycles)  */
    /* Audio is resampled externally at the end of a frame. */
    if (config.hq_fm)
    {
      fm_cycles_ratio = 72 * 15 * (1 << 11);
      Fir_Resampler_time_ratio(mclk / (double)snd.sample_rate / (72.0 * 15.0), config.rolloff);
    }
  }

#ifdef LOGSOUND
  error("%f mcycles per second\n", mclk);
  error("%d mcycles per PSG sample\n", psg_cycles_ratio);
  error("%d mcycles per FM sample\n", fm_cycles_ratio);
#endif
}


void sound_shutdown(void)
{
  Fir_Resampler_shutdown();
  SN76489_Shutdown();
}
 
/* Reset sound chips emulation */
void sound_reset(void)
{
  YM_Reset();
  SN76489_Reset();
  fm_cycles_count = 0;
  psg_cycles_count = 0;
}

void sound_restore()
{
  int size;
  uint8 *ptr, *temp;

  /* save YM context */
  if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
  {
    size = YM2612GetContextSize();
    ptr = YM2612GetContextPtr();
  }
  else
  {
    size = YM2413GetContextSize();
    ptr = YM2413GetContextPtr();
  }
  temp = malloc(size);
  if (temp)
  {
    memcpy(temp, ptr, size);
  }

  /* reinitialize sound chips */
  sound_init();

  /* restore YM context */
  if (temp)
  {
    if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
    {
      YM2612Restore(temp);
    }
    else
    {
      YM2413Restore(temp);
    }
    free(temp);
  }
}

int sound_context_save(uint8 *state)
{
  int bufferptr = 0;
  
  if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
  {
    bufferptr = YM2612SaveContext(state);
  }
  else
  {
    save_param(YM2413GetContextPtr(),YM2413GetContextSize());
  }

  save_param(SN76489_GetContextPtr(),SN76489_GetContextSize());
  save_param(&fm_cycles_count,sizeof(fm_cycles_count));
  save_param(&psg_cycles_count,sizeof(psg_cycles_count));

  return bufferptr;
}

int sound_context_load(uint8 *state)
{
  int bufferptr = 0;

  if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
  {
    bufferptr = YM2612LoadContext(state);
  }
  else
  {
    load_param(YM2413GetContextPtr(),YM2413GetContextSize());
  }

  load_param(SN76489_GetContextPtr(),SN76489_GetContextSize());

  load_param(&fm_cycles_count,sizeof(fm_cycles_count));
  load_param(&psg_cycles_count,sizeof(psg_cycles_count));
  fm_cycles_count = psg_cycles_count;

  return bufferptr;
}

/* End of frame update, return the number of samples run so far.  */
int sound_update(unsigned int cycles)
{
  int size, avail;

  /* run PSG & FM chips until end of frame */
  cycles <<= 11;
  psg_update(cycles);
  fm_update(cycles);

  /* check number of available FM samples */
  if (config.hq_fm)
  {
    size = Fir_Resampler_avail();
  }
  else
  {
    size = (snd.fm.pos - snd.fm.buffer) >> 1;
  }
#ifdef LOGSOUND
  error("%d FM samples available\n",size);
#endif

  /* check number of available PSG samples */
  avail = snd.psg.pos - snd.psg.buffer;
#ifdef LOGSOUND
  error("%d PSG samples available\n", avail);
#endif

  /* resynchronize FM & PSG chips */
  if (size > avail)
  {
    /* FM chip is ahead */
    fm_cycles_count += SN76489_Clocks(size - avail) * psg_cycles_ratio;

    /* return number of available samples */
    size = avail;
  }
  else
  {
    /* PSG chip is ahead */
    psg_cycles_count += SN76489_Clocks(avail - size) * psg_cycles_ratio;
  }

#ifdef LOGSOUND
  error("%lu PSG cycles run\n",psg_cycles_count);
  error("%lu FM cycles run \n",fm_cycles_count);
#endif

  /* adjust PSG & FM cycle counts for next frame */
  psg_cycles_count -= cycles;
  fm_cycles_count  -= cycles;

#ifdef LOGSOUND
  error("%lu PSG cycles left\n",psg_cycles_count);
  error("%lu FM cycles left\n",fm_cycles_count);
#endif

  return size;
}

/* Reset FM chip */
void fm_reset(unsigned int cycles)
{
  fm_update(cycles << 11);
  YM_Reset();
}

/* Write FM chip */
void fm_write(unsigned int cycles, unsigned int address, unsigned int data)
{
  if (address & 1) fm_update(cycles << 11);
  YM_Write(address, data);
}

/* Read FM status (YM2612 only) */
unsigned int fm_read(unsigned int cycles, unsigned int address)
{
  fm_update(cycles << 11);
  return YM2612Read();
}

/* Write PSG chip */
void psg_write(unsigned int cycles, unsigned int data)
{
  psg_update(cycles << 11);
  SN76489_Write(data);
}
