#include "burnint.h"
#include "burn_sound.h"
#include "burn_ym2610.h"

void (*BurnYM2610Update)(INT16* pSoundBuf, INT32 nSegmentEnd);

static INT32 (*BurnYM2610StreamCallback)(INT32 nSoundRate);

static INT32 nBurnYM2610SoundRate;

static INT16* pBuffer;
static INT16* pYM2610Buffer[6];

static INT32* pAYBuffer;

static INT32 nYM2610Position;
static INT32 nAY8910Position;

static UINT32 nSampleSize;
static INT32 nFractionalPosition;

static INT32 bYM2610AddSignal;
static INT32 bYM2610MixSound;

// ----------------------------------------------------------------------------
// Dummy functions

static void YM2610UpdateDummy(INT16*, INT32 /* nSegmentEnd */)
{
	return;
}

static INT32 YM2610StreamCallbackDummy(INT32 /* nSoundRate */)
{
	return 0;
}

// ----------------------------------------------------------------------------
// Execute YM2610 for part of a frame

static void AY8910Render(INT32 nSegmentLength)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2610Initted) bprintf(PRINT_ERROR, _T("BurnYM2610 AY8910Render called without init\n"));
#endif

	if (nAY8910Position >= nSegmentLength) {
		return;
	}

//	bprintf(PRINT_NORMAL, _T("    AY8910 render %6i -> %6i\n"), nAY8910Position, nSegmentLength);

	nSegmentLength -= nAY8910Position;

	pYM2610Buffer[2] = pBuffer + 2 * 4096 + 4 + nAY8910Position;
	pYM2610Buffer[3] = pBuffer + 3 * 4096 + 4 + nAY8910Position;
	pYM2610Buffer[4] = pBuffer + 4 * 4096 + 4 + nAY8910Position;

	AY8910Update(0, &pYM2610Buffer[2], nSegmentLength);

	nAY8910Position += nSegmentLength;
}

static void YM2610Render(INT32 nSegmentLength)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2610Initted) bprintf(PRINT_ERROR, _T("YM2610Render called without init\n"));
#endif

	if (nYM2610Position >= nSegmentLength) {
		return;
	}

//	bprintf(PRINT_NORMAL, _T("    YM2610 render %6i -> %6i\n", nYM2610Position, nSegmentLength));

	nSegmentLength -= nYM2610Position;

	pYM2610Buffer[0] = pBuffer + 0 * 4096 + 4 + nYM2610Position;
	pYM2610Buffer[1] = pBuffer + 1 * 4096 + 4 + nYM2610Position;

	YM2610UpdateOne(0, &pYM2610Buffer[0], nSegmentLength);

	nYM2610Position += nSegmentLength;
}

// ----------------------------------------------------------------------------
// Update the sound buffer

static void YM2610UpdateResample(INT16* pSoundBuf, INT32 nSegmentEnd)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2610Initted) bprintf(PRINT_ERROR, _T("YM2610UpdateResample called without init\n"));
#endif

	INT32 nSegmentLength = nSegmentEnd;
	INT32 nSamplesNeeded = nSegmentEnd * nBurnYM2610SoundRate / nBurnSoundRate + 1;

//	bprintf(PRINT_NORMAL, _T("    YM2610 update        -> %6i\n", nSegmentLength));

	if (nSamplesNeeded < nAY8910Position) {
		nSamplesNeeded = nAY8910Position;
	}
	if (nSamplesNeeded < nYM2610Position) {
		nSamplesNeeded = nYM2610Position;
	}

	if (nSegmentLength > nBurnSoundLen) {
		nSegmentLength = nBurnSoundLen;
	}
	nSegmentLength <<= 1;

	YM2610Render(nSamplesNeeded);
	AY8910Render(nSamplesNeeded);

	pYM2610Buffer[0] = pBuffer + 0 * 4096 + 4;
	pYM2610Buffer[1] = pBuffer + 1 * 4096 + 4;
	pYM2610Buffer[2] = pBuffer + 2 * 4096 + 4;
	pYM2610Buffer[3] = pBuffer + 3 * 4096 + 4;
	pYM2610Buffer[4] = pBuffer + 4 * 4096 + 4;
	pYM2610Buffer[5] = pBuffer + 5 * 4096 + 4;

	for (INT32 i = (nFractionalPosition >> 16) - 4; i < nSamplesNeeded; i++) {
		pYM2610Buffer[5][i] = (((INT32)pYM2610Buffer[2][i] + pYM2610Buffer[3][i] + pYM2610Buffer[4][i]) * (4096 * 60 / 100)) >> 12;
	}

	for (INT32 i = (nFractionalPosition & 0xFFFF0000) >> 15; i < nSegmentLength; i += 2, nFractionalPosition += nSampleSize) {
		INT32 nSample;

#define CLIP(A) ((A) < -0x8000 ? -0x8000 : (A) > 0x7fff ? 0x7fff : (A))

		if (bYM2610MixSound) {
			nSample = INTERPOLATE4PS_16BIT((nFractionalPosition >> 4) & 0x0FFF,
										   pYM2610Buffer[0][(nFractionalPosition >> 16) - 3] + pYM2610Buffer[5][(nFractionalPosition >> 16) - 3] + pYM2610Buffer[1][(nFractionalPosition >> 16) - 3],
										   pYM2610Buffer[0][(nFractionalPosition >> 16) - 2] + pYM2610Buffer[5][(nFractionalPosition >> 16) - 2] + pYM2610Buffer[1][(nFractionalPosition >> 16) - 1],
										   pYM2610Buffer[0][(nFractionalPosition >> 16) - 1] + pYM2610Buffer[5][(nFractionalPosition >> 16) - 1] + pYM2610Buffer[1][(nFractionalPosition >> 16) - 2],
										   pYM2610Buffer[0][(nFractionalPosition >> 16) - 0] + pYM2610Buffer[5][(nFractionalPosition >> 16) - 0] + pYM2610Buffer[1][(nFractionalPosition >> 16) - 0]);
										   
			if (bYM2610AddSignal) {
				pSoundBuf[i + 0] += CLIP(nSample);
				pSoundBuf[i + 1] += CLIP(nSample);
			} else {
				pSoundBuf[i + 0] = CLIP(nSample);
				pSoundBuf[i + 1] = CLIP(nSample);
			}
		} else {
			// Left channel
			nSample = INTERPOLATE4PS_16BIT((nFractionalPosition >> 4) & 0x0FFF,
										   pYM2610Buffer[0][(nFractionalPosition >> 16) - 3] + pYM2610Buffer[5][(nFractionalPosition >> 16) - 3],
										   pYM2610Buffer[0][(nFractionalPosition >> 16) - 2] + pYM2610Buffer[5][(nFractionalPosition >> 16) - 2],
										   pYM2610Buffer[0][(nFractionalPosition >> 16) - 1] + pYM2610Buffer[5][(nFractionalPosition >> 16) - 1],
										   pYM2610Buffer[0][(nFractionalPosition >> 16) - 0] + pYM2610Buffer[5][(nFractionalPosition >> 16) - 0]);
			if (bYM2610AddSignal) {
				pSoundBuf[i + 0] += CLIP(nSample);
			} else {
				pSoundBuf[i + 0] = CLIP(nSample);
			}

			// Right channel
			nSample = INTERPOLATE4PS_16BIT((nFractionalPosition >> 4) & 0x0FFF,
										   pYM2610Buffer[1][(nFractionalPosition >> 16) - 3] + pYM2610Buffer[5][(nFractionalPosition >> 16) - 3],
										   pYM2610Buffer[1][(nFractionalPosition >> 16) - 2] + pYM2610Buffer[5][(nFractionalPosition >> 16) - 2],
										   pYM2610Buffer[1][(nFractionalPosition >> 16) - 1] + pYM2610Buffer[5][(nFractionalPosition >> 16) - 1],
										   pYM2610Buffer[1][(nFractionalPosition >> 16) - 0] + pYM2610Buffer[5][(nFractionalPosition >> 16) - 0]);
			if (bYM2610AddSignal) {
				pSoundBuf[i + 1] += CLIP(nSample);
			} else {
				pSoundBuf[i + 1] = CLIP(nSample);
			}
		}

#undef CLIP

	}

	if (nSegmentEnd >= nBurnSoundLen) {
		INT32 nExtraSamples = nSamplesNeeded - (nFractionalPosition >> 16);

//		bprintf(PRINT_NORMAL, _T("   %6i rendered, %i extra, %i <- %i\n"), nSamplesNeeded, nExtraSamples, nExtraSamples, (nFractionalPosition >> 16) + nExtraSamples - 1);

		for (INT32 i = -4; i < nExtraSamples; i++) {
			pYM2610Buffer[0][i] = pYM2610Buffer[0][(nFractionalPosition >> 16) + i];
			pYM2610Buffer[1][i] = pYM2610Buffer[1][(nFractionalPosition >> 16) + i];
			pYM2610Buffer[2][i] = pYM2610Buffer[2][(nFractionalPosition >> 16) + i];
			pYM2610Buffer[3][i] = pYM2610Buffer[3][(nFractionalPosition >> 16) + i];
			pYM2610Buffer[4][i] = pYM2610Buffer[4][(nFractionalPosition >> 16) + i];
		}

		nFractionalPosition &= 0xFFFF;

		nYM2610Position = nExtraSamples;
		nAY8910Position = nExtraSamples;

		dTime += 100.0 / nBurnFPS;
	}
}

static void YM2610UpdateNormal(INT16* pSoundBuf, INT32 nSegmentEnd)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2610Initted) bprintf(PRINT_ERROR, _T("YM2610UpdateNormal called without init\n"));
#endif

	INT32 nSegmentLength = nSegmentEnd;

//	bprintf(PRINT_NORMAL, _T("    YM2610 update        -> %6i\n", nSegmentLength));

	if (nSegmentEnd < nAY8910Position) {
		nSegmentEnd = nAY8910Position;
	}
	if (nSegmentEnd < nYM2610Position) {
		nSegmentEnd = nYM2610Position;
	}

	if (nSegmentLength > nBurnSoundLen) {
		nSegmentLength = nBurnSoundLen;
	}

	YM2610Render(nSegmentEnd);
	AY8910Render(nSegmentEnd);

	pYM2610Buffer[0] = pBuffer + 4 + 0 * 4096;
	pYM2610Buffer[1] = pBuffer + 4 + 1 * 4096;
	pYM2610Buffer[2] = pBuffer + 4 + 2 * 4096;
	pYM2610Buffer[3] = pBuffer + 4 + 3 * 4096;
	pYM2610Buffer[4] = pBuffer + 4 + 4 * 4096;

	if (bBurnUseMMX) {
#if defined BUILD_X86_ASM
		for (INT32 n = nFractionalPosition; n < nSegmentLength; n++) {
			pAYBuffer[n] = pYM2610Buffer[2][n] + pYM2610Buffer[3][n] + pYM2610Buffer[4][n];
		}
		BurnSoundCopy_FM_OPN_A(pYM2610Buffer[0], pAYBuffer, pSoundBuf, nSegmentLength, 65536 * 60 / 100, 65536 * 60 / 100);
#endif
	} else {
		for (INT32 n = nFractionalPosition; n < nSegmentLength; n++) {
			INT32 nAYSample, nTotalSample;

			nAYSample  = pYM2610Buffer[2][n];
			nAYSample += pYM2610Buffer[3][n];
			nAYSample += pYM2610Buffer[4][n];

			nAYSample  *= 4096 * 60 / 100;
			nAYSample >>= 12;
	
#define CLIP(A) A = ((A) < -0x8000 ? -0x8000 : (A) > 0x7fff ? 0x7fff : (A))

			if (bYM2610MixSound) {
				nTotalSample = nAYSample + pYM2610Buffer[0][n] + pYM2610Buffer[1][n];;
				CLIP(nTotalSample);
				
				if (bYM2610AddSignal) {
					pSoundBuf[(n << 1) + 0] += nTotalSample;
					pSoundBuf[(n << 1) + 1] += nTotalSample;
				} else {
					pSoundBuf[(n << 1) + 0] = nTotalSample;
					pSoundBuf[(n << 1) + 1] = nTotalSample;
				}
			} else {
				nTotalSample = nAYSample + pYM2610Buffer[0][n];
				CLIP(nTotalSample);
			
				if (bYM2610AddSignal) {
					pSoundBuf[(n << 1) + 0] += nTotalSample;
				} else {
					pSoundBuf[(n << 1) + 0] = nTotalSample;
				}

				nTotalSample = nAYSample + pYM2610Buffer[1][n];
				CLIP(nTotalSample);
			
				if (bYM2610AddSignal) {
					pSoundBuf[(n << 1) + 1] += nTotalSample;
				} else {
					pSoundBuf[(n << 1) + 1] = nTotalSample;
				}
			}
#undef CLIP
		}
	}

	nFractionalPosition = nSegmentLength;

	if (nSegmentEnd >= nBurnSoundLen) {
		INT32 nExtraSamples = nSegmentEnd - nBurnSoundLen;

		for (INT32 i = 0; i < nExtraSamples; i++) {
			pYM2610Buffer[0][i] = pYM2610Buffer[0][nBurnSoundLen + i];
			pYM2610Buffer[1][i] = pYM2610Buffer[1][nBurnSoundLen + i];
			pYM2610Buffer[2][i] = pYM2610Buffer[2][nBurnSoundLen + i];
			pYM2610Buffer[3][i] = pYM2610Buffer[3][nBurnSoundLen + i];
			pYM2610Buffer[4][i] = pYM2610Buffer[4][nBurnSoundLen + i];
		}

		nFractionalPosition = 0;

		nYM2610Position = nExtraSamples;
		nAY8910Position = nExtraSamples;

		dTime += 100.0 / nBurnFPS;
	}
}

// ----------------------------------------------------------------------------
// Callbacks for YM2610 core

void BurnYM2610UpdateRequest()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2610Initted) bprintf(PRINT_ERROR, _T("YM2610UpdateRequest called without init\n"));
#endif

	YM2610Render(BurnYM2610StreamCallback(nBurnYM2610SoundRate));
}

static void BurnAY8910UpdateRequest()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2610Initted) bprintf(PRINT_ERROR, _T("BurnYM2610 BurnAY8910UpdateRequest called without init\n"));
#endif

	AY8910Render(BurnYM2610StreamCallback(nBurnYM2610SoundRate));
}

// ----------------------------------------------------------------------------
// Initialisation, etc.

void BurnYM2610Reset()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2610Initted) bprintf(PRINT_ERROR, _T("BurnYM2610Reset called without init\n"));
#endif

	BurnTimerReset();

	YM2610ResetChip(0);
}

void BurnYM2610Exit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2610Initted) bprintf(PRINT_ERROR, _T("BurnYM2610Exit called without init\n"));
#endif

	YM2610Shutdown();
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
	
	bYM2610AddSignal = 0;
	bYM2610MixSound = 0;
	
	DebugSnd_YM2610Initted = 0;
}

void BurnYM2610MapADPCMROM(UINT8* YM2610ADPCMAROM, INT32 nYM2610ADPCMASize, UINT8* YM2610ADPCMBROM, INT32 nYM2610ADPCMBSize)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2610Initted) bprintf(PRINT_ERROR, _T("BurnYM2610MapADPCMROM called without init\n"));
#endif

	YM2610SetRom(0, YM2610ADPCMAROM, nYM2610ADPCMASize, YM2610ADPCMBROM, nYM2610ADPCMBSize);
}

INT32 BurnYM2610Init(INT32 nClockFrequency, UINT8* YM2610ADPCMAROM, INT32* nYM2610ADPCMASize, UINT8* YM2610ADPCMBROM, INT32* nYM2610ADPCMBSize, FM_IRQHANDLER IRQCallback, INT32 (*StreamCallback)(INT32), double (*GetTimeCallback)(), INT32 bAddSignal)
{
	DebugSnd_YM2610Initted = 1;
	
	BurnTimerInit(&YM2610TimerOver, GetTimeCallback);

	if (nBurnSoundRate <= 0) {
		BurnYM2610StreamCallback = YM2610StreamCallbackDummy;

		BurnYM2610Update = YM2610UpdateDummy;

		AY8910InitYM(0, nClockFrequency, 11025, NULL, NULL, NULL, NULL, BurnAY8910UpdateRequest);
		YM2610Init(1, nClockFrequency, 11025, (void**)(&YM2610ADPCMAROM), nYM2610ADPCMASize, (void**)(&YM2610ADPCMBROM), nYM2610ADPCMBSize, &BurnOPNTimerCallback, IRQCallback);
		return 0;
	}

	BurnYM2610StreamCallback = StreamCallback;

	if (nFMInterpolation == 3) {
		// Set YM2610 core samplerate to match the hardware
		nBurnYM2610SoundRate = nClockFrequency / 144;
		// Bring YM2610 core samplerate within usable range
		while (nBurnYM2610SoundRate > nBurnSoundRate * 3) {
			nBurnYM2610SoundRate >>= 1;
		}

		BurnYM2610Update = YM2610UpdateResample;

		nSampleSize = (UINT32)nBurnYM2610SoundRate * (1 << 16) / nBurnSoundRate;
	} else {
		nBurnYM2610SoundRate = nBurnSoundRate;

		BurnYM2610Update = YM2610UpdateNormal;
	}

	AY8910InitYM(0, nClockFrequency, nBurnYM2610SoundRate, NULL, NULL, NULL, NULL, BurnAY8910UpdateRequest);
	YM2610Init(1, nClockFrequency, nBurnYM2610SoundRate, (void**)(&YM2610ADPCMAROM), nYM2610ADPCMASize, (void**)(&YM2610ADPCMBROM), nYM2610ADPCMBSize, &BurnOPNTimerCallback, IRQCallback);

	pBuffer = (INT16*)malloc(4096 * 6 * sizeof(INT16));
	memset(pBuffer, 0, 4096 * 6 * sizeof(INT16));

	pAYBuffer = (INT32*)malloc(4096 * sizeof(INT32));
	memset(pAYBuffer, 0, 4096 * sizeof(INT32));

	nYM2610Position = 0;
	nAY8910Position = 0;

	nFractionalPosition = 0;
	bYM2610AddSignal = bAddSignal;

	return 0;
}

void BurnYM2610SetSoundMixMode(INT32 Mix)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2610Initted) bprintf(PRINT_ERROR, _T("BurnYM2610SetSoundMixMode called without init\n"));
#endif

	bYM2610MixSound = Mix;
}

void BurnYM2610Scan(INT32 nAction, INT32* pnMin)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2610Initted) bprintf(PRINT_ERROR, _T("BurnYM2610Scan called without init\n"));
#endif

	BurnTimerScan(nAction, pnMin);
	AY8910Scan(nAction, pnMin);

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(nYM2610Position);
		SCAN_VAR(nAY8910Position);
	}
}
