/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-rsp-hle - jpeg.c                                          *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2012 Bobby Smiles                                       *
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define M64P_PLUGIN_PROTOTYPES 1
#include "m64p_types.h"
#include "m64p_plugin.h"
#include "hle.h"

// transposed JPEG QTable
static unsigned QTable_T[64] =
{
    16, 12, 14, 14,  18,  24,  49,  72,
    11, 12, 13, 17,  22,  35,  64,  92,
    10, 14, 16, 22,  37,  55,  78,  95,
    16, 19, 24, 29,  56,  64,  87,  98,
    24, 26, 40, 51,  68,  81, 103, 112,
    40, 58, 57, 87, 109, 104, 121, 100,
    51, 60, 69, 80, 103, 113, 120, 103,
    61, 55, 56, 62,  77,  92, 101,  99
};

// ZigZag indices
static unsigned ZigZag[64] =
{
     0,  1,  5,  6, 14, 15, 27, 28,
     2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,
     9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

// Lazy way of transposing a block
static unsigned Transpose[64] =
{
    0,  8, 16, 24, 32, 40, 48, 56,
    1,  9, 17, 25, 33, 41, 49, 57,
    2, 10, 18, 26, 34, 42, 50, 58,
    3, 11, 19, 27, 35, 43, 51, 59,
    4, 12, 20, 28, 36, 44, 52, 60,
    5, 13, 21, 29, 37, 45, 53, 61,
    6, 14, 22, 30, 38, 46, 54, 62,
    7, 15, 23, 31, 39, 47, 55, 63
};

static const unsigned char clamp(short x)
{
    return (x & (0xff00)) ? ((-x) >> 15) & 0xff : x;
}

static short saturate(int x)
{
    if (x > 32767) { x = 32767; } else if (x < -32768) { x = -32768; }
    return x;
}


void ob_jpg_uncompress(OSTask_t *task)
{
    // Fetch arguments
    unsigned pBuffer = task->data_ptr;
    unsigned nMacroBlocks = task->data_size;
    signed QScale = task->yield_data_size;

    // Rescale QTable if needed
    unsigned i;
    unsigned qtable[64];
    unsigned mb;

    int y_dc = 0;
    int u_dc = 0;
    int v_dc = 0;

    DebugMessage(M64MSG_VERBOSE, "OB Task: *buffer=%x, #MB=%d, Qscale=%d\n", pBuffer, nMacroBlocks, QScale);

    if (QScale != 0) {
        if (QScale > 0) {
            for(i = 0; i < 64; i++) {
                unsigned q  = QTable_T[i] * QScale;
                if (q > 32767) q = 32767;
                qtable[i] = q;
            }
        }
        else {
            unsigned Shift = -QScale;
            for(i = 0; i < 64; i++) {
                qtable[i] = QTable_T[i] >> Shift;
            }
        }
    }

    // foreach MB
    for(mb=0; mb < nMacroBlocks; mb++) {
        unsigned sb;
        short macroblock[2][0x300/2];
        unsigned y_offset = 0;

        // load MB into short_buffer
        unsigned offset = pBuffer + 0x300*mb;
        for(i = 0; i < 0x300/2; i++) {
            unsigned short s = rsp.RDRAM[(offset+0)^S8];
            s <<= 8;
            s += rsp.RDRAM[(offset+1)^S8];
            macroblock[0][i] = s;
            offset += 2;
        }

        // foreach SB
        for(sb = 0; sb < 6; sb++) {

            // apply delta to DC
            int dc = (signed)macroblock[0][sb*0x40];
            switch(sb) {
            case 0: case 1: case 2: case 3: y_dc += dc; macroblock[1][sb*0x40] = y_dc & 0xffff; break;
            case 4: u_dc += dc; macroblock[1][sb*0x40] = u_dc & 0xffff; break;
            case 5: v_dc += dc; macroblock[1][sb*0x40] = v_dc & 0xffff; break;
            }

            // zigzag reordering
            for(i = 1; i < 64; i++) {
                macroblock[1][sb*0x40+i] = macroblock[0][sb*0x40+ZigZag[i]];
            }

            // Apply Dequantization
            if (QScale != 0) {
                for(i = 0; i < 64; i++) {
                    int v = macroblock[1][sb*0x40+i] * qtable[i];
                    macroblock[1][sb*0x40+i] = saturate(v);
                }
            }

            // Transpose
            for(i = 0; i < 64; i++) {
                macroblock[0][sb*0x40+i] = macroblock[1][sb*0x40+Transpose[i]];
            }

            // Apply Invert Discrete Cosinus Transform
            idct(&macroblock[0][sb*0x40], &macroblock[1][sb*0x40]);

            // Clamp values between [0..255]
            for(i = 0; i < 64; i++) {
                macroblock[0][sb*0x40+i] = clamp(macroblock[1][sb*0x40+i]);
            }
        }

        // Texel Formatting
        offset = pBuffer + 0x300*mb;
        for(i = 0; i < 8; i++) {
            // U
            rsp.RDRAM[(offset+0x00)^S8] = (unsigned char)macroblock[0][(0x200 + i*0x10)/2];
            rsp.RDRAM[(offset+0x04)^S8] = (unsigned char)macroblock[0][(0x202 + i*0x10)/2];
            rsp.RDRAM[(offset+0x08)^S8] = (unsigned char)macroblock[0][(0x204 + i*0x10)/2];
            rsp.RDRAM[(offset+0x0c)^S8] = (unsigned char)macroblock[0][(0x206 + i*0x10)/2];
            rsp.RDRAM[(offset+0x10)^S8] = (unsigned char)macroblock[0][(0x208 + i*0x10)/2];
            rsp.RDRAM[(offset+0x14)^S8] = (unsigned char)macroblock[0][(0x20a + i*0x10)/2];
            rsp.RDRAM[(offset+0x18)^S8] = (unsigned char)macroblock[0][(0x20c + i*0x10)/2];
            rsp.RDRAM[(offset+0x1c)^S8] = (unsigned char)macroblock[0][(0x20e + i*0x10)/2];
            rsp.RDRAM[(offset+0x20)^S8] = (unsigned char)macroblock[0][(0x200 + i*0x10)/2];
            rsp.RDRAM[(offset+0x24)^S8] = (unsigned char)macroblock[0][(0x202 + i*0x10)/2];
            rsp.RDRAM[(offset+0x28)^S8] = (unsigned char)macroblock[0][(0x204 + i*0x10)/2];
            rsp.RDRAM[(offset+0x2c)^S8] = (unsigned char)macroblock[0][(0x206 + i*0x10)/2];
            rsp.RDRAM[(offset+0x30)^S8] = (unsigned char)macroblock[0][(0x208 + i*0x10)/2];
            rsp.RDRAM[(offset+0x34)^S8] = (unsigned char)macroblock[0][(0x20a + i*0x10)/2];
            rsp.RDRAM[(offset+0x38)^S8] = (unsigned char)macroblock[0][(0x20c + i*0x10)/2];
            rsp.RDRAM[(offset+0x3c)^S8] = (unsigned char)macroblock[0][(0x20e + i*0x10)/2];

            // V
            rsp.RDRAM[(offset+0x02)^S8] = (unsigned char)macroblock[0][(0x280 + i*0x10)/2];
            rsp.RDRAM[(offset+0x06)^S8] = (unsigned char)macroblock[0][(0x282 + i*0x10)/2];
            rsp.RDRAM[(offset+0x0a)^S8] = (unsigned char)macroblock[0][(0x284 + i*0x10)/2];
            rsp.RDRAM[(offset+0x0e)^S8] = (unsigned char)macroblock[0][(0x286 + i*0x10)/2];
            rsp.RDRAM[(offset+0x12)^S8] = (unsigned char)macroblock[0][(0x288 + i*0x10)/2];
            rsp.RDRAM[(offset+0x16)^S8] = (unsigned char)macroblock[0][(0x28a + i*0x10)/2];
            rsp.RDRAM[(offset+0x1a)^S8] = (unsigned char)macroblock[0][(0x28c + i*0x10)/2];
            rsp.RDRAM[(offset+0x1e)^S8] = (unsigned char)macroblock[0][(0x28e + i*0x10)/2];
            rsp.RDRAM[(offset+0x22)^S8] = (unsigned char)macroblock[0][(0x280 + i*0x10)/2];
            rsp.RDRAM[(offset+0x26)^S8] = (unsigned char)macroblock[0][(0x282 + i*0x10)/2];
            rsp.RDRAM[(offset+0x2a)^S8] = (unsigned char)macroblock[0][(0x284 + i*0x10)/2];
            rsp.RDRAM[(offset+0x2e)^S8] = (unsigned char)macroblock[0][(0x286 + i*0x10)/2];
            rsp.RDRAM[(offset+0x32)^S8] = (unsigned char)macroblock[0][(0x288 + i*0x10)/2];
            rsp.RDRAM[(offset+0x36)^S8] = (unsigned char)macroblock[0][(0x28a + i*0x10)/2];
            rsp.RDRAM[(offset+0x3a)^S8] = (unsigned char)macroblock[0][(0x28c + i*0x10)/2];
            rsp.RDRAM[(offset+0x3e)^S8] = (unsigned char)macroblock[0][(0x28e + i*0x10)/2];

            // Ya/Yb
            rsp.RDRAM[(offset+0x01)^S8] = (unsigned char)macroblock[0][(y_offset + 0x00)/2];
            rsp.RDRAM[(offset+0x03)^S8] = (unsigned char)macroblock[0][(y_offset + 0x02)/2];
            rsp.RDRAM[(offset+0x05)^S8] = (unsigned char)macroblock[0][(y_offset + 0x04)/2];
            rsp.RDRAM[(offset+0x07)^S8] = (unsigned char)macroblock[0][(y_offset + 0x06)/2];
            rsp.RDRAM[(offset+0x09)^S8] = (unsigned char)macroblock[0][(y_offset + 0x08)/2];
            rsp.RDRAM[(offset+0x0b)^S8] = (unsigned char)macroblock[0][(y_offset + 0x0a)/2];
            rsp.RDRAM[(offset+0x0d)^S8] = (unsigned char)macroblock[0][(y_offset + 0x0c)/2];
            rsp.RDRAM[(offset+0x0f)^S8] = (unsigned char)macroblock[0][(y_offset + 0x0e)/2];
            rsp.RDRAM[(offset+0x21)^S8] = (unsigned char)macroblock[0][(y_offset + 0x10)/2];
            rsp.RDRAM[(offset+0x23)^S8] = (unsigned char)macroblock[0][(y_offset + 0x12)/2];
            rsp.RDRAM[(offset+0x25)^S8] = (unsigned char)macroblock[0][(y_offset + 0x14)/2];
            rsp.RDRAM[(offset+0x27)^S8] = (unsigned char)macroblock[0][(y_offset + 0x16)/2];
            rsp.RDRAM[(offset+0x29)^S8] = (unsigned char)macroblock[0][(y_offset + 0x18)/2];
            rsp.RDRAM[(offset+0x2b)^S8] = (unsigned char)macroblock[0][(y_offset + 0x1a)/2];
            rsp.RDRAM[(offset+0x2d)^S8] = (unsigned char)macroblock[0][(y_offset + 0x1c)/2];
            rsp.RDRAM[(offset+0x2f)^S8] = (unsigned char)macroblock[0][(y_offset + 0x1e)/2];

            // Ya+1/Yb+1
            rsp.RDRAM[(offset+0x11)^S8] = (unsigned char)macroblock[0][(y_offset + 0x80)/2];
            rsp.RDRAM[(offset+0x13)^S8] = (unsigned char)macroblock[0][(y_offset + 0x82)/2];
            rsp.RDRAM[(offset+0x15)^S8] = (unsigned char)macroblock[0][(y_offset + 0x84)/2];
            rsp.RDRAM[(offset+0x17)^S8] = (unsigned char)macroblock[0][(y_offset + 0x86)/2];
            rsp.RDRAM[(offset+0x19)^S8] = (unsigned char)macroblock[0][(y_offset + 0x88)/2];
            rsp.RDRAM[(offset+0x1b)^S8] = (unsigned char)macroblock[0][(y_offset + 0x8a)/2];
            rsp.RDRAM[(offset+0x1d)^S8] = (unsigned char)macroblock[0][(y_offset + 0x8c)/2];
            rsp.RDRAM[(offset+0x1f)^S8] = (unsigned char)macroblock[0][(y_offset + 0x8e)/2];
            rsp.RDRAM[(offset+0x31)^S8] = (unsigned char)macroblock[0][(y_offset + 0x90)/2];
            rsp.RDRAM[(offset+0x33)^S8] = (unsigned char)macroblock[0][(y_offset + 0x92)/2];
            rsp.RDRAM[(offset+0x35)^S8] = (unsigned char)macroblock[0][(y_offset + 0x94)/2];
            rsp.RDRAM[(offset+0x37)^S8] = (unsigned char)macroblock[0][(y_offset + 0x96)/2];
            rsp.RDRAM[(offset+0x39)^S8] = (unsigned char)macroblock[0][(y_offset + 0x98)/2];
            rsp.RDRAM[(offset+0x3b)^S8] = (unsigned char)macroblock[0][(y_offset + 0x9a)/2];
            rsp.RDRAM[(offset+0x3d)^S8] = (unsigned char)macroblock[0][(y_offset + 0x9c)/2];
            rsp.RDRAM[(offset+0x3f)^S8] = (unsigned char)macroblock[0][(y_offset + 0x9e)/2];

            offset += 0x40;
            y_offset += (i == 3) ? 0xa0 : 0x20;
        }
    }
}



static short yuv2rgba16_clamp(short x)
{
    if (x > 0xff0) { x = 0xff0; } else if (x < 0) { x = 0; }
    return (x & 0xf80);
}


static unsigned short yuv2rgba16(float y, float u, float v)
{
    unsigned short r, g, b;

    r = yuv2rgba16_clamp((short)(y            + 1.4025*v));
    g = yuv2rgba16_clamp((short)(y - 0.3443*u - 0.7144*v));
    b = yuv2rgba16_clamp((short)(y + 1.7729*u           ));

    return (r << 4) | (g >> 1) | (b >> 6) | 1;
}


void ps_jpg_uncompress(OSTask_t *task)
{
    unsigned int iMBsize, oMBsize, nSubBlocks, mb;

    // arguments for pokemon stadium jpg decompression
    static struct 
    {
         unsigned pMacroBlocks; // address of Macroblocks
         unsigned nMacroBlocks; // # of Macroblocks
         unsigned mode;         // specify subsampling mode (as far as I understand)
         unsigned pQTables[3];  // address of QTable for Y,U,V channel
    } ps_jpg_data;

    short QTables[3][64];

    unsigned i,j;

    // We don't support task yielding
    if (task->flags & 0x1) {
        DebugMessage(M64MSG_VERBOSE, "ps_jpg_uncompress doesn't support task yielding");
        return;
    }

    // Fetch arguments
    memcpy(&ps_jpg_data, rsp.RDRAM+task->data_ptr, sizeof(ps_jpg_data));

    DebugMessage(M64MSG_VERBOSE, "SB Task: *MB=%x, #MB=%d, mode=%d, *Qy=%x, *Qu=%x, Qv=%x",
        ps_jpg_data.pMacroBlocks,
        ps_jpg_data.nMacroBlocks,
        ps_jpg_data.mode,
        ps_jpg_data.pQTables[0],
        ps_jpg_data.pQTables[1],
        ps_jpg_data.pQTables[2]);

    // Setup input & output MB size, and #of subblocks
    iMBsize = (ps_jpg_data.mode == 0) ? 0x200 : 0x300;
    oMBsize = (ps_jpg_data.mode == 0) ? 0x100 : 0x200;
    nSubBlocks = ps_jpg_data.mode + 4;

    // Load QTables
    for(j = 0; j < 3; j++) {
        for(i = 0; i < 64; i++) {
            unsigned short s = rsp.RDRAM[(ps_jpg_data.pQTables[j] + 2*i)^S8];
            s <<= 8;
            s |= rsp.RDRAM[(ps_jpg_data.pQTables[j] + 2*i+1)^S8];
            QTables[j][i] = s;
        }
    }

    // foreach MB
    for(mb=0; mb < ps_jpg_data.nMacroBlocks; mb++) {
        unsigned sb;
        short macroblock[2][0x300/2];
        unsigned int y_offset, u_offset;

        // load MB into short_buffer
        unsigned offset = ps_jpg_data.pMacroBlocks + iMBsize*mb;
        for(i = 0; i < iMBsize/2; i++) {
            unsigned short s = rsp.RDRAM[(offset+0)^S8];
            s <<= 8;
            s |= rsp.RDRAM[(offset+1)^S8];
            macroblock[0][i] = s;
            offset += 2;
        }

        // Apply Dequantization (Y subblocks)
        for(sb = 0; sb < nSubBlocks-2; sb++) {
            for(i = 0; i < 64; i++) {
                int v = macroblock[0][sb*0x40+i]*QTables[0][i];
                macroblock[0][sb*0x40+i] = saturate(v) << 4;
            }
        }

        // Apply Dequantization (U,V subblocks)
        for(j = 1; sb < nSubBlocks; sb++, j++) {
            for(i = 0; i < 64; i++) {
                int v = macroblock[0][sb*0x40+i]*QTables[j][i];
                macroblock[0][sb*0x40+i] = saturate(v) << 4;
            }
        }

        // foreach SubBlocks
        for(sb = 0; sb < nSubBlocks; sb++) {
            // ZigZag (transposed)
            for(i = 0; i < 64; i++) {
                macroblock[1][sb*0x40+i] = macroblock[0][sb*0x40+ZigZag[i]];
            }

            // Apply Invert Discrete Cosinus Transform
            idct(&macroblock[1][sb*0x40], &macroblock[0][sb*0x40]);
        }

        // Texel Formatting (RGBA16)
        offset = ps_jpg_data.pMacroBlocks + oMBsize*mb;
        y_offset = 0;
        u_offset = oMBsize/2;

        if (ps_jpg_data.mode == 0)
        {
            // I have not encountered this case in Pokemon stadium (but ucode disassembly say so...)

            unsigned short rgba[16];
            for(i = 0; i < 8; i++) {
                rgba[0]  = yuv2rgba16((float)macroblock[0][y_offset+0]+2048.0f, (float)macroblock[0][u_offset+0], (float)macroblock[0][u_offset+64+0]);
                rgba[1]  = yuv2rgba16((float)macroblock[0][y_offset+1]+2048.0f, (float)macroblock[0][u_offset+0], (float)macroblock[0][u_offset+64+0]);
                rgba[2]  = yuv2rgba16((float)macroblock[0][y_offset+2]+2048.0f, (float)macroblock[0][u_offset+1], (float)macroblock[0][u_offset+64+1]);
                rgba[3]  = yuv2rgba16((float)macroblock[0][y_offset+3]+2048.0f, (float)macroblock[0][u_offset+1], (float)macroblock[0][u_offset+64+1]);
                rgba[4]  = yuv2rgba16((float)macroblock[0][y_offset+4]+2048.0f, (float)macroblock[0][u_offset+2], (float)macroblock[0][u_offset+64+2]);
                rgba[5]  = yuv2rgba16((float)macroblock[0][y_offset+5]+2048.0f, (float)macroblock[0][u_offset+2], (float)macroblock[0][u_offset+64+2]);
                rgba[6]  = yuv2rgba16((float)macroblock[0][y_offset+6]+2048.0f, (float)macroblock[0][u_offset+3], (float)macroblock[0][u_offset+64+3]);
                rgba[7]  = yuv2rgba16((float)macroblock[0][y_offset+7]+2048.0f, (float)macroblock[0][u_offset+3], (float)macroblock[0][u_offset+64+3]);

                rgba[8]  = yuv2rgba16((float)macroblock[0][y_offset+64+0]+2048.0f, (float)macroblock[0][u_offset+4], (float)macroblock[0][u_offset+64+4]);
                rgba[9]  = yuv2rgba16((float)macroblock[0][y_offset+64+1]+2048.0f, (float)macroblock[0][u_offset+4], (float)macroblock[0][u_offset+64+4]);
                rgba[10] = yuv2rgba16((float)macroblock[0][y_offset+64+2]+2048.0f, (float)macroblock[0][u_offset+5], (float)macroblock[0][u_offset+64+5]);
                rgba[11] = yuv2rgba16((float)macroblock[0][y_offset+64+3]+2048.0f, (float)macroblock[0][u_offset+5], (float)macroblock[0][u_offset+64+5]);
                rgba[12] = yuv2rgba16((float)macroblock[0][y_offset+64+4]+2048.0f, (float)macroblock[0][u_offset+6], (float)macroblock[0][u_offset+64+6]);
                rgba[13] = yuv2rgba16((float)macroblock[0][y_offset+64+5]+2048.0f, (float)macroblock[0][u_offset+6], (float)macroblock[0][u_offset+64+6]);
                rgba[14] = yuv2rgba16((float)macroblock[0][y_offset+64+6]+2048.0f, (float)macroblock[0][u_offset+7], (float)macroblock[0][u_offset+64+7]);
                rgba[15] = yuv2rgba16((float)macroblock[0][y_offset+64+7]+2048.0f, (float)macroblock[0][u_offset+7], (float)macroblock[0][u_offset+64+7]);

                for(j = 0; j < 16; j++) {
                    rsp.RDRAM[(offset++)^S8] = (unsigned char)(rgba[j] >> 8);
                    rsp.RDRAM[(offset++)^S8] = (unsigned char)(rgba[j] & 0xff);
                }

                y_offset += 8;
                u_offset += 8;
            }
        }
        else
        {
            unsigned short rgba[32];
            for(i = 0; i < 8; i++) {
                for(j = 0; j < 2; j++) {
                    rgba[j*16+0]  = yuv2rgba16((float)macroblock[0][y_offset+0]+2048.0f, (float)macroblock[0][u_offset+0], (float)macroblock[0][u_offset+64+0]);
                    rgba[j*16+1]  = yuv2rgba16((float)macroblock[0][y_offset+1]+2048.0f, (float)macroblock[0][u_offset+0], (float)macroblock[0][u_offset+64+0]);
                    rgba[j*16+2]  = yuv2rgba16((float)macroblock[0][y_offset+2]+2048.0f, (float)macroblock[0][u_offset+1], (float)macroblock[0][u_offset+64+1]);
                    rgba[j*16+3]  = yuv2rgba16((float)macroblock[0][y_offset+3]+2048.0f, (float)macroblock[0][u_offset+1], (float)macroblock[0][u_offset+64+1]);
                    rgba[j*16+4]  = yuv2rgba16((float)macroblock[0][y_offset+4]+2048.0f, (float)macroblock[0][u_offset+2], (float)macroblock[0][u_offset+64+2]);
                    rgba[j*16+5]  = yuv2rgba16((float)macroblock[0][y_offset+5]+2048.0f, (float)macroblock[0][u_offset+2], (float)macroblock[0][u_offset+64+2]);
                    rgba[j*16+6]  = yuv2rgba16((float)macroblock[0][y_offset+6]+2048.0f, (float)macroblock[0][u_offset+3], (float)macroblock[0][u_offset+64+3]);
                    rgba[j*16+7]  = yuv2rgba16((float)macroblock[0][y_offset+7]+2048.0f, (float)macroblock[0][u_offset+3], (float)macroblock[0][u_offset+64+3]);

                    rgba[j*16+8]  = yuv2rgba16((float)macroblock[0][y_offset+64+0]+2048.0f, (float)macroblock[0][u_offset+4], (float)macroblock[0][u_offset+64+4]);
                    rgba[j*16+9]  = yuv2rgba16((float)macroblock[0][y_offset+64+1]+2048.0f, (float)macroblock[0][u_offset+4], (float)macroblock[0][u_offset+64+4]);
                    rgba[j*16+10] = yuv2rgba16((float)macroblock[0][y_offset+64+2]+2048.0f, (float)macroblock[0][u_offset+5], (float)macroblock[0][u_offset+64+5]);
                    rgba[j*16+11] = yuv2rgba16((float)macroblock[0][y_offset+64+3]+2048.0f, (float)macroblock[0][u_offset+5], (float)macroblock[0][u_offset+64+5]);
                    rgba[j*16+12] = yuv2rgba16((float)macroblock[0][y_offset+64+4]+2048.0f, (float)macroblock[0][u_offset+6], (float)macroblock[0][u_offset+64+6]);
                    rgba[j*16+13] = yuv2rgba16((float)macroblock[0][y_offset+64+5]+2048.0f, (float)macroblock[0][u_offset+6], (float)macroblock[0][u_offset+64+6]);
                    rgba[j*16+14] = yuv2rgba16((float)macroblock[0][y_offset+64+6]+2048.0f, (float)macroblock[0][u_offset+7], (float)macroblock[0][u_offset+64+7]);
                    rgba[j*16+15] = yuv2rgba16((float)macroblock[0][y_offset+64+7]+2048.0f, (float)macroblock[0][u_offset+7], (float)macroblock[0][u_offset+64+7]);
                    y_offset += 8;
                }

                for(j = 0; j < 32; j++) {
                    rsp.RDRAM[(offset++)^S8] = (unsigned char)(rgba[j] >> 8);
                    rsp.RDRAM[(offset++)^S8] = (unsigned char)(rgba[j] & 0xff);
                }

                if (i == 3) y_offset += 64;
                u_offset += 8;
            }
        }
    }
}

