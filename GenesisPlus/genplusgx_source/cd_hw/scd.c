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

#include "shared.h"

/*--------------------------------------------------------------------------*/
/* Unused area (return open bus data, i.e prefetched instruction word)      */
/*--------------------------------------------------------------------------*/
static unsigned int s68k_read_bus_8(unsigned int address)
{
#ifdef LOGERROR
  error("[SUB 68k] Unused read8 %08X (%08X)\n", address, s68k.pc);
#endif
  address = s68k.pc | (address & 1);
  return READ_BYTE(s68k.memory_map[((address)>>16)&0xff].base, (address) & 0xffff);
}

static unsigned int s68k_read_bus_16(unsigned int address)
{
#ifdef LOGERROR
  error("[SUB 68k] Unused read16 %08X (%08X)\n", address, s68k.pc);
#endif
  address = s68k.pc;
  return *(uint16 *)(s68k.memory_map[((address)>>16)&0xff].base + ((address) & 0xffff));
}

static void s68k_unused_8_w(unsigned int address, unsigned int data)
{
#ifdef LOGERROR
  error("[SUB 68k] Unused write8 %08X = %02X (%08X)\n", address, data, s68k.pc);
#endif
}

static void s68k_unused_16_w(unsigned int address, unsigned int data)
{
#ifdef LOGERROR
  error("[SUB 68k] Unused write16 %08X = %04X (%08X)\n", address, data, s68k.pc);
#endif
}

/*--------------------------------------------------------------------------*/
/* PRG-RAM DMA access                                                       */
/*--------------------------------------------------------------------------*/
void prg_ram_dma_w(unsigned int words)
{
  uint16 data;

  /* CDC buffer source address */
  uint16 src_index = cdc.dac.w & 0x3ffe;
  
  /* PRG-RAM destination address*/
  uint32 dst_index = (scd.regs[0x0a>>1].w << 3) & 0x7fffe;
  
  /* update DMA destination address */
  scd.regs[0x0a>>1].w += (words >> 2);

  /* update DMA source address */
  cdc.dac.w += (words << 1);

  /* check PRG-RAM write protected area */
  if (dst_index < (scd.regs[0x02>>1].byte.h << 9))
  {
    return;
  }

  /* DMA transfer */
  while (words--)
  {
    /* read 16-bit word from CDC buffer */
    data = *(uint16 *)(cdc.ram + src_index);

#ifdef LSB_FIRST
    /* source data is stored in big endian format */
    data = ((data >> 8) | (data << 8)) & 0xffff;
#endif

    /* write 16-bit word to PRG-RAM */
    *(uint16 *)(scd.prg_ram + dst_index) = data ;

    /* increment CDC buffer source address */
    src_index = (src_index + 2) & 0x3ffe;

    /* increment PRG-RAM destination address */
    dst_index = (dst_index + 2) & 0x7fffe;
  }
}

/*--------------------------------------------------------------------------*/
/* PRG-RAM write protected area                                             */
/*--------------------------------------------------------------------------*/
static void prg_ram_write_byte(unsigned int address, unsigned int data)
{
  address &= 0x7ffff;
  if (address >= (scd.regs[0x02>>1].byte.h << 9))
  {
    WRITE_BYTE(scd.prg_ram, address, data);
    return;
  }
#ifdef LOGERROR
  error("[SUB 68k] PRG-RAM protected write8 %08X = %02X (%08X)\n", address, data, s68k.pc);
#endif
}

static void prg_ram_write_word(unsigned int address, unsigned int data)
{
  address &= 0x7fffe;
  if (address >= (scd.regs[0x02>>1].byte.h << 9))
  {
    *(uint16 *)(scd.prg_ram + address) = data;
    return;
  }
#ifdef LOGERROR
  error("[SUB 68k] PRG-RAM protected write16 %08X = %02X (%08X)\n", address, data, s68k.pc);
#endif
}

/*--------------------------------------------------------------------------*/
/* internal backup RAM (8KB)                                                */
/*--------------------------------------------------------------------------*/
static unsigned int bram_read_byte(unsigned int address)
{
  /* LSB only */
  if (address & 1)
  {
    return scd.bram[(address >> 1) & 0x1fff];
  }

  return 0xff;
}

static unsigned int bram_read_word(unsigned int address)
{
  return (scd.bram[(address >> 1) & 0x1fff] | 0xff00);
}

static void bram_write_byte(unsigned int address, unsigned int data)
{
  /* LSB only */
  if (address & 1)
  {
    scd.bram[(address >> 1) & 0x1fff] = data;
  }
}

static void bram_write_word(unsigned int address, unsigned int data)
{
  scd.bram[(address >> 1) & 0x1fff] = data & 0xff;
}

/*--------------------------------------------------------------------------*/
/* PCM chip & Gate-Array area                                               */
/*--------------------------------------------------------------------------*/

static void s68k_poll_detect(reg)
{
  /* detect SUB-CPU register polling */
  if (s68k.poll.detected == (1 << reg))
  {
    if (s68k.cycles <= s68k.poll.cycle)
    {
      if (s68k.pc == s68k.poll.pc)
      {
        /* stop SUB-CPU until register is modified by MAIN-CPU */
#ifdef LOG_SCD
        error("s68k stopped from %d cycles\n", s68k.cycles);
#endif
        s68k.cycles = s68k.cycle_end;
        s68k.stopped = 1 << reg;
      }
      return;
    }
  }
  else
  {
    /* set SUB-CPU register access flag */
    s68k.poll.detected = 1 << reg;
  }

  /* restart SUB-CPU polling detection */
  s68k.poll.cycle = s68k.cycles + 320;
  s68k.poll.pc = s68k.pc;
}

static void s68k_poll_sync(reg)
{
  /* relative MAIN-CPU cycle counter */
  unsigned int cycles = (s68k.cycles * MCYCLES_PER_LINE) / SCYCLES_PER_LINE;

  /* sync MAIN-CPU with SUB-CPU */
  if (!m68k.stopped && (m68k.cycles < cycles))
  {
    m68k_run(cycles);
  }

  /* MAIN-CPU stopped on register polling ? */
  if (m68k.stopped & (3 << reg))
  {
    /* sync MAIN-CPU with SUB-CPU */
    m68k.cycles = cycles;

    /* restart MAIN-CPU */
    m68k.stopped = 0;
#ifdef LOG_SCD
    error("m68k started from %d cycles\n", cycles);
#endif
  }

  /* clear CPU register(s) access flags */
  m68k.poll.detected &= ~(3 << reg);
  s68k.poll.detected &= ~(3 << reg);
}

static unsigned int scd_read_byte(unsigned int address)
{
  /* PCM area (8K) is mirrored into $FF0000-$FF7FFF */
  if (address < 0xff8000)
  {
    /* get /LDS only */
    if (address & 1)
    {
      return pcm_read((address >> 1) & 0x1fff);
    }

    return s68k_read_bus_8(address);
  }

#ifdef LOG_SCD
  error("[%d][%d]read byte CD register %X (%X)\n", v_counter, s68k.cycles, address, s68k.pc);
#endif

  /* Memory Mode */
  if (address == 0xff8003)
  {
    s68k_poll_detect(0x03);
    return scd.regs[0x03>>1].byte.l;
  }

  /* MAIN-CPU communication flags */
  if (address == 0xff800e)
  {
    s68k_poll_detect(0x0e);
    return scd.regs[0x0e>>1].byte.h;
  }

  /* CDC register data (controlled by BIOS, byte access only ?) */
  if (address == 0xff8007)
  {
    unsigned int data = cdc_reg_r();
#ifdef LOG_CDC
    error("CDC register %X read 0x%02X (%X)\n", scd.regs[0x04>>1].byte.l & 0x0F, data, s68k.pc);
#endif
    return data;
  }
  
  /* LED status */
  if (address == 0xff8000)
  {
    /* register $00 is reserved for MAIN-CPU, we use $06 instead */
    return scd.regs[0x06>>1].byte.h;
  }

  /* RESET status */
  if (address == 0xff8001)
  {
    /* always return 1 */
    return 0x01;
  }

  /* Font data */
  if ((address >= 0xff8050) && (address <= 0xff8056))
  {
    /* shifted 4-bit input (xxxx00) */
    uint8 bits = (scd.regs[0x4e>>1].w >> (((address & 6) ^ 6) << 1)) << 2;
    
    /* color code */
    uint8 code = scd.regs[0x4c>>1].byte.l;
    
    /* 16-bit font data (4 pixels = 16 bits) */
    uint16 data = (code >> (bits & 4)) & 0x0f;

    bits = bits >> 1;
    data = data | (((code >> (bits & 4)) << 4) & 0xf0);

    bits = bits >> 1;
    data = data | (((code >> (bits & 4)) << 8) & 0xf00);

    bits = bits >> 1;
    data = data | (((code >> (bits & 4)) << 12) & 0xf000);

    return (address & 1) ? (data & 0xff) : (data >> 8);
  }

  /* MAIN-CPU communication words */
  if ((address & 0x1f0) == 0x10)
  {
    s68k_poll_detect(address & 0x1f);
  }

  /* default registers */
  if (address & 1)
  {
    /* register LSB */
    return scd.regs[(address >> 1) & 0xff].byte.l;
  }

  /* register MSB */
  return scd.regs[(address >> 1) & 0xff].byte.h;
}

static unsigned int scd_read_word(unsigned int address)
{
  /* PCM area (8K) is mirrored into $FF0000-$FF7FFF */
  if (address < 0xff8000)
  {
    /* get /LDS only */
    return pcm_read((address >> 1) & 0x1fff);
  }

#ifdef LOG_SCD
  error("[%d][%d]read word CD register %X (%X)\n", v_counter, s68k.cycles, address, s68k.pc);
#endif

  /* Memory Mode */
  if (address == 0xff8002)
  {
    s68k_poll_detect(0x03);
    return scd.regs[0x03>>1].w;
  }

  /* CDC host data (word access only ?) */
  if (address == 0xff8008)
  {
    return cdc_host_r();
  }

  /* LED & RESET status */
  if (address == 0xff8000)
  {
    /* register $00 is reserved for MAIN-CPU, we use $06 instead */
    return scd.regs[0x06>>1].w;
  }

  /* Stopwatch counter (word access only ?) */
  if (address == 0xff800c)
  {
    /* cycle-accurate counter value */
    return (scd.regs[0x0c>>1].w + ((s68k.cycles - scd.stopwatch) / TIMERS_SCYCLES_RATIO)) & 0xfff;
  }

  /* Font data */
  if ((address >= 0xff8050) && (address <= 0xff8056))
  {
    /* shifted 4-bit input (xxxx00) */
    uint8 bits = (scd.regs[0x4e>>1].w >> (((address & 6) ^ 6) << 1)) << 2;
    
    /* color code */
    uint8 code = scd.regs[0x4c>>1].byte.l;
    
    /* 16-bit font data (4 pixels = 16 bits) */
    uint16 data = (code >> (bits & 4)) & 0x0f;

    bits = bits >> 1;
    data = data | (((code >> (bits & 4)) << 4) & 0xf0);

    bits = bits >> 1;
    data = data | (((code >> (bits & 4)) << 8) & 0xf00);

    bits = bits >> 1;
    data = data | (((code >> (bits & 4)) << 12) & 0xf000);

    return data;
  }

  /* MAIN-CPU communication words */
  if ((address & 0x1f0) == 0x10)
  {
    /* relative MAIN-CPU cycle counter */
    unsigned int cycles = (s68k.cycles * MCYCLES_PER_LINE) / SCYCLES_PER_LINE;

    /* sync MAIN-CPU with SUB-CPU */
    if (!m68k.stopped && (m68k.cycles < cycles))
    {
      m68k_run(cycles);
    }

    s68k_poll_detect(address & 0x1e);
  }

  /* default registers */
  return scd.regs[(address >> 1) & 0xff].w;
}

INLINE void word_ram_switch(uint8 mode)
{
  int i;
  uint16 *ptr1 = (uint16 *)(scd.word_ram_2M);
  uint16 *ptr2 = (uint16 *)(scd.word_ram[0]);
  uint16 *ptr3 = (uint16 *)(scd.word_ram[1]);

  if (mode & 0x04)
  {
    /* 2M -> 1M mode */
    for (i=0; i<0x10000; i++)
    {
      *ptr2++=*ptr1++;
      *ptr3++=*ptr1++;
    }
  }
  else
  {
    /* 1M -> 2M mode */
    for (i=0; i<0x10000; i++)
    {
      *ptr1++=*ptr2++;
      *ptr1++=*ptr3++;
    }

    /* allow Word-RAM access from both CPU in 2M mode (fixes sync issues in Mortal Kombat) */
    for (i=scd.cartridge.boot+0x20; i<scd.cartridge.boot+0x24; i++)
    {
      /* MAIN-CPU: $200000-$23FFFF is mapped to 256K Word-RAM */
      m68k.memory_map[i].base    = scd.word_ram_2M + ((i & 0x03) << 16);
      m68k.memory_map[i].read8   = NULL;
      m68k.memory_map[i].read16  = NULL;
      m68k.memory_map[i].write8  = NULL;
      m68k.memory_map[i].write16 = NULL;
      zbank_memory_map[i].read   = NULL;
      zbank_memory_map[i].write  = NULL;
    }

    for (i=0x08; i<0x0c; i++)
    {
      /* SUB-CPU: $080000-$0BFFFF is mapped to 256K Word-RAM */
      s68k.memory_map[i].read8   = NULL;
      s68k.memory_map[i].read16  = NULL;
      s68k.memory_map[i].write8  = NULL;
      s68k.memory_map[i].write16 = NULL;
    }

    for (i=0x0c; i<0x0e; i++)
    {
      /* SUB-CPU: $0C0000-$0DFFFF is unmapped */
      s68k.memory_map[i].read8   = s68k_read_bus_8;
      s68k.memory_map[i].read16  = s68k_read_bus_16;
      s68k.memory_map[i].write8  = s68k_unused_8_w;
      s68k.memory_map[i].write16 = s68k_unused_16_w;
    }
  }
}

static void scd_write_byte(unsigned int address, unsigned int data)
{
  /* PCM area (8K) is mirrored into $FF0000-$FF7FFF */
  if (address < 0xff8000)
  {
    /* get /LDS only */
    if (address & 1)
    {
      pcm_write((address >> 1) & 0x1fff, data);
      return;
    }

    s68k_unused_8_w(address, data);
    return;
  }

#ifdef LOG_SCD
  error("[%d][%d]write byte CD register %X -> 0x%02x (%X)\n", v_counter, s68k.cycles, address, data, s68k.pc);
#endif

  /* Gate-Array registers */
  switch (address & 0x1ff)
  {
    case 0x00: /* LED status */
    {
      /* register $00 is reserved for MAIN-CPU, use $06 instead */
      scd.regs[0x06 >> 1].byte.h = data;
      return;
    }

    case 0x01: /* RESET status */
    {
      /* RESET bit cleared ? */      
      if (!(data & 0x01))
      {
        /* reset CD hardware */
        scd_reset(0);
      }
      return;
    }

    case 0x03: /* Memory Mode */
    {
      s68k_poll_sync(0x02);

      /* detect MODE & RET bits modifications */
      if ((data ^ scd.regs[0x03 >> 1].byte.l) & 0x05)
      {
        int i;
        
        /* MODE bit */
        if (data & 0x04)
        {
          /* 2M->1M mode switch */
          if (!(scd.regs[0x03 >> 1].byte.l & 0x04))
          {
            /* re-arrange Word-RAM banks */
            word_ram_switch(0x04);
          }

          /* RET bit in 1M Mode */
          if (data & 0x01)
          {
            /* Word-RAM 1 assigned to MAIN-CPU */
            for (i=scd.cartridge.boot+0x20; i<scd.cartridge.boot+0x22; i++)
            {
              /* Word-RAM 1 data mapped at $200000-$21FFFF */
              m68k.memory_map[i].base = scd.word_ram[1] + ((i & 0x01) << 16);
            }

            for (i=scd.cartridge.boot+0x22; i<scd.cartridge.boot+0x24; i++)
            {
              /* VRAM cell image mapped at $220000-$23FFFF */
              m68k.memory_map[i].read8   = cell_ram_1_read8;
              m68k.memory_map[i].read16  = cell_ram_1_read16;
              m68k.memory_map[i].write8  = cell_ram_1_write8;
              m68k.memory_map[i].write16 = cell_ram_1_write16;
              zbank_memory_map[i].read   = cell_ram_1_read8;
              zbank_memory_map[i].write  = cell_ram_1_write8;
            }

            /* Word-RAM 0 assigned to SUB-CPU */
            for (i=0x08; i<0x0c; i++)
            {
              /* DOT image mapped at $080000-$0BFFFF */
              s68k.memory_map[i].read8   = dot_ram_0_read8;
              s68k.memory_map[i].read16  = dot_ram_0_read16;
              s68k.memory_map[i].write8  = dot_ram_0_write8;
              s68k.memory_map[i].write16 = dot_ram_0_write16;
            }

            for (i=0x0c; i<0x0e; i++)
            {
              /* Word-RAM 0 data mapped at $0C0000-$0DFFFF */
              s68k.memory_map[i].base    = scd.word_ram[0] + ((i & 0x01) << 16);
              s68k.memory_map[i].read8   = NULL;
              s68k.memory_map[i].read16  = NULL;
              s68k.memory_map[i].write8  = NULL;
              s68k.memory_map[i].write16 = NULL;
            }

            /* writing 1 to RET bit in 1M mode returns Word-RAM to MAIN-CPU in 2M mode */
            scd.dmna = 0;
          }
          else
          {
            /* Word-RAM 0 assigned to MAIN-CPU */
            for (i=scd.cartridge.boot+0x20; i<scd.cartridge.boot+0x22; i++)
            {
              /* Word-RAM 0 data mapped at $200000-$21FFFF */
              m68k.memory_map[i].base = scd.word_ram[0] + ((i & 0x01) << 16);
            }

            for (i=scd.cartridge.boot+0x22; i<scd.cartridge.boot+0x24; i++)
            {
              /* VRAM cell image mapped at $220000-$23FFFF */
              m68k.memory_map[i].read8   = cell_ram_0_read8;
              m68k.memory_map[i].read16  = cell_ram_0_read16;
              m68k.memory_map[i].write8  = cell_ram_0_write8;
              m68k.memory_map[i].write16 = cell_ram_0_write16;
              zbank_memory_map[i].read   = cell_ram_0_read8;
              zbank_memory_map[i].write  = cell_ram_0_write8;
            }

            /* Word-RAM 1 assigned to SUB-CPU */
            for (i=0x08; i<0x0c; i++)
            {
              /* DOT image mapped at $080000-$0BFFFF */
              s68k.memory_map[i].read8   = dot_ram_1_read8;
              s68k.memory_map[i].read16  = dot_ram_1_read16;
              s68k.memory_map[i].write8  = dot_ram_1_write8;
              s68k.memory_map[i].write16 = dot_ram_1_write16;
            }

            for (i=0x0c; i<0x0e; i++)
            {
              /* Word-RAM 1 data mapped at $0C0000-$0DFFFF */
              s68k.memory_map[i].base    = scd.word_ram[1] + ((i & 0x01) << 16);
              s68k.memory_map[i].read8   = NULL;
              s68k.memory_map[i].read16  = NULL;
              s68k.memory_map[i].write8  = NULL;
              s68k.memory_map[i].write16 = NULL;
            }
          }

          /* clear DMNA bit (swap completed) */
          scd.regs[0x02 >> 1].byte.l = (scd.regs[0x02 >> 1].byte.l & ~0x1f) | (data & 0x1d);
          return;
        }
        else
        {
          /* 1M->2M mode switch */
          if (scd.regs[0x02 >> 1].byte.l & 0x04)
          {
            /* re-arrange Word-RAM banks */
            word_ram_switch(0x00);

            /* RET bit set during 1M mode ? */
            data |= ~scd.dmna & 0x01;
            
            /* check if RET bit is cleared */
            if (!(data & 0x01))
            {
              /* set DMNA bit */
              data |= 0x02;

              /* mask BK0-1 bits (MAIN-CPU side only) */
              scd.regs[0x02 >> 1].byte.l = (scd.regs[0x02 >> 1].byte.l & ~0x1f) | (data & 0x1f);
              return;
            }
          }
          
          /* RET bit set in 2M mode */
          if (data & 0x01)
          {
            /* Word-RAM is returned to MAIN-CPU */
            scd.dmna = 0;

            /* clear DMNA bit */
            scd.regs[0x02 >> 1].byte.l = (scd.regs[0x02 >> 1].byte.l & ~0x1f) | (data & 0x1d);
            return;
          }
        }
      }

      /* update PM0-1 & MODE bits */
      scd.regs[0x02 >> 1].byte.l = (scd.regs[0x02 >> 1].byte.l & ~0x1c) | (data & 0x1c);
      return;
    }

    case 0x07: /* CDC register write */
    {
      cdc_reg_w(data);
      return;
    }

    case 0x0e: /* MAIN-CPU communication flags, normally read-only (Space Ace, Dragon's Lair) */
    {
      /* ROR8 operation */
      data = (data >> 1) | ((data << 7) & 1);
    }

    case 0x0f:  /* SUB-CPU communication flags */
    {
      s68k_poll_sync(0x0e);
      scd.regs[0x0f>>1].byte.l = data;
      return;
    }

    case 0x31: /* Timer */
    {
      /* reload timer (one timer clock = 384 CPU cycles) */
      scd.timer = data * TIMERS_SCYCLES_RATIO;

      /* only non-zero data starts timer, writing zero stops it */
      if (data)
      {
        /* adjust regarding current CPU cycle */
        scd.timer += (s68k.cycles - scd.cycles);
      }

      scd.regs[0x30>>1].byte.l = data;
      return;
    }

    case 0x33: /* Interrupts */
    {
      /* update register value before updating interrupts */
      scd.regs[0x32>>1].byte.l = data;

      /* update IEN2 flag */
      scd.regs[0x00].byte.h = (scd.regs[0x00].byte.h & 0x7f) | ((data & 0x04) << 5);
      
      /* update IRQ level */
      s68k_update_irq((scd.pending & data) >> 1);
      return;
    }

    case 0x37: /* CDD control (controlled by BIOS, byte access only ?) */
    {
      /* CDD communication started ? */
      if ((data & 0x04) && !(scd.regs[0x37>>1].byte.l & 0x04))
      {
        /* reset CDD cycle counter */
        cdd.cycles = (scd.cycles - s68k.cycles) * 3;

        /* set pending interrupt level 4 */
        scd.pending |= (1 << 4);

        /* update IRQ level if interrupt is enabled */
        if (scd.regs[0x32>>1].byte.l & 0x10)
        {
          s68k_update_irq((scd.pending & scd.regs[0x32>>1].byte.l) >> 1);
        }
      }

      scd.regs[0x37>>1].byte.l = data;
      return;
    }

    default:
    {
      /* SUB-CPU communication words */
      if ((address & 0xf0) == 0x20)
      {
        s68k_poll_sync((address - 0x10) & 0x1e);
      }

      /* default registers */
      if (address & 1)
      {
        /* register LSB */
        scd.regs[(address >> 1) & 0xff].byte.l = data;
        return;
      }

      /* register MSB */
      scd.regs[(address >> 1) & 0xff].byte.h = data;
      return;
    }
  }
}

static void scd_write_word(unsigned int address, unsigned int data)
{
  /* PCM area (8K) is mirrored into $FF0000-$FF7FFF */
  if (address < 0xff8000)
  {
    /* get /LDS only */
    pcm_write((address >> 1) & 0x1fff, data);
    return;
  }

#ifdef LOG_SCD
  error("[%d][%d]write word CD register %X -> 0x%04x (%X)\n", v_counter, s68k.cycles, address, data, s68k.pc);
#endif

  /* Gate-Array registers */
  switch (address & 0x1fe)
  {
    case 0x00: /* LED status & RESET */
    {
      /* only update LED status (register $00 is reserved for MAIN-CPU, use $06 instead) */
      scd.regs[0x06>>1].byte.h = data >> 8;

      /* RESET bit cleared ? */      
      if (!(data & 0x01))
      {
        /* reset CD hardware */
        scd_reset(0);
      }
      return;
    }

    case 0x02: /* Memory Mode */
    {
      s68k_poll_sync(0x02);

      /* detect MODE & RET bits modifications */
      if ((data ^ scd.regs[0x03>>1].byte.l) & 0x05)
      {
        int i;

        /* MODE bit */
        if (data & 0x04)
        {
          /* 2M->1M mode switch */
          if (!(scd.regs[0x03 >> 1].byte.l & 0x04))
          {
            /* re-arrange Word-RAM banks */
            word_ram_switch(0x04);
          }

          /* RET bit in 1M Mode */
          if (data & 0x01)
          {
            /* Word-RAM 1 assigned to MAIN-CPU */
            for (i=scd.cartridge.boot+0x20; i<scd.cartridge.boot+0x22; i++)
            {
              /* Word-RAM 1 data mapped at $200000-$21FFFF */
              m68k.memory_map[i].base = scd.word_ram[1] + ((i & 0x01) << 16);
            }

            for (i=scd.cartridge.boot+0x22; i<scd.cartridge.boot+0x24; i++)
            {
              /* VRAM cell image mapped at $220000-$23FFFF */
              m68k.memory_map[i].read8   = cell_ram_1_read8;
              m68k.memory_map[i].read16  = cell_ram_1_read16;
              m68k.memory_map[i].write8  = cell_ram_1_write8;
              m68k.memory_map[i].write16 = cell_ram_1_write16;
              zbank_memory_map[i].read   = cell_ram_1_read8;
              zbank_memory_map[i].write  = cell_ram_1_write8;
            }

            /* Word-RAM 0 assigned to SUB-CPU */
            for (i=0x08; i<0x0c; i++)
            {
              /* DOT image mapped at $080000-$0BFFFF */
              s68k.memory_map[i].read8   = dot_ram_0_read8;
              s68k.memory_map[i].read16  = dot_ram_0_read16;
              s68k.memory_map[i].write8  = dot_ram_0_write8;
              s68k.memory_map[i].write16 = dot_ram_0_write16;
            }

            for (i=0x0c; i<0x0e; i++)
            {
              /* Word-RAM 0 data mapped at $0C0000-$0DFFFF */
              s68k.memory_map[i].base    = scd.word_ram[0] + ((i & 0x01) << 16);
              s68k.memory_map[i].read8   = NULL;
              s68k.memory_map[i].read16  = NULL;
              s68k.memory_map[i].write8  = NULL;
              s68k.memory_map[i].write16 = NULL;
            }

            /* writing 1 to RET bit in 1M mode returns Word-RAM to MAIN-CPU in 2M mode */
            scd.dmna = 0;
          }
          else
          {
            /* Word-RAM 0 assigned to MAIN-CPU */
            for (i=scd.cartridge.boot+0x20; i<scd.cartridge.boot+0x22; i++)
            {
              /* Word-RAM 0 data mapped at $200000-$21FFFF */
              m68k.memory_map[i].base = scd.word_ram[0] + ((i & 0x01) << 16);
            }

            for (i=scd.cartridge.boot+0x22; i<scd.cartridge.boot+0x24; i++)
            {
              /* VRAM cell image mapped at $220000-$23FFFF */
              m68k.memory_map[i].read8   = cell_ram_0_read8;
              m68k.memory_map[i].read16  = cell_ram_0_read16;
              m68k.memory_map[i].write8  = cell_ram_0_write8;
              m68k.memory_map[i].write16 = cell_ram_0_write16;
              zbank_memory_map[i].read   = cell_ram_0_read8;
              zbank_memory_map[i].write  = cell_ram_0_write8;
            }

            /* Word-RAM 1 assigned to SUB-CPU */
            for (i=0x08; i<0x0c; i++)
            {
              /* DOT image mapped at $080000-$0BFFFF */
              s68k.memory_map[i].read8   = dot_ram_1_read8;
              s68k.memory_map[i].read16  = dot_ram_1_read16;
              s68k.memory_map[i].write8  = dot_ram_1_write8;
              s68k.memory_map[i].write16 = dot_ram_1_write16;
            }

            for (i=0x0c; i<0x0e; i++)
            {
              /* Word-RAM 1 data mapped at $0C0000-$0DFFFF */
              s68k.memory_map[i].base    = scd.word_ram[1] + ((i & 0x01) << 16);
              s68k.memory_map[i].read8   = NULL;
              s68k.memory_map[i].read16  = NULL;
              s68k.memory_map[i].write8  = NULL;
              s68k.memory_map[i].write16 = NULL;
            }
          }

          /* clear DMNA bit (swap completed) */
          scd.regs[0x03>>1].byte.l = (scd.regs[0x03>>1].byte.l & ~0x1f) | (data & 0x1d);
          return;
        }
        else
        {
          /* 1M->2M mode switch */
          if (scd.regs[0x03>>1].byte.l & 0x04)
          {
            /* re-arrange Word-RAM banks */
            word_ram_switch(0x00);

            /* RET bit set during 1M mode ? */
            data |= ~scd.dmna & 0x01;
            
            /* check if RET bit is cleared */
            if (!(data & 0x01))
            {
              /* set DMNA bit */
              data |= 0x02;

              /* mask BK0-1 bits (MAIN-CPU side only) */
              scd.regs[0x03>>1].byte.l = (scd.regs[0x03>>1].byte.l & ~0x1f) | (data & 0x1f);
              return;
            }
          }
          
          /* RET bit set in 2M mode */
          if (data & 0x01)
          {
            /* Word-RAM is returned to MAIN-CPU */
            scd.dmna = 0;

            /* clear DMNA bit */
            scd.regs[0x03>>1].byte.l = (scd.regs[0x03>>1].byte.l & ~0x1f) | (data & 0x1d);
            return;
          }
        }
      }

      /* update PM0-1 & MODE bits */
      scd.regs[0x03>>1].byte.l = (scd.regs[0x03>>1].byte.l & ~0x1c) | (data & 0x1c);
      return;
    }

    case 0x06: /* CDC register write */
    {
      cdc_reg_w(data);
      return;
    }

    case 0x0c: /* Stopwatch (word access only) */
    {
      /* synchronize the counter with SUB-CPU */
      int ticks = (s68k.cycles - scd.stopwatch) / TIMERS_SCYCLES_RATIO;
      scd.stopwatch += (ticks * TIMERS_SCYCLES_RATIO);

      /* any writes clear the counter */
      scd.regs[0x0c>>1].w = 0;
      return;
    }

    case 0x0e:  /* SUB-CPU communication flags */
    {
      s68k_poll_sync(0x0e);

      /* MSB is read-only */
      scd.regs[0x0f>>1].byte.l = data;
      return;
    }

    case 0x30: /* Timer */
    {
      /* LSB only */
      data &= 0xff;

      /* reload timer (one timer clock = 384 CPU cycles) */
      scd.timer = data * TIMERS_SCYCLES_RATIO;

      /* only non-zero data starts timer, writing zero stops it */
      if (data)
      {
        /* adjust regarding current CPU cycle */
        scd.timer += (s68k.cycles - scd.cycles);
      }

      scd.regs[0x30>>1].byte.l = data;
      return;
    }

    case 0x32: /* Interrupts */
    {
      /* LSB only */
      data &= 0xff;

      /* update register value before updating interrupts */
      scd.regs[0x32>>1].byte.l = data;

      /* update IEN2 flag */
      scd.regs[0x00].byte.h = (scd.regs[0x00].byte.h & 0x7f) | ((data & 0x04) << 5);
      
      /* update IRQ level */
      s68k_update_irq((scd.pending & data) >> 1);
      return;
    }

    case 0x4a: /* CDD command 9 (controlled by BIOS, word access only ?) */
    {
      scd.regs[0x4a>>1].w = 0;
      cdd_process();
#ifdef LOG_CDD
      error("CDD command: %02x %02x %02x %02x %02x %02x %02x %02x\n",scd.regs[0x42>>1].byte.h, scd.regs[0x42>>1].byte.l, scd.regs[0x44>>1].byte.h, scd.regs[0x44>>1].byte.l, scd.regs[0x46>>1].byte.h, scd.regs[0x46>>1].byte.l, scd.regs[0x48>>1].byte.h, scd.regs[0x48>>1].byte.l);
      error("CDD status:  %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",scd.regs[0x38>>1].byte.h, scd.regs[0x38>>1].byte.l, scd.regs[0x3a>>1].byte.h, scd.regs[0x3a>>1].byte.l, scd.regs[0x3c>>1].byte.h, scd.regs[0x3c>>1].byte.l, scd.regs[0x3e>>1].byte.h, scd.regs[0x3e>>1].byte.l, scd.regs[0x40>>1].byte.h, scd.regs[0x40>>1].byte.l);
#endif
      break;
    }

    case 0x66: /* Trace vector base address */
    {
      scd.regs[0x66>>1].w = data;
      
      /* start GFX operation */
      gfx_start(data, s68k.cycles);
      return;
    }

    default:
    {
      /* SUB-CPU communication words */
      if ((address & 0xf0) == 0x20)
      {
        s68k_poll_sync((address - 0x10) & 0x1e);
      }

      /* default registers */
      scd.regs[(address >> 1) & 0xff].w = data;
      return;
    }
  }
}


void scd_init(void)
{
  int i;
  
  /****************************************************************/
  /*  MAIN-CPU low memory map ($000000-$7FFFFF)                   */
  /****************************************************************/

  /* 0x00: boot from CD (Mode 2), 0x40: boot from cartridge (Mode 1) */
  uint8 base = scd.cartridge.boot;

  /* $400000-$7FFFFF (resp. $000000-$3FFFFF): cartridge area (4MB) */
  cd_cart_init();

  /* $000000-$1FFFFF (resp. $400000-$5FFFFF): CD memory area */
  for (i=base; i<base+0x20; i++)
  {
    if (i & 2)
    {
      /* PRG-RAM (first 128KB bank, mirrored) */
      m68k.memory_map[i].base    = scd.prg_ram + ((i & 1) << 16);
      m68k.memory_map[i].read8   = NULL;
      m68k.memory_map[i].read16  = NULL;
      m68k.memory_map[i].write8  = NULL;
      m68k.memory_map[i].write16 = NULL;
      zbank_memory_map[i].read   = NULL;
      zbank_memory_map[i].write  = NULL;

    }
    else
    {
      /* internal ROM (128KB mirrored) (Flux expects it to be mapped at $440000-$45FFFF) */
      m68k.memory_map[i].base    = scd.bootrom + ((i & 1) << 16);
      m68k.memory_map[i].read8   = NULL;
      m68k.memory_map[i].read16  = NULL;
      m68k.memory_map[i].write8  = m68k_unused_8_w;
      m68k.memory_map[i].write16 = m68k_unused_16_w;
      zbank_memory_map[i].read   = NULL;
      zbank_memory_map[i].write  = zbank_unused_w;
    }
  }

  /* $200000-$3FFFFF (resp. $600000-$7FFFFF): Word-RAM in 2M mode (256KB mirrored) */
  for (i=base+0x20; i<base+0x40; i++)
  {
    m68k.memory_map[i].base    = scd.word_ram_2M + ((i & 3) << 16);
    m68k.memory_map[i].read8   = NULL;
    m68k.memory_map[i].read16  = NULL;
    m68k.memory_map[i].write8  = NULL;
    m68k.memory_map[i].write16 = NULL;
    zbank_memory_map[i].read   = NULL;
    zbank_memory_map[i].write  = NULL;
  }
  
  /****************************************************************/
  /*  SUB-CPU memory map ($000000-$FFFFFF)                        */
  /****************************************************************/

  /* $000000-$07FFFF: PRG-RAM (512KB) */
  for (i=0x00; i<0x08; i++)
  {
    s68k.memory_map[i].base    = scd.prg_ram + (i << 16);
    s68k.memory_map[i].read8   = NULL;
    s68k.memory_map[i].read16  = NULL;

    /* first 128KB is write-protected */
    s68k.memory_map[i].write8  = (i < 0x02) ? prg_ram_write_byte : NULL;
    s68k.memory_map[i].write16 = (i < 0x02) ? prg_ram_write_word : NULL;
  }

  /* $080000-$0BFFFF:  Word-RAM in 2M mode (256KB)*/
  for (i=0x08; i<0x0c; i++)
  {
    /* allow Word-RAM access from both CPU in 2M mode (fixes sync issues in Mortal Kombat) */
    s68k.memory_map[i].base    = scd.word_ram_2M + ((i & 3) << 16);
    s68k.memory_map[i].read8   = NULL;
    s68k.memory_map[i].read16  = NULL;
    s68k.memory_map[i].write8  = NULL;
    s68k.memory_map[i].write16 = NULL;
  }
  
  /* $0C0000-$FD0000: Unused area (Word-RAM mirrored ?) */
  for (i=0x0c; i<0xfd; i++)
  {
    s68k.memory_map[i].base     = scd.word_ram_2M + ((i & 3) << 16);
    s68k.memory_map[i].read8    = s68k_read_bus_8;
    s68k.memory_map[i].read16   = s68k_read_bus_16;
    s68k.memory_map[i].write8   = s68k_unused_8_w;
    s68k.memory_map[i].write16  = s68k_unused_16_w;
  }

  /* $FD0000-$FF0000 (odd address only): 8KB backup RAM, mirrored(Wonder Mega / X'Eye BIOS access it at $FD0000-$FD1FFF) */
  for (i=0xfd; i<0xff; i++)
  {
    s68k.memory_map[i].base     = NULL;
    s68k.memory_map[i].read8    = bram_read_byte;
    s68k.memory_map[i].read16   = bram_read_word;
    s68k.memory_map[i].write8   = bram_write_byte;
    s68k.memory_map[i].write16  = bram_write_word;
  }

  /* $FF0000-$FFFFFF: PCM hardware & SUB-CPU registers  */
  s68k.memory_map[0xff].base     = NULL;
  s68k.memory_map[0xff].read8    = scd_read_byte;
  s68k.memory_map[0xff].read16   = scd_read_word;
  s68k.memory_map[0xff].write8   = scd_write_byte;
  s68k.memory_map[0xff].write16  = scd_write_word;

  /* Initialize CD hardware */
  cdc_init();
  gfx_init();

  /* Clear RAM */
  memset(scd.prg_ram, 0x00, sizeof(scd.prg_ram));
  memset(scd.word_ram, 0x00, sizeof(scd.word_ram));
  memset(scd.word_ram_2M, 0x00, sizeof(scd.word_ram_2M));
  memset(scd.bram, 0x00, sizeof(scd.bram));
}

void scd_reset(int hard)
{
  /* TODO: figure what exactly is resetted when RESET bit is cleared by SUB-CPU */
  if (hard)
  {
    /* Clear all ASIC registers by default */
    memset(scd.regs, 0, sizeof(scd.regs));

    /* Clear pending DMNA write status */
    scd.dmna = 0;

    /* H-INT default vector */
    *(uint16 *)(m68k.memory_map[0].base + 0x70) = 0x00FF;
    *(uint16 *)(m68k.memory_map[0].base + 0x72) = 0xFFFF;

    /* Power ON initial values (MAIN-CPU side) */
    scd.regs[0x00>>1].w = 0x0002;
    scd.regs[0x02>>1].w = 0x0001;

    /* 2M mode */
    word_ram_switch(0);
  }
  else
  {
    /* Clear only SUB-CPU side registers */
    memset(&scd.regs[0x04>>1], 0, sizeof(scd.regs) - 4);
  }

  /* SUB-CPU side default values */
  scd.regs[0x08>>1].w = 0xffff;
  scd.regs[0x0a>>1].w = 0xffff;
  scd.regs[0x36>>1].w = 0x0100;
  scd.regs[0x40>>1].w = 0x000f;
  scd.regs[0x42>>1].w = 0xffff;
  scd.regs[0x44>>1].w = 0xffff;
  scd.regs[0x46>>1].w = 0xffff;
  scd.regs[0x48>>1].w = 0xffff;
  scd.regs[0x4a>>1].w = 0xffff;

  /* RESET register always return 1 (register $06 is unused by both sides, it is used for SUB-CPU first register) */
  scd.regs[0x06>>1].byte.l = 0x01;

  /* Reset Timer & Stopwatch counters */
  scd.timer = 0;
  scd.stopwatch = 0;

  /* Reset frame cycle counter */
  scd.cycles = 0;

  /* Clear pending interrupts */
  scd.pending = 0;

  /* Clear CPU polling detection */
  memset(&m68k.poll, 0, sizeof(m68k.poll));
  memset(&s68k.poll, 0, sizeof(s68k.poll));

  /* Reset CD hardware */
  cdd_reset();
  cdc_reset();
  gfx_reset();
  pcm_reset();
}

void scd_update(unsigned int cycles)
{
  /* update CDC DMA transfer */
  if (cdc.dma_w)
  {
    cdc_dma_update();
  }

  /* run both CPU in sync until end of line */
  do
  {
    m68k_run(cycles);
    s68k_run(scd.cycles + SCYCLES_PER_LINE);
  }
  while ((m68k.cycles < cycles) || (s68k.cycles < (scd.cycles + SCYCLES_PER_LINE)));

  /* increment CD hardware cycle counter */
  scd.cycles += SCYCLES_PER_LINE;

  /* CDD processing at 75Hz (one clock = 12500000/75 = 500000/3 CPU clocks) */
  cdd.cycles += (SCYCLES_PER_LINE * 3);
  if (cdd.cycles >= (500000 * 4))
  {
    /* reload CDD cycle counter */
    cdd.cycles -= (500000 * 4);

    /* update CDD sector */
    cdd_update();

    /* check if a new CDD command has been processed */
    if (!(scd.regs[0x4a>>1].byte.l & 0xf0))
    {
      /* reset CDD command wait flag */
      scd.regs[0x4a>>1].byte.l = 0xf0;

      /* pending level 4 interrupt */
      scd.pending |= (1 << 4);

      /* level 4 interrupt enabled */
      if (scd.regs[0x32>>1].byte.l & 0x10)
      {
        /* update IRQ level */
        s68k_update_irq((scd.pending & scd.regs[0x32>>1].byte.l) >> 1);
      }
    }
  }

  /* Timer */
  if (scd.timer)
  {
    /* decrement timer */
    scd.timer -= SCYCLES_PER_LINE;
    if (scd.timer <= 0)
    {
      /* reload timer (one timer clock = 384 CPU cycles) */
      scd.timer += (scd.regs[0x30>>1].byte.l * TIMERS_SCYCLES_RATIO);

      /* level 3 interrupt enabled ? */
      if (scd.regs[0x32>>1].byte.l & 0x08)
      {
        /* trigger level 3 interrupt */
        scd.pending |= (1 << 3);

        /* update IRQ level */
        s68k_update_irq((scd.pending & scd.regs[0x32>>1].byte.l) >> 1);
      }
    }
  }

  /* GFX processing */
  if (scd.regs[0x58>>1].byte.h & 0x80)
  {
    /* update graphics operation if running */
    gfx_update(scd.cycles);
  }
}

void scd_end_frame(unsigned int cycles)
{
  /* run Stopwatch until end of frame */
  int ticks = (cycles - scd.stopwatch) / TIMERS_SCYCLES_RATIO;
  scd.regs[0x0c>>1].w = (scd.regs[0x0c>>1].w + ticks) & 0xfff;

  /* adjust Stopwatch counter for next frame (can be negative) */
  scd.stopwatch += (ticks * TIMERS_SCYCLES_RATIO) - cycles;

  /* adjust SUB-CPU & GPU cycle counters for next frame */
  s68k.cycles -= cycles;
  gfx.cycles  -= cycles;

  /* reset CPU registers polling */
  m68k.poll.cycle = 0;
  s68k.poll.cycle = 0;
}

int scd_context_save(uint8 *state)
{
  uint16 tmp16;
  uint32 tmp32;
  int bufferptr = 0;

  /* internal harware */
  save_param(scd.regs, sizeof(scd.regs));
  save_param(&scd.cycles, sizeof(scd.cycles));
  save_param(&scd.timer, sizeof(scd.timer));
  save_param(&scd.pending, sizeof(scd.pending));
  save_param(&scd.dmna, sizeof(scd.dmna));

  /* GFX processor */
  bufferptr += gfx_context_save(&state[bufferptr]);

  /* CD Data controller */
  bufferptr += cdc_context_save(&state[bufferptr]);

  /* CD Drive processor */
  bufferptr += cdd_context_save(&state[bufferptr]);

  /* PCM chip */
  bufferptr += pcm_context_save(&state[bufferptr]);

  /* PRG-RAM */
  save_param(scd.prg_ram, sizeof(scd.prg_ram));

  /* Word-RAM */
  if (scd.regs[0x03>>1].byte.l & 0x04)
  {
    /* 1M mode */
    save_param(scd.word_ram, sizeof(scd.word_ram));
  }
  else
  {
    /* 2M mode */
    save_param(scd.word_ram_2M, sizeof(scd.word_ram_2M));
  }

  /* MAIN-CPU & SUB-CPU polling */
  save_param(&m68k.poll, sizeof(m68k.poll));
  save_param(&s68k.poll, sizeof(s68k.poll));

  /* H-INT default vector */
  tmp16 = *(uint16 *)(m68k.memory_map[0].base + 0x72);
  save_param(&tmp16, 2);

  /* SUB-CPU internal state */
  save_param(&s68k.cycles, sizeof(s68k.cycles));
  save_param(&s68k.int_level, sizeof(s68k.int_level));
  save_param(&s68k.stopped, sizeof(s68k.stopped));

  /* SUB-CPU registers */
  tmp32 = s68k_get_reg(M68K_REG_D0);  save_param(&tmp32, 4);
  tmp32 = s68k_get_reg(M68K_REG_D1);  save_param(&tmp32, 4);
  tmp32 = s68k_get_reg(M68K_REG_D2);  save_param(&tmp32, 4);
  tmp32 = s68k_get_reg(M68K_REG_D3);  save_param(&tmp32, 4);
  tmp32 = s68k_get_reg(M68K_REG_D4);  save_param(&tmp32, 4);
  tmp32 = s68k_get_reg(M68K_REG_D5);  save_param(&tmp32, 4);
  tmp32 = s68k_get_reg(M68K_REG_D6);  save_param(&tmp32, 4);
  tmp32 = s68k_get_reg(M68K_REG_D7);  save_param(&tmp32, 4);
  tmp32 = s68k_get_reg(M68K_REG_A0);  save_param(&tmp32, 4);
  tmp32 = s68k_get_reg(M68K_REG_A1);  save_param(&tmp32, 4);
  tmp32 = s68k_get_reg(M68K_REG_A2);  save_param(&tmp32, 4);
  tmp32 = s68k_get_reg(M68K_REG_A3);  save_param(&tmp32, 4);
  tmp32 = s68k_get_reg(M68K_REG_A4);  save_param(&tmp32, 4);
  tmp32 = s68k_get_reg(M68K_REG_A5);  save_param(&tmp32, 4);
  tmp32 = s68k_get_reg(M68K_REG_A6);  save_param(&tmp32, 4);
  tmp32 = s68k_get_reg(M68K_REG_A7);  save_param(&tmp32, 4);
  tmp32 = s68k_get_reg(M68K_REG_PC);  save_param(&tmp32, 4);
  tmp16 = s68k_get_reg(M68K_REG_SR);  save_param(&tmp16, 2); 
  tmp32 = s68k_get_reg(M68K_REG_USP); save_param(&tmp32, 4);
  tmp32 = s68k_get_reg(M68K_REG_ISP); save_param(&tmp32, 4);

  /* bootable MD cartridge */
  if (scd.cartridge.boot)
  {
    bufferptr += md_cart_context_save(&state[bufferptr]);
  }

  return bufferptr;
}

int scd_context_load(uint8 *state)
{
  int i;
  uint16 tmp16;
  uint32 tmp32;
  int bufferptr = 0;

  /* internal harware */
  load_param(scd.regs, sizeof(scd.regs));
  load_param(&scd.cycles, sizeof(scd.cycles));
  load_param(&scd.timer, sizeof(scd.timer));
  load_param(&scd.pending, sizeof(scd.pending));
  load_param(&scd.dmna, sizeof(scd.dmna));

  /* GFX processor */
  bufferptr += gfx_context_load(&state[bufferptr]);

  /* CD Data controller */
  bufferptr += cdc_context_load(&state[bufferptr]);

  /* CD Drive processor */
  bufferptr += cdd_context_load(&state[bufferptr]);

  /* PCM chip */
  bufferptr += pcm_context_load(&state[bufferptr]);

  /* PRG-RAM */
  load_param(scd.prg_ram, sizeof(scd.prg_ram));

  /* PRG-RAM 128k bank mapped to $020000-$03FFFF (resp. $420000-$43FFFF) */
  m68k.memory_map[scd.cartridge.boot + 0x02].base = scd.prg_ram + ((scd.regs[0x03>>1].byte.l & 0xc0) << 11);
  m68k.memory_map[scd.cartridge.boot + 0x03].base = m68k.memory_map[scd.cartridge.boot + 0x02].base + 0x10000;

  /* Word-RAM */
  if (scd.regs[0x03>>1].byte.l & 0x04)
  {
    /* 1M Mode */
    load_param(scd.word_ram, sizeof(scd.word_ram));
  
    if (scd.regs[0x03>>1].byte.l & 0x01)
    {
      /* Word-RAM 1 assigned to MAIN-CPU */
      for (i=scd.cartridge.boot+0x20; i<scd.cartridge.boot+0x22; i++)
      {
        /* Word-RAM 1 data mapped at $200000-$21FFFF */
        m68k.memory_map[i].base = scd.word_ram[1] + ((i & 0x01) << 16);
      }

      for (i=scd.cartridge.boot+0x22; i<scd.cartridge.boot+0x24; i++)
      {
        /* VRAM cell image mapped at $220000-$23FFFF */
        m68k.memory_map[i].read8   = cell_ram_1_read8;
        m68k.memory_map[i].read16  = cell_ram_1_read16;
        m68k.memory_map[i].write8  = cell_ram_1_write8;
        m68k.memory_map[i].write16 = cell_ram_1_write16;
        zbank_memory_map[i].read   = cell_ram_1_read8;
        zbank_memory_map[i].write  = cell_ram_1_write8;
      }

      /* Word-RAM 0 assigned to SUB-CPU */
      for (i=0x08; i<0x0c; i++)
      {
        /* DOT image mapped at $080000-$0BFFFF */
        s68k.memory_map[i].read8   = dot_ram_0_read8;
        s68k.memory_map[i].read16  = dot_ram_0_read16;
        s68k.memory_map[i].write8  = dot_ram_0_write8;
        s68k.memory_map[i].write16 = dot_ram_0_write16;
      }

      for (i=0x0c; i<0x0e; i++)
      {
        /* Word-RAM 0 data mapped at $0C0000-$0DFFFF */
        s68k.memory_map[i].base    = scd.word_ram[0] + ((i & 0x01) << 16);
        s68k.memory_map[i].read8   = NULL;
        s68k.memory_map[i].read16  = NULL;
        s68k.memory_map[i].write8  = NULL;
        s68k.memory_map[i].write16 = NULL;
      }
    }
    else
    {
      /* Word-RAM 0 assigned to MAIN-CPU */
      for (i=scd.cartridge.boot+0x20; i<scd.cartridge.boot+0x22; i++)
      {
        /* Word-RAM 0 data mapped at $200000-$21FFFF */
        m68k.memory_map[i].base = scd.word_ram[0] + ((i & 0x01) << 16);
      }

      for (i=scd.cartridge.boot+0x22; i<scd.cartridge.boot+0x24; i++)
      {
        /* VRAM cell image mapped at $220000-$23FFFF */
        m68k.memory_map[i].read8   = cell_ram_0_read8;
        m68k.memory_map[i].read16  = cell_ram_0_read16;
        m68k.memory_map[i].write8  = cell_ram_0_write8;
        m68k.memory_map[i].write16 = cell_ram_0_write16;
        zbank_memory_map[i].read   = cell_ram_0_read8;
        zbank_memory_map[i].write  = cell_ram_0_write8;
      }

      /* Word-RAM 1 assigned to SUB-CPU */
      for (i=0x08; i<0x0c; i++)
      {
        /* DOT image mapped at $080000-$0BFFFF */
        s68k.memory_map[i].read8   = dot_ram_1_read8;
        s68k.memory_map[i].read16  = dot_ram_1_read16;
        s68k.memory_map[i].write8  = dot_ram_1_write8;
        s68k.memory_map[i].write16 = dot_ram_1_write16;
      }

      for (i=0x0c; i<0x0e; i++)
      {
        /* Word-RAM 1 data mapped at $0C0000-$0DFFFF */
        s68k.memory_map[i].base    = scd.word_ram[1] + ((i & 0x01) << 16);
        s68k.memory_map[i].read8   = NULL;
        s68k.memory_map[i].read16  = NULL;
        s68k.memory_map[i].write8  = NULL;
        s68k.memory_map[i].write16 = NULL;
      }
    }
  }
  else
  {
    /* 2M mode */
    load_param(scd.word_ram_2M, sizeof(scd.word_ram_2M));

    for (i=scd.cartridge.boot+0x20; i<scd.cartridge.boot+0x24; i++)
    {
      /* MAIN-CPU: $200000-$23FFFF is mapped to 256K Word-RAM */
      m68k.memory_map[i].base    = scd.word_ram_2M + ((i & 0x03) << 16);
      m68k.memory_map[i].read8   = NULL;
      m68k.memory_map[i].read16  = NULL;
      m68k.memory_map[i].write8  = NULL;
      m68k.memory_map[i].write16 = NULL;
      zbank_memory_map[i].read   = NULL;
      zbank_memory_map[i].write  = NULL;
    }

    for (i=0x08; i<0x0c; i++)
    {
      /* SUB-CPU: $080000-$0BFFFF is mapped to 256K Word-RAM */
      s68k.memory_map[i].read8   = NULL;
      s68k.memory_map[i].read16  = NULL;
      s68k.memory_map[i].write8  = NULL;
      s68k.memory_map[i].write16 = NULL;
    }

    for (i=0x0c; i<0x0e; i++)
    {
      /* SUB-CPU: $0C0000-$0DFFFF is unmapped */
      s68k.memory_map[i].read8   = s68k_read_bus_8;
      s68k.memory_map[i].read16  = s68k_read_bus_16;
      s68k.memory_map[i].write8  = s68k_unused_8_w;
      s68k.memory_map[i].write16 = s68k_unused_16_w;
    }
  }

  /* MAIN-CPU & SUB-CPU polling */
  load_param(&m68k.poll, sizeof(m68k.poll));
  load_param(&s68k.poll, sizeof(s68k.poll));

  /* H-INT default vector */
  load_param(&tmp16, 2);
  *(uint16 *)(m68k.memory_map[0].base + 0x72) = tmp16;

  /* SUB-CPU internal state */
  load_param(&s68k.cycles, sizeof(s68k.cycles));
  load_param(&s68k.int_level, sizeof(s68k.int_level));
  load_param(&s68k.stopped, sizeof(s68k.stopped));

  /* SUB-CPU registers */
  load_param(&tmp32, 4); s68k_set_reg(M68K_REG_D0, tmp32);
  load_param(&tmp32, 4); s68k_set_reg(M68K_REG_D1, tmp32);
  load_param(&tmp32, 4); s68k_set_reg(M68K_REG_D2, tmp32);
  load_param(&tmp32, 4); s68k_set_reg(M68K_REG_D3, tmp32);
  load_param(&tmp32, 4); s68k_set_reg(M68K_REG_D4, tmp32);
  load_param(&tmp32, 4); s68k_set_reg(M68K_REG_D5, tmp32);
  load_param(&tmp32, 4); s68k_set_reg(M68K_REG_D6, tmp32);
  load_param(&tmp32, 4); s68k_set_reg(M68K_REG_D7, tmp32);
  load_param(&tmp32, 4); s68k_set_reg(M68K_REG_A0, tmp32);
  load_param(&tmp32, 4); s68k_set_reg(M68K_REG_A1, tmp32);
  load_param(&tmp32, 4); s68k_set_reg(M68K_REG_A2, tmp32);
  load_param(&tmp32, 4); s68k_set_reg(M68K_REG_A3, tmp32);
  load_param(&tmp32, 4); s68k_set_reg(M68K_REG_A4, tmp32);
  load_param(&tmp32, 4); s68k_set_reg(M68K_REG_A5, tmp32);
  load_param(&tmp32, 4); s68k_set_reg(M68K_REG_A6, tmp32);
  load_param(&tmp32, 4); s68k_set_reg(M68K_REG_A7, tmp32);
  load_param(&tmp32, 4); s68k_set_reg(M68K_REG_PC, tmp32);  
  load_param(&tmp16, 2); s68k_set_reg(M68K_REG_SR, tmp16);
  load_param(&tmp32, 4); s68k_set_reg(M68K_REG_USP,tmp32);
  load_param(&tmp32, 4); s68k_set_reg(M68K_REG_ISP,tmp32);

  /* bootable MD cartridge hardware */
  if (scd.cartridge.boot)
  {
    bufferptr += md_cart_context_load(&state[bufferptr]);
  }

  return bufferptr;
}

int scd_68k_irq_ack(int level)
{
#ifdef LOG_SCD
  error("INT ack level %d  (%X)\n", level, s68k.pc);
#endif

#if 0
  /* level 5 interrupt is normally acknowledged by CDC */
  if (level != 5)
#endif
  {
    /* clear pending interrupt flag */
    scd.pending &= ~(1 << level);

    /* level 2 interrupt acknowledge */
    if (level == 2)
    {
      /* clear IFL2 flag */
      scd.regs[0x00].byte.h &= ~0x01;
    }

    /* update IRQ level */
    s68k_update_irq((scd.pending & scd.regs[0x32>>1].byte.l) >> 1);
  }

  return M68K_INT_ACK_AUTOVECTOR;
}
