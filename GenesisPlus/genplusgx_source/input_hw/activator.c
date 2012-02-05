/***************************************************************************************
 *  Genesis Plus
 *  Sega Activator support
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
} activator[2];

void activator_reset(int index)
{

  activator[index].State = 0x40;
  activator[index].Counter = 0;
}

static inline unsigned char activator_read(int port)
{
  /* IR sensors 1-16 data (active low) */
  uint16 data = ~input.pad[port];

  /* Device index */
  port = port >> 2;

  /* D1 = D0 (data is ready) */
  uint8 temp = (activator[port].State & 0x01) << 1;

  switch (activator[port].Counter)
  {
    case 0: /* x x x x 0 1 0 0 */
      temp |= 0x04;
      break;

    case 1: /* x x l1 l2 l3 l4 1 1 */
      temp |= ((data << 2) & 0x3C);
      break;

    case 2: /* x x l5 l6 l7 l8 0 0 */
      temp |= ((data >> 2) & 0x3C);
      break;

    case 3: /* x x h1 h2 h3 h4 1 1 */
      temp |= ((data >> 6) & 0x3C);
      break;

    case 4: /* x x h5 h6 h7 h8 0 0 */
      temp |= ((data >> 10) & 0x3C);
      break;
  }

  return temp;
}

static inline void activator_write(int index, unsigned char data, unsigned char mask)
{
  /* update bits set as output only */
  data = (activator[index].State & ~mask) | (data & mask);

  /* TH transitions */
  if ((activator[index].State ^ data) & 0x40)
  {
    /* reset sequence cycle */
    activator[index].Counter = 0;
  }
  else
  {
    /* D0 transitions */
    if ((activator[index].State ^ data) & 0x01)
    {
      /* increment sequence cycle */
      if (activator[index].Counter < 4)
      {
        activator[index].Counter++;
      }
    }
  }

  /* update internal state */
  activator[index].State = data;
}

unsigned char activator_1_read(void)
{
  return activator_read(0);
}

unsigned char activator_2_read(void)
{
  return activator_read(4);
}

void activator_1_write(unsigned char data, unsigned char mask)
{
  activator_write(0, data, mask);
}

void activator_2_write(unsigned char data, unsigned char mask)
{
  activator_write(1, data, mask);
}
