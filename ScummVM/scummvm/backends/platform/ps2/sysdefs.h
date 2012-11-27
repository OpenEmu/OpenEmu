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

#ifndef __SYSDEFS_H__
#define __SYSDEFS_H__

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long uint64;
typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
typedef signed long int64;

enum Interrupts {
	INT_GS = 0,
	INT_VBLANK_START = 2,
	INT_VBLANK_END = 3,
	INT_TIMER0 = 9
};

// dma 2 registers
#define D2_CHCR (*(volatile uint32*)0x1000A000)
#define D2_QWC  (*(volatile uint32*)0x1000A020)
#define D2_TADR (*(volatile uint32*)0x1000A030)
#define D2_MADR (*(volatile uint32*)0x1000A010)
#define D2_ASR1 (*(volatile uint32*)0x1000A050)
#define D2_ASR0 (*(volatile uint32*)0x1000A040)

#define D_CTRL  (*(volatile uint32*)0x1000E000)
#define D_STAT  (*(volatile uint32*)0x1000E010)
#define D_PCR   (*(volatile uint32*)0x1000E020)
#define D_SQWC  (*(volatile uint32*)0x1000E030)
#define D_RBSR  (*(volatile uint32*)0x1000E040)
#define D_RBOR  (*(volatile uint32*)0x1000E050)
#define D_STADR (*(volatile uint32*)0x1000E060)

#define CIM2 (1 << 18)
#define CIS2 (1 << 2)


// timer 0 registers
#define T0_COUNT (*(volatile uint32*)0x10000000)
#define T0_MODE  (*(volatile uint32*)0x10000010)
#define T0_COMP  (*(volatile uint32*)0x10000020)
#define T0_HOLD  (*(volatile uint32*)0x10000030)

#define TIMER_MODE(clks, gate, gates, gatem, zeroret, cue, cmpe, ovfe, equf, ovff) \
	((clks) | ((gate) << 2) | ((gates) << 3) | ((gatem) << 4) | ((zeroret) << 6) | \
	((cue) << 7) | ((cmpe) << 8) | ((ovfe) << 9) | ((equf) << 10) | ((ovff) << 11))

#endif // __SYSDEFS_H__
