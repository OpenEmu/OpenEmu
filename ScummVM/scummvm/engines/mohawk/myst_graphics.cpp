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

#include "mohawk/myst.h"
#include "mohawk/myst_graphics.h"
#include "mohawk/resource.h"

#include "common/substream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "engines/util.h"
#include "graphics/decoders/jpeg.h"
#include "graphics/decoders/pict.h"

namespace Mohawk {

MystGraphics::MystGraphics(MohawkEngine_Myst* vm) : GraphicsManager(), _vm(vm) {
	_bmpDecoder = new MystBitmap();

	_viewport = Common::Rect(544, 332);

	// The original version of Myst could run in 8bpp color too.
	// However, it dithered videos to 8bpp and they looked considerably
	// worse (than they already did :P). So we're not even going to
	// support 8bpp mode in Myst (Myst ME required >8bpp anyway).
	initGraphics(_viewport.width(), _viewport.height(), true, NULL); // What an odd screen size!

	_pixelFormat = _vm->_system->getScreenFormat();

	if (_pixelFormat.bytesPerPixel == 1)
		error("Myst requires greater than 256 colors to run");

	// Initialize our buffer
	_backBuffer = new Graphics::Surface();
	_backBuffer->create(_vm->_system->getWidth(), _vm->_system->getHeight(), _pixelFormat);

	_nextAllowedDrawTime = _vm->_system->getMillis();
	_enableDrawingTimeSimulation = 0;
}

MystGraphics::~MystGraphics() {
	delete _bmpDecoder;

	_backBuffer->free();
	delete _backBuffer;
}

MohawkSurface *MystGraphics::decodeImage(uint16 id) {
	// We need to grab the image from the current stack archive, however, we  don't know
	// if it's a PICT or WDIB resource. If it's Myst ME it's most likely a PICT, and if it's
	// original it's definitely a WDIB. However, Myst ME throws us another curve ball in
	// that PICT resources can contain WDIB's instead of PICT's.
	Common::SeekableReadStream *dataStream = NULL;

	if (_vm->getFeatures() & GF_ME && _vm->hasResource(ID_PICT, id)) {
		// The PICT resource exists. However, it could still contain a MystBitmap
		// instead of a PICT image...
		dataStream = _vm->getResource(ID_PICT, id);
	} else {
		// No PICT, so the WDIB must exist. Let's go grab it.
		dataStream = _vm->getResource(ID_WDIB, id);
	}

	bool isPict = false;

	if (_vm->getFeatures() & GF_ME) {
		// Here we detect whether it's really a PICT or a WDIB. Since a MystBitmap
		// would be compressed, there's no way to detect for the BM without a hack.
		// So, we search for the PICT version opcode for detection.
		dataStream->seek(512 + 10); // 512 byte pict header
		isPict = (dataStream->readUint32BE() == 0x001102FF);
		dataStream->seek(0);
	}

	MohawkSurface *mhkSurface = 0;

	if (isPict) {
		Graphics::PICTDecoder pict;

		if (!pict.loadStream(*dataStream))
			error("Could not decode Myst ME PICT");

		mhkSurface = new MohawkSurface(pict.getSurface()->convertTo(_pixelFormat));
	} else {
		mhkSurface = _bmpDecoder->decodeImage(dataStream);
		mhkSurface->convertToTrueColor();
	}

	assert(mhkSurface);
	return mhkSurface;
}

void MystGraphics::copyImageSectionToScreen(uint16 image, Common::Rect src, Common::Rect dest) {
	Graphics::Surface *surface = findImage(image)->getSurface();

	// Make sure the image is bottom aligned in the dest rect
	dest.top = dest.bottom - MIN<int>(surface->h, dest.height());

	// Convert from bitmap coordinates to surface coordinates
	uint16 top = surface->h - (src.top + MIN<int>(surface->h, dest.height()));

	// Do not draw the top pixels if the image is too tall
	if (dest.height() > _viewport.height())
		top += dest.height() - _viewport.height();

	// Clip the destination rect to the screen
	if (dest.right > _vm->_system->getWidth() || dest.bottom > _vm->_system->getHeight())
		dest.debugPrint(4, "Clipping destination rect to the screen");
	dest.right = CLIP<int>(dest.right, 0, _vm->_system->getWidth());
	dest.bottom = CLIP<int>(dest.bottom, 0, _vm->_system->getHeight());

	uint16 width = MIN<int>(surface->w, dest.width());
	uint16 height = MIN<int>(surface->h, dest.height());

	// Clamp Width and Height to within src surface dimensions
	if (src.left + width > surface->w)
		width = surface->w - src.left;
	if (src.top + height > surface->h)
		height = surface->h - src.top;

	debug(3, "MystGraphics::copyImageSectionToScreen()");
	debug(3, "\tImage: %d", image);
	debug(3, "\tsrc.left: %d", src.left);
	debug(3, "\tsrc.top: %d", src.top);
	debug(3, "\tdest.left: %d", dest.left);
	debug(3, "\tdest.top: %d", dest.top);
	debug(3, "\twidth: %d", width);
	debug(3, "\theight: %d", height);

	simulatePreviousDrawDelay(dest);

	_vm->_system->copyRectToScreen(surface->getBasePtr(src.left, top), surface->pitch, dest.left, dest.top, width, height);
}

void MystGraphics::copyImageSectionToBackBuffer(uint16 image, Common::Rect src, Common::Rect dest) {
	Graphics::Surface *surface = findImage(image)->getSurface();

	// Make sure the image is bottom aligned in the dest rect
	dest.top = dest.bottom - MIN<int>(surface->h, dest.height());

	// Convert from bitmap coordinates to surface coordinates
	uint16 top = surface->h - (src.top + MIN<int>(surface->h, dest.height()));

	// Do not draw the top pixels if the image is too tall
	if (dest.height() > _viewport.height()) {
		top += dest.height() - _viewport.height();
	}

	// Clip the destination rect to the screen
	if (dest.right > _vm->_system->getWidth() || dest.bottom > _vm->_system->getHeight())
		dest.debugPrint(4, "Clipping destination rect to the screen");
	dest.right = CLIP<int>(dest.right, 0, _vm->_system->getWidth());
	dest.bottom = CLIP<int>(dest.bottom, 0, _vm->_system->getHeight());

	uint16 width = MIN<int>(surface->w, dest.width());
	uint16 height = MIN<int>(surface->h, dest.height());

	// Clamp Width and Height to within src surface dimensions
	if (src.left + width > surface->w)
		width = surface->w - src.left;
	if (src.top + height > surface->h)
		height = surface->h - src.top;

	debug(3, "MystGraphics::copyImageSectionToBackBuffer()");
	debug(3, "\tImage: %d", image);
	debug(3, "\tsrc.left: %d", src.left);
	debug(3, "\tsrc.top: %d", src.top);
	debug(3, "\tdest.left: %d", dest.left);
	debug(3, "\tdest.top: %d", dest.top);
	debug(3, "\twidth: %d", width);
	debug(3, "\theight: %d", height);

	for (uint16 i = 0; i < height; i++)
		memcpy(_backBuffer->getBasePtr(dest.left, i + dest.top), surface->getBasePtr(src.left, top + i), width * surface->format.bytesPerPixel);
}

void MystGraphics::copyImageToScreen(uint16 image, Common::Rect dest) {
	copyImageSectionToScreen(image, Common::Rect(544, 333), dest);
}

void MystGraphics::copyImageToBackBuffer(uint16 image, Common::Rect dest) {
	copyImageSectionToBackBuffer(image, Common::Rect(544, 333), dest);
}

void MystGraphics::copyBackBufferToScreen(Common::Rect r) {
	r.clip(_viewport);

	simulatePreviousDrawDelay(r);

	_vm->_system->copyRectToScreen(_backBuffer->getBasePtr(r.left, r.top), _backBuffer->pitch, r.left, r.top, r.width(), r.height());
}

void MystGraphics::runTransition(uint16 type, Common::Rect rect, uint16 steps, uint16 delay) {

	// Do not artificially delay during transitions
	int oldEnableDrawingTimeSimulation = _enableDrawingTimeSimulation;
	_enableDrawingTimeSimulation = 0;

	switch (type) {
	case 0:	{
			debugC(kDebugScript, "Left to Right");

			uint16 step = (rect.right - rect.left) / steps;
			Common::Rect area = rect;
			for (uint i = 0; i < steps; i++) {
				area.left = rect.left + step * i;
				area.right = area.left + step;

				_vm->_system->delayMillis(delay);

				copyBackBufferToScreen(area);
				_vm->_system->updateScreen();
			}
			if (area.right < rect.right) {
				area.left = area.right;
				area.right = rect.right;

				copyBackBufferToScreen(area);
				_vm->_system->updateScreen();
			}
		}
		break;
	case 1:	{
			debugC(kDebugScript, "Right to Left");

			uint16 step = (rect.right - rect.left) / steps;
			Common::Rect area = rect;
			for (uint i = 0; i < steps; i++) {
				area.right = rect.right - step * i;
				area.left = area.right - step;

				_vm->_system->delayMillis(delay);

				copyBackBufferToScreen(area);
				_vm->_system->updateScreen();
			}
			if (area.left > rect.left) {
				area.right = area.left;
				area.left = rect.left;

				copyBackBufferToScreen(area);
				_vm->_system->updateScreen();
			}
		}
		break;
	case 5:	{
			debugC(kDebugScript, "Top to Bottom");

			uint16 step = (rect.bottom - rect.top) / steps;
			Common::Rect area = rect;
			for (uint i = 0; i < steps; i++) {
				area.top = rect.top + step * i;
				area.bottom = area.top + step;

				_vm->_system->delayMillis(delay);

				copyBackBufferToScreen(area);
				_vm->_system->updateScreen();
			}
			if (area.bottom < rect.bottom) {
				area.top = area.bottom;
				area.bottom = rect.bottom;

				copyBackBufferToScreen(area);
				_vm->_system->updateScreen();
			}
		}
		break;
	case 6:	{
			debugC(kDebugScript, "Bottom to Top");

			uint16 step = (rect.bottom - rect.top) / steps;
			Common::Rect area = rect;
			for (uint i = 0; i < steps; i++) {
				area.bottom = rect.bottom - step * i;
				area.top = area.bottom - step;

				_vm->_system->delayMillis(delay);

				copyBackBufferToScreen(area);
				_vm->_system->updateScreen();
			}
			if (area.top > rect.top) {
				area.bottom = area.top;
				area.top = rect.top;

				copyBackBufferToScreen(area);
				_vm->_system->updateScreen();
			}
		}
		break;
	default:
		warning("Unknown Update Direction");

		//TODO: Replace minimal implementation
		copyBackBufferToScreen(rect);
		_vm->_system->updateScreen();
		break;
	}

	_enableDrawingTimeSimulation = oldEnableDrawingTimeSimulation;
}

void MystGraphics::drawRect(Common::Rect rect, RectState state) {
	rect.clip(_viewport);

	// Useful with debugging. Shows where hotspots are on the screen and whether or not they're active.
	if (!rect.isValidRect() || rect.width() == 0 || rect.height() == 0)
		return;

	Graphics::Surface *screen = _vm->_system->lockScreen();

	if (state == kRectEnabled)
		screen->frameRect(rect, _pixelFormat.RGBToColor(0, 255, 0));
	else if (state == kRectUnreachable)
		screen->frameRect(rect, _pixelFormat.RGBToColor(0, 0, 255));
	else
		screen->frameRect(rect, _pixelFormat.RGBToColor(255, 0, 0));

	_vm->_system->unlockScreen();
}

void MystGraphics::drawLine(const Common::Point &p1, const Common::Point &p2, uint32 color) {
	_backBuffer->drawLine(p1.x, p1.y, p2.x, p2.y, color);
}

void MystGraphics::enableDrawingTimeSimulation(bool enable) {
	if (enable)
		_enableDrawingTimeSimulation++;
	else
		_enableDrawingTimeSimulation--;

	if (_enableDrawingTimeSimulation < 0)
		_enableDrawingTimeSimulation = 0;
}

void MystGraphics::simulatePreviousDrawDelay(const Common::Rect &dest) {
	uint32 time = 0;

	if (_enableDrawingTimeSimulation) {
		time = _vm->_system->getMillis();

		// Do not draw anything new too quickly after the previous draw call
		// so that images stay at least a little while on screen
		// This is enabled only for scripted draw calls
		if (time < _nextAllowedDrawTime)
			_vm->_system->delayMillis(_nextAllowedDrawTime - time);
	}

	// Next draw call allowed at DELAY + AERA * COEFF milliseconds from now
	time = _vm->_system->getMillis();
	_nextAllowedDrawTime = time + _constantDrawDelay + dest.height() * dest.width() / _proportionalDrawDelay;
}

void MystGraphics::copyBackBufferToScreenWithSaturation(int16 saturation) {
	Graphics::Surface *screen = _vm->_system->lockScreen();

	for (uint16 y = 0; y < _viewport.height(); y++)
		for (uint16 x = 0; x < _viewport.width(); x++) {
			uint32 color;
			uint8 r, g, b;

			if (_pixelFormat.bytesPerPixel == 2)
				color = *(const uint16 *)_backBuffer->getBasePtr(x, y);
			else
				color = *(const uint32 *)_backBuffer->getBasePtr(x, y);

			_pixelFormat.colorToRGB(color, r, g, b);

			r = CLIP<int16>((int16)r - saturation, 0, 255);
			g = CLIP<int16>((int16)g - saturation, 0, 255);
			b = CLIP<int16>((int16)b - saturation, 0, 255);

			color = _pixelFormat.RGBToColor(r, g, b);

			if (_pixelFormat.bytesPerPixel == 2) {
				uint16 *dst = (uint16 *)screen->getBasePtr(x, y);
				*dst = color;
			} else {
				uint32 *dst = (uint32 *)screen->getBasePtr(x, y);
				*dst = color;
			}
		}

	_vm->_system->unlockScreen();
	_vm->_system->updateScreen();
}

void MystGraphics::fadeToBlack() {
	for (int16 i = 0; i < 256; i += 32) {
		copyBackBufferToScreenWithSaturation(i);
	}
}

void MystGraphics::fadeFromBlack() {
	for (int16 i = 256; i >= 0; i -= 32) {
		copyBackBufferToScreenWithSaturation(i);
	}
}

} // End of namespace Mohawk
