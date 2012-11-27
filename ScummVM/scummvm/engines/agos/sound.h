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

#ifndef AGOS_SOUND_H
#define AGOS_SOUND_H

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "agos/intern.h"
#include "common/str.h"

namespace Common {
class File;
}

namespace AGOS {

class BaseSound;

class AGOSEngine;

class Sound {
private:
	AGOSEngine *_vm;

	Audio::Mixer *_mixer;

	BaseSound *_voice;
	BaseSound *_effects;

	bool _effectsPaused;
	bool _ambientPaused;
	bool _sfx5Paused;

	uint16 *_filenums;
	uint32 *_offsets;
	uint16 _lastVoiceFile;

	Audio::SoundHandle _voiceHandle;
	Audio::SoundHandle _effectsHandle;
	Audio::SoundHandle _ambientHandle;
	Audio::SoundHandle _sfx5Handle;

	bool _hasEffectsFile;
	bool _hasVoiceFile;
	uint16 _ambientPlaying;

	// Personal Nightmare specfic
	byte *_soundQueuePtr;
	uint16 _soundQueueNum;
	uint32 _soundQueueSize;
	uint16 _soundQueueFreq;
public:
	Sound(AGOSEngine *vm, const GameSpecificSettings *gss, Audio::Mixer *mixer);
	~Sound();

	enum TypeFlags {
		TYPE_AMBIENT = 1 << 0,
		TYPE_SFX     = 1 << 1,
		TYPE_SFX5    = 1 << 2
	};

protected:
	void loadVoiceFile(const GameSpecificSettings *gss);
	void loadSfxFile(const GameSpecificSettings *gss);

public:
	void readSfxFile(const Common::String &filename);
	void loadSfxTable(const char *gameFilename, uint32 base);
	void readVoiceFile(const Common::String &filename);

	void playVoice(uint sound);
	void playEffects(uint sound);
	void playAmbient(uint sound);

	// Personal Nightmare specfic
	void handleSoundQueue();
	void queueSound(byte *ptr, uint16 sound, uint32 size, uint16 freq);

	// Elvira 1/2 and Waxworks specific
	void playRawData(byte *soundData, uint sound, uint size, uint freq);

	// Feeble Files specific
	void playAmbientData(byte *soundData, uint sound, uint pan, uint vol);
	void playSfxData(byte *soundData, uint sound, uint pan, uint vol);
	void playSfx5Data(byte *soundData, uint sound, uint pan, uint vol);
	void playSoundData(Audio::SoundHandle *handle, byte *soundData, uint sound, int pan = 0, int vol = 0, bool loop = false);
	void playVoiceData(byte *soundData, uint sound);
	void switchVoiceFile(const GameSpecificSettings *gss, uint disc);

	bool hasVoice() const;
	bool isSfxActive() const;
	bool isVoiceActive() const;
	void stopAllSfx();
	void stopSfx();
	void stopSfx5();
	void stopVoice();
	void stopAll();
	void effectsPause(bool b);
	void ambientPause(bool b);
};

} // End of namespace AGOS

#endif
