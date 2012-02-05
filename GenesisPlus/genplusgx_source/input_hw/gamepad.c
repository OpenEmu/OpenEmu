/***************************************************************************************
 *  Genesis Plus
 *  3-Buttons & 6-Buttons pad support (incl. 4-WayPlay & J-Cart handlers)
 *
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald (original code)
 *  Eke-Eke (2007-2011), additional code & fixes for the GCN/Wii port
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
#include "gamepad.h"

static struct
{
  uint8 State;
  uint8 Counter;
  uint8 Timeout;
} gamepad[MAX_DEVICES];

static uint8 pad_index;


void gamepad_reset(int port)
{
  /* default state (Gouketsuji Ichizoku / Power Instinct, Samurai Spirits / Samurai Shodown) */
  gamepad[port].State = 0x40;
  gamepad[port].Counter = 0;
  gamepad[port].Timeout = 0;

  /* reset pad index (4-WayPlay) */
  pad_index = 0;
}

void gamepad_refresh(int port)
{
  /* 6-buttons pad */
  if (gamepad[port].Timeout++ > 25)
  {
    gamepad[port].Counter = 0;
    gamepad[port].Timeout = 0;
  }
}

static inline unsigned char gamepad_read(int port)
{
  /* bit 7 is latched, returns current TH state */
  unsigned int data = (gamepad[port].State & 0x40) | 0x3F;

  /* pad value */
  unsigned int val = input.pad[port];

  /* get current step (TH state) */
  unsigned int step = (gamepad[port].Counter & 6) | ((data >> 6) & 1);

  switch (step)
  {
    case 1: /*** First High  ***/
    case 3: /*** Second High ***/
    case 5: /*** Third High  ***/
    {
      /* TH = 1 : ?1CBRLDU */
      data &= ~(val & 0x3F);
      break;
    }

    case 0: /*** First low  ***/
    case 2: /*** Second low ***/
    {
      /* TH = 0 : ?0SA00DU */
      data &= ~(val & 0x03);
      data &= ~((val >> 2) & 0x30);
      data &= ~0x0C;
      break;
    }

    /* 6buttons specific (taken from gen-hw.txt) */
    /* A 6-button gamepad allows the extra buttons to be read based on how */
    /* many times TH is switched from 1 to 0 (and not 0 to 1). Observe the */
    /* following sequence */
    /*
       TH = 1 : ?1CBRLDU    3-button pad return value
       TH = 0 : ?0SA00DU    3-button pad return value
       TH = 1 : ?1CBRLDU    3-button pad return value
       TH = 0 : ?0SA0000    D3-0 are forced to '0'
       TH = 1 : ?1CBMXYZ    Extra buttons returned in D3-0
       TH = 0 : ?0SA1111    D3-0 are forced to '1'
    */
    case 4: /*** Third Low ***/
    {
      /* TH = 0 : ?0SA0000    D3-0 are forced to '0'*/
      data &= ~((val >> 2) & 0x30);
      data &= ~0x0F;
      break;
    }

    case 6: /*** Fourth Low ***/
    {
      /* TH = 0 : ?0SA1111    D3-0 are forced to '1'*/
      data &= ~((val >> 2) & 0x30);
      break;
    }

    case 7: /*** Fourth High ***/
    {
      /* TH = 1 : ?1CBMXYZ    Extra buttons returned in D3-0*/
      data &= ~(val & 0x30);
      data &= ~((val >> 8) & 0x0F);
      break;
    }
  }

  return data;
}

static inline void gamepad_write(int port, unsigned char data, unsigned char mask)
{
  /* update bits set as output only */
  data = (gamepad[port].State & ~mask) | (data & mask);

  if (input.dev[port] == DEVICE_PAD6B)
  {
    /* check TH transitions */
    if ((gamepad[port].State ^ data) & 0x40)
    {
      gamepad[port].Counter++;
      gamepad[port].Timeout = 0;
    }
  }

  /* update internal state */
  gamepad[port].State = data;
}


/*--------------------------------------------------------------------------*/
/*  Default ports handlers                                                  */
/*--------------------------------------------------------------------------*/

unsigned char gamepad_1_read(void)
{
  return gamepad_read(0);
}

unsigned char gamepad_2_read(void)
{
  return gamepad_read(4);
}

void gamepad_1_write(unsigned char data, unsigned char mask)
{
  gamepad_write(0, data, mask);
}

void gamepad_2_write(unsigned char data, unsigned char mask)
{
  gamepad_write(4, data, mask);
}

/*--------------------------------------------------------------------------*/
/*  4-WayPlay ports handler                                                 */
/*--------------------------------------------------------------------------*/

unsigned char wayplay_1_read(void)
{
  if (pad_index < 4)
  {
    return gamepad_read(pad_index);
  }

  /* multitap detection */
  return 0x70;
}

unsigned char wayplay_2_read(void)
{
  return 0x7F;
}

void wayplay_1_write(unsigned char data, unsigned char mask)
{
  if (pad_index < 4)
  {
    gamepad_write(pad_index, data, mask);
  }
}

void wayplay_2_write(unsigned char data, unsigned char mask)
{
  if ((mask & 0x70) == 0x70)
  {
    pad_index = (data & 0x70) >> 4;
  }
}


/*--------------------------------------------------------------------------*/
/*  J-Cart memory handlers                                                  */
/*--------------------------------------------------------------------------*/

unsigned int jcart_read(unsigned int address)
{
   /* TH2 output read is fixed to zero (fixes Micro Machines 2) */
   return ((gamepad_read(5) & 0x7F) | ((gamepad_read(6) & 0x3F) << 8));
}

void jcart_write(unsigned int address, unsigned int data)
{
  gamepad_write(5, (data & 1) << 6, 0x40);
  gamepad_write(6, (data & 1) << 6, 0x40);
  return;
}
