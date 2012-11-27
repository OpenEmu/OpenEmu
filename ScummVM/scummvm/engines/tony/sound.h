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

#ifndef TONY_SOUND_H
#define TONY_SOUND_H

#include "audio/mixer.h"
#include "common/file.h"
#include "tony/gfxcore.h"
#include "tony/loc.h"
#include "tony/utils.h"

namespace Audio {
class RewindableAudioStream;
}

namespace Tony {

class FPStream;
class FPSfx;

enum SoundCodecs {
	FPCODEC_RAW,
	FPCODEC_ADPCM
};

/**
 * Sound driver For Tony Tough
 */

class FPSound {
private:
	bool _soundSupported;

public:
	/**
	 * Default constructor. Initializes the attributes.
	 *
	 */

	FPSound();

	/**
	 * Destroy the object and free the memory
	 *
	 */

	~FPSound();

	/**
	 * Initializes the object, and prepare everything you need to create streams and sound effects.
	 *
	 * @returns             True is everything is OK, False otherwise
	 */

	bool init();

	/**
	 * Allocates an object of type FPStream, and return its pointer
	 *
	 * @param streamPtr     Will contain a pointer to the object you just created.
	 *
	 * @returns             True is everything is OK, False otherwise
	 */

	bool createStream(FPStream **streamPtr);

	/**
	 * Allocates an object of type FpSfx, and return its pointer
	 *
	 * @param sfxPtr        Will contain a pointer to the object you just created.
	 *
	 * @returns             True is everything is OK, False otherwise
	 */

	bool createSfx(FPSfx **sfxPtr);

	/**
	 * Set the general volume
	 *
	 * @param volume        Volume to set (0-63)
	 */

	void setMasterVolume(int volume);

	/**
	 * Get the general volume
	 *
	 * @param volume        Variable that will contain the volume (0-63)
	 */

	void getMasterVolume(int *volume);
};

class FPSfx {
private:
	bool _soundSupported;                  // True if the sound is active
	bool _fileLoaded;                      // True is a file is opened
	bool _loop;                            // True is sound effect should loop
	int  _lastVolume;

	bool _isVoice;
	bool _paused;

	Audio::AudioStream *_loopStream;
	Audio::RewindableAudioStream *_rewindableStream;
	Audio::SoundHandle _handle;

public:
	uint32 _hEndOfBuffer;

	/**
	 * Check process for whether sounds have finished playing
	 */
	static void soundCheckProcess(CORO_PARAM, const void *param);

	/**
	 * Default constructor.
	 *
	 * @remarks             Do *NOT* declare an object directly, but rather
	 *                      create it using FPSound::CreateSfx()
	 *
	 */

	FPSfx(bool soundOn);

	/**
	 * Default Destructor.
	 *
	 * @remarks             It is also stops the sound effect that may be
	 *                      currently played, and free the memory it uses.
	 *
	 */

	~FPSfx();

	/**
	 * Releases the memory used by the object.
	 *
	 * @remarks             Must be called when the object is no longer used and
	 *                      **ONLY** if the object was created by
	 *                      FPSound::CreateStream().
	 *                      Object pointers are no longer valid after this call.
	 */

	void release();

	/**
	 * Opens a file and loads a sound effect.
	 *
	 * @param fileName      Sfx filename
	 * @param codec         CODEC used to uncompress the samples
	 *
	 * @returns             True is everything is OK, False otherwise
	 */

	bool loadFile(const char *fileName, uint32 codec = FPCODEC_RAW);
	bool loadWave(Common::SeekableReadStream *stream);
	bool loadVoiceFromVDB(Common::File &vdbFP);

	/**
	 * Play the Sfx in memory.
	 *
	 * @returns             True is everything is OK, False otherwise
	 */

	bool play();

	/**
	 * Stops a Sfx.
	 *
	 * @returns             True is everything is OK, False otherwise
	 */

	bool stop();

	/**
	 * Pauses a Sfx.
	 *
	 */

	void setPause(bool pause);

	/**
	 * Enables or disables the Sfx loop.
	 *
	 * @param loop         True to enable the loop, False to disable
	 *
	 * @remarks             The loop must be activated BEFORE the sfx starts
	 *                      playing. Any changes made during the play will have
	 *                      no effect until the sfx is stopped then played again.
	 */

	void setLoop(bool loop);

	/**
	 * Change the volume of Sfx
	 *
	 * @param volume        Volume to be set (0-63)
	 *
	 */

	void setVolume(int volume);

	/**
	 * Gets the Sfx volume
	 *
	 * @param volumePtr     Will contain the current Sfx volume
	 *
	 */

	void getVolume(int *volumePtr);

	/**
	 * Returns true if the underlying sound has ended
	 */
	bool endOfBuffer() const;
};

class FPStream {
private:
	uint32 _bufferSize;                  // Buffer size (bytes)
	uint32 _size;                        // Stream size (bytes)
	uint32 _codec;                       // CODEC used

	Common::File _file;                  // File handle used for the stream

	bool _soundSupported;                // True if the sound is active
	bool _fileLoaded;                    // True if the file is open
	bool _loop;                          // True if the stream should loop
	bool _doFadeOut;                     // True if fade out is required
	bool _syncExit;
	bool _paused;
	int  _lastVolume;
	FPStream *_syncToPlay;

	Audio::AudioStream *_loopStream;
	Audio::RewindableAudioStream *_rewindableStream;
	Audio::SoundHandle _handle;

public:

	/**
	 * Default constructor.
	 *
	 * @remarks             Do *NOT* declare an object directly, but rather
	 *                      create it using FPSound::CreateStream()
	 */

	FPStream(bool soundOn);

	/**
	 * Default destructor.
	 *
	 * @remarks             It calls CloseFile() if needed.
	 */

	~FPStream();

	/**
	 * Releases the memory object.
	 *
	 * @remarks             Must be called when the object is no longer used
	 *                      and **ONLY** if the object was created by
	 *                      FPSound::CreateStream().
	 *                      Object pointers are no longer valid after this call.
	 */

	void release();

	/**
	 * Opens a file stream
	 *
	 * @param fileName      Filename to be opened
	 * @param codec         CODEC to be used to uncompress samples
	 *
	 * @returns             True is everything is OK, False otherwise
	 */

	bool loadFile(const Common::String &fileName, uint32 codec = FPCODEC_RAW, int sync = 2000);

	/**
	 * Closes a file stream (opened or not).
	 *
	 * @returns             For safety, the destructor calls unloadFile() if it has not
	 *                      been mentioned explicitly.
	 *
	 * @remarks             It is necessary to call this function to free the
	 *                      memory used by the stream.
	 */

	bool unloadFile();

	/**
	 * Play the stream.
	 *
	 * @returns             True is everything is OK, False otherwise
	 */

	bool play();
	void playFast();

	/**
	 * Closes the stream.
	 *
	 * @returns             True is everything is OK, False otherwise
	 */

	bool stop();
	void waitForSync(FPStream *toPlay);

	/**
	 * Pause sound effect
	 *
	 * @param pause         True enables pause, False disables it
	 */

	void setPause(bool pause);

	/**
	 * Unables or disables stream loop.
	 *
	 * @param loop          True enable loop, False disables it
	 *
	 * @remarks             The loop must be activated BEFORE the stream starts
	 *                      playing. Any changes made during the play will have no
	 *                      effect until the stream is stopped then played again.
	 */

	void setLoop(bool loop);

	/**
	 * Change the volume of the stream
	 *
	 * @param volume        Volume to be set (0-63)
	 */

	void setVolume(int volume);

	/**
	 * Gets the volume of the stream
	 *
	 * @param volumePtr     Variable that will contain the current volume
	 *
	 */

	void getVolume(int *volumePtr);
};

} // End of namespace Tony

#endif
