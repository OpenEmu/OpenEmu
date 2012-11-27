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

#ifdef ENABLE_HE

#include "scumm/he/animation_he.h"
#include "scumm/he/intern_he.h"

#include "audio/audiostream.h"
#include "video/smk_decoder.h"

#ifdef USE_BINK
#include "video/bink_decoder.h"
#endif

namespace Scumm {

MoviePlayer::MoviePlayer(ScummEngine_v90he *vm, Audio::Mixer *mixer) : _vm(vm) {
#ifdef USE_BINK
	if (_vm->_game.heversion >= 100 && (_vm->_game.features & GF_16BIT_COLOR))
		_video = new Video::BinkDecoder();
	else
#endif
		_video = new Video::SmackerDecoder();

	_flags = 0;
	_wizResNum = 0;
}

MoviePlayer::~MoviePlayer() {
	delete _video;
}

int MoviePlayer::getImageNum() {
	if (!_video->isVideoLoaded())
		return 0;

	return _wizResNum;
}

int MoviePlayer::load(const char *filename, int flags, int image) {
	if (_video->isVideoLoaded())
		_video->close();

	// Ensure that Bink will use our PixelFormat
	_video->setDefaultHighColorFormat(g_system->getScreenFormat());

	if (!_video->loadFile(filename)) {
		warning("Failed to load video file %s", filename);
		return -1;
	}

	_video->start();

	debug(1, "Playing video %s", filename);

	if (flags & 2)
		_vm->_wiz->createWizEmptyImage(image, 0, 0, _video->getWidth(), _video->getHeight());

	_flags = flags;
	_wizResNum = image;
	return 0;
}

void MoviePlayer::copyFrameToBuffer(byte *dst, int dstType, uint x, uint y, uint pitch) {
	uint h = _video->getHeight();
	uint w = _video->getWidth();

	const Graphics::Surface *surface = _video->decodeNextFrame();

	if (!surface)
		return;

	byte *src = (byte *)surface->pixels;

	if (_video->hasDirtyPalette())
		_vm->setPaletteFromPtr(_video->getPalette(), 256);

	if (_vm->_game.features & GF_16BIT_COLOR) {
		if (surface->format.bytesPerPixel == 1) {
			dst += y * pitch + x * 2;
			do {
				for (uint i = 0; i < w; i++) {
					uint16 color = READ_LE_UINT16(_vm->_hePalettes + _vm->_hePaletteSlot + 768 + src[i] * 2);
					switch (dstType) {
					case kDstScreen:
						WRITE_UINT16(dst + i * 2, color);
						break;
					case kDstResource:
						WRITE_LE_UINT16(dst + i * 2, color);
						break;
					default:
						error("copyFrameToBuffer: Unknown dstType %d", dstType);
					}
				}
				dst += pitch;
				src += w;
			} while (--h);
		} else {
			dst += y * pitch + x * 2;
			do {
				for (uint i = 0; i < w; i++) {
					uint16 color = *((uint16 *)src + i);
					switch (dstType) {
					case kDstScreen:
						WRITE_UINT16(dst + i * 2, color);
						break;
					case kDstResource:
						WRITE_LE_UINT16(dst + i * 2, color);
						break;
					default:
						error("copyFrameToBuffer: Unknown dstType %d", dstType);
					}
				}
				dst += pitch;
				src += surface->pitch;
			} while (--h);
		}
	} else {
		dst += y * pitch + x;
		do {
			memcpy(dst, src, w);
			dst += pitch;
			src += w;
		} while (--h);
	}
}

void MoviePlayer::handleNextFrame() {
	if (!_video->isVideoLoaded())
		return;

	VirtScreen *pvs = &_vm->_virtscr[kMainVirtScreen];

	if (_flags & 2) {
		uint8 *dstPtr = _vm->getResourceAddress(rtImage, _wizResNum);
		assert(dstPtr);
		uint8 *dst = _vm->findWrappedBlock(MKTAG('W','I','Z','D'), dstPtr, 0, 0);
		assert(dst);
		copyFrameToBuffer(dst, kDstResource, 0, 0, _vm->_screenWidth * _vm->_bytesPerPixel);
	} else if (_flags & 1) {
		copyFrameToBuffer(pvs->getBackPixels(0, 0), kDstScreen, 0, 0, pvs->pitch);

		Common::Rect imageRect(_video->getWidth(), _video->getHeight());
		_vm->restoreBackgroundHE(imageRect);
	} else {
		copyFrameToBuffer(pvs->getPixels(0, 0), kDstScreen, 0, 0, pvs->pitch);

		Common::Rect imageRect(_video->getWidth(), _video->getHeight());
		_vm->markRectAsDirty(kMainVirtScreen, imageRect);
	}

	if (_video->endOfVideo())
		_video->close();
}

void MoviePlayer::close() {
	_video->close();
}

int MoviePlayer::getWidth() const {
	return _video->getWidth();
}

int MoviePlayer::getHeight() const {
	return _video->getHeight();
}

int MoviePlayer::getFrameCount() const {
	return _video->getFrameCount();
}

int MoviePlayer::getCurFrame() const {
	return _video->endOfVideo() ? -1 : _video->getCurFrame() + 1;
}

} // End of namespace Scumm

#endif // ENABLE_HE
