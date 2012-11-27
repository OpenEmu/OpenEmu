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

#include "common/file.h"
#include "common/textconsole.h"

#include "graphics/cursorman.h"
#include "graphics/palette.h"

#include "agi/agi.h"
#include "agi/graphics.h"

namespace Agi {

#define DEV_X0(x) ((x) << 1)
#define DEV_X1(x) (((x) << 1) + 1)
#define DEV_Y(x) (x)

#ifndef MAX_INT
#  define MAX_INT (int)((unsigned)~0 >> 1)
#endif

#include "agi/font.h"

/**
 * 16 color RGB palette.
 * This array contains the 6-bit RGB values of the EGA palette exported
 * to the console drivers.
 */
static const uint8 egaPalette[16 * 3] = {
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x2a,
	0x00, 0x2a, 0x00,
	0x00, 0x2a, 0x2a,
	0x2a, 0x00, 0x00,
	0x2a, 0x00, 0x2a,
	0x2a, 0x15, 0x00,
	0x2a, 0x2a, 0x2a,
	0x15, 0x15, 0x15,
	0x15, 0x15, 0x3f,
	0x15, 0x3f, 0x15,
	0x15, 0x3f, 0x3f,
	0x3f, 0x15, 0x15,
	0x3f, 0x15, 0x3f,
	0x3f, 0x3f, 0x15,
	0x3f, 0x3f, 0x3f
};

/**
 * Atari ST AGI palette.
 * Used by all of the tested Atari ST AGI games
 * from Donald Duck's Playground (1986) to Manhunter II (1989).
 * 16 RGB colors. 3 bits per color component.
 */
static const uint8 atariStAgiPalette[16 * 3] = {
	0x0, 0x0, 0x0,
	0x0, 0x0, 0x7,
	0x0, 0x4, 0x0,
	0x0, 0x5, 0x4,
	0x5, 0x0, 0x0,
	0x5, 0x3, 0x6,
	0x4, 0x3, 0x0,
	0x5, 0x5, 0x5,
	0x3, 0x3, 0x2,
	0x0, 0x5, 0x7,
	0x0, 0x6, 0x0,
	0x0, 0x7, 0x6,
	0x7, 0x2, 0x3,
	0x7, 0x4, 0x7,
	0x7, 0x7, 0x4,
	0x7, 0x7, 0x7
};

/**
 * Second generation Apple IIGS AGI palette.
 * A 16-color, 12-bit RGB palette.
 *
 * Used by at least the following Apple IIGS AGI versions:
 * 1.003 (Leisure Suit Larry I  v1.0E, intro says 1987)
 * 1.005 (AGI Demo 2            1987-06-30?)
 * 1.006 (King's Quest I        v1.0S 1988-02-23)
 * 1.007 (Police Quest I        v2.0B 1988-04-21 8:00am)
 * 1.013 (King's Quest II       v2.0A 1988-06-16 (CE))
 * 1.013 (Mixed-Up Mother Goose v2.0A 1988-05-31 10:00am)
 * 1.014 (King's Quest III      v2.0A 1988-08-28 (CE))
 * 1.014 (Space Quest II        v2.0A, LOGIC.141 says 1988)
 * 2.004 (Manhunter I           v2.0E 1988-10-05 (CE))
 * 2.006 (King's Quest IV       v1.0K 1988-11-22 (CE))
 * 3.001 (Black Cauldron        v1.0O 1989-02-24 (CE))
 * 3.003 (Gold Rush!            v1.0M 1989-02-28 (CE))
 */
static const uint8 appleIIgsAgiPaletteV2[16 * 3] = {
	0x0, 0x0, 0x0,
	0x0, 0x0, 0xF,
	0x0, 0x8, 0x0,
	0x0, 0xD, 0xB,
	0xC, 0x0, 0x0,
	0xB, 0x7, 0xD,
	0x8, 0x5, 0x0,
	0xB, 0xB, 0xB,
	0x7, 0x7, 0x7,
	0x0, 0xB, 0xF,
	0x0, 0xE, 0x0,
	0x0, 0xF, 0xD,
	0xF, 0x9, 0x8,
	0xD, 0x9, 0xF, // Only this differs from the 1st generation palette
	0xE, 0xE, 0x0,
	0xF, 0xF, 0xF
};

/**
 * First generation Amiga & Apple IIGS AGI palette.
 * A 16-color, 12-bit RGB palette.
 *
 * Used by at least the following Amiga AGI versions:
 * 2.082 (King's Quest I   v1.0U 1986)
 * 2.082 (Space Quest I    v1.2  1986)
 * 2.090 (King's Quest III v1.01 1986-11-08)
 * 2.107 (King's Quest II  v2.0J 1987-01-29)
 * x.yyy (Black Cauldron   v2.00 1987-06-14)
 * x.yyy (Larry I          v1.05 1987-06-26)
 *
 * Also used by at least the following Apple IIGS AGI versions:
 * 1.002 (Space Quest I, intro says v2.2 1987)
 */
static const uint8 amigaAgiPaletteV1[16 * 3] = {
	0x0, 0x0, 0x0,
	0x0, 0x0, 0xF,
	0x0, 0x8, 0x0,
	0x0, 0xD, 0xB,
	0xC, 0x0, 0x0,
	0xB, 0x7, 0xD,
	0x8, 0x5, 0x0,
	0xB, 0xB, 0xB,
	0x7, 0x7, 0x7,
	0x0, 0xB, 0xF,
	0x0, 0xE, 0x0,
	0x0, 0xF, 0xD,
	0xF, 0x9, 0x8,
	0xF, 0x7, 0x0,
	0xE, 0xE, 0x0,
	0xF, 0xF, 0xF
};

/**
 * Second generation Amiga AGI palette.
 * A 16-color, 12-bit RGB palette.
 *
 * Used by at least the following Amiga AGI versions:
 * 2.202 (Space Quest II v2.0F. Intro says 1988. ScummVM 0.10.0 detects as 1986-12-09)
 */
static const uint8 amigaAgiPaletteV2[16 * 3] = {
	0x0, 0x0, 0x0,
	0x0, 0x0, 0xF,
	0x0, 0x8, 0x0,
	0x0, 0xD, 0xB,
	0xC, 0x0, 0x0,
	0xB, 0x7, 0xD,
	0x8, 0x5, 0x0,
	0xB, 0xB, 0xB,
	0x7, 0x7, 0x7,
	0x0, 0xB, 0xF,
	0x0, 0xE, 0x0,
	0x0, 0xF, 0xD,
	0xF, 0x9, 0x8,
	0xD, 0x0, 0xF,
	0xE, 0xE, 0x0,
	0xF, 0xF, 0xF
};

/**
 * Third generation Amiga AGI palette.
 * A 16-color, 12-bit RGB palette.
 *
 * Used by at least the following Amiga AGI versions:
 * 2.310 (Police Quest I   v2.0B 1989-02-22)
 * 2.316 (Gold Rush!       v2.05 1989-03-09)
 * x.yyy (Manhunter I      v1.06 1989-03-18)
 * 2.333 (Manhunter II     v3.06 1989-08-17)
 * 2.333 (King's Quest III v2.15 1989-11-15)
 */
static const uint8 amigaAgiPaletteV3[16 * 3] = {
	0x0, 0x0, 0x0,
	0x0, 0x0, 0xB,
	0x0, 0xB, 0x0,
	0x0, 0xB, 0xB,
	0xB, 0x0, 0x0,
	0xB, 0x0, 0xB,
	0xC, 0x7, 0x0,
	0xB, 0xB, 0xB,
	0x7, 0x7, 0x7,
	0x0, 0x0, 0xF,
	0x0, 0xF, 0x0,
	0x0, 0xF, 0xF,
	0xF, 0x0, 0x0,
	0xF, 0x0, 0xF,
	0xF, 0xF, 0x0,
	0xF, 0xF, 0xF
};

/**
 * 16 color amiga-ish palette.
 */
static const uint8 newPalette[16 * 3] = {
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x3f,
	0x00, 0x2A, 0x00,
	0x00, 0x2A, 0x2A,
	0x33, 0x00, 0x00,
	0x2f, 0x1c, 0x37,
	0x23, 0x14, 0x00,
	0x2f, 0x2f, 0x2f,
	0x15, 0x15, 0x15,
	0x00, 0x2f, 0x3f,
	0x00, 0x33, 0x15,
	0x15, 0x3F, 0x3F,
	0x3f, 0x27, 0x23,
	0x3f, 0x15, 0x3f,
	0x3b, 0x3b, 0x00,
	0x3F, 0x3F, 0x3F
};

/**
 * 256 color palette used with AGI256 and AGI256-2 games.
 * Uses full 8 bits per color component.
 */
static const uint8 vgaPalette[256 * 3] = {
	0x00, 0x00, 0x00,
	0x00, 0x00, 0xA8,
	0x00, 0xA8, 0x00,
	0x00, 0xA8, 0xA8,
	0xA8, 0x00, 0x00,
	0xA8, 0x00, 0xA8,
	0xA8, 0x54, 0x00,
	0xA8, 0xA8, 0xA8,
	0x54, 0x54, 0x54,
	0x54, 0x54, 0xFC,
	0x54, 0xFC, 0x54,
	0x54, 0xFC, 0xFC,
	0xFC, 0x54, 0x54,
	0xFC, 0x54, 0xFC,
	0xFC, 0xFC, 0x54,
	0xFC, 0xFC, 0xFC,
	0x00, 0x00, 0x00,
	0x14, 0x14, 0x14,
	0x20, 0x20, 0x20,
	0x2C, 0x2C, 0x2C,
	0x38, 0x38, 0x38,
	0x44, 0x44, 0x44,
	0x50, 0x50, 0x50,
	0x60, 0x60, 0x60,
	0x70, 0x70, 0x70,
	0x80, 0x80, 0x80,
	0x90, 0x90, 0x90,
	0xA0, 0xA0, 0xA0,
	0xB4, 0xB4, 0xB4,
	0xC8, 0xC8, 0xC8,
	0xE0, 0xE0, 0xE0,
	0xFC, 0xFC, 0xFC,
	0x00, 0x00, 0xFC,
	0x40, 0x00, 0xFC,
	0x7C, 0x00, 0xFC,
	0xBC, 0x00, 0xFC,
	0xFC, 0x00, 0xFC,
	0xFC, 0x00, 0xBC,
	0xFC, 0x00, 0x7C,
	0xFC, 0x00, 0x40,
	0xFC, 0x00, 0x00,
	0xFC, 0x40, 0x00,
	0xFC, 0x7C, 0x00,
	0xFC, 0xBC, 0x00,
	0xFC, 0xFC, 0x00,
	0xBC, 0xFC, 0x00,
	0x7C, 0xFC, 0x00,
	0x40, 0xFC, 0x00,
	0x00, 0xFC, 0x00,
	0x00, 0xFC, 0x40,
	0x00, 0xFC, 0x7C,
	0x00, 0xFC, 0xBC,
	0x00, 0xFC, 0xFC,
	0x00, 0xBC, 0xFC,
	0x00, 0x7C, 0xFC,
	0x00, 0x40, 0xFC,
	0x7C, 0x7C, 0xFC,
	0x9C, 0x7C, 0xFC,
	0xBC, 0x7C, 0xFC,
	0xDC, 0x7C, 0xFC,
	0xFC, 0x7C, 0xFC,
	0xFC, 0x7C, 0xDC,
	0xFC, 0x7C, 0xBC,
	0xFC, 0x7C, 0x9C,
	0xFC, 0x7C, 0x7C,
	0xFC, 0x9C, 0x7C,
	0xFC, 0xBC, 0x7C,
	0xFC, 0xDC, 0x7C,
	0xFC, 0xFC, 0x7C,
	0xDC, 0xFC, 0x7C,
	0xBC, 0xFC, 0x7C,
	0x9C, 0xFC, 0x7C,
	0x7C, 0xFC, 0x7C,
	0x7C, 0xFC, 0x9C,
	0x7C, 0xFC, 0xBC,
	0x7C, 0xFC, 0xDC,
	0x7C, 0xFC, 0xFC,
	0x7C, 0xDC, 0xFC,
	0x7C, 0xBC, 0xFC,
	0x7C, 0x9C, 0xFC,
	0xB4, 0xB4, 0xFC,
	0xC4, 0xB4, 0xFC,
	0xD8, 0xB4, 0xFC,
	0xE8, 0xB4, 0xFC,
	0xFC, 0xB4, 0xFC,
	0xFC, 0xB4, 0xE8,
	0xFC, 0xB4, 0xD8,
	0xFC, 0xB4, 0xC4,
	0xFC, 0xB4, 0xB4,
	0xFC, 0xC4, 0xB4,
	0xFC, 0xD8, 0xB4,
	0xFC, 0xE8, 0xB4,
	0xFC, 0xFC, 0xB4,
	0xE8, 0xFC, 0xB4,
	0xD8, 0xFC, 0xB4,
	0xC4, 0xFC, 0xB4,
	0xB4, 0xFC, 0xB4,
	0xB4, 0xFC, 0xC4,
	0xB4, 0xFC, 0xD8,
	0xB4, 0xFC, 0xE8,
	0xB4, 0xFC, 0xFC,
	0xB4, 0xE8, 0xFC,
	0xB4, 0xD8, 0xFC,
	0xB4, 0xC4, 0xFC,
	0x00, 0x00, 0x70,
	0x1C, 0x00, 0x70,
	0x38, 0x00, 0x70,
	0x54, 0x00, 0x70,
	0x70, 0x00, 0x70,
	0x70, 0x00, 0x54,
	0x70, 0x00, 0x38,
	0x70, 0x00, 0x1C,
	0x70, 0x00, 0x00,
	0x70, 0x1C, 0x00,
	0x70, 0x38, 0x00,
	0x70, 0x54, 0x00,
	0x70, 0x70, 0x00,
	0x54, 0x70, 0x00,
	0x38, 0x70, 0x00,
	0x1C, 0x70, 0x00,
	0x00, 0x70, 0x00,
	0x00, 0x70, 0x1C,
	0x00, 0x70, 0x38,
	0x00, 0x70, 0x54,
	0x00, 0x70, 0x70,
	0x00, 0x54, 0x70,
	0x00, 0x38, 0x70,
	0x00, 0x1C, 0x70,
	0x38, 0x38, 0x70,
	0x44, 0x38, 0x70,
	0x54, 0x38, 0x70,
	0x60, 0x38, 0x70,
	0x70, 0x38, 0x70,
	0x70, 0x38, 0x60,
	0x70, 0x38, 0x54,
	0x70, 0x38, 0x44,
	0x70, 0x38, 0x38,
	0x70, 0x44, 0x38,
	0x70, 0x54, 0x38,
	0x70, 0x60, 0x38,
	0x70, 0x70, 0x38,
	0x60, 0x70, 0x38,
	0x54, 0x70, 0x38,
	0x44, 0x70, 0x38,
	0x38, 0x70, 0x38,
	0x38, 0x70, 0x44,
	0x38, 0x70, 0x54,
	0x38, 0x70, 0x60,
	0x38, 0x70, 0x70,
	0x38, 0x60, 0x70,
	0x38, 0x54, 0x70,
	0x38, 0x44, 0x70,
	0x50, 0x50, 0x70,
	0x58, 0x50, 0x70,
	0x60, 0x50, 0x70,
	0x68, 0x50, 0x70,
	0x70, 0x50, 0x70,
	0x70, 0x50, 0x68,
	0x70, 0x50, 0x60,
	0x70, 0x50, 0x58,
	0x70, 0x50, 0x50,
	0x70, 0x58, 0x50,
	0x70, 0x60, 0x50,
	0x70, 0x68, 0x50,
	0x70, 0x70, 0x50,
	0x68, 0x70, 0x50,
	0x60, 0x70, 0x50,
	0x58, 0x70, 0x50,
	0x50, 0x70, 0x50,
	0x50, 0x70, 0x58,
	0x50, 0x70, 0x60,
	0x50, 0x70, 0x68,
	0x50, 0x70, 0x70,
	0x50, 0x68, 0x70,
	0x50, 0x60, 0x70,
	0x50, 0x58, 0x70,
	0x00, 0x00, 0x40,
	0x10, 0x00, 0x40,
	0x20, 0x00, 0x40,
	0x30, 0x00, 0x40,
	0x40, 0x00, 0x40,
	0x40, 0x00, 0x30,
	0x40, 0x00, 0x20,
	0x40, 0x00, 0x10,
	0x40, 0x00, 0x00,
	0x40, 0x10, 0x00,
	0x40, 0x20, 0x00,
	0x40, 0x30, 0x00,
	0x40, 0x40, 0x00,
	0x30, 0x40, 0x00,
	0x20, 0x40, 0x00,
	0x10, 0x40, 0x00,
	0x00, 0x40, 0x00,
	0x00, 0x40, 0x10,
	0x00, 0x40, 0x20,
	0x00, 0x40, 0x30,
	0x00, 0x40, 0x40,
	0x00, 0x30, 0x40,
	0x00, 0x20, 0x40,
	0x00, 0x10, 0x40,
	0x20, 0x20, 0x40,
	0x28, 0x20, 0x40,
	0x30, 0x20, 0x40,
	0x38, 0x20, 0x40,
	0x40, 0x20, 0x40,
	0x40, 0x20, 0x38,
	0x40, 0x20, 0x30,
	0x40, 0x20, 0x28,
	0x40, 0x20, 0x20,
	0x40, 0x28, 0x20,
	0x40, 0x30, 0x20,
	0x40, 0x38, 0x20,
	0x40, 0x40, 0x20,
	0x38, 0x40, 0x20,
	0x30, 0x40, 0x20,
	0x28, 0x40, 0x20,
	0x20, 0x40, 0x20,
	0x20, 0x40, 0x28,
	0x20, 0x40, 0x30,
	0x20, 0x40, 0x38,
	0x20, 0x40, 0x40,
	0x20, 0x38, 0x40,
	0x20, 0x30, 0x40,
	0x20, 0x28, 0x40,
	0x2C, 0x2C, 0x40,
	0x30, 0x2C, 0x40,
	0x34, 0x2C, 0x40,
	0x3C, 0x2C, 0x40,
	0x40, 0x2C, 0x40,
	0x40, 0x2C, 0x3C,
	0x40, 0x2C, 0x34,
	0x40, 0x2C, 0x30,
	0x40, 0x2C, 0x2C,
	0x40, 0x30, 0x2C,
	0x40, 0x34, 0x2C,
	0x40, 0x3C, 0x2C,
	0x40, 0x40, 0x2C,
	0x3C, 0x40, 0x2C,
	0x34, 0x40, 0x2C,
	0x30, 0x40, 0x2C,
	0x2C, 0x40, 0x2C,
	0x2C, 0x40, 0x30,
	0x2C, 0x40, 0x34,
	0x2C, 0x40, 0x3C,
	0x2C, 0x40, 0x40,
	0x2C, 0x3C, 0x40,
	0x2C, 0x34, 0x40,
	0x2C, 0x30, 0x40,
	0x40, 0x40, 0x40,
	0x38, 0x38, 0x38,
	0x30, 0x30, 0x30,
	0x28, 0x28, 0x28,
	0x24, 0x24, 0x24,
	0x1C, 0x1C, 0x1C,
	0x14, 0x14, 0x14,
	0x0C, 0x0C, 0x0C
};

static const uint16 cgaMap[16] = {
	0x0000,			//  0 - black
	0x0d00,			//  1 - blue
	0x0b00,			//  2 - green
	0x0f00,			//  3 - cyan
	0x000b,			//  4 - red
	0x0b0d,			//  5 - magenta
	0x000d,			//  6 - brown
	0x0b0b,			//  7 - gray
	0x0d0d,			//  8 - dark gray
	0x0b0f,			//  9 - light blue
	0x0b0d,			// 10 - light green
	0x0f0d,			// 11 - light cyan
	0x0f0d,			// 12 - light red
	0x0f00,			// 13 - light magenta
	0x0f0b,			// 14 - yellow
	0x0f0f			// 15 - white
};

struct UpdateBlock {
	int x1, y1;
	int x2, y2;
};

static struct UpdateBlock update = {
	MAX_INT, MAX_INT, 0, 0
};

GfxMgr::GfxMgr(AgiBase *vm) : _vm(vm) {
	_shakeH = NULL;
	_shakeV = NULL;
	_agipalFileNum = 0;
	_currentCursorPalette = 0;	// cursor palette not set
}


//
//  Layer 4:  640x480?  ==================  User display
//                              ^
//                              |  do_update(), put_block()
//                              |
//  Layer 3:  640x480?  ==================  Framebuffer
//                              ^
//                              |  flush_block(), put_pixels()
//                              |
//  Layer 2:  320x200   ==================  AGI engine screen (console), put_pixel()
//                              |
//  Layer 1:  160x336   ==================  AGI screen
//
//  Upper half (160x168) of Layer 1 is for the normal 16 color & control line/priority info.
//  Lower half (160x168) of Layer 1 is for the 256 color information (Used with AGI256 & AGI256-2).
//

#define SHAKE_MAG 3

void GfxMgr::shakeStart() {
	int i;

	if ((_shakeH = (uint8 *)malloc(GFX_WIDTH * SHAKE_MAG)) == NULL)
		return;

	if ((_shakeV = (uint8 *)malloc(SHAKE_MAG * (GFX_HEIGHT - SHAKE_MAG))) == NULL) {
		free(_shakeH);
		return;
	}

	for (i = 0; i < GFX_HEIGHT - SHAKE_MAG; i++) {
		memcpy(_shakeV + i * SHAKE_MAG, _agiScreen + i * GFX_WIDTH, SHAKE_MAG);
	}

	for (i = 0; i < SHAKE_MAG; i++) {
		memcpy(_shakeH + i * GFX_WIDTH, _agiScreen + i * GFX_WIDTH, GFX_WIDTH);
	}
}

void GfxMgr::shakeScreen(int n) {
	int i;

	if (n == 0) {
		for (i = 0; i < (GFX_HEIGHT - SHAKE_MAG); i++) {
			memmove(&_agiScreen[GFX_WIDTH * i],
					&_agiScreen[GFX_WIDTH * (i + SHAKE_MAG) + SHAKE_MAG],
					GFX_WIDTH - SHAKE_MAG);
		}
	} else {
		for (i = GFX_HEIGHT - SHAKE_MAG - 1; i >= 0; i--) {
			memmove(&_agiScreen[GFX_WIDTH * (i + SHAKE_MAG) + SHAKE_MAG],
					&_agiScreen[GFX_WIDTH * i], GFX_WIDTH - SHAKE_MAG);
		}
	}
}

void GfxMgr::shakeEnd() {
	int i;

	for (i = 0; i < GFX_HEIGHT - SHAKE_MAG; i++) {
		memcpy(_agiScreen + i * GFX_WIDTH, _shakeV + i * SHAKE_MAG, SHAKE_MAG);
	}

	for (i = 0; i < SHAKE_MAG; i++) {
		memcpy(_agiScreen + i * GFX_WIDTH, _shakeH + i * GFX_WIDTH, GFX_WIDTH);
	}

	flushBlock(0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);

	free(_shakeV);
	free(_shakeH);
}

void GfxMgr::putTextCharacter(int l, int x, int y, unsigned char c, int fg, int bg, bool checkerboard, const uint8 *font) {
	int x1, y1, xx, yy, cc;
	const uint8 *p;

	p = font + ((unsigned int)c * CHAR_LINES);
	for (y1 = 0; y1 < CHAR_LINES; y1++) {
		for (x1 = 0; x1 < CHAR_COLS; x1++) {
			xx = x + x1;
			yy = y + y1;
			cc = (*p & (1 << (7 - x1))) ? fg : bg;
			_agiScreen[xx + yy * GFX_WIDTH] = cc;
		}

		p++;
	}

	// Simple checkerboard effect to simulate "greyed out" text.
	// This is what Sierra's interpreter does for things like menu items
	// that aren't selectable (such as separators). -- dsymonds
	if (checkerboard) {
		for (yy = y; yy < y + CHAR_LINES; yy++)
			for (xx = x + (~yy & 1); xx < x + CHAR_COLS; xx += 2)
				_agiScreen[xx + yy * GFX_WIDTH] = 15;
	}

	// FIXME: we don't want this when we're writing on the
	//        console!
	flushBlock(x, y, x + CHAR_COLS - 1, y + CHAR_LINES - 1);
}

void GfxMgr::drawRectangle(int x1, int y1, int x2, int y2, int c) {
	int y, w, h;
	uint8 *p0;

	if (x1 >= GFX_WIDTH)
		x1 = GFX_WIDTH - 1;
	if (y1 >= GFX_HEIGHT)
		y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)
		x2 = GFX_WIDTH - 1;
	if (y2 >= GFX_HEIGHT)
		y2 = GFX_HEIGHT - 1;

	w = x2 - x1 + 1;
	h = y2 - y1 + 1;
	p0 = &_agiScreen[x1 + y1 * GFX_WIDTH];
	for (y = 0; y < h; y++) {
		memset(p0, c, w);
		p0 += GFX_WIDTH;
	}
}

void GfxMgr::drawFrame(int x1, int y1, int x2, int y2, int c1, int c2) {
	int y, w;
	uint8 *p0;

	// top line
	w = x2 - x1 + 1;
	p0 = &_agiScreen[x1 + y1 * GFX_WIDTH];
	memset(p0, c1, w);

	// bottom line
	p0 = &_agiScreen[x1 + y2 * GFX_WIDTH];
	memset(p0, c2, w);

	// side lines
	for (y = y1; y <= y2; y++) {
		_agiScreen[x1 + y * GFX_WIDTH] = c1;
		_agiScreen[x2 + y * GFX_WIDTH] = c2;
	}
}

void GfxMgr::drawBox(int x1, int y1, int x2, int y2, int color1, int color2, int m) {
	x1 += m;
	y1 += m;
	x2 -= m;
	y2 -= m;

	drawRectangle(x1, y1, x2, y2, color1);
	drawFrame(x1 + 2, y1 + 2, x2 - 2, y2 - 2, color2, color2);
	flushBlock(x1, y1, x2, y2);
}

void GfxMgr::printCharacter(int x, int y, char c, int fg, int bg) {
	x *= CHAR_COLS;
	y *= CHAR_LINES;

	putTextCharacter(0, x, y, c, fg, bg);
	// redundant! already inside put_text_character!
	// flush_block (x, y, x + CHAR_COLS - 1, y + CHAR_LINES - 1);
}

/**
 * Draw a default style button.
 * Swaps background and foreground color if button is in focus or being pressed.
 * @param x  x coordinate of the button
 * @param y  y coordinate of the button
 * @param a  set if the button has focus
 * @param p  set if the button is pressed
 * @param fgcolor foreground color of the button when it is neither in focus nor being pressed
 * @param bgcolor background color of the button when it is neither in focus nor being pressed
 */
void GfxMgr::drawDefaultStyleButton(int x, int y, const char *s, int a, int p, int fgcolor, int bgcolor) {
	int textOffset     = _vm->_defaultButtonStyle.getTextOffset(a > 0, p > 0);
	AgiTextColor color = _vm->_defaultButtonStyle.getColor     (a > 0, p > 0, fgcolor, bgcolor);
	bool border        = _vm->_defaultButtonStyle.getBorder    (a > 0, p > 0);

	rawDrawButton(x, y, s, color.fg, color.bg, border, textOffset);
}

/**
 * Draw a button using the currently chosen style.
 * Amiga-style is used for the Amiga-rendering mode, PC-style is used otherwise.
 * @param x  x coordinate of the button
 * @param y  y coordinate of the button
 * @param hasFocus  set if the button has focus
 * @param pressed  set if the button is pressed
 * @param positive  set if button is positive, otherwise button is negative (Only matters with Amiga-style buttons)
 * TODO: Make Amiga-style buttons a bit wider as they were in Amiga AGI games.
 */
void GfxMgr::drawCurrentStyleButton(int x, int y, const char *label, bool hasFocus, bool pressed, bool positive) {
	int textOffset     = _vm->_buttonStyle.getTextOffset(hasFocus, pressed);
	AgiTextColor color = _vm->_buttonStyle.getColor(hasFocus, pressed, positive);
	bool border        = _vm->_buttonStyle.getBorder(hasFocus, pressed);

	rawDrawButton(x, y, label, color.fg, color.bg, border, textOffset);
}

void GfxMgr::rawDrawButton(int x, int y, const char *s, int fgcolor, int bgcolor, bool border, int textOffset) {
	int len = strlen(s);
	int x1, y1, x2, y2;

	x1 = x - 3;
	y1 = y - 3;
	x2 = x + CHAR_COLS * len + 2;
	y2 = y + CHAR_LINES + 2;

	// Draw a filled rectangle that's larger than the button. Used for drawing
	// a border around the button as the button itself is drawn after this.
	drawRectangle(x1, y1, x2, y2, border ? BUTTON_BORDER : MSG_BOX_COLOR);

	while (*s) {
		putTextCharacter(0, x + textOffset, y + textOffset, *s++, fgcolor, bgcolor);
		x += CHAR_COLS;
	}

	x1 -= 2;
	y1 -= 2;
	x2 += 2;
	y2 += 2;

	flushBlock(x1, y1, x2, y2);
}

int GfxMgr::testButton(int x, int y, const char *s) {
	int len = strlen(s);
	Common::Rect rect(x - 3, y - 3, x + CHAR_COLS * len + 3, y + CHAR_LINES + 3);
	return rect.contains(_vm->_mouse.x, _vm->_mouse.y);
}

void GfxMgr::putBlock(int x1, int y1, int x2, int y2) {
	gfxPutBlock(x1, y1, x2, y2);
}

void GfxMgr::putScreen() {
	putBlock(0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);
}

/*
 * Public functions
 */

/**
 * Initialize the color palette
 * This function initializes the color palette using the specified
 * RGB palette.
 * @param p           A pointer to the source RGB palette.
 * @param colorCount  Count of colors in the source palette.
 * @param fromBits    Bits per source color component.
 * @param toBits      Bits per destination color component.
 */
void GfxMgr::initPalette(const uint8 *p, uint colorCount, uint fromBits, uint toBits) {
	const uint srcMax  = (1 << fromBits) - 1;
	const uint destMax = (1 << toBits) - 1;
	for (uint col = 0; col < colorCount; col++) {
		for (uint comp = 0; comp < 3; comp++) { // Convert RGB components
			_palette[col * 3 + comp] = (p[col * 3 + comp] * destMax) / srcMax;
		}
	}
}

void GfxMgr::gfxSetPalette() {
	g_system->getPaletteManager()->setPalette(_palette, 0, 256);
}

//Gets AGIPAL Data
void GfxMgr::setAGIPal(int p0) {
	//If 0 from savefile, do not use
	if (p0 == 0)
		return;

	char filename[15];
	sprintf(filename, "pal.%d", p0);

	Common::File agipal;
	if (!agipal.open(filename)) {
		warning("Couldn't open AGIPAL palette file '%s'. Not changing palette", filename);
		return; // Needed at least by Naturette 3 which uses AGIPAL but provides no palette files
	}

	//Chunk0 holds colors 0-7
	agipal.read(&_agipalPalette[0], 24);

	//Chunk1 is the same as the chunk0

	//Chunk2 chunk holds colors 8-15
	agipal.seek(24, SEEK_CUR);
	agipal.read(&_agipalPalette[24], 24);

	//Chunk3 is the same as the chunk2

	//Chunks4-7 are duplicates of chunks0-3

	if (agipal.eos() || agipal.err()) {
		warning("Couldn't read AGIPAL palette from '%s'. Not changing palette", filename);
		return;
	}

	// Use only the lowest 6 bits of each color component (Red, Green and Blue)
	// because VGA used only 6 bits per color component (i.e. VGA had 18-bit colors).
	// This should now be identical to the original AGIPAL-hack's behavior.
	bool validVgaPalette = true;
	for (int i = 0; i < 16 * 3; i++) {
		if (_agipalPalette[i] >= (1 << 6)) {
			_agipalPalette[i] &= 0x3F; // Leave only the lowest 6 bits of each color component
			validVgaPalette = false;
		}
	}

	if (!validVgaPalette)
		warning("Invalid AGIPAL palette (Over 6 bits per color component) in '%s'. Using only the lowest 6 bits per color component", filename);

	_agipalFileNum = p0;

	initPalette(_agipalPalette);
	gfxSetPalette();

	debug(1, "Using AGIPAL palette from '%s'", filename);
}

int GfxMgr::getAGIPalFileNum() {
	return _agipalFileNum;
}

// put a block onto the screen
void GfxMgr::gfxPutBlock(int x1, int y1, int x2, int y2) {
	if (x1 >= GFX_WIDTH)
		x1 = GFX_WIDTH - 1;
	if (y1 >= GFX_HEIGHT)
		y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)
		x2 = GFX_WIDTH - 1;
	if (y2 >= GFX_HEIGHT)
		y2 = GFX_HEIGHT - 1;

	g_system->copyRectToScreen(_screen + y1 * 320 + x1, 320, x1, y1, x2 - x1 + 1, y2 - y1 + 1);
}

/**
 * A black and white SCI-style arrow cursor (11x16).
 * 0 = Transparent.
 * 1 = Black (#000000 in 24-bit RGB).
 * 2 = White (#FFFFFF in 24-bit RGB).
 */
static const byte sciMouseCursor[] = {
	1,1,0,0,0,0,0,0,0,0,0,
	1,2,1,0,0,0,0,0,0,0,0,
	1,2,2,1,0,0,0,0,0,0,0,
	1,2,2,2,1,0,0,0,0,0,0,
	1,2,2,2,2,1,0,0,0,0,0,
	1,2,2,2,2,2,1,0,0,0,0,
	1,2,2,2,2,2,2,1,0,0,0,
	1,2,2,2,2,2,2,2,1,0,0,
	1,2,2,2,2,2,2,2,2,1,0,
	1,2,2,2,2,2,2,2,2,2,1,
	1,2,2,2,2,2,1,0,0,0,0,
	1,2,1,0,1,2,2,1,0,0,0,
	1,1,0,0,1,2,2,1,0,0,0,
	0,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,0,1,2,2,1,0
};

/**
 * A black and white Apple IIGS style arrow cursor (9x11).
 * 0 = Transparent.
 * 1 = Black (#000000 in 24-bit RGB).
 * 2 = White (#FFFFFF in 24-bit RGB).
 */
static const byte appleIIgsMouseCursor[] = {
	2,2,0,0,0,0,0,0,0,
	2,1,2,0,0,0,0,0,0,
	2,1,1,2,0,0,0,0,0,
	2,1,1,1,2,0,0,0,0,
	2,1,1,1,1,2,0,0,0,
	2,1,1,1,1,1,2,0,0,
	2,1,1,1,1,1,1,2,0,
	2,1,1,1,1,1,1,1,2,
	2,1,1,2,1,1,2,2,0,
	2,2,2,0,2,1,1,2,0,
	0,0,0,0,0,2,2,2,0
};

/**
 * RGB-palette for the black and white SCI and Apple IIGS arrow cursors.
 */
static const byte sciMouseCursorPalette[] = {
	0x00, 0x00, 0x00, // Black
	0xFF, 0xFF, 0xFF  // White
};

/**
 * An Amiga-style arrow cursor (8x11).
 * 0 = Transparent.
 * 1 = Black     (#000000 in 24-bit RGB).
 * 2 = Red       (#DE2021 in 24-bit RGB).
 * 3 = Light red (#FFCFAD in 24-bit RGB).
 */
static const byte amigaMouseCursor[] = {
	2,3,1,0,0,0,0,0,
	2,2,3,1,0,0,0,0,
	2,2,2,3,1,0,0,0,
	2,2,2,2,3,1,0,0,
	2,2,2,2,2,3,1,0,
	2,2,2,2,2,2,3,1,
	2,0,2,2,3,1,0,0,
	0,0,0,2,3,1,0,0,
	0,0,0,2,2,3,1,0,
	0,0,0,0,2,3,1,0,
	0,0,0,0,2,2,3,1
};

/**
 * RGB-palette for the Amiga-style arrow cursor
 * and the Amiga-style busy cursor.
 */
static const byte amigaMouseCursorPalette[] = {
	0x00, 0x00, 0x00, // Black
	0xDE, 0x20, 0x21, // Red
	0xFF, 0xCF, 0xAD  // Light red
};

/**
 * An Amiga-style busy cursor showing an hourglass (13x16).
 * 0 = Transparent.
 * 1 = Black     (#000000 in 24-bit RGB).
 * 2 = Red       (#DE2021 in 24-bit RGB).
 * 3 = Light red (#FFCFAD in 24-bit RGB).
 */
static const byte busyAmigaMouseCursor[] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,2,2,2,2,2,2,2,2,2,2,2,1,
	1,2,2,2,2,2,2,2,2,2,2,2,1,
	0,1,3,3,3,3,3,3,3,3,3,1,0,
	0,0,1,3,3,3,3,3,3,3,1,0,0,
	0,0,0,1,3,3,3,3,3,1,0,0,0,
	0,0,0,0,1,3,3,3,1,0,0,0,0,
	0,0,0,0,0,1,3,1,0,0,0,0,0,
	0,0,0,0,0,1,3,1,0,0,0,0,0,
	0,0,0,0,1,2,3,2,1,0,0,0,0,
	0,0,0,1,2,2,3,2,2,1,0,0,0,
	0,0,1,2,2,2,3,2,2,2,1,0,0,
	0,1,2,2,2,3,3,3,2,2,2,1,0,
	1,3,3,3,3,3,3,3,3,3,3,3,1,
	1,3,3,3,3,3,3,3,3,3,3,3,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1
};

void GfxMgr::setCursor(bool amigaStyleCursor, bool busy) {
	if (busy) {
		CursorMan.replaceCursorPalette(amigaMouseCursorPalette, 1, ARRAYSIZE(amigaMouseCursorPalette) / 3);
		CursorMan.replaceCursor(busyAmigaMouseCursor, 13, 16, 7, 8, 0);

		return;
	}

	if (!amigaStyleCursor) {
		CursorMan.replaceCursorPalette(sciMouseCursorPalette, 1, ARRAYSIZE(sciMouseCursorPalette) / 3);
		CursorMan.replaceCursor(sciMouseCursor, 11, 16, 1, 1, 0);
	} else { // amigaStyleCursor
		CursorMan.replaceCursorPalette(amigaMouseCursorPalette, 1, ARRAYSIZE(amigaMouseCursorPalette) / 3);
		CursorMan.replaceCursor(amigaMouseCursor, 8, 11, 1, 1, 0);
	}
}

void GfxMgr::setCursorPalette(bool amigaStyleCursor) {
	if (!amigaStyleCursor) {
		if (_currentCursorPalette != 1) {
			CursorMan.replaceCursorPalette(sciMouseCursorPalette, 1, ARRAYSIZE(sciMouseCursorPalette) / 3);
			_currentCursorPalette = 1;
		}
	} else { // amigaStyleCursor
		if (_currentCursorPalette != 2) {
			CursorMan.replaceCursorPalette(amigaMouseCursorPalette, 1, ARRAYSIZE(amigaMouseCursorPalette) / 3);
			_currentCursorPalette = 2;
		}
	}
}

/**
 * Initialize graphics device.
 *
 * @see deinit_video()
 */
int GfxMgr::initVideo() {
	if (_vm->getFeatures() & (GF_AGI256 | GF_AGI256_2))
		initPalette(vgaPalette, 256, 8);
	else if (_vm->_renderMode == Common::kRenderEGA)
		initPalette(egaPalette);
	else
		initPalette(newPalette);

	if ((_agiScreen = (uint8 *)calloc(GFX_WIDTH, GFX_HEIGHT)) == NULL)
		return errNotEnoughMemory;

	gfxSetPalette();

	setCursor(_vm->_renderMode == Common::kRenderAmiga);

	return errOK;
}

/**
 * Deinitialize graphics device.
 *
 * @see init_video()
 */
int GfxMgr::deinitVideo() {
	free(_agiScreen);

	return errOK;
}

int GfxMgr::initMachine() {
	_screen = (unsigned char *)malloc(320 * 200);
	_vm->_clockCount = 0;

	return errOK;
}

int GfxMgr::deinitMachine() {
	free(_screen);

	return errOK;
}

/**
 * Write pixels on the output device.
 * This function writes a row of pixels on the output device. Only the
 * lower 4 bits of each pixel in the row will be used, making this
 * function suitable for use with rows from the AGI screen.
 * @param x x coordinate of the row start (AGI coord.)
 * @param y y coordinate of the row start (AGI coord.)
 * @param n number of pixels in the row
 * @param p pointer to the row start in the AGI screen (Always use sbuf16c as base, not sbuf256c)
 * FIXME: CGA rendering doesn't work correctly with AGI256 or AGI256-2.
 */
void GfxMgr::putPixelsA(int x, int y, int n, uint8 *p) {
	const uint rShift = _vm->_debug.priority ? 4 : 0; // Priority information is in the top 4 bits of a byte taken from sbuf16c.

	// Choose the correct screen to read from. If AGI256 or AGI256-2 is used and we're not trying to show the priority information,
	// then choose the 256 color screen, otherwise choose the 16 color screen (Which also has the priority information).
	p += ((_vm->getFeatures() & (GF_AGI256 | GF_AGI256_2)) && !_vm->_debug.priority) ? FROM_SBUF16_TO_SBUF256_OFFSET : 0;

	if (_vm->_renderMode == Common::kRenderCGA) {
		for (x *= 2; n--; p++, x += 2) {
			register uint16 q = (cgaMap[(*p & 0xf0) >> 4] << 4) | cgaMap[*p & 0x0f];
			*(uint16 *)&_agiScreen[x + y * GFX_WIDTH] = (q >> rShift) & 0x0f0f;
		}
	} else {
		const uint16 mask = ((_vm->getFeatures() & (GF_AGI256 | GF_AGI256_2)) && !_vm->_debug.priority) ? 0xffff : 0x0f0f;
		for (x *= 2; n--; p++, x += 2) {
			register uint16 q = ((uint16)*p << 8) | *p;
			*(uint16 *)&_agiScreen[x + y * GFX_WIDTH] = (q >> rShift) & mask;
		}
	}
}

/**
 * Schedule blocks for blitting on the output device.
 * This function gets the coordinates of a block in the AGI screen and
 * schedule it to be updated in the output device.
 * @param x1 x coordinate of the upper left corner of the block (AGI coord.)
 * @param y1 y coordinate of the upper left corner of the block (AGI coord.)
 * @param x2 x coordinate of the lower right corner of the block (AGI coord.)
 * @param y2 y coordinate of the lower right corner of the block (AGI coord.)
 *
 * @see do_update()
 */
void GfxMgr::scheduleUpdate(int x1, int y1, int x2, int y2) {
	if (x1 < update.x1)
		update.x1 = x1;
	if (y1 < update.y1)
		update.y1 = y1;
	if (x2 > update.x2)
		update.x2 = x2;
	if (y2 > update.y2)
		update.y2 = y2;
}

/**
 * Update scheduled blocks on the output device.
 * This function exposes the blocks scheduled for updating to the output
 * device. Blocks can be scheduled at any point of the AGI cycle.
 *
 * @see schedule_update()
 */
void GfxMgr::doUpdate() {
	if (update.x1 <= update.x2 && update.y1 <= update.y2) {
		gfxPutBlock(update.x1, update.y1, update.x2, update.y2);
	}

	// reset update block variables
	update.x1 = MAX_INT;
	update.y1 = MAX_INT;
	update.x2 = 0;
	update.y2 = 0;

	g_system->updateScreen();
}

/**
 * Updates a block of the framebuffer with contents of the AGI engine screen.
 * This function updates a block in the output device with the contents of
 * the AGI engine screen, handling console transparency.
 * @param x1 x coordinate of the upper left corner of the block
 * @param y1 y coordinate of the upper left corner of the block
 * @param x2 x coordinate of the lower right corner of the block
 * @param y2 y coordinate of the lower right corner of the block
 *
 * @see flush_block_a()
 */
void GfxMgr::flushBlock(int x1, int y1, int x2, int y2) {
	int y, w;
	uint8 *p0;

	scheduleUpdate(x1, y1, x2, y2);

	p0 = &_agiScreen[x1 + y1 * GFX_WIDTH];
	w = x2 - x1 + 1;

	for (y = y1; y <= y2; y++) {
		memcpy(_screen + 320 * y + x1, p0, w);
		p0 += GFX_WIDTH;
	}
}

/**
 * Updates a block of the framebuffer receiving AGI picture coordinates.
 * @param x1 x AGI picture coordinate of the upper left corner of the block
 * @param y1 y AGI picture coordinate of the upper left corner of the block
 * @param x2 x AGI picture coordinate of the lower right corner of the block
 * @param y2 y AGI picture coordinate of the lower right corner of the block
 *
 * @see flush_block()
 */
void GfxMgr::flushBlockA(int x1, int y1, int x2, int y2) {
	//y1 += 8;
	//y2 += 8;
	flushBlock(DEV_X0(x1), DEV_Y(y1), DEV_X1(x2), DEV_Y(y2));
}

/**
 * Updates the framebuffer with contents of the AGI engine screen (console-aware).
 * This function updates the output device with the contents of the AGI
 * screen, handling console transparency.
 */
void GfxMgr::flushScreen() {
	flushBlock(0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);

	doUpdate();
}

/**
 * Clear the output device screen (console-aware).
 * This function clears the output device screen and updates the
 * output device. Contents of the AGI screen are left untouched. This
 * function can be used to simulate a switch to a text mode screen in
 * a graphic-only device.
 * @param c  color to clear the screen
 */
void GfxMgr::clearScreen(int c) {
	memset(_agiScreen, c, GFX_WIDTH * GFX_HEIGHT);
	flushScreen();
}

/**
 * Save a block of the AGI engine screen
 */
void GfxMgr::saveBlock(int x1, int y1, int x2, int y2, uint8 *b) {
	uint8 *p0;
	int w, h;

	p0 = &_agiScreen[x1 + GFX_WIDTH * y1];
	w = x2 - x1 + 1;
	h = y2 - y1 + 1;
	while (h--) {
		memcpy(b, p0, w);
		b += w;
		p0 += GFX_WIDTH;
	}
}

/**
 * Restore a block of the AGI engine screen
 */
void GfxMgr::restoreBlock(int x1, int y1, int x2, int y2, uint8 *b) {
	uint8 *p0;
	int w, h;

	p0 = &_agiScreen[x1 + GFX_WIDTH * y1];
	w = x2 - x1 + 1;
	h = y2 - y1 + 1;
	while (h--) {
		memcpy(p0, b, w);
		b += w;
		p0 += GFX_WIDTH;
	}
	flushBlock(x1, y1, x2, y2);
}

} // End of namespace Agi
