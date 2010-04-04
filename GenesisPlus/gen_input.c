/***************************************************************************************
 *  Genesis Plus
 *  Peripheral Input Support
 *
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald (original code)
 *  Eke-Eke (2007,2008,2009), additional code & fixes for the GCN/Wii port
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

t_input input;

/*****************************************************************************
 * LIGHTGUN specific functions
 *
 *****************************************************************************/
/* H counter values for a 256-pixel wide display (342 pixel max.) */
static uint8 hc_256[171] = {
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

/* H counter values for a 320-pixel wide display (420 pixels max.) */
static uint8 hc_320[210] = {
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

static inline void lightgun_reset(int num)
{
  input.analog[num][0] = bitmap.viewport.w >> 1;
  input.analog[num][1] = bitmap.viewport.h >> 1;
}

static inline void lightgun_update(int num)
{
  if ((input.analog[num][1] == v_counter + input.y_offset))
  {
    /* HL enabled ? */
    if (io_reg[5] & 0x80)
    {
      /* External Interrupt ? */
      if (reg[11] & 0x08) irq_status = (irq_status & ~0x40) | 0x12;

      /* HVC Latch:
        1) some games does not set HVC latch but instead use bigger X offset 
        2) for games using H40 mode, the gun routine scales up the Hcounter value,
           H-Counter range is approx. 292 pixel clocks
      */
      hc_latch = 0x100;
      if (reg[12] & 1) hc_latch |= hc_320[((input.analog[num][0] * 290) / (2 * 320) + input.x_offset) % 210];
      else hc_latch |= hc_256[(input.analog[num][0] / 2 + input.x_offset)%171];
    }
  }
}

/* Sega Menacer specific */
uint32 menacer_read()
{
  /* pins should return 0 by default (fix Body Count when mouse is enabled) */
  int retval = 0x00;
  if (input.pad[4] & INPUT_B)     retval |= 0x01;
  if (input.pad[4] & INPUT_A)     retval |= 0x02;
  if (input.pad[4] & INPUT_C)     retval |= 0x04;
  if (input.pad[4] & INPUT_START) retval |= 0x08;

  return retval;
}

/* Konami Justifier specific */
uint32 justifier_read()
{
  /* TL & TR pins should always return 1 (write only) */
  /* LEFT & RIGHT pins should always return 0 (needed during gun detection) */
  int retval = 0x73; 

  switch (io_reg[2])
  {
    case 0x40:  /* gun detection */
      return 0x30;

    case 0x00:  /* gun #1 enabled */
      if (input.pad[4] & INPUT_A)     retval &= ~0x01;
      if (input.pad[4] & INPUT_START) retval &= ~0x02;
      return retval;

    case 0x20:  /* gun #2 enabled */
      if (input.pad[5] & INPUT_A)     retval &= ~0x01;
      if (input.pad[5] & INPUT_START) retval &= ~0x02;
      return retval;

    default:  /* guns disabled */
      return retval;
  }
}

/*****************************************************************************
 * SEGA MOUSE specific functions
 *
 *****************************************************************************/
struct mega_mouse
{
  uint8 State;
  uint8 Counter;
  uint8 Wait;
  uint8 Port;
} mouse;

static inline void mouse_reset()
{
  mouse.State   = 0x60;
  mouse.Counter = 0;
  mouse.Wait = 0;
  mouse.Port = (input.system[0] == SYSTEM_MOUSE) ? 0 : 4;
}

void mouse_write(uint32 data)
{
  if (mouse.Counter == 0)
  {
    /* TH 1->0 transition */
    if ((mouse.State&0x40) && !(data&0x40))
    {
      /* start acquisition */
      mouse.Counter = 1;
    }
  }
  else
  {
    /* TR transition */
    if ((mouse.State&0x20) != (data&0x20))
    {
      mouse.Counter ++; /* increment phase */
      mouse.Wait = 1;   /* mouse latency */

      if (mouse.Counter > 9) mouse.Counter = 9;
    }
  }

  /* end of acquisition (TH=1) */
  if (data&0x40) mouse.Counter = 0;

  /* update internal state */
  mouse.State = data;
}

uint32 mouse_read()
{
  int temp = 0x00;

  switch (mouse.Counter)
  {
    case 0:     /* initial */
      temp = 0x00;
      break;

    case 1:     /* xxxx1011 */
      temp = 0x0B;
      break;

    case 2:     /* xxxx1111 */
      temp = 0x0F;
      break;

    case 3:     /* xxxx1111 */
      temp = 0x0F;
      break;

    case 4:   /* Axis sign and overflow */
      if (input.analog[2][0] < 0)         temp |= 0x01;
      if (input.analog[2][1] < 0)         temp |= 0x02;
      if (abs(input.analog[2][0]) > 255)  temp |= 0x04;
      if (abs(input.analog[2][1]) > 255)  temp |= 0x08;
      break;

    case 5:   /* Buttons state */
      if (input.pad[mouse.Port] & INPUT_A)     temp |= 0x01;
      if (input.pad[mouse.Port] & INPUT_C)     temp |= 0x02;
      if (input.pad[mouse.Port] & INPUT_B)     temp |= 0x04;
      if (input.pad[mouse.Port] & INPUT_START) temp |= 0x08;
      break;

    case 6:   /* X Axis MSB */
      temp = (input.analog[2][0] >> 4) & 0x0f;
      break;
      
    case 7:   /* X Axis LSB */
      temp = (input.analog[2][0] & 0x0f);
      break;

    case 8:   /* Y Axis MSB */
      temp = (input.analog[2][1] >> 4) & 0x0f;
      break;
      
    case 9:  /* Y Axis LSB */
      temp = (input.analog[2][1] & 0x0f);
      break;
  }

  /* TR-TL handshaking */
  if (mouse.Wait)
  {
    /* wait before ACK, fix some buggy mouse routine (Shangai 2, Wack World,...) */
    mouse.Wait = 0;

    /* TL = !TR */
    temp |= (~mouse.State & 0x20) >> 1;
  }
  else
  {
    /* TL = TR */
    temp |= (mouse.State & 0x20) >> 1;
  }

  return temp;
}


/*****************************************************************************
 * GAMEPAD specific functions (2PLAYERS/4WAYPLAY) 
 *
 *****************************************************************************/
struct pad
{
  uint8 State;
  uint8 Counter;
  uint8 Delay;
} gamepad[MAX_DEVICES];

static inline void gamepad_raz(uint32 i)
{
  gamepad[i].Counter = 0;
  gamepad[i].Delay   = 0;
}

static inline void gamepad_reset(uint32 i)
{
  gamepad[i].State = 0x40;
  if (input.dev[i] == DEVICE_6BUTTON) gamepad_raz(i);
}

static inline void gamepad_update(uint32 i)
{
  if (gamepad[i].Delay++ > 25) gamepad_raz(i);
}

static inline uint32 gamepad_read(uint32 i)
{
  int control;
  int retval = 0x7F;

  control = (gamepad[i].State & 0x40) >> 6; /* current TH state */

  if (input.dev[i] == DEVICE_6BUTTON)
  {
    control += (gamepad[i].Counter & 3) << 1; /* TH transitions counter */
  }

  switch (control)
  {
    case 1: /*** First High  ***/
    case 3: /*** Second High ***/
    case 5: /*** Third High  ***/

      /* TH = 1 : ?1CBRLDU */
      if (input.pad[i] & INPUT_C)     retval &= ~0x20;
      if (input.pad[i] & INPUT_B)     retval &= ~0x10;
      if (input.pad[i] & INPUT_UP)    retval &= ~0x01;
      if (input.pad[i] & INPUT_DOWN)  retval &= ~0x02;
      if (input.pad[i] & INPUT_LEFT)  retval &= ~0x04;
      if (input.pad[i] & INPUT_RIGHT) retval &= ~0x08;
      break;

    case 0: /*** First low  ***/
    case 2: /*** Second low ***/

      /* TH = 0 : ?0SA00DU */
      if (input.pad[i] & INPUT_A)     retval &= ~0x10;
      if (input.pad[i] & INPUT_START) retval &= ~0x20;
      if (input.pad[i] & INPUT_UP)    retval &= ~0x01;
      if (input.pad[i] & INPUT_DOWN)  retval &= ~0x02;
      retval &= 0xB3; 
      break;

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

      /* TH = 0 : ?0SA0000    D3-0 are forced to '0'*/
      if (input.pad[i] & INPUT_A)     retval &= ~0x10;
      if (input.pad[i] & INPUT_START) retval &= ~0x20;
      retval &= 0xB0;
      break;

    case 6: /*** Fourth Low ***/

      /* TH = 0 : ?0SA1111    D3-0 are forced to '1'*/
      if (input.pad[i] & INPUT_A)     retval &= ~0x10;
      if (input.pad[i] & INPUT_START) retval &= ~0x20;
      retval &= 0xBF;
      break;

    case 7: /*** Fourth High ***/

      /* TH = 1 : ?1CBMXYZ    Extra buttons returned in D3-0*/
      if (input.pad[i] & INPUT_X)    retval &= ~0x04;
      if (input.pad[i] & INPUT_Y)    retval &= ~0x02;
      if (input.pad[i] & INPUT_Z)    retval &= ~0x01;
      if (input.pad[i] & INPUT_B)    retval &= ~0x10;
      if (input.pad[i] & INPUT_C)    retval &= ~0x20;
      if (input.pad[i] & INPUT_MODE) retval &= ~0x08;
      break;

    default:
      break;
  }

  /* bit7 is latched */
  return retval;
}

static inline void gamepad_write(uint32 i, uint32 data)
{
  if (input.dev[i] == DEVICE_6BUTTON)
  {
    /* TH=0 to TH=1 transition */
    if (!(gamepad[i].State & 0x40) && (data & 0x40))
    {
      gamepad[i].Counter++;
      gamepad[i].Delay = 0;
    }
  }

  gamepad[i].State = data;
}


/*****************************************************************************
 * TEAMPLAYER adapter
 *
 *****************************************************************************/
struct teamplayer
{
  uint8 State;
  uint8 Counter;
  uint8 Table[12];
} teamplayer[2];

static inline void teamplayer_reset(uint32 port)
{
  int i;
  int index = 0;
  int pad_input = 0;

  teamplayer[port].State = 0x60; /* TH = 1, TR = 1 */
  teamplayer[port].Counter = 0;

  /* this table determines which gamepad input should be returned during acquisition sequence
     index     = teamplayer read table index: 0=1st read, 1=2nd read, ...
     pad_input = gamepad input 0-14: 0=P1_DIR, 1=P1_SABC, 2=P1_MXYZ, 4=P2_DIR, 5=P2_SABC, ...
  */  
  for (i=0; i<4; i++)
  {
    if (input.dev[(4*port) + i] == DEVICE_3BUTTON)
    {
      teamplayer[port].Table[index++] = pad_input;
      teamplayer[port].Table[index++] = pad_input + 1;
    }
    else if (input.dev[(4*port) + i] == DEVICE_6BUTTON)
    {
      teamplayer[port].Table[index++] = pad_input;
      teamplayer[port].Table[index++] = pad_input + 1;
      teamplayer[port].Table[index++] = pad_input + 2;
    }
    pad_input += 4;
  }
}

/* SEGA teamplayer returns successively:
   - PAD1 inputs
   - PAD2 inputs
   - PAD3 inputs
   - PAD4 inputs

   Each PAD inputs is obtained through 2 or 3 sequential reads:
   1/ DIR buttons
   2/ START,A,C,B buttons
   3/ MODE, X,Y,Z buttons (6Button only !)
*/
static inline uint32 teamplayer_read_device(uint32 port, uint32 index)
{
  int retval = 0x7F;
  int pad_input = teamplayer[port].Table[index] & 0x03;
  int pad_num = (4 * port) + ((teamplayer[port].Table[index] >> 2) & 0x03);

  switch (pad_input)
  {
    case 0:
       /* Directions Buttons */
       if (input.pad[pad_num] & INPUT_UP)    retval &= ~0x01;
       if (input.pad[pad_num] & INPUT_DOWN)  retval &= ~0x02;
       if (input.pad[pad_num] & INPUT_LEFT)  retval &= ~0x04;
       if (input.pad[pad_num] & INPUT_RIGHT) retval &= ~0x08;
       break;

    case 1:
       /* S,A,C,B Buttons */
       if (input.pad[pad_num] & INPUT_B)     retval &= ~0x01;
       if (input.pad[pad_num] & INPUT_C)     retval &= ~0x02;
       if (input.pad[pad_num] & INPUT_A)     retval &= ~0x04;
       if (input.pad[pad_num] & INPUT_START) retval &= ~0x08;
       break;

    case 2:
       /* M,X,Y,Z Buttons (6-Buttons only)*/
       if (input.pad[pad_num] & INPUT_Z)    retval &= ~0x01;
       if (input.pad[pad_num] & INPUT_Y)    retval &= ~0x02;
       if (input.pad[pad_num] & INPUT_X)    retval &= ~0x04;
       if (input.pad[pad_num] & INPUT_MODE) retval &= ~0x08;
       break;
  }

  return retval; 
}

static inline uint32 teamplayer_read(uint32 port)
{
  int retval = 0x7F;
  int padnum;

  switch (teamplayer[port].Counter) /* acquisition sequence steps */
  {
    case 0: /* initial state: TH = 1, TR = 1 */
      retval = 0x73; 
      break;

    case 1: /* start request: TH = 0, TR = 1 */
      retval = 0x3F; 
      break;

    case 2:
    case 3: /* ack request: TH=0, TR handshake */
      retval = 0x00;
      break;

    case 4:
    case 5:
    case 6:
    case 7: /* gamepads type */
      padnum = (4 * port) + teamplayer[port].Counter - 4;
      retval = input.dev[padnum];
      break;

    default: /* gamepads inputs acquisition */
      retval = teamplayer_read_device(port, teamplayer[port].Counter - 8);
      break;
  }

  /* TL must match TR state */
  retval &= ~0x10;
  if (teamplayer[port].State & 0x20) retval |= 0x10;

  return retval;
}

static inline void teamplayer_write(uint32 port, uint32 data)
{
  int old_state = teamplayer[port].State;
  teamplayer[port].State = (data & io_reg[port+4]) | (teamplayer[port].State & ~io_reg[port+4]);
  if (old_state != teamplayer[port].State) teamplayer[port].Counter ++;
  if ((data&0x60) == 0x60) teamplayer[port].Counter = 0;
}

/*****************************************************************************
 * 4WAYPLAY adapter
 *
 *****************************************************************************/
static inline void wayplay_write(uint32 port, uint32 data)
{
  if (port == 0) gamepad_write(input.current, data);
  else input.current = (data >> 4) & 0x07;
}

static inline uint32 wayplay_read(uint32 port)
{
  if (port == 1) return 0x7F;
  if (input.current >= 4) return 0x70; /* multitap detection (TH2 = 1) */
  return gamepad_read(input.current);  /* 0x0C = Pad1, 0x1C = Pad2, ... */
}


/*****************************************************************************
 * I/O wrappers
 *
 *****************************************************************************/
uint32 gamepad_1_read (void)
{
  return gamepad_read(0);
}

uint32 gamepad_2_read (void)
{
  return gamepad_read(4);
}

void gamepad_1_write (uint32 data)
{
  gamepad_write(0, data);
}

void gamepad_2_write (uint32 data)
{
  gamepad_write(4, data);
}

uint32 wayplay_1_read (void)
{
  return wayplay_read(0);
}

uint32 wayplay_2_read (void)
{
  return wayplay_read(1);
}

void wayplay_1_write (uint32 data)
{
  wayplay_write(0, data);
}

void wayplay_2_write (uint32 data)
{
  wayplay_write(1, data);
}

uint32 teamplayer_1_read (void)
{
  return teamplayer_read(0);
}

uint32 teamplayer_2_read (void)
{
  return teamplayer_read(1);
}

void teamplayer_1_write (uint32 data)
{
  teamplayer_write(0, data);
}

void teamplayer_2_write (uint32 data)
{
  teamplayer_write(1, data);
}

uint32 jcart_read(uint32 address)
{
  return (gamepad_read(5) | ((gamepad_read(6)&0x3f) << 8)); /* fixes Micro Machines 2 (is it correct ?) */
}

void jcart_write(uint32 address, uint32 data)
{
  gamepad_write(5, (data&1) << 6);
  gamepad_write(6, (data&1) << 6);
  return;
}

/*****************************************************************************
 * Generic INPUTS Control
 *
 *****************************************************************************/
void input_reset ()
{
  int i,j;

  input.max = 0;
  input.current = 0;

  for (i=0; i<MAX_DEVICES; i++)
  {
    input.dev[i] = NO_DEVICE;
    input.pad[i] = 0;
  }

  switch (input.system[0])
  {
    case SYSTEM_GAMEPAD:
      if (input.max == MAX_INPUTS) return;
      input.dev[0] = config.input[input.max].padtype;
      input.max ++;
      gamepad_reset(0);
      break;

    case SYSTEM_MOUSE:
      if (input.max == MAX_INPUTS) return;
      input.dev[0] = DEVICE_MOUSE;
      input.max ++;
      mouse_reset();
      break;

    case SYSTEM_WAYPLAY:
      for (j=0; j< 4; j++)
      {
        if (input.max == MAX_INPUTS) return;
        input.dev[j] = config.input[input.max].padtype;
        input.max ++;
        gamepad_reset(j);
      }
      break;

    case SYSTEM_TEAMPLAYER:
      for (j=0; j<4; j++)
      {
        if (input.max == MAX_INPUTS) return;
        input.dev[j] = config.input[input.max].padtype;
        input.max ++;
      }
      teamplayer_reset(0);
      break;
  }

  switch (input.system[1])
  {
    case SYSTEM_GAMEPAD:
      if (input.max == MAX_INPUTS) return;
      input.dev[4] = config.input[input.max].padtype;
      input.max ++;
      gamepad_reset(4);
      break;

    case SYSTEM_MOUSE:
      if (input.max == MAX_INPUTS) return;
      input.dev[4] = DEVICE_MOUSE;
      input.max ++;
      mouse_reset();
      break;

    case SYSTEM_MENACER:
      if (input.max == MAX_INPUTS) return;
      input.dev[4] = DEVICE_LIGHTGUN;
      lightgun_reset(0);
      break;

    case SYSTEM_JUSTIFIER:
      for (j=4; j<6; j++)
      {
        if (input.max == MAX_INPUTS) return;
        input.dev[j] = DEVICE_LIGHTGUN;
        lightgun_reset(j - 4);
        input.max ++;
      }
      break;

     case SYSTEM_TEAMPLAYER:
      for (j=4; j<8; j++)
      {
        if (input.max == MAX_INPUTS) return;
        input.dev[j] = config.input[input.max].padtype;
        input.max ++;
      }
      teamplayer_reset(1);
      break;
  }

  /* J-CART: add two gamepad inputs */
  if (j_cart)
  {
    input.dev[5] = config.input[2].padtype;
    input.dev[6] = config.input[3].padtype;
    gamepad_reset(5);
    gamepad_reset(6);
  }
}

void input_update()
{
  int i;
  switch (input.system[0])
  {
    case SYSTEM_GAMEPAD:
      if (input.dev[0] == DEVICE_6BUTTON) gamepad_update(0);
      break;

    case SYSTEM_WAYPLAY:
      for (i=0; i<4; i++)
      {
        if (input.dev[i] == DEVICE_6BUTTON) gamepad_update(i);
      }
      break;
  }

  switch (input.system[1])
  {
    case SYSTEM_GAMEPAD:
      if (input.dev[4] == DEVICE_6BUTTON) gamepad_update(4);
      break;

    case SYSTEM_MENACER:
      lightgun_update(0);
      break;

    case SYSTEM_JUSTIFIER:
      if ((io_reg[2] & 0x30) == 0x00) lightgun_update(0);
      if ((io_reg[2] & 0x30) == 0x20) lightgun_update(1);
      break;
  }
}

void input_raz()
{
  int i;
  switch (input.system[0])
  {
    case SYSTEM_GAMEPAD:
      if (input.dev[0] == DEVICE_6BUTTON) gamepad_raz(0);
      break;

    case SYSTEM_WAYPLAY:
      for (i=0; i<4; i++)
      {
        if (input.dev[i] == DEVICE_6BUTTON) gamepad_raz(i);
      }
      break;
  }
  
  switch (input.system[1])
  {
    case SYSTEM_GAMEPAD:
      if (input.dev[4] == DEVICE_6BUTTON) gamepad_raz(4);
      break;
  }
}
