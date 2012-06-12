/*
 * rdevice.h - Atari850 emulation header file
 *
 * Copyright (c) ???? Tom Hunt, Chris Martin
 * Copyright (c) 2003,2008 Atari800 development team (see DOC/CREDITS)
 *
 * This file is part of the Atari800 emulator project which emulates
 * the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
 *
 * Atari800 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atari800 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atari800; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef RDEVICE_H_
#define RDEVICE_H_

extern void RDevice_OPEN(void);
extern void RDevice_CLOS(void);
extern void RDevice_READ(void);
extern void RDevice_WRIT(void);
extern void RDevice_STAT(void);
extern void RDevice_SPEC(void);
extern void RDevice_INIT(void);

extern int RDevice_serial_enabled;
extern char RDevice_serial_device[];

extern void RDevice_Exit(void);

#endif /* RDEVICE_H_ */
