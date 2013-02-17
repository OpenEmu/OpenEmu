/***************************************************************************************
 *  Genesis Plus
 *  Z80 bus handlers (Genesis & Master System modes)
 *
 *  Support for SG-1000, Mark-III, Master System, Game Gear & Mega Drive ports access
 *
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald (original code)
 *  Copyright (C) 2007-2011  Eke-Eke (Genesis Plus GX)
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
/*  Handlers for access to unused addresses and those which make the        */
/*  machine lock up.                                                        */
/*--------------------------------------------------------------------------*/

INLINE void z80_unused_w(unsigned int address, unsigned char data)
{
#ifdef LOGERROR
  error("Z80 unused write %04X = %02X (%x)\n", address, data, Z80.pc.w.l);
#endif
}

INLINE unsigned char z80_unused_r(unsigned int address)
{
#ifdef LOGERROR
  error("Z80 unused read %04X (%x)\n", address, Z80.pc.w.l);
#endif
  return 0xFF;
}

INLINE void z80_lockup_w(unsigned int address, unsigned char data)
{
#ifdef LOGERROR
  error("Z80 lockup write %04X = %02X (%x)\n", address, data, Z80.pc.w.l);
#endif
  if (!config.force_dtack)
  {
    Z80.cycles = 0xFFFFFFFF;
    zstate = 0;
  }
}

INLINE unsigned char z80_lockup_r(unsigned int address)
{
#ifdef LOGERROR
  error("Z80 lockup read %04X (%x)\n", address, Z80.pc.w.l);
#endif
  if (!config.force_dtack)
  {
    Z80.cycles = 0xFFFFFFFF;
    zstate = 0;
  }
  return 0xFF;
}


/*--------------------------------------------------------------------------*/
/*  Z80 Memory handlers (Genesis mode)                                      */
/*--------------------------------------------------------------------------*/

unsigned char z80_memory_r(unsigned int address)
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
      return fm_read(Z80.cycles, address & 3);
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
      if (zbank_memory_map[address >> 16].read)
      {
        return (*zbank_memory_map[address >> 16].read)(address);
      }
      return READ_BYTE(m68k.memory_map[address >> 16].base, address & 0xFFFF);
    }
  }
}


void z80_memory_w(unsigned int address, unsigned char data)
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
      fm_write(Z80.cycles, address & 3, data);
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
      if (zbank_memory_map[address >> 16].write)
      {
        (*zbank_memory_map[address >> 16].write)(address, data);
        return;
      }
      WRITE_BYTE(m68k.memory_map[address >> 16].base, address & 0xFFFF, data);
      return;
    }
  }
}

/*--------------------------------------------------------------------------*/
/*  Unused Port handlers                                                    */
/*                                                                          */
/*  Ports are unused when not in Mark III compatibility mode.               */
/*                                                                          */
/*  Genesis games that access ports anyway:                                 */
/*    Thunder Force IV reads port $BF in it's interrupt handler.            */
/*                                                                          */
/*--------------------------------------------------------------------------*/

unsigned char z80_unused_port_r(unsigned int port)
{
#if LOGERROR
  error("Z80 unused read from port %04X (%x)\n", port, Z80.pc.w.l);
#endif
  if (system_hw == SYSTEM_SMS)
  {
    unsigned int address = (Z80.pc.w.l - 1) & 0xFFFF;
    return z80_readmap[address >> 10][address & 0x3FF];
  }
  return 0xFF;
}

void z80_unused_port_w(unsigned int port, unsigned char data)
{
#if LOGERROR
  error("Z80 unused write to port %04X = %02X (%x)\n", port, data, Z80.pc.w.l);
#endif
}

/*--------------------------------------------------------------------------*/
/* MegaDrive / Genesis port handlers (Master System compatibility mode)     */
/*--------------------------------------------------------------------------*/

void z80_md_port_w(unsigned int port, unsigned char data)
{
  switch (port & 0xC1)
  {
    case 0x01:
    {
      io_z80_write(1, data, Z80.cycles + PBC_CYCLE_OFFSET);
      return;
    }

    case 0x40:
    case 0x41:
    {
      SN76489_Write(Z80.cycles, data);
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
      port &= 0xFF;

      if ((port >= 0xF0) && (config.ym2413 & 1))
      {
        fm_write(Z80.cycles, port&3, data);
        return;
      }

      z80_unused_port_w(port, data);
      return;
    }
  }
}

unsigned char z80_md_port_r(unsigned int port)
{
  switch (port & 0xC1)
  {
    case 0x40:
    {
      return ((vdp_hvc_r(Z80.cycles - 15) >> 8) & 0xFF);
    }

    case 0x41:
    {
      return (vdp_hvc_r(Z80.cycles - 15) & 0xFF);
    }

    case 0x80:
    {
      return vdp_z80_data_r();
    }

    case 0x81:
    {
      return vdp_z80_ctrl_r(Z80.cycles);
    }

    default:
    {
      port &= 0xFF;

      if ((port == 0xC0) || (port == 0xC1) || (port == 0xDC) || (port == 0xDD))
      {
        return io_z80_read(port & 1);
      }

      /* read FM chip if enabled */
      if ((port >= 0xF0) && (config.ym2413 & 1))
      {
        return YM2413Read(port & 3); 
      }

      return z80_unused_port_r(port);
    }
  }
}


/*--------------------------------------------------------------------------*/
/* Game Gear port handlers                                                  */
/*--------------------------------------------------------------------------*/

void z80_gg_port_w(unsigned int port, unsigned char data)
{
  switch(port & 0xC1)
  {
    case 0x00:
    case 0x01:
    {
      port &= 0xFF;

      if (port < 0x07)
      {
        if (system_hw == SYSTEM_GG)
        {
          io_gg_write(port, data);
          return;
        }

        z80_unused_port_w(port & 0xFF, data);
        return;
      }

      io_z80_write(port & 1, data, Z80.cycles + SMS_CYCLE_OFFSET);
      return;
    }

    case 0x40:
    case 0x41:
    {
      SN76489_Write(Z80.cycles, data);
      return;
    }

    case 0x80:
    {
      vdp_z80_data_w(data);
      return;
    }

    case 0x81:
    {
      vdp_sms_ctrl_w(data);
      return;
    }

    default:
    {
      z80_unused_port_w(port & 0xFF, data);
      return;
    }
  }
}

unsigned char z80_gg_port_r(unsigned int port)
{
  switch(port & 0xC1)
  {
    case 0x00:
    case 0x01:
    {
      port &= 0xFF;

      if (port < 0x07)
      {
        if (system_hw == SYSTEM_GG)
        {
          return io_gg_read(port);
        }
      }

      return z80_unused_port_r(port);
    }

    case 0x40:
    {
      return ((vdp_hvc_r(Z80.cycles) >> 8) & 0xFF);
    }

    case 0x41:
    {
      return (vdp_hvc_r(Z80.cycles) & 0xFF);
    }

    case 0x80:
    {
      return vdp_z80_data_r();
    }

    case 0x81:
    {
      return vdp_z80_ctrl_r(Z80.cycles);
    }

    default:
    {
      port &= 0xFF;

      if ((port == 0xC0) || (port == 0xC1) || (port == 0xDC) || (port == 0xDD))
      {
        return io_z80_read(port & 1);
      }

      return z80_unused_port_r(port);
    }
  }
}


/*--------------------------------------------------------------------------*/
/* Master System port handlers                                              */
/*--------------------------------------------------------------------------*/

void z80_ms_port_w(unsigned int port, unsigned char data)
{
  switch (port & 0xC1)
  {
    case 0x00:
    case 0x01:
    {
      io_z80_write(port & 1, data, Z80.cycles + SMS_CYCLE_OFFSET);
      return;
    }

    case 0x40:
    case 0x41:
    {
      SN76489_Write(Z80.cycles, data);
      return;
    }

    case 0x80:
    {
      vdp_z80_data_w(data);
      return;
    }

    case 0x81:
    {
      vdp_sms_ctrl_w(data);
      return;
    }

    default:
    {
      if (!(port & 4) && (config.ym2413 & 1))
      {
        fm_write(Z80.cycles, port & 3, data);
        return;
      }

      z80_unused_port_w(port & 0xFF, data);
      return;
    }
  }
}

unsigned char z80_ms_port_r(unsigned int port)
{
  switch (port & 0xC1)
  {
    case 0x00:
    case 0x01:
    {
      return z80_unused_port_r(port & 0xFF);
    }

    case 0x40:
    {
      return ((vdp_hvc_r(Z80.cycles) >> 8) & 0xFF);
    }

    case 0x41:
    {
      return (vdp_hvc_r(Z80.cycles) & 0xFF);
    }

    case 0x80:
    {
      return vdp_z80_data_r();
    }

    case 0x81:
    {
      return vdp_z80_ctrl_r(Z80.cycles);
    }

    default:
    {
      /* read FM chip if enabled */
      if (!(port & 4) && (config.ym2413 & 1))
      {
        /* check if I/O ports are disabled */
        if (io_reg[0x0E] & 0x04)
        {
          return YM2413Read(port & 3);
        }
        else
        {
          return YM2413Read(port & 3) & io_z80_read(port & 1);
        }
      }

      /* check if I/O ports are enabled */
      if (!(io_reg[0x0E] & 0x04))
      {
        return io_z80_read(port & 1);
      }

      return z80_unused_port_r(port & 0xFF);
    }
  }
}


/*--------------------------------------------------------------------------*/
/* Mark III port handlers                                                   */
/*--------------------------------------------------------------------------*/

void z80_m3_port_w(unsigned int port, unsigned char data)
{
  switch (port & 0xC1)
  {
    case 0x00:
    case 0x01:
    {
      z80_unused_port_w(port, data);
      return;
    }

    case 0x40:
    case 0x41:
    {
      SN76489_Write(Z80.cycles, data);
      return;
    }

    case 0x80:
    {
      vdp_z80_data_w(data);
      return;
    }

    case 0x81:
    {
      vdp_sms_ctrl_w(data);
      return;
    }

    default:
    {
      if (!(port & 4) && (config.ym2413 & 1))
      {
        fm_write(Z80.cycles, port & 3, data);
        return;
      }

      z80_unused_port_w(port & 0xFF, data);
      return;
    }
  }
}

unsigned char z80_m3_port_r(unsigned int port)
{
  switch (port & 0xC1)
  {
    case 0x00:
    case 0x01:
    {
      return z80_unused_port_r(port & 0xFF);
    }

    case 0x40:
    {
      return ((vdp_hvc_r(Z80.cycles) >> 8) & 0xFF);
    }

    case 0x41:
    {
      return (vdp_hvc_r(Z80.cycles) & 0xFF);
    }

    case 0x80:
    {
      return vdp_z80_data_r();
    }

    case 0x81:
    {
      return vdp_z80_ctrl_r(Z80.cycles);
    }

    default:
    {
      /* read FM chip if enabled */
      if (!(port & 4) && (config.ym2413 & 1))
      {
        /* I/O ports are automatically disabled */
        return YM2413Read(port & 3);
      }

      /* read I/O ports   */
      return io_z80_read(port & 1);
    }
  }
}


/*--------------------------------------------------------------------------*/
/* SG-1000 port handlers                                                     */
/*--------------------------------------------------------------------------*/

void z80_sg_port_w(unsigned int port, unsigned char data)
{
  switch(port & 0xC1)
  {
    case 0x40:
    case 0x41:
    {
      SN76489_Write(Z80.cycles, data);
      return;
    }

    case 0x80:
    {
      vdp_z80_data_w(data);
      return;
    }

    case 0x81:
    {
      vdp_tms_ctrl_w(data);
      return;
    }

    default:
    {
      z80_unused_port_w(port & 0xFF, data);
      return;
    }
  }
}

unsigned char z80_sg_port_r(unsigned int port)
{
  switch (port & 0xC1)
  {
    case 0x80:
    {
      return vdp_z80_data_r();
    }

    case 0x81:
    {
      return vdp_z80_ctrl_r(Z80.cycles);
    }

    case 0xC0:
    case 0xC1:
    {
      return io_z80_read(port & 1);
    }

    default:
    {
      return z80_unused_port_r(port);
    }
  }
}
