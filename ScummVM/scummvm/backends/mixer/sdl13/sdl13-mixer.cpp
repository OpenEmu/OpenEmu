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

#include "common/scummsys.h"

#if defined(SDL_BACKEND)

#include "backends/mixer/sdl13/sdl13-mixer.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "common/textconsole.h"

#ifdef GP2X
#define SAMPLES_PER_SEC 11025
#else
#define SAMPLES_PER_SEC 22050
#endif
//#define SAMPLES_PER_SEC 44100

Sdl13MixerManager::Sdl13MixerManager()
	:
	SdlMixerManager(),
	_device(0) {

}

Sdl13MixerManager::~Sdl13MixerManager() {
	_mixer->setReady(false);

	SDL_CloseAudioDevice(_device);

	delete _mixer;
}

void Sdl13MixerManager::init() {
	// Start SDL Audio subsystem
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
		error("Could not initialize SDL: %s", SDL_GetError());
	}

	// Get the desired audio specs
	SDL_AudioSpec desired = getAudioSpec(SAMPLES_PER_SEC);

	// Start SDL audio with the desired specs
	_device = SDL_OpenAudioDevice(NULL, 0, &desired, &_obtained,
			SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

	if (_device <= 0) {
		warning("Could not open audio device: %s", SDL_GetError());

		_mixer = new Audio::MixerImpl(g_system, desired.freq);
		assert(_mixer);
		_mixer->setReady(false);
	} else {
		debug(1, "Output sample rate: %d Hz", _obtained.freq);

		_mixer = new Audio::MixerImpl(g_system, _obtained.freq);
		assert(_mixer);
		_mixer->setReady(true);

		startAudio();
	}
}

void Sdl13MixerManager::startAudio() {
	// Start the sound system
	SDL_PauseAudioDevice(_device, 0);
}

void Sdl13MixerManager::suspendAudio() {
	SDL_CloseAudioDevice(_device);
	_audioSuspended = true;
}

int Sdl13MixerManager::resumeAudio() {
	if (!_audioSuspended)
		return -2;

	_device = SDL_OpenAudioDevice(NULL, 0, &_obtained, NULL, 0);
	if (_device <= 0) {
		return -1;
	}

	SDL_PauseAudioDevice(_device, 0);
	_audioSuspended = false;
	return 0;
}

#endif
