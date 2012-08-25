#include "burnint.h"
#include "burn_sound.h"
#include "burn_y8950.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "m6809_intf.h"
#include "hd6309_intf.h"
#include "m6800_intf.h"
#include "m6502_intf.h"

// Timer Related

#define MAX_TIMER_VALUE ((1 << 30) - 65536)

static double dTimeY8950;									// Time elapsed since the emulated machine was started

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

extern "C" double BurnTimerGetTimeY8950()
{
	return dTimeY8950 + pTimerTimeCallback();
}

// ---------------------------------------------------------------------------
// Update timers

static INT32 nTicksTotal, nTicksDone, nTicksExtra;

INT32 BurnTimerUpdateY8950(INT32 nCycles)
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

void BurnTimerEndFrameY8950(INT32 nCycles)
{
	INT32 nTicks = MAKE_TIMER_TICKS(nCycles, nCPUClockspeed);

	BurnTimerUpdateY8950(nCycles);

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

void BurnTimerUpdateEndY8950()
{
	pCPURunEnd();

	nTicksTotal = 0;
}

void BurnOPLTimerCallbackY8950(INT32 c, double period)
{
	pCPURunEnd();

	if (period == 0.0) {
		nTimerCount[c] = MAX_TIMER_VALUE;
		return;
	}

	nTimerCount[c]  = (INT32)(period * (double)TIMER_TICKS_PER_SECOND);
	nTimerCount[c] += MAKE_TIMER_TICKS(pCPUTotalCycles(), nCPUClockspeed);
}

void BurnTimerScanY8950(INT32 nAction, INT32* pnMin)
{
	if (pnMin && *pnMin < 0x029521) {
		*pnMin = 0x029521;
	}

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(nTimerCount);
		SCAN_VAR(nTimerStart);
		SCAN_VAR(dTimeY8950);

		SCAN_VAR(nTicksDone);
	}
}

void BurnTimerExitY8950()
{
	nCPUClockspeed = 0;
	pCPUTotalCycles = NULL;
	pCPURun = NULL;
	pCPURunEnd = NULL;

	return;
}

void BurnTimerResetY8950()
{
	nTimerCount[0] = nTimerCount[1] = MAX_TIMER_VALUE;
	nTimerStart[0] = nTimerStart[1] = MAX_TIMER_VALUE;

	dTimeY8950 = 0.0;

	nTicksDone = 0;
}

INT32 BurnTimerInitY8950(INT32 (*pOverCallback)(INT32, INT32), double (*pTimeCallback)())
{
	BurnTimerExitY8950();

	pTimerOverCallback = pOverCallback;
	pTimerTimeCallback = pTimeCallback ? pTimeCallback : BurnTimerTimeCallbackDummy;

	BurnTimerResetY8950();

	return 0;
}

INT32 BurnTimerAttachSekY8950(INT32 nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = SekTotalCycles;
	pCPURun = SekRun;
	pCPURunEnd = SekRunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

	return 0;
}

INT32 BurnTimerAttachZetY8950(INT32 nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = ZetTotalCycles;
	pCPURun = ZetRun;
	pCPURunEnd = ZetRunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

	return 0;
}

INT32 BurnTimerAttachM6809Y8950(INT32 nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = M6809TotalCycles;
	pCPURun = M6809Run;
	pCPURunEnd = M6809RunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

	return 0;
}

INT32 BurnTimerAttachHD6309Y8950(INT32 nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = HD6309TotalCycles;
	pCPURun = HD6309Run;
	pCPURunEnd = HD6309RunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

	return 0;
}

INT32 BurnTimerAttachM6800Y8950(INT32 nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = M6800TotalCycles;
	pCPURun = M6800Run;
	pCPURunEnd = M6800RunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

	return 0;
}

INT32 BurnTimerAttachHD63701Y8950(INT32 nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = M6800TotalCycles;
	pCPURun = HD63701Run;
	pCPURunEnd = HD63701RunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

	return 0;
}

INT32 BurnTimerAttachM6803Y8950(INT32 nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = M6800TotalCycles;
	pCPURun = M6803Run;
	pCPURunEnd = M6803RunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

	return 0;
}

INT32 BurnTimerAttachM6502Y8950(INT32 nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = M6502TotalCycles;
	pCPURun = M6502Run;
	pCPURunEnd = M6502RunEnd; // doesn't do anything...

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

	return 0;
}

// Sound Related

#define MAX_Y8950	2

void (*BurnY8950Update)(INT16* pSoundBuf, INT32 nSegmentEnd);

static INT32 (*BurnY8950StreamCallback)(INT32 nSoundRate);

static INT32 nBurnY8950SoundRate;

static INT16* pBuffer;
static INT16* pY8950Buffer[MAX_Y8950];

static INT32 nY8950Position;

static UINT32 nSampleSize;
static INT32 nFractionalPosition;

static INT32 nNumChips = 0;

static INT32 bY8950AddSignal;

static double Y8950Volumes[1 * MAX_Y8950];
static INT32 Y8950RouteDirs[1 * MAX_Y8950];

// ----------------------------------------------------------------------------
// Dummy functions

static void Y8950UpdateDummy(INT16* , INT32)
{
	return;
}

static int Y8950StreamCallbackDummy(INT32)
{
	return 0;
}

// ----------------------------------------------------------------------------
// Execute Y8950 for part of a frame

static void Y8950Render(INT32 nSegmentLength)
{
#if defined FBA_DEBUG
	if (!DebugSnd_Y8950Initted) bprintf(PRINT_ERROR, _T("Y8950Render called without init\n"));
#endif

	if (nY8950Position >= nSegmentLength) {
		return;
	}

	nSegmentLength -= nY8950Position;

	Y8950UpdateOne(0, pBuffer + 0 * 4096 + 4 + nY8950Position, nSegmentLength);
	
	if (nNumChips > 1) {
		Y8950UpdateOne(1, pBuffer + 1 * 4096 + 4 + nY8950Position, nSegmentLength);
	}

	nY8950Position += nSegmentLength;
}

// ----------------------------------------------------------------------------
// Update the sound buffer

static void Y8950UpdateResample(INT16* pSoundBuf, INT32 nSegmentEnd)
{
#if defined FBA_DEBUG
	if (!DebugSnd_Y8950Initted) bprintf(PRINT_ERROR, _T("Y8950UpdateResample called without init\n"));
#endif

	INT32 nSegmentLength = nSegmentEnd;
	INT32 nSamplesNeeded = nSegmentEnd * nBurnY8950SoundRate / nBurnSoundRate + 1;


	if (nSamplesNeeded < nY8950Position) {
		nSamplesNeeded = nY8950Position;
	}

	if (nSegmentLength > nBurnSoundLen) {
		nSegmentLength = nBurnSoundLen;
	}
	nSegmentLength <<= 1;

	Y8950Render(nSamplesNeeded);
	
	pY8950Buffer[0] = pBuffer + 0 * 4096 + 4;
	
	if (nNumChips > 1) {
		pY8950Buffer[1] = pBuffer + 1 * 4096 + 4;
	}
	
	for (INT32 i = (nFractionalPosition & 0xFFFF0000) >> 15; i < nSegmentLength; i += 2, nFractionalPosition += nSampleSize) {
		INT32 nLeftSample[4] = {0, 0, 0, 0};
		INT32 nRightSample[4] = {0, 0, 0, 0};
		INT32 nTotalLeftSample, nTotalRightSample;
		
		if ((Y8950RouteDirs[BURN_SND_Y8950_ROUTE] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample[0] += (INT32)(pY8950Buffer[0][(nFractionalPosition >> 16) - 3] * Y8950Volumes[BURN_SND_Y8950_ROUTE]);
			nLeftSample[1] += (INT32)(pY8950Buffer[0][(nFractionalPosition >> 16) - 2] * Y8950Volumes[BURN_SND_Y8950_ROUTE]);
			nLeftSample[2] += (INT32)(pY8950Buffer[0][(nFractionalPosition >> 16) - 1] * Y8950Volumes[BURN_SND_Y8950_ROUTE]);
			nLeftSample[3] += (INT32)(pY8950Buffer[0][(nFractionalPosition >> 16) - 0] * Y8950Volumes[BURN_SND_Y8950_ROUTE]);
		}
		if ((Y8950RouteDirs[BURN_SND_Y8950_ROUTE] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample[0] += (INT32)(pY8950Buffer[0][(nFractionalPosition >> 16) - 3] * Y8950Volumes[BURN_SND_Y8950_ROUTE]);
			nRightSample[1] += (INT32)(pY8950Buffer[0][(nFractionalPosition >> 16) - 2] * Y8950Volumes[BURN_SND_Y8950_ROUTE]);
			nRightSample[2] += (INT32)(pY8950Buffer[0][(nFractionalPosition >> 16) - 1] * Y8950Volumes[BURN_SND_Y8950_ROUTE]);
			nRightSample[3] += (INT32)(pY8950Buffer[0][(nFractionalPosition >> 16) - 0] * Y8950Volumes[BURN_SND_Y8950_ROUTE]);
		}
		
		if (nNumChips > 1) {
			if ((Y8950RouteDirs[1 + BURN_SND_Y8950_ROUTE] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
				nLeftSample[0] += (INT32)(pY8950Buffer[1][(nFractionalPosition >> 16) - 3] * Y8950Volumes[1 + BURN_SND_Y8950_ROUTE]);
				nLeftSample[1] += (INT32)(pY8950Buffer[1][(nFractionalPosition >> 16) - 2] * Y8950Volumes[1 + BURN_SND_Y8950_ROUTE]);
				nLeftSample[2] += (INT32)(pY8950Buffer[1][(nFractionalPosition >> 16) - 1] * Y8950Volumes[1 + BURN_SND_Y8950_ROUTE]);
				nLeftSample[3] += (INT32)(pY8950Buffer[1][(nFractionalPosition >> 16) - 0] * Y8950Volumes[1 + BURN_SND_Y8950_ROUTE]);
			}
			if ((Y8950RouteDirs[1 + BURN_SND_Y8950_ROUTE] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
				nRightSample[0] += (INT32)(pY8950Buffer[1][(nFractionalPosition >> 16) - 3] * Y8950Volumes[1 + BURN_SND_Y8950_ROUTE]);
				nRightSample[1] += (INT32)(pY8950Buffer[1][(nFractionalPosition >> 16) - 2] * Y8950Volumes[1 + BURN_SND_Y8950_ROUTE]);
				nRightSample[2] += (INT32)(pY8950Buffer[1][(nFractionalPosition >> 16) - 1] * Y8950Volumes[1 + BURN_SND_Y8950_ROUTE]);
				nRightSample[3] += (INT32)(pY8950Buffer[1][(nFractionalPosition >> 16) - 0] * Y8950Volumes[1 + BURN_SND_Y8950_ROUTE]);
			}
		}
		
		nTotalLeftSample = INTERPOLATE4PS_16BIT((nFractionalPosition >> 4) & 0x0fff, nLeftSample[0], nLeftSample[1], nLeftSample[2], nLeftSample[3]);
		nTotalRightSample = INTERPOLATE4PS_16BIT((nFractionalPosition >> 4) & 0x0fff, nRightSample[0], nRightSample[1], nRightSample[2], nRightSample[3]);
		
		nTotalLeftSample = BURN_SND_CLIP(nTotalLeftSample);
		nTotalRightSample = BURN_SND_CLIP(nTotalRightSample);
			
		if (bY8950AddSignal) {
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
			pY8950Buffer[0][i] = pY8950Buffer[0][(nFractionalPosition >> 16) + i];
			
			if (nNumChips > 1) {
				pY8950Buffer[1][i] = pY8950Buffer[1][(nFractionalPosition >> 16) + i];
			}
		}

		nFractionalPosition &= 0xFFFF;

		nY8950Position = nExtraSamples;
	}
}

static void Y8950UpdateNormal(INT16* pSoundBuf, INT32 nSegmentEnd)
{
#if defined FBA_DEBUG
	if (!DebugSnd_Y8950Initted) bprintf(PRINT_ERROR, _T("Y8950UpdateNormal called without init\n"));
#endif

	INT32 nSegmentLength = nSegmentEnd;

	if (nSegmentEnd < nY8950Position) {
		nSegmentEnd = nY8950Position;
	}

	if (nSegmentLength > nBurnSoundLen) {
		nSegmentLength = nBurnSoundLen;
	}

	Y8950Render(nSegmentEnd);

	pY8950Buffer[0] = pBuffer + 4 + 0 * 4096;
	
	if (nNumChips > 1) {
		pY8950Buffer[1] = pBuffer + 4 + 1 * 4096;
	}

	for (INT32 n = nFractionalPosition; n < nSegmentLength; n++) {
		INT32 nLeftSample = 0, nRightSample = 0;
		
		if ((Y8950RouteDirs[BURN_SND_Y8950_ROUTE] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(pY8950Buffer[0][n] * Y8950Volumes[BURN_SND_Y8950_ROUTE]);
		}
		if ((Y8950RouteDirs[BURN_SND_Y8950_ROUTE] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(pY8950Buffer[0][n] * Y8950Volumes[BURN_SND_Y8950_ROUTE]);
		}
		
		if (nNumChips > 1) {
			if ((Y8950RouteDirs[1 + BURN_SND_Y8950_ROUTE] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
				nLeftSample += (INT32)(pY8950Buffer[1][n] * Y8950Volumes[1 + BURN_SND_Y8950_ROUTE]);
			}
			if ((Y8950RouteDirs[1 + BURN_SND_Y8950_ROUTE] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
				nRightSample += (INT32)(pY8950Buffer[1][n] * Y8950Volumes[1 + BURN_SND_Y8950_ROUTE]);
			}
		}
		
		nLeftSample = BURN_SND_CLIP(nLeftSample);
		nRightSample = BURN_SND_CLIP(nRightSample);
			
		if (bY8950AddSignal) {
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
			pY8950Buffer[i] = pY8950Buffer[nBurnSoundLen + i];
		}

		nFractionalPosition = 0;

		nY8950Position = nExtraSamples;

	}
}

// ----------------------------------------------------------------------------
// Callbacks for Y8950 core

void BurnY8950UpdateRequest(INT32, INT32)
{
#if defined FBA_DEBUG
	if (!DebugSnd_Y8950Initted) bprintf(PRINT_ERROR, _T("BurnY8950UpdateRequest called without init\n"));
#endif

	Y8950Render(BurnY8950StreamCallback(nBurnY8950SoundRate));
}

// ----------------------------------------------------------------------------
// Initialisation, etc.

void BurnY8950Reset()
{
#if defined FBA_DEBUG
	if (!DebugSnd_Y8950Initted) bprintf(PRINT_ERROR, _T("BurnY8950Reset called without init\n"));
#endif

	BurnTimerResetY8950();

	for (INT32 i = 0; i < nNumChips; i++) {
		Y8950ResetChip(i);
	}
}

void BurnY8950Exit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_Y8950Initted) bprintf(PRINT_ERROR, _T("BurnY8950Exit called without init\n"));
#endif

	Y8950Shutdown();

	BurnTimerExitY8950();

	BurnFree(pBuffer);
	
	nNumChips = 0;
	bY8950AddSignal = 0;
	
	DebugSnd_Y8950Initted = 0;
}

INT32 BurnY8950Init(INT32 num, INT32 nClockFrequency, UINT8* Y8950ADPCM0ROM, INT32 nY8950ADPCM0Size, UINT8* Y8950ADPCM1ROM, INT32 nY8950ADPCM1Size, OPL_IRQHANDLER IRQCallback, INT32 (*StreamCallback)(INT32), INT32 bAddSignal)
{
	BurnTimerInitY8950(&Y8950TimerOver, NULL);

	if (nBurnSoundRate <= 0) {
		BurnY8950StreamCallback = Y8950StreamCallbackDummy;

		BurnY8950Update = Y8950UpdateDummy;

		Y8950Init(num, nClockFrequency, 11025);
		return 0;
	}

	BurnY8950StreamCallback = StreamCallback;

	if (nFMInterpolation == 3) {
		// Set Y8950 core samplerate to match the hardware
		nBurnY8950SoundRate = nClockFrequency / 72;
		// Bring Y8950 core samplerate within usable range
		while (nBurnY8950SoundRate > nBurnSoundRate * 3) {
			nBurnY8950SoundRate >>= 1;
		}

		BurnY8950Update = Y8950UpdateResample;

		nSampleSize = (UINT32)nBurnY8950SoundRate * (1 << 16) / nBurnSoundRate;
		nFractionalPosition = 0;
	} else {
		nBurnY8950SoundRate = nBurnSoundRate;

		BurnY8950Update = Y8950UpdateNormal;
	}

	Y8950Init(num, nClockFrequency, nBurnY8950SoundRate);
	Y8950SetIRQHandler(0, IRQCallback, 0);
	Y8950SetTimerHandler(0, &BurnOPLTimerCallbackY8950, 0);
	Y8950SetUpdateHandler(0, &BurnY8950UpdateRequest, 0);
	Y8950SetDeltaTMemory(0, Y8950ADPCM0ROM, nY8950ADPCM0Size);
	if (num > 1) {
//		Y8950SetIRQHandler(1, IRQCallback, 0); // ??
		Y8950SetTimerHandler(1, &BurnOPLTimerCallbackY8950, 0);
		Y8950SetUpdateHandler(1, &BurnY8950UpdateRequest, 0);
		Y8950SetDeltaTMemory(1, Y8950ADPCM1ROM, nY8950ADPCM1Size);
	}

	pBuffer = (INT16*)BurnMalloc(4096 * num * sizeof(INT16));
	memset(pBuffer, 0, 4096 * num * sizeof(INT16));

	nY8950Position = 0;

	nFractionalPosition = 0;
	
	nNumChips = num;
	bY8950AddSignal = bAddSignal;
	
	// default routes
	Y8950Volumes[BURN_SND_Y8950_ROUTE] = 1.00;
	Y8950RouteDirs[BURN_SND_Y8950_ROUTE] = BURN_SND_ROUTE_BOTH;
	if (nNumChips > 1) {
		Y8950Volumes[1 + BURN_SND_Y8950_ROUTE] = 1.00;
		Y8950RouteDirs[1 + BURN_SND_Y8950_ROUTE] = BURN_SND_ROUTE_BOTH;
	}
	
	DebugSnd_Y8950Initted = 1;

	return 0;
}

void BurnY8950SetRoute(INT32 nChip, INT32 nIndex, double nVolume, INT32 nRouteDir)
{
#if defined FBA_DEBUG
	if (!DebugSnd_Y8950Initted) bprintf(PRINT_ERROR, _T("BurnY8950SetRoute called without init\n"));
	if (nIndex < 0 || nIndex > 1) bprintf(PRINT_ERROR, _T("BurnY8950SetRoute called with invalid index %i\n"), nIndex);
	if (nChip >= nNumChips) bprintf(PRINT_ERROR, _T("BurnY8950SetRoute called with invalid chip %i\n"), nChip);
#endif
	
	if (nChip == 0) {
		Y8950Volumes[nIndex] = nVolume;
		Y8950RouteDirs[nIndex] = nRouteDir;
	}
	
	if (nChip == 1) {
		Y8950Volumes[1 + nIndex] = nVolume;
		Y8950RouteDirs[1 + nIndex] = nRouteDir;
	}
}

void BurnY8950Scan(INT32 nAction, INT32* pnMin)
{
	#if defined FBA_DEBUG
	if (!DebugSnd_Y8950Initted) bprintf(PRINT_ERROR, _T("BurnY8950Scan called without init\n"));
#endif
	
	BurnTimerScanY8950(nAction, pnMin);
	FMOPLScan(FM_OPL_SAVESTATE_Y8950, 0, nAction, pnMin);
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(nY8950Position);
	}
}

#undef MAX_Y8950
