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

#include "osys_n64.h"
#include "backends/timer/default/default-timer.h"

void checkTimers(void) {
	OSystem_N64 *osys = (OSystem_N64 *)g_system;

	uint32 curTime = osys->getMillis();

	// Timer checking & firing
	if (osys->_timerCallback && (curTime >= osys->_timerCallbackNext)) {
		osys->_timerCallback(osys->_timerCallbackTimer);
		osys->_timerCallbackNext = curTime + osys->_timerCallbackTimer;
	}
}

void disableAudioPlayback(void) {
	if (!_audioEnabled) return;

	_audioEnabled = false;

	while (AI_busy()); // Wait for audio to stop
}

void enableAudioPlayback(void) {
	static bool _firstRun = true;

	OSystem_N64 *osys = (OSystem_N64 *)g_system;
	Audio::MixerImpl *localmixer = (Audio::MixerImpl *)osys->getMixer();

	uint32 sampleBufferSize = 3072;

	initAudioInterface(osys->_viClockRate, DEFAULT_SOUND_SAMPLE_RATE, 16, sampleBufferSize);
	osys->_audioBufferSize = getAIBufferSize();

	if (_firstRun) {
		localmixer->setReady(true);
		_firstRun = false;
	}

	disable_interrupts();

	_audioEnabled = true;

	sndCallback();
	sndCallback();

	registerAIhandler(sndCallback); // Lib checks if i try to register it multiple times

	enable_interrupts();
}

static volatile Uint32 _requiredSoundSlots = 0;

void vblCallback(void) {
	// Switch display buffer
	switchDisplayBuffer();

	// If audio buffer got depleted, ask for more slots to refill.
	if (_audioEnabled && !AI_busy() && !_requiredSoundSlots) {
		sndCallback();
		sndCallback();
	}

	((OSystem_N64 *)g_system)->readControllerAnalogInput();
}

void sndCallback() {
	// Signal that an audio buffer finished playing and that we need more samples
	if (_requiredSoundSlots < 2)
		_requiredSoundSlots++;
}

void refillAudioBuffers(void) {
	if (!_audioEnabled) return;

	OSystem_N64 *osys = (OSystem_N64 *)g_system;
	byte *sndBuf;
	Audio::MixerImpl *localmixer = (Audio::MixerImpl *)osys->getMixer();

	while (_requiredSoundSlots) {
		sndBuf = (byte *)getAIBuffer();

		localmixer->mixCallback((byte *)sndBuf, osys->_audioBufferSize);

		putAIBuffer();

		_requiredSoundSlots--;
	}
}

int timer_handler(int t) {
	DefaultTimerManager *tm = (DefaultTimerManager *)g_system->getTimerManager();
	tm->handler();
	return t;
}
