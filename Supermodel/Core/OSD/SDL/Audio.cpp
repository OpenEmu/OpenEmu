/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/
 
/*
 * Audio.cpp
 * 
 * SDL audio playback. Implements the OSD audio interface.
 */

#include "Supermodel.h"

#ifdef SUPERMODEL_OSX
#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>
#else
#include <SDL.h>
#include <SDL_audio.h>
#endif

#include <cmath>

// Model3 audio output is 44.1KHz 2-channel sound and frame rate is 60fps
#define SAMPLE_RATE 44100
#define NUM_CHANNELS 2
#define SUPERMODEL_FPS 60

#define BYTES_PER_SAMPLE (NUM_CHANNELS * sizeof(INT16))
#define SAMPLES_PER_FRAME (SAMPLE_RATE / SUPERMODEL_FPS)
#define BYTES_PER_FRAME (SAMPLES_PER_FRAME * BYTES_PER_SAMPLE) 

#define MAX_LATENCY 100

static bool enabled = true;         // True if sound output is enabled
static unsigned latency = 20;       // Audio latency to use (ie size of audio buffer) as percentage of max buffer size
static bool underRunLoop = true;    // True if should loop back to beginning of buffer on under-run, otherwise sound is just skipped

static unsigned playSamples = 512;  // Size (in samples) of callback play buffer

static UINT32 audioBufferSize = 0;  // Size (in bytes) of audio buffer
static INT8	*audioBuffer = NULL;    // Audio buffer

static UINT32 writePos = 0;         // Current position at which writing into buffer
static UINT32 playPos = 0;          // Current position at which playing data in buffer via callback

static bool writeWrapped = false;   // True if write position has wrapped around at end of buffer but play position has not done so yet

static unsigned underRuns = 0;      // Number of buffer under-runs that have occured
static unsigned overRuns = 0;       // Number of buffer over-runs that have occured

static AudioCallbackFPtr callback = NULL; // Pointer to audio callback that is called when audio buffer is less than half empty
static void *callbackData = NULL;         // Pointer to data to be passed to audio callback when it is called

void SetAudioCallback(AudioCallbackFPtr newCallback, void *newData)
{
	// Lock audio whilst changing callback pointers
	SDL_LockAudio();

	callback = newCallback;
	callbackData = newData;

	SDL_UnlockAudio();
}

void SetAudioEnabled(bool newEnabled)
{
	enabled = newEnabled;
}

static void PlayCallback(void *data, Uint8 *stream, int len)
{
	//printf("PlayCallback(%d) [writePos = %u, writeWrapped = %s, playPos = %u, audioBufferSize = %u]\n", 
	//	len, writePos, (writeWrapped ? "true" : "false"), playPos, audioBufferSize);
	
	// Get current write position and adjust it if write has wrapped but play position has not
	UINT32 adjWritePos = writePos;
	if (writeWrapped)
		adjWritePos += audioBufferSize;

	// Check if play position overlaps write position (ie buffer under-run)
	if (playPos + len > adjWritePos)
	{
		underRuns++;

		//printf("Audio buffer under-run #%u in PlayCallback(%d) [writePos = %u, writeWrapped = %s, playPos = %u, audioBufferSize = %u]\n",
		//	underRuns, len, writePos, (writeWrapped ? "true" : "false"), playPos, audioBufferSize);
		
		// See what action to take on under-run
		if (underRunLoop)
		{
			// If loop, then move play position back to beginning of data in buffer
			playPos = adjWritePos + BYTES_PER_FRAME;

			// Check if play position has moved past end of buffer
			if (playPos >= audioBufferSize)
				// If so, wrap it around to beginning again (but keep write wrapped flag as before)
				playPos -= audioBufferSize;
			else 
				// Otherwise, set write wrapped flag as will now appear as if write has wrapped but play position has not
				writeWrapped = true;
		}
		else
		{
			// Otherwise, just copy silence to audio output stream and exit
			memset(stream, 0, len);
			return;
		}
	}
	
	INT8* src1;
	INT8* src2;
	UINT32 len1;
	UINT32 len2;

	// Check if play region extends past end of buffer
	if (playPos + len > audioBufferSize)
	{
		// If so, split play region into two
		src1 = audioBuffer + playPos;
		src2 = audioBuffer;
		len1 = audioBufferSize - playPos;
		len2 = len - len1;
	}
	else
	{
		// Otherwise, just copy whole region
		src1 = audioBuffer + playPos;
		src2 = 0;
		len1 = len;
		len2 = 0;
	}

	// Check if audio is enabled
	if (enabled)
	{
		// If so, copy play region into audio output stream
		memcpy(stream, src1, len1);
		
		// Also, if not looping on under-runs then blank region out
		if (!underRunLoop)
			memset(src1, 0, len1);

		if (len2)
		{
			// If region was split into two, copy second half into audio output stream as well
			memcpy(stream + len1, src2, len2);

			// Also, if not looping on under-runs then blank region out
			if (!underRunLoop)
				memset(src2, 0, len2);
		}
	}
	else
		// Otherwise, just copy silence to audio output stream
		memset(stream, 0, len);

	// Move play position forward for next time
	playPos += len;

	bool bufferFull = adjWritePos + 2 * BYTES_PER_FRAME > playPos + audioBufferSize;

	// Check if play position has moved past end of buffer
	if (playPos >= audioBufferSize)
	{
		// If so, wrap it around to beginning again and reset write wrapped flag
		playPos -= audioBufferSize;
		writeWrapped = false;
	}

	// If buffer is not full then call audio callback
	if (callback && !bufferFull)
		callback(callbackData);
}

static void MixChannels(unsigned numSamples, INT16 *leftBuffer, INT16 *rightBuffer, void *dest)
{
	INT16 *p = (INT16*)dest;
	
#if (NUM_CHANNELS == 1)
	for (unsigned i = 0; i < numSamples; i++)
		*p++ = leftBuffer[i] + rightBuffer[i];	// TODO: these should probably be clipped! 
#else
	if (g_Config.flipStereo)	// swap left and right channels
	{
		for (unsigned i = 0; i < numSamples; i++)
		{
			*p++ = rightBuffer[i];
			*p++ = leftBuffer[i];
		}
	}
	else						// stereo as God intended!
	{
		for (unsigned i = 0; i < numSamples; i++)
		{
			*p++ = leftBuffer[i];
			*p++ = rightBuffer[i];
		}
	}
#endif	// NUM_CHANNELS
}

static void LogAudioInfo(SDL_AudioSpec *fmt)
{
	InfoLog("Audio device information:");
	InfoLog("    Frequency: %d", fmt->freq);
	InfoLog("     Channels: %d", fmt->channels);
	InfoLog("Sample Format: %d", fmt->format);
	InfoLog("");
}

bool OpenAudio()
{
	// Initialize SDL audio sub-system
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
		return ErrorLog("Unable to initialize SDL audio sub-system: %s\n", SDL_GetError());

	// Set up audio specification
	SDL_AudioSpec fmt;
	memset(&fmt, 0, sizeof(SDL_AudioSpec));
	fmt.freq = SAMPLE_RATE;
	fmt.channels = NUM_CHANNELS;
	fmt.format = AUDIO_S16SYS;
	fmt.samples = playSamples;
	fmt.callback = PlayCallback;
	
	// Try opening SDL audio output with that specification
	SDL_AudioSpec obtained;
	if (SDL_OpenAudio(&fmt, &obtained) < 0)
		return ErrorLog("Unable to open 44.1KHz 2-channel audio with SDL: %s\n", SDL_GetError());
	LogAudioInfo(&obtained);
		
	// Check if obtained format is what we really requested
	if ((obtained.freq!=fmt.freq) || (obtained.channels!=fmt.channels) || (obtained.format!=fmt.format))
		ErrorLog("Incompatible audio settings (44.1KHz, 16-bit required). Check drivers!\n");
		
	// Check what buffer sample size was actually obtained, and use that
	playSamples = obtained.samples;

	// Create audio buffer
	audioBufferSize = SAMPLE_RATE * BYTES_PER_SAMPLE * latency / MAX_LATENCY;
	int minBufferSize = 3 * BYTES_PER_FRAME;
	audioBufferSize = max<int>(minBufferSize, audioBufferSize);
	audioBuffer = new(std::nothrow) INT8[audioBufferSize];
	if (audioBuffer == NULL)
	{
		float audioBufMB = (float)audioBufferSize / (float)0x100000;
		return ErrorLog("Insufficient memory for audio latency buffer (need %1.1f MB).", audioBufMB);	
	}
	memset(audioBuffer, 0, sizeof(INT8) * audioBufferSize);
	
	// Set initial play position to be beginning of buffer and initial write position to be half-way into buffer
	playPos = 0;
	writePos = min<int>(audioBufferSize - BYTES_PER_FRAME, (BYTES_PER_FRAME + audioBufferSize) / 2);
	writeWrapped = false;

	// Reset counters
	underRuns = 0;
	overRuns = 0;

	// Start audio playing
	SDL_PauseAudio(0);
	return OKAY;
}

bool OutputAudio(unsigned numSamples, INT16 *leftBuffer, INT16 *rightBuffer)
{
	//printf("OutputAudio(%u) [writePos = %u, writeWrapped = %s, playPos = %u, audioBufferSize = %u]\n",
	//	numSamples, writePos, (writeWrapped ? "true" : "false"), playPos, audioBufferSize);

	UINT32 bytesRemaining;
	UINT32 bytesToCopy;
	INT16 *src;

	// Number of samples should never be more than max number of samples per frame
	if (numSamples > SAMPLES_PER_FRAME)
		numSamples = SAMPLES_PER_FRAME;

	// Mix together left and right channels into single chunk of data
	INT16 mixBuffer[NUM_CHANNELS * SAMPLES_PER_FRAME];
	MixChannels(numSamples, leftBuffer, rightBuffer, mixBuffer);
	
	// Lock SDL audio callback so that it doesn't interfere with following code
	SDL_LockAudio();
	
	// Calculate number of bytes for current sound chunk
	UINT32 numBytes = numSamples * BYTES_PER_SAMPLE;
	
	// Get end of current play region (writing must occur past this point)
	UINT32 playEndPos = playPos + BYTES_PER_FRAME;
	
	// Undo any wrap-around of the write position that may have occured to create following ordering: playPos < playEndPos < writePos
	if (playEndPos > writePos && writeWrapped)
		writePos += audioBufferSize;

	// Check if play region has caught up with write position and now overlaps it (ie buffer under-run)
	if (playEndPos > writePos)
	{
		underRuns++;

		//printf("Audio buffer under-run #%u in OutputAudio(%u) [writePos = %u, writeWrapped = %s, playPos = %u, audioBufferSize = %u, numBytes = %u]\n",
		//	underRuns, numSamples, writePos, (writeWrapped ? "true" : "false"), playPos, audioBufferSize, numBytes);
		
		// See what action to take on under-run
		if (underRunLoop)
		{
			// If loop, then move play position back to beginning of data in buffer
			playPos = writePos + numBytes + BYTES_PER_FRAME;
			
			// Check if play position has moved past end of buffer
			if (playPos >= audioBufferSize)
				// If so, wrap it around to beginning again (but keep write wrapped flag as before)
				playPos -= audioBufferSize;
			else 
			{
				// Otherwise, set write wrapped flag as will now appear as if write has wrapped but play position has not
				writeWrapped = true;
				writePos += audioBufferSize;
			}
		}
		else
		{
			// Otherwise, bump write position forward in chunks until it is past end of play region
			do
			{
				writePos += numBytes;
			}
			while (playEndPos > writePos);
		}
	}

	// Check if write position has caught up with play region and now overlaps it (ie buffer over-run)
	bool overRun = writePos + numBytes > playPos + audioBufferSize;
	
	bool bufferFull = writePos + 2 * BYTES_PER_FRAME > playPos + audioBufferSize;

	// Move write position back to within buffer
	if (writePos >= audioBufferSize)
		writePos -= audioBufferSize;

	// Handle buffer over-run
	if (overRun)
	{
		overRuns++;

		//printf("Audio buffer over-run #%u in OutputAudio(%u) [writePos = %u, writeWrapped = %s, playPos = %u, audioBufferSize = %u, numBytes = %u]\n",
		//	overRuns, numSamples, writePos, (writeWrapped ? "true" : "false"), playPos, audioBufferSize, numBytes);
		
		bufferFull = true;

		// Discard current chunk of data
		goto Finish;
	}

	src = mixBuffer;
	INT8 *dst1;
	INT8 *dst2;
	UINT32 len1;
	UINT32 len2;

	// Check if write region extends past end of buffer
	if (writePos + numBytes > audioBufferSize)
	{
		// If so, split write region into two
		dst1 = audioBuffer + writePos;
		dst2 = audioBuffer;
		len1 = audioBufferSize - writePos;
		len2 = numBytes - len1;
	}
	else
	{
		// Otherwise, just copy whole region
		dst1 = audioBuffer + writePos;
		dst2 = 0;
		len1 = numBytes;
		len2 = 0;
	}

	// Copy chunk to write position in buffer
	bytesRemaining = numBytes;
	bytesToCopy = (bytesRemaining > len1 ? len1 : bytesRemaining);
	memcpy(dst1, src, bytesToCopy);

	// Adjust for number of bytes copied
	bytesRemaining -= bytesToCopy;
	src = (INT16*)((UINT8*)src + bytesToCopy);

	if (bytesRemaining)
	{
		// If write region was split into two, copy second half of chunk into buffer as well
		bytesToCopy = (bytesRemaining > len2 ? len2 : bytesRemaining);
		memcpy(dst2, src, bytesToCopy);
	}

	// Move write position forward for next time
	writePos += numBytes;

	// Check if write position has moved past end of buffer
	if (writePos >= audioBufferSize)
	{
		// If so, wrap it around to beginning again and set write wrapped flag
		writePos -= audioBufferSize;
		writeWrapped = true;
	}

Finish:
	// Unlock SDL audio callback
	SDL_UnlockAudio();

	// Return whether buffer is half full
	return bufferFull;
}

void CloseAudio()
{
	// Close SDL audio output
	SDL_CloseAudio();

	// Delete audio buffer
	if (audioBuffer != NULL)
	{
		delete[] audioBuffer;
		audioBuffer = NULL;
	}
}