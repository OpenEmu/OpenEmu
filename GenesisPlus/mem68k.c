/***************************************************************************************
 *  Genesis Plus
 *  68k memory handlers
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

#include "m68kcpu.h"
#include "shared.h"

uint32 m68k_read_bus_8(uint32 address)
{
#ifdef LOGERROR
  error("Unused read8 %08X (%08X)\n", address, m68k_get_reg (NULL, M68K_REG_PC));
#endif
  return m68k_read_pcrelative_8(REG_PC | (address&1));

}

uint32 m68k_read_bus_16(uint32 address)
{
#ifdef LOGERROR
  error("Unused read16 %08X (%08X)\n", address, m68k_get_reg (NULL, M68K_REG_PC));
#endif
  return m68k_read_pcrelative_16(REG_PC);
}


void m68k_unused_8_w (uint32 address, uint32 data)
{
#ifdef LOGERROR
  error("Unused write8 %08X = %02X (%08X)\n", address, data, m68k_get_reg (NULL, M68K_REG_PC));
#endif
}

void m68k_unused_16_w (uint32 address, uint32 data)
{
#ifdef LOGERROR
  error("Unused write16 %08X = %04X (%08X)\n", address, data, m68k_get_reg (NULL, M68K_REG_PC));
#endif
}

/*
  Functions to handle memory accesses which cause the Genesis to halt
  either temporarily (press RESET button to restart) or unrecoverably
  (cycle power to restart).
*/

void m68k_lockup_w_8 (uint32 address, uint32 data)
{
#ifdef LOGERROR
  error ("Lockup %08X = %02X (%08X)\n", address, data, m68k_get_reg (NULL, M68K_REG_PC));
#endif
  gen_running = config.force_dtack;
  if (!gen_running) m68k_end_timeslice ();
}

void m68k_lockup_w_16 (uint32 address, uint32 data)
{
#ifdef LOGERROR
  error ("Lockup %08X = %04X (%08X)\n", address, data, m68k_get_reg (NULL, M68K_REG_PC));
#endif
  gen_running = config.force_dtack;
  if (!gen_running) m68k_end_timeslice ();
}

uint32 m68k_lockup_r_8 (uint32 address)
{ 
#ifdef LOGERROR
  error ("Lockup %08X.b (%08X)\n", address, m68k_get_reg (NULL, M68K_REG_PC));
#endif
  gen_running = config.force_dtack;
  if (!gen_running) m68k_end_timeslice ();
  return -1;
}

uint32 m68k_lockup_r_16 (uint32 address)
{
#ifdef LOGERROR
  error ("Lockup %08X.w (%08X)\n", address, m68k_get_reg (NULL, M68K_REG_PC));
#endif
  gen_running = config.force_dtack;
  if (!gen_running) m68k_end_timeslice ();
  return -1;
}

/* PICO data */
static int pico_page[7] = {0x00,0x01,0x03,0x07,0x0F,0x1F,0x3F};

/*--------------------------------------------------------------------------*/
/* 68000 memory handlers                                                    */
/*--------------------------------------------------------------------------*/

/******* EEPROM  **********************************************/

uint32 eeprom_read_byte(uint32 address)
{
  if (address == eeprom.type.sda_out_adr) return eeprom_read(address, 0);
  else return READ_BYTE(cart_rom, address);
}

uint32 eeprom_read_word(uint32 address)
{
  if (address == (eeprom.type.sda_out_adr & 0xfffffe)) return eeprom_read(address, 1);
  else return *(uint16 *)(cart_rom + address);
}

void eeprom_write_byte(uint32 address, uint32 data)
{
  if ((address == eeprom.type.sda_in_adr) || (address == eeprom.type.scl_adr))
    eeprom_write(address, data, 0);
  else m68k_unused_8_w(address, data);
}

void eeprom_write_word(uint32 address, uint32 data)
{
  if ((address == (eeprom.type.sda_in_adr&0xfffffe)) || (address == (eeprom.type.scl_adr&0xfffffe)))
    eeprom_write(address, data, 1);
  else m68k_unused_16_w (address, data);
}


/******* Z80  *************************************************/

uint32 z80_read_byte(uint32 address)
{
  if (zbusack) return m68k_read_bus_8(address);
          
  switch ((address >> 13) & 3)
  {
    case 2:   /* YM2612 */
      return fm_read(0, address & 3);

    case 3:   /* MISC */
      if ((address & 0xff00) == 0x7f00) return m68k_lockup_r_8(address); /* VDP */
      else return (m68k_read_bus_8(address) | 0xff);

    default: /* ZRAM */
      return zram[address & 0x1fff];
  }
}
            
uint32 z80_read_word(uint32 address)
{
  if (zbusack) return m68k_read_bus_16(address);
      
  switch ((address >> 13) & 3)
  {
    case 2:   /* YM2612 */
    {
      int temp = fm_read(0, address & 3);
      return (temp << 8 | temp);
    }

    case 3:   /* MISC */
      if ((address & 0xff00) == 0x7f00) return m68k_lockup_r_16(address); /* VDP */
      else return (m68k_read_bus_16(address) | 0xffff);

    default:  /* ZRAM */
    {
      int temp = zram[address & 0x1fff];
      return (temp << 8 | temp);
    }
  }
}
      
void z80_write_byte(uint32 address, uint32 data)
{
  if (zbusack)
  {
    m68k_unused_8_w(address, data);
    return;
  }
      
  switch ((address >> 13) & 3)
  {
    case 2: /* YM2612 */
      fm_write(0, address & 3, data);
      return;
      
    case 3:
      switch ((address >> 8) & 0x7f)
      {
        case 0x60:  /* Bank register */
          gen_bank_w(data & 1);
          return;

        case 0x7f:  /* VDP */
          m68k_lockup_w_8(address, data);
          return;
      
        default:
          m68k_unused_8_w(address, data);
          return;
      }
      
    default: /* ZRAM */
      zram[address & 0x1fff] = data;
      count_m68k++; /* Z80 bus latency (Pacman 2: New Adventures) */
      return;
  }
}

void z80_write_word(uint32 address, uint32 data)
{
  /* Z80 still hold the bus ? */
  if (zbusack)
  {
    m68k_unused_16_w(address, data);
    return;
  }

  switch ((address >> 13) & 3)
  {
    case 2: /* YM2612 */
      fm_write (0, address & 3, data >> 8);
      return;

    case 3:
      switch ((address >> 8) & 0x7f)
      {
        case 0x60:  /* Bank register */
          gen_bank_w ((data >> 8) & 1);
          return;

        case 0x7f:  /* VDP */
          m68k_lockup_w_16(address, data);
          return;
        
        default:
          m68k_unused_16_w(address, data);
          return;
      }

    default: /* ZRAM */
      zram[address & 0x1fff] = data >> 8;
      return;
  }
}

        
/******* I/O & CTRL ******************************************/
          
uint32 ctrl_io_read_byte(uint32 address)
{
  switch ((address >> 8) & 0xff)
  {
    case 0x00:  /* I/O chip */
      if (address & 0xe0) return m68k_read_bus_8(address);
      else return (io_read((address >> 1) & 0x0f));

    case 0x11:  /* BUSACK */
      if (address & 1) return m68k_read_bus_8(address);
      else return ((m68k_read_pcrelative_8(REG_PC) & 0xfe) | zbusack);

    case 0x30:  /* TIME */
      if (cart_hw.time_r) return cart_hw.time_r(address);
      else return m68k_read_bus_8(address);

    case 0x10:  /* MEMORY MODE */
    case 0x12:  /* RESET */
    case 0x20:  /* MEGA-CD */
    case 0x40:  /* TMSS */
    case 0x41:  /* BOOTROM */
    case 0x44:  /* RADICA */
    case 0x50:  /* SVP REGISTERS */
      return m68k_read_bus_8(address);

    default:  /* Invalid address */
      return m68k_lockup_r_8(address);
  }
}

uint32 ctrl_io_read_word(uint32 address)
{
  switch ((address >> 8) & 0xff)
  {
    case 0x00:  /* I/O chip */
    {
      if (address & 0xe0) return m68k_read_bus_16(address);
      int temp = io_read((address >> 1) & 0x0f);
      return (temp << 8 | temp);
    }

    case 0x11:  /* BUSACK */
      return ((m68k_read_pcrelative_16(REG_PC) & 0xfeff) | (zbusack << 8));

    case 0x30:  /* TIME */
      if (cart_hw.time_r) return cart_hw.time_r(address);
      else return m68k_read_bus_16(address);

    case 0x50:  /* SVP */
      if (svp)
      {
        if ((address & 0xfd) == 0) return svp->ssp1601.gr[SSP_XST].h;
        else if ((address & 0xff) == 4)
        {
          uint32 temp = svp->ssp1601.gr[SSP_PM0].h;
          svp->ssp1601.gr[SSP_PM0].h &= ~1;
          return temp;
        }
      }
      return m68k_read_bus_16(address);

    case 0x10:  /* MEMORY MODE */
    case 0x12:  /* RESET */
    case 0x20:  /* MEGA-CD */
    case 0x40:  /* TMSS */
    case 0x41:  /* BOOTROM */
    case 0x44:  /* RADICA */
      return m68k_read_bus_16(address);
    
    default:  /* Invalid address */
      return m68k_lockup_r_16(address);
  }
}

void ctrl_io_write_byte(uint32 address, uint32 data)
{        
  switch ((address >> 8) & 0xff)
  {
    case 0x00:  /* I/O chip */
      if ((address & 0xe1) == 0x01) io_write((address >> 1) & 0x0f, data); /* get /LWR only */
      else m68k_unused_8_w(address, data);
      return;

    case 0x11:  /* BUSREQ */
      if (address & 1) m68k_unused_8_w(address, data);
      else gen_busreq_w(data & 1);
      return;

    case 0x12:  /* RESET */
      if (address & 1) m68k_unused_8_w(address, data);
      else gen_reset_w(data & 1);
      return;

    case 0x30:  /* TIME */
      cart_hw.time_w(address, data);
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
      else m68k_unused_8_w (address, data);
      return;

    case 0x10:  /* MEMORY MODE */
    case 0x20:  /* MEGA-CD */
    case 0x40:  /* TMSS */
    case 0x44:  /* RADICA */
    case 0x50:  /* SVP REGISTERS */
      m68k_unused_8_w(address, data);
      return;

    default:  /* Invalid address */
      m68k_lockup_w_8(address, data);
      return;
  }
}

void ctrl_io_write_word(uint32 address, uint32 data)
{        
  switch ((address >> 8) & 0xff)
  {
    case 0x00:  /* I/O chip */
      if (address & 0xe0) m68k_unused_16_w (address, data);
      else io_write ((address >> 1) & 0x0f, data & 0xff);
      return;
        
    case 0x11:  /* BUSREQ */
      gen_busreq_w ((data >> 8) & 1);
      return;
        
    case 0x12:  /* RESET */
      gen_reset_w ((data >> 8) & 1);
      return;
            
    case 0x50:  /* SVP REGISTERS */
      if (svp)
      {
        if (address & 0xfd) m68k_unused_16_w(address, data);
        else
        {
          /* just guessing here (Notaz) */
          svp->ssp1601.gr[SSP_XST].h = data;
          svp->ssp1601.gr[SSP_PM0].h |= 2;
          svp->ssp1601.emu_status &= ~SSP_WAIT_PM0;
        }
      }
      else m68k_unused_16_w (address, data);
      return;

    case 0x30:  /* TIME */
      cart_hw.time_w(address & 0xfe, data >> 8);
      cart_hw.time_w(address, data & 0xff);
      return;
          
    case 0x41:  /* BOOTROM */
                
      m68k_memory_map[0].base = (data & 1) ?  default_rom : bios_rom;

      /* autodetect BIOS ROM file */
      if (!(config.bios_enabled & 2))
      {
        config.bios_enabled |= 2;
        memcpy(bios_rom, cart_rom, 0x800);
        memset(cart_rom, 0, genromsize);
      }
      return;
          
    case 0x10:  /* MEMORY MODE */
    case 0x20:  /* MEGA-CD */
    case 0x40:  /* TMSS */
    case 0x44:  /* RADICA */
      m68k_unused_16_w (address, data);
      return;
            
    default:  /* Unused */
      m68k_lockup_w_16 (address, data);
      return;
  }
}


/******* VDP *************************************************/

uint32 vdp_read_byte(uint32 address)
{
  switch (address & 0xfd)
  {
    case 0x00:  /* DATA */
      return (vdp_data_r() >> 8);
          
    case 0x01:  /* DATA */
      return (vdp_data_r() & 0xff);
  
    case 0x04:  /* CTRL */
      return ((m68k_read_pcrelative_8(REG_PC) & 0xfc) | ((vdp_ctrl_r() >> 8) & 3));

    case 0x05:  /* CTRL */
      return (vdp_ctrl_r() & 0xff);
          
    case 0x08:  /* HVC */
    case 0x0c:
      return (vdp_hvc_r() >> 8);
          
    case 0x09:  /* HVC */
    case 0x0d:
      return (vdp_hvc_r() & 0xff);

    case 0x18:  /* Unused */
    case 0x19:
    case 0x1c:
    case 0x1d:
      return m68k_read_bus_8(address);

    default:    /* Invalid address */
      return m68k_lockup_r_8(address);
  }
}

uint32 vdp_read_word(uint32 address)
{
  switch (address & 0xfc)
  {
    case 0x00:  /* DATA */
      return vdp_data_r();
            
    case 0x04:  /* CTRL */
      return ((vdp_ctrl_r() & 0x3FF) | (m68k_read_pcrelative_16(REG_PC) & 0xFC00));
          
    case 0x08:  /* HVC */
    case 0x0c:
      return vdp_hvc_r();
          
    case 0x18:  /* Unused */
    case 0x1c:
      return m68k_read_bus_16(address);

    default:    /* Invalid address */
      return m68k_lockup_r_16(address);
  }
}

void vdp_write_byte(uint32 address, uint32 data)
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
      else m68k_unused_8_w(address, data);
      return;
              
    case 0x18: /* Unused */
      m68k_unused_8_w(address, data);
      return;

    case 0x1c:  /* TEST register */
      vdp_test_w(data << 8 | data);
      return;

    default:  /* Invalid address */
      m68k_lockup_w_8(address, data);
      return;
  }
}

void vdp_write_word(uint32 address, uint32 data)
{
  switch (address & 0xfc)
  {
    case 0x00:  /* DATA */
      vdp_data_w(data);
      return;

    case 0x04:  /* CTRL */
      vdp_ctrl_w(data);
      return;

    case 0x10:  /* PSG */
    case 0x14:
      psg_write(0, data & 0xff);
      return;

    case 0x18:  /* Unused */
      m68k_unused_16_w(address, data);
      return;
    
    case 0x1c:  /* Test register */
      vdp_test_w(data);
      return;

    default:  /* Invalid address */
      m68k_lockup_w_16 (address, data);
      return;
  }
}


/******* PICO ************************************************/

uint32 pico_read_byte(uint32 address)
{
  /* PICO */
  switch (address & 0xff)
  {
    case 0x01:  /* VERSION register */
      return (0x40);

    case 0x03:  /* IO register */
    {
      uint8 retval = 0xff;
      if (input.pad[0] & INPUT_B)     retval &= ~0x10;
      if (input.pad[0] & INPUT_A)     retval &= ~0x80;
      if (input.pad[0] & INPUT_UP)    retval &= ~0x01;
      if (input.pad[0] & INPUT_DOWN)  retval &= ~0x02;
      if (input.pad[0] & INPUT_LEFT)  retval &= ~0x04;
      if (input.pad[0] & INPUT_RIGHT) retval &= ~0x08;
      retval &= ~0x20;
      retval &= ~0x40;
      return retval;
    }

    case 0x05:  /* MSB PEN X coordinate */
      return (input.analog[0][0] >> 8);

    case 0x07:  /* LSB PEN X coordinate */
      return (input.analog[0][0] & 0xff);

    case 0x09:  /* MSB PEN Y coordinate */
      return (input.analog[0][1] >> 8);

    case 0x0b:  /* LSB PEN Y coordinate */
      return (input.analog[0][1] & 0xff);

    case 0x0d:  /* PAGE register */
      return pico_page[pico_current]; /* TODO */

    case 0x10:  /* PCM registers */
    case 0x11:
    case 0x12:
    case 0x13:
      return 0x80; /* TODO */
        
        default:
      return m68k_read_bus_8(address);
  }
}

uint32 pico_read_word(uint32 address)
{
  return ((m68k_read_bus_8(address) << 8) | pico_read_byte(address | 1));
}
