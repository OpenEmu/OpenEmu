/***************************************************************************************
 *  Genesis Plus
 *  XE-A1P analog controller support
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

  /* Current internal cycle (0-7) */
  unsigned int cycle = xe_a1p.Counter & 7;

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
