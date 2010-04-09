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

#ifndef _GEN_INPUT_H_
#define _GEN_INPUT_H_ 

/* Max. number of devices */
#define MAX_DEVICES (8)

/* Device types */
#define DEVICE_3BUTTON      (0x00)  /* 3-button gamepad */
#define DEVICE_6BUTTON      (0x01)  /* 6-button gamepad */
#define DEVICE_LIGHTGUN     (0x02)  /* Sega Menacer or Konami Justifier */
#define DEVICE_MOUSE        (0x03)  /* Sega Mouse */
#define DEVICE_2BUTTON      (0x04)  /* 2-button gamepad (not supported) */
#define NO_DEVICE           (0x0F)  /* unconnected */

/* Input bitmasks */
#define INPUT_MODE      (0x00000800)
#define INPUT_Z         (0x00000400)
#define INPUT_Y         (0x00000200)
#define INPUT_X         (0x00000100)
#define INPUT_START     (0x00000080)
#define INPUT_C         (0x00000040)
#define INPUT_B         (0x00000020)
#define INPUT_A         (0x00000010)
#define INPUT_LEFT      (0x00000008)
#define INPUT_RIGHT     (0x00000004)
#define INPUT_DOWN      (0x00000002)
#define INPUT_UP        (0x00000001)

/* System IO ports */
#define NO_SYSTEM           (0) /* Unconnected Port*/
#define SYSTEM_GAMEPAD      (1) /* Single Gamepad */
#define SYSTEM_MOUSE        (2) /* Sega Mouse */
#define SYSTEM_MENACER      (3) /* Sega Menacer (port 2) */
#define SYSTEM_JUSTIFIER    (4) /* Konami Justifier (port 2) */
#define SYSTEM_TEAMPLAYER   (5) /* Sega TeamPlayer */
#define SYSTEM_WAYPLAY      (6) /* EA 4-Way Play (use both ports) */

typedef struct
{
  uint8   dev[MAX_DEVICES];     /* Can be any of the DEVICE_* values */
  uint32  pad[MAX_DEVICES];     /* Can be any of the INPUT_* bitmasks */
  uint8   system[2];            /* Can be any of the SYSTEM_* bitmasks */
  uint8   max;                  /* maximum number of connected devices */
  uint8   current;              /* current PAD number (4WAYPLAY) */
  int     analog[3][2];         /* analog devices */
  int     x_offset;
  int     y_offset;
} t_input;

/* Global variables */
extern t_input input;

/* Function prototypes */
extern void input_reset (void);
extern void input_update(void);
extern void input_raz(void);

/* Peripherals specific */
extern void mouse_write(uint32 data);
extern uint32 mouse_read(void);
extern uint32 menacer_read (void);
extern uint32 justifier_read (void);
extern uint32 gamepad_1_read (void);
extern uint32 gamepad_2_read (void);
extern void gamepad_1_write (uint32 data);
extern void gamepad_2_write (uint32 data);
extern uint32 wayplay_1_read (void);
extern uint32 wayplay_2_read (void);
extern void wayplay_1_write (uint32 data);
extern void wayplay_2_write (uint32 data);
extern uint32 teamplayer_1_read (void);
extern uint32 teamplayer_2_read (void);
extern void teamplayer_1_write (uint32 data);
extern void teamplayer_2_write (uint32 data);
extern uint32 jcart_read(uint32 address);
extern void jcart_write(uint32 address, uint32 data);

#endif
