/*
 * Texture Filtering
 * Version:  1.0
 *
 * Copyright (C) 2007  Hiroshi Morii   All Rights Reserved.
 * Email koolsmoky(at)users.sourceforge.net
 * Web   http://www.3dfxzone.it/koolsmoky
 *
 * this is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * this is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Make; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* Based on Derek Liauw Kie Fa and Rice1964 Super2xSaI code */

  int row0, row1, row2, row3;
  int col0, col1, col2, col3;

  uint16 x;
  uint16 y;

  for (y = 0; y < height; y++) {
    if ((y > 0) && (y < height - 1)) {
      row0 = width;
      row0 = -row0;
      row1 = 0;
      row2 = width;
      row3 = (y == height - 2 ? width : width << 1);
    } else {
      row0 = 0;
      row1 = 0;
      row2 = 0;
      row3 = 0;
    }

    for (x = 0; x < width; x++) {
//--------------------------------------- B0 B1 B2 B3
//                                         4  5  6 S2
//                                         1  2  3 S1
//                                        A0 A1 A2 A3
      if ((x > 0) && (x < width - 1)) {
        col0 = -1;
        col1 = 0;
        col2 = 1;
        col3 = (x == width - 2 ? 1 : 2);
      } else {
        col0 = 0;
        col1 = 0;
        col2 = 0;
        col3 = 0;
      }

      colorB0 = *(srcPtr + col0 + row0);
      colorB1 = *(srcPtr + col1 + row0);
      colorB2 = *(srcPtr + col2 + row0);
      colorB3 = *(srcPtr + col3 + row0);

      color4 = *(srcPtr + col0 + row1);
      color5 = *(srcPtr + col1 + row1);
      color6 = *(srcPtr + col2 + row1);
      colorS2 = *(srcPtr + col3 + row1);

      color1 = *(srcPtr + col0 + row2);
      color2 = *(srcPtr + col1 + row2);
      color3 = *(srcPtr + col2 + row2);
      colorS1 = *(srcPtr + col3 + row2);

      colorA0 = *(srcPtr + col0 + row3);
      colorA1 = *(srcPtr + col1 + row3);
      colorA2 = *(srcPtr + col2 + row3);
      colorA3 = *(srcPtr + col3 + row3);

//--------------------------------------
      if (color2 == color6 && color5 != color3)
        product2b = product1b = color2;
      else if (color5 == color3 && color2 != color6)
        product2b = product1b = color5;
      else if (color5 == color3 && color2 == color6) {
        int r = 0;

        r += GET_RESULT(color6, color5, color1, colorA1);
        r += GET_RESULT(color6, color5, color4, colorB1);
        r += GET_RESULT(color6, color5, colorA2, colorS1);
        r += GET_RESULT(color6, color5, colorB2, colorS2);

        if (r > 0)
          product2b = product1b = color6;
        else if (r < 0)
          product2b = product1b = color5;
        else
          product2b = product1b = SAI_INTERPOLATE(color5, color6);

      } else {

        if (color6 == color3 && color3 == colorA1 && color2 != colorA2 && color3 != colorA0)
          product2b = SAI_Q_INTERPOLATE(color3, color3, color3, color2);
        else if (color5 == color2 && color2 == colorA2 && colorA1 != color3 && color2 != colorA3)
          product2b = SAI_Q_INTERPOLATE(color2, color2, color2, color3);
        else
          product2b = SAI_INTERPOLATE(color2, color3);

        if (color6 == color3 && color6 == colorB1 && color5 != colorB2 && color6 != colorB0)
          product1b = SAI_Q_INTERPOLATE(color6, color6, color6, color5);
        else if (color5 == color2 && color5 == colorB2 && colorB1 != color6 && color5 != colorB3)
          product1b = SAI_Q_INTERPOLATE(color6, color5, color5, color5);
        else
          product1b = SAI_INTERPOLATE(color5, color6);
      }

      if (color5 == color3 && color2 != color6 && color4 == color5 && color5 != colorA2)
        product2a = SAI_INTERPOLATE(color2, color5);
      else if (color5 == color1 && color6 == color5 && color4 != color2 && color5 != colorA0)
        product2a = SAI_INTERPOLATE(color2, color5);
      else
        product2a = color2;

      if (color2 == color6 && color5 != color3 && color1 == color2 && color2 != colorB2)
        product1a = SAI_INTERPOLATE(color2, color5);
      else if (color4 == color2 && color3 == color2 && color1 != color5 && color2 != colorB0)
        product1a = SAI_INTERPOLATE(color2, color5);
      else
        product1a = color5;


      destPtr[0] = product1a;
      destPtr[1] = product1b;
      destPtr[destWidth] = product2a;
      destPtr[destWidth + 1] = product2b;

      srcPtr++;
      destPtr += 2;
    }
    srcPtr += (pitch-width);
    destPtr += (((pitch-width)<<1)+(pitch<<1));
  }
