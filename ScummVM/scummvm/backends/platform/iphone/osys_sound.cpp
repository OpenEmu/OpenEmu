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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "osys_main.h"

void OSystem_IPHONE::AQBufferCallback(void *in, AudioQueueRef inQ, AudioQueueBufferRef outQB) {
	//printf("AQBufferCallback()\n");
	if (s_AudioQueue.frameCount > 0 && s_soundCallback != NULL) {
		outQB->mAudioDataByteSize = 4 * s_AudioQueue.frameCount;
		s_soundCallback(s_soundParam, (byte *)outQB->mAudioData, outQB->mAudioDataByteSize);
		AudioQueueEnqueueBuffer(inQ, outQB, 0, NULL);
	} else {
		AudioQueueStop(s_AudioQueue.queue, false);
	}
}

void OSystem_IPHONE::mixCallback(void *sys, byte *samples, int len) {
	OSystem_IPHONE *this_ = (OSystem_IPHONE *)sys;
	assert(this_);

	if (this_->_mixer) {
		this_->_mixer->mixCallback(samples, len);
	}
}

void OSystem_IPHONE::setupMixer() {
	_mixer = new Audio::MixerImpl(this, AUDIO_SAMPLE_RATE);

	s_soundCallback = mixCallback;
	s_soundParam = this;

	startSoundsystem();
}

void OSystem_IPHONE::startSoundsystem() {
	s_AudioQueue.dataFormat.mSampleRate = AUDIO_SAMPLE_RATE;
	s_AudioQueue.dataFormat.mFormatID = kAudioFormatLinearPCM;
	s_AudioQueue.dataFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	s_AudioQueue.dataFormat.mBytesPerPacket = 4;
	s_AudioQueue.dataFormat.mFramesPerPacket = 1;
	s_AudioQueue.dataFormat.mBytesPerFrame = 4;
	s_AudioQueue.dataFormat.mChannelsPerFrame = 2;
	s_AudioQueue.dataFormat.mBitsPerChannel = 16;
	s_AudioQueue.frameCount = WAVE_BUFFER_SIZE;

	if (AudioQueueNewOutput(&s_AudioQueue.dataFormat, AQBufferCallback, &s_AudioQueue, 0, kCFRunLoopCommonModes, 0, &s_AudioQueue.queue)) {
		printf("Couldn't set the AudioQueue callback!\n");
		_mixer->setReady(false);
		return;
	}

	uint32 bufferBytes = s_AudioQueue.frameCount * s_AudioQueue.dataFormat.mBytesPerFrame;

	for (int i = 0; i < AUDIO_BUFFERS; i++) {
		if (AudioQueueAllocateBuffer(s_AudioQueue.queue, bufferBytes, &s_AudioQueue.buffers[i])) {
			printf("Error allocating AudioQueue buffer!\n");
			_mixer->setReady(false);
			return;
		}

		AQBufferCallback(&s_AudioQueue, s_AudioQueue.queue, s_AudioQueue.buffers[i]);
	}

	AudioQueueSetParameter(s_AudioQueue.queue, kAudioQueueParam_Volume, 1.0);
	if (AudioQueueStart(s_AudioQueue.queue, NULL)) {
		printf("Error starting the AudioQueue!\n");
		_mixer->setReady(false);
		return;
	}

	_mixer->setReady(true);
}

void OSystem_IPHONE::stopSoundsystem() {
	AudioQueueStop(s_AudioQueue.queue, true);

	for (int i = 0; i < AUDIO_BUFFERS; i++) {
		AudioQueueFreeBuffer(s_AudioQueue.queue, s_AudioQueue.buffers[i]);
	}

	AudioQueueDispose(s_AudioQueue.queue, true);
	_mixer->setReady(false);
}
