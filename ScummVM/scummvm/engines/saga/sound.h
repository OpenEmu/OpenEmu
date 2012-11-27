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

// Sound class

#ifndef SAGA_SOUND_H
#define SAGA_SOUND_H

#include "common/file.h"
#include "audio/mixer.h"
#include "audio/timestamp.h"

namespace Audio {
class RewindableAudioStream;
}

namespace Saga {

#define SOUND_HANDLES 10

enum SOUND_FLAGS {
	SOUND_LOOP = 1
};

struct SoundBuffer {
	Audio::RewindableAudioStream *stream;
	Audio::Timestamp streamLength;
};

enum sndHandleType {
	kFreeHandle,
	kEffectHandle,
	kVoiceHandle
};

struct SndHandle {
	Audio::SoundHandle handle;
	sndHandleType type;
	int resId;
};

class Sound {
public:

	Sound(SagaEngine *vm, Audio::Mixer *mixer);
	~Sound();

	void playSound(SoundBuffer &buffer, int volume, bool loop, int resId);
	void pauseSound();
	void resumeSound();
	void stopSound();

	void playVoice(SoundBuffer &buffer);
	void pauseVoice();
	void resumeVoice();
	void stopVoice();

	void stopAll();

	void setVolume();

 private:

	void playSoundBuffer(Audio::SoundHandle *handle, const SoundBuffer &buffer, int volume,
				sndHandleType handleType, bool loop);

	SndHandle *getHandle();

	SagaEngine *_vm;
	Audio::Mixer *_mixer;

	SndHandle _handles[SOUND_HANDLES];
};

} // End of namespace Saga

#endif
