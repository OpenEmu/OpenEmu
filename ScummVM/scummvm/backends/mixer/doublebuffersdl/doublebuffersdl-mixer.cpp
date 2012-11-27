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

#if defined(MACOSX) || defined(GP2X) || defined(CAANOO) || defined(GP2XWIZ)

#include "backends/mixer/doublebuffersdl/doublebuffersdl-mixer.h"

DoubleBufferSDLMixerManager::DoubleBufferSDLMixerManager()
	:
	_soundMutex(0), _soundCond(0), _soundThread(0),
	_soundThreadIsRunning(false), _soundThreadShouldQuit(false) {

}

DoubleBufferSDLMixerManager::~DoubleBufferSDLMixerManager() {
	deinitThreadedMixer();
}

void DoubleBufferSDLMixerManager::startAudio() {
	_soundThreadIsRunning = false;
	_soundThreadShouldQuit = false;

	// Create mutex and condition variable
	_soundMutex = SDL_CreateMutex();
	_soundCond = SDL_CreateCond();

	// Create two sound buffers
	_activeSoundBuf = 0;
	uint bufSize = _obtained.samples * 4;
	_soundBufSize = bufSize;
	_soundBuffers[0] = (byte *)calloc(1, bufSize);
	_soundBuffers[1] = (byte *)calloc(1, bufSize);

	_soundThreadIsRunning = true;

	// Finally start the thread
	_soundThread = SDL_CreateThread(mixerProducerThreadEntry, this);

	SdlMixerManager::startAudio();
}

void DoubleBufferSDLMixerManager::mixerProducerThread() {
	byte nextSoundBuffer;

	SDL_LockMutex(_soundMutex);
	while (true) {
		// Wait till we are allowed to produce data
		SDL_CondWait(_soundCond, _soundMutex);

		if (_soundThreadShouldQuit)
			break;

		// Generate samples and put them into the next buffer
		nextSoundBuffer = _activeSoundBuf ^ 1;
		_mixer->mixCallback(_soundBuffers[nextSoundBuffer], _soundBufSize);

		// Swap buffers
		_activeSoundBuf = nextSoundBuffer;
	}
	SDL_UnlockMutex(_soundMutex);
}

int SDLCALL DoubleBufferSDLMixerManager::mixerProducerThreadEntry(void *arg) {
	DoubleBufferSDLMixerManager *mixer = (DoubleBufferSDLMixerManager *)arg;
	assert(mixer);
	mixer->mixerProducerThread();
	return 0;
}

void DoubleBufferSDLMixerManager::deinitThreadedMixer() {
	// Kill thread?? _soundThread

	if (_soundThreadIsRunning) {
		// Signal the producer thread to end, and wait for it to actually finish.
		_soundThreadShouldQuit = true;
		SDL_CondBroadcast(_soundCond);
		SDL_WaitThread(_soundThread, NULL);

		// Kill the mutex & cond variables.
		// Attention: AT this point, the mixer callback must not be running
		// anymore, else we will crash!
		SDL_DestroyMutex(_soundMutex);
		SDL_DestroyCond(_soundCond);

		_soundThreadIsRunning = false;

		free(_soundBuffers[0]);
		free(_soundBuffers[1]);
	}
}

void DoubleBufferSDLMixerManager::callbackHandler(byte *samples, int len) {
	assert(_mixer);
	assert((int)_soundBufSize == len);

	// Lock mutex, to ensure our data is not overwritten by the producer thread
	SDL_LockMutex(_soundMutex);

	// Copy data from the current sound buffer
	memcpy(samples, _soundBuffers[_activeSoundBuf], len);

	// Unlock mutex and wake up the produced thread
	SDL_UnlockMutex(_soundMutex);
	SDL_CondSignal(_soundCond);
}

#endif
