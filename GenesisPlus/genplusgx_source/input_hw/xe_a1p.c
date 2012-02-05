/***************************************************************************************
 *  Genesis Plus
 *  XE-A1P analog controller support
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
  uint8 Counter;
  uint8 Latency;
} xe_a1p;

void xe_a1p_reset(void)
{
  input.analog[0][0] = 128;
  input.analog[0][1] = 128;
  input.analog[1][0] = 128;
  xe_a1p.State = 0x40;
  xe_a1p.Counter = 0;
  xe_a1p.Latency = 0;
}

unsigned char xe_a1p_read()
{
  unsigned int temp = 0x40;

  /* Left Stick X & Y analog values (bidirectional) */
  int x = input.analog[0][0];
  int y = input.analog[0][1];

  /* Right Stick X or Y value (unidirectional) */
  int z = input.analog[1][0];

  /* Buttons status (active low) */
  uint16 pad = ~input.pad[0];
  
  /* Current 4-bit data cycle */
  /* There are eight internal data cycle for each 5 acquisition sequence */
  /* First 4 return the same 4-bit data, next 4 return next 4-bit data */
  switch (xe_a1p.Counter >> 2)
  {
    case 0:
      temp |= ((pad >> 8) & 0x0F); /* E1 E2 Start Select */
      break;
    case 1:
      temp |= ((pad >> 4) & 0x0F); /* A B C D */
      break;
    case 2:
      temp |= ((x >> 4) & 0x0F);
      break;
    case 3:
      temp |= ((y >> 4) & 0x0F);
      break;
    case 4:
      break;
    case 5:
      temp |= ((z >> 4) & 0x0F);
      break;
    case 6:
      temp |= (x & 0x0F);
      break;
    case 7:
      temp |= (y & 0x0F);
      break;
    case 8:
      break;
    case 9:
      temp |= (z & 0x0F);
      break;
  }

  /* Get current internal cycle (0-7) */
  unsigned int cycle = xe_a1p.Counter & 7;

  /* TL indicates which part of data is returned (0=1st part, 1=2nd part) */
  temp |= ((cycle & 4) << 2);

  /* TR indicates if data is ready (0=ready, 1=not ready) */
  /* Fastest One input routine actually expects this bit to switch between 0 & 1 */
  /* so we make the first read of a data cycle return 1 then 0 for remaining reads */
  temp |= (!(cycle & 3) << 5);

  /* Automatically increment data cycle on each read (within current acquisition sequence) */
  cycle = (cycle + 1) & 7;

  /* Update internal cycle counter */
  xe_a1p.Counter = (xe_a1p.Counter & ~7) | cycle;

  /* Update internal latency on each read */
  xe_a1p.Latency++;

  return temp;
}

void xe_a1p_write(unsigned char data, unsigned char mask)
{
  /* update bits set as output only */
  data = (xe_a1p.State & ~mask) | (data & mask);

  /* look for TH 1->0 transitions */
  if (!(data & 0x40) && (xe_a1p.State & 0x40))
  {
    /* reset acquisition cycle */
    xe_a1p.Latency = xe_a1p.Counter = 0;
  }
  else
  {
    /* some games immediately write new data to TH */
    /* so we make sure first sequence has actually been handled */
    if (xe_a1p.Latency > 2)
    {
      /* next acquisition sequence */
      xe_a1p.Counter = (xe_a1p.Counter & ~7) + 8;

      /* 5 sequence max with 8 cycles each */
      if (xe_a1p.Counter > 32)
      {
        xe_a1p.Counter = 32;
      }
    }
  }

  /* update internal state */
  xe_a1p.State = data;
}
