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
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include <malloc.h>

#include "osystem.h"

#define SFX_THREAD_STACKSIZE (1024 * 128)
#define SFX_THREAD_PRIO 80
#define SFX_THREAD_FRAG_SIZE (1024 * 8)
#define SFX_BUFFERS 3

static lwpq_t sfx_queue;
static lwp_t sfx_thread;
static u8 *sfx_stack;
static bool sfx_thread_running = false;
static bool sfx_thread_quit = false;

static u32 sb_hw;
static u8 *sound_buffer[SFX_BUFFERS];

static void audio_switch_buffers() {
	sb_hw = (sb_hw + 1) % SFX_BUFFERS;
	AUDIO_InitDMA((u32) sound_buffer[sb_hw], SFX_THREAD_FRAG_SIZE);
	LWP_ThreadSignal(sfx_queue);
}

static void * sfx_thread_func(void *arg) {
	Audio::MixerImpl *mixer = (Audio::MixerImpl *) arg;
	u8 sb_sw;

	while (true) {
		LWP_ThreadSleep(sfx_queue);

		if (sfx_thread_quit)
			break;

		// the hardware uses two buffers: a front and a back buffer
		// we use 3 buffers here: two are beeing pushed to the DSP,
		// and the free one is where our mixer writes to
		// thus the latency of our stream is:
		// 8192 [frag size] / 48000 / 2 [16bit] / 2 [stereo] * 2 [hw buffers]
		// -> 85.3ms
		sb_sw = (sb_hw + 1) % SFX_BUFFERS;
		mixer->mixCallback(sound_buffer[sb_sw], SFX_THREAD_FRAG_SIZE);
		DCFlushRange(sound_buffer[sb_sw], SFX_THREAD_FRAG_SIZE);
	}

	return NULL;
}

void OSystem_Wii::initSfx() {
	_mixer = new Audio::MixerImpl(this, 48000);

	sfx_thread_running = false;
	sfx_thread_quit = false;

	sfx_stack = (u8 *) memalign(32, SFX_THREAD_STACKSIZE);

	if (sfx_stack) {
		memset(sfx_stack, 0, SFX_THREAD_STACKSIZE);

		LWP_InitQueue(&sfx_queue);

		s32 res = LWP_CreateThread(&sfx_thread, sfx_thread_func, _mixer, sfx_stack,
									SFX_THREAD_STACKSIZE, SFX_THREAD_PRIO);

		if (res) {
			printf("ERROR creating sfx thread: %d\n", res);
			LWP_CloseQueue(sfx_queue);
			return;
		}

		sfx_thread_running = true;
	}

	for (u32 i = 0; i < SFX_BUFFERS; ++i) {
		sound_buffer[i] = (u8 *) memalign(32, SFX_THREAD_FRAG_SIZE);
		memset(sound_buffer[i], 0, SFX_THREAD_FRAG_SIZE);
		DCFlushRange(sound_buffer[i], SFX_THREAD_FRAG_SIZE);
	}

	_mixer->setReady(true);

	sb_hw = 0;

	AUDIO_SetDSPSampleRate(AI_SAMPLERATE_48KHZ);
	AUDIO_RegisterDMACallback(audio_switch_buffers);
	AUDIO_InitDMA((u32) sound_buffer[sb_hw], SFX_THREAD_FRAG_SIZE);
	AUDIO_StartDMA();
}

void OSystem_Wii::deinitSfx() {
	if (_mixer)
		_mixer->setReady(false);

	AUDIO_StopDMA();
	AUDIO_RegisterDMACallback(NULL);

	if (sfx_thread_running) {
		sfx_thread_quit = true;
		LWP_ThreadBroadcast(sfx_queue);

		LWP_JoinThread(sfx_thread, NULL);
		LWP_CloseQueue(sfx_queue);

		free(sfx_stack);
		sfx_thread_running = false;

		for (u32 i = 0; i < SFX_BUFFERS; ++i)
			free(sound_buffer[i]);
	}
}
