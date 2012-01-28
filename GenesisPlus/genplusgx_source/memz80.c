/***************************************************************************************
 *  Genesis Plus
 *  Z80 bus controller (MD & MS compatibility modes)
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

/*--------------------------------------------------------------------------*/
/*  Handlers for access to unused addresses and those which make the        */
/*  machine lock up.                                                        */
/*--------------------------------------------------------------------------*/

static inline void z80_unused_w(unsigned int address, unsigned char data)
{
#ifdef LOGERROR
  error("Z80 unused write %04X = %02X (%x)\n", address, data, Z80.pc.w.l);
#endif
}

static inline unsigned char z80_unused_r(unsigned int address)
{
#ifdef LOGERROR
  error("Z80 unused read %04X (%x)\n", address, Z80.pc.w.l);
#endif
  return 0xFF;
}

static inline void z80_lockup_w(unsigned int address, unsigned char data)
{
#ifdef LOGERROR
  error("Z80 lockup write %04X = %02X (%x)\n", address, data, Z80.pc.w.l);
#endif
  if (!config.force_dtack)
  {
    mcycles_z80 = 0xFFFFFFFF;
    zstate = 0;
  }
}

static inline unsigned char z80_lockup_r(unsigned int address)
{
#ifdef LOGERROR
  error("Z80 lockup read %04X (%x)\n", address, Z80.pc);
#endif
  if (!config.force_dtack)
  {
    mcycles_z80 = 0xFFFFFFFF;
    zstate = 0;
  }
  return 0xFF;
}


/*--------------------------------------------------------------------------*/
/*  Z80 Memory handlers (Genesis mode)                                      */
/*--------------------------------------------------------------------------*/

unsigned char z80_md_memory_r(unsigned int address)
{
  switch((address >> 13) & 7)
  {
    case 0: /* $0000-$3FFF: Z80 RAM (8K mirrored) */
    case 1:
    {
      return zram[address & 0x1FFF];
    }

    case 2: /* $4000-$5FFF: YM2612 */
    {
      return fm_read(mcycles_z80, address & 3);
    }

    case 3: /* $7F00-$7FFF: VDP */
    {
      if ((address >> 8) == 0x7F)
      {
        return (*zbank_memory_map[0xc0].read)(address);
      }
      return z80_unused_r(address);
    }
      
    default: /* $8000-$FFFF: 68k bank (32K) */
    {
      address = zbank | (address & 0x7FFF);
      unsigned int slot = address >> 16;
      if (zbank_memory_map[slot].read)
      {
        return (*zbank_memory_map[slot].read)(address);
      }
      return READ_BYTE(m68k_memory_map[slot].base, address & 0xFFFF);
    }
  }
}


void z80_md_memory_w(unsigned int address, unsigned char data)
{
  switch((address >> 13) & 7)
  {
    case 0: /* $0000-$3FFF: Z80 RAM (8K mirrored) */
    case 1: 
    {
      zram[address & 0x1FFF] = data;
      return;
    }

    case 2: /* $4000-$5FFF: YM2612 */
    {
      fm_write(mcycles_z80, address & 3, data);
      return;
    }

    case 3: /* Bank register and VDP */
    {
      switch(address >> 8)
      {
        case 0x60: /* $6000-$60FF: Bank register */
        {
          gen_zbank_w(data & 1);
          return;
        }

        case 0x7F: /* $7F00-$7FFF: VDP */
        {
          (*zbank_memory_map[0xc0].write)(address, data);
          return;
        }

        default:
        {
          z80_unused_w(address, data);
          return;
        }
      }
    }

    default: /* $8000-$FFFF: 68k bank (32K) */
    {
      address = zbank | (address & 0x7FFF);
      unsigned int slot = address >> 16;
      if (zbank_memory_map[slot].write)
      {
        (*zbank_memory_map[slot].write)(address, data);
        return;
      }
      WRITE_BYTE(m68k_memory_map[slot].base, address & 0xFFFF, data);
      return;
    }
  }
}


/*--------------------------------------------------------------------------*/
/*  Z80 Memory handlers (Master System mode)                                */
/*--------------------------------------------------------------------------*/

unsigned char z80_sms_memory_r(unsigned int address)
{
  return z80_readmap[(address) >> 10][(address) & 0x03FF];
}

/*--------------------------------------------------------------------------*/
/*  Z80 Port handlers                                                       */
/*--------------------------------------------------------------------------*/
/*
  Ports are unused when not in Mark III compatibility mode.

  Genesis games that access ports anyway:
    Thunder Force IV reads port $BF in it's interrupt handler.
*/

unsigned char z80_unused_port_r(unsigned int port)
{
#if LOGERROR
  error("Z80 unused read from port %04X (%x)\n", port, Z80.pc.w.l);
#endif
  return 0xFF;
}

void z80_unused_port_w(unsigned int port, unsigned char data)
{
#if LOGERROR
  error("Z80 unused write to port %04X = %02X (%x)\n", port, data, Z80.pc.w.l);
#endif
}

void z80_sms_port_w(unsigned int port, unsigned char data)
{
  switch (port & 0xC1)
  {
    case 0x01:
    {
      io_z80_write(data);
      return;
    }

    case 0x40:
    case 0x41:
    {
      psg_write(mcycles_z80, data);
      return;
    }

    case 0x80:
    {
      vdp_z80_data_w(data);
      return;
    }

    case 0x81:
    {
      vdp_z80_ctrl_w(data);
      return;
    }

    default:
    {
      if ((port & 0xFF) == 0x3E)
      {
        /* Memory Control Register */
        /* NB: this register does not exist on MD hardware but is partially emulated to support BIOS ROM image files */
        if (data & 0x40)
        {
          /* Assume only BIOS would disable Cartridge ROM */
          if (data & 0x08)
          {
            /* BIOS ROM disabled */
            sms_cart_switch(0);
          }
          else
          {
            /* BIOS ROM enabled */
            sms_cart_switch(1);
          }
        }
        return;
      }
      z80_unused_port_w(port, data);
      return;
    }
  }
}

unsigned char z80_sms_port_r(unsigned int port)
{
  switch (port & 0xC1)
  {
    case 0x40:
    {
      return ((vdp_hvc_r(mcycles_z80) >> 8) & 0xFF);
    }

    case 0x41:
    {
      return (vdp_hvc_r(mcycles_z80) & 0xFF);
    }

    case 0x80:
    {
      return vdp_z80_data_r();
    }

    case 0x81:
    {
      return (vdp_ctrl_r(mcycles_z80) & 0xFF);
    }

    default:
    {
      port &= 0xFF;
      if ((port == 0xC0) || (port == 0xC1) || (port == 0xDC) || (port == 0xDD) || (port == 0xDE) || (port == 0xDF))
      {
        return io_z80_read(port & 1);
      }

      return z80_unused_port_r(port);
    }
  }
}
