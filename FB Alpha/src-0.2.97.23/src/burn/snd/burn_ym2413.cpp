#include "burnint.h"
#include "burn_sound.h"
#include "burn_ym2413.h"

void (*BurnYM2413Render)(INT16* pSoundBuf, INT32 nSegmentLength);

static INT32 nBurnYM2413SoundRate;

static INT16* pBuffer;
static INT16* pYM2413Buffer[2];

static INT32 nBurnPosition;
static UINT32 nSampleSize;
static INT32 nFractionalPosition;
static UINT32 nSamplesRendered;

static double YM2413Volumes[2];
static INT32 YM2413RouteDirs[2];

#if 0
static void YM2413RenderResample(INT16* pSoundBuf, INT32 nSegmentLength)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2413Initted) bprintf(PRINT_ERROR, _T("YM2413RenderResample called without init\n"));
#endif

	// not currently supported (old code crashed)
}
#endif

static void YM2413RenderNormal(INT16* pSoundBuf, INT32 nSegmentLength)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2413Initted) bprintf(PRINT_ERROR, _T("YM2413RenderNormal called without init\n"));
#endif

	nBurnPosition += nSegmentLength;

	pYM2413Buffer[0] = pBuffer;
	pYM2413Buffer[1] = pBuffer + nSegmentLength;

	YM2413UpdateOne(0, pYM2413Buffer, nSegmentLength);
	
	for (INT32 n = 0; n < nSegmentLength; n++) {
		INT32 nLeftSample = 0, nRightSample = 0;
		
		if ((YM2413RouteDirs[BURN_SND_YM2413_YM2413_ROUTE_1] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(pYM2413Buffer[0][n] * YM2413Volumes[BURN_SND_YM2413_YM2413_ROUTE_1]);
		}
		if ((YM2413RouteDirs[BURN_SND_YM2413_YM2413_ROUTE_1] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(pYM2413Buffer[0][n] * YM2413Volumes[BURN_SND_YM2413_YM2413_ROUTE_1]);
		}
		
		if ((YM2413RouteDirs[BURN_SND_YM2413_YM2413_ROUTE_2] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(pYM2413Buffer[1][n] * YM2413Volumes[BURN_SND_YM2413_YM2413_ROUTE_2]);
		}
		if ((YM2413RouteDirs[BURN_SND_YM2413_YM2413_ROUTE_2] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(pYM2413Buffer[1][n] * YM2413Volumes[BURN_SND_YM2413_YM2413_ROUTE_2]);
		}
		
		nLeftSample = BURN_SND_CLIP(nLeftSample);
		nRightSample = BURN_SND_CLIP(nRightSample);
			
		pSoundBuf[(n << 1) + 0] = nLeftSample;
		pSoundBuf[(n << 1) + 1] = nRightSample;
	}
}

void BurnYM2413Reset()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2413Initted) bprintf(PRINT_ERROR, _T("BurnYM2413Reset called without init\n"));
#endif

	YM2413ResetChip(0);
}

void BurnYM2413Exit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2413Initted) bprintf(PRINT_ERROR, _T("BurnYM2413Exit called without init\n"));
#endif
	YM2413Shutdown();

	if (pBuffer) {
		free(pBuffer);
		pBuffer = NULL;
	}
	
	DebugSnd_YM2413Initted = 0;
}

INT32 BurnYM2413Init(INT32 nClockFrequency)
{
	DebugSnd_YM2413Initted = 1;
	
	if (nBurnSoundRate <= 0) {
		YM2413Init(1, nClockFrequency, 11025);
		return 0;
	}

#if 0
	if (nFMInterpolation == 3) {
		// Set YM2413 core samplerate to match the hardware
		nBurnYM2413SoundRate = nClockFrequency >> 6;
		// Bring YM2413 core samplerate within usable range
		while (nBurnYM2413SoundRate > nBurnSoundRate * 3) {
			nBurnYM2413SoundRate >>= 1;
		}

		BurnYM2413Render = YM2413RenderResample;
	} else {
		nBurnYM2413SoundRate = nBurnSoundRate;
		BurnYM2413Render = YM2413RenderNormal;
	}
#else
	nBurnYM2413SoundRate = nBurnSoundRate;
	BurnYM2413Render = YM2413RenderNormal;
#endif

	YM2413Init(1, nClockFrequency, nBurnYM2413SoundRate);

	pBuffer = (INT16*)malloc(65536 * 2 * sizeof(INT16));
	memset(pBuffer, 0, 65536 * 2 * sizeof(INT16));

	nSampleSize = (UINT32)nBurnYM2413SoundRate * (1 << 16) / nBurnSoundRate;
	nFractionalPosition = 4 << 16;
	nSamplesRendered = 0;
	nBurnPosition = 0;
	
	// default routes
	YM2413Volumes[BURN_SND_YM2413_YM2413_ROUTE_1] = 1.00;
	YM2413Volumes[BURN_SND_YM2413_YM2413_ROUTE_2] = 1.00;
	YM2413RouteDirs[BURN_SND_YM2413_YM2413_ROUTE_1] = BURN_SND_ROUTE_BOTH;
	YM2413RouteDirs[BURN_SND_YM2413_YM2413_ROUTE_2] = BURN_SND_ROUTE_BOTH;

	return 0;
}

void BurnYM2413SetRoute(INT32 nIndex, double nVolume, INT32 nRouteDir)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2413Initted) bprintf(PRINT_ERROR, _T("BurnYM2413SetRoute called without init\n"));
	if (nIndex < 0 || nIndex > 1) bprintf(PRINT_ERROR, _T("BurnYM2413SetRoute called with invalid index %i\n"), nIndex);
#endif
	
	YM2413Volumes[nIndex] = nVolume;
	YM2413RouteDirs[nIndex] = nRouteDir;
}

void BurnYM2413Scan(INT32 nAction)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2413Initted) bprintf(PRINT_ERROR, _T("BurnYM2413Scan called without init\n"));
#endif

	if ((nAction & ACB_DRIVER_DATA) == 0) {
		return;
	}
	
	YM2413Scan(0, nAction);
}
