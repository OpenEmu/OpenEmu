/***************************************************************************************
 *  Genesis Plus
 *  Team Player support
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
  uint8 Table[12];
} teamplayer[2];


void teamplayer_init(int port)
{
  int i,padnum;
  int index = 0;

  /* this table determines which gamepad input should be returned during acquisition sequence
     index  = teamplayer read table index: 0=1st read, 1=2nd read, ...
     table  = high bits are pad index, low bits are pad input shift: 0=RLDU, 4=SABC, 8=MXYZ
  */  
  for (i=0; i<4; i++)
  {
    padnum = (4 * port) + i;
    if (input.dev[padnum] == DEVICE_PAD3B)
    {
      padnum = padnum << 4;
      teamplayer[port].Table[index++] = padnum;
      teamplayer[port].Table[index++] = padnum | 4;
    }
    else
    {
      padnum = padnum << 4;
      teamplayer[port].Table[index++] = padnum;
      teamplayer[port].Table[index++] = padnum | 4;
      teamplayer[port].Table[index++] = padnum | 8;
    }
  }
}

void teamplayer_reset(int port)
{
  teamplayer[port].State = 0x60; /* TH = 1, TR = 1 */
  teamplayer[port].Counter = 0;
}

INLINE unsigned int teamplayer_read(int port)
{
  unsigned int counter = teamplayer[port].Counter;

  /* acquisition sequence */
  switch (counter)
  {
    case 0: /* initial state: TH = 1, TR = 1 -> RLDU = 0011 */
    {
      return 0x73;
    }

    case 1: /* start request: TH = 0, TR = 1 -> RLDU = 1111 */
    {
      return 0x3F; 
    }

    case 2:
    case 3: /* ack request: TH=0, TR=0/1 -> RLDU = 0000 */
    {
      /* TL should match TR */
      return ((teamplayer[port].State & 0x20) >> 1);
    }

    case 4:
    case 5:
    case 6:
    case 7: /* PAD type */
    {
      unsigned int retval = input.dev[(port << 2) + (counter - 4)];

      /* TL should match TR */
      return (((teamplayer[port].State & 0x20) >> 1) | retval);
    }

    default: /* PAD status */
    {
      unsigned int retval = 0x0F;

      /* SEGA teamplayer returns successively PAD1 -> PAD2 -> PAD3 -> PAD4 inputs */
      unsigned int padnum = teamplayer[port].Table[counter - 8] >> 4;

      /* Each PAD inputs is obtained through 2 or 3 sequential reads: RLDU -> SACB -> MXYZ */
      retval &= ~(input.pad[padnum] >> (teamplayer[port].Table[counter - 8] & 0x0F));

      /* TL should match TR */
      return (((teamplayer[port].State & 0x20) >> 1) | retval);
    }
  }
}

INLINE void teamplayer_write(int port, unsigned char data, unsigned char mask)
{
  /* update bits set as output only */
  unsigned int state = (teamplayer[port].State & ~mask) | (data & mask);

  /* TH & TR handshaking */
  if ((teamplayer[port].State ^ state) & 0x60)
  {
    if (state & 0x40) 
    {
      /* TH high -> reset counter */
      teamplayer[port].Counter = 0;
    }
    else
    {
      /* increment counter */
      teamplayer[port].Counter++;
    }

    /* update internal state */
    teamplayer[port].State = state;
  }
}

unsigned char teamplayer_1_read(void)
{
  return teamplayer_read(0);
}

unsigned char teamplayer_2_read(void)
{
  return teamplayer_read(1);
}

void teamplayer_1_write(unsigned char data, unsigned char mask)
{
  teamplayer_write(0, data, mask);
}

void teamplayer_2_write(unsigned char data, unsigned char mask)
{
  teamplayer_write(1, data, mask);
}
