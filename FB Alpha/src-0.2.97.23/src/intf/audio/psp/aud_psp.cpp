// psp sound module
#include "burner.h"
#include <math.h>
#include <pspaudio.h>

static unsigned int nSoundFps;	

static int nAudLoopLenSample;
static int nAudLoopLen;

static short* pAudioSndBuf[2] = {NULL, NULL};
static int nAudioThreadId = -1;
static int nAudioChannel = -1;
static volatile int bAudioTerminate = 1;
static volatile int nAudioBufIdx = 0;
static volatile int nCurrentSeg = 0;

extern "C" void pspSoundCopy11025(short *pDest, short *pSrc, int nLen);
extern "C" void pspSoundCopy22050(short *pDest, short *pSrc, int nLen);
extern "C" void pspSoundCopy44100(short *pDest, short *pSrc, int nLen);

static void (*pspSoundCopy) (short *pDest, short *pSrc, int nLen);

/** SOUND THREAD **/

static int AudioChannelThread(SceSize args, void *argp)
{
	while (!bAudioTerminate) {
		if (bAudPlaying && (nCurrentSeg == nAudSegCount)) {
			nAudioBufIdx ^= 1;
			nCurrentSeg = 0;
			sceAudioOutputPannedBlocking(nAudioChannel, PSP_AUDIO_VOLUME_MAX, PSP_AUDIO_VOLUME_MAX, pAudioSndBuf[1 - nAudioBufIdx]);
		//} else {
		//	sceKernelSleepThread();
		}
	}

	sceKernelExitThread(0);

	return 0;
}

/************************************/

static int pspBlankSound()
{
	if (pAudioSndBuf[0]) {
		/* Check to see if there's any Cache issue between the CPU and the sound hardware */
		memset(pAudioSndBuf[0], 0, (nAudLoopLen << 1) + (nAudSegLen << 2));
	}

	return 0;
}

static int pspSoundGetNextSoundFiller(int)							// int bDraw
{
	if (nAudNextSound == NULL) {
		return 1;
	}
	memset(nAudNextSound, 0, nAudSegLen << 2);						// Write silence into the buffer

	return 0;
}

int (*pspSoundGetNextSound) (int bDraw) = pspSoundGetNextSoundFiller;	// Callback used to request more sound

static int pspSoundCheck()
{
	char *pData;
	int nPspSegLen;

	if (pAudioSndBuf[0] == NULL)
		return 1;

	if (nCurrentSeg == nAudSegCount) {
		//Sleep(2);													// Don't need to do anything for a bit
		return 0;
	}

	pData = (char*)pAudioSndBuf[nAudioBufIdx];
	nPspSegLen = nAudSegLen;

	if (nAudSampleRate == 22050)
		nPspSegLen <<= 1;
	else if (nAudSampleRate == 11025)
		nPspSegLen <<= 2;

	pData += nCurrentSeg * nPspSegLen;

	pspSoundCopy((short*)pData, nAudNextSound, nAudSegLen);
	pspSoundGetNextSound(1);									// get more sound into nAudNextSound

	nCurrentSeg++;
	//if ((nCurrentSeg == nAudSegCount) && (nAudioThreadId >= 0)) {
	//	sceKernelWakeupThread(nAudioThreadId);
	//}
	if (bAudPlaying && (nCurrentSeg == nAudSegCount)) {
		sceAudioOutputPanned(nAudioChannel, PSP_AUDIO_VOLUME_MAX, PSP_AUDIO_VOLUME_MAX, pAudioSndBuf[nAudioBufIdx]);
		nAudioBufIdx ^= 1;
		nCurrentSeg = 0;
	}

	return 0;
}

static int pspSetCallback(int (*pCallback)(int))
{
	if (pCallback == NULL) {
		pspSoundGetNextSound = pspSoundGetNextSoundFiller;
	} else {
		pspSoundGetNextSound = pCallback;
	}
	return 0;
}

static int pspSoundExit()
{
	bAudPlaying = 0;
	bAudioTerminate = 1;
	bAudOkay = 0;													// This module is no longer okay

	if (nAudioThreadId >= 0) {
		sceKernelDeleteThread(nAudioThreadId);
		nAudioThreadId = -1;
	}

	if (nAudioChannel >= 0) {
		sceAudioChRelease(nAudioChannel);
		nAudioChannel = -1;
	}

	if (pAudioSndBuf[0]) {
		free(pAudioSndBuf[0]);
		pAudioSndBuf[0] = NULL;
		pAudioSndBuf[1] = NULL;
		nAudNextSound = NULL;
	}

	return 0;
}

static int pspSoundInit()
{
	//if (nAudSampleRate <= 0) {
	//	return 1;
	//}
	if ((nAudSampleRate != 11025) && (nAudSampleRate != 22050) && (nAudSampleRate != 44100)) {
		return 1;
	}
	nSoundFps = nAppVirtualFps;
	nAudSegLen = (nAudSampleRate * 100 + (nSoundFps >> 1)) / nSoundFps;
	nAudLoopLenSample = nAudSegLen * nAudSegCount;

	if (nAudSampleRate == 11025) {
		nAudLoopLenSample << 2;
		pspSoundCopy = pspSoundCopy11025;
	} else if (nAudSampleRate == 22050) {
		nAudLoopLenSample << 1;
		pspSoundCopy = pspSoundCopy22050;
	} else {
		pspSoundCopy = pspSoundCopy44100;
	}

	nAudLoopLenSample = PSP_AUDIO_SAMPLE_ALIGN(nAudLoopLenSample);
	nAudLoopLen = nAudLoopLenSample << 2;

	pAudioSndBuf[0] = (short*)malloc((nAudLoopLen << 1) + (nAudSegLen << 2));
	pAudioSndBuf[1] = pAudioSndBuf[0] + (nAudLoopLenSample << 1);
	nAudNextSound = pAudioSndBuf[1] + (nAudLoopLenSample << 1);		// The next sound block to put in the stream

	if (pAudioSndBuf[0] == NULL) {
		pspSoundExit();
		return 1;
	}

	nAudioChannel = sceAudioChReserve(PSP_AUDIO_NEXT_CHANNEL, nAudLoopLenSample, PSP_AUDIO_FORMAT_STEREO);

	if (nAudioChannel < 0) {
		pspSoundExit();
		return 1;
	}

	//nAudioThreadId = sceKernelCreateThread("audio_thread", AudioChannelThread, 0x18, 0x1000, PSP_THREAD_ATTR_USER, NULL);

	//if (nAudioThreadId < 0) {
	//	pspSoundExit();
	//	return 1;
	//}

	memset(pAudioSndBuf[0], 0, (nAudLoopLen << 1) + (nAudSegLen << 2));

	bAudioTerminate = 0;

	//if (sceKernelStartThread(nAudioThreadId, 0, NULL)) {
	//	pspSoundExit();
	//	return 1;
	//}

	bAudOkay = 1;											// This module was initted okay
	return 0;
}

static int pspSoundPlay()
{
	if (bAudOkay == 0) {
		return 1;
	}
	pspBlankSound();
	bAudPlaying = 1;

	return 0;
}

static int pspSoundStop()
{
	bAudPlaying = 0;
	return 0;
}

static int pspSoundSetVolume()
{
	return 1;
}

static int pspSoundGetSettings(InterfaceInfo* pInfo)
{
	{
	}
	return 1;
}

struct AudOut AudOutPSP = { pspBlankSound, pspSoundCheck, pspSoundInit, pspSetCallback, pspSoundPlay, pspSoundStop, pspSoundExit, pspSoundSetVolume, pspSoundGetSettings, _T("psp audio output") };
