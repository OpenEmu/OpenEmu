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

#ifndef GRAPHICS_SCALER_DOWNSCALER_H
#define GRAPHICS_SCALER_DOWNSCALER_H

#include "common/scummsys.h"
#include "graphics/scaler.h"

/**
 * This filter (down)scales the source image by a factor of 1/2.
 * For example, a 320x200 image is scaled to 160x100.
 */
DECLARE_SCALER(DownscaleAllByHalf);

/**
 * This filter (down)scales the source image horizontally by a factor of 1/2.
 * For example, a 320x200 image is scaled to 160x200.
 */
DECLARE_SCALER(DownscaleHorizByHalf);

/**
 * This filter (down)scales the source image horizontally by a factor of 3/4.
 * For example, a 320x200 image is scaled to 240x200.
 */
DECLARE_SCALER(DownscaleHorizByThreeQuarters);

#endif
