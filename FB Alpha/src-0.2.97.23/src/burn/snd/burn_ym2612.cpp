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

static INT32 nFractionalPosition;

static INT32 nNumChips = 0;
static INT32 bYM2612AddSignal;

// ----------------------------------------------------------------------------
// Dummy functions

static void YM2612UpdateDummy(INT16*, INT32 /* nSegmentEnd */)
{
	return;
}

static INT32 YM2612StreamCallbackDummy(INT32 /* nSoundRate */)
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

//	bprintf(PRINT_NORMAL, _T("    YM2612 render %6i -> %6i\n", nYM2612Position, nSegmentLength));

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
static void YM2612UpdateNormal(INT16* pSoundBuf, INT32 nSegmentEnd)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2612Initted) bprintf(PRINT_ERROR, _T("YM2612UpdateNormal called without init\n"));
#endif

	INT32 nSegmentLength = nSegmentEnd;
	INT32 i;

//	bprintf(PRINT_NORMAL, _T("    YM2612 update        -> %6i\n", nSegmentLength));

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
		INT32 nTotalSample;

		nTotalSample = pYM2612Buffer[0][n];
		if (nNumChips > 1) nTotalSample += pYM2612Buffer[2][n];
		if (nTotalSample < -32768) {
			nTotalSample = -32768;
		} else {
			if (nTotalSample > 32767) {
				nTotalSample = 32767;
			}
		}
		
		if (bYM2612AddSignal) {
			pSoundBuf[(n << 1) + 0] += nTotalSample;
		} else {
			pSoundBuf[(n << 1) + 0] = nTotalSample;
		}
		
		nTotalSample = pYM2612Buffer[1][n];
		if (nNumChips > 1) nTotalSample += pYM2612Buffer[3][n];
		if (nTotalSample < -32768) {
			nTotalSample = -32768;
		} else {
			if (nTotalSample > 32767) {
				nTotalSample = 32767;
			}
		}
		
		if (bYM2612AddSignal) {
			pSoundBuf[(n << 1) + 1] += nTotalSample;
		} else {
			pSoundBuf[(n << 1) + 1] = nTotalSample;
		}
	}


	nFractionalPosition = nSegmentLength;

	if (nSegmentEnd >= nBurnSoundLen) {
		int nExtraSamples = nSegmentEnd - nBurnSoundLen;

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

	nBurnYM2612SoundRate = nBurnSoundRate;
	BurnYM2612Update = YM2612UpdateNormal;
	
	YM2612Init(num, nClockFrequency, nBurnYM2612SoundRate, &BurnOPNTimerCallback, IRQCallback);

	pBuffer = (INT16*)malloc(4096 * 2 * num * sizeof(INT16));
	memset(pBuffer, 0, 4096 * 2 * num * sizeof(INT16));
	
	nYM2612Position = 0;
	nFractionalPosition = 0;
	
	nNumChips = num;
	bYM2612AddSignal = bAddSignal;

	return 0;
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
