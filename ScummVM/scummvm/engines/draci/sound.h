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

#ifndef DRACI_SOUND_H
#define DRACI_SOUND_H

#include "common/str.h"
#include "common/file.h"
#include "common/list.h"
#include "audio/mixer.h"

namespace Common {
class Archive;
class SeekableReadStream;
}

namespace Draci {

enum SoundFormat { RAW, RAW80, MP3, OGG, FLAC };	// RAW80 means skip the first 80 bytes

/**
 *  Represents individual files inside the archive.
 */
struct SoundSample {
	uint _offset;		// For internal use of LegacySoundArchive
	uint _length;

	uint _frequency;	// Only when _format == RAW or RAW80
	SoundFormat _format;

	byte *_data;		// At most one of these two pointer can be non-NULL
	Common::SeekableReadStream* _stream;

	SoundSample() : _offset(0), _length(0), _frequency(0), _format(RAW), _data(NULL), _stream(NULL) { }
	// The standard copy constructor is good enough, since we only store numbers and pointers.
	// Don't call close() automaticall in the destructor, otherwise copying causes SIGSEGV.
	void close() {
		delete[] _data;
		delete _stream;
		_data = NULL;
		_stream = NULL;
	}
};

/**
 * An abstract wrapper around archives of sound samples or dubbing.
 */
class SoundArchive {
public:
	SoundArchive() { }
	virtual ~SoundArchive() { }

	/**
	 * Returns the number of sound samples in the archive.  Zero means that
	 * a fake empty archive has been opened and the caller may consider
	 * opening a different one, for example with compressed music.
	 */
	virtual uint size() const = 0;

	/**
	 * Checks whether there is an archive opened. Should be called before reading
	 * from the archive to check whether opening of the archive has succeeded.
	 */
	virtual bool isOpen() const = 0;

	/**
	 * Removes cached samples from memory.
	 */
	virtual void clearCache() = 0;

	/**
	 * Caches a given sample into memory and returns a pointer into it.  We
	 * own the returned pointer, but the user may call .close() on it,
	 * which deallocates the memory of the actual sample data.  If freq is
	 * nonzero, then the sample is played at a different frequency (only
	 * works for uncompressed samples).
	 */
	virtual SoundSample *getSample(int i, uint freq) = 0;
};

/**
 * Reads CD.SAM (with dubbing) and CD2.SAM (with sound samples) from the
 * original game.  Caches all read samples in a thread-safe manner.
 */
class LegacySoundArchive : public SoundArchive {
public:
	LegacySoundArchive(const char *path, uint defaultFreq) :
	_path(NULL), _samples(NULL), _sampleCount(0), _defaultFreq(defaultFreq), _opened(false), _f(NULL) {
		openArchive(path);
	}
	virtual ~LegacySoundArchive() { closeArchive(); }

	void openArchive(const char *path);
	void closeArchive();

	virtual uint size() const { return _sampleCount; }
	virtual bool isOpen() const { return _opened; }

	virtual void clearCache();
	virtual SoundSample *getSample(int i, uint freq);

private:
	const char *_path;    ///< Path to file
	SoundSample *_samples;          ///< Internal array of files
	uint _sampleCount;         ///< Number of files in archive
	uint _defaultFreq;	///< The default sampling frequency of the archived samples
	bool _opened;            ///< True if the archive is opened, false otherwise
	Common::File *_f;	///< Opened file
};

/**
 * Reads ZIP archives with uncompressed files containing lossy-compressed
 * samples in arbitrary format.  Doesn't do any real caching and is
 * thread-safe.
 */
class ZipSoundArchive : public SoundArchive {
public:
	ZipSoundArchive() : _archive(NULL), _path(NULL), _extension(NULL), _format(RAW), _sampleCount(0), _defaultFreq(0), _cache() { }
	virtual ~ZipSoundArchive() { closeArchive(); }

	void openArchive(const char *path, const char *extension, SoundFormat format, int raw_frequency = 0);
	void closeArchive();

	virtual uint size() const { return _sampleCount; }
	virtual bool isOpen() const { return _archive != NULL; }

	virtual void clearCache();
	virtual SoundSample *getSample(int i, uint freq);

private:
	Common::Archive *_archive;
	const char *_path;
	const char *_extension;
	SoundFormat _format;
	uint _sampleCount;
	uint _defaultFreq;

	// Since we typically play at most 1 dubbing at a time, we could get
	// away with having just 1 record allocated and reusing it each time.
	// However, that would be thread-unsafe if two samples were played.
	// Although the player does not do that, it is nicer to allow for it in
	// principle.  For each dubbed sentence, we allocate a new record in
	// the following link-list, which is cleared during each location
	// change.  The dubbed samples themselves are manually deallocated
	// after they end.
	Common::List<SoundSample> _cache;
};

#define SOUND_HANDLES 10

enum sndHandleType {
	kFreeHandle,
	kEffectHandle,
	kVoiceHandle
};

struct SndHandle {
	Audio::SoundHandle handle;
	sndHandleType type;
};

// Taken from engines/saga/sound.h and simplified (in particular, removed
// decompression until we support compressed files too).
class Sound {
public:

	Sound(Audio::Mixer *mixer);
	~Sound() {}

	uint playSound(const SoundSample *buffer, int volume, bool loop);
	void pauseSound();
	void resumeSound();
	void stopSound();
	bool isMutedSound() const { return _muteSound; }

	uint playVoice(const SoundSample *buffer);
	void pauseVoice();
	void resumeVoice();
	void stopVoice();
	bool isMutedVoice() const { return _muteVoice; }

	void stopAll() { stopVoice(); stopSound(); }

	void setVolume();

	bool showSubtitles() const { return _showSubtitles; }
	int talkSpeed() const { return _talkSpeed; }

 private:
	// Returns the length of the sound sample in miliseconds.
	uint playSoundBuffer(Audio::SoundHandle *handle, const SoundSample &buffer, int volume,
				sndHandleType handleType, bool loop);

	SndHandle *getHandle();

	Audio::Mixer *_mixer;

	bool _muteSound;
	bool _muteVoice;
	bool _showSubtitles;
	int _talkSpeed;

	SndHandle _handles[SOUND_HANDLES];
};

} // End of namespace Draci

#endif // DRACI_SOUND_H
