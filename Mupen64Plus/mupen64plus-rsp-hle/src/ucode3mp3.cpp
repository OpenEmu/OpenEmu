/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-rsp-hle - ucode3mp3.h                                     *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2009 Richard Goedeken                                   *
 *   Copyright (C) 2002 Hacktarux                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

# include <string.h>
# include <stdio.h>

extern "C" {
  #include "hle.h"
}

static u16 DeWindowLUT [0x420] = {
    0x0000, 0xFFF3, 0x005D, 0xFF38, 0x037A, 0xF736, 0x0B37, 0xC00E,
    0x7FFF, 0x3FF2, 0x0B37, 0x08CA, 0x037A, 0x00C8, 0x005D, 0x000D,
    0x0000, 0xFFF3, 0x005D, 0xFF38, 0x037A, 0xF736, 0x0B37, 0xC00E,
    0x7FFF, 0x3FF2, 0x0B37, 0x08CA, 0x037A, 0x00C8, 0x005D, 0x000D,
    0x0000, 0xFFF2, 0x005F, 0xFF1D, 0x0369, 0xF697, 0x0A2A, 0xBCE7,
    0x7FEB, 0x3CCB, 0x0C2B, 0x082B, 0x0385, 0x00AF, 0x005B, 0x000B,
    0x0000, 0xFFF2, 0x005F, 0xFF1D, 0x0369, 0xF697, 0x0A2A, 0xBCE7,
    0x7FEB, 0x3CCB, 0x0C2B, 0x082B, 0x0385, 0x00AF, 0x005B, 0x000B,
    0x0000, 0xFFF1, 0x0061, 0xFF02, 0x0354, 0xF5F9, 0x0905, 0xB9C4,
    0x7FB0, 0x39A4, 0x0D08, 0x078C, 0x038C, 0x0098, 0x0058, 0x000A,
    0x0000, 0xFFF1, 0x0061, 0xFF02, 0x0354, 0xF5F9, 0x0905, 0xB9C4,
    0x7FB0, 0x39A4, 0x0D08, 0x078C, 0x038C, 0x0098, 0x0058, 0x000A,
    0x0000, 0xFFEF, 0x0062, 0xFEE6, 0x033B, 0xF55C, 0x07C8, 0xB6A4,
    0x7F4D, 0x367E, 0x0DCE, 0x06EE, 0x038F, 0x0080, 0x0056, 0x0009,
    0x0000, 0xFFEF, 0x0062, 0xFEE6, 0x033B, 0xF55C, 0x07C8, 0xB6A4,
    0x7F4D, 0x367E, 0x0DCE, 0x06EE, 0x038F, 0x0080, 0x0056, 0x0009,
    0x0000, 0xFFEE, 0x0063, 0xFECA, 0x031C, 0xF4C3, 0x0671, 0xB38C,
    0x7EC2, 0x335D, 0x0E7C, 0x0652, 0x038E, 0x006B, 0x0053, 0x0008,
    0x0000, 0xFFEE, 0x0063, 0xFECA, 0x031C, 0xF4C3, 0x0671, 0xB38C,
    0x7EC2, 0x335D, 0x0E7C, 0x0652, 0x038E, 0x006B, 0x0053, 0x0008,
    0x0000, 0xFFEC, 0x0064, 0xFEAC, 0x02F7, 0xF42C, 0x0502, 0xB07C,
    0x7E12, 0x3041, 0x0F14, 0x05B7, 0x038A, 0x0056, 0x0050, 0x0007,
    0x0000, 0xFFEC, 0x0064, 0xFEAC, 0x02F7, 0xF42C, 0x0502, 0xB07C,
    0x7E12, 0x3041, 0x0F14, 0x05B7, 0x038A, 0x0056, 0x0050, 0x0007,
    0x0000, 0xFFEB, 0x0064, 0xFE8E, 0x02CE, 0xF399, 0x037A, 0xAD75,
    0x7D3A, 0x2D2C, 0x0F97, 0x0520, 0x0382, 0x0043, 0x004D, 0x0007,
    0x0000, 0xFFEB, 0x0064, 0xFE8E, 0x02CE, 0xF399, 0x037A, 0xAD75,
    0x7D3A, 0x2D2C, 0x0F97, 0x0520, 0x0382, 0x0043, 0x004D, 0x0007,
    0xFFFF, 0xFFE9, 0x0063, 0xFE6F, 0x029E, 0xF30B, 0x01D8, 0xAA7B,
    0x7C3D, 0x2A1F, 0x1004, 0x048B, 0x0377, 0x0030, 0x004A, 0x0006,
    0xFFFF, 0xFFE9, 0x0063, 0xFE6F, 0x029E, 0xF30B, 0x01D8, 0xAA7B,
    0x7C3D, 0x2A1F, 0x1004, 0x048B, 0x0377, 0x0030, 0x004A, 0x0006,
    0xFFFF, 0xFFE7, 0x0062, 0xFE4F, 0x0269, 0xF282, 0x001F, 0xA78D,
    0x7B1A, 0x271C, 0x105D, 0x03F9, 0x036A, 0x001F, 0x0046, 0x0006,
    0xFFFF, 0xFFE7, 0x0062, 0xFE4F, 0x0269, 0xF282, 0x001F, 0xA78D,
    0x7B1A, 0x271C, 0x105D, 0x03F9, 0x036A, 0x001F, 0x0046, 0x0006,
    0xFFFF, 0xFFE4, 0x0061, 0xFE2F, 0x022F, 0xF1FF, 0xFE4C, 0xA4AF,
    0x79D3, 0x2425, 0x10A2, 0x036C, 0x0359, 0x0010, 0x0043, 0x0005,
    0xFFFF, 0xFFE4, 0x0061, 0xFE2F, 0x022F, 0xF1FF, 0xFE4C, 0xA4AF,
    0x79D3, 0x2425, 0x10A2, 0x036C, 0x0359, 0x0010, 0x0043, 0x0005,
    0xFFFF, 0xFFE2, 0x005E, 0xFE10, 0x01EE, 0xF184, 0xFC61, 0xA1E1,
    0x7869, 0x2139, 0x10D3, 0x02E3, 0x0346, 0x0001, 0x0040, 0x0004,
    0xFFFF, 0xFFE2, 0x005E, 0xFE10, 0x01EE, 0xF184, 0xFC61, 0xA1E1,
    0x7869, 0x2139, 0x10D3, 0x02E3, 0x0346, 0x0001, 0x0040, 0x0004,
    0xFFFF, 0xFFE0, 0x005B, 0xFDF0, 0x01A8, 0xF111, 0xFA5F, 0x9F27,
    0x76DB, 0x1E5C, 0x10F2, 0x025E, 0x0331, 0xFFF3, 0x003D, 0x0004,
    0xFFFF, 0xFFE0, 0x005B, 0xFDF0, 0x01A8, 0xF111, 0xFA5F, 0x9F27,
    0x76DB, 0x1E5C, 0x10F2, 0x025E, 0x0331, 0xFFF3, 0x003D, 0x0004,
    0xFFFF, 0xFFDE, 0x0057, 0xFDD0, 0x015B, 0xF0A7, 0xF845, 0x9C80,
    0x752C, 0x1B8E, 0x1100, 0x01DE, 0x0319, 0xFFE7, 0x003A, 0x0003,
    0xFFFF, 0xFFDE, 0x0057, 0xFDD0, 0x015B, 0xF0A7, 0xF845, 0x9C80,
    0x752C, 0x1B8E, 0x1100, 0x01DE, 0x0319, 0xFFE7, 0x003A, 0x0003,
    0xFFFE, 0xFFDB, 0x0053, 0xFDB0, 0x0108, 0xF046, 0xF613, 0x99EE,
    0x735C, 0x18D1, 0x10FD, 0x0163, 0x0300, 0xFFDC, 0x0037, 0x0003,
    0xFFFE, 0xFFDB, 0x0053, 0xFDB0, 0x0108, 0xF046, 0xF613, 0x99EE,
    0x735C, 0x18D1, 0x10FD, 0x0163, 0x0300, 0xFFDC, 0x0037, 0x0003,
    0xFFFE, 0xFFD8, 0x004D, 0xFD90, 0x00B0, 0xEFF0, 0xF3CC, 0x9775,
    0x716C, 0x1624, 0x10EA, 0x00EE, 0x02E5, 0xFFD2, 0x0033, 0x0003,
    0xFFFE, 0xFFD8, 0x004D, 0xFD90, 0x00B0, 0xEFF0, 0xF3CC, 0x9775,
    0x716C, 0x1624, 0x10EA, 0x00EE, 0x02E5, 0xFFD2, 0x0033, 0x0003,
    0xFFFE, 0xFFD6, 0x0047, 0xFD72, 0x0051, 0xEFA6, 0xF16F, 0x9514,
    0x6F5E, 0x138A, 0x10C8, 0x007E, 0x02CA, 0xFFC9, 0x0030, 0x0003,
    0xFFFE, 0xFFD6, 0x0047, 0xFD72, 0x0051, 0xEFA6, 0xF16F, 0x9514,
    0x6F5E, 0x138A, 0x10C8, 0x007E, 0x02CA, 0xFFC9, 0x0030, 0x0003,
    0xFFFE, 0xFFD3, 0x0040, 0xFD54, 0xFFEC, 0xEF68, 0xEEFC, 0x92CD,
    0x6D33, 0x1104, 0x1098, 0x0014, 0x02AC, 0xFFC0, 0x002D, 0x0002,
    0xFFFE, 0xFFD3, 0x0040, 0xFD54, 0xFFEC, 0xEF68, 0xEEFC, 0x92CD,
    0x6D33, 0x1104, 0x1098, 0x0014, 0x02AC, 0xFFC0, 0x002D, 0x0002,
    0x0030, 0xFFC9, 0x02CA, 0x007E, 0x10C8, 0x138A, 0x6F5E, 0x9514,
    0xF16F, 0xEFA6, 0x0051, 0xFD72, 0x0047, 0xFFD6, 0xFFFE, 0x0003,
    0x0030, 0xFFC9, 0x02CA, 0x007E, 0x10C8, 0x138A, 0x6F5E, 0x9514,
    0xF16F, 0xEFA6, 0x0051, 0xFD72, 0x0047, 0xFFD6, 0xFFFE, 0x0003,
    0x0033, 0xFFD2, 0x02E5, 0x00EE, 0x10EA, 0x1624, 0x716C, 0x9775,
    0xF3CC, 0xEFF0, 0x00B0, 0xFD90, 0x004D, 0xFFD8, 0xFFFE, 0x0003,
    0x0033, 0xFFD2, 0x02E5, 0x00EE, 0x10EA, 0x1624, 0x716C, 0x9775,
    0xF3CC, 0xEFF0, 0x00B0, 0xFD90, 0x004D, 0xFFD8, 0xFFFE, 0x0003,
    0x0037, 0xFFDC, 0x0300, 0x0163, 0x10FD, 0x18D1, 0x735C, 0x99EE,
    0xF613, 0xF046, 0x0108, 0xFDB0, 0x0053, 0xFFDB, 0xFFFE, 0x0003,
    0x0037, 0xFFDC, 0x0300, 0x0163, 0x10FD, 0x18D1, 0x735C, 0x99EE,
    0xF613, 0xF046, 0x0108, 0xFDB0, 0x0053, 0xFFDB, 0xFFFE, 0x0003,
    0x003A, 0xFFE7, 0x0319, 0x01DE, 0x1100, 0x1B8E, 0x752C, 0x9C80,
    0xF845, 0xF0A7, 0x015B, 0xFDD0, 0x0057, 0xFFDE, 0xFFFF, 0x0003,
    0x003A, 0xFFE7, 0x0319, 0x01DE, 0x1100, 0x1B8E, 0x752C, 0x9C80,
    0xF845, 0xF0A7, 0x015B, 0xFDD0, 0x0057, 0xFFDE, 0xFFFF, 0x0004,
    0x003D, 0xFFF3, 0x0331, 0x025E, 0x10F2, 0x1E5C, 0x76DB, 0x9F27,
    0xFA5F, 0xF111, 0x01A8, 0xFDF0, 0x005B, 0xFFE0, 0xFFFF, 0x0004,
    0x003D, 0xFFF3, 0x0331, 0x025E, 0x10F2, 0x1E5C, 0x76DB, 0x9F27,
    0xFA5F, 0xF111, 0x01A8, 0xFDF0, 0x005B, 0xFFE0, 0xFFFF, 0x0004,
    0x0040, 0x0001, 0x0346, 0x02E3, 0x10D3, 0x2139, 0x7869, 0xA1E1,
    0xFC61, 0xF184, 0x01EE, 0xFE10, 0x005E, 0xFFE2, 0xFFFF, 0x0004,
    0x0040, 0x0001, 0x0346, 0x02E3, 0x10D3, 0x2139, 0x7869, 0xA1E1,
    0xFC61, 0xF184, 0x01EE, 0xFE10, 0x005E, 0xFFE2, 0xFFFF, 0x0005,
    0x0043, 0x0010, 0x0359, 0x036C, 0x10A2, 0x2425, 0x79D3, 0xA4AF,
    0xFE4C, 0xF1FF, 0x022F, 0xFE2F, 0x0061, 0xFFE4, 0xFFFF, 0x0005,
    0x0043, 0x0010, 0x0359, 0x036C, 0x10A2, 0x2425, 0x79D3, 0xA4AF,
    0xFE4C, 0xF1FF, 0x022F, 0xFE2F, 0x0061, 0xFFE4, 0xFFFF, 0x0006,
    0x0046, 0x001F, 0x036A, 0x03F9, 0x105D, 0x271C, 0x7B1A, 0xA78D,
    0x001F, 0xF282, 0x0269, 0xFE4F, 0x0062, 0xFFE7, 0xFFFF, 0x0006,
    0x0046, 0x001F, 0x036A, 0x03F9, 0x105D, 0x271C, 0x7B1A, 0xA78D,
    0x001F, 0xF282, 0x0269, 0xFE4F, 0x0062, 0xFFE7, 0xFFFF, 0x0006,
    0x004A, 0x0030, 0x0377, 0x048B, 0x1004, 0x2A1F, 0x7C3D, 0xAA7B,
    0x01D8, 0xF30B, 0x029E, 0xFE6F, 0x0063, 0xFFE9, 0xFFFF, 0x0006,
    0x004A, 0x0030, 0x0377, 0x048B, 0x1004, 0x2A1F, 0x7C3D, 0xAA7B,
    0x01D8, 0xF30B, 0x029E, 0xFE6F, 0x0063, 0xFFE9, 0xFFFF, 0x0007,
    0x004D, 0x0043, 0x0382, 0x0520, 0x0F97, 0x2D2C, 0x7D3A, 0xAD75,
    0x037A, 0xF399, 0x02CE, 0xFE8E, 0x0064, 0xFFEB, 0x0000, 0x0007,
    0x004D, 0x0043, 0x0382, 0x0520, 0x0F97, 0x2D2C, 0x7D3A, 0xAD75,
    0x037A, 0xF399, 0x02CE, 0xFE8E, 0x0064, 0xFFEB, 0x0000, 0x0007,
    0x0050, 0x0056, 0x038A, 0x05B7, 0x0F14, 0x3041, 0x7E12, 0xB07C,
    0x0502, 0xF42C, 0x02F7, 0xFEAC, 0x0064, 0xFFEC, 0x0000, 0x0007,
    0x0050, 0x0056, 0x038A, 0x05B7, 0x0F14, 0x3041, 0x7E12, 0xB07C,
    0x0502, 0xF42C, 0x02F7, 0xFEAC, 0x0064, 0xFFEC, 0x0000, 0x0008,
    0x0053, 0x006B, 0x038E, 0x0652, 0x0E7C, 0x335D, 0x7EC2, 0xB38C,
    0x0671, 0xF4C3, 0x031C, 0xFECA, 0x0063, 0xFFEE, 0x0000, 0x0008,
    0x0053, 0x006B, 0x038E, 0x0652, 0x0E7C, 0x335D, 0x7EC2, 0xB38C,
    0x0671, 0xF4C3, 0x031C, 0xFECA, 0x0063, 0xFFEE, 0x0000, 0x0009,
    0x0056, 0x0080, 0x038F, 0x06EE, 0x0DCE, 0x367E, 0x7F4D, 0xB6A4,
    0x07C8, 0xF55C, 0x033B, 0xFEE6, 0x0062, 0xFFEF, 0x0000, 0x0009,
    0x0056, 0x0080, 0x038F, 0x06EE, 0x0DCE, 0x367E, 0x7F4D, 0xB6A4,
    0x07C8, 0xF55C, 0x033B, 0xFEE6, 0x0062, 0xFFEF, 0x0000, 0x000A,
    0x0058, 0x0098, 0x038C, 0x078C, 0x0D08, 0x39A4, 0x7FB0, 0xB9C4,
    0x0905, 0xF5F9, 0x0354, 0xFF02, 0x0061, 0xFFF1, 0x0000, 0x000A,
    0x0058, 0x0098, 0x038C, 0x078C, 0x0D08, 0x39A4, 0x7FB0, 0xB9C4,
    0x0905, 0xF5F9, 0x0354, 0xFF02, 0x0061, 0xFFF1, 0x0000, 0x000B,
    0x005B, 0x00AF, 0x0385, 0x082B, 0x0C2B, 0x3CCB, 0x7FEB, 0xBCE7,
    0x0A2A, 0xF697, 0x0369, 0xFF1D, 0x005F, 0xFFF2, 0x0000, 0x000B,
    0x005B, 0x00AF, 0x0385, 0x082B, 0x0C2B, 0x3CCB, 0x7FEB, 0xBCE7,
    0x0A2A, 0xF697, 0x0369, 0xFF1D, 0x005F, 0xFFF2, 0x0000, 0x000D,
    0x005D, 0x00C8, 0x037A, 0x08CA, 0x0B37, 0x3FF2, 0x7FFF, 0xC00E,
    0x0B37, 0xF736, 0x037A, 0xFF38, 0x005D, 0xFFF3, 0x0000, 0x000D,
    0x005D, 0x00C8, 0x037A, 0x08CA, 0x0B37, 0x3FF2, 0x7FFF, 0xC00E,
    0x0B37, 0xF736, 0x037A, 0xFF38, 0x005D, 0xFFF3, 0x0000, 0x0000
};

//static u16 myVector[32][8];

static u8 mp3data[0x1000];

static s32 v[32];

static void MP3AB0 () {
    // Part 2 - 100% Accurate
    const u16 LUT2[8] = { 0xFEC4, 0xF4FA, 0xC5E4, 0xE1C4, 
                          0x1916, 0x4A50, 0xA268, 0x78AE };
    const u16 LUT3[4] = { 0xFB14, 0xD4DC, 0x31F2, 0x8E3A };
    int i;

    for (i = 0; i < 8; i++) {
        v[16+i] = v[0+i] + v[8+i];
        v[24+i] = ((v[0+i] - v[8+i]) * LUT2[i]) >> 0x10;
    }

    // Part 3: 4-wide butterflies

    for (i=0; i < 4; i++) {
        v[0+i]  = v[16+i] + v[20+i];
        v[4+i]  = ((v[16+i] - v[20+i]) * LUT3[i]) >> 0x10;

        v[8+i]  = v[24+i] + v[28+i];
        v[12+i] = ((v[24+i] - v[28+i]) * LUT3[i]) >> 0x10;
    }
                
    // Part 4: 2-wide butterflies - 100% Accurate

    for (i = 0; i < 16; i+=4) {
        v[16+i] = v[0+i] + v[2+i];
        v[18+i] = ((v[0+i] - v[2+i]) * 0xEC84) >> 0x10;

        v[17+i] = v[1+i] + v[3+i];
        v[19+i] = ((v[1+i] - v[3+i]) * 0x61F8) >> 0x10;
    }
}

static void InnerLoop ();

    u32 inPtr, outPtr;

    u32 t6;// = 0x08A0; // I think these are temporary storage buffers
    u32 t5;// = 0x0AC0;
    u32 t4;// = (inst1 & 0x1E);

void MP3 () {
    // Initialization Code
    u32 readPtr; // s5
    u32 writePtr; // s6
    //u32 Count = 0x0480; // s4
    u32 tmp;
    //u32 inPtr, outPtr;

    t6 = 0x08A0; // I think these are temporary storage buffers
    t5 = 0x0AC0;
    t4 = (inst1 & 0x1E);

    writePtr = inst2 & 0xFFFFFF;
    readPtr  = writePtr;
    memcpy (mp3data+0xCE8, rsp.RDRAM+readPtr, 8); // Just do that for efficiency... may remove and use directly later anyway
    readPtr += 8; // This must be a header byte or whatnot

    for (int cnt = 0; cnt < 0x480; cnt += 0x180) {
        memcpy (mp3data+0xCF0, rsp.RDRAM+readPtr, 0x180); // DMA: 0xCF0 <- RDRAM[s5] : 0x180
        inPtr  = 0xCF0; // s7
        outPtr = 0xE70; // s3
// --------------- Inner Loop Start --------------------
        for (int cnt2 = 0; cnt2 < 0x180; cnt2 += 0x40) {
            t6 &= 0xFFE0;
            t5 &= 0xFFE0;
            t6 |= t4;
            t5 |= t4;
            InnerLoop ();
            t4 = (t4-2)&0x1E;
            tmp = t6;
            t6 = t5;
            t5 = tmp;
            //outPtr += 0x40;
            inPtr += 0x40;
        }
// --------------- Inner Loop End --------------------
        memcpy (rsp.RDRAM+writePtr, mp3data+0xe70, 0x180);
        writePtr += 0x180;
        readPtr  += 0x180;
    }
}



static void InnerLoop () {
                // Part 1: 100% Accurate

                int i;
                v[0] = *(s16 *)(mp3data+inPtr+(0x00^S16)); v[31] = *(s16 *)(mp3data+inPtr+(0x3E^S16)); v[0] += v[31];
                v[1] = *(s16 *)(mp3data+inPtr+(0x02^S16)); v[30] = *(s16 *)(mp3data+inPtr+(0x3C^S16)); v[1] += v[30];
                v[2] = *(s16 *)(mp3data+inPtr+(0x06^S16)); v[28] = *(s16 *)(mp3data+inPtr+(0x38^S16)); v[2] += v[28];
                v[3] = *(s16 *)(mp3data+inPtr+(0x04^S16)); v[29] = *(s16 *)(mp3data+inPtr+(0x3A^S16)); v[3] += v[29];

                v[4] = *(s16 *)(mp3data+inPtr+(0x0E^S16)); v[24] = *(s16 *)(mp3data+inPtr+(0x30^S16)); v[4] += v[24];
                v[5] = *(s16 *)(mp3data+inPtr+(0x0C^S16)); v[25] = *(s16 *)(mp3data+inPtr+(0x32^S16)); v[5] += v[25];
                v[6] = *(s16 *)(mp3data+inPtr+(0x08^S16)); v[27] = *(s16 *)(mp3data+inPtr+(0x36^S16)); v[6] += v[27];
                v[7] = *(s16 *)(mp3data+inPtr+(0x0A^S16)); v[26] = *(s16 *)(mp3data+inPtr+(0x34^S16)); v[7] += v[26];

                v[8] = *(s16 *)(mp3data+inPtr+(0x1E^S16)); v[16] = *(s16 *)(mp3data+inPtr+(0x20^S16)); v[8] += v[16];
                v[9] = *(s16 *)(mp3data+inPtr+(0x1C^S16)); v[17] = *(s16 *)(mp3data+inPtr+(0x22^S16)); v[9] += v[17];
                v[10]= *(s16 *)(mp3data+inPtr+(0x18^S16)); v[19] = *(s16 *)(mp3data+inPtr+(0x26^S16)); v[10]+= v[19];
                v[11]= *(s16 *)(mp3data+inPtr+(0x1A^S16)); v[18] = *(s16 *)(mp3data+inPtr+(0x24^S16)); v[11]+= v[18];

                v[12]= *(s16 *)(mp3data+inPtr+(0x10^S16)); v[23] = *(s16 *)(mp3data+inPtr+(0x2E^S16)); v[12]+= v[23];
                v[13]= *(s16 *)(mp3data+inPtr+(0x12^S16)); v[22] = *(s16 *)(mp3data+inPtr+(0x2C^S16)); v[13]+= v[22];
                v[14]= *(s16 *)(mp3data+inPtr+(0x16^S16)); v[20] = *(s16 *)(mp3data+inPtr+(0x28^S16)); v[14]+= v[20];
                v[15]= *(s16 *)(mp3data+inPtr+(0x14^S16)); v[21] = *(s16 *)(mp3data+inPtr+(0x2A^S16)); v[15]+= v[21];

                // Part 2-4

                MP3AB0 ();

                // Part 5 - 1-Wide Butterflies - 100% Accurate but need SSVs!!!

                u32 t0 = t6 + 0x100;
                u32 t1 = t6 + 0x200;
                u32 t2 = t5 + 0x100;
                u32 t3 = t5 + 0x200;
                /*RSP_GPR[0x8].W = t0;
                RSP_GPR[0x9].W = t1;
                RSP_GPR[0xA].W = t2;
                RSP_GPR[0xB].W = t3;

                RSP_Vect[0].DW[1] = 0xB504A57E00016A09;
                RSP_Vect[0].DW[0] = 0x0002D4130005A827;
*/

                // 0x13A8
                v[1] = 0;
                v[11] = ((v[16] - v[17]) * 0xB504) >> 0x10;

                v[16] = -v[16] -v[17];
                v[2] = v[18] + v[19];
                // ** Store v[11] -> (T6 + 0)**
                *(s16 *)(mp3data+((t6+(short)0x0))) = (short)v[11];
                
                
                v[11] = -v[11];
                // ** Store v[16] -> (T3 + 0)**
                *(s16 *)(mp3data+((t3+(short)0x0))) = (short)v[16];
                // ** Store v[11] -> (T5 + 0)**
                *(s16 *)(mp3data+((t5+(short)0x0))) = (short)v[11];
                // 0x13E8 - Verified....
                v[2] = -v[2];
                // ** Store v[2] -> (T2 + 0)**
                *(s16 *)(mp3data+((t2+(short)0x0))) = (short)v[2];
                v[3]  = (((v[18] - v[19]) * 0x16A09) >> 0x10) + v[2];
                // ** Store v[3] -> (T0 + 0)**
                *(s16 *)(mp3data+((t0+(short)0x0))) = (short)v[3];
                // 0x1400 - Verified
                v[4] = -v[20] -v[21];
                v[6] = v[22] + v[23];
                v[5] = ((v[20] - v[21]) * 0x16A09) >> 0x10;
                // ** Store v[4] -> (T3 + 0xFF80)
                *(s16 *)(mp3data+((t3+(short)0xFF80))) = (short)v[4];
                v[7] = ((v[22] - v[23]) * 0x2D413) >> 0x10;
                v[5] = v[5] - v[4];
                v[7] = v[7] - v[5];
                v[6] = v[6] + v[6];
                v[5] = v[5] - v[6];
                v[4] = -v[4] - v[6];
                // *** Store v[7] -> (T1 + 0xFF80)
                *(s16 *)(mp3data+((t1+(short)0xFF80))) = (short)v[7];
                // *** Store v[4] -> (T2 + 0xFF80)
                *(s16 *)(mp3data+((t2+(short)0xFF80))) = (short)v[4];
                // *** Store v[5] -> (T0 + 0xFF80)
                *(s16 *)(mp3data+((t0+(short)0xFF80))) = (short)v[5];
                v[8] = v[24] + v[25];


                v[9] = ((v[24] - v[25]) * 0x16A09) >> 0x10;
                v[2] = v[8] + v[9];
                v[11] = ((v[26] - v[27]) * 0x2D413) >> 0x10;
                v[13] = ((v[28] - v[29]) * 0x2D413) >> 0x10;

                v[10] = v[26] + v[27]; v[10] = v[10] + v[10];
                v[12] = v[28] + v[29]; v[12] = v[12] + v[12];
                v[14] = v[30] + v[31];
                v[3] = v[8] + v[10];
                v[14] = v[14] + v[14];
                v[13] = (v[13] - v[2]) + v[12];
                v[15] = (((v[30] - v[31]) * 0x5A827) >> 0x10) - (v[11] + v[2]);
                v[14] = -(v[14] + v[14]) + v[3];
                v[17] = v[13] - v[10];
                v[9] = v[9] + v[14];
                // ** Store v[9] -> (T6 + 0x40)
                *(s16 *)(mp3data+((t6+(short)0x40))) = (short)v[9];
                v[11] = v[11] - v[13];
                // ** Store v[17] -> (T0 + 0xFFC0)
                *(s16 *)(mp3data+((t0+(short)0xFFC0))) = (short)v[17];
                v[12] = v[8] - v[12];
                // ** Store v[11] -> (T0 + 0x40)
                *(s16 *)(mp3data+((t0+(short)0x40))) = (short)v[11];
                v[8] = -v[8];
                // ** Store v[15] -> (T1 + 0xFFC0)
                *(s16 *)(mp3data+((t1+(short)0xFFC0))) = (short)v[15];
                v[10] = -v[10] -v[12];
                // ** Store v[12] -> (T2 + 0x40)
                *(s16 *)(mp3data+((t2+(short)0x40))) = (short)v[12];
                // ** Store v[8] -> (T3 + 0xFFC0)
                *(s16 *)(mp3data+((t3+(short)0xFFC0))) = (short)v[8];
                // ** Store v[14] -> (T5 + 0x40)
                *(s16 *)(mp3data+((t5+(short)0x40))) = (short)v[14];
                // ** Store v[10] -> (T2 + 0xFFC0)
                *(s16 *)(mp3data+((t2+(short)0xFFC0))) = (short)v[10];
                // 0x14FC - Verified...

                // Part 6 - 100% Accurate

                v[0] = *(s16 *)(mp3data+inPtr+(0x00^S16)); v[31] = *(s16 *)(mp3data+inPtr+(0x3E^S16)); v[0] -= v[31];
                v[1] = *(s16 *)(mp3data+inPtr+(0x02^S16)); v[30] = *(s16 *)(mp3data+inPtr+(0x3C^S16)); v[1] -= v[30];
                v[2] = *(s16 *)(mp3data+inPtr+(0x06^S16)); v[28] = *(s16 *)(mp3data+inPtr+(0x38^S16)); v[2] -= v[28];
                v[3] = *(s16 *)(mp3data+inPtr+(0x04^S16)); v[29] = *(s16 *)(mp3data+inPtr+(0x3A^S16)); v[3] -= v[29];

                v[4] = *(s16 *)(mp3data+inPtr+(0x0E^S16)); v[24] = *(s16 *)(mp3data+inPtr+(0x30^S16)); v[4] -= v[24];
                v[5] = *(s16 *)(mp3data+inPtr+(0x0C^S16)); v[25] = *(s16 *)(mp3data+inPtr+(0x32^S16)); v[5] -= v[25];
                v[6] = *(s16 *)(mp3data+inPtr+(0x08^S16)); v[27] = *(s16 *)(mp3data+inPtr+(0x36^S16)); v[6] -= v[27];
                v[7] = *(s16 *)(mp3data+inPtr+(0x0A^S16)); v[26] = *(s16 *)(mp3data+inPtr+(0x34^S16)); v[7] -= v[26];

                v[8] = *(s16 *)(mp3data+inPtr+(0x1E^S16)); v[16] = *(s16 *)(mp3data+inPtr+(0x20^S16)); v[8] -= v[16];
                v[9] = *(s16 *)(mp3data+inPtr+(0x1C^S16)); v[17] = *(s16 *)(mp3data+inPtr+(0x22^S16)); v[9] -= v[17];
                v[10]= *(s16 *)(mp3data+inPtr+(0x18^S16)); v[19] = *(s16 *)(mp3data+inPtr+(0x26^S16)); v[10]-= v[19];
                v[11]= *(s16 *)(mp3data+inPtr+(0x1A^S16)); v[18] = *(s16 *)(mp3data+inPtr+(0x24^S16)); v[11]-= v[18];

                v[12]= *(s16 *)(mp3data+inPtr+(0x10^S16)); v[23] = *(s16 *)(mp3data+inPtr+(0x2E^S16)); v[12]-= v[23];
                v[13]= *(s16 *)(mp3data+inPtr+(0x12^S16)); v[22] = *(s16 *)(mp3data+inPtr+(0x2C^S16)); v[13]-= v[22];
                v[14]= *(s16 *)(mp3data+inPtr+(0x16^S16)); v[20] = *(s16 *)(mp3data+inPtr+(0x28^S16)); v[14]-= v[20];
                v[15]= *(s16 *)(mp3data+inPtr+(0x14^S16)); v[21] = *(s16 *)(mp3data+inPtr+(0x2A^S16)); v[15]-= v[21];

                //0, 1, 3, 2, 7, 6, 4, 5, 7, 6, 4, 5, 0, 1, 3, 2
                const u16 LUT6[16] = { 0xFFB2, 0xFD3A, 0xF10A, 0xF854,
                                       0xBDAE, 0xCDA0, 0xE76C, 0xDB94,
                                       0x1920, 0x4B20, 0xAC7C, 0x7C68,
                                       0xABEC, 0x9880, 0xDAE8, 0x839C };
                for (i = 0; i < 16; i++) {
                    v[0+i] = (v[0+i] * LUT6[i]) >> 0x10;
                }
                v[0] = v[0] + v[0]; v[1] = v[1] + v[1];
                v[2] = v[2] + v[2]; v[3] = v[3] + v[3]; v[4] = v[4] + v[4];
                v[5] = v[5] + v[5]; v[6] = v[6] + v[6]; v[7] = v[7] + v[7];
                v[12] = v[12] + v[12]; v[13] = v[13] + v[13]; v[15] = v[15] + v[15];
                
                MP3AB0 ();

                // Part 7: - 100% Accurate + SSV - Unoptimized

                v[0] = ( v[17] + v[16] ) >> 1;
                v[1] = ((v[17] * (int)((short)0xA57E * 2)) + (v[16] * 0xB504)) >> 0x10;
                v[2] = -v[18] -v[19];
                v[3] = ((v[18] - v[19]) * 0x16A09) >> 0x10;
                v[4] = v[20] + v[21] + v[0];
                v[5] = (((v[20] - v[21]) * 0x16A09) >> 0x10) + v[1];
                v[6] = (((v[22] + v[23]) << 1) + v[0]) - v[2];
                v[7] = (((v[22] - v[23]) * 0x2D413) >> 0x10) + v[0] + v[1] + v[3];
                // 0x16A8
                // Save v[0] -> (T3 + 0xFFE0)
                *(s16 *)(mp3data+((t3+(short)0xFFE0))) = (short)-v[0];
                v[8] = v[24] + v[25];
                v[9] = ((v[24] - v[25]) * 0x16A09) >> 0x10;
                v[10] = ((v[26] + v[27]) << 1) + v[8];
                v[11] = (((v[26] - v[27]) * 0x2D413) >> 0x10) + v[8] + v[9];
                v[12] = v[4] - ((v[28] + v[29]) << 1);
                // ** Store v12 -> (T2 + 0x20)
                *(s16 *)(mp3data+((t2+(short)0x20))) = (short)v[12];
                v[13] = (((v[28] - v[29]) * 0x2D413) >> 0x10) - v[12] - v[5];
                v[14] = v[30] + v[31];
                v[14] = v[14] + v[14];
                v[14] = v[14] + v[14];
                v[14] = v[6] - v[14];
                v[15] = (((v[30] - v[31]) * 0x5A827) >> 0x10) - v[7];
                // Store v14 -> (T5 + 0x20)
                *(s16 *)(mp3data+((t5+(short)0x20))) = (short)v[14];
                v[14] = v[14] + v[1];
                // Store v[14] -> (T6 + 0x20)
                *(s16 *)(mp3data+((t6+(short)0x20))) = (short)v[14];
                // Store v[15] -> (T1 + 0xFFE0)
                *(s16 *)(mp3data+((t1+(short)0xFFE0))) = (short)v[15];
                v[9] = v[9] + v[10];
                v[1] = v[1] + v[6];
                v[6] = v[10] - v[6];
                v[1] = v[9] - v[1];
                // Store v[6] -> (T5 + 0x60)
                *(s16 *)(mp3data+((t5+(short)0x60))) = (short)v[6];
                v[10] = v[10] + v[2];
                v[10] = v[4] - v[10];
                // Store v[10] -> (T2 + 0xFFA0)
                *(s16 *)(mp3data+((t2+(short)0xFFA0))) = (short)v[10];
                v[12] = v[2] - v[12];
                // Store v[12] -> (T2 + 0xFFE0)
                *(s16 *)(mp3data+((t2+(short)0xFFE0))) = (short)v[12];
                v[5] = v[4] + v[5];
                v[4] = v[8] - v[4];
                // Store v[4] -> (T2 + 0x60)
                *(s16 *)(mp3data+((t2+(short)0x60))) = (short)v[4];
                v[0] = v[0] - v[8];
                // Store v[0] -> (T3 + 0xFFA0)
                *(s16 *)(mp3data+((t3+(short)0xFFA0))) = (short)v[0];
                v[7] = v[7] - v[11];
                // Store v[7] -> (T1 + 0xFFA0)
                *(s16 *)(mp3data+((t1+(short)0xFFA0))) = (short)v[7];
                v[11] = v[11] - v[3];
                // Store v[1] -> (T6 + 0x60)
                *(s16 *)(mp3data+((t6+(short)0x60))) = (short)v[1];
                v[11] = v[11] - v[5];
                // Store v[11] -> (T0 + 0x60)
                *(s16 *)(mp3data+((t0+(short)0x60))) = (short)v[11];
                v[3] = v[3] - v[13];
                // Store v[3] -> (T0 + 0x20)
                *(s16 *)(mp3data+((t0+(short)0x20))) = (short)v[3];
                v[13] = v[13] + v[2];
                // Store v[13] -> (T0 + 0xFFE0)
                *(s16 *)(mp3data+((t0+(short)0xFFE0))) = (short)v[13];
                //v[2] = ;
                v[2] = (v[5] - v[2]) - v[9];
                // Store v[2] -> (T0 + 0xFFA0)
                *(s16 *)(mp3data+((t0+(short)0xFFA0))) = (short)v[2];
                // 0x7A8 - Verified...

                // Step 8 - Dewindowing
    
                //u64 *DW = (u64 *)&DeWindowLUT[0x10-(t4>>1)];
                u32 offset = 0x10-(t4>>1);

                u32 addptr = t6 & 0xFFE0;
                offset = 0x10-(t4>>1);

                s32 v2=0, v4=0, v6=0, v8=0;
                //s32 z2=0, z4=0, z6=0, z8=0;

                offset = 0x10-(t4>>1);// + x*0x40;
                int x;
                for (x = 0; x < 8; x++) {
                    v2 = v4 = v6 = v8 = 0;

                    //addptr = t1;
                
                    for (i = 7; i >= 0; i--) {
                        v2 += ((int)*(s16 *)(mp3data+(addptr)+0x00) * (short)DeWindowLUT[offset+0x00] + 0x4000) >> 0xF;
                        v4 += ((int)*(s16 *)(mp3data+(addptr)+0x10) * (short)DeWindowLUT[offset+0x08] + 0x4000) >> 0xF;
                        v6 += ((int)*(s16 *)(mp3data+(addptr)+0x20) * (short)DeWindowLUT[offset+0x20] + 0x4000) >> 0xF;
                        v8 += ((int)*(s16 *)(mp3data+(addptr)+0x30) * (short)DeWindowLUT[offset+0x28] + 0x4000) >> 0xF;
                        addptr+=2; offset++;
                    }
                    s32 v0  = v2 + v4;
                    s32 v18 = v6 + v8;
                    //Clamp(v0);
                    //Clamp(v18);
                    // clamp???
                    *(s16 *)(mp3data+(outPtr^S16)) = v0;
                    *(s16 *)(mp3data+((outPtr+2)^S16)) = v18;
                    outPtr+=4;
                    addptr += 0x30;
                    offset += 0x38;
                }

                offset = 0x10-(t4>>1) + 8*0x40;
                v2 = v4 = 0;
                for (i = 0; i < 4; i++) {
                    v2 += ((int)*(s16 *)(mp3data+(addptr)+0x00) * (short)DeWindowLUT[offset+0x00] + 0x4000) >> 0xF;
                    v2 += ((int)*(s16 *)(mp3data+(addptr)+0x10) * (short)DeWindowLUT[offset+0x08] + 0x4000) >> 0xF;
                    addptr+=2; offset++;
                    v4 += ((int)*(s16 *)(mp3data+(addptr)+0x00) * (short)DeWindowLUT[offset+0x00] + 0x4000) >> 0xF;
                    v4 += ((int)*(s16 *)(mp3data+(addptr)+0x10) * (short)DeWindowLUT[offset+0x08] + 0x4000) >> 0xF;
                    addptr+=2; offset++;
                }
                s32 mult6 = *(s32 *)(mp3data+0xCE8);
                s32 mult4 = *(s32 *)(mp3data+0xCEC);
                if (t4 & 0x2) {
                    v2 = (v2 * *(u32 *)(mp3data+0xCE8)) >> 0x10;
                    *(s16 *)(mp3data+(outPtr^S16)) = v2;
                } else {
                    v4 = (v4 * *(u32 *)(mp3data+0xCE8)) >> 0x10;
                    *(s16 *)(mp3data+(outPtr^S16)) = v4;
                    mult4 = *(u32 *)(mp3data+0xCE8);
                }
                addptr -= 0x50;

                for (x = 0; x < 8; x++) {
                    v2 = v4 = v6 = v8 = 0;

                    offset = (0x22F-(t4>>1) + x*0x40);
                
                    for (i = 0; i < 4; i++) {
                        v2 += ((int)*(s16 *)(mp3data+(addptr    )+0x20) * (short)DeWindowLUT[offset+0x00] + 0x4000) >> 0xF;
                        v2 -= ((int)*(s16 *)(mp3data+((addptr+2))+0x20) * (short)DeWindowLUT[offset+0x01] + 0x4000) >> 0xF;
                        v4 += ((int)*(s16 *)(mp3data+(addptr    )+0x30) * (short)DeWindowLUT[offset+0x08] + 0x4000) >> 0xF;
                        v4 -= ((int)*(s16 *)(mp3data+((addptr+2))+0x30) * (short)DeWindowLUT[offset+0x09] + 0x4000) >> 0xF;
                        v6 += ((int)*(s16 *)(mp3data+(addptr    )+0x00) * (short)DeWindowLUT[offset+0x20] + 0x4000) >> 0xF;
                        v6 -= ((int)*(s16 *)(mp3data+((addptr+2))+0x00) * (short)DeWindowLUT[offset+0x21] + 0x4000) >> 0xF;
                        v8 += ((int)*(s16 *)(mp3data+(addptr    )+0x10) * (short)DeWindowLUT[offset+0x28] + 0x4000) >> 0xF;
                        v8 -= ((int)*(s16 *)(mp3data+((addptr+2))+0x10) * (short)DeWindowLUT[offset+0x29] + 0x4000) >> 0xF;
                        addptr+=4; offset+=2;
                    }
                    s32 v0  = v2 + v4;
                    s32 v18 = v6 + v8;
                    //Clamp(v0);
                    //Clamp(v18);
                    // clamp???
                    *(s16 *)(mp3data+((outPtr+2)^S16)) = v0;
                    *(s16 *)(mp3data+((outPtr+4)^S16)) = v18;
                    outPtr+=4;
                    addptr -= 0x50;
                }

                int tmp = outPtr;
                s32 hi0 = mult6;
                s32 hi1 = mult4;
                s32 v;

                hi0 = (int)hi0 >> 0x10;
                hi1 = (int)hi1 >> 0x10;
                for (i = 0; i < 8; i++) {
                    // v0
                    v = (*(s16 *)(mp3data+((tmp-0x40)^S16)) * hi0);
                    if (v > 32767) v = 32767; else if (v < -32767) v = -32767;
                    *(s16 *)((u8 *)mp3data+((tmp-0x40)^S16)) = (s16)v;
                    // v17
                    v = (*(s16 *)(mp3data+((tmp-0x30)^S16)) * hi0);
                    if (v > 32767) v = 32767; else if (v < -32767) v = -32767;
                    *(s16 *)((u8 *)mp3data+((tmp-0x30)^S16)) = v;
                    // v2
                    v = (*(s16 *)(mp3data+((tmp-0x1E)^S16)) * hi1);
                    if (v > 32767) v = 32767; else if (v < -32767) v = -32767;
                    *(s16 *)((u8 *)mp3data+((tmp-0x1E)^S16)) = v;
                    // v4
                    v = (*(s16 *)(mp3data+((tmp-0xE)^S16)) * hi1);
                    if (v > 32767) v = 32767; else if (v < -32767) v = -32767;
                    *(s16 *)((u8 *)mp3data+((tmp-0xE)^S16)) = v;
                    tmp += 2;
                }
}

