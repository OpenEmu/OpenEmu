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

#ifndef ENGINES_UTIL_H
#define ENGINES_UTIL_H

#include "common/scummsys.h"
#include "common/list.h"
#include "graphics/pixelformat.h"

/**
 * Setup the backend's graphics mode.
 */
void initCommonGFX(bool defaultTo1XScaler);

/**
 * Setup the backend's screen size and graphics mode.
 *
 * Shows an various warnings on certain backend graphics
 * transaction failures (aspect switch, fullscreen switch, etc.).
 *
 * Errors out when backend is not able to switch to the specified
 * mode.
 *
 * Defaults to 256 color paletted mode if no graphics format is provided.
 * Uses the backend's preferred format if graphics format pointer is NULL.
 * Finds the best compatible format if a list of graphics formats is provided.
 */
void initGraphics(int width, int height, bool defaultTo1xScaler);
void initGraphics(int width, int height, bool defaultTo1xScaler, const Graphics::PixelFormat *format);
void initGraphics(int width, int height, bool defaultTo1xScaler, const Common::List<Graphics::PixelFormat> &formatList);

#endif
