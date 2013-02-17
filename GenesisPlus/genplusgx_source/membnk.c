/***************************************************************************************
 *  Genesis Plus
 *  Z80 bank access to 68k bus
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

/*
  Handlers for access to unused addresses and those which make the
  machine lock up.
*/

unsigned int zbank_unused_r(unsigned int address)
{
#ifdef LOGERROR
  error("Z80 bank unused read %06X (%x)\n", address, Z80.pc.d);
#endif
  return 0xFF;
}

void zbank_unused_w(unsigned int address, unsigned int data)
{
#ifdef LOGERROR
  error("Z80 bank unused write %06X = %02X (%x)\n", address, data, Z80.pc.d);
#endif
}

unsigned int zbank_lockup_r(unsigned int address)
{
#ifdef LOGERROR
  error("Z80 bank lockup read %06X (%x)\n", address, Z80.pc.d);
#endif
  if (!config.force_dtack)
  {
    Z80.cycles = 0xFFFFFFFF;
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
    Z80.cycles = 0xFFFFFFFF;
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
        gen_zbusreq_w(data & 1, Z80.cycles);
        return;
      }
      zbank_unused_w(address, data);
      return;
    }

    case 0x12:  /* RESET */
    {
      if (!(address & 1))
      {
        gen_zreset_w(data & 1, Z80.cycles);
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
      if ((config.bios & 1) && (address & 1))
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
      return (((vdp_68k_ctrl_r(Z80.cycles) >> 8) & 3) | 0xFC);
    }

    case 0x05:    /* CTRL */
    {
      return (vdp_68k_ctrl_r(Z80.cycles) & 0xFF);
    }
      
    case 0x08:    /* HVC */
    case 0x0C:
    {
      return (vdp_hvc_r(Z80.cycles) >> 8);
    }

    case 0x09:    /* HVC */
    case 0x0D:
    {
      return (vdp_hvc_r(Z80.cycles) & 0xFF);
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
        SN76489_Write(Z80.cycles, data);
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
