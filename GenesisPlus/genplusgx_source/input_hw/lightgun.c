/***************************************************************************************
 *  Genesis Plus
 *  Sega Light Phaser, Menacer & Konami Justifiers support
 *
 *  Copyright Eke-Eke (2007-2011)
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

/************************************************************************************/
/*                                                                                  */
/* H-counter values returned in H40 & H32 modes                                     */
/*                                                                                  */
/* Inside VDP, dot counter register is 9-bit, with only upper 8 bits being returned */
/*                                                                                  */
/* The number of dots per raster line is 342 in H32 mode and 420 in H40 mode        */
/*                                                                                  */
/************************************************************************************/

static const uint8 hc_256[171] =
{
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
  0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
  0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
  0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
  0x90, 0x91, 0x92, 0x93,
                                                        0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
  0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};

static const uint8 hc_320[210] =
{
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
  0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
  0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
  0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
  0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
  0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
  0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6,
                                            0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED,
  0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD,
  0xFE, 0xFF
};

static struct
{
  uint8 State;
  uint8 Port;
} lightgun;


void lightgun_reset(int port)
{
  input.analog[port][0] = bitmap.viewport.w >> 1;
  input.analog[port][1] = bitmap.viewport.h >> 1;
  lightgun.State = 0x40;
  lightgun.Port = 4;
}

void lightgun_refresh(int port)
{
  /* Check that lightgun is enabled */
  if (port == lightgun.Port)
  {
    /* check if line falls within current gun Y position */
    if ((input.analog[port][1] == v_counter + input.y_offset))
    {
      /* HL enabled ? */
      if (io_reg[5] & 0x80)
      {
        /* External Interrupt ? */
        if (reg[11] & 0x08) 
        {
          m68k_irq_state |= 0x12;
        }

        /* HV Counter Latch:
          1) some games does not enable HVC latch but instead use bigger X offset 
              --> we force the HV counter value read by the gun routine 
          2) for games using H40 mode, the gun routine scales up the Hcounter value
              --> H-Counter range is approx. 290 dot clocks
        */
        hvc_latch = 0x10000 | (v_counter << 8);
        if (reg[12] & 1) 
        {
          hvc_latch |= hc_320[((input.analog[port][0] * 290) / (2 * 320) + input.x_offset) % 210];
        }
        else
        {
          hvc_latch |= hc_256[(input.analog[port][0] / 2 + input.x_offset) % 171];
        }
      }
    }
  }
}


/*--------------------------------------------------------------------------*/
/*  Sega Phaser                                                             */
/*--------------------------------------------------------------------------*/

static inline unsigned char phaser_read(int port)
{
  /* FIRE button status (active low) */
  unsigned char temp = ~(input.pad[port] & 0x10);

  /* Check that TH is set as an input */
  if (io_reg[0] & (0x02 << (port >> 1)))
  {
    /* Get current X position (phaser is only used in MS compatiblity mode) */
    int hcounter = hctab[(mcycles_z80 + Z80_CYCLE_OFFSET) % MCYCLES_PER_LINE];

    /* Compare with gun position */
    int dx = input.analog[port][0] - (hcounter << 1);
    int dy = input.analog[port][1] - (v_counter);

    /* Check if current pixel is within lightgun spot ? */
    if ((abs(dy) <= 5) && (abs(dx) <= 60))
    {
      /* set TH low */
      temp &= ~0x40;

      /* prevents multiple latch at each port read */
      if (lightgun.State)
      {
        /* latch estimated HC value */
        hvc_latch = 0x10000 | (input.x_offset + (input.analog[port][0] >> 1));
        lightgun.State = 0;
      }
      else
      {
        lightgun.State = 1;
      }
    }
  }

  return temp & 0x7F;
}

unsigned char phaser_1_read(void)
{
  return phaser_read(0);
}

unsigned char phaser_2_read(void)
{
  return phaser_read(4);
}


/*--------------------------------------------------------------------------*/
/*  Sega Menacer                                                            */
/*--------------------------------------------------------------------------*/

unsigned char menacer_read(void)
{
  /* Return START,A,B,C buttons status in D0-D3 (active high) */
  /* TL & TR pins always return 0 (normally set as output) */
  return ((input.pad[4] >> 4) & 0x0F);
}


/*--------------------------------------------------------------------------*/
/*  Konami Justifiers                                                       */
/*--------------------------------------------------------------------------*/

unsigned char justifier_read(void)
{
  /* Gun detection */
  if (lightgun.State & 0x40)
  {
    return 0x30;
  }

  /* Return A & START button status in D0-D1 (active low) */
  /* TL & TR pins should always return 1 (normally set as output) */
  /* LEFT & RIGHT pins should always return 0 */
  return (((~input.pad[lightgun.Port] >> 6) & 0x03) | 0x70);
}

void justifier_write(unsigned char data, unsigned char mask)
{
  /* update bits set as output only */
  data = (lightgun.State & ~mask) | (data & mask);

  /* gun index */
  lightgun.Port = 4 + ((data >> 5) & 1);

  /* update internal state */
  lightgun.State = data;
}
