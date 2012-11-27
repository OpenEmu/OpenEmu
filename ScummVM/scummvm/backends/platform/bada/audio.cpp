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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <FSysSettingInfo.h>
#include <FAppAppRegistry.h>

#include "backends/platform/bada/audio.h"
#include "backends/platform/bada/system.h"

#define TIMER_INCREMENT		 10
#define TIMER_INTERVAL		 40
#define MIN_TIMER_INTERVAL 10
#define MAX_TIMER_INTERVAL 160
#define INIT_LEVEL				 3
#define CONFIG_KEY				 L"audiovol"

// sound level pre-sets
const int levels[] = {0, 1, 10, 45, 70, 99};

AudioThread::AudioThread() :
	_mixer(0),
	_timer(0),
	_audioOut(0),
	_head(0),
	_tail(0),
	_ready(0),
	_interval(TIMER_INTERVAL),
	_playing(-1),
	_muted(true) {
}

Audio::MixerImpl *AudioThread::Construct(OSystem *system) {
	logEntered();

	if (IsFailed(Thread::Construct(THREAD_TYPE_EVENT_DRIVEN,
																 DEFAULT_STACK_SIZE,
																 THREAD_PRIORITY_HIGH))) {
		AppLog("Failed to create AudioThread");
		return NULL;
	}

	_mixer = new Audio::MixerImpl(system, 44100);
	return _mixer;
}

AudioThread::~AudioThread() {
	logEntered();
}

bool AudioThread::isSilentMode() {
	bool silentMode;
	String key(L"SilentMode");
	Osp::System::SettingInfo::GetValue(key, silentMode);
	return silentMode;
}

void AudioThread::setMute(bool on) {
	if (_audioOut && _timer) {
		_muted = on;
		if (on) {
			_timer->Cancel();
		} else {
			_timer->Start(_interval);
		}
	}
}

int AudioThread::setVolume(bool up, bool minMax) {
	int level = -1;
	int numLevels = sizeof(levels) / sizeof(levels[0]);

	if (_audioOut) {
		int volume = _audioOut->GetVolume();
		if (minMax) {
			level = up ? numLevels - 1 : 0;
			volume = levels[level];
		} else {
			// adjust volume to be one of the preset values
			for (int i = 0; i < numLevels && level == -1; i++) {
				if (volume == levels[i]) {
					level = i;
					if (up) {
						if (i + 1 < numLevels) {
							level = i + 1;
						}
					} else if (i > 0) {
						level = i - 1;
					}
				}
			}

			// default to INIT_LEVEL when current not preset value
			if (level == -1) {
				level = INIT_LEVEL;
			}
			volume = levels[level];
		}

		_audioOut->SetVolume(volume);

		// remember the chosen setting
		AppRegistry *registry = Application::GetInstance()->GetAppRegistry();
		if (registry) {
			registry->Set(CONFIG_KEY, volume);
		}
	}
	return level;
}

bool AudioThread::OnStart(void) {
	logEntered();

	_audioOut = new Osp::Media::AudioOut();
	if (!_audioOut ||
			IsFailed(_audioOut->Construct(*this))) {
		AppLog("Failed to create AudioOut");
		return false;
	}

	int sampleRate = _mixer->getOutputRate();

	// ideally we would update _mixer with GetOptimizedSampleRate here
	if (_audioOut->GetOptimizedSampleRate() != sampleRate) {
		AppLog("Non optimal sample rate %d", _audioOut->GetOptimizedSampleRate());
	}

	if (IsFailed(_audioOut->Prepare(AUDIO_TYPE_PCM_S16_LE,
																 AUDIO_CHANNEL_TYPE_STEREO,
																 sampleRate))) {
		AppLog("Failed to prepare AudioOut %d", sampleRate);
		return false;
	}

	int bufferSize = _audioOut->GetMinBufferSize();
	for (int i = 0; i < NUM_AUDIO_BUFFERS; i++) {
		if (IsFailed(_audioBuffer[i].Construct(bufferSize))) {
			AppLog("Failed to create audio buffer");
			return false;
		}
	}

	_timer = new Timer();
	if (!_timer || IsFailed(_timer->Construct(*this))) {
		AppLog("Failed to create audio timer");
		return false;
	}

	if (IsFailed(_timer->Start(_interval))) {
		AppLog("failed to start audio timer");
		return false;
	}

	// get the volume from the app-registry
	int volume = levels[INIT_LEVEL];
	AppRegistry *registry = Application::GetInstance()->GetAppRegistry();
	if (registry) {
		if (E_KEY_NOT_FOUND == registry->Get(CONFIG_KEY, volume)) {
			registry->Add(CONFIG_KEY, volume);
			volume = levels[INIT_LEVEL];
		} else {
			AppLog("Setting volume: %d", volume);
		}
	}

	_muted = false;
	_mixer->setReady(true);
	_audioOut->SetVolume(isSilentMode() ? 0 : volume);
	_audioOut->Start();
	return true;
}

void AudioThread::OnStop(void) {
	logEntered();

	_mixer->setReady(false);

	if (_timer) {
		if (!_muted) {
			_timer->Cancel();
		}
		delete _timer;
	}

	if (_audioOut) {
		_audioOut->Reset();
		delete _audioOut;
	}
}

void AudioThread::OnAudioOutErrorOccurred(Osp::Media::AudioOut &src, result r) {
	logEntered();
}

void AudioThread::OnAudioOutInterrupted(Osp::Media::AudioOut &src) {
	logEntered();
}

void AudioThread::OnAudioOutReleased(Osp::Media::AudioOut &src) {
	logEntered();
	_audioOut->Start();
}

void AudioThread::OnAudioOutBufferEndReached(Osp::Media::AudioOut &src) {
	if (_ready > 0) {
		_playing = _tail;
		_audioOut->WriteBuffer(_audioBuffer[_tail]);
		_tail = (_tail + 1) % NUM_AUDIO_BUFFERS;
		_ready--;
	} else {
		// audio buffer empty: decrease timer inverval
		_playing = -1;
		_interval -= TIMER_INCREMENT;
		if (_interval < MIN_TIMER_INTERVAL) {
			_interval = MIN_TIMER_INTERVAL;
		}
	}
}

void AudioThread::OnTimerExpired(Timer &timer) {
	if (_ready < NUM_AUDIO_BUFFERS) {
		uint len = _audioBuffer[_head].GetCapacity();
		int samples = _mixer->mixCallback((byte *)_audioBuffer[_head].GetPointer(), len);
		if (samples) {
			_head = (_head + 1) % NUM_AUDIO_BUFFERS;
			_ready++;
		}
	} else {
		// audio buffer full: increase timer inverval
		_interval += TIMER_INCREMENT;
		if (_interval > MAX_TIMER_INTERVAL) {
			_interval = MAX_TIMER_INTERVAL;
		}
	}

	if (_ready && _playing == -1) {
		OnAudioOutBufferEndReached(*_audioOut);
	}

	_timer->Start(_interval);
}

