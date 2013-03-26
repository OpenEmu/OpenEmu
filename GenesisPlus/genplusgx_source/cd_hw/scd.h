/***************************************************************************************
 *  Genesis Plus
 *  Mega CD / Sega CD hardware
 *
 *  Copyright (C) 2012-2013  Eke-Eke (Genesis Plus GX)
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
#ifndef _HW_SCD_
#define _HW_SCD_

#include "cdd.h"
#include "cdc.h"
#include "gfx.h"
#include "pcm.h"
#include "cd_cart.h"

#define scd ext.cd_hw

/* 5000000 SCD clocks/s = ~3184 clocks/line with a Master Clock of 53.693175 MHz */
/* This would be slightly (~30 clocks) more on PAL systems because of the slower */
/* Master Clock (53.203424 MHz) but not enough to really care about since clocks */
/* are not running in sync anyway. */
#define SCD_CLOCK 50000000
#define SCYCLES_PER_LINE 3184 

/* Timer & Stopwatch clocks divider */
#define TIMERS_SCYCLES_RATIO (384 * 4)

/* CD hardware */
typedef struct 
{
  cd_cart_t cartridge;        /* ROM/RAM Cartridge */
  uint8 bootrom[0x20000];     /* 128K internal BOOT ROM */
  uint8 prg_ram[0x80000];     /* 512K PRG-RAM */
  uint8 word_ram[2][0x20000]; /* 2 x 128K Word RAM (1M mode) */
  uint8 word_ram_2M[0x40000]; /* 256K Word RAM (2M mode) */
  uint8 bram[0x2000];         /* 8K Backup RAM */
  reg16_t regs[0x100];        /* 256 x 16-bit ASIC registers */
  uint32 cycles;              /* Master clock counter */
  int32 stopwatch;            /* Stopwatch counter */
  int32 timer;                /* Timer counter */
  uint8 pending;              /* Pending interrupts */
  uint8 dmna;                 /* Pending DMNA write status */
  gfx_t gfx_hw;               /* Graphics processor */
  cdc_t cdc_hw;               /* CD data controller */
  cdd_t cdd_hw;               /* CD drive processor */
  pcm_t pcm_hw;               /* PCM chip */
} cd_hw_t;

/* Function prototypes */
extern void scd_init(void);
extern void scd_reset(int hard);
extern void scd_update(unsigned int cycles);
extern void scd_end_frame(unsigned int cycles);
extern int scd_context_load(uint8 *state);
extern int scd_context_save(uint8 *state);
extern int scd_68k_irq_ack(int level);
extern void prg_ram_dma_w(unsigned int words);

#endif
