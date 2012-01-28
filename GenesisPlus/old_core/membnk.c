/***************************************************************************************
 *  Genesis Plus
 *  M68k Bank access from Z80
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

/*
  Handlers for access to unused addresses and those which make the
  machine lock up.
*/

uint32 zbank_unused_r(uint32 address)
{
#ifdef LOGERROR
  error("Z80 bank unused read %06X\n", address);
#endif
  return (address & 1) ? 0x00 : 0xFF;
}

void zbank_unused_w(uint32 address, uint32 data)
{
#ifdef LOGERROR
  error("Z80 bank unused write %06X = %02X\n", address, data);
#endif
}

uint32 zbank_lockup_r(uint32 address)
{
#ifdef LOGERROR
  error("Z80 bank lockup read %06X\n", address);
#endif
   gen_running = config.force_dtack;
  return 0xFF;
}

void zbank_lockup_w(uint32 address, uint32 data)
{
#ifdef LOGERROR
  error("Z80 bank lockup write %06X = %02X\n", address, data);
#endif
   gen_running = config.force_dtack;
}

/* I/O & Control registers */
uint32 zbank_read_ctrl_io(uint32 address)
{
  switch ((address >> 8) & 0xff)
  {
    case 0x00:  /* I/O chip */
      if (address & 0xe0) return zbank_unused_r(address);
      else return (io_read((address >> 1) & 0x0f));

    case 0x11:  /* BUSACK */
      if (address & 1) return zbank_unused_r(address);
      else return (0xfe | zbusack);

    case 0x30:  /* TIME */
      if (cart_hw.time_r) return cart_hw.time_r(address);
      else return zbank_unused_r(address);

    case 0x10:  /* MEMORY MODE */
    case 0x12:  /* RESET */
    case 0x20:  /* MEGA-CD */
    case 0x40:  /* TMSS */
    case 0x41:  /* BOOTROM */
    case 0x44:  /* RADICA */
    case 0x50:  /* SVP REGISTERS */
      return zbank_unused_r(address);

    default:  /* Invalid address */
      return zbank_lockup_r(address);
  }
}

void zbank_write_ctrl_io(uint32 address, uint32 data)
{
  switch ((address >> 8) & 0xff)
  {
    case 0x00:  /* I/O chip */
      if ((address & 0xe1) == 0x01) io_write((address >> 1) & 0x0f, data); /* get /LWR only */
      else zbank_unused_w(address, data);
      return;

    case 0x11:  /* BUSREQ */
      if (address & 1) zbank_unused_w(address, data);
      else gen_busreq_w(data & 1);
      return;

    case 0x12:  /* RESET */
      if (address & 1) zbank_unused_w(address, data);
      else gen_reset_w(data & 1);
      return;

    case 0x30:  /* TIME */
      if (cart_hw.time_w) cart_hw.time_w(address, data);
      else zbank_unused_w(address, data);
      return;

    case 0x41:  /* BOOTROM */
      if (address & 1)
      {
        m68k_memory_map[0].base = (data & 1) ?  default_rom : bios_rom;
    
        /* autodetect BIOS ROM file */
        if (!(config.bios_enabled & 2))
        {
          config.bios_enabled |= 2;
          memcpy(bios_rom, cart_rom, 0x800);
          memset(cart_rom, 0, genromsize);
        }
      }
      else zbank_unused_w (address, data);
      return;

    case 0x10:  /* MEMORY MODE */
    case 0x20:  /* MEGA-CD */
    case 0x40:  /* TMSS */
    case 0x44:  /* RADICA */
    case 0x50:  /* SVP REGISTERS */
      zbank_unused_w(address, data);
      return;

    default:  /* Invalid address */
      zbank_lockup_w(address, data);
      return;
  }
}


/* VDP */
uint32 zbank_read_vdp(uint32 address)
{
  switch (address & 0xfd)
  {
    case 0x00:    /* DATA */
      return (vdp_data_r() >> 8);
      
    case 0x01:    /* DATA */
      return (vdp_data_r() & 0xff);
      
    case 0x04:    /* CTRL */
      return (0xfc | ((vdp_ctrl_r() >> 8) & 3));

    case 0x05:    /* CTRL */
      return (vdp_ctrl_r() & 0xff);
      
    case 0x08:    /* HVC */
    case 0x0c:
      return (vdp_hvc_r() >> 8);
      
    case 0x09:    /* HVC */
    case 0x0d:
      return (vdp_hvc_r() & 0xff);
        
    case 0x18:    /* Unused */
    case 0x19:
    case 0x1c:
    case 0x1d:
      return zbank_unused_r(address);

    default:    /* Invalid address */
      return zbank_lockup_r(address);
  }
}

void zbank_write_vdp(uint32 address, uint32 data)
{
  switch (address & 0xfc)
  {
    case 0x00:  /* Data port */
      vdp_data_w(data << 8 | data);
      return;

    case 0x04:  /* Control port */
      vdp_ctrl_w(data << 8 | data);
      return;

    case 0x10:  /* PSG */
    case 0x14:
      if (address & 1) psg_write(0, data);
      else zbank_unused_w(address, data);
      return;
              
    case 0x18: /* Unused */
      zbank_unused_w(address, data);
      return;

    case 0x1c:  /* TEST register */
      vdp_test_w(data << 8 | data);
      return;

    default:  /* Invalid address */
      zbank_lockup_w(address, data);
      return;
  }
}
