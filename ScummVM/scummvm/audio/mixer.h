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

#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H

#include "common/types.h"
#include "common/noncopyable.h"

namespace Audio {

class AudioStream;
class Channel;
class Timestamp;

/**
 * A SoundHandle instances corresponds to a specific sound
 * being played via the mixer. It can be used to control that
 * sound (pause it, stop it, etc.).
 * @see The Mixer class
 */
class SoundHandle {
	friend class Channel;
	friend class MixerImpl;
	uint32 _val;
public:
	inline SoundHandle() : _val(0xFFFFFFFF) {}
};

/**
 * The main audio mixer handles mixing of an arbitrary number of
 * audio streams (in the form of AudioStream instances).
 */
class Mixer : Common::NonCopyable {
public:
	enum SoundType {
		kPlainSoundType = 0,

		kMusicSoundType = 1,
		kSFXSoundType = 2,
		kSpeechSoundType = 3
	};

	enum {
		kMaxChannelVolume = 255,
		kMaxMixerVolume = 256
	};

public:
	Mixer() {}
	virtual ~Mixer() {}



	/**
	 * Is the mixer ready and setup? This may not be the case on systems which
	 * don't support digital sound output. In that case, the mixer proc may
	 * never be called. That in turn can cause breakage in games which try to
	 * sync with an audio stream. In particular, the AdLib MIDI emulation...
	 *
	 * @return whether the mixer is ready and setup
	 *
	 * @todo get rid of this?
	 */
	virtual bool isReady() const = 0;


	/**
	 * Start playing the given audio stream.
	 *
	 * Note that the sound id assigned below is unique. At most one stream
	 * with a given id can play at any given time. Trying to play a sound
	 * with an id that is already in use causes the new sound to be not played.
	 *
	 * @param type	the type (voice/sfx/music) of the stream
	 * @param handle	a SoundHandle which can be used to reference and control
	 *                  the stream via suitable mixer methods
	 * @param stream	the actual AudioStream to be played
	 * @param id	a unique id assigned to this stream
	 * @param volume	the volume with which to play the sound, ranging from 0 to 255
	 * @param balance	the balance with which to play the sound, ranging from -127 to 127 (full left to full right), 0 is balanced, -128 is invalid
	 * @param autofreeStream	a flag indicating whether the stream should be
	 *                          freed after playback finished
	 * @param permanent	a flag indicating whether a plain stopAll call should
	 *                  not stop this particular stream
	 * @param reverseStereo	a flag indicating whether left and right channels shall be swapped
	 */
	virtual void playStream(
		SoundType type,
		SoundHandle *handle,
		AudioStream *stream,
		int id = -1,
		byte volume = kMaxChannelVolume,
		int8 balance = 0,
		DisposeAfterUse::Flag autofreeStream = DisposeAfterUse::YES,
		bool permanent = false,
		bool reverseStereo = false) = 0;

	/**
	 * Stop all currently playing sounds.
	 */
	virtual void stopAll() = 0;

	/**
	 * Stop playing the sound with given ID.
	 *
	 * @param id the ID of the sound to affect
	 */
	virtual void stopID(int id) = 0;

	/**
	 * Stop playing the sound corresponding to the given handle.
	 *
	 * @param handle the sound to affect
	 */
	virtual void stopHandle(SoundHandle handle) = 0;



	/**
	 * Pause/unpause all sounds, including all regular and permanent
	 * channels
	 *
	 * @param paused true to pause everything, false to unpause
	 */
	virtual void pauseAll(bool paused) = 0;

	/**
	 * Pause/unpause the sound with the given ID.
	 *
	 * @param id the ID of the sound to affect
	 * @param paused true to pause the sound, false to unpause it
	 */
	virtual void pauseID(int id, bool paused) = 0;

	/**
	 * Pause/unpause the sound corresponding to the given handle.
	 *
	 * @param handle the sound to affect
	 * @param paused true to pause the sound, false to unpause it
	 */
	virtual void pauseHandle(SoundHandle handle, bool paused) = 0;



	/**
	 * Check if a sound with the given ID is active.
	 *
	 * @param id the ID of the sound to query
	 * @return true if the sound is active
	 */
	virtual bool isSoundIDActive(int id) = 0;

	/**
	 * Get the sound ID of handle sound
	 *
	 * @param handle sound to query
	 * @return sound ID if active
	 */
	virtual int getSoundID(SoundHandle handle) = 0;

	/**
	 * Check if a sound with the given handle is active.
	 *
	 * @param handle sound to query
	 * @return true if the sound is active
	 */
	virtual bool isSoundHandleActive(SoundHandle handle) = 0;


	/**
	 * Set the mute state for a given sound type.
	 *
	 * @param type the sound type
	 * @param mute Whether to mute (= true) or not (= false).
	 */
	virtual void muteSoundType(SoundType type, bool mute) = 0;

	/**
	 * Query the mute state for a given sound type.
	 *
	 * @param type the sound type
	 */
	virtual bool isSoundTypeMuted(SoundType type) const = 0;

	/**
	 * Set the channel volume for the given handle.
	 *
	 * @param handle the sound to affect
	 * @param volume the new channel volume (0 - kMaxChannelVolume)
	 */
	virtual void setChannelVolume(SoundHandle handle, byte volume) = 0;

	/**
	 * Get the channel volume for the given handle.
	 *
	 * @param handle the sound to affect
	 * @return channel volume
	 */
	virtual byte getChannelVolume(SoundHandle handle) = 0;

	/**
	 * Set the channel balance for the given handle.
	 *
	 * @param handle the sound to affect
	 * @param balance the new channel balance:
	 *        (-127 ... 0 ... 127) corresponds to (left ... center ... right)
	 */
	virtual void setChannelBalance(SoundHandle handle, int8 balance) = 0;

	/**
	 * Get the channel balance for the given handle.
	 *
	 * @param handle the sound to affect
	 * @return channel balance
	 */
	virtual int8 getChannelBalance(SoundHandle handle) = 0;

	/**
	 * Get approximation of for how long the channel has been playing.
	 */
	virtual uint32 getSoundElapsedTime(SoundHandle handle) = 0;

	/**
	 * Get approximation of for how long the channel has been playing.
	 */
	virtual Timestamp getElapsedTime(SoundHandle handle) = 0;

	/**
	 * Check whether any channel of the given sound type is active.
	 * For example, this can be used to check whether any SFX sound
	 * is currently playing, by checking for type kSFXSoundType.
	 *
	 * @param  type the sound type to look for
	 * @return true if any channels of the specified type are active.
	 */
	virtual bool hasActiveChannelOfType(SoundType type) = 0;

	/**
	 * Set the volume for the given sound type.
	 *
	 * @param type the sound type
	 * @param volume the new global volume, 0 - kMaxMixerVolume
	 */
	virtual void setVolumeForSoundType(SoundType type, int volume) = 0;

	/**
	 * Query the global volume.
	 *
	 * @param type the sound type
	 * @return the global music volume, 0 - kMaxMixerVolume
	 */
	virtual int getVolumeForSoundType(SoundType type) const = 0;

	/**
	 * Query the system's audio output sample rate.
	 *
	 * @return the output sample rate in Hz
	 */
	virtual uint getOutputRate() const = 0;
};


} // End of namespace Audio

#endif
