/***************************************************************************************
 *  Genesis Plus
 *  Sega Sports Pad support
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

static struct
{
  uint8 State;
  uint8 Counter;
} sportspad[2];

void sportspad_reset(int index)
{
  input.analog[index << 2][0] = 128;
  input.analog[index << 2][1] = 128;
  sportspad[index].State = 0x40;
  sportspad[index].Counter = 0;
}

INLINE unsigned char sportspad_read(int port)
{
  /* Buttons 1(B) & 2(C) status (active low) */
  unsigned char temp = ~(input.pad[port] & 0x30);

  /* Pad index */
  int index = port >> 2;

  /* Clear low bits */
  temp &= 0x70;

  /* Detect current state */
  switch (sportspad[index].Counter & 3)
  {
    case 1:
    {
      /* X position high bits */
      temp |= (input.analog[port][0] >> 4) & 0x0F;
      break;
    }

    case 2:
    {
      /* X position low bits */
      temp |= input.analog[port][0] & 0x0F;
      break;
    }

    case 3:
    {
      /* Y position high bits */
      temp |= (input.analog[port][1] >> 4) & 0x0F;
      break;
    }

    default:
    {
      /* Y position low bits */
      temp |= input.analog[port][1] & 0x0F;
      break;
    }
  }

  return temp;
}

INLINE void sportspad_write(int index, unsigned char data, unsigned char mask)
{
  /* update bits set as output only */
  data = (sportspad[index].State & ~mask) | (data & mask);

  /* check TH transitions */
  if ((data ^ sportspad[index].State) & 0x40)
  {
    sportspad[index].Counter++;
  }

  /* update internal state */
  sportspad[index].State = data;
}

unsigned char sportspad_1_read(void)
{
  return sportspad_read(0);
}

unsigned char sportspad_2_read(void)
{
  return sportspad_read(4);
}

void sportspad_1_write(unsigned char data, unsigned char mask)
{
  sportspad_write(0, data, mask);
}

void sportspad_2_write(unsigned char data, unsigned char mask)
{
  sportspad_write(1, data, mask);
}
