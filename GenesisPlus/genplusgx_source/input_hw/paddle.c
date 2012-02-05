/***************************************************************************************
 *  Genesis Plus
 *  Sega Paddle Control support
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

static struct
{
  uint8 State;
} paddle[2];

void paddle_reset(int index)
{
  input.analog[index << 2][0] = 128;
  paddle[index].State = 0x40;
}

static inline unsigned char paddle_read(int port)
{
  /* FIRE button status (active low) */
  unsigned char temp = ~(input.pad[port] & 0x10);

  /* Pad index */
  int index = port >> 2;

  /* Clear low bits */
  temp &= 0x70;

  /* Japanese model: automatic flip-flop */
  if (region_code < REGION_USA)
  {
    paddle[index].State ^= 0x40;
  }

  if (paddle[index].State & 0x40)
  {
    /* return higher bits */
    temp |= (input.analog[port][0] >> 4) & 0x0F;
  }
  else
  {
    /* return lower bits */
    temp |= input.analog[port][0] & 0x0F;

    /* set TR low */
    temp &= ~0x20;
  }

  return temp;
}

static inline void paddle_write(int index, unsigned char data, unsigned char mask)
{
  /* update bits set as output only */
  paddle[index].State = (paddle[index].State & ~mask) | (data & mask);
}


unsigned char paddle_1_read(void)
{
  return paddle_read(0);
}

unsigned char paddle_2_read(void)
{
  return paddle_read(4);
}

void paddle_1_write(unsigned char data, unsigned char mask)
{
  paddle_write(0, data, mask);
}

void paddle_2_write(unsigned char data, unsigned char mask)
{
  paddle_write(1, data, mask);
}
