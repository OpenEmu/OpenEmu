#include "burnint.h"
#include "burn_sound.h"
#include "burn_ym2151.h"

void (*BurnYM2151Render)(INT16* pSoundBuf, INT32 nSegmentLength);

UINT8 BurnYM2151Registers[0x0100];
UINT32 nBurnCurrentYM2151Register;

static INT32 nBurnYM2151SoundRate;

static INT16* pBuffer;
static INT16* pYM2151Buffer[2];

static INT32 nBurnPosition;
static UINT32 nSampleSize;
static UINT32 nFractionalPosition;
static UINT32 nSamplesRendered;

static INT32 nYM2151Volume;

static void YM2151RenderResample(INT16* pSoundBuf, INT32 nSegmentLength)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2151Initted) bprintf(PRINT_ERROR, _T("YM2151RenderResample called without init\n"));
#endif
	
	nBurnPosition += nSegmentLength;

	if (nBurnPosition >= nBurnSoundRate) {
		nBurnPosition = nSegmentLength;

		pYM2151Buffer[0][1] = pYM2151Buffer[0][(nFractionalPosition >> 16) - 3];
		pYM2151Buffer[0][2] = pYM2151Buffer[0][(nFractionalPosition >> 16) - 2];
		pYM2151Buffer[0][3] = pYM2151Buffer[0][(nFractionalPosition >> 16) - 1];

		pYM2151Buffer[1][1] = pYM2151Buffer[1][(nFractionalPosition >> 16) - 3];
		pYM2151Buffer[1][2] = pYM2151Buffer[1][(nFractionalPosition >> 16) - 2];
		pYM2151Buffer[1][3] = pYM2151Buffer[1][(nFractionalPosition >> 16) - 1];

		nSamplesRendered -= (nFractionalPosition >> 16) - 4;

		for (UINT32 i = 0; i <= nSamplesRendered; i++) {
			pYM2151Buffer[0][4 + i] = pYM2151Buffer[0][(nFractionalPosition >> 16) + i];
			pYM2151Buffer[1][4 + i] = pYM2151Buffer[1][(nFractionalPosition >> 16) + i];
		}

		nFractionalPosition &= 0x0000FFFF;
		nFractionalPosition |= 4 << 16;
	}

	pYM2151Buffer[0] = pBuffer + 4 + nSamplesRendered;
	pYM2151Buffer[1] = pBuffer + 4 + nSamplesRendered + 65536;

	YM2151UpdateOne(0, pYM2151Buffer, (UINT32)(nBurnPosition + 1) * nBurnYM2151SoundRate / nBurnSoundRate - nSamplesRendered);
	nSamplesRendered += (UINT32)(nBurnPosition + 1) * nBurnYM2151SoundRate / nBurnSoundRate - nSamplesRendered;

	pYM2151Buffer[0] = pBuffer;
	pYM2151Buffer[1] = pBuffer + 65536;

	nSegmentLength <<= 1;

	for (INT32 i = 0; i < nSegmentLength; i += 2, nFractionalPosition += nSampleSize) {

		// Left channel
		pSoundBuf[i + 0] = INTERPOLATE4PS_CUSTOM((nFractionalPosition >> 4) & 0x0FFF,
												 pYM2151Buffer[0][(nFractionalPosition >> 16) - 3],
												 pYM2151Buffer[0][(nFractionalPosition >> 16) - 2],
												 pYM2151Buffer[0][(nFractionalPosition >> 16) - 1],
												 pYM2151Buffer[0][(nFractionalPosition >> 16) - 0],
												 nYM2151Volume);

		// Right channel
		pSoundBuf[i + 1] = INTERPOLATE4PS_CUSTOM((nFractionalPosition >> 4) & 0x0FFF,
												 pYM2151Buffer[1][(nFractionalPosition >> 16) - 3],
												 pYM2151Buffer[1][(nFractionalPosition >> 16) - 2],
												 pYM2151Buffer[1][(nFractionalPosition >> 16) - 1],
												 pYM2151Buffer[1][(nFractionalPosition >> 16) - 0],
												 nYM2151Volume);
	}
}

static void YM2151RenderNormal(INT16* pSoundBuf, INT32 nSegmentLength)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2151Initted) bprintf(PRINT_ERROR, _T("YM2151RenderNormal called without init\n"));
#endif

	nBurnPosition += nSegmentLength;

	pYM2151Buffer[0] = pBuffer;
	pYM2151Buffer[1] = pBuffer + nSegmentLength;

	YM2151UpdateOne(0, pYM2151Buffer, nSegmentLength);

	if (bBurnUseMMX) {
#if defined BUILD_X86_ASM
		BurnSoundCopy_FM_A(pYM2151Buffer[0], pYM2151Buffer[1], pSoundBuf, nSegmentLength, nYM2151Volume, nYM2151Volume);
#endif
	} else {
		for (INT32 n = 0; n < nSegmentLength; n++) {
			for (INT32 i = 0; i < 2; i++) {
				INT32 nSample = pYM2151Buffer[i][n] * (nYM2151Volume >> 10);
				nSample >>= 8;
				
				if (nSample < -32768) nSample = -32768;
				if (nSample > 32767) nSample = 32767;
			
				pSoundBuf[(n << 1) + i] = nSample;
			}
		}
	}
}

void BurnYM2151Reset()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2151Initted) bprintf(PRINT_ERROR, _T("BurnYM2151Reset called without init\n"));
#endif

	YM2151ResetChip(0);
}

void BurnYM2151Exit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2151Initted) bprintf(PRINT_ERROR, _T("BurnYM2151Exit called without init\n"));
#endif

	BurnYM2151SetIrqHandler(NULL);
	BurnYM2151SetPortHandler(NULL);

	YM2151Shutdown();

	if (pBuffer) {
		free(pBuffer);
		pBuffer = NULL;
	}
	
	DebugSnd_YM2151Initted = 0;
}

INT32 BurnYM2151Init(INT32 nClockFrequency, float nVolume)
{
	DebugSnd_YM2151Initted = 1;
	
	if (nBurnSoundRate <= 0) {
		YM2151Init(1, nClockFrequency, 11025);
		return 0;
	}

	if (nFMInterpolation == 3) {
		// Set YM2151 core samplerate to match the hardware
		nBurnYM2151SoundRate = nClockFrequency >> 6;
		// Bring YM2151 core samplerate within usable range
		while (nBurnYM2151SoundRate > nBurnSoundRate * 3) {
			nBurnYM2151SoundRate >>= 1;
		}

		BurnYM2151Render = YM2151RenderResample;

		nYM2151Volume = (INT32)((double)16384.0 * 100.0 / nVolume);
	} else {
		nBurnYM2151SoundRate = nBurnSoundRate;
		BurnYM2151Render = YM2151RenderNormal;

		nYM2151Volume = (INT32)((double)65536.0 * 100.0 / nVolume);
	}

	YM2151Init(1, nClockFrequency, nBurnYM2151SoundRate);

	pBuffer = (INT16*)malloc(65536 * 2 * sizeof(INT16));
	memset(pBuffer, 0, 65536 * 2 * sizeof(INT16));

	nSampleSize = (UINT32)nBurnYM2151SoundRate * (1 << 16) / nBurnSoundRate;
	nFractionalPosition = 4 << 16;
	nSamplesRendered = 0;
	nBurnPosition = 0;

	return 0;
}

void BurnYM2151Scan(INT32 nAction)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2151Initted) bprintf(PRINT_ERROR, _T("BurnYM2151Scan called without init\n"));
#endif
	
	if ((nAction & ACB_DRIVER_DATA) == 0) {
		return;
	}
	SCAN_VAR(nBurnCurrentYM2151Register);
	SCAN_VAR(BurnYM2151Registers);

	if (nAction & ACB_WRITE) {
		for (INT32 i = 0; i < 0x0100; i++) {
			YM2151WriteReg(0, i, BurnYM2151Registers[i]);
		}
	}
}
