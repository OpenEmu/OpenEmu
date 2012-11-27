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

#ifdef _WIN32_WCE

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/config-manager.h"
#include "backends/platform/wince/wince-sdl.h"
#include "backends/mixer/wincesdl/wincesdl-mixer.h"
#include "common/system.h"
#include "common/textconsole.h"

#ifdef USE_VORBIS
#ifndef USE_TREMOR
#include <vorbis/vorbisfile.h>
#else
#ifdef USE_TREMOLO
#include <tremolo/ivorbisfile.h>
#else
#include <tremor/ivorbisfile.h>
#endif
#endif
#endif

#define SAMPLES_PER_SEC_OLD 11025
#define SAMPLES_PER_SEC_NEW 22050

WINCESdlMixerManager::WINCESdlMixerManager() {

}

WINCESdlMixerManager::~WINCESdlMixerManager() {

}

void WINCESdlMixerManager::init() {
	SDL_AudioSpec desired;
	int thread_priority;

	uint32 sampleRate = compute_sample_rate();
	if (sampleRate == 0)
		warning("OSystem_WINCE3::setupMixer called with sample rate 0 - audio will not work");
	else if (_mixer && _mixer->getOutputRate() == sampleRate) {
		debug(1, "Skipping sound mixer re-init: samplerate is good");
		return;
	}

	memset(&desired, 0, sizeof(desired));
	desired.freq = sampleRate;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = 128;
	desired.callback = private_sound_proc;
	desired.userdata = this;

	// Create the mixer instance
	if (_mixer == 0)
		_mixer = new Audio::MixerImpl(g_system, sampleRate);

	// Add sound thread priority
	if (!ConfMan.hasKey("sound_thread_priority"))
		thread_priority = THREAD_PRIORITY_NORMAL;
	else
		thread_priority = ConfMan.getInt("sound_thread_priority");

	desired.thread_priority = thread_priority;

	SDL_CloseAudio();
	if (SDL_OpenAudio(&desired, NULL) != 0) {
		warning("Could not open audio device: %s", SDL_GetError());
		_mixer->setReady(false);

	} else {
		debug(1, "Sound opened OK, mixing at %d Hz", sampleRate);

		// Re-create mixer to match the output rate
		int vol1 = _mixer->getVolumeForSoundType(Audio::Mixer::kPlainSoundType);
		int vol2 = _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType);
		int vol3 = _mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType);
		int vol4 = _mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType);
		delete _mixer;
		_mixer = new Audio::MixerImpl(g_system, sampleRate);
		_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, vol1);
		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, vol2);
		_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, vol3);
		_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, vol4);
		_mixer->setReady(true);
		SDL_PauseAudio(0);
	}
}

void WINCESdlMixerManager::private_sound_proc(void *param, byte *buf, int len) {
	WINCESdlMixerManager *this_ = (WINCESdlMixerManager *)param;
	assert(this_);

	if (this_->_mixer)
		this_->_mixer->mixCallback(buf, len);
	if (!OSystem_WINCE3::_soundMaster)
		memset(buf, 0, len);
}

uint32 WINCESdlMixerManager::compute_sample_rate() {
	uint32 sampleRate;

	// Force at least medium quality FM synthesis for FOTAQ
	Common::String gameid(ConfMan.get("gameid"));
	if (gameid == "queen") {
		if (!((ConfMan.hasKey("FM_high_quality") && ConfMan.getBool("FM_high_quality")) ||
			(ConfMan.hasKey("FM_medium_quality") && ConfMan.getBool("FM_medium_quality")))) {
			ConfMan.setBool("FM_medium_quality", true);
			ConfMan.flushToDisk();
		}
	} else {
		if (!ConfMan.hasKey("FM_high_quality") && !ConfMan.hasKey("FM_medium_quality")) {
			ConfMan.setBool("FM_high_quality", true);
			ConfMan.flushToDisk();
		}
	}
	// See if the output frequency is forced by the game
	if (gameid == "ft" || gameid == "dig" || gameid == "comi" || gameid == "queen" || gameid == "sword" || gameid == "agi")
			sampleRate = SAMPLES_PER_SEC_NEW;
	else {
		if (ConfMan.hasKey("high_sample_rate") && !ConfMan.getBool("high_sample_rate"))
			sampleRate = SAMPLES_PER_SEC_OLD;
		else
			sampleRate = SAMPLES_PER_SEC_NEW;
	}

#ifdef USE_VORBIS
	// Modify the sample rate on the fly if OGG is involved
	if (sampleRate == SAMPLES_PER_SEC_OLD)
		if (checkOggHighSampleRate())
			 sampleRate = SAMPLES_PER_SEC_NEW;
#endif

	return sampleRate;
}

#ifdef USE_VORBIS
bool WINCESdlMixerManager::checkOggHighSampleRate() {
	char trackFile[255];
	FILE *testFile;
	OggVorbis_File *test_ov_file = new OggVorbis_File;

	// FIXME: The following sprintf assumes that "path" is always
	// terminated by a path separator. This is *not* true in general.
	// This code really should check for the path separator, or even
	// better, use the FSNode API.
	sprintf(trackFile, "%sTrack1.ogg", ConfMan.get("path").c_str());
	// Check if we have an OGG audio track
	testFile = fopen(trackFile, "rb");
	if (testFile) {
		if (!ov_open(testFile, test_ov_file, NULL, 0)) {
			bool highSampleRate = (ov_info(test_ov_file, -1)->rate == 22050);
			ov_clear(test_ov_file);
			delete test_ov_file;
			return highSampleRate;
		}
	}

	// Do not test for OGG samples - too big and too slow anyway :)

	delete test_ov_file;
	return false;
}
#endif

#endif
