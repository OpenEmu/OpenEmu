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
 *
 */

#ifndef KYRA_SOUND_H
#define KYRA_SOUND_H

#include "kyra/kyra_v1.h"

#include "common/scummsys.h"
#include "common/str.h"

#include "audio/mixer.h"

namespace Audio {
class AudioStream;
class SeekableAudioStream;
} // End of namespace Audio

namespace Kyra {

// Helper structs to format the data passed to the various initAudioResourceInfo() implementations
struct SoundResourceInfo_PC {
	SoundResourceInfo_PC(const char *const *files, int numFiles) : fileList(files), fileListSize(numFiles) {}
	const char *const *fileList;
	uint fileListSize;
};

struct SoundResourceInfo_Towns {
	SoundResourceInfo_Towns(const char *const *files, int numFiles, const int32 *cdaTbl, int cdaTblSize) : fileList(files), fileListSize(numFiles), cdaTable(cdaTbl), cdaTableSize(cdaTblSize) {}
	const char *const *fileList;
	uint fileListSize;
	const int32 *cdaTable;
	uint cdaTableSize;
};

struct SoundResourceInfo_PC98 {
	SoundResourceInfo_PC98(const char *fileNamePattern) : pattern(fileNamePattern) {}
	const char *pattern;
};

struct SoundResourceInfo_TownsPC98V2 {
	SoundResourceInfo_TownsPC98V2(const char *const *files, int numFiles, const char *fileNamePattern, const uint16 *cdaTbl, int cdaTblSize) : fileList(files), fileListSize(numFiles), pattern(fileNamePattern), cdaTable(cdaTbl), cdaTableSize(cdaTblSize) {}
	const char *const *fileList;
	uint fileListSize;
	const char *pattern;
	const uint16 *cdaTable;
	uint cdaTableSize;
};

/**
 * Analog audio output device API for Kyrandia games.
 * It contains functionality to play music tracks,
 * sound effects and voices.
 */
class Sound {
public:
	Sound(KyraEngine_v1 *vm, Audio::Mixer *mixer);
	virtual ~Sound();

	enum kType {
		kAdLib,
		kMidiMT32,
		kMidiGM,
		kTowns,
		kPC98,
		kPCSpkr,
		kAmiga
	};

	virtual kType getMusicType() const = 0;
	virtual kType getSfxType() const;

	/**
	 * Initializes the output device.
	 *
	 * @return true on success, else false
	 */
	virtual bool init() = 0;

	/**
	 * Updates the device, this is needed for some devices.
	 */
	virtual void process() {}

	/**
	 * Updates internal volume settings according to ConfigManager.
	 */
	virtual void updateVolumeSettings() {}

	/**
	 * Assigns static resource data with information on how to load
	 * audio resources to
	 *
	 * @param	set				value defined in AudioResourceSet enum
	 *			info			various types of resource info data (file list, file name pattern, struct, etc. - depending on the inheriting driver type)
	 */
	virtual void initAudioResourceInfo(int set, void *info) = 0;

	/**
	 * Select audio resource set.
	 *
	 * @param	set				value defined in AudioResourceSet enum
	 */
	virtual void selectAudioResourceSet(int set) = 0;

	/**
	 * Checks if a given sound file is present.
	 *
	 * @param track track number
	 * @return true if available, false otherwise
	 */
	virtual bool hasSoundFile(uint file) const = 0;

	/**
	 * Load a specifc sound file for use of
	 * playing music and sound effects.
	 */
	virtual void loadSoundFile(uint file) = 0;

	/**
	 * Load a sound file for playing music
	 * (and sometimes sound effects) from.
	 */
	virtual void loadSoundFile(Common::String file) = 0;

	/**
	 * Load a sound file for playing sound
	 * effects from.
	 */
	virtual void loadSfxFile(Common::String file) {}

	/**
	 * Plays the specified track.
	 *
	 * @param track track number
	 */
	virtual void playTrack(uint8 track) = 0;

	/**
	 * Stop playback of the current track.
	 */
	virtual void haltTrack() = 0;

	/**
	 * Plays the specified sound effect.
	 *
	 * @param track sound effect id
	 */
	virtual void playSoundEffect(uint8 track, uint8 volume = 0xff) = 0;

	/**
	 * Stop playback of all sfx tracks.
	 */
	virtual void stopAllSoundEffects() {}

	/**
	 * Checks if the sound driver plays any sound.
	 *
	 * @return true if playing, false otherwise
	 */
	virtual bool isPlaying() const;

	/**
	 * Starts fading out the volume.
	 *
	 * This keeps fading out the output until
	 * it is silenced, but does not change
	 * the volume set by setVolume! It will
	 * automatically reset the volume when
	 * playing a new track or sound effect.
	 */
	virtual void beginFadeOut() = 0;

	/**
	* Stops all audio playback when paused. Continues after end of pause.
	*/
	virtual void pause(bool paused) {}

	void enableMusic(int enable) { _musicEnabled = enable; }
	int musicEnabled() const { return _musicEnabled; }
	void enableSFX(bool enable) { _sfxEnabled = enable; }
	bool sfxEnabled() const { return _sfxEnabled; }

	/**
	 * Checks whether a voice file with the given name is present
	 *
	 * @param file     file name
	 * @return true if available, false otherwise
	 */
	bool isVoicePresent(const char *file) const;

	/**
	 * Plays the specified voice file.
	 *
	 * Also before starting to play the
	 * specified voice file, it stops the
	 * current voice.
	 *
	 * @param file      file to be played
	 * @param volume    volume of the voice file
	 * @param isSfx     marks file as sfx instead of voice
	 * @param handle    store a copy of the sound handle
	 * @return playtime of the voice file (-1 marks unknown playtime)
	 */
	virtual int32 voicePlay(const char *file, Audio::SoundHandle *handle = 0, uint8 volume = 255, uint8 priority = 255, bool isSfx = false);

	Audio::SeekableAudioStream *getVoiceStream(const char *file) const;

	bool playVoiceStream(Audio::AudioStream *stream, Audio::SoundHandle *handle = 0, uint8 volume = 255, uint8 priority = 255, bool isSfx = false);

	/**
	 * Checks if a voice is being played.
	 *
	 * @return true when playing, else false
	 */
	bool voiceIsPlaying(const Audio::SoundHandle *handle = 0) const;

	/**
	 * Checks if all voice handles are used.
	 *
	 * @return false when a handle is free, else true
	 */
	bool allVoiceChannelsPlaying() const;

	/**
	 * Checks how long a voice has been playing
	 *
	 * @return time in milliseconds
	 */
	uint32 voicePlayedTime(const Audio::SoundHandle &handle) const {
		return _mixer->getSoundElapsedTime(handle);
	}

	/**
	 * Stops playback of the current voice.
	 */
	void voiceStop(const Audio::SoundHandle *handle = 0);

	/**
	 * Receive notifications from a song at certain points.
	 */
	virtual int checkTrigger() { return 0; }

	/**
	 * Reset sound trigger.
	 */
	virtual void resetTrigger() {}
protected:
	enum {
		kNumChannelHandles = 4
	};

	struct SoundChannel {
		SoundChannel() : handle(), priority(0) {}
		Audio::SoundHandle handle;
		int priority;
	};

	SoundChannel _soundChannels[kNumChannelHandles];

	int _musicEnabled;
	bool _sfxEnabled;

	KyraEngine_v1 *_vm;
	Audio::Mixer *_mixer;

private:
	struct SpeechCodecs {
		const char *fileext;
		Audio::SeekableAudioStream *(*streamFunc)(
			Common::SeekableReadStream *stream,
			DisposeAfterUse::Flag disposeAfterUse);
	};

	static const SpeechCodecs _supportedCodecs[];
};

class MixedSoundDriver : public Sound {
public:
	MixedSoundDriver(KyraEngine_v1 *vm, Audio::Mixer *mixer, Sound *music, Sound *sfx);
	~MixedSoundDriver();

	virtual kType getMusicType() const;
	virtual kType getSfxType() const;

	virtual bool init();
	virtual void process();

	virtual void updateVolumeSettings();

	virtual void initAudioResourceInfo(int set, void *info);
	virtual void selectAudioResourceSet(int set);
	virtual bool hasSoundFile(uint file) const;
	virtual void loadSoundFile(uint file);
	virtual void loadSoundFile(Common::String file);

	virtual void loadSfxFile(Common::String file);

	virtual void playTrack(uint8 track);
	virtual void haltTrack();
	virtual bool isPlaying() const;

	virtual void playSoundEffect(uint8 track, uint8 volume = 0xff);

	virtual void stopAllSoundEffects();

	virtual void beginFadeOut();
	virtual void pause(bool paused);
private:
	Sound *_music, *_sfx;
};

} // End of namespace Kyra

#endif
