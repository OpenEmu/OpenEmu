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

#include "common/system.h"
#include "common/timer.h"

#include "scumm/actor.h"
#include "scumm/saveload.h"
#include "scumm/scumm_v7.h"
#include "scumm/sound.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"
#include "scumm/imuse_digi/dimuse_codecs.h"
#include "scumm/imuse_digi/dimuse_track.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"

namespace Scumm {

void IMuseDigital::timer_handler(void *refCon) {
	IMuseDigital *imuseDigital = (IMuseDigital *)refCon;
	imuseDigital->callback();
}

IMuseDigital::IMuseDigital(ScummEngine_v7 *scumm, Audio::Mixer *mixer, int fps)
	: _vm(scumm), _mixer(mixer) {
	assert(_vm);
	assert(mixer);

	_pause = false;
	_sound = new ImuseDigiSndMgr(_vm);
	assert(_sound);
	_callbackFps = fps;
	resetState();
	for (int l = 0; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		_track[l] = new Track;
		assert(_track[l]);
		memset(_track[l], 0, sizeof(Track));
		_track[l]->trackId = l;
	}
	_vm->getTimerManager()->installTimerProc(timer_handler, 1000000 / _callbackFps, this, "IMuseDigital");

	_audioNames = NULL;
	_numAudioNames = 0;
}

IMuseDigital::~IMuseDigital() {
	_vm->getTimerManager()->removeTimerProc(timer_handler);
	stopAllSounds();
	for (int l = 0; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		delete _track[l];
	}
	delete _sound;
	free(_audioNames);
}

static int32 makeMixerFlags(Track *track) {
	const int32 flags = track->mixerFlags;
	int32 mixerFlags = 0;
	if (flags & kFlagUnsigned)
		mixerFlags |= Audio::FLAG_UNSIGNED;
	if (flags & kFlag16Bits)
		mixerFlags |= Audio::FLAG_16BITS;

#ifdef SCUMM_LITTLE_ENDIAN
	if (track->sndDataExtComp)
		mixerFlags |= Audio::FLAG_LITTLE_ENDIAN;
#endif
	if (flags & kFlagStereo)
		mixerFlags |= Audio::FLAG_STEREO;
	return mixerFlags;
}

void IMuseDigital::resetState() {
	_curMusicState = 0;
	_curMusicSeq = 0;
	_curMusicCue = 0;
	memset(_attributes, 0, sizeof(_attributes));
	_nextSeqToPlay = 0;
	_stopingSequence = 0;
	_radioChatterSFX = 0;
	_triggerUsed = false;
}

void IMuseDigital::saveOrLoad(Serializer *ser) {
	Common::StackLock lock(_mutex, "IMuseDigital::saveOrLoad()");

	const SaveLoadEntry mainEntries[] = {
		MK_OBSOLETE(IMuseDigital, _volVoice, sleInt32, VER(31), VER(42)),
		MK_OBSOLETE(IMuseDigital, _volSfx, sleInt32, VER(31), VER(42)),
		MK_OBSOLETE(IMuseDigital, _volMusic, sleInt32, VER(31), VER(42)),
		MKLINE(IMuseDigital, _curMusicState, sleInt32, VER(31)),
		MKLINE(IMuseDigital, _curMusicSeq, sleInt32, VER(31)),
		MKLINE(IMuseDigital, _curMusicCue, sleInt32, VER(31)),
		MKLINE(IMuseDigital, _nextSeqToPlay, sleInt32, VER(31)),
		MKLINE(IMuseDigital, _radioChatterSFX, sleByte, VER(76)),
		MKARRAY(IMuseDigital, _attributes[0], sleInt32, 188, VER(31)),
		MKEND()
	};

	const SaveLoadEntry trackEntries[] = {
		MKLINE(Track, pan, sleInt8, VER(31)),
		MKLINE(Track, vol, sleInt32, VER(31)),
		MKLINE(Track, volFadeDest, sleInt32, VER(31)),
		MKLINE(Track, volFadeStep, sleInt32, VER(31)),
		MKLINE(Track, volFadeDelay, sleInt32, VER(31)),
		MKLINE(Track, volFadeUsed, sleByte, VER(31)),
		MKLINE(Track, soundId, sleInt32, VER(31)),
		MKARRAY(Track, soundName[0], sleByte, 15, VER(31)),
		MKLINE(Track, used, sleByte, VER(31)),
		MKLINE(Track, toBeRemoved, sleByte, VER(31)),
		MKLINE(Track, souStreamUsed, sleByte, VER(31)),
		MK_OBSOLETE(Track, mixerStreamRunning, sleByte, VER(31), VER(76)),
		MKLINE(Track, soundPriority, sleInt32, VER(31)),
		MKLINE(Track, regionOffset, sleInt32, VER(31)),
		MK_OBSOLETE(Track, trackOffset, sleInt32, VER(31), VER(31)),
		MKLINE(Track, dataOffset, sleInt32, VER(31)),
		MKLINE(Track, curRegion, sleInt32, VER(31)),
		MKLINE(Track, curHookId, sleInt32, VER(31)),
		MKLINE(Track, volGroupId, sleInt32, VER(31)),
		MKLINE(Track, soundType, sleInt32, VER(31)),
		MKLINE(Track, feedSize, sleInt32, VER(31)),
		MKLINE(Track, dataMod12Bit, sleInt32, VER(31)),
		MKLINE(Track, mixerFlags, sleInt32, VER(31)),
		MK_OBSOLETE(Track, mixerVol, sleInt32, VER(31), VER(42)),
		MK_OBSOLETE(Track, mixerPan, sleInt32, VER(31), VER(42)),
		MKLINE(Track, sndDataExtComp, sleByte, VER(45)),
		MKEND()
	};

	ser->saveLoadEntries(this, mainEntries);

	for (int l = 0; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		Track *track = _track[l];
		if (ser->isLoading()) {
			memset(track, 0, sizeof(Track));
		}
		ser->saveLoadEntries(track, trackEntries);
		if (ser->isLoading()) {
			_track[l]->trackId = l;
			if (!track->used)
				continue;
			if ((track->toBeRemoved) || (track->souStreamUsed) || (track->curRegion == -1)) {
				track->used = false;
				continue;
			}

			// TODO: The code below has a lot in common with that in IMuseDigital::startSound.
			// Try to refactor them to reduce the code duplication.

			track->soundDesc = _sound->openSound(track->soundId, track->soundName, track->soundType, track->volGroupId, -1);
			if (!track->soundDesc)
				track->soundDesc = _sound->openSound(track->soundId, track->soundName, track->soundType, track->volGroupId, 1);
			if (!track->soundDesc)
				track->soundDesc = _sound->openSound(track->soundId, track->soundName, track->soundType, track->volGroupId, 2);

			if (!track->soundDesc) {
				warning("IMuseDigital::saveOrLoad: Can't open sound so will not be resumed");
				track->used = false;
				continue;
			}

			track->sndDataExtComp = _sound->isSndDataExtComp(track->soundDesc);
			track->dataOffset = _sound->getRegionOffset(track->soundDesc, track->curRegion);
			int bits = _sound->getBits(track->soundDesc);
			int channels = _sound->getChannels(track->soundDesc);
			int freq = _sound->getFreq(track->soundDesc);
			track->feedSize = freq * channels;
			track->mixerFlags = 0;
			if (channels == 2)
				track->mixerFlags = kFlagStereo;

			if ((bits == 12) || (bits == 16)) {
				track->mixerFlags |= kFlag16Bits;
				track->feedSize *= 2;
			} else if (bits == 8) {
				track->mixerFlags |= kFlagUnsigned;
			} else
				error("IMuseDigital::saveOrLoad(): Can't handle %d bit samples", bits);

			track->stream = Audio::makeQueuingAudioStream(freq, (track->mixerFlags & kFlagStereo) != 0);

			_mixer->playStream(track->getType(), &track->mixChanHandle, track->stream, -1, track->getVol(), track->getPan(),
							DisposeAfterUse::YES, false, (track->mixerFlags & kFlagStereo) != 0);
			_mixer->pauseHandle(track->mixChanHandle, true);
		}
	}
}

void IMuseDigital::callback() {
	Common::StackLock lock(_mutex, "IMuseDigital::callback()");

	for (int l = 0; l < MAX_DIGITAL_TRACKS + MAX_DIGITAL_FADETRACKS; l++) {
		Track *track = _track[l];
		if (track->used) {
			// Ignore tracks which are about to finish. Also, if it did finish in the meantime,
			// mark it as unused.
			if (!track->stream) {
				if (!_mixer->isSoundHandleActive(track->mixChanHandle))
					memset(track, 0, sizeof(Track));
				continue;
			}

			if (_pause)
				return;

			if (track->volFadeUsed) {
				if (track->volFadeStep < 0) {
					if (track->vol > track->volFadeDest) {
						track->vol += track->volFadeStep;
						if (track->vol < track->volFadeDest) {
							track->vol = track->volFadeDest;
							track->volFadeUsed = false;
						}
						if (track->vol == 0) {
							// Fade out complete -> remove this track
							flushTrack(track);
							continue;
						}
					}
				} else if (track->volFadeStep > 0) {
					if (track->vol < track->volFadeDest) {
						track->vol += track->volFadeStep;
						if (track->vol > track->volFadeDest) {
							track->vol = track->volFadeDest;
							track->volFadeUsed = false;
						}
					}
				}
				debug(5, "Fade: sound(%d), Vol(%d)", track->soundId, track->vol / 1000);
			}

			if (!track->souStreamUsed) {
				assert(track->stream);
				byte *tmpSndBufferPtr = NULL;
				int32 curFeedSize = 0;

				if (track->curRegion == -1) {
					switchToNextRegion(track);
					if (!track->stream)	// Seems we reached the end of the stream
						continue;
				}

				int bits = _sound->getBits(track->soundDesc);
				int channels = _sound->getChannels(track->soundDesc);

				int32 feedSize = track->feedSize / _callbackFps;

				if (track->stream->endOfData()) {
					feedSize *= 2;
				}

				if ((bits == 12) || (bits == 16)) {
					if (channels == 1)
						feedSize &= ~1;
					if (channels == 2)
						feedSize &= ~3;
				} else {
					if (channels == 2)
						feedSize &= ~1;
				}

				if (feedSize == 0)
					continue;

				do {
					if (bits == 12) {
						byte *tmpPtr = NULL;

						feedSize += track->dataMod12Bit;
						int tmpFeedSize12Bits = (feedSize * 3) / 4;
						int tmpLength12Bits = (tmpFeedSize12Bits / 3) * 4;
						track->dataMod12Bit = feedSize - tmpLength12Bits;

						int32 tmpOffset = (track->regionOffset * 3) / 4;
						int tmpFeedSize = _sound->getDataFromRegion(track->soundDesc, track->curRegion, &tmpPtr, tmpOffset, tmpFeedSize12Bits);
						curFeedSize = BundleCodecs::decode12BitsSample(tmpPtr, &tmpSndBufferPtr, tmpFeedSize);

						free(tmpPtr);
					} else if (bits == 16) {
						curFeedSize = _sound->getDataFromRegion(track->soundDesc, track->curRegion, &tmpSndBufferPtr, track->regionOffset, feedSize);
						if (channels == 1) {
							curFeedSize &= ~1;
						}
						if (channels == 2) {
							curFeedSize &= ~3;
						}
					} else if (bits == 8) {
						curFeedSize = _sound->getDataFromRegion(track->soundDesc, track->curRegion, &tmpSndBufferPtr, track->regionOffset, feedSize);
						if (_radioChatterSFX && track->soundId == 10000) {
							if (curFeedSize > feedSize)
								curFeedSize = feedSize;
							byte *buf = (byte *)malloc(curFeedSize);
							int index = 0;
							int count = curFeedSize - 4;
							byte *ptr_1 = tmpSndBufferPtr;
							byte *ptr_2 = tmpSndBufferPtr + 4;
							int value = ptr_1[0] - 0x80;
							value += ptr_1[1] - 0x80;
							value += ptr_1[2] - 0x80;
							value += ptr_1[3] - 0x80;
							do {
								int t = *ptr_1++;
								int v = t - (value / 4);
								value = *ptr_2++ - 0x80 + (value - t + 0x80);
								buf[index++] = v * 2 + 0x80;
							} while (--count);
							buf[curFeedSize - 1] = 0x80;
							buf[curFeedSize - 2] = 0x80;
							buf[curFeedSize - 3] = 0x80;
							buf[curFeedSize - 4] = 0x80;
							free(tmpSndBufferPtr);
							tmpSndBufferPtr = buf;
						}
						if (channels == 2) {
							curFeedSize &= ~1;
						}
					}

					if (curFeedSize > feedSize)
						curFeedSize = feedSize;

					if (_mixer->isReady()) {
						track->stream->queueBuffer(tmpSndBufferPtr, curFeedSize, DisposeAfterUse::YES, makeMixerFlags(track));
						track->regionOffset += curFeedSize;
					} else
						free(tmpSndBufferPtr);

					if (_sound->isEndOfRegion(track->soundDesc, track->curRegion)) {
						switchToNextRegion(track);
						if (!track->stream)	// Seems we reached the end of the stream
							break;
					}
					feedSize -= curFeedSize;
					assert(feedSize >= 0);
				} while (feedSize != 0);
			}
			if (_mixer->isReady()) {
				_mixer->setChannelVolume(track->mixChanHandle, track->getVol());
				_mixer->setChannelBalance(track->mixChanHandle, track->getPan());
			}
		}
	}
}

void IMuseDigital::switchToNextRegion(Track *track) {
	assert(track);

	if (track->trackId >= MAX_DIGITAL_TRACKS) {
		flushTrack(track);
		debug(5, "SwToNeReg(trackId:%d) - fadetrack can't go next region, exiting SwToNeReg", track->trackId);
		return;
	}

	int num_regions = _sound->getNumRegions(track->soundDesc);

	if (++track->curRegion == num_regions) {
		flushTrack(track);
		debug(5, "SwToNeReg(trackId:%d) - end of region, exiting SwToNeReg", track->trackId);
		return;
	}

	ImuseDigiSndMgr::SoundDesc *soundDesc = track->soundDesc;
	if (_triggerUsed && track->soundDesc->numMarkers) {
		if (_sound->checkForTriggerByRegionAndMarker(soundDesc, track->curRegion, _triggerParams.marker)) {
			debug(5, "SwToNeReg(trackId:%d) - trigger %s reached", track->trackId, _triggerParams.marker);
			debug(5, "SwToNeReg(trackId:%d) - exit current region %d", track->trackId, track->curRegion);
			debug(5, "SwToNeReg(trackId:%d) - call cloneToFadeOutTrack(delay:%d)", track->trackId, _triggerParams.fadeOutDelay);
			Track *fadeTrack = cloneToFadeOutTrack(track, _triggerParams.fadeOutDelay);
			if (fadeTrack) {
				fadeTrack->dataOffset = _sound->getRegionOffset(fadeTrack->soundDesc, fadeTrack->curRegion);
				fadeTrack->regionOffset = 0;
				debug(5, "SwToNeReg(trackId:%d)-sound(%d) select region %d, curHookId: %d", fadeTrack->trackId, fadeTrack->soundId, fadeTrack->curRegion, fadeTrack->curHookId);
				fadeTrack->curHookId = 0;
			}
			flushTrack(track);
			startMusic(_triggerParams.filename, _triggerParams.soundId, _triggerParams.hookId, _triggerParams.volume);
			_triggerUsed = false;
			return;
		}
	}

	int jumpId = _sound->getJumpIdByRegionAndHookId(soundDesc, track->curRegion, track->curHookId);
	if (jumpId != -1) {
		int region = _sound->getRegionIdByJumpId(soundDesc, jumpId);
		assert(region != -1);
		int sampleHookId = _sound->getJumpHookId(soundDesc, jumpId);
		assert(sampleHookId != -1);
		debug(5, "SwToNeReg(trackId:%d) - JUMP found - sound:%d, track hookId:%d, data hookId:%d", track->trackId, track->soundId, track->curHookId, sampleHookId);
		if (track->curHookId == sampleHookId) {
			int fadeDelay = (60 * _sound->getJumpFade(soundDesc, jumpId)) / 1000;
			debug(5, "SwToNeReg(trackId:%d) - sound(%d) match hookId", track->trackId, track->soundId);
			if (fadeDelay) {
				debug(5, "SwToNeReg(trackId:%d) - call cloneToFadeOutTrack(delay:%d)", track->trackId, fadeDelay);
				Track *fadeTrack = cloneToFadeOutTrack(track, fadeDelay);
				if (fadeTrack) {
					fadeTrack->dataOffset = _sound->getRegionOffset(fadeTrack->soundDesc, fadeTrack->curRegion);
					fadeTrack->regionOffset = 0;
					debug(5, "SwToNeReg(trackId:%d) - sound(%d) faded track, select region %d, curHookId: %d", fadeTrack->trackId, fadeTrack->soundId, fadeTrack->curRegion, fadeTrack->curHookId);
					fadeTrack->curHookId = 0;
				}
			}
			track->curRegion = region;
			debug(5, "SwToNeReg(trackId:%d) - sound(%d) jump to region %d, curHookId: %d", track->trackId, track->soundId, track->curRegion, track->curHookId);
			track->curHookId = 0;
		} else {
			debug(5, "SwToNeReg(trackId:%d) - Normal switch region, sound(%d), hookId(%d)", track->trackId, track->soundId, track->curHookId);
		}
	} else {
		debug(5, "SwToNeReg(trackId:%d) - Normal switch region, sound(%d), hookId(%d)", track->trackId, track->soundId, track->curHookId);
	}

	debug(5, "SwToNeReg(trackId:%d) - sound(%d), select region %d", track->trackId, track->soundId, track->curRegion);
	track->dataOffset = _sound->getRegionOffset(soundDesc, track->curRegion);
	track->regionOffset = 0;
	debug(5, "SwToNeReg(trackId:%d) - end of func", track->trackId);
}

} // End of namespace Scumm
