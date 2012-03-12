#include "burnint.h"
#include "burn_sound.h"
#include "burn_ym2203.h"

#define MAX_YM2203	2

void (*BurnYM2203Update)(INT16* pSoundBuf, INT32 nSegmentEnd);

static INT32 (*BurnYM2203StreamCallback)(INT32 nSoundRate);

static INT32 nBurnYM2203SoundRate;

static INT16* pBuffer;
static INT16* pYM2203Buffer[5 * MAX_YM2203];

static INT32 nYM2203Position;
static INT32 nAY8910Position;

static UINT32 nSampleSize;
static INT32 nFractionalPosition;

static INT32 nNumChips = 0;

static INT32 bYM2203AddSignal;
static INT32 nYM2203VolumeShift;

// ----------------------------------------------------------------------------
// Dummy functions

static void YM2203UpdateDummy(INT16*, INT32 /* nSegmentEnd */)
{
	return;
}

static INT32 YM2203StreamCallbackDummy(INT32 /* nSoundRate */)
{
	return 0;
}

// ----------------------------------------------------------------------------
// Execute YM2203 for part of a frame

static void AY8910Render(INT32 nSegmentLength)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2203Initted) bprintf(PRINT_ERROR, _T("BurnYM2203 AY8910Render called without init\n"));
#endif

	if (nAY8910Position >= nSegmentLength) {
		return;
	}

//	bprintf(PRINT_NORMAL, _T("    AY8910 render %6i -> %6i\n"), nAY8910Position, nSegmentLength);

	nSegmentLength -= nAY8910Position;

	pYM2203Buffer[1] = pBuffer + 1 * 4096 + 4 + nAY8910Position;
	pYM2203Buffer[2] = pBuffer + 2 * 4096 + 4 + nAY8910Position;
	pYM2203Buffer[3] = pBuffer + 3 * 4096 + 4 + nAY8910Position;

	AY8910Update(0, &pYM2203Buffer[1], nSegmentLength);
	
	if (nNumChips > 1) {
		pYM2203Buffer[6] = pBuffer + 6 * 4096 + 4 + nAY8910Position;
		pYM2203Buffer[7] = pBuffer + 7 * 4096 + 4 + nAY8910Position;
		pYM2203Buffer[8] = pBuffer + 8 * 4096 + 4 + nAY8910Position;

		AY8910Update(1, &pYM2203Buffer[6], nSegmentLength);
	}

	nAY8910Position += nSegmentLength;
}

static void YM2203Render(INT32 nSegmentLength)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2203Initted) bprintf(PRINT_ERROR, _T("YM2203Render called without init\n"));
#endif

	if (nYM2203Position >= nSegmentLength) {
		return;
	}

//	bprintf(PRINT_NORMAL, _T("    YM2203 render %6i -> %6i\n", nYM2203Position, nSegmentLength));

	nSegmentLength -= nYM2203Position;

	pYM2203Buffer[0] = pBuffer + 0 * 4096 + 4 + nYM2203Position;

	YM2203UpdateOne(0, pYM2203Buffer[0], nSegmentLength);
	
	if (nNumChips > 1) {
		pYM2203Buffer[5] = pBuffer + 5 * 4096 + 4 + nYM2203Position;

		YM2203UpdateOne(1, pYM2203Buffer[5], nSegmentLength);
	}

	nYM2203Position += nSegmentLength;
}

// ----------------------------------------------------------------------------
// Update the sound buffer

static void YM2203UpdateResample(INT16* pSoundBuf, INT32 nSegmentEnd)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2203Initted) bprintf(PRINT_ERROR, _T("YM2203UpdateResample called without init\n"));
#endif

	INT32 nSegmentLength = nSegmentEnd;
	INT32 nSamplesNeeded = nSegmentEnd * nBurnYM2203SoundRate / nBurnSoundRate + 1;

//	bprintf(PRINT_NORMAL, _T("    YM2203 update        -> %6i\n", nSegmentLength));

	if (nSamplesNeeded < nAY8910Position) {
		nSamplesNeeded = nAY8910Position;
	}
	if (nSamplesNeeded < nYM2203Position) {
		nSamplesNeeded = nYM2203Position;
	}

	if (nSegmentLength > nBurnSoundLen) {
		nSegmentLength = nBurnSoundLen;
	}
	nSegmentLength <<= 1;

	YM2203Render(nSamplesNeeded);
	AY8910Render(nSamplesNeeded);

	pYM2203Buffer[0] = pBuffer + 0 * 4096 + 4;
	pYM2203Buffer[1] = pBuffer + 1 * 4096 + 4;
	pYM2203Buffer[2] = pBuffer + 2 * 4096 + 4;
	pYM2203Buffer[3] = pBuffer + 3 * 4096 + 4;
	pYM2203Buffer[4] = pBuffer + 4 * 4096 + 4;
	
	if (nNumChips > 1) {
		pYM2203Buffer[5] = pBuffer + 5 * 4096 + 4;
		pYM2203Buffer[6] = pBuffer + 6 * 4096 + 4;
		pYM2203Buffer[7] = pBuffer + 7 * 4096 + 4;
		pYM2203Buffer[8] = pBuffer + 8 * 4096 + 4;
		pYM2203Buffer[9] = pBuffer + 9 * 4096 + 4;
	}

	for (INT32 i = (nFractionalPosition >> 16) - 4; i < nSamplesNeeded; i++) {
		pYM2203Buffer[4][i] = (((INT32)pYM2203Buffer[1][i] + pYM2203Buffer[2][i] + pYM2203Buffer[3][i]) * (4096 * 60 / 100)) >> 12;
		if (nNumChips > 1) pYM2203Buffer[9][i] = (((INT32)pYM2203Buffer[6][i] + pYM2203Buffer[7][i] + pYM2203Buffer[8][i]) * (4096 * 60 / 100)) >> 12;
	}

	for (INT32 i = (nFractionalPosition & 0xFFFF0000) >> 15; i < nSegmentLength; i += 2, nFractionalPosition += nSampleSize) {
		INT32 nSample, nSample2 = 0;

#define CLIP(A) ((A) < -0x8000 ? -0x8000 : (A) > 0x7fff ? 0x7fff : (A))

		
		nSample = INTERPOLATE4PS_16BIT((nFractionalPosition >> 4) & 0x0FFF,
									   (pYM2203Buffer[0][(nFractionalPosition >> 16) - 3] + pYM2203Buffer[4][(nFractionalPosition >> 16) - 3]) >> nYM2203VolumeShift,
									   (pYM2203Buffer[0][(nFractionalPosition >> 16) - 2] + pYM2203Buffer[4][(nFractionalPosition >> 16) - 2]) >> nYM2203VolumeShift,
									   (pYM2203Buffer[0][(nFractionalPosition >> 16) - 1] + pYM2203Buffer[4][(nFractionalPosition >> 16) - 1]) >> nYM2203VolumeShift,
									   (pYM2203Buffer[0][(nFractionalPosition >> 16) - 0] + pYM2203Buffer[4][(nFractionalPosition >> 16) - 0]) >> nYM2203VolumeShift);
		if (nNumChips > 1) nSample2 = INTERPOLATE4PS_16BIT((nFractionalPosition >> 4) & 0x0FFF,
									   (pYM2203Buffer[5][(nFractionalPosition >> 16) - 3] + pYM2203Buffer[9][(nFractionalPosition >> 16) - 3]) >> nYM2203VolumeShift,
									   (pYM2203Buffer[5][(nFractionalPosition >> 16) - 2] + pYM2203Buffer[9][(nFractionalPosition >> 16) - 2]) >> nYM2203VolumeShift,
									   (pYM2203Buffer[5][(nFractionalPosition >> 16) - 1] + pYM2203Buffer[9][(nFractionalPosition >> 16) - 1]) >> nYM2203VolumeShift,
									   (pYM2203Buffer[5][(nFractionalPosition >> 16) - 0] + pYM2203Buffer[9][(nFractionalPosition >> 16) - 0]) >> nYM2203VolumeShift);
									   
		nSample = CLIP(nSample);
		nSample2 = CLIP(nSample2);
		
		if (bYM2203AddSignal) {
			pSoundBuf[i + 0] += CLIP(nSample + nSample2);
			pSoundBuf[i + 1] += CLIP(nSample + nSample2);
		} else {
			pSoundBuf[i + 0] = CLIP(nSample + nSample2);
			pSoundBuf[i + 1] = CLIP(nSample + nSample2);
		}
		
#undef CLIP

	}

	if (nSegmentEnd >= nBurnSoundLen) {
		INT32 nExtraSamples = nSamplesNeeded - (nFractionalPosition >> 16);

//		bprintf(PRINT_NORMAL, _T("   %6i rendered, %i extra, %i <- %i\n"), nSamplesNeeded, nExtraSamples, nExtraSamples, (nFractionalPosition >> 16) + nExtraSamples - 1);

		for (INT32 i = -4; i < nExtraSamples; i++) {
			pYM2203Buffer[0][i] = pYM2203Buffer[0][(nFractionalPosition >> 16) + i];
			pYM2203Buffer[1][i] = pYM2203Buffer[1][(nFractionalPosition >> 16) + i];
			pYM2203Buffer[2][i] = pYM2203Buffer[2][(nFractionalPosition >> 16) + i];
			pYM2203Buffer[3][i] = pYM2203Buffer[3][(nFractionalPosition >> 16) + i];
			
			if (nNumChips > 1) {
				pYM2203Buffer[5][i] = pYM2203Buffer[5][(nFractionalPosition >> 16) + i];
				pYM2203Buffer[6][i] = pYM2203Buffer[6][(nFractionalPosition >> 16) + i];
				pYM2203Buffer[7][i] = pYM2203Buffer[7][(nFractionalPosition >> 16) + i];
				pYM2203Buffer[8][i] = pYM2203Buffer[8][(nFractionalPosition >> 16) + i];
			}
		}

		nFractionalPosition &= 0xFFFF;

		nYM2203Position = nExtraSamples;
		nAY8910Position = nExtraSamples;

		dTime += 100.0 / nBurnFPS;
	}
}

static void YM2203UpdateNormal(INT16* pSoundBuf, INT32 nSegmentEnd)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2203Initted) bprintf(PRINT_ERROR, _T("YM2203UpdateNormal called without init\n"));
#endif

	INT32 nSegmentLength = nSegmentEnd;
	INT32 i;

//	bprintf(PRINT_NORMAL, _T("    YM2203 update        -> %6i\n", nSegmentLength));

	if (nSegmentEnd < nAY8910Position) {
		nSegmentEnd = nAY8910Position;
	}
	if (nSegmentEnd < nYM2203Position) {
		nSegmentEnd = nYM2203Position;
	}

	if (nSegmentLength > nBurnSoundLen) {
		nSegmentLength = nBurnSoundLen;
	}

	YM2203Render(nSegmentEnd);
	AY8910Render(nSegmentEnd);

	pYM2203Buffer[0] = pBuffer + 4 + 0 * 4096;
	pYM2203Buffer[1] = pBuffer + 4 + 1 * 4096;
	pYM2203Buffer[2] = pBuffer + 4 + 2 * 4096;
	pYM2203Buffer[3] = pBuffer + 4 + 3 * 4096;
	if (nNumChips > 1) {
		pYM2203Buffer[5] = pBuffer + 5 + 4 * 4096;
		pYM2203Buffer[6] = pBuffer + 6 + 5 * 4096;
		pYM2203Buffer[7] = pBuffer + 7 + 6 * 4096;
		pYM2203Buffer[8] = pBuffer + 8 + 7 * 4096;
	}

	for (INT32 n = nFractionalPosition; n < nSegmentLength; n++) {
		INT32 nAYSample; INT32 nTotalSample;

		nAYSample  = pYM2203Buffer[1][n];
		nAYSample += pYM2203Buffer[2][n];
		nAYSample += pYM2203Buffer[3][n];
		if (nNumChips > 1) {
			nAYSample += pYM2203Buffer[6][n];
			nAYSample += pYM2203Buffer[7][n];
			nAYSample += pYM2203Buffer[8][n];
		}

		nAYSample  *= 4096 * 60 / 100;
		nAYSample >>= 12;
		
		nTotalSample = nAYSample + pYM2203Buffer[0][n];
		if (nNumChips > 1) nTotalSample += pYM2203Buffer[5][n];
		
		nTotalSample >>= nYM2203VolumeShift;
		
		if (nTotalSample < -32768) {
			nTotalSample = -32768;
		} else {
			if (nTotalSample > 32767) {
				nTotalSample = 32767;
			}
		}
		
		if (bYM2203AddSignal) {
			pSoundBuf[(n << 1) + 0] += nTotalSample;
			pSoundBuf[(n << 1) + 1] += nTotalSample;
		} else {
			pSoundBuf[(n << 1) + 0] = nTotalSample;
			pSoundBuf[(n << 1) + 1] = nTotalSample;
		}
	}

	nFractionalPosition = nSegmentLength;

	if (nSegmentEnd >= nBurnSoundLen) {
		INT32 nExtraSamples = nSegmentEnd - nBurnSoundLen;

		for (i = 0; i < nExtraSamples; i++) {
			pYM2203Buffer[0][i] = pYM2203Buffer[0][nBurnSoundLen + i];
			pYM2203Buffer[1][i] = pYM2203Buffer[1][nBurnSoundLen + i];
			pYM2203Buffer[2][i] = pYM2203Buffer[2][nBurnSoundLen + i];
			pYM2203Buffer[3][i] = pYM2203Buffer[3][nBurnSoundLen + i];
			if (nNumChips > 1) {
				pYM2203Buffer[5][i] = pYM2203Buffer[5][nBurnSoundLen + i];
				pYM2203Buffer[6][i] = pYM2203Buffer[6][nBurnSoundLen + i];
				pYM2203Buffer[7][i] = pYM2203Buffer[7][nBurnSoundLen + i];
				pYM2203Buffer[8][i] = pYM2203Buffer[8][nBurnSoundLen + i];
			}
		}

		nFractionalPosition = 0;

		nYM2203Position = nExtraSamples;
		nAY8910Position = nExtraSamples;

		dTime += 100.0 / nBurnFPS;
	}
}

// ----------------------------------------------------------------------------
// Callbacks for YM2203 core

void BurnYM2203UpdateRequest()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2203Initted) bprintf(PRINT_ERROR, _T("BurnYM2203UpdateRequest called without init\n"));
#endif

	YM2203Render(BurnYM2203StreamCallback(nBurnYM2203SoundRate));
}

static void BurnAY8910UpdateRequest()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2203Initted) bprintf(PRINT_ERROR, _T("BurnYM2203 BurnAY8910UpdateRequest called without init\n"));
#endif

	AY8910Render(BurnYM2203StreamCallback(nBurnYM2203SoundRate));
}

// ----------------------------------------------------------------------------
// Initialisation, etc.

void BurnYM2203Reset()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2203Initted) bprintf(PRINT_ERROR, _T("BurnYM2203Reset called without init\n"));
#endif

	BurnTimerReset();
	
	for (INT32 i = 0; i < nNumChips; i++) {
		YM2203ResetChip(i);
		AY8910Reset(i);
	}
}

void BurnYM2203Exit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2203Initted) bprintf(PRINT_ERROR, _T("BurnYM2203Exit called without init\n"));
#endif

	YM2203Shutdown();
	
	for (INT32 i = 0; i < nNumChips; i++) {
		AY8910Exit(i);
	}

	BurnTimerExit();

	if (pBuffer) {
		free(pBuffer);
		pBuffer = NULL;
	}
	
	nNumChips = 0;
	bYM2203AddSignal = 0;
	nYM2203VolumeShift = 0;
	
	DebugSnd_YM2203Initted = 0;
}

INT32 BurnYM2203Init(INT32 num, INT32 nClockFrequency, FM_IRQHANDLER IRQCallback, INT32 (*StreamCallback)(INT32), double (*GetTimeCallback)(), INT32 bAddSignal)
{
	DebugSnd_YM2203Initted = 1;
	
	if (num > MAX_YM2203) num = MAX_YM2203;
	
	BurnTimerInit(&YM2203TimerOver, GetTimeCallback);

	if (nBurnSoundRate <= 0) {
		BurnYM2203StreamCallback = YM2203StreamCallbackDummy;

		BurnYM2203Update = YM2203UpdateDummy;

		for (INT32 i = 0; i < num; i++) {
			AY8910InitYM(i, nClockFrequency, 11025, NULL, NULL, NULL, NULL, BurnAY8910UpdateRequest);
		}
		YM2203Init(num, nClockFrequency, 11025, &BurnOPNTimerCallback, IRQCallback);
		return 0;
	}

	BurnYM2203StreamCallback = StreamCallback;

	if (nFMInterpolation == 3) {
		// Set YM2203 core samplerate to match the hardware
		nBurnYM2203SoundRate = nClockFrequency / (144 * num);
		// Bring YM2203 core samplerate within usable range
		while (nBurnYM2203SoundRate > nBurnSoundRate * 3) {
			nBurnYM2203SoundRate >>= 1;
		}

		BurnYM2203Update = YM2203UpdateResample;

		nSampleSize = (UINT32)nBurnYM2203SoundRate * (1 << 16) / nBurnSoundRate;
	} else {
		nBurnYM2203SoundRate = nBurnSoundRate;

		BurnYM2203Update = YM2203UpdateNormal;
	}

	for (INT32 i = 0; i < num; i++) {
		AY8910InitYM(i, nClockFrequency, nBurnYM2203SoundRate, NULL, NULL, NULL, NULL, BurnAY8910UpdateRequest);
	}
	
	YM2203Init(num, nClockFrequency, nBurnYM2203SoundRate, &BurnOPNTimerCallback, IRQCallback);

	pBuffer = (INT16*)malloc(4096 * 5 * num * sizeof(INT16));
	memset(pBuffer, 0, 4096 * 5 * num * sizeof(INT16));

	nYM2203Position = 0;
	nAY8910Position = 0;
	nFractionalPosition = 0;
	
	nNumChips = num;
	bYM2203AddSignal = bAddSignal;

	return 0;
}

void BurnYM2203SetVolumeShift(INT32 Shift)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2203Initted) bprintf(PRINT_ERROR, _T("BurnYM2203SetVolumeShift called without init\n"));
#endif

	nYM2203VolumeShift = Shift;
}

void BurnYM2203Scan(INT32 nAction, INT32* pnMin)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2203Initted) bprintf(PRINT_ERROR, _T("BurnYM2203Scan called without init\n"));
#endif

	BurnTimerScan(nAction, pnMin);
	AY8910Scan(nAction, pnMin);

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(nYM2203Position);
		SCAN_VAR(nAY8910Position);
	}
}

#undef MAX_YM2203
