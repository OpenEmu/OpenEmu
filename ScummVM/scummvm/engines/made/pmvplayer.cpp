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

#include "made/pmvplayer.h"
#include "made/made.h"
#include "made/screen.h"
#include "made/graphics.h"

#include "common/file.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/events.h"

#include "audio/decoders/raw.h"
#include "audio/audiostream.h"

#include "graphics/surface.h"

namespace Made {

PmvPlayer::PmvPlayer(MadeEngine *vm, Audio::Mixer *mixer) : _fd(NULL), _vm(vm), _mixer(mixer) {
}

PmvPlayer::~PmvPlayer() {
}

bool PmvPlayer::play(const char *filename) {

	_aborted = false;
	_surface = NULL;

	_fd = new Common::File();
	if (!_fd->open(filename)) {
		delete _fd;
		return false;
	}

	uint32 chunkType, chunkSize, prevChunkSize = 0;

	readChunk(chunkType, chunkSize);	// "MOVE"
	if (chunkType != MKTAG('M','O','V','E')) {
		warning("Unexpected PMV video header, expected 'MOVE'");
		delete _fd;
		return false;
	}

	readChunk(chunkType, chunkSize);	// "MHED"
	if (chunkType != MKTAG('M','H','E','D')) {
		warning("Unexpected PMV video header, expected 'MHED'");
		delete _fd;
		return false;
	}

	uint frameDelay = _fd->readUint16LE();
	_fd->skip(4);	// always 0?
	uint frameCount = _fd->readUint16LE();
	_fd->skip(4);	// always 0?

	uint soundFreq = _fd->readUint16LE();
	// Note: There seem to be weird sound frequencies in PMV videos.
	// Not sure why, but leaving those original frequencies intact
	// results to sound being choppy. Therefore, we set them to more
	// "common" values here (11025 instead of 11127 and 22050 instead
	// of 22254)
	if (soundFreq == 11127) soundFreq = 11025;
	if (soundFreq == 22254) soundFreq = 22050;

	for (int i = 0; i < 22; i++) {
		int unk = _fd->readUint16LE();
		debug(2, "%i ", unk);
	}

	_mixer->stopAll();

	// Read palette
	_fd->read(_paletteRGB, 768);
	_vm->_screen->setRGBPalette(_paletteRGB);

	uint32 frameNumber = 0;
	uint16 chunkCount = 0;
	uint32 soundSize = 0;
	uint32 soundChunkOfs = 0, palChunkOfs = 0;
	uint32 palSize = 0;
	byte *frameData = 0, *audioData, *soundData, *palData, *imageData;
	bool firstTime = true;

	uint32 soundStartTime = 0, skipFrames = 0;

	uint32 bytesRead;
	uint16 width, height, cmdOffs, pixelOffs, maskOffs, lineSize;

	// TODO: Sound can still be a little choppy. A bug in the decoder or -
	// perhaps more likely - do we have to implement double buffering to
	// get it to work well?
	_audioStream = Audio::makeQueuingAudioStream(soundFreq, false);

	while (!_vm->shouldQuit() && !_aborted && !_fd->eos() && frameNumber < frameCount) {

		int32 frameTime = _vm->_system->getMillis();

		readChunk(chunkType, chunkSize);
		if (chunkType != MKTAG('M','F','R','M')) {
			warning("Unknown chunk type");
		}

		// Only reallocate the frame data buffer if its size has changed
		if (prevChunkSize != chunkSize || !frameData) {
			delete[] frameData;
			frameData = new byte[chunkSize];
		}

		prevChunkSize = chunkSize;

		bytesRead = _fd->read(frameData, chunkSize);

		if (bytesRead < chunkSize || _fd->eos())
			break;

		soundChunkOfs = READ_LE_UINT32(frameData + 8);
		palChunkOfs = READ_LE_UINT32(frameData + 16);

		// Handle audio
		if (soundChunkOfs) {
			audioData = frameData + soundChunkOfs - 8;
			chunkSize = READ_LE_UINT16(audioData + 4);
			chunkCount = READ_LE_UINT16(audioData + 6);

			debug(1, "chunkCount = %d; chunkSize = %d; total = %d\n", chunkCount, chunkSize, chunkCount * chunkSize);

			soundSize = chunkCount * chunkSize;
			soundData = (byte *)malloc(soundSize);
			decompressSound(audioData + 8, soundData, chunkSize, chunkCount);
			_audioStream->queueBuffer(soundData, soundSize, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);
		}

		// Handle palette
		if (palChunkOfs) {
			palData = frameData + palChunkOfs - 8;
			palSize = READ_LE_UINT32(palData + 4);
			decompressPalette(palData + 8, _paletteRGB, palSize);
			_vm->_screen->setRGBPalette(_paletteRGB);
		}

		// Handle video
		imageData = frameData + READ_LE_UINT32(frameData + 12) - 8;

		// frameNum @0
		width = READ_LE_UINT16(imageData + 8);
		height = READ_LE_UINT16(imageData + 10);
		cmdOffs = READ_LE_UINT16(imageData + 12);
		pixelOffs = READ_LE_UINT16(imageData + 16);
		maskOffs = READ_LE_UINT16(imageData + 20);
		lineSize = READ_LE_UINT16(imageData + 24);

		debug(2, "width = %d; height = %d; cmdOffs = %04X; pixelOffs = %04X; maskOffs = %04X; lineSize = %d\n",
			width, height, cmdOffs, pixelOffs, maskOffs, lineSize);

		if (!_surface) {
			_surface = new Graphics::Surface();
			_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
		}

		decompressMovieImage(imageData, *_surface, cmdOffs, pixelOffs, maskOffs, lineSize);

		if (firstTime) {
			_mixer->playStream(Audio::Mixer::kPlainSoundType, &_audioStreamHandle, _audioStream);
			soundStartTime = g_system->getMillis();
			skipFrames = 0;
			firstTime = false;
		}

		handleEvents();
		updateScreen();

		if (skipFrames == 0) {
			int32 waitTime = (frameNumber * frameDelay) -
				(g_system->getMillis() - soundStartTime) - (_vm->_system->getMillis() - frameTime);

			if (waitTime < 0) {
				skipFrames = -waitTime / frameDelay;
				warning("Video A/V sync broken, skipping %d frame(s)", skipFrames + 1);
			} else if (waitTime > 0)
				g_system->delayMillis(waitTime);

		} else
			skipFrames--;

		frameNumber++;

	}

	delete[] frameData;

	_audioStream->finish();
	_mixer->stopHandle(_audioStreamHandle);

	//delete _audioStream;
	delete _fd;
	_surface->free();
	delete _surface;

	return !_aborted;

}

void PmvPlayer::readChunk(uint32 &chunkType, uint32 &chunkSize) {
	chunkType = _fd->readUint32BE();
	chunkSize = _fd->readUint32LE();

	debug(2, "ofs = %08X; chunkType = %c%c%c%c; chunkSize = %d\n",
		_fd->pos(),
		(chunkType >> 24) & 0xFF, (chunkType >> 16) & 0xFF, (chunkType >> 8) & 0xFF, chunkType & 0xFF,
		chunkSize);

}

void PmvPlayer::handleEvents() {
	Common::Event event;
	while (_vm->_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
				_aborted = true;
			break;
		default:
			break;
		}
	}
}

void PmvPlayer::updateScreen() {
	_vm->_system->copyRectToScreen(_surface->pixels, _surface->pitch,
									(320 - _surface->w) / 2, (200 - _surface->h) / 2, _surface->w, _surface->h);
	_vm->_system->updateScreen();
}

void PmvPlayer::decompressPalette(byte *palData, byte *outPal, uint32 palDataSize) {
	byte *palDataEnd = palData + palDataSize;
	while (palData < palDataEnd) {
		byte count = *palData++;
		byte entry = *palData++;
		if (count == 255 && entry == 255)
			break;
		memcpy(&outPal[entry * 3], palData, (count + 1) * 3);
		palData += (count + 1) * 3;
	}
}

}
