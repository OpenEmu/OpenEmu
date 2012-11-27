/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
#
# This file is dual licensed, with permission by the original author
# TyRaNiD, under both the Academic Free License version 2.0 and the GNU
# General Public License version 2 or later.
#
# This means you can choose whether to use this code under the terms of
# the Academic Free License version 2.0, or under the terms of the GNU
# General Public License version 2 or later. As long as you comply to the
# terms of at least one of these, you are allowed to use the code as
# permitted by the respective license.
#
# $Id$
# USB Keyboard Driver for PS2
*/

#ifndef __PS2KBD_H__
#define __PS2KBD_H__

#define PS2KBD_RPC_ID	0xb0b0b80

#define PS2KBD_LED_NUMLOCK   1
#define PS2KBD_LED_CAPSLOCK  2
#define PS2KBD_LED_SCRLOCK   4
#define PS2KBD_LED_COMPOSE   8
#define PS2KBD_LED_KANA      16

#define PS2KBD_LED_MASK      0x1F;

#define PS2KBD_ESCAPE_KEY    0x1B

#define PS2KBD_LEFT_CTRL   (1 << 0)
#define PS2KBD_LEFT_SHIFT  (1 << 1)
#define PS2KBD_LEFT_ALT    (1 << 2)
#define PS2KBD_LEFT_GUI    (1 << 3)
#define PS2KBD_RIGHT_CTRL  (1 << 4)
#define PS2KBD_RIGHT_SHIFT (1 << 5)
#define PS2KBD_RIGHT_ALT   (1 << 6)
#define PS2KBD_RIGHT_GUI   (1 << 7)

#define PS2KBD_CTRL    (PS2KBD_LEFT_CTRL | PS2KBD_RIGHT_CTRL)
#define PS2KBD_SHIFT  (PS2KBD_LEFT_SHIFT | PS2KBD_RIGHT_SHIFT)
#define PS2KBD_ALT    (PS2KBD_LEFT_ALT | PS2KBD_RIGHT_ALT)
#define PS2KBD_GUI    (PS2KBD_LEFT_GUI | PS2KBD_RIGHT_GUI)

#define PS2KBD_RAWKEY_UP   0xF0
#define PS2KBD_RAWKEY_DOWN 0xF1

typedef struct _kbd_rawkey {
  u8 state;
  u8 key;
} kbd_rawkey __attribute__ ((packed));

#define PS2KBD_READMODE_NORMAL 1
#define PS2KBD_READMODE_RAW    2

/* Notes on read mode */
/* In normal readmode (default) read multiples of 1 character off the keyboard file. These are
   processed by the keymaps so that you get back ASCII data */
/* In raw readmode must read multiples of 2. First byte indicates state (i.e. Up or Down)
   Second byte is the USB key code for that key. This table is presented in the USB HID Usage Tables manaual
   from usb.org */

#define PS2KBD_KEYMAP_SIZE 256

typedef struct _kbd_keymap

{
  u8 keymap[PS2KBD_KEYMAP_SIZE];
  u8 shiftkeymap[PS2KBD_KEYMAP_SIZE];
  u8 keycap[PS2KBD_KEYMAP_SIZE];
} kbd_keymap;


/* IRPC function numbers */
#define KBD_RPC_SETREADMODE     1 /* Sets up keymapped or raw mode */
#define KBD_RPC_SETLEDS         2 /* Sets the LED state for ALL keyboards connected */
#define KBD_RPC_SETREPEATRATE   3 /* Sets the repeat rate of the keyboard */
#define KBD_RPC_SETKEYMAP       4 /* Sets the keymap for the standard keys, non shifted and shifted */
#define KBD_RPC_SETCTRLMAP      5 /* Sets the control key mapping */
#define KBD_RPC_SETALTMAP       6 /* Sets the alt key mapping */
#define KBD_RPC_SETSPECIALMAP   7 /* Sets the special key mapping */
#define KBD_RPC_FLUSHBUFFER     9 /* Flush the internal buffer, probably best after a keymap change */
#define KBD_RPC_RESETKEYMAP    10 /* Reset keymaps to default states */
#define KBD_RPC_READKEY		   11
#define KBD_RPC_READRAW		   12

/* Note on keymaps. In normal keymap a 0 would indicate no key */
/* Key maps are represented by 3 256*8bit tables. First table maps USB key to a char when not shifted */
/* Second table maps USB key to a char when shifted */
/* Third table contains boolean values. If 1 then the key is shifted/unshifted in capslock, else capslock is ignored */

#endif
