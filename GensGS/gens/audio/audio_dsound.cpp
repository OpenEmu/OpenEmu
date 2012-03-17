/**
 * Gens: Audio class - DirectSound
 */


#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio_dsound.hpp"
#include "emulator/g_main.hpp"
#include "ui/gens_ui.hpp"
#include "gens/gens_window.hpp"

#include "gens_core/sound/psg.h"
#include "gens_core/sound/ym2612.h"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/sound/pcm.h"
#include "gens_core/misc/cpuflags.h"

#ifdef GENS_X86_ASM
#include "audio_mmx.h" // MMX audio functions
#endif


Audio_DSound::Audio_DSound()
{
	// Initialize DSound buffers to NULL.
	lpDS = NULL;
	lpDSPrimary = NULL;
	lpDSBuffer = NULL;
}


Audio_DSound::~Audio_DSound()
{
}


/**
 * initSound(): Initialize sound.
 * @return 1 on success; -1 if sound was already initialized; 0 on error.
 */
int Audio_DSound::initSound(void)
{
	HRESULT rval;
	WAVEFORMATEX wfx;
	int i, videoLines;
	
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
	
	// Attempt to initialize DirectSound.
	rval = DirectSoundCreate(NULL, &lpDS, NULL);
	if (rval != DS_OK)
		return 0;
	
	setCooperativeLevel();
	
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	
	rval = lpDS->CreateSoundBuffer(&dsbdesc, &lpDSPrimary, NULL);
	if (rval != DS_OK)
	{
		lpDS->Release();
		lpDS = NULL;
		return 0;
	}
	
	Bytes_Per_Unit = 2 * (m_stereo ? 2 : 1);
	
	memset(&wfx, 0, sizeof(WAVEFORMATEX));
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = (m_stereo ? 2 : 1);
	wfx.nSamplesPerSec = m_soundRate;
	wfx.wBitsPerSample = 16;
	wfx.nBlockAlign = Bytes_Per_Unit;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * Bytes_Per_Unit;
	
	rval = lpDSPrimary->SetFormat(&wfx);
	if (rval != DS_OK)
	{
		printf("ERR: 0x%X\n", rval);
		lpDSPrimary->Release();
		lpDSPrimary = NULL;
		lpDS->Release();
		lpDS = NULL;
		return 0;
	}
	
	memset(&MainWfx, 0, sizeof(WAVEFORMATEX));
	MainWfx.wFormatTag = WAVE_FORMAT_PCM;
	MainWfx.nChannels = (m_stereo ? 2 : 1);
	MainWfx.nSamplesPerSec = m_soundRate;
	MainWfx.wBitsPerSample = 16;
	MainWfx.nBlockAlign = Bytes_Per_Unit;
	MainWfx.nAvgBytesPerSec = MainWfx.nSamplesPerSec * Bytes_Per_Unit;
	
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS;
	dsbdesc.dwBufferBytes = SBuffer_Length = m_segLength * Sound_Segs * Bytes_Per_Unit;
	dsbdesc.lpwfxFormat = &MainWfx;
	
	//sprintf(STR, "Seg l : %d   Num Seg : %d   Taille : %d", Seg_Length, Sound_Segs, Bytes_Per_Unit);
	//MessageBox(HWnd, STR, "", MB_OK);
	
	rval = lpDS->CreateSoundBuffer(&dsbdesc, &lpDSBuffer, NULL);
	if (rval != DS_OK)
	{
		lpDS->Release();
		lpDS = NULL;
		return 0;
	}
	
	// Sound is initialized.
	m_soundInitialized = true;
	return 1;
}


/**
 * endSound(): Stop sound output.
 */
void Audio_DSound::endSound(void)
{
	if (!m_soundInitialized)
		return;
	
	m_soundInitialized = false;
	
	if (lpDSPrimary)
	{
		lpDSPrimary->Release();
		lpDSPrimary = NULL;
	}
	
	if (lpDSBuffer)
	{
		lpDSBuffer->Stop();
		m_soundIsPlaying = false;
		lpDSBuffer->Release();
		lpDSBuffer = NULL;
	}
	
	if (lpDS)
	{
		lpDS->Release();
		lpDS = NULL;
	}
}


int Audio_DSound::getCurrentSeg(void)
{
	unsigned int R;
	
	lpDSBuffer->GetCurrentPosition(&R, NULL);
	return (R / (m_segLength * Bytes_Per_Unit));
}


int Audio_DSound::checkSoundTiming(void)
{
	unsigned int R;
	
	lpDSBuffer->GetCurrentPosition(&R, NULL);
	
	RP = R / (m_segLength * Bytes_Per_Unit);
	if (RP == ((WP + 1) & (Sound_Segs - 1)))
		return 2;
	
	if (RP != WP)
		return 1;
	
	return 0;
}


/**
 * writeSoundBuffer(): Write the sound buffer to the audio output.
 * @param dumpBuf Sound dumping buffer.
 * @return 1 on success.
 */
int Audio_DSound::writeSoundBuffer(void *dumpBuf)
{
	LPVOID lpvPtr1;
	DWORD dwBytes1;
	HRESULT rval;
	
	if (dumpBuf)
	{
		if (m_stereo)
			dumpSoundStereo(reinterpret_cast<short*>(dumpBuf), m_segLength);
		else
			dumpSoundMono(reinterpret_cast<short*>(dumpBuf), m_segLength);
		return 1;
	}
	
	rval = lpDSBuffer->Lock(WP * m_segLength * Bytes_Per_Unit, m_segLength * Bytes_Per_Unit,
				&lpvPtr1, &dwBytes1, NULL, NULL, 0);
	
	if (rval == DSERR_BUFFERLOST)
	{
		lpDSBuffer->Restore();
		rval = lpDSBuffer->Lock(WP * m_segLength * Bytes_Per_Unit, m_segLength * Bytes_Per_Unit,
					&lpvPtr1, &dwBytes1, NULL, NULL, 0);
	}
	
	if (rval == DSERR_BUFFERLOST || !lpvPtr1)
		return 0;
	
	// TODO: Master Volume support
#if 0
	for(int i = 0; i < Seg_Length; i++) 
	{
		Seg_R[i] *= MastVol;
		Seg_R[i] >>= 8;
		Seg_L[i] *= MastVol;
		Seg_L[i] >>= 8;
	}
#endif
	
	if (m_stereo)
	{
#ifdef GENS_X86_ASM
		if (CPU_Flags & CPUFLAG_MMX)
			writeSoundStereo_MMX(Seg_L, Seg_R, reinterpret_cast<short*>(lpvPtr1), m_segLength);
		else
#endif
			writeSoundStereo(reinterpret_cast<short*>(lpvPtr1), m_segLength);
	}
	else
	{
#ifdef GENS_X86_ASM
		if (CPU_Flags & CPUFLAG_MMX)
			writeSoundMono_MMX(Seg_L, Seg_R, reinterpret_cast<short*>(lpvPtr1), m_segLength);
		else
#endif
			writeSoundMono(reinterpret_cast<short*>(lpvPtr1), m_segLength);
	}
	
	lpDSBuffer->Unlock(lpvPtr1, dwBytes1, NULL, NULL);
	
	return 1;
}


int Audio_DSound::clearSoundBuffer(void)
{
	LPVOID lpvPtr1;
	DWORD dwBytes1;
	HRESULT rval;
	int i;

	if (!m_soundInitialized)
		return 0;
	
	rval = lpDSBuffer->Lock(0, m_segLength * Sound_Segs * Bytes_Per_Unit, &lpvPtr1, &dwBytes1, NULL, NULL, 0);

	if (rval == DSERR_BUFFERLOST)
	{
		lpDSBuffer->Restore();
		rval = lpDSBuffer->Lock(0, m_segLength * Sound_Segs * Bytes_Per_Unit, &lpvPtr1, &dwBytes1, NULL, NULL, 0);
	}
	
	if (rval != DS_OK)
		return 0;
	
	signed short *w = reinterpret_cast<signed short*>(lpvPtr1);
	
	for (i = 0; i < (m_segLength * Sound_Segs * Bytes_Per_Unit); i += 2)
	{
		*w++ = 0;
	}
	
	rval = lpDSBuffer->Unlock(lpvPtr1, dwBytes1, NULL, NULL);
	
	if (rval == DS_OK)
		return 1;
	return 0;
}


int Audio_DSound::playSound(void)
{
	HRESULT rval;
	
	if (m_soundIsPlaying)
		return 1;
	
	rval = lpDSBuffer->Play(0, 0, DSBPLAY_LOOPING);
	
	clearSoundBuffer();
	
	if (rval != DS_OK)
		return 0;
	
	m_soundIsPlaying = true;
	return 1;
}


int Audio_DSound::stopSound(void)
{
	HRESULT rval;
	
	rval = lpDSBuffer->Stop();
	
	if (rval != DS_OK)
		return 0;
	
	m_soundIsPlaying = false;
	return 1;
}


bool Audio_DSound::lotsInAudioBuffer(void)
{
	// TODO: Figure out what to do here for DSound...
	return false;
}


void Audio_DSound::wpSegWait(void)
{
	while (WP == getCurrentSeg())
	{
		GensUI::sleep(1);
	}
}


void Audio_DSound::wpInc(void)
{
	WP = (WP + 1) & (Sound_Segs - 1);
	if (WP == getCurrentSeg())
		WP = (WP + Sound_Segs - 1) & (Sound_Segs - 1);
}


/**
 * waitForAudioBuffer(): Wait for the audio buffer to empty out.
 * This function is used for Auto Frame Skip.
 */
void Audio_DSound::waitForAudioBuffer(void)
{
	RP = getCurrentSeg();
	while (WP != RP)
	{
		writeSoundBuffer(NULL);
		WP = (WP + 1) & (Sound_Segs - 1);
		input->updateControllers();
	}
}


/**
 * setCooperativeLevel(): Sets the cooperative level.
 */
void Audio_DSound::setCooperativeLevel(void)
{
	if (!Gens_hWnd || !lpDS)
		return;
	
	HRESULT rval;
	rval = lpDS->SetCooperativeLevel(Gens_hWnd, DSSCL_PRIORITY);
	if (rval != DS_OK)
	{
		fprintf(stderr, "%s(): lpDS->SetCooperativeLevel() failed.\n", __func__);
		// TODO: Error handling code.
	}
	else
	{
		fprintf(stderr, "%s(): lpDS->SetCooperativeLevel() succeeded.\n", __func__);
	}
}
