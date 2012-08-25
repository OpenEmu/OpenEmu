#include "burnint.h"
#include "burn_sound.h"
#include "burn_ym2203.h"

#define MAX_YM2203	2

void (*BurnYM2203Update)(INT16* pSoundBuf, INT32 nSegmentEnd);

static INT32 (*BurnYM2203StreamCallback)(INT32 nSoundRate);

static INT32 nBurnYM2203SoundRate;

static INT16* pBuffer;
static INT16* pYM2203Buffer[4 * MAX_YM2203];

static INT32 nYM2203Position;
static INT32 nAY8910Position;

static UINT32 nSampleSize;
static INT32 nFractionalPosition;

static INT32 nNumChips = 0;

static INT32 bYM2203AddSignal;

static double YM2203Volumes[4 * MAX_YM2203];
static INT32 YM2203RouteDirs[4 * MAX_YM2203];

static double YM2203LeftVolumes[4 * MAX_YM2203];
static double YM2203RightVolumes[4 * MAX_YM2203];

INT32 bYM2203UseSeperateVolumes; // support custom Taito panning hardware

// ----------------------------------------------------------------------------
// Dummy functions

static void YM2203UpdateDummy(INT16*, INT32)
{
	return;
}

static INT32 YM2203StreamCallbackDummy(INT32)
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

	nSegmentLength -= nAY8910Position;

	pYM2203Buffer[1] = pBuffer + 1 * 4096 + 4 + nAY8910Position;
	pYM2203Buffer[2] = pBuffer + 2 * 4096 + 4 + nAY8910Position;
	pYM2203Buffer[3] = pBuffer + 3 * 4096 + 4 + nAY8910Position;

	AY8910Update(0, &pYM2203Buffer[1], nSegmentLength);
	
	if (nNumChips > 1) {
		pYM2203Buffer[5] = pBuffer + 5 * 4096 + 4 + nAY8910Position;
		pYM2203Buffer[6] = pBuffer + 6 * 4096 + 4 + nAY8910Position;
		pYM2203Buffer[7] = pBuffer + 7 * 4096 + 4 + nAY8910Position;

		AY8910Update(1, &pYM2203Buffer[5], nSegmentLength);
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

	nSegmentLength -= nYM2203Position;

	pYM2203Buffer[0] = pBuffer + 0 * 4096 + 4 + nYM2203Position;

	YM2203UpdateOne(0, pYM2203Buffer[0], nSegmentLength);
	
	if (nNumChips > 1) {
		pYM2203Buffer[4] = pBuffer + 4 * 4096 + 4 + nYM2203Position;

		YM2203UpdateOne(1, pYM2203Buffer[4], nSegmentLength);
	}

	nYM2203Position += nSegmentLength;
}

// ----------------------------------------------------------------------------
// Update the sound buffer

#define INTERPOLATE_ADD_SOUND_LEFT(route, buffer)																	\
	if ((YM2203RouteDirs[route] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {									\
		nLeftSample[0] += (INT32)(pYM2203Buffer[buffer][(nFractionalPosition >> 16) - 3] * YM2203Volumes[route]);	\
		nLeftSample[1] += (INT32)(pYM2203Buffer[buffer][(nFractionalPosition >> 16) - 2] * YM2203Volumes[route]);	\
		nLeftSample[2] += (INT32)(pYM2203Buffer[buffer][(nFractionalPosition >> 16) - 1] * YM2203Volumes[route]);	\
		nLeftSample[3] += (INT32)(pYM2203Buffer[buffer][(nFractionalPosition >> 16) - 0] * YM2203Volumes[route]);	\
	}

#define INTERPOLATE_ADD_SOUND_RIGHT(route, buffer)																	\
	if ((YM2203RouteDirs[route] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {									\
		nRightSample[0] += (INT32)(pYM2203Buffer[buffer][(nFractionalPosition >> 16) - 3] * YM2203Volumes[route]);	\
		nRightSample[1] += (INT32)(pYM2203Buffer[buffer][(nFractionalPosition >> 16) - 2] * YM2203Volumes[route]);	\
		nRightSample[2] += (INT32)(pYM2203Buffer[buffer][(nFractionalPosition >> 16) - 1] * YM2203Volumes[route]);	\
		nRightSample[3] += (INT32)(pYM2203Buffer[buffer][(nFractionalPosition >> 16) - 0] * YM2203Volumes[route]);	\
	}
	
#define SPLIT_INTERPOLATE_ADD_SOUND_LEFT(route, buffer)																\
	nLeftSample[0] += (INT32)(pYM2203Buffer[buffer][(nFractionalPosition >> 16) - 3] * YM2203LeftVolumes[route]);	\
	nLeftSample[1] += (INT32)(pYM2203Buffer[buffer][(nFractionalPosition >> 16) - 2] * YM2203LeftVolumes[route]);	\
	nLeftSample[2] += (INT32)(pYM2203Buffer[buffer][(nFractionalPosition >> 16) - 1] * YM2203LeftVolumes[route]);	\
	nLeftSample[3] += (INT32)(pYM2203Buffer[buffer][(nFractionalPosition >> 16) - 0] * YM2203LeftVolumes[route]);
	
#define SPLIT_INTERPOLATE_ADD_SOUND_RIGHT(route, buffer)																\
	nRightSample[0] += (INT32)(pYM2203Buffer[buffer][(nFractionalPosition >> 16) - 3] * YM2203RightVolumes[route]);	\
	nRightSample[1] += (INT32)(pYM2203Buffer[buffer][(nFractionalPosition >> 16) - 2] * YM2203RightVolumes[route]);	\
	nRightSample[2] += (INT32)(pYM2203Buffer[buffer][(nFractionalPosition >> 16) - 1] * YM2203RightVolumes[route]);	\
	nRightSample[3] += (INT32)(pYM2203Buffer[buffer][(nFractionalPosition >> 16) - 0] * YM2203RightVolumes[route]);

static void YM2203UpdateResample(INT16* pSoundBuf, INT32 nSegmentEnd)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2203Initted) bprintf(PRINT_ERROR, _T("YM2203UpdateResample called without init\n"));
#endif

	INT32 nSegmentLength = nSegmentEnd;
	INT32 nSamplesNeeded = nSegmentEnd * nBurnYM2203SoundRate / nBurnSoundRate + 1;

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
	
	if (nNumChips > 1) {
		pYM2203Buffer[4] = pBuffer + 4 * 4096 + 4;
		pYM2203Buffer[5] = pBuffer + 5 * 4096 + 4;
		pYM2203Buffer[6] = pBuffer + 6 * 4096 + 4;
		pYM2203Buffer[7] = pBuffer + 7 * 4096 + 4;
	}

	for (INT32 i = (nFractionalPosition & 0xFFFF0000) >> 15; i < nSegmentLength; i += 2, nFractionalPosition += nSampleSize) {
		INT32 nLeftSample[4] = {0, 0, 0, 0};
		INT32 nRightSample[4] = {0, 0, 0, 0};
		INT32 nTotalLeftSample, nTotalRightSample;
		
		if (bYM2203UseSeperateVolumes) {
			SPLIT_INTERPOLATE_ADD_SOUND_LEFT  (BURN_SND_YM2203_AY8910_ROUTE_1, 1)
			SPLIT_INTERPOLATE_ADD_SOUND_LEFT  (BURN_SND_YM2203_AY8910_ROUTE_2, 2)
			SPLIT_INTERPOLATE_ADD_SOUND_LEFT  (BURN_SND_YM2203_AY8910_ROUTE_3, 3)
			SPLIT_INTERPOLATE_ADD_SOUND_LEFT  (BURN_SND_YM2203_YM2203_ROUTE  , 0)
			
			SPLIT_INTERPOLATE_ADD_SOUND_RIGHT (BURN_SND_YM2203_AY8910_ROUTE_1, 1)
			SPLIT_INTERPOLATE_ADD_SOUND_RIGHT (BURN_SND_YM2203_AY8910_ROUTE_2, 2)
			SPLIT_INTERPOLATE_ADD_SOUND_RIGHT (BURN_SND_YM2203_AY8910_ROUTE_3, 3)
			SPLIT_INTERPOLATE_ADD_SOUND_RIGHT (BURN_SND_YM2203_YM2203_ROUTE  , 0)
			
			if (nNumChips > 1) {
				SPLIT_INTERPOLATE_ADD_SOUND_LEFT  (4 + BURN_SND_YM2203_AY8910_ROUTE_1, 5)
				SPLIT_INTERPOLATE_ADD_SOUND_LEFT  (4 + BURN_SND_YM2203_AY8910_ROUTE_2, 6)
				SPLIT_INTERPOLATE_ADD_SOUND_LEFT  (4 + BURN_SND_YM2203_AY8910_ROUTE_3, 7)
				SPLIT_INTERPOLATE_ADD_SOUND_LEFT  (4 + BURN_SND_YM2203_YM2203_ROUTE  , 4)
			
				SPLIT_INTERPOLATE_ADD_SOUND_RIGHT (4 + BURN_SND_YM2203_AY8910_ROUTE_1, 5)
				SPLIT_INTERPOLATE_ADD_SOUND_RIGHT (4 + BURN_SND_YM2203_AY8910_ROUTE_2, 6)
				SPLIT_INTERPOLATE_ADD_SOUND_RIGHT (4 + BURN_SND_YM2203_AY8910_ROUTE_3, 7)
				SPLIT_INTERPOLATE_ADD_SOUND_RIGHT (4 + BURN_SND_YM2203_YM2203_ROUTE  , 4)
			}
		} else {		
			INTERPOLATE_ADD_SOUND_LEFT  (BURN_SND_YM2203_AY8910_ROUTE_1, 1)
			INTERPOLATE_ADD_SOUND_RIGHT (BURN_SND_YM2203_AY8910_ROUTE_1, 1)
			INTERPOLATE_ADD_SOUND_LEFT  (BURN_SND_YM2203_AY8910_ROUTE_2, 2)
			INTERPOLATE_ADD_SOUND_RIGHT (BURN_SND_YM2203_AY8910_ROUTE_2, 2)
			INTERPOLATE_ADD_SOUND_LEFT  (BURN_SND_YM2203_AY8910_ROUTE_3, 3)
			INTERPOLATE_ADD_SOUND_RIGHT (BURN_SND_YM2203_AY8910_ROUTE_3, 3)
			INTERPOLATE_ADD_SOUND_LEFT  (BURN_SND_YM2203_YM2203_ROUTE  , 0)
			INTERPOLATE_ADD_SOUND_RIGHT (BURN_SND_YM2203_YM2203_ROUTE  , 0)
		
			if (nNumChips > 1) {
				INTERPOLATE_ADD_SOUND_LEFT  (4 + BURN_SND_YM2203_AY8910_ROUTE_1, 5)
				INTERPOLATE_ADD_SOUND_RIGHT (4 + BURN_SND_YM2203_AY8910_ROUTE_1, 5)
				INTERPOLATE_ADD_SOUND_LEFT  (4 + BURN_SND_YM2203_AY8910_ROUTE_2, 6)
				INTERPOLATE_ADD_SOUND_RIGHT (4 + BURN_SND_YM2203_AY8910_ROUTE_2, 6)
				INTERPOLATE_ADD_SOUND_LEFT  (4 + BURN_SND_YM2203_AY8910_ROUTE_3, 7)
				INTERPOLATE_ADD_SOUND_RIGHT (4 + BURN_SND_YM2203_AY8910_ROUTE_3, 7)
				INTERPOLATE_ADD_SOUND_LEFT  (4 + BURN_SND_YM2203_YM2203_ROUTE  , 4)
				INTERPOLATE_ADD_SOUND_RIGHT (4 + BURN_SND_YM2203_YM2203_ROUTE  , 4)
			}
		}
		
		nTotalLeftSample = INTERPOLATE4PS_16BIT((nFractionalPosition >> 4) & 0x0fff, nLeftSample[0], nLeftSample[1], nLeftSample[2], nLeftSample[3]);
		nTotalRightSample = INTERPOLATE4PS_16BIT((nFractionalPosition >> 4) & 0x0fff, nRightSample[0], nRightSample[1], nRightSample[2], nRightSample[3]);
		
		nTotalLeftSample = BURN_SND_CLIP(nTotalLeftSample);
		nTotalRightSample = BURN_SND_CLIP(nTotalRightSample);
			
		if (bYM2203AddSignal) {
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
			pYM2203Buffer[0][i] = pYM2203Buffer[0][(nFractionalPosition >> 16) + i];
			pYM2203Buffer[1][i] = pYM2203Buffer[1][(nFractionalPosition >> 16) + i];
			pYM2203Buffer[2][i] = pYM2203Buffer[2][(nFractionalPosition >> 16) + i];
			pYM2203Buffer[3][i] = pYM2203Buffer[3][(nFractionalPosition >> 16) + i];
			
			if (nNumChips > 1) {
				pYM2203Buffer[4][i] = pYM2203Buffer[4][(nFractionalPosition >> 16) + i];
				pYM2203Buffer[5][i] = pYM2203Buffer[5][(nFractionalPosition >> 16) + i];
				pYM2203Buffer[6][i] = pYM2203Buffer[6][(nFractionalPosition >> 16) + i];
				pYM2203Buffer[7][i] = pYM2203Buffer[7][(nFractionalPosition >> 16) + i];
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
		pYM2203Buffer[4] = pBuffer + 4 + 4 * 4096;
		pYM2203Buffer[5] = pBuffer + 4 + 5 * 4096;
		pYM2203Buffer[6] = pBuffer + 4 + 6 * 4096;
		pYM2203Buffer[7] = pBuffer + 4 + 7 * 4096;
	}

	for (INT32 n = nFractionalPosition; n < nSegmentLength; n++) {
		INT32 nLeftSample = 0, nRightSample = 0;
		
		if (bYM2203UseSeperateVolumes) {
			nLeftSample += (INT32)(pYM2203Buffer[1][n] * YM2203LeftVolumes[BURN_SND_YM2203_AY8910_ROUTE_1]);
			nLeftSample += (INT32)(pYM2203Buffer[2][n] * YM2203LeftVolumes[BURN_SND_YM2203_AY8910_ROUTE_2]);
			nLeftSample += (INT32)(pYM2203Buffer[3][n] * YM2203LeftVolumes[BURN_SND_YM2203_AY8910_ROUTE_3]);
			nLeftSample += (INT32)(pYM2203Buffer[0][n] * YM2203LeftVolumes[BURN_SND_YM2203_YM2203_ROUTE]);
			
			nRightSample += (INT32)(pYM2203Buffer[1][n] * YM2203RightVolumes[BURN_SND_YM2203_AY8910_ROUTE_1]);
			nRightSample += (INT32)(pYM2203Buffer[2][n] * YM2203RightVolumes[BURN_SND_YM2203_AY8910_ROUTE_2]);
			nRightSample += (INT32)(pYM2203Buffer[3][n] * YM2203RightVolumes[BURN_SND_YM2203_AY8910_ROUTE_3]);
			nRightSample += (INT32)(pYM2203Buffer[0][n] * YM2203RightVolumes[BURN_SND_YM2203_YM2203_ROUTE]);
			
			if (nNumChips > 1) {
				nLeftSample += (INT32)(pYM2203Buffer[5][n] * YM2203LeftVolumes[4 + BURN_SND_YM2203_AY8910_ROUTE_1]);
				nLeftSample += (INT32)(pYM2203Buffer[6][n] * YM2203LeftVolumes[4 + BURN_SND_YM2203_AY8910_ROUTE_2]);
				nLeftSample += (INT32)(pYM2203Buffer[7][n] * YM2203LeftVolumes[4 + BURN_SND_YM2203_AY8910_ROUTE_3]);
				nLeftSample += (INT32)(pYM2203Buffer[4][n] * YM2203LeftVolumes[4 + BURN_SND_YM2203_YM2203_ROUTE]);
			
				nRightSample += (INT32)(pYM2203Buffer[5][n] * YM2203RightVolumes[4 + BURN_SND_YM2203_AY8910_ROUTE_1]);
				nRightSample += (INT32)(pYM2203Buffer[6][n] * YM2203RightVolumes[4 + BURN_SND_YM2203_AY8910_ROUTE_2]);
				nRightSample += (INT32)(pYM2203Buffer[7][n] * YM2203RightVolumes[4 + BURN_SND_YM2203_AY8910_ROUTE_3]);
				nRightSample += (INT32)(pYM2203Buffer[4][n] * YM2203RightVolumes[4 + BURN_SND_YM2203_YM2203_ROUTE]);
			}
		} else {
			if ((YM2203RouteDirs[BURN_SND_YM2203_AY8910_ROUTE_1] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
				nLeftSample += (INT32)(pYM2203Buffer[1][n] * YM2203Volumes[BURN_SND_YM2203_AY8910_ROUTE_1]);
			}
			if ((YM2203RouteDirs[BURN_SND_YM2203_AY8910_ROUTE_1] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
				nRightSample += (INT32)(pYM2203Buffer[1][n] * YM2203Volumes[BURN_SND_YM2203_AY8910_ROUTE_1]);
			}
		
			if ((YM2203RouteDirs[BURN_SND_YM2203_AY8910_ROUTE_2] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
				nLeftSample += (INT32)(pYM2203Buffer[2][n] * YM2203Volumes[BURN_SND_YM2203_AY8910_ROUTE_2]);
			}
			if ((YM2203RouteDirs[BURN_SND_YM2203_AY8910_ROUTE_2] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
				nRightSample += (INT32)(pYM2203Buffer[2][n] * YM2203Volumes[BURN_SND_YM2203_AY8910_ROUTE_2]);
			}
		
			if ((YM2203RouteDirs[BURN_SND_YM2203_AY8910_ROUTE_3] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
				nLeftSample += (INT32)(pYM2203Buffer[3][n] * YM2203Volumes[BURN_SND_YM2203_AY8910_ROUTE_3]);
			}
			if ((YM2203RouteDirs[BURN_SND_YM2203_AY8910_ROUTE_3] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
				nRightSample += (INT32)(pYM2203Buffer[3][n] * YM2203Volumes[BURN_SND_YM2203_AY8910_ROUTE_3]);
			}
		
			if ((YM2203RouteDirs[BURN_SND_YM2203_YM2203_ROUTE] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
				nLeftSample += (INT32)(pYM2203Buffer[0][n] * YM2203Volumes[BURN_SND_YM2203_YM2203_ROUTE]);
			}
			if ((YM2203RouteDirs[BURN_SND_YM2203_YM2203_ROUTE] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
				nRightSample += (INT32)(pYM2203Buffer[0][n] * YM2203Volumes[BURN_SND_YM2203_YM2203_ROUTE]);
			}

			if (nNumChips > 1) {
				if ((YM2203RouteDirs[4 + BURN_SND_YM2203_AY8910_ROUTE_1] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
					nLeftSample += (INT32)(pYM2203Buffer[5][n] * YM2203Volumes[4 + BURN_SND_YM2203_AY8910_ROUTE_1]);
				}
				if ((YM2203RouteDirs[4 + BURN_SND_YM2203_AY8910_ROUTE_1] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
					nRightSample += (INT32)(pYM2203Buffer[5][n] * YM2203Volumes[4 + BURN_SND_YM2203_AY8910_ROUTE_1]);
				}
		
				if ((YM2203RouteDirs[4 + BURN_SND_YM2203_AY8910_ROUTE_2] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
					nLeftSample += (INT32)(pYM2203Buffer[6][n] * YM2203Volumes[4 + BURN_SND_YM2203_AY8910_ROUTE_2]);
				}
				if ((YM2203RouteDirs[4 + BURN_SND_YM2203_AY8910_ROUTE_2] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
					nRightSample += (INT32)(pYM2203Buffer[6][n] * YM2203Volumes[4 + BURN_SND_YM2203_AY8910_ROUTE_2]);
				}
		
				if ((YM2203RouteDirs[4 + BURN_SND_YM2203_AY8910_ROUTE_3] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
					nLeftSample += (INT32)(pYM2203Buffer[7][n] * YM2203Volumes[4 + BURN_SND_YM2203_AY8910_ROUTE_3]);
				}
				if ((YM2203RouteDirs[4 + BURN_SND_YM2203_AY8910_ROUTE_3] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
					nRightSample += (INT32)(pYM2203Buffer[7][n] * YM2203Volumes[4 + BURN_SND_YM2203_AY8910_ROUTE_3]);
				}
		
				if ((YM2203RouteDirs[4 + BURN_SND_YM2203_YM2203_ROUTE] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
					nLeftSample += (INT32)(pYM2203Buffer[4][n] * YM2203Volumes[4 + BURN_SND_YM2203_YM2203_ROUTE]);
				}
				if ((YM2203RouteDirs[4 + BURN_SND_YM2203_YM2203_ROUTE] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
					nRightSample += (INT32)(pYM2203Buffer[4][n] * YM2203Volumes[4 + BURN_SND_YM2203_YM2203_ROUTE]);
				}
			}
		}
		
		nLeftSample = BURN_SND_CLIP(nLeftSample);
		nRightSample = BURN_SND_CLIP(nRightSample);
			
		if (bYM2203AddSignal) {
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
			pYM2203Buffer[0][i] = pYM2203Buffer[0][nBurnSoundLen + i];
			pYM2203Buffer[1][i] = pYM2203Buffer[1][nBurnSoundLen + i];
			pYM2203Buffer[2][i] = pYM2203Buffer[2][nBurnSoundLen + i];
			pYM2203Buffer[3][i] = pYM2203Buffer[3][nBurnSoundLen + i];
			if (nNumChips > 1) {
				pYM2203Buffer[4][i] = pYM2203Buffer[4][nBurnSoundLen + i];
				pYM2203Buffer[5][i] = pYM2203Buffer[5][nBurnSoundLen + i];
				pYM2203Buffer[6][i] = pYM2203Buffer[6][nBurnSoundLen + i];
				pYM2203Buffer[7][i] = pYM2203Buffer[7][nBurnSoundLen + i];
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
	bYM2203UseSeperateVolumes = 0;
	
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

	pBuffer = (INT16*)malloc(4096 * 4 * num * sizeof(INT16));
	memset(pBuffer, 0, 4096 * 4 * num * sizeof(INT16));

	nYM2203Position = 0;
	nAY8910Position = 0;
	nFractionalPosition = 0;
	
	nNumChips = num;
	bYM2203AddSignal = bAddSignal;
	
	// default routes
	YM2203Volumes[BURN_SND_YM2203_YM2203_ROUTE] = 1.00;
	YM2203Volumes[BURN_SND_YM2203_AY8910_ROUTE_1] = 1.00;
	YM2203Volumes[BURN_SND_YM2203_AY8910_ROUTE_2] = 1.00;
	YM2203Volumes[BURN_SND_YM2203_AY8910_ROUTE_3] = 1.00;
	YM2203RouteDirs[BURN_SND_YM2203_YM2203_ROUTE] = BURN_SND_ROUTE_BOTH;
	YM2203RouteDirs[BURN_SND_YM2203_AY8910_ROUTE_1] = BURN_SND_ROUTE_BOTH;
	YM2203RouteDirs[BURN_SND_YM2203_AY8910_ROUTE_2] = BURN_SND_ROUTE_BOTH;
	YM2203RouteDirs[BURN_SND_YM2203_AY8910_ROUTE_3] = BURN_SND_ROUTE_BOTH;
	
	bYM2203UseSeperateVolumes = 0;
	YM2203LeftVolumes[BURN_SND_YM2203_YM2203_ROUTE] = 1.00;
	YM2203LeftVolumes[BURN_SND_YM2203_AY8910_ROUTE_1] = 1.00;
	YM2203LeftVolumes[BURN_SND_YM2203_AY8910_ROUTE_2] = 1.00;
	YM2203LeftVolumes[BURN_SND_YM2203_AY8910_ROUTE_3] = 1.00;
	YM2203RightVolumes[BURN_SND_YM2203_YM2203_ROUTE] = 1.00;
	YM2203RightVolumes[BURN_SND_YM2203_AY8910_ROUTE_1] = 1.00;
	YM2203RightVolumes[BURN_SND_YM2203_AY8910_ROUTE_2] = 1.00;
	YM2203RightVolumes[BURN_SND_YM2203_AY8910_ROUTE_3] = 1.00;	
	
	if (nNumChips > 0) {
		YM2203Volumes[4 + BURN_SND_YM2203_YM2203_ROUTE] = 1.00;
		YM2203Volumes[4 + BURN_SND_YM2203_AY8910_ROUTE_1] = 1.00;
		YM2203Volumes[4 + BURN_SND_YM2203_AY8910_ROUTE_2] = 1.00;
		YM2203Volumes[4 + BURN_SND_YM2203_AY8910_ROUTE_3] = 1.00;
		YM2203RouteDirs[4 + BURN_SND_YM2203_YM2203_ROUTE] = BURN_SND_ROUTE_BOTH;
		YM2203RouteDirs[4 + BURN_SND_YM2203_AY8910_ROUTE_1] = BURN_SND_ROUTE_BOTH;
		YM2203RouteDirs[4 + BURN_SND_YM2203_AY8910_ROUTE_2] = BURN_SND_ROUTE_BOTH;
		YM2203RouteDirs[4 + BURN_SND_YM2203_AY8910_ROUTE_3] = BURN_SND_ROUTE_BOTH;
		
		YM2203LeftVolumes[4 + BURN_SND_YM2203_YM2203_ROUTE] = 1.00;
		YM2203LeftVolumes[4 + BURN_SND_YM2203_AY8910_ROUTE_1] = 1.00;
		YM2203LeftVolumes[4 + BURN_SND_YM2203_AY8910_ROUTE_2] = 1.00;
		YM2203LeftVolumes[4 + BURN_SND_YM2203_AY8910_ROUTE_3] = 1.00;
		YM2203RightVolumes[4 + BURN_SND_YM2203_YM2203_ROUTE] = 1.00;
		YM2203RightVolumes[4 + BURN_SND_YM2203_AY8910_ROUTE_1] = 1.00;
		YM2203RightVolumes[4 + BURN_SND_YM2203_AY8910_ROUTE_2] = 1.00;
		YM2203RightVolumes[4 + BURN_SND_YM2203_AY8910_ROUTE_3] = 1.00;	
	}

	return 0;
}

void BurnYM2203SetRoute(INT32 nChip, INT32 nIndex, double nVolume, INT32 nRouteDir)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2203Initted) bprintf(PRINT_ERROR, _T("BurnYM2203SetRoute called without init\n"));
	if (nIndex < 0 || nIndex > 3) bprintf(PRINT_ERROR, _T("BurnYM2203SetRoute called with invalid index %i\n"), nIndex);
	if (nChip >= nNumChips) bprintf(PRINT_ERROR, _T("BurnYM2203SetRoute called with invalid chip %i\n"), nChip);
#endif

	if (nChip == 0) {
		YM2203Volumes[nIndex] = nVolume;
		YM2203RouteDirs[nIndex] = nRouteDir;
	}
	
	if (nChip == 1) {
		YM2203Volumes[4 + nIndex] = nVolume;
		YM2203RouteDirs[4 + nIndex] = nRouteDir;
	}
}

void BurnYM2203SetLeftVolume(INT32 nChip, INT32 nIndex, double nLeftVolume)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2203Initted) bprintf(PRINT_ERROR, _T("BurnYM2203SetLeftVolume called without init\n"));
	if (nIndex < 0 || nIndex > 3) bprintf(PRINT_ERROR, _T("BurnYM2203SetLeftVolume called with invalid index %i\n"), nIndex);
	if (nChip >= nNumChips) bprintf(PRINT_ERROR, _T("BurnYM2203SetLeftVolume called with invalid chip %i\n"), nChip);
#endif
	
	if (nChip == 0) YM2203LeftVolumes[nIndex] = nLeftVolume;
	if (nChip == 1) YM2203LeftVolumes[4 + nIndex] = nLeftVolume;
}

void BurnYM2203SetRightVolume(INT32 nChip, INT32 nIndex, double nRightVolume)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2203Initted) bprintf(PRINT_ERROR, _T("BurnYM2203SetRightVolume called without init\n"));
	if (nIndex < 0 || nIndex > 3) bprintf(PRINT_ERROR, _T("BurnYM2203SetRightVolume called with invalid index %i\n"), nIndex);
	if (nChip >= nNumChips) bprintf(PRINT_ERROR, _T("BurnYM2203SetRightVolume called with invalid chip %i\n"), nChip);
#endif
	
	if (nChip == 0) YM2203RightVolumes[nIndex] = nRightVolume;
	if (nChip == 1) YM2203RightVolumes[4 + nIndex] = nRightVolume;
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
