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



#ifdef ENABLE_AGOS2

#include "common/endian.h"
#include "common/events.h"
#include "common/file.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "graphics/cursorman.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "agos/animation.h"
#include "agos/intern.h"
#include "agos/agos.h"

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"

#include "gui/message.h"

namespace AGOS {

MoviePlayer::MoviePlayer(AGOSEngine_Feeble *vm)
	: _vm(vm) {
	_mixer = _vm->_mixer;

	_leftButtonDown = false;
	_rightButtonDown = false;
	_skipMovie = false;

	memset(baseName, 0, sizeof(baseName));

	_ticks = 0;
}

MoviePlayer::~MoviePlayer() {
}

void MoviePlayer::play() {
	if (_vm->getBitFlag(40)) {
		_vm->setBitFlag(42, false);
		startSound();
		return;
	}

	_leftButtonDown = false;
	_rightButtonDown = false;
	_skipMovie = false;

	_vm->_mixer->stopAll();

	_ticks = _vm->_system->getMillis();

	startSound();

	playVideo();
	stopVideo();

	_vm->o_killAnimate();

	if (_vm->getBitFlag(41)) {
		_vm->fillBackFromFront();
	} else {
		uint8 palette[768];
		memset(palette, 0, sizeof(palette));
		_vm->clearSurfaces();
		_vm->_system->getPaletteManager()->setPalette(palette, 0, 256);
	}

	_vm->fillBackGroundFromBack();
	_vm->_fastFadeOutFlag = true;
}

void MoviePlayer::handleNextFrame() {
	Common::Event event;
	Common::EventManager *eventMan = _vm->_system->getEventManager();
	while (eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
				_leftButtonDown = true;
				_rightButtonDown = true;
			} else if (event.kbd.keycode == Common::KEYCODE_PAUSE) {
				_vm->pause();
			}
			break;
		case Common::EVENT_LBUTTONDOWN:
			_leftButtonDown = true;
			break;
		case Common::EVENT_RBUTTONDOWN:
			_rightButtonDown = true;
			break;
		case Common::EVENT_LBUTTONUP:
			_leftButtonDown = false;
			break;
		case Common::EVENT_RBUTTONUP:
			_rightButtonDown = false;
			break;
		default:
			break;
		}
	}

	if (_leftButtonDown && _rightButtonDown && !_vm->getBitFlag(41)) {
		_skipMovie = true;
		_mixer->stopHandle(_bgSound);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Movie player for DXA movies
///////////////////////////////////////////////////////////////////////////////

const char *const MoviePlayerDXA::_sequenceList[90] = {
	"agent32",
	"Airlock",
	"Badluck",
	"bentalk1",
	"bentalk2",
	"bentalk3",
	"BigFight",
	"BLOWLAB",
	"breakdown",
	"bridge",
	"button2",
	"cargo",
	"COACH",
	"Colatalk",
	"cygnus2",
	"dream",
	"escape2",
	"FASALL",
	"fbikewurb",
	"feebdel",
	"Feebohno",
	"feebpump",
	"feefone1",
	"feefone2",
	"founder2",
	"founder3",
	"founder4",
	"fxmadsam",
	"fxwakeup",
	"gate",
	"Get Car",
	"getaxe",
	"getlift",
	"icetrench",
	"intomb1",
	"intomb2",
	"Jackpot",
	"knockout",
	"labocto",
	"longfeeb",
	"Mainmin",
	"maznat",
	"meetsquid",
	"mflirt",
	"mfxHappy",
	"Mix_Feeb1",
	"Mix_Feeb2",
	"Mix_Feeb3",
	"Mix_Guardscn",
	"Mlights1",
	"MLights2",
	"MProtest",
	"mudman",
	"munlock",
	"MUS5P2",
	"MUSOSP1",
	"Omenter",
	"Omnicofe",
	"OUTMIN~1",
	"Readbook",
	"Rebelhq",
	"RebelHQ2",
	"Reedin",
	"rescue1",
	"rescue2",
	"samcar",
	"Samdead",
	"scanner",
	"Sleepy",
	"spitbrai",
	"statue1",
	"statue2",
	"sva1",
	"sva2",
	"Teeter",
	"Temple2",
	"Temple3",
	"Temple4",
	"Temple5",
	"Temple6",
	"Temple7",
	"Temple8",
	"Tic-tac2",
	"torture",
	"transmit",
	"Typey",
	"ventfall",
	"ventoff",
	"wasting",
	"wurbatak"
};

MoviePlayerDXA::MoviePlayerDXA(AGOSEngine_Feeble *vm, const char *name)
	: MoviePlayer(vm) {
	debug(0, "Creating DXA cutscene player");

	memset(baseName, 0, sizeof(baseName));
	memcpy(baseName, name, strlen(name));

	_sequenceNum = 0;
}

bool MoviePlayerDXA::load() {
	if ((_vm->getPlatform() == Common::kPlatformAmiga || _vm->getPlatform() == Common::kPlatformMacintosh) &&
		_vm->_language != Common::EN_ANY) {
		_sequenceNum = 0;
		for (uint i = 0; i < 90; i++) {
			if (!scumm_stricmp(baseName, _sequenceList[i]))
				_sequenceNum = i;
		}
	}

	Common::String videoName = Common::String::format("%s.dxa", baseName);
	Common::SeekableReadStream *videoStream = _vm->_archives.createReadStreamForMember(videoName);
	if (!videoStream)
		error("Failed to load video file %s", videoName.c_str());
	if (!loadStream(videoStream))
		error("Failed to load video stream from file %s", videoName.c_str());

	debug(0, "Playing video %s", videoName.c_str());

	CursorMan.showMouse(false);
	return true;
}

void MoviePlayerDXA::copyFrameToBuffer(byte *dst, uint x, uint y, uint pitch) {
	uint h = getHeight();
	uint w = getWidth();

	const Graphics::Surface *surface = decodeNextFrame();

	if (!surface)
		return;

	byte *src = (byte *)surface->pixels;
	dst += y * pitch + x;

	do {
		memcpy(dst, src, w);
		dst += pitch;
		src += w;
	} while (--h);

	if (hasDirtyPalette())
		g_system->getPaletteManager()->setPalette(getPalette(), 0, 256);
}

void MoviePlayerDXA::playVideo() {
	// Most of the videos included in the Amiga version, reduced the
	// resoluton to 384 x 280, so require the screen to be cleared,
	// before starting playing those videos.
	if (getWidth() == 384 && getHeight() == 280) {
		_vm->clearSurfaces();
	}

	while (!endOfVideo() && !_skipMovie && !_vm->shouldQuit())
		handleNextFrame();
}

void MoviePlayerDXA::stopVideo() {
	close();
	_mixer->stopHandle(_bgSound);
}

void MoviePlayerDXA::startSound() {
	start();

	if (_bgSoundStream != NULL) {
		_vm->_mixer->stopHandle(_bgSound);
		_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_bgSound, _bgSoundStream, -1, getVolume(), getBalance());
	}
}

void MoviePlayerDXA::nextFrame() {
	if (_bgSoundStream && _vm->_mixer->isSoundHandleActive(_bgSound) && needsUpdate()) {
		copyFrameToBuffer(_vm->getBackBuf(), 465, 222, _vm->_screenWidth);
		return;
	}

	if (_vm->_interactiveVideo == TYPE_LOOPING && endOfVideo()) {
		rewind();
		startSound();
	}

	if (!endOfVideo()) {
		if (_vm->_interactiveVideo == TYPE_OMNITV) {
			copyFrameToBuffer(_vm->getBackBuf(), 465, 222, _vm->_screenWidth);
		} else if (_vm->_interactiveVideo == TYPE_LOOPING) {
			copyFrameToBuffer(_vm->getBackBuf(), (_vm->_screenWidth - getWidth()) / 2, (_vm->_screenHeight - getHeight()) / 2, _vm->_screenWidth);
		}
	} else if (_vm->_interactiveVideo == TYPE_OMNITV) {
		close();
		_vm->_interactiveVideo = 0;
		_vm->_variableArray[254] = 6747;
	}
}

void MoviePlayerDXA::handleNextFrame() {
	if (processFrame())
		_vm->_system->updateScreen();

	MoviePlayer::handleNextFrame();
}

bool MoviePlayerDXA::processFrame() {
	Graphics::Surface *screen = _vm->_system->lockScreen();
	copyFrameToBuffer((byte *)screen->pixels, (_vm->_screenWidth - getWidth()) / 2, (_vm->_screenHeight - getHeight()) / 2, screen->pitch);
	_vm->_system->unlockScreen();

	uint32 soundTime = _mixer->getSoundElapsedTime(_bgSound);
	uint32 nextFrameStartTime = ((Video::VideoDecoder::VideoTrack *)getTrack(0))->getNextFrameStartTime();

	if ((_bgSoundStream == NULL) || soundTime < nextFrameStartTime) {

		if (_bgSoundStream && _mixer->isSoundHandleActive(_bgSound)) {
			while (_mixer->isSoundHandleActive(_bgSound) && soundTime < nextFrameStartTime) {
				_vm->_system->delayMillis(10);
				soundTime = _mixer->getSoundElapsedTime(_bgSound);
			}
			// In case the background sound ends prematurely, update
			// _ticks so that we can still fall back on the no-sound
			// sync case for the subsequent frames.
			_ticks = _vm->_system->getMillis();
		} else {
			_ticks += getTimeToNextFrame();
			while (_vm->_system->getMillis() < _ticks)
				_vm->_system->delayMillis(10);
		}

		return true;
	}

	warning("dropped frame %i", getCurFrame());
	return false;
}

void MoviePlayerDXA::readSoundData(Common::SeekableReadStream *stream) {
	uint32 tag = stream->readUint32BE();

	if (tag == MKTAG('W','A','V','E')) {
		uint32 size = stream->readUint32BE();

		if (_sequenceNum) {
			Common::File in;

			stream->skip(size);

			in.open("audio.wav");
			if (!in.isOpen()) {
				error("Can't read offset file 'audio.wav'");
			}

			in.seek(_sequenceNum * 8, SEEK_SET);
			uint32 offset = in.readUint32LE();
			size = in.readUint32LE();

			in.seek(offset, SEEK_SET);
			_bgSoundStream = Audio::makeWAVStream(in.readStream(size), DisposeAfterUse::YES);
			in.close();
		} else {
			_bgSoundStream = Audio::makeWAVStream(stream->readStream(size), DisposeAfterUse::YES);
		}
	} else {
		_bgSoundStream = Audio::SeekableAudioStream::openStreamFile(baseName);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Movie player for Smacker movies
///////////////////////////////////////////////////////////////////////////////


MoviePlayerSMK::MoviePlayerSMK(AGOSEngine_Feeble *vm, const char *name)
	: MoviePlayer(vm), SmackerDecoder() {
	debug(0, "Creating SMK cutscene player");

	memset(baseName, 0, sizeof(baseName));
	memcpy(baseName, name, strlen(name));
}

bool MoviePlayerSMK::load() {
	Common::String videoName = Common::String::format("%s.smk", baseName);

	Common::SeekableReadStream *videoStream = _vm->_archives.createReadStreamForMember(videoName);
	if (!videoStream)
		error("Failed to load video file %s", videoName.c_str());
	if (!loadStream(videoStream))
		error("Failed to load video stream from file %s", videoName.c_str());

	debug(0, "Playing video %s", videoName.c_str());

	CursorMan.showMouse(false);

	return true;
}

void MoviePlayerSMK::copyFrameToBuffer(byte *dst, uint x, uint y, uint pitch) {
	uint h = getHeight();
	uint w = getWidth();

	const Graphics::Surface *surface = decodeNextFrame();

	if (!surface)
		return;

	byte *src = (byte *)surface->pixels;
	dst += y * pitch + x;

	do {
		memcpy(dst, src, w);
		dst += pitch;
		src += w;
	} while (--h);

	if (hasDirtyPalette())
		g_system->getPaletteManager()->setPalette(getPalette(), 0, 256);
}

void MoviePlayerSMK::playVideo() {
	while (!endOfVideo() && !_skipMovie && !_vm->shouldQuit())
		handleNextFrame();
}

void MoviePlayerSMK::stopVideo() {
	close();
}

void MoviePlayerSMK::startSound() {
	start();
}

void MoviePlayerSMK::handleNextFrame() {
	processFrame();

	MoviePlayer::handleNextFrame();
}

void MoviePlayerSMK::nextFrame() {
	if (_vm->_interactiveVideo == TYPE_LOOPING && endOfVideo())
		rewind();

	if (!endOfVideo()) {
		decodeNextFrame();
		if (_vm->_interactiveVideo == TYPE_OMNITV) {
			copyFrameToBuffer(_vm->getBackBuf(), 465, 222, _vm->_screenWidth);
		} else if (_vm->_interactiveVideo == TYPE_LOOPING) {
			copyFrameToBuffer(_vm->getBackBuf(), (_vm->_screenWidth - getWidth()) / 2, (_vm->_screenHeight - getHeight()) / 2, _vm->_screenWidth);
		}
	} else if (_vm->_interactiveVideo == TYPE_OMNITV) {
		close();
		_vm->_interactiveVideo = 0;
		_vm->_variableArray[254] = 6747;
	}
}

bool MoviePlayerSMK::processFrame() {
	Graphics::Surface *screen = _vm->_system->lockScreen();
	copyFrameToBuffer((byte *)screen->pixels, (_vm->_screenWidth - getWidth()) / 2, (_vm->_screenHeight - getHeight()) / 2, screen->pitch);
	_vm->_system->unlockScreen();

	uint32 waitTime = getTimeToNextFrame();

	if (!waitTime && !endOfVideoTracks()) {
		warning("dropped frame %i", getCurFrame());
		return false;
	}

	_vm->_system->updateScreen();

	// Wait before showing the next frame
	_vm->_system->delayMillis(waitTime);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Factory function for creating the appropriate cutscene player
///////////////////////////////////////////////////////////////////////////////

MoviePlayer *makeMoviePlayer(AGOSEngine_Feeble *vm, const char *name) {
	char baseName[40];
	char filename[20];

	int baseLen = strlen(name) - 4;
	memset(baseName, 0, sizeof(baseName));
	memcpy(baseName, name, baseLen);

	if (vm->getLanguage() == Common::DE_DEU && baseLen >= 8) {
		// Check short filename to work around
		// bug in a German Windows 2CD version.
		char shortName[20];
		memset(shortName, 0, sizeof(shortName));
		memcpy(shortName, baseName, 6);

		sprintf(filename, "%s~1.dxa", shortName);
		if (vm->_archives.hasFile(filename)) {
			memset(baseName, 0, sizeof(baseName));
			memcpy(baseName, filename, 8);
		}

		sprintf(filename, "%s~1.smk", shortName);
		if (vm->_archives.hasFile(filename)) {
			memset(baseName, 0, sizeof(baseName));
			memcpy(baseName, filename, 8);
		}
	}

	sprintf(filename, "%s.dxa", baseName);
	if (vm->_archives.hasFile(filename)) {
		return new MoviePlayerDXA(vm, baseName);
	}

	sprintf(filename, "%s.smk", baseName);
	if (vm->_archives.hasFile(filename)) {
		return new MoviePlayerSMK(vm, baseName);
	}

	Common::String buf = Common::String::format(_("Cutscene file '%s' not found!"), baseName);
	GUI::MessageDialog dialog(buf, _("OK"));
	dialog.runModal();

	return NULL;
}

} // End of namespace AGOS

#endif // ENABLE_AGOS2
