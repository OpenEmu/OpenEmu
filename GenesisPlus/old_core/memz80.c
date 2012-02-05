/***************************************************************************************
 *  Genesis Plus
 *  Z80 memory handler
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
#define LOG_PORT 0      /* 1= Log Z80 I/O port accesses */

/*
  Handlers for access to unused addresses and those which make the
  machine lock up.
*/
static inline void z80_unused_w(unsigned int address, unsigned int data)
{
#ifdef LOGERROR
  error("Z80 unused write %04X = %02X\n", address, data);
#endif
}

static inline unsigned int z80_unused_r(unsigned int address)
{
#ifdef LOGERROR
  error("Z80 unused read %04X\n", address);
#endif
  return 0xff;
}

static inline void z80_lockup_w(unsigned int address, unsigned int data)
{
#ifdef LOGERROR
  error("Z80 lockup write %04X = %02X\n", address, data);
#endif
   gen_running = config.force_dtack;
}

static inline unsigned int z80_lockup_r(unsigned int address)
{
#ifdef LOGERROR
  error("Z80 lockup read %04X\n", address);
#endif
  gen_running = config.force_dtack;
  return 0xff;
}
/*
    VDP access
*/
static inline unsigned int z80_vdp_r(unsigned int address)
{
  switch (address & 0xfd)
  {
    case 0x00:  /* DATA */
      return (vdp_data_r() >> 8);
          
    case 0x01:  /* DATA */
      return (vdp_data_r() & 0xff);

    case 0x04:  /* CTRL */
      return (0xfc | ((vdp_ctrl_r() >> 8) & 3));
          
    case 0x05:  /* CTRL */
      return (vdp_ctrl_r() & 0xff);

    case 0x08:  /* HVC */
    case 0x0c:
      return (vdp_hvc_r() >> 8);

    case 0x09:  /* HVC */
    case 0x0d:
      return (vdp_hvc_r() & 0xff);

    case 0x18: /* Unused */
    case 0x19:
    case 0x1c:
    case 0x1d:
      return z80_unused_r(address);

    default:    /* Invalid address */
      return z80_lockup_r(address);
  }
}

static inline void z80_vdp_w(unsigned int address, unsigned int data)
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
      else z80_unused_w(address, data);
      return;

    case 0x18: /* Unused */
      z80_unused_w(address, data);
      return;

    case 0x1c: /* Test register */
      vdp_test_w(data << 8 | data);
      return;

    default:  /* Invalid address */
      z80_lockup_w(address, data);
      return;
  }
}

/*
    Z80 memory handlers
*/
unsigned int cpu_readmem16(unsigned int address)
{
  switch((address >> 13) & 7)
  {
    case 0: /* Work RAM */
    case 1:
      return zram[address & 0x1fff];

    case 2: /* YM2612 */
      return fm_read(1, address & 3);

    case 3: /* VDP */
      if ((address >> 8) == 0x7f) return z80_vdp_r (address);
      return z80_unused_r(address);

    default: /* V-bus bank */
    {
      address = zbank | (address & 0x7fff);
      int slot = address >> 16;
      if (zbank_memory_map[slot].read) return (*zbank_memory_map[slot].read)(address);
      else return READ_BYTE(m68k_memory_map[slot].base, address&0xffff);
    }
  }
}


void cpu_writemem16(unsigned int address, unsigned int data)
{
  switch((address >> 13) & 7)
  {
    case 0: /* Work RAM */
    case 1: 
      zram[address & 0x1fff] = data;
      return;

    case 2: /* YM2612 */
      fm_write(1, address & 3, data);
      return;

    case 3: /* Bank register and VDP */
      switch(address >> 8)
      {
        case 0x60:
          gen_bank_w(data & 1);
          return;

        case 0x7f:
          z80_vdp_w(address, data);
          return;

        default:
          z80_unused_w(address, data);
          return;
      }
      return;

    default: /* V-bus bank */
    {
      address = zbank | (address & 0x7fff);
      int slot = address >> 16;
      if (zbank_memory_map[slot].write) (*zbank_memory_map[slot].write)(address, data);
      else WRITE_BYTE(m68k_memory_map[slot].base, address&0xffff, data);
      return;
    }
  }
}

/*
  Port handlers. Ports are unused when not in Mark III compatability mode.

  Games that access ports anyway:
    Thunder Force IV reads port $BF in it's interrupt handler.
*/

unsigned int cpu_readport16(unsigned int port)
{
#if LOG_PORT
  error("Z80 read port %04X\n", port);
#endif
  return 0xFF;
}

void cpu_writeport16(unsigned int port, unsigned int data)
{
#if LOG_PORT
  error("Z80 write %02X to port %04X\n", data, port);
#endif
}
