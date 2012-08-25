#include "burnint.h"
#include "burn_sound.h"
#include "burn_ymf278b.h"

static INT32 (*BurnYMF278BStreamCallback)(INT32 nSoundRate);

static INT16* pBuffer;
static INT16* pYMF278BBuffer[2];

static INT32 nYMF278BPosition;
static INT32 nFractionalPosition;

static double YMF278BVolumes[2];
static INT32 YMF278BRouteDirs[2];

// ----------------------------------------------------------------------------
// Dummy functions

static INT32 YMF278BStreamCallbackDummy(INT32 /* nSoundRate */)
{
	return 0;
}


// ----------------------------------------------------------------------------
// Execute YMF278B for part of a frame

static void YMF278BRender(INT32 nSegmentLength)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YMF278BInitted) bprintf(PRINT_ERROR, _T("YMF278BRender called without init\n"));
#endif

	if (nYMF278BPosition >= nSegmentLength) {
		return;
	}

//	bprintf(PRINT_NORMAL, _T("    YMF278B render %6i -> %6i\n"), nYMF278BPosition, nSegmentLength);

	nSegmentLength -= nYMF278BPosition;

	pYMF278BBuffer[0] = pBuffer + 0 * 4096 + 4 + nYMF278BPosition;
	pYMF278BBuffer[1] = pBuffer + 1 * 4096 + 4 + nYMF278BPosition;

	ymf278b_pcm_update(0, pYMF278BBuffer, nSegmentLength);

	nYMF278BPosition += nSegmentLength;
}

// ----------------------------------------------------------------------------
// Update the sound buffer

void BurnYMF278BUpdate(INT32 nSegmentEnd)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YMF278BInitted) bprintf(PRINT_ERROR, _T("BurnYMF278BUpdate called without init\n"));
#endif

	INT16* pSoundBuf = pBurnSoundOut;
	INT32 nSegmentLength = nSegmentEnd;

//	bprintf(PRINT_NORMAL, _T("    YMF278B render %6i -> %6i\n"), nYMF278BPosition, nSegmentEnd);

	if (nBurnSoundRate == 0) {
		return;
	}

	if (nSegmentEnd < nYMF278BPosition) {
		nSegmentEnd = nYMF278BPosition;
	}

	if (nSegmentLength > nBurnSoundLen) {
		nSegmentLength = nBurnSoundLen;
	}

	YMF278BRender(nSegmentEnd);

	pYMF278BBuffer[0] = pBuffer + 0 * 4096 + 4;
	pYMF278BBuffer[1] = pBuffer + 1 * 4096 + 4;

	for (INT32 n = nFractionalPosition; n < nSegmentLength; n++) {
		INT32 nLeftSample = 0, nRightSample = 0;

		if ((YMF278BRouteDirs[BURN_SND_YMF278B_YMF278B_ROUTE_1] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(pYMF278BBuffer[0][n] * YMF278BVolumes[BURN_SND_YMF278B_YMF278B_ROUTE_1]);
		}
		if ((YMF278BRouteDirs[BURN_SND_YMF278B_YMF278B_ROUTE_1] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(pYMF278BBuffer[0][n] * YMF278BVolumes[BURN_SND_YMF278B_YMF278B_ROUTE_1]);
		}
		
		if ((YMF278BRouteDirs[BURN_SND_YMF278B_YMF278B_ROUTE_2] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(pYMF278BBuffer[1][n] * YMF278BVolumes[BURN_SND_YMF278B_YMF278B_ROUTE_2]);
		}
		if ((YMF278BRouteDirs[BURN_SND_YMF278B_YMF278B_ROUTE_2] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(pYMF278BBuffer[1][n] * YMF278BVolumes[BURN_SND_YMF278B_YMF278B_ROUTE_2]);
		}
		
		nLeftSample = BURN_SND_CLIP(nLeftSample);
		nRightSample = BURN_SND_CLIP(nRightSample);
			
		pSoundBuf[(n << 1) + 0] = nLeftSample;
		pSoundBuf[(n << 1) + 1] = nRightSample;
	}

	nFractionalPosition = nSegmentLength;

	if (nSegmentEnd >= nBurnSoundLen) {
		INT32 nExtraSamples = nSegmentEnd - nBurnSoundLen;

		for (INT32 i = 0; i < nExtraSamples; i++) {
			pYMF278BBuffer[0][i] = pYMF278BBuffer[0][nBurnSoundLen + i];
			pYMF278BBuffer[1][i] = pYMF278BBuffer[1][nBurnSoundLen + i];
		}

		nFractionalPosition = 0;

		nYMF278BPosition = nExtraSamples;

	}
}

// ----------------------------------------------------------------------------

void BurnYMF278BSelectRegister(INT32 nRegister, UINT8 nValue)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YMF278BInitted) bprintf(PRINT_ERROR, _T("BurnYMF278BSelectRegister called without init\n"));
#endif

	switch (nRegister) {
		case 0:
//			bprintf(PRINT_NORMAL, _T("    YMF278B register A -> %i\n"), nValue);
			YMF278B_control_port_0_A_w(nValue);
			break;
		case 1:
			YMF278B_control_port_0_B_w(nValue);
			break;
		case 2:
//			bprintf(PRINT_NORMAL, _T("    YMF278B register C -> %i\n"), nValue);
			YMF278B_control_port_0_C_w(nValue);
			break;
	}
}
void BurnYMF278BWriteRegister(INT32 nRegister, UINT8 nValue)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YMF278BInitted) bprintf(PRINT_ERROR, _T("BurnYMF278BWriteRegister called without init\n"));
#endif

	switch (nRegister) {
		case 0:
			BurnYMF278BUpdate(BurnYMF278BStreamCallback(nBurnSoundRate));
			YMF278B_data_port_0_A_w(nValue);
			break;
		case 1:
			YMF278B_data_port_0_B_w(nValue);
			break;
		case 2:
			BurnYMF278BUpdate(BurnYMF278BStreamCallback(nBurnSoundRate));
			YMF278B_data_port_0_C_w(nValue);
			break;
	}
}

UINT8 BurnYMF278BReadStatus()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YMF278BInitted) bprintf(PRINT_ERROR, _T("BurnYMF278BReadStatus called without init\n"));
#endif

	BurnYMF278BUpdate(BurnYMF278BStreamCallback(nBurnSoundRate));
	return YMF278B_status_port_0_r();
}

UINT8 BurnYMF278BReadData()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YMF278BInitted) bprintf(PRINT_ERROR, _T("BurnYMF278BReadData called without init\n"));
#endif

	return YMF278B_data_port_0_r();
}

// ----------------------------------------------------------------------------

void BurnYMF278BReset()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YMF278BInitted) bprintf(PRINT_ERROR, _T("BurnYMF278BReset called without init\n"));
#endif

	BurnTimerReset();
}

void BurnYMF278BExit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YMF278BInitted) bprintf(PRINT_ERROR, _T("BurnYMF278BExit called without init\n"));
#endif

	YMF278B_sh_stop();

	BurnTimerExit();

	if (pBuffer) {
		free(pBuffer);
		pBuffer = NULL;
	}
	
	DebugSnd_YMF278BInitted = 0;
}

INT32 BurnYMF278BInit(INT32 /* nClockFrequency */, UINT8* YMF278BROM, void (*IRQCallback)(INT32, INT32), INT32 (*StreamCallback)(INT32))
{
	DebugSnd_YMF278BInitted = 1;
	BurnYMF278BExit();	
	DebugSnd_YMF278BInitted = 1;

	BurnYMF278BStreamCallback = YMF278BStreamCallbackDummy;
	if (StreamCallback) {
		BurnYMF278BStreamCallback = StreamCallback;
	}

	ymf278b_start(0, YMF278BROM, IRQCallback, BurnYMFTimerCallback, YMF278B_STD_CLOCK, nBurnSoundRate);
	BurnTimerInit(ymf278b_timer_over, NULL);

	pBuffer = (INT16*)malloc(4096 * 2 * sizeof(INT16));
	memset(pBuffer, 0, 4096 * 2 * sizeof(INT16));

	nYMF278BPosition = 0;

	nFractionalPosition = 0;
	
	// default routes
	YMF278BVolumes[BURN_SND_YMF278B_YMF278B_ROUTE_1] = 1.00;
	YMF278BVolumes[BURN_SND_YMF278B_YMF278B_ROUTE_2] = 1.00;
	YMF278BRouteDirs[BURN_SND_YMF278B_YMF278B_ROUTE_1] = BURN_SND_ROUTE_LEFT;
	YMF278BRouteDirs[BURN_SND_YMF278B_YMF278B_ROUTE_2] = BURN_SND_ROUTE_RIGHT;

	return 0;
}

void BurnYMF278BSetRoute(INT32 nIndex, double nVolume, INT32 nRouteDir)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YMF278BInitted) bprintf(PRINT_ERROR, _T("BurnYMF278BSetRoute called without init\n"));
	if (nIndex < 0 || nIndex > 1) bprintf(PRINT_ERROR, _T("BurnYMF278BSetRoute called with invalid index %i\n"), nIndex);
#endif

	YMF278BVolumes[nIndex] = nVolume;
	YMF278BRouteDirs[nIndex] = nRouteDir;
}

void BurnYMF278BScan(INT32 nAction, INT32* pnMin)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YMF278BInitted) bprintf(PRINT_ERROR, _T("BurnYMF278BScan called without init\n"));
#endif

	BurnTimerScan(nAction, pnMin);
}
