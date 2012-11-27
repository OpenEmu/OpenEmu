/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#include "common/file.h"
#include "common/macresman.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/surface.h"
#include "graphics/decoders/pict.h"
#include "video/video_decoder.h"

#include "pegasus/pegasus.h"
#include "pegasus/surface.h"

namespace Pegasus {

Surface::Surface() {
	_ownsSurface = false;
	_surface = 0;
}

Surface::~Surface() {
	deallocateSurface();
}

void Surface::deallocateSurface() {
	if (_surface) {
		if (_ownsSurface) {
			_surface->free();
			delete _surface;
		}

		_surface = 0;
		_bounds = Common::Rect();
		_ownsSurface = false;
	}
}

void Surface::shareSurface(Surface *surface) {
	deallocateSurface();

	if (surface) {
		_ownsSurface = false;
		_surface = surface->getSurface();
		surface->getSurfaceBounds(_bounds);
	}
}

void Surface::allocateSurface(const Common::Rect &bounds) {
	deallocateSurface();

	if (bounds.isEmpty())
		return;

	_bounds = bounds;
	_surface = new Graphics::Surface();
	_surface->create(bounds.width(), bounds.height(), g_system->getScreenFormat());
	_ownsSurface = true;
}

void Surface::getImageFromPICTFile(const Common::String &fileName) {
	Common::File pict;
	if (!pict.open(fileName))
		error("Could not open picture '%s'", fileName.c_str());

	if (!getImageFromPICTStream(&pict))
		error("Failed to load PICT '%s'", fileName.c_str());
}

void Surface::getImageFromPICTResource(Common::MacResManager *resFork, uint16 id) {
	Common::SeekableReadStream *res = resFork->getResource(MKTAG('P', 'I', 'C', 'T'), id);
	if (!res)
		error("Could not open PICT resource %d from '%s'", id, resFork->getBaseFileName().c_str());

	if (!getImageFromPICTStream(res))
		error("Failed to load PICT resource %d from '%s'", id, resFork->getBaseFileName().c_str());

	delete res;
}

bool Surface::getImageFromPICTStream(Common::SeekableReadStream *stream) {
	Graphics::PICTDecoder pict;

	if (!pict.loadStream(*stream))
		return false;

	_surface = pict.getSurface()->convertTo(g_system->getScreenFormat(), pict.getPalette());
	_ownsSurface = true;
	_bounds = Common::Rect(0, 0, _surface->w, _surface->h);
	return true;
}

void Surface::getImageFromMovieFrame(Video::VideoDecoder *video, TimeValue time) {
	video->seek(Audio::Timestamp(0, time, 600));
	const Graphics::Surface *frame = video->decodeNextFrame();

	if (frame) {
		if (!_surface)
			_surface = new Graphics::Surface();

		_surface->copyFrom(*frame);
		_ownsSurface = true;
		_bounds = Common::Rect(0, 0, _surface->w, _surface->h);
	} else {
		deallocateSurface();
	}
}

void Surface::copyToCurrentPort() const {
	copyToCurrentPort(_bounds);
}

void Surface::copyToCurrentPortTransparent() const {
	copyToCurrentPortTransparent(_bounds);
}

void Surface::copyToCurrentPort(const Common::Rect &rect) const {
	copyToCurrentPort(rect, rect);
}

void Surface::copyToCurrentPortTransparent(const Common::Rect &rect) const {
	copyToCurrentPortTransparent(rect, rect);
}

void Surface::copyToCurrentPort(const Common::Rect &srcRect, const Common::Rect &dstRect) const {
	Graphics::Surface *screen = ((PegasusEngine *)g_engine)->_gfx->getCurSurface();
	byte *src = (byte *)_surface->getBasePtr(srcRect.left, srcRect.top);
	byte *dst = (byte *)screen->getBasePtr(dstRect.left, dstRect.top);

	int lineSize = srcRect.width() * _surface->format.bytesPerPixel;

	for (int y = 0; y < srcRect.height(); y++) {
		memcpy(dst, src, lineSize);
		src += _surface->pitch;
		dst += screen->pitch;
	}
}

void Surface::copyToCurrentPortTransparent(const Common::Rect &srcRect, const Common::Rect &dstRect) const {
	Graphics::Surface *screen = ((PegasusEngine *)g_engine)->_gfx->getCurSurface();
	byte *src = (byte *)_surface->getBasePtr(srcRect.left, srcRect.top);
	byte *dst = (byte *)screen->getBasePtr(dstRect.left, dstRect.top);

	int lineSize = srcRect.width() * _surface->format.bytesPerPixel;

	for (int y = 0; y < srcRect.height(); y++) {
		for (int x = 0; x < srcRect.width(); x++) {
			if (g_system->getScreenFormat().bytesPerPixel == 2) {
				uint16 color = READ_UINT16(src);
				if (!isTransparent(color))
					memcpy(dst, src, 2);
			} else if (g_system->getScreenFormat().bytesPerPixel == 4) {
				uint32 color = READ_UINT32(src);
				if (!isTransparent(color))
					memcpy(dst, src, 4);
			}

			src += g_system->getScreenFormat().bytesPerPixel;
			dst += g_system->getScreenFormat().bytesPerPixel;
		}

		src += _surface->pitch - lineSize;
		dst += screen->pitch - lineSize;
	}
}

void Surface::copyToCurrentPortMasked(const Common::Rect &srcRect, const Common::Rect &dstRect, const Surface *mask) const {
	Graphics::Surface *screen = ((PegasusEngine *)g_engine)->_gfx->getCurSurface();
	byte *src = (byte *)_surface->getBasePtr(srcRect.left, srcRect.top);
	byte *dst = (byte *)screen->getBasePtr(dstRect.left, dstRect.top);

	int lineSize = srcRect.width() * _surface->format.bytesPerPixel;

	for (int y = 0; y < srcRect.height(); y++) {
		byte *maskSrc = (byte *)mask->getSurface()->getBasePtr(0, y);

		for (int x = 0; x < srcRect.width(); x++) {
			if (g_system->getScreenFormat().bytesPerPixel == 2) {
				uint16 color = READ_UINT16(maskSrc);
				if (!isTransparent(color))
					memcpy(dst, src, 2);
			} else if (g_system->getScreenFormat().bytesPerPixel == 4) {
				uint32 color = READ_UINT32(maskSrc);
				if (!isTransparent(color))
					memcpy(dst, src, 4);
			}

			src += g_system->getScreenFormat().bytesPerPixel;
			maskSrc += g_system->getScreenFormat().bytesPerPixel;
			dst += g_system->getScreenFormat().bytesPerPixel;
		}

		src += _surface->pitch - lineSize;
		dst += screen->pitch - lineSize;
	}
}

void Surface::copyToCurrentPortTransparentGlow(const Common::Rect &srcRect, const Common::Rect &dstRect) const {
	// This is the same as copyToCurrentPortTransparent(), but turns the red value of each
	// pixel all the way up.

	Graphics::Surface *screen = ((PegasusEngine *)g_engine)->_gfx->getCurSurface();
	byte *src = (byte *)_surface->getBasePtr(srcRect.left, srcRect.top);
	byte *dst = (byte *)screen->getBasePtr(dstRect.left, dstRect.top);

	int lineSize = srcRect.width() * _surface->format.bytesPerPixel;

	for (int y = 0; y < srcRect.height(); y++) {
		for (int x = 0; x < srcRect.width(); x++) {
			if (g_system->getScreenFormat().bytesPerPixel == 2) {
				uint16 color = READ_UINT16(src);
				if (!isTransparent(color))
					WRITE_UINT16(dst, getGlowColor(color));
			} else if (g_system->getScreenFormat().bytesPerPixel == 4) {
				uint32 color = READ_UINT32(src);
				if (!isTransparent(color))
					WRITE_UINT32(dst, getGlowColor(color));
			}

			src += g_system->getScreenFormat().bytesPerPixel;
			dst += g_system->getScreenFormat().bytesPerPixel;
		}

		src += _surface->pitch - lineSize;
		dst += screen->pitch - lineSize;
	}
}

void Surface::scaleTransparentCopy(const Common::Rect &srcRect, const Common::Rect &dstRect) const {
	// I'm doing simple linear scaling here
	// dstRect(x, y) = srcRect(x * srcW / dstW, y * srcH / dstH);

	Graphics::Surface *screen = ((PegasusEngine *)g_engine)->_gfx->getCurSurface();

	int srcW = srcRect.width();
	int srcH = srcRect.height();
	int dstW = dstRect.width();
	int dstH = dstRect.height();

	for (int y = 0; y < dstH; y++) {
		for (int x = 0; x < dstW; x++) {
			if (g_system->getScreenFormat().bytesPerPixel == 2) {
				uint16 color = READ_UINT16((byte *)_surface->getBasePtr(
						x * srcW / dstW + srcRect.left,
						y * srcH / dstH + srcRect.top));
				if (!isTransparent(color))
					WRITE_UINT16((byte *)screen->getBasePtr(x + dstRect.left, y + dstRect.top), color);
			} else if (g_system->getScreenFormat().bytesPerPixel == 4) {
				uint32 color = READ_UINT32((byte *)_surface->getBasePtr(
						x * srcW / dstW + srcRect.left,
						y * srcH / dstH + srcRect.top));
				if (!isTransparent(color))
					WRITE_UINT32((byte *)screen->getBasePtr(x + dstRect.left, y + dstRect.top), color);
			}
		}
	}
}

void Surface::scaleTransparentCopyGlow(const Common::Rect &srcRect, const Common::Rect &dstRect) const {
	// This is the same as scaleTransparentCopy(), but turns the red value of each
	// pixel all the way up.

	Graphics::Surface *screen = ((PegasusEngine *)g_engine)->_gfx->getCurSurface();

	int srcW = srcRect.width();
	int srcH = srcRect.height();
	int dstW = dstRect.width();
	int dstH = dstRect.height();

	for (int y = 0; y < dstH; y++) {
		for (int x = 0; x < dstW; x++) {
			if (g_system->getScreenFormat().bytesPerPixel == 2) {
				uint16 color = READ_UINT16((byte *)_surface->getBasePtr(
						x * srcW / dstW + srcRect.left,
						y * srcH / dstH + srcRect.top));
				if (!isTransparent(color))
					WRITE_UINT16((byte *)screen->getBasePtr(x + dstRect.left, y + dstRect.top), getGlowColor(color));
			} else if (g_system->getScreenFormat().bytesPerPixel == 4) {
				uint32 color = READ_UINT32((byte *)_surface->getBasePtr(
						x * srcW / dstW + srcRect.left,
						y * srcH / dstH + srcRect.top));
				if (!isTransparent(color))
					WRITE_UINT32((byte *)screen->getBasePtr(x + dstRect.left, y + dstRect.top), getGlowColor(color));
			}
		}
	}
}

uint32 Surface::getGlowColor(uint32 color) const {
	// Can't just 'or' it on like the original did :P
	byte r, g, b;
	g_system->getScreenFormat().colorToRGB(color, r, g, b);
	return g_system->getScreenFormat().RGBToColor(0xff, g, b);
}

bool Surface::isTransparent(uint32 color) const {
	// HACK: Seems we're truncating some color data somewhere...
	uint32 transColor1 = g_system->getScreenFormat().RGBToColor(0xff, 0xff, 0xff);
	uint32 transColor2 = g_system->getScreenFormat().RGBToColor(0xf8, 0xf8, 0xf8);

	return color == transColor1 || color == transColor2;
}

PixelImage::PixelImage() {
	_transparent = false;
}

void PixelImage::drawImage(const Common::Rect &sourceBounds, const Common::Rect &destBounds) {
	if (!isSurfaceValid())
		return;

	// Draw from sourceBounds to destBounds based on _transparent
	if (_transparent)
		copyToCurrentPortTransparent(sourceBounds, destBounds);
	else
		copyToCurrentPort(sourceBounds, destBounds);
}

void Frame::initFromPICTFile(const Common::String &fileName, bool transparent) {
	getImageFromPICTFile(fileName);
	_transparent = transparent;
}

void Frame::initFromPICTResource(Common::MacResManager *resFork, uint16 id, bool transparent) {
	getImageFromPICTResource(resFork, id);
	_transparent = transparent;
}

void Frame::initFromMovieFrame(Video::VideoDecoder *video, TimeValue time, bool transparent) {
	getImageFromMovieFrame(video, time);
	_transparent = transparent;
}

void Picture::draw(const Common::Rect &r) {
	Common::Rect surfaceBounds;
	getSurfaceBounds(surfaceBounds);
	Common::Rect r1 = r;

	Common::Rect bounds;
	getBounds(bounds);
	surfaceBounds.moveTo(bounds.left, bounds.top);
	r1 = r1.findIntersectingRect(surfaceBounds);
	getSurfaceBounds(surfaceBounds);

	Common::Rect r2 = r1;
	r2.translate(surfaceBounds.left - bounds.left, surfaceBounds.top - bounds.top);
	drawImage(r2, r1);
}

void Picture::initFromPICTFile(const Common::String &fileName, bool transparent) {
	Frame::initFromPICTFile(fileName, transparent);

	Common::Rect surfaceBounds;
	getSurfaceBounds(surfaceBounds);
	sizeElement(surfaceBounds.width(), surfaceBounds.height());
}

void Picture::initFromPICTResource(Common::MacResManager *resFork, uint16 id, bool transparent) {
	Frame::initFromPICTResource(resFork, id, transparent);

	Common::Rect surfaceBounds;
	getSurfaceBounds(surfaceBounds);
	sizeElement(surfaceBounds.width(), surfaceBounds.height());
}

void Picture::initFromMovieFrame(Video::VideoDecoder *video, TimeValue time, bool transparent) {
	Frame::initFromMovieFrame(video, time, transparent);

	Common::Rect surfaceBounds;
	getSurfaceBounds(surfaceBounds);
	sizeElement(surfaceBounds.width(), surfaceBounds.height());
}

} // End of namespace Pegasus
