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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_BASE_SOUND_H
#define WINTERMUTE_BASE_SOUND_H

#include "engines/wintermute/base/base.h"
#include "engines/wintermute/dctypes.h"    // Added by ClassView
#include "engines/wintermute/persistent.h"
#include "audio/mixer.h"

namespace Wintermute {

class BaseSoundBuffer;
class BaseSound : public BaseClass {
public:
	bool setPan(float pan);
	int getVolume();
	int getVolumePercent();
	bool setVolumePercent(int percent);
	bool setVolume(int volume);
	bool setPrivateVolume(int volume);
	bool setLoopStart(uint32 pos);
	uint32 getPositionTime();
	bool setPositionTime(uint32 time);
	bool isPlaying();
	bool isPaused();
	DECLARE_PERSISTENT(BaseSound, BaseClass)
	bool resume();
	bool pause(bool freezePaused = false);
	bool stop();
	bool play(bool looping = false);
	uint32 getLength();
	const char *getFilename() { return _soundFilename.c_str(); }
	bool setSoundSimple();
	bool setSound(const Common::String &filename, Audio::Mixer::SoundType type = Audio::Mixer::kSFXSoundType, bool streamed = false);
	BaseSound(BaseGame *inGame);
	virtual ~BaseSound();

	bool applyFX(TSFXType type = SFX_NONE, float param1 = 0, float param2 = 0, float param3 = 0, float param4 = 0);
private:
	Common::String _soundFilename;
	bool _soundStreamed;
	Audio::Mixer::SoundType _soundType;
	int _soundPrivateVolume;
	uint32 _soundLoopStart;
	uint32 _soundPosition;
	bool _soundPlaying;
	bool _soundLooping;
	bool _soundPaused;
	bool _soundFreezePaused;
	TSFXType _sFXType;
	float _sFXParam1;
	float _sFXParam2;
	float _sFXParam3;
	float _sFXParam4;
	BaseSoundBuffer *_sound;
};

} // end of namespace Wintermute

#endif
