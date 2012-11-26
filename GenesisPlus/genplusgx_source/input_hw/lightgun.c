/***************************************************************************************
 *  Genesis Plus
 *  Sega Light Phaser, Menacer & Konami Justifiers support
 *
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
  input.analog[port][0] = bitmap.viewport.w / 2;
  input.analog[port][1] = bitmap.viewport.h / 2;
  lightgun.State = 0x40;
  lightgun.Port = 4;
}

void lightgun_refresh(int port)
{
  /* Check that lightgun is enabled */
  if (port == lightgun.Port)
  {
    /* screen Y position */
    int y = input.analog[port][1] + input.y_offset;

    /* check if active line falls within current gun Y position */
    if ((y == v_counter) && (y < bitmap.viewport.h))
    {
      /* HL enabled ? */
      if (io_reg[5] & 0x80)
      {
        /* screen X position */
        int x = input.analog[port][0];

        /* Sega Menacer specific */
        if (input.system[1] == SYSTEM_MENACER)
        {
          /* raw position is scaled up by games */
          if (system_hw == SYSTEM_MCD)
          {
            x = (x * 304) / 320;
          }
          else
          {
            x = (x * 289) / 320;
          }
        }

        /* External Interrupt ? */
        if (reg[11] & 0x08) 
        {
          m68k_update_irq(2);
        }

        /* force HV Counter Latch (some games does not lock HV Counter but instead use larger offset value) */
        hvc_latch = 0x10000 | (y << 8);
        if (reg[12] & 1) 
        {
          hvc_latch |= hc_320[((x / 2) + input.x_offset) % 210];
        }
        else
        {
          hvc_latch |= hc_256[((x / 2) + input.x_offset) % 171];
        }
      }
    }
  }
}


/*--------------------------------------------------------------------------*/
/*  Sega Phaser                                                             */
/*--------------------------------------------------------------------------*/

INLINE unsigned char phaser_read(int port)
{
  /* TL returns TRIGGER (INPUT_A) button status (active low) */
  unsigned char temp = ~((input.pad[port] >> 2) & 0x10);

  /* Check that TH is set as an input */
  if (io_reg[0x0F] & (0x02 << (port >> 1)))
  {
    /* Get current X position (phaser is only used in MS compatiblity mode) */
    int hcounter = hctab[(Z80.cycles + SMS_CYCLE_OFFSET) % MCYCLES_PER_LINE];

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
  /* D0=??? (INPUT_B), D1=TRIGGER (INPUT_A), D2=??? (INPUT_C), D3= START (INPUT_START) (active high) */
  /* TL & TR pins always return 0 (normally set as output)  */
  /* TH always return 1 (0 on active pixel but button acquisition is always done during VBLANK) */
  unsigned data = input.pad[4] >> 4;
  return ((data & 0x09) | ((data >> 1) & 0x02) | ((data << 1) & 0x04) | 0x40);
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

  /* Return TRIGGER (INPUT_A) & START (INPUT_START) button status in D0-D1 (active low) */
  /* TL & TR pins should always return 1 (normally set as output) */
  /* LEFT & RIGHT pins should always return 0 */
  return (((~input.pad[lightgun.Port] >> 6) & 0x03) | 0x70);
}

void justifier_write(unsigned char data, unsigned char mask)
{
  /* update bits set as output only, other bits are cleared (fixes Lethal Enforcers 2) */
  data &= mask;

  /* gun index */
  lightgun.Port = 4 + ((data >> 5) & 1);

  /* update internal state */
  lightgun.State = data;
}
