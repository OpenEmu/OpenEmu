/***************************************************************************************
 *  Genesis Plus
 *  I/O Chip
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

uint8 io_reg[0x10];
uint8 region_code = REGION_USA;

/*****************************************************************************
 * I/O chip functions                                                        *
 *                                                                           *
 *****************************************************************************/
struct port_t
{
  void (*data_w)(uint32 data);
  uint32 (*data_r)(void);
} port[3];

void io_reset(void)
{
  /* I/O register default settings */
  uint8 io_def[0x10] =
  {
    0xA0,
    0x7F, 0x7F, 0x7F,
    0x00, 0x00, 0x00,
    0xFF, 0x00, 0x00,
    0xFF, 0x00, 0x00,
    0xFB, 0x00, 0x00,
  };

  /* Initialize I/O registers */
  memcpy (io_reg, io_def, 0x10);

  /* Initialize Port handlers */
  switch (input.system[0])
  {
    case SYSTEM_GAMEPAD:
      port[0].data_w = gamepad_1_write;
      port[0].data_r = gamepad_1_read;
      break;

    case SYSTEM_MOUSE:
      port[0].data_w = mouse_write;
      port[0].data_r = mouse_read;
      break;

    case SYSTEM_WAYPLAY:
      port[0].data_w = wayplay_1_write;  
      port[0].data_r = wayplay_1_read;
      break;

    case SYSTEM_TEAMPLAYER:
      port[0].data_w = teamplayer_1_write;  
      port[0].data_r = teamplayer_1_read;
      break;

    default:
      port[0].data_w = NULL;
      port[0].data_r = NULL;
      break;
  }

  switch (input.system[1])
  {
    case SYSTEM_GAMEPAD:
      port[1].data_w = gamepad_2_write;
      port[1].data_r = gamepad_2_read;
      break;

    case SYSTEM_MOUSE:
      port[1].data_w = mouse_write;
      port[1].data_r = mouse_read;
      break;

    case SYSTEM_MENACER:
      port[1].data_w = NULL;
      port[1].data_r = menacer_read;
      break;

    case SYSTEM_JUSTIFIER:
      port[1].data_w = NULL;
      port[1].data_r = justifier_read;
      break;

    case SYSTEM_WAYPLAY:
      port[1].data_w = wayplay_2_write;  
      port[1].data_r = wayplay_2_read;
      break;

    case SYSTEM_TEAMPLAYER:
      port[1].data_w = teamplayer_2_write;  
      port[1].data_r = teamplayer_2_read;
      break;

    default:
      port[1].data_w = NULL;
      port[1].data_r = NULL;
      break;
  }

  /* External Port (unconnected) */
  port[2].data_w = NULL;
  port[2].data_r = NULL;

  /* Initialize Input Devices */
  input_reset();
}

void io_write(uint32 offset, uint32 value)
{
  switch (offset)
  {
    case 0x01: /* Port A Data */
    case 0x02: /* Port B Data */
    case 0x03: /* Port C Data */
      io_reg[offset] = ((value & 0x80) | (value & io_reg[offset+3]));
      if(port[offset-1].data_w) port[offset-1].data_w(value);
      return;

    case 0x05:      /* Port B Ctrl */
      if (((value & 0x7F) == 0x7F) &&
        ((input.system[0] == SYSTEM_TEAMPLAYER) ||
         (input.system[1] == SYSTEM_TEAMPLAYER)))
      {
        /* autodetect 4-Way play ! */
        input.system[0] = SYSTEM_WAYPLAY;
        input.system[1] = SYSTEM_WAYPLAY;
        port[0].data_w = wayplay_1_write;  
        port[0].data_r = wayplay_1_read;
        port[1].data_w = wayplay_2_write;  
        port[1].data_r = wayplay_2_read;
        input_reset();
      }

    case 0x04:      /* Port A Ctrl */
    case 0x06:      /* Port C Ctrl */
      io_reg[offset] = value & 0xFF;
      io_reg[offset-3] = ((io_reg[offset-3] & 0x80) | (io_reg[offset-3] & io_reg[offset]));
      return;

    case 0x07:      /* Port A TxData */
    case 0x0A:      /* Port B TxData */
    case 0x0D:      /* Port C TxData */
      io_reg[offset] = value;
      return;

    case 0x09:      /* Port A S-Ctrl */
    case 0x0C:      /* Port B S-Ctrl */
    case 0x0F:      /* Port C S-Ctrl */
      io_reg[offset] = (value & 0xF8);
      return;
  }
}

uint32 io_read(uint32 offset)
{
  switch(offset)
  {
    case 0x00: /* Version register */
    {
      uint8 has_scd = 0x20; /* No Sega CD unit attached */
      uint8 gen_ver = (config.bios_enabled == 3) ? 0x01 : 0x00; /* hardware version */
      return (region_code | has_scd | gen_ver);
    }

    case 0x01: /* Port A Data */
    case 0x02: /* Port B Data */
    case 0x03: /* Port C Data */
    {
      uint8 input = 0x7F;   /* default input state */
      if(port[offset-1].data_r) input = port[offset-1].data_r();
      return (io_reg[offset] | ((~io_reg[offset+3]) & input));
    }

    default:
      return (io_reg[offset]);
  }
}
