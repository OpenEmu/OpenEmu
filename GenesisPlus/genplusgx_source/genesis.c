/***************************************************************************************
 *  Genesis Plus
 *  Internal Hardware & Bus controllers
 *
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald (original code)
 *  Eke-Eke (2007-2011), additional code & fixes for the GCN/Wii port
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

uint8 tmss[4];            /* TMSS security register */
uint8 bios_rom[0x800];    /* OS ROM   */
uint8 work_ram[0x10000];  /* 68K RAM  */
uint8 zram[0x2000];       /* Z80 RAM  */
uint32 zbank;             /* Z80 bank window address */
uint8 zstate;             /* Z80 bus state (d0 = BUSACK, d1 = /RESET) */

/* PICO data */
uint8 pico_current;
uint8 pico_page[7];

/*--------------------------------------------------------------------------*/
/* Init, reset, shutdown functions                                          */
/*--------------------------------------------------------------------------*/

void gen_init(void)
{
  int i;

  /* initialize 68k */
  m68k_set_cpu_type(M68K_CPU_TYPE_68000);
  m68k_init();

  /* initialize Z80 */
  z80_init(0,z80_irq_callback);

  /* initialize 68k memory map */
  /* $000000-$7FFFFF is affected to cartridge area (see md_cart.c) */
  for (i=0x80; i<0x100; i++)
  {
    /* $800000-$FFFFFF is affected to WRAM (see VDP DMA) */
    m68k_memory_map[i].base     = work_ram;
    m68k_memory_map[i].read8    = NULL;
    m68k_memory_map[i].read16   = NULL;
    m68k_memory_map[i].write8   = NULL;
    m68k_memory_map[i].write16  = NULL;
    zbank_memory_map[i].read    = NULL;
    zbank_memory_map[i].write   = NULL;
  }

  /* initialize 68k memory handlers */
  for (i=0x80; i<0xe0; i++)
  {
    /* $800000-$DFFFFF : illegal area by default */
    m68k_memory_map[i].read8    = m68k_lockup_r_8;
    m68k_memory_map[i].read16   = m68k_lockup_r_16;
    m68k_memory_map[i].write8   = m68k_lockup_w_8;
    m68k_memory_map[i].write16  = m68k_lockup_w_16;
    zbank_memory_map[i].read    = zbank_lockup_r;
    zbank_memory_map[i].write   = zbank_lockup_w;
  }

  /* $A10000-$A1FFFF : I/O & Control registers */
  m68k_memory_map[0xa1].read8   = ctrl_io_read_byte;
  m68k_memory_map[0xa1].read16  = ctrl_io_read_word;
  m68k_memory_map[0xa1].write8  = ctrl_io_write_byte;
  m68k_memory_map[0xa1].write16 = ctrl_io_write_word;
  zbank_memory_map[0xa1].read   = zbank_read_ctrl_io;
  zbank_memory_map[0xa1].write  = zbank_write_ctrl_io;

  /* $C0xxxx, $C8xxxx, $D0xxxx, $D8xxxx : VDP ports */
  for (i=0xc0; i<0xe0; i+=8)
  {
    m68k_memory_map[i].read8    = vdp_read_byte;
    m68k_memory_map[i].read16   = vdp_read_word;
    m68k_memory_map[i].write8   = vdp_write_byte;
    m68k_memory_map[i].write16  = vdp_write_word;
    zbank_memory_map[i].read    = zbank_read_vdp;
    zbank_memory_map[i].write   = zbank_write_vdp;
  }

  /* MS COMPATIBILITY mode */
  if (system_hw == SYSTEM_PBC)
  {
    /* initialize Z80 read handler */
    /* NB: memory map & write handler are defined by cartridge hardware */
    z80_readmem = z80_sms_memory_r;

    /* initialize Z80 ports handlers */
    z80_writeport = z80_sms_port_w;
    z80_readport  = z80_sms_port_r;

    /* initialize MS cartridge hardware */
    sms_cart_init();
  }
  else
  {
    /* PICO hardware */
    if (system_hw == SYSTEM_PICO)
    {
      /* additional registers mapped to $800000-$80FFFF */
      m68k_memory_map[0x80].read8   = pico_read_byte;
      m68k_memory_map[0x80].read16  = pico_read_word;
      m68k_memory_map[0x80].write8  = m68k_unused_8_w;
      m68k_memory_map[0x80].write16 = m68k_unused_16_w;

      /* there is no I/O area (Notaz) */
      m68k_memory_map[0xa1].read8   = m68k_read_bus_8;
      m68k_memory_map[0xa1].read16  = m68k_read_bus_16;
      m68k_memory_map[0xa1].write8  = m68k_unused_8_w;
      m68k_memory_map[0xa1].write16 = m68k_unused_16_w;

      /* page registers */
      pico_current = 0x00;
      pico_page[0] = 0x00;
      pico_page[1] = 0x01;
      pico_page[2] = 0x03;
      pico_page[3] = 0x07;
      pico_page[4] = 0x0F;
      pico_page[5] = 0x1F;
      pico_page[6] = 0x3F;
    }

    /* initialize Z80 memory map */
    /* $0000-$3FFF is mapped to Z80 RAM (8K mirrored) */
    /* $4000-$FFFF is mapped to hardware but Z80.PC should never point there */
    for (i=0; i<64; i++)
    {
      z80_readmap[i] = &zram[(i & 7) << 10];
    }

    /* initialize Z80 memory handlers */
    z80_writemem  = z80_md_memory_w;
    z80_readmem   = z80_md_memory_r;

    /* initialize Z80 port handlers */
    z80_writeport = z80_unused_port_w;
    z80_readport  = z80_unused_port_r;

    /* initialize MD cartridge hardware */
    md_cart_init();
  }
}

void gen_reset(int hard_reset)
{
  /* System Reset */
  if (hard_reset)
  {
    /* clear RAM */
    memset (work_ram, 0x00, sizeof (work_ram));
    memset (zram, 0x00, sizeof (zram));

    /* TMSS & OS ROM support */
    if (config.tmss & 1)
    {
      /* clear TMSS register */
      memset(tmss, 0x00, sizeof(tmss));

      /* VDP access is locked by default */
      int i;
      for (i=0xc0; i<0xe0; i+=8)
      {
        m68k_memory_map[i].read8   = m68k_lockup_r_8;
        m68k_memory_map[i].read16  = m68k_lockup_r_16;
        m68k_memory_map[i].write8  = m68k_lockup_w_8;
        m68k_memory_map[i].write16 = m68k_lockup_w_16;
        zbank_memory_map[i].read   = zbank_lockup_r;
        zbank_memory_map[i].write  = zbank_lockup_w;
      }

      /* OS ROM is mapped at $000000-$0007FF */
      if (config.tmss & 2)
      {
        m68k_memory_map[0].base = bios_rom;
      }
    }
  }
  else
  {
    /* reset YM2612 (on hard reset, this is done by sound_reset) */
    fm_reset(0);
  }

  /* 68k & Z80 could restart anywhere in VDP frame (Bonkers, Eternal Champions, X-Men 2) */
  mcycles_68k = mcycles_z80 = (uint32)((MCYCLES_PER_LINE * lines_per_frame) * ((double)rand() / (double)RAND_MAX));

  if (system_hw == SYSTEM_PBC)
  {
    /* reset MS cartridge hardware */
    sms_cart_reset();

    /* Z80 is running */
    zstate = 1;

    /* 68k is halted */
    m68k_pulse_halt();
  }
  else
  {
    /* reset MD cartridge hardware */
    md_cart_reset(hard_reset);

    /* Z80 bus is released & Z80 is reseted */
    m68k_memory_map[0xa0].read8   = m68k_read_bus_8;
    m68k_memory_map[0xa0].read16  = m68k_read_bus_16;
    m68k_memory_map[0xa0].write8  = m68k_unused_8_w;
    m68k_memory_map[0xa0].write16 = m68k_unused_16_w;
    zstate = 0;

    /* assume default bank is $000000-$007FFF */
    zbank = 0;  

    /* reset 68k */
    m68k_pulse_reset();
  }

  /* reset Z80 */
  z80_reset();
}

void gen_shutdown(void)
{
  z80_exit();
}


/*-----------------------------------------------------------------------*/
/*  OS ROM / TMSS register control functions (Genesis mode)              */
/*-----------------------------------------------------------------------*/

void gen_tmss_w(unsigned int offset, unsigned int data)
{
  /* write TMSS regisiter */
  WRITE_WORD(tmss, offset, data);

  /* VDP requires "SEGA" value to be written in TMSS register */
  int i;
  if (strncmp((char *)tmss, "SEGA", 4) == 0)
  {
    for (i=0xc0; i<0xe0; i+=8)
    {
      m68k_memory_map[i].read8    = vdp_read_byte;
      m68k_memory_map[i].read16   = vdp_read_word;
      m68k_memory_map[i].write8   = vdp_write_byte;
      m68k_memory_map[i].write16  = vdp_write_word;
      zbank_memory_map[i].read    = zbank_read_vdp;
      zbank_memory_map[i].write   = zbank_write_vdp;
    }
  }
  else
  {
    for (i=0xc0; i<0xe0; i+=8)
    {
      m68k_memory_map[i].read8    = m68k_lockup_r_8;
      m68k_memory_map[i].read16   = m68k_lockup_r_16;
      m68k_memory_map[i].write8   = m68k_lockup_w_8;
      m68k_memory_map[i].write16  = m68k_lockup_w_16;
      zbank_memory_map[i].read    = zbank_lockup_r;
      zbank_memory_map[i].write   = zbank_lockup_w;
    }
  }
}

void gen_bankswitch_w(unsigned int data)
{
  /* OS ROM has not been loaded yet */
  if (!(config.tmss & 2))
  {
    config.tmss |= 2;
    memcpy(bios_rom, cart.rom, 0x800);
    memset(cart.rom, 0xff, cart.romsize);
  }

  if (data & 1)
  {
    /* enable CART */
    m68k_memory_map[0].base = cart.base;
  }
  else
  {
    /* enable internal BIOS ROM */
    m68k_memory_map[0].base = bios_rom;
  }
}

unsigned int gen_bankswitch_r(void)
{
  return (m68k_memory_map[0].base == cart.base);
}


/*-----------------------------------------------------------------------*/
/* Z80 Bus controller chip functions (Genesis mode)                      */
/* ----------------------------------------------------------------------*/

void gen_zbusreq_w(unsigned int data, unsigned int cycles)
{
  if (data)  /* !ZBUSREQ asserted */
  {
    /* check if Z80 is going to be stopped */
    if (zstate == 1)
    {
      /* resynchronize with 68k */ 
      z80_run(cycles);

      /* enable 68k access to Z80 bus */
      m68k_memory_map[0xa0].read8   = z80_read_byte;
      m68k_memory_map[0xa0].read16  = z80_read_word;
      m68k_memory_map[0xa0].write8  = z80_write_byte;
      m68k_memory_map[0xa0].write16 = z80_write_word;
    }

    /* update Z80 bus status */
    zstate |= 2;
  }
  else  /* !ZBUSREQ released */
  {
    /* check if Z80 is going to be restarted */
    if (zstate == 3)
    {
      /* resynchronize with 68k */
      mcycles_z80 = cycles;

      /* disable 68k access to Z80 bus */
      m68k_memory_map[0xa0].read8   = m68k_read_bus_8;
      m68k_memory_map[0xa0].read16  = m68k_read_bus_16;
      m68k_memory_map[0xa0].write8  = m68k_unused_8_w;
      m68k_memory_map[0xa0].write16 = m68k_unused_16_w;
    }

    /* update Z80 bus status */
    zstate &= 1;
  }
}

void gen_zreset_w(unsigned int data, unsigned int cycles)
{
  if (data)  /* !ZRESET released */
  {
    /* check if Z80 is going to be restarted */
    if (zstate == 0)
    {
      /* resynchronize with 68k */
      mcycles_z80 = cycles;

      /* reset Z80 & YM2612 */
      z80_reset();
      fm_reset(cycles);
    }

    /* check if 68k access to Z80 bus is granted */
    else if (zstate == 2)
    {
      /* enable 68k access to Z80 bus */
      m68k_memory_map[0xa0].read8   = z80_read_byte;
      m68k_memory_map[0xa0].read16  = z80_read_word;
      m68k_memory_map[0xa0].write8  = z80_write_byte;
      m68k_memory_map[0xa0].write16 = z80_write_word;

      /* reset Z80 & YM2612 */
      z80_reset();
      fm_reset(cycles);
    }

    /* update Z80 bus status */
    zstate |= 1;
  }
  else  /* !ZRESET asserted */
  {
    /* check if Z80 is going to be stopped */
    if (zstate == 1)
    {
      /* resynchronize with 68k */
      z80_run(cycles);
    }

    /* check if 68k had access to Z80 bus */
    else if (zstate == 3)
    {
      /* disable 68k access to Z80 bus */
      m68k_memory_map[0xa0].read8   = m68k_read_bus_8;
      m68k_memory_map[0xa0].read16  = m68k_read_bus_16;
      m68k_memory_map[0xa0].write8  = m68k_unused_8_w;
      m68k_memory_map[0xa0].write16 = m68k_unused_16_w;
    }

    /* stop YM2612 */
    fm_reset(cycles);

    /* update Z80 bus status */
    zstate &= 2;
  }
}

void gen_zbank_w (unsigned int data)
{
  zbank = ((zbank >> 1) | ((data & 1) << 23)) & 0xFF8000;
}


/*-----------------------------------------------------------------------*/
/* Z80 interrupt callback                                                */
/* ----------------------------------------------------------------------*/

int z80_irq_callback (int param)
{
  return 0xFF;
}
