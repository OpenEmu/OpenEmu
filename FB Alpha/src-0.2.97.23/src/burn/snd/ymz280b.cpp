// Yamaha YMZ280B module
#include <math.h>
#include "burnint.h"
#include "ymz280b.h"
#include "burn_sound.h"

static INT32 nYMZ280BSampleRate;

UINT8* YMZ280BROM;
void (*pYMZ280BRAMWrite)(INT32 offset, INT32 nValue) = NULL;
INT32 (*pYMZ280BRAMRead)(INT32 offset) = NULL;

UINT32 nYMZ280BStatus;
UINT32 nYMZ280BRegister;

static bool bYMZ280BEnable;

static bool bYMZ280BIRQEnable;
static INT32 nYMZ280BIRQMask;
static INT32 nYMZ280BIRQStatus;
void (*YMZ280BIRQCallback)(INT32 nStatus) = NULL;

static INT32* pBuffer = NULL;

static double nYMZ280BFrequency;

static INT32 YMZ280BDeltaTable[16];

static INT32 YMZ280BStepShift[8] = {0x0E6, 0x0E6, 0x0E6, 0x0E6, 0x133, 0x199, 0x200, 0x266};

static double YMZ280BVolumes[2];
static INT32 YMZ280BRouteDirs[2];

struct sYMZ280BChannelInfo {
	bool bEnabled;
	bool bPlaying;
	bool bLoop;
	INT32 nMode;
	INT32 nFrequency;
	INT32 nSample;
	INT32 nLoopSample;
	UINT32 nSampleSize;
	UINT32 nPosition;
	INT32 nFractionalPosition;
	INT32 nStep;
	INT32 nLoopStep;
	UINT32 nSampleStart;
	UINT32 nSampleStop;
	UINT32 nLoopStart;
	UINT32 nLoopStop;
	INT32 nVolume;
	INT32 nVolumeLeft;
	INT32 nVolumeRight;
	INT32 nPan;

	INT32 nOutput;
	INT32 nPreviousOutput;

	INT32 nBufPos;
};

static INT32 nActiveChannel, nDelta, nSample, nCount, nRamReadAddress;
static INT32* buf;

sYMZ280BChannelInfo YMZ280BChannelInfo[8];
static sYMZ280BChannelInfo* channelInfo;

static INT32* YMZ280BChannelData[8];

void YMZ280BReset()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YMZ280BInitted) bprintf(PRINT_ERROR, _T("YMZ280BReset called without init\n"));
#endif

	memset(&YMZ280BChannelInfo[0], 0, sizeof(YMZ280BChannelInfo));

	nYMZ280BIRQMask = 0;
	nYMZ280BIRQStatus = 0;
	nYMZ280BStatus = 0;
	bYMZ280BEnable = false;
	nRamReadAddress = 0;

	for (INT32 j = 0; j < 8; j++) {
		memset(YMZ280BChannelData[j], 0, 0x1000 * sizeof(INT32));
		YMZ280BChannelInfo[j].nBufPos = 4;
	}

	return;
}

inline void YMZ280BSetSampleSize(const INT32 nChannel)
{
	double rate = (double)(YMZ280BChannelInfo[nChannel].nFrequency + 1) * nYMZ280BFrequency * 512;
	rate /= nYMZ280BSampleRate * 3;

	YMZ280BChannelInfo[nChannel].nSampleSize = (UINT32)rate;
}

INT32 YMZ280BScan()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YMZ280BInitted) bprintf(PRINT_ERROR, _T("YMZ280BScan called without init\n"));
#endif

	SCAN_VAR(nYMZ280BStatus);
	SCAN_VAR(nYMZ280BRegister);

	SCAN_VAR(bYMZ280BEnable);

	SCAN_VAR(bYMZ280BIRQEnable);
	SCAN_VAR(nYMZ280BIRQMask);
	SCAN_VAR(nYMZ280BIRQStatus);
	SCAN_VAR(nRamReadAddress);

	for (INT32 j = 0; j < 8; j++) {
		SCAN_VAR(YMZ280BChannelInfo[j]);
		YMZ280BSetSampleSize(j);
	}

	return 0;
}

INT32 YMZ280BInit(INT32 nClock, void (*IRQCallback)(INT32))
{
	DebugSnd_YMZ280BInitted = 1;
	
	nYMZ280BFrequency = nClock;

	if (nBurnSoundRate > 0) {
		nYMZ280BSampleRate = nBurnSoundRate;
	} else {
		nYMZ280BSampleRate = 11025;
	}

	// Compute sample deltas
	for (INT32 n = 0; n < 16; n++) {
		nDelta = (n & 7) * 2 + 1;
		if (n & 8) {
			nDelta = -nDelta;
		}
		YMZ280BDeltaTable[n] = nDelta;
	}

	YMZ280BIRQCallback = IRQCallback;

	if (pBuffer) {
		free(pBuffer);
		pBuffer = NULL;
	}
	pBuffer = (INT32*)malloc(nYMZ280BSampleRate *  2 * sizeof(INT32));

	for (INT32 j = 0; j < 8; j++) {
		YMZ280BChannelData[j] = (INT32*)malloc(0x1000 * sizeof(INT32));
	}

	// default routes
	YMZ280BVolumes[BURN_SND_YMZ280B_YMZ280B_ROUTE_1] = 1.00;
	YMZ280BVolumes[BURN_SND_YMZ280B_YMZ280B_ROUTE_2] = 1.00;
	YMZ280BRouteDirs[BURN_SND_YMZ280B_YMZ280B_ROUTE_1] = BURN_SND_ROUTE_LEFT;
	YMZ280BRouteDirs[BURN_SND_YMZ280B_YMZ280B_ROUTE_2] = BURN_SND_ROUTE_RIGHT;

	YMZ280BReset();

	return 0;
}

void YMZ280BSetRoute(INT32 nIndex, double nVolume, INT32 nRouteDir)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YMZ280BInitted) bprintf(PRINT_ERROR, _T("BurnYMZ280BSetRoute called without init\n"));
	if (nIndex < 0 || nIndex > 1) bprintf(PRINT_ERROR, _T("BurnYMZ280BSetRoute called with invalid index %i\n"), nIndex);
#endif

	YMZ280BVolumes[nIndex] = nVolume;
	YMZ280BRouteDirs[nIndex] = nRouteDir;
}

void YMZ280BExit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YMZ280BInitted) bprintf(PRINT_ERROR, _T("YMZ280BExit called without init\n"));
#endif

	if (pBuffer) {
		free(pBuffer);
		pBuffer = NULL;
	}

	YMZ280BIRQCallback = NULL;
	pYMZ280BRAMWrite = NULL;
	pYMZ280BRAMRead = NULL;
	
	DebugSnd_YMZ280BInitted = 0;
}

inline static void UpdateIRQStatus()
{
	nYMZ280BIRQStatus = 0;
	if (bYMZ280BIRQEnable && (nYMZ280BStatus & nYMZ280BIRQMask)) {
		nYMZ280BIRQStatus = 1;
	}

	if ((YMZ280BIRQCallback != NULL)) {
		YMZ280BIRQCallback(nYMZ280BIRQStatus);
	}
}

inline static void ComputeVolume(sYMZ280BChannelInfo* channel)
{
	if (channel->nPan == 8) {
		channel->nVolumeLeft = channel->nVolume;
		channel->nVolumeRight = channel->nVolume;
	} else {
		if (channel->nPan < 8) {
			channel->nVolumeLeft = channel->nVolume;
			channel->nVolumeRight = channel->nVolume * channel->nPan / 8;
		} else {
			channel->nVolumeLeft = channel->nVolume * (15 - channel->nPan) / 8;
			channel->nVolumeRight = channel->nVolume;
		}
	}
}

inline static void RampChannel()
{
#if 1
	if (channelInfo->nSample != 0) {
		if (channelInfo->nSample > 0) {
			INT32 nRamp = 64 * 32678 / nYMZ280BSampleRate;
			while (nCount-- && channelInfo->nSample > nRamp) {
				channelInfo->nSample -= nRamp;
				*buf++ += channelInfo->nSample * channelInfo->nVolumeLeft;
				*buf++ += channelInfo->nSample * channelInfo->nVolumeRight;
			}
			if (channelInfo->nSample < nRamp) {
				channelInfo->nSample = 0;
			}
		} else {
			INT32 nRamp = 0 - 64 * 32678 / nYMZ280BSampleRate;
			while (nCount-- && channelInfo->nSample < nRamp) {
				channelInfo->nSample -= nRamp;
				*buf++ += channelInfo->nSample * channelInfo->nVolumeLeft;
				*buf++ += channelInfo->nSample * channelInfo->nVolumeRight;
			}
			if (channelInfo->nSample > nRamp) {
				channelInfo->nSample = 0;
			}
		}
	}
#endif
}

inline static void decode_adpcm()
{
	// Get next value & compute delta
	nDelta = YMZ280BROM[channelInfo->nPosition >> 1];
	if (channelInfo->nPosition & 1) {
		nDelta &= 0x0F;
	} else {
		nDelta >>= 4;
	}

	nSample = channelInfo->nSample + channelInfo->nStep * YMZ280BDeltaTable[nDelta] / 8;
	if (nSample > 32767) {
		nSample = 32767;
	} else {
		if (nSample < -32768) {
			nSample = -32768;
		}
	}
	channelInfo->nSample = nSample;

	channelInfo->nStep = channelInfo->nStep * YMZ280BStepShift[nDelta & 7] / 256;
	if (channelInfo->nStep > 0x6000) {
		channelInfo->nStep = 0x6000;
	} else {
		if (channelInfo->nStep < 127) {
			channelInfo->nStep = 127;
		}
	}

	channelInfo->nPosition++;
}

inline static void decode_pcm8()
{
	nDelta = YMZ280BROM[channelInfo->nPosition >> 1];

	channelInfo->nSample = (INT8)nDelta * 256;
	channelInfo->nPosition+=2;
}

inline static void decode_pcm16()
{
	nDelta = (INT16)((YMZ280BROM[channelInfo->nPosition / 2 + 1] << 8) + YMZ280BROM[channelInfo->nPosition / 2]);

	channelInfo->nSample = nDelta;
	channelInfo->nPosition+=4;
}

inline static void decode_none()
{
	channelInfo->nSample=0;
}

static void (*decode_table[4])() = { decode_none, decode_adpcm, decode_pcm8, decode_pcm16 };

inline static void ComputeOutput_Linear()
{
	nSample = channelInfo->nPreviousOutput + (channelInfo->nOutput - channelInfo->nPreviousOutput) * (channelInfo->nFractionalPosition >> 12) / (0x01000000 >> 12);

	*buf++ += nSample * channelInfo->nVolumeLeft;
	*buf++ += nSample * channelInfo->nVolumeRight;
}

inline static void ComputeOutput_Cubic()
{
	if (channelInfo->nBufPos > 0x0FF0) {
		YMZ280BChannelData[nActiveChannel][0] = YMZ280BChannelData[nActiveChannel][channelInfo->nBufPos - 4];
		YMZ280BChannelData[nActiveChannel][1] = YMZ280BChannelData[nActiveChannel][channelInfo->nBufPos - 3];
		YMZ280BChannelData[nActiveChannel][2] = YMZ280BChannelData[nActiveChannel][channelInfo->nBufPos - 2];
		YMZ280BChannelData[nActiveChannel][3] = YMZ280BChannelData[nActiveChannel][channelInfo->nBufPos - 1];
		channelInfo->nBufPos = 4;
	}

	nSample = INTERPOLATE4PS_16BIT(channelInfo->nFractionalPosition >> 12,
								   YMZ280BChannelData[nActiveChannel][channelInfo->nBufPos - 4],
								   YMZ280BChannelData[nActiveChannel][channelInfo->nBufPos - 3],
								   YMZ280BChannelData[nActiveChannel][channelInfo->nBufPos - 2],
								   YMZ280BChannelData[nActiveChannel][channelInfo->nBufPos - 1]);

	*buf++ += nSample * channelInfo->nVolumeLeft;
	*buf++ += nSample * channelInfo->nVolumeRight;
}

inline static void RenderADPCM_Linear()
{
	while (nCount--) {
		if (channelInfo->nFractionalPosition >= 0x01000000) {

			channelInfo->nPreviousOutput = channelInfo->nOutput;

			do {
				// Check for end of sample
				if (channelInfo->nPosition >= channelInfo->nSampleStop) {
					channelInfo->bPlaying = false;

					RampChannel();

					nYMZ280BStatus |= 1 << nActiveChannel;
					UpdateIRQStatus();

					return;
				} else {

					decode_table[YMZ280BChannelInfo[nActiveChannel].nMode](); // decode one sample

					// Advance sample position
					channelInfo->nFractionalPosition -= 0x01000000;
				}

			} while (channelInfo->nFractionalPosition >= 0x01000000);

			channelInfo->nOutput = channelInfo->nSample;
		}

		ComputeOutput_Linear();

		channelInfo->nFractionalPosition += channelInfo->nSampleSize;
	}
}

inline static void RenderADPCMLoop_Linear()
{
	while (nCount--) {
		if (channelInfo->nFractionalPosition >= 0x01000000) {

			channelInfo->nPreviousOutput = channelInfo->nOutput;

			do {
				// Check for end of sample
				if (channelInfo->nPosition == channelInfo->nLoopStop) {
					channelInfo->nStep = channelInfo->nLoopStep;
					channelInfo->nSample = channelInfo->nLoopSample;
					channelInfo->nPosition = channelInfo->nLoopStart;
				} else {
					// Store the state of the channel at the point where the loop starts
					if (channelInfo->nPosition == channelInfo->nLoopStart) {
						channelInfo->nLoopStep = channelInfo->nStep;
						channelInfo->nLoopSample = channelInfo->nSample;
					}
				}

				decode_table[YMZ280BChannelInfo[nActiveChannel].nMode](); // decode one sample

				// Advance sample position
				channelInfo->nFractionalPosition -= 0x01000000;

			} while (channelInfo->nFractionalPosition >= 0x01000000);

			channelInfo->nOutput = channelInfo->nSample;
		}

		ComputeOutput_Linear();

		channelInfo->nFractionalPosition += channelInfo->nSampleSize;
	}
}
inline static void RenderADPCM_Cubic()
{
	while (nCount--) {
		while (channelInfo->nFractionalPosition >= 0x01000000) {

			// Check for end of sample
			if (channelInfo->nPosition >= channelInfo->nSampleStop) {
				channelInfo->bPlaying = false;

				RampChannel();

				nYMZ280BStatus |= 1 << nActiveChannel;
				UpdateIRQStatus();

				return;
			} else {

				decode_table[YMZ280BChannelInfo[nActiveChannel].nMode](); // decode one sample

				// Advance sample position
				channelInfo->nFractionalPosition -= 0x01000000;

				YMZ280BChannelData[nActiveChannel][channelInfo->nBufPos++] = channelInfo->nSample;
			}
		}

		ComputeOutput_Cubic();

		channelInfo->nFractionalPosition += channelInfo->nSampleSize;
	}
}

inline static void RenderADPCMLoop_Cubic()
{
	while (nCount--) {

		while (channelInfo->nFractionalPosition >= 0x01000000) {
			// Check for end of sample
			if (channelInfo->nPosition >= channelInfo->nLoopStop) {

				channelInfo->nStep = channelInfo->nLoopStep;
				channelInfo->nSample = channelInfo->nLoopSample;
				channelInfo->nPosition = channelInfo->nLoopStart;
			} else {
				// Store the state of the channel at the point where the loop starts
				if (channelInfo->nPosition == channelInfo->nLoopStart) {
					channelInfo->nLoopStep = channelInfo->nStep;
					channelInfo->nLoopSample = channelInfo->nSample;
				}
			}

			decode_table[YMZ280BChannelInfo[nActiveChannel].nMode](); // decode one sample

			// Advance sample position
			channelInfo->nFractionalPosition -= 0x01000000;

			YMZ280BChannelData[nActiveChannel][channelInfo->nBufPos++] = channelInfo->nSample;
		}

		ComputeOutput_Cubic();

		channelInfo->nFractionalPosition += channelInfo->nSampleSize;
	}
}

INT32 YMZ280BRender(INT16* pSoundBuf, INT32 nSegmentLength)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YMZ280BInitted) bprintf(PRINT_ERROR, _T("YMZ280BRender called without init\n"));
#endif

	memset(pBuffer, 0, nSegmentLength * 2 * sizeof(INT32));

	for (nActiveChannel = 0; nActiveChannel < 8; nActiveChannel++) {
		nCount = nSegmentLength;
		buf = pBuffer;
		channelInfo = &YMZ280BChannelInfo[nActiveChannel];

		if (channelInfo->bPlaying) {
			if (nInterpolation < 3) {
				if (channelInfo->bEnabled && channelInfo->bLoop) {
					RenderADPCMLoop_Linear();
				} else {
					RenderADPCM_Linear();
				}
			} else {
				if (channelInfo->bEnabled && channelInfo->bLoop) {
					RenderADPCMLoop_Cubic();
				} else {
					RenderADPCM_Cubic();
				}
			}
		} else {
			RampChannel();
		}
	}

	for (INT32 i = 0; i < nSegmentLength; i++) {
		INT32 nLeftSample = 0, nRightSample = 0;
			
		if ((YMZ280BRouteDirs[BURN_SND_YMZ280B_YMZ280B_ROUTE_1] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)((pBuffer[(i << 1) + 0] >> 8) * YMZ280BVolumes[BURN_SND_YMZ280B_YMZ280B_ROUTE_1]);
		}
		if ((YMZ280BRouteDirs[BURN_SND_YMZ280B_YMZ280B_ROUTE_1] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)((pBuffer[(i << 1) + 0] >> 8) * YMZ280BVolumes[BURN_SND_YMZ280B_YMZ280B_ROUTE_1]);
		}
			
		if ((YMZ280BRouteDirs[BURN_SND_YMZ280B_YMZ280B_ROUTE_2] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)((pBuffer[(i << 1) + 1] >> 8) * YMZ280BVolumes[BURN_SND_YMZ280B_YMZ280B_ROUTE_2]);
		}
		if ((YMZ280BRouteDirs[BURN_SND_YMZ280B_YMZ280B_ROUTE_2] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)((pBuffer[(i << 1) + 1] >> 8) * YMZ280BVolumes[BURN_SND_YMZ280B_YMZ280B_ROUTE_2]);
		}
			
		pSoundBuf[(i << 1) + 0] = BURN_SND_CLIP(nLeftSample);
		pSoundBuf[(i << 1) + 1] = BURN_SND_CLIP(nRightSample);
	}

	return 0;
}

void YMZ280BWriteRegister(UINT8 nValue)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YMZ280BInitted) bprintf(PRINT_ERROR, _T("YMZ280BWriteRegister called without init\n"));
#endif

	if (nYMZ280BRegister < 0x80) {
		INT32 nWriteChannel = (nYMZ280BRegister >> 2) & 0x07;

		switch (nYMZ280BRegister & 0x63) {

			// Miscellaneous
			case 0:															// Frequency
				YMZ280BChannelInfo[nWriteChannel].nFrequency &= 0x0100;
				YMZ280BChannelInfo[nWriteChannel].nFrequency |= nValue;
				YMZ280BSetSampleSize(nWriteChannel);
				break;
			case 1:	{														// Start/mode/freq
				YMZ280BChannelInfo[nWriteChannel].nFrequency &= 0x00FF;
				YMZ280BChannelInfo[nWriteChannel].nFrequency |= ((nValue & 1) << 8);
				YMZ280BSetSampleSize(nWriteChannel);

				YMZ280BChannelInfo[nWriteChannel].bLoop = (nValue & 0x10);
				YMZ280BChannelInfo[nWriteChannel].nMode = ((nValue >> 5) & 0x03);

				if ((nValue & 0x80) == 0) {
					YMZ280BChannelInfo[nWriteChannel].bEnabled = false;
					if (!YMZ280BChannelInfo[nWriteChannel].bLoop) {
						YMZ280BChannelInfo[nWriteChannel].bPlaying = false;
					}
				} else {
					if (!YMZ280BChannelInfo[nWriteChannel].bEnabled) {
						YMZ280BChannelInfo[nWriteChannel].bEnabled = true;
						YMZ280BChannelInfo[nWriteChannel].bPlaying = true;
						YMZ280BChannelInfo[nWriteChannel].nPosition = YMZ280BChannelInfo[nWriteChannel].nSampleStart;
						YMZ280BChannelInfo[nWriteChannel].nStep = 127;

						if (YMZ280BChannelInfo[nWriteChannel].nMode > 1) {
#ifdef DEBUG
		//					bprintf(0,_T("Sample Start: %08X - Stop: %08X.\n"),YMZ280BChannelInfo[nWriteChannel].nSampleStart, YMZ280BChannelInfo[nWriteChannel].nSampleStop);
#endif
						}

#if 0
						if (nInterpolation < 3) {
							YMZ280BChannelInfo[nWriteChannel].nFractionalPosition = 0;
							YMZ280BChannelInfo[nWriteChannel].nPreviousOutput = YMZ280BChannelInfo[nWriteChannel].nSample;
							YMZ280BChannelInfo[nWriteChannel].nOutput = YMZ280BChannelInfo[nWriteChannel].nSample;
						} else {
							YMZ280BChannelInfo[nWriteChannel].nFractionalPosition = 0x03000000;
							YMZ280BChannelData[nWriteChannel][0] = YMZ280BChannelInfo[nWriteChannel].nSample;
							YMZ280BChannelInfo[nWriteChannel].nBufPos = 1;
						}
#else
						YMZ280BChannelInfo[nWriteChannel].nSample = 0;

						if (nInterpolation < 3) {
							YMZ280BChannelInfo[nWriteChannel].nFractionalPosition = 0;
							YMZ280BChannelInfo[nWriteChannel].nPreviousOutput = 0;
							YMZ280BChannelInfo[nWriteChannel].nOutput = 0;
						} else {
							YMZ280BChannelInfo[nWriteChannel].nFractionalPosition = 0x03000000;
							YMZ280BChannelData[nWriteChannel][3] = 0;
							YMZ280BChannelInfo[nWriteChannel].nBufPos = 1;
						}
#endif
					}
				}

				break;
			}
			case 2:																	// Volume
				YMZ280BChannelInfo[nWriteChannel].nVolume = nValue;
				ComputeVolume(&YMZ280BChannelInfo[nWriteChannel]);
				break;
			case 3:																	// Pan
				YMZ280BChannelInfo[nWriteChannel].nPan = nValue & 0x0F;
				ComputeVolume(&YMZ280BChannelInfo[nWriteChannel]);
				break;

			// Hi bits
			case 0x20:
				YMZ280BChannelInfo[nWriteChannel].nSampleStart &= 0x0001FFFE;
				YMZ280BChannelInfo[nWriteChannel].nSampleStart |= (nValue << 17);
				break;
			case 0x21:
				YMZ280BChannelInfo[nWriteChannel].nLoopStart &= 0x0001FFFE;
				YMZ280BChannelInfo[nWriteChannel].nLoopStart |= (nValue << 17);
				break;
			case 0x22:
				YMZ280BChannelInfo[nWriteChannel].nLoopStop &= 0x0001FFFE;
				YMZ280BChannelInfo[nWriteChannel].nLoopStop |= (nValue << 17);
				break;
			case 0x23:
				YMZ280BChannelInfo[nWriteChannel].nSampleStop &= 0x0001FFFE;
				YMZ280BChannelInfo[nWriteChannel].nSampleStop |= (nValue << 17);
				break;

			// Mid bits
			case 0x40:
				YMZ280BChannelInfo[nWriteChannel].nSampleStart &= 0x01FE01FE;
				YMZ280BChannelInfo[nWriteChannel].nSampleStart |= (nValue << 9);
				break;
			case 0x41:
				YMZ280BChannelInfo[nWriteChannel].nLoopStart &= 0x01FE01FE;
				YMZ280BChannelInfo[nWriteChannel].nLoopStart |= (nValue << 9);
				break;
			case 0x42:
				YMZ280BChannelInfo[nWriteChannel].nLoopStop &= 0x01FE01FE;
				YMZ280BChannelInfo[nWriteChannel].nLoopStop |= (nValue << 9);
				break;
			case 0x43:
				YMZ280BChannelInfo[nWriteChannel].nSampleStop &= 0x01FE01FE;
				YMZ280BChannelInfo[nWriteChannel].nSampleStop |= (nValue << 9);
				break;

			// Lo bits
			case 0x60:
				YMZ280BChannelInfo[nWriteChannel].nSampleStart &= 0x01FFFE00;
				YMZ280BChannelInfo[nWriteChannel].nSampleStart |= (nValue << 1);
				break;
			case 0x61:
				YMZ280BChannelInfo[nWriteChannel].nLoopStart &= 0x01FFFE00;
				YMZ280BChannelInfo[nWriteChannel].nLoopStart |= (nValue << 1);
				break;
			case 0x62:
				YMZ280BChannelInfo[nWriteChannel].nLoopStop &= 0x01FFFE00;
				YMZ280BChannelInfo[nWriteChannel].nLoopStop |= (nValue << 1);
				break;
			case 0x63:
				YMZ280BChannelInfo[nWriteChannel].nSampleStop &= 0x01FFFE00;
				YMZ280BChannelInfo[nWriteChannel].nSampleStop |= (nValue << 1);
				break;

		}
   	} else {
		switch (nYMZ280BRegister)
		{
			case 0x84:	// ROM readback / RAM write (high)
				nRamReadAddress &= 0x00ffff;
				nRamReadAddress |= (nValue << 16);
				break;

			case 0x85:	// ROM readback / RAM write (med)
				nRamReadAddress &= 0xff00ff;
				nRamReadAddress |= (nValue <<  8);
				break;

			case 0x86:	// ROM readback / RAM write (low)
				nRamReadAddress &= 0xffff00;
				nRamReadAddress |= (nValue <<  0);
				break;

			case 0x87:	// RAM write
				if (pYMZ280BRAMWrite) {
					pYMZ280BRAMWrite(nRamReadAddress, nValue);
				}
				break;

			case 0xfe:	// Set IRQ mask
				nYMZ280BIRQMask = nValue;
				UpdateIRQStatus();
				break;

			case 0xff:	// Start/stop playing, enable/disable IRQ
				{
					if (nValue & 0x10) {
						bYMZ280BIRQEnable = true;
					} else {
						bYMZ280BIRQEnable = false;
					}
					UpdateIRQStatus();
	
					if (bYMZ280BEnable && !(nValue & 0x80)) {
						bYMZ280BEnable = false;
						for (INT32 n = 0; n < 8; n++) {
							YMZ280BChannelInfo[n].bPlaying = false;
						}
					} else {
						if (!bYMZ280BEnable && (nValue & 0x80)) {
							bYMZ280BEnable = true;
							for (INT32 n = 0; n < 8; n++) {
								if (YMZ280BChannelInfo[n].bEnabled && YMZ280BChannelInfo[n].bLoop) {
									YMZ280BChannelInfo[n].bPlaying = true;
								}
							}
						}
					}
				}
				break;
		}
	}
}

UINT32 YMZ280BReadStatus()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YMZ280BInitted) bprintf(PRINT_ERROR, _T("YMZ280BReadStatus called without init\n"));
#endif

	UINT32 nStatus = nYMZ280BStatus;
	nYMZ280BStatus = 0;

	UpdateIRQStatus();

	return nStatus;
}

UINT32 YMZ280BReadRAM()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YMZ280BInitted) bprintf(PRINT_ERROR, _T("YMZ280BReadRAM called without init\n"));
#endif
	
	if (pYMZ280BRAMRead) {
		return pYMZ280BRAMRead(nRamReadAddress++ - 1);
	}

	return 0;
}
