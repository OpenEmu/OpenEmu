/***************************************************************************************
 *  Genesis Plus
 *  Z80 bank access to 68k bus
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

unsigned int zbank_unused_r(unsigned int address)
{
#ifdef LOGERROR
  error("Z80 bank unused read %06X (%x)\n", address, Z80.pc.d);
#endif
  return (address & 1) ? 0x00 : 0xFF;
}

void zbank_unused_w(unsigned int address, unsigned int data)
{
#ifdef LOGERROR
  error("Z80 bank unused write %06X = %02X\n", address, data);
#endif
}

unsigned int zbank_lockup_r(unsigned int address)
{
#ifdef LOGERROR
  error("Z80 bank lockup read %06X (%x)\n", address, Z80.pc.d);
#endif
  if (!config.force_dtack)
  {
    mcycles_z80 = 0xFFFFFFFF;
    zstate = 0;
  }
  return 0xFF;
}

void zbank_lockup_w(unsigned int address, unsigned int data)
{
#ifdef LOGERROR
  error("Z80 bank lockup write %06X = %02X (%x)\n", address, data, Z80.pc.d);
#endif
  if (!config.force_dtack)
  {
    mcycles_z80 = 0xFFFFFFFF;
    zstate = 0;
  }
}

/* I/O & Control registers */
unsigned int zbank_read_ctrl_io(unsigned int address)
{
  switch ((address >> 8) & 0xFF)
  {
    case 0x00:  /* I/O chip */
    {
      if (!(address & 0xE0))
      {
        return (io_68k_read((address >> 1) & 0x0F));
      }
      return zbank_unused_r(address);
    }

    case 0x11:  /* BUSACK */
    {
      if (address & 1)
      {
        return zbank_unused_r(address);
      }
      return 0xFF;
    }

    case 0x30:  /* TIME */
    {
      if (cart.hw.time_r)
      {
        unsigned int data = cart.hw.time_r(address);
        if (address & 1)
        {
          return (data & 0xFF);
        }
        return (data >> 8);
      }
      return zbank_unused_r(address);
    }

    case 0x41:  /* OS ROM */
    {
      if (address & 1)
      {
        return (gen_bankswitch_r() | 0xFE);
      }
      return zbank_unused_r(address);
    }

    case 0x10:  /* MEMORY MODE */
    case 0x12:  /* RESET */
    case 0x20:  /* MEGA-CD */
    case 0x40:  /* TMSS */
    case 0x44:  /* RADICA */
    case 0x50:  /* SVP REGISTERS */
    {
      return zbank_unused_r(address);
    }

    default:  /* Invalid address */
    {
      return zbank_lockup_r(address);
    }
  }
}

void zbank_write_ctrl_io(unsigned int address, unsigned int data)
{
  switch ((address >> 8) & 0xFF)
  {
    case 0x00:  /* I/O chip */
    {
      /* get /LWR only */
      if ((address & 0xE1) == 0x01)
      {
        io_68k_write((address >> 1) & 0x0F, data);
        return;
      }
      zbank_unused_w(address, data);
      return;
    }

    case 0x11:  /* BUSREQ */
    {
      if (!(address & 1))
      {
        gen_zbusreq_w(data & 1, mcycles_z80);
        return;
      }
      zbank_unused_w(address, data);
      return;
    }

    case 0x12:  /* RESET */
    {
      if (!(address & 1))
      {
        gen_zreset_w(data & 1, mcycles_z80);
        return;
      }
      zbank_unused_w(address, data);
      return;
    }

    case 0x30:  /* TIME */
    {
      cart.hw.time_w(address, data);
      return;
    }

    case 0x41:  /* OS ROM */
    {
      if (address & 1)
      {
        gen_bankswitch_w(data & 1);
        return;
      }
      zbank_unused_w(address, data);
      return;
    }

    case 0x10:  /* MEMORY MODE */
    case 0x20:  /* MEGA-CD */
    case 0x40:  /* TMSS */
    case 0x44:  /* RADICA */
    case 0x50:  /* SVP REGISTERS */
    {
      zbank_unused_w(address, data);
      return;
    }

    default:  /* Invalid address */
    {
      zbank_lockup_w(address, data);
      return;
    }
  }
}


/* VDP */
unsigned int zbank_read_vdp(unsigned int address)
{
  switch (address & 0xFD)
  {
    case 0x00:    /* DATA */
    {
      return (vdp_68k_data_r() >> 8);
    }
      
    case 0x01:    /* DATA */
    {
      return (vdp_68k_data_r() & 0xFF);
    }
      
    case 0x04:    /* CTRL */
    {
      return (((vdp_ctrl_r(mcycles_z80) >> 8) & 3) | 0xFC);
    }

    case 0x05:    /* CTRL */
    {
      return (vdp_ctrl_r(mcycles_z80) & 0xFF);
    }
      
    case 0x08:    /* HVC */
    case 0x0C:
    {
      return (vdp_hvc_r(mcycles_z80) >> 8);
    }

    case 0x09:    /* HVC */
    case 0x0D:
    {
      return (vdp_hvc_r(mcycles_z80) & 0xFF);
    }

    case 0x18:    /* Unused */
    case 0x19:
    case 0x1C:
    case 0x1D:
    {
      return zbank_unused_r(address);
    }

    default:    /* Invalid address */
    {
      return zbank_lockup_r(address);
    }
  }
}

void zbank_write_vdp(unsigned int address, unsigned int data)
{
  switch (address & 0xFC)
  {
    case 0x00:  /* Data port */
    {
      vdp_68k_data_w(data << 8 | data);
      return;
    }

    case 0x04:  /* Control port */
    {
      vdp_68k_ctrl_w(data << 8 | data);
      return;
    }

    case 0x10:  /* PSG */
    case 0x14:
    {
      if (address & 1)
      {
        psg_write(mcycles_z80, data);
        return;
      }
      zbank_unused_w(address, data);
      return;
    }
             
    case 0x18: /* Unused */
    {
      zbank_unused_w(address, data);
      return;
    }

    case 0x1C:  /* TEST register */
    {
      vdp_test_w(data << 8 | data);
      return;
    }

    default:  /* Invalid address */
    {
      zbank_lockup_w(address, data);
      return;
    }
  }
}
