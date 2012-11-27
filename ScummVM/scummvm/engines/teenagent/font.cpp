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

#include "teenagent/font.h"

#include "teenagent/pack.h"
#include "teenagent/teenagent.h"

#include "common/endian.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/ptr.h"
#include "graphics/surface.h"

namespace TeenAgent {

Font::Font() : _gridColor(0xd0), _shadowColor(0), _height(0), _widthPack(0), _data(0) {
}

Font::~Font() {
	delete[] _data;
}

void Font::load(const Pack &pack, int id, byte height, byte widthPack) {
	delete[] _data;
	_data = NULL;

	Common::ScopedPtr<Common::SeekableReadStream> s(pack.getStream(id));
	if (!s)
		error("loading font %d failed", id);

	_data = new byte[s->size()];
	s->read(_data, s->size());
	debugC(0, kDebugFont, "font size: %d", s->size());

	_height = height;
	_widthPack = widthPack;
}

uint Font::render(Graphics::Surface *surface, int x, int y, char c, byte color) {
	unsigned idx = (unsigned char)c;
	if (idx < 0x20 || idx >= 0x81) {
		debugC(0, kDebugFont, "unhandled char 0x%02x", idx);
		return 0;
	}
	idx -= 0x20;
	byte *glyph = _data + READ_LE_UINT16(_data + idx * 2);

	int h = glyph[0], w = glyph[1];
	if (surface == NULL || surface->pixels == NULL || y + h <= 0 || y >= kScreenHeight || x + w <= 0 || x >= kScreenWidth)
		return w - _widthPack;

	int i0 = 0, j0 = 0;
	if (x < 0) {
		j0 = -x;
		x = 0;
	}
	if (y < 0) {
		i0 = -y;
		y = 0;
	}
	debugC(0, kDebugFont, "char %c, width: %dx%d", c, w, h);
	glyph += 2;
	glyph += i0 * w + j0;
	byte *dst = (byte *)surface->getBasePtr(x, y);
	byte *end = (byte *)surface->getBasePtr(0, surface->h);
	for (int i = i0; i < h && dst < end; ++i) {
		for (int j = j0; j < w; ++j) {
			byte v = *glyph++;
			switch (v) {
			case 0:
				break;
			case 1:
				dst[j] = _shadowColor;
				break;
			case 2:
				dst[j] = color;
				break;
			default:
				dst[j] = v;
			}
		}
		dst += surface->pitch;
	}
	return w - _widthPack;
}

static uint findInStr(const Common::String &str, char c, uint pos = 0) {
	while (pos < str.size() && str[pos] != c) ++pos;
	return pos;
}

uint Font::render(Graphics::Surface *surface, int x, int y, const Common::String &str, byte color, bool showGrid) {
	if (surface != NULL) {
		uint maxW = render(NULL, 0, 0, str, false);
		if (showGrid)
			grid(surface, x - 4, y - 2, maxW + 8, 8 + 6, _gridColor);

		uint i = 0, j;
		do {
			j = findInStr(str, '\n', i);
			Common::String line(str.c_str() + i, j - i);
			debugC(0, kDebugFont, "line: %s", line.c_str());

			if (y + (int)_height >= 0) {
				uint w = render(NULL, 0, 0, line, false);
				int xp = x + (maxW - w) / 2;
				for (uint k = 0; k < line.size(); ++k) {
					xp += render(surface, xp, y, line[k], color);
				}
			} else if (y >= kScreenHeight)
				break;

			y += _height;
			i = j + 1;
		} while (i < str.size());
		return maxW;
	} else {
		// surface == NULL;
		uint w = 0, maxW = 0;
		for (uint i = 0; i < str.size(); ++i) {
			char c = str[i];
			if (c == '\n') {
				y += _height;
				if (w > maxW)
					maxW = w;
				w = 0;
				continue;
			}
			w += render(NULL, 0, 0, c, color);
		}
		if (w > maxW)
			maxW = w;

		return maxW;
	}
}

void Font::grid(Graphics::Surface *surface, int x, int y, int w, int h, byte color) {
	byte *dst = (byte *)surface->getBasePtr(x, y);
	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			if (((i ^ j) & 1) == 0)
				dst[j] = color;
		}
		dst += surface->pitch;
	}
}

} // End of namespace TeenAgent
