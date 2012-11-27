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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * List of all process identifiers
 */

#ifndef TINSEL_PID_H	// prevent multiple includes
#define TINSEL_PID_H

namespace Tinsel {

#define	PID_DESTROY	0x8000			// process id of any process that is to be destroyed between scenes

#define	PID_EFFECTS	(0x0010 | PID_DESTROY)	// generic special effects process id
#define	PID_FLASH	(PID_EFFECTS + 1)	// flash color process
#define	PID_CYCLE	(PID_EFFECTS + 2)	// cycle color range process
#define	PID_MORPH	(PID_EFFECTS + 3)	// morph process
#define	PID_FADER	(PID_EFFECTS + 4)	// fader process
#define	PID_FADE_BGND	(PID_EFFECTS + 5)	// fade background color process

#define	PID_BACKGND	(0x0020 | PID_DESTROY)	// background update process id

#define	PID_MOUSE	0x0030			// mouse button checking process id

#define	PID_JOYSTICK	0x0040			// joystick button checking process id

#define	PID_KEYBOARD	0x0050			// keyboard scanning process

#define	PID_CURSOR	0x0060			// cursor process
#define	PID_CUR_TRAIL	(PID_CURSOR + 1)	// cursor trail process

#define	PID_SCROLL	(0x0070 | PID_DESTROY)	// scroll process

#define	PID_INVENTORY	0x0080			// inventory process

#define	PID_POSITION	(0x0090 | PID_DESTROY)	// cursor position process

#define	PID_TAG		(0x00A0 | PID_DESTROY)	// tag process

#define	PID_TCODE	(0x00B0 | PID_DESTROY)	// tinsel code process

#define	PID_MASTER_SCR	0x00C0			// tinsel master script process

#define	PID_MOVER	(0x00D0 | PID_DESTROY)	// moving actor process

#define	PID_REEL	(0x00E0 | PID_DESTROY)	// process for each film reel

#define	PID_MIDI	(0x00F0 | PID_DESTROY)	// process to poll MIDI sound driver

#define PID_BMV		0x0100				// Movie player process

#define PID_BTN_CLICK 0x110				// process to handle mouse button clicks

#define PID_PROCESS	(0x0110 | PID_DESTROY)	// Scene process base

#define PID_GPROCESS	0x0120			// Global process base

} // End of namespace Tinsel

#endif	// TINSEL_PID_H
