#include "burnint.h"
#include "burn_sound.h"
#include "burn_ym2612.h"

#define MAX_YM2612	2

void (*BurnYM2612Update)(INT16* pSoundBuf, INT32 nSegmentEnd);

static INT32 (*BurnYM2612StreamCallback)(INT32 nSoundRate);

static INT32 nBurnYM2612SoundRate;

static INT16* pBuffer;
static INT16* pYM2612Buffer[2 * MAX_YM2612];

static INT32 nYM2612Position;

static UINT32 nSampleSize;
static INT32 nFractionalPosition;

static INT32 nNumChips = 0;
static INT32 bYM2612AddSignal;

static double YM2612Volumes[2 * MAX_YM2612];
static INT32 YM2612RouteDirs[2 * MAX_YM2612];

// ----------------------------------------------------------------------------
// Dummy functions

static void YM2612UpdateDummy(INT16*, INT32)
{
	return;
}

static INT32 YM2612StreamCallbackDummy(INT32)
{
	return 0;
}

// ----------------------------------------------------------------------------
// Execute YM2612 for part of a frame

static void YM2612Render(INT32 nSegmentLength)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2612Initted) bprintf(PRINT_ERROR, _T("YM2612Render called without init\n"));
#endif
	
	if (nYM2612Position >= nSegmentLength) {
		return;
	}

	nSegmentLength -= nYM2612Position;
	
	pYM2612Buffer[0] = pBuffer + 0 * 4096 + 4 + nYM2612Position;
	pYM2612Buffer[1] = pBuffer + 1 * 4096 + 4 + nYM2612Position;

	YM2612UpdateOne(0, &pYM2612Buffer[0], nSegmentLength);
		
	if (nNumChips > 1) {
		pYM2612Buffer[2] = pBuffer + 2 * 4096 + 4 + nYM2612Position;
		pYM2612Buffer[3] = pBuffer + 3 * 4096 + 4 + nYM2612Position;

		YM2612UpdateOne(1, &pYM2612Buffer[2], nSegmentLength);
	}

	nYM2612Position += nSegmentLength;
}

// ----------------------------------------------------------------------------

// Update the sound buffer

#define INTERPOLATE_ADD_SOUND_LEFT(route, buffer)																	\
	if ((YM2612RouteDirs[route] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {									\
		nLeftSample[0] += (INT32)(pYM2612Buffer[buffer][(nFractionalPosition >> 16) - 3] * YM2612Volumes[route]);	\
		nLeftSample[1] += (INT32)(pYM2612Buffer[buffer][(nFractionalPosition >> 16) - 2] * YM2612Volumes[route]);	\
		nLeftSample[2] += (INT32)(pYM2612Buffer[buffer][(nFractionalPosition >> 16) - 1] * YM2612Volumes[route]);	\
		nLeftSample[3] += (INT32)(pYM2612Buffer[buffer][(nFractionalPosition >> 16) - 0] * YM2612Volumes[route]);	\
	}

#define INTERPOLATE_ADD_SOUND_RIGHT(route, buffer)																	\
	if ((YM2612RouteDirs[route] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {									\
		nRightSample[0] += (INT32)(pYM2612Buffer[buffer][(nFractionalPosition >> 16) - 3] * YM2612Volumes[route]);	\
		nRightSample[1] += (INT32)(pYM2612Buffer[buffer][(nFractionalPosition >> 16) - 2] * YM2612Volumes[route]);	\
		nRightSample[2] += (INT32)(pYM2612Buffer[buffer][(nFractionalPosition >> 16) - 1] * YM2612Volumes[route]);	\
		nRightSample[3] += (INT32)(pYM2612Buffer[buffer][(nFractionalPosition >> 16) - 0] * YM2612Volumes[route]);	\
	}

static void YM2612UpdateResample(INT16* pSoundBuf, INT32 nSegmentEnd)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2612Initted) bprintf(PRINT_ERROR, _T("YM2612UpdateResample called without init\n"));
#endif

	INT32 nSegmentLength = nSegmentEnd;
	INT32 nSamplesNeeded = nSegmentEnd * nBurnYM2612SoundRate / nBurnSoundRate + 1;

	if (nSamplesNeeded < nYM2612Position) {
		nSamplesNeeded = nYM2612Position;
	}

	if (nSegmentLength > nBurnSoundLen) {
		nSegmentLength = nBurnSoundLen;
	}
	nSegmentLength <<= 1;

	YM2612Render(nSamplesNeeded);

	pYM2612Buffer[0] = pBuffer + 0 * 4096 + 4;
	pYM2612Buffer[1] = pBuffer + 1 * 4096 + 4;
	if (nNumChips > 1) {
		pYM2612Buffer[2] = pBuffer + 2 * 4096 + 4;
		pYM2612Buffer[3] = pBuffer + 3 * 4096 + 4;
	}

	for (INT32 i = (nFractionalPosition & 0xFFFF0000) >> 15; i < nSegmentLength; i += 2, nFractionalPosition += nSampleSize) {
		INT32 nLeftSample[4] = {0, 0, 0, 0};
		INT32 nRightSample[4] = {0, 0, 0, 0};
		INT32 nTotalLeftSample, nTotalRightSample;

		INTERPOLATE_ADD_SOUND_LEFT  (BURN_SND_YM2612_YM2612_ROUTE_1, 0)
		INTERPOLATE_ADD_SOUND_RIGHT (BURN_SND_YM2612_YM2612_ROUTE_1, 0)
		INTERPOLATE_ADD_SOUND_LEFT  (BURN_SND_YM2612_YM2612_ROUTE_2, 1)
		INTERPOLATE_ADD_SOUND_RIGHT (BURN_SND_YM2612_YM2612_ROUTE_2, 1)
		
		if (nNumChips > 1) {
			INTERPOLATE_ADD_SOUND_LEFT  (2 + BURN_SND_YM2612_YM2612_ROUTE_1, 2)
			INTERPOLATE_ADD_SOUND_RIGHT (2 + BURN_SND_YM2612_YM2612_ROUTE_1, 2)
			INTERPOLATE_ADD_SOUND_LEFT  (2 + BURN_SND_YM2612_YM2612_ROUTE_2, 3)
			INTERPOLATE_ADD_SOUND_RIGHT (2 + BURN_SND_YM2612_YM2612_ROUTE_2, 3)
		}
		
		nTotalLeftSample = INTERPOLATE4PS_16BIT((nFractionalPosition >> 4) & 0x0fff, nLeftSample[0], nLeftSample[1], nLeftSample[2], nLeftSample[3]);
		nTotalRightSample = INTERPOLATE4PS_16BIT((nFractionalPosition >> 4) & 0x0fff, nRightSample[0], nRightSample[1], nRightSample[2], nRightSample[3]);
		
		nTotalLeftSample = BURN_SND_CLIP(nTotalLeftSample);
		nTotalRightSample = BURN_SND_CLIP(nTotalRightSample);
			
		if (bYM2612AddSignal) {
			pSoundBuf[i + 0] += nTotalLeftSample;
			pSoundBuf[i + 1] += nTotalRightSample;
		} else {
			pSoundBuf[i + 0] = nTotalLeftSample;
			pSoundBuf[i + 1] = nTotalRightSample;
		}
	}
	
	if (nSegmentEnd >= nBurnSoundLen) {
		INT32 nExtraSamples = nSamplesNeeded - (nFractionalPosition >> 16);

		for (INT32 i = -4; i < nExtraSamples; i++) {
			pYM2612Buffer[0][i] = pYM2612Buffer[0][(nFractionalPosition >> 16) + i];
			pYM2612Buffer[1][i] = pYM2612Buffer[1][(nFractionalPosition >> 16) + i];
			if (nNumChips > 1) {
				pYM2612Buffer[2][i] = pYM2612Buffer[2][(nFractionalPosition >> 16) + i];
				pYM2612Buffer[3][i] = pYM2612Buffer[3][(nFractionalPosition >> 16) + i];
			}
		}

		nFractionalPosition &= 0xFFFF;

		nYM2612Position = nExtraSamples;

		dTime += 100.0 / nBurnFPS;
	}
}

static void YM2612UpdateNormal(INT16* pSoundBuf, INT32 nSegmentEnd)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2612Initted) bprintf(PRINT_ERROR, _T("YM2612UpdateNormal called without init\n"));
#endif

	INT32 nSegmentLength = nSegmentEnd;
	INT32 i;

	if (nSegmentEnd < nYM2612Position) {
		nSegmentEnd = nYM2612Position;
	}

	if (nSegmentLength > nBurnSoundLen) {
		nSegmentLength = nBurnSoundLen;
	}
	
	YM2612Render(nSegmentEnd);

	pYM2612Buffer[0] = pBuffer + 4 + 0 * 4096;
	pYM2612Buffer[1] = pBuffer + 4 + 1 * 4096;
	if (nNumChips > 1) {
		pYM2612Buffer[2] = pBuffer + 4 + 2 * 4096;
		pYM2612Buffer[3] = pBuffer + 4 + 3 * 4096;
	}

	for (INT32 n = nFractionalPosition; n < nSegmentLength; n++) {
		INT32 nLeftSample = 0, nRightSample = 0;

		if ((YM2612RouteDirs[BURN_SND_YM2612_YM2612_ROUTE_1] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(pYM2612Buffer[0][n] * YM2612Volumes[BURN_SND_YM2612_YM2612_ROUTE_1]);
		}
		if ((YM2612RouteDirs[BURN_SND_YM2612_YM2612_ROUTE_1] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(pYM2612Buffer[0][n] * YM2612Volumes[BURN_SND_YM2612_YM2612_ROUTE_1]);
		}
		
		if ((YM2612RouteDirs[BURN_SND_YM2612_YM2612_ROUTE_2] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(pYM2612Buffer[1][n] * YM2612Volumes[BURN_SND_YM2612_YM2612_ROUTE_2]);
		}
		if ((YM2612RouteDirs[BURN_SND_YM2612_YM2612_ROUTE_2] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(pYM2612Buffer[1][n] * YM2612Volumes[BURN_SND_YM2612_YM2612_ROUTE_2]);
		}
		
		if (nNumChips > 1) {
			if ((YM2612RouteDirs[2 + BURN_SND_YM2612_YM2612_ROUTE_1] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
				nLeftSample += (INT32)(pYM2612Buffer[2][n] * YM2612Volumes[2 + BURN_SND_YM2612_YM2612_ROUTE_1]);
			}
			if ((YM2612RouteDirs[2 + BURN_SND_YM2612_YM2612_ROUTE_1] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
				nRightSample += (INT32)(pYM2612Buffer[2][n] * YM2612Volumes[2 + BURN_SND_YM2612_YM2612_ROUTE_1]);
			}
		
			if ((YM2612RouteDirs[2 + BURN_SND_YM2612_YM2612_ROUTE_2] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
				nLeftSample += (INT32)(pYM2612Buffer[3][n] * YM2612Volumes[2 + BURN_SND_YM2612_YM2612_ROUTE_2]);
			}
			if ((YM2612RouteDirs[2 + BURN_SND_YM2612_YM2612_ROUTE_2] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
				nRightSample += (INT32)(pYM2612Buffer[3][n] * YM2612Volumes[2 + BURN_SND_YM2612_YM2612_ROUTE_2]);
			}
		}
		
		nLeftSample = BURN_SND_CLIP(nLeftSample);
		nRightSample = BURN_SND_CLIP(nRightSample);
			
		if (bYM2612AddSignal) {
			pSoundBuf[(n << 1) + 0] += nLeftSample;
			pSoundBuf[(n << 1) + 1] += nRightSample;
		} else {
			pSoundBuf[(n << 1) + 0] = nLeftSample;
			pSoundBuf[(n << 1) + 1] = nRightSample;
		}
	}

	nFractionalPosition = nSegmentLength;

	if (nSegmentEnd >= nBurnSoundLen) {
		INT32 nExtraSamples = nSegmentEnd - nBurnSoundLen;

		for (i = 0; i < nExtraSamples; i++) {
			pYM2612Buffer[0][i] = pYM2612Buffer[0][nBurnSoundLen + i];
			pYM2612Buffer[1][i] = pYM2612Buffer[1][nBurnSoundLen + i];
			if (nNumChips > 1) {
				pYM2612Buffer[2][i] = pYM2612Buffer[2][nBurnSoundLen + i];
				pYM2612Buffer[3][i] = pYM2612Buffer[3][nBurnSoundLen + i];
			}
		}

		nFractionalPosition = 0;

		nYM2612Position = nExtraSamples;

		dTime += 100.0 / nBurnFPS;
	}
}

// ----------------------------------------------------------------------------
// Callbacks for YM2612 core

void BurnYM2612UpdateRequest()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2612Initted) bprintf(PRINT_ERROR, _T("YM2612UpdateRequest called without init\n"));
#endif

	YM2612Render(BurnYM2612StreamCallback(nBurnYM2612SoundRate));
}

// ----------------------------------------------------------------------------
// Initialisation, etc.

void BurnYM2612Reset()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2612Initted) bprintf(PRINT_ERROR, _T("BurnYM2612Reset called without init\n"));
#endif

	BurnTimerReset();
	
	for (INT32 i = 0; i < nNumChips; i++) {
		YM2612ResetChip(i);
	}
}

void BurnYM2612Exit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2612Initted) bprintf(PRINT_ERROR, _T("BurnYM2612Exit called without init\n"));
#endif

	YM2612Shutdown();

	BurnTimerExit();

	if (pBuffer) {
		free(pBuffer);
		pBuffer = NULL;
	}
	
	nNumChips = 0;
	bYM2612AddSignal = 0;
	
	DebugSnd_YM2612Initted = 0;
}

INT32 BurnYM2612Init(INT32 num, INT32 nClockFrequency, FM_IRQHANDLER IRQCallback, INT32 (*StreamCallback)(INT32), double (*GetTimeCallback)(), INT32 bAddSignal)
{
	DebugSnd_YM2612Initted = 1;
	
	if (num > MAX_YM2612) num = MAX_YM2612;

	BurnTimerInit(&YM2612TimerOver, GetTimeCallback);

	if (nBurnSoundRate <= 0) {
		BurnYM2612StreamCallback = YM2612StreamCallbackDummy;

		BurnYM2612Update = YM2612UpdateDummy;

		YM2612Init(num, nClockFrequency, 11025, &BurnOPNTimerCallback, IRQCallback);
		return 0;
	}

	BurnYM2612StreamCallback = StreamCallback;

	if (nFMInterpolation == 3) {
		// Set YM2612 core samplerate to match the hardware
		nBurnYM2612SoundRate = nClockFrequency / 144;
		// Bring YM2612 core samplerate within usable range
		while (nBurnYM2612SoundRate > nBurnSoundRate * 3) {
			nBurnYM2612SoundRate >>= 1;
		}

		BurnYM2612Update = YM2612UpdateResample;

		nSampleSize = (UINT32)nBurnYM2612SoundRate * (1 << 16) / nBurnSoundRate;
	} else {
		nBurnYM2612SoundRate = nBurnSoundRate;

		BurnYM2612Update = YM2612UpdateNormal;
	}
	
	YM2612Init(num, nClockFrequency, nBurnYM2612SoundRate, &BurnOPNTimerCallback, IRQCallback);

	pBuffer = (INT16*)malloc(4096 * 2 * num * sizeof(INT16));
	memset(pBuffer, 0, 4096 * 2 * num * sizeof(INT16));
	
	nYM2612Position = 0;
	nFractionalPosition = 0;
	
	nNumChips = num;
	bYM2612AddSignal = bAddSignal;
	
	// default routes
	YM2612Volumes[BURN_SND_YM2612_YM2612_ROUTE_1] = 1.00;
	YM2612Volumes[BURN_SND_YM2612_YM2612_ROUTE_2] = 1.00;
	YM2612RouteDirs[BURN_SND_YM2612_YM2612_ROUTE_1] = BURN_SND_ROUTE_LEFT;
	YM2612RouteDirs[BURN_SND_YM2612_YM2612_ROUTE_2] = BURN_SND_ROUTE_RIGHT;
	
	if (nNumChips > 0) {
		YM2612Volumes[2 + BURN_SND_YM2612_YM2612_ROUTE_1] = 1.00;
		YM2612Volumes[2 + BURN_SND_YM2612_YM2612_ROUTE_2] = 1.00;
		YM2612RouteDirs[2 + BURN_SND_YM2612_YM2612_ROUTE_1] = BURN_SND_ROUTE_LEFT;
		YM2612RouteDirs[2 + BURN_SND_YM2612_YM2612_ROUTE_2] = BURN_SND_ROUTE_RIGHT;
	}

	return 0;
}

void BurnYM2612SetRoute(INT32 nChip, INT32 nIndex, double nVolume, INT32 nRouteDir)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2612Initted) bprintf(PRINT_ERROR, _T("BurnYM2612SetRoute called without init\n"));
	if (nIndex < 0 || nIndex > 1) bprintf(PRINT_ERROR, _T("BurnYM2612SetRoute called with invalid index %i\n"), nIndex);
	if (nChip >= nNumChips) bprintf(PRINT_ERROR, _T("BurnYM2612SetRoute called with invalid chip %i\n"), nChip);
#endif

	if (nChip == 0) {
		YM2612Volumes[nIndex] = nVolume;
		YM2612RouteDirs[nIndex] = nRouteDir;
	}
	
	if (nChip == 1) {
		YM2612Volumes[2 + nIndex] = nVolume;
		YM2612RouteDirs[2 + nIndex] = nRouteDir;
	}
}

void BurnYM2612Scan(INT32 nAction, INT32* pnMin)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2612Initted) bprintf(PRINT_ERROR, _T("BurnYM2612Scan called without init\n"));
#endif

	BurnTimerScan(nAction, pnMin);

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(nYM2612Position);
	}
}

#undef MAX_YM2612
