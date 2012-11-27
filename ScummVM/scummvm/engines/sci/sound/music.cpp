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

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "common/config-manager.h"

#include "sci/sci.h"
#include "sci/console.h"
#include "sci/resource.h"
#include "sci/engine/features.h"
#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/sound/midiparser_sci.h"
#include "sci/sound/music.h"

//#define DISABLE_REMAPPING

namespace Sci {

SciMusic::SciMusic(SciVersion soundVersion, bool useDigitalSFX)
	: _soundVersion(soundVersion), _soundOn(true), _masterVolume(0), _globalReverb(0), _useDigitalSFX(useDigitalSFX) {

	// Reserve some space in the playlist, to avoid expensive insertion
	// operations
	_playList.reserve(10);

	for (int i = 0; i < 16; i++) {
		_usedChannel[i] = 0;
		_channelRemap[i] = -1;
	}

	_queuedCommands.reserve(1000);
}

SciMusic::~SciMusic() {
	if (_pMidiDrv) {
		_pMidiDrv->close();
		delete _pMidiDrv;
	}
}

void SciMusic::init() {
	// system init
	_pMixer = g_system->getMixer();
	// SCI sound init
	_dwTempo = 0;

	Common::Platform platform = g_sci->getPlatform();
	uint32 deviceFlags = MDT_PCSPK | MDT_PCJR | MDT_ADLIB | MDT_MIDI;

	// Default to MIDI in SCI2.1+ games, as many don't have AdLib support.
	// Also, default to MIDI for Windows versions of SCI1.1 games, as their
	// soundtrack is written for GM.
	if (getSciVersion() >= SCI_VERSION_2_1 || g_sci->_features->useAltWinGMSound())
		deviceFlags |= MDT_PREFER_GM;

	// Currently our CMS implementation only supports SCI1(.1)
	if (getSciVersion() >= SCI_VERSION_1_EGA_ONLY && getSciVersion() <= SCI_VERSION_1_1)
		deviceFlags |= MDT_CMS;

	if (g_sci->getPlatform() == Common::kPlatformFMTowns) {
		if (getSciVersion() > SCI_VERSION_1_EARLY)
			deviceFlags = MDT_TOWNS;
		else
			deviceFlags |= MDT_TOWNS;
	}

	uint32 dev = MidiDriver::detectDevice(deviceFlags);
	_musicType = MidiDriver::getMusicType(dev);

	if (g_sci->_features->useAltWinGMSound() && _musicType != MT_GM) {
		warning("A Windows CD version with an alternate MIDI soundtrack has been chosen, "
				"but no MIDI music device has been selected. Reverting to the DOS soundtrack");
		g_sci->_features->forceDOSTracks();
	}

	switch (_musicType) {
	case MT_ADLIB:
		// FIXME: There's no Amiga sound option, so we hook it up to AdLib
		if (g_sci->getPlatform() == Common::kPlatformAmiga || platform == Common::kPlatformMacintosh)
			_pMidiDrv = MidiPlayer_AmigaMac_create(_soundVersion);
		else
			_pMidiDrv = MidiPlayer_AdLib_create(_soundVersion);
		break;
	case MT_PCJR:
		_pMidiDrv = MidiPlayer_PCJr_create(_soundVersion);
		break;
	case MT_PCSPK:
		_pMidiDrv = MidiPlayer_PCSpeaker_create(_soundVersion);
		break;
	case MT_CMS:
		_pMidiDrv = MidiPlayer_CMS_create(_soundVersion);
		break;
	case MT_TOWNS:
		_pMidiDrv = MidiPlayer_FMTowns_create(_soundVersion);
		break;
	default:
		if (ConfMan.getBool("native_fb01"))
			_pMidiDrv = MidiPlayer_Fb01_create(_soundVersion);
		else
			_pMidiDrv = MidiPlayer_Midi_create(_soundVersion);
	}

	if (_pMidiDrv && !_pMidiDrv->open()) {
		_pMidiDrv->setTimerCallback(this, &miditimerCallback);
		_dwTempo = _pMidiDrv->getBaseTempo();
	} else {
		if (g_sci->getGameId() == GID_FUNSEEKER) {
			// HACK: The Fun Seeker's Guide demo doesn't have patch 3 and the version
			// of the Adlib driver (adl.drv) that it includes is unsupported. That demo
			// doesn't have any sound anyway, so this shouldn't be fatal.
		} else {
			error("Failed to initialize sound driver");
		}
	}

	// Find out what the first possible channel is (used, when doing channel
	// remapping).
	_driverFirstChannel = _pMidiDrv->getFirstChannel();
	_driverLastChannel = _pMidiDrv->getLastChannel();
	if (getSciVersion() <= SCI_VERSION_0_LATE)
		_globalReverb = _pMidiDrv->getReverb();	// Init global reverb for SCI0
}

void SciMusic::miditimerCallback(void *p) {
	SciMusic *sciMusic = (SciMusic *)p;

	Common::StackLock lock(sciMusic->_mutex);
	sciMusic->onTimer();
}

void SciMusic::onTimer() {
	const MusicList::iterator end = _playList.end();
	// sending out queued commands that were "sent" via main thread
	sendMidiCommandsFromQueue();

	for (MusicList::iterator i = _playList.begin(); i != end; ++i)
		(*i)->onTimer();
}

void SciMusic::putMidiCommandInQueue(byte status, byte firstOp, byte secondOp) {
	putMidiCommandInQueue(status | ((uint32)firstOp << 8) | ((uint32)secondOp << 16));
}

void SciMusic::putMidiCommandInQueue(uint32 midi) {
	_queuedCommands.push_back(midi);
}

// This sends the stored commands from queue to driver (is supposed to get
// called only during onTimer()). At least mt32 emulation doesn't like getting
// note-on commands from main thread (if we directly send, we would get a crash
// during piano scene in lsl5).
void SciMusic::sendMidiCommandsFromQueue() {
	uint curCommand = 0;
	uint commandCount = _queuedCommands.size();

	while (curCommand < commandCount) {
		_pMidiDrv->send(_queuedCommands[curCommand]);
		curCommand++;
	}
	_queuedCommands.clear();
}

void SciMusic::clearPlayList() {
	// we must NOT lock our mutex here. Playlist is modified inside soundKill() which will lock the mutex
	//  during deletion. If we lock it here, a deadlock may occur within soundStop() because that one
	//  calls the mixer, which will also lock the mixer mutex and if the mixer thread is active during
	//  that time, we will get a deadlock.
	while (!_playList.empty()) {
		soundStop(_playList[0]);
		soundKill(_playList[0]);
	}
}

void SciMusic::pauseAll(bool pause) {
	const MusicList::iterator end = _playList.end();
	for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
		soundToggle(*i, pause);
	}
}

void SciMusic::stopAll() {
	const MusicList::iterator end = _playList.end();
	for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
		soundStop(*i);
	}
}

void SciMusic::soundSetSoundOn(bool soundOnFlag) {
	Common::StackLock lock(_mutex);

	_soundOn = soundOnFlag;
	_pMidiDrv->playSwitch(soundOnFlag);
}

uint16 SciMusic::soundGetVoices() {
	Common::StackLock lock(_mutex);

	return _pMidiDrv->getPolyphony();
}

MusicEntry *SciMusic::getSlot(reg_t obj) {
	Common::StackLock lock(_mutex);

	const MusicList::iterator end = _playList.end();
	for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
		if ((*i)->soundObj == obj)
			return *i;
	}

	return NULL;
}

// We return the currently active music slot for SCI0
MusicEntry *SciMusic::getActiveSci0MusicSlot() {
	const MusicList::iterator end = _playList.end();
	MusicEntry *highestPrioritySlot = NULL;
	for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
		MusicEntry *playSlot = *i;
		if (playSlot->pMidiParser) {
			if (playSlot->status == kSoundPlaying)
				return playSlot;
			if (playSlot->status == kSoundPaused) {
				if ((!highestPrioritySlot) || (highestPrioritySlot->priority < playSlot->priority))
					highestPrioritySlot = playSlot;
			}
		}
	}
	return highestPrioritySlot;
}

void SciMusic::setGlobalReverb(int8 reverb) {
	Common::StackLock lock(_mutex);
	if (reverb != 127) {
		// Set global reverb normally
		_globalReverb = reverb;

		// Check the reverb of the active song...
		const MusicList::iterator end = _playList.end();
		for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
			if ((*i)->status == kSoundPlaying) {
				if ((*i)->reverb == 127)			// Active song has no reverb
					_pMidiDrv->setReverb(reverb);	// Set the global reverb
				break;
			}
		}
	} else {
		// Set reverb of the active song
		const MusicList::iterator end = _playList.end();
		for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
			if ((*i)->status == kSoundPlaying) {
				_pMidiDrv->setReverb((*i)->reverb);	// Set the song's reverb
				break;
			}
		}
	}
}

byte SciMusic::getCurrentReverb() {
	Common::StackLock lock(_mutex);
	return _pMidiDrv->getReverb();
}

static bool musicEntryCompare(const MusicEntry *l, const MusicEntry *r) {
	return (l->priority > r->priority);
}

void SciMusic::sortPlayList() {
	// Sort the play list in descending priority order
	Common::sort(_playList.begin(), _playList.end(), musicEntryCompare);
}

void SciMusic::soundInitSnd(MusicEntry *pSnd) {
	int channelFilterMask = 0;
	SoundResource::Track *track = pSnd->soundRes->getTrackByType(_pMidiDrv->getPlayId());

	// If MIDI device is selected but there is no digital track in sound
	// resource try to use Adlib's digital sample if possible. Also, if the
	// track couldn't be found, load the digital track, as some games depend on
	// this (e.g. the Longbow demo).
	if (!track || (_useDigitalSFX && track->digitalChannelNr == -1)) {
		SoundResource::Track *digital = pSnd->soundRes->getDigitalTrack();
		if (digital)
			track = digital;
	}

	if (track) {
		// Play digital sample
		if (track->digitalChannelNr != -1) {
			byte *channelData = track->channels[track->digitalChannelNr].data;
			delete pSnd->pStreamAud;
			byte flags = Audio::FLAG_UNSIGNED;
			// Amiga SCI1 games had signed sound data
			if (_soundVersion >= SCI_VERSION_1_EARLY && g_sci->getPlatform() == Common::kPlatformAmiga)
				flags = 0;
			int endPart = track->digitalSampleEnd > 0 ? (track->digitalSampleSize - track->digitalSampleEnd) : 0;
			pSnd->pStreamAud = Audio::makeRawStream(channelData + track->digitalSampleStart,
								track->digitalSampleSize - track->digitalSampleStart - endPart,
								track->digitalSampleRate, flags, DisposeAfterUse::NO);
			delete pSnd->pLoopStream;
			pSnd->pLoopStream = 0;
			pSnd->soundType = Audio::Mixer::kSFXSoundType;
			pSnd->hCurrentAud = Audio::SoundHandle();
		} else {
			// play MIDI track
			Common::StackLock lock(_mutex);
			pSnd->soundType = Audio::Mixer::kMusicSoundType;
			if (pSnd->pMidiParser == NULL) {
				pSnd->pMidiParser = new MidiParser_SCI(_soundVersion, this);
				pSnd->pMidiParser->setMidiDriver(_pMidiDrv);
				pSnd->pMidiParser->setTimerRate(_dwTempo);
				pSnd->pMidiParser->setMasterVolume(_masterVolume);
			}

			pSnd->pauseCounter = 0;

			// Find out what channels to filter for SCI0
			channelFilterMask = pSnd->soundRes->getChannelFilterMask(_pMidiDrv->getPlayId(), _pMidiDrv->hasRhythmChannel());

			pSnd->pMidiParser->mainThreadBegin();
			// loadMusic() below calls jumpToTick.
			// Disable sound looping and hold before jumpToTick is called,
			// otherwise the song may keep looping forever when it ends in
			// jumpToTick (e.g. LSL3, when going left from room 210).
			uint16 prevLoop = pSnd->loop;
			int16 prevHold = pSnd->hold;
			pSnd->loop = 0;
			pSnd->hold = -1;

			pSnd->pMidiParser->loadMusic(track, pSnd, channelFilterMask, _soundVersion);
			pSnd->reverb = pSnd->pMidiParser->getSongReverb();

			// Restore looping and hold
			pSnd->loop = prevLoop;
			pSnd->hold = prevHold;
			pSnd->pMidiParser->mainThreadEnd();
		}
	}
}

// This one checks, if requested channel is available -> in that case give
// caller that channel. Otherwise look for an unused one
int16 SciMusic::tryToOwnChannel(MusicEntry *caller, int16 bestChannel) {
#ifdef DISABLE_REMAPPING
	return bestChannel;
#endif

	// Don't even try this for SCI0
	if (_soundVersion <= SCI_VERSION_0_LATE)
		return bestChannel;
	if (!_usedChannel[bestChannel]) {
		// currently unused, so give it to caller directly
		_usedChannel[bestChannel] = caller;
		_channelRemap[bestChannel] = bestChannel;
		return bestChannel;
	}
	// otherwise look for unused channel
	for (int channelNr = _driverFirstChannel; channelNr < 15; channelNr++) {
		if (channelNr == 9) // never map to channel 9 (percussion)
			continue;
		if (!_usedChannel[channelNr]) {
			_usedChannel[channelNr] = caller;
			_channelRemap[bestChannel] = channelNr;
			return channelNr;
		}
	}
	// nothing found, don't map channel at all
	//  sierra did this as well, although i'm not sure if we act exactly the same way
	//  maybe they removed channels from previous playing music
	return -1;
}

void SciMusic::freeChannels(MusicEntry *caller) {
	// Remove used channels
	for (int i = 0; i < 15; i++) {
		if (_usedChannel[i] == caller) {
			if (_channelRemap[i] != -1) {
				// athrxx: The original handles this differently. It seems to be checking for (and effecting) necessary
				// remaps / resets etc. more or less all the time. There are several more tables to keep track of everything.
				// I don't know whether all of that is needed and to which SCI versions it applies, though.
				// At least it is necessary to release the allocated channels inside the driver. Otherwise these channels
				// won't be available any more (e.g. after half of the KQ5 FM-Towns intro there will be no more music
				// since the driver can't pick up any more channels). The channels also have to be reset to
				// default values, since the original does the same (although in a different manny) and the music will be wrong
				// otherwise (at least KQ5 FM-Towns).

				sendMidiCommand(0x4000e0 | _channelRemap[i]);	// Reset pitch wheel
				sendMidiCommand(0x0040b0 | _channelRemap[i]);	// Release pedal
				sendMidiCommand(0x004bb0 | _channelRemap[i]);	// Release assigned driver channels
			}
			_usedChannel[i] = 0;
			_channelRemap[i] = -1;
		}
	}
	// Also tell midiparser, that he lost ownership
	caller->pMidiParser->lostChannels();
}

void SciMusic::soundPlay(MusicEntry *pSnd) {
	_mutex.lock();

	uint playListCount = _playList.size();
	uint playListNo = playListCount;
	MusicEntry *alreadyPlaying = NULL;

	// searching if sound is already in _playList
	for (uint i = 0; i < playListCount; i++) {
		if (_playList[i] == pSnd)
			playListNo = i;
		if ((_playList[i]->status == kSoundPlaying) && (_playList[i]->pMidiParser))
			alreadyPlaying = _playList[i];
	}
	if (playListNo == playListCount) { // not found
		_playList.push_back(pSnd);
		sortPlayList();
	}

	_mutex.unlock();	// unlock to perform mixer-related calls

	if (pSnd->pMidiParser) {
		if ((_soundVersion <= SCI_VERSION_0_LATE) && (alreadyPlaying)) {
			// Music already playing in SCI0?
			if (pSnd->priority > alreadyPlaying->priority) {
				// And new priority higher? pause previous music and play new one immediately.
				// Example of such case: lsl3, when getting points (jingle is played then)
				soundPause(alreadyPlaying);
				alreadyPlaying->isQueued = true;
			} else {
				// And new priority equal or lower? queue up music and play it afterwards done by
				//  SoundCommandParser::updateSci0Cues()
				// Example of such case: iceman room 14
				pSnd->isQueued = true;
				pSnd->status = kSoundPaused;
				return;
			}
		}
	}

	if (pSnd->pStreamAud) {
		if (!_pMixer->isSoundHandleActive(pSnd->hCurrentAud)) {
			// Sierra SCI ignores volume set when playing samples via kDoSound
			//  At least freddy pharkas/CD has a script bug that sets volume to 0
			//  when playing the "score" sample
			if (pSnd->loop > 1) {
				pSnd->pLoopStream = new Audio::LoopingAudioStream(pSnd->pStreamAud,
																pSnd->loop, DisposeAfterUse::NO);
				_pMixer->playStream(pSnd->soundType, &pSnd->hCurrentAud,
										pSnd->pLoopStream, -1, _pMixer->kMaxChannelVolume, 0,
										DisposeAfterUse::NO);
			} else {
				// Rewind in case we play the same sample multiple times
				// (non-looped) like in pharkas right at the start
				pSnd->pStreamAud->rewind();
				_pMixer->playStream(pSnd->soundType, &pSnd->hCurrentAud,
										pSnd->pStreamAud, -1, _pMixer->kMaxChannelVolume, 0,
										DisposeAfterUse::NO);
			}
		}
	} else {
		if (pSnd->pMidiParser) {
			Common::StackLock lock(_mutex);
			pSnd->pMidiParser->mainThreadBegin();

			if (pSnd->status != kSoundPaused) {
				// Stop any in progress music fading, as that will reset the
				// volume of the sound channels that the faded song occupies..
				// Fixes bug #3266480 and partially fixes bug #3041738.
				for (uint i = 0; i < playListCount; i++) {
					// Is another MIDI song being faded down? If yes, stop it
					// immediately instead
					if (_playList[i]->fadeStep < 0 && _playList[i]->pMidiParser) {
						_playList[i]->status = kSoundStopped;
						if (_soundVersion <= SCI_VERSION_0_LATE)
							_playList[i]->isQueued = false;
						_playList[i]->pMidiParser->stop();
						freeChannels(_playList[i]);
						_playList[i]->fadeStep = 0;
					}
				}
			}

			pSnd->pMidiParser->tryToOwnChannels();
			if (pSnd->status != kSoundPaused)
				pSnd->pMidiParser->sendInitCommands();
			pSnd->pMidiParser->setVolume(pSnd->volume);

			// Disable sound looping and hold before jumpToTick is called,
			// otherwise the song may keep looping forever when it ends in jumpToTick.
			// This is needed when loading saved games, or when a game
			// stops the same sound twice (e.g. LSL3 Amiga, going left from
			// room 210 to talk with Kalalau). Fixes bugs #3083151 and #3106107.
			uint16 prevLoop = pSnd->loop;
			int16 prevHold = pSnd->hold;
			pSnd->loop = 0;
			pSnd->hold = -1;

			if (pSnd->status == kSoundStopped)
				pSnd->pMidiParser->jumpToTick(0);
			else
				// Fast forward to the last position and perform associated events when loading
				pSnd->pMidiParser->jumpToTick(pSnd->ticker, true, true, true);

			// Restore looping and hold
			pSnd->loop = prevLoop;
			pSnd->hold = prevHold;
			pSnd->pMidiParser->mainThreadEnd();
		}
	}

	pSnd->status = kSoundPlaying;
}

void SciMusic::soundStop(MusicEntry *pSnd) {
	SoundStatus previousStatus = pSnd->status;
	pSnd->status = kSoundStopped;
	if (_soundVersion <= SCI_VERSION_0_LATE)
		pSnd->isQueued = false;
	if (pSnd->pStreamAud)
		_pMixer->stopHandle(pSnd->hCurrentAud);

	if (pSnd->pMidiParser) {
		Common::StackLock lock(_mutex);
		pSnd->pMidiParser->mainThreadBegin();
		// We shouldn't call stop in case it's paused, otherwise we would send
		// allNotesOff() again
		if (previousStatus == kSoundPlaying)
			pSnd->pMidiParser->stop();
		freeChannels(pSnd);
		pSnd->pMidiParser->mainThreadEnd();
	}

	pSnd->fadeStep = 0; // end fading, if fading was in progress
}

void SciMusic::soundSetVolume(MusicEntry *pSnd, byte volume) {
	assert(volume <= MUSIC_VOLUME_MAX);
	if (pSnd->pStreamAud) {
		// we simply ignore volume changes for samples, because sierra sci also
		//  doesn't support volume for samples via kDoSound
	} else if (pSnd->pMidiParser) {
		Common::StackLock lock(_mutex);
		pSnd->pMidiParser->mainThreadBegin();
		pSnd->pMidiParser->setVolume(volume);
		pSnd->pMidiParser->mainThreadEnd();
	}
}

// this is used to set volume of the sample, used for fading only!
void SciMusic::soundSetSampleVolume(MusicEntry *pSnd, byte volume) {
	assert(volume <= MUSIC_VOLUME_MAX);
	assert(pSnd->pStreamAud);
	_pMixer->setChannelVolume(pSnd->hCurrentAud, volume * 2); // Mixer is 0-255, SCI is 0-127
}

void SciMusic::soundSetPriority(MusicEntry *pSnd, byte prio) {
	Common::StackLock lock(_mutex);

	pSnd->priority = prio;
	sortPlayList();
}

void SciMusic::soundKill(MusicEntry *pSnd) {
	pSnd->status = kSoundStopped;

	if (pSnd->pMidiParser) {
		Common::StackLock lock(_mutex);
		pSnd->pMidiParser->mainThreadBegin();
		pSnd->pMidiParser->unloadMusic();
		pSnd->pMidiParser->mainThreadEnd();
		delete pSnd->pMidiParser;
		pSnd->pMidiParser = NULL;
	}

	if (pSnd->pStreamAud) {
		_pMixer->stopHandle(pSnd->hCurrentAud);
		delete pSnd->pStreamAud;
		pSnd->pStreamAud = NULL;
		delete pSnd->pLoopStream;
		pSnd->pLoopStream = 0;
	}

	Common::StackLock lock(_mutex);
	uint sz = _playList.size(), i;
	// Remove sound from playlist
	for (i = 0; i < sz; i++) {
		if (_playList[i] == pSnd) {
			delete _playList[i]->soundRes;
			delete _playList[i];
			_playList.remove_at(i);
			break;
		}
	}
}

void SciMusic::soundPause(MusicEntry *pSnd) {
	// SCI seems not to be pausing samples played back by kDoSound at all
	//  It only stops looping samples (actually doesn't loop them again before they are unpaused)
	//  Examples: Space Quest 1 death by acid drops (pause is called even specifically for the sample, see bug #3038048)
	//             Eco Quest 1 during the intro when going to the abort-menu
	//             In both cases sierra sci keeps playing
	//            Leisure Suit Larry 1 doll scene - it seems that pausing here actually just stops
	//             further looping from happening
	//  This is a somewhat bigger change, I'm leaving in the old code in here just in case
	//  I'm currently pausing looped sounds directly, non-looped sounds won't get paused
	if ((pSnd->pStreamAud) && (!pSnd->pLoopStream))
		return;
	pSnd->pauseCounter++;
	if (pSnd->status != kSoundPlaying)
		return;
	pSnd->status = kSoundPaused;
	if (pSnd->pStreamAud) {
		_pMixer->pauseHandle(pSnd->hCurrentAud, true);
	} else {
		if (pSnd->pMidiParser) {
			Common::StackLock lock(_mutex);
			pSnd->pMidiParser->mainThreadBegin();
			pSnd->pMidiParser->pause();
			freeChannels(pSnd);
			pSnd->pMidiParser->mainThreadEnd();
		}
	}
}

void SciMusic::soundResume(MusicEntry *pSnd) {
	if (pSnd->pauseCounter > 0)
		pSnd->pauseCounter--;
	if (pSnd->pauseCounter != 0)
		return;
	if (pSnd->status != kSoundPaused)
		return;
	if (pSnd->pStreamAud) {
		_pMixer->pauseHandle(pSnd->hCurrentAud, false);
		pSnd->status = kSoundPlaying;
	} else {
		soundPlay(pSnd);
	}
}

void SciMusic::soundToggle(MusicEntry *pSnd, bool pause) {
	if (pause)
		soundPause(pSnd);
	else
		soundResume(pSnd);
}

uint16 SciMusic::soundGetMasterVolume() {
	return _masterVolume;
}

void SciMusic::soundSetMasterVolume(uint16 vol) {
	_masterVolume = vol;

	Common::StackLock lock(_mutex);

	const MusicList::iterator end = _playList.end();
	for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
		if ((*i)->pMidiParser)
			(*i)->pMidiParser->setMasterVolume(vol);
	}
}

void SciMusic::sendMidiCommand(uint32 cmd) {
	Common::StackLock lock(_mutex);
	_pMidiDrv->send(cmd);
}

void SciMusic::sendMidiCommand(MusicEntry *pSnd, uint32 cmd) {
	Common::StackLock lock(_mutex);
	if (!pSnd->pMidiParser)
		error("tried to cmdSendMidi on non midi slot (%04x:%04x)", PRINT_REG(pSnd->soundObj));

	pSnd->pMidiParser->mainThreadBegin();
	pSnd->pMidiParser->sendFromScriptToDriver(cmd);
	pSnd->pMidiParser->mainThreadEnd();
}

void SciMusic::printPlayList(Console *con) {
	Common::StackLock lock(_mutex);

	const char *musicStatus[] = { "Stopped", "Initialized", "Paused", "Playing" };

	for (uint32 i = 0; i < _playList.size(); i++) {
		MusicEntry *song = _playList[i];
		con->DebugPrintf("%d: %04x:%04x (%s), resource id: %d, status: %s, %s type\n",
						i, PRINT_REG(song->soundObj),
						g_sci->getEngineState()->_segMan->getObjectName(song->soundObj),
						song->resourceId, musicStatus[song->status],
						song->pMidiParser ? "MIDI" : "digital audio");
	}
}

void SciMusic::printSongInfo(reg_t obj, Console *con) {
	Common::StackLock lock(_mutex);

	const char *musicStatus[] = { "Stopped", "Initialized", "Paused", "Playing" };

	const MusicList::iterator end = _playList.end();
	for (MusicList::iterator i = _playList.begin(); i != end; ++i) {
		MusicEntry *song = *i;
		if (song->soundObj == obj) {
			con->DebugPrintf("Resource id: %d, status: %s\n", song->resourceId, musicStatus[song->status]);
			con->DebugPrintf("dataInc: %d, hold: %d, loop: %d\n", song->dataInc, song->hold, song->loop);
			con->DebugPrintf("signal: %d, priority: %d\n", song->signal, song->priority);
			con->DebugPrintf("ticker: %d, volume: %d\n", song->ticker, song->volume);

			if (song->pMidiParser) {
				con->DebugPrintf("Type: MIDI\n");
				if (song->soundRes) {
					SoundResource::Track *track = song->soundRes->getTrackByType(_pMidiDrv->getPlayId());
					con->DebugPrintf("Channels: %d\n", track->channelCount);
				}
			} else if (song->pStreamAud || song->pLoopStream) {
				con->DebugPrintf("Type: digital audio (%s), sound active: %s\n",
					song->pStreamAud ? "non looping" : "looping",
					_pMixer->isSoundHandleActive(song->hCurrentAud) ? "yes" : "no");
				if (song->soundRes) {
					con->DebugPrintf("Sound resource information:\n");
					SoundResource::Track *track = song->soundRes->getTrackByType(_pMidiDrv->getPlayId());
					if (track && track->digitalChannelNr != -1) {
						con->DebugPrintf("Sample size: %d, sample rate: %d, channels: %d, digital channel number: %d\n",
							track->digitalSampleSize, track->digitalSampleRate, track->channelCount, track->digitalChannelNr);
					}
				}
			}

			return;
		}
	}

	con->DebugPrintf("Song object not found in playlist");
}

MusicEntry::MusicEntry() {
	soundObj = NULL_REG;

	soundRes = 0;
	resourceId = 0;

	isQueued = false;

	dataInc = 0;
	ticker = 0;
	signal = 0;
	priority = 0;
	loop = 0;
	volume = MUSIC_VOLUME_DEFAULT;
	hold = -1;
	reverb = -1;

	pauseCounter = 0;
	sampleLoopCounter = 0;

	fadeTo = 0;
	fadeStep = 0;
	fadeTicker = 0;
	fadeTickerStep = 0;
	fadeSetVolume = false;
	fadeCompleted = false;
	stopAfterFading = false;

	status = kSoundStopped;

	soundType = Audio::Mixer::kMusicSoundType;

	pStreamAud = 0;
	pLoopStream = 0;
	pMidiParser = 0;
}

MusicEntry::~MusicEntry() {
}

void MusicEntry::onTimer() {
	if (!signal) {
		if (!signalQueue.empty()) {
			// no signal set, but signal in queue, set that one
			signal = signalQueue[0];
			signalQueue.remove_at(0);
		}
	}

	if (status != kSoundPlaying)
		return;

	// Fade MIDI and digital sound effects
	if (fadeStep)
		doFade();

	// Only process MIDI streams in this thread, not digital sound effects
	if (pMidiParser) {
		pMidiParser->onTimer();
		ticker = (uint16)pMidiParser->getTick();
	}
}

void MusicEntry::doFade() {
	if (fadeTicker)
		fadeTicker--;
	else {
		fadeTicker = fadeTickerStep;
		volume += fadeStep;
		if (((fadeStep > 0) && (volume >= fadeTo)) || ((fadeStep < 0) && (volume <= fadeTo))) {
			volume = fadeTo;
			fadeStep = 0;
			fadeCompleted = true;
		}

		// Only process MIDI streams in this thread, not digital sound effects
		if (pMidiParser) {
			pMidiParser->setVolume(volume);
		}

		fadeSetVolume = true; // set flag so that SoundCommandParser::cmdUpdateCues will set the volume of the stream
	}
}

void MusicEntry::setSignal(int newSignal) {
	// For SCI0, we cache the signals to set, as some songs might
	// update their signal faster than kGetEvent is called (which is where
	// we manually invoke kDoSoundUpdateCues for SCI0 games). SCI01 and
	// newer handle signalling inside kDoSoundUpdateCues. Refer to bug #3042981
	if (g_sci->_features->detectDoSoundType() <= SCI_VERSION_0_LATE) {
		if (!signal) {
			signal = newSignal;
		} else {
			// signal already set and waiting for getting to scripts, queue new one
			signalQueue.push_back(newSignal);
		}
	} else {
		// Set the signal directly for newer games, otherwise the sound
		// object might be deleted already later on (refer to bug #3045913)
		signal = newSignal;
	}
}

} // End of namespace Sci
