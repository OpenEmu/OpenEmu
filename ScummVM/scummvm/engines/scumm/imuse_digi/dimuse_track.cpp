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

#include "common/config-manager.h"
#include "common/timer.h"

#include "scumm/actor.h"
#include "scumm/scumm_v7.h"
#include "scumm/sound.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"
#include "scumm/imuse_digi/dimuse_track.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace Scumm {

int IMuseDigital::allocSlot(int priority) {
	int l, lowest_priority = 127;
	int trackId = -1;

	for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		if (!_track[l]->used) {
			trackId = l;
			break;
		}
	}

	if (trackId == -1) {
		debug(5, "IMuseDigital::allocSlot(): All slots are full");
		for (l = 0; l < MAX_DIGITAL_TRACKS; l++) {
			Track *track = _track[l];
			if (track->used && !track->toBeRemoved &&
					(lowest_priority > track->soundPriority) && !track->souStreamUsed) {
				lowest_priority = track->soundPriority;
				trackId = l;
			}
		}
		if (lowest_priority <= priority) {
			assert(trackId != -1);
			Track *track = _track[trackId];

			// Stop the track immediately
			_mixer->stopHandle(track->mixChanHandle);
			if (track->soundDesc) {
				_sound->closeSound(track->soundDesc);
			}

			// Mark it as unused
			memset(track, 0, sizeof(Track));

			debug(5, "IMuseDigital::allocSlot(): Removed sound %d from track %d", _track[trackId]->soundId, trackId);
		} else {
			debug(5, "IMuseDigital::allocSlot(): Priority sound too low");
			return -1;
		}
	}

	return trackId;
}

void IMuseDigital::startSound(int soundId, const char *soundName, int soundType, int volGroupId, Audio::AudioStream *input, int hookId, int volume, int priority, Track *otherTrack) {
	Common::StackLock lock(_mutex, "IMuseDigital::startSound()");
	debug(5, "IMuseDigital::startSound(%d) - begin func", soundId);

	int l = allocSlot(priority);
	if (l == -1) {
		warning("IMuseDigital::startSound() Can't start sound - no free slots");
		return;
	}
	debug(5, "IMuseDigital::startSound(%d, trackId:%d)", soundId, l);

	Track *track = _track[l];

	// Reset the track
	memset(track, 0, sizeof(Track));

	track->pan = 64;
	track->vol = volume * 1000;
	track->soundId = soundId;
	track->volGroupId = volGroupId;
	track->curHookId = hookId;
	track->soundPriority = priority;
	track->curRegion = -1;
	track->soundType = soundType;
	track->trackId = l;

	int bits = 0, freq = 0, channels = 0;

	track->souStreamUsed = (input != 0);

	if (track->souStreamUsed) {
		_mixer->playStream(track->getType(), &track->mixChanHandle, input, -1, track->getVol(), track->getPan(),
							DisposeAfterUse::YES, false, (track->mixerFlags & kFlagStereo) != 0);
	} else {
		strcpy(track->soundName, soundName);
		track->soundDesc = _sound->openSound(soundId, soundName, soundType, volGroupId, -1);
		if (!track->soundDesc)
			track->soundDesc = _sound->openSound(soundId, soundName, soundType, volGroupId, 1);
		if (!track->soundDesc)
			track->soundDesc = _sound->openSound(soundId, soundName, soundType, volGroupId, 2);

		if (!track->soundDesc)
			return;

		track->sndDataExtComp = _sound->isSndDataExtComp(track->soundDesc);

		bits = _sound->getBits(track->soundDesc);
		channels = _sound->getChannels(track->soundDesc);
		freq = _sound->getFreq(track->soundDesc);

		if ((soundId == kTalkSoundID) && (soundType == IMUSE_BUNDLE)) {
			if (_vm->_actorToPrintStrFor != 0xFF && _vm->_actorToPrintStrFor != 0) {
				Actor *a = _vm->derefActor(_vm->_actorToPrintStrFor, "IMuseDigital::startSound");
				freq = (freq * a->_talkFrequency) / 256;
				track->pan = a->_talkPan;
				track->vol = a->_talkVolume * 1000;
			}

			// The volume is set to zero, when using subtitles only setting in COMI
			if (ConfMan.getBool("speech_mute") || _vm->VAR(_vm->VAR_VOICE_MODE) == 2) {
				track->vol = 0;
			}
		}

		assert(bits == 8 || bits == 12 || bits == 16);
		assert(channels == 1 || channels == 2);
		assert(0 < freq && freq <= 65535);

		track->feedSize = freq * channels;
		if (channels == 2)
			track->mixerFlags = kFlagStereo;

		if ((bits == 12) || (bits == 16)) {
			track->mixerFlags |= kFlag16Bits;
			track->feedSize *= 2;
		} else if (bits == 8) {
			track->mixerFlags |= kFlagUnsigned;
		} else
			error("IMuseDigital::startSound(): Can't handle %d bit samples", bits);

		if (otherTrack && otherTrack->used && !otherTrack->toBeRemoved) {
			track->curRegion = otherTrack->curRegion;
			track->dataOffset = otherTrack->dataOffset;
			track->regionOffset = otherTrack->regionOffset;
			track->dataMod12Bit = otherTrack->dataMod12Bit;
		}

		track->stream = Audio::makeQueuingAudioStream(freq, track->mixerFlags & kFlagStereo);
		_mixer->playStream(track->getType(), &track->mixChanHandle, track->stream, -1, track->getVol(), track->getPan(),
							DisposeAfterUse::YES, false, (track->mixerFlags & kFlagStereo) != 0);
	}

	track->used = true;
}

void IMuseDigital::setPriority(int soundId, int priority) {
	Common::StackLock lock(_mutex, "IMuseDigital::setPriority()");
	debug(5, "IMuseDigital::setPriority(%d, %d)", soundId, priority);
	assert ((priority >= 0) && (priority <= 127));

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->soundId == soundId)) {
			debug(5, "IMuseDigital::setPriority(%d) - setting", soundId);
			track->soundPriority = priority;
		}
	}
}

void IMuseDigital::setVolume(int soundId, int volume) {
	Common::StackLock lock(_mutex, "IMuseDigital::setVolume()");
	debug(5, "IMuseDigital::setVolume(%d, %d)", soundId, volume);

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->soundId == soundId)) {
			debug(5, "IMuseDigital::setVolume(%d) - setting", soundId);
			track->vol = volume * 1000;
		}
	}
}

void IMuseDigital::setHookId(int soundId, int hookId) {
	Common::StackLock lock(_mutex, "IMuseDigital::setHookId()");

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->soundId == soundId)) {
			track->curHookId = hookId;
		}
	}
}

int IMuseDigital::getCurMusicSoundId() {
	Common::StackLock lock(_mutex, "IMuseDigital::getCurMusicSoundId()");
	int soundId = -1;

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			soundId = track->soundId;
			break;
		}
	}

	return soundId;
}

void IMuseDigital::setPan(int soundId, int pan) {
	Common::StackLock lock(_mutex, "IMuseDigital::setPan()");
	debug(5, "IMuseDigital::setPan(%d, %d)", soundId, pan);

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->soundId == soundId)) {
			debug(5, "IMuseDigital::setPan(%d) - setting", soundId);
			track->pan = pan;
		}
	}
}

void IMuseDigital::selectVolumeGroup(int soundId, int volGroupId) {
	Common::StackLock lock(_mutex, "IMuseDigital::selectVolumeGroup()");
	debug(5, "IMuseDigital::setGroupVolume(%d, %d)", soundId, volGroupId);
	assert((volGroupId >= 1) && (volGroupId <= 4));

	if (volGroupId == 4)
		volGroupId = 3;

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->soundId == soundId)) {
			debug(5, "IMuseDigital::setVolumeGroup(%d) - setting", soundId);
			track->volGroupId = volGroupId;
		}
	}
}

void IMuseDigital::setFade(int soundId, int destVolume, int delay60HzTicks) {
	Common::StackLock lock(_mutex, "IMuseDigital::setFade()");
	debug(5, "IMuseDigital::setFade(%d, %d, %d)", soundId, destVolume, delay60HzTicks);

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->soundId == soundId)) {
			debug(5, "IMuseDigital::setFade(%d) - setting", soundId);
			track->volFadeDelay = delay60HzTicks;
			track->volFadeDest = destVolume * 1000;
			track->volFadeStep = (track->volFadeDest - track->vol) * 60 * (1000 / _callbackFps) / (1000 * delay60HzTicks);
			track->volFadeUsed = true;
		}
	}
}

void IMuseDigital::fadeOutMusicAndStartNew(int fadeDelay, const char *filename, int soundId) {
	Common::StackLock lock(_mutex, "IMuseDigital::fadeOutMusicAndStartNew()");
	debug(5, "IMuseDigital::fadeOutMusicAndStartNew(fade:%d, file:%s, sound:%d)", fadeDelay, filename, soundId);

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			debug(5, "IMuseDigital::fadeOutMusicAndStartNew(sound:%d) - starting", soundId);
			startMusicWithOtherPos(filename, soundId, 0, 127, track);
			cloneToFadeOutTrack(track, fadeDelay);
			flushTrack(track);
			break;
		}
	}
}

void IMuseDigital::fadeOutMusic(int fadeDelay) {
	Common::StackLock lock(_mutex, "IMuseDigital::fadeOutMusic()");
	debug(5, "IMuseDigital::fadeOutMusic(fade:%d)", fadeDelay);

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			debug(5, "IMuseDigital::fadeOutMusic(fade:%d, sound:%d)", fadeDelay, track->soundId);
			cloneToFadeOutTrack(track, fadeDelay);
			flushTrack(track);
			break;
		}
	}
}

void IMuseDigital::setHookIdForMusic(int hookId) {
	Common::StackLock lock(_mutex, "IMuseDigital::setHookIdForMusic()");
	debug(5, "IMuseDigital::setHookIdForMusic(hookId:%d)", hookId);

	for (int l = 0; l < MAX_DIGITAL_TRACKS; l++) {
		Track *track = _track[l];
		if (track->used && !track->toBeRemoved && (track->volGroupId == IMUSE_VOLGRP_MUSIC)) {
			debug(5, "IMuseDigital::setHookIdForMusic - setting for sound:%d", track->soundId);
			track->curHookId = hookId;
			break;
		}
	}
}

void IMuseDigital::setTrigger(TriggerParams *trigger) {
	Common::StackLock lock(_mutex, "IMuseDigital::setTrigger()");
	debug(5, "IMuseDigital::setTrigger(%s)", trigger->filename);

	memcpy(&_triggerParams, trigger, sizeof(TriggerParams));
	_triggerUsed = true;
}

Track *IMuseDigital::cloneToFadeOutTrack(Track *track, int fadeDelay) {
	assert(track);
	Track *fadeTrack;

	debug(5, "cloneToFadeOutTrack(soundId:%d, fade:%d) - begin of func", track->trackId, fadeDelay);

	if (track->toBeRemoved) {
		error("cloneToFadeOutTrack: Tried to clone a track to be removed, please bug report");
		return NULL;
	}

	assert(track->trackId < MAX_DIGITAL_TRACKS);
	fadeTrack = _track[track->trackId + MAX_DIGITAL_TRACKS];

	if (fadeTrack->used) {
		debug(5, "cloneToFadeOutTrack: No free fade track, force flush fade soundId:%d", fadeTrack->soundId);
		flushTrack(fadeTrack);
		_mixer->stopHandle(fadeTrack->mixChanHandle);
	}

	// Clone the settings of the given track
	memcpy(fadeTrack, track, sizeof(Track));
	fadeTrack->trackId = track->trackId + MAX_DIGITAL_TRACKS;

	// Clone the sound.
	// leaving bug number for now #1635361
	ImuseDigiSndMgr::SoundDesc *soundDesc = _sound->cloneSound(track->soundDesc);
	if (!soundDesc) {
		// it fail load open old song after switch to diffrent CDs
		// so gave up
		error("Game not supported while playing on 2 diffrent CDs");
	}
	track->soundDesc = soundDesc;

	// Set the volume fading parameters to indicate a fade out
	fadeTrack->volFadeDelay = fadeDelay;
	fadeTrack->volFadeDest = 0;
	fadeTrack->volFadeStep = (fadeTrack->volFadeDest - fadeTrack->vol) * 60 * (1000 / _callbackFps) / (1000 * fadeDelay);
	fadeTrack->volFadeUsed = true;

	// Create an appendable output buffer
	fadeTrack->stream = Audio::makeQueuingAudioStream(_sound->getFreq(fadeTrack->soundDesc), track->mixerFlags & kFlagStereo);
	_mixer->playStream(track->getType(), &fadeTrack->mixChanHandle, fadeTrack->stream, -1, fadeTrack->getVol(), fadeTrack->getPan(),
							DisposeAfterUse::YES, false, (track->mixerFlags & kFlagStereo) != 0);
	fadeTrack->used = true;

	debug(5, "cloneToFadeOutTrack() - end of func, soundId %d, fade soundId %d", track->soundId, fadeTrack->soundId);

	return fadeTrack;
}

} // End of namespace Scumm
