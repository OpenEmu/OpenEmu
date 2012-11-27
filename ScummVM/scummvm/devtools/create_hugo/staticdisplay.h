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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef STATICDISPLAY_H
#define STATICDISPLAY_H

#define SIZE_PAL_ARRAY 3*16

#if 1
// Color table of standard 16 VGA colors
// Values from "Programmers guide to EGA/VGA cards" Ferraro, p303
#define V1 168                                      // Low intensity value
#define V2 255                                      // High intensity value
#define V3 87                                       // Special for Brown/Gray
#define V4 32                                       // De-saturate hi intensity


byte _palette[SIZE_PAL_ARRAY] = {
	0,   0,   0,                                    // BLACK
	0,   0,   V1,                                   // BLUE
	0,   V1,  0,                                    // GREEN
	0,   V1,  V1,                                   // CYAN
	V1,  0,   0,                                    // RED
	V1,  0,   V1,                                   // MAGENTA
	V1,  V3,  0,                                    // BROWN
	V1,  V1,  V1,                                   // WHITE (LIGHT GRAY)
	V3,  V3,  V3,                                   // GRAY  (DARK GRAY)
	V4,  V4,  V2,                                   // LIGHTBLUE
	V4,  V2,  V4,                                   // LIGHTGREEN
	V4,  V2,  V2,                                   // LIGHTCYAN
	V2,  V4,  V4,                                   // LIGHTRED
	V2,  V4,  V2,                                   // LIGHTMAGENTA
	V2,  V2,  V4,                                   // YELLOW
	V2,  V2,  V2                                    // BRIGHTWHITE
};
#else
// Original paletter found in original exe.
// Currently disabled, because the result is quite ugly!
// Color table of nearest standard 16 colors in system static palette
#define C1 191                                      // Low intensity value
#define C2 255                                      // High intensity value
#define C3 127                                      // Special for Brown/Gray
byte _palette[SIZE_PAL_ARRAY] = {
	0,   0,   0,                                    // BLACK
	0,   0,   C1,                                   // BLUE
	0,   C1,  0,                                    // GREEN
	0,   C1,  C1,                                   // CYAN
	C1,  0,   0,                                    // RED
	C1,  0,   C1,                                   // MAGENTA
	C3,  C3,  0,                                    // BROWN
	C1,  C1,  C1,                                   // WHITE (LIGHT GRAY)
	C3,  C3,  C3,                                   // GRAY  (DARK GRAY)
	0,   0,   C2,                                   // LIGHTBLUE
	0,   C2,  0,                                    // LIGHTGREEN
	0,   C2,  C2,                                   // LIGHTCYAN
	C2,  0,   0,                                    // LIGHTRED
	C2,  0,   C2,                                   // LIGHTMAGENTA
	C2,  C2,  0,                                    // YELLOW
	C2,  C2,  C2                                    // BRIGHTWHITE
};
#endif


#endif
