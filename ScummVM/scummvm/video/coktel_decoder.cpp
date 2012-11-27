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

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/types.h"
#include "common/util.h"

#include "video/coktel_decoder.h"

#include "video/codecs/codec.h"
#include "video/codecs/indeo3.h"

#ifdef VIDEO_COKTELDECODER_H

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/adpcm_intern.h"
#include "common/memstream.h"

static const uint32 kVideoCodecIndeo3 = MKTAG('i','v','3','2');

namespace Video {

CoktelDecoder::State::State() : flags(0), speechId(0) {
}


CoktelDecoder::CoktelDecoder(Audio::Mixer *mixer, Audio::Mixer::SoundType soundType) :
	_mixer(mixer), _soundType(soundType), _width(0), _height(0), _x(0), _y(0),
	_defaultX(0), _defaultY(0), _features(0), _frameCount(0), _paletteDirty(false),
	_ownSurface(true), _frameRate(12), _hasSound(false), _soundEnabled(false),
	_soundStage(kSoundNone), _audioStream(0), _startTime(0), _pauseStartTime(0),
	_isPaused(false) {

	assert(_mixer);

	memset(_palette, 0, 768);
}

CoktelDecoder::~CoktelDecoder() {
}

bool CoktelDecoder::evaluateSeekFrame(int32 &frame, int whence) const {
	if (!isVideoLoaded())
		// Nothing to do
		return false;

	// Find the frame to which to seek
	if      (whence == SEEK_CUR)
		frame += _curFrame;
	else if (whence == SEEK_END)
		frame = _frameCount - frame - 1;
	else if (whence == SEEK_SET)
		frame--;
	else
		return false;

	if ((frame < -1) || (frame >= ((int32) _frameCount)))
		// Out of range
		return false;

	return true;
}

void CoktelDecoder::setSurfaceMemory(void *mem, uint16 width, uint16 height, uint8 bpp) {
	freeSurface();

	if (!hasVideo())
		return;

	// Sanity checks
	assert((width > 0) && (height > 0));
	assert(bpp == getPixelFormat().bytesPerPixel);

	// Create a surface over this memory
	_surface.w      = width;
	_surface.h      = height;
	_surface.pitch  = width * bpp;
	_surface.pixels = mem;
	// TODO: Check whether it is fine to assume we want the setup PixelFormat.
	_surface.format = getPixelFormat();

	_ownSurface = false;
}

void CoktelDecoder::setSurfaceMemory() {
	freeSurface();
	createSurface();

	_ownSurface = true;
}

const Graphics::Surface *CoktelDecoder::getSurface() const {
	if (!isVideoLoaded())
		return 0;

	return &_surface;
}

bool CoktelDecoder::hasSurface() {
	return _surface.pixels != 0;
}

void CoktelDecoder::createSurface() {
	if (hasSurface())
		return;

	if (!hasVideo())
		return;

	if ((_width > 0) && (_height > 0))
		_surface.create(_width, _height, getPixelFormat());

	_ownSurface = true;
}

void CoktelDecoder::freeSurface() {
	if (!_ownSurface) {
		_surface.w      = 0;
		_surface.h      = 0;
		_surface.pitch  = 0;
		_surface.pixels = 0;
		_surface.format = Graphics::PixelFormat();
	} else
		_surface.free();

	_ownSurface = true;
}

void CoktelDecoder::setXY(uint16 x, uint16 y) {
	_x = x;
	_y = y;
}

void CoktelDecoder::setXY() {
	setXY(_defaultX, _defaultY);
}

void CoktelDecoder::setFrameRate(Common::Rational frameRate) {
	_frameRate = frameRate;
}

uint16 CoktelDecoder::getDefaultX() const {
	return _defaultX;
}

uint16 CoktelDecoder::getDefaultY() const {
	return _defaultY;
}

const Common::List<Common::Rect> &CoktelDecoder::getDirtyRects() const {
	return _dirtyRects;
}

bool CoktelDecoder::hasPalette() const {
	return (_features & kFeaturesPalette) != 0;
}

bool CoktelDecoder::hasVideo() const {
	return true;
}

bool CoktelDecoder::hasSound() const {
	return _hasSound;
}

bool CoktelDecoder::isSoundEnabled() const {
	return _soundEnabled;
}

bool CoktelDecoder::isSoundPlaying() const {
	return _audioStream && _mixer->isSoundHandleActive(_audioHandle);
}

void CoktelDecoder::enableSound() {
	if (!hasSound() || isSoundEnabled())
		return;

	// Sanity check
	if (_mixer->getOutputRate() == 0)
		return;

	// Only possible on the first frame
	if (_curFrame > -1)
		return;

	_soundEnabled = true;
}

void CoktelDecoder::disableSound() {
	if (_audioStream) {
		if ((_soundStage == kSoundPlaying) || (_soundStage == kSoundFinished)) {
			_audioStream->finish();
			_mixer->stopHandle(_audioHandle);
		}

		delete _audioStream;
	}

	_soundEnabled = false;
	_soundStage   = kSoundNone;

	_audioStream = 0;
}

void CoktelDecoder::finishSound() {
	if (!_audioStream)
		return;

	_audioStream->finish();
	_soundStage = kSoundFinished;
}

void CoktelDecoder::colorModeChanged() {
}

bool CoktelDecoder::getFrameCoords(int16 frame, int16 &x, int16 &y, int16 &width, int16 &height) {
	return false;
}

bool CoktelDecoder::hasEmbeddedFiles() const {
	return false;
}

bool CoktelDecoder::hasEmbeddedFile(const Common::String &fileName) const {
	return false;
}

Common::SeekableReadStream *CoktelDecoder::getEmbeddedFile(const Common::String &fileName) const {
	return 0;
}

int32 CoktelDecoder::getSubtitleIndex() const {
	return -1;
}

bool CoktelDecoder::isPaletted() const {
	return true;
}

int CoktelDecoder::getCurFrame() const {
	return _curFrame;
}

void CoktelDecoder::close() {
	disableSound();
	freeSurface();

	_x = 0;
	_y = 0;

	_defaultX = 0;
	_defaultY = 0;

	_features = 0;

	_curFrame   = -1;
	_frameCount =  0;

	_startTime = 0;

	_hasSound = false;

	_isPaused = false;
}

Audio::Mixer::SoundType CoktelDecoder::getSoundType() const {
	return _soundType;
}

Audio::AudioStream *CoktelDecoder::getAudioStream() const {
	return _audioStream;
}

uint16 CoktelDecoder::getWidth() const {
	return _width;
}

uint16 CoktelDecoder::getHeight() const {
	return _height;
}

uint32 CoktelDecoder::getFrameCount() const {
	return _frameCount;
}

const byte *CoktelDecoder::getPalette() {
	_paletteDirty = false;
	return _palette;
}

bool CoktelDecoder::hasDirtyPalette() const {
	return (_features & kFeaturesPalette) && _paletteDirty;
}

uint32 CoktelDecoder::deLZ77(byte *dest, const byte *src, uint32 srcSize, uint32 destSize) {
	uint32 frameLength = READ_LE_UINT32(src);
	if (frameLength > destSize) {
		warning("CoktelDecoder::deLZ77(): Uncompressed size bigger than buffer size (%d > %d)", frameLength, destSize);
		return 0;
	}

	assert(srcSize >= 4);

	uint32 realSize = frameLength;

	src     += 4;
	srcSize -= 4;

	uint16 bufPos1;
	bool mode;
	if ((READ_LE_UINT16(src) == 0x1234) && (READ_LE_UINT16(src + 2) == 0x5678)) {
		assert(srcSize >= 4);

		src     += 4;
		srcSize -= 4;

		bufPos1 = 273;
		mode    = 1; // 123Ch (cmp al, 12h)
	} else {
		bufPos1 = 4078;
		mode    = 0; // 275h (jnz +2)
	}

	byte buf[4370];
	memset(buf, 32, bufPos1);

	uint8 chunkCount    = 1;
	uint8 chunkBitField = 0;

	while (frameLength > 0) {
		chunkCount--;

		if (chunkCount == 0) {
			chunkCount    = 8;
			chunkBitField = *src++;
		}

		if (chunkBitField % 2) {
			assert(srcSize >= 1);

			chunkBitField >>= 1;
			buf[bufPos1] = *src;
			*dest++ = *src++;
			bufPos1 = (bufPos1 + 1) % 4096;
			frameLength--;
			srcSize--;
			continue;
		}
		chunkBitField >>= 1;

		assert(srcSize >= 2);

		uint16 tmp = READ_LE_UINT16(src);
		uint16 chunkLength = ((tmp & 0xF00) >> 8) + 3;

		src     += 2;
		srcSize -= 2;

		if ((mode && ((chunkLength & 0xFF) == 0x12)) ||
				(!mode && (chunkLength == 0))) {
			assert(srcSize >= 1);

			chunkLength = *src++ + 0x12;
			srcSize--;
		}

		uint16 bufPos2 = (tmp & 0xFF) + ((tmp >> 4) & 0x0F00);
		if (((tmp + chunkLength) >= 4096) ||
				((chunkLength + bufPos1) >= 4096)) {

			for (int i = 0; i < chunkLength; i++, dest++) {
				*dest = buf[bufPos2];
				buf[bufPos1] = buf[bufPos2];
				bufPos1 = (bufPos1 + 1) % 4096;
				bufPos2 = (bufPos2 + 1) % 4096;
			}

		} else if (((tmp + chunkLength) < bufPos1) ||
				((chunkLength + bufPos1) < bufPos2)) {

			memcpy(dest, buf + bufPos2, chunkLength);
			memmove(buf + bufPos1, buf + bufPos2, chunkLength);

			dest    += chunkLength;
			bufPos1 += chunkLength;
			bufPos2 += chunkLength;

		} else {

			for (int i = 0; i < chunkLength; i++, dest++, bufPos1++, bufPos2++) {
				*dest = buf[bufPos2];
				buf[bufPos1] = buf[bufPos2];
			}

		}
		frameLength -= chunkLength;

	}

	return realSize;
}

void CoktelDecoder::deRLE(byte *&destPtr, const byte *&srcPtr, int16 destLen, int16 srcLen) {
	srcPtr++;

	if (srcLen & 1) {
		byte data = *srcPtr++;

		if (destLen > 0) {
			*destPtr++ = data;
			destLen--;
		}
	}

	srcLen >>= 1;

	while (srcLen > 0) {
		uint8 tmp = *srcPtr++;
		if (tmp & 0x80) { // Verbatim copy
			tmp &= 0x7F;

			int16 copyCount = MAX<int16>(0, MIN<int16>(destLen, tmp * 2));

			memcpy(destPtr, srcPtr, copyCount);

			srcPtr  += tmp * 2;
			destPtr += copyCount;
			destLen -= copyCount;
		} else { // 2 bytes tmp times
			for (int i = 0; (i < tmp) && (destLen > 0); i++) {
				for (int j = 0; j < 2; j++) {
					if (destLen <= 0)
						break;

					*destPtr++ = srcPtr[j];
					destLen--;
				}
			}
			srcPtr += 2;
		}
		srcLen -= tmp;
	}
}

// A whole, completely filled block
void CoktelDecoder::renderBlockWhole(Graphics::Surface &dstSurf, const byte *src, Common::Rect &rect) {
	Common::Rect srcRect = rect;

	rect.clip(dstSurf.w, dstSurf.h);

	byte *dst = (byte *)dstSurf.pixels + (rect.top * dstSurf.pitch) + rect.left * dstSurf.format.bytesPerPixel;
	for (int i = 0; i < rect.height(); i++) {
		memcpy(dst, src, rect.width() * dstSurf.format.bytesPerPixel);

		src += srcRect.width() * dstSurf.format.bytesPerPixel;
		dst += dstSurf.pitch;
	}
}

// A quarter-wide whole, completely filled block
void CoktelDecoder::renderBlockWhole4X(Graphics::Surface &dstSurf, const byte *src, Common::Rect &rect) {
	Common::Rect srcRect = rect;

	rect.clip(dstSurf.w, dstSurf.h);

	byte *dst = (byte *)dstSurf.pixels + (rect.top * dstSurf.pitch) + rect.left;
	for (int i = 0; i < rect.height(); i++) {
		      byte *dstRow = dst;
		const byte *srcRow = src;

		int16 count = rect.width();
		while (count >= 0) {
			memset(dstRow, *srcRow, MIN<int16>(count, 4));

			count  -= 4;
			dstRow += 4;
			srcRow += 1;
		}

		src += srcRect.width() / 4;
		dst += dstSurf.pitch;
	}
}

// A half-high whole, completely filled block
void CoktelDecoder::renderBlockWhole2Y(Graphics::Surface &dstSurf, const byte *src, Common::Rect &rect) {
	Common::Rect srcRect = rect;

	rect.clip(dstSurf.w, dstSurf.h);

	int16 height = rect.height();

	byte *dst = (byte *)dstSurf.pixels + (rect.top * dstSurf.pitch) + rect.left;
	while (height > 1) {
		memcpy(dst                 , src, rect.width());
		memcpy(dst + dstSurf.pitch, src, rect.width());

		height -= 2;
		src    += srcRect.width();
		dst    += 2 * dstSurf.pitch;
	}

	if (height == 1)
		memcpy(dst, src, rect.width());
}

// A sparse block
void CoktelDecoder::renderBlockSparse(Graphics::Surface &dstSurf, const byte *src, Common::Rect &rect) {
	Common::Rect srcRect = rect;

	rect.clip(dstSurf.w, dstSurf.h);

	byte *dst = (byte *)dstSurf.pixels + (rect.top * dstSurf.pitch) + rect.left;
	for (int i = 0; i < rect.height(); i++) {
		byte *dstRow = dst;
		int16 pixWritten = 0;

		while (pixWritten < srcRect.width()) {
			int16 pixCount = *src++;

			if (pixCount & 0x80) { // Data
				int16 copyCount;

				pixCount  = MIN((pixCount & 0x7F) + 1, srcRect.width() - pixWritten);
				copyCount = CLIP<int16>(rect.width() - pixWritten, 0, pixCount);
				memcpy(dstRow, src, copyCount);

				pixWritten += pixCount;
				dstRow     += pixCount;
				src        += pixCount;
			} else { // "Hole"
				pixWritten += pixCount + 1;
				dstRow     += pixCount + 1;
			}

		}

		dst += dstSurf.pitch;
	}
}

// A half-high sparse block
void CoktelDecoder::renderBlockSparse2Y(Graphics::Surface &dstSurf, const byte *src, Common::Rect &rect) {
	warning("renderBlockSparse2Y");

	Common::Rect srcRect = rect;

	rect.clip(dstSurf.w, dstSurf.h);

	byte *dst = (byte *)dstSurf.pixels + (rect.top * dstSurf.pitch) + rect.left;
	for (int i = 0; i < rect.height(); i += 2) {
		byte *dstRow = dst;
		int16 pixWritten = 0;

		while (pixWritten < srcRect.width()) {
			int16 pixCount = *src++;

			if (pixCount & 0x80) { // Data
				pixCount  = MIN((pixCount & 0x7F) + 1, srcRect.width() - pixWritten);
				memcpy(dstRow                 , src, pixCount);
				memcpy(dstRow + dstSurf.pitch, src, pixCount);

				pixWritten += pixCount;
				dstRow     += pixCount;
				src        += pixCount;
			} else { // "Hole"
				pixWritten += pixCount + 1;
				dstRow     += pixCount + 1;
			}

		}

		dst += dstSurf.pitch;
	}
}

void CoktelDecoder::renderBlockRLE(Graphics::Surface &dstSurf, const byte *src, Common::Rect &rect) {
	Common::Rect srcRect = rect;

	rect.clip(dstSurf.w, dstSurf.h);

	byte *dst = (byte *)dstSurf.pixels + (rect.top * dstSurf.pitch) + rect.left;
	for (int i = 0; i < rect.height(); i++) {
		byte *dstRow = dst;
		int16 pixWritten = 0;

		while (pixWritten < srcRect.width()) {
			int16 pixCount = *src++;

			if (pixCount & 0x80) {
				int16 copyCount;

				pixCount  = MIN((pixCount & 0x7F) + 1, srcRect.width() - pixWritten);
				copyCount = CLIP<int16>(rect.width() - pixWritten, 0, pixCount);

				if (*src != 0xFF) { // Normal copy

					memcpy(dstRow, src, copyCount);
					dstRow += copyCount;
					src    += pixCount;
				} else
					deRLE(dstRow, src, copyCount, pixCount);

				pixWritten += pixCount;
			} else { // "Hole"
				int16 copyCount = CLIP<int16>(rect.width() - pixWritten, 0, pixCount + 1);

				dstRow     += copyCount;
				pixWritten += pixCount + 1;
			}

		}

		dst += dstSurf.pitch;
	}
}

Common::Rational CoktelDecoder::getFrameRate() const {
	return _frameRate;
}

uint32 CoktelDecoder::getTimeToNextFrame() const {
	if (endOfVideo() || _curFrame < 0)
		return 0;

	uint32 elapsedTime        = g_system->getMillis() - _startTime;
	uint32 nextFrameStartTime = (Common::Rational((_curFrame + 1) * 1000) / getFrameRate()).toInt();

	if (nextFrameStartTime <= elapsedTime)
		return 0;

	return nextFrameStartTime - elapsedTime;
}

uint32 CoktelDecoder::getStaticTimeToNextFrame() const {
	return (1000 / _frameRate).toInt();
}

void CoktelDecoder::pauseVideo(bool pause) {
	if (_isPaused != pause) {
		if (_isPaused) {
			// Add the time we were paused to the initial starting time
			_startTime += g_system->getMillis() - _pauseStartTime;
		} else {
			// Store the time we paused for use later
			_pauseStartTime = g_system->getMillis();
		}

		_isPaused = pause;
	}
}

inline void CoktelDecoder::unsignedToSigned(byte *buffer, int length) {
	while (length-- > 0) *buffer++ ^= 0x80;
}

bool CoktelDecoder::endOfVideo() const {
	return !isVideoLoaded() || (getCurFrame() >= (int32)getFrameCount() - 1);
}


PreIMDDecoder::PreIMDDecoder(uint16 width, uint16 height,
	Audio::Mixer *mixer, Audio::Mixer::SoundType soundType) : CoktelDecoder(mixer, soundType),
	_stream(0), _videoBuffer(0), _videoBufferSize(0) {

	_width  = width;
	_height = height;
}

PreIMDDecoder::~PreIMDDecoder() {
	close();
}

bool PreIMDDecoder::reloadStream(Common::SeekableReadStream *stream) {
	if (!_stream)
		return false;

	if (!stream->seek(_stream->pos())) {
		close();
		return false;
	}

	delete _stream;
	_stream = stream;

	return true;
}

bool PreIMDDecoder::seek(int32 frame, int whence, bool restart) {
	if (!evaluateSeekFrame(frame, whence))
		return false;

	if (frame == _curFrame)
		// Nothing to do
		return true;

	// Run through the frames
	_curFrame = -1;
	_stream->seek(2);
	while (_curFrame != frame) {
		uint16 frameSize = _stream->readUint16LE();

		_stream->skip(frameSize + 2);

		_curFrame++;
	}

	return true;
}

bool PreIMDDecoder::loadStream(Common::SeekableReadStream *stream) {
	// Since PreIMDs don't have any width and height values stored,
	// we need them to be specified in the constructor
	assert((_width > 0) && (_height > 0));

	close();

	_stream = stream;

	_stream->seek(0);

	_frameCount = _stream->readUint16LE();

	_videoBufferSize = _width * _height;
	_videoBuffer     = new byte[_videoBufferSize];

	memset(_videoBuffer, 0, _videoBufferSize);

	return true;
}

void PreIMDDecoder::close() {
	CoktelDecoder::close();

	delete _stream;

	delete[] _videoBuffer;

	_stream = 0;

	_videoBuffer     = 0;
	_videoBufferSize = 0;
}

bool PreIMDDecoder::isVideoLoaded() const {
	return _stream != 0;
}

const Graphics::Surface *PreIMDDecoder::decodeNextFrame() {
	if (!isVideoLoaded() || endOfVideo())
		return 0;

	createSurface();

	processFrame();
	renderFrame();

	if (_curFrame == 0)
		_startTime = g_system->getMillis();

	return &_surface;
}

void PreIMDDecoder::processFrame() {
	_curFrame++;

	uint16 frameSize = _stream->readUint16LE();
	if (_stream->eos() || (frameSize == 0))
		return;

	uint32 nextFramePos = _stream->pos() + frameSize + 2;

	byte cmd;

	cmd = _stream->readByte();
	frameSize--;

	if (cmd == 0) {
		// Palette. Ignored by Fascination, though.

		// NOTE: If we ever find another game using this format,
		//       palettes may need to be evaluated.

		_stream->skip(768);

		frameSize -= 769;

		cmd = _stream->readByte();
	}

	if (cmd != 2) {
		// Partial frame data

		uint32 fSize   = frameSize;
		uint32 vidSize = _videoBufferSize;

		byte *vidBuffer = _videoBuffer;

		while ((fSize > 0) && (vidSize > 0)) {
			uint32 n = _stream->readByte();
			fSize--;

			if ((n & 0x80) != 0) {
				// Data

				n = MIN<uint32>((n & 0x7F) + 1, MIN(fSize, vidSize));

				_stream->read(vidBuffer, n);

				vidBuffer += n;
				vidSize   -= n;
				fSize     -= n;

			} else {
				// Skip

				n = MIN<uint32>(n + 1, vidSize);

				vidBuffer += n;
				vidSize   -= n;
			}
		}

	} else {
		// Full direct frame

		uint32 vidSize = MIN<uint32>(_videoBufferSize, frameSize);

		_stream->read(_videoBuffer, vidSize);
	}

	_stream->seek(nextFramePos);
}

// Just a simple blit
void PreIMDDecoder::renderFrame() {
	_dirtyRects.clear();

	uint16 w = CLIP<int32>(_surface.w - _x, 0, _width);
	uint16 h = CLIP<int32>(_surface.h - _y, 0, _height);

	const byte *src = _videoBuffer;
	      byte *dst = (byte *)_surface.pixels + (_y * _surface.pitch) + _x;

	uint32 frameDataSize = _videoBufferSize;

	while (h-- > 0) {
		uint32 n = MIN<uint32>(w, frameDataSize);

		memcpy(dst, src, n);

		src += _width;
		dst += _surface.pitch;

		frameDataSize -= n;
	}

	_dirtyRects.push_back(Common::Rect(_x, _y, _x + _width, _y + _height));
}

Graphics::PixelFormat PreIMDDecoder::getPixelFormat() const {
	return Graphics::PixelFormat::createFormatCLUT8();
}


IMDDecoder::IMDDecoder(Audio::Mixer *mixer, Audio::Mixer::SoundType soundType) : CoktelDecoder(mixer, soundType),
	_stream(0), _version(0), _stdX(-1), _stdY(-1), _stdWidth(-1), _stdHeight(-1),
	_flags(0), _firstFramePos(0), _framePos(0), _frameCoords(0), _videoBufferSize(0),
	_soundFlags(0), _soundFreq(0), _soundSliceSize(0), _soundSlicesCount(0) {

	_videoBuffer   [0] = 0;
	_videoBuffer   [1] = 0;
	_videoBufferLen[0] = 0;
	_videoBufferLen[1] = 0;
}

IMDDecoder::~IMDDecoder() {
	close();
}

bool IMDDecoder::reloadStream(Common::SeekableReadStream *stream) {
	if (!_stream)
		return false;

	if (!stream->seek(_stream->pos())) {
		close();
		return false;
	}

	delete _stream;
	_stream = stream;

	return true;
}

bool IMDDecoder::seek(int32 frame, int whence, bool restart) {
	if (!evaluateSeekFrame(frame, whence))
		return false;

	if (frame == _curFrame)
		// Nothing to do
		return true;

	// Try every possible way to find a file offset to that frame
	uint32 framePos = 0;
	if (frame == -1) {
		// First frame, we know that position

		framePos = _firstFramePos;

	} else if (frame == 0) {
		// Second frame, can be calculated from the first frame's position

		framePos = _firstFramePos;
		_stream->seek(framePos);
		framePos += _stream->readUint16LE() + 4;

	} else if (_framePos) {
		// If we have an array of frame positions, use that

		framePos = _framePos[frame + 1];

	} else if (restart && (_soundStage == kSoundNone)) {
		// If we are asked to restart the video if necessary and have no
		// audio to worry about, restart the video and run through the frames

		_curFrame = 0;
		_stream->seek(_firstFramePos);

		for (int i = ((frame > _curFrame) ? _curFrame : 0); i <= frame; i++)
			processFrame();

		return true;

	} else {
		// Not possible

		warning("IMDDecoder::seek(): Frame %d is not directly accessible", frame + 1);
		return false;
	}

	// Seek
	_stream->seek(framePos);
	_curFrame = frame;

	return true;
}

void IMDDecoder::setXY(uint16 x, uint16 y) {
	// Adjusting the standard coordinates
	if (_stdX != -1) {
		if (x != 0xFFFF)
			_stdX = _stdX - _x + x;
		if (y != 0xFFFF)
			_stdY = _stdY - _y + y;
	}

	// Going through the coordinate table as well
	if (_frameCoords) {
		for (uint32 i = 0; i < _frameCount; i++) {
			if (_frameCoords[i].left != -1) {
				if (x != 0xFFFF) {
					_frameCoords[i].left  = _frameCoords[i].left  - _x + x;
					_frameCoords[i].right = _frameCoords[i].right - _x + x;
				}
				if (y != 0xFFFF) {
					_frameCoords[i].top    = _frameCoords[i].top    - _y + y;
					_frameCoords[i].bottom = _frameCoords[i].bottom - _y + y;
				}
			}
		}
	}

	if (x != 0xFFFF)
		_x = x;
	if (y != 0xFFFF)
		_y = y;
}

bool IMDDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	_stream = stream;

	uint16 handle;

	handle   = _stream->readUint16LE();
	_version = _stream->readByte();

	// Version checking
	if ((handle != 0) || (_version < 2)) {
		warning("IMDDecoder::loadStream(): Version incorrect (%d, 0x%X)", handle, _version);
		close();
		return false;
	}

	// Rest header
	_features      = _stream->readByte();
	_frameCount    = _stream->readUint16LE();
	_defaultX      = _stream->readSint16LE();
	_defaultY      = _stream->readSint16LE();
	_width         = _stream->readSint16LE();
	_height        = _stream->readSint16LE();
	_flags         = _stream->readUint16LE();
	_firstFramePos = _stream->readUint16LE();

	_x = _defaultX;
	_y = _defaultY;

	// IMDs always have video
	_features |= kFeaturesVideo;
	// IMDs always have palettes
	_features |= kFeaturesPalette;

	// Palette
	for (int i = 0; i < 768; i++)
		_palette[i] = _stream->readByte() << 2;

	_paletteDirty = true;

	if (!loadCoordinates()) {
		close();
		return false;
	}

	uint32 framePosPos, frameCoordsPos;
	if (!loadFrameTableOffsets(framePosPos, frameCoordsPos)) {
		close();
		return false;
	}

	if (!assessAudioProperties()) {
		close();
		return false;
	}

	if (!assessVideoProperties()) {
		close();
		return false;
	}

	if (!loadFrameTables(framePosPos, frameCoordsPos)) {
		close();
		return false;
	}

	// Seek to the first frame
	_stream->seek(_firstFramePos);

	return true;
}

bool IMDDecoder::loadCoordinates() {
	// Standard coordinates
	if (_version >= 3) {
		uint16 count = _stream->readUint16LE();
		if (count > 1) {
			warning("IMDDecoder::loadCoordinates(): More than one standard coordinate quad found (%d)", count);
			return false;
		}

		if (count != 0) {
			_stdX      = _stream->readSint16LE();
			_stdY      = _stream->readSint16LE();
			_stdWidth  = _stream->readSint16LE();
			_stdHeight = _stream->readSint16LE();
			_features |= kFeaturesStdCoords;
		} else
			_stdX = _stdY = _stdWidth = _stdHeight = -1;

	} else
		_stdX = _stdY = _stdWidth = _stdHeight = -1;

	return true;
}

bool IMDDecoder::loadFrameTableOffsets(uint32 &framePosPos, uint32 &frameCoordsPos) {
	framePosPos    = 0;
	frameCoordsPos = 0;

	// Frame positions
	if (_version >= 4) {
		framePosPos = _stream->readUint32LE();
		if (framePosPos != 0) {
			_framePos  = new uint32[_frameCount];
			_features |= kFeaturesFramePos;
		}
	}

	// Frame coordinates
	if (_features & kFeaturesFrameCoords)
		frameCoordsPos = _stream->readUint32LE();

	return true;
}

bool IMDDecoder::assessVideoProperties() {
	uint32 suggestedVideoBufferSize = 0;

	// Sizes of the frame data and extra video buffer
	if (_features & kFeaturesDataSize) {
		uint32 size1, size2;

		size1 = _stream->readUint16LE();
		if (size1 == 0) {
			size1 = _stream->readUint32LE();
			size2 = _stream->readUint32LE();
		} else
			size2 = _stream->readUint16LE();

		suggestedVideoBufferSize = MAX(size1, size2);
	}

	_videoBufferSize = _width * _height + 1000;

	if (suggestedVideoBufferSize > _videoBufferSize) {
		warning("Suggested video buffer size greater than what should be needed (%d, %d, %dx%d",
			suggestedVideoBufferSize, _videoBufferSize, _width, _height);

		_videoBufferSize = suggestedVideoBufferSize;
	}

	for (int i = 0; i < 2; i++) {
		_videoBuffer[i] = new byte[_videoBufferSize];
		memset(_videoBuffer[i], 0, _videoBufferSize);
	}

	return true;
}

bool IMDDecoder::assessAudioProperties() {
	if (_features & kFeaturesSound) {
		_soundFreq        = _stream->readSint16LE();
		_soundSliceSize   = _stream->readSint16LE();
		_soundSlicesCount = _stream->readSint16LE();

		if (_soundFreq < 0)
			_soundFreq = -_soundFreq;

		if (_soundSlicesCount < 0)
			_soundSlicesCount = -_soundSlicesCount - 1;

		if (_soundSlicesCount > 40) {
			warning("IMDDecoder::assessAudioProperties(): More than 40 sound slices found (%d)", _soundSlicesCount);
			return false;
		}

		_frameRate = Common::Rational(_soundFreq, _soundSliceSize);

		_hasSound     = true;
		_soundEnabled = true;
		_soundStage   = kSoundLoaded;

		_audioStream = Audio::makeQueuingAudioStream(_soundFreq, false);
	}

	return true;
}

bool IMDDecoder::loadFrameTables(uint32 framePosPos, uint32 frameCoordsPos) {
	// Positions table
	if (_framePos) {
		_stream->seek(framePosPos);
		for (uint32 i = 0; i < _frameCount; i++)
			_framePos[i] = _stream->readUint32LE();
	}

	// Coordinates table
	if (_features & kFeaturesFrameCoords) {
		_stream->seek(frameCoordsPos);
		_frameCoords = new Coord[_frameCount];
		assert(_frameCoords);
		for (uint32 i = 0; i < _frameCount; i++) {
			_frameCoords[i].left   = _stream->readSint16LE();
			_frameCoords[i].top    = _stream->readSint16LE();
			_frameCoords[i].right  = _stream->readSint16LE();
			_frameCoords[i].bottom = _stream->readSint16LE();
		}
	}

	return true;
}

void IMDDecoder::close() {
	CoktelDecoder::close();

	delete _stream;

	delete[] _framePos;
	delete[] _frameCoords;

	delete[] _videoBuffer[0];
	delete[] _videoBuffer[1];

	_stream = 0;

	_version = 0;

	_stdX      = -1;
	_stdY      = -1;
	_stdWidth  = -1;
	_stdHeight = -1;

	_flags         = 0;

	_firstFramePos = 0;
	_framePos      = 0;
	_frameCoords   = 0;

	_videoBufferSize   = 0;
	_videoBuffer   [0] = 0;
	_videoBuffer   [1] = 0;
	_videoBufferLen[0] = 0;
	_videoBufferLen[1] = 0;

	_soundFlags       = 0;
	_soundFreq        = 0;
	_soundSliceSize   = 0;
	_soundSlicesCount = 0;

	_hasSound     = false;
	_soundEnabled = false;
	_soundStage   = kSoundNone;
}

bool IMDDecoder::isVideoLoaded() const {
	return _stream != 0;
}

const Graphics::Surface *IMDDecoder::decodeNextFrame() {
	if (!isVideoLoaded() || endOfVideo())
		return 0;

	createSurface();

	processFrame();

	if (_curFrame == 0)
		_startTime = g_system->getMillis();

	return &_surface;
}

void IMDDecoder::processFrame() {
	_curFrame++;

	_dirtyRects.clear();

	uint32 cmd = 0;
	bool hasNextCmd = false;
	bool startSound = false;

	do {
		cmd = _stream->readUint16LE();

		if ((cmd & kCommandBreakMask) == kCommandBreak) {
			// Flow control

			if (cmd == kCommandBreak) {
				_stream->skip(2);
				cmd = _stream->readUint16LE();
			}

			// Break
			if (cmd == kCommandBreakSkip0) {
				continue;
			} else if (cmd == kCommandBreakSkip16) {
				cmd = _stream->readUint16LE();
				_stream->skip(cmd);
				continue;
			} else if (cmd == kCommandBreakSkip32) {
				cmd = _stream->readUint32LE();
				_stream->skip(cmd);
				continue;
			}
		}

		// Audio
		if (cmd == kCommandNextSound) {

			nextSoundSlice(hasNextCmd);
			cmd = _stream->readUint16LE();

		} else if (cmd == kCommandStartSound) {

			startSound = initialSoundSlice(hasNextCmd);
			cmd = _stream->readUint16LE();

		} else
			emptySoundSlice(hasNextCmd);

		// Set palette
		if (cmd == kCommandPalette) {
			_stream->skip(2);

			_paletteDirty = true;

			for (int i = 0; i < 768; i++)
				_palette[i] = _stream->readByte() << 2;

			cmd = _stream->readUint16LE();
		}

		hasNextCmd = false;

		if (cmd == kCommandJump) {
			// Jump to frame

			int16 frame = _stream->readSint16LE();
			if (_framePos) {
				_curFrame = frame - 1;
				_stream->seek(_framePos[frame]);

				hasNextCmd = true;
			}

		} else if (cmd == kCommandVideoData) {

			_videoBufferLen[0] = _stream->readUint32LE() + 2;
			_stream->read(_videoBuffer[0], _videoBufferLen[0]);

			Common::Rect rect = calcFrameCoords(_curFrame);

			if (renderFrame(rect))
				_dirtyRects.push_back(rect);

		} else if (cmd != 0) {

			_videoBufferLen[0] = cmd + 2;
			_stream->read(_videoBuffer[0], _videoBufferLen[0]);

			Common::Rect rect = calcFrameCoords(_curFrame);

			if (renderFrame(rect))
				_dirtyRects.push_back(rect);

		}

	} while (hasNextCmd);

	// Start the audio stream if necessary
	if (startSound && _soundEnabled) {
			_mixer->playStream(_soundType, &_audioHandle, _audioStream,
					-1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
		_soundStage = kSoundPlaying;
	}

	// End the audio stream if necessary
	if ((_curFrame >= (int32)(_frameCount - 1)) && (_soundStage == kSoundPlaying)) {
		_audioStream->finish();
		_soundStage = kSoundFinished;
	}

}

Common::Rect IMDDecoder::calcFrameCoords(uint32 frame) {
	Common::Rect rect;

	if (frame == 0) {
		// First frame is always a full "keyframe"

		rect.left   = _x;
		rect.top    = _y;
		rect.right  = _x + _width;
		rect.bottom = _y + _height;
	} else if (_frameCoords && ((_frameCoords[frame].left != -1))) {
		// We have frame coordinates for that frame

		rect.left   = _frameCoords[frame].left;
		rect.top    = _frameCoords[frame].top;
		rect.right  = _frameCoords[frame].right  + 1;
		rect.bottom = _frameCoords[frame].bottom + 1;
	} else if (_stdX != -1) {
		// We have standard coordinates

		rect.left   = _stdX;
		rect.top    = _stdY;
		rect.right  = _stdX + _stdWidth;
		rect.bottom = _stdY + _stdHeight;
	} else {
		// Otherwise, it must be a full "keyframe"

		rect.left   = _x;
		rect.top    = _y;
		rect.right  = _x + _width;
		rect.bottom = _y + _height;
	}

	return rect;
}

bool IMDDecoder::renderFrame(Common::Rect &rect) {
	if (!rect.isValidRect())
		// Invalid rendering area
		return false;

	// Clip the rendering area to the video's visible area
	rect.clip(Common::Rect(_x, _y, _x + _width, _y + _height));
	if (!rect.isValidRect() || rect.isEmpty())
		// Result is empty => nothing to do
		return false;

	byte  *dataPtr  = _videoBuffer[0];
	uint32 dataSize = _videoBufferLen[0] - 1;

	uint8 type = *dataPtr++;

	if (type & 0x10) {
		// Palette data

		// One byte index
		int index = *dataPtr++;

		int count = MIN((255 - index) * 3, 48);
		for (int i = 0; i < count; i++)
			_palette[index * 3 + i] = dataPtr[i] << 2;

		dataPtr  += 48;
		dataSize -= 49;
		type ^= 0x10;

		_paletteDirty = true;
	}

	if (type & 0x80) {
		// Frame data is compressed

		type &= 0x7F;

		if ((type == 2) && (rect.width() == _surface.w) && (_x == 0)) {
			// Directly uncompress onto the video surface
			const int offsetX = rect.left * _surface.format.bytesPerPixel;
			const int offsetY = (_y + rect.top) * _surface.pitch;
			const int offset  = offsetX + offsetY;

			if (deLZ77((byte *)_surface.pixels + offset, dataPtr, dataSize,
			           _surface.w * _surface.h * _surface.format.bytesPerPixel - offset))
				return true;
		}

		_videoBufferLen[1] = deLZ77(_videoBuffer[1], dataPtr, dataSize, _videoBufferSize);

		dataPtr  = _videoBuffer[1];
		dataSize = _videoBufferLen[1];
	}

	// Evaluate the block type
	if      (type == 0x01)
		renderBlockSparse  (_surface, dataPtr, rect);
	else if (type == 0x02)
		renderBlockWhole   (_surface, dataPtr, rect);
	else if (type == 0x42)
		renderBlockWhole4X (_surface, dataPtr, rect);
	else if ((type & 0x0F) == 0x02)
		renderBlockWhole2Y (_surface, dataPtr, rect);
	else
		renderBlockSparse2Y(_surface, dataPtr, rect);

	return true;
}

void IMDDecoder::nextSoundSlice(bool hasNextCmd) {
	if (hasNextCmd || !_soundEnabled || !_audioStream) {
		// Skip sound

		_stream->skip(_soundSliceSize);
		return;
	}

	// Read, convert, queue

	byte *soundBuf = (byte *)malloc(_soundSliceSize);

	_stream->read(soundBuf, _soundSliceSize);
	unsignedToSigned(soundBuf, _soundSliceSize);

	_audioStream->queueBuffer(soundBuf, _soundSliceSize, DisposeAfterUse::YES, 0);
}

bool IMDDecoder::initialSoundSlice(bool hasNextCmd) {
	int dataLength = _soundSliceSize * _soundSlicesCount;

	if (hasNextCmd || !_soundEnabled) {
		// Skip sound

		_stream->skip(dataLength);
		return false;
	}

	if (!_audioStream || (_soundStage == kSoundFinished)) {
		delete _audioStream;

		_audioStream = Audio::makeQueuingAudioStream(_soundFreq, false);
		_soundStage  = kSoundLoaded;
	}

	// Read, convert, queue

	byte *soundBuf = (byte *)malloc(dataLength);

	_stream->read(soundBuf, dataLength);
	unsignedToSigned(soundBuf, dataLength);

	_audioStream->queueBuffer(soundBuf, dataLength, DisposeAfterUse::YES, 0);

	return _soundStage == kSoundLoaded;
}

void IMDDecoder::emptySoundSlice(bool hasNextCmd) {
	if (hasNextCmd || !_soundEnabled || !_audioStream)
		return;

	// Create an empty sound buffer and queue it

	byte *soundBuf = (byte *)malloc(_soundSliceSize);

	memset(soundBuf, 0, _soundSliceSize);

	_audioStream->queueBuffer(soundBuf, _soundSliceSize, DisposeAfterUse::YES, 0);
}

Graphics::PixelFormat IMDDecoder::getPixelFormat() const {
	return Graphics::PixelFormat::createFormatCLUT8();
}


VMDDecoder::File::File() {
	offset   = 0;
	size     = 0;
	realSize = 0;
}


VMDDecoder::Part::Part() {
	type    = kPartTypeSeparator;
	field_1 = 0;
	field_E = 0;
	size    = 0;
	left    = 0;
	top     = 0;
	right   = 0;
	bottom  = 0;
	id      = 0;
	flags   = 0;
}


VMDDecoder::Frame::Frame() {
	parts  = 0;
	offset = 0;
}

VMDDecoder::Frame::~Frame() {
	delete[] parts;
}

VMDDecoder::VMDDecoder(Audio::Mixer *mixer, Audio::Mixer::SoundType soundType) : CoktelDecoder(mixer, soundType),
	_stream(0), _version(0), _flags(0), _frameInfoOffset(0), _partsPerFrame(0), _frames(0),
	_soundFlags(0), _soundFreq(0), _soundSliceSize(0), _soundSlicesCount(0),
	_soundBytesPerSample(0), _soundStereo(0), _soundHeaderSize(0), _soundDataSize(0),
	_soundLastFilledFrame(0), _audioFormat(kAudioFormat8bitRaw),
	_hasVideo(false), _videoCodec(0), _blitMode(0), _bytesPerPixel(0),
	_firstFramePos(0), _videoBufferSize(0), _externalCodec(false), _codec(0),
	_subtitle(-1), _isPaletted(true), _autoStartSound(true) {

	_videoBuffer   [0] = 0;
	_videoBuffer   [1] = 0;
	_videoBuffer   [2] = 0;
	_videoBufferLen[0] = 0;
	_videoBufferLen[1] = 0;
	_videoBufferLen[2] = 0;
}

VMDDecoder::~VMDDecoder() {
	close();
}

bool VMDDecoder::reloadStream(Common::SeekableReadStream *stream) {
	if (!_stream)
		return false;

	if (!stream->seek(_stream->pos())) {
		close();
		return false;
	}

	delete _stream;
	_stream = stream;

	return true;
}

bool VMDDecoder::seek(int32 frame, int whence, bool restart) {
	if (!evaluateSeekFrame(frame, whence))
		return false;

	if (frame == _curFrame)
		// Nothing to do
		return true;

	// Restart sound
	if (_hasSound && (frame == -1) &&
			((_soundStage == kSoundNone) || (_soundStage == kSoundFinished))) {

		delete _audioStream;

		_soundStage  = kSoundLoaded;
		_audioStream = Audio::makeQueuingAudioStream(_soundFreq, _soundStereo != 0);
	}

	_subtitle = -1;

	if ((_blitMode > 0) && (_flags & 0x4000)) {
		if (_curFrame > frame) {
			_stream->seek(_frames[0].offset);
			_curFrame = -1;
		}

		while (frame > _curFrame)
			decodeNextFrame();

		return true;
	}

	// Seek
	_stream->seek(_frames[frame + 1].offset);
	_curFrame = frame;
	_startTime = g_system->getMillis() - ((frame + 2) * getStaticTimeToNextFrame());


	return true;
}

void VMDDecoder::setXY(uint16 x, uint16 y) {
	uint16 curX = _x;
	uint16 setX =  x;

	if ((x != 0xFFFF) && (_blitMode == 1)) {
		curX *= _bytesPerPixel;
		setX *= _bytesPerPixel;
	}

	for (uint32 i = 0; i < _frameCount; i++) {
		for (int j = 0; j < _partsPerFrame; j++) {

			if (_frames[i].parts[j].type == kPartTypeVideo) {
				if (x != 0xFFFF) {
					_frames[i].parts[j].left  = _frames[i].parts[j].left  - curX + setX;
					_frames[i].parts[j].right = _frames[i].parts[j].right - curX + setX;
				}
				if (y != 0xFFFF) {
					_frames[i].parts[j].top    = _frames[i].parts[j].top    - _y + y;
					_frames[i].parts[j].bottom = _frames[i].parts[j].bottom - _y + y;
				}
			}

		}
	}

	if (x != 0xFFFF)
		_x = x;
	if (y != 0xFFFF)
		_y = y;
}

bool VMDDecoder::openExternalCodec() {
	delete _codec;
	_codec = 0;

	if (_externalCodec) {
		if (_videoCodec == kVideoCodecIndeo3) {
			_isPaletted = false;

			_codec = new Indeo3Decoder(_width, _height);

		} else {
			warning("VMDDecoder::openExternalCodec(): Unknown video codec FourCC \"%s\"",
					tag2str(_videoCodec));
			return false;
		}
	}

	return true;
}

void VMDDecoder::colorModeChanged() {
	openExternalCodec();
}

bool VMDDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	_stream = stream;

	_stream->seek(0);

	uint16 headerLength;
	uint16 handle;

	headerLength = _stream->readUint16LE();
	handle       = _stream->readUint16LE();
	_version     = _stream->readUint16LE();

	// Version checking
	if (headerLength == 50) {
		// Newer version, used in Addy 5 upwards
		warning("VMDDecoder::loadStream(): TODO: Addy 5 videos");
	} else if (headerLength == 814) {
		// Old version
		_features |= kFeaturesPalette;
	} else {
		warning("VMDDecoder::loadStream(): Version incorrect (%d, %d, %d)", headerLength, handle, _version);
		close();
		return false;
	}

	_frameCount = _stream->readUint16LE();

	_defaultX = _stream->readSint16LE();
	_defaultY = _stream->readSint16LE();
	_width    = _stream->readSint16LE();
	_height   = _stream->readSint16LE();

	_x = _defaultX;
	_y = _defaultY;

	if ((_width != 0) && (_height != 0)) {

		_hasVideo = true;
		_features |= kFeaturesVideo;

	} else
		_hasVideo = false;

	_bytesPerPixel = 1;
	if (_version & 4)
		_bytesPerPixel = handle + 1;

	if (_bytesPerPixel > 3) {
		warning("VMDDecoder::loadStream(): Requested %d bytes per pixel (%d, %d, %d)",
				_bytesPerPixel, headerLength, handle, _version);
		close();
		return false;
	}

	_flags = _stream->readUint16LE();

	_partsPerFrame = _stream->readUint16LE();
	_firstFramePos = _stream->readUint32LE();

	_videoCodec = _stream->readUint32BE();

	if (_features & kFeaturesPalette) {
		for (int i = 0; i < 768; i++)
			_palette[i] = _stream->readByte() << 2;

		_paletteDirty = true;
	}

	uint32 videoBufferSize1 = _stream->readUint32LE();
	uint32 videoBufferSize2 = _stream->readUint32LE();

	_videoBufferSize = MAX(videoBufferSize1, videoBufferSize2);

	if (_hasVideo) {
		if (!assessVideoProperties()) {
			close();
			return false;
		}
	}

	_soundFreq        = _stream->readSint16LE();
	_soundSliceSize   = _stream->readSint16LE();
	_soundSlicesCount = _stream->readSint16LE();
	_soundFlags       = _stream->readUint16LE();

	_hasSound = (_soundFreq != 0);

	if (_hasSound) {
		if (!assessAudioProperties()) {
			close();
			return false;
		}
	} else
		_frameRate = 12;

	_frameInfoOffset = _stream->readUint32LE();

	int numFiles;
	if (!readFrameTable(numFiles)) {
		close();
		return false;
	}

	_stream->seek(_firstFramePos);

	if (numFiles == 0)
		return true;

	_files.reserve(numFiles);
	if (!readFiles()) {
		close();
		return false;
	}

	_stream->seek(_firstFramePos);
	return true;
}

bool VMDDecoder::assessVideoProperties() {
	_isPaletted = true;

	if ((_version & 2) && !(_version & 8)) {
		_externalCodec   = true;
		_videoBufferSize = 0;
	} else
		_externalCodec = false;

	if (!openExternalCodec())
		return false;

	if (_externalCodec)
		_blitMode = 0;
	else if (_bytesPerPixel == 1)
		_blitMode = 0;
	else if ((_bytesPerPixel == 2) || (_bytesPerPixel == 3)) {
		int n = (_flags & 0x80) ? 2 : 3;

		_blitMode      = _bytesPerPixel - 1;
		_bytesPerPixel = n;

		_isPaletted = false;
	}

	if (_blitMode == 1) {
		_width    /= _bytesPerPixel;
		_defaultX /= _bytesPerPixel;
		_x        /= _bytesPerPixel;
	}

	if (_hasVideo) {
		uint32 suggestedVideoBufferSize = _videoBufferSize;

		_videoBufferSize = _width * _height * _bytesPerPixel + 1000;

		if ((suggestedVideoBufferSize > _videoBufferSize) && (suggestedVideoBufferSize < 2097152)) {
			warning("Suggested video buffer size greater than what should be needed (%d, %d, %dx%d",
				suggestedVideoBufferSize, _videoBufferSize, _width, _height);

			_videoBufferSize = suggestedVideoBufferSize;
		}

		for (int i = 0; i < 3; i++) {
			_videoBuffer[i] = new byte[_videoBufferSize];
			memset(_videoBuffer[i], 0, _videoBufferSize);

			_8bppSurface[i].w      = _width * _bytesPerPixel;
			_8bppSurface[i].h      = _height;
			_8bppSurface[i].pitch  = _width * _bytesPerPixel;
			_8bppSurface[i].pixels = _videoBuffer[i];
			_8bppSurface[i].format = Graphics::PixelFormat::createFormatCLUT8();
		}
	}

	return true;
}

bool VMDDecoder::assessAudioProperties() {
	bool supportedFormat = true;

	_features |= kFeaturesSound;

	_soundStereo = (_soundFlags & 0x8000) ? 1 : ((_soundFlags & 0x200) ? 2 : 0);

	if (_soundSliceSize < 0) {
		_soundBytesPerSample = 2;
		_soundSliceSize      = -_soundSliceSize;

		if (_soundFlags & 0x10) {
			_audioFormat     = kAudioFormat16bitADPCM;
			_soundHeaderSize = 3;
			_soundDataSize   = _soundSliceSize >> 1;

			if (_soundStereo > 0)
				supportedFormat = false;

		} else {
			_audioFormat     = kAudioFormat16bitDPCM;
			_soundHeaderSize = 1;
			_soundDataSize   = _soundSliceSize;

			if (_soundStereo == 1) {
				supportedFormat = false;
			} else if (_soundStereo == 2) {
				_soundDataSize = 2 * _soundDataSize + 2;
				_soundHeaderSize = 4;
			}

		}
	} else {
		_soundBytesPerSample = 1;
		_audioFormat         = kAudioFormat8bitRaw;
		_soundHeaderSize     = 0;
		_soundDataSize       = _soundSliceSize;

		if (_soundStereo > 0)
			supportedFormat = false;
	}

	if (!supportedFormat) {
		warning("VMDDecoder::assessAudioProperties(): Unsupported audio format: %d bits, encoding %d, stereo %d",
				_soundBytesPerSample * 8, _audioFormat, _soundStereo);
		return false;
	}

	_frameRate = Common::Rational(_soundFreq, _soundSliceSize);

	_hasSound     = true;
	_soundEnabled = true;
	_soundStage   = kSoundLoaded;

	_audioStream = Audio::makeQueuingAudioStream(_soundFreq, _soundStereo != 0);

	return true;
}

bool VMDDecoder::readFrameTable(int &numFiles) {
	numFiles = 0;

	_stream->seek(_frameInfoOffset);
	_frames = new Frame[_frameCount];
	for (uint16 i = 0; i < _frameCount; i++) {
		_frames[i].parts = new Part[_partsPerFrame];
		_stream->skip(2); // Unknown
		_frames[i].offset = _stream->readUint32LE();
	}

	_soundLastFilledFrame = 0;
	for (uint16 i = 0; i < _frameCount; i++) {
		bool separator = false;

		for (uint16 j = 0; j < _partsPerFrame; j++) {

			_frames[i].parts[j].type    = (PartType) _stream->readByte();
			_frames[i].parts[j].field_1 = _stream->readByte();
			_frames[i].parts[j].size    = _stream->readUint32LE();

			if (_frames[i].parts[j].type == kPartTypeAudio) {

				_frames[i].parts[j].flags = _stream->readByte();
				_stream->skip(9); // Unknown

				if (_frames[i].parts[j].flags != 3)
					_soundLastFilledFrame = i;

			} else if (_frames[i].parts[j].type == kPartTypeVideo) {

				_frames[i].parts[j].left    = _stream->readUint16LE();
				_frames[i].parts[j].top     = _stream->readUint16LE();
				_frames[i].parts[j].right   = _stream->readUint16LE();
				_frames[i].parts[j].bottom  = _stream->readUint16LE();
				_frames[i].parts[j].field_E = _stream->readByte();
				_frames[i].parts[j].flags   = _stream->readByte();

			} else if (_frames[i].parts[j].type == kPartTypeSubtitle) {
				_frames[i].parts[j].id = _stream->readUint16LE();
				// Speech text file name
				_stream->skip(8);
			} else if (_frames[i].parts[j].type == kPartTypeFile) {
				if (!separator)
					numFiles++;
				_stream->skip(10);
			} else if (_frames[i].parts[j].type == kPartTypeSeparator) {
				separator = true;
				_stream->skip(10);
			} else {
				// Unknown type
				_stream->skip(10);
			}

		}
	}

	return true;
}

bool VMDDecoder::readFiles() {
	uint32 ssize = _stream->size();
	for (uint16 i = 0; i < _frameCount; i++) {
		_stream->seek(_frames[i].offset);

		for (uint16 j = 0; j < _partsPerFrame; j++) {
			if (_frames[i].parts[j].type == kPartTypeSeparator)
				break;

			if (_frames[i].parts[j].type == kPartTypeFile) {
				File file;

				file.offset   = _stream->pos() + 20;
				file.size     = _frames[i].parts[j].size;
				file.realSize = _stream->readUint32LE();

				char name[16];

				_stream->read(name, 16);
				name[15] = '\0';

				file.name = name;

				_stream->skip(_frames[i].parts[j].size - 20);

				if ((((uint32) file.realSize) >= ssize) || (file.name == ""))
					continue;

				_files.push_back(file);

			} else
				_stream->skip(_frames[i].parts[j].size);
		}
	}

	return true;
}

void VMDDecoder::close() {
	CoktelDecoder::close();

	delete _stream;

	delete[] _frames;

	delete[] _videoBuffer[0];
	delete[] _videoBuffer[1];
	delete[] _videoBuffer[2];

	delete _codec;

	_files.clear();


	_stream = 0;

	_version = 0;
	_flags   = 0;

	_frameInfoOffset = 0;
	_partsPerFrame   = 0;
	_frames          = 0;

	_soundFlags           = 0;
	_soundFreq            = 0;
	_soundSliceSize       = 0;
	_soundSlicesCount     = 0;
	_soundBytesPerSample  = 0;
	_soundStereo          = 0;
	_soundHeaderSize      = 0;
	_soundDataSize        = 0;
	_soundLastFilledFrame = 0;
	_audioFormat          = kAudioFormat8bitRaw;

	_hasVideo      = false;
	_videoCodec    = 0;
	_blitMode      = 0;
	_bytesPerPixel = 0;

	_firstFramePos = 0;

	_videoBufferSize   = 0;
	_videoBuffer   [0] = 0;
	_videoBuffer   [1] = 0;
	_videoBuffer   [2] = 0;
	_videoBufferLen[0] = 0;
	_videoBufferLen[1] = 0;
	_videoBufferLen[2] = 0;

	_externalCodec = false;
	_codec         = 0;

	_isPaletted = true;
}

bool VMDDecoder::isVideoLoaded() const {
	return _stream != 0;
}

const Graphics::Surface *VMDDecoder::decodeNextFrame() {
	if (!isVideoLoaded() || endOfVideo())
		return 0;

	createSurface();

	processFrame();

	if (_curFrame == 0)
		_startTime = g_system->getMillis();

	return &_surface;
}

void VMDDecoder::processFrame() {
	_curFrame++;

	_dirtyRects.clear();

	_subtitle     = -1;

	bool startSound = false;

	for (uint16 i = 0; i < _partsPerFrame; i++) {
		uint32 pos = _stream->pos();

		Part &part = _frames[_curFrame].parts[i];

		if (part.type == kPartTypeAudio) {

			if (part.flags == 1) {
				// Next sound slice data

				if (_soundEnabled) {
					filledSoundSlice(part.size);

					if (_soundStage == kSoundLoaded)
						startSound = true;

				} else
					_stream->skip(part.size);

			} else if (part.flags == 2) {
				// Initial sound data (all slices)

				if (_soundEnabled) {
					uint32 mask = _stream->readUint32LE();
					filledSoundSlices(part.size - 4, mask);

					if (_soundStage == kSoundLoaded)
						startSound = true;

				} else
					_stream->skip(part.size);

			} else if (part.flags == 3) {
				// Empty sound slice

				if (_soundEnabled) {
					if ((uint32)_curFrame < _soundLastFilledFrame)
						emptySoundSlice(_soundDataSize * _soundBytesPerSample);

					if (_soundStage == kSoundLoaded)
						startSound = true;
				}

				_stream->skip(part.size);
			} else if (part.flags == 4) {
				warning("VMDDecoder::processFrame(): TODO: Addy 5 sound type 4 (%d)", part.size);
				disableSound();
				_stream->skip(part.size);
			} else {
				warning("VMDDecoder::processFrame(): Unknown sound type %d", part.flags);
				_stream->skip(part.size);
			}

			_stream->seek(pos + part.size);

		} else if ((part.type == kPartTypeVideo) && !_hasVideo) {

			warning("VMDDecoder::processFrame(): Header claims there's no video, but video found (%d)", part.size);
			_stream->skip(part.size);

		} else if ((part.type == kPartTypeVideo) && _hasVideo) {

			uint32 size = part.size;

			// New palette
			if (part.flags & 2) {
				uint8 index = _stream->readByte();
				uint8 count = _stream->readByte();

				for (int j = 0; j < ((count + 1) * 3); j++)
					_palette[index * 3 + j] = _stream->readByte() << 2;

				_stream->skip((255 - count) * 3);

				_paletteDirty = true;

				size -= (768 + 2);
			}

			_stream->read(_videoBuffer[0], size);
			_videoBufferLen[0] = size;

			Common::Rect rect(part.left, part.top, part.right + 1, part.bottom + 1);
			if (renderFrame(rect))
				_dirtyRects.push_back(rect);

		} else if (part.type == kPartTypeSeparator) {

			// Ignore

		} else if (part.type == kPartTypeFile) {

			// Ignore
			_stream->skip(part.size);

		} else if (part.type == kPartType4) {

			// Unknown, ignore
			_stream->skip(part.size);

		} else if (part.type == kPartTypeSubtitle) {

			_subtitle = part.id;
			_stream->skip(part.size);

		} else {

			warning("VMDDecoder::processFrame(): Unknown frame part type %d, size %d (%d of %d)",
					part.type, part.size, i + 1, _partsPerFrame);

		}
	}

	if (startSound && _soundEnabled) {
		if (_hasSound && _audioStream) {
			if (_autoStartSound)
				_mixer->playStream(_soundType, &_audioHandle, _audioStream,
						-1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
			_soundStage = kSoundPlaying;
		} else
			_soundStage = kSoundNone;
	}

	if (((uint32)_curFrame == (_frameCount - 1)) && (_soundStage == 2)) {
		_audioStream->finish();
		_soundStage = kSoundFinished;
	}
}

bool VMDDecoder::renderFrame(Common::Rect &rect) {
	Common::Rect realRect, fakeRect;
	if (!getRenderRects(rect, realRect, fakeRect))
		return false;

	if (_externalCodec) {
		if (!_codec)
			return false;

		Common::MemoryReadStream frameStream(_videoBuffer[0], _videoBufferLen[0]);
		const Graphics::Surface *codecSurf = _codec->decodeImage(&frameStream);
		if (!codecSurf)
			return false;

		rect = Common::Rect(_x, _y, _x + codecSurf->w, _y + codecSurf->h);
		rect.clip(Common::Rect(_x, _y, _x + _width, _y + _height));

		renderBlockWhole(_surface, (const byte *) codecSurf->pixels, rect);
		return true;
	}

	uint8  srcBuffer = 0;
	byte  *dataPtr   = _videoBuffer[srcBuffer];
	uint32 dataSize  = _videoBufferLen[srcBuffer] - 1;

	uint8 type = *dataPtr++;

	if (type & 0x80) {
		// Frame data is compressed

		type &= 0x7F;

		if ((type == 2) && (rect.width() == _surface.w) && (_x == 0) && (_blitMode == 0)) {
			// Directly uncompress onto the video surface
			const int offsetX = rect.left * _surface.format.bytesPerPixel;
			const int offsetY = (_y + rect.top) * _surface.pitch;
			const int offset  = offsetX - offsetY;

			if (deLZ77((byte *)_surface.pixels + offset, dataPtr, dataSize,
			           _surface.w * _surface.h * _surface.format.bytesPerPixel - offset))
				return true;
		}

		srcBuffer = 1;
		_videoBufferLen[srcBuffer] =
			deLZ77(_videoBuffer[srcBuffer], dataPtr, dataSize, _videoBufferSize);

		dataPtr  = _videoBuffer[srcBuffer];
		dataSize = _videoBufferLen[srcBuffer];
	}

	Common::Rect      *blockRect = &fakeRect;
	Graphics::Surface *surface   = &_surface;
	if (_blitMode == 0) {
		*blockRect = Common::Rect(blockRect->left  + _x, blockRect->top    + _y,
		                          blockRect->right + _x, blockRect->bottom + _y);
	} else {
		surface = &_8bppSurface[2];
	}

	// Evaluate the block type
	if      (type == 0x01)
		renderBlockSparse  (*surface, dataPtr, *blockRect);
	else if (type == 0x02)
		renderBlockWhole   (*surface, dataPtr, *blockRect);
	else if (type == 0x03)
		renderBlockRLE     (*surface, dataPtr, *blockRect);
	else if (type == 0x42)
		renderBlockWhole4X (*surface, dataPtr, *blockRect);
	else if ((type & 0x0F) == 0x02)
		renderBlockWhole2Y (*surface, dataPtr, *blockRect);
	else
		renderBlockSparse2Y(*surface, dataPtr, *blockRect);

	if (_blitMode > 0) {
		if      (_bytesPerPixel == 2)
			blit16(*surface, *blockRect);
		else if (_bytesPerPixel == 3)
			blit24(*surface, *blockRect);

		*blockRect = Common::Rect(blockRect->left  + _x, blockRect->top    + _y,
		                          blockRect->right + _x, blockRect->bottom + _y);
	}

	rect = *blockRect;
	return true;
}

bool VMDDecoder::getRenderRects(const Common::Rect &rect,
		Common::Rect &realRect, Common::Rect &fakeRect) {

	realRect = rect;
	fakeRect = rect;

	if        (_blitMode == 0) {

		realRect = Common::Rect(realRect.left  - _x, realRect.top    - _y,
		                        realRect.right - _x, realRect.bottom - _y);

		fakeRect = Common::Rect(fakeRect.left  - _x, fakeRect.top    - _y,
		                        fakeRect.right - _x, fakeRect.bottom - _y);

	} else if (_blitMode == 1) {

		realRect = Common::Rect(rect.left  / _bytesPerPixel, rect.top,
		                        rect.right / _bytesPerPixel, rect.bottom);

		realRect = Common::Rect(realRect.left  - _x, realRect.top    - _y,
		                        realRect.right - _x, realRect.bottom - _y);

		fakeRect = Common::Rect(fakeRect.left  - _x * _bytesPerPixel, fakeRect.top    - _y,
		                        fakeRect.right - _x * _bytesPerPixel, fakeRect.bottom - _y);

	} else if (_blitMode == 2) {

		fakeRect = Common::Rect(rect.left  * _bytesPerPixel, rect.top,
		                        rect.right * _bytesPerPixel, rect.bottom);

		realRect = Common::Rect(realRect.left  - _x, realRect.top    - _y,
		                        realRect.right - _x, realRect.bottom - _y);

		fakeRect = Common::Rect(fakeRect.left  - _x * _bytesPerPixel, fakeRect.top    - _y,
		                        fakeRect.right - _x * _bytesPerPixel, fakeRect.bottom - _y);

	}

	realRect.clip(Common::Rect(_surface.w, _surface.h));
	fakeRect.clip(Common::Rect(_surface.w * _bytesPerPixel, _surface.h));

	if (!realRect.isValidRect() || realRect.isEmpty())
		return false;
	if (!fakeRect.isValidRect() || realRect.isEmpty())
		return false;

	return true;
}

void VMDDecoder::blit16(const Graphics::Surface &srcSurf, Common::Rect &rect) {
	rect = Common::Rect(rect.left / 2, rect.top, rect.right / 2, rect.bottom);

	Common::Rect srcRect = rect;

	rect.clip(_surface.w, _surface.h);

	Graphics::PixelFormat pixelFormat = getPixelFormat();

	const byte *src = (byte *)srcSurf.pixels +
		(srcRect.top * srcSurf.pitch) + srcRect.left * _bytesPerPixel;
	byte *dst = (byte *)_surface.pixels +
		((_y + rect.top) * _surface.pitch) + (_x + rect.left) * _surface.format.bytesPerPixel;

	for (int i = 0; i < rect.height(); i++) {
		const byte *srcRow = src;
		      byte *dstRow = dst;

		for (int j = 0; j < rect.width(); j++, srcRow += 2, dstRow += _surface.format.bytesPerPixel) {
			uint16 data = READ_LE_UINT16(srcRow);

			byte r = ((data & 0x7C00) >> 10) << 3;
			byte g = ((data & 0x03E0) >>  5) << 3;
			byte b = ((data & 0x001F) >>  0) << 3;

			uint32 c = pixelFormat.RGBToColor(r, g, b);
			if ((r == 0) && (g == 0) && (b == 0))
				c = 0;

			if (_surface.format.bytesPerPixel == 2)
				*((uint16 *)dstRow) = (uint16) c;
		}

		src += srcSurf .pitch;
		dst += _surface.pitch;
	}
}

void VMDDecoder::blit24(const Graphics::Surface &srcSurf, Common::Rect &rect) {
	rect = Common::Rect(rect.left / 3, rect.top, rect.right / 3, rect.bottom);

	Common::Rect srcRect = rect;

	rect.clip(_surface.w, _surface.h);

	Graphics::PixelFormat pixelFormat = getPixelFormat();

	const byte *src = (byte *)srcSurf.pixels +
		(srcRect.top * srcSurf.pitch) + srcRect.left * _bytesPerPixel;
	byte *dst = (byte *)_surface.pixels +
		((_y + rect.top) * _surface.pitch) + (_x + rect.left) * _surface.format.bytesPerPixel;

	for (int i = 0; i < rect.height(); i++) {
		const byte *srcRow = src;
		      byte *dstRow = dst;

		for (int j = 0; j < rect.width(); j++, srcRow += 3, dstRow += _surface.format.bytesPerPixel) {
			byte r = srcRow[2];
			byte g = srcRow[1];
			byte b = srcRow[0];

			uint32 c = pixelFormat.RGBToColor(r, g, b);
			if ((r == 0) && (g == 0) && (b == 0))
				c = 0;

			if (_surface.format.bytesPerPixel == 2)
				*((uint16 *)dstRow) = (uint16) c;
		}

		src += srcSurf .pitch;
		dst += _surface.pitch;
	}
}

void VMDDecoder::emptySoundSlice(uint32 size) {
	byte *soundBuf = (byte *)malloc(size);

	if (soundBuf) {
		uint32 flags = 0;
		memset(soundBuf, 0, size);
		flags |= (_soundBytesPerSample == 2) ? Audio::FLAG_16BITS : 0;
		flags |= (_soundStereo > 0) ? Audio::FLAG_STEREO : 0;

		_audioStream->queueBuffer(soundBuf, size, DisposeAfterUse::YES, flags);
	}
}

void VMDDecoder::filledSoundSlice(uint32 size) {
	if (!_audioStream) {
		_stream->skip(size);
		return;
	}

	Common::SeekableReadStream *data = _stream->readStream(size);
	Audio::AudioStream *sliceStream = 0;

	if (_audioFormat == kAudioFormat8bitRaw)
		sliceStream = create8bitRaw(data);
	else if (_audioFormat == kAudioFormat16bitDPCM)
		sliceStream = create16bitDPCM(data);
	else if (_audioFormat == kAudioFormat16bitADPCM)
		sliceStream = create16bitADPCM(data);

	if (sliceStream)
		_audioStream->queueAudioStream(sliceStream);
}

void VMDDecoder::filledSoundSlices(uint32 size, uint32 mask) {
	bool fillInfo[32];

	uint8 max;
	uint8 n = evaluateMask(mask, fillInfo, max);

	int32 extraSize;

	extraSize = size - n * _soundDataSize;

	if (_soundSlicesCount > 32)
		extraSize -= (_soundSlicesCount - 32) * _soundDataSize;

	if (n > 0)
		extraSize /= n;

	for (uint8 i = 0; i < max; i++)
		if (fillInfo[i])
			filledSoundSlice(_soundDataSize + extraSize);
		else
			emptySoundSlice(_soundDataSize * _soundBytesPerSample);

	if (_soundSlicesCount > 32)
		filledSoundSlice((_soundSlicesCount - 32) * _soundDataSize + _soundHeaderSize);
}

uint8 VMDDecoder::evaluateMask(uint32 mask, bool *fillInfo, uint8 &max) {
	max = MIN<int>(_soundSlicesCount - 1, 31);

	uint8 n = 0;
	for (int i = 0; i < max; i++) {

		if (!(mask & 1)) {
			n++;
			*fillInfo++ = true;
		} else
			*fillInfo++ = false;

		mask >>= 1;
	}

	return n;
}

Audio::AudioStream *VMDDecoder::create8bitRaw(Common::SeekableReadStream *stream) {
	int flags = Audio::FLAG_UNSIGNED;

	if (_soundStereo != 0)
		flags |= Audio::FLAG_STEREO;

	return Audio::makeRawStream(stream, _soundFreq, flags, DisposeAfterUse::YES);
}

class DPCMStream : public Audio::AudioStream {
public:
	DPCMStream(Common::SeekableReadStream *stream, int rate, int channels) {
		_stream = stream;
		_rate = rate;
		_channels = channels;
	}

	~DPCMStream() {
		delete _stream;
	}

	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return _channels == 2; }
	int getRate() const { return _rate; }
	bool endOfData() const { return _stream->pos() >= _stream->size() || _stream->eos() || _stream->err(); }

private:
	Common::SeekableReadStream *_stream;
	int _channels;
	int _rate;
	int _buffer[2];
};

int DPCMStream::readBuffer(int16 *buffer, const int numSamples) {
	static const uint16 tableDPCM[128] = {
		0x0000, 0x0008, 0x0010, 0x0020, 0x0030, 0x0040, 0x0050, 0x0060, 0x0070, 0x0080,
		0x0090, 0x00A0, 0x00B0, 0x00C0, 0x00D0, 0x00E0, 0x00F0, 0x0100, 0x0110, 0x0120,
		0x0130, 0x0140, 0x0150, 0x0160, 0x0170, 0x0180, 0x0190, 0x01A0, 0x01B0, 0x01C0,
		0x01D0, 0x01E0, 0x01F0, 0x0200, 0x0208, 0x0210, 0x0218, 0x0220, 0x0228, 0x0230,
		0x0238, 0x0240, 0x0248, 0x0250, 0x0258, 0x0260, 0x0268, 0x0270, 0x0278, 0x0280,
		0x0288, 0x0290, 0x0298, 0x02A0, 0x02A8, 0x02B0, 0x02B8, 0x02C0, 0x02C8, 0x02D0,
		0x02D8, 0x02E0, 0x02E8, 0x02F0, 0x02F8, 0x0300, 0x0308, 0x0310, 0x0318, 0x0320,
		0x0328, 0x0330, 0x0338, 0x0340, 0x0348, 0x0350, 0x0358, 0x0360, 0x0368, 0x0370,
		0x0378, 0x0380, 0x0388, 0x0390, 0x0398, 0x03A0, 0x03A8, 0x03B0, 0x03B8, 0x03C0,
		0x03C8, 0x03D0, 0x03D8, 0x03E0, 0x03E8, 0x03F0, 0x03F8, 0x0400, 0x0440, 0x0480,
		0x04C0, 0x0500, 0x0540, 0x0580, 0x05C0, 0x0600, 0x0640, 0x0680, 0x06C0, 0x0700,
		0x0740, 0x0780, 0x07C0, 0x0800, 0x0900, 0x0A00, 0x0B00, 0x0C00, 0x0D00, 0x0E00,
		0x0F00, 0x1000, 0x1400, 0x1800, 0x1C00, 0x2000, 0x3000, 0x4000
	};

	assert((numSamples % _channels) == 0);

	int samples = 0;

	// Our starting position
	if (_stream->pos() == 0) {
		for (int i = 0; i < _channels; i++)
			*buffer++ = _buffer[i] = _stream->readSint16LE();

		samples += _channels;
	}

	while (!endOfData() && samples < numSamples) {
		for (int i = 0; i < _channels; i++) {
			byte data = _stream->readByte();

			if (data & 0x80)
				_buffer[i] -= tableDPCM[data & 0x7f];
			else
				_buffer[i] += tableDPCM[data];

			*buffer++ = _buffer[i] = CLIP<int32>(_buffer[i], -32768, 32767);
		}

		samples += _channels;
	}

	return samples;
}

Audio::AudioStream *VMDDecoder::create16bitDPCM(Common::SeekableReadStream *stream) {
	return new DPCMStream(stream, _soundFreq, (_soundStereo == 0) ? 1 : 2);
}

class VMD_ADPCMStream : public Audio::DVI_ADPCMStream {
public:
	VMD_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse,
			int rate, int channels) : Audio::DVI_ADPCMStream(stream, disposeAfterUse, stream->size(), rate, channels, 0) {
		// FIXME: Using the same predictor/index for two channels probably won't work
		// properly However, we have no samples of this, so an assert is here for now.
		// Also, since the DPCM stereo has a second predictor, I'm lead to believe
		// all VMD with ADPCM are mono unless they changed the code in a later
		// revision.
		assert(channels == 1);
		_startPredictorValue = stream->readSint16LE();
		_startIndexValue = stream->readByte();
		_startpos = 3;
		reset();
	}

protected:
	virtual void reset() {
		Audio::DVI_ADPCMStream::reset();
		_status.ima_ch[0].last = _startPredictorValue;
		_status.ima_ch[0].stepIndex = _startIndexValue;
	}

private:
	int32 _startPredictorValue;
	int32 _startIndexValue;
};

Audio::AudioStream *VMDDecoder::create16bitADPCM(Common::SeekableReadStream *stream) {
	return new VMD_ADPCMStream(stream, DisposeAfterUse::YES, _soundFreq, (_soundStereo == 0) ? 1 : 2);
}

Graphics::PixelFormat VMDDecoder::getPixelFormat() const {
	if (_externalCodec) {
		if (_codec)
			return _codec->getPixelFormat();

		// If we don't have the needed codec, just assume it's in the
		// current screen format
		return g_system->getScreenFormat();
	}

	if (_blitMode > 0)
		return g_system->getScreenFormat();

	return Graphics::PixelFormat::createFormatCLUT8();
}

bool VMDDecoder::getPartCoords(int16 frame, PartType type, int16 &x, int16 &y, int16 &width, int16 &height) {
	if (frame >= ((int32) _frameCount))
		return false;

	Frame &f = _frames[frame];

	// Look for a part matching the requested type, stopping at a separator
	Part *part = 0;
	for (int i = 0; i < _partsPerFrame; i++) {
		Part &p = f.parts[i];

		if ((p.type == kPartTypeSeparator) || (p.type == type)) {
			part = &p;
			break;
		}
	}

	if (!part)
		return false;

	x      = part->left;
	y      = part->top;
	width  = part->right  - part->left + 1;
	height = part->bottom - part->top  + 1;

	return true;
}

bool VMDDecoder::getFrameCoords(int16 frame, int16 &x, int16 &y, int16 &width, int16 &height) {
	return getPartCoords(frame, kPartTypeVideo, x, y, width, height);
}

bool VMDDecoder::hasEmbeddedFiles() const {
	return !_files.empty();
}

bool VMDDecoder::hasEmbeddedFile(const Common::String &fileName) const {
	for (Common::Array<File>::const_iterator file = _files.begin(); file != _files.end(); ++file)
		if (!file->name.compareToIgnoreCase(fileName))
			return true;

	return false;
}

Common::SeekableReadStream *VMDDecoder::getEmbeddedFile(const Common::String &fileName) const {
	const File *file = 0;

	for (Common::Array<File>::const_iterator it = _files.begin(); it != _files.end(); ++it)
		if (!it->name.compareToIgnoreCase(fileName)) {
			file = &*it;
			break;
		}

	if (!file)
		return 0;

	if ((file->size - 20) != file->realSize) {
		warning("VMDDecoder::getEmbeddedFile(): Sizes for \"%s\" differ! (%d, %d)",
				fileName.c_str(), (file->size - 20), file->realSize);
		return 0;
	}

	if (!_stream->seek(file->offset)) {
		warning("VMDDecoder::getEmbeddedFile(): Can't seek to offset %d to (file \"%s\")",
				file->offset, fileName.c_str());
		return 0;
	}

	byte *data = (byte *) malloc(file->realSize);
	if (_stream->read(data, file->realSize) != file->realSize) {
		free(data);
		warning("VMDDecoder::getEmbeddedFile(): Couldn't read %d bytes (file \"%s\")",
				file->realSize, fileName.c_str());
		return 0;
	}

	Common::MemoryReadStream *stream =
		new Common::MemoryReadStream(data, file->realSize, DisposeAfterUse::YES);

	return stream;
}

int32 VMDDecoder::getSubtitleIndex() const {
	return _subtitle;
}

bool VMDDecoder::hasVideo() const {
	return _hasVideo;
}

bool VMDDecoder::isPaletted() const {
	return _isPaletted;
}

void VMDDecoder::setAutoStartSound(bool autoStartSound) {
	_autoStartSound = autoStartSound;
}

AdvancedVMDDecoder::AdvancedVMDDecoder(Audio::Mixer::SoundType soundType) {
	_decoder = new VMDDecoder(g_system->getMixer(), soundType);
	_decoder->setAutoStartSound(false);
}

AdvancedVMDDecoder::~AdvancedVMDDecoder() {
	close();
	delete _decoder;
}

bool AdvancedVMDDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	if (!_decoder->loadStream(stream))
		return false;

	if (_decoder->hasVideo()) {
		_videoTrack = new VMDVideoTrack(_decoder);
		addTrack(_videoTrack);
	}

	if (_decoder->hasSound()) {
		_audioTrack = new VMDAudioTrack(_decoder);
		addTrack(_audioTrack);
	}

	return true;
}

void AdvancedVMDDecoder::close() {
	VideoDecoder::close();
	_decoder->close();
}

AdvancedVMDDecoder::VMDVideoTrack::VMDVideoTrack(VMDDecoder *decoder) : _decoder(decoder) {
}

uint16 AdvancedVMDDecoder::VMDVideoTrack::getWidth() const {
	return _decoder->getWidth();
}

uint16 AdvancedVMDDecoder::VMDVideoTrack::getHeight() const {
	return _decoder->getHeight();
}

Graphics::PixelFormat AdvancedVMDDecoder::VMDVideoTrack::getPixelFormat() const {
	return _decoder->getPixelFormat();
}

int AdvancedVMDDecoder::VMDVideoTrack::getCurFrame() const {
	return _decoder->getCurFrame();
}

int AdvancedVMDDecoder::VMDVideoTrack::getFrameCount() const {
	return _decoder->getFrameCount();
}

const Graphics::Surface *AdvancedVMDDecoder::VMDVideoTrack::decodeNextFrame() {
	return _decoder->decodeNextFrame();
}

const byte *AdvancedVMDDecoder::VMDVideoTrack::getPalette() const {
	return _decoder->getPalette();
}

bool AdvancedVMDDecoder::VMDVideoTrack::hasDirtyPalette() const {
	return _decoder->hasDirtyPalette();
}

Common::Rational AdvancedVMDDecoder::VMDVideoTrack::getFrameRate() const {
	return _decoder->getFrameRate();
}

AdvancedVMDDecoder::VMDAudioTrack::VMDAudioTrack(VMDDecoder *decoder) : _decoder(decoder) {
}

Audio::Mixer::SoundType AdvancedVMDDecoder::VMDAudioTrack::getSoundType() const {
	return _decoder->getSoundType();
}

Audio::AudioStream *AdvancedVMDDecoder::VMDAudioTrack::getAudioStream() const {
	return _decoder->getAudioStream();
}

} // End of namespace Video

#endif // VIDEO_COKTELDECODER_H
