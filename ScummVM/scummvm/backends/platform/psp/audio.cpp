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

#include <pspthreadman.h>
#include <pspaudio.h>

#include "common/scummsys.h"
#include "backends/platform/psp/audio.h"

//#define __PSP_DEBUG_FUNCS__	/* For debugging function calls */
//#define __PSP_DEBUG_PRINT__	/* For debug printouts */

#include "backends/platform/psp/trace.h"

bool PspAudio::open(uint32 freq, uint32 numOfChannels, uint32 numOfSamples, callbackFunc callback, void *userData) {
	DEBUG_ENTER_FUNC();
	if (_init) {
		PSP_ERROR("audio device already initialized\n");
		return true;
	}

	PSP_DEBUG_PRINT("freq[%d], numOfChannels[%d], numOfSamples[%d], callback[%p], userData[%x]\n",
			freq, numOfChannels, numOfSamples, callback, (uint32)userData);

	numOfSamples = PSP_AUDIO_SAMPLE_ALIGN(numOfSamples);
	uint32 bufLen = numOfSamples * numOfChannels * NUM_BUFFERS * sizeof(uint16);

	PSP_DEBUG_PRINT("total buffer size[%d]\n", bufLen);

	_buffers[0] = (byte *)memalign(64, bufLen);
	if (!_buffers[0]) {
		PSP_ERROR("failed to allocate memory for audio buffers\n");
		return false;
	}
	memset(_buffers[0], 0, bufLen);	// clean the buffer

	// Fill in the rest of the buffer pointers
	byte *pBuffer = _buffers[0];
	for (int i = 1; i < NUM_BUFFERS; i++) {
		pBuffer += numOfSamples * numOfChannels * sizeof(uint16);
		_buffers[i] = pBuffer;
	}

	// Reserve a HW channel for our audio
	_pspChannel = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, numOfSamples, numOfChannels == 2 ? PSP_AUDIO_FORMAT_STEREO : PSP_AUDIO_FORMAT_MONO);
	if (_pspChannel < 0) {
		PSP_ERROR("failed to reserve audio channel\n");
		return false;
	}

	PSP_DEBUG_PRINT("reserved channel[%d] for audio\n", _pspChannel);

	// Save our data
	_numOfChannels = numOfChannels;
	_numOfSamples = numOfSamples;
	_bufferSize = numOfSamples * numOfChannels * sizeof(uint16);	// should be the right size to send the app
	_callback = callback;
	_userData = userData;
	_bufferToFill = 0;
	_bufferToPlay = 0;

	_init = true;
	_paused = true;	// start in paused mode

	threadCreateAndStart("audioThread", PRIORITY_AUDIO_THREAD, STACK_AUDIO_THREAD);	// start the consumer thread

	return true;
}

// The real thread function
void PspAudio::threadFunction() {
	assert(_callback);
	PSP_DEBUG_PRINT_FUNC("audio thread started\n");

	while (_init) {		// Keep looping so long as we haven't been told to stop
		if (_paused)
			PSP_DEBUG_PRINT("audio thread paused\n");
		while (_paused) {	// delay until we stop pausing
			PspThread::delayMicros(100000);	// 100ms
			if (!_paused)
				PSP_DEBUG_PRINT("audio thread unpaused\n");
		}

		PSP_DEBUG_PRINT("remaining samples[%d]\n", _remainingSamples);

		PSP_DEBUG_PRINT("filling buffer[%d]\n", _bufferToFill);
		_callback(_userData, _buffers[_bufferToFill], _bufferSize); // ask mixer to fill in data
		nextBuffer(_bufferToFill);

		PSP_DEBUG_PRINT("playing buffer[%d].\n", _bufferToPlay);
		playBuffer();
		nextBuffer(_bufferToPlay);
	} // while _init

	// destroy everything
	free(_buffers[0]);
	sceAudioChRelease(_pspChannel);
	PSP_DEBUG_PRINT("audio thread exiting. ****************************\n");
}

// Much faster than using %, especially with conditional moves (MIPS)
inline void PspAudio::nextBuffer(int &bufferIdx) {
	DEBUG_ENTER_FUNC();
	bufferIdx++;
	if (bufferIdx >= NUM_BUFFERS)
		bufferIdx = 0;
}

// Don't do it with blocking
inline bool PspAudio::playBuffer() {
	DEBUG_ENTER_FUNC();
	int ret;
	if (_numOfChannels == 1)
		ret = sceAudioOutputBlocking(_pspChannel, PSP_AUDIO_VOLUME_MAX, _buffers[_bufferToPlay]);
	else
		ret = sceAudioOutputPannedBlocking(_pspChannel, PSP_AUDIO_VOLUME_MAX, PSP_AUDIO_VOLUME_MAX, _buffers[_bufferToPlay]);

	if (ret < 0) {
		PSP_ERROR("failed to output audio. Error[%d]\n", ret);
		return false;
	}
	return true;
}

void PspAudio::close() {
	PSP_DEBUG_PRINT("close has been called ***************\n");
	_init = false;
}
