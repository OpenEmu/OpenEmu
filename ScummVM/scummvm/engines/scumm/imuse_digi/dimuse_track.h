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
 */

#if !defined(SCUMM_IMUSE_DIGI_TRACK_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_IMUSE_DIGI_TRACK_H

#include "common/scummsys.h"

namespace Scumm {


// These flag bits correspond exactly to the sound mixer flags of March 2007.
// We don't want to use the mixer flags directly, because then our saved games
// will break in interesting ways if the mixer flags are ever assigned new
// values. Now they should keep working, as long as these flags don't change.

enum {
	kFlagUnsigned = 1 << 0,
	kFlag16Bits = 1 << 1,
	kFlagStereo = 1 << 3
};

struct Track {
	int trackId;		// used to identify track by value (0-15)

	int8 pan;			// panning value of sound
	int32 vol;			// volume level (values 0-127 * 1000)
	int32 volFadeDest;	// volume level which fading target (values 0-127 * 1000)
	int32 volFadeStep;	// delta of step while changing volume at each imuse callback
	int32 volFadeDelay;	// time in ms how long fading volume must be
	bool volFadeUsed;	// flag if fading is in progress

	int32 soundId;		// sound id used by scumm script
	char soundName[15]; // sound name but also filename of sound in bundle data
	bool used;			// flag mean that track is used
	bool toBeRemoved;   // flag mean that track need to be free
	bool souStreamUsed;	// flag mean that track use stream from sou file
	bool sndDataExtComp;// flag mean that sound data is compressed by scummvm tools
	int32 soundPriority;// priority level of played sound (0-127)
	int32 regionOffset; // offset to sound data relative to begining of current region
	int32 dataOffset;	// offset to sound data relative to begining of 'DATA' chunk
	int32 curRegion;	// id of current used region
	int32 curHookId;	// id of current used hook id
	int32 volGroupId;	// id of volume group (IMUSE_VOLGRP_VOICE, IMUSE_VOLGRP_SFX, IMUSE_VOLGRP_MUSIC)
	int32 soundType;	// type of sound data (kSpeechSoundType, kSFXSoundType, kMusicSoundType)
	int32 feedSize;		// size of sound data needed to be filled at each callback iteration
	int32 dataMod12Bit;	// value used between all callback to align 12 bit source of data
	int32 mixerFlags;	// flags for sound mixer's channel (kFlagStereo, kFlag16Bits, kFlagUnsigned)

	ImuseDigiSndMgr::SoundDesc *soundDesc;	// sound handle used by iMuse sound manager
	Audio::SoundHandle mixChanHandle;					// sound mixer's channel handle
	Audio::QueuingAudioStream *stream;		// sound mixer's audio stream handle for *.la1 and *.bun

	Track() : soundId(-1), used(false), stream(NULL) {
	}

	int getPan() const { return (pan != 64) ? 2 * pan - 127 : 0; }
	int getVol() const { return vol / 1000; }
	Audio::Mixer::SoundType getType() const {
		Audio::Mixer::SoundType type;
		if (volGroupId == 1)
			type = Audio::Mixer::kSpeechSoundType;
		else if (volGroupId == 2)
			type = Audio::Mixer::kSFXSoundType;
		else if (volGroupId == 3)
			type = Audio::Mixer::kMusicSoundType;
		else
			error("Track::getType(): invalid sound type");
		return type;
	}
};

} // End of namespace Scumm

#endif
