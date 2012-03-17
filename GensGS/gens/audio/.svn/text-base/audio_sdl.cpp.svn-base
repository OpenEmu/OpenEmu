/**
 * Gens: Audio class - SDL
 */


#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>

#include "audio_sdl.hpp"
#include "emulator/g_main.hpp"

#include "gens_core/sound/psg.h"
#include "gens_core/sound/ym2612.h"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/sound/pcm.h"
#include "gens_core/misc/cpuflags.h"

#ifdef GENS_X86_ASM
#include "audio_mmx.h" // MMX audio functions
#endif


Audio_SDL::Audio_SDL()
{
	audio_len = 0;
	pMsndOut = NULL;
	audiobuf = NULL;
}

Audio_SDL::~Audio_SDL()
{
}


/**
 * AudioCallback(): SDL audio callback. (Static function)
 * @param user Pointer to Audio_SDL class.
 * @param buffer
 * @param len
 */
void Audio_SDL::AudioCallback(void *user, Uint8 *buffer, int len)
{
	((Audio_SDL*)user)->audioCallback_int(buffer, len);
}


/**
 * audioCallback_int(): SDL audio callback. (Member function)
 * @param buffer
 * @param len
 */
void Audio_SDL::audioCallback_int(Uint8 *buffer, int len)
{
	if (audio_len < (int)len)
	{
		memcpy(buffer, audiobuf, audio_len);
		audio_len = 0;
		return;
	}
	memcpy(buffer, audiobuf, len);
	audio_len -= len;
	memcpy(audiobuf, audiobuf + len, audio_len);
}


/**
 * initSound(): Initialize sound.
 * @return 1 on success; -1 if sound was already initialized; 0 on error.
 */
int Audio_SDL::initSound(void)
{
	int i;
	int videoLines;
	
	if (m_soundInitialized)
		return -1;
	
	// Make sure sound is ended first.
	endSound();
	
	switch (m_soundRate)
	{
		case 11025:
			m_segLength = (CPU_Mode ? 220 : 184);
			break;
		case 22050:
			m_segLength = (CPU_Mode ? 441 : 368);
			break;
		case 44100:
			m_segLength = (CPU_Mode ? 882 : 735);
			break;
	}
	
	videoLines = (CPU_Mode ? 312 : 262);
	for (i = 0; i < videoLines; i++)
	{
		Sound_Extrapol[i][0] = ((m_segLength * i) / videoLines);
		Sound_Extrapol[i][1] = (((m_segLength * (i + 1)) / videoLines) - Sound_Extrapol[i][0]);
	}
	for (i = 0; i < m_segLength; i++)
		Sound_Interpol[i] = ((videoLines * i) / m_segLength);
	
	// Clear the segment buffers.
	memset(Seg_L, 0, m_segLength << 2);
	memset(Seg_R, 0, m_segLength << 2);
	
	// Attempt to initialize SDL audio.
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1)
		return 0;
	
	// Allocate the segment buffer.
	pMsndOut = static_cast<unsigned char*>(malloc(m_segLength << 2));
	
	// Set up the SDL audio specification.
	SDL_AudioSpec spec;
	spec.freq = m_soundRate;
	spec.format = AUDIO_S16SYS;
	spec.channels = !m_stereo ? 1 : 2; // TODO: Initializing 1 channel seems to double-free if it's later changed...
	spec.samples = 1024;
	spec.callback = AudioCallback;
	audiobuf = static_cast<unsigned char*>(malloc((spec.samples * spec.channels * 2 * 4) * sizeof(short)));
	
	// "user" parameter for the callback function is a pointer to this object.
	spec.userdata = this;
	
	memset(audiobuf, 0, (spec.samples * spec.channels * 2 * 4) * sizeof(short));
	if (SDL_OpenAudio(&spec, 0) != 0)
		return 0;
	SDL_PauseAudio(0);
	
	// Sound is initialized.
	m_soundInitialized = true;
	return 1;
}


/**
 * endSound(): Stop sound output.
 */
void Audio_SDL::endSound(void)
{
	SDL_PauseAudio(1);
	free(audiobuf);
	audiobuf = NULL;
	
	free(pMsndOut);
	pMsndOut = NULL;
	
	if (m_soundInitialized)
	{
		m_soundIsPlaying = false;
		m_soundInitialized = false;
	}
	
	// Shut down SDL audio.
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}


int Audio_SDL::getCurrentSeg(void)
{
	// We aren't using DIRECTX so these won't work :(
	
	/*
	unsigned int R;
	
	lpDSBuffer->GetCurrentPosition(&R, NULL);
	return(R / (Seg_Length * Bytes_Per_Unit));
	*/
	
	return 0;
}


int Audio_SDL::checkSoundTiming(void)
{
	// Not used for SDL...
	return 0;
}


/**
 * writeSoundBuffer(): Write the sound buffer to the audio output.
 * @param dumpBuf Sound dumping buffer.
 * @return 1 on success.
 */
int Audio_SDL::writeSoundBuffer(void *dumpBuf)
{
	struct timespec rqtp = {0, 1000000};
	
	SDL_LockAudio();
	
	// TODO: Fix dumpBuf support.
#if 0
	if (dumpBuf)
	{
		if (m_stereo)
			dumpSoundStereo(dumpBuf, m_segLength);
		else
			dumpSoundMono(dumpBuf, m_segLength);
	}
#endif /* 0 */
	
	if (m_stereo)
	{
#ifdef GENS_X86_ASM
		if (CPU_Flags & CPUFLAG_MMX)
			writeSoundStereo_MMX(Seg_L, Seg_R, reinterpret_cast<short*>(pMsndOut), m_segLength);
		else
#endif
			writeSoundStereo(reinterpret_cast<short*>(pMsndOut), m_segLength);
	}
	else
	{
#ifdef GENS_X86_ASM
		if (CPU_Flags & CPUFLAG_MMX)
			writeSoundMono_MMX(Seg_L, Seg_R, reinterpret_cast<short*>(pMsndOut), m_segLength);
		else
#endif
			writeSoundMono(reinterpret_cast<short*>(pMsndOut), m_segLength);
	}
	
	memcpy(audiobuf + audio_len, pMsndOut, m_segLength * 4);
	audio_len += m_segLength * 4;
	
	SDL_UnlockAudio();
	
	// TODO: Figure out if there's a way to get rid of this.
	while (audio_len > 1024 * 2 * 2 * 4)
	{
		nanosleep(&rqtp, NULL);	
		if (fast_forward)
			audio_len = 1024;
	} //SDL_Delay(1); 
	
	return 1;
}


int Audio_SDL::clearSoundBuffer(void)
{
	// NOTE: This is only used for DirectSound...
#if 0
	LPVOID lpvPtr1;
	DWORD dwBytes1;
	HRESULT rval;
	int i;

	if (!Sound_Initialised) return 0;

	rval = lpDSBuffer->Lock (0, Seg_Length * Sound_Segs * Bytes_Per_Unit, &lpvPtr1, &dwBytes1, NULL, NULL, 0);

	if (rval == DSERR_BUFFERLOST)
	{
	lpDSBuffer->Restore ();
	rval = lpDSBuffer->Lock (0, Seg_Length * Sound_Segs * Bytes_Per_Unit, &lpvPtr1, &dwBytes1, NULL, NULL, 0);

}

	if (rval == DS_OK)
	{
	signed short *w = (signed short *) lpvPtr1;

	for (i = 0; i < Seg_Length * Sound_Segs * Bytes_Per_Unit; i += 2)
	*w++ = (signed short) 0;

	rval = lpDSBuffer->Unlock (lpvPtr1, dwBytes1, NULL, NULL);

	if (rval == DS_OK) return 1;
}
#endif
	return 0;
}


int Audio_SDL::playSound(void)
{
	// NOTE: DirectSound only - what is this function used for?
#if 0
	HRESULT rval;

	if (Sound_Is_Playing)  return 1;

	rval = lpDSBuffer->Play (0, 0, DSBPLAY_LOOPING);

	Clear_Sound_Buffer ();

	if (rval != DS_OK) return 0;
#endif
	m_soundIsPlaying = true;
	return 1;
}


int Audio_SDL::stopSound(void)
{
	// NOTE: DirectSound only - what is this function used for?
#if 0
	HRESULT rval;

	rval = lpDSBuffer->Stop ();

	if (rval != DS_OK) return 0;
#endif
	m_soundIsPlaying = false;
	return 1;
}


/**
 * waitForAudioBuffer(): Wait for the audio buffer to empty out.
 * This function is used for Auto Frame Skip.
 */
void Audio_SDL::waitForAudioBuffer(void)
{
	writeSoundBuffer(NULL);
	while (audio_len <= (m_segLength * Seg_To_Buffer))
	{
		Update_Frame_Fast();
		writeSoundBuffer(NULL);
	}
}
