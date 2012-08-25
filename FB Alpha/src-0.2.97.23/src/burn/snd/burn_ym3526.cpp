#include "burnint.h"
#include "burn_sound.h"
#include "burn_ym3526.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "m6809_intf.h"
#include "hd6309_intf.h"
#include "m6800_intf.h"
#include "m6502_intf.h"

// Timer Related

#define MAX_TIMER_VALUE ((1 << 30) - 65536)

static double dTimeYM3526;									// Time elapsed since the emulated machine was started

static INT32 nTimerCount[2], nTimerStart[2];

// Callbacks
static INT32 (*pTimerOverCallback)(INT32, INT32);
static double (*pTimerTimeCallback)();

static INT32 nCPUClockspeed = 0;
static INT32 (*pCPUTotalCycles)() = NULL;
static INT32 (*pCPURun)(INT32) = NULL;
static void (*pCPURunEnd)() = NULL;

// ---------------------------------------------------------------------------
// Running time

static double BurnTimerTimeCallbackDummy()
{
	return 0.0;
}

extern "C" double BurnTimerGetTimeYM3526()
{
	return dTimeYM3526 + pTimerTimeCallback();
}

// ---------------------------------------------------------------------------
// Update timers

static INT32 nTicksTotal, nTicksDone, nTicksExtra;

INT32 BurnTimerUpdateYM3526(INT32 nCycles)
{
	INT32 nIRQStatus = 0;

	nTicksTotal = MAKE_TIMER_TICKS(nCycles, nCPUClockspeed);

	while (nTicksDone < nTicksTotal) {
		INT32 nTimer, nCyclesSegment, nTicksSegment;

		// Determine which timer fires first
		if (nTimerCount[0] <= nTimerCount[1]) {
			nTicksSegment = nTimerCount[0];
		} else {
			nTicksSegment = nTimerCount[1];
		}
		if (nTicksSegment > nTicksTotal) {
			nTicksSegment = nTicksTotal;
		}

		nCyclesSegment = MAKE_CPU_CYLES(nTicksSegment + nTicksExtra, nCPUClockspeed);

		pCPURun(nCyclesSegment - pCPUTotalCycles());

		nTicksDone = MAKE_TIMER_TICKS(pCPUTotalCycles() + 1, nCPUClockspeed) - 1;

		nTimer = 0;
		if (nTicksDone >= nTimerCount[0]) {
			if (nTimerStart[0] == MAX_TIMER_VALUE) {
				nTimerCount[0] = MAX_TIMER_VALUE;
			} else {
				nTimerCount[0] += nTimerStart[0];
			}
			nTimer |= 1;
		}
		if (nTicksDone >= nTimerCount[1]) {
			if (nTimerStart[1] == MAX_TIMER_VALUE) {
				nTimerCount[1] = MAX_TIMER_VALUE;
			} else {
				nTimerCount[1] += nTimerStart[1];
			}
			nTimer |= 2;
		}
		if (nTimer & 1) {
			nIRQStatus |= pTimerOverCallback(0, 0);
		}
		if (nTimer & 2) {
			nIRQStatus |= pTimerOverCallback(0, 1);
		}
	}

	return nIRQStatus;
}

void BurnTimerEndFrameYM3526(INT32 nCycles)
{
	int nTicks = MAKE_TIMER_TICKS(nCycles, nCPUClockspeed);

	BurnTimerUpdateYM3526(nCycles);

	if (nTimerCount[0] < MAX_TIMER_VALUE) {
		nTimerCount[0] -= nTicks;
	}
	if (nTimerCount[1] < MAX_TIMER_VALUE) {
		nTimerCount[1] -= nTicks;
	}

	nTicksDone -= nTicks;
	if (nTicksDone < 0) {
		nTicksDone = 0;
	}
}

void BurnTimerUpdateEndYM3526()
{
	pCPURunEnd();

	nTicksTotal = 0;
}

void BurnOPLTimerCallbackYM3526(INT32 c, double period)
{
	pCPURunEnd();

	if (period == 0.0) {
		nTimerCount[c] = MAX_TIMER_VALUE;
		return;
	}

	nTimerCount[c]  = (INT32)(period * (double)TIMER_TICKS_PER_SECOND);
	nTimerCount[c] += MAKE_TIMER_TICKS(pCPUTotalCycles(), nCPUClockspeed);
}

void BurnTimerScanYM3526(INT32 nAction, INT32* pnMin)
{
	if (pnMin && *pnMin < 0x029521) {
		*pnMin = 0x029521;
	}

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(nTimerCount);
		SCAN_VAR(nTimerStart);
		SCAN_VAR(dTimeYM3526);

		SCAN_VAR(nTicksDone);
	}
}

void BurnTimerExitYM3526()
{
	nCPUClockspeed = 0;
	pCPUTotalCycles = NULL;
	pCPURun = NULL;
	pCPURunEnd = NULL;

	return;
}

void BurnTimerResetYM3526()
{
	nTimerCount[0] = nTimerCount[1] = MAX_TIMER_VALUE;
	nTimerStart[0] = nTimerStart[1] = MAX_TIMER_VALUE;

	dTimeYM3526 = 0.0;

	nTicksDone = 0;
}

INT32 BurnTimerInitYM3526(INT32 (*pOverCallback)(INT32, INT32), double (*pTimeCallback)())
{
	BurnTimerExitYM3526();

	pTimerOverCallback = pOverCallback;
	pTimerTimeCallback = pTimeCallback ? pTimeCallback : BurnTimerTimeCallbackDummy;

	BurnTimerResetYM3526();

	return 0;
}

INT32 BurnTimerAttachSekYM3526(INT32 nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = SekTotalCycles;
	pCPURun = SekRun;
	pCPURunEnd = SekRunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

	return 0;
}

INT32 BurnTimerAttachZetYM3526(INT32 nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = ZetTotalCycles;
	pCPURun = ZetRun;
	pCPURunEnd = ZetRunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

	return 0;
}

INT32 BurnTimerAttachM6809YM3526(INT32 nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = M6809TotalCycles;
	pCPURun = M6809Run;
	pCPURunEnd = M6809RunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

	return 0;
}

INT32 BurnTimerAttachHD6309YM3526(INT32 nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = HD6309TotalCycles;
	pCPURun = HD6309Run;
	pCPURunEnd = HD6309RunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

	return 0;
}

INT32 BurnTimerAttachM6800YM3526(INT32 nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = M6800TotalCycles;
	pCPURun = M6800Run;
	pCPURunEnd = M6800RunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

	return 0;
}

INT32 BurnTimerAttachHD63701YM3526(INT32 nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = M6800TotalCycles;
	pCPURun = HD63701Run;
	pCPURunEnd = HD63701RunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

	return 0;
}

INT32 BurnTimerAttachM6803YM3526(INT32 nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = M6800TotalCycles;
	pCPURun = M6803Run;
	pCPURunEnd = M6803RunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

	return 0;
}

INT32 BurnTimerAttachM6502YM3526(INT32 nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = M6502TotalCycles;
	pCPURun = M6502Run;
	pCPURunEnd = M6502RunEnd; // doesn't do anything...

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

	return 0;
}

// Sound Related

void (*BurnYM3526Update)(INT16* pSoundBuf, INT32 nSegmentEnd);

static INT32 (*BurnYM3526StreamCallback)(INT32 nSoundRate);

static INT32 nBurnYM3526SoundRate;

static INT16* pBuffer;
static INT16* pYM3526Buffer;

static INT32 nYM3526Position;

static UINT32 nSampleSize;
static INT32 nFractionalPosition;

static INT32 bYM3526AddSignal;

static double YM3526Volumes[1];
static INT32 YM3526RouteDirs[1];

// ----------------------------------------------------------------------------
// Dummy functions

static void YM3526UpdateDummy(INT16* , INT32)
{
	return;
}

static INT32 YM3526StreamCallbackDummy(INT32)
{
	return 0;
}

// ----------------------------------------------------------------------------
// Execute YM3526 for part of a frame

static void YM3526Render(INT32 nSegmentLength)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM3526Initted) bprintf(PRINT_ERROR, _T("YM3526Render called without init\n"));
#endif

	if (nYM3526Position >= nSegmentLength) {
		return;
	}

	nSegmentLength -= nYM3526Position;

	YM3526UpdateOne(0, pBuffer + 0 * 4096 + 4 + nYM3526Position, nSegmentLength);

	nYM3526Position += nSegmentLength;
}

// ----------------------------------------------------------------------------
// Update the sound buffer

static void YM3526UpdateResample(INT16* pSoundBuf, INT32 nSegmentEnd)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM3526Initted) bprintf(PRINT_ERROR, _T("YM3526UpdateResample called without init\n"));
#endif

	INT32 nSegmentLength = nSegmentEnd;
	INT32 nSamplesNeeded = nSegmentEnd * nBurnYM3526SoundRate / nBurnSoundRate + 1;

	if (nSamplesNeeded < nYM3526Position) {
		nSamplesNeeded = nYM3526Position;
	}

	if (nSegmentLength > nBurnSoundLen) {
		nSegmentLength = nBurnSoundLen;
	}
	nSegmentLength <<= 1;

	YM3526Render(nSamplesNeeded);

	pYM3526Buffer = pBuffer + 0 * 4096 + 4;

	for (INT32 i = (nFractionalPosition & 0xFFFF0000) >> 15; i < nSegmentLength; i += 2, nFractionalPosition += nSampleSize) {
		INT32 nLeftSample[4] = {0, 0, 0, 0};
		INT32 nRightSample[4] = {0, 0, 0, 0};
		INT32 nTotalLeftSample, nTotalRightSample;
		
		if ((YM3526RouteDirs[BURN_SND_YM3526_ROUTE] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample[0] += (INT32)(pYM3526Buffer[(nFractionalPosition >> 16) - 3] * YM3526Volumes[BURN_SND_YM3526_ROUTE]);
			nLeftSample[1] += (INT32)(pYM3526Buffer[(nFractionalPosition >> 16) - 2] * YM3526Volumes[BURN_SND_YM3526_ROUTE]);
			nLeftSample[2] += (INT32)(pYM3526Buffer[(nFractionalPosition >> 16) - 1] * YM3526Volumes[BURN_SND_YM3526_ROUTE]);
			nLeftSample[3] += (INT32)(pYM3526Buffer[(nFractionalPosition >> 16) - 0] * YM3526Volumes[BURN_SND_YM3526_ROUTE]);
		}
		if ((YM3526RouteDirs[BURN_SND_YM3526_ROUTE] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample[0] += (INT32)(pYM3526Buffer[(nFractionalPosition >> 16) - 3] * YM3526Volumes[BURN_SND_YM3526_ROUTE]);
			nRightSample[1] += (INT32)(pYM3526Buffer[(nFractionalPosition >> 16) - 2] * YM3526Volumes[BURN_SND_YM3526_ROUTE]);
			nRightSample[2] += (INT32)(pYM3526Buffer[(nFractionalPosition >> 16) - 1] * YM3526Volumes[BURN_SND_YM3526_ROUTE]);
			nRightSample[3] += (INT32)(pYM3526Buffer[(nFractionalPosition >> 16) - 0] * YM3526Volumes[BURN_SND_YM3526_ROUTE]);
		}
		
		nTotalLeftSample = INTERPOLATE4PS_16BIT((nFractionalPosition >> 4) & 0x0fff, nLeftSample[0], nLeftSample[1], nLeftSample[2], nLeftSample[3]);
		nTotalRightSample = INTERPOLATE4PS_16BIT((nFractionalPosition >> 4) & 0x0fff, nRightSample[0], nRightSample[1], nRightSample[2], nRightSample[3]);
		
		nTotalLeftSample = BURN_SND_CLIP(nTotalLeftSample);
		nTotalRightSample = BURN_SND_CLIP(nTotalRightSample);
			
		if (bYM3526AddSignal) {
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
			pYM3526Buffer[i] = pYM3526Buffer[(nFractionalPosition >> 16) + i];
		}

		nFractionalPosition &= 0xFFFF;

		nYM3526Position = nExtraSamples;
	}
}

static void YM3526UpdateNormal(INT16* pSoundBuf, INT32 nSegmentEnd)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM3526Initted) bprintf(PRINT_ERROR, _T("YM3526UpdateNormal called without init\n"));
#endif

	INT32 nSegmentLength = nSegmentEnd;

	if (nSegmentEnd < nYM3526Position) {
		nSegmentEnd = nYM3526Position;
	}

	if (nSegmentLength > nBurnSoundLen) {
		nSegmentLength = nBurnSoundLen;
	}

	YM3526Render(nSegmentEnd);

	pYM3526Buffer = pBuffer + 4 + 0 * 4096;

	for (INT32 n = nFractionalPosition; n < nSegmentLength; n++) {
		INT32 nLeftSample = 0, nRightSample = 0;
		
		if ((YM3526RouteDirs[BURN_SND_YM3526_ROUTE] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(pYM3526Buffer[n] * YM3526Volumes[BURN_SND_YM3526_ROUTE]);
		}
		if ((YM3526RouteDirs[BURN_SND_YM3526_ROUTE] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(pYM3526Buffer[n] * YM3526Volumes[BURN_SND_YM3526_ROUTE]);
		}
		
		nLeftSample = BURN_SND_CLIP(nLeftSample);
		nRightSample = BURN_SND_CLIP(nRightSample);
			
		if (bYM3526AddSignal) {
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
			pYM3526Buffer[i] = pYM3526Buffer[nBurnSoundLen + i];
		}

		nFractionalPosition = 0;

		nYM3526Position = nExtraSamples;

	}
}

// ----------------------------------------------------------------------------
// Callbacks for YM3526 core

void BurnYM3526UpdateRequest(int, int)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM3526Initted) bprintf(PRINT_ERROR, _T("BurnYM3526UpdateRequest called without init\n"));
#endif

	YM3526Render(BurnYM3526StreamCallback(nBurnYM3526SoundRate));
}

// ----------------------------------------------------------------------------
// Initialisation, etc.

void BurnYM3526Reset()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM3526Initted) bprintf(PRINT_ERROR, _T("BurnYM3526Reset called without init\n"));
#endif

	BurnTimerResetYM3526();

	YM3526ResetChip(0);
}

void BurnYM3526Exit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM3526Initted) bprintf(PRINT_ERROR, _T("BurnYM3526Exit called without init\n"));
#endif

	YM3526Shutdown();

	BurnTimerExitYM3526();

	if (pBuffer) {
		free(pBuffer);
		pBuffer = NULL;
	}
	
	bYM3526AddSignal = 0;
	
	DebugSnd_YM3526Initted = 0;
}

INT32 BurnYM3526Init(INT32 nClockFrequency, OPL_IRQHANDLER IRQCallback, INT32 (*StreamCallback)(INT32), INT32 bAddSignal)
{
	DebugSnd_YM3526Initted = 1;
	
	BurnTimerInitYM3526(&YM3526TimerOver, NULL);

	if (nBurnSoundRate <= 0) {
		BurnYM3526StreamCallback = YM3526StreamCallbackDummy;

		BurnYM3526Update = YM3526UpdateDummy;

		YM3526Init(1, nClockFrequency, 11025);
		return 0;
	}

	BurnYM3526StreamCallback = StreamCallback;

	if (nFMInterpolation == 3) {
		// Set YM3526 core samplerate to match the hardware
		nBurnYM3526SoundRate = nClockFrequency / 72;
		// Bring YM3526 core samplerate within usable range
		while (nBurnYM3526SoundRate > nBurnSoundRate * 3) {
			nBurnYM3526SoundRate >>= 1;
		}

		BurnYM3526Update = YM3526UpdateResample;

		nSampleSize = (UINT32)nBurnYM3526SoundRate * (1 << 16) / nBurnSoundRate;
		nFractionalPosition = 0;
	} else {
		nBurnYM3526SoundRate = nBurnSoundRate;

		BurnYM3526Update = YM3526UpdateNormal;
	}

	YM3526Init(1, nClockFrequency, nBurnYM3526SoundRate);
	YM3526SetIRQHandler(0, IRQCallback, 0);
	YM3526SetTimerHandler(0, &BurnOPLTimerCallbackYM3526, 0);
	YM3526SetUpdateHandler(0, &BurnYM3526UpdateRequest, 0);

	pBuffer = (INT16*)malloc(4096 * sizeof(INT16));
	memset(pBuffer, 0, 4096 * sizeof(INT16));

	nYM3526Position = 0;

	nFractionalPosition = 0;
	
	bYM3526AddSignal = bAddSignal;
	
	// default routes
	YM3526Volumes[BURN_SND_YM3526_ROUTE] = 1.00;
	YM3526RouteDirs[BURN_SND_YM3526_ROUTE] = BURN_SND_ROUTE_BOTH;

	return 0;
}

void BurnYM3526SetRoute(INT32 nIndex, double nVolume, INT32 nRouteDir)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM3526Initted) bprintf(PRINT_ERROR, _T("BurnYM3526SetRoute called without init\n"));
	if (nIndex < 0 || nIndex > 1) bprintf(PRINT_ERROR, _T("BurnYM3526SetRoute called with invalid index %i\n"), nIndex);
#endif
	
	YM3526Volumes[nIndex] = nVolume;
	YM3526RouteDirs[nIndex] = nRouteDir;
}

void BurnYM3526Scan(INT32 nAction, INT32* pnMin)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM3526Initted) bprintf(PRINT_ERROR, _T("BurnYM3526Scan called without init\n"));
#endif

	BurnTimerScanYM3526(nAction, pnMin);
	FMOPLScan(FM_OPL_SAVESTATE_YM3526, 0, nAction, pnMin);
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(nYM3526Position);
	}
}
