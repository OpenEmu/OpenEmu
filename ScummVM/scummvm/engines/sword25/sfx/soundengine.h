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
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

/*
 * BS_SoundEngine
 * -------------
 * This is the sound engine interface that contains all the methods a sound
 * engine must implement.
 * Implementations of the sound engine have to be derived from this class.
 * It should be noted that a sound engine must maintain a list of all the
 * samples it uses, and that these samples should be released when the
 * destructor is called.
 *
 * Autor: Malte Thiesen
 */

#ifndef SWORD25_SOUNDENGINE_H
#define SWORD25_SOUNDENGINE_H

#include "sword25/kernel/common.h"
#include "sword25/kernel/resservice.h"
#include "sword25/kernel/persistable.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace Sword25 {

#define SOUND_HANDLES 32

enum sndHandleType {
	kFreeHandle,
	kAllocatedHandle
};

struct SndHandle {
	Audio::SoundHandle handle;
	sndHandleType type;
	uint32 id;

	Common::String fileName;
	int sndType;
	float volume;
	float pan;
	bool loop;
	int loopStart;
	int loopEnd;
	uint layer;
};


class SoundEngine : public ResourceService, public Persistable {
public:
	enum SOUND_TYPES {
		MUSIC = 0,
		SPEECH = 1,
		SFX = 2
	};

	/**
	 * The callback function of PlayDynamicSoundEx
	 * @param UserData      User-specified pointer
	 * @param Data          Pointer to the data buffer
	 * @param DataLength    Length of the data to be written in bytes
	*/
	typedef void (*DynamicSoundReadCallback)(void *UserData, void *Data, uint DataLength);

	SoundEngine(Kernel *pKernel);
	~SoundEngine() {}

	/**
	 * Initializes the sound engine
	 * @param SampleRate    Specifies the sample rate to use.
	 * @param Channels      The maximum number of channels. The default is 32.
	 * @return              Returns true on success, otherwise false.
	 * @remark              Calls to other methods may take place only if this
	 * method was called successfully.
	 */
	bool init(uint sampleRate, uint channels = 32);

	/**
	 * Performs a "tick" of the sound engine
	 *
	 * This method should be called once per frame. It can be used by implementations
	 * of the sound engine that are not running in their own thread, or to perform
	 * additional administrative tasks that are needed.
	 */
	void update();

	/**
	 * Sets the default volume for the different sound types
	 * @param Volume        The default volume level (0 = off, 1 = full volume)
	 * @param Type          The SoundType whose volume is to be changed
	 */
	void setVolume(float volume, SOUND_TYPES type);

	/**
	 * Specifies the default volume of different sound types
	 * @param Type          The SoundType
	 * @return              Returns the standard sound volume for the given type
	 * (0 = off, 1 = full volume).
	*/
	float getVolume(SOUND_TYPES type);

	/**
	 * Pauses all the sounds that are playing.
	 */
	void pauseAll();

	/**
	 * Resumes all currently stopped sounds
	 */
	void resumeAll();

	/**
	 * Pauses all sounds of a given layer.
	 * @param Layer         The Sound Layer
	*/
	void pauseLayer(uint layer);

	/**
	 * Resumes all the sounds in a layer that was previously stopped with PauseLayer()
	 * @param Layer         The Sound Layer
	*/
	void resumeLayer(uint layer);

	/**
	 * Plays a sound
	 * @param FileName      The filename of the sound to be played
	 * @param Type          The type of sound
	 * @param Volume        The volume of the sound (0 = off, 1 = full volume)
	 * @param Pan           Panning (-1 = full left, 1 = right)
	 * @param Loop          Indicates whether the sound should be looped
	 * @param LoopStart     Indicates the starting loop point. If a value less than 0 is passed, the start
	 * of the sound is used.
	 * @param LoopEnd       Indicates the ending loop point. If a avlue is passed less than 0, the end of
	 * the sound is used.
	 * @param Layer         The sound layer
	 * @return              Returns true if the playback of the sound was started successfully.
	 * @remark              If more control is needed over the playing, eg. changing the sound parameters
	 * for Volume and Panning, then PlaySoundEx should be used.
	*/
	bool playSound(const Common::String &fileName, SOUND_TYPES type, float volume = 1.0f, float pan = 0.0f, bool loop = false, int loopStart = -1, int loopEnd = -1, uint layer = 0);

	/**
	 * Plays a sound
	 * @param Type          The type of sound
	 * @param Volume        The volume of the sound (0 = off, 1 = full volume)
	 * @param Pan           Panning (-1 = full left, 1 = right)
	 * @param Loop          Indicates whether the sound should be looped
	 * @param LoopStart     Indicates the starting loop point. If a value less than 0 is passed, the start
	 * of the sound is used.
	 * @param LoopEnd       Indicates the ending loop point. If a avlue is passed less than 0, the end of
	 * the sound is used.
	 * @param Layer         The sound layer
	 * @return              Returns a handle to the sound. With this handle, the sound can be manipulated during playback.
	 * @remark              If more control is needed over the playing, eg. changing the sound parameters
	 * for Volume and Panning, then PlaySoundEx should be used.
	 */
	uint playSoundEx(const Common::String &fileName, SOUND_TYPES type, float volume = 1.0f, float pan = 0.0f, bool loop = false, int loopStart = -1, int loopEnd = -1, uint layer = 0, uint handleId = 0x1337);

	/**
	 * Sets the volume of a playing sound
	 * @param Handle        The sound handle
	 * @param Volume        The volume of the sound (0 = off, 1 = full volume)
	 */
	void setSoundVolume(uint handle, float volume);

	/**
	 * Sets the panning of a playing sound
	 * @param Handle        The sound handle
	 * @param Pan           Panning (-1 = full left, 1 = right)
	 */
	void setSoundPanning(uint handle, float pan);

	/**
	 * Pauses a playing sound
	 * @param Handle        The sound handle
	 */
	void pauseSound(uint handle);

	/**
	 * Resumes a paused sound
	 * @param Handle        The sound handle
	 */
	void resumeSound(uint handle);

	/**
	 * Stops a playing sound
	 * @param Handle        The sound handle
	 * @remark              Calling this method invalidates the passed handle; it can no longer be used.
	 */
	void stopSound(uint handle);

	/**
	 * Returns whether a sound is paused
	 * @param Handle        The sound handle
	 * @return              Returns true if the sound is paused, false otherwise.
	 */
	bool isSoundPaused(uint handle);

	/**
	 * Returns whether a sound is still playing.
	 * @param Handle        The sound handle
	 * @return              Returns true if the sound is playing, false otherwise.
	*/
	bool isSoundPlaying(uint handle);

	/**
	 * Returns the volume of a playing sound (0 = off, 1 = full volume)
	 */
	float getSoundVolume(uint handle);

	/**
	 * Returns the panning of a playing sound (-1 = full left, 1 = right)
	 */
	float getSoundPanning(uint handle);

	Resource    *loadResource(const Common::String &fileName);
	bool         canLoadResource(const Common::String &fileName);

	bool persist(OutputPersistenceBlock &writer);
	bool unpersist(InputPersistenceBlock &reader);

private:
	bool registerScriptBindings();
	SndHandle *getHandle(uint *id);
	SndHandle *findHandle(uint id);

private:
	Audio::Mixer *_mixer;
	SndHandle _handles[SOUND_HANDLES];

	uint32 _maxHandleId;
};

} // End of namespace Sword25

#endif
