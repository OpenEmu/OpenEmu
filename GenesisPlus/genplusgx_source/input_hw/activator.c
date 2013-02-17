/***************************************************************************************
 *  Genesis Plus
 *  Sega Activator support
 *
 *  Copyright (C) 2011  Eke-Eke (Genesis Plus GX)
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

INLINE unsigned char activator_read(int port)
{
  /* IR sensors 1-16 data (active low) */
  uint16 data = ~input.pad[port << 2];

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

INLINE void activator_write(int index, unsigned char data, unsigned char mask)
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
  return activator_read(1);
}

void activator_1_write(unsigned char data, unsigned char mask)
{
  activator_write(0, data, mask);
}

void activator_2_write(unsigned char data, unsigned char mask)
{
  activator_write(1, data, mask);
}
