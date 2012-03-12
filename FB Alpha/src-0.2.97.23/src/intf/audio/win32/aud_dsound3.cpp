// DirectSound module
#include "burner.h"
#include "aud_dsp.h"
#include <math.h>

#include <InitGuid.h>
#define DIRECTSOUND_VERSION  0x0300			// Only need version from DirectX 3
#include <dsound.h>

#include "dsound_core.h"

// Sound is split into a series of 'segs', one seg for each frame
// The Loop buffer is a multiple of this seg length.

static IDirectSound* pDS = NULL;			// DirectSound interface
static IDirectSoundBuffer* pdsbPrim = NULL;	// Primary buffer
static IDirectSoundBuffer* pdsbLoop = NULL;	// (Secondary) Loop buffer
static int cbLoopLen = 0;					// Loop length (in bytes) calculated

int (*DSoundGetNextSound)(int);				// Callback used to request more sound

static int nDSoundFps;						// Application fps * 100
static long nDSoundVol = 0;

static int DSoundGetNextSoundFiller(int)							// int bDraw
{
	if (nAudNextSound == NULL) {
		return 1;
	}
	memset(nAudNextSound, 0, nAudSegLen << 2);						// Write silence into the buffer

	return 0;
}

static inline int DxSetCallback(int (*pCallback)(int))
{
	if (pCallback == NULL) {
		DSoundGetNextSound = DSoundGetNextSoundFiller;
	} else {
		DSoundGetNextSound = pCallback;
	}

	return 0;
}

static int DxBlankSound()
{
	void *pData = NULL, *pData2 = NULL;
	DWORD cbLen = 0, cbLen2 = 0;

	// Lock the Loop buffer
	if (FAILED(pdsbLoop->Lock(0, cbLoopLen, &pData, &cbLen, &pData2, &cbLen2, 0))) {
		return 1;
	}
	memset(pData, 0, cbLen);

	// Unlock (2nd 0 is because we wrote nothing to second part)
	pdsbLoop->Unlock(pData, cbLen, pData2, 0);

	// Also blank the nAudNextSound buffer
	if (nAudNextSound) {
		memset(nAudNextSound, 0, nAudSegLen << 2);
	}

	return 0;
}

static int nDSoundNextSeg = 0;										// We have filled the sound in the loop up to the beginning of 'nNextSeg'

#define WRAP_INC(x) { x++; if (x >= nAudSegCount) x = 0; }

bool bRunFrame = false;

// This function checks the DSound loop, and if necessary does a callback to update the emulation
static int DxSoundCheck()
{
	int nPlaySeg = 0, nFollowingSeg = 0;
	DWORD nPlay = 0, nWrite = 0;

	if (pdsbLoop == NULL) {
		return 1;
	}

	// We should do nothing until nPlay has left nDSoundNextSeg
	pdsbLoop->GetCurrentPosition(&nPlay, &nWrite);

	nPlaySeg = nPlay / (nAudSegLen << 2);

	if (nPlaySeg > nAudSegCount -1 ) {
		nPlaySeg = nAudSegCount - 1;
	}
	if (nPlaySeg < 0) {												// important to ensure nPlaySeg clipped for below
		nPlaySeg = 0;
	}

	if (nDSoundNextSeg == nPlaySeg) {
		Sleep(2);													// Don't need to do anything for a bit
		//bRunFrame = true;
		return 0;
	}

	// work out which seg we will fill next
	nFollowingSeg = nDSoundNextSeg;
	WRAP_INC(nFollowingSeg);

	while (nDSoundNextSeg != nPlaySeg) {
		void *pData = NULL, *pData2 = NULL;
		DWORD cbLen = 0, cbLen2 = 0;
		int bDraw;

		// fill nNextSeg

		// Lock the relevant seg of the loop buffer
		if (SUCCEEDED(pdsbLoop->Lock(nDSoundNextSeg * (nAudSegLen << 2), nAudSegLen << 2, &pData, &cbLen, &pData2, &cbLen2, 0))) {
			// Locked the segment, so write the sound we calculated last time
			memcpy(pData, nAudNextSound, nAudSegLen << 2);

			// Unlock (2nd 0 is because we wrote nothing to second part)
			pdsbLoop->Unlock(pData, cbLen, pData2, 0);
		}

		bDraw = (nFollowingSeg == nPlaySeg)	|| bAlwaysDrawFrames;	// If this is the last seg of sound, flag bDraw (to draw the graphics)

		if(bDraw) {
			DSoundGetNextSound(bDraw);									// get more sound into nAudNextSound
		}

		if(!bDraw) {
			bRunFrame = true;
			RunIdle();
		}

		if (nAudDSPModule[0])	{
			DspDo(nAudNextSound, nAudSegLen);
		}

		nDSoundNextSeg = nFollowingSeg;
		WRAP_INC(nFollowingSeg);
	}

	return 0;
}

static int DxSoundExit()
{
	DspExit();

	if (nAudNextSound) {
		free(nAudNextSound);
		nAudNextSound = NULL;
	}

	DSoundGetNextSound = NULL;

	// Release the (Secondary) Loop Sound Buffer
	RELEASE(pdsbLoop);
	// Release the Primary Sound Buffer
	RELEASE(pdsbPrim);
	// Release the DirectSound interface
	RELEASE(pDS);

	return 0;
}

static int DxSoundInit()
{
	int nRet = 0;
	DSBUFFERDESC dsbd;
	WAVEFORMATEX wfx;

	if (nAudSampleRate <= 0) {
		return 1;
	}

	nDSoundFps = nAppVirtualFps;

	// Calculate the Seg Length and Loop length (round to nearest sample)
	nAudSegLen = (nAudSampleRate[0] * 100 + (nDSoundFps >> 1)) / nDSoundFps;
	cbLoopLen = (nAudSegLen * nAudSegCount) << 2;

	// Make the format of the sound
	memset(&wfx, 0, sizeof(wfx));
	wfx.cbSize = sizeof(wfx);
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 2;										  // stereo
	wfx.nSamplesPerSec = nAudSampleRate[0];					  // sample rate
	wfx.wBitsPerSample = 16;								  // 16-bit
	wfx.nBlockAlign = 4;									  // bytes per sample
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

	// Create the DirectSound interface
	if (FAILED(_DirectSoundCreate(NULL, &pDS, NULL))) {
		return 1;
	}

	// Set the coop level
	nRet = pDS->SetCooperativeLevel(hScrnWnd, DSSCL_PRIORITY);

	// Make the primary sound buffer
	memset(&dsbd, 0, sizeof(dsbd));
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
	if (FAILED(pDS->CreateSoundBuffer(&dsbd, &pdsbPrim, NULL))) {
		DxSoundExit();
		return 1;
	}

	{
		// Set the format of the primary sound buffer (not critical if it fails)
		if (nAudSampleRate[0] < 44100) {
			wfx.nSamplesPerSec = 44100;
		}
		pdsbPrim->SetFormat(&wfx);

		wfx.nSamplesPerSec = nAudSampleRate[0];
	}

	// Make the loop sound buffer
	memset(&dsbd, 0, sizeof(dsbd));
	dsbd.dwSize = sizeof(dsbd);
	// A standard secondary buffer (accurate position, plays in the background, and can notify).
	dsbd.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_CTRLVOLUME;
	dsbd.dwBufferBytes = cbLoopLen;
	dsbd.lpwfxFormat = &wfx;								// Same format as the primary buffer
	if (FAILED(pDS->CreateSoundBuffer(&dsbd, &pdsbLoop, NULL))) {
		AudSoundExit();
		return 1;
	}

	nAudNextSound = (short*)malloc(nAudSegLen << 2);		// The next sound block to put in the stream
	if (nAudNextSound == NULL) {
		DxSoundExit();
		return 1;
	}

	DxSetCallback(NULL);

	DspInit();

	return 0;
}

static int DxSoundPlay()
{
	DxBlankSound();
	pdsbLoop->SetVolume(nDSoundVol);

	// Play the looping buffer
	if (FAILED(pdsbLoop->Play(0, 0, DSBPLAY_LOOPING))) {
		return 1;
	}
	bAudPlaying = 1;

	return 0;
}

static int DxSoundStop()
{
	bAudPlaying = 0;

	if (bAudOkay == 0) {
		return 1;
	}

	// Stop the looping buffer
	pdsbLoop->Stop();

	return 0;
}

static int DxSoundSetVolume()
{
	if (nAudVolume == 10000) {
		nDSoundVol = DSBVOLUME_MAX;
	} else {
		if (nAudVolume == 0) {
			nDSoundVol = DSBVOLUME_MIN;
		} else {
			nDSoundVol = DSBVOLUME_MAX - (long)(10000.0 * pow(10.0, nAudVolume / -5000.0)) + 100;
		}
	}

	if (nDSoundVol < DSBVOLUME_MIN) {
		nDSoundVol = DSBVOLUME_MIN;
	}

	if (FAILED(pdsbLoop->SetVolume(nDSoundVol))) {
		return 0;
	}

	return 1;
}

static int DxGetSettings(InterfaceInfo* pInfo)
{
	TCHAR szString[MAX_PATH] = _T("");

	_sntprintf(szString, MAX_PATH, _T("Audio is delayed by approx. %ims"), int(100000.0 / (nDSoundFps / (nAudSegCount - 1.0))));
	IntInfoAddStringModule(pInfo, szString);

	return 0;
}

struct AudOut AudOutDx = { DxBlankSound, DxSoundCheck, DxSoundInit, DxSetCallback, DxSoundPlay, DxSoundStop, DxSoundExit, DxSoundSetVolume, DxGetSettings, _T("DirectSound3 audio output") };
