/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - cfb.c                                                   *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 Marshallh                                          *
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

#include <ultra64.h>

#include "include/config.h"

#ifdef SCREEN_BPP_8
u8      _cfb[NUM_CFB][SCREEN_WIDTH * SCREEN_HEIGHT]; /* 8-bit */
#else
#ifdef SCREEN_BPP_16
u16     _cfb[NUM_CFB][SCREEN_WIDTH * SCREEN_HEIGHT]; /* 16-bit */
#else
#ifdef SCREEN_BPP_32
u32     _cfb[NUM_CFB][SCREEN_WIDTH * SCREEN_HEIGHT]; /* 32-bit */
#endif
#endif
#endif

