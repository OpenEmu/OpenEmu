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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef GRAPHICS_SCALER_ASPECT_H
#define GRAPHICS_SCALER_ASPECT_H

#include "common/scummsys.h"
#include "graphics/scaler.h"

/**
 * TODO: explain
 */
FORCEINLINE int real2Aspect(int y) {
	return y + (y + 1) / 5;
}

/**
 * TODO: explain
 */
FORCEINLINE int aspect2Real(int y) {
	return (y * 5 + 4) / 6;
}

/**
 * TODO: explain
 */
void makeRectStretchable(int &x, int &y, int &w, int &h);

/**
 * TODO: explain
 */
int stretch200To240(uint8 *buf,
                    uint32 pitch,
                    int width,
                    int height,
                    int srcX,
                    int srcY,
                    int origSrcY);


/**
 * This filter (up)scales the source image vertically by a factor of 6/5.
 * For example, a 320x200 image is scaled to 320x240.
 *
 * The main difference to the code in graphics/scaler/aspect.cpp is the
 * out-of-place operation, omitting a straight blit step the sdl backend
 * does. Also, tests show unaligned access errors with the stock aspect scaler.
 */
DECLARE_SCALER(Normal1xAspect);

#ifdef USE_ARM_SCALER_ASM
DECLARE_SCALER(Normal2xAspect);
#endif

#endif
