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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_UTILS_H
#define WINTERMUTE_UTILS_H

#include "engines/wintermute/wintypes.h"
#include "engines/wintermute/math/rect32.h"

namespace Wintermute {

class BaseGame;

class BaseUtils {
public:
	static void swap(int *a, int *b);
	static float normalizeAngle(float angle);

	static void createPath(const char *path, bool pathOnly = false);

	static void debugMessage(const char *text);
	static char *setString(char **string, const char *value);

	static char *strEntry(int entry, const char *str, const char delim = ',');

	static int randomInt(int from, int to);
	static float randomFloat(float from, float to);
	static float randomAngle(float from, float to);

	static void RGBtoHSL(uint32 rgbColor, byte *outH, byte *outS, byte *outL);
	static uint32 HSLtoRGB(byte H, byte S, byte L);

private:
	static float Hue2RGB(float v1, float v2, float vH);
};

} // end of namespace Wintermute

#endif
