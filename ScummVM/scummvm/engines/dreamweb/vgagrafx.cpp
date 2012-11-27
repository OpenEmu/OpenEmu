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

#include "dreamweb/dreamweb.h"
#include "engines/util.h"
#include "graphics/surface.h"
#include "graphics/decoders/pcx.h"

namespace DreamWeb {

const uint16 kZoomx = 8;
const uint16 kZoomy = 132;

void DreamWebEngine::multiGet(uint8 *dst, uint16 x, uint16 y, uint8 w, uint8 h) {
	assert(x < 320);
	assert(y < 200);
	const uint8 *src = workspace() + x + y * kScreenwidth;
	if (y + h > 200)
		h = 200 - y;
	if (x + w > 320)
		w = 320 - x;
	//debug(1, "multiGet %u,%u %ux%u -> segment: %04x->%04x", x, y, w, h, (uint16)ds, (uint16)es);
	for (unsigned l = 0; l < h; ++l) {
		const uint8 *src_p = src + kScreenwidth * l;
		uint8 *dst_p = dst + w * l;
		memcpy(dst_p, src_p, w);
	}
}

void DreamWebEngine::multiPut(const uint8 *src, uint16 x, uint16 y, uint8 w, uint8 h) {
	assert(x < 320);
	assert(y < 200);
	uint8 *dst = workspace() + x + y * kScreenwidth;
	if (y + h > 200)
		h = 200 - y;
	if (x + w > 320)
		w = 320 - x;
	//debug(1, "multiPut %ux%u -> segment: %04x->%04x", w, h, (uint16)ds, (uint16)es);
	for (unsigned l = 0; l < h; ++l) {
		const uint8 *src_p = src + w * l;
		uint8 *dst_p = dst + kScreenwidth * l;
		memcpy(dst_p, src_p, w);
	}
}

void DreamWebEngine::multiDump(uint16 x, uint16 y, uint8 width, uint8 height) {
	unsigned offset = x + y * kScreenwidth;
	//debug(1, "multiDump %ux%u(segment: %04x) -> %d,%d(address: %d)", w, h, (uint16)ds, x, y, offset);
	blit(workspace() + offset, kScreenwidth, x, y, width, height);
}

void DreamWebEngine::workToScreen() {
	blit(workspace(), 320, 0, 0, 320, 200);
}

void DreamWebEngine::frameOutNm(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y) {
	dst += pitch * y + x;

	for (uint16 j = 0; j < height; ++j) {
		memcpy(dst, src, width);
		dst += pitch;
		src += width;
	}
}

void DreamWebEngine::frameOutBh(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y) {
	uint16 stride = pitch - width;
	dst += y * pitch + x;

	for (uint16 i = 0; i < height; ++i) {
		for (uint16 j = 0; j < width; ++j) {
			if (*dst == 0xff) {
				*dst = *src;
			}
			++src;
			++dst;
		}
		dst += stride;
	}
}

void DreamWebEngine::frameOutFx(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y) {
	uint16 stride = pitch - width;
	dst += y * pitch + x;
	dst -= width;

	for (uint16 j = 0; j < height; ++j) {
		for (uint16 i = 0; i < width; ++i) {
			uint8 pixel = src[width - i - 1];
			if (pixel)
				*dst = pixel;
			++dst;
		}
		src += width;
		dst += stride;
	}
}

void DreamWebEngine::doShake() {
	uint8 &counter = _vars._shakeCounter;
	if (counter == 48)
		return;

	++counter;
	static const int shakeTable[] = {
		0, -2,  3, -2,  0,  2,  4, -1,
		1, -3,  3,  2,  0, -2,  3, -2,
		0,  2,  4, -1,  1, -3,  3,  2,
		0, -2,  3, -2,  0,  2,  4, -1,

		1, -3,  3,  2,  0, -2,  3, -2,
		0,  2,  4, -1,  1, -3,  3,  2,
		0, -2,  3, -2,  0,  2,  4, -1,
		1, -3,  3,  2,  0, -2,  3, -2,

		0,  2,  4, -1,  1, -3,  3,  2,
		0, -2,  3, -2,  0,  2,  4, -1,
		1, -3,  3,  2,  0, -2,  3, -2,
		0,  2,  4, -1,  1, -3,  3,  2,

		0, -2,  3, -2,  0,  2,  4, -1,
		1, -3,  3,  0,
	};
	assert(counter < ARRAYSIZE(shakeTable));
	int offset = shakeTable[counter];
	setShakePos(offset >= 0 ? offset : -offset);
}

void DreamWebEngine::setMode() {
	waitForVSync();
	initGraphics(320, 200, false);
}

void DreamWebEngine::showPCX(const Common::String &suffix) {
	Common::String name = getDatafilePrefix() + suffix;
	Common::File pcxFile;
	if (!pcxFile.open(name)) {
		warning("showpcx: Could not open '%s'", name.c_str());
		return;
	}

	Graphics::PCXDecoder pcx;
	if (!pcx.loadStream(pcxFile)) {
		warning("showpcx: Could not process '%s'", name.c_str());
		return;
	}

	// Read the 16-color palette into the 'maingamepal' buffer. Note that
	// the color components have to be adjusted from 8 to 6 bits.
	memset(_mainPal, 0xff, 256 * 3);
	memcpy(_mainPal, pcx.getPalette(), 48);
	for (int i = 0; i < 48; i++) {
		_mainPal[i] >>= 2;
	}

	Graphics::Surface *s = g_system->lockScreen();
	s->fillRect(Common::Rect(640, 480), 0);
	const Graphics::Surface *pcxSurface = pcx.getSurface();
	if (pcxSurface->format.bytesPerPixel != 1)
		error("Invalid bytes per pixel in PCX surface (%d)", pcxSurface->format.bytesPerPixel);
	for (uint16 y = 0; y < pcxSurface->h; y++)
		memcpy((byte *)s->getBasePtr(0, y), pcxSurface->getBasePtr(0, y), pcxSurface->w);
	g_system->unlockScreen();
}

void DreamWebEngine::frameOutV(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, int16 x, int16 y) {
	// NB : These resilience checks were not in the original engine, but did they result in undefined behaviour
	// or was something broken during porting to C++?
	assert(pitch == 320);

	if (x < 0) {
		assert(width >= -x);
		width -= -x;
		src += -x;
		x = 0;
	}
	if (y < 0) {
		assert(height >= -y);
		height -= -y;
		src += (-y) * width;
		y = 0;
	}
	if (x >= 320)
		return;
	if (y >= 200)
		return;
	if (x + width > 320) {
		width = 320 - x;
	}
	if (y + height > 200) {
		height = 200 - y;
	}

	uint16 stride = pitch - width;
	dst += pitch * y + x;

	for (uint16 j = 0; j < height; ++j) {
		for (uint16 i = 0; i < width; ++i) {
			uint8 pixel = *src++;
			if (pixel)
				*dst = pixel;
			++dst;
		}
		dst += stride;
	}
}

void DreamWebEngine::showFrame(const GraphicsFile &frameData, uint16 x, uint16 y, uint16 frameNumber, uint8 effectsFlag) {
	uint8 width, height;
	showFrame(frameData, x, y, frameNumber, effectsFlag, &width, &height);
}

void DreamWebEngine::showFrameInternal(const uint8 *pSrc, uint16 x, uint16 y, uint8 effectsFlag, uint8 width, uint8 height) {
	if (effectsFlag) {
		if (effectsFlag & 128) { //centred
			x -= width / 2;
			y -= height / 2;
		}
		if (effectsFlag & 64) { // diffDest
			error("Unsupported DreamWebEngine::showFrame effectsFlag %d", effectsFlag);
			/*
			frameOutFx(es.ptr(0, dx * *height), pSrc, dx, *width, *height, x, y);
			return;
			*/
		}
		if (effectsFlag & 8) { // printList
			//addToPrintList(x - _mapAdX, y - _mapAdY); // NB: Commented in the original asm
		}
		if (effectsFlag & 4) { // flippedX
			frameOutFx(workspace(), pSrc, 320, width, height, x, y);
			return;
		}
		if (effectsFlag & 2) { // noMask
			frameOutNm(workspace(), pSrc, 320, width, height, x, y);
			return;
		}
		if (effectsFlag & 32) {
			frameOutBh(workspace(), pSrc, 320, width, height, x, y);
			return;
		}
	}
	// "noEffects"
	frameOutV(workspace(), pSrc, 320, width, height, x, y);
}

void DreamWebEngine::showFrame(const GraphicsFile &frameData, uint16 x, uint16 y, uint16 frameNumber, uint8 effectsFlag, uint8 *width, uint8 *height) {
	const Frame *frame = &frameData._frames[frameNumber];
	if ((frame->width == 0) && (frame->height == 0)) {
		*width = 0;
		*height = 0;
		return;
	}

	// "notBlankShow"
	if ((effectsFlag & 128) == 0) {
		x += frame->x;
		y += frame->y;
	}

	// "skipOffsets"
	*width = frame->width;
	*height = frame->height;
	const uint8 *pSrc = frameData.getFrameData(frameNumber);

	showFrameInternal(pSrc, x, y, effectsFlag, *width, *height);
}

void DreamWebEngine::clearWork() {
	memset(workspace(), 0, 320*200);
}

void DreamWebEngine::dumpZoom() {
	if (_vars._zoomOn == 1)
		multiDump(kZoomx + 5, kZoomy + 4, 46, 40);
}

void DreamWebEngine::crosshair() {
	uint8 frame;
	if ((_commandType != 3) && (_commandType < 10)) {
		frame = 9;
	} else {
		frame = 29;
	}
	showFrame(_icons1, kZoomx + 24, kZoomy + 19, frame, 0);
}

void DreamWebEngine::getUnderZoom() {
	multiGet(_zoomSpace, kZoomx + 5, kZoomy + 4, 46, 40);
}

void DreamWebEngine::putUnderZoom() {
	multiPut(_zoomSpace, kZoomx + 5, kZoomy + 4, 46, 40);
}

void DreamWebEngine::zoomIcon() {
	if (_vars._zoomOn == 0)
		return;
	showFrame(_icons1, kZoomx, kZoomy-1, 8, 0);
}

void DreamWebEngine::zoom() {
	if (_vars._watchingTime != 0)
		return;
	if (_vars._zoomOn != 1)
		return;
	if (_commandType >= 199) {
		putUnderZoom();
		return;
	}
	uint16 srcOffset = (_oldPointerY - 9) * 320 + (_oldPointerX - 11);
	uint16 dstOffset = (kZoomy + 4) * 320 + (kZoomx + 5);
	const uint8 *src = workspace() + srcOffset;
	uint8 *dst = workspace() + dstOffset;
	for (size_t i = 0; i < 20; ++i) {
		for (size_t j = 0; j < 23; ++j) {
			uint8 v = src[j];
			dst[2*j+0] = v;
			dst[2*j+1] = v;
			dst[2*j+320] = v;
			dst[2*j+321] = v;
		}
		src += 320;
		dst += 320*2;
	}
	crosshair();
	_didZoom = 1;
}

void DreamWebEngine::panelToMap() {
	multiGet(_mapStore, _mapXStart + _mapAdX, _mapYStart + _mapAdY, _mapXSize, _mapYSize);
}

void DreamWebEngine::mapToPanel() {
	multiPut(_mapStore, _mapXStart + _mapAdX, _mapYStart + _mapAdY, _mapXSize, _mapYSize);
}

void DreamWebEngine::dumpMap() {
	multiDump(_mapXStart + _mapAdX, _mapYStart + _mapAdY, _mapXSize, _mapYSize);
}

bool DreamWebEngine::pixelCheckSet(const ObjPos *pos, uint8 x, uint8 y) {
	x -= pos->xMin;
	y -= pos->yMin;
	SetObject *setObject = getSetAd(pos->index);
	const Frame &frame = _setFrames._frames[setObject->index];
	const uint8 *ptr = _setFrames.getFrameData(setObject->index) + y * frame.width + x;
	return *ptr != 0;
}

void DreamWebEngine::loadPalFromIFF() {
	Common::File palFile;
	uint8* buf = new uint8[2000];
	palFile.open(getDatafilePrefix() + "PAL");
	palFile.read(buf, 2000);
	palFile.close();

	const uint8 *src = buf + 0x30;
	uint8 *dst = _mainPal;
	for (size_t i = 0; i < 256*3; ++i) {
		uint8 c = src[i] / 4;
		if (_brightPalette) {
			if (c) {
				c = c + c / 2 + c / 4;
				if (c > 63)
					c = 63;
			}
		}
		dst[i] = c;
	}

	delete[] buf;
}

void DreamWebEngine::createPanel() {
	showFrame(_icons2, 0, 8, 0, 2);
	showFrame(_icons2, 160, 8, 0, 2);
	showFrame(_icons2, 0, 104, 0, 2);
	showFrame(_icons2, 160, 104, 0, 2);
}

void DreamWebEngine::createPanel2() {
	createPanel();
	showFrame(_icons2, 0, 0, 5, 2);
	showFrame(_icons2, 160, 0, 5, 2);
}

void DreamWebEngine::showPanel() {
	showFrame(_icons1, 72, 0, 19, 0);
	showFrame(_icons1, 192, 0, 19, 0);
}

void DreamWebEngine::transferFrame(uint8 from, uint8 to, uint8 offset) {
	const Frame &freeFrame = _freeFrames._frames[3*from + offset];
	Frame &exFrame = _exFrames._frames[3*to + offset];

	exFrame.width = freeFrame.width;
	exFrame.height = freeFrame.height;
	exFrame.x = freeFrame.x;
	exFrame.y = freeFrame.y;
	uint16 byteCount = freeFrame.width * freeFrame.height;

	const uint8 *src = _freeFrames.getFrameData(3*from + offset);
	uint8 *dst = _exFrames._data + _vars._exFramePos;
	memcpy(dst, src, byteCount);

	exFrame.setPtr(_vars._exFramePos);
	_vars._exFramePos += byteCount;
}

} // End of namespace DreamWeb
