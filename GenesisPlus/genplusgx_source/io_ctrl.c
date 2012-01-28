/***************************************************************************************
 *  Genesis Plus
 *  I/O controller (MD & MS compatibility modes)
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
#include "gamepad.h"
#include "lightgun.h"
#include "mouse.h"
#include "activator.h"
#include "xe_a1p.h"
#include "teamplayer.h"
#include "paddle.h"
#include "sportspad.h"

uint8 io_reg[0x10];

uint8 region_code = REGION_USA;

static struct port_t
{
  void (*data_w)(unsigned char data, unsigned char mask);
  unsigned char (*data_r)(void);
} port[3];

static void dummy_write(unsigned char data, unsigned char mask)
{
}

static unsigned char dummy_read(void)
{
  return 0x7F;
}

/*****************************************************************************
 * I/O chip functions                                                        *
 *                                                                           *
 *****************************************************************************/
void io_init(void)
{
  /* Initialize connected peripherals */
  input_init();

  /* Initialize IO Ports handlers & connected peripherals */
  switch (input.system[0])
  {
    case SYSTEM_MS_GAMEPAD:
    {
      port[0].data_w = dummy_write;
      port[0].data_r = gamepad_1_read;
      break;
    }

    case SYSTEM_MD_GAMEPAD:
    {
      port[0].data_w = gamepad_1_write;
      port[0].data_r = gamepad_1_read;
      break;
    }

    case SYSTEM_MOUSE:
    {
      port[0].data_w = mouse_write;
      port[0].data_r = mouse_read;
      break;
    }

    case SYSTEM_ACTIVATOR:
    {
      port[0].data_w = activator_1_write;
      port[0].data_r = activator_1_read;
      break;
    }

    case SYSTEM_XE_A1P:
    {
      port[0].data_w = xe_a1p_write;
      port[0].data_r = xe_a1p_read;
      break;
    }

    case SYSTEM_WAYPLAY:
    {
      port[0].data_w = wayplay_1_write;
      port[0].data_r = wayplay_1_read;
      break;
    }

    case SYSTEM_TEAMPLAYER:
    {
      port[0].data_w = teamplayer_1_write;
      port[0].data_r = teamplayer_1_read;
      break;
    }

    case SYSTEM_LIGHTPHASER:
    {
      port[0].data_w = dummy_write;
      port[0].data_r = phaser_1_read;
      break;
    }

    case SYSTEM_PADDLE:
    {
      port[0].data_w = paddle_1_write;
      port[0].data_r = paddle_1_read;
      break;
    }

    case SYSTEM_SPORTSPAD:
    {
      port[0].data_w = sportspad_1_write;
      port[0].data_r = sportspad_1_read;
      break;
    }

    default:
    {
      port[0].data_w = dummy_write;
      port[0].data_r = dummy_read;
      break;
    }
  }

  switch (input.system[1])
  {
    case SYSTEM_MS_GAMEPAD:
    {
      port[1].data_w = dummy_write;
      port[1].data_r = gamepad_2_read;
      break;
    }

    case SYSTEM_MD_GAMEPAD:
    {
      port[1].data_w = gamepad_2_write;
      port[1].data_r = gamepad_2_read;
      break;
    }

    case SYSTEM_MOUSE:
    {
      port[1].data_w = mouse_write;
      port[1].data_r = mouse_read;
      break;
    }

    case SYSTEM_ACTIVATOR:
    {
      port[1].data_w = activator_2_write;
      port[1].data_r = activator_2_read;
      break;
    }

    case SYSTEM_MENACER:
    {
      port[1].data_w = dummy_write;
      port[1].data_r = menacer_read;
      break;
    }

    case SYSTEM_JUSTIFIER:
    {
      port[1].data_w = justifier_write;
      port[1].data_r = justifier_read;
      break;
    }

    case SYSTEM_WAYPLAY:
    {
      port[1].data_w = wayplay_2_write;
      port[1].data_r = wayplay_2_read;
      break;
    }

    case SYSTEM_TEAMPLAYER:
    {
      port[1].data_w = teamplayer_2_write;
      port[1].data_r = teamplayer_2_read;
      break;
    }

    case SYSTEM_LIGHTPHASER:
    {
      port[1].data_w = dummy_write;
      port[1].data_r = phaser_2_read;
      break;
    }

    case SYSTEM_PADDLE:
    {
      port[1].data_w = paddle_2_write;
      port[1].data_r = paddle_2_read;
      break;
    }

    case SYSTEM_SPORTSPAD:
    {
      port[1].data_w = sportspad_2_write;
      port[1].data_r = sportspad_2_read;
      break;
    }

    default:
    {
      port[1].data_w = dummy_write;
      port[1].data_r = dummy_read;
      break;
    }
  }

  /* External Port (unconnected) */
  port[2].data_w = dummy_write;
  port[2].data_r = dummy_read;
}


void io_reset(void)
{
  /* Reset I/O registers */
  if (system_hw == SYSTEM_PBC)
  {
    /* SMS compatibility mode control register */
    io_reg[0x00] = 0xFF;
  }
  else
  {
    /* Genesis mode registers */
    io_reg[0x00] = region_code | 0x20 | (config.tmss & 1);
    io_reg[0x01] = 0x00;
    io_reg[0x02] = 0x00;
    io_reg[0x03] = 0x00;
    io_reg[0x04] = 0x00;
    io_reg[0x05] = 0x00;
    io_reg[0x06] = 0x00;
    io_reg[0x07] = 0xFF;
    io_reg[0x08] = 0x00;
    io_reg[0x09] = 0x00;
    io_reg[0x0A] = 0xFF;
    io_reg[0x0B] = 0x00;
    io_reg[0x0C] = 0x00;
    io_reg[0x0D] = 0xFB;
    io_reg[0x0E] = 0x00;
    io_reg[0x0F] = 0x00;
  }

  /* Reset connected peripherals */
  input_reset();
}

void io_68k_write(unsigned int offset, unsigned int data)
{
  switch (offset)
  {
    case 0x01:  /* Port A Data */
    case 0x02:  /* Port B Data */
    case 0x03:  /* Port C Data */
    {
      io_reg[offset] = data;
      port[offset-1].data_w(data, io_reg[offset + 3]);
      return;
    }

    case 0x04:  /* Port A Ctrl */
    case 0x05:  /* Port B Ctrl */
    case 0x06:  /* Port C Ctrl */
    {
      if (data != io_reg[offset])
      {
        io_reg[offset] = data;
        port[offset-4].data_w(io_reg[offset-3], data);
      }
      return;
    }

    case 0x07:  /* Port A TxData */
    case 0x0A:  /* Port B TxData */
    case 0x0D:  /* Port C TxData */
    {
      io_reg[offset] = data;
      return;
    }

    case 0x09:  /* Port A S-Ctrl */
    case 0x0C:  /* Port B S-Ctrl */
    case 0x0F:  /* Port C S-Ctrl */
    {
      io_reg[offset] = data & 0xF8;
      return;
    }

    default:  /* Read-only ports */
    {
      return;
    }
  }
}

unsigned int io_68k_read(unsigned int offset)
{
  switch(offset)
  {
    case 0x01:  /* Port A Data */
    case 0x02:  /* Port B Data */
    case 0x03:  /* Port C Data */
    {
      unsigned int mask = 0x80 | io_reg[offset + 3];
      unsigned int data = port[offset-1].data_r();
      return (io_reg[offset] & mask) | (data & ~mask);
    }

    default:  /* return register value */
    {
      return io_reg[offset];
    }
  }
}

void io_z80_write(unsigned int data)
{
/* pins can't be configured as output on japanese models */
  if (region_code & REGION_USA)
  {
    /* 
      Bit  Function
      --------------
      D7 : Port B TH pin output level (1=high, 0=low)
      D6 : Port B TR pin output level (1=high, 0=low)
      D5 : Port A TH pin output level (1=high, 0=low)
      D4 : Port A TR pin output level (1=high, 0=low)
      D3 : Port B TH pin direction (1=input, 0=output)
      D2 : Port B TR pin direction (1=input, 0=output)
      D1 : Port A TH pin direction (1=input, 0=output)
      D0 : Port A TR pin direction (1=input, 0=output)
    */

    /* Send TR/TH state to connected peripherals */
    port[0].data_w((data << 1) & 0x60, (~io_reg[0] << 5) & 0x60);
    port[1].data_w((data >> 1) & 0x60, (~io_reg[0] << 3) & 0x60);


    /* Check for TH low-to-high transitions on both ports */
    if ((!(io_reg[0] & 0x80) && (data & 0x80)) ||
        (!(io_reg[0] & 0x20) && (data & 0x20)))
    {
      /* Latch new HVC */
      hvc_latch = hctab[(mcycles_z80 + Z80_CYCLE_OFFSET) % MCYCLES_PER_LINE] | 0x10000;
    }
  }
  else
  {
    /* outputs return fixed value */
    data &= 0x0F;
  }

  /* Update control register */
  io_reg[0] = data;
}

unsigned int io_z80_read(unsigned int offset)
{
  /* Read port A & port B input data */
  unsigned int data = (port[0].data_r()) | (port[1].data_r() << 8);

  /* Read control register value */
  unsigned int ctrl = io_reg[0];

  /* I/O ports */
  if (offset)
  {
   /* 
     Bit  Function
     --------------
     D7 : Port B TH pin input
     D6 : Port A TH pin input
     D5 : Unused (0 on Mega Drive, 1 otherwise)
     D4 : RESET button (always 1 on Mega Drive)
     D3 : Port B TR pin input
     D2 : Port B TL pin input
     D1 : Port B Right pin input
     D0 : Port B Left pin input
    */
    data = ((data >> 10) & 0x0F) | (data & 0x40) | ((data >> 7) & 0x80) | 0x10;

    /* Adjust port B TH state if configured as output */
    if (!(ctrl & 0x08))
    {
      data &= ~0x80;
      data |= (ctrl & 0x80);
    }

    /* Adjust port A TH state if configured as output */
    if (!(ctrl & 0x02))
    {
      data &= ~0x40;
      data |= ((ctrl & 0x20) << 1);
    }

    /* Adjust port B TR state if configured as output */
    if (!(ctrl & 0x04))
    {
      data &= ~0x08;
      data |= ((ctrl & 0x40) >> 3);
    }
  }
  else
  {
   /* 
     Bit  Function
     --------------
     D7 : Port B Down pin input
     D6 : Port B Up pin input
     D5 : Port A TR pin input
     D4 : Port A TL pin input
     D3 : Port A Right pin input
     D2 : Port A Left pin input
     D1 : Port A Down pin input
     D0 : Port A Up pin input
    */
    data = (data & 0x3F) | ((data >> 2) & 0xC0);

    /* Adjust port A TR state if configured as output */
    if (!(ctrl & 0x01))
    {
      data &= ~0x20;
      data |= ((ctrl & 0x10) << 1);
    }
  }

  return data;
}

