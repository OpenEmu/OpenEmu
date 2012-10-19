/***************************************************************************************
 *  Genesis Plus
 *  I/O controller (Genesis & Master System modes)
 *
 *  Support for Master System (315-5216, 315-5237 & 315-5297), Game Gear & Mega Drive I/O chips
 *
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald (original code)
 *  Copyright (C) 2007-2012  Eke-Eke (Genesis Plus GX)
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
 * I/O chip initialization                                                   *
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
  if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
  {
    io_reg[0x00] = region_code | (config.bios & 1);
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

    /* CD unit detection */
    if (system_hw != SYSTEM_MCD)
    {
      io_reg[0x00] |= 0x20;
    }
  }
  else
  {
    /* Game Gear specific registers */
    io_reg[0x00] = 0x80 | (region_code >> 1);
    io_reg[0x01] = 0x00;
    io_reg[0x02] = 0xFF;
    io_reg[0x03] = 0x00;
    io_reg[0x04] = 0xFF;
    io_reg[0x05] = 0x00;
    io_reg[0x06] = 0xFF;

    /* initial !RESET input */
    io_reg[0x0D] = IO_RESET_HI;

    /* default !CONT input */
    if (system_hw != SYSTEM_PBC)
    {
      io_reg[0x0D] |= IO_CONT1_HI;
    }

    /* Control registers */
    io_reg[0x0E] = 0x00;
    io_reg[0x0F] = 0xFF;
  }

  /* Reset connected peripherals */
  input_reset();
}


/*****************************************************************************
 * I/O ports access from 68k (Genesis mode)                                  *
 *                                                                           *
 *****************************************************************************/

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


/*****************************************************************************
 *  I/O ports access from Z80                                                *
 *                                                                           *
 *****************************************************************************/

void io_z80_write(unsigned int offset, unsigned int data, unsigned int cycles)
{
  if (offset)
  {
    /* I/O Control register */
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
      port[0].data_w((data << 1) & 0x60, (~io_reg[0x0F] << 5) & 0x60);
      port[1].data_w((data >> 1) & 0x60, (~io_reg[0x0F] << 3) & 0x60);


      /* Check for TH low-to-high transitions on both ports */
      if ((!(io_reg[0x0F] & 0x80) && (data & 0x80)) ||
          (!(io_reg[0x0F] & 0x20) && (data & 0x20)))
      {
        /* Latch new HVC */
        hvc_latch = hctab[cycles % MCYCLES_PER_LINE] | 0x10000;
     }

      /* Update I/O Control register */
      io_reg[0x0F] = data;
    }
    else
    {
      /* TH output is fixed to 0 & TR is always an input on japanese hardware */
      io_reg[0x0F] = (data | 0x05) & 0x5F;

      /* Port $DD bits D4-D5 return D0-D2 (cf. http://www2.odn.ne.jp/~haf09260/Sms/EnrSms.htm) */
      io_reg[0x0D] = ((data & 0x01) << 4) | ((data & 0x04) << 3);
    }
  }
  else
  {
    /* Update Memory Control register */
    io_reg[0x0E] = data;

    /* Switch cartridge & BIOS ROM */
    sms_cart_switch(~data);
  }
}

unsigned int io_z80_read(unsigned int offset)
{
  /* Read port A & port B input data */
  unsigned int data = (port[0].data_r()) | (port[1].data_r() << 8);

  /* I/O control register value */
  unsigned int ctrl = io_reg[0x0F];

  /* I/O ports */
  if (offset)
  {
   /* 
     Bit  Function
     --------------
     D7 : Port B TH pin input
     D6 : Port A TH pin input
     D5 : CONT input (0 on Mega Drive hardware, 1 otherwise)
     D4 : RESET button (1: default, 0: pressed, only on Master System hardware)
     D3 : Port B TR pin input
     D2 : Port B TL pin input
     D1 : Port B Right pin input
     D0 : Port B Left pin input
    */
    data = ((data >> 10) & 0x0F) | (data & 0x40) | ((data >> 7) & 0x80) | io_reg[0x0D];

    /* clear !RESET input */
    io_reg[0x0D] |= IO_RESET_HI;

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


/*****************************************************************************
 * Game Gear communication ports access                                       *
 *                                                                           *
 *****************************************************************************/

void io_gg_write(unsigned int offset, unsigned int data)
{
  switch (offset)
  {
    case 1: /* Parallel data register */
      io_reg[1] = data;
      return;

    case 2: /* Data direction register and NMI enable */
      io_reg[2] = data;
      return;

    case 3: /* Transmit data buffer */
      io_reg[3] = data;
      return;

    case 5: /* Serial control (bits 0-2 are read-only) */
      io_reg[5] = data & 0xF8;
      return;

    case 6: /* PSG Stereo output control */
      io_reg[6] = data;
      SN76489_Config(Z80.cycles, config.psg_preamp, config.psgBoostNoise, data);
      return;

    default: /* Read-only */
      return;
  }
}

unsigned int io_gg_read(unsigned int offset)
{
  switch (offset)
  {
    case 0: /* Mode Register */
      return (io_reg[0] & ~(input.pad[0] & INPUT_START));

    case 1: /* Parallel data register (not connected) */
      return ((io_reg[1] & ~(io_reg[2] & 0x7F)) | (io_reg[2] & 0x7F));

    case 2: /* Data direction register and NMI enable */
      return io_reg[2];

    case 3: /* Transmit data buffer */
      return io_reg[3];

    case 4: /* Receive data buffer */
      return io_reg[4];

    case 5: /* Serial control */
      return io_reg[5];

    default: /* Write-Only */
      return 0xFF;
  }
}

