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

#ifndef SCUMM_SMUSH_FONT_H
#define SCUMM_SMUSH_FONT_H

#include "common/scummsys.h"
#include "scumm/nut_renderer.h"

namespace Scumm {

class SmushFont : public NutRenderer {
protected:
	int16 _color;
	bool _new_colors;
	bool _original;


	int getStringWidth(const char *str);
	int getStringHeight(const char *str);
	int draw2byte(byte *buffer, int dst_width, int x, int y, int idx);
	int drawChar(byte *buffer, int dst_width, int x, int y, byte chr);
	void drawSubstring(const char *str, byte *buffer, int dst_width, int x, int y);

public:
	SmushFont(ScummEngine *vm, const char *filename, bool use_original_colors, bool new_colors);

	void setColor(byte c) { _color = c; }
	void drawString    (const char *str, byte *buffer, int dst_width, int dst_height, int x, int y, bool center);
	void drawStringWrap(const char *str, byte *buffer, int dst_width, int dst_height, int x, int y, int left, int right, bool center);
};

} // End of namespace Scumm

#endif
