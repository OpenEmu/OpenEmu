/***************************************************************************************
 *  Genesis Plus
 *  Input peripherals support
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

#ifndef _INPUT_H_
#define _INPUT_H_

/* Max. number of devices */
#define MAX_DEVICES (8)

/* Ports configuration */
#define NO_SYSTEM           (0) /* unconnected port*/
#define SYSTEM_MD_GAMEPAD   (1) /* single 3-buttons or 6-buttons Control Pad */
#define SYSTEM_MOUSE        (2) /* Sega Mouse */
#define SYSTEM_MENACER      (3) /* Sega Menacer (port B only) */
#define SYSTEM_JUSTIFIER    (4) /* Konami Justifiers (port B only) */
#define SYSTEM_XE_A1P       (5) /* XE-A1P analog controller (port A only) */
#define SYSTEM_ACTIVATOR    (6) /* Sega Activator */
#define SYSTEM_MS_GAMEPAD   (7) /* single 2-buttons Control Pad (Master System) */
#define SYSTEM_LIGHTPHASER  (8) /* Sega Light Phaser (Master System) */
#define SYSTEM_PADDLE       (9) /* Sega Paddle Control (Master System) */
#define SYSTEM_SPORTSPAD   (10) /* Sega Sports Pad (Master System) */
#define SYSTEM_TEAMPLAYER  (11) /* Sega TeamPlayer */
#define SYSTEM_WAYPLAY     (12) /* EA 4-Way Play (use both ports) */

/* Device type */
#define DEVICE_PAD3B      (0x00) /* 3-buttons Control Pad */
#define DEVICE_PAD6B      (0x01) /* 6-buttons Control Pad */
#define DEVICE_MOUSE      (0x02) /* Sega Mouse */
#define DEVICE_XE_A1P     (0x03) /* XE-A1P analog controller */
#define DEVICE_ACTIVATOR  (0x04) /* Activator */
#define DEVICE_LIGHTGUN   (0x05) /* Sega Light Phaser, Menacer or Konami Justifiers */
#define DEVICE_PAD2B      (0x06) /* 2-buttons Control Pad */
#define DEVICE_PADDLE     (0x07) /* Sega Paddle Control */
#define DEVICE_SPORTSPAD  (0x08) /* Sega Sports Pad */
#define DEVICE_TABLET     (0x09) /* PICO tablet & pen */
#define NO_DEVICE         (0xff) /* unconnected device */

/* Default Input bitmasks */
#define INPUT_MODE         (0x0800)
#define INPUT_X            (0x0400)
#define INPUT_Y            (0x0200)
#define INPUT_Z            (0x0100)
#define INPUT_START        (0x0080)
#define INPUT_A            (0x0040)
#define INPUT_C            (0x0020)
#define INPUT_B            (0x0010)
#define INPUT_RIGHT        (0x0008)
#define INPUT_LEFT         (0x0004)
#define INPUT_DOWN         (0x0002)
#define INPUT_UP           (0x0001)

/* Master System specific bitmasks */
#define INPUT_BUTTON2      (0x0020)
#define INPUT_BUTTON1      (0x0010)

/* Mouse specific bitmask */
#define INPUT_MOUSE_CENTER (0x0040)
#define INPUT_MOUSE_RIGHT  (0x0020)
#define INPUT_MOUSE_LEFT   (0x0010)

/* XE-1AP specific bitmask */
#define INPUT_XE_E1        (0x0800)
#define INPUT_XE_E2        (0x0400)
#define INPUT_XE_START     (0x0200)
#define INPUT_XE_SELECT    (0x0100)
#define INPUT_XE_A         (0x0080)
#define INPUT_XE_B         (0x0040)
#define INPUT_XE_C         (0x0020)
#define INPUT_XE_D         (0x0010)

/* Activator specific bitmasks */
#define INPUT_ACTIVATOR_8U (0x8000)
#define INPUT_ACTIVATOR_8L (0x4000)
#define INPUT_ACTIVATOR_7U (0x2000)
#define INPUT_ACTIVATOR_7L (0x1000)
#define INPUT_ACTIVATOR_6U (0x0800)
#define INPUT_ACTIVATOR_6L (0x0400)
#define INPUT_ACTIVATOR_5U (0x0200)
#define INPUT_ACTIVATOR_5L (0x0100)
#define INPUT_ACTIVATOR_4U (0x0080)
#define INPUT_ACTIVATOR_4L (0x0040)
#define INPUT_ACTIVATOR_3U (0x0020)
#define INPUT_ACTIVATOR_3L (0x0010)
#define INPUT_ACTIVATOR_2U (0x0008)
#define INPUT_ACTIVATOR_2L (0x0004)
#define INPUT_ACTIVATOR_1U (0x0002)
#define INPUT_ACTIVATOR_1L (0x0001)

typedef struct
{
  uint8 system[2];              /* can be one of the SYSTEM_* values */
  uint8 dev[MAX_DEVICES];       /* can be one of the DEVICE_* values */
  uint16 pad[MAX_DEVICES];      /* digital inputs (any of INPUT_* values)  */
  int16 analog[MAX_DEVICES][2]; /* analog inputs (x/y) */
  uint8 x_offset;               /* gun horizontal offset */
  uint8 y_offset;               /* gun vertical offset */
} t_input;

/* Global variables */
extern t_input input;
extern int old_system[2];

/* Function prototypes */
extern void input_init(void);
extern void input_reset(void);
extern void input_refresh(void);

#endif
