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

#ifndef CESCALER_H
#define CESCALER_H

#include "common/scummsys.h"
#include "common/system.h"
#include "graphics/scaler.h"
#include "graphics/scaler/intern.h"

/**
 * This filter (down)scales the source image horizontally by a factor of 11/16
 * and vertically by 7/8. For example, a 320x200 image is scaled to 220x175.
 */
DECLARE_SCALER(SmartphoneLandscape);

#endif
