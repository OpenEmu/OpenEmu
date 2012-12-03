/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-rsp-hle - idct.c                                          *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2012 Bobby Smiles                                       *
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

/**
 * Fast 2D IDCT using separable formulation and normalization
 * Computations use single precision floats
 * Implementation based on Wikipedia :
 * http://fr.wikipedia.org/wiki/Transform%C3%A9e_en_cosinus_discr%C3%A8te
 **/

// Normalized to C4 = 1
#define C3   1.175875602f
#define C6   0.541196100f       
 
#define K1   0.765366865f   //  C2-C6
#define K2  -1.847759065f   // -C2-C6
#define K3  -0.390180644f   //  C5-C3
#define K4  -1.961570561f   // -C5-C3
#define K5   1.501321110f   //  C1+C3-C5-C7
#define K6   2.053119869f   //  C1+C3-C5+C7
#define K7   3.072711027f   //  C1+C3+C5-C7
#define K8   0.298631336f   // -C1+C3+C5-C7
#define K9  -0.899976223f   //  C7-C3
#define K10 -2.562915448f   // -C1-C3


static void idct_1d(float *x, float *dst, unsigned every)
{
    float e[4];
    float f[4];
    float x26, x1357, x15, x37, x17, x35;

    x15   =  K3 * (x[1] + x[5]);
    x37   =  K4 * (x[3] + x[7]);
    x17   =  K9 * (x[1] + x[7]);
    x35   = K10 * (x[3] + x[5]);
    x1357 =  C3 * (x[1] + x[3] + x[5] + x[7]);
    x26   =  C6 * (x[2] + x[6]);

    f[0] = x[0] + x[4];
    f[1] = x[0] - x[4];
    f[2] = x26 + K1*x[2];
    f[3] = x26 + K2*x[6];

    e[0] = x1357 + x15 + K5*x[1] + x17;
    e[1] = x1357 + x37 + K7*x[3] + x35;
    e[2] = x1357 + x15 + K6*x[5] + x35;
    e[3] = x1357 + x37 + K8*x[7] + x17;

    *dst = f[0] + f[2] + e[0]; dst += every;
    *dst = f[1] + f[3] + e[1]; dst += every;
    *dst = f[1] - f[3] + e[2]; dst += every;
    *dst = f[0] - f[2] + e[3]; dst += every;
    *dst = f[0] - f[2] - e[3]; dst += every;
    *dst = f[1] - f[3] - e[2]; dst += every;
    *dst = f[1] + f[3] - e[1]; dst += every;
    *dst = f[0] + f[2] - e[0]; dst += every;
}


void idct(short *iblock, short *oblock)
{
    float x[8];
    float tblock[64]; // temporary block

    unsigned i = 0;
    unsigned j = 0;

    // idct 1d on rows (+transposition)
    for(i=0; i<8; i++) {
        for(j=0; j < 8; j++) {
            x[j] = (float)iblock[i*8+j];
        }

        idct_1d(&x[0], &tblock[i], 8);
    }

    // idct 1d on columns (thanks to previous transposition)
    for(i=0; i<8; i++) {
        idct_1d(&tblock[i*8], &x[0], 1);

        // c4 = 1 normalization implies a division by 8
        for(j=0; j < 8; j++) {
            oblock[i+j*8] = (short)x[j] >> 3;
        }
    }
}

