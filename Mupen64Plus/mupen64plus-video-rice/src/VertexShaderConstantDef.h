/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - VertexShaderConstantDef.h                               *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2003 Rice1964                                           *
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

#define iPos        v0
#define iNormal     v1
#define iColor      v2
#define iT0         v3

#define R_NORMAL    r1
#define R_DIFFUSE   r2
#define R_TEMP1     r3
#define R_TEMP2     r4
#define R_TEMP3     r5
#define R_SPECULAR  r6
#define R_POS       r7

#define CV_ZERO         0
#define CV_ONE          1
#define CV_HALF         2
#define CV_TENTH        3
#define CV_NINE_TENTH   4
#define CV_200          5

#define CV_WORLDVIEWPROJ_0 10
#define CV_WORLDVIEWPROJ_1 11
#define CV_WORLDVIEWPROJ_2 12
#define CV_WORLDVIEWPROJ_3 13

#define CV_WORLDVIEW_0 14
#define CV_WORLDVIEW_1 15
#define CV_WORLDVIEW_2 16
#define CV_WORLDVIEW_3 17

#define CV_LIGHT1_DIRECTION 30
#define CV_LIGHT2_DIRECTION 31
#define CV_LIGHT3_DIRECTION 32
#define CV_LIGHT4_DIRECTION 33
#define CV_LIGHT5_DIRECTION 34
#define CV_LIGHT6_DIRECTION 35
#define CV_LIGHT7_DIRECTION 36

#define CV_LIGHT0_AMBIENT 20
#define CV_LIGHT1_DIFFUSE 21
#define CV_LIGHT2_DIFFUSE 22
#define CV_LIGHT3_DIFFUSE 23
#define CV_LIGHT4_DIFFUSE 24
#define CV_LIGHT5_DIFFUSE 25
#define CV_LIGHT6_DIFFUSE 26
#define CV_LIGHT7_DIFFUSE 27

#define USE_PRIMARY_DEPTH   40
#define PRIMARY_DEPTH       41

#define FOG_IS_ENABLED      42

#define USE_PRIMARY_COLOR   43
#define PRIMARY_COLOR       44

#define LIGHTING_ENABLED    45

#define FORCE_VTX_ALPHA     46
#define VTX_ALPHA           47

#define Z_HACK_ENABLE       49

#define T0_SCALE_X_Y      50
#define T1_SCALE_X_Y      51
#define T0_OFFSET_X_Y     52
#define T1_OFFSET_X_Y     53

#define FOG_MUL         60
#define FOG_ADD         61

