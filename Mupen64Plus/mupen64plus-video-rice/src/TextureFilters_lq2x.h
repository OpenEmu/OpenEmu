/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - TextureFilters_lq2x.h                                   *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2003 MaxSt ( maxst@hiend3d.com )                        *
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

case 0 : 
case 2 : 
case 4 : 
case 6 : 
case 8 : 
case 12 : 
case 16 : 
case 20 : 
case 24 : 
case 28 : 
case 32 : 
case 34 : 
case 36 : 
case 38 : 
case 40 : 
case 44 : 
case 48 : 
case 52 : 
case 56 : 
case 60 : 
case 64 : 
case 66 : 
case 68 : 
case 70 : 
case 96 : 
case 98 : 
case 100 : 
case 102 : 
case 128 : 
case 130 : 
case 132 : 
case 134 : 
case 136 : 
case 140 : 
case 144 : 
case 148 : 
case 152 : 
case 156 : 
case 160 : 
case 162 : 
case 164 : 
case 166 : 
case 168 : 
case 172 : 
case 176 : 
case 180 : 
case 184 : 
case 188 : 
case 192 : 
case 194 : 
case 196 : 
case 198 : 
case 224 : 
case 226 : 
case 228 : 
case 230 : 
{
  P0 = IC(0);
  P1 = IC(0);
  P2 = IC(0);
  P3 = IC(0);
} break;
case 1 : 
case 5 : 
case 9 : 
case 13 : 
case 17 : 
case 21 : 
case 25 : 
case 29 : 
case 33 : 
case 37 : 
case 41 : 
case 45 : 
case 49 : 
case 53 : 
case 57 : 
case 61 : 
case 65 : 
case 69 : 
case 97 : 
case 101 : 
case 129 : 
case 133 : 
case 137 : 
case 141 : 
case 145 : 
case 149 : 
case 153 : 
case 157 : 
case 161 : 
case 165 : 
case 169 : 
case 173 : 
case 177 : 
case 181 : 
case 185 : 
case 189 : 
case 193 : 
case 197 : 
case 225 : 
case 229 : 
{
  P0 = IC(1);
  P1 = IC(1);
  P2 = IC(1);
  P3 = IC(1);
} break;
case 3 : 
case 35 : 
case 67 : 
case 99 : 
case 131 : 
case 163 : 
case 195 : 
case 227 : 
{
  P0 = IC(2);
  P1 = IC(2);
  P2 = IC(2);
  P3 = IC(2);
} break;
case 7 : 
case 39 : 
case 71 : 
case 103 : 
case 135 : 
case 167 : 
case 199 : 
case 231 : 
{
  P0 = IC(3);
  P1 = IC(3);
  P2 = IC(3);
  P3 = IC(3);
} break;
case 10 : 
case 138 : 
{
  P1 = IC(0);
  P2 = IC(0);
  P3 = IC(0);
  if (HQ2X_MUL) {
    P0 = IC(0);
  } else {
    P0 = I211(0, 1, 3);
  }
} break;
case 11 : 
case 27 : 
case 75 : 
case 139 : 
case 155 : 
case 203 : 
{
  P1 = IC(2);
  P2 = IC(2);
  P3 = IC(2);
  if (HQ2X_MUL) {
    P0 = IC(2);
  } else {
    P0 = I211(2, 1, 3);
  }
} break;
case 14 : 
case 142 : 
{
  P2 = IC(0);
  P3 = IC(0);
  if (HQ2X_MUL) {
    P0 = IC(0);
    P1 = IC(0);
  } else {
    P0 = I332(1, 3, 0);
    P1 = I31(0, 1);
  }
} break;
case 15 : 
case 143 : 
case 207 : 
{
  P2 = IC(4);
  P3 = IC(4);
  if (HQ2X_MUL) {
    P0 = IC(4);
    P1 = IC(4);
  } else {
    P0 = I332(1, 3, 4);
    P1 = I31(4, 1);
  }
} break;
case 18 : 
case 22 : 
case 30 : 
case 50 : 
case 54 : 
case 62 : 
case 86 : 
case 118 : 
{
  P0 = IC(0);
  P2 = IC(0);
  P3 = IC(0);
  if (HQ2X_MUR) {
    P1 = IC(0);
  } else {
    P1 = I211(0, 1, 5);
  }
} break;
case 19 : 
case 51 : 
{
  P2 = IC(2);
  P3 = IC(2);
  if (HQ2X_MUR) {
    P0 = IC(2);
    P1 = IC(2);
  } else {
    P0 = I31(2, 1);
    P1 = I332(1, 5, 2);
  }
} break;
case 23 : 
case 55 : 
case 119 : 
{
  P2 = IC(3);
  P3 = IC(3);
  if (HQ2X_MUR) {
    P0 = IC(3);
    P1 = IC(3);
  } else {
    P0 = I31(3, 1);
    P1 = I332(1, 5, 3);
  }
} break;
case 26 : 
{
  P2 = IC(0);
  P3 = IC(0);
  if (HQ2X_MUL) {
    P0 = IC(0);
  } else {
    P0 = I211(0, 1, 3);
  }
  if (HQ2X_MUR) {
    P1 = IC(0);
  } else {
    P1 = I211(0, 1, 5);
  }
} break;
case 31 : 
case 95 : 
{
  P2 = IC(4);
  P3 = IC(4);
  if (HQ2X_MUL) {
    P0 = IC(4);
  } else {
    P0 = I211(4, 1, 3);
  }
  if (HQ2X_MUR) {
    P1 = IC(4);
  } else {
    P1 = I211(4, 1, 5);
  }
} break;
case 42 : 
case 170 : 
{
  P1 = IC(0);
  P3 = IC(0);
  if (HQ2X_MUL) {
    P0 = IC(0);
    P2 = IC(0);
  } else {
    P0 = I332(1, 3, 0);
    P2 = I31(0, 3);
  }
} break;
case 43 : 
case 171 : 
case 187 : 
{
  P1 = IC(2);
  P3 = IC(2);
  if (HQ2X_MUL) {
    P0 = IC(2);
    P2 = IC(2);
  } else {
    P0 = I332(1, 3, 2);
    P2 = I31(2, 3);
  }
} break;
case 46 : 
case 174 : 
{
  P1 = IC(0);
  P2 = IC(0);
  P3 = IC(0);
  if (HQ2X_MUL) {
    P0 = IC(0);
  } else {
    P0 = I611(0, 1, 3);
  }
} break;
case 47 : 
case 175 : 
{
  P1 = IC(4);
  P2 = IC(4);
  P3 = IC(4);
  if (HQ2X_MUL) {
    P0 = IC(4);
  } else {
    P0 = I1411(4, 1, 3);
  }
} break;
case 58 : 
case 154 : 
case 186 : 
{
  P2 = IC(0);
  P3 = IC(0);
  if (HQ2X_MUL) {
    P0 = IC(0);
  } else {
    P0 = I611(0, 1, 3);
  }
  if (HQ2X_MUR) {
    P1 = IC(0);
  } else {
    P1 = I611(0, 1, 5);
  }
} break;
case 59 : 
{
  P2 = IC(2);
  P3 = IC(2);
  if (HQ2X_MUL) {
    P0 = IC(2);
  } else {
    P0 = I211(2, 1, 3);
  }
  if (HQ2X_MUR) {
    P1 = IC(2);
  } else {
    P1 = I611(2, 1, 5);
  }
} break;
case 63 : 
{
  P2 = IC(4);
  P3 = IC(4);
  if (HQ2X_MUL) {
    P0 = IC(4);
  } else {
    P0 = I1411(4, 1, 3);
  }
  if (HQ2X_MUR) {
    P1 = IC(4);
  } else {
    P1 = I211(4, 1, 5);
  }
} break;
case 72 : 
case 76 : 
case 104 : 
case 106 : 
case 108 : 
case 110 : 
case 120 : 
case 124 : 
{
  P0 = IC(0);
  P1 = IC(0);
  P3 = IC(0);
  if (HQ2X_MDL) {
    P2 = IC(0);
  } else {
    P2 = I211(0, 3, 7);
  }
} break;
case 73 : 
case 77 : 
case 105 : 
case 109 : 
case 125 : 
{
  P1 = IC(1);
  P3 = IC(1);
  if (HQ2X_MDL) {
    P0 = IC(1);
    P2 = IC(1);
  } else {
    P0 = I31(1, 3);
    P2 = I332(3, 7, 1);
  }
} break;
case 74 : 
{
  P1 = IC(0);
  P3 = IC(0);
  if (HQ2X_MDL) {
    P2 = IC(0);
  } else {
    P2 = I211(0, 3, 7);
  }
  if (HQ2X_MUL) {
    P0 = IC(0);
  } else {
    P0 = I211(0, 1, 3);
  }
} break;
case 78 : 
case 202 : 
case 206 : 
{
  P1 = IC(0);
  P3 = IC(0);
  if (HQ2X_MDL) {
    P2 = IC(0);
  } else {
    P2 = I611(0, 3, 7);
  }
  if (HQ2X_MUL) {
    P0 = IC(0);
  } else {
    P0 = I611(0, 1, 3);
  }
} break;
case 79 : 
{
  P1 = IC(4);
  P3 = IC(4);
  if (HQ2X_MDL) {
    P2 = IC(4);
  } else {
    P2 = I611(4, 3, 7);
  }
  if (HQ2X_MUL) {
    P0 = IC(4);
  } else {
    P0 = I211(4, 1, 3);
  }
} break;
case 80 : 
case 208 : 
case 210 : 
case 216 : 
{
  P0 = IC(0);
  P1 = IC(0);
  P2 = IC(0);
  if (HQ2X_MDR) {
    P3 = IC(0);
  } else {
    P3 = I211(0, 5, 7);
  }
} break;
case 81 : 
case 209 : 
case 217 : 
{
  P0 = IC(1);
  P1 = IC(1);
  P2 = IC(1);
  if (HQ2X_MDR) {
    P3 = IC(1);
  } else {
    P3 = I211(1, 5, 7);
  }
} break;
case 82 : 
case 214 : 
case 222 : 
{
  P0 = IC(0);
  P2 = IC(0);
  if (HQ2X_MDR) {
    P3 = IC(0);
  } else {
    P3 = I211(0, 5, 7);
  }
  if (HQ2X_MUR) {
    P1 = IC(0);
  } else {
    P1 = I211(0, 1, 5);
  }
} break;
case 83 : 
case 115 : 
{
  P0 = IC(2);
  P2 = IC(2);
  if (HQ2X_MDR) {
    P3 = IC(2);
  } else {
    P3 = I611(2, 5, 7);
  }
  if (HQ2X_MUR) {
    P1 = IC(2);
  } else {
    P1 = I611(2, 1, 5);
  }
} break;
case 84 : 
case 212 : 
{
  P0 = IC(0);
  P2 = IC(0);
  if (HQ2X_MDR) {
    P1 = IC(0);
    P3 = IC(0);
  } else {
    P1 = I31(0, 5);
    P3 = I332(5, 7, 0);
  }
} break;
case 85 : 
case 213 : 
case 221 : 
{
  P0 = IC(1);
  P2 = IC(1);
  if (HQ2X_MDR) {
    P1 = IC(1);
    P3 = IC(1);
  } else {
    P1 = I31(1, 5);
    P3 = I332(5, 7, 1);
  }
} break;
case 87 : 
{
  P0 = IC(3);
  P2 = IC(3);
  if (HQ2X_MDR) {
    P3 = IC(3);
  } else {
    P3 = I611(3, 5, 7);
  }
  if (HQ2X_MUR) {
    P1 = IC(3);
  } else {
    P1 = I211(3, 1, 5);
  }
} break;
case 88 : 
case 248 : 
case 250 : 
{
  P0 = IC(0);
  P1 = IC(0);
  if (HQ2X_MDL) {
    P2 = IC(0);
  } else {
    P2 = I211(0, 3, 7);
  }
  if (HQ2X_MDR) {
    P3 = IC(0);
  } else {
    P3 = I211(0, 5, 7);
  }
} break;
case 89 : 
case 93 : 
{
  P0 = IC(1);
  P1 = IC(1);
  if (HQ2X_MDL) {
    P2 = IC(1);
  } else {
    P2 = I611(1, 3, 7);
  }
  if (HQ2X_MDR) {
    P3 = IC(1);
  } else {
    P3 = I611(1, 5, 7);
  }
} break;
case 90 : 
{
  if (HQ2X_MDL) {
    P2 = IC(0);
  } else {
    P2 = I611(0, 3, 7);
  }
  if (HQ2X_MDR) {
    P3 = IC(0);
  } else {
    P3 = I611(0, 5, 7);
  }
  if (HQ2X_MUL) {
    P0 = IC(0);
  } else {
    P0 = I611(0, 1, 3);
  }
  if (HQ2X_MUR) {
    P1 = IC(0);
  } else {
    P1 = I611(0, 1, 5);
  }
} break;
case 91 : 
{
  if (HQ2X_MDL) {
    P2 = IC(2);
  } else {
    P2 = I611(2, 3, 7);
  }
  if (HQ2X_MDR) {
    P3 = IC(2);
  } else {
    P3 = I611(2, 5, 7);
  }
  if (HQ2X_MUL) {
    P0 = IC(2);
  } else {
    P0 = I211(2, 1, 3);
  }
  if (HQ2X_MUR) {
    P1 = IC(2);
  } else {
    P1 = I611(2, 1, 5);
  }
} break;
case 92 : 
{
  P0 = IC(0);
  P1 = IC(0);
  if (HQ2X_MDL) {
    P2 = IC(0);
  } else {
    P2 = I611(0, 3, 7);
  }
  if (HQ2X_MDR) {
    P3 = IC(0);
  } else {
    P3 = I611(0, 5, 7);
  }
} break;
case 94 : 
{
  if (HQ2X_MDL) {
    P2 = IC(0);
  } else {
    P2 = I611(0, 3, 7);
  }
  if (HQ2X_MDR) {
    P3 = IC(0);
  } else {
    P3 = I611(0, 5, 7);
  }
  if (HQ2X_MUL) {
    P0 = IC(0);
  } else {
    P0 = I611(0, 1, 3);
  }
  if (HQ2X_MUR) {
    P1 = IC(0);
  } else {
    P1 = I211(0, 1, 5);
  }
} break;
case 107 : 
case 123 : 
{
  P1 = IC(2);
  P3 = IC(2);
  if (HQ2X_MDL) {
    P2 = IC(2);
  } else {
    P2 = I211(2, 3, 7);
  }
  if (HQ2X_MUL) {
    P0 = IC(2);
  } else {
    P0 = I211(2, 1, 3);
  }
} break;
case 111 : 
{
  P1 = IC(4);
  P3 = IC(4);
  if (HQ2X_MDL) {
    P2 = IC(4);
  } else {
    P2 = I211(4, 3, 7);
  }
  if (HQ2X_MUL) {
    P0 = IC(4);
  } else {
    P0 = I1411(4, 1, 3);
  }
} break;
case 112 : 
case 240 : 
{
  P0 = IC(0);
  P1 = IC(0);
  if (HQ2X_MDR) {
    P2 = IC(0);
    P3 = IC(0);
  } else {
    P2 = I31(0, 7);
    P3 = I332(5, 7, 0);
  }
} break;
case 113 : 
case 241 : 
{
  P0 = IC(1);
  P1 = IC(1);
  if (HQ2X_MDR) {
    P2 = IC(1);
    P3 = IC(1);
  } else {
    P2 = I31(1, 7);
    P3 = I332(5, 7, 1);
  }
} break;
case 114 : 
{
  P0 = IC(0);
  P2 = IC(0);
  if (HQ2X_MDR) {
    P3 = IC(0);
  } else {
    P3 = I611(0, 5, 7);
  }
  if (HQ2X_MUR) {
    P1 = IC(0);
  } else {
    P1 = I611(0, 1, 5);
  }
} break;
case 116 : 
{
  P0 = IC(0);
  P1 = IC(0);
  P2 = IC(0);
  if (HQ2X_MDR) {
    P3 = IC(0);
  } else {
    P3 = I611(0, 5, 7);
  }
} break;
case 117 : 
{
  P0 = IC(1);
  P1 = IC(1);
  P2 = IC(1);
  if (HQ2X_MDR) {
    P3 = IC(1);
  } else {
    P3 = I611(1, 5, 7);
  }
} break;
case 121 : 
{
  P0 = IC(1);
  P1 = IC(1);
  if (HQ2X_MDL) {
    P2 = IC(1);
  } else {
    P2 = I211(1, 3, 7);
  }
  if (HQ2X_MDR) {
    P3 = IC(1);
  } else {
    P3 = I611(1, 5, 7);
  }
} break;
case 122 : 
{
  if (HQ2X_MDL) {
    P2 = IC(0);
  } else {
    P2 = I211(0, 3, 7);
  }
  if (HQ2X_MDR) {
    P3 = IC(0);
  } else {
    P3 = I611(0, 5, 7);
  }
  if (HQ2X_MUL) {
    P0 = IC(0);
  } else {
    P0 = I611(0, 1, 3);
  }
  if (HQ2X_MUR) {
    P1 = IC(0);
  } else {
    P1 = I611(0, 1, 5);
  }
} break;
case 126 : 
{
  P0 = IC(0);
  P3 = IC(0);
  if (HQ2X_MDL) {
    P2 = IC(0);
  } else {
    P2 = I211(0, 3, 7);
  }
  if (HQ2X_MUR) {
    P1 = IC(0);
  } else {
    P1 = I211(0, 1, 5);
  }
} break;
case 127 : 
{
  P3 = IC(4);
  if (HQ2X_MDL) {
    P2 = IC(4);
  } else {
    P2 = I211(4, 3, 7);
  }
  if (HQ2X_MUL) {
    P0 = IC(4);
  } else {
    P0 = I1411(4, 1, 3);
  }
  if (HQ2X_MUR) {
    P1 = IC(4);
  } else {
    P1 = I211(4, 1, 5);
  }
} break;
case 146 : 
case 150 : 
case 178 : 
case 182 : 
case 190 : 
{
  P0 = IC(0);
  P2 = IC(0);
  if (HQ2X_MUR) {
    P1 = IC(0);
    P3 = IC(0);
  } else {
    P1 = I332(1, 5, 0);
    P3 = I31(0, 5);
  }
} break;
case 147 : 
case 179 : 
{
  P0 = IC(2);
  P2 = IC(2);
  P3 = IC(2);
  if (HQ2X_MUR) {
    P1 = IC(2);
  } else {
    P1 = I611(2, 1, 5);
  }
} break;
case 151 : 
case 183 : 
{
  P0 = IC(3);
  P2 = IC(3);
  P3 = IC(3);
  if (HQ2X_MUR) {
    P1 = IC(3);
  } else {
    P1 = I1411(3, 1, 5);
  }
} break;
case 158 : 
{
  P2 = IC(0);
  P3 = IC(0);
  if (HQ2X_MUL) {
    P0 = IC(0);
  } else {
    P0 = I611(0, 1, 3);
  }
  if (HQ2X_MUR) {
    P1 = IC(0);
  } else {
    P1 = I211(0, 1, 5);
  }
} break;
case 159 : 
{
  P2 = IC(4);
  P3 = IC(4);
  if (HQ2X_MUL) {
    P0 = IC(4);
  } else {
    P0 = I211(4, 1, 3);
  }
  if (HQ2X_MUR) {
    P1 = IC(4);
  } else {
    P1 = I1411(4, 1, 5);
  }
} break;
case 191 : 
{
  P2 = IC(4);
  P3 = IC(4);
  if (HQ2X_MUL) {
    P0 = IC(4);
  } else {
    P0 = I1411(4, 1, 3);
  }
  if (HQ2X_MUR) {
    P1 = IC(4);
  } else {
    P1 = I1411(4, 1, 5);
  }
} break;
case 200 : 
case 204 : 
case 232 : 
case 236 : 
case 238 : 
{
  P0 = IC(0);
  P1 = IC(0);
  if (HQ2X_MDL) {
    P2 = IC(0);
    P3 = IC(0);
  } else {
    P2 = I332(3, 7, 0);
    P3 = I31(0, 7);
  }
} break;
case 201 : 
case 205 : 
{
  P0 = IC(1);
  P1 = IC(1);
  P3 = IC(1);
  if (HQ2X_MDL) {
    P2 = IC(1);
  } else {
    P2 = I611(1, 3, 7);
  }
} break;
case 211 : 
{
  P0 = IC(2);
  P1 = IC(2);
  P2 = IC(2);
  if (HQ2X_MDR) {
    P3 = IC(2);
  } else {
    P3 = I211(2, 5, 7);
  }
} break;
case 215 : 
{
  P0 = IC(3);
  P2 = IC(3);
  if (HQ2X_MDR) {
    P3 = IC(3);
  } else {
    P3 = I211(3, 5, 7);
  }
  if (HQ2X_MUR) {
    P1 = IC(3);
  } else {
    P1 = I1411(3, 1, 5);
  }
} break;
case 218 : 
{
  if (HQ2X_MDL) {
    P2 = IC(0);
  } else {
    P2 = I611(0, 3, 7);
  }
  if (HQ2X_MDR) {
    P3 = IC(0);
  } else {
    P3 = I211(0, 5, 7);
  }
  if (HQ2X_MUL) {
    P0 = IC(0);
  } else {
    P0 = I611(0, 1, 3);
  }
  if (HQ2X_MUR) {
    P1 = IC(0);
  } else {
    P1 = I611(0, 1, 5);
  }
} break;
case 219 : 
{
  P1 = IC(2);
  P2 = IC(2);
  if (HQ2X_MDR) {
    P3 = IC(2);
  } else {
    P3 = I211(2, 5, 7);
  }
  if (HQ2X_MUL) {
    P0 = IC(2);
  } else {
    P0 = I211(2, 1, 3);
  }
} break;
case 220 : 
{
  P0 = IC(0);
  P1 = IC(0);
  if (HQ2X_MDL) {
    P2 = IC(0);
  } else {
    P2 = I611(0, 3, 7);
  }
  if (HQ2X_MDR) {
    P3 = IC(0);
  } else {
    P3 = I211(0, 5, 7);
  }
} break;
case 223 : 
{
  P2 = IC(4);
  if (HQ2X_MDR) {
    P3 = IC(4);
  } else {
    P3 = I211(4, 5, 7);
  }
  if (HQ2X_MUL) {
    P0 = IC(4);
  } else {
    P0 = I211(4, 1, 3);
  }
  if (HQ2X_MUR) {
    P1 = IC(4);
  } else {
    P1 = I1411(4, 1, 5);
  }
} break;
case 233 : 
case 237 : 
{
  P0 = IC(1);
  P1 = IC(1);
  P3 = IC(1);
  if (HQ2X_MDL) {
    P2 = IC(1);
  } else {
    P2 = I1411(1, 3, 7);
  }
} break;
case 234 : 
{
  P1 = IC(0);
  P3 = IC(0);
  if (HQ2X_MDL) {
    P2 = IC(0);
  } else {
    P2 = I211(0, 3, 7);
  }
  if (HQ2X_MUL) {
    P0 = IC(0);
  } else {
    P0 = I611(0, 1, 3);
  }
} break;
case 235 : 
{
  P1 = IC(2);
  P3 = IC(2);
  if (HQ2X_MDL) {
    P2 = IC(2);
  } else {
    P2 = I1411(2, 3, 7);
  }
  if (HQ2X_MUL) {
    P0 = IC(2);
  } else {
    P0 = I211(2, 1, 3);
  }
} break;
case 239 : 
{
  P1 = IC(4);
  P3 = IC(4);
  if (HQ2X_MDL) {
    P2 = IC(4);
  } else {
    P2 = I1411(4, 3, 7);
  }
  if (HQ2X_MUL) {
    P0 = IC(4);
  } else {
    P0 = I1411(4, 1, 3);
  }
} break;
case 242 : 
{
  P0 = IC(0);
  P2 = IC(0);
  if (HQ2X_MDR) {
    P3 = IC(0);
  } else {
    P3 = I211(0, 5, 7);
  }
  if (HQ2X_MUR) {
    P1 = IC(0);
  } else {
    P1 = I611(0, 1, 5);
  }
} break;
case 243 : 
{
  P0 = IC(2);
  P1 = IC(2);
  if (HQ2X_MDR) {
    P2 = IC(2);
    P3 = IC(2);
  } else {
    P2 = I31(2, 7);
    P3 = I332(5, 7, 2);
  }
} break;
case 244 : 
{
  P0 = IC(0);
  P1 = IC(0);
  P2 = IC(0);
  if (HQ2X_MDR) {
    P3 = IC(0);
  } else {
    P3 = I1411(0, 5, 7);
  }
} break;
case 245 : 
{
  P0 = IC(1);
  P1 = IC(1);
  P2 = IC(1);
  if (HQ2X_MDR) {
    P3 = IC(1);
  } else {
    P3 = I1411(1, 5, 7);
  }
} break;
case 246 : 
{
  P0 = IC(0);
  P2 = IC(0);
  if (HQ2X_MDR) {
    P3 = IC(0);
  } else {
    P3 = I1411(0, 5, 7);
  }
  if (HQ2X_MUR) {
    P1 = IC(0);
  } else {
    P1 = I211(0, 1, 5);
  }
} break;
case 247 : 
{
  P0 = IC(3);
  P2 = IC(3);
  if (HQ2X_MDR) {
    P3 = IC(3);
  } else {
    P3 = I1411(3, 5, 7);
  }
  if (HQ2X_MUR) {
    P1 = IC(3);
  } else {
    P1 = I1411(3, 1, 5);
  }
} break;
case 249 : 
{
  P0 = IC(1);
  P1 = IC(1);
  if (HQ2X_MDL) {
    P2 = IC(1);
  } else {
    P2 = I1411(1, 3, 7);
  }
  if (HQ2X_MDR) {
    P3 = IC(1);
  } else {
    P3 = I211(1, 5, 7);
  }
} break;
case 251 : 
{
  P1 = IC(2);
  if (HQ2X_MDL) {
    P2 = IC(2);
  } else {
    P2 = I1411(2, 3, 7);
  }
  if (HQ2X_MDR) {
    P3 = IC(2);
  } else {
    P3 = I211(2, 5, 7);
  }
  if (HQ2X_MUL) {
    P0 = IC(2);
  } else {
    P0 = I211(2, 1, 3);
  }
} break;
case 252 : 
{
  P0 = IC(0);
  P1 = IC(0);
  if (HQ2X_MDL) {
    P2 = IC(0);
  } else {
    P2 = I211(0, 3, 7);
  }
  if (HQ2X_MDR) {
    P3 = IC(0);
  } else {
    P3 = I1411(0, 5, 7);
  }
} break;
case 253 : 
{
  P0 = IC(1);
  P1 = IC(1);
  if (HQ2X_MDL) {
    P2 = IC(1);
  } else {
    P2 = I1411(1, 3, 7);
  }
  if (HQ2X_MDR) {
    P3 = IC(1);
  } else {
    P3 = I1411(1, 5, 7);
  }
} break;
case 254 : 
{
  P0 = IC(0);
  if (HQ2X_MDL) {
    P2 = IC(0);
  } else {
    P2 = I211(0, 3, 7);
  }
  if (HQ2X_MDR) {
    P3 = IC(0);
  } else {
    P3 = I1411(0, 5, 7);
  }
  if (HQ2X_MUR) {
    P1 = IC(0);
  } else {
    P1 = I211(0, 1, 5);
  }
} break;
case 255 : 
{
  if (HQ2X_MDL) {
    P2 = IC(4);
  } else {
    P2 = I1411(4, 3, 7);
  }
  if (HQ2X_MDR) {
    P3 = IC(4);
  } else {
    P3 = I1411(4, 5, 7);
  }
  if (HQ2X_MUL) {
    P0 = IC(4);
  } else {
    P0 = I1411(4, 1, 3);
  }
  if (HQ2X_MUR) {
    P1 = IC(4);
  } else {
    P1 = I1411(4, 1, 5);
  }
} break;

