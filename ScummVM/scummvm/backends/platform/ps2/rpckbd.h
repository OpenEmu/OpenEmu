/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef __RPCKBD_H__
#define __RPCKBD_H__

#include "backends/platform/ps2/iop/rpckbd/include/ps2kbd.h"

typedef kbd_rawkey PS2KbdRawKey;
typedef kbd_keymap PS2KbdKeyMap;

#ifdef __cplusplus
extern "C" {
#endif
	int PS2KbdInit(void);
	int PS2KbdRead(char *key);
	int PS2KbdReadRaw(PS2KbdRawKey *key);
	int PS2KbdSetReadmode(u32 readmode);
	int PS2KbdSetLeds(u8 leds);
	int PS2KbdSetKeymap(PS2KbdKeyMap *keymaps);
	int PS2KbdSetCtrlmap(u8 *ctrlmap);
	int PS2KbdSetAltmap(u8 *altmap);
	int PS2KbdSetSpecialmap(u8 *special);
	int PS2KbdFlushBuffer(void);
	int PS2KbdResetKeymap(void);
#ifdef __cplusplus
}
#endif

#endif
