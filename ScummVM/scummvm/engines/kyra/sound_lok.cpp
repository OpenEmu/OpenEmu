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

#include "kyra/kyra_lok.h"
#include "kyra/sound.h"

#include "common/system.h"

namespace Kyra {

void KyraEngine_LoK::snd_playSoundEffect(int track, int volume) {
	if (_flags.platform == Common::kPlatformPC98) {
		if (track < 16 || track > 119)
			track = 58;
		else
			track -= 16;
	}

	if (_flags.platform == Common::kPlatformFMTowns && track == 49) {
		snd_playWanderScoreViaMap(56, 1);
		return;
	}

	KyraEngine_v1::snd_playSoundEffect(track);
}

void KyraEngine_LoK::snd_playWanderScoreViaMap(int command, int restart) {
	if (restart)
		_lastMusicCommand = -1;

	if (_flags.platform == Common::kPlatformFMTowns) {
		if (command >= 35 && command <= 38) {
			snd_playSoundEffect(command - 20);
		} else if (command >= 2) {
			if (_lastMusicCommand != command)
				// the original does -2 here we handle this inside _sound->playTrack()
				_sound->playTrack(command);
		} else {
			_sound->beginFadeOut();
		}
		_lastMusicCommand = command;
	} else if (_flags.platform == Common::kPlatformPC98) {
		if (command == 1) {
			_sound->beginFadeOut();
		} else if ((command >= 2 && command < 53) || command == 55) {
			if (_lastMusicCommand != command)
				_sound->playTrack(command);
		} else {
			_sound->haltTrack();
		}
		_lastMusicCommand = command;
	} else {
		KyraEngine_v1::snd_playWanderScoreViaMap(command, restart);
	}
}

void KyraEngine_LoK::snd_playVoiceFile(int id) {
	Common::String vocFile = Common::String::format("%03d", id);
	_speechPlayTime = _sound->voicePlay(vocFile.c_str(), &_speechHandle);
}

void KyraEngine_LoK::snd_voiceWaitForFinish(bool ingame) {
	while (_sound->voiceIsPlaying() && !skipFlag()) {
		if (ingame)
			delay(10, true);
		else
			_system->delayMillis(10);
	}
}

uint32 KyraEngine_LoK::snd_getVoicePlayTime() {
	if (!snd_voiceIsPlaying())
		return 0;
	return (_speechPlayTime != -1 ? _speechPlayTime : 0);
}

} // End of namespace Kyra
