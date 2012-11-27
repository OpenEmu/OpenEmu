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

#include "common/config-manager.h"
#include "sci/sound/audio.h"
#include "sci/sound/music.h"
#include "sci/sound/soundcmd.h"

#include "sci/engine/features.h"
#include "sci/engine/kernel.h"
#include "sci/engine/object.h"
#include "sci/engine/selector.h"

namespace Sci {

SoundCommandParser::SoundCommandParser(ResourceManager *resMan, SegManager *segMan, Kernel *kernel, AudioPlayer *audio, SciVersion soundVersion) :
	_resMan(resMan), _segMan(segMan), _kernel(kernel), _audio(audio), _soundVersion(soundVersion) {

	// Check if the user wants synthesized or digital sound effects in SCI1.1
	// games based on the prefer_digitalsfx config setting

	// In SCI2 and later games, this check should always be true - there was
	// always only one version of each sound effect or digital music track
	// (e.g. the menu music in GK1 - there is a sound effect with the same
	// resource number, but it's totally unrelated to the menu music).
	// The GK1 demo (very late SCI1.1) does the same thing
	// TODO: Check the QFG4 demo
	_useDigitalSFX = (getSciVersion() >= SCI_VERSION_2 || g_sci->getGameId() == GID_GK1 || ConfMan.getBool("prefer_digitalsfx"));

	_music = new SciMusic(_soundVersion, _useDigitalSFX);
	_music->init();
}

SoundCommandParser::~SoundCommandParser() {
	delete _music;
}

reg_t SoundCommandParser::kDoSoundInit(int argc, reg_t *argv, reg_t acc) {
	debugC(kDebugLevelSound, "kDoSound(init): %04x:%04x", PRINT_REG(argv[0]));
	processInitSound(argv[0]);
	return acc;
}

int SoundCommandParser::getSoundResourceId(reg_t obj) {
	int resourceId = obj.getSegment() ? readSelectorValue(_segMan, obj, SELECTOR(number)) : -1;
	// Modify the resourceId for the Windows versions that have an alternate MIDI soundtrack, like SSCI did.
	if (g_sci && g_sci->_features->useAltWinGMSound()) {
		// Check if the alternate MIDI song actually exists...
		// There are cases where it just doesn't exist (e.g. SQ4, room 530 -
		// bug #3392767). In these cases, use the DOS tracks instead.
		if (resourceId && _resMan->testResource(ResourceId(kResourceTypeSound, resourceId + 1000)))
			resourceId += 1000;
	}

	return resourceId;
}

void SoundCommandParser::initSoundResource(MusicEntry *newSound) {
	if (newSound->resourceId && _resMan->testResource(ResourceId(kResourceTypeSound, newSound->resourceId)))
		newSound->soundRes = new SoundResource(newSound->resourceId, _resMan, _soundVersion);
	else
		newSound->soundRes = 0;

	// In SCI1.1 games, sound effects are started from here. If we can find
	// a relevant audio resource, play it, otherwise switch to synthesized
	// effects. If the resource exists, play it using map 65535 (sound
	// effects map)
	bool checkAudioResource = getSciVersion() >= SCI_VERSION_1_1;
	// Hoyle 4 has garbled audio resources in place of the sound resources.
	if (g_sci->getGameId() == GID_HOYLE4)
		checkAudioResource = false;

	if (checkAudioResource && _resMan->testResource(ResourceId(kResourceTypeAudio, newSound->resourceId))) {
		// Found a relevant audio resource, create an audio stream if there is
		// no associated sound resource, or if both resources exist and the
		// user wants the digital version.
		if (_useDigitalSFX || !newSound->soundRes) {
			int sampleLen;
			newSound->pStreamAud = _audio->getAudioStream(newSound->resourceId, 65535, &sampleLen);
			newSound->soundType = Audio::Mixer::kSFXSoundType;
		}
	}

	if (!newSound->pStreamAud && newSound->soundRes)
		_music->soundInitSnd(newSound);
}

void SoundCommandParser::processInitSound(reg_t obj) {
	int resourceId = getSoundResourceId(obj);

	// Check if a track with the same sound object is already playing
	MusicEntry *oldSound = _music->getSlot(obj);
	if (oldSound)
		processDisposeSound(obj);

	MusicEntry *newSound = new MusicEntry();
	newSound->resourceId = resourceId;
	newSound->soundObj = obj;
	newSound->loop = readSelectorValue(_segMan, obj, SELECTOR(loop));
	newSound->priority = readSelectorValue(_segMan, obj, SELECTOR(pri)) & 0xFF;
	if (_soundVersion >= SCI_VERSION_1_EARLY)
		newSound->volume = CLIP<int>(readSelectorValue(_segMan, obj, SELECTOR(vol)), 0, MUSIC_VOLUME_MAX);
	newSound->reverb = -1;	// initialize to SCI invalid, it'll be set correctly in soundInitSnd() below

	debugC(kDebugLevelSound, "kDoSound(init): %04x:%04x number %d, loop %d, prio %d, vol %d", PRINT_REG(obj),
			resourceId,	newSound->loop, newSound->priority, newSound->volume);

	initSoundResource(newSound);

	_music->pushBackSlot(newSound);

	if (newSound->soundRes || newSound->pStreamAud) {
		// Notify the engine
		if (_soundVersion <= SCI_VERSION_0_LATE)
			writeSelectorValue(_segMan, obj, SELECTOR(state), kSoundInitialized);
		else
			writeSelector(_segMan, obj, SELECTOR(nodePtr), obj);
	}
}

reg_t SoundCommandParser::kDoSoundPlay(int argc, reg_t *argv, reg_t acc) {
	debugC(kDebugLevelSound, "kDoSound(play): %04x:%04x", PRINT_REG(argv[0]));
	processPlaySound(argv[0]);
	return acc;
}

void SoundCommandParser::processPlaySound(reg_t obj) {
	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("kDoSound(play): Slot not found (%04x:%04x), initializing it manually", PRINT_REG(obj));
		// The sound hasn't been initialized for some reason, so initialize it
		// here. Happens in KQ6, room 460, when giving the creature (child) to
		// the bookworm. Fixes bugs #3413301 and #3421098.
		processInitSound(obj);
		musicSlot = _music->getSlot(obj);
		if (!musicSlot)
			error("Failed to initialize uninitialized sound slot");
	}

	int resourceId = getSoundResourceId(obj);

	if (musicSlot->resourceId != resourceId) { // another sound loaded into struct
		processDisposeSound(obj);
		processInitSound(obj);
		// Find slot again :)
		musicSlot = _music->getSlot(obj);
	}

	writeSelector(_segMan, obj, SELECTOR(handle), obj);

	if (_soundVersion >= SCI_VERSION_1_EARLY) {
		writeSelector(_segMan, obj, SELECTOR(nodePtr), obj);
		writeSelectorValue(_segMan, obj, SELECTOR(min), 0);
		writeSelectorValue(_segMan, obj, SELECTOR(sec), 0);
		writeSelectorValue(_segMan, obj, SELECTOR(frame), 0);
		writeSelectorValue(_segMan, obj, SELECTOR(signal), 0);
	} else {
		writeSelectorValue(_segMan, obj, SELECTOR(state), kSoundPlaying);
	}

	musicSlot->loop = readSelectorValue(_segMan, obj, SELECTOR(loop));
	musicSlot->priority = readSelectorValue(_segMan, obj, SELECTOR(priority));
	// Reset hold when starting a new song. kDoSoundSetHold is always called after
	// kDoSoundPlay to set it properly, if needed. Fixes bug #3413589.
	musicSlot->hold = -1;
	if (_soundVersion >= SCI_VERSION_1_EARLY)
		musicSlot->volume = readSelectorValue(_segMan, obj, SELECTOR(vol));

	debugC(kDebugLevelSound, "kDoSound(play): %04x:%04x number %d, loop %d, prio %d, vol %d", PRINT_REG(obj),
			resourceId, musicSlot->loop, musicSlot->priority, musicSlot->volume);

	_music->soundPlay(musicSlot);
}

reg_t SoundCommandParser::kDoSoundRestore(int argc, reg_t *argv, reg_t acc) {
	// Called after loading, to restore the playlist
	// We don't really use or need this
	return acc;
}

reg_t SoundCommandParser::kDoSoundDummy(int argc, reg_t *argv, reg_t acc) {
	warning("cmdDummy invoked");	// not supposed to occur
	return acc;
}

reg_t SoundCommandParser::kDoSoundDispose(int argc, reg_t *argv, reg_t acc) {
	debugC(kDebugLevelSound, "kDoSound(dispose): %04x:%04x", PRINT_REG(argv[0]));
	processDisposeSound(argv[0]);
	return acc;
}

void SoundCommandParser::processDisposeSound(reg_t obj) {
	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("kDoSound(dispose): Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	processStopSound(obj, false);

	_music->soundKill(musicSlot);
	writeSelectorValue(_segMan, obj, SELECTOR(handle), 0);
	if (_soundVersion >= SCI_VERSION_1_EARLY)
		writeSelector(_segMan, obj, SELECTOR(nodePtr), NULL_REG);
	else
		writeSelectorValue(_segMan, obj, SELECTOR(state), kSoundStopped);
}

reg_t SoundCommandParser::kDoSoundStop(int argc, reg_t *argv, reg_t acc) {
	debugC(kDebugLevelSound, "kDoSound(stop): %04x:%04x", PRINT_REG(argv[0]));
	processStopSound(argv[0], false);
	return acc;
}

void SoundCommandParser::processStopSound(reg_t obj, bool sampleFinishedPlaying) {
	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("kDoSound(stop): Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	if (_soundVersion <= SCI_VERSION_0_LATE) {
		writeSelectorValue(_segMan, obj, SELECTOR(state), kSoundStopped);
	} else {
		writeSelectorValue(_segMan, obj, SELECTOR(handle), 0);
	}

	// Set signal selector in sound SCI0 games only, when the sample has
	// finished playing. If we don't set it at all, we get a problem when using
	// vaporizer on the 2 guys. If we set it all the time, we get no music in
	// sq3new and kq1.
	// FIXME: This *may* be wrong, it's impossible to find out in Sierra DOS
	//        SCI, because SCI0 under DOS didn't have sfx drivers included.
	// We need to set signal in sound SCI1+ games all the time.
	if ((_soundVersion > SCI_VERSION_0_LATE) || sampleFinishedPlaying)
		writeSelectorValue(_segMan, obj, SELECTOR(signal), SIGNAL_OFFSET);

	musicSlot->dataInc = 0;
	musicSlot->signal = 0;
	_music->soundStop(musicSlot);
}

reg_t SoundCommandParser::kDoSoundPause(int argc, reg_t *argv, reg_t acc) {
	if (argc == 1)
		debugC(kDebugLevelSound, "kDoSound(pause): %04x:%04x", PRINT_REG(argv[0]));
	else
		debugC(kDebugLevelSound, "kDoSound(pause): %04x:%04x, %04x:%04x", PRINT_REG(argv[0]), PRINT_REG(argv[1]));

	if (_soundVersion <= SCI_VERSION_0_LATE) {
		// SCI0 games give us 0/1 for either resuming or pausing the current music
		//  this one doesn't count, so pausing 2 times and resuming once means here that we are supposed to resume
		uint16 value = argv[0].toUint16();
		MusicEntry *musicSlot = _music->getActiveSci0MusicSlot();
		switch (value) {
		case 1:
			if ((musicSlot) && (musicSlot->status == kSoundPlaying)) {
				_music->soundPause(musicSlot);
				writeSelectorValue(_segMan, musicSlot->soundObj, SELECTOR(state), kSoundPaused);
			}
			return make_reg(0, 0);
		case 0:
			if ((musicSlot) && (musicSlot->status == kSoundPaused)) {
				_music->soundResume(musicSlot);
				writeSelectorValue(_segMan, musicSlot->soundObj, SELECTOR(state), kSoundPlaying);
				return make_reg(0, 1);
			}
			return make_reg(0, 0);
		default:
			error("kDoSound(pause): parameter 0 is invalid for sound-sci0");
		}
	}

	reg_t obj = argv[0];
	uint16 value = argc > 1 ? argv[1].toUint16() : 0;
	if (!obj.getSegment()) {		// pause the whole playlist
		_music->pauseAll(value);
	} else {	// pause a playlist slot
		MusicEntry *musicSlot = _music->getSlot(obj);
		if (!musicSlot) {
			// This happens quite frequently
			debugC(kDebugLevelSound, "kDoSound(pause): Slot not found (%04x:%04x)", PRINT_REG(obj));
			return acc;
		}

		_music->soundToggle(musicSlot, value);
	}
	return acc;
}

// SCI0 only command
//  It's called right after restoring a game - it's responsible to kick off playing music again
//  we don't need this at all, so we don't do anything here
reg_t SoundCommandParser::kDoSoundResumeAfterRestore(int argc, reg_t *argv, reg_t acc) {
	return acc;
}

reg_t SoundCommandParser::kDoSoundMute(int argc, reg_t *argv, reg_t acc) {
	uint16 previousState = _music->soundGetSoundOn();
	if (argc > 0) {
		debugC(kDebugLevelSound, "kDoSound(mute): %d", argv[0].toUint16());
		_music->soundSetSoundOn(argv[0].toUint16());
	}

	return make_reg(0, previousState);
}

reg_t SoundCommandParser::kDoSoundMasterVolume(int argc, reg_t *argv, reg_t acc) {
	acc = make_reg(0, _music->soundGetMasterVolume());

	if (argc > 0) {
		debugC(kDebugLevelSound, "kDoSound(masterVolume): %d", argv[0].toSint16());
		int vol = CLIP<int16>(argv[0].toSint16(), 0, MUSIC_MASTERVOLUME_MAX);
		vol = vol * Audio::Mixer::kMaxMixerVolume / MUSIC_MASTERVOLUME_MAX;
		ConfMan.setInt("music_volume", vol);
		ConfMan.setInt("sfx_volume", vol);
		g_engine->syncSoundSettings();
	}
	return acc;
}

reg_t SoundCommandParser::kDoSoundFade(int argc, reg_t *argv, reg_t acc) {
	reg_t obj = argv[0];

	// The object can be null in several SCI0 games (e.g. Camelot, KQ1, KQ4, MUMG).
	// Check bugs #3035149, #3036942 and #3578335.
	// In this case, we just ignore the call.
	if (obj.isNull() && argc == 1)
		return acc;

	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		debugC(kDebugLevelSound, "kDoSound(fade): Slot not found (%04x:%04x)", PRINT_REG(obj));
		return acc;
	}

	int volume = musicSlot->volume;

	// If sound is not playing currently, set signal directly
	if (musicSlot->status != kSoundPlaying) {
		debugC(kDebugLevelSound, "kDoSound(fade): %04x:%04x fading requested, but sound is currently not playing", PRINT_REG(obj));
		writeSelectorValue(_segMan, obj, SELECTOR(signal), SIGNAL_OFFSET);
		return acc;
	}

	switch (argc) {
	case 1: // SCI0
		// SCI0 fades out all the time and when fadeout is done it will also
		// stop the music from playing
		musicSlot->fadeTo = 0;
		musicSlot->fadeStep = -5;
		musicSlot->fadeTickerStep = 10 * 16667 / _music->soundGetTempo();
		musicSlot->fadeTicker = 0;
		break;

	case 4: // SCI01+
	case 5: // SCI1+ (SCI1 late sound scheme), with fade and continue
		if (argc == 5) {
			// TODO: We currently treat this argument as a boolean, but may
			// have to handle different non-zero values differently. (e.g.,
			// some KQ6 scripts pass 3 here).
			// There is a script bug in KQ6, room 460 (the room with the flying
			// books). An object is passed here, which should not be treated as
			// a true flag. Fixes bugs #3555404 and #3291115.
			musicSlot->stopAfterFading = (argv[4].isNumber() && argv[4].toUint16() != 0);
		} else {
			musicSlot->stopAfterFading = false;
		}

		musicSlot->fadeTo = CLIP<uint16>(argv[1].toUint16(), 0, MUSIC_VOLUME_MAX);
		// Check if the song is already at the requested volume. If it is, don't
		// perform any fading. Happens for example during the intro of Longbow.
		if (musicSlot->fadeTo != musicSlot->volume) {
			// sometimes we get objects in that position, fix it up (ffs. workarounds)
			if (!argv[1].getSegment())
				musicSlot->fadeStep = volume > musicSlot->fadeTo ? -argv[3].toUint16() : argv[3].toUint16();
			else
				musicSlot->fadeStep = volume > musicSlot->fadeTo ? -5 : 5;
			musicSlot->fadeTickerStep = argv[2].toUint16() * 16667 / _music->soundGetTempo();
		} else {
			// Stop the music, if requested. Fixes bug #3555404.
			if (musicSlot->stopAfterFading)
				processStopSound(obj, false);
		}

		musicSlot->fadeTicker = 0;

		// WORKAROUND/HACK: In the labyrinth in KQ6, when falling in the pit and
		// lighting the lantern, the game scripts perform a fade in of the game
		// music, but set it to stop after fading. Remove that flag here. This is
		// marked as both a workaround and a hack because this issue could be a
		// problem with our fading code and an incorrect handling of that
		// parameter, or a script bug in that scene. Fixes bug #3267956.
		if (g_sci->getGameId() == GID_KQ6 && g_sci->getEngineState()->currentRoomNumber() == 406 &&
			musicSlot->resourceId == 400)
			musicSlot->stopAfterFading = false;
		break;

	default:
		error("kDoSound(fade): unsupported argc %d", argc);
	}

	debugC(kDebugLevelSound, "kDoSound(fade): %04x:%04x to %d, step %d, ticker %d", PRINT_REG(obj), musicSlot->fadeTo, musicSlot->fadeStep, musicSlot->fadeTickerStep);
	return acc;
}

reg_t SoundCommandParser::kDoSoundGetPolyphony(int argc, reg_t *argv, reg_t acc) {
	return make_reg(0, _music->soundGetVoices());	// Get the number of voices
}

reg_t SoundCommandParser::kDoSoundUpdate(int argc, reg_t *argv, reg_t acc) {
	reg_t obj = argv[0];

	debugC(kDebugLevelSound, "kDoSound(update): %04x:%04x", PRINT_REG(argv[0]));

	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("kDoSound(update): Slot not found (%04x:%04x)", PRINT_REG(obj));
		return acc;
	}

	musicSlot->loop = readSelectorValue(_segMan, obj, SELECTOR(loop));
	int16 objVol = CLIP<int>(readSelectorValue(_segMan, obj, SELECTOR(vol)), 0, 255);
	if (objVol != musicSlot->volume)
		_music->soundSetVolume(musicSlot, objVol);
	uint32 objPrio = readSelectorValue(_segMan, obj, SELECTOR(pri));
	if (objPrio != musicSlot->priority)
		_music->soundSetPriority(musicSlot, objPrio);
	return acc;
}

reg_t SoundCommandParser::kDoSoundUpdateCues(int argc, reg_t *argv, reg_t acc) {
	processUpdateCues(argv[0]);
	return acc;
}

void SoundCommandParser::processUpdateCues(reg_t obj) {
	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("kDoSound(updateCues): Slot not found (%04x:%04x)", PRINT_REG(obj));
		return;
	}

	if (musicSlot->pStreamAud) {
		// Update digital sound effect slots
		uint currentLoopCounter = 0;

		if (musicSlot->pLoopStream)
			currentLoopCounter = musicSlot->pLoopStream->getCompleteIterations();

		if (currentLoopCounter != musicSlot->sampleLoopCounter) {
			// during last time we looped at least one time, update loop accordingly
			musicSlot->loop -= currentLoopCounter - musicSlot->sampleLoopCounter;
			musicSlot->sampleLoopCounter = currentLoopCounter;
		}
		if (musicSlot->status == kSoundPlaying) {
			if (!_music->soundIsActive(musicSlot)) {
				processStopSound(obj, true);
			} else {
				_music->updateAudioStreamTicker(musicSlot);
			}
		} else if (musicSlot->status == kSoundPaused) {
			_music->updateAudioStreamTicker(musicSlot);
		}
		// We get a flag from MusicEntry::doFade() here to set volume for the stream
		if (musicSlot->fadeSetVolume) {
			_music->soundSetSampleVolume(musicSlot, musicSlot->volume);
			musicSlot->fadeSetVolume = false;
		}
	} else if (musicSlot->pMidiParser) {
		// Update MIDI slots
		if (musicSlot->signal == 0) {
			if (musicSlot->dataInc != readSelectorValue(_segMan, obj, SELECTOR(dataInc))) {
				if (SELECTOR(dataInc) > -1)
					writeSelectorValue(_segMan, obj, SELECTOR(dataInc), musicSlot->dataInc);
				writeSelectorValue(_segMan, obj, SELECTOR(signal), musicSlot->dataInc + 127);
			}
		} else {
			// Sync the signal of the sound object
			writeSelectorValue(_segMan, obj, SELECTOR(signal), musicSlot->signal);
			// We need to do this especially because state selector needs to get updated
			if (musicSlot->signal == SIGNAL_OFFSET)
				processStopSound(obj, false);
		}
	} else {
		// Slot actually has no data (which would mean that a sound-resource w/
		// unsupported data is used.
		//  (example lsl5 - sound resource 744 - it's Roland exclusive
		writeSelectorValue(_segMan, obj, SELECTOR(signal), SIGNAL_OFFSET);
		// If we don't set signal here, at least the switch to the mud wrestling
		// room in lsl5 will not work.
	}

	if (musicSlot->fadeCompleted) {
		musicSlot->fadeCompleted = false;
		// We need signal for sci0 at least in iceman as well (room 14,
		// fireworks).
		// It is also needed in other games, e.g. LSL6 when talking to the
		// receptionist (bug #3192166).
		writeSelectorValue(_segMan, obj, SELECTOR(signal), SIGNAL_OFFSET);
		if (_soundVersion <= SCI_VERSION_0_LATE) {
			processStopSound(obj, false);
		} else {
			if (musicSlot->stopAfterFading)
				processStopSound(obj, false);
		}
	}

	// Sync loop selector for SCI0
	if (_soundVersion <= SCI_VERSION_0_LATE)
		writeSelectorValue(_segMan, obj, SELECTOR(loop), musicSlot->loop);

	musicSlot->signal = 0;

	if (_soundVersion >= SCI_VERSION_1_EARLY) {
		writeSelectorValue(_segMan, obj, SELECTOR(min), musicSlot->ticker / 3600);
		writeSelectorValue(_segMan, obj, SELECTOR(sec), musicSlot->ticker % 3600 / 60);
		writeSelectorValue(_segMan, obj, SELECTOR(frame), musicSlot->ticker);
	}
}

reg_t SoundCommandParser::kDoSoundSendMidi(int argc, reg_t *argv, reg_t acc) {
	reg_t obj = argv[0];
	byte channel = argv[1].toUint16() & 0xf;
	byte midiCmd = argv[2].toUint16() & 0xff;

	// TODO: first there is a 4-parameter variant of this call which needs to get reversed
	//        second the current code isn't 100% accurate, sierra sci does checks on the 4th parameter
	if (argc == 4)
		return acc;

	uint16 controller = argv[3].toUint16();
	uint16 param = argv[4].toUint16();

	debugC(kDebugLevelSound, "kDoSound(sendMidi): %04x:%04x, %d, %d, %d, %d", PRINT_REG(obj), channel, midiCmd, controller, param);
	if (channel)
		channel--; // channel is given 1-based, we are using 0-based

	uint32 midiCommand = (channel | midiCmd) | ((uint32)controller << 8) | ((uint32)param << 16);

	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		// TODO: maybe it's possible to call this with obj == 0:0 and send directly?!
		// if so, allow it
		//_music->sendMidiCommand(_midiCommand);
		warning("kDoSound(sendMidi): Slot not found (%04x:%04x)", PRINT_REG(obj));
		return acc;
	}
	_music->sendMidiCommand(musicSlot, midiCommand);
	return acc;
}

reg_t SoundCommandParser::kDoSoundGlobalReverb(int argc, reg_t *argv, reg_t acc) {
	byte prevReverb = _music->getCurrentReverb();
	byte reverb = argv[0].toUint16() & 0xF;

	if (argc == 1) {
		debugC(kDebugLevelSound, "doSoundGlobalReverb: %d", argv[0].toUint16() & 0xF);
		if (reverb <= 10)
			_music->setGlobalReverb(reverb);
	}

	return make_reg(0, prevReverb);
}

reg_t SoundCommandParser::kDoSoundSetHold(int argc, reg_t *argv, reg_t acc) {
	reg_t obj = argv[0];

	debugC(kDebugLevelSound, "doSoundSetHold: %04x:%04x, %d", PRINT_REG(argv[0]), argv[1].toUint16());

	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("kDoSound(setHold): Slot not found (%04x:%04x)", PRINT_REG(obj));
		return acc;
	}

	// Set the special hold marker ID where the song should be looped at.
	musicSlot->hold = argv[1].toSint16();
	return acc;
}

reg_t SoundCommandParser::kDoSoundGetAudioCapability(int argc, reg_t *argv, reg_t acc) {
	// Tests for digital audio support
	return make_reg(0, 1);
}

reg_t SoundCommandParser::kDoSoundStopAll(int argc, reg_t *argv, reg_t acc) {
	// TODO: this can't be right, this gets called in kq1 - e.g. being in witch house, getting the note
	//  now the point jingle plays and after a messagebox they call this - and would stop the background effects with it
	//  this doesn't make sense, so i disable it for now
	return acc;

	Common::StackLock(_music->_mutex);

	const MusicList::iterator end = _music->getPlayListEnd();
	for (MusicList::iterator i = _music->getPlayListStart(); i != end; ++i) {
		if (_soundVersion <= SCI_VERSION_0_LATE) {
			writeSelectorValue(_segMan, (*i)->soundObj, SELECTOR(state), kSoundStopped);
		} else {
			writeSelectorValue(_segMan, (*i)->soundObj, SELECTOR(handle), 0);
			writeSelectorValue(_segMan, (*i)->soundObj, SELECTOR(signal), SIGNAL_OFFSET);
		}

		(*i)->dataInc = 0;
		_music->soundStop(*i);
	}
	return acc;
}

reg_t SoundCommandParser::kDoSoundSetVolume(int argc, reg_t *argv, reg_t acc) {
	reg_t obj = argv[0];
	int16 value = argv[1].toSint16();

	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		// Do not throw a warning if the sound can't be found, as in some games
		// this is called before the actual sound is loaded (e.g. SQ4CD, with
		// the drum sounds of the energizer bunny at the beginning), so this is
		// normal behavior.
		//warning("cmdSetSoundVolume: Slot not found (%04x:%04x)", PRINT_REG(obj));
		return acc;
	}

	debugC(kDebugLevelSound, "kDoSound(setVolume): %d", value);

	value = CLIP<int>(value, 0, MUSIC_VOLUME_MAX);

	if (musicSlot->volume != value) {
		musicSlot->volume = value;
		_music->soundSetVolume(musicSlot, value);
		writeSelectorValue(_segMan, obj, SELECTOR(vol), value);
	}
	return acc;
}

reg_t SoundCommandParser::kDoSoundSetPriority(int argc, reg_t *argv, reg_t acc) {
	reg_t obj = argv[0];
	int16 value = argv[1].toSint16();

	debugC(kDebugLevelSound, "kDoSound(setPriority): %04x:%04x, %d", PRINT_REG(obj), value);

	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		debugC(kDebugLevelSound, "kDoSound(setPriority): Slot not found (%04x:%04x)", PRINT_REG(obj));
		return acc;
	}

	if (value == -1) {
		uint16 resourceId = musicSlot->resourceId;

		// Set priority from the song data
		Resource *song = _resMan->findResource(ResourceId(kResourceTypeSound, resourceId), 0);
		if (song->data[0] == 0xf0)
			_music->soundSetPriority(musicSlot, song->data[1]);
		else
			warning("kDoSound(setPriority): Attempt to unset song priority when there is no built-in value");

		//pSnd->prio=0;field_15B=0
		writeSelectorValue(_segMan, obj, SELECTOR(flags), readSelectorValue(_segMan, obj, SELECTOR(flags)) & 0xFD);
	} else {
		// Scripted priority

		//pSnd->field_15B=1;
		writeSelectorValue(_segMan, obj, SELECTOR(flags), readSelectorValue(_segMan, obj, SELECTOR(flags)) | 2);
		//DoSOund(0xF,hobj,w)
	}
	return acc;
}

reg_t SoundCommandParser::kDoSoundSetLoop(int argc, reg_t *argv, reg_t acc) {
	reg_t obj = argv[0];
	int16 value = argv[1].toSint16();

	debugC(kDebugLevelSound, "kDoSound(setLoop): %04x:%04x, %d", PRINT_REG(obj), value);

	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		// Apparently, it's perfectly normal for a game to call cmdSetSoundLoop
		// before actually initializing the sound and adding it to the playlist
		// with cmdInitSound. Usually, it doesn't matter if the game doesn't
		// request to loop the sound, so in this case, don't throw any warning,
		// otherwise do, because the sound won't be looped.
		if (value == -1) {
			warning("kDoSound(setLoop): Slot not found (%04x:%04x) and the song was requested to be looped", PRINT_REG(obj));
		} else {
			// Doesn't really matter
		}
		return acc;
	}
	if (value == -1) {
		musicSlot->loop = 0xFFFF;
	} else {
		musicSlot->loop = 1; // actually plays the music once
	}

	writeSelectorValue(_segMan, obj, SELECTOR(loop), musicSlot->loop);
	return acc;
}

reg_t SoundCommandParser::kDoSoundSuspend(int argc, reg_t *argv, reg_t acc) {
	// TODO
	warning("kDoSound(suspend): STUB");
	return acc;
}

void SoundCommandParser::updateSci0Cues() {
	bool noOnePlaying = true;
	MusicEntry *pWaitingForPlay = NULL;

	const MusicList::iterator end = _music->getPlayListEnd();
	for (MusicList::iterator i = _music->getPlayListStart(); i != end; ++i) {
		// Is the sound stopped, and the sound object updated too? If yes, skip
		// this sound, as SCI0 only allows one active song.
		if  ((*i)->isQueued) {
			pWaitingForPlay = (*i);
			// FIXME(?): In iceman 2 songs are queued when playing the door
			// sound - if we use the first song for resuming then it's the wrong
			// one. Both songs have same priority. Maybe the new sound function
			// in sci0 is somehow responsible.
			continue;
		}
		if ((*i)->signal == 0 && (*i)->status != kSoundPlaying)
			continue;

		processUpdateCues((*i)->soundObj);
		noOnePlaying = false;
	}

	if (noOnePlaying && pWaitingForPlay) {
		// If there is a queued entry, play it now ffs: SciMusic::soundPlay()
		pWaitingForPlay->isQueued = false;
		_music->soundPlay(pWaitingForPlay);
	}
}

void SoundCommandParser::clearPlayList() {
	_music->clearPlayList();
}

void SoundCommandParser::printPlayList(Console *con) {
	_music->printPlayList(con);
}

void SoundCommandParser::printSongInfo(reg_t obj, Console *con) {
	_music->printSongInfo(obj, con);
}

void SoundCommandParser::stopAllSounds() {
	_music->stopAll();
}

void SoundCommandParser::startNewSound(int number) {
	Common::StackLock lock(_music->_mutex);

	// Overwrite the first sound in the playlist
	MusicEntry *song = *_music->getPlayListStart();
	reg_t soundObj = song->soundObj;
	processDisposeSound(soundObj);
	writeSelectorValue(_segMan, soundObj, SELECTOR(number), number);
	processInitSound(soundObj);
	processPlaySound(soundObj);
}

void SoundCommandParser::setMasterVolume(int vol) {
	// 0...15
	_music->soundSetMasterVolume(vol);
}

void SoundCommandParser::pauseAll(bool pause) {
	_music->pauseAll(pause);
}

MusicType SoundCommandParser::getMusicType() const {
	assert(_music);
	return _music->soundGetMusicType();
}

} // End of namespace Sci
