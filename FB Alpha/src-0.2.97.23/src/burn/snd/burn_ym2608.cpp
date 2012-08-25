#include "burnint.h"
#include "burn_sound.h"
#include "burn_ym2608.h"

void (*BurnYM2608Update)(INT16* pSoundBuf, INT32 nSegmentEnd);

static INT32 (*BurnYM2608StreamCallback)(INT32 nSoundRate);

static INT32 nBurnYM2608SoundRate;

static INT16* pBuffer;
static INT16* pYM2608Buffer[6];

static INT32* pAYBuffer;

static INT32 nYM2608Position;
static INT32 nAY8910Position;

static UINT32 nSampleSize;
static INT32 nFractionalPosition;

static INT32 bYM2608AddSignal;

static double YM2608Volumes[3];
static INT32 YM2608RouteDirs[3];

// ----------------------------------------------------------------------------
// Dummy functions

static void YM2608UpdateDummy(INT16*, INT32)
{
	return;
}

static INT32 YM2608StreamCallbackDummy(INT32)
{
	return 0;
}

// ----------------------------------------------------------------------------
// Execute YM2608 for part of a frame

static void AY8910Render(INT32 nSegmentLength)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2608Initted) bprintf(PRINT_ERROR, _T("BurnYM2608 AY8910Render called without init\n"));
#endif

	if (nAY8910Position >= nSegmentLength) {
		return;
	}

	nSegmentLength -= nAY8910Position;

	pYM2608Buffer[2] = pBuffer + 2 * 4096 + 4 + nAY8910Position;
	pYM2608Buffer[3] = pBuffer + 3 * 4096 + 4 + nAY8910Position;
	pYM2608Buffer[4] = pBuffer + 4 * 4096 + 4 + nAY8910Position;

	AY8910Update(0, &pYM2608Buffer[2], nSegmentLength);

	nAY8910Position += nSegmentLength;
}

static void YM2608Render(INT32 nSegmentLength)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2608Initted) bprintf(PRINT_ERROR, _T("YM2608Render called without init\n"));
#endif

	if (nYM2608Position >= nSegmentLength) {
		return;
	}

	nSegmentLength -= nYM2608Position;

	pYM2608Buffer[0] = pBuffer + 0 * 4096 + 4 + nYM2608Position;
	pYM2608Buffer[1] = pBuffer + 1 * 4096 + 4 + nYM2608Position;

	YM2608UpdateOne(0, &pYM2608Buffer[0], nSegmentLength);

	nYM2608Position += nSegmentLength;
}

// ----------------------------------------------------------------------------
// Update the sound buffer

static void YM2608UpdateResample(INT16* pSoundBuf, INT32 nSegmentEnd)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2608Initted) bprintf(PRINT_ERROR, _T("YM2608UpdateResample called without init\n"));
#endif

	INT32 nSegmentLength = nSegmentEnd;
	INT32 nSamplesNeeded = nSegmentEnd * nBurnYM2608SoundRate / nBurnSoundRate + 1;

	if (nSamplesNeeded < nAY8910Position) {
		nSamplesNeeded = nAY8910Position;
	}
	if (nSamplesNeeded < nYM2608Position) {
		nSamplesNeeded = nYM2608Position;
	}

	if (nSegmentLength > nBurnSoundLen) {
		nSegmentLength = nBurnSoundLen;
	}
	nSegmentLength <<= 1;

	YM2608Render(nSamplesNeeded);
	AY8910Render(nSamplesNeeded);

	pYM2608Buffer[0] = pBuffer + 0 * 4096 + 4;
	pYM2608Buffer[1] = pBuffer + 1 * 4096 + 4;
	pYM2608Buffer[2] = pBuffer + 2 * 4096 + 4;
	pYM2608Buffer[3] = pBuffer + 3 * 4096 + 4;
	pYM2608Buffer[4] = pBuffer + 4 * 4096 + 4;
	pYM2608Buffer[5] = pBuffer + 5 * 4096 + 4;

	for (INT32 i = (nFractionalPosition >> 16) - 4; i < nSamplesNeeded; i++) {
		pYM2608Buffer[5][i] = (INT32)((pYM2608Buffer[2][i] + pYM2608Buffer[3][i] + pYM2608Buffer[4][i]) * YM2608Volumes[BURN_SND_YM2608_AY8910_ROUTE]);
	}

	for (INT32 i = (nFractionalPosition & 0xFFFF0000) >> 15; i < nSegmentLength; i += 2, nFractionalPosition += nSampleSize) {
		INT32 nLeftSample[4] = {0, 0, 0, 0};
		INT32 nRightSample[4] = {0, 0, 0, 0};
		INT32 nTotalLeftSample, nTotalRightSample;
		
		if ((YM2608RouteDirs[BURN_SND_YM2608_AY8910_ROUTE] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample[0] += pYM2608Buffer[5][(nFractionalPosition >> 16) - 3];
			nLeftSample[1] += pYM2608Buffer[5][(nFractionalPosition >> 16) - 2];
			nLeftSample[2] += pYM2608Buffer[5][(nFractionalPosition >> 16) - 1];
			nLeftSample[3] += pYM2608Buffer[5][(nFractionalPosition >> 16) - 0];
		}
		if ((YM2608RouteDirs[BURN_SND_YM2608_AY8910_ROUTE] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample[0] += pYM2608Buffer[5][(nFractionalPosition >> 16) - 3];
			nRightSample[1] += pYM2608Buffer[5][(nFractionalPosition >> 16) - 2];
			nRightSample[2] += pYM2608Buffer[5][(nFractionalPosition >> 16) - 1];
			nRightSample[3] += pYM2608Buffer[5][(nFractionalPosition >> 16) - 0];
		}
		
		if ((YM2608RouteDirs[BURN_SND_YM2608_YM2608_ROUTE_1] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample[0] += (INT32)(pYM2608Buffer[0][(nFractionalPosition >> 16) - 3] * YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_1]);
			nLeftSample[1] += (INT32)(pYM2608Buffer[0][(nFractionalPosition >> 16) - 2] * YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_1]);
			nLeftSample[2] += (INT32)(pYM2608Buffer[0][(nFractionalPosition >> 16) - 1] * YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_1]);
			nLeftSample[3] += (INT32)(pYM2608Buffer[0][(nFractionalPosition >> 16) - 0] * YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_1]);
		}
		if ((YM2608RouteDirs[BURN_SND_YM2608_YM2608_ROUTE_1] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample[0] += (INT32)(pYM2608Buffer[0][(nFractionalPosition >> 16) - 3] * YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_1]);
			nRightSample[1] += (INT32)(pYM2608Buffer[0][(nFractionalPosition >> 16) - 2] * YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_1]);
			nRightSample[2] += (INT32)(pYM2608Buffer[0][(nFractionalPosition >> 16) - 1] * YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_1]);
			nRightSample[3] += (INT32)(pYM2608Buffer[0][(nFractionalPosition >> 16) - 0] * YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_1]);
		}
		
		if ((YM2608RouteDirs[BURN_SND_YM2608_YM2608_ROUTE_2] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample[0] += (INT32)(pYM2608Buffer[1][(nFractionalPosition >> 16) - 3] * YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_2]);
			nLeftSample[1] += (INT32)(pYM2608Buffer[1][(nFractionalPosition >> 16) - 2] * YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_2]);
			nLeftSample[2] += (INT32)(pYM2608Buffer[1][(nFractionalPosition >> 16) - 1] * YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_2]);
			nLeftSample[3] += (INT32)(pYM2608Buffer[1][(nFractionalPosition >> 16) - 0] * YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_2]);
		}
		if ((YM2608RouteDirs[BURN_SND_YM2608_YM2608_ROUTE_2] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample[0] += (INT32)(pYM2608Buffer[1][(nFractionalPosition >> 16) - 3] * YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_2]);
			nRightSample[1] += (INT32)(pYM2608Buffer[1][(nFractionalPosition >> 16) - 2] * YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_2]);
			nRightSample[2] += (INT32)(pYM2608Buffer[1][(nFractionalPosition >> 16) - 1] * YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_2]);
			nRightSample[3] += (INT32)(pYM2608Buffer[1][(nFractionalPosition >> 16) - 0] * YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_2]);
		}
		
		nTotalLeftSample = INTERPOLATE4PS_16BIT((nFractionalPosition >> 4) & 0x0fff, nLeftSample[0], nLeftSample[1], nLeftSample[2], nLeftSample[3]);
		nTotalRightSample = INTERPOLATE4PS_16BIT((nFractionalPosition >> 4) & 0x0fff, nRightSample[0], nRightSample[1], nRightSample[2], nRightSample[3]);
		
		nTotalLeftSample = BURN_SND_CLIP(nTotalLeftSample);
		nTotalRightSample = BURN_SND_CLIP(nTotalRightSample);
			
		if (bYM2608AddSignal) {
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
			pYM2608Buffer[0][i] = pYM2608Buffer[0][(nFractionalPosition >> 16) + i];
			pYM2608Buffer[1][i] = pYM2608Buffer[1][(nFractionalPosition >> 16) + i];
			pYM2608Buffer[2][i] = pYM2608Buffer[2][(nFractionalPosition >> 16) + i];
			pYM2608Buffer[3][i] = pYM2608Buffer[3][(nFractionalPosition >> 16) + i];
			pYM2608Buffer[4][i] = pYM2608Buffer[4][(nFractionalPosition >> 16) + i];
		}

		nFractionalPosition &= 0xFFFF;

		nYM2608Position = nExtraSamples;
		nAY8910Position = nExtraSamples;

		dTime += 100.0 / nBurnFPS;
	}
}

static void YM2608UpdateNormal(INT16* pSoundBuf, INT32 nSegmentEnd)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2608Initted) bprintf(PRINT_ERROR, _T("YM2608UpdateNormal called without init\n"));
#endif

	INT32 nSegmentLength = nSegmentEnd;

	if (nSegmentEnd < nAY8910Position) {
		nSegmentEnd = nAY8910Position;
	}
	if (nSegmentEnd < nYM2608Position) {
		nSegmentEnd = nYM2608Position;
	}

	if (nSegmentLength > nBurnSoundLen) {
		nSegmentLength = nBurnSoundLen;
	}

	YM2608Render(nSegmentEnd);
	AY8910Render(nSegmentEnd);

	pYM2608Buffer[0] = pBuffer + 4 + 0 * 4096;
	pYM2608Buffer[1] = pBuffer + 4 + 1 * 4096;
	pYM2608Buffer[2] = pBuffer + 4 + 2 * 4096;
	pYM2608Buffer[3] = pBuffer + 4 + 3 * 4096;
	pYM2608Buffer[4] = pBuffer + 4 + 4 * 4096;

	for (INT32 n = nFractionalPosition; n < nSegmentLength; n++) {
		INT32 nAYSample, nLeftSample = 0, nRightSample = 0;

		nAYSample  = pYM2608Buffer[2][n];
		nAYSample += pYM2608Buffer[3][n];
		nAYSample += pYM2608Buffer[4][n];

		if ((YM2608RouteDirs[BURN_SND_YM2608_AY8910_ROUTE] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(nAYSample * YM2608Volumes[BURN_SND_YM2608_AY8910_ROUTE]);
		}
		if ((YM2608RouteDirs[BURN_SND_YM2608_AY8910_ROUTE] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(nAYSample * YM2608Volumes[BURN_SND_YM2608_AY8910_ROUTE]);
		}
		
		if ((YM2608RouteDirs[BURN_SND_YM2608_YM2608_ROUTE_1] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(pYM2608Buffer[0][n] * YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_1]);
		}
		if ((YM2608RouteDirs[BURN_SND_YM2608_YM2608_ROUTE_1] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(pYM2608Buffer[0][n] * YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_1]);
		}
		
		if ((YM2608RouteDirs[BURN_SND_YM2608_YM2608_ROUTE_2] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(pYM2608Buffer[1][n] * YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_2]);
		}
		if ((YM2608RouteDirs[BURN_SND_YM2608_YM2608_ROUTE_2] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(pYM2608Buffer[1][n] * YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_2]);
		}
		
		nLeftSample = BURN_SND_CLIP(nLeftSample);
		nRightSample = BURN_SND_CLIP(nRightSample);
			
		if (bYM2608AddSignal) {
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

		for (INT32 i = 0; i < nExtraSamples; i++) {
			pYM2608Buffer[0][i] = pYM2608Buffer[0][nBurnSoundLen + i];
			pYM2608Buffer[1][i] = pYM2608Buffer[1][nBurnSoundLen + i];
			pYM2608Buffer[2][i] = pYM2608Buffer[2][nBurnSoundLen + i];
			pYM2608Buffer[3][i] = pYM2608Buffer[3][nBurnSoundLen + i];
			pYM2608Buffer[4][i] = pYM2608Buffer[4][nBurnSoundLen + i];
		}

		nFractionalPosition = 0;

		nYM2608Position = nExtraSamples;
		nAY8910Position = nExtraSamples;

		dTime += 100.0 / nBurnFPS;
	}
}

// ----------------------------------------------------------------------------
// Callbacks for YM2608 core

void BurnYM2608UpdateRequest()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2608Initted) bprintf(PRINT_ERROR, _T("BurnYM2608UpdateRequest called without init\n"));
#endif

	YM2608Render(BurnYM2608StreamCallback(nBurnYM2608SoundRate));
}

static void BurnAY8910UpdateRequest()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2608Initted) bprintf(PRINT_ERROR, _T("BurnYM2608 BurnAY8910UpdateRequest called without init\n"));
#endif

	AY8910Render(BurnYM2608StreamCallback(nBurnYM2608SoundRate));
}

// ----------------------------------------------------------------------------
// Initialisation, etc.

void BurnYM2608Reset()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2608Initted) bprintf(PRINT_ERROR, _T("BurnYM2608Reset called without init\n"));
#endif

	BurnTimerReset();

	YM2608ResetChip(0);
}

void BurnYM2608Exit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2608Initted) bprintf(PRINT_ERROR, _T("BurnYM2608Exit called without init\n"));
#endif

	YM2608Shutdown();
	AY8910Exit(0);

	BurnTimerExit();

	if (pBuffer) {
		free(pBuffer);
		pBuffer = NULL;
	}
	if (pAYBuffer) {
		free(pAYBuffer);
		pAYBuffer = NULL;
	}
	
	bYM2608AddSignal = 0;
	
	DebugSnd_YM2608Initted = 0;
}

INT32 BurnYM2608Init(INT32 nClockFrequency, UINT8* YM2608ADPCMROM, INT32* nYM2608ADPCMSize, FM_IRQHANDLER IRQCallback, INT32 (*StreamCallback)(INT32), double (*GetTimeCallback)(), INT32 bAddSignal)
{
	DebugSnd_YM2608Initted = 1;
	
	BurnTimerInit(&YM2608TimerOver, GetTimeCallback);

	if (nBurnSoundRate <= 0) {
		BurnYM2608StreamCallback = YM2608StreamCallbackDummy;

		BurnYM2608Update = YM2608UpdateDummy;

		AY8910InitYM(0, nClockFrequency, 11025, NULL, NULL, NULL, NULL, BurnAY8910UpdateRequest);
		YM2608Init(1, nClockFrequency, 11025, (void**)(&YM2608ADPCMROM), nYM2608ADPCMSize, &BurnOPNTimerCallback, IRQCallback);
		return 0;
	}

	BurnYM2608StreamCallback = StreamCallback;

	if (nFMInterpolation == 3) {
		// Set YM2608 core samplerate to match the hardware
		nBurnYM2608SoundRate = nClockFrequency / 144;
		// Bring YM2608 core samplerate within usable range
		while (nBurnYM2608SoundRate > nBurnSoundRate * 3) {
			nBurnYM2608SoundRate >>= 1;
		}

		BurnYM2608Update = YM2608UpdateResample;

		nSampleSize = (UINT32)nBurnYM2608SoundRate * (1 << 16) / nBurnSoundRate;
		nFractionalPosition = 0;
	} else {
		nBurnYM2608SoundRate = nBurnSoundRate;

		BurnYM2608Update = YM2608UpdateNormal;
	}

	AY8910InitYM(0, nClockFrequency, nBurnYM2608SoundRate, NULL, NULL, NULL, NULL, BurnAY8910UpdateRequest);
	YM2608Init(1, nClockFrequency, nBurnYM2608SoundRate, (void**)(&YM2608ADPCMROM), nYM2608ADPCMSize, &BurnOPNTimerCallback, IRQCallback);

	pBuffer = (INT16*)malloc(4096 * 6 * sizeof(INT16));
	memset(pBuffer, 0, 4096 * 6 * sizeof(INT16));

	pAYBuffer = (INT32*)malloc(4096 * sizeof(INT32));
	memset(pAYBuffer, 0, 4096 * sizeof(INT32));

	nYM2608Position = 0;
	nAY8910Position = 0;
	
	bYM2608AddSignal = bAddSignal;
	
	// default routes
	YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_1] = 1.00;
	YM2608Volumes[BURN_SND_YM2608_YM2608_ROUTE_2] = 1.00;
	YM2608Volumes[BURN_SND_YM2608_AY8910_ROUTE] = 1.00;
	YM2608RouteDirs[BURN_SND_YM2608_YM2608_ROUTE_1] = BURN_SND_ROUTE_LEFT;
	YM2608RouteDirs[BURN_SND_YM2608_YM2608_ROUTE_2] = BURN_SND_ROUTE_RIGHT;
	YM2608RouteDirs[BURN_SND_YM2608_AY8910_ROUTE] = BURN_SND_ROUTE_BOTH;

	return 0;
}

void BurnYM2608SetRoute(INT32 nIndex, double nVolume, INT32 nRouteDir)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2608Initted) bprintf(PRINT_ERROR, _T("BurnYM2608SetRoute called without init\n"));
	if (nIndex < 0 || nIndex > 2) bprintf(PRINT_ERROR, _T("BurnYM2608SetRoute called with invalid index %i\n"), nIndex);
#endif
	
	YM2608Volumes[nIndex] = nVolume;
	YM2608RouteDirs[nIndex] = nRouteDir;
}

void BurnYM2608Scan(INT32 nAction, INT32* pnMin)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2608Initted) bprintf(PRINT_ERROR, _T("BurnYM2608Scan called without init\n"));
#endif

	BurnTimerScan(nAction, pnMin);
	AY8910Scan(nAction, pnMin);

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(nYM2608Position);
		SCAN_VAR(nAY8910Position);
	}
}
