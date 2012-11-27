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

#include "teenagent/surface.h"
#include "teenagent/pack.h"
#include "teenagent/teenagent.h"

#include "common/stream.h"
#include "common/debug.h"

namespace TeenAgent {

Surface::Surface() : x(0), y(0) {
}

Surface::~Surface() {
	free();
}

void Surface::load(Common::SeekableReadStream &stream, Type type) {
	debugC(0, kDebugSurface, "load()");
	free();

	x = y = 0;

	uint16 w_ = stream.readUint16LE();
	uint16 h_ = stream.readUint16LE();

	if (type != kTypeLan) {
		uint16 pos = stream.readUint16LE();
		x = pos % kScreenWidth;
		y = pos / kScreenWidth;
	}

	debugC(0, kDebugSurface, "declared info: %ux%u (%04xx%04x) -> %u,%u", w_, h_, w_, h_, x, y);
	if (stream.eos() || w_ == 0)
		return;

	if (w_ * h_ > stream.size()) {
		debugC(0, kDebugSurface, "invalid surface %ux%u -> %u,%u", w_, h_, x, y);
		return;
	}

	debugC(0, kDebugSurface, "creating surface %ux%u -> %u,%u", w_, h_, x, y);
	create(w_, h_, Graphics::PixelFormat::createFormatCLUT8());

	stream.read(pixels, w_ * h_);
}

Common::Rect Surface::render(Graphics::Surface *surface, int dx, int dy, bool mirror, Common::Rect srcRect, uint zoom) const {
	if (srcRect.isEmpty()) {
		srcRect = Common::Rect(0, 0, w, h);
	}
	Common::Rect dstRect(x + dx, y + dy, x + dx + zoom * srcRect.width() / 256, y + dy + zoom * srcRect.height() / 256);
	if (dstRect.left < 0) {
		srcRect.left = -dstRect.left;
		dstRect.left = 0;
	}
	if (dstRect.right > surface->w) {
		srcRect.right -= dstRect.right - surface->w;
		dstRect.right = surface->w;
	}
	if (dstRect.top < 0) {
		srcRect.top -= dstRect.top;
		dstRect.top = 0;
	}
	if (dstRect.bottom > surface->h) {
		srcRect.bottom -= dstRect.bottom - surface->h;
		dstRect.bottom = surface->h;
	}
	if (srcRect.isEmpty() || dstRect.isEmpty())
		return Common::Rect();

	if (zoom == 256) {
		const byte *src = (const byte *)getBasePtr(0, srcRect.top);
		byte *dstBase = (byte *)surface->getBasePtr(dstRect.left, dstRect.top);

		for (int i = srcRect.top; i < srcRect.bottom; ++i) {
			byte *dst = dstBase;
			for (int j = srcRect.left; j < srcRect.right; ++j) {
				byte p = src[(mirror ? w - j - 1 : j)];
				if (p != 0xff)
					*dst++ = p;
				else
					++dst;
			}
			dstBase += surface->pitch;
			src += pitch;
		}
	} else {
		byte *dst = (byte *)surface->getBasePtr(dstRect.left, dstRect.top);
		for (int i = 0; i < dstRect.height(); ++i) {
			for (int j = 0; j < dstRect.width(); ++j) {
				int px = j * 256 / zoom;
				const byte *src = (const byte *)getBasePtr(srcRect.left + (mirror ? w - px - 1 : px), srcRect.top + i * 256 / zoom);
				byte p = *src;
				if (p != 0xff)
					dst[j] = p;
			}
			dst += surface->pitch;
		}
	}
	return dstRect;
}

} // End of namespace TeenAgent
