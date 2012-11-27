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

/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#include "audio/audiostream.h"
#include "audio/decoders/adpcm.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/wave.h"
#include "common/textconsole.h"
#include "tony/game.h"
#include "tony/tony.h"

namespace Tony {

/*
 * Tony uses a [0,63] volume scale (where 0 is silent and 63 is loudest).
 * The original game engine linearly mapped this scale into DirectSound's
 * [-10000, 0] scale (where -10000 is silent), which is a logarithmic scale.
 *
 * This means that Tony's scale is logarithmic as well, and must be converted
 * to the linear scale used by the mixer.
 */
static int remapVolume(int volume) {
	double dsvol = (double)(63 - volume) * -10000.0 / 63.0;
	return (int)((double)Audio::Mixer::kMaxChannelVolume * pow(10.0, dsvol / 2000.0) + 0.5);
}

/****************************************************************************\
*       FPSOUND Methods
\****************************************************************************/

/**
 * Default constructor. Initializes the attributes.
 *
 */
FPSound::FPSound() {
	_soundSupported = false;
}

/**
 * Initializes the object, and prepare everything you need to create streams and sound effects.
 *
 * @returns     True is everything is OK, False otherwise
 */
bool FPSound::init() {
	_soundSupported = g_system->getMixer()->isReady();
	return _soundSupported;
}

/**
 * Destroy the object and free the memory
 *
 */
FPSound::~FPSound() {
}

/**
 * Allocates an object of type FPStream, and return its pointer
 *
 * @param streamPtr    Will contain a pointer to the object you just created.
 *
 * @returns     True is everything is OK, False otherwise
 */
bool FPSound::createStream(FPStream **streamPtr) {
	(*streamPtr) = new FPStream(_soundSupported);

	return (*streamPtr != NULL);
}

/**
 * Allocates an object of type FpSfx, and return its pointer
 *
 * @param soundPtr        Will contain a pointer to the object you just created.
 *
 * @returns     True is everything is OK, False otherwise
 */
bool FPSound::createSfx(FPSfx **sfxPtr) {
	(*sfxPtr) = new FPSfx(_soundSupported);

	return (*sfxPtr != NULL);
}

/**
 * Set the general volume
 *
 * @param volume            Volume to set (0-63)
 */
void FPSound::setMasterVolume(int volume) {
	if (!_soundSupported)
		return;

	// WORKAROUND: We don't use remapVolume() here, so that the main option screen exposes
	// a linear scale to the user. This is an improvement over the original game
	// where the user had to deal with a logarithmic volume scale.
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, CLIP<int>(volume, 0, 63) * Audio::Mixer::kMaxChannelVolume / 63);
}

/**
 * Get the general volume
 *
 * @param volumePtr           Variable that will contain the volume (0-63)
 */
void FPSound::getMasterVolume(int *volumePtr) {
	if (!_soundSupported)
		return;

	*volumePtr = g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kPlainSoundType) * 63 / Audio::Mixer::kMaxChannelVolume;
}

/**
 * Default constructor.
 *
 * @remarks                   Do *NOT* declare an object directly, but rather
 *                            create it using FPSound::CreateSfx()
 *
 */
FPSfx::FPSfx(bool soundOn) {
	_soundSupported = soundOn;
	_fileLoaded = false;
	_lastVolume = 63;
	_hEndOfBuffer = CoroScheduler.createEvent(true, false);
	_isVoice = false;
	_loopStream = 0;
	_rewindableStream = 0;
	_paused = false;

	g_vm->_activeSfx.push_back(this);
}

/**
 * Default Destructor.
 *
 * @remarks                   It is also stops the sound effect that may be
 *                            currently played, and free the memory it uses.
 *
 */
FPSfx::~FPSfx() {
	if (!_soundSupported)
		return;

	g_system->getMixer()->stopHandle(_handle);
	g_vm->_activeSfx.remove(this);

	if (_loopStream)
		delete _loopStream; // _rewindableStream is deleted by deleting _loopStream
	else
		delete _rewindableStream;

	// Free the buffer end event
	CoroScheduler.closeEvent(_hEndOfBuffer);
}

/**
 * Releases the memory used by the object.
 *
 * @remarks                Must be called when the object is no longer used and
 *                         **ONLY** if the object was created by
 *                         FPSound::CreateStream().
 *                         Object pointers are no longer valid after this call.
 */
void FPSfx::release() {
	delete this;
}

bool FPSfx::loadWave(Common::SeekableReadStream *stream) {
	if (!stream)
		return false;

	_rewindableStream = Audio::makeWAVStream(stream, DisposeAfterUse::YES);

	if (!_rewindableStream)
		return false;

	_fileLoaded = true;
	setVolume(_lastVolume);
	return true;
}

bool FPSfx::loadVoiceFromVDB(Common::File &vdbFP) {
	if (!_soundSupported)
		return true;

	uint32 size = vdbFP.readUint32LE();
	uint32 rate = vdbFP.readUint32LE();
	_isVoice = true;

	_rewindableStream = Audio::makeADPCMStream(vdbFP.readStream(size), DisposeAfterUse::YES, 0, Audio::kADPCMDVI, rate, 1);

	_fileLoaded = true;
	setVolume(62);
	return true;
}

/**
 * Opens a file and loads a sound effect.
 *
 * @param fileName         Sfx filename
 * @param codec            CODEC used to uncompress the samples
 *
 * @returns                True is everything is OK, False otherwise
 */
bool FPSfx::loadFile(const char *fileName, uint32 codec) {
	if (!_soundSupported)
		return true;

	Common::File file;
	if (!file.open(fileName)) {
		warning("FPSfx::LoadFile(): Cannot open sfx file!");
		return false;
	}

	if (file.readUint32BE() != MKTAG('A', 'D', 'P', 0x10)) {
		warning("FPSfx::LoadFile(): Invalid ADP header!");
		return false;
	}

	uint32 rate = file.readUint32LE();
	uint32 channels = file.readUint32LE();

	Common::SeekableReadStream *buffer = file.readStream(file.size() - file.pos());

	if (codec == FPCODEC_ADPCM) {
		_rewindableStream = Audio::makeADPCMStream(buffer, DisposeAfterUse::YES, 0, Audio::kADPCMDVI, rate, channels);
	} else {
		byte flags = Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;

		if (channels == 2)
			flags |= Audio::FLAG_STEREO;

		_rewindableStream = Audio::makeRawStream(buffer, rate, flags, DisposeAfterUse::YES);
	}

	_fileLoaded = true;
	return true;
}

/**
 * Play the Sfx in memory.
 *
 * @returns                True is everything is OK, False otherwise
 */
bool FPSfx::play() {
	stop(); // sanity check

	if (_fileLoaded) {
		CoroScheduler.resetEvent(_hEndOfBuffer);

		_rewindableStream->rewind();

		Audio::AudioStream *stream = _rewindableStream;

		if (_loop) {
			if (!_loopStream)
				_loopStream = Audio::makeLoopingAudioStream(_rewindableStream, 0);

			stream = _loopStream;
		}

		g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_handle, stream, -1,
				Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);

		setVolume(_lastVolume);

		if (_paused)
			g_system->getMixer()->pauseHandle(_handle, true);
	}

	return true;
}

/**
 * Stops a Sfx.
 *
 * @returns                True is everything is OK, False otherwise
 */
bool FPSfx::stop() {
	if (_fileLoaded) {
		g_system->getMixer()->stopHandle(_handle);
		_paused = false;
	}

	return true;
}

/**
 * Enables or disables the Sfx loop.
 *
 * @param loop          True to enable the loop, False to disable
 *
 * @remarks             The loop must be activated BEFORE the sfx starts
 *                      playing. Any changes made during the play will have
 *                      no effect until the sfx is stopped then played again.
 */
void FPSfx::setLoop(bool loop) {
	_loop = loop;
}

/**
 * Pauses a Sfx.
 *
 */
void FPSfx::setPause(bool pause) {
	if (_fileLoaded) {
		if (g_system->getMixer()->isSoundHandleActive(_handle) && (pause ^ _paused))
			g_system->getMixer()->pauseHandle(_handle, pause);

		_paused = pause;
	}
}

/**
 * Change the volume of Sfx
 *
 * @param volume        Volume to be set (0-63)
 *
 */
void FPSfx::setVolume(int volume) {
	if (volume > 63)
		volume = 63;

	if (volume < 0)
		volume = 0;

	_lastVolume = volume;

	if (_isVoice) {
		if (!GLOBALS._bCfgDubbing)
			volume = 0;
		else {
			volume -= (10 - GLOBALS._nCfgDubbingVolume) * 2;
			if (volume < 0)
				volume = 0;
		}
	} else {
		if (!GLOBALS._bCfgSFX)
			volume = 0;
		else {
			volume -= (10 - GLOBALS._nCfgSFXVolume) * 2;
			if (volume < 0)
				volume = 0;
		}
	}

	if (g_system->getMixer()->isSoundHandleActive(_handle))
		g_system->getMixer()->setChannelVolume(_handle, remapVolume(volume));
}

/**
 * Gets the Sfx volume
 *
 * @param volumePtr     Will contain the current Sfx volume
 *
 */
void FPSfx::getVolume(int *volumePtr) {
	if (g_system->getMixer()->isSoundHandleActive(_handle))
		*volumePtr = _lastVolume;
	else
		*volumePtr = 0;
}

/**
 * Returns true if the underlying sound has ended
 */
bool FPSfx::endOfBuffer() const {
	return !g_system->getMixer()->isSoundHandleActive(_handle) && (!_rewindableStream || _rewindableStream->endOfData());
}

/**
 * Continually checks to see if active sounds have finished playing
 * Sets the event signalling the sound has ended
 */
void FPSfx::soundCheckProcess(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
		Common::List<FPSfx *>::iterator i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	for (;;) {
		// Check each active sound
		for (_ctx->i = g_vm->_activeSfx.begin(); _ctx->i != g_vm->_activeSfx.end(); ++_ctx->i) {
			FPSfx *sfx = *_ctx->i;
			if (sfx->endOfBuffer())
				CoroScheduler.setEvent(sfx->_hEndOfBuffer);
		}

		// Delay until the next check is done
		CORO_INVOKE_1(CoroScheduler.sleep, 50);
	}

	CORO_END_CODE;
}

/**
 * Default constructor.
 *
 * @remarks             Do *NOT* declare an object directly, but rather
 *                      create it using FPSound::CreateStream()
 */
FPStream::FPStream(bool soundOn) {
	_soundSupported = soundOn;
	_fileLoaded = false;
	_paused = false;
	_loop = false;
	_doFadeOut = false;
	_syncExit = false;
	_bufferSize = _size = 0;
	_lastVolume = 0;
	_syncToPlay = NULL;
	_loopStream = NULL;
	_rewindableStream = NULL;
}

/**
 * Default destructor.
 *
 * @remarks             It calls CloseFile() if needed.
 */
FPStream::~FPStream() {
	if (!_soundSupported)
		return;

	if (g_system->getMixer()->isSoundHandleActive(_handle))
		stop();

	if (_fileLoaded)
		unloadFile();

	_syncToPlay = NULL;
}

/**
 * Releases the memory object.
 *
 * @remarks             Must be called when the object is no longer used
 *                      and **ONLY** if the object was created by
 *                      FPSound::CreateStream().
 *                      Object pointers are no longer valid after this call.
 */
void FPStream::release() {
	delete this;
}

/**
 * Opens a file stream
 *
 * @param fileName      Filename to be opened
 * @param codec         CODEC to be used to uncompress samples
 *
 * @returns             True is everything is OK, False otherwise
 */
bool FPStream::loadFile(const Common::String &fileName, uint32 codec, int bufSize) {
	if (!_soundSupported)
		return true;

	if (_fileLoaded)
		unloadFile();

	// Save the codec type
	_codec = codec;

	// Open the file stream for reading
	if (!_file.open(fileName)) {
		// Fallback: try with an extra '0' prefix
		if (!_file.open("0" + fileName))
			return false;
		warning("FPStream::loadFile(): Fallback from %s to %s", fileName.c_str(), _file.getName());
	}

	// Save the size of the stream
	_size = _file.size();

	switch (_codec) {
	case FPCODEC_RAW:
		_rewindableStream = Audio::makeRawStream(&_file, 44100, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN | Audio::FLAG_STEREO, DisposeAfterUse::NO);
		break;

	case FPCODEC_ADPCM:
		_rewindableStream = Audio::makeADPCMStream(&_file, DisposeAfterUse::NO, 0, Audio::kADPCMDVI, 44100, 2);
		break;

	default:
		_file.close();
		return false;
	}

	// All done
	_fileLoaded = true;
	_paused = false;

	setVolume(63);

	return true;
}

/**
 * Closes a file stream (opened or not).
 *
 * @returns             For safety, the destructor calls unloadFile() if it has not
 *                      been mentioned explicitly.
 *
 * @remarks             It is necessary to call this function to free the
 *                      memory used by the stream.
 */
bool FPStream::unloadFile() {
	if (!_soundSupported || !_fileLoaded)
		return true;

	assert(!g_system->getMixer()->isSoundHandleActive(_handle));

	// Closes the file handle stream
	delete _loopStream;
	delete _rewindableStream;
	_loopStream = NULL;
	_rewindableStream = NULL;
	_file.close();

	// Flag that the file is no longer in memory
	_fileLoaded = false;

	return true;
}

/**
 * Play the stream.
 *
 * @returns             True is everything is OK, False otherwise
 */

bool FPStream::play() {
	if (!_soundSupported || !_fileLoaded)
		return false;

	stop();

	_rewindableStream->rewind();

	Audio::AudioStream *stream = _rewindableStream;

	if (_loop) {
		if (!_loopStream)
			_loopStream = new Audio::LoopingAudioStream(_rewindableStream, 0, DisposeAfterUse::NO);

		stream = _loopStream;
	}

	// FIXME: Should this be kMusicSoundType or KPlainSoundType?
	g_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, &_handle, stream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
	setVolume(_lastVolume);
	_paused = false;

	return true;
}

/**
 * Closes the stream.
 *
 * @returns             True is everything is OK, False otherwise
 *
 */
bool FPStream::stop() {
	if (!_soundSupported)
		return true;

	if (!_fileLoaded)
		return false;

	if (!g_system->getMixer()->isSoundHandleActive(_handle))
		return false;

	g_system->getMixer()->stopHandle(_handle);

	_paused = false;

	return true;
}

void FPStream::waitForSync(FPStream *toPlay) {
	// FIXME: The idea here is that you wait for this stream to reach
	// a buffer which is a multiple of nBufSize/nSync, and then the
	// thread stops it and immediately starts the 'toplay' stream.

	stop();
	toPlay->play();
}

/**
 * Unables or disables stream loop.
 *
 * @param loop          True enable loop, False disables it
 *
 * @remarks             The loop must be activated BEFORE the stream starts
 *                      playing. Any changes made during the play will have no
 *                      effect until the stream is stopped then played again.
 */
void FPStream::setLoop(bool loop) {
	_loop = loop;
}

/**
 * Pause sound effect
 *
 * @param pause         True enables pause, False disables it
 */
void FPStream::setPause(bool pause) {
	if (!_fileLoaded)
		return;

	if (pause == _paused)
		return;

	if (g_system->getMixer()->isSoundHandleActive(_handle))
		g_system->getMixer()->pauseHandle(_handle, pause);

	_paused = pause;

	// Trick to reset the volume after a possible new sound configuration
	setVolume(_lastVolume);
}

/**
 * Change the volume of the stream
 *
 * @param volume        Volume to be set (0-63)
 *
 */
void FPStream::setVolume(int volume) {
	if (volume > 63)
		volume = 63;

	if (volume < 0)
		volume = 0;

	_lastVolume = volume;

	if (!GLOBALS._bCfgMusic)
		volume = 0;
	else {
		volume -= (10 - GLOBALS._nCfgMusicVolume) * 2;
		if (volume < 0)
			volume = 0;
	}

	if (g_system->getMixer()->isSoundHandleActive(_handle))
		g_system->getMixer()->setChannelVolume(_handle, remapVolume(volume));
}

/**
 * Gets the volume of the stream
 *
 * @param volumePtr     Variable that will contain the current volume
 *
 */
void FPStream::getVolume(int *volumePtr) {
	if (g_system->getMixer()->isSoundHandleActive(_handle))
		*volumePtr = _lastVolume;
	else
		*volumePtr = 0;
}

} // End of namespace Tony
